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

    Ship(cargo::Cargo);
};

class Money {
public:
    int value;

    Money(int value);
};

class Player {};

}  // namespace components
}  // namespace st
