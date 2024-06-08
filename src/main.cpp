#include "camera.hpp"
#include "cargo.hpp"
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
class Transform {
public:
    Vector2 position;
    float rotation;

    Transform(Vector2 position, float rotation = 0.0)
        : position(position)
        , rotation(rotation) {}
};

class DynamicBody {
public:
    Vector2 linear_velocity = {0.0, 0.0};
    float angular_velocity = 0.0;
    Vector2 net_force = {0.0, 0.0};
    float net_torque = 0.0;

    float mass;
    float linear_damping;

    float moment_of_inertia;
    float angular_damping;

    DynamicBody(
        float mass, float linear_damping, float moment_of_inertia, float angular_damping
    )
        : mass(mass)
        , linear_damping(linear_damping)
        , moment_of_inertia(moment_of_inertia)
        , angular_damping(angular_damping) {}

    void apply_force(Vector2 direction, float magnitude) {
        direction = Vector2Normalize(direction);
        Vector2 force = Vector2Scale(direction, magnitude);
        this->net_force = Vector2Add(this->net_force, force);
    }

    void apply_torque(float magnitude) {
        this->net_torque += magnitude * 1.0;
    }

    static DynamicBody create_ship() {
        DynamicBody body(1000.0, 1000.0, 1.0, 10.0);
        return body;
    }
};

class Port {
public:
    const float radius;
    cargo::Cargo cargo;

    Port(float radius)
        : radius(radius) {}
};

class Ship {
public:
    cargo::Cargo cargo;
};

class Player {};

class Game {
private:
    float dt = 1.0 / 60.0;
    bool window_should_close = false;

public:
    Game() {

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
            Transform transform(position);
            auto body = DynamicBody::create_ship();
            this->create_player_ship(transform, body);
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
                        Transform transform(position);
                        Port port(3.0);
                        this->create_port(transform, port);
                    }
                }
            }
        }
    }

    ~Game() {
        ui::unload();
        terrain::unload();
        resources::unload();
        renderer::unload();
    }

    void run() {
        static float last_update_time = 0.0;

        while (!this->window_should_close) {
            float time = GetTime();

            while (time - last_update_time >= this->dt) {
                this->update();
                last_update_time += this->dt;
            }

            this->draw();
        }
    }

private:
    entt::entity create_ship(Transform transform, DynamicBody dynamic_body) {
        auto entity = registry::registry.create();
        registry::registry.emplace<Transform>(entity, transform);
        registry::registry.emplace<DynamicBody>(entity, dynamic_body);
        registry::registry.emplace<Ship>(entity);

        return entity;
    }

    entt::entity create_player_ship(Transform transform, DynamicBody dynamic_body) {
        auto entity = this->create_ship(transform, dynamic_body);
        registry::registry.emplace<Player>(entity);

        return entity;
    }

    entt::entity create_port(Transform transform, Port port) {
        auto entity = registry::registry.create();
        registry::registry.emplace<Transform>(entity, transform);
        registry::registry.emplace<Port>(entity, port);

        return entity;
    }

    void update_player_ship_movement() {
        static float torque = 30.0;
        static float force = 4000.0;

        auto entity = registry::registry.view<Player>().front();
        auto &body = registry::registry.get<DynamicBody>(entity);

        auto transform = registry::registry.get<Transform>(entity);
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

        auto player_entity = registry::registry.view<Player>().front();
        auto &player_transform = registry::registry.get<Transform>(player_entity);

        auto view = registry::registry.view<Transform, Port>();
        for (auto port_entity : view) {
            auto [port_transform, port] = view.get(port_entity);
            float dist = Vector2Distance(
                player_transform.position, port_transform.position
            );
            if (dist <= port.radius) {
                shop::open();
                return;
            }
        }
    }

    void update_dynamic_bodies() {
        auto view = registry::registry.view<Transform, DynamicBody>();
        for (auto entity : view) {
            auto [transform, body] = view.get(entity);

            // update linear velocity
            Vector2 damping_force = Vector2Scale(
                body.linear_velocity, -body.linear_damping
            );
            Vector2 net_force = Vector2Add(body.net_force, damping_force);
            Vector2 linear_acceleration = Vector2Scale(net_force, 1.0f / body.mass);
            body.linear_velocity = Vector2Add(
                body.linear_velocity, Vector2Scale(linear_acceleration, this->dt)
            );
            body.net_force = {0.0, 0.0};

            // update angular velocity
            float damping_torque = body.angular_velocity * -body.angular_damping;
            float net_torque = body.net_torque + damping_torque;
            float angular_acceleration = net_torque / body.moment_of_inertia;
            body.angular_velocity += angular_acceleration * dt;
            body.net_torque = 0.0;

            // apply linear velocity
            Vector2 linear_step = Vector2Scale(body.linear_velocity, dt);
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
            float angular_step = body.angular_velocity * dt;
            transform.rotation = transform.rotation + angular_step;
            if (fabs(body.angular_velocity) < EPSILON) {
                body.angular_velocity = 0.0;
            }
        }
    }

    void update_window_should_close() {
        bool is_alt_f4_pressed = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4);
        bool is_escape_pressed = IsKeyPressed(KEY_ESCAPE);
        this->window_should_close = (WindowShouldClose() || is_alt_f4_pressed)
                                    && !is_escape_pressed;
    }

    void update() {
        if (!shop::check_if_opened()) {
            camera::update();
            this->update_player_ship_movement();
            this->update_player_entering_port();
            this->update_dynamic_bodies();
        }

        this->update_window_should_close();
    }

    void draw_ships() {
        static float height = 0.5;
        static float width = 1.0;

        Shader shader = resources::sprite_shader;
        renderer::set_game_camera(shader);
        BeginShaderMode(shader);

        auto view = registry::registry.view<Transform, Ship>();
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

        Shader shader = resources::sprite_shader;
        renderer::set_game_camera(shader);
        BeginShaderMode(shader);

        auto view = registry::registry.view<Transform, Port>();
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
        this->draw_ports();
        this->draw_ships();

        shop::update_and_draw();

        EndDrawing();
    }
};
}  // namespace st

int main() {
    auto game = st::Game();
    game.run();
}
