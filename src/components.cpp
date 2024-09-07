#include "components.hpp"

#include "cargo.hpp"
#include <cmath>
#include <vector>

namespace st {
namespace components {

Transform::Transform(Vector2 position, float rotation)
    : position(position)
    , rotation(rotation) {}

Vector2 Transform::get_forward() {
    return {cosf(this->rotation), sinf(this->rotation)};
}

Path::Path(std::vector<Vector2> points)
    : points(points) {}

Port::Port(float radius, cargo::Cargo cargo)
    : radius(radius)
    , cargo(cargo) {}

Money::Money() = default;

Money::Money(int value)
    : value(value) {}

}  // namespace components
}  // namespace st
