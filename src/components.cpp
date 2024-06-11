#include "components.hpp"

#include "cargo.hpp"

namespace st {
namespace components {

Transform::Transform(Vector2 position, float rotation)
    : position(position)
    , rotation(rotation) {}

Port::Port(float radius)
    : radius(radius)
    , cargo(cargo::create_port_preset()) {}

Ship::Ship()
    : cargo(cargo::create_ship_preset()) {}

}  // namespace components
}  // namespace st
