#include <cstdio>
#include "engine.h"

// Спрайт 8x8: «смайлик», сгенерированный в коде
static uint16_t g_pix[64];
static Sprite g_sprite{8, 8, g_pix, 0xF81F};   // розовый = прозрачный

class Demo : public Game {
    float x = 160, y = 120;
    int frames = 0;
    uint16_t Y = Renderer::rgb(255, 220, 0), K = Renderer::rgb(0, 0, 0);

public:
    void init(Engine&) override {
        static const char* art[8] = {
            "..YYYY..", ".YYYYYY.", "YYKYYKYY", "YYYYYYYY",
            "YKYYYYKY", "YYKKKKYY", ".YYYYYY.", "..YYYY..",
        };
        for (int r = 0; r < 8; ++r)
            for (int c = 0; c < 8; ++c)
                g_pix[r * 8 + c] = art[r][c] == 'Y' ? Y : art[r][c] == 'K' ? K : g_sprite.key;
    }

    void update(Engine& e, float dt) override {
        const float speed = 120;
        if (e.input.held(BTN_LEFT))  x -= speed * dt;
        if (e.input.held(BTN_RIGHT)) x += speed * dt;
        if (e.input.held(BTN_UP))    y -= speed * dt;
        if (e.input.held(BTN_DOWN))  y += speed * dt;
        if (e.input.hit(BTN_A)) e.audio.tone(0, 880, 120, 80);
        if (e.input.hit(BTN_B)) e.audio.tone(1, 220, 120, 150);
        ++frames;
    }

    void draw(Renderer& r) override {
        r.clear(Renderer::rgb(20, 24, 48));
        r.rect(0, 0, Renderer::W, Renderer::H, Renderer::rgb(80, 90, 160));
        r.text(8, 8, "HELLO CONSOLE", Renderer::rgb(255, 255, 255), 2);
        char buf[32];
        std::snprintf(buf, sizeof buf, "FRAME: %d", frames);
        r.text(8, 28, buf, Renderer::rgb(160, 200, 255));
        r.text(8, 224, "ARROWS  Z:BEEP  X:BOOP  ENTER+BKSP:EXIT", Renderer::rgb(120, 130, 170));
        // 4x масштаб спрайта вручную через fillRect (спрайт 8x8 -> 32x32)
        for (int sy = 0; sy < 8; ++sy)
            for (int sx = 0; sx < 8; ++sx) {
                uint16_t p = g_pix[sy * 8 + sx];
                if (p != g_sprite.key) r.fillRect((int)x + sx * 4, (int)y + sy * 4, 4, 4, p);
            }
    }
};

Game* create_game() { return new Demo(); }
