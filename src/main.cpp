#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "stb/stb_perlin.h"
#include "ui.hpp"
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <string>
#include <utility>

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

class Port {
public:
    const float radius;
    Port(float radius)
        : radius(radius) {}
};

class Ship {};
class Player {};

class Camera {
public:
    float view_width;
    rl::Vector2 position;
    rl::Vector2 target;

    Camera(float view_width, rl::Vector2 position)
        : view_width(view_width)
        , position(position)
        , target(position) {}

    void reset_position(rl::Vector2 position) {
        this->position = position;
        this->target = position;
    }
};

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
    rl::Shader terrain_shader;
    rl::Shader sprite_shader;

    rl::Texture product_icons_texture;

    int screen_width;
    int screen_height;
    float dt = 1.0 / 60.0;

    Terrain terrain;
    Camera camera;

    entt::registry registry;
    entt::entity player_moored_port = entt::null;

    bool window_should_close = false;

public:
    Game()
        : screen_width(1500)
        , screen_height(1000)
        , terrain(200, 4.0, 0.6)
        , camera(50.0, terrain.get_center()) {

        // window
        SetConfigFlags(rl::FLAG_MSAA_4X_HINT);
        rl::InitWindow(screen_width, screen_height, "Sea Trader");
        rl::SetTargetFPS(60);

        // ui
        ui::load();

        // shaders
        this->terrain_shader = rl::LoadShader(
            "./resources/shaders/base.vert", "./resources/shaders/terrain.frag"
        );
        this->sprite_shader = rl::LoadShader(
            "./resources/shaders/base.vert", "./resources/shaders/sprite.frag"
        );

        // sprites
        this->product_icons_texture = rl::LoadTexture(
            "./resources/sprites/product_icons_64.png"
        );
        SetTextureFilter(this->product_icons_texture, rl::TEXTURE_FILTER_BILINEAR);

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
        rl::CloseWindow();
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
        auto entity = this->registry.create();
        this->registry.emplace<Transform>(entity, transform);
        this->registry.emplace<DynamicBody>(entity, dynamic_body);
        this->registry.emplace<Ship>(entity);

        return entity;
    }

    entt::entity create_player_ship(Transform transform, DynamicBody dynamic_body) {
        auto entity = this->create_ship(transform, dynamic_body);
        this->registry.emplace<Player>(entity);

        return entity;
    }

    entt::entity create_port(Transform transform, Port port) {
        auto entity = this->registry.create();
        this->registry.emplace<Transform>(entity, transform);
        this->registry.emplace<Port>(entity, port);

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

        auto entity = registry.view<Player>().front();
        auto &body = registry.get<DynamicBody>(entity);

        auto transform = registry.get<Transform>(entity);
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

        auto player_entity = registry.view<Player>().front();
        auto &player_transform = registry.get<Transform>(player_entity);

        auto view = registry.view<Transform, Port>();
        for (auto port_entity : view) {
            auto [port_transform, port] = view.get(port_entity);
            float dist = rl::Vector2Distance(
                player_transform.position, port_transform.position
            );
            if (dist <= port.radius) {
                this->player_moored_port = port_entity;
                return;
            }
        }
    }

    void update_dynamic_bodies() {
        auto view = registry.view<Transform, DynamicBody>();
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
        if (this->player_moored_port == entt::null) {
            this->update_camera();
            this->update_player_ship_movement();
            this->update_player_entering_port();
            this->update_dynamic_bodies();
        }

        this->update_window_should_close();
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

        SetShaderValue(shader, position_loc, &camera.position, rl::SHADER_UNIFORM_VEC2);
        SetShaderValue(
            shader, view_width_loc, &camera.view_width, rl::SHADER_UNIFORM_FLOAT
        );
        SetShaderValue(shader, aspect_loc, &aspect, rl::SHADER_UNIFORM_FLOAT);
    }

    void set_screen_camera(rl::Shader shader) {
        rl::Vector2 position = {screen_width / 2.0f, screen_height / 2.0f};
        Camera camera = Camera(this->screen_width, position);
        this->set_camera(camera, shader);
    }

    void draw_terrain() {
        rl::Shader shader = this->terrain_shader;
        rl::Texture texture = this->terrain.get_heights_texture();
        this->set_camera(this->camera, shader);

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

        rl::Shader shader = this->sprite_shader;
        this->set_camera(this->camera, shader);
        BeginShaderMode(shader);

        auto view = registry.view<Transform, Ship>();
        for (auto entity : view) {
            auto [transform] = view.get(entity);

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

    void update_and_draw_player_moored_port_ui() {
        if (rl::IsKeyPressed(rl::KEY_ESCAPE)) {
            this->player_moored_port = entt::null;
            return;
        }

        rl::Shader shader = this->sprite_shader;
        this->set_screen_camera(shader);

        // ---------------------------------------------------------------
        // products shop
        static const float product_icon_size_src = 64.0;
        static const int column_name_font_size = 32;
        static const int product_name_font_size = 20;
        static const int n_rows = 10;
        static const float pane_width = 600.0;
        static const float pane_border = 20.0;
        static const float row_border = 3.0;
        static const float row_height = 60.0;
        static const float product_icon_size_dst = row_height - 2.0 * row_border;
        static const float ui_icon_size_dst = 0.7 * (row_height - 2.0 * row_border);
        static const float row_width = pane_width - 2.0 * pane_border;
        static const float row_gap = 5.0;
        static const float pane_height = 2.0 * pane_border + (n_rows + 1) * row_height
                                         + n_rows * row_gap;
        static const float mid_col_width = 220.0;

        static int selected_product_i = -1;
        static int selected_product_n_buy = 0;
        static double last_increment_time_left = 0.0;
        static double last_increment_time_right = 0.0;

        // pane
        float pane_x = 0.5 * (this->screen_width - pane_width);
        float pane_y = 0.5 * (this->screen_height - pane_height);
        rl::Rectangle pane_rect = {
            .x = pane_x, .y = pane_y, .width = pane_width, .height = pane_height
        };
        ui::rect_cold(pane_rect);

        const float row_x = pane_x + pane_border;
        const float mid_x = pane_x + 0.5 * pane_width;

        // header
        const float header_y = pane_y + pane_border;
        rl::Rectangle header_rect = {
            .x = row_x, .y = header_y, .width = row_width, .height = row_height
        };
        ui::rect_cold(header_rect);

        // product panes
        const float row_y = header_y + row_height + row_gap;
        for (int i = 0; i < n_rows; ++i) {
            float offset_y = (row_height + row_gap) * i;

            rl::Rectangle dst = {
                .x = row_x,
                .y = row_y + offset_y,
                .width = row_width,
                .height = row_height
            };

            if (ui::radio_button_rect(dst, &selected_product_i, i)) {
                selected_product_n_buy = 0;
            }
        }

        // rows
        const float icon_x = mid_x - 0.5 * mid_col_width + row_border;
        const float icon_y = row_y + row_border;
        for (int i = 0; i < n_rows; ++i) {
            bool is_selected = selected_product_i == i;
            float offset_y = (row_height + row_gap) * i;

            // product icon
            rl::Rectangle src = {
                .x = i * product_icon_size_src,
                .y = 0.0,
                .width = product_icon_size_src,
                .height = product_icon_size_src
            };
            rl::Rectangle dst = {
                .x = icon_x,
                .y = icon_y + offset_y,
                .width = product_icon_size_dst,
                .height = product_icon_size_dst
            };
            ui::sprite(this->product_icons_texture, src, dst);

            // product name
            float gap = 10.0;
            auto text = "Product Name";
            float text_y = dst.y;
            float text_x = dst.x + dst.width + gap;

            if (is_selected) {
                ui::text_dark(text, text_x, text_y, product_name_font_size);
            } else {
                ui::text_mild(text, text_x, text_y, product_name_font_size);
            }

            // selected product
            if (is_selected) {
                // buy amount
                {
                    int font_size = 0.8 * product_name_font_size;
                    auto n_str = std::to_string(std::abs(selected_product_n_buy));
                    float offset_y = product_name_font_size + row_border;

                    std::string text;
                    if (selected_product_n_buy >= 0) {
                        text = "Buy: " + n_str;
                        ui::text_info(text, text_x, text_y + offset_y, font_size);
                    } else {
                        text = "Sell: " + n_str;
                        ui::text_error(text, text_x, text_y + offset_y, font_size);
                    }
                }

                float mid_y = row_y + offset_y + 0.5 * row_height;

                // arrows
                ui::increment_button_sprite(
                    ui::SpriteName::LEFT_ARROW_ICON_SRC,
                    {.x = row_x + row_border,
                     .y = mid_y - 0.5f * ui_icon_size_dst,
                     .width = ui_icon_size_dst,
                     .height = ui_icon_size_dst},
                    &last_increment_time_left,
                    &selected_product_n_buy,
                    +1,
                    0,
                    100
                );

                ui::increment_button_sprite(
                    ui::SpriteName::RIGHT_ARROW_ICON_SRC,
                    {.x = row_x + row_width - row_border - ui_icon_size_dst,
                     .y = mid_y - 0.5f * ui_icon_size_dst,
                     .width = ui_icon_size_dst,
                     .height = ui_icon_size_dst},
                    &last_increment_time_right,
                    &selected_product_n_buy,
                    -1,
                    0,
                    100
                );
            }
        }

        // column names
        float text_y = header_y + 0.5 * (row_height - column_name_font_size);

        {
            auto text = "Name";
            int text_width = rl::MeasureText(text, column_name_font_size);
            float text_x = mid_x - 0.5 * text_width;
            ui::text_light(text, text_x, text_y, column_name_font_size);
        }

        {
            auto text = "Buy";
            int text_width = rl::MeasureText(text, column_name_font_size);
            float col_right_x = mid_x - 0.5 * mid_col_width;
            float col_mid_x = 0.5 * (row_x + col_right_x);
            float text_x = col_mid_x - 0.5 * text_width;
            ui::text_light(text, text_x, text_y, column_name_font_size);
        }

        {
            auto text = "Sell";
            int text_width = rl::MeasureText(text, column_name_font_size);
            float col_left_x = mid_x + 0.5 * mid_col_width;
            float col_mid_x = 0.5 * (row_x + row_width + col_left_x);
            float text_x = col_mid_x - 0.5 * text_width;
            ui::text_light(text, text_x, text_y, column_name_font_size);
        }

        // column vertical lines
        float line_top_y = pane_y + pane_border;
        float line_bot_y = pane_y + pane_height - pane_border;

        {
            float line_x = mid_x - 0.5 * mid_col_width;
            ui::line_mild(line_x, line_top_y, line_x, line_bot_y);
        }

        {
            float line_x = mid_x + 0.5 * mid_col_width;
            ui::line_mild(line_x, line_top_y, line_x, line_bot_y);
        }
    }

    void draw_ports() {
        static float radius = 0.8;

        rl::Shader shader = this->sprite_shader;
        this->set_camera(this->camera, shader);
        BeginShaderMode(shader);

        auto view = registry.view<Transform, Port>();
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

        this->draw_terrain();
        this->draw_ports();
        this->draw_ships();

        ui::begin();
        if (this->player_moored_port != entt::null) {
            this->update_and_draw_player_moored_port_ui();
        }

        rl::EndDrawing();
    }
};
}  // namespace st

int main() {
    auto game = st::Game();
    game.run();
}
