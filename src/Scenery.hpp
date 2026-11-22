#pragma once

#include <SFML/Graphics.hpp>
#include <random>
#include <vector>

enum class Weather
{
    Sunny,
    Cloudy,
    Rainy
};

// The farm around the plots: grass, house, pond, trees and fence (baked once into
// a texture), plus the living bits: chimney smoke, a duck, butterflies, clouds and rain.
class Scenery
{
public:
    Scenery();

    void build();
    void setWeather(Weather weather);
    void update(float dt);

    void drawGround(sf::RenderTarget& target) const;  // static background + duck + smoke
    void drawAmbient(sf::RenderTarget& target) const; // butterflies and cloud shadows (above plots)
    void drawWeather(sf::RenderTarget& target) const; // rain and colour tint

private:
    struct Puff
    {
        sf::Vector2f pos;
        float        life;
    };
    struct Butterfly
    {
        sf::Vector2f pos;
        sf::Vector2f target;
        sf::Color    color;
        float        t;
    };
    struct CloudShadow
    {
        sf::Vector2f pos;
        float        scale;
        float        speed;
    };
    struct RainDrop
    {
        sf::Vector2f pos;
        float        speed;
    };
    struct Ripple
    {
        sf::Vector2f pos;
        float        life;
    };

    float random(float min, float max);
    void  paintBackground(sf::RenderTarget& target);
    void  drawDuck(sf::RenderTarget& target) const;

    sf::RenderTexture background;
    bool              built = false;

    Weather                  weather = Weather::Sunny;
    float                    time    = 0.f;
    float                    puffTimer = 0.f;
    std::vector<Puff>        puffs;
    std::vector<Butterfly>   butterflies;
    std::vector<CloudShadow> clouds;
    std::vector<RainDrop>    rain;
    std::vector<Ripple>      ripples;
    std::mt19937             rng;
};
