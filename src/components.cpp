#include "components.hpp"

namespace st {
namespace components {

Transform::Transform(Vector2 position, float rotation)
    : position(position)
    , rotation(rotation) {}

Port::Port(float radius)
    : radius(radius) {}

}  // namespace components
}  // namespace st
