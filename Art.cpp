#include "Art.hpp"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float kPi = 3.14159265f;

sf::RenderStates at(sf::RenderStates states, sf::Vector2f center, float scale, float rotationDeg = 0.f)
{
    states.transform.translate(center);
    if (rotationDeg != 0.f)
        states.transform.rotate(sf::degrees(rotationDeg));
    states.transform.scale({scale, scale});
    return states;
}

sf::Color produceColor(CropType type, bool golden)
{
    if (golden)
        return sf::Color(252, 212, 78);
    return Crop::info(type).color;
}

const sf::Color kLeaf(98, 172, 84);
const sf::Color kLeafDark(70, 140, 64);
} // namespace

namespace art
{
// ---- colours ----------------------------------------------------------------
sf::Color darker(sf::Color c, float factor)
{
    return sf::Color(static_cast<std::uint8_t>(c.r * factor),
                     static_cast<std::uint8_t>(c.g * factor),
                     static_cast<std::uint8_t>(c.b * factor),
                     c.a);
}

sf::Color lighter(sf::Color c, float amount)
{
    return mix(c, sf::Color(255, 255, 255, c.a), amount);
}

sf::Color withAlpha(sf::Color c, std::uint8_t alpha)
{
    c.a = alpha;
    return c;
}

sf::Color mix(sf::Color a, sf::Color b, float t)
{
    t           = std::clamp(t, 0.f, 1.f);
    auto lerp8 = [t](std::uint8_t x, std::uint8_t y)
    { return static_cast<std::uint8_t>(static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t); };
    return sf::Color(lerp8(a.r, b.r), lerp8(a.g, b.g), lerp8(a.b, b.b), lerp8(a.a, b.a));
}

// ---- shapes -------------------------------------------------------------------
sf::ConvexShape roundedRect(sf::Vector2f size, float radius, int cornerPoints)
{
    radius = std::min({radius, size.x / 2.f, size.y / 2.f});
    sf::ConvexShape shape(static_cast<std::size_t>(cornerPoints * 4));
    const sf::Vector2f centers[4] = {{size.x - radius, radius},
                                     {size.x - radius, size.y - radius},
                                     {radius, size.y - radius},
                                     {radius, radius}};
    std::size_t index = 0;
    for (int corner = 0; corner < 4; ++corner)
    {
        const float start = -kPi / 2.f + static_cast<float>(corner) * kPi / 2.f;
        for (int i = 0; i < cornerPoints; ++i)
        {
            const float a = start + (kPi / 2.f) * static_cast<float>(i) / static_cast<float>(cornerPoints - 1);
            shape.setPoint(index++, centers[corner] + sf::Vector2f(std::cos(a) * radius, std::sin(a) * radius));
        }
    }
    return shape;
}

sf::ConvexShape ellipse(sf::Vector2f radii, int points)
{
    sf::ConvexShape shape(static_cast<std::size_t>(points));
    for (int i = 0; i < points; ++i)
    {
        const float a = 2.f * kPi * static_cast<float>(i) / static_cast<float>(points);
        shape.setPoint(static_cast<std::size_t>(i), {std::cos(a) * radii.x, std::sin(a) * radii.y});
    }
    return shape;
}

sf::ConvexShape teardrop(float radius, float tail, int points)
{
    // Upper half is a circle, lower half narrows smoothly to a point at (0, tail).
    sf::ConvexShape shape(static_cast<std::size_t>(points));
    for (int i = 0; i < points; ++i)
    {
        const float a = 2.f * kPi * static_cast<float>(i) / static_cast<float>(points); // 0 = right, pi/2 = bottom
        const float s = std::sin(a);
        float       x = std::cos(a) * radius;
        float       y = s * radius;
        if (s > 0.f)
        {
            y = s * tail;
            x *= 1.f - 0.25f * s;
        }
        shape.setPoint(static_cast<std::size_t>(i), {x, y});
    }
    return shape;
}

sf::ConvexShape star(float outer, float inner, int tips)
{
    sf::ConvexShape shape(static_cast<std::size_t>(tips * 2));
    for (int i = 0; i < tips * 2; ++i)
    {
        const float a = -kPi / 2.f + kPi * static_cast<float>(i) / static_cast<float>(tips);
        const float r = (i % 2 == 0) ? outer : inner;
        shape.setPoint(static_cast<std::size_t>(i), {std::cos(a) * r, std::sin(a) * r});
    }
    return shape;
}

// ---- immediate drawing ------------------------------------------------------------
void fillRoundedRect(sf::RenderTarget& target,
                     sf::FloatRect     rect,
                     float             radius,
                     sf::Color         fill,
                     float             outline,
                     sf::Color         outlineColor,
                     sf::RenderStates  states)
{
    auto shape = roundedRect(rect.size, radius);
    shape.setPosition(rect.position);
    shape.setFillColor(fill);
    if (outline > 0.f)
    {
        shape.setOutlineThickness(outline);
        shape.setOutlineColor(outlineColor);
    }
    target.draw(shape, states);
}

void fillEllipse(sf::RenderTarget& target,
                 sf::Vector2f      center,
                 sf::Vector2f      radii,
                 sf::Color         fill,
                 float             rotationDeg,
                 float             outline,
                 sf::Color         outlineColor,
                 sf::RenderStates  states)
{
    auto shape = ellipse(radii);
    shape.setPosition(center);
    shape.setRotation(sf::degrees(rotationDeg));
    shape.setFillColor(fill);
    if (outline > 0.f)
    {
        shape.setOutlineThickness(outline);
        shape.setOutlineColor(outlineColor);
    }
    target.draw(shape, states);
}

void fillCircle(sf::RenderTarget& target,
                sf::Vector2f      center,
                float             radius,
                sf::Color         fill,
                float             outline,
                sf::Color         outlineColor,
                sf::RenderStates  states)
{
    sf::CircleShape shape(radius, std::max<std::size_t>(12, static_cast<std::size_t>(radius * 2.5f)));
    shape.setOrigin({radius, radius});
    shape.setPosition(center);
    shape.setFillColor(fill);
    if (outline > 0.f)
    {
        shape.setOutlineThickness(outline);
        shape.setOutlineColor(outlineColor);
    }
    target.draw(shape, states);
}

void fillRect(sf::RenderTarget& target, sf::FloatRect rect, sf::Color fill, float rotationDeg, sf::RenderStates states)
{
    sf::RectangleShape shape(rect.size);
    shape.setOrigin(rect.size / 2.f);
    shape.setPosition(rect.position + rect.size / 2.f);
    shape.setRotation(sf::degrees(rotationDeg));
    shape.setFillColor(fill);
    target.draw(shape, states);
}

// ---- face ---------------------------------------------------------------------------
void drawFace(sf::RenderTarget& target, sf::RenderStates states, sf::Vector2f center, float scale, bool blink)
{
    const sf::RenderStates s = at(states, center, scale);
    const sf::Color        eye(66, 44, 38);

    for (float side : {-1.f, 1.f})
    {
        if (blink)
        {
            fillRoundedRect(target, {{side * 4.f - 2.f, -1.4f}, {4.f, 1.3f}}, 0.6f, eye, 0.f, {}, s);
        }
        else
        {
            fillCircle(target, {side * 4.f, -1.f}, 1.9f, eye, 0.f, {}, s);
            fillCircle(target, {side * 4.f + 0.7f, -1.7f}, 0.7f, sf::Color::White, 0.f, {}, s);
        }
        fillEllipse(target, {side * 6.8f, 2.3f}, {2.2f, 1.3f}, sf::Color(255, 120, 140, 140), 0.f, 0.f, {}, s);
    }

    // Smile: a little arc of dots.
    for (int i = 0; i <= 6; ++i)
    {
        const float a = kPi * (0.15f + 0.7f * static_cast<float>(i) / 6.f);
        fillCircle(target, {std::cos(a) * 2.2f, 0.6f + std::sin(a) * 2.f}, 0.55f, eye, 0.f, {}, s);
    }
}

// ---- produce -------------------------------------------------------------------------
void drawProduce(sf::RenderTarget& target,
                 sf::RenderStates  states,
                 CropType          type,
                 sf::Vector2f      center,
                 float             scale,
                 bool              golden,
                 bool              face,
                 bool              blink)
{
    const sf::RenderStates s     = at(states, center, scale);
    const sf::Color        body  = produceColor(type, golden);
    const sf::Color        edge  = darker(body, 0.78f);
    const float            lineW = 1.4f;

    if (golden)
    {
        // Warm halo so lucky crops stand out, even the already-yellow sunflower.
        fillCircle(target, {0.f, -2.f}, 25.f, sf::Color(255, 236, 140, 70), 0.f, {}, s);
        fillCircle(target, {0.f, -2.f}, 19.f, sf::Color(255, 244, 170, 90), 0.f, {}, s);
    }

    switch (type)
    {
        case CropType::Carrot:
        {
            fillEllipse(target, {-5.f, -19.f}, {3.4f, 9.f}, kLeaf, -28.f, lineW, kLeafDark, s);
            fillEllipse(target, {5.f, -19.f}, {3.4f, 9.f}, kLeaf, 28.f, lineW, kLeafDark, s);
            fillEllipse(target, {0.f, -22.f}, {3.6f, 10.f}, lighter(kLeaf, 0.1f), 0.f, lineW, kLeafDark, s);

            auto root = teardrop(11.f, 26.f);
            root.setPosition({0.f, -8.f});
            root.setFillColor(body);
            root.setOutlineThickness(lineW);
            root.setOutlineColor(edge);
            target.draw(root, s);
            fillRoundedRect(target, {{-7.f, 5.f}, {6.f, 1.5f}}, 0.7f, edge, 0.f, {}, s);
            fillRoundedRect(target, {{1.f, 10.f}, {5.f, 1.5f}}, 0.7f, edge, 0.f, {}, s);
            fillEllipse(target, {-5.f, -12.f}, {2.5f, 1.5f}, sf::Color(255, 255, 255, 110), -30.f, 0.f, {}, s);
            if (face)
                drawFace(target, s, {0.f, -6.f}, 0.85f, blink);
            break;
        }
        case CropType::Strawberry:
        {
            auto berry = teardrop(12.f, 15.f);
            berry.setPosition({0.f, -1.f});
            berry.setFillColor(body);
            berry.setOutlineThickness(lineW);
            berry.setOutlineColor(edge);
            target.draw(berry, s);
            const sf::Color      seed = golden ? sf::Color(255, 245, 190) : sf::Color(255, 226, 140);
            const sf::Vector2f seeds[] = {{-7.f, -2.f}, {7.f, -3.f}, {-4.f, 6.f}, {4.f, 7.f}, {0.f, 11.f}, {-8.f, 3.f}, {8.f, 2.f}};
            for (auto p : seeds)
                fillEllipse(target, p, {0.8f, 1.3f}, seed, 0.f, 0.f, {}, s);
            for (int i = 0; i < 5; ++i)
            {
                const float a = (static_cast<float>(i) - 2.f) * 32.f;
                const float r = a * kPi / 180.f;
                fillEllipse(target, {std::sin(r) * 5.5f, -11.f - std::cos(r) * 1.5f}, {2.3f, 4.8f}, kLeaf, a, 1.f, kLeafDark, s);
            }
            fillRoundedRect(target, {{-1.f, -19.f}, {2.f, 6.f}}, 1.f, kLeafDark, 0.f, {}, s);
            fillEllipse(target, {-5.f, -6.f}, {2.5f, 1.5f}, sf::Color(255, 255, 255, 120), -30.f, 0.f, {}, s);
            if (face)
                drawFace(target, s, {0.f, 0.f}, 0.8f, blink);
            break;
        }
        case CropType::Sunflower:
        {
            const sf::Color petal     = body;
            const sf::Color petalEdge = darker(body, 0.85f);
            for (int i = 0; i < 12; ++i)
            {
                const float a = static_cast<float>(i) * 30.f;
                const float r = a * kPi / 180.f;
                fillEllipse(target, {std::cos(r) * 12.f, std::sin(r) * 12.f}, {7.5f, 3.6f}, petal, a, 1.f, petalEdge, s);
            }
            const sf::Color seedHead = golden ? sf::Color(214, 150, 52) : sf::Color(150, 96, 56);
            fillCircle(target, {0.f, 0.f}, 9.f, seedHead, 1.4f, darker(seedHead, 0.72f), s);
            fillCircle(target, {0.f, 0.f}, 6.f, lighter(seedHead, 0.1f), 0.f, {}, s);
            if (face)
                drawFace(target, s, {0.f, 0.f}, 0.72f, blink);
            break;
        }
        case CropType::Pumpkin:
        {
            fillRoundedRect(target, {{-2.5f, -20.f}, {5.f, 9.f}}, 2.f, sf::Color(112, 132, 60), 1.f, sf::Color(80, 98, 40), s);
            fillEllipse(target, {7.f, -17.f}, {5.f, 2.8f}, kLeaf, -25.f, 1.f, kLeafDark, s);
            fillEllipse(target, {-8.5f, 0.f}, {10.f, 12.f}, darker(body, 0.93f), 0.f, lineW, edge, s);
            fillEllipse(target, {8.5f, 0.f}, {10.f, 12.f}, darker(body, 0.93f), 0.f, lineW, edge, s);
            fillEllipse(target, {0.f, 0.f}, {10.f, 13.f}, body, 0.f, lineW, edge, s);
            fillEllipse(target, {-4.f, -6.f}, {2.6f, 1.6f}, sf::Color(255, 255, 255, 110), -30.f, 0.f, {}, s);
            if (face)
                drawFace(target, s, {0.f, 1.f}, 0.85f, blink);
            break;
        }
    }
}

// ---- icons ----------------------------------------------------------------------------
void drawCoin(sf::RenderTarget& target, sf::Vector2f center, float radius, float squash)
{
    sf::RenderStates s;
    s.transform.translate(center).scale({std::max(0.15f, squash), 1.f});
    fillCircle(target, {0.f, 0.f}, radius, sf::Color(250, 200, 62), radius * 0.16f, sf::Color(205, 148, 40), s);
    fillCircle(target, {0.f, 0.f}, radius * 0.62f, sf::Color(242, 184, 50), 0.f, {}, s);
    fillRoundedRect(target, {{-radius * 0.12f, -radius * 0.38f}, {radius * 0.24f, radius * 0.76f}}, radius * 0.1f, sf::Color(255, 226, 120), 0.f, {}, s);
    fillEllipse(target, {-radius * 0.38f, -radius * 0.42f}, {radius * 0.22f, radius * 0.14f}, sf::Color(255, 255, 255, 170), -35.f, 0.f, {}, s);
}

void drawSparkle(sf::RenderTarget& target, sf::Vector2f center, float size, sf::Color color, float rotationDeg)
{
    auto shape = star(size, size * 0.28f, 4);
    shape.setPosition(center);
    shape.setRotation(sf::degrees(rotationDeg));
    shape.setFillColor(color);
    target.draw(shape);
}

void drawWaterDrop(sf::RenderTarget& target, sf::Vector2f center, float size, sf::Color color)
{
    auto drop = teardrop(size, size * 1.5f);
    drop.setPosition(center);
    drop.setRotation(sf::degrees(180.f));
    drop.setFillColor(color);
    drop.setOutlineThickness(1.2f);
    drop.setOutlineColor(darker(color, 0.75f));
    target.draw(drop);
    fillEllipse(target, center + sf::Vector2f(-size * 0.35f, -size * 0.1f), {size * 0.22f, size * 0.32f}, sf::Color(255, 255, 255, 170), 20.f);
}

void drawWateringCan(sf::RenderTarget& target, sf::Vector2f center, float scale, float tiltDeg)
{
    const sf::RenderStates s = at({}, center, scale, tiltDeg);
    const sf::Color        body(112, 176, 226);
    const sf::Color        edge = darker(body, 0.72f);

    // handle (behind the body)
    fillCircle(target, {-1.f, -9.f}, 7.f, sf::Color::Transparent, 2.6f, edge, s);
    // spout
    fillRect(target, {{6.f, -6.f}, {15.f, 4.5f}}, edge, -38.f, s);
    fillRect(target, {{6.6f, -5.4f}, {14.f, 3.f}}, body, -38.f, s);
    fillEllipse(target, {19.5f, -12.f}, {3.6f, 2.4f}, lighter(body, 0.2f), -38.f, 1.f, edge, s);
    // body
    fillRoundedRect(target, {{-11.f, -7.f}, {22.f, 17.f}}, 4.f, body, 1.4f, edge, s);
    fillRoundedRect(target, {{-11.f, 1.f}, {22.f, 3.f}}, 1.f, lighter(body, 0.25f), 0.f, {}, s);
    fillEllipse(target, {-6.f, -3.f}, {2.4f, 1.5f}, sf::Color(255, 255, 255, 150), -20.f, 0.f, {}, s);
}

void drawSeedPacket(sf::RenderTarget& target, sf::Vector2f center, float scale, CropType type)
{
    const sf::RenderStates s     = at({}, center, scale);
    const sf::Color        paper(252, 241, 218);
    const sf::Color        band = lighter(Crop::info(type).color, 0.35f);
    fillRoundedRect(target, {{-12.f, -15.f}, {24.f, 30.f}}, 3.f, paper, 1.4f, sf::Color(196, 168, 128), s);
    fillRoundedRect(target, {{-12.f, -15.f}, {24.f, 7.f}}, 3.f, band, 0.f, {}, s);
    for (int i = 0; i < 4; ++i)
        fillCircle(target, {-7.5f + static_cast<float>(i) * 5.f, -8.f}, 2.5f, band, 0.f, {}, s);
    drawProduce(target, s, type, {0.f, 4.f}, 0.42f, false, false);
}

void drawSun(sf::RenderTarget& target, sf::Vector2f center, float scale, float spin)
{
    const sf::RenderStates s = at({}, center, scale, spin);
    for (int i = 0; i < 8; ++i)
    {
        sf::RenderStates r = s;
        r.transform.rotate(sf::degrees(static_cast<float>(i) * 45.f));
        fillRoundedRect(target, {{-1.6f, -15.f}, {3.2f, 5.f}}, 1.6f, sf::Color(250, 196, 70), 0.f, {}, r);
    }
    fillCircle(target, {0.f, 0.f}, 8.f, sf::Color(255, 214, 90), 1.2f, sf::Color(232, 168, 52), s);
}

void drawCloud(sf::RenderTarget& target, sf::Vector2f center, float scale, sf::Color color)
{
    const sf::RenderStates s    = at({}, center, scale);
    const sf::Color        edge = darker(color, 0.82f);
    for (int pass = 0; pass < 2; ++pass)
    {
        const float     o = pass == 0 ? 1.4f : 0.f;
        const sf::Color c = pass == 0 ? edge : color;
        fillCircle(target, {-7.f, 2.f}, 6.f + o, c, 0.f, {}, s);
        fillCircle(target, {0.f, -2.f}, 8.f + o, c, 0.f, {}, s);
        fillCircle(target, {8.f, 2.f}, 6.f + o, c, 0.f, {}, s);
        fillRoundedRect(target, {{-13.f - o, 0.f - o}, {26.f + 2 * o, 8.f + 2 * o}}, 4.f, c, 0.f, {}, s);
    }
}

void drawRainCloud(sf::RenderTarget& target, sf::Vector2f center, float scale)
{
    for (int i = 0; i < 3; ++i)
        drawWaterDrop(target, center + sf::Vector2f((static_cast<float>(i) - 1.f) * 7.f * scale, 13.f * scale), 2.2f * scale, sf::Color(110, 175, 230));
    drawCloud(target, center, scale, sf::Color(222, 228, 240));
}

void drawMoon(sf::RenderTarget& target, sf::Vector2f center, float radius, sf::Color background)
{
    fillCircle(target, center, radius, sf::Color(252, 226, 140));
    fillCircle(target, center + sf::Vector2f(radius * 0.45f, -radius * 0.3f), radius * 0.82f, background);
    drawSparkle(target, center + sf::Vector2f(radius * 0.9f, radius * 0.55f), radius * 0.35f, sf::Color(252, 226, 140));
}

void drawSpeaker(sf::RenderTarget& target, sf::Vector2f center, float scale, bool muted, sf::Color color)
{
    const sf::RenderStates s = at({}, center, scale);
    fillRoundedRect(target, {{-8.f, -3.5f}, {5.f, 7.f}}, 1.f, color, 0.f, {}, s);
    sf::ConvexShape cone(4);
    cone.setPoint(0, {-4.f, -3.5f});
    cone.setPoint(1, {2.f, -8.f});
    cone.setPoint(2, {2.f, 8.f});
    cone.setPoint(3, {-4.f, 3.5f});
    cone.setFillColor(color);
    target.draw(cone, s);
    if (muted)
    {
        fillRect(target, {{3.f, -1.f}, {9.f, 2.2f}}, color, 45.f, s);
        fillRect(target, {{3.f, -1.f}, {9.f, 2.2f}}, color, -45.f, s);
    }
    else
    {
        for (int ring = 0; ring < 2; ++ring)
        {
            const float r = 5.f + static_cast<float>(ring) * 4.f;
            for (int i = 0; i <= 6; ++i)
            {
                const float a = -0.8f + 1.6f * static_cast<float>(i) / 6.f;
                fillCircle(target, {2.f + std::cos(a) * r, std::sin(a) * r}, 1.f, color, 0.f, {}, s);
            }
        }
    }
}

void drawRibbon(sf::RenderTarget& target, sf::Vector2f center, float scale)
{
    const sf::RenderStates s = at({}, center, scale);
    const sf::Color        red(226, 86, 96);
    fillRoundedRect(target, {{-9.f, 4.f}, {8.f, 22.f}}, 2.f, red, 1.f, darker(red), [&] {
        sf::RenderStates r = s;
        r.transform.rotate(sf::degrees(18.f), {-5.f, 4.f});
        return r;
    }());
    fillRoundedRect(target, {{1.f, 4.f}, {8.f, 22.f}}, 2.f, red, 1.f, darker(red), [&] {
        sf::RenderStates r = s;
        r.transform.rotate(sf::degrees(-18.f), {5.f, 4.f});
        return r;
    }());
    const sf::Color petal(110, 170, 230);
    for (int i = 0; i < 12; ++i)
    {
        const float a = static_cast<float>(i) * kPi / 6.f;
        fillCircle(target, {std::cos(a) * 11.f, std::sin(a) * 11.f}, 5.f, petal, 1.f, darker(petal), s);
    }
    fillCircle(target, {0.f, 0.f}, 10.f, sf::Color(252, 210, 80), 1.4f, sf::Color(214, 160, 40), s);
    drawSparkle(target, center, 6.f * scale, sf::Color(255, 250, 220));
}

void drawSprout(sf::RenderTarget& target, sf::Vector2f center, float scale)
{
    const sf::RenderStates s = at({}, center, scale);
    fillRoundedRect(target, {{-1.5f, -4.f}, {3.f, 12.f}}, 1.5f, kLeafDark, 0.f, {}, s);
    fillEllipse(target, {-6.f, -5.f}, {6.5f, 3.6f}, kLeaf, -25.f, 1.2f, kLeafDark, s);
    fillEllipse(target, {6.f, -7.f}, {6.5f, 3.6f}, lighter(kLeaf, 0.1f), 25.f, 1.2f, kLeafDark, s);
}
} // namespace art
