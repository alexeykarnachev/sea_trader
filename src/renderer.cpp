#include "renderer.hpp"

#include "camera.hpp"
#include "resources.hpp"

namespace rl {
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
}  // namespace rl

namespace st {
namespace renderer {

int screen_width = 1500;
int screen_height = 1000;

void load() {
    SetConfigFlags(rl::FLAG_MSAA_4X_HINT);
    rl::InitWindow(screen_width, screen_height, "Sea Trader");
    rl::SetTargetFPS(60);
}

void unload() {
    rl::CloseWindow();
}

void set_camera(const camera::Camera &camera, rl::Shader shader) {
    rl::rlDrawRenderBatchActive();

    int position_loc = GetShaderLocation(shader, "camera.position");
    int view_width_loc = GetShaderLocation(shader, "camera.view_width");
    int aspect_loc = GetShaderLocation(shader, "camera.aspect");

    float aspect = (float)screen_width / screen_height;

    SetShaderValue(shader, position_loc, &camera.position, rl::SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, view_width_loc, &camera.view_width, rl::SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, aspect_loc, &aspect, rl::SHADER_UNIFORM_FLOAT);
}

void set_screen_camera(rl::Shader shader) {
    rl::Vector2 position = {screen_width / 2.0f, screen_height / 2.0f};
    auto camera = camera::Camera(screen_width, position);
    set_camera(camera, shader);
}

void draw_product_icon(int product_idx, rl::Rectangle dst) {
    float x = product_idx * 64.0;
    rl::Rectangle src = {.x = x, .y = 0.0, .width = 64.0, .height = 64.0};
    rl::DrawTexturePro(
        resources::product_icons_texture, src, dst, {0.0, 0.0}, 0.0, rl::WHITE
    );
}

}  // namespace renderer
}  // namespace st
