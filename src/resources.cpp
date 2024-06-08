#include "resources.hpp"

namespace rl {
#include "raylib/raylib.h"
}

namespace st {
namespace resources {

rl::Texture product_icons_texture;

rl::Shader terrain_shader;
rl::Shader sprite_shader;

void load() {
    terrain_shader = rl::LoadShader(
        "./resources/shaders/base.vert", "./resources/shaders/terrain.frag"
    );
    sprite_shader = rl::LoadShader(
        "./resources/shaders/base.vert", "./resources/shaders/sprite.frag"
    );

    product_icons_texture = rl::LoadTexture("./resources/sprites/product_icons_64.png");
    SetTextureFilter(product_icons_texture, rl::TEXTURE_FILTER_BILINEAR);
}

void unload() {
    rl::UnloadTexture(product_icons_texture);
    rl::UnloadShader(terrain_shader);
    rl::UnloadShader(sprite_shader);
}

}  // namespace resources

}  // namespace st
