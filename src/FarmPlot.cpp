#include "FarmPlot.hpp"

#include <cmath>
#include <istream>
#include <ostream>

#include "Art.hpp"

namespace
{
constexpr float kPi       = 3.14159265f;
constexpr float kBumpTime = 0.28f;

const sf::Color kSoil(178, 128, 88);
const sf::Color kWetSoil(122, 86, 62);
const sf::Color kLeaf(98, 172, 84);
const sf::Color kLeafDark(70, 140, 64);
const sf::Color kLeafDeep(78, 150, 72);
} // namespace

FarmPlot::FarmPlot(sf::Vector2f position, float size, bool unlocked) :
position(position),
size(size),
unlocked(unlocked),
phase(std::fmod(position.x * 0.37f + position.y * 0.61f, 6.28f))
{
}

void FarmPlot::update(float dt)
{
    if (bumpTimer > 0.f)
        bumpTimer = std::max(0.f, bumpTimer - dt);
}

bool FarmPlot::contains(sf::Vector2f point) const
{
    return getBounds().contains(point);
}

sf::FloatRect FarmPlot::getBounds() const
{
    return {position, {size, size}};
}

sf::Vector2f FarmPlot::getCenter() const
{
    return position + sf::Vector2f(size / 2.f, size / 2.f);
}

// ---- actions ----------------------------------------------------------------------
void FarmPlot::unlock()
{
    unlocked = true;
    bump();
}

bool FarmPlot::plant(CropType type)
{
    if (!unlocked || hasCrop)
        return false;
    crop      = Crop(type);
    hasCrop   = true;
    growth    = 0;
    ready     = false;
    golden    = false;
    regrowing = false;
    bump();
    return true;
}

bool FarmPlot::water()
{
    if (!unlocked || watered || ready)
        return false;
    watered = true;
    bump();
    return true;
}

void FarmPlot::grow(std::mt19937& rng)
{
    if (unlocked && hasCrop && !ready && watered)
    {
        ++growth;
        if (growth >= crop.getDaysToGrow())
        {
            ready     = true;
            regrowing = false;
            golden    = std::uniform_int_distribution<int>(1, 100)(rng) <= 8; // 8% chance of a golden crop
        }
    }
    watered = false;
}

int FarmPlot::harvest()
{
    if (!ready)
        return 0;

    const int value = crop.getSellPrice() * (golden ? 3 : 1);
    ready           = false;
    golden          = false;

    if (crop.getRegrowDays() > 0)
    {
        growth    = crop.getDaysToGrow() - crop.getRegrowDays();
        regrowing = true;
    }
    else
    {
        hasCrop   = false;
        growth    = 0;
        regrowing = false;
    }
    bump();
    return value;
}

void FarmPlot::bump()
{
    bumpTimer = kBumpTime;
}

// ---- queries --------------------------------------------------------------------------
bool FarmPlot::isUnlocked() const
{
    return unlocked;
}

bool FarmPlot::isEmpty() const
{
    return !hasCrop;
}

bool FarmPlot::isReady() const
{
    return ready;
}

bool FarmPlot::isWatered() const
{
    return watered;
}

bool FarmPlot::isGolden() const
{
    return golden;
}

const Crop& FarmPlot::getCrop() const
{
    return crop;
}

int FarmPlot::getDaysLeft() const
{
    if (!hasCrop || ready)
        return 0;
    return crop.getDaysToGrow() - growth;
}

int FarmPlot::stage() const
{
    if (!hasCrop)
        return -1;
    if (ready)
        return 3;
    if (regrowing)
        return 2;
    if (growth == 0)
        return 0;
    const float progress = static_cast<float>(growth) / static_cast<float>(crop.getDaysToGrow());
    return progress < 0.6f ? 1 : 2;
}

// ---- saving -----------------------------------------------------------------------------
void FarmPlot::save(std::ostream& out) const
{
    out << unlocked << ' ' << hasCrop << ' ' << static_cast<int>(crop.getType()) << ' ' << growth << ' ' << watered << ' '
        << ready << ' ' << golden << ' ' << regrowing;
}

bool FarmPlot::load(std::istream& in)
{
    int  type = 0;
    bool u = false, h = false, w = false, r = false, g = false, rg = false;
    int  gr = 0;
    if (!(in >> u >> h >> type >> gr >> w >> r >> g >> rg))
        return false;
    unlocked  = u;
    hasCrop   = h;
    crop      = Crop(Crop::fromIndex(type));
    growth    = std::max(0, gr);
    watered   = w;
    ready     = r;
    golden    = g;
    regrowing = rg;
    return true;
}

// ---- drawing ------------------------------------------------------------------------------
void FarmPlot::draw(sf::RenderTarget& target, float time, bool hovered) const
{
    const float      half = size / 2.f;
    sf::RenderStates base;
    base.transform.translate(getCenter());

    const sf::FloatRect tile{{-half, -half}, {size, size}};

    if (!unlocked)
    {
        // Wild grass waiting to be bought.
        art::fillRoundedRect(target, tile, 16.f, sf::Color(170, 210, 136, hovered ? 200 : 120), 2.5f, sf::Color(255, 255, 255, hovered ? 220 : 110), base);
        const sf::Color plus(255, 255, 255, hovered ? 235 : 150);
        art::fillRoundedRect(target, {{-12.f, -3.f}, {24.f, 6.f}}, 3.f, plus, 0.f, {}, base);
        art::fillRoundedRect(target, {{-3.f, -12.f}, {6.f, 24.f}}, 3.f, plus, 0.f, {}, base);
        return;
    }

    // Shadow, soil and furrows.
    const sf::Color soil = watered ? kWetSoil : kSoil;
    art::fillRoundedRect(target, {{-half, -half + 5.f}, {size, size}}, 16.f, sf::Color(70, 50, 30, 50), 0.f, {}, base);
    art::fillRoundedRect(target, tile, 16.f, soil, 3.f, art::darker(soil, 0.78f), base);
    for (int i = 0; i < 3; ++i)
    {
        const float y = -24.f + static_cast<float>(i) * 22.f;
        art::fillRoundedRect(target, {{-28.f, y}, {56.f, 5.f}}, 2.5f, art::darker(soil, 0.88f), 0.f, {}, base);
        art::fillRoundedRect(target, {{-26.f, y + 5.f}, {52.f, 2.f}}, 1.f, art::lighter(soil, 0.12f), 0.f, {}, base);
    }
    if (watered)
    {
        art::fillEllipse(target, {-20.f, 22.f}, {6.f, 2.f}, sf::Color(255, 255, 255, 40), 0.f, 0.f, {}, base);
        art::fillEllipse(target, {22.f, -10.f}, {4.f, 1.5f}, sf::Color(255, 255, 255, 40), 0.f, 0.f, {}, base);
    }

    drawCrop(target, base, time);

    if (hovered)
        art::fillRoundedRect(target, tile, 16.f, sf::Color::Transparent, 3.f, sf::Color(255, 250, 220, 230), base);

    // Little status bubbles in the corner.
    sf::Vector2f corner = getCenter() + sf::Vector2f(half - 10.f, -half + 10.f);
    if (hasCrop && !ready)
    {
        if (watered)
        {
            art::drawWaterDrop(target, corner, 4.f, sf::Color(110, 180, 235));
        }
        else
        {
            corner.y += std::sin(time * 3.f + phase) * 2.f;
            art::fillCircle(target, corner, 9.f, sf::Color(255, 255, 255, 235), 1.5f, sf::Color(170, 200, 220));
            art::drawWaterDrop(target, corner + sf::Vector2f(0.f, 1.f), 3.4f, sf::Color(160, 205, 240));
        }
    }
    else if (ready)
    {
        const float twinkle = 0.6f + 0.4f * std::sin(time * 5.f + phase);
        art::drawSparkle(target, corner, 7.f * twinkle, golden ? sf::Color(255, 220, 90) : sf::Color(255, 255, 240), time * 40.f);
    }
}

void FarmPlot::drawCrop(sf::RenderTarget& target, sf::RenderStates states, float time) const
{
    const int st = stage();
    if (st < 0)
        return;

    float squish = 0.f;
    if (bumpTimer > 0.f)
        squish = std::sin((1.f - bumpTimer / kBumpTime) * kPi) * 0.2f;
    const float sway = std::sin(time * 2.2f + phase) * (st >= 2 ? 3.f : 2.f);

    sf::RenderStates s = states;
    s.transform.translate({0.f, 14.f}); // soil line
    s.transform.scale({1.f + squish, 1.f - squish});
    s.transform.rotate(sf::degrees(sway));

    const bool  blink = std::fmod(time + phase * 2.f, 4.2f) < 0.13f;
    const float bob   = st == 3 ? std::sin(time * 3.f + phase) * 1.5f : 0.f;
    const auto  type  = crop.getType();

    if (st == 0)
    {
        art::fillEllipse(target, {0.f, 0.f}, {15.f, 6.f}, art::darker(kSoil, 0.85f), 0.f, 0.f, {}, s);
        for (sf::Vector2f p : {sf::Vector2f(-6.f, -1.f), sf::Vector2f(0.f, -3.f), sf::Vector2f(6.f, -1.f)})
            art::fillEllipse(target, p, {2.f, 1.4f}, sf::Color(246, 226, 170), 0.f, 0.f, {}, s);
        return;
    }

    if (st == 1)
    {
        art::fillRoundedRect(target, {{-1.5f, -14.f}, {3.f, 14.f}}, 1.5f, kLeafDark, 0.f, {}, s);
        art::fillEllipse(target, {-6.f, -14.f}, {6.5f, 3.6f}, kLeaf, -25.f, 1.2f, kLeafDark, s);
        art::fillEllipse(target, {6.f, -16.f}, {6.5f, 3.6f}, art::lighter(kLeaf, 0.1f), 25.f, 1.2f, kLeafDark, s);
        return;
    }

    auto strawberryBush = [&]
    {
        art::fillCircle(target, {-11.f, -9.f}, 10.f, kLeafDeep, 1.4f, kLeafDark, s);
        art::fillCircle(target, {11.f, -9.f}, 10.f, kLeafDeep, 1.4f, kLeafDark, s);
        art::fillCircle(target, {0.f, -17.f}, 11.f, kLeaf, 1.4f, kLeafDark, s);
    };
    auto pumpkinVine = [&]
    {
        art::fillCircle(target, {-18.f, -18.f}, 4.f, sf::Color::Transparent, 1.6f, kLeafDark, s);
        art::fillEllipse(target, {-14.f, -8.f}, {11.f, 7.f}, kLeafDeep, -15.f, 1.4f, kLeafDark, s);
        art::fillEllipse(target, {13.f, -10.f}, {10.f, 7.f}, kLeaf, 20.f, 1.4f, kLeafDark, s);
    };

    if (st == 2)
    {
        switch (type)
        {
            case CropType::Carrot:
                art::fillEllipse(target, {-7.f, -14.f}, {3.6f, 11.f}, kLeaf, -22.f, 1.2f, kLeafDark, s);
                art::fillEllipse(target, {7.f, -14.f}, {3.6f, 11.f}, kLeaf, 22.f, 1.2f, kLeafDark, s);
                art::fillEllipse(target, {0.f, -18.f}, {3.8f, 13.f}, art::lighter(kLeaf, 0.1f), 0.f, 1.2f, kLeafDark, s);
                art::fillEllipse(target, {0.f, -1.f}, {7.f, 3.5f}, Crop::info(type).color, 0.f, 1.f, art::darker(Crop::info(type).color), s);
                break;
            case CropType::Strawberry:
                strawberryBush();
                for (sf::Vector2f f : {sf::Vector2f(-7.f, -15.f), sf::Vector2f(8.f, -9.f)})
                {
                    for (int i = 0; i < 5; ++i)
                    {
                        const float a = static_cast<float>(i) * 2.f * kPi / 5.f;
                        art::fillCircle(target, f + sf::Vector2f(std::cos(a), std::sin(a)) * 3.f, 2.6f, sf::Color::White, 0.f, {}, s);
                    }
                    art::fillCircle(target, f, 1.9f, sf::Color(250, 206, 80), 0.f, {}, s);
                }
                break;
            case CropType::Sunflower:
                art::fillRoundedRect(target, {{-1.8f, -38.f}, {3.6f, 38.f}}, 1.8f, kLeafDark, 0.f, {}, s);
                art::fillEllipse(target, {-8.f, -14.f}, {8.f, 3.8f}, kLeaf, -20.f, 1.2f, kLeafDark, s);
                art::fillEllipse(target, {8.f, -25.f}, {8.f, 3.8f}, kLeaf, 20.f, 1.2f, kLeafDark, s);
                for (int i = 0; i < 4; ++i)
                {
                    const float a = -90.f + (static_cast<float>(i) - 1.5f) * 30.f;
                    const float r = a * kPi / 180.f;
                    art::fillEllipse(target, {std::cos(r) * 6.f, -40.f + std::sin(r) * 6.f}, {3.5f, 2.f}, sf::Color(252, 210, 80), a, 0.f, {}, s);
                }
                art::fillCircle(target, {0.f, -40.f}, 6.f, kLeaf, 1.2f, kLeafDark, s);
                break;
            case CropType::Pumpkin:
                pumpkinVine();
                art::fillEllipse(target, {1.f, -5.f}, {8.f, 6.f}, sf::Color(166, 200, 96), 0.f, 1.2f, sf::Color(120, 156, 64), s);
                art::fillRoundedRect(target, {{0.f, -13.f}, {2.f, 4.f}}, 1.f, kLeafDark, 0.f, {}, s);
                break;
        }
        return;
    }

    // st == 3: ripe and ready to harvest
    switch (type)
    {
        case CropType::Carrot:
            art::drawProduce(target, s, type, {0.f, -13.f + bob}, 1.05f, golden, true, blink);
            art::fillEllipse(target, {0.f, 3.f}, {17.f, 6.f}, art::darker(kSoil, 0.9f), 0.f, 0.f, {}, s);
            break;
        case CropType::Strawberry:
            strawberryBush();
            art::drawProduce(target, s, type, {-13.f, -6.f}, 0.5f, false, false);
            art::drawProduce(target, s, type, {13.f, -8.f}, 0.5f, false, false);
            art::drawProduce(target, s, type, {0.f, -13.f + bob}, 0.95f, golden, true, blink);
            break;
        case CropType::Sunflower:
            art::fillRoundedRect(target, {{-1.8f, -44.f}, {3.6f, 44.f}}, 1.8f, kLeafDark, 0.f, {}, s);
            art::fillEllipse(target, {-9.f, -14.f}, {9.f, 4.f}, kLeaf, -20.f, 1.2f, kLeafDark, s);
            art::fillEllipse(target, {9.f, -26.f}, {9.f, 4.f}, kLeaf, 20.f, 1.2f, kLeafDark, s);
            art::drawProduce(target, s, type, {0.f, -46.f + bob}, 0.95f, golden, true, blink);
            break;
        case CropType::Pumpkin:
            pumpkinVine();
            art::drawProduce(target, s, type, {0.f, -12.f + bob * 0.5f}, 1.15f, golden, true, blink);
            break;
    }
}
