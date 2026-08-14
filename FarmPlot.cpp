#include "FarmPlot.hpp"

FarmPlot::FarmPlot(float x,float y)
{
    soil.setSize({90.f, 90.f});
    soil.setPosition({x,y});
    soil.setFillColor(sf::Color(170, 125, 85));
}

void FarmPlot::draw(sf::RenderWindow& window)
{
    window.draw(soil);

    if (growthStage == 1)
    {
        // Tiny sprout 🌱

        sf::RectangleShape stem({4.f, 18.f});
        stem.setFillColor(sf::Color(75, 150, 75));

        stem.setPosition({
            soil.getPosition().x + 43.f,
            soil.getPosition().y + 38.f
        });

        window.draw(stem);

        sf::CircleShape leftLeaf(8.f);
        leftLeaf.setFillColor(sf::Color(90, 175, 90));

        leftLeaf.setPosition({
            soil.getPosition().x + 30.f,
            soil.getPosition().y + 32.f
        });

        window.draw(leftLeaf);

        sf::CircleShape rightLeaf(8.f);
        rightLeaf.setFillColor(sf::Color(100, 185, 95));

        rightLeaf.setPosition({
            soil.getPosition().x + 48.f,
            soil.getPosition().y + 28.f
        });

        window.draw(rightLeaf);
    }

    else if (growthStage == 2)
    {
        // Growing carrot plant 🌿

        sf::RectangleShape stem({6.f, 35.f});
        stem.setFillColor(sf::Color(70, 145, 70));

        stem.setPosition({
            soil.getPosition().x + 42.f,
            soil.getPosition().y + 25.f
        });

        window.draw(stem);

        // Leaves
        sf::CircleShape leaf1(13.f);
        leaf1.setFillColor(sf::Color(75, 165, 80));

        leaf1.setPosition({
            soil.getPosition().x + 20.f,
            soil.getPosition().y + 15.f
        });

        window.draw(leaf1);

        sf::CircleShape leaf2(15.f);
        leaf2.setFillColor(sf::Color(85, 175, 85));

        leaf2.setPosition({
            soil.getPosition().x + 38.f,
            soil.getPosition().y + 8.f
        });

        window.draw(leaf2);

        sf::CircleShape leaf3(12.f);
        leaf3.setFillColor(sf::Color(70, 155, 75));

        leaf3.setPosition({
            soil.getPosition().x + 52.f,
            soil.getPosition().y + 18.f
        });

        window.draw(leaf3);
    }

    else if (growthStage == 3)
    {
        // Fully grown carrot 🥕

        // Green leaves
        sf::CircleShape leaf1(14.f);
        leaf1.setFillColor(sf::Color(65, 150, 70));

        leaf1.setPosition({
            soil.getPosition().x + 30.f,
            soil.getPosition().y + 12.f
        });

        window.draw(leaf1);

        sf::CircleShape leaf2(16.f);
        leaf2.setFillColor(sf::Color(75, 165, 75));

        leaf2.setPosition({
            soil.getPosition().x + 40.f,
            soil.getPosition().y + 5.f
        });

        window.draw(leaf2);

        sf::CircleShape leaf3(13.f);
        leaf3.setFillColor(sf::Color(60, 145, 65));

        leaf3.setPosition({
            soil.getPosition().x + 52.f,
            soil.getPosition().y + 15.f
        });

        window.draw(leaf3);

        // Carrot
        sf::CircleShape carrot(18.f);
        carrot.setFillColor(sf::Color(235, 145, 55));

        carrot.setPosition({
            soil.getPosition().x + 27.f,
            soil.getPosition().y + 42.f
        });

        window.draw(carrot);
    }

    // Water indicator 💧
    if (watered)
    {
        sf::CircleShape waterDrop(5.f);
        waterDrop.setFillColor(sf::Color(100, 180, 220));

        waterDrop.setPosition({
            soil.getPosition().x + 72.f,
            soil.getPosition().y + 8.f
        });

        window.draw(waterDrop);
    }
}

void FarmPlot::plant()
{
    if (growthStage == 0)
    {
        growthStage = 1;
        watered = false;
        soil.setFillColor(sf::Color(150, 105, 70));
    }
}

void FarmPlot::water()
{
    if (growthStage > 0)
    {
        watered = true;
    }
}

bool FarmPlot::contains(sf::Vector2f position)
{
    return soil.getGlobalBounds().contains(position);
}

void FarmPlot::grow()
{
    if (growthStage > 0 &&
        growthStage < 3 &&
        watered)
    {
        growthStage++;
    }

    watered = false;
}

bool FarmPlot::isEmpty() const
{
    return growthStage == 0;
}

bool FarmPlot::isWatered() const
{
    return watered;
}

bool FarmPlot::harvest()
{
    if (growthStage == 3)
    {
        growthStage = 0;
        watered = false;

        soil.setFillColor(sf::Color(170, 125, 85));

        return true;
    }

    return false;
}
