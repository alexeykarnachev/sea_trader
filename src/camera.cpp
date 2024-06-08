#include "camera.hpp"

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include <algorithm>

namespace st {
namespace camera {

static float view_width = 50.0;
static Vector2 target = {0.0, 0.0};

static float min_view_width = 10.0f;
static float max_view_width = 500.0f;
static float zoom_speed = 8.0f;

void set_target(Vector2 pos) {
    target = pos;
}

Vector2 get_position() {
    return target;
}

float get_view_width() {
    return view_width;
}

Vector2 get_screen_to_world(Vector2 p) {
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();

    p.x /= screen_width;
    p.y /= screen_height;
    float aspect = (float)screen_width / screen_height;
    float view_height = view_width / aspect;
    Vector2 center = get_position();
    float left = center.x - 0.5 * view_width;
    float top = center.y - 0.5 * view_height;
    float x = left + view_width * p.x;
    float y = top + view_height * p.y;

    return {x, y};
}

void update() {
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
        Vector2 delta = GetMouseDelta();
        Vector2 curr = GetMousePosition();
        Vector2 prev = Vector2Add(curr, delta);

        curr = get_screen_to_world(curr);
        prev = get_screen_to_world(prev);
        delta = Vector2Subtract(curr, prev);

        Vector2 position = Vector2Add(get_position(), delta);
        set_target(position);
    }

    float wheel_move = GetMouseWheelMove();
    float zoom = 0.0;
    if (wheel_move > 0.0) {
        zoom = -zoom_speed;
    } else if (wheel_move < 0.0) {
        zoom = zoom_speed;
    }
    view_width += zoom;
    view_width = std::max(view_width, min_view_width);
    view_width = std::min(view_width, max_view_width);
}

}  // namespace camera
}  // namespace st
