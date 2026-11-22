#include "Scenery.hpp"

#include <algorithm>
#include <cmath>

#include "Art.hpp"
#include "Layout.hpp"
#include "Ui.hpp"

namespace
{
constexpr float kPi = 3.14159265f;

const sf::Vector2f kPondCenter(130.f, 556.f);
const sf::Vector2f kPondRadii(78.f, 34.f);
const sf::Vector2f kChimneyTop(147.f, 146.f);

void drawTuft(sf::RenderTarget& target, sf::Vector2f pos, sf::Color color)
{
    art::fillEllipse(target, pos + sf::Vector2f(-3.f, 0.f), {1.4f, 5.f}, color, -22.f);
    art::fillEllipse(target, pos + sf::Vector2f(0.f, -1.f), {1.4f, 6.f}, color, 0.f);
    art::fillEllipse(target, pos + sf::Vector2f(3.f, 0.f), {1.4f, 5.f}, color, 22.f);
}

void drawFlower(sf::RenderTarget& target, sf::Vector2f pos, sf::Color petal)
{
    art::fillRoundedRect(target, {pos + sf::Vector2f(-0.7f, 2.f), {1.4f, 6.f}}, 0.7f, sf::Color(90, 150, 76));
    for (int i = 0; i < 5; ++i)
    {
        const float a = static_cast<float>(i) * 2.f * kPi / 5.f - kPi / 2.f;
        art::fillCircle(target, pos + sf::Vector2f(std::cos(a), std::sin(a)) * 2.7f, 2.4f, petal);
    }
    art::fillCircle(target, pos, 1.6f, sf::Color(250, 206, 80));
}

void drawTree(sf::RenderTarget& target, sf::Vector2f base, float s, bool apples)
{
    const sf::Color leaf(104, 172, 88), leafDark(76, 140, 66), trunk(150, 102, 68);
    art::fillEllipse(target, base + sf::Vector2f(0.f, 2.f), {40.f * s, 11.f * s}, sf::Color(40, 70, 30, 50));
    art::fillRoundedRect(target, {base + sf::Vector2f(-7.f * s, -36.f * s), {14.f * s, 38.f * s}}, 5.f * s, trunk, 2.f, art::darker(trunk));

    const sf::Vector3f blobs[] = {{0.f, -62.f, 30.f}, {-24.f, -46.f, 22.f}, {24.f, -46.f, 22.f}, {0.f, -38.f, 24.f}};
    for (auto b : blobs)
        art::fillCircle(target, base + sf::Vector2f(b.x, b.y) * s, (b.z + 2.5f) * s, leafDark);
    for (auto b : blobs)
        art::fillCircle(target, base + sf::Vector2f(b.x, b.y) * s, b.z * s, leaf);
    art::fillCircle(target, base + sf::Vector2f(-8.f, -72.f) * s, 9.f * s, art::lighter(leaf, 0.18f));
    art::fillCircle(target, base + sf::Vector2f(-26.f, -52.f) * s, 6.f * s, art::lighter(leaf, 0.14f));

    if (apples)
    {
        for (sf::Vector2f a : {sf::Vector2f(-14.f, -60.f), sf::Vector2f(12.f, -70.f), sf::Vector2f(20.f, -44.f), sf::Vector2f(-22.f, -38.f), sf::Vector2f(4.f, -42.f)})
        {
            art::fillCircle(target, base + a * s, 4.2f * s, sf::Color(232, 82, 92), 1.f, sf::Color(180, 52, 64));
            art::fillCircle(target, base + (a + sf::Vector2f(-1.2f, -1.4f)) * s, 1.2f * s, sf::Color(255, 220, 220));
        }
    }
}

void drawBush(sf::RenderTarget& target, sf::Vector2f pos, float s)
{
    const sf::Color leaf(112, 180, 94), dark(80, 146, 70);
    art::fillEllipse(target, pos + sf::Vector2f(0.f, 8.f * s), {26.f * s, 7.f * s}, sf::Color(40, 70, 30, 45));
    for (sf::Vector3f b : {sf::Vector3f(-12.f, 0.f, 12.f), sf::Vector3f(12.f, 0.f, 12.f), sf::Vector3f(0.f, -6.f, 14.f)})
        art::fillCircle(target, pos + sf::Vector2f(b.x, b.y) * s, (b.z + 2.f) * s, dark);
    for (sf::Vector3f b : {sf::Vector3f(-12.f, 0.f, 12.f), sf::Vector3f(12.f, 0.f, 12.f), sf::Vector3f(0.f, -6.f, 14.f)})
        art::fillCircle(target, pos + sf::Vector2f(b.x, b.y) * s, b.z * s, leaf);
    art::fillCircle(target, pos + sf::Vector2f(-4.f, -12.f) * s, 4.f * s, art::lighter(leaf, 0.2f));
}

void drawPost(sf::RenderTarget& target, sf::Vector2f center, float size = 14.f)
{
    const sf::Color wood(184, 132, 88);
    art::fillRoundedRect(target, {center - sf::Vector2f(size / 2.f, size / 2.f - 3.f), {size, size}}, 4.f, sf::Color(70, 50, 30, 60));
    art::fillRoundedRect(target, {center - sf::Vector2f(size / 2.f, size / 2.f), {size, size}}, 4.f, wood, 2.f, art::darker(wood, 0.72f));
    art::fillRoundedRect(target, {center - sf::Vector2f(size / 2.f - 3.f, size / 2.f - 3.f), {size - 6.f, 3.f}}, 1.5f, art::lighter(wood, 0.25f));
}
} // namespace

Scenery::Scenery() : rng(std::random_device{}())
{
    for (int i = 0; i < 3; ++i)
        butterflies.push_back({{random(50.f, 950.f), random(120.f, 560.f)}, {500.f, 300.f},
                               i == 0 ? sf::Color(250, 180, 200) : (i == 1 ? sf::Color(190, 170, 240) : sf::Color(255, 226, 120)),
                               random(0.f, 10.f)});
    for (int i = 0; i < 4; ++i)
        clouds.push_back({{random(-200.f, 1000.f), random(120.f, 600.f)}, random(0.8f, 1.5f), random(10.f, 22.f)});
    for (int i = 0; i < 160; ++i)
        rain.push_back({{random(0.f, 1100.f), random(-50.f, 700.f)}, random(520.f, 720.f)});
}

float Scenery::random(float min, float max)
{
    return std::uniform_real_distribution<float>(min, max)(rng);
}

void Scenery::build()
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 4;
    if (!background.resize({layout::kWidth, layout::kHeight}, settings))
    {
        if (!background.resize({layout::kWidth, layout::kHeight}))
            return;
    }
    background.setSmooth(true);
    paintBackground(background);
    background.display();
    built = true;
}

void Scenery::paintBackground(sf::RenderTarget& target)
{
    using namespace layout;
    std::mt19937 local(20240601); // fixed seed: the same cosy farm every time
    auto rnd = [&](float a, float b) { return std::uniform_real_distribution<float>(a, b)(local); };

    target.clear(sf::Color(150, 198, 118));

    // Soft vertical grass gradient.
    sf::VertexArray grad(sf::PrimitiveType::TriangleStrip, 4);
    grad[0] = {{0.f, 0.f}, sf::Color(166, 210, 130)};
    grad[1] = {{static_cast<float>(kWidth), 0.f}, sf::Color(166, 210, 130)};
    grad[2] = {{0.f, static_cast<float>(kHeight)}, sf::Color(138, 188, 110)};
    grad[3] = {{static_cast<float>(kWidth), static_cast<float>(kHeight)}, sf::Color(138, 188, 110)};
    target.draw(grad);

    // Lighter mowed patches.
    for (int i = 0; i < 14; ++i)
        art::fillEllipse(target, {rnd(0.f, 1000.f), rnd(80.f, 700.f)}, {rnd(40.f, 110.f), rnd(18.f, 40.f)}, sf::Color(255, 255, 230, 18));

    // Field bed inside the fence.
    art::fillRoundedRect(target, {{kFenceLeft + 6.f, kFenceTop + 6.f}, {kFenceRight - kFenceLeft - 12.f, kFenceBottom - kFenceTop - 12.f}}, 18.f, sf::Color(132, 178, 104));

    // Sandy path from the gate down to the toolbar.
    const sf::Color sand(232, 210, 164);
    art::fillRoundedRect(target, {{kGateLeft + 2.f, kFenceBottom - 14.f}, {kGateRight - kGateLeft - 4.f, 240.f}}, 20.f, art::darker(sand, 0.9f));
    art::fillRoundedRect(target, {{kGateLeft + 5.f, kFenceBottom - 12.f}, {kGateRight - kGateLeft - 10.f, 240.f}}, 18.f, sand);
    for (int i = 0; i < 9; ++i)
        art::fillEllipse(target, {rnd(kGateLeft + 12.f, kGateRight - 12.f), rnd(kFenceBottom + 6.f, 600.f)}, {rnd(2.f, 4.f), rnd(1.5f, 2.5f)}, sf::Color(205, 182, 140));

    // Stepping stones from the house.
    for (sf::Vector2f p : {sf::Vector2f(110.f, 322.f), sf::Vector2f(122.f, 348.f), sf::Vector2f(142.f, 370.f), sf::Vector2f(168.f, 386.f)})
    {
        art::fillEllipse(target, p + sf::Vector2f(0.f, 2.f), {12.f, 6.f}, sf::Color(90, 110, 80, 60));
        art::fillEllipse(target, p, {12.f, 6.f}, sf::Color(214, 208, 196), 0.f, 1.5f, sf::Color(170, 162, 150));
    }

    // Grass tufts everywhere outside the fence.
    for (int i = 0; i < 260; ++i)
    {
        sf::Vector2f p{rnd(0.f, 1000.f), rnd(80.f, 700.f)};
        if (p.x > kFenceLeft - 10.f && p.x < kFenceRight + 10.f && p.y > kFenceTop - 10.f && p.y < kFenceBottom + 10.f)
            continue;
        drawTuft(target, p, sf::Color(104, 156, 84, 170));
    }

    // Pond.
    art::fillEllipse(target, kPondCenter + sf::Vector2f(0.f, 3.f), kPondRadii + sf::Vector2f(10.f, 8.f), sf::Color(200, 186, 150));
    art::fillEllipse(target, kPondCenter, kPondRadii + sf::Vector2f(8.f, 6.f), sf::Color(226, 214, 176));
    art::fillEllipse(target, kPondCenter, kPondRadii, sf::Color(112, 182, 222));
    art::fillEllipse(target, kPondCenter + sf::Vector2f(0.f, 6.f), kPondRadii - sf::Vector2f(14.f, 12.f), sf::Color(96, 166, 210));
    art::fillEllipse(target, kPondCenter + sf::Vector2f(-34.f, -14.f), {16.f, 3.f}, sf::Color(255, 255, 255, 90));
    for (sf::Vector2f lp : {sf::Vector2f(78.f, 560.f), sf::Vector2f(186.f, 548.f), sf::Vector2f(170.f, 572.f)})
    {
        art::fillEllipse(target, lp, {9.f, 5.f}, sf::Color(108, 176, 96), 0.f, 1.2f, sf::Color(76, 140, 66));
        art::fillRect(target, {lp + sf::Vector2f(2.f, -1.f), {8.f, 2.f}}, sf::Color(96, 166, 210), 20.f);
    }
    drawFlower(target, {186.f, 546.f}, sf::Color(250, 176, 200));
    for (float x : {52.f, 58.f, 206.f, 212.f})
    {
        art::fillRoundedRect(target, {{x, 520.f}, {3.f, 24.f}}, 1.5f, sf::Color(96, 150, 76));
        art::fillEllipse(target, {x + 1.5f, 520.f}, {2.5f, 6.f}, sf::Color(150, 110, 70));
    }

    // Flowers dotted around.
    const sf::Color petals[] = {{255, 255, 255}, {250, 176, 200}, {255, 222, 120}, {200, 180, 245}, {255, 190, 150}};
    for (int i = 0; i < 70; ++i)
    {
        sf::Vector2f p{rnd(10.f, 990.f), rnd(90.f, 690.f)};
        if (p.x > kFenceLeft - 16.f && p.x < kFenceRight + 16.f && p.y > kFenceTop - 16.f && p.y < kFenceBottom + 16.f)
            continue;
        if (p.x > kGateLeft - 6.f && p.x < kGateRight + 6.f && p.y > kFenceBottom)
            continue;
        if (p.x < 200.f && p.y > 120.f && p.y < 400.f)
            continue; // house
        if ((p - kPondCenter).x * (p - kPondCenter).x / (100.f * 100.f) + (p - kPondCenter).y * (p - kPondCenter).y / (50.f * 50.f) < 1.f)
            continue;
        if (p.y > 600.f)
            continue; // toolbar area
        drawFlower(target, p, petals[i % 5]);
    }

    // ---- House -----------------------------------------------------------------
    art::fillEllipse(target, {110.f, 304.f}, {82.f, 12.f}, sf::Color(40, 70, 30, 55));
    const sf::Color wall(250, 238, 212), wallEdge(196, 168, 128), roof(224, 114, 104);
    art::fillRoundedRect(target, {{138.f, 150.f}, {18.f, 50.f}}, 3.f, sf::Color(196, 118, 96), 2.f, sf::Color(150, 84, 66));
    art::fillRoundedRect(target, {{135.f, 146.f}, {24.f, 8.f}}, 3.f, sf::Color(176, 104, 84));
    art::fillRoundedRect(target, {{52.f, 208.f}, {116.f, 94.f}}, 6.f, wall, 3.f, wallEdge);
    for (int i = 0; i < 4; ++i)
        art::fillRect(target, {{56.f, 228.f + static_cast<float>(i) * 18.f}, {108.f, 1.5f}}, sf::Color(230, 214, 184));

    sf::ConvexShape roofShape(3);
    roofShape.setPoint(0, {34.f, 220.f});
    roofShape.setPoint(1, {110.f, 148.f});
    roofShape.setPoint(2, {186.f, 220.f});
    roofShape.setFillColor(roof);
    roofShape.setOutlineThickness(3.f);
    roofShape.setOutlineColor(art::darker(roof, 0.75f));
    target.draw(roofShape);
    for (int i = 0; i < 12; ++i)
        art::fillCircle(target, {40.f + static_cast<float>(i) * 12.6f, 221.f}, 6.5f, art::darker(roof, 0.88f));
    art::fillCircle(target, {110.f, 190.f}, 11.f, sf::Color(184, 222, 242), 3.f, sf::Color(170, 120, 80));
    art::fillRect(target, {{109.f, 180.f}, {2.f, 20.f}}, sf::Color(170, 120, 80));
    art::fillRect(target, {{100.f, 189.f}, {20.f, 2.f}}, sf::Color(170, 120, 80));

    for (float wx : {62.f, 132.f})
    {
        art::fillRoundedRect(target, {{wx, 236.f}, {26.f, 24.f}}, 3.f, sf::Color(184, 222, 242), 2.5f, sf::Color(170, 120, 80));
        art::fillRect(target, {{wx + 12.f, 236.f}, {2.f, 24.f}}, sf::Color(170, 120, 80));
        art::fillRect(target, {{wx, 247.f}, {26.f, 2.f}}, sf::Color(170, 120, 80));
        art::fillEllipse(target, {wx + 7.f, 242.f}, {3.f, 2.f}, sf::Color(255, 255, 255, 150), -30.f);
        art::fillRoundedRect(target, {{wx - 3.f, 262.f}, {32.f, 7.f}}, 2.f, sf::Color(170, 120, 80));
        for (int f = 0; f < 4; ++f)
            art::fillCircle(target, {wx + 2.f + static_cast<float>(f) * 7.f, 260.f}, 2.6f, f % 2 ? sf::Color(250, 176, 200) : sf::Color(255, 222, 120));
    }
    art::fillRoundedRect(target, {{96.f, 252.f}, {28.f, 50.f}}, 10.f, sf::Color(176, 118, 76), 2.5f, sf::Color(130, 84, 52));
    art::fillRect(target, {{109.f, 256.f}, {2.f, 44.f}}, sf::Color(150, 98, 62));
    art::fillCircle(target, {119.f, 279.f}, 2.2f, sf::Color(250, 204, 80));
    art::fillRoundedRect(target, {{94.f, 302.f}, {32.f, 6.f}}, 3.f, sf::Color(214, 120, 110));

    // ---- Trees, bushes and a produce crate on the right --------------------------
    drawTree(target, {890.f, 220.f}, 1.05f, true);
    drawTree(target, {948.f, 372.f}, 0.8f, false);
    drawTree(target, {40.f, 470.f}, 0.75f, false);
    drawBush(target, {840.f, 330.f}, 0.8f);
    drawBush(target, {182.f, 440.f}, 0.85f);
    drawBush(target, {960.f, 520.f}, 0.9f);

    const sf::Color crate(196, 146, 96);
    art::fillEllipse(target, {868.f, 492.f}, {46.f, 9.f}, sf::Color(40, 70, 30, 55));
    art::drawProduce(target, {}, CropType::Pumpkin, {856.f, 446.f}, 0.85f, false, true);
    art::drawProduce(target, {}, CropType::Carrot, {884.f, 444.f}, 0.75f, false, true);
    art::drawProduce(target, {}, CropType::Strawberry, {838.f, 452.f}, 0.6f, false, false);
    art::fillRoundedRect(target, {{826.f, 454.f}, {84.f, 38.f}}, 5.f, crate, 2.5f, art::darker(crate, 0.72f));
    art::fillRect(target, {{830.f, 466.f}, {76.f, 2.f}}, art::darker(crate, 0.82f));
    art::fillRect(target, {{830.f, 478.f}, {76.f, 2.f}}, art::darker(crate, 0.82f));

    // ---- Fence -------------------------------------------------------------------------
    const sf::Color rail(198, 146, 100);
    auto railH = [&](float x1, float x2, float y)
    { art::fillRoundedRect(target, {{x1, y - 3.f}, {x2 - x1, 6.f}}, 3.f, rail, 1.5f, art::darker(rail, 0.72f)); };
    auto railV = [&](float x, float y1, float y2)
    { art::fillRoundedRect(target, {{x - 3.f, y1}, {6.f, y2 - y1}}, 3.f, rail, 1.5f, art::darker(rail, 0.72f)); };

    railH(kFenceLeft, kFenceRight, kFenceTop);
    railV(kFenceLeft, kFenceTop, kFenceBottom);
    railV(kFenceRight, kFenceTop, kFenceBottom);
    railH(kFenceLeft, kGateLeft, kFenceBottom);
    railH(kGateRight, kFenceRight, kFenceBottom);

    for (float x = kFenceLeft; x <= kFenceRight + 1.f; x += 48.67f)
        drawPost(target, {x, kFenceTop});
    for (float y = kFenceTop + 49.5f; y < kFenceBottom; y += 49.5f)
    {
        drawPost(target, {kFenceLeft, y});
        drawPost(target, {kFenceRight, y});
    }
    for (float x = kFenceLeft; x <= kGateLeft - 20.f; x += 51.6f)
        drawPost(target, {x, kFenceBottom});
    for (float x = kFenceRight; x >= kGateRight + 20.f; x -= 51.6f)
        drawPost(target, {x, kFenceBottom});
    drawPost(target, {kGateLeft, kFenceBottom}, 18.f);
    drawPost(target, {kGateRight, kFenceBottom}, 18.f);
}

void Scenery::setWeather(Weather w)
{
    weather = w;
}

void Scenery::update(float dt)
{
    time += dt;

    // Chimney smoke.
    puffTimer -= dt;
    if (puffTimer <= 0.f)
    {
        puffTimer = 0.8f;
        puffs.push_back({kChimneyTop, 0.f});
    }
    for (auto& p : puffs)
    {
        p.life += dt;
        p.pos += sf::Vector2f(10.f + std::sin(p.life * 2.f) * 6.f, -16.f) * dt;
    }
    puffs.erase(std::remove_if(puffs.begin(), puffs.end(), [](const Puff& p) { return p.life > 3.f; }), puffs.end());

    // Butterflies drift between random spots.
    for (auto& b : butterflies)
    {
        b.t += dt;
        sf::Vector2f d    = b.target - b.pos;
        const float  dist = std::sqrt(d.x * d.x + d.y * d.y);
        if (dist < 10.f)
            b.target = {random(40.f, 960.f), random(110.f, 580.f)};
        else
            b.pos += d / dist * 40.f * dt + sf::Vector2f(std::sin(b.t * 3.1f), std::cos(b.t * 2.3f)) * 30.f * dt;
    }

    const float cloudSpeed = weather == Weather::Sunny ? 1.f : 1.6f;
    for (auto& c : clouds)
    {
        c.pos.x += c.speed * cloudSpeed * dt;
        if (c.pos.x > 1180.f)
            c.pos = {-180.f, random(120.f, 600.f)};
    }

    if (weather == Weather::Rainy)
    {
        for (auto& r : rain)
        {
            r.pos += sf::Vector2f(-0.25f, 1.f) * r.speed * dt;
            if (r.pos.y > 700.f)
            {
                if (random(0.f, 1.f) < 0.35f)
                    ripples.push_back({{r.pos.x, random(90.f, 690.f)}, 0.f});
                r.pos = {random(0.f, 1150.f), random(-60.f, -10.f)};
            }
        }
    }
    for (auto& r : ripples)
        r.life += dt;
    ripples.erase(std::remove_if(ripples.begin(), ripples.end(), [](const Ripple& r) { return r.life > 0.5f; }), ripples.end());
}

void Scenery::drawGround(sf::RenderTarget& target) const
{
    if (built)
    {
        sf::Sprite sprite(background.getTexture());
        target.draw(sprite);
    }

    drawDuck(target);

    for (const auto& p : puffs)
    {
        const float k = p.life / 3.f;
        art::fillCircle(target, p.pos, 5.f + k * 10.f, sf::Color(250, 250, 250, static_cast<std::uint8_t>(150.f * (1.f - k))));
    }
}

void Scenery::drawDuck(sf::RenderTarget& target) const
{
    const float        a      = time * 0.25f;
    const sf::Vector2f pos    = kPondCenter + sf::Vector2f(std::cos(a) * 42.f, std::sin(a) * 12.f + std::sin(time * 3.f) * 1.f);
    const float        facing = -std::sin(a) >= 0.f ? 1.f : -1.f; // direction of travel along x

    art::fillEllipse(target, pos + sf::Vector2f(0.f, 7.f), {18.f, 4.f}, sf::Color(255, 255, 255, 60));
    art::fillEllipse(target, pos, {12.f, 8.f}, sf::Color(255, 255, 255), 0.f, 1.5f, sf::Color(200, 200, 210));
    art::fillEllipse(target, pos + sf::Vector2f(-3.f * facing, -1.f), {6.f, 4.f}, sf::Color(236, 236, 244), facing * 15.f);
    const sf::Vector2f head = pos + sf::Vector2f(9.f * facing, -9.f);
    art::fillCircle(target, head, 6.f, sf::Color(255, 255, 255), 1.5f, sf::Color(200, 200, 210));
    art::fillEllipse(target, head + sf::Vector2f(6.f * facing, 1.f), {4.f, 2.f}, sf::Color(250, 170, 60));
    art::fillCircle(target, head + sf::Vector2f(2.f * facing, -1.f), 1.3f, sf::Color(60, 44, 40));
}

void Scenery::drawAmbient(sf::RenderTarget& target) const
{
    const std::uint8_t shade = weather == Weather::Sunny ? 16 : 26;
    for (const auto& c : clouds)
    {
        const sf::Color col(40, 60, 70, shade);
        art::fillEllipse(target, c.pos, sf::Vector2f(70.f, 26.f) * c.scale, col);
        art::fillEllipse(target, c.pos + sf::Vector2f(-46.f, 8.f) * c.scale, sf::Vector2f(40.f, 18.f) * c.scale, col);
        art::fillEllipse(target, c.pos + sf::Vector2f(50.f, 6.f) * c.scale, sf::Vector2f(44.f, 18.f) * c.scale, col);
    }

    if (weather == Weather::Rainy)
        return;
    for (const auto& b : butterflies)
    {
        const float flap = 0.25f + 0.75f * std::abs(std::sin(b.t * 14.f));
        for (float side : {-1.f, 1.f})
        {
            sf::RenderStates s;
            s.transform.translate(b.pos).scale({flap, 1.f});
            art::fillEllipse(target, {side * 5.f, -2.f}, {5.f, 4.f}, b.color, side * 20.f, 1.f, art::darker(b.color, 0.75f), s);
            art::fillEllipse(target, {side * 4.f, 3.f}, {3.5f, 3.f}, art::lighter(b.color, 0.2f), -side * 20.f, 1.f, art::darker(b.color, 0.75f), s);
        }
        art::fillEllipse(target, b.pos, {1.4f, 4.5f}, sf::Color(80, 60, 60));
    }
}

void Scenery::drawWeather(sf::RenderTarget& target) const
{
    if (weather == Weather::Cloudy)
    {
        art::fillRect(target, {{0.f, 0.f}, {1000.f, 700.f}}, sf::Color(90, 100, 130, 22));
        return;
    }
    if (weather != Weather::Rainy)
        return;

    art::fillRect(target, {{0.f, 0.f}, {1000.f, 700.f}}, sf::Color(70, 90, 140, 50));
    for (const auto& r : ripples)
    {
        const float k = r.life / 0.5f;
        art::fillEllipse(target, r.pos, sf::Vector2f(3.f + 8.f * k, 1.5f + 3.f * k), sf::Color::Transparent, 0.f, 1.2f,
                         sf::Color(220, 235, 255, static_cast<std::uint8_t>(150.f * (1.f - k))));
    }
    for (const auto& r : rain)
        art::fillRect(target, {r.pos, {1.6f, 13.f}}, sf::Color(210, 228, 255, 150), 14.f);
}
