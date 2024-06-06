#pragma once

#include <string>
namespace rl {
#include "raylib/raylib.h"
}  // namespace rl

namespace st {
namespace ui {
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
void sprite(rl::Texture, rl::Rectangle src, rl::Rectangle dst);

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
bool button_rect(rl::Rectangle dst);

// -----------------------------------------------------------------------
// radio button
bool radio_button_sprite(rl::Rectangle src, rl::Rectangle dst, int *store, int value);
bool radio_button_rect(rl::Rectangle dst, int *store, int value);

// -----------------------------------------------------------------------
// arrow button
bool button_left_arrow(float x, float y, float size);
bool button_right_arrow(float x, float y, float size);
}  // namespace ui

}  // namespace st
