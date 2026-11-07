#include "engine.h"

void Mixer::tone(int ch, int freq_hz, int volume, int duration_ms) {
    if (ch < 0 || ch >= CHANNELS || freq_hz <= 0) return;
    Channel& c = ch_[ch];
    c.step.store((uint32_t)(((uint64_t)freq_hz << 32) / SAMPLE_RATE));
    c.remaining.store(duration_ms < 0 ? -1 : duration_ms * SAMPLE_RATE / 1000);
    c.vol.store(volume < 0 ? 0 : volume > 255 ? 255 : volume);
}

void Mixer::stop(int ch) {
    if (ch >= 0 && ch < CHANNELS) ch_[ch].vol.store(0);
}

void Mixer::render(int16_t* out, int frames) {
    for (int i = 0; i < frames; ++i) {
        int32_t sum = 0;
        for (auto& c : ch_) {
            int32_t vol = c.vol.load(std::memory_order_relaxed);
            int32_t rem = c.remaining.load(std::memory_order_relaxed);
            if (vol == 0 || rem == 0) continue;
            c.phase += c.step.load(std::memory_order_relaxed);
            int32_t s = vol * 32;   // 4 канала * 8160 < 32767
            sum += (c.phase & 0x80000000u) ? s : -s;
            if (rem > 0) c.remaining.store(rem - 1, std::memory_order_relaxed);
        }
        out[i] = (int16_t)(sum > 32767 ? 32767 : sum < -32768 ? -32768 : sum);
    }
}
