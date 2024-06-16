#include "ship.hpp"

#include "cargo.hpp"
#include "components.hpp"
#include "dynamic_body.hpp"
#include "registry.hpp"

namespace st {
namespace ship {

Ship::Ship(entt::entity entity, cargo::Cargo cargo)
    : entity(entity)
    , cargo(cargo) {}

void Ship::move(bool is_forward) {
    auto &body = registry::registry.get<dynamic_body::DynamicBody>(this->entity);
    auto transform = registry::registry.get<components::Transform>(this->entity);

    Vector2 forward = transform.get_forward();
    int sign = is_forward ? 1 : -1;

    body.apply_force(forward, sign * this->force);
}

void Ship::rotate(bool is_right) {
    auto &body = registry::registry.get<dynamic_body::DynamicBody>(this->entity);

    int sign = is_right ? 1 : -1;

    body.apply_torque(sign * this->torque);
}

}  // namespace ship
}  // namespace st
