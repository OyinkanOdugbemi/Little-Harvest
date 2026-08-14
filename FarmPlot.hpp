#pragma once

#include <SFML/Graphics.hpp>

enum class CropType
{
    Carrot,
    Strawberry
};

class FarmPlot
{
    public:
        FarmPlot(float x, float y, CropType cropType);
        void draw(sf::RenderWindow& window);
        void plant();
        void water();
        void grow();
        bool harvest();

        bool isWatered() const;
        bool contains(sf::Vector2f position);
        bool isEmpty() const;
        
    
    private:
        sf::RectangleShape soil;
        CropType cropType;
        int growthStage = 0;
        bool watered = false;

};