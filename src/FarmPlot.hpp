#pragma once

#include <SFML/Graphics.hpp>
#include <iosfwd>
#include <random>

#include "Crop.hpp"

// One square of farmland. It can be locked (still grass), empty soil,
// or holding a crop that grows a little every day it was watered.
class FarmPlot
{
public:
    FarmPlot(sf::Vector2f position, float size, bool unlocked);

    void update(float dt);
    void draw(sf::RenderTarget& target, float time, bool hovered) const;

    bool          contains(sf::Vector2f point) const;
    sf::FloatRect getBounds() const;
    sf::Vector2f  getCenter() const;

    // Actions. Each returns true if something happened.
    void unlock();
    bool plant(CropType type);
    bool water();
    void grow(std::mt19937& rng); // called once at the start of every new day
    int  harvest();               // returns coins earned (0 if nothing was ripe)
    void bump();                  // little squish animation

    bool        isUnlocked() const;
    bool        isEmpty() const;
    bool        isReady() const;
    bool        isWatered() const;
    bool        isGolden() const;
    const Crop& getCrop() const;
    int         getDaysLeft() const;

    void save(std::ostream& out) const;
    bool load(std::istream& in);

private:
    int  stage() const; // -1 nothing, 0 seed, 1 sprout, 2 growing, 3 ripe
    void drawCrop(sf::RenderTarget& target, sf::RenderStates states, float time) const;

    sf::Vector2f position;
    float        size;
    bool         unlocked = false;
    bool         hasCrop  = false;
    Crop         crop;
    int          growth    = 0; // watered days so far
    bool         watered   = false;
    bool         ready     = false;
    bool         golden    = false;
    bool         regrowing = false;

    float bumpTimer = 0.f;
    float phase     = 0.f; // so neighbouring plants don't sway in sync
};
