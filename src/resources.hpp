#pragma once

namespace rl {
#include "raylib/raylib.h"
}

namespace st {
namespace resources {

extern rl::Texture product_icons_texture;

extern rl::Shader terrain_shader;
extern rl::Shader sprite_shader;

void load();
void unload();
}  // namespace resources
}  // namespace st
