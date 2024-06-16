#include "ai.hpp"

#include "dynamic_body.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "registry.hpp"

namespace st {
namespace ai {
entt::registry registry;

AI::AI(AIType type, entt::entity entity)
    : type(type)
    , entity(entity) {}

void AI::update() {
    auto &body = registry::registry.get<dynamic_body::DynamicBody>(this->entity);
}

}  // namespace ai

}  // namespace st
