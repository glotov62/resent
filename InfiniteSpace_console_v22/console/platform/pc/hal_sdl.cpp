// HAL для ПК на SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <cstdlib>
#include "hal.h"
#include "engine.h"   // enum Button

namespace {
SDL_Window* g_win = nullptr;
SDL_Renderer* g_ren = nullptr;
SDL_Texture* g_tex = nullptr;
SDL_AudioDeviceID g_audio = 0;
bool g_quit = false;

struct AudioCtx { hal::AudioCallback cb; void* user; } g_actx;

void sdl_audio_cb(void* ud, Uint8* stream, int len) {
    auto* c = static_cast<AudioCtx*>(ud);
    c->cb(reinterpret_cast<int16_t*>(stream), len / (int)sizeof(int16_t), c->user);
}
} // namespace

namespace hal {

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return false;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");   // пиксельное масштабирование
    g_win = SDL_CreateWindow("Console (PC)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             SCREEN_W * 3, SCREEN_H * 3, SDL_WINDOW_RESIZABLE);
    if (!g_win) return false;
    g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED);
    if (!g_ren) g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_SOFTWARE);
    if (!g_ren) return false;
    SDL_RenderSetLogicalSize(g_ren, SCREEN_W, SCREEN_H);
    SDL_RenderSetIntegerScale(g_ren, SDL_FALSE);
    g_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING,
                              SCREEN_W, SCREEN_H);
    return g_tex != nullptr;
}

void shutdown() {
    if (g_audio) SDL_CloseAudioDevice(g_audio);
    if (g_tex) SDL_DestroyTexture(g_tex);
    if (g_ren) SDL_DestroyRenderer(g_ren);
    if (g_win) SDL_DestroyWindow(g_win);
    SDL_Quit();
}

void present(const uint16_t* fb) {
    SDL_UpdateTexture(g_tex, nullptr, fb, SCREEN_W * (int)sizeof(uint16_t));
    SDL_RenderClear(g_ren);
    SDL_RenderCopy(g_ren, g_tex, nullptr, nullptr);
    SDL_RenderPresent(g_ren);
}

void* alloc_framebuffer(size_t bytes) { return std::calloc(1, bytes); }

uint32_t poll_buttons() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) g_quit = true;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) g_quit = true;
    }
    const Uint8* k = SDL_GetKeyboardState(nullptr);
    uint32_t b = 0;
    if (k[SDL_SCANCODE_UP])    b |= BTN_UP;
    if (k[SDL_SCANCODE_DOWN])  b |= BTN_DOWN;
    if (k[SDL_SCANCODE_LEFT])  b |= BTN_LEFT;
    if (k[SDL_SCANCODE_RIGHT]) b |= BTN_RIGHT;
    if (k[SDL_SCANCODE_Z])     b |= BTN_A;
    if (k[SDL_SCANCODE_X])     b |= BTN_B;
    if (k[SDL_SCANCODE_RETURN]) b |= BTN_START;
    if (k[SDL_SCANCODE_BACKSPACE] || k[SDL_SCANCODE_RSHIFT]) b |= BTN_SELECT;
    return b;
}

bool quit_requested() { return g_quit; }

uint64_t micros() {
    return SDL_GetPerformanceCounter() * 1000000ull / SDL_GetPerformanceFrequency();
}

void sleep_us(uint32_t us) { SDL_Delay(us / 1000); }

bool audio_init(int sample_rate, AudioCallback cb, void* user) {
    g_actx = {cb, user};
    SDL_AudioSpec want{}, have{};
    want.freq = sample_rate;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = sdl_audio_cb;
    want.userdata = &g_actx;
    g_audio = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (!g_audio) { SDL_Log("audio: %s", SDL_GetError()); return false; }
    SDL_PauseAudioDevice(g_audio, 0);
    return true;
}

const char* storage_root() {
    const char* e = std::getenv("CONSOLE_SD");
    return e ? e : "./sdcard";
}

void exit_to_launcher() { g_quit = true; }

} // namespace hal
