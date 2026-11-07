#include "engine.h"

static void audio_cb(int16_t* out, int frames, void* user) {
    static_cast<Mixer*>(user)->render(out, frames);
}

void Engine::run(Game& g) {
    if (!hal::init()) return;
    hal::audio_init(Mixer::SAMPLE_RATE, audio_cb, &audio);
    g.init(*this);

    const uint64_t step = 1000000 / 60;
    uint64_t prev = hal::micros(), acc = 0;

    while (!quit_ && !hal::quit_requested()) {
        uint64_t now = hal::micros();
        acc += now - prev;
        prev = now;
        if (acc > 250000) acc = 250000;   // не «догонять» после паузы

        while (acc >= step) {
            input.update(hal::poll_buttons());
            // Системная комбинация: выход в лаунчер из любой игры
            if ((input.down & (BTN_START | BTN_SELECT)) == (BTN_START | BTN_SELECT)) {
                hal::exit_to_launcher();
                quit_ = true;
                break;
            }
            g.update(*this, 1.0f / 60.0f);
            acc -= step;
        }
        if (quit_) break;

        g.draw(gfx);
        hal::present(gfx.data());

        if (acc < step) hal::sleep_us((uint32_t)((step - acc) / 2));
    }
    hal::shutdown();
}
