#pragma once

#include "raylib/raylib.h"

namespace st {
namespace resources {

extern Texture product_icons_texture;

extern Shader terrain_shader;
extern Shader sprite_shader;

void load();
void unload();
}  // namespace resources
}  // namespace st
