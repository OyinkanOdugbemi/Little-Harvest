#include "Game.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "Art.hpp"
#include "Layout.hpp"
#include "Ui.hpp"

namespace
{
constexpr float kPi = 3.14159265f;

const char* const kSaveFile = "littleharvest_save.txt";

constexpr float kSleepFadeOut = 0.8f;
constexpr float kSleepHold    = 1.9f;
constexpr float kSleepEnd     = 2.7f;

const sf::Vector2f kCoinIcon(726.f, 42.f);

const sf::Color kInfo(122, 178, 98);
const sf::Color kWarn(232, 108, 118);
const sf::Color kGood(250, 196, 70);
const sf::Color kNight(38, 36, 72);

std::string withCommas(int value)
{
    std::string digits = std::to_string(std::abs(value));
    for (int i = static_cast<int>(digits.size()) - 3; i > 0; i -= 3)
        digits.insert(static_cast<std::size_t>(i), ",");
    return value < 0 ? "-" + digits : digits;
}

std::string plural(int n, const std::string& word)
{
    return std::to_string(n) + " " + word + (n == 1 ? "" : "s");
}
} // namespace

// =====================================================================================
// Setup
// =====================================================================================
Game::Game() : rng(std::random_device{}())
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    window.create(sf::VideoMode({layout::kWidth, layout::kHeight}),
                  "Little Harvest",
                  sf::Style::Titlebar | sf::Style::Close,
                  sf::State::Windowed,
                  settings);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    loadFont();
    setupIcon();
    scenery.build();

    handCursor  = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand);
    arrowCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);

    resetFarm();
    hasSave = std::filesystem::exists(kSaveFile);
    if (hasSave && !loadGame())
    {
        hasSave = false;
        resetFarm();
    }
}

void Game::loadFont()
{
    // Works whether the game is started from the project folder or the build folder.
    for (const char* path : {"assets/fonts/Nunito-Regular.ttf", "../assets/fonts/Nunito-Regular.ttf", "../../assets/fonts/Nunito-Regular.ttf"})
    {
        if (std::filesystem::exists(path) && font.openFromFile(path))
        {
            font.setSmooth(true);
            return;
        }
    }
    throw std::runtime_error("Could not find assets/fonts/Nunito-Regular.ttf - run the game from the project or build folder.");
}

void Game::setupIcon()
{
    // Render a smiling carrot into a texture and use it as the window icon.
    sf::RenderTexture canvas;
    if (!canvas.resize({64, 64}))
        return;
    canvas.clear(sf::Color::Transparent);
    art::drawProduce(canvas, {}, CropType::Carrot, {32.f, 36.f}, 1.45f, false, true);
    canvas.display();
    const sf::Image icon = canvas.getTexture().copyToImage();
    window.setIcon(icon);
}

void Game::resetFarm()
{
    plots.clear();
    for (int row = 0; row < layout::kRows; ++row)
    {
        for (int col = 0; col < layout::kCols; ++col)
        {
            const sf::Vector2f pos{layout::kGridX + static_cast<float>(col) * (layout::kTile + layout::kGap),
                                   layout::kGridY + static_cast<float>(row) * (layout::kTile + layout::kGap)};
            const bool         starter = (row == 1 || row == 2) && col >= 1 && col <= 4; // 8 plots in the middle
            plots.emplace_back(pos, layout::kTile, starter);
        }
    }

    day          = 1;
    coins        = 100;
    seeds        = {5, 0, 0, 0};
    weather      = Weather::Sunny;
    tomorrow     = Weather::Sunny;
    plotsBought  = 0;
    harvested    = 0;
    earned       = 0;
    ribbonEarned = false;
    selectedSlot = 0;
    shownCoins   = static_cast<float>(coins);
    scenery.setWeather(weather);
    effects.clear();
}

void Game::startNewGame()
{
    resetFarm();
    tomorrow = rollWeather();
    screen   = Screen::Playing;
    panel    = Panel::Help; // show the controls on a fresh farm
    firstDay = true;
    saveGame();
    hasSave = true;
    sounds.play(Sfx::Morning);
}

// ---- save file ----------------------------------------------------------------------
void Game::saveGame() const
{
    std::ofstream out(kSaveFile);
    if (!out)
        return;
    out << "LittleHarvest 1\n";
    out << "day " << day << "\n";
    out << "coins " << coins << "\n";
    out << "seeds";
    for (int s : seeds)
        out << ' ' << s;
    out << "\n";
    out << "weather " << static_cast<int>(weather) << ' ' << static_cast<int>(tomorrow) << "\n";
    out << "stats " << plotsBought << ' ' << harvested << ' ' << earned << ' ' << ribbonEarned << "\n";
    out << "plots " << plots.size() << "\n";
    for (const auto& plot : plots)
    {
        plot.save(out);
        out << "\n";
    }
}

bool Game::loadGame()
{
    std::ifstream in(kSaveFile);
    std::string   magic, key;
    int           version = 0;
    if (!(in >> magic >> version) || magic != "LittleHarvest" || version != 1)
        return false;

    int w = 0, t = 0, count = 0;
    if (!(in >> key >> day) || key != "day")
        return false;
    if (!(in >> key >> coins) || key != "coins")
        return false;
    if (!(in >> key) || key != "seeds")
        return false;
    for (int& s : seeds)
        if (!(in >> s))
            return false;
    if (!(in >> key >> w >> t) || key != "weather")
        return false;
    if (!(in >> key >> plotsBought >> harvested >> earned >> ribbonEarned) || key != "stats")
        return false;
    if (!(in >> key >> count) || key != "plots" || count != static_cast<int>(plots.size()))
        return false;
    for (auto& plot : plots)
        if (!plot.load(in))
            return false;

    weather    = static_cast<Weather>(std::clamp(w, 0, 2));
    tomorrow   = static_cast<Weather>(std::clamp(t, 0, 2));
    shownCoins = static_cast<float>(coins);
    scenery.setWeather(weather);
    return true;
}

// =====================================================================================
// Main loop
// =====================================================================================
void Game::run()
{
    sf::Clock clock;
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
            handleEvent(*event);

        const float dt = std::min(clock.restart().asSeconds(), 0.05f);
        update(dt);
        render();
    }
}

void Game::handleEvent(const sf::Event& event)
{
    if (event.is<sf::Event::Closed>())
    {
        if (screen == Screen::Playing && !sleeping)
            saveGame();
        window.close();
        return;
    }

    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::F12)
        {
            screenshotRequested = true;
            return;
        }
        if (key->code == sf::Keyboard::Key::M)
        {
            sounds.setMuted(!sounds.isMuted());
            sounds.play(Sfx::Click);
            return;
        }
        if (!sleeping)
            handleKey(key->code, key->shift);
        return;
    }

    if (sleeping)
        return;

    if (const auto* press = event.getIf<sf::Event::MouseButtonPressed>())
    {
        const sf::Vector2f pos = window.mapPixelToCoords(press->position);
        const bool         shift = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
        handleClick(pos, press->button, shift);
    }
    else if (event.is<sf::Event::MouseButtonReleased>())
    {
        dragging = false;
    }
    else if (const auto* move = event.getIf<sf::Event::MouseMoved>())
    {
        if (dragging && panel == Panel::None && screen == Screen::Playing)
        {
            const int index = plotAt(window.mapPixelToCoords(move->position));
            if (index >= 0 && index != lastPlot)
            {
                lastPlot = index;
                if (actionFor(plots[static_cast<std::size_t>(index)], dragRight) == dragAction)
                {
                    if (!applyAction(dragAction, index, true))
                        dragging = false;
                }
            }
        }
    }
    else if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>())
    {
        if (screen == Screen::Playing && panel == Panel::None)
            selectSlot((selectedSlot + (wheel->delta < 0.f ? 1 : kSlotCount - 1)) % kSlotCount);
    }
    else if (event.is<sf::Event::FocusLost>())
    {
        dragging = false;
    }
}

void Game::handleKey(sf::Keyboard::Key key, bool shift)
{
    using K = sf::Keyboard::Key;

    if (key == K::Escape)
    {
        if (panel != Panel::None)
        {
            panel = Panel::None;
            sounds.play(Sfx::Click);
        }
        else if (screen == Screen::Playing)
        {
            saveGame();
            hasSave        = true;
            screen         = Screen::Title;
            confirmNewFarm = false;
            sounds.play(Sfx::Click);
        }
        return;
    }

    if (screen == Screen::Title)
    {
        if (panel == Panel::Help && (key == K::Enter || key == K::Space || key == K::H))
        {
            panel = Panel::None;
            return;
        }
        if (key == K::Enter || key == K::Space)
        {
            if (hasSave)
            {
                screen = Screen::Playing;
                sounds.play(Sfx::Morning);
            }
            else
            {
                startNewGame();
            }
        }
        else if (key == K::H)
        {
            panel = Panel::Help;
        }
        return;
    }

    // Playing
    if (panel == Panel::Ribbon)
    {
        if (key == K::Enter || key == K::Space)
            panel = Panel::None;
        return;
    }
    if (key == K::B)
    {
        panel = panel == Panel::Shop ? Panel::None : Panel::Shop;
        sounds.play(Sfx::Click);
        return;
    }
    if (key == K::H)
    {
        panel = panel == Panel::Help ? Panel::None : Panel::Help;
        sounds.play(Sfx::Click);
        return;
    }
    if (panel == Panel::Help && (key == K::Enter || key == K::Space))
    {
        panel = Panel::None;
        return;
    }
    if (panel == Panel::Shop)
    {
        if (key >= K::Num1 && key <= K::Num4)
            buySeeds(static_cast<int>(key) - static_cast<int>(K::Num1), shift ? 5 : 1);
        return;
    }
    if (panel != Panel::None)
        return;

    if (key >= K::Num1 && key <= K::Num5)
        selectSlot(static_cast<int>(key) - static_cast<int>(K::Num1));
    else if (key >= K::Numpad1 && key <= K::Numpad5)
        selectSlot(static_cast<int>(key) - static_cast<int>(K::Numpad1));
    else if (key == K::Tab)
        selectSlot((selectedSlot + 1) % kSlotCount);
    else if (key == K::Space || key == K::Enter)
        startSleep();
}

void Game::handleClick(sf::Vector2f pos, sf::Mouse::Button button, bool shift)
{
    if (panel != Panel::None)
    {
        handlePanelClick(pos, shift);
        return;
    }
    if (screen == Screen::Title)
    {
        handleTitleClick(pos);
        return;
    }

    const bool left  = button == sf::Mouse::Button::Left;
    const bool right = button == sf::Mouse::Button::Right;
    if (!left && !right)
        return;

    if (left)
    {
        // HUD buttons
        if ((pos - sf::Vector2f(902.f, 42.f)).lengthSquared() < 20.f * 20.f)
        {
            panel = Panel::Help;
            sounds.play(Sfx::Click);
            return;
        }
        if ((pos - sf::Vector2f(948.f, 42.f)).lengthSquared() < 20.f * 20.f)
        {
            sounds.setMuted(!sounds.isMuted());
            sounds.play(Sfx::Click);
            return;
        }
        if (shopButtonRect().contains(pos))
        {
            panel = Panel::Shop;
            sounds.play(Sfx::Click);
            return;
        }
        if (sleepButtonRect().contains(pos))
        {
            startSleep();
            return;
        }
        for (int i = 0; i < kSlotCount; ++i)
        {
            if (slotRect(i).contains(pos))
            {
                selectSlot(i);
                return;
            }
        }
    }

    const int index = plotAt(pos);
    if (index < 0)
        return;

    const Action action = actionFor(plots[static_cast<std::size_t>(index)], right);
    if (action == Action::None)
    {
        auto& plot = plots[static_cast<std::size_t>(index)];
        if (plot.isWatered() && !plot.isReady())
            showToast("Already watered today - sleep to let it grow!", kInfo);
        return;
    }
    if (applyAction(action, index, false) && action != Action::Unlock)
    {
        dragging   = true;
        dragRight  = right;
        dragAction = action;
        lastPlot   = index;
    }
}

bool Game::handlePanelClick(sf::Vector2f pos, bool shift)
{
    switch (panel)
    {
        case Panel::Shop:
            for (int i = 0; i < kCropCount; ++i)
            {
                if (shopBuyRect(i).contains(pos))
                {
                    buySeeds(i, shift ? 5 : 1);
                    return true;
                }
            }
            if (shopCloseRect().contains(pos) || !shopPanelRect().contains(pos))
            {
                panel = Panel::None;
                sounds.play(Sfx::Click);
            }
            return true;
        case Panel::Help:
            if (helpCloseRect().contains(pos) || !helpPanelRect().contains(pos))
            {
                panel = Panel::None;
                sounds.play(Sfx::Click);
            }
            return true;
        case Panel::Ribbon:
            if (ribbonCloseRect().contains(pos))
            {
                panel = Panel::None;
                sounds.play(Sfx::Click);
            }
            return true;
        case Panel::None:
            break;
    }
    return false;
}

void Game::handleTitleClick(sf::Vector2f pos)
{
    for (const auto& [id, rect] : titleButtons())
    {
        if (!rect.contains(pos))
            continue;
        sounds.play(Sfx::Click);
        switch (id)
        {
            case TitleButton::Continue:
                screen = Screen::Playing;
                sounds.play(Sfx::Morning);
                break;
            case TitleButton::NewFarm:
                if (hasSave && !confirmNewFarm)
                {
                    confirmNewFarm = true; // ask once before wiping the old farm
                }
                else
                {
                    confirmNewFarm = false;
                    startNewGame();
                }
                break;
            case TitleButton::HowToPlay:
                panel = Panel::Help;
                break;
        }
        return;
    }
    confirmNewFarm = false;
}

// =====================================================================================
// Gameplay
// =====================================================================================
Game::Action Game::actionFor(const FarmPlot& plot, bool rightClick) const
{
    if (!plot.isUnlocked())
        return rightClick ? Action::None : Action::Unlock;
    if (plot.isReady())
        return rightClick ? Action::None : Action::Harvest;
    if (rightClick || selectedSlot == kCanSlot)
        return plot.isWatered() ? Action::None : Action::Water;
    if (plot.isEmpty())
        return Action::Plant;
    return plot.isWatered() ? Action::None : Action::Water;
}

bool Game::applyAction(Action action, int index, bool quiet)
{
    FarmPlot&          plot   = plots[static_cast<std::size_t>(index)];
    const sf::Vector2f center = plot.getCenter();

    switch (action)
    {
        case Action::Harvest:
        {
            const bool      golden = plot.isGolden();
            const CropInfo& info   = plot.getCrop().getInfo();
            const int       value  = plot.harvest();
            if (value <= 0)
                return false;
            ++harvested;
            effects.leafBurst(center, golden ? sf::Color(252, 212, 78) : info.color);
            effects.floatingText((golden ? "Golden! +" : "+") + std::to_string(value), center + sf::Vector2f(0.f, -34.f),
                                 golden ? sf::Color(255, 222, 90) : sf::Color(255, 246, 200), golden ? 28u : 24u);
            sounds.play(Sfx::Harvest, 0.95f + static_cast<float>(index % 5) * 0.03f);
            if (golden)
            {
                effects.sparkles(center, 18, sf::Color(255, 226, 110), 60.f);
                sounds.play(Sfx::Golden);
            }
            addCoins(value, center);
            return true;
        }
        case Action::Plant:
        {
            const int crop = selectedSlot;
            if (crop < 0 || crop >= kCropCount)
                return false;
            if (seeds[static_cast<std::size_t>(crop)] <= 0)
            {
                showToast("Out of " + Crop::info(Crop::fromIndex(crop)).name + " seeds! Press B to visit the shop.", kWarn);
                sounds.play(Sfx::Error);
                return false;
            }
            if (!plot.plant(Crop::fromIndex(crop)))
                return false;
            --seeds[static_cast<std::size_t>(crop)];
            effects.dirtPuff(center + sf::Vector2f(0.f, 14.f));
            sounds.play(Sfx::Plant, 0.9f + static_cast<float>(index % 4) * 0.05f);
            return true;
        }
        case Action::Water:
        {
            if (!plot.water())
                return false;
            effects.splash(center + sf::Vector2f(0.f, 10.f));
            sounds.play(Sfx::Water, 0.92f + static_cast<float>(index % 4) * 0.05f);
            return true;
        }
        case Action::Unlock:
        {
            const int price = plotPrice();
            if (coins < price)
            {
                if (!quiet)
                {
                    showToast("This plot costs " + std::to_string(price) + " coins - keep harvesting!", kWarn);
                    sounds.play(Sfx::Error);
                }
                return false;
            }
            coins -= price;
            ++plotsBought;
            plot.unlock();
            effects.sparkles(center, 22, sf::Color(255, 246, 200), 55.f);
            effects.dirtPuff(center);
            sounds.play(Sfx::Unlock);
            showToast("New plot unlocked! Your farm is growing.", kGood);
            return true;
        }
        case Action::None:
            break;
    }
    return false;
}

void Game::addCoins(int amount, sf::Vector2f from)
{
    coins += amount;
    earned += amount;
    effects.flyingCoins(from, kCoinIcon, std::clamp(amount / 15, 2, 10));

    if (!ribbonEarned && coins >= kGoalCoins)
    {
        ribbonEarned = true;
        panel        = Panel::Ribbon;
        dragging     = false;
        effects.confetti(static_cast<float>(layout::kWidth));
        sounds.play(Sfx::Ribbon);
        saveGame();
    }
}

void Game::selectSlot(int slot)
{
    if (slot < 0 || slot >= kSlotCount || slot == selectedSlot)
        return;
    selectedSlot = slot;
    sounds.play(Sfx::Click, 1.f + static_cast<float>(slot) * 0.06f);
}

bool Game::buySeeds(int cropIndex, int amount)
{
    const CropInfo& info  = Crop::info(Crop::fromIndex(cropIndex));
    const int       afford = coins / info.seedCost;
    amount                 = std::min(amount, afford);
    if (amount <= 0)
    {
        showToast("Not enough coins for " + info.name + " seeds.", kWarn);
        sounds.play(Sfx::Error);
        return false;
    }
    coins -= info.seedCost * amount;
    seeds[static_cast<std::size_t>(cropIndex)] += amount;
    sounds.play(Sfx::Buy);
    const sf::FloatRect button = shopBuyRect(cropIndex);
    effects.sparkles(button.position + button.size / 2.f, 8, sf::Color(255, 246, 200), 30.f);
    effects.floatingText("+" + std::to_string(amount) + " " + info.name, button.position + sf::Vector2f(button.size.x / 2.f, -6.f),
                         sf::Color(255, 246, 200), 18);
    if (selectedSlot == kCanSlot || seeds[static_cast<std::size_t>(selectedSlot)] == 0)
        selectedSlot = cropIndex; // handy: switch to what you just bought
    return true;
}

int Game::plotPrice() const
{
    return 40 + plotsBought * 20;
}

Weather Game::rollWeather()
{
    const int roll = std::uniform_int_distribution<int>(1, 100)(rng);
    if (roll <= 22)
        return Weather::Rainy;
    if (roll <= 45)
        return Weather::Cloudy;
    return Weather::Sunny;
}

std::string Game::weatherName(Weather w) const
{
    switch (w)
    {
        case Weather::Sunny:
            return "Sunny";
        case Weather::Cloudy:
            return "Cloudy";
        case Weather::Rainy:
            return "Rainy";
    }
    return "";
}

void Game::startSleep()
{
    if (sleeping)
        return;
    sleeping     = true;
    sleepTimer   = 0.f;
    dayAdvanced  = false;
    morningSound = false;
    dragging     = false;
    firstDay     = false;
    sounds.play(Sfx::Sleep);
}

void Game::advanceDay()
{
    ++day;
    weather  = tomorrow;
    tomorrow = rollWeather();
    scenery.setWeather(weather);

    for (auto& plot : plots)
        plot.grow(rng);

    int rained = 0;
    if (weather == Weather::Rainy)
    {
        for (auto& plot : plots)
            if (plot.isUnlocked() && !plot.isEmpty() && plot.water())
                ++rained;
    }

    int ready = 0, golden = 0, thirsty = 0;
    for (const auto& plot : plots)
    {
        ready += plot.isReady() ? 1 : 0;
        golden += plot.isGolden() ? 1 : 0;
        thirsty += (!plot.isEmpty() && !plot.isReady() && !plot.isWatered()) ? 1 : 0;
    }

    if (golden > 0)
        morningMessage = "Wow, a golden crop is ready! It sells for 3x.";
    else if (weather == Weather::Rainy && rained > 0)
        morningMessage = "It's raining! Your crops are watered for today.";
    else if (ready > 0)
        morningMessage = plural(ready, "crop") + (ready == 1 ? " is" : " are") + " ready to harvest!";
    else if (thirsty > 0)
        morningMessage = "Good morning! " + plural(thirsty, "plant") + " would love some water.";
    else
        morningMessage = "Good morning! A lovely day for planting.";

    effects.clear();
    saveGame();
}

void Game::showToast(const std::string& text, sf::Color accent)
{
    toastText   = text;
    toastAccent = accent;
    toastTimer  = 3.2f;
}

// =====================================================================================
// Update
// =====================================================================================
void Game::update(float dt)
{
    time += dt;
    scenery.update(dt);
    for (auto& plot : plots)
        plot.update(dt);

    const int arrived = effects.update(dt);
    if (arrived > 0)
    {
        coinBounce = 1.f;
        sounds.play(Sfx::Coin, 1.f + static_cast<float>(std::uniform_int_distribution<int>(0, 4)(rng)) * 0.04f);
    }
    coinBounce = std::max(0.f, coinBounce - dt * 4.f);
    shownCoins += (static_cast<float>(coins) - shownCoins) * std::min(1.f, dt * 7.f);
    if (std::abs(shownCoins - static_cast<float>(coins)) < 0.5f)
        shownCoins = static_cast<float>(coins);

    if (toastTimer > 0.f)
        toastTimer -= dt;

    if (sleeping)
    {
        sleepTimer += dt;
        if (!dayAdvanced && sleepTimer >= kSleepFadeOut)
        {
            advanceDay();
            dayAdvanced = true;
        }
        if (!morningSound && sleepTimer >= kSleepHold)
        {
            morningSound = true;
            sounds.play(Sfx::Morning);
        }
        if (sleepTimer >= kSleepEnd)
        {
            sleeping = false;
            showToast(morningMessage, weather == Weather::Rainy ? sf::Color(120, 182, 230) : kGood);
        }
    }

    // Pointer cursor over anything clickable.
    const bool wantHand = !sleeping && isOverClickable(mousePosition());
    if (wantHand != usingHand && handCursor && arrowCursor)
    {
        window.setMouseCursor(wantHand ? *handCursor : *arrowCursor);
        usingHand = wantHand;
    }
}

// =====================================================================================
// Layout
// =====================================================================================
sf::FloatRect Game::slotRect(int slot) const
{
    return {{318.f + static_cast<float>(slot) * 74.f, 619.f}, {64.f, 62.f}};
}

sf::FloatRect Game::shopButtonRect() const
{
    return {{64.f, 620.f}, {196.f, 62.f}};
}

sf::FloatRect Game::sleepButtonRect() const
{
    return {{740.f, 620.f}, {196.f, 62.f}};
}

sf::FloatRect Game::shopPanelRect() const
{
    return {{200.f, 112.f}, {600.f, 478.f}};
}

sf::FloatRect Game::shopBuyRect(int row) const
{
    return {{636.f, 236.f + static_cast<float>(row) * 74.f}, {136.f, 48.f}};
}

sf::FloatRect Game::shopCloseRect() const
{
    return {{420.f, 530.f}, {160.f, 46.f}};
}

sf::FloatRect Game::helpPanelRect() const
{
    return {{150.f, 96.f}, {700.f, 512.f}};
}

sf::FloatRect Game::helpCloseRect() const
{
    return {{420.f, 548.f}, {160.f, 48.f}};
}

sf::FloatRect Game::ribbonCloseRect() const
{
    return {{430.f, 470.f}, {140.f, 48.f}};
}

std::vector<std::pair<Game::TitleButton, sf::FloatRect>> Game::titleButtons() const
{
    std::vector<std::pair<TitleButton, sf::FloatRect>> buttons;
    float                                              y = hasSave ? 360.f : 392.f;
    if (hasSave)
    {
        buttons.emplace_back(TitleButton::Continue, sf::FloatRect{{370.f, y}, {260.f, 58.f}});
        y += 68.f;
    }
    buttons.emplace_back(TitleButton::NewFarm, sf::FloatRect{{370.f, y}, {260.f, 58.f}});
    y += 68.f;
    buttons.emplace_back(TitleButton::HowToPlay, sf::FloatRect{{370.f, y}, {260.f, 58.f}});
    return buttons;
}

int Game::plotAt(sf::Vector2f pos) const
{
    for (std::size_t i = 0; i < plots.size(); ++i)
        if (plots[i].contains(pos))
            return static_cast<int>(i);
    return -1;
}

sf::Vector2f Game::mousePosition() const
{
    return window.mapPixelToCoords(sf::Mouse::getPosition(window));
}

bool Game::isOverClickable(sf::Vector2f pos) const
{
    if (screen == Screen::Title && panel == Panel::None)
    {
        for (const auto& [id, rect] : titleButtons())
            if (rect.contains(pos))
                return true;
        return false;
    }
    switch (panel)
    {
        case Panel::Shop:
            for (int i = 0; i < kCropCount; ++i)
                if (shopBuyRect(i).contains(pos))
                    return true;
            return shopCloseRect().contains(pos);
        case Panel::Help:
            return helpCloseRect().contains(pos);
        case Panel::Ribbon:
            return ribbonCloseRect().contains(pos);
        case Panel::None:
            break;
    }
    if (shopButtonRect().contains(pos) || sleepButtonRect().contains(pos))
        return true;
    for (int i = 0; i < kSlotCount; ++i)
        if (slotRect(i).contains(pos))
            return true;
    if ((pos - sf::Vector2f(902.f, 42.f)).lengthSquared() < 400.f || (pos - sf::Vector2f(948.f, 42.f)).lengthSquared() < 400.f)
        return true;
    const int index = plotAt(pos);
    return index >= 0 && actionFor(plots[static_cast<std::size_t>(index)], false) != Action::None;
}

// =====================================================================================
// Rendering
// =====================================================================================
void Game::render()
{
    window.clear(sf::Color(150, 198, 118));

    if (screen == Screen::Title)
    {
        drawTitle();
    }
    else
    {
        drawWorld();
        drawHint();
        drawHud();
        drawHotbar();
        drawToast();
        if (panel == Panel::None && !sleeping)
        {
            drawTooltip();
            drawCursorIcon();
        }
    }

    if (panel != Panel::None)
    {
        dimBackground();
        if (panel == Panel::Shop)
            drawShop();
        else if (panel == Panel::Help)
            drawHelp();
        else if (panel == Panel::Ribbon)
            drawRibbonPanel();
    }

    effects.draw(window, font);

    if (sleeping)
        drawSleepOverlay();

    if (screenshotRequested)
    {
        screenshotRequested = false;
        sf::Texture shot(window.getSize());
        shot.update(window);
        std::string name;
        do
        {
            name = "screenshot_" + std::to_string(++screenshotCount) + ".png";
        } while (std::filesystem::exists(name));
        if (shot.copyToImage().saveToFile(name))
            showToast("Saved " + name, kInfo);
    }

    window.display();
}

void Game::drawWorld()
{
    scenery.drawGround(window);

    const int hovered = (panel == Panel::None && !sleeping) ? plotAt(mousePosition()) : -1;
    for (std::size_t i = 0; i < plots.size(); ++i)
        plots[i].draw(window, time, static_cast<int>(i) == hovered);

    scenery.drawAmbient(window);
    scenery.drawWeather(window);
}

void Game::dimBackground()
{
    art::fillRect(window, {{0.f, 0.f}, {static_cast<float>(layout::kWidth), static_cast<float>(layout::kHeight)}}, sf::Color(50, 36, 24, 120));
}

void Game::drawWeatherIcon(Weather w, sf::Vector2f center, float scale)
{
    switch (w)
    {
        case Weather::Sunny:
            art::drawSun(window, center, scale, time * 20.f);
            break;
        case Weather::Cloudy:
            art::drawSun(window, center + sf::Vector2f(5.f, -4.f) * scale, scale * 0.75f, time * 20.f);
            art::drawCloud(window, center + sf::Vector2f(-2.f, 3.f) * scale, scale * 0.85f);
            break;
        case Weather::Rainy:
            art::drawRainCloud(window, center + sf::Vector2f(0.f, -3.f) * scale, scale * 0.9f);
            break;
    }
}

void Game::drawHud()
{
    using namespace ui::palette;
    ui::drawPanel(window, {{14.f, 10.f}, {972.f, 64.f}}, cream, sf::Color(226, 206, 170), 20.f, 3.f);

    // Title
    art::drawSprout(window, {44.f, 44.f}, 1.3f);
    ui::drawText(window, font, "Little Harvest", {66.f, 42.f}, 28, sf::Color(96, 140, 76), ui::Align::Left, true);

    // Day & weather
    art::fillRoundedRect(window, {{350.f, 19.f}, {300.f, 46.f}}, 23.f, creamDark);
    ui::drawText(window, font, "Day " + std::to_string(day), {372.f, 42.f}, 24, ink, ui::Align::Left, true);
    drawWeatherIcon(weather, {486.f, 42.f}, 1.05f);
    ui::drawText(window, font, weatherName(weather), {510.f, 34.f}, 17, ink, ui::Align::Left, true);
    ui::drawText(window, font, "Tomorrow: " + weatherName(tomorrow), {510.f, 53.f}, 13, inkSoft);

    // Coins
    const float bounce = std::sin(coinBounce * kPi) * 4.f;
    art::fillRoundedRect(window, {{698.f, 19.f}, {170.f, 46.f}}, 23.f, creamDark);
    art::drawCoin(window, kCoinIcon - sf::Vector2f(0.f, bounce), 13.f + bounce * 0.4f);
    ui::drawText(window, font, withCommas(static_cast<int>(std::round(shownCoins))), {748.f, 42.f}, 24, ink, ui::Align::Left, true);
    if (ribbonEarned)
        art::drawRibbon(window, {850.f, 40.f}, 0.62f);

    // Help & sound buttons
    const sf::Vector2f mouse = mousePosition();
    for (int i = 0; i < 2; ++i)
    {
        const sf::Vector2f c{902.f + static_cast<float>(i) * 46.f, 42.f};
        const bool         hover = panel == Panel::None && (mouse - c).lengthSquared() < 400.f;
        art::fillCircle(window, c + sf::Vector2f(0.f, 3.f), 18.f, art::darker(leaf, 0.75f));
        art::fillCircle(window, c, 18.f, hover ? art::lighter(leaf, 0.15f) : leaf);
        if (i == 0)
            ui::drawText(window, font, "?", c, 22, sf::Color::White, ui::Align::Center, true);
        else
            art::drawSpeaker(window, c + sf::Vector2f(-1.f, 0.f), 1.f, sounds.isMuted(), sf::Color::White);
    }
}

void Game::drawHotbar()
{
    using namespace ui::palette;
    const sf::Vector2f mouse = mousePosition();

    ui::drawPanel(window, {{306.f, 610.f}, {388.f, 80.f}}, cream, sf::Color(226, 206, 170), 20.f, 3.f);

    for (int i = 0; i < kSlotCount; ++i)
    {
        const bool    selected = i == selectedSlot;
        const bool    hover    = panel == Panel::None && slotRect(i).contains(mouse);
        sf::FloatRect r        = slotRect(i);
        if (selected)
            r.position.y -= 4.f;

        art::fillRoundedRect(window, r, 14.f, selected ? sf::Color(255, 240, 200) : (hover ? sf::Color(250, 236, 206) : creamDark), selected ? 3.f : 0.f,
                             selected ? sf::Color(240, 180, 70) : sf::Color::Transparent);

        const sf::Vector2f c = r.position + sf::Vector2f(32.f, 30.f);
        const float        wobble = selected ? std::sin(time * 4.f) * 3.f : 0.f;
        if (i == kCanSlot)
        {
            art::drawWateringCan(window, c + sf::Vector2f(-3.f, 2.f), 1.15f, wobble);
        }
        else
        {
            art::drawSeedPacket(window, c, 1.15f, Crop::fromIndex(i));
            const int count = seeds[static_cast<std::size_t>(i)];
            const sf::Vector2f badge = r.position + sf::Vector2f(54.f, 52.f);
            art::fillCircle(window, badge, 11.f, count > 0 ? sf::Color(232, 124, 112) : sf::Color(190, 180, 168), 2.f, sf::Color(255, 248, 231));
            ui::drawText(window, font, std::to_string(count), badge, count > 99 ? 10 : 13, sf::Color::White, ui::Align::Center, true);
        }
        ui::drawText(window, font, std::to_string(i + 1), r.position + sf::Vector2f(9.f, 11.f), 13, inkSoft, ui::Align::Left, true);

        if (selected)
        {
            const std::string label = i == kCanSlot ? "Watering can" : Crop::info(Crop::fromIndex(i)).name + " seeds";
            const float       w     = ui::textWidth(font, label, 15, true) + 24.f;
            const sf::FloatRect tag{{c.x - w / 2.f, 584.f}, {w, 24.f}};
            art::fillRoundedRect(window, tag, 12.f, sf::Color(94, 72, 54, 220));
            ui::drawText(window, font, label, tag.position + tag.size / 2.f, 15, cream, ui::Align::Center, true);
        }
    }

    // Shop button
    ui::drawButton(window, font, shopButtonRect(), "", sf::Color(232, 120, 116), mouse);
    {
        const sf::FloatRect r    = shopButtonRect();
        const float         lift = (panel == Panel::None && r.contains(mouse)) ? 2.f : 0.f;
        art::drawSeedPacket(window, r.position + sf::Vector2f(36.f, 28.f - lift), 0.95f, CropType::Strawberry);
        ui::drawText(window, font, "Seed Shop", r.position + sf::Vector2f(62.f, 21.f - lift), 21, sf::Color::White, ui::Align::Left, true, 2.f,
                     sf::Color(160, 74, 72));
        ui::drawText(window, font, "press B", r.position + sf::Vector2f(63.f, 41.f - lift), 13, sf::Color(255, 226, 222));
    }

    // Sleep button
    ui::drawButton(window, font, sleepButtonRect(), "", sf::Color(118, 132, 204), mouse);
    {
        const sf::FloatRect r    = sleepButtonRect();
        const float         lift = (panel == Panel::None && r.contains(mouse)) ? 2.f : 0.f;
        art::drawMoon(window, r.position + sf::Vector2f(36.f, 28.f - lift), 13.f, art::lighter(sf::Color(118, 132, 204), lift > 0.f ? 0.14f : 0.f));
        ui::drawText(window, font, "Sleep", r.position + sf::Vector2f(62.f, 21.f - lift), 21, sf::Color::White, ui::Align::Left, true, 2.f,
                     sf::Color(70, 80, 150));
        ui::drawText(window, font, "end the day - Space", r.position + sf::Vector2f(63.f, 41.f - lift), 13, sf::Color(226, 230, 255));
    }
}

void Game::drawHint()
{
    std::string hint;
    if (firstDay)
        hint = "Click the soil to plant, then click your seedlings to water them. Sleep to grow!";
    else
        hint = "Left-click: plant, water or harvest   -   Right-click: water   -   Drag across plots to go faster";
    ui::drawText(window, font, hint, {500.f, 517.f}, 15, sf::Color(255, 252, 238), ui::Align::Center, true, 2.5f, sf::Color(86, 128, 64, 200));
}

void Game::drawToast()
{
    if (toastTimer <= 0.f || toastText.empty())
        return;
    const float appear = std::min(1.f, (3.2f - toastTimer) / 0.18f);
    const float fade   = std::min(1.f, toastTimer / 0.4f);
    const float k      = std::min(appear, fade);
    const auto  alpha  = static_cast<std::uint8_t>(255.f * k);

    const float         w = ui::textWidth(font, toastText, 18, true) + 56.f;
    const sf::FloatRect r{{500.f - w / 2.f, 538.f + (1.f - appear) * 12.f}, {w, 38.f}};
    art::fillRoundedRect(window, {r.position + sf::Vector2f(0.f, 4.f), r.size}, 19.f, sf::Color(60, 40, 20, static_cast<std::uint8_t>(50.f * k)));
    art::fillRoundedRect(window, r, 19.f, sf::Color(255, 248, 231, alpha), 3.f, art::withAlpha(toastAccent, alpha));
    art::fillCircle(window, r.position + sf::Vector2f(22.f, 19.f), 6.f, art::withAlpha(toastAccent, alpha));
    ui::drawText(window, font, toastText, r.position + sf::Vector2f(36.f, 19.f), 18, sf::Color(94, 72, 54, alpha), ui::Align::Left, true);
}

void Game::drawTooltip()
{
    const sf::Vector2f mouse = mousePosition();
    const int          index = plotAt(mouse);
    if (index < 0)
        return;
    const FarmPlot& plot = plots[static_cast<std::size_t>(index)];

    std::string title, line;
    sf::Color   lineColor = ui::palette::inkSoft;
    if (!plot.isUnlocked())
    {
        title     = "Wild grass";
        line      = "Click to buy this plot for " + std::to_string(plotPrice()) + " coins";
        lineColor = coins >= plotPrice() ? sf::Color(96, 150, 76) : sf::Color(200, 90, 96);
    }
    else if (plot.isEmpty())
    {
        title = plot.isWatered() ? "Damp soil" : "Empty soil";
        if (selectedSlot == kCanSlot)
            line = plot.isWatered() ? "Already watered" : "Click to water";
        else
        {
            const CropInfo& info = Crop::info(Crop::fromIndex(selectedSlot));
            const int       have = seeds[static_cast<std::size_t>(selectedSlot)];
            line                 = have > 0 ? "Click to plant " + info.name + " (" + std::to_string(have) + " left)" : "No " + info.name + " seeds - visit the shop";
            if (have == 0)
                lineColor = sf::Color(200, 90, 96);
        }
    }
    else
    {
        const CropInfo& info = plot.getCrop().getInfo();
        title                = plot.isGolden() ? "Golden " + info.name + "!" : info.name;
        if (plot.isReady())
        {
            line      = "Ready! Click to harvest for " + std::to_string(info.sellPrice * (plot.isGolden() ? 3 : 1)) + " coins";
            lineColor = sf::Color(96, 150, 76);
        }
        else
        {
            const int left = plot.getDaysLeft();
            line           = plural(left, "more day") + " to grow - " + (plot.isWatered() ? "watered" : "needs water!");
            if (!plot.isWatered())
                lineColor = sf::Color(70, 140, 200);
        }
    }

    const float   w = std::max(ui::textWidth(font, title, 17, true), ui::textWidth(font, line, 14)) + 26.f;
    sf::FloatRect r{mouse + sf::Vector2f(18.f, 22.f), {w, 50.f}};
    if (r.position.x + r.size.x > 990.f)
        r.position.x = mouse.x - r.size.x - 14.f;
    if (r.position.y + r.size.y > 600.f)
        r.position.y = mouse.y - r.size.y - 14.f;
    ui::drawPanel(window, r, sf::Color(255, 250, 238, 245), sf::Color(226, 206, 170), 12.f, 2.f);
    ui::drawText(window, font, title, r.position + sf::Vector2f(13.f, 16.f), 17, ui::palette::ink, ui::Align::Left, true);
    ui::drawText(window, font, line, r.position + sf::Vector2f(13.f, 35.f), 14, lineColor);
}

void Game::drawCursorIcon()
{
    const sf::Vector2f mouse = mousePosition();
    const int          index = plotAt(mouse);
    if (index < 0)
        return;
    const FarmPlot& plot = plots[static_cast<std::size_t>(index)];
    const Action    act  = actionFor(plot, false);
    const sf::Vector2f p = mouse + sf::Vector2f(-14.f, -16.f);
    if (act == Action::Water)
        art::drawWateringCan(window, p, 0.9f, -25.f + std::sin(time * 8.f) * 6.f);
    else if (act == Action::Plant)
        art::drawSeedPacket(window, p, 0.7f, Crop::fromIndex(selectedSlot));
}

void Game::drawShop()
{
    using namespace ui::palette;
    const sf::FloatRect panelRect = shopPanelRect();
    const sf::Vector2f  mouse     = mousePosition();
    ui::drawPanel(window, panelRect, cream, sf::Color(214, 180, 132), 22.f, 4.f);

    // Striped awning
    const float stripeW = panelRect.size.x / 10.f;
    for (int i = 0; i < 10; ++i)
    {
        const sf::Color c = i % 2 == 0 ? sf::Color(236, 120, 118) : sf::Color(255, 246, 226);
        const float     x = panelRect.position.x + static_cast<float>(i) * stripeW;
        art::fillRoundedRect(window, {{x, panelRect.position.y - 18.f}, {stripeW, 52.f}}, i == 0 || i == 9 ? 10.f : 0.f, c);
        art::fillCircle(window, {x + stripeW / 2.f, panelRect.position.y + 34.f}, stripeW / 2.f, c);
    }
    art::fillRoundedRect(window, {{panelRect.position.x - 8.f, panelRect.position.y - 26.f}, {panelRect.size.x + 16.f, 12.f}}, 6.f, woodDark);

    ui::drawText(window, font, "Seed Shop", {500.f, 196.f}, 32, ink, ui::Align::Center, true);
    ui::drawText(window, font, "Shift+click (or Shift+1-4) buys 5", {500.f, 222.f}, 14, inkSoft, ui::Align::Center);

    for (int i = 0; i < kCropCount; ++i)
    {
        const CropInfo&     info = Crop::info(Crop::fromIndex(i));
        const float         y    = 236.f + static_cast<float>(i) * 74.f;
        const sf::FloatRect row{{224.f, y - 6.f}, {552.f, 62.f}};
        art::fillRoundedRect(window, row, 14.f, creamDark);

        art::drawProduce(window, {}, Crop::fromIndex(i), {262.f, y + 26.f}, 0.95f, false, true,
                         std::fmod(time + static_cast<float>(i), 4.f) < 0.12f);
        ui::drawText(window, font, info.name, {296.f, y + 13.f}, 21, ink, ui::Align::Left, true);
        std::string details = "Grows in " + plural(info.daysToGrow, "day") + "  -  sells for " + std::to_string(info.sellPrice);
        ui::drawText(window, font, details, {296.f, y + 35.f}, 14, inkSoft);
        ui::drawText(window, font, info.blurb, {296.f, y + 51.f}, 13, info.regrowDays > 0 ? sf::Color(200, 80, 96) : sf::Color(150, 128, 104));

        ui::drawText(window, font, "Have " + std::to_string(seeds[static_cast<std::size_t>(i)]), {624.f, y + 26.f}, 15, inkSoft, ui::Align::Right, true);

        const sf::FloatRect buy     = shopBuyRect(i);
        const bool          afford  = coins >= info.seedCost;
        const bool          hovered = ui::drawButton(window, font, buy, "", sf::Color(122, 178, 98), mouse, 18, afford);
        const float         lift    = hovered ? 2.f : 0.f;
        ui::drawText(window, font, "Buy", buy.position + sf::Vector2f(18.f, 21.f - lift), 19, sf::Color::White, ui::Align::Left, true, 2.f,
                     afford ? sf::Color(74, 120, 58) : sf::Color(150, 140, 128));
        art::drawCoin(window, buy.position + sf::Vector2f(78.f, 21.f - lift), 9.f);
        ui::drawText(window, font, std::to_string(info.seedCost), buy.position + sf::Vector2f(92.f, 21.f - lift), 19, sf::Color::White, ui::Align::Left, true,
                     2.f, afford ? sf::Color(74, 120, 58) : sf::Color(150, 140, 128));
    }

    ui::drawButton(window, font, shopCloseRect(), "Close", sf::Color(184, 132, 88), mouse, 20);
}

void Game::drawHelp()
{
    using namespace ui::palette;
    const sf::FloatRect r     = helpPanelRect();
    const sf::Vector2f  mouse = mousePosition();
    ui::drawPanel(window, r, cream, sf::Color(214, 180, 132), 22.f, 4.f);

    ui::drawText(window, font, "How to Play", {500.f, 136.f}, 32, ink, ui::Align::Center, true);

    struct Line
    {
        int         icon;
        std::string a;
        std::string b;
    };
    const Line lines[] = {
        {0, "Pick seeds with 1-4 (or click the toolbar), then click soil to plant.", "You start with 5 carrot seeds and 100 coins."},
        {1, "Click a growing crop, or right-click anything, to water it.", "Press 5 for the watering can. Drag across plots to do lots at once."},
        {2, "Crops grow one step every night they were watered.", "A thirsty plant shows a little white bubble."},
        {3, "Press Space or Sleep to start a new day.", "Rainy days water every crop for you. Check tomorrow's forecast!"},
        {4, "Click ripe crops to sell them. Buy seeds in the shop (B).", "Click a + tile to buy more land. Each plot costs a bit more."},
        {5, "Lucky golden crops sell for 3x the price!", "Reach 1,000 coins to win the Golden Harvest Ribbon."},
    };

    float y = 186.f;
    for (const auto& line : lines)
    {
        const sf::Vector2f icon{196.f, y + 8.f};
        art::fillCircle(window, icon, 22.f, creamDark);
        switch (line.icon)
        {
            case 0:
                art::drawSeedPacket(window, icon, 0.95f, CropType::Carrot);
                break;
            case 1:
                art::drawWateringCan(window, icon + sf::Vector2f(-3.f, 2.f), 0.95f);
                break;
            case 2:
                art::drawSprout(window, icon + sf::Vector2f(0.f, 4.f), 1.2f);
                break;
            case 3:
                art::drawMoon(window, icon, 11.f, creamDark);
                break;
            case 4:
                art::drawCoin(window, icon, 11.f);
                break;
            case 5:
                art::drawProduce(window, {}, CropType::Strawberry, icon + sf::Vector2f(0.f, 2.f), 0.75f, true, true);
                break;
        }
        ui::drawText(window, font, line.a, {234.f, y}, 17, ink, ui::Align::Left, true);
        ui::drawText(window, font, line.b, {234.f, y + 20.f}, 15, inkSoft);
        y += 56.f;
    }

    std::string footer = "M: mute sound   -   F12: screenshot   -   Esc: close / back to title";
    ui::drawText(window, font, footer, {500.f, 526.f}, 14, inkSoft, ui::Align::Center);
    if (screen == Screen::Playing && harvested > 0)
    {
        const std::string stats = "Your farm so far: " + plural(harvested, "crop") + " harvested, " + withCommas(earned) + " coins earned";
        ui::drawText(window, font, stats, {500.f, 508.f}, 14, sf::Color(96, 150, 76), ui::Align::Center, true);
    }
    ui::drawButton(window, font, helpCloseRect(), "Got it!", sf::Color(122, 178, 98), mouse, 21);
}

void Game::drawRibbonPanel()
{
    using namespace ui::palette;
    const sf::FloatRect r{{280.f, 180.f}, {440.f, 356.f}};
    ui::drawPanel(window, r, cream, sf::Color(240, 190, 80), 24.f, 4.f);
    art::drawRibbon(window, {500.f, 262.f + std::sin(time * 3.f) * 3.f}, 2.4f);
    for (int i = 0; i < 6; ++i)
    {
        const float a = time * 0.8f + static_cast<float>(i) * kPi / 3.f;
        art::drawSparkle(window, sf::Vector2f(500.f, 262.f) + sf::Vector2f(std::cos(a) * 80.f, std::sin(a) * 50.f), 6.f + 2.f * std::sin(time * 4.f + static_cast<float>(i)),
                         sf::Color(250, 204, 80));
    }
    ui::drawText(window, font, "Golden Harvest Ribbon!", {500.f, 362.f}, 30, ink, ui::Align::Center, true);
    ui::drawText(window, font, "You saved up " + withCommas(kGoalCoins) + " coins in " + plural(day, "day") + ".", {500.f, 398.f}, 17, inkSoft,
                 ui::Align::Center);
    ui::drawText(window, font, "Your little farm is the pride of the valley.", {500.f, 422.f}, 17, inkSoft, ui::Align::Center);
    ui::drawText(window, font, "Keep farming as long as you like!", {500.f, 446.f}, 15, sf::Color(96, 150, 76), ui::Align::Center, true);
    ui::drawButton(window, font, ribbonCloseRect(), "Yay!", sf::Color(240, 170, 60), mousePosition(), 22);
}

void Game::drawTitle()
{
    using namespace ui::palette;
    scenery.drawGround(window);
    for (std::size_t i = 0; i < plots.size(); ++i)
        plots[i].draw(window, time, false);
    scenery.drawAmbient(window);
    art::fillRect(window, {{0.f, 0.f}, {1000.f, 700.f}}, sf::Color(255, 248, 231, 70));

    const sf::FloatRect card{{250.f, 96.f}, {500.f, hasSave ? 520.f : 492.f}};
    ui::drawPanel(window, card, sf::Color(255, 248, 231, 245), sf::Color(214, 180, 132), 28.f, 4.f);

    // Bouncing produce friends
    for (int i = 0; i < kCropCount; ++i)
    {
        const float x   = 410.f + static_cast<float>(i) * 60.f;
        const float hop = std::abs(std::sin(time * 2.6f + static_cast<float>(i) * 0.8f)) * 10.f;
        art::fillEllipse(window, {x, 182.f}, {16.f - hop * 0.5f, 4.f}, sf::Color(60, 40, 20, 40));
        art::drawProduce(window, {}, Crop::fromIndex(i), {x, 160.f - hop}, 1.05f, false, true, std::fmod(time + static_cast<float>(i) * 1.3f, 3.5f) < 0.12f);
    }

    ui::drawText(window, font, "Little Harvest", {500.f, 236.f}, 62, sf::Color(108, 160, 84), ui::Align::Center, true, 5.f, sf::Color::White);
    ui::drawText(window, font, "a tiny, cozy farming game", {500.f, 288.f}, 20, inkSoft, ui::Align::Center);
    art::drawSparkle(window, {262.f + 30.f, 220.f}, 7.f + std::sin(time * 3.f) * 2.f, sf::Color(250, 204, 80), time * 30.f);
    art::drawSparkle(window, {708.f, 250.f}, 6.f + std::cos(time * 3.f) * 2.f, sf::Color(240, 150, 160), -time * 30.f);

    if (hasSave)
        ui::drawText(window, font, "Your farm: day " + std::to_string(day) + "  -  " + withCommas(coins) + " coins", {500.f, 328.f}, 15, sf::Color(96, 150, 76),
                     ui::Align::Center, true);

    const sf::Vector2f mouse = panel == Panel::None ? mousePosition() : sf::Vector2f(-100.f, -100.f);
    for (const auto& [id, rect] : titleButtons())
    {
        switch (id)
        {
            case TitleButton::Continue:
                ui::drawButton(window, font, rect, "Continue", sf::Color(122, 178, 98), mouse, 24);
                break;
            case TitleButton::NewFarm:
                ui::drawButton(window, font, rect, confirmNewFarm ? "Click again to restart" : "New Farm",
                               confirmNewFarm ? sf::Color(232, 108, 118) : (hasSave ? sf::Color(240, 170, 80) : sf::Color(122, 178, 98)), mouse,
                               confirmNewFarm ? 19 : 24);
                break;
            case TitleButton::HowToPlay:
                ui::drawButton(window, font, rect, "How to Play", sf::Color(118, 160, 214), mouse, 24);
                break;
        }
    }
    ui::drawText(window, font, "Made with C++ and SFML 3   -   Enter to start   -   M to mute", {500.f, card.position.y + card.size.y - 26.f}, 13, inkSoft,
                 ui::Align::Center);
}

void Game::drawSleepOverlay()
{
    float alpha = 0.f;
    if (sleepTimer < kSleepFadeOut)
        alpha = sleepTimer / kSleepFadeOut;
    else if (sleepTimer < kSleepHold)
        alpha = 1.f;
    else
        alpha = 1.f - (sleepTimer - kSleepHold) / (kSleepEnd - kSleepHold);
    alpha = std::clamp(alpha, 0.f, 1.f);

    art::fillRect(window, {{0.f, 0.f}, {1000.f, 700.f}}, art::withAlpha(kNight, static_cast<std::uint8_t>(240.f * alpha)));

    // Twinkling stars
    std::mt19937 starRng(99);
    for (int i = 0; i < 40; ++i)
    {
        const sf::Vector2f p{std::uniform_real_distribution<float>(20.f, 980.f)(starRng), std::uniform_real_distribution<float>(20.f, 680.f)(starRng)};
        const float        tw = 0.5f + 0.5f * std::sin(time * 3.f + static_cast<float>(i));
        art::drawSparkle(window, p, 2.f + tw * 3.f, sf::Color(255, 240, 200, static_cast<std::uint8_t>(200.f * alpha * tw)));
    }

    if (dayAdvanced)
    {
        const auto        a = static_cast<std::uint8_t>(255.f * alpha);
        const sf::Color   cream(255, 248, 231, a);
        ui::drawText(window, font, "Day " + std::to_string(day), {500.f, 300.f}, 64, cream, ui::Align::Center, true);
        if (a > 40)
            drawWeatherIcon(weather, {450.f, 372.f}, 1.6f);
        ui::drawText(window, font, weatherName(weather) + " morning", {478.f, 372.f}, 24, sf::Color(220, 226, 255, a), ui::Align::Left, true);
    }
    else
    {
        const auto a = static_cast<std::uint8_t>(255.f * alpha);
        art::drawMoon(window, {500.f, 320.f}, 34.f, art::withAlpha(kNight, 255));
        for (int i = 0; i < 3; ++i)
        {
            const float k = std::fmod(time * 0.8f + static_cast<float>(i) * 0.33f, 1.f);
            ui::drawText(window, font, "z", {540.f + k * 40.f + static_cast<float>(i) * 6.f, 300.f - k * 50.f}, 18u + static_cast<unsigned>(i) * 6u,
                         sf::Color(255, 248, 231, static_cast<std::uint8_t>(static_cast<float>(a) * (1.f - k))), ui::Align::Center, true);
        }
    }
}
