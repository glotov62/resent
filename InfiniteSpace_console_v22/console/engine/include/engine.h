#pragma once
#include <atomic>
#include <cstdint>
#include <string>
#include "hal.h"

enum Button : uint32_t {
    BTN_UP = 1u << 0, BTN_DOWN = 1u << 1, BTN_LEFT = 1u << 2, BTN_RIGHT = 1u << 3,
    BTN_A = 1u << 4,  BTN_B = 1u << 5,    BTN_START = 1u << 6, BTN_SELECT = 1u << 7,
};

struct Input {
    uint32_t down = 0, pressed = 0, released = 0;
    bool held(uint32_t b) const { return (down & b) != 0; }
    bool hit(uint32_t b) const { return (pressed & b) != 0; }   // нажата в этом кадре
    void update(uint32_t now) { pressed = now & ~down; released = down & ~now; down = now; }
};

struct Sprite {
    int w, h;
    const uint16_t* pixels;   // RGB565, w*h
    uint16_t key;             // цвет прозрачности
};

class Renderer {
public:
    static constexpr int W = hal::SCREEN_W, H = hal::SCREEN_H;
    static constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
        return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
    }

    Renderer();
    void clear(uint16_t c);
    void pixel(int x, int y, uint16_t c);
    void fillRect(int x, int y, int w, int h, uint16_t c);
    void rect(int x, int y, int w, int h, uint16_t c);
    void sprite(const Sprite& s, int x, int y, bool flipX = false);
    // Шрифт 3x5, только A-Z 0-9 и - : . (строчные -> заглавные)
    void text(int x, int y, const char* str, uint16_t c, int scale = 1);

    const uint16_t* data() const { return fb_; }

private:
    uint16_t* fb_;
};

// Простой микшер: 4 канала прямоугольной волны. Расширяй: сэмплы, шум, огибающие.
class Mixer {
public:
    static constexpr int CHANNELS = 4;
    static constexpr int SAMPLE_RATE = 22050;

    void tone(int ch, int freq_hz, int volume /*0..255*/, int duration_ms = -1);
    void stop(int ch);
    void render(int16_t* out, int frames);   // вызывается из аудио-потока

private:
    struct Channel {
        std::atomic<uint32_t> step{0};
        std::atomic<int32_t> vol{0};
        std::atomic<int32_t> remaining{0};   // сэмплов; -1 = бесконечно
        uint32_t phase = 0;
    };
    Channel ch_[CHANNELS];
};

class Engine;

class Game {
public:
    virtual ~Game() = default;
    virtual void init(Engine&) {}
    virtual void update(Engine& e, float dt) = 0;   // фиксированные 60 Гц
    virtual void draw(Renderer& r) = 0;
};

class Engine {
public:
    Renderer gfx;
    Mixer audio;
    Input input;

    void run(Game& g);
    void quit() { quit_ = true; }
    std::string path(const char* rel) const {
        return std::string(hal::storage_root()) + "/" + rel;
    }

private:
    bool quit_ = false;
};

// Каждая игра определяет эту функцию; точку входа даёт платформа.
Game* create_game();
