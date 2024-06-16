#include "game.hpp"

#include "camera.hpp"
#include "cargo.hpp"
#include "components.hpp"
#include "constants.hpp"
#include "dynamic_body.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "registry.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "shop.hpp"
#include "terrain.hpp"
#include "ui.hpp"
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace st {
namespace game {

static bool WINDOW_SHOULD_CLOSE = false;

entt::entity create_ship(Vector2 position) {
    auto entity = registry::registry.create();

    components::Transform transform(position, 0.0);
    components::Money money(1000);

    auto ship = cargo::create_ship_preset();
    auto body = dynamic_body::create_ship_preset(entity);

    registry::registry.emplace<components::Transform>(entity, transform);
    registry::registry.emplace<dynamic_body::DynamicBody>(entity, body);
    registry::registry.emplace<components::Ship>(entity, ship);
    registry::registry.emplace<components::Money>(entity, money);

    return entity;
}

entt::entity create_player(Vector2 position) {
    auto entity = create_ship(position);
    registry::registry.emplace<components::Player>(entity);

    return entity;
}

entt::entity create_port(components::Transform transform, components::Port port) {
    auto entity = registry::registry.create();
    registry::registry.emplace<components::Transform>(entity, transform);
    registry::registry.emplace<components::Port>(entity, port);
    registry::registry.emplace<components::Money>(entity, 500000);

    return entity;
}

void update_player_ship_movement() {
    auto entity = registry::registry.view<components::Player>().front();
    auto &body = registry::registry.get<dynamic_body::DynamicBody>(entity);
    auto ship = registry::registry.get<components::Ship>(entity);

    auto transform = registry::registry.get<components::Transform>(entity);
    Vector2 forward = transform.get_forward();

    if (IsKeyDown(KEY_A)) body.apply_torque(-ship.torque);
    if (IsKeyDown(KEY_D)) body.apply_torque(ship.torque);

    if (IsKeyDown(KEY_W)) body.apply_force(forward, ship.force);
    if (IsKeyDown(KEY_S)) body.apply_force(forward, -ship.force);
}

void update_player_entering_port() {
    bool is_enter_pressed = IsKeyPressed(KEY_ENTER);
    if (!is_enter_pressed) return;

    auto player_entity = registry::registry.view<components::Player>().front();
    auto &player_transform = registry::registry.get<components::Transform>(player_entity);

    auto view = registry::registry.view<components::Transform, components::Port>();
    for (auto port_entity : view) {
        auto [port_transform, port] = view.get(port_entity);
        float dist = Vector2Distance(player_transform.position, port_transform.position);
        if (dist <= port.radius) {
            shop::open(port_entity);
            return;
        }
    }
}

void update_dynamic_bodies() {
    auto view = registry::registry.view<dynamic_body::DynamicBody>();
    for (auto entity : view) {
        auto &body = registry::registry.get<dynamic_body::DynamicBody>(entity);
        body.update();
    }
}

void update_window_should_close() {
    bool is_alt_f4_pressed = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4);
    WINDOW_SHOULD_CLOSE = (WindowShouldClose() || is_alt_f4_pressed);
}

void update() {
    if (!shop::check_if_opened()) {
        camera::update();
        update_player_ship_movement();
        update_player_entering_port();
        update_dynamic_bodies();
    }

    update_window_should_close();
}

void draw_ships() {
    static float height = 0.5;
    static float width = 1.0;

    Shader shader = resources::SPRITE_SHADER;
    renderer::set_game_camera(shader);
    BeginShaderMode(shader);

    auto view = registry::registry.view<components::Transform, components::Ship>();
    for (auto entity : view) {
        auto [transform, ship] = view.get(entity);

        Vector2 origin = {0.5f * width, 0.5f * height};
        Rectangle rect = {
            .x = transform.position.x,
            .y = transform.position.y,
            .width = width,
            .height = height
        };
        DrawRectanglePro(rect, origin, RAD2DEG * transform.rotation, DARKBROWN);
    }

    EndShaderMode();
}

void draw_ports() {
    static float radius = 0.8;

    Shader shader = resources::SPRITE_SHADER;
    renderer::set_game_camera(shader);
    BeginShaderMode(shader);

    auto view = registry::registry.view<components::Transform, components::Port>();
    for (auto entity : view) {
        auto [transform, port] = view.get(entity);
        DrawCircleV(transform.position, radius, RED);
        DrawRing(
            transform.position, port.radius, port.radius + 0.15, 0.0, 360.0, 32, RED
        );
    }
}

void draw() {
    BeginDrawing();
    ClearBackground(BLACK);
    ui::begin();

    terrain::draw();
    draw_ports();
    draw_ships();

    shop::update_and_draw();

    EndDrawing();
}

void load() {
    renderer::load();
    resources::load();
    terrain::load();
    ui::load();

    Vector2 terrain_center = terrain::get_world_center();
    camera::set_target(terrain_center);

    // ---------------------------------------------------------------
    // create player
    {
        Vector2 position = terrain_center;
        position.x -= 8.0;
        position.y -= 10.0;
        create_player(position);
    }

    // ---------------------------------------------------------------
    // create ports
    {
        float min_d = 2.0;
        float max_d = 5.0;
        static const int size = 40;

        int n = 0;
        Vector2 candidates[size * size];

        // iterate on quadrants
        int terrain_size = terrain::get_world_size();
        for (float y0 = 0.0; y0 < terrain_size; y0 += size) {
            for (float x0 = 0.0; x0 < terrain_size; x0 += size) {
                n = 0;

                // find all candidate positions in the quadrant
                for (float y = y0; y < y0 + size; y += 1.0) {
                    for (float x = x0; x < x0 + size; x += 1.0) {
                        Vector2 position = {x, y};
                        float d = terrain::get_dist_to_water(position);
                        if (d >= min_d && d <= max_d) {
                            candidates[n++] = position;
                        }
                    }
                }

                // pick one candidate position from the quadrant
                if (n > 0) {
                    int idx = std::rand() % n;
                    Vector2 position = candidates[idx];
                    components::Transform transform(position, 0.0);
                    components::Port port(3.0, cargo::create_port_preset());
                    create_port(transform, port);
                }
            }
        }
    }
}

void unload() {
    ui::unload();
    terrain::unload();
    resources::unload();
    renderer::unload();
}

void run() {
    load();

    float last_update_time = 0.0;
    while (!WINDOW_SHOULD_CLOSE) {
        float time = GetTime();

        while (time - last_update_time >= DT) {
            update();
            last_update_time += DT;
        }

        draw();
    }

    unload();
}
}  // namespace game
}  // namespace st
