#pragma once

#include <SFML/Audio.hpp>
#include <array>
#include <vector>

enum class Sfx
{
    Click,
    Plant,
    Water,
    Harvest,
    Coin,
    Buy,
    Error,
    Unlock,
    Sleep,
    Morning,
    Golden,
    Ribbon,
    Count
};

// Every sound effect is synthesised in code when the game starts,
// so there are no audio files to ship.
class SoundBank
{
public:
    SoundBank();

    void play(Sfx sfx, float pitch = 1.f);
    void setMuted(bool muted);
    bool isMuted() const;

private:
    static constexpr std::size_t kCount = static_cast<std::size_t>(Sfx::Count);

    std::array<sf::SoundBuffer, kCount>       buffers;
    std::array<std::vector<sf::Sound>, kCount> voices;
    std::array<std::size_t, kCount>           next{};
    bool                                      muted = false;
    bool                                      ready = false;
};
