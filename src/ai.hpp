#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace st {
namespace ai {

enum class AIType {
    DUMMY,
};

struct AIDataDummy {};

class AI {
private:
    void update_dummy();

public:
    AIType type;
    union {
        AIDataDummy dummy;
    } data;

    entt::entity entity;

    AI(AIType type, entt::entity entity);

    void update();
};

}  // namespace ai
}  // namespace st
