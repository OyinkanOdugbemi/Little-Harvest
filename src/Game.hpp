#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "Crop.hpp"
#include "Effects.hpp"
#include "FarmPlot.hpp"
#include "Scenery.hpp"
#include "SoundBank.hpp"

class Game
{
public:
    Game();
    void run();

private:
    enum class Screen
    {
        Title,
        Playing
    };
    enum class Panel
    {
        None,
        Shop,
        Help,
        Ribbon
    };
    enum class Action
    {
        None,
        Harvest,
        Plant,
        Water,
        Unlock
    };
    enum class TitleButton
    {
        Continue,
        NewFarm,
        HowToPlay
    };

    static constexpr int kCanSlot   = kCropCount; // the watering can sits after the seeds
    static constexpr int kSlotCount = kCropCount + 1;
    static constexpr int kGoalCoins = 1000;

    // ---- setup & saving ----
    void loadFont();
    void setupIcon();
    void resetFarm();
    void startNewGame();
    bool loadGame();
    void saveGame() const;

    // ---- loop ----
    void handleEvent(const sf::Event& event);
    void handleKey(sf::Keyboard::Key key, bool shift);
    void handleClick(sf::Vector2f pos, sf::Mouse::Button button, bool shift);
    bool handlePanelClick(sf::Vector2f pos, bool shift);
    void handleTitleClick(sf::Vector2f pos);
    void update(float dt);
    void render();

    // ---- gameplay ----
    Action actionFor(const FarmPlot& plot, bool rightClick) const;
    bool   applyAction(Action action, int plotIndex, bool quiet);
    void   selectSlot(int slot);
    bool   buySeeds(int cropIndex, int amount);
    void   startSleep();
    void   advanceDay();
    int    plotPrice() const;
    void   addCoins(int amount, sf::Vector2f from);
    void   showToast(const std::string& text, sf::Color accent);
    Weather rollWeather();
    std::string weatherName(Weather w) const;

    // ---- drawing ----
    void drawWorld();
    void drawHud();
    void drawHotbar();
    void drawHint();
    void drawToast();
    void drawTooltip();
    void drawCursorIcon();
    void drawShop();
    void drawHelp();
    void drawRibbonPanel();
    void drawTitle();
    void drawSleepOverlay();
    void drawWeatherIcon(Weather w, sf::Vector2f center, float scale);
    void dimBackground();

    // ---- layout ----
    sf::FloatRect slotRect(int slot) const;
    sf::FloatRect shopButtonRect() const;
    sf::FloatRect sleepButtonRect() const;
    sf::FloatRect shopPanelRect() const;
    sf::FloatRect shopBuyRect(int row) const;
    sf::FloatRect shopCloseRect() const;
    sf::FloatRect helpPanelRect() const;
    sf::FloatRect helpCloseRect() const;
    sf::FloatRect ribbonCloseRect() const;
    std::vector<std::pair<TitleButton, sf::FloatRect>> titleButtons() const;
    int          plotAt(sf::Vector2f pos) const;
    sf::Vector2f mousePosition() const;
    bool         isOverClickable(sf::Vector2f pos) const;

    sf::RenderWindow window;
    sf::Font         font;
    std::mt19937     rng;
    Effects          effects;
    Scenery          scenery;
    SoundBank        sounds;
    std::vector<FarmPlot> plots;

    Screen screen = Screen::Title;
    Panel  panel  = Panel::None;

    // farm state (saved)
    int                          day          = 1;
    int                          coins        = 100;
    std::array<int, kCropCount>  seeds{};
    Weather                      weather      = Weather::Sunny;
    Weather                      tomorrow     = Weather::Sunny;
    int                          plotsBought  = 0;
    int                          harvested    = 0;
    int                          earned       = 0;
    bool                         ribbonEarned = false;

    // UI state
    int   selectedSlot   = 0;
    float shownCoins     = 100.f;
    float coinBounce     = 0.f;
    float time           = 0.f;
    bool  hasSave        = false;
    bool  confirmNewFarm = false;
    bool  firstDay       = false;

    // drag-to-repeat
    bool   dragging   = false;
    bool   dragRight  = false;
    Action dragAction = Action::None;
    int    lastPlot   = -1;

    // toast
    std::string toastText;
    sf::Color   toastAccent;
    float       toastTimer = 0.f;

    // sleeping transition
    bool        sleeping     = false;
    float       sleepTimer   = 0.f;
    bool        dayAdvanced  = false;
    bool        morningSound = false;
    std::string morningMessage;

    // misc
    bool                     screenshotRequested = false;
    int                      screenshotCount     = 0;
    std::optional<sf::Cursor> handCursor;
    std::optional<sf::Cursor> arrowCursor;
    bool                     usingHand = false;
};
