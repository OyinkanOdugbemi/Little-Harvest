#pragma once

#include <SFML/Graphics/Color.hpp>
#include <string>

enum class CropType
{
    Carrot = 0,
    Strawberry,
    Sunflower,
    Pumpkin
};

constexpr int kCropCount = 4;

// Everything the game needs to know about one kind of crop.
struct CropInfo
{
    std::string name;
    int         seedCost;   // coins for one seed
    int         sellPrice;  // coins for one harvest
    int         daysToGrow; // watered days from seed to ripe
    int         regrowDays; // 0 = single harvest, otherwise days until it is ripe again
    sf::Color   color;      // main colour of the produce
    std::string blurb;      // short description for the shop
};

class Crop
{
public:
    explicit Crop(CropType type = CropType::Carrot);

    static const CropInfo& info(CropType type);
    static CropType        fromIndex(int index);

    CropType        getType() const;
    const CropInfo& getInfo() const;
    std::string     getName() const;
    int             getSellPrice() const;
    int             getSeedCost() const;
    int             getDaysToGrow() const;
    int             getRegrowDays() const;

private:
    CropType type;
};
