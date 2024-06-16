#pragma once

#include "cargo.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace st {
namespace ship {

enum class ControllerType {
    MANUAL,
    DUMMY,
};

class Ship {
private:
    void move(bool is_forward);
    void rotate(bool is_right);

    void update_controller_manual();
    void update_controller_dummy();

public:
    entt::entity entity;
    ControllerType controller_type;

    cargo::Cargo cargo;
    float torque = 30.0;
    float force = 4000.0;

    Ship(entt::entity entity, ControllerType controller_type, cargo::Cargo);

    void update();
};

}  // namespace ship
}  // namespace st
