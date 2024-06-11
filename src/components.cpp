#include "components.hpp"

#include "cargo.hpp"

namespace st {
namespace components {

Transform::Transform(Vector2 position, float rotation)
    : position(position)
    , rotation(rotation) {}

Port::Port(float radius)
    : radius(radius)
    , cargo(cargo::Cargo(100000)) {}

Ship::Ship()
    : cargo(cargo::Cargo(1000)) {}

}  // namespace components
}  // namespace st
