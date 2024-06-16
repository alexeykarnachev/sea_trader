#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "raylib/raylib.h"

namespace st {
namespace dynamic_body {
class DynamicBody {
public:
    entt::entity entity;

    Vector2 linear_velocity = {0.0, 0.0};
    float angular_velocity = 0.0;
    Vector2 net_force = {0.0, 0.0};
    float net_torque = 0.0;

    float mass;
    float linear_damping;

    float moment_of_inertia;
    float angular_damping;

    DynamicBody(
        entt::entity entity,
        float mass,
        float linear_damping,
        float moment_of_inertia,
        float angular_damping
    );

    void apply_force(Vector2 direction, float magnitude);
    void apply_torque(float magnitude);

    void update();
};

DynamicBody create_ship_preset(entt::entity entity);

}  // namespace dynamic_body
}  // namespace st
