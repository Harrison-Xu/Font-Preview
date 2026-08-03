/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lvgl.h"
#include "preview_model.h"

#include <array>
#include <cstdint>

namespace app {
class AssetManager;
}

namespace view {

constexpr int kScreenWidth = 320;
constexpr int kScreenHeight = 170;
constexpr int kSidebarWidth = 78;

class PreviewScreen {
public:
    PreviewScreen(model::PreviewModel& model, app::AssetManager& assets);
    ~PreviewScreen();

    PreviewScreen(const PreviewScreen&) = delete;
    PreviewScreen& operator=(const PreviewScreen&) = delete;

    bool build();
    void handle_key(uint32_t key, bool pressed);
    lv_obj_t* root() const;

private:
    struct FieldRow {
        lv_obj_t* container{nullptr};
        lv_obj_t* caption{nullptr};
        lv_obj_t* value{nullptr};
    };

    void refresh();
    void apply_colors();
    void refresh_values();
    void position_sample();
    void scroll_preview(int delta);
    void start_size_repeat(uint32_t key);
    void stop_size_repeat();
    static void size_repeat_cb(lv_timer_t* timer);
    bool load_ui_fonts();
    FieldRow create_field_row(std::size_t index, const char* caption);

    model::PreviewModel& model_;
    app::AssetManager& assets_;
    lv_obj_t* root_{nullptr};
    lv_obj_t* sidebar_{nullptr};
    lv_obj_t* preview_panel_{nullptr};
    lv_obj_t* sample_{nullptr};
    std::array<FieldRow, model::PreviewModel::kFieldCount> rows_{};
    lv_font_t* ui_regular_{nullptr};
    lv_font_t* ui_small_{nullptr};
    lv_font_t* ui_bold_{nullptr};
    lv_timer_t* size_repeat_timer_{nullptr};
    uint32_t repeating_key_{0};
    int preview_scroll_y_{0};
};

} // namespace view
