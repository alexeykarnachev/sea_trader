#pragma once

namespace rl {
#include "raylib/raylib.h"
}  // namespace rl

namespace st {
namespace camera {
class Camera {
public:
    float view_width;
    rl::Vector2 position;
    rl::Vector2 target;

    Camera(float view_width, rl::Vector2 position);
    void reset_position(rl::Vector2 position);
};

}  // namespace camera
}  // namespace st
