#pragma once

#include <SFML/Graphics.hpp>

#include "Crop.hpp"

// Everything in Little Harvest is drawn with plain SFML shapes, no image files.
// These helpers build the soft, rounded pieces the art is made from.
namespace art
{
// ---- colour helpers -------------------------------------------------------
sf::Color darker(sf::Color c, float factor = 0.75f);
sf::Color lighter(sf::Color c, float amount = 0.3f);
sf::Color withAlpha(sf::Color c, std::uint8_t alpha);
sf::Color mix(sf::Color a, sf::Color b, float t);

// ---- shape builders (centred on 0,0 unless noted) --------------------------
sf::ConvexShape roundedRect(sf::Vector2f size, float radius, int cornerPoints = 6); // top-left at 0,0
sf::ConvexShape ellipse(sf::Vector2f radii, int points = 32);
sf::ConvexShape teardrop(float radius, float tail, int points = 36); // round top, point at +y
sf::ConvexShape star(float outer, float inner, int tips = 4);

// ---- immediate drawing -----------------------------------------------------
void fillRoundedRect(sf::RenderTarget& target,
                     sf::FloatRect     rect,
                     float             radius,
                     sf::Color         fill,
                     float             outline      = 0.f,
                     sf::Color         outlineColor = sf::Color::Transparent,
                     sf::RenderStates  states       = sf::RenderStates::Default);

void fillEllipse(sf::RenderTarget& target,
                 sf::Vector2f      center,
                 sf::Vector2f      radii,
                 sf::Color         fill,
                 float             rotationDeg  = 0.f,
                 float             outline      = 0.f,
                 sf::Color         outlineColor = sf::Color::Transparent,
                 sf::RenderStates  states       = sf::RenderStates::Default);

void fillCircle(sf::RenderTarget& target,
                sf::Vector2f      center,
                float             radius,
                sf::Color         fill,
                float             outline      = 0.f,
                sf::Color         outlineColor = sf::Color::Transparent,
                sf::RenderStates  states       = sf::RenderStates::Default);

void fillRect(sf::RenderTarget& target,
              sf::FloatRect     rect,
              sf::Color         fill,
              float             rotationDeg = 0.f,
              sf::RenderStates  states      = sf::RenderStates::Default);

// ---- characters and icons ----------------------------------------------------
// A tiny smiling face. `center` is between the eyes.
void drawFace(sf::RenderTarget& target, sf::RenderStates states, sf::Vector2f center, float scale, bool blink);

// Ripe produce (the part you sell). About 36px tall at scale 1.
void drawProduce(sf::RenderTarget& target,
                 sf::RenderStates  states,
                 CropType          type,
                 sf::Vector2f      center,
                 float             scale,
                 bool              golden = false,
                 bool              face   = true,
                 bool              blink  = false);

void drawCoin(sf::RenderTarget& target, sf::Vector2f center, float radius, float squash = 1.f);
void drawSparkle(sf::RenderTarget& target, sf::Vector2f center, float size, sf::Color color, float rotationDeg = 0.f);
void drawWaterDrop(sf::RenderTarget& target, sf::Vector2f center, float size, sf::Color color);
void drawWateringCan(sf::RenderTarget& target, sf::Vector2f center, float scale, float tiltDeg = 0.f);
void drawSeedPacket(sf::RenderTarget& target, sf::Vector2f center, float scale, CropType type);
void drawSun(sf::RenderTarget& target, sf::Vector2f center, float scale, float spin = 0.f);
void drawCloud(sf::RenderTarget& target, sf::Vector2f center, float scale, sf::Color color = sf::Color(250, 250, 255));
void drawRainCloud(sf::RenderTarget& target, sf::Vector2f center, float scale);
void drawMoon(sf::RenderTarget& target, sf::Vector2f center, float radius, sf::Color background);
void drawSpeaker(sf::RenderTarget& target, sf::Vector2f center, float scale, bool muted, sf::Color color);
void drawRibbon(sf::RenderTarget& target, sf::Vector2f center, float scale);
void drawSprout(sf::RenderTarget& target, sf::Vector2f center, float scale);
} // namespace art
