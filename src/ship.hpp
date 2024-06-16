#pragma once

#include "cargo.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace st {
namespace ship {

class Ship {
public:
    entt::entity entity;

    cargo::Cargo cargo;
    float torque = 30.0;
    float force = 4000.0;

    Ship(entt::entity entity, cargo::Cargo);

    void move(bool is_forward);
    void rotate(bool is_right);
};

}  // namespace ship
}  // namespace st
