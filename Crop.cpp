#include "Crop.hpp"

#include <array>

namespace
{
const std::array<CropInfo, kCropCount> kCrops = {{
    {"Carrot", 10, 25, 2, 0, sf::Color(245, 145, 55), "Quick and crunchy"},
    {"Strawberry", 20, 30, 3, 2, sf::Color(232, 72, 92), "Regrows after picking!"},
    {"Sunflower", 30, 75, 4, 0, sf::Color(252, 205, 70), "Tall, bright and happy"},
    {"Pumpkin", 60, 180, 6, 0, sf::Color(238, 132, 48), "Slow, but worth a fortune"},
}};
} // namespace

Crop::Crop(CropType type) : type(type)
{
}

const CropInfo& Crop::info(CropType type)
{
    return kCrops[static_cast<std::size_t>(type)];
}

CropType Crop::fromIndex(int index)
{
    if (index < 0 || index >= kCropCount)
        return CropType::Carrot;
    return static_cast<CropType>(index);
}

CropType Crop::getType() const
{
    return type;
}

const CropInfo& Crop::getInfo() const
{
    return info(type);
}

std::string Crop::getName() const
{
    return getInfo().name;
}

int Crop::getSellPrice() const
{
    return getInfo().sellPrice;
}

int Crop::getSeedCost() const
{
    return getInfo().seedCost;
}

int Crop::getDaysToGrow() const
{
    return getInfo().daysToGrow;
}

int Crop::getRegrowDays() const
{
    return getInfo().regrowDays;
}
