#include "resources.hpp"

#include "raylib/raylib.h"

namespace st {
namespace resources {

Texture product_icons_texture;

Shader terrain_shader;
Shader sprite_shader;

void load() {
    terrain_shader = LoadShader(
        "./resources/shaders/base.vert", "./resources/shaders/terrain.frag"
    );
    sprite_shader = LoadShader(
        "./resources/shaders/base.vert", "./resources/shaders/sprite.frag"
    );

    product_icons_texture = LoadTexture("./resources/sprites/product_icons_64.png");
    SetTextureFilter(product_icons_texture, TEXTURE_FILTER_BILINEAR);
}

void unload() {
    UnloadTexture(product_icons_texture);
    UnloadShader(terrain_shader);
    UnloadShader(sprite_shader);
}

}  // namespace resources

}  // namespace st
