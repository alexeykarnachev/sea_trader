#pragma once

#include "raylib/raylib.h"

namespace st {
namespace ui {

namespace color {
// line
const Color LINE_LIGHT = {245, 245, 245, 255};
const Color LINE_MILD = {150, 150, 150, 255};
const Color LINE_DARK = {10, 10, 10, 255};

// border
const Color BORDER = {30, 10, 5, 255};

// rect
const Color RECT_COLD = {70, 50, 30, 255};
const Color RECT_HOVER = {110, 90, 70, 255};

// sprite tints
const Color SPRITE_COLD = {150, 150, 150, 255};
const Color SPRITE_HOVER = {200, 200, 200, 255};
const Color SPRITE_DOWN = {255, 255, 255, 255};
const Color SPRITE_SELECTED = {255, 255, 255, 255};

// text
const Color TEXT_LIGHT = {245, 245, 245, 255};
const Color TEXT_MILD = {150, 150, 150, 255};
const Color TEXT_DARK = {10, 10, 10, 255};
const Color TEXT_POSITIVE = {50, 255, 50, 255};
const Color TEXT_NEGATIVE = {255, 50, 50, 255};

// button
const Color BUTTON_COLD = {90, 70, 50, 255};
const Color BUTTON_HOVER = {110, 90, 70, 255};
const Color BUTTON_DOWN = {140, 120, 100, 255};

// radio button
const Color RADIO_BUTTON_COLD = {80, 60, 40, 255};
const Color RADIO_BUTTON_HOVER = {100, 80, 60, 255};
const Color RADIO_BUTTON_DOWN = {140, 120, 100, 255};
const Color RADIO_BUTTON_SELECTED = {140, 120, 100, 255};

// increment button
const Color INCREMENT_BUTTON_COLD = {80, 60, 40, 255};
const Color INCREMENT_BUTTON_HOVER = {100, 80, 60, 255};
const Color INCREMENT_BUTTON_DOWN = {140, 120, 100, 255};
}  // namespace color

enum class SpriteName {
    RIGHT_ARROW_ICON,
    LEFT_ARROW_ICON,
    ACCEPT_ICON,
    CANCEL_ICON,
};

void load();
void unload();
void begin();

// -----------------------------------------------------------------------
// sprite
void sprite(Texture texture, Rectangle src, Rectangle dst);
void sprite(SpriteName sprite_name, Rectangle dst);

// -----------------------------------------------------------------------
// button
bool button_sprite(Texture texture, Rectangle src, Rectangle dst);
bool button_sprite(SpriteName sprite_name, Rectangle dst);
bool button_rect(Rectangle dst);

// -----------------------------------------------------------------------
// radio button
bool radio_button_sprite(
    Texture texture, Rectangle src, Rectangle dst, int *store, int value
);
bool radio_button_sprite(SpriteName sprite_name, Rectangle dst, int *store, int value);
bool radio_button_rect(Rectangle dst, int *store, int value);

// -----------------------------------------------------------------------
// increment button by reference
bool increment_button_sprite(
    Texture texture, Rectangle src, Rectangle dst, int *value, int speed, int min, int max
);
bool increment_button_sprite(
    SpriteName sprite_name, Rectangle dst, int *value, int speed, int min, int max
);
bool increment_button_rect(Rectangle dst, int *value, int speed, int min, int max);

// -----------------------------------------------------------------------
// return increment value
int increment_button_sprite(Texture texture, Rectangle src, Rectangle dst, int speed);
int increment_button_sprite(SpriteName sprite_name, Rectangle dst, int speed);
int increment_button_rect(Rectangle dst, int speed);

}  // namespace ui
}  // namespace st
