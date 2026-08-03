#include "linux_input.h"

#include <cstdint>
#include <cstring>

#if !USE_DESKTOP
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#ifndef APP_KEY_INPUT_DEVICE
#define APP_KEY_INPUT_DEVICE ""
#endif

namespace platform {
namespace {

KeyEventHandler key_event_handler = nullptr;
void* key_event_user_data = nullptr;

struct KeyRouterState {
    uint32_t last_key{0};
    bool last_key_pressed{false};
};

void key_event_cb(lv_event_t* event) {
    auto* router = static_cast<KeyRouterState*>(lv_event_get_user_data(event));
    if (!router) return;

    auto* indev = lv_indev_active();
    if (!indev) {
        return;
    }

    const auto key = lv_indev_get_key(indev);
    const bool pressed = lv_indev_get_state(indev) == LV_INDEV_STATE_PRESSED;

    if (pressed && (!router->last_key_pressed || router->last_key != key)) {
        if (key_event_handler) key_event_handler(key, true, key_event_user_data);
    }
    else if (!pressed && router->last_key_pressed && router->last_key == key) {
        if (key_event_handler) key_event_handler(key, false, key_event_user_data);
    }

    router->last_key = key;
    router->last_key_pressed = pressed;
}

void key_router_delete_cb(lv_event_t* event) {
    delete static_cast<KeyRouterState*>(lv_event_get_user_data(event));
}

#if !USE_DESKTOP
struct EvdevKeypad {
    int fd{-1};
    lv_indev_state_t state{LV_INDEV_STATE_RELEASED};
    uint32_t key{0};
};

uint32_t map_evdev_key(uint16_t code) {
    switch (code) {
        case KEY_ESC:
            return LV_KEY_ESC;
        case KEY_UP:
        case KEY_F:
            return LV_KEY_UP;
        case KEY_DOWN:
        case KEY_X:
            return LV_KEY_DOWN;
        case KEY_LEFT:
        case KEY_Z:
            return LV_KEY_LEFT;
        case KEY_RIGHT:
        case KEY_C:
            return LV_KEY_RIGHT;
        case KEY_L:
            return 'l';
        case KEY_M:
            return 'm';
        default:
            return 0;
    }
}

bool has_nav_keys(int fd) {
    unsigned long key_bits[(KEY_MAX / (sizeof(unsigned long) * 8)) + 1] = {};
    if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits) < 0) {
        return false;
    }

    auto has_key = [&](int code) {
        const auto bits_per_word = static_cast<int>(sizeof(unsigned long) * 8);
        return (key_bits[code / bits_per_word] & (1UL << (code % bits_per_word))) != 0;
    };

    return has_key(KEY_ESC) || has_key(KEY_F) || has_key(KEY_X) || has_key(KEY_Z) || has_key(KEY_C) ||
           has_key(KEY_L) || has_key(KEY_M) ||
           has_key(KEY_UP) || has_key(KEY_DOWN) || has_key(KEY_LEFT) || has_key(KEY_RIGHT);
}

void evdev_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    auto* keypad = static_cast<EvdevKeypad*>(lv_indev_get_driver_data(indev));
    if (!keypad) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    input_event input{};
    while (read(keypad->fd, &input, sizeof(input)) == sizeof(input)) {
        if (input.type != EV_KEY) {
            continue;
        }

        const auto key = map_evdev_key(input.code);
        if (!key || input.value == 2) {
            continue;
        }

        keypad->key = key;
        keypad->state = input.value ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        data->continue_reading = true;
        break;
    }

    data->key = keypad->key;
    data->state = keypad->state;
}

void evdev_delete_cb(lv_event_t* event) {
    auto* indev = static_cast<lv_indev_t*>(lv_event_get_target(event));
    auto* keypad = static_cast<EvdevKeypad*>(lv_indev_get_driver_data(indev));
    if (!keypad) {
        return;
    }

    if (keypad->fd >= 0) {
        close(keypad->fd);
    }
    delete keypad;
}

lv_indev_t* create_keypad_from_fd(int fd) {
    auto* keypad = new EvdevKeypad;
    keypad->fd = fd;

    auto* indev = lv_indev_create();
    if (!indev) {
        delete keypad;
        close(fd);
        return nullptr;
    }

    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev, evdev_read_cb);
    lv_indev_set_driver_data(indev, keypad);
    lv_indev_add_event_cb(indev, evdev_delete_cb, LV_EVENT_DELETE, nullptr);
    attach_key_router(indev);
    return indev;
}

lv_indev_t* try_create_keypad(const char* path) {
    if (!path || path[0] == '\0') {
        return nullptr;
    }

    const int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        LV_LOG_WARN("failed to open input device %s: %s", path, strerror(errno));
        return nullptr;
    }

    if (!has_nav_keys(fd)) {
        close(fd);
        return nullptr;
    }

    input_event stale{};
    while (read(fd, &stale, sizeof(stale)) == sizeof(stale)) {
    }

    LV_LOG_INFO("using evdev key input %s", path);
    return create_keypad_from_fd(fd);
}

void discover_keypads(lv_display_t* display) {
    const char* configured_device = APP_KEY_INPUT_DEVICE;
    if (configured_device[0] != '\0') {
        auto* indev = try_create_keypad(configured_device);
        if (indev) {
            lv_indev_set_display(indev, display);
        }
        return;
    }

    auto* dir = opendir("/dev/input");
    if (!dir) {
        LV_LOG_WARN("failed to open /dev/input: %s", strerror(errno));
        return;
    }

    while (auto* entry = readdir(dir)) {
        if (std::strncmp(entry->d_name, "event", 5) != 0) {
            continue;
        }

        std::string path = "/dev/input/";
        path += entry->d_name;
        auto* indev = try_create_keypad(path.c_str());
        if (indev) {
            lv_indev_set_display(indev, display);
        }
    }

    closedir(dir);
}
#endif

} // namespace

void init_key_input(lv_display_t* display) {
#if !USE_DESKTOP
    discover_keypads(display);
#else
    LV_UNUSED(display);
#endif
}

void attach_key_router(lv_indev_t* indev) {
    if (!indev || lv_indev_get_type(indev) != LV_INDEV_TYPE_KEYPAD) {
        return;
    }

    auto* router = new KeyRouterState;
    lv_indev_add_event_cb(indev, key_event_cb, LV_EVENT_KEY, router);
    lv_indev_add_event_cb(indev, key_router_delete_cb, LV_EVENT_DELETE, router);
}

void set_key_event_handler(KeyEventHandler handler, void* user_data) {
    key_event_handler = handler;
    key_event_user_data = user_data;
}

} // namespace platform
