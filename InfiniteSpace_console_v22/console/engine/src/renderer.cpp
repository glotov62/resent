#include <algorithm>
#include <cstring>
#include "engine.h"

// Шрифт 3x5: 5 строк по 3 бита (старший бит = левый пиксель)
static const uint8_t FONT[36][5] = {
    {7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},{5,5,7,1,1},   // 0-4
    {7,4,7,1,7},{7,4,7,5,7},{7,1,1,1,1},{7,5,7,5,7},{7,5,7,1,7},   // 5-9
    {2,5,7,5,5},{6,5,6,5,6},{7,4,4,4,7},{6,5,5,5,6},{7,4,6,4,7},   // A-E
    {7,4,6,4,4},{7,4,5,5,7},{5,5,7,5,5},{7,2,2,2,7},{1,1,1,5,7},   // F-J
    {5,5,6,5,5},{4,4,4,4,7},{5,7,7,5,5},{6,5,5,5,5},{7,5,5,5,7},   // K-O
    {7,5,7,4,4},{7,5,5,7,1},{6,5,6,5,5},{7,4,7,1,7},{7,2,2,2,2},   // P-T
    {5,5,5,5,7},{5,5,5,5,2},{5,5,7,7,5},{5,5,2,5,5},{5,5,2,2,2},   // U-Y
    {7,1,2,4,7},                                                    // Z
};

static const uint8_t* glyph(char ch) {
    static const uint8_t dash[5] = {0,0,7,0,0}, colon[5] = {0,2,0,2,0}, dot[5] = {0,0,0,0,2};
    if (ch >= '0' && ch <= '9') return FONT[ch - '0'];
    if (ch >= 'a' && ch <= 'z') ch -= 32;
    if (ch >= 'A' && ch <= 'Z') return FONT[10 + ch - 'A'];
    if (ch == '-') return dash;
    if (ch == ':') return colon;
    if (ch == '.') return dot;
    return nullptr;
}

Renderer::Renderer() {
    fb_ = static_cast<uint16_t*>(hal::alloc_framebuffer(W * H * sizeof(uint16_t)));
    clear(0);
}

void Renderer::clear(uint16_t c) {
    for (int i = 0; i < W * H; ++i) fb_[i] = c;
}

void Renderer::pixel(int x, int y, uint16_t c) {
    if ((unsigned)x < (unsigned)W && (unsigned)y < (unsigned)H) fb_[y * W + x] = c;
}

void Renderer::fillRect(int x, int y, int w, int h, uint16_t c) {
    int x0 = std::max(x, 0), y0 = std::max(y, 0);
    int x1 = std::min(x + w, W), y1 = std::min(y + h, H);
    for (int yy = y0; yy < y1; ++yy)
        for (int xx = x0; xx < x1; ++xx) fb_[yy * W + xx] = c;
}

void Renderer::rect(int x, int y, int w, int h, uint16_t c) {
    fillRect(x, y, w, 1, c);
    fillRect(x, y + h - 1, w, 1, c);
    fillRect(x, y, 1, h, c);
    fillRect(x + w - 1, y, 1, h, c);
}

void Renderer::sprite(const Sprite& s, int x, int y, bool flipX) {
    for (int sy = 0; sy < s.h; ++sy) {
        int dy = y + sy;
        if ((unsigned)dy >= (unsigned)H) continue;
        for (int sx = 0; sx < s.w; ++sx) {
            int dx = x + sx;
            if ((unsigned)dx >= (unsigned)W) continue;
            uint16_t p = s.pixels[sy * s.w + (flipX ? s.w - 1 - sx : sx)];
            if (p != s.key) fb_[dy * W + dx] = p;
        }
    }
}

void Renderer::text(int x, int y, const char* str, uint16_t c, int scale) {
    for (; *str; ++str, x += 4 * scale) {
        const uint8_t* g = glyph(*str);
        if (!g) continue;
        for (int row = 0; row < 5; ++row)
            for (int col = 0; col < 3; ++col)
                if (g[row] & (4 >> col))
                    fillRect(x + col * scale, y + row * scale, scale, scale, c);
    }
}
