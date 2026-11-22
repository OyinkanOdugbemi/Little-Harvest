#pragma once

#include <SFML/Graphics.hpp>
#include <random>
#include <string>
#include <vector>

// Particles, floating "+25" numbers and coins that fly up to the coin counter.
class Effects
{
public:
    Effects();

    // Returns how many flying coins reached their target this frame.
    int  update(float dt);
    void draw(sf::RenderTarget& target, const sf::Font& font) const;
    void clear();

    void dirtPuff(sf::Vector2f at);
    void splash(sf::Vector2f at);
    void leafBurst(sf::Vector2f at, sf::Color produce);
    void sparkles(sf::Vector2f at, int count, sf::Color color, float spread = 40.f);
    void confetti(float width);
    void floatingText(const std::string& text, sf::Vector2f at, sf::Color color, unsigned size = 24);
    void flyingCoins(sf::Vector2f from, sf::Vector2f to, int count);

private:
    enum class Shape
    {
        Circle,
        Square,
        Star,
        Leaf,
        Drop
    };

    struct Particle
    {
        sf::Vector2f pos;
        sf::Vector2f vel;
        float        life;
        float        maxLife;
        float        size;
        float        gravity;
        float        drag;
        float        angle;
        float        spin;
        sf::Color    color;
        Shape        shape;
    };

    struct FloatText
    {
        std::string  text;
        sf::Vector2f pos;
        float        life;
        float        maxLife;
        sf::Color    color;
        unsigned     size;
    };

    struct Coin
    {
        sf::Vector2f from;
        sf::Vector2f control;
        sf::Vector2f to;
        float        t;
        float        delay;
        float        duration;
    };

    float random(float min, float max);
    void  add(Shape shape, sf::Vector2f pos, sf::Vector2f vel, float life, float size, sf::Color color, float gravity = 0.f, float drag = 0.f);

    std::vector<Particle>  particles;
    std::vector<FloatText> texts;
    std::vector<Coin>      coins;
    std::mt19937           rng;
};
