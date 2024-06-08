#pragma once

#include "raylib/raylib.h"

namespace st {
namespace resources {

extern Texture PRODUCT_ICONS_TEXTURE;

extern Shader TERRAIN_SHADER;
extern Shader SPRITE_SHADER;

void load();
void unload();
}  // namespace resources
}  // namespace st
