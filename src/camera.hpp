#pragma once

#include "raylib/raylib.h"

namespace st {
namespace camera {

void set_target(Vector2 pos);
Vector2 get_position();
float get_view_width();

void update();

}  // namespace camera
}  // namespace st
