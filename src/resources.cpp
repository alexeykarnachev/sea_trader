#include "resources.hpp"

#include "raylib/raylib.h"

namespace st {
namespace resources {

Texture PRODUCT_ICONS_TEXTURE;

Shader TERRAIN_SHADER;
Shader SPRITE_SHADER;

void load() {
    TERRAIN_SHADER = LoadShader(
        "./resources/shaders/base.vert", "./resources/shaders/terrain.frag"
    );
    SPRITE_SHADER = LoadShader(
        "./resources/shaders/base.vert", "./resources/shaders/sprite.frag"
    );

    PRODUCT_ICONS_TEXTURE = LoadTexture("./resources/sprites/product_icons_64.png");
    SetTextureFilter(PRODUCT_ICONS_TEXTURE, TEXTURE_FILTER_BILINEAR);
}

void unload() {
    UnloadTexture(PRODUCT_ICONS_TEXTURE);
    UnloadShader(TERRAIN_SHADER);
    UnloadShader(SPRITE_SHADER);
}

}  // namespace resources

}  // namespace st
