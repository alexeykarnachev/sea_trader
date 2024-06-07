#include "camera.hpp"

namespace rl {
#include "raylib/raylib.h"
}  // namespace rl

namespace st {
namespace camera {

Camera::Camera(float view_width, rl::Vector2 position)
    : view_width(view_width)
    , position(position)
    , target(position) {}

void Camera::reset_position(rl::Vector2 position) {
    this->position = position;
    this->target = position;
}

}  // namespace camera
}  // namespace st
