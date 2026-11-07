// Змейка. Поле 20x14 клеток по 16 пикселей, сверху строка со счётом.
#include <cstdio>
#include <cstdint>
#include "engine.h"

namespace {

constexpr int CELL = 16;
constexpr int COLS = Renderer::W / CELL;        // 20
constexpr int ROWS = Renderer::H / CELL - 1;    // 14 (верхняя строка под счёт)
constexpr int OFFSET_Y = CELL;                  // поле начинается ниже строки счёта

enum Dir { UP, RIGHT, DOWN, LEFT };
const int DX[4] = {0, 1, 0, -1};
const int DY[4] = {-1, 0, 1, 0};

enum State { PLAYING, PAUSED, DEAD };

struct Pt { int x, y; };

} // namespace

class Snake : public Game {
    Pt body[COLS * ROWS];     // body[0] = голова
    int len = 0;
    Dir dir = RIGHT, nextDir = RIGHT;
    Pt food{0, 0};
    int score = 0, best = 0;
    float timer = 0;
    State state = PLAYING;
    uint32_t rng = 1;

    // Цвета
    uint16_t cBg    = Renderer::rgb(16, 20, 32);
    uint16_t cField = Renderer::rgb(24, 32, 52);
    uint16_t cHead  = Renderer::rgb(120, 255, 120);
    uint16_t cBody  = Renderer::rgb(40, 190, 70);
    uint16_t cFood  = Renderer::rgb(255, 70, 70);
    uint16_t cText  = Renderer::rgb(230, 230, 240);

    int random(int n) {   // простой генератор случайных чисел (xorshift)
        rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
        return (int)(rng % (uint32_t)n);
    }

    bool onSnake(int x, int y, int count) const {
        for (int i = 0; i < count; ++i)
            if (body[i].x == x && body[i].y == y) return true;
        return false;
    }

    void placeFood() {
        do {
            food = {random(COLS), random(ROWS)};
        } while (onSnake(food.x, food.y, len));
    }

    void reset() {
        len = 3;
        for (int i = 0; i < len; ++i) body[i] = {COLS / 2 - i, ROWS / 2};
        dir = nextDir = RIGHT;
        score = 0;
        timer = 0;
        state = PLAYING;
        placeFood();
    }

    float stepInterval() const {   // чем больше съедено, тем быстрее
        float t = 0.16f - 0.004f * score;
        return t < 0.06f ? 0.06f : t;
    }

    void die(Engine& e) {
        state = DEAD;
        if (score > best) best = score;
        e.audio.tone(1, 150, 150, 400);
    }

    void step(Engine& e) {
        dir = nextDir;
        Pt head{body[0].x + DX[dir], body[0].y + DY[dir]};

        // Врезались в стену
        if (head.x < 0 || head.x >= COLS || head.y < 0 || head.y >= ROWS) { die(e); return; }

        bool eat = (head.x == food.x && head.y == food.y);

        // Врезались в себя (хвост освобождает клетку, если не едим)
        if (onSnake(head.x, head.y, eat ? len : len - 1)) { die(e); return; }

        if (eat) ++len;
        for (int i = len - 1; i > 0; --i) body[i] = body[i - 1];
        body[0] = head;

        if (eat) {
            ++score;
            e.audio.tone(0, 880, 120, 60);
            placeFood();
        }
    }

public:
    void init(Engine&) override {
        rng = (uint32_t)hal::micros() | 1;
        reset();
    }

    void update(Engine& e, float dt) override {
        if (state == DEAD) {
            if (e.input.hit(BTN_A) || e.input.hit(BTN_START)) reset();
            return;
        }
        if (e.input.hit(BTN_START)) state = (state == PLAYING) ? PAUSED : PLAYING;
        if (state == PAUSED) return;

        // Поворот: нельзя развернуться на 180 градусов
        if (e.input.hit(BTN_UP)    && dir != DOWN)  nextDir = UP;
        if (e.input.hit(BTN_DOWN)  && dir != UP)    nextDir = DOWN;
        if (e.input.hit(BTN_LEFT)  && dir != RIGHT) nextDir = LEFT;
        if (e.input.hit(BTN_RIGHT) && dir != LEFT)  nextDir = RIGHT;

        timer += dt;
        if (timer >= stepInterval()) {
            timer -= stepInterval();
            step(e);
        }
    }

    void draw(Renderer& r) override {
        r.clear(cBg);

        char buf[32];
        std::snprintf(buf, sizeof buf, "SCORE: %d", score);
        r.text(6, 4, buf, cText, 2);
        std::snprintf(buf, sizeof buf, "BEST: %d", best);
        r.text(Renderer::W - 6 - 4 * (int)std::snprintf(buf, sizeof buf, "BEST: %d", best) * 2 + 2,
               4, buf, cText, 2);

        r.fillRect(0, OFFSET_Y, COLS * CELL, ROWS * CELL, cField);

        // Еда
        r.fillRect(food.x * CELL + 2, OFFSET_Y + food.y * CELL + 2, CELL - 4, CELL - 4, cFood);

        // Змейка (зазор в 1 пиксель между сегментами)
        for (int i = len - 1; i >= 0; --i)
            r.fillRect(body[i].x * CELL + 1, OFFSET_Y + body[i].y * CELL + 1,
                       CELL - 2, CELL - 2, i == 0 ? cHead : cBody);

        if (state == DEAD) {
            r.text((Renderer::W - 9 * 12) / 2, 90, "GAME OVER", cFood, 3);
            r.text((Renderer::W - 17 * 8) / 2, 120, "PRESS A TO RETRY", cText, 2);
        } else if (state == PAUSED) {
            r.text((Renderer::W - 5 * 12) / 2, 100, "PAUSE", cText, 3);
        }
    }
};

Game* create_game() { return new Snake(); }
