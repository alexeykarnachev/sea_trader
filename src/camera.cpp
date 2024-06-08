#include "camera.hpp"

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include <algorithm>

namespace st {
namespace camera {

static float VIEW_WIDTH = 50.0;
static Vector2 TARGET = {0.0, 0.0};

static float MIN_VIEW_WIDTH = 10.0f;
static float MAX_VIEW_WIDTH = 500.0f;
static float ZOOM_SPEED = 8.0f;

void set_target(Vector2 pos) {
    TARGET = pos;
}

Vector2 get_position() {
    return TARGET;
}

float get_view_width() {
    return VIEW_WIDTH;
}

Vector2 get_screen_to_world(Vector2 p) {
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();

    p.x /= screen_width;
    p.y /= screen_height;
    float aspect = (float)screen_width / screen_height;
    float view_height = VIEW_WIDTH / aspect;
    Vector2 center = get_position();
    float left = center.x - 0.5 * VIEW_WIDTH;
    float top = center.y - 0.5 * view_height;
    float x = left + VIEW_WIDTH * p.x;
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
        zoom = -ZOOM_SPEED;
    } else if (wheel_move < 0.0) {
        zoom = ZOOM_SPEED;
    }
    VIEW_WIDTH += zoom;
    VIEW_WIDTH = std::max(VIEW_WIDTH, MIN_VIEW_WIDTH);
    VIEW_WIDTH = std::min(VIEW_WIDTH, MAX_VIEW_WIDTH);
}

}  // namespace camera
}  // namespace st
