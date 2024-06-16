#pragma once

#include "raylib/raylib.h"

namespace st {
namespace terrain {

void load();
void unload();

int get_world_size();
Vector2 get_world_center();
Rectangle get_world_rect();
float get_height(Vector2 pos);
float get_dist_to_water(Vector2 pos);

bool check_if_water(float h);
bool check_if_water(Vector2 pos);

void draw();

}  // namespace terrain

}  // namespace st
