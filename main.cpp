#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "FarmPlot.hpp"


int main()
{
    sf::RenderWindow window(
        sf::VideoMode({1000, 700}),
        "Little Harvest"
    );

    sf::Font font("assets/fonts/Nunito-Regular.ttf");
    std::vector<FarmPlot> plots;
    int currentDay = 1;
    int seeds = 5;
    int coins = 100;
    bool shopOpen = false;

    sf::Text dayText(font);
    dayText.setCharacterSize(32);
    dayText.setFillColor(sf::Color(70, 70, 60));
    dayText.setPosition({450.f, 25.f});
    dayText.setString("Day 1");

    sf::RectangleShape topBar(sf::Vector2f(1000.f, 90.f));
    topBar.setFillColor(sf::Color(255, 248, 230));
    topBar.setPosition({0.f, 0.f});

    sf::Text titleText(font);
    titleText.setCharacterSize(28);
    titleText.setFillColor(sf::Color(80, 100, 70));
    titleText.setString("Little Harvest");
    titleText.setPosition({30.f, 28.f});

    sf::Text moneyText(font);
    moneyText.setCharacterSize(26);
    moneyText.setFillColor(sf::Color(90, 80, 60));
    moneyText.setString("Coins: 100");
    moneyText.setPosition({800.f, 30.f});

    sf::Text seedText(font);
    seedText.setCharacterSize(24);
    seedText.setFillColor(sf::Color(90, 110, 70));
    seedText.setString("Seeds: 5");
    seedText.setPosition({650.f, 31.f});

    sf::RectangleShape shopPanel(sf::Vector2f(420.f, 300.f));
    shopPanel.setFillColor(sf::Color(255, 248, 230));
    shopPanel.setOutlineColor(sf::Color(120, 140, 90));
    shopPanel.setOutlineThickness(4.f);
    shopPanel.setPosition({290.f, 190.f});

    sf::Text shopTitle(font);
    shopTitle.setCharacterSize(32);
    shopTitle.setFillColor(sf::Color(80, 100, 70));
    shopTitle.setString("Farm Shop");
    shopTitle.setPosition({405.f, 215.f});

    sf::Text shopInfo(font);
    shopInfo.setCharacterSize(22);
    shopInfo.setFillColor(sf::Color(90, 80, 60));
    shopInfo.setString(
    "Carrot Seeds\n\n1 seed = 10 coins"
);
    shopInfo.setPosition({345.f, 270.f});

    sf::Text shopClose(font);shopClose.setCharacterSize(18);
    shopClose.setFillColor(sf::Color(110, 110, 100));
    shopClose.setString("Press B to close");
    shopClose.setPosition({410.f, 440.f});

    sf::RectangleShape buyButton(sf::Vector2f(180.f, 55.f));
    buyButton.setFillColor(sf::Color(120, 165, 95));
    buyButton.setPosition({410.f, 365.f});

    sf::Text buyButtonText(font);
    buyButtonText.setCharacterSize(22);
    buyButtonText.setFillColor(sf::Color::White);
    buyButtonText.setString("BUY SEED");
    buyButtonText.setPosition({450.f, 378.f});

const int rows = 3;
const int columns = 5;

const float spacing = 10.f;
const float plotSize = 90.f;

for (int row = 0; row < rows; row++)
{
    for (int column = 0; column < columns; column++)
    {
        float x = 200.f + column * (plotSize + spacing);
        float y = 150.f + row * (plotSize + spacing);

        plots.emplace_back(x, y);
    }
}

while (window.isOpen())
{
    while (auto event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }

// Mouse input
if (const auto* mousePressed =
    event->getIf<sf::Event::MouseButtonPressed>())
{
    sf::Vector2f mousePosition(
        static_cast<float>(mousePressed->position.x),
        static_cast<float>(mousePressed->position.y)
    );

// LEFT CLICK = PLANT OR HARVEST
if (mousePressed->button == sf::Mouse::Button::Left)
{
    for (auto& plot : plots)
    {
        if (plot.contains(mousePosition))
        {
            // Harvest fully grown crop
            if (!plot.isEmpty())
            {
                if (plot.harvest())
                {
                    coins += 20;

                    moneyText.setString(
                        "Coins: " + std::to_string(coins)
                    );
                }
            }

            // Plant a new seed
            else if (seeds > 0)
            {
                plot.plant();

                seeds--;

                seedText.setString(
                    "Seeds: " + std::to_string(seeds)
                );
            }
        }
    }
}

    // RIGHT CLICK = WATER
    if (mousePressed->button == sf::Mouse::Button::Right)
    {
        for (auto& plot : plots)
        {
            if (plot.contains(mousePosition))
            {
                plot.water();
            }
        }
    }

    // SHOP BUTTON
if (shopOpen &&
    mousePressed->button == sf::Mouse::Button::Left)
{
    if (buyButton.getGlobalBounds().contains(mousePosition))
    {
        if (coins >= 10)
        {
            coins -= 10;
            seeds++;

            moneyText.setString(
                "Coins: " + std::to_string(coins)
            );

            seedText.setString(
                "Seeds: " + std::to_string(seeds)
            );
        }
    }
}
}

        // Keyboard input
        if (const auto* keyPressed =
            event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::B)
                {
                    shopOpen = !shopOpen;
                }

    if (keyPressed->code == sf::Keyboard::Key::Enter &&
        shopOpen)
    {
        if (coins >= 10)
        {
            coins -= 10;
            seeds++;

            moneyText.setString(
                "Coins: " + std::to_string(coins)
            );

            seedText.setString(
                "Seeds: " + std::to_string(seeds)
            );
        }
    }

    if (keyPressed->code == sf::Keyboard::Key::Space)
    {
        currentDay++;

        dayText.setString(
            "Day " + std::to_string(currentDay)
        );

        for (auto& plot : plots)
        {
            plot.grow();
        }
    }
}
    }

    window.clear(sf::Color(185, 225, 210));

    sf::RectangleShape grass(sf::Vector2f(1000.f, 700.f));
    grass.setFillColor(sf::Color(145, 190, 120));
    window.draw(grass);

    for (auto& plot : plots)
    {
        plot.draw(window);
    }

    window.draw(topBar);
    window.draw(titleText);
    window.draw(dayText);
    window.draw(seedText);
    window.draw(moneyText);
    
    if (shopOpen)
{
    sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

    if (buyButton.getGlobalBounds().contains(
        sf::Vector2f(
            static_cast<float>(mousePosition.x),
            static_cast<float>(mousePosition.y)
        )))
    {
        buyButton.setFillColor(sf::Color(140, 185, 110));
    }
    else
    {
        buyButton.setFillColor(sf::Color(120, 165, 95));
    }

    window.draw(shopPanel);
    window.draw(shopTitle);
    window.draw(shopInfo);
    window.draw(buyButton);
    window.draw(buyButtonText);
    window.draw(shopClose);
}

    window.display();
}

    return 0;
}
