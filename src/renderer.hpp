#pragma once

#include "raylib/raylib.h"

namespace st {
namespace renderer {

void load();
void unload();

void set_game_camera(Shader shader);
void set_screen_camera(Shader shader);

void draw_product_icon(int product_idx, Rectangle dst);

}  // namespace renderer
}  // namespace st
