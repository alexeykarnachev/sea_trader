#pragma once

#include "raylib/raylib.h"

namespace st {
namespace camera {

extern float VIEW_WIDTH;
extern Vector2 TARGET;

void set_target(Vector2 pos);
Vector2 get_position();
float get_view_width();

void update();

}  // namespace camera
}  // namespace st
