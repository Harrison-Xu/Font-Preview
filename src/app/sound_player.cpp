/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#include "sound_player.h"

#include "asset_manager.h"
#include "logger.h"
#include "sdl_mixer_compat.h"

namespace app {
namespace {

constexpr uint32_t kSdlInitAudio = 0x00000010U;
constexpr uint16_t kAudioS16LittleEndian = 0x8010U;
constexpr int kSampleRate = 22050;
constexpr int kOutputChannels = 2;
constexpr int kBufferSamples = 512;
constexpr int kPlaybackChannel = 0;
constexpr int kMixerVolume = 26; // About 20% of SDL_mixer's 0-128 range.

constexpr std::array<const char*, static_cast<std::size_t>(SoundCue::Count)> kCueAssets = {
    "audio/uisfx-arcade-select.wav",
    "audio/uisfx-arcade-blocked.wav",
    "audio/uisfx-arcade-long-press.wav",
};

} // namespace

SoundPlayer::SoundPlayer(AssetManager& assets) : assets_(assets) {}

SoundPlayer::~SoundPlayer() {
    shutdown();
}

bool SoundPlayer::initialize() {
    if (available_) return true;

    if (SDL_InitSubSystem(kSdlInitAudio) != 0) {
        LOG_WARN("failed to initialize SDL audio: {}", SDL_GetError());
        return false;
    }
    audio_subsystem_initialized_ = true;

    if (Mix_OpenAudio(kSampleRate, kAudioS16LittleEndian, kOutputChannels, kBufferSamples) != 0) {
        LOG_WARN("failed to open SDL_mixer audio: {}", SDL_GetError());
        shutdown();
        return false;
    }
    mixer_open_ = true;
    Mix_AllocateChannels(1);
    Mix_Volume(kPlaybackChannel, kMixerVolume);

    for (std::size_t index = 0; index < kCueAssets.size(); ++index) {
        const auto path = assets_.resolve(kCueAssets[index]);
        if (path.empty()) {
            LOG_WARN("UI sound asset not found: {}", kCueAssets[index]);
            shutdown();
            return false;
        }

        auto* source = SDL_RWFromFile(path.string().c_str(), "rb");
        if (!source) {
            LOG_WARN("failed to open UI sound {}: {}", path.string(), SDL_GetError());
            shutdown();
            return false;
        }

        chunks_[index] = Mix_LoadWAV_RW(source, 1);
        if (!chunks_[index]) {
            LOG_WARN("failed to load UI sound {}: {}", path.string(), SDL_GetError());
            shutdown();
            return false;
        }
    }

    available_ = true;
    LOG_INFO("UISFX Arcade sounds ready");
    return true;
}

bool SoundPlayer::available() const {
    return available_;
}

void SoundPlayer::play(SoundCue cue) {
    if (!available_) return;

    const auto index = static_cast<std::size_t>(cue);
    if (index >= chunks_.size() || !chunks_[index]) return;

    Mix_HaltChannel(kPlaybackChannel);
    if (Mix_PlayChannelTimed(kPlaybackChannel, chunks_[index], 0, -1) < 0) {
        LOG_WARN("failed to play UISFX cue: {}", SDL_GetError());
    }
}

void SoundPlayer::shutdown() {
    if (mixer_open_) Mix_HaltChannel(-1);
    for (auto*& chunk : chunks_) {
        if (chunk) {
            Mix_FreeChunk(chunk);
            chunk = nullptr;
        }
    }
    if (mixer_open_) {
        Mix_CloseAudio();
        mixer_open_ = false;
    }
    if (audio_subsystem_initialized_) {
        SDL_QuitSubSystem(kSdlInitAudio);
        audio_subsystem_initialized_ = false;
    }
    available_ = false;
}

} // namespace app
