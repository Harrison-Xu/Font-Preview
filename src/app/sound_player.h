/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <array>
#include <cstddef>

struct Mix_Chunk;

namespace app {

class AssetManager;

enum class SoundCue {
    Select = 0,
    Blocked,
    LongPress,
    Count,
};

class SoundPlayer {
public:
    explicit SoundPlayer(AssetManager& assets);
    ~SoundPlayer();

    SoundPlayer(const SoundPlayer&) = delete;
    SoundPlayer& operator=(const SoundPlayer&) = delete;

    bool initialize();
    bool available() const;
    void play(SoundCue cue);

private:
    void shutdown();

    AssetManager& assets_;
    std::array<Mix_Chunk*, static_cast<std::size_t>(SoundCue::Count)> chunks_{};
    bool available_{false};
    bool audio_subsystem_initialized_{false};
    bool mixer_open_{false};
};

} // namespace app
