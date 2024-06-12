#pragma once

#include "entt/entity/fwd.hpp"

namespace st {
namespace shop {

void open(entt::entity port_entity);
bool check_if_opened();
void update_and_draw();

}  // namespace shop
}  // namespace st
