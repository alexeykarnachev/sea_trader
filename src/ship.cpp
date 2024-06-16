#include "ship.hpp"

#include "cargo.hpp"
#include "components.hpp"
#include "dynamic_body.hpp"
#include "registry.hpp"

namespace st {
namespace ship {

Ship::Ship(entt::entity entity, ControllerType controller_type, cargo::Cargo cargo)
    : entity(entity)
    , controller_type(controller_type)
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

void Ship::update_controller_manual() {
    if (IsKeyDown(KEY_A)) this->rotate(false);
    if (IsKeyDown(KEY_D)) this->rotate(true);

    if (IsKeyDown(KEY_W)) this->move(true);
    if (IsKeyDown(KEY_S)) this->move(false);
}

void Ship::update_controller_dummy() {
    this->rotate(true);
    this->move(true);
}

void Ship::update() {
    switch (this->controller_type) {
        case ControllerType::MANUAL: this->update_controller_manual(); break;
        case ControllerType::DUMMY: this->update_controller_dummy(); break;
    }
}

}  // namespace ship
}  // namespace st
