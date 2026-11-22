#include "Effects.hpp"

#include <algorithm>
#include <cmath>

#include "Art.hpp"
#include "Ui.hpp"

namespace
{
constexpr float kPi = 3.14159265f;
}

Effects::Effects() : rng(std::random_device{}())
{
}

float Effects::random(float min, float max)
{
    return std::uniform_real_distribution<float>(min, max)(rng);
}

void Effects::add(Shape shape, sf::Vector2f pos, sf::Vector2f vel, float life, float size, sf::Color color, float gravity, float drag)
{
    particles.push_back({pos, vel, life, life, size, gravity, drag, random(0.f, 360.f), random(-240.f, 240.f), color, shape});
}

int Effects::update(float dt)
{
    for (auto& p : particles)
    {
        p.vel.y += p.gravity * dt;
        p.vel *= std::max(0.f, 1.f - p.drag * dt);
        p.pos += p.vel * dt;
        p.angle += p.spin * dt;
        p.life -= dt;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p) { return p.life <= 0.f; }),
                    particles.end());

    for (auto& t : texts)
    {
        const float k = t.life / t.maxLife;
        t.pos.y -= (20.f + 50.f * k) * dt;
        t.life -= dt;
    }
    texts.erase(std::remove_if(texts.begin(), texts.end(), [](const FloatText& t) { return t.life <= 0.f; }), texts.end());

    int arrived = 0;
    for (auto& c : coins)
    {
        if (c.delay > 0.f)
        {
            c.delay -= dt;
            continue;
        }
        c.t += dt / c.duration;
        if (c.t >= 1.f)
            ++arrived;
    }
    coins.erase(std::remove_if(coins.begin(), coins.end(), [](const Coin& c) { return c.t >= 1.f; }), coins.end());
    return arrived;
}

void Effects::draw(sf::RenderTarget& target, const sf::Font& font) const
{
    for (const auto& p : particles)
    {
        const float k     = p.life / p.maxLife;
        const auto  alpha = static_cast<std::uint8_t>(std::clamp(k / 0.4f, 0.f, 1.f) * static_cast<float>(p.color.a));
        const auto  color = art::withAlpha(p.color, alpha);
        const float size  = p.size * (0.5f + 0.5f * std::min(1.f, k * 2.f));

        switch (p.shape)
        {
            case Shape::Circle:
                art::fillCircle(target, p.pos, size, color);
                break;
            case Shape::Square:
                art::fillRect(target, {p.pos - sf::Vector2f(size, size * 0.6f), {size * 2.f, size * 1.2f}}, color, p.angle);
                break;
            case Shape::Star:
                art::drawSparkle(target, p.pos, size, color, p.angle);
                break;
            case Shape::Leaf:
                art::fillEllipse(target, p.pos, {size, size * 0.5f}, color, p.angle);
                break;
            case Shape::Drop:
                art::fillEllipse(target, p.pos, {size * 0.6f, size}, color, std::atan2(p.vel.x, -p.vel.y) * -180.f / kPi);
                break;
        }
    }

    for (const auto& c : coins)
    {
        if (c.delay > 0.f)
            continue;
        const float t   = c.t * c.t * (3.f - 2.f * c.t); // smoothstep
        const float u   = 1.f - t;
        const auto  pos = c.from * (u * u) + c.control * (2.f * u * t) + c.to * (t * t);
        art::drawCoin(target, pos, 8.f, std::abs(std::cos(c.t * 12.f)));
    }

    for (const auto& t : texts)
    {
        const float k     = t.life / t.maxLife;
        const auto  alpha = static_cast<std::uint8_t>(std::clamp(k / 0.35f, 0.f, 1.f) * 255.f);
        const float pop   = 1.f + 0.25f * std::max(0.f, (k - 0.85f) / 0.15f);
        ui::drawText(target,
                     font,
                     t.text,
                     t.pos,
                     static_cast<unsigned>(static_cast<float>(t.size) * pop),
                     art::withAlpha(t.color, alpha),
                     ui::Align::Center,
                     true,
                     3.f,
                     sf::Color(110, 74, 46, alpha));
    }
}

void Effects::clear()
{
    particles.clear();
    texts.clear();
    coins.clear();
}

void Effects::dirtPuff(sf::Vector2f at)
{
    for (int i = 0; i < 10; ++i)
    {
        const float a = random(-kPi, 0.f);
        add(Shape::Circle,
            at + sf::Vector2f(random(-12.f, 12.f), random(-4.f, 4.f)),
            {std::cos(a) * random(30.f, 80.f), std::sin(a) * random(30.f, 90.f)},
            random(0.35f, 0.6f),
            random(2.5f, 4.5f),
            sf::Color(150, 108, 74),
            260.f,
            1.5f);
    }
}

void Effects::splash(sf::Vector2f at)
{
    // Drops raining down from a little above, then bouncing out.
    for (int i = 0; i < 10; ++i)
        add(Shape::Drop,
            at + sf::Vector2f(random(-22.f, 22.f), random(-60.f, -40.f)),
            {random(-10.f, 10.f), random(160.f, 220.f)},
            random(0.22f, 0.32f),
            random(3.f, 4.5f),
            sf::Color(120, 190, 240, 230));
    for (int i = 0; i < 10; ++i)
    {
        const float a = random(-kPi, 0.f);
        add(Shape::Circle,
            at + sf::Vector2f(random(-14.f, 14.f), random(0.f, 8.f)),
            {std::cos(a) * random(40.f, 110.f), std::sin(a) * random(60.f, 140.f)},
            random(0.4f, 0.65f),
            random(2.f, 3.5f),
            sf::Color(150, 205, 245, 230),
            380.f);
    }
}

void Effects::leafBurst(sf::Vector2f at, sf::Color produce)
{
    for (int i = 0; i < 12; ++i)
    {
        const float a = random(-kPi * 0.95f, -kPi * 0.05f);
        add(i % 3 == 0 ? Shape::Circle : Shape::Leaf,
            at,
            {std::cos(a) * random(60.f, 150.f), std::sin(a) * random(80.f, 200.f)},
            random(0.5f, 0.85f),
            random(3.f, 6.f),
            i % 3 == 0 ? produce : sf::Color(110, 180, 90),
            420.f,
            0.8f);
    }
}

void Effects::sparkles(sf::Vector2f at, int count, sf::Color color, float spread)
{
    for (int i = 0; i < count; ++i)
    {
        const float a = random(0.f, 2.f * kPi);
        const float r = random(0.f, spread);
        add(Shape::Star,
            at + sf::Vector2f(std::cos(a), std::sin(a)) * r * 0.4f,
            sf::Vector2f(std::cos(a), std::sin(a)) * r * 1.6f,
            random(0.5f, 1.f),
            random(3.f, 7.f),
            color,
            -20.f,
            2.f);
    }
}

void Effects::confetti(float width)
{
    const sf::Color colors[] = {
        {240, 120, 130}, {250, 204, 80}, {120, 190, 235}, {130, 200, 110}, {200, 150, 230}, {255, 170, 90}};
    for (int i = 0; i < 160; ++i)
        add(i % 2 ? Shape::Square : Shape::Leaf,
            {random(0.f, width), random(-120.f, -10.f)},
            {random(-60.f, 60.f), random(40.f, 160.f)},
            random(2.8f, 4.5f),
            random(3.5f, 6.f),
            colors[i % 6],
            60.f,
            0.3f);
}

void Effects::floatingText(const std::string& text, sf::Vector2f at, sf::Color color, unsigned size)
{
    texts.push_back({text, at, 1.3f, 1.3f, color, size});
}

void Effects::flyingCoins(sf::Vector2f from, sf::Vector2f to, int count)
{
    for (int i = 0; i < count; ++i)
    {
        const sf::Vector2f start = from + sf::Vector2f(random(-12.f, 12.f), random(-10.f, 6.f));
        const sf::Vector2f mid   = (start + to) / 2.f + sf::Vector2f(random(-120.f, 120.f), random(-40.f, 60.f));
        coins.push_back({start, mid, to, 0.f, static_cast<float>(i) * 0.06f, random(0.55f, 0.8f)});
    }
}
