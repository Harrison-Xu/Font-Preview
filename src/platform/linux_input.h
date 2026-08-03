#pragma once

#include "lvgl.h"

#include <cstdint>

namespace platform {

using KeyEventHandler = void (*)(uint32_t key, bool pressed, void* user_data);

void init_key_input(lv_display_t* display);
void attach_key_router(lv_indev_t* indev);
void set_key_event_handler(KeyEventHandler handler, void* user_data);

} // namespace platform
