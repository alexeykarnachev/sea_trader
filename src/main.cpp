#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "stb/stb_perlin.h"
#include <cfloat>
#include <cmath>
#include <cstdio>

namespace rl {
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
}  // namespace rl

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

class Player {};

class Camera {
public:
    float view_width;
    rl::Vector2 target;

    Camera(float view_width, rl::Vector2 target)
        : view_width(view_width)
        , target(target) {}
};

class World {
private:
    int size;

    float heights_resolution;
    float *heights_data;
    rl::Texture heights_texture;

    void init_heights() {
        this->heights_texture.id = 0;

        float offset_x = 0.0;
        float offset_y = 0.0;
        float scale = 2.0;

        float lacunarity = 1.4;
        float gain = 1.0;
        int octaves = 8;

        int size = this->size * this->heights_resolution;
        this->heights_data = (float *)malloc(size * size * sizeof(float));

        float max_h = -FLT_MAX;
        float min_h = FLT_MAX;
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                float nx = (float)(x + offset_x) * (scale / (float)size);
                float ny = (float)(y + offset_y) * (scale / (float)size);
                float h = stb_perlin_fbm_noise3(nx, ny, 0.0, lacunarity, gain, octaves);

                max_h = std::max(max_h, h);
                min_h = std::min(min_h, h);
                this->heights_data[y * size + x] = h;
            }
        }

        for (int i = 0; i < size * size; ++i) {
            float *height = &this->heights_data[i];
            *height = (*height - min_h) / (max_h - min_h);
        }
    }

public:
    const float water_level;

    World(int size, float heights_resolution, float water_level)
        : size(size)
        , heights_resolution(heights_resolution)
        , water_level(water_level) {
        this->init_heights();
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
        int size = this->size * this->heights_resolution;
        if (this->heights_texture.id == 0) {
            rl::Image image;
            image.data = this->heights_data;
            image.width = size;
            image.height = size;
            image.mipmaps = 1;
            image.format = rl::PIXELFORMAT_UNCOMPRESSED_R32;

            this->heights_texture = LoadTextureFromImage(image);
            // SetTextureFilter(this->heights_texture, TEXTURE_FILTER_BILINEAR);
        }
        return this->heights_texture;
    }
};

class Game {
private:
    rl::Shader terrain_shader;
    rl::Shader sprite_shader;

    int screen_width;
    int screen_height;
    float dt = 1.0 / 60.0;

    World world;
    Camera camera;

    entt::registry registry;

public:
    Game()
        : screen_width(1500)
        , screen_height(1000)
        , world(100, 5.0, 0.6)
        , camera(50.0, world.get_center()) {

        SetConfigFlags(rl::FLAG_MSAA_4X_HINT);
        rl::InitWindow(screen_width, screen_height, "Sea Trader");
        rl::SetTargetFPS(60);

        this->terrain_shader = rl::LoadShader(
            "./resources/shaders/base.vert", "./resources/shaders/terrain.frag"
        );
        this->sprite_shader = rl::LoadShader(
            "./resources/shaders/base.vert", "./resources/shaders/sprite.frag"
        );

        {
            Transform transform(this->world.get_center());
            auto body = DynamicBody::create_ship();
            this->create_player_ship(transform, body);
        }
    }

    ~Game() {
        rl::CloseWindow();
    }

    entt::entity create_ship(Transform transform, DynamicBody dynamic_body) {
        auto entity = this->registry.create();
        this->registry.emplace<Transform>(entity, transform);
        this->registry.emplace<DynamicBody>(entity, dynamic_body);

        return entity;
    }

    entt::entity create_player_ship(Transform transform, DynamicBody dynamic_body) {
        auto entity = this->create_ship(transform, dynamic_body);
        this->registry.emplace<Player>(entity);

        return entity;
    }

    void run() {
        static float last_update_time = 0.0;

        while (!rl::WindowShouldClose()) {
            float time = rl::GetTime();

            while (time - last_update_time >= this->dt) {
                this->update();
                last_update_time += this->dt;
            }

            this->draw();
        }
    }

private:
    void update_camera() {
        if (IsMouseButtonDown(rl::MOUSE_MIDDLE_BUTTON)) {
            rl::Vector2 delta = rl::GetMouseDelta();
            rl::Vector2 curr = rl::GetMousePosition();
            rl::Vector2 prev = Vector2Add(curr, delta);

            curr = this->get_screen_to_world(curr);
            prev = this->get_screen_to_world(prev);
            delta = Vector2Subtract(curr, prev);

            this->camera.target = Vector2Add(this->camera.target, delta);
        }

        float wheel_move = rl::GetMouseWheelMove();
        float zoom = 0.0;
        if (wheel_move > 0.0) {
            zoom = -5.0;
        } else if (wheel_move < 0.0) {
            zoom = 5.0;
        }
        this->camera.view_width += zoom;
        this->camera.view_width = std::max(this->camera.view_width, 10.0f);
        this->camera.view_width = std::min(this->camera.view_width, 100.0f);
    }

    void update_player_input() {
        auto entity = registry.view<Player, Transform, DynamicBody>().front();
        auto &body = registry.get<DynamicBody>(entity);

        auto transform = registry.get<Transform>(entity);
        float rotation = transform.rotation;
        rl::Vector2 forward = {cosf(rotation), sinf(rotation)};

        if (rl::IsKeyDown(rl::KEY_A)) body.apply_torque(-5.0);
        if (rl::IsKeyDown(rl::KEY_D)) body.apply_torque(5.0);

        if (rl::IsKeyDown(rl::KEY_W)) body.apply_force(forward, 1000.0);
        if (rl::IsKeyDown(rl::KEY_S)) body.apply_force(forward, -1000.0);
    }

    void update_dynamic_bodies() {
        auto view = registry.view<Transform, DynamicBody>();
        for (auto entity : view) {
            auto [transform, body] = view.get(entity);

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

            // angular torque
            float damping_torque = body.angular_velocity * -body.angular_damping;
            float net_torque = body.net_torque + damping_torque;
            float angular_acceleration = net_torque / body.moment_of_inertia;
            body.angular_velocity += angular_acceleration * dt;
            body.net_torque = 0.0;

            // apply linear velocity
            rl::Vector2 linear_step = rl::Vector2Scale(body.linear_velocity, dt);
            transform.position = rl::Vector2Add(transform.position, linear_step);
            if (rl::Vector2Length(body.linear_velocity) < EPSILON) {
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

    void update() {
        this->update_camera();
        this->update_player_input();
        this->update_dynamic_bodies();
    }

    rl::Vector2 get_screen_to_world(rl::Vector2 p) {
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

    void set_camera(const Camera &camera, rl::Shader shader) {
        rl::rlDrawRenderBatchActive();

        int position_loc = GetShaderLocation(shader, "camera.position");
        int view_width_loc = GetShaderLocation(shader, "camera.view_width");
        int aspect_loc = GetShaderLocation(shader, "camera.aspect");

        float aspect = (float)this->screen_width / this->screen_height;

        SetShaderValue(shader, position_loc, &camera.target, rl::SHADER_UNIFORM_VEC2);
        SetShaderValue(
            shader, view_width_loc, &camera.view_width, rl::SHADER_UNIFORM_FLOAT
        );
        SetShaderValue(shader, aspect_loc, &aspect, rl::SHADER_UNIFORM_FLOAT);
    }

    void set_screen_camera(rl::Shader shader) {
        rl::Vector2 target = {screen_width / 2.0f, screen_height / 2.0f};
        Camera camera = Camera(this->screen_width, target);
        this->set_camera(camera, shader);
    }

    void draw_terrain() {
        rl::Shader shader = this->terrain_shader;
        rl::Texture texture = this->world.get_heights_texture();
        this->set_camera(this->camera, shader);

        int water_level_loc = GetShaderLocation(shader, "water_level");
        SetShaderValue(
            shader, water_level_loc, &this->world.water_level, rl::SHADER_UNIFORM_FLOAT
        );

        BeginShaderMode(shader);
        rl::Rectangle src = {
            .x = 0.0,
            .y = 0.0,
            .width = (float)texture.width,
            .height = (float)texture.height
        };
        rl::Rectangle dst = this->world.get_rect();
        DrawTexturePro(texture, src, dst, {0, 0}, 0, rl::WHITE);

        rl::EndShaderMode();
    }

    void draw_ships() {
        rl::Shader shader = this->sprite_shader;
        this->set_camera(this->camera, shader);
        BeginShaderMode(shader);

        auto view = registry.view<Transform>();
        for (auto entity : view) {
            auto [transform] = view.get(entity);
            float height = 0.5;
            float width = 1.0;
            rl::Vector2 origin = {0.5f * width, 0.5f * height};
            rl::Rectangle rect = {
                .x = transform.position.x - origin.x,
                .y = transform.position.y - origin.y,
                .width = width,
                .height = height
            };
            DrawRectanglePro(rect, origin, RAD2DEG * transform.rotation, rl::DARKBROWN);
        }

        rl::EndShaderMode();
    }

    void draw() {
        rl::BeginDrawing();
        ClearBackground(rl::BLACK);
        this->draw_terrain();
        this->draw_ships();
        rl::EndDrawing();
    }
};
}  // namespace st

int main() {
    auto game = st::Game();
    game.run();
}
