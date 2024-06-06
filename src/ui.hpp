#pragma once

#include <string>
namespace rl {
#include "raylib/raylib.h"
}  // namespace rl

namespace st {
namespace ui {

enum class SpriteName {
    RIGHT_ARROW_ICON_SRC,
    LEFT_ARROW_ICON_SRC,
};

void load();
void unload();
void begin();

// -----------------------------------------------------------------------
// line
void line(float x0, float y0, float x1, float y1, rl::Color color);
void line_light(float x0, float y0, float x1, float y1);
void line_mild(float x0, float y0, float x1, float y1);
void line_dark(float x0, float y0, float x1, float y1);

// -----------------------------------------------------------------------
// rect
void rect(rl::Rectangle dst, rl::Color color);
void rect_cold(rl::Rectangle dst);
void rect_hover(rl::Rectangle dst);

// -----------------------------------------------------------------------
// sprite
void sprite(rl::Texture texture, rl::Rectangle src, rl::Rectangle dst);
void sprite(SpriteName sprite_name, rl::Rectangle dst);

// -----------------------------------------------------------------------
// text
void text(std::string str, float x, float y, int size, rl::Color color);
void text_light(std::string str, float x, float y, int size);
void text_mild(std::string str, float x, float y, int size);
void text_dark(std::string str, float x, float y, int size);
void text_info(std::string str, float x, float y, int size);
void text_error(std::string str, float x, float y, int size);

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
