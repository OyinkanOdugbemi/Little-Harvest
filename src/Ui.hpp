#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Small immediate-mode UI helpers: text, panels and buttons.
namespace ui
{
namespace palette
{
inline const sf::Color cream(255, 248, 231);
inline const sf::Color creamDark(242, 228, 198);
inline const sf::Color paper(250, 238, 212);
inline const sf::Color ink(94, 72, 54);
inline const sf::Color inkSoft(140, 116, 92);
inline const sf::Color wood(184, 132, 88);
inline const sf::Color woodDark(138, 94, 62);
inline const sf::Color leaf(122, 178, 98);
inline const sf::Color leafDark(86, 140, 70);
inline const sf::Color berry(232, 108, 118);
inline const sf::Color sky(120, 182, 230);
inline const sf::Color gold(250, 204, 80);
inline const sf::Color shadow(60, 40, 20, 46);
} // namespace palette

enum class Align
{
    Left,
    Center,
    Right
};

// Draws one line of text. `pos.y` is the vertical centre of the capital letters,
// so text lines up neatly with icons drawn at the same y.
void drawText(sf::RenderTarget&  target,
              const sf::Font&    font,
              const std::string& text,
              sf::Vector2f       pos,
              unsigned           size,
              sf::Color          color,
              Align              align        = Align::Left,
              bool               bold         = false,
              float              outline      = 0.f,
              sf::Color          outlineColor = sf::Color::Transparent);

float textWidth(const sf::Font& font, const std::string& text, unsigned size, bool bold = false);

// A rounded panel with a soft drop shadow.
void drawPanel(sf::RenderTarget& target, sf::FloatRect rect, sf::Color fill, sf::Color border, float radius = 16.f, float borderWidth = 3.f);

// A chunky rounded button. Returns true if the mouse is over it.
bool drawButton(sf::RenderTarget&  target,
                const sf::Font&    font,
                sf::FloatRect      rect,
                const std::string& label,
                sf::Color          color,
                sf::Vector2f       mouse,
                unsigned           textSize = 20,
                bool               enabled  = true);
} // namespace ui
