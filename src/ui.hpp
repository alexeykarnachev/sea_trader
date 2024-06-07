#pragma once

namespace rl {
#include "raylib/raylib.h"
}  // namespace rl

namespace st {
namespace ui {

namespace color {
// line
const rl::Color LINE_LIGHT = {245, 245, 245, 255};
const rl::Color LINE_MILD = {150, 150, 150, 255};
const rl::Color LINE_DARK = {10, 10, 10, 255};

// rect
const rl::Color RECT_COLD = {70, 50, 30, 255};
const rl::Color RECT_HOVER = {110, 90, 70, 255};

// sprite tints
const rl::Color SPRITE_COLD = {90, 70, 50, 255};
const rl::Color SPRITE_HOVER = {110, 90, 70, 255};
const rl::Color SPRITE_DOWN = {245, 245, 245, 255};
const rl::Color SPRITE_SELECTED = {245, 245, 245, 255};

// text
const rl::Color TEXT_LIGHT = {245, 245, 245, 255};
const rl::Color TEXT_MILD = {150, 150, 150, 255};
const rl::Color TEXT_DARK = {10, 10, 10, 255};
const rl::Color TEXT_BUY = {50, 255, 50, 255};
const rl::Color TEXT_SELL = {255, 50, 50, 255};

// button
const rl::Color BUTTON_COLD = {90, 70, 50, 255};
const rl::Color BUTTON_HOVER = {110, 90, 70, 255};
const rl::Color BUTTON_DOWN = {140, 120, 100, 255};

// radio button
const rl::Color RADIO_BUTTON_COLD = {80, 60, 40, 255};
const rl::Color RADIO_BUTTON_HOVER = {100, 80, 60, 255};
const rl::Color RADIO_BUTTON_DOWN = {140, 120, 100, 255};
const rl::Color RADIO_BUTTON_SELECTED = {140, 120, 100, 255};

// increment button
const rl::Color INCREMENT_BUTTON_COLD = {80, 60, 40, 255};
const rl::Color INCREMENT_BUTTON_HOVER = {100, 80, 60, 255};
const rl::Color INCREMENT_BUTTON_DOWN = {140, 120, 100, 255};
}  // namespace color

enum class SpriteName {
    RIGHT_ARROW_ICON_SRC,
    LEFT_ARROW_ICON_SRC,
};

void load();
void unload();
void begin();

// -----------------------------------------------------------------------
// sprite
void sprite(rl::Texture texture, rl::Rectangle src, rl::Rectangle dst);
void sprite(SpriteName sprite_name, rl::Rectangle dst);

// -----------------------------------------------------------------------
// button
bool button_sprite(rl::Texture texture, rl::Rectangle src, rl::Rectangle dst);
bool button_sprite(SpriteName sprite_name, rl::Rectangle dst);
bool button_rect(rl::Rectangle dst);

// -----------------------------------------------------------------------
// radio button
bool radio_button_sprite(
    rl::Texture texture, rl::Rectangle src, rl::Rectangle dst, int *store, int value
);
bool radio_button_sprite(
    SpriteName sprite_name, rl::Rectangle dst, int *store, int value
);
bool radio_button_rect(rl::Rectangle dst, int *store, int value);

// -----------------------------------------------------------------------
// increment button
bool increment_button_sprite(
    rl::Texture texture,
    rl::Rectangle src,
    rl::Rectangle dst,
    int *value,
    int speed,
    int min,
    int max
);
bool increment_button_sprite(
    SpriteName sprite_name, rl::Rectangle dst, int *value, int speed, int min, int max
);
bool increment_button_rect(rl::Rectangle dst, int *value, int speed, int min, int max);
}  // namespace ui

}  // namespace st
