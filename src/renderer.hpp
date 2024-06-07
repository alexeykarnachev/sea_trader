#pragma once

#include "camera.hpp"
namespace rl {
#include "raylib/raylib.h"
}  // namespace rl

namespace st {
namespace renderer {

extern rl::Shader terrain_shader;
extern rl::Shader sprite_shader;

void load();
void unload();
void set_camera(const camera::Camera &camera, rl::Shader shader);

void set_screen_camera(rl::Shader shader);

}  // namespace renderer
}  // namespace st
