/*
 * Minimal SDL2/SDL2_mixer declarations used by Font Preview.
 *
 * CardputerZero ships the runtime libraries but its BSP sysroot does not
 * include their development headers. These declarations cover only the
 * stable C ABI used by the application.
 */

#pragma once

#include <cstdint>

extern "C" {

typedef struct SDL_RWops SDL_RWops;
typedef struct Mix_Chunk Mix_Chunk;

int SDL_InitSubSystem(uint32_t flags);
void SDL_QuitSubSystem(uint32_t flags);
const char* SDL_GetError(void);
SDL_RWops* SDL_RWFromFile(const char* file, const char* mode);

int Mix_OpenAudio(int frequency, uint16_t format, int channels, int chunksize);
void Mix_CloseAudio(void);
int Mix_AllocateChannels(int numchans);
Mix_Chunk* Mix_LoadWAV_RW(SDL_RWops* src, int freesrc);
void Mix_FreeChunk(Mix_Chunk* chunk);
int Mix_PlayChannelTimed(int channel, Mix_Chunk* chunk, int loops, int ticks);
int Mix_HaltChannel(int channel);
int Mix_Volume(int channel, int volume);

} // extern "C"
