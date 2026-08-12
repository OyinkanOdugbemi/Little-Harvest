#include <SFML/Graphics.hpp>
#include <vector>
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

    sf::Text dayText(font);
    dayText.setCharacterSize(32);
    dayText.setFillColor(sf::Color(70, 70, 60));
    dayText.setPosition({30.f, 25.f});

const int rows = 3;
const int columns = 5;

const float spacing = 10.f;
const float plotSize = 90.f;

for (int row = 0; row < rows; row++)
{
    for (int column = 0; column < columns; column++)
    {
        float x = 200.f + column * (plotSize + spacing);
        float y = 200.f + row * (plotSize + spacing);

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
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f mousePosition(
                    static_cast<float>(mousePressed->position.x),
                    static_cast<float>(mousePressed->position.y)
                );

                for (auto& plot : plots)
                {
                    if (plot.contains(mousePosition))
                    {
                        plot.plant();
                    }
                }
            }
        }

        // Keyboard input
        if (const auto* keyPressed =
            event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Space)
            {
                currentDay++;
                dayText.setString("Day " + std::to_string(currentDay));
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

    window.display();
}

    return 0;
}
