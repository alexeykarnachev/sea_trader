#pragma once

#include "camera.hpp"

namespace rl {
#include "raylib/raylib.h"
}  // namespace rl

namespace st {
namespace renderer {

void load();
void unload();
void set_camera(const camera::Camera &camera, rl::Shader shader);

void set_screen_camera(rl::Shader shader);

void draw_product_icon(int product_idx, rl::Rectangle dst);
}  // namespace renderer
}  // namespace st
