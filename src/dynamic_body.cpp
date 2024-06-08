#include "dynamic_body.hpp"

#include "raylib/raylib.h"
#include "raylib/raymath.h"

namespace st {
namespace dynamic_body {

DynamicBody::DynamicBody(
    float mass, float linear_damping, float moment_of_inertia, float angular_damping
)
    : mass(mass)
    , linear_damping(linear_damping)
    , moment_of_inertia(moment_of_inertia)
    , angular_damping(angular_damping) {}

void DynamicBody::apply_force(Vector2 direction, float magnitude) {
    direction = Vector2Normalize(direction);
    Vector2 force = Vector2Scale(direction, magnitude);
    this->net_force = Vector2Add(this->net_force, force);
}

void DynamicBody::apply_torque(float magnitude) {
    this->net_torque += magnitude * 1.0;
}

DynamicBody create_ship_preset() {
    DynamicBody body(1000.0, 1000.0, 1.0, 10.0);
    return body;
}

}  // namespace dynamic_body
}  // namespace st
