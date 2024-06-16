#pragma once

#include "cargo.hpp"
#include "raylib/raylib.h"

namespace st {
namespace components {

class Transform {
public:
    Vector2 position;
    float rotation;

    Transform(Vector2 position, float rotation);

    Vector2 get_forward();
};

class Port {
public:
    const float radius;
    cargo::Cargo cargo;

    Port(float radius, cargo::Cargo);
};

class Ship {
public:
    cargo::Cargo cargo;
    float torque = 30.0;
    float force = 4000.0;

    Ship(cargo::Cargo);
};

class Money {
public:
    int value = 0;

    Money();
    Money(int value);
};

class Player {};

}  // namespace components
}  // namespace st
