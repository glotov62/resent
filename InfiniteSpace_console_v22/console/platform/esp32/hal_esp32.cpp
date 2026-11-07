// СКЕЛЕТ HAL для ESP32-S3 (ESP-IDF). Не проверялся на железе — заполнишь под свой дисплей.
// Сигнатуры те же, что в engine/include/hal.h, остальной движок менять не нужно.
#include "hal.h"
#include "engine.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace hal {

bool init() {
    // TODO: esp_lcd (SPI или i80) + DMA, подсветка
    // TODO: GPIO кнопок (pull-up, активный низкий), антидребезг
    // TODO: SD через SDMMC -> esp_vfs_fat_sdmmc_mount("/sd", ...)
    return true;
}
void shutdown() {}

void present(const uint16_t* fb) {
    // TODO: esp_lcd_panel_draw_bitmap(panel, 0, 0, SCREEN_W, SCREEN_H, fb);
    // Внимание: SPI-панели ждут big-endian RGB565 -> байтсвап (или SPI_DEVICE_... / LV-подход).
    (void)fb;
}

void* alloc_framebuffer(size_t bytes) {
    return heap_caps_calloc(1, bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

uint32_t poll_buttons() {
    uint32_t b = 0;
    // TODO: if (!gpio_get_level(PIN_UP)) b |= BTN_UP; ...
    return b;
}
bool quit_requested() { return false; }

uint64_t micros() { return (uint64_t)esp_timer_get_time(); }
void sleep_us(uint32_t us) { vTaskDelay(pdMS_TO_TICKS(us / 1000)); }

bool audio_init(int sample_rate, AudioCallback cb, void* user) {
    // TODO: i2s_new_channel + std-режим 16 бит моно на MAX98357A,
    // задача на втором ядре: cb(buf, N, user); i2s_channel_write(...)
    (void)sample_rate; (void)cb; (void)user;
    return true;
}

const char* storage_root() { return "/sd"; }

void exit_to_launcher() {
    const esp_partition_t* f = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, nullptr);
    if (f) esp_ota_set_boot_partition(f);
    esp_restart();
}

} // namespace hal

// Точка входа (в игре тот же create_game())
extern "C" void app_main() {
    static Engine engine;
    Game* game = create_game();
    engine.run(*game);
}
