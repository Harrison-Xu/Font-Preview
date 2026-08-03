/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstddef>

namespace model {

enum class PreviewField : std::size_t {
    Font = 0,
    Language,
    Typeface,
    Size,
    Weight,
    Color,
};

enum class PreviewFont : std::size_t {
    NotoCjk = 0,
    Go,
    Inter,
    DejaVu,
    JetBrainsMono,
};

enum class PreviewLanguage : std::size_t {
    SimplifiedChinese = 0,
    TraditionalChinese,
    Japanese,
    Korean,
    English,
    Portuguese,
    Czech,
    Greek,
    Russian,
    Code,
};

enum class PreviewTypeface : std::size_t {
    Sans = 0,
    SansItalic,
    Serif,
    SerifItalic,
    Mono,
    MonoItalic,
};

enum class PreviewWeight : std::size_t {
    Regular = 0,
    Bold,
};

enum class PreviewColor : std::size_t {
    Dark = 0,
    Light,
    Sepia,
    Terminal,
    Solarized,
    Navy,
    Amber,
    Slate,
};

class PreviewModel {
public:
    static constexpr int kMinimumFontSize = 8;
    static constexpr int kMaximumFontSize = 24;
    static constexpr std::size_t kFieldCount = 6;

    PreviewField selected_field() const;
    PreviewFont font() const;
    PreviewLanguage language() const;
    PreviewTypeface typeface() const;
    PreviewWeight weight() const;
    PreviewColor color() const;
    int font_size() const;

    void select_previous_field();
    void select_next_field();
    void select_previous_value();
    void select_next_value();

    const char* font_name() const;
    const char* language_name() const;
    const char* typeface_name() const;
    const char* weight_name() const;
    const char* color_name() const;
    const char* sample_text() const;
    const char* font_file_name() const;
    bool has_font_face() const;
    bool uses_synthetic_italic() const;

private:
    void change_value(int direction);

    PreviewField selected_field_{PreviewField::Font};
    PreviewFont font_{PreviewFont::NotoCjk};
    PreviewLanguage language_{PreviewLanguage::SimplifiedChinese};
    PreviewTypeface typeface_{PreviewTypeface::Sans};
    PreviewWeight weight_{PreviewWeight::Regular};
    PreviewColor color_{PreviewColor::Dark};
    int font_size_{20};
};

} // namespace model
