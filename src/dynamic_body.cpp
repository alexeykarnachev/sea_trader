#include "dynamic_body.hpp"

#include "components.hpp"
#include "constants.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "registry.hpp"
#include "terrain.hpp"

namespace st {
namespace dynamic_body {

DynamicBody::DynamicBody(
    entt::entity entity,
    float mass,
    float linear_damping,
    float moment_of_inertia,
    float angular_damping
)
    : entity(entity)
    , mass(mass)
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

void DynamicBody::update() {
    auto &transform = registry::registry.get<components::Transform>(this->entity);

    // update linear velocity
    Vector2 damping_force = Vector2Scale(this->linear_velocity, -this->linear_damping);
    Vector2 net_force = Vector2Add(this->net_force, damping_force);
    Vector2 linear_acceleration = Vector2Scale(net_force, 1.0f / this->mass);
    this->linear_velocity = Vector2Add(
        this->linear_velocity, Vector2Scale(linear_acceleration, DT)
    );
    this->net_force = {0.0, 0.0};

    // update angular velocity
    float damping_torque = this->angular_velocity * -this->angular_damping;
    float net_torque = this->net_torque + damping_torque;
    float angular_acceleration = net_torque / this->moment_of_inertia;
    this->angular_velocity += angular_acceleration * DT;
    this->net_torque = 0.0;

    // apply linear velocity
    Vector2 linear_step = Vector2Scale(this->linear_velocity, DT);
    Vector2 position = Vector2Add(transform.position, linear_step);
    if (Vector2Length(this->linear_velocity) < EPSILON) {
        this->linear_velocity = {0.0, 0.0};
    }

    if (terrain::check_if_water(position)) {
        transform.position = position;
    } else {
        this->linear_velocity = {0.0, 0.0};
    }

    // apply angular velocity
    float angular_step = this->angular_velocity * DT;
    transform.rotation = transform.rotation + angular_step;
    if (fabs(this->angular_velocity) < EPSILON) {
        this->angular_velocity = 0.0;
    }
}

DynamicBody create_ship_preset(entt::entity entity) {
    DynamicBody body(entity, 1000.0, 1000.0, 1.0, 10.0);
    return body;
}

}  // namespace dynamic_body
}  // namespace st
