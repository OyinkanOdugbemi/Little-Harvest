#include "Ui.hpp"

#include <cmath>

#include "Art.hpp"

namespace ui
{
void drawText(sf::RenderTarget&  target,
              const sf::Font&    font,
              const std::string& string,
              sf::Vector2f       pos,
              unsigned           size,
              sf::Color          color,
              Align              align,
              bool               bold,
              float              outline,
              sf::Color          outlineColor)
{
    sf::Text text(font, sf::String::fromUtf8(string.begin(), string.end()), size);
    text.setFillColor(color);
    if (bold)
        text.setStyle(sf::Text::Bold);
    if (outline > 0.f)
    {
        text.setOutlineThickness(outline);
        text.setOutlineColor(outlineColor);
    }

    const sf::FloatRect bounds    = text.getLocalBounds();
    const float         capHeight = font.getGlyph(U'H', size, bold).bounds.size.y;
    float               originX   = bounds.position.x;
    if (align == Align::Center)
        originX += bounds.size.x / 2.f;
    else if (align == Align::Right)
        originX += bounds.size.x;

    text.setOrigin({originX, static_cast<float>(size) - capHeight / 2.f});
    text.setPosition({std::round(pos.x), std::round(pos.y)});
    target.draw(text);
}

float textWidth(const sf::Font& font, const std::string& string, unsigned size, bool bold)
{
    sf::Text text(font, sf::String::fromUtf8(string.begin(), string.end()), size);
    if (bold)
        text.setStyle(sf::Text::Bold);
    return text.getLocalBounds().size.x;
}

void drawPanel(sf::RenderTarget& target, sf::FloatRect rect, sf::Color fill, sf::Color border, float radius, float borderWidth)
{
    art::fillRoundedRect(target, {rect.position + sf::Vector2f(0.f, 6.f), rect.size}, radius, palette::shadow);
    art::fillRoundedRect(target, rect, radius, fill, borderWidth, border);
}

bool drawButton(sf::RenderTarget&  target,
                const sf::Font&    font,
                sf::FloatRect      rect,
                const std::string& label,
                sf::Color          color,
                sf::Vector2f       mouse,
                unsigned           textSize,
                bool               enabled)
{
    const bool hovered = enabled && rect.contains(mouse);
    sf::Color  face    = enabled ? color : sf::Color(196, 186, 170);
    if (hovered)
        face = art::lighter(face, 0.14f);

    const float lift = hovered ? 2.f : 0.f;
    const float base = 5.f; // depth of the button "lip"

    art::fillRoundedRect(target, {rect.position + sf::Vector2f(0.f, 4.f), rect.size}, 14.f, palette::shadow);
    art::fillRoundedRect(target, {rect.position + sf::Vector2f(0.f, base - lift), rect.size - sf::Vector2f(0.f, base)}, 14.f, art::darker(face, 0.78f));
    sf::FloatRect top{rect.position - sf::Vector2f(0.f, lift), rect.size - sf::Vector2f(0.f, base)};
    art::fillRoundedRect(target, top, 14.f, face);
    art::fillRoundedRect(target, {top.position + sf::Vector2f(8.f, 4.f), {top.size.x - 16.f, 5.f}}, 2.5f, sf::Color(255, 255, 255, 60));

    if (!label.empty())
        drawText(target,
                 font,
                 label,
                 top.position + top.size / 2.f,
                 textSize,
                 enabled ? sf::Color::White : sf::Color(245, 240, 230),
                 Align::Center,
                 true,
                 2.f,
                 art::darker(face, 0.62f));
    return hovered;
}
} // namespace ui
