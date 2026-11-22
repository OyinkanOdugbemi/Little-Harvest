#pragma once

// Screen layout shared by the game, the scenery and the UI.
namespace layout
{
constexpr unsigned kWidth  = 1000;
constexpr unsigned kHeight = 700;

// Farm grid
constexpr int   kCols  = 6;
constexpr int   kRows  = 4;
constexpr float kTile  = 80.f;
constexpr float kGap   = 12.f;
constexpr float kGridX = 230.f;
constexpr float kGridY = 118.f;

// Fence around the farm (the gate is a gap in the bottom side)
constexpr float kFenceLeft   = 208.f;
constexpr float kFenceTop    = 98.f;
constexpr float kFenceRight  = 792.f;
constexpr float kFenceBottom = 494.f;
constexpr float kGateLeft    = 468.f;
constexpr float kGateRight   = 532.f;
} // namespace layout
