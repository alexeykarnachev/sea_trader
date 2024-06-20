#include "renderer.hpp"

#include "camera.hpp"
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include "resources.hpp"

namespace st {
namespace renderer {

int SCREEN_WIDTH = 1500;
int SCREEN_HEIGHT = 1000;

void load() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sea Trader");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
}

void unload() {
    CloseWindow();
}

void set_camera(Vector2 position, float view_width, Shader shader) {
    rlDrawRenderBatchActive();

    int position_loc = GetShaderLocation(shader, "camera.position");
    int view_width_loc = GetShaderLocation(shader, "camera.view_width");
    int aspect_loc = GetShaderLocation(shader, "camera.aspect");

    float aspect = (float)GetScreenWidth() / GetScreenHeight();

    SetShaderValue(shader, position_loc, &position, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, view_width_loc, &view_width, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, aspect_loc, &aspect, SHADER_UNIFORM_FLOAT);
}

void set_game_camera(Shader shader) {
    Vector2 position = camera::get_position();
    float view_width = camera::get_view_width();
    set_camera(position, view_width, shader);
}

void set_screen_camera(Shader shader) {
    float view_width = GetScreenWidth();
    Vector2 position = {view_width / 2.0f, GetScreenHeight() / 2.0f};
    set_camera(position, view_width, shader);
}

void draw_product_icon(int product_idx, Rectangle dst) {
    float x = product_idx * 64.0;
    Rectangle src = {.x = x, .y = 0.0, .width = 64.0, .height = 64.0};
    DrawTexturePro(resources::PRODUCT_ICONS_TEXTURE, src, dst, {0.0, 0.0}, 0.0, WHITE);
}

}  // namespace renderer
}  // namespace st
