#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "stb/stb_perlin.h"
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <utility>

namespace rl {
#include "raylib/raylib.h"
#include "raylib/raymath.h"
}  // namespace rl

// -----------------------------------------------------------------------
#include "camera.hpp"
#include "cargo.hpp"
#include "registry.hpp"
#include "shop.hpp"
#include "ui.hpp"

namespace st {
class Transform {
public:
    rl::Vector2 position;
    float rotation;

    Transform(rl::Vector2 position, float rotation = 0.0)
        : position(position)
        , rotation(rotation) {}
};

class DynamicBody {
public:
    rl::Vector2 linear_velocity = {0.0, 0.0};
    float angular_velocity = 0.0;
    rl::Vector2 net_force = {0.0, 0.0};
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

    void apply_force(rl::Vector2 direction, float magnitude) {
        direction = rl::Vector2Normalize(direction);
        rl::Vector2 force = rl::Vector2Scale(direction, magnitude);
        this->net_force = rl::Vector2Add(this->net_force, force);
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

class Terrain {
private:
    float *heights;
    rl::Texture heights_texture;

    float *dists_to_water;

    void init_heights() {
        this->heights_texture.id = 0;

        float offset_x = 0.0;
        float offset_y = 0.0;
        float scale = 2.0;

        float lacunarity = 1.4;
        float gain = 1.0;
        int octaves = 8;

        int size = this->size * this->resolution;
        this->heights = (float *)malloc(size * size * sizeof(float));

        float max_height = -FLT_MAX;
        float min_height = FLT_MAX;
        for (int i = 0; i < size * size; ++i) {
            int y = i / size;
            int x = i % size;

            float nx = (float)(x + offset_x) * (scale / (float)size);
            float ny = (float)(y + offset_y) * (scale / (float)size);
            float height = stb_perlin_fbm_noise3(nx, ny, 0.0, lacunarity, gain, octaves);

            max_height = std::max(max_height, height);
            min_height = std::min(min_height, height);
            this->heights[i] = height;
        }

        for (int i = 0; i < size * size; ++i) {
            float *height = &this->heights[i];
            *height = (*height - min_height) / (max_height - min_height);
        }
    }

    void init_dists_to_water() {
        int size = this->size * this->resolution;
        this->dists_to_water = (float *)malloc(size * size * sizeof(float));
        std::fill(this->dists_to_water, this->dists_to_water + size * size, -1.0);

        std::queue<std::pair<int, float>> queue;
        for (int i = 0; i < size * size; ++i) {
            float height = this->heights[i];
            if (this->is_water(height)) {
                this->dists_to_water[i] = 0.0;
                queue.push({i, 0.0});
            }
        }

        std::pair<int, int> directions[8] = {
            {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}
        };

        while (!queue.empty()) {
            auto [i0, d0] = queue.front();
            int y0 = i0 / size;
            int x0 = i0 % size;

            queue.pop();

            for (auto [dx, dy] : directions) {
                int x1 = x0 + dx;
                int y1 = y0 + dy;
                int i1 = y1 * size + x1;

                if (x1 >= 0 && x1 < size && y1 >= 0 && y1 < size) {
                    if (this->dists_to_water[i1] >= 0.0) continue;

                    float height = this->heights[i1];
                    float d1 = 0.0;
                    if (!this->is_water(height)) {
                        float dd = dx == 0 || dy == 0 ? 1.0 : 1.4142;
                        d1 = d0 + dd;
                    }
                    this->dists_to_water[i1] = d1;
                    queue.push({i1, d1});
                }
            }
        }
    }

    int world_to_data_idx(rl::Vector2 position) {
        int size = this->size * this->resolution;
        int x = position.x * this->resolution;
        int y = position.y * this->resolution;
        int idx = y * size + x;

        return idx;
    }

public:
    const int size;
    const float resolution;
    const float water_level;

    Terrain(int size, float resolution, float water_level)
        : size(size)
        , resolution(resolution)
        , water_level(water_level) {

        this->init_heights();
        this->init_dists_to_water();
    }

    rl::Vector2 get_center() {
        return {this->size / 2.0f, this->size / 2.0f};
    }

    rl::Rectangle get_rect() {
        return {
            .x = 0.0, .y = 0.0, .width = (float)this->size, .height = (float)this->size
        };
    }

    rl::Texture get_heights_texture() {
        int size = this->size * this->resolution;
        if (this->heights_texture.id == 0) {
            rl::Image image;
            image.data = this->heights;
            image.width = size;
            image.height = size;
            image.mipmaps = 1;
            image.format = rl::PIXELFORMAT_UNCOMPRESSED_R32;

            this->heights_texture = LoadTextureFromImage(image);
            // SetTextureFilter(this->heights_texture, TEXTURE_FILTER_BILINEAR);
        }
        return this->heights_texture;
    }

    float get_height(rl::Vector2 position) {
        int idx = this->world_to_data_idx(position);
        return this->heights[idx];
    }

    float get_dist_to_water(rl::Vector2 position) {
        int idx = this->world_to_data_idx(position);
        return this->dists_to_water[idx];
    }

    bool is_water(rl::Vector2 position) {
        float height = this->get_height(position);
        return this->is_water(height);
    }

    bool is_water(float height) {
        return height <= this->water_level;
    }
};

class Game {
private:
    float dt = 1.0 / 60.0;

    Terrain terrain;
    camera::Camera camera;

    bool window_should_close = false;

public:
    Game()
        : terrain(200, 4.0, 0.6)
        , camera(50.0, terrain.get_center()) {

        renderer::load();
        resources::load();
        ui::load();

        // ---------------------------------------------------------------
        // create player
        rl::Vector2 terrain_center = this->terrain.get_center();

        {
            rl::Vector2 position = terrain_center;
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
            rl::Vector2 candidates[size * size];

            // iterate on quadrants
            for (float y0 = 0.0; y0 < terrain.size; y0 += size) {
                for (float x0 = 0.0; x0 < terrain.size; x0 += size) {
                    n = 0;

                    // find all candidate positions in the quadrant
                    for (float y = y0; y < y0 + size; y += 1.0) {
                        for (float x = x0; x < x0 + size; x += 1.0) {
                            rl::Vector2 position = {x, y};
                            float d = this->terrain.get_dist_to_water(position);
                            if (d >= min_d && d <= max_d) {
                                candidates[n++] = position;
                            }
                        }
                    }

                    // pick one candidate position from the quadrant
                    if (n > 0) {
                        int idx = std::rand() % n;
                        rl::Vector2 position = candidates[idx];
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
        resources::unload();
        renderer::unload();
    }

    void run() {
        static float last_update_time = 0.0;

        while (!this->window_should_close) {
            float time = rl::GetTime();

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

    void update_camera() {
        static float min_view_width = 10.0f;
        static float max_view_width = 500.0f;
        static float zoom_speed = 8.0f;

        if (IsMouseButtonDown(rl::MOUSE_MIDDLE_BUTTON)) {
            rl::Vector2 delta = rl::GetMouseDelta();
            rl::Vector2 curr = rl::GetMousePosition();
            rl::Vector2 prev = Vector2Add(curr, delta);

            curr = this->get_screen_to_world(curr);
            prev = this->get_screen_to_world(prev);
            delta = Vector2Subtract(curr, prev);

            rl::Vector2 position = Vector2Add(this->camera.position, delta);
            this->camera.reset_position(position);
        }

        float wheel_move = rl::GetMouseWheelMove();
        float zoom = 0.0;
        if (wheel_move > 0.0) {
            zoom = -zoom_speed;
        } else if (wheel_move < 0.0) {
            zoom = zoom_speed;
        }
        this->camera.view_width += zoom;
        this->camera.view_width = std::max(this->camera.view_width, min_view_width);
        this->camera.view_width = std::min(this->camera.view_width, max_view_width);
    }

    void update_player_ship_movement() {
        static float torque = 30.0;
        static float force = 4000.0;

        auto entity = registry::registry.view<Player>().front();
        auto &body = registry::registry.get<DynamicBody>(entity);

        auto transform = registry::registry.get<Transform>(entity);
        float rotation = transform.rotation;
        rl::Vector2 forward = {cosf(rotation), sinf(rotation)};

        if (rl::IsKeyDown(rl::KEY_A)) body.apply_torque(-torque);
        if (rl::IsKeyDown(rl::KEY_D)) body.apply_torque(torque);

        if (rl::IsKeyDown(rl::KEY_W)) body.apply_force(forward, force);
        if (rl::IsKeyDown(rl::KEY_S)) body.apply_force(forward, -force);
    }

    void update_player_entering_port() {
        bool is_enter_pressed = rl::IsKeyPressed(rl::KEY_ENTER);
        if (!is_enter_pressed) return;

        auto player_entity = registry::registry.view<Player>().front();
        auto &player_transform = registry::registry.get<Transform>(player_entity);

        auto view = registry::registry.view<Transform, Port>();
        for (auto port_entity : view) {
            auto [port_transform, port] = view.get(port_entity);
            float dist = rl::Vector2Distance(
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
            rl::Vector2 damping_force = rl::Vector2Scale(
                body.linear_velocity, -body.linear_damping
            );
            rl::Vector2 net_force = rl::Vector2Add(body.net_force, damping_force);
            rl::Vector2 linear_acceleration = rl::Vector2Scale(
                net_force, 1.0f / body.mass
            );
            body.linear_velocity = rl::Vector2Add(
                body.linear_velocity, rl::Vector2Scale(linear_acceleration, this->dt)
            );
            body.net_force = {0.0, 0.0};

            // update angular velocity
            float damping_torque = body.angular_velocity * -body.angular_damping;
            float net_torque = body.net_torque + damping_torque;
            float angular_acceleration = net_torque / body.moment_of_inertia;
            body.angular_velocity += angular_acceleration * dt;
            body.net_torque = 0.0;

            // apply linear velocity
            rl::Vector2 linear_step = rl::Vector2Scale(body.linear_velocity, dt);
            rl::Vector2 position = rl::Vector2Add(transform.position, linear_step);
            if (rl::Vector2Length(body.linear_velocity) < EPSILON) {
                body.linear_velocity = {0.0, 0.0};
            }

            if (this->terrain.is_water(position)) {
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
        bool is_alt_f4_pressed = IsKeyDown(rl::KEY_LEFT_ALT) && IsKeyPressed(rl::KEY_F4);
        bool is_escape_pressed = rl::IsKeyPressed(rl::KEY_ESCAPE);
        this->window_should_close = (rl::WindowShouldClose() || is_alt_f4_pressed)
                                    && !is_escape_pressed;
    }

    void update() {
        if (!shop::get_is_opened()) {
            this->update_camera();
            this->update_player_ship_movement();
            this->update_player_entering_port();
            this->update_dynamic_bodies();
        }

        this->update_window_should_close();
    }

    rl::Vector2 get_screen_to_world(rl::Vector2 p) {
        int screen_width = rl::GetScreenWidth();
        int screen_height = rl::GetScreenHeight();

        p.x /= screen_width;
        p.y /= screen_height;
        float aspect = (float)screen_width / screen_height;
        float view_height = camera.view_width / aspect;
        rl::Vector2 center = camera.target;
        float left = center.x - 0.5 * camera.view_width;
        float top = center.y - 0.5 * view_height;
        float x = left + camera.view_width * p.x;
        float y = top + view_height * p.y;

        return {x, y};
    }

    void draw_terrain() {
        rl::Shader shader = resources::terrain_shader;
        rl::Texture texture = this->terrain.get_heights_texture();
        renderer::set_camera(this->camera, shader);

        int water_level_loc = GetShaderLocation(shader, "water_level");
        SetShaderValue(
            shader, water_level_loc, &this->terrain.water_level, rl::SHADER_UNIFORM_FLOAT
        );

        BeginShaderMode(shader);
        rl::Rectangle src = {
            .x = 0.0,
            .y = 0.0,
            .width = (float)texture.width,
            .height = (float)texture.height
        };
        rl::Rectangle dst = this->terrain.get_rect();
        DrawTexturePro(texture, src, dst, {0, 0}, 0, rl::WHITE);

        rl::EndShaderMode();
    }

    void draw_ships() {
        static float height = 0.5;
        static float width = 1.0;

        rl::Shader shader = resources::sprite_shader;
        renderer::set_camera(this->camera, shader);
        BeginShaderMode(shader);

        auto view = registry::registry.view<Transform, Ship>();
        for (auto entity : view) {
            auto [transform, ship] = view.get(entity);

            rl::Vector2 origin = {0.5f * width, 0.5f * height};
            rl::Rectangle rect = {
                .x = transform.position.x,
                .y = transform.position.y,
                .width = width,
                .height = height
            };
            DrawRectanglePro(rect, origin, RAD2DEG * transform.rotation, rl::DARKBROWN);
        }

        rl::EndShaderMode();
    }

    void draw_ports() {
        static float radius = 0.8;

        rl::Shader shader = resources::sprite_shader;
        renderer::set_camera(this->camera, shader);
        BeginShaderMode(shader);

        auto view = registry::registry.view<Transform, Port>();
        for (auto entity : view) {
            auto [transform, port] = view.get(entity);
            rl::DrawCircleV(transform.position, radius, rl::RED);
            rl::DrawRing(
                transform.position,
                port.radius,
                port.radius + 0.15,
                0.0,
                360.0,
                32,
                rl::RED
            );
        }
    }

    void draw() {
        rl::BeginDrawing();
        ClearBackground(rl::BLACK);
        ui::begin();

        this->draw_terrain();
        this->draw_ports();
        this->draw_ships();

        shop::update_and_draw();

        rl::EndDrawing();
    }
};
}  // namespace st

int main() {
    auto game = st::Game();
    game.run();
}
