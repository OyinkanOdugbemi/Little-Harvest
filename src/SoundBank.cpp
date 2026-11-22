#include "SoundBank.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>

namespace
{
constexpr unsigned kRate = 44100;
constexpr float    kPi   = 3.14159265f;

enum class Wave
{
    Sine,
    Triangle,
    Square,
    Noise
};

// A tiny additive synthesiser: tones are mixed into a float buffer.
class Synth
{
public:
    void tone(float start, float duration, float freqStart, float freqEnd, float volume, Wave wave, float attack = 0.004f)
    {
        const auto first = static_cast<std::size_t>(start * kRate);
        const auto count = static_cast<std::size_t>(duration * kRate);
        if (data.size() < first + count)
            data.resize(first + count, 0.f);

        float phase = 0.f;
        float low   = 0.f;
        for (std::size_t i = 0; i < count; ++i)
        {
            const float t = static_cast<float>(i) / kRate;
            const float k = static_cast<float>(i) / static_cast<float>(count);
            const float f = freqStart + (freqEnd - freqStart) * k;
            phase += 2.f * kPi * f / kRate;

            float v = 0.f;
            switch (wave)
            {
                case Wave::Sine:
                    v = std::sin(phase);
                    break;
                case Wave::Triangle:
                    v = 2.f / kPi * std::asin(std::sin(phase));
                    break;
                case Wave::Square:
                    v = std::tanh(4.f * std::sin(phase)) * 0.6f;
                    break;
                case Wave::Noise:
                    low += (noise(rng) - low) * 0.18f;
                    v = low * 2.f;
                    break;
            }
            const float env = std::min(1.f, t / attack) * (1.f - k) * (1.f - k);
            data[first + i] += v * env * volume;
        }
    }

    void notes(float start, std::initializer_list<float> freqs, float step, float duration, float volume, Wave wave)
    {
        float t = start;
        for (float f : freqs)
        {
            tone(t, duration, f, f, volume, wave);
            t += step;
        }
    }

    std::vector<std::int16_t> pcm() const
    {
        std::vector<std::int16_t> out(data.size());
        for (std::size_t i = 0; i < data.size(); ++i)
            out[i] = static_cast<std::int16_t>(std::tanh(data[i]) * 30000.f);
        return out;
    }

private:
    std::vector<float>                    data;
    std::mt19937                          rng{7};
    std::uniform_real_distribution<float> noise{-1.f, 1.f};
};

// Note frequencies (Hz)
constexpr float G4 = 392.00f, C5 = 523.25f, E5 = 659.25f, G5 = 783.99f, A5 = 880.0f, B5 = 987.77f;
constexpr float C6 = 1046.5f, E6 = 1318.5f, G6 = 1568.0f, C7 = 2093.0f, E7 = 2637.0f, G7 = 3136.0f;

Synth makeSound(Sfx sfx)
{
    Synth s;
    switch (sfx)
    {
        case Sfx::Click:
            s.tone(0.f, 0.05f, 1100.f, 900.f, 0.25f, Wave::Sine);
            break;
        case Sfx::Plant:
            s.tone(0.f, 0.05f, 180.f, 90.f, 0.35f, Wave::Noise);
            s.tone(0.01f, 0.11f, 320.f, 760.f, 0.38f, Wave::Sine);
            break;
        case Sfx::Water:
            s.tone(0.f, 0.32f, 0.f, 0.f, 0.32f, Wave::Noise, 0.04f);
            s.tone(0.04f, 0.07f, 600.f, 1100.f, 0.22f, Wave::Sine);
            s.tone(0.13f, 0.07f, 700.f, 1300.f, 0.18f, Wave::Sine);
            s.tone(0.21f, 0.06f, 800.f, 1400.f, 0.14f, Wave::Sine);
            break;
        case Sfx::Harvest:
            s.tone(0.f, 0.06f, 300.f, 500.f, 0.3f, Wave::Sine);
            s.notes(0.04f, {C6, G6}, 0.08f, 0.16f, 0.28f, Wave::Triangle);
            break;
        case Sfx::Coin:
            s.tone(0.f, 0.05f, B5, B5, 0.2f, Wave::Square);
            s.tone(0.05f, 0.22f, E6, E6, 0.2f, Wave::Square);
            break;
        case Sfx::Buy:
            s.notes(0.f, {E5, G5, C6}, 0.06f, 0.12f, 0.3f, Wave::Triangle);
            break;
        case Sfx::Error:
            s.tone(0.f, 0.09f, 240.f, 220.f, 0.2f, Wave::Square);
            s.tone(0.11f, 0.13f, 200.f, 180.f, 0.2f, Wave::Square);
            break;
        case Sfx::Unlock:
            s.notes(0.f, {C5, E5, G5}, 0.07f, 0.12f, 0.3f, Wave::Triangle);
            s.tone(0.21f, 0.4f, C6, C6, 0.3f, Wave::Triangle);
            s.tone(0.21f, 0.4f, E6, E6, 0.12f, Wave::Sine);
            break;
        case Sfx::Sleep:
            s.notes(0.f, {G5, E5, C5, G4}, 0.16f, 0.45f, 0.26f, Wave::Sine);
            break;
        case Sfx::Morning:
            s.notes(0.f, {C5, E5, G5, C6}, 0.11f, 0.35f, 0.24f, Wave::Sine);
            s.tone(0.5f, 0.07f, 2400.f, 3200.f, 0.12f, Wave::Sine);
            s.tone(0.6f, 0.08f, 2600.f, 3400.f, 0.12f, Wave::Sine);
            break;
        case Sfx::Golden:
            s.notes(0.f, {G6, C7, E7, G7}, 0.05f, 0.18f, 0.16f, Wave::Sine);
            break;
        case Sfx::Ribbon:
            s.notes(0.f, {C5, C5, C5}, 0.12f, 0.1f, 0.28f, Wave::Triangle);
            s.tone(0.36f, 0.25f, G5, G5, 0.28f, Wave::Triangle);
            s.tone(0.62f, 0.7f, C6, C6, 0.3f, Wave::Triangle);
            s.tone(0.62f, 0.7f, E6, E6, 0.14f, Wave::Sine);
            s.tone(0.62f, 0.7f, A5, A5, 0.0f, Wave::Sine);
            break;
        case Sfx::Count:
            break;
    }
    return s;
}
} // namespace

SoundBank::SoundBank()
{
    ready = true;
    for (std::size_t i = 0; i < kCount; ++i)
    {
        const auto samples = makeSound(static_cast<Sfx>(i)).pcm();
        if (samples.empty() ||
            !buffers[i].loadFromSamples(samples.data(), samples.size(), 1, kRate, {sf::SoundChannel::Mono}))
        {
            ready = false;
            continue;
        }
        voices[i].reserve(4);
        for (int v = 0; v < 4; ++v)
            voices[i].emplace_back(buffers[i]);
    }
}

void SoundBank::play(Sfx sfx, float pitch)
{
    const auto i = static_cast<std::size_t>(sfx);
    if (muted || i >= kCount || voices[i].empty())
        return;
    sf::Sound& voice = voices[i][next[i]];
    next[i]          = (next[i] + 1) % voices[i].size();
    voice.setPitch(pitch);
    voice.setVolume(70.f);
    voice.play();
}

void SoundBank::setMuted(bool m)
{
    muted = m;
}

bool SoundBank::isMuted() const
{
    return muted;
}
