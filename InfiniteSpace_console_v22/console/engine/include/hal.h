// HAL: единственное место, где движок соприкасается с железом/ОС.
// Реализации: platform/pc/hal_sdl.cpp (SDL2) и platform/esp32/hal_esp32.cpp
#pragma once
#include <cstdint>
#include <cstddef>

namespace hal {

constexpr int SCREEN_W = 240;
constexpr int SCREEN_H = 320;

bool init();
void shutdown();

// Кадр RGB565, SCREEN_W*SCREEN_H пикселей
void present(const uint16_t* fb);
// Память под кадр (на ESP32 -> PSRAM)
void* alloc_framebuffer(size_t bytes);

// Битовая маска нажатых кнопок (см. enum Button). Также обрабатывает события ОС.
uint32_t poll_buttons();
bool quit_requested();

uint64_t micros();
void sleep_us(uint32_t us);

// Моно, int16. Колбэк вызывается из аудио-потока/задачи.
using AudioCallback = void (*)(int16_t* out, int frames, void* user);
bool audio_init(int sample_rate, AudioCallback cb, void* user);

// Корень "SD-карты": "/sd" на ESP32, ./sdcard на ПК (или переменная CONSOLE_SD)
const char* storage_root();

// Выход в лаунчер (ESP32: boot -> factory + restart; ПК: просто закрыть окно)
void exit_to_launcher();

} // namespace hal
