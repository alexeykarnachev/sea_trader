#include "game.hpp"

#include "camera.hpp"
#include "components.hpp"
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

static const float DT = 1.0 / 60.0;
static bool WINDOW_SHOULD_CLOSE = false;

entt::entity create_ship(
    components::Transform transform, dynamic_body::DynamicBody dynamic_body
) {
    auto entity = registry::registry.create();
    registry::registry.emplace<components::Transform>(entity, transform);
    registry::registry.emplace<dynamic_body::DynamicBody>(entity, dynamic_body);
    registry::registry.emplace<components::Ship>(entity);

    return entity;
}

entt::entity create_player_ship(
    components::Transform transform, dynamic_body::DynamicBody dynamic_body
) {
    auto entity = create_ship(transform, dynamic_body);
    registry::registry.emplace<components::Player>(entity);

    return entity;
}

entt::entity create_port(components::Transform transform, components::Port port) {
    auto entity = registry::registry.create();
    registry::registry.emplace<components::Transform>(entity, transform);
    registry::registry.emplace<components::Port>(entity, port);

    return entity;
}

void update_player_ship_movement() {
    static float torque = 30.0;
    static float force = 4000.0;

    auto entity = registry::registry.view<components::Player>().front();
    auto &body = registry::registry.get<dynamic_body::DynamicBody>(entity);

    auto transform = registry::registry.get<components::Transform>(entity);
    float rotation = transform.rotation;
    Vector2 forward = {cosf(rotation), sinf(rotation)};

    if (IsKeyDown(KEY_A)) body.apply_torque(-torque);
    if (IsKeyDown(KEY_D)) body.apply_torque(torque);

    if (IsKeyDown(KEY_W)) body.apply_force(forward, force);
    if (IsKeyDown(KEY_S)) body.apply_force(forward, -force);
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
    auto view = registry::registry.view<components::Transform, dynamic_body::DynamicBody>(
    );
    for (auto entity : view) {
        auto [transform, body] = view.get(entity);

        // update linear velocity
        Vector2 damping_force = Vector2Scale(body.linear_velocity, -body.linear_damping);
        Vector2 net_force = Vector2Add(body.net_force, damping_force);
        Vector2 linear_acceleration = Vector2Scale(net_force, 1.0f / body.mass);
        body.linear_velocity = Vector2Add(
            body.linear_velocity, Vector2Scale(linear_acceleration, DT)
        );
        body.net_force = {0.0, 0.0};

        // update angular velocity
        float damping_torque = body.angular_velocity * -body.angular_damping;
        float net_torque = body.net_torque + damping_torque;
        float angular_acceleration = net_torque / body.moment_of_inertia;
        body.angular_velocity += angular_acceleration * DT;
        body.net_torque = 0.0;

        // apply linear velocity
        Vector2 linear_step = Vector2Scale(body.linear_velocity, DT);
        Vector2 position = Vector2Add(transform.position, linear_step);
        if (Vector2Length(body.linear_velocity) < EPSILON) {
            body.linear_velocity = {0.0, 0.0};
        }

        if (terrain::check_if_water(position)) {
            transform.position = position;
        } else {
            body.linear_velocity = {0.0, 0.0};
        }

        // apply angular velocity
        float angular_step = body.angular_velocity * DT;
        transform.rotation = transform.rotation + angular_step;
        if (fabs(body.angular_velocity) < EPSILON) {
            body.angular_velocity = 0.0;
        }
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
        components::Transform transform(position, 0.0);
        auto body = dynamic_body::create_ship_preset();
        create_player_ship(transform, body);
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
                    components::Port port(3.0);
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
