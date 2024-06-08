#include "terrain.hpp"

#include "raylib/raylib.h"
#include "renderer.hpp"
#include "resources.hpp"
#include "stb/stb_perlin.h"
#include <algorithm>
#include <cfloat>
#include <cstdlib>
#include <queue>

namespace st {
namespace terrain {

static const int world_size = 200;
static const float resolution = 4.0;
static const float water_level = 0.6;
static const int data_size = world_size * resolution;

static float *heights;
static float *dists_to_water;
static Texture heights_texture;

void load() {
    // -------------------------------------------------------------------
    // init heights
    heights_texture.id = 0;

    float offset_x = 0.0;
    float offset_y = 0.0;
    float scale = 2.0;

    float lacunarity = 1.4;
    float gain = 1.0;
    int octaves = 8;

    heights = (float *)malloc(data_size * data_size * sizeof(float));

    float max_height = -FLT_MAX;
    float min_height = FLT_MAX;
    for (int i = 0; i < data_size * data_size; ++i) {
        int y = i / data_size;
        int x = i % data_size;

        float nx = (float)(x + offset_x) * (scale / (float)data_size);
        float ny = (float)(y + offset_y) * (scale / (float)data_size);
        float height = stb_perlin_fbm_noise3(nx, ny, 0.0, lacunarity, gain, octaves);

        max_height = std::max(max_height, height);
        min_height = std::min(min_height, height);
        heights[i] = height;
    }

    for (int i = 0; i < data_size * data_size; ++i) {
        float *height = &heights[i];
        *height = (*height - min_height) / (max_height - min_height);
    }

    // -------------------------------------------------------------------
    // init dists_to_water
    dists_to_water = (float *)malloc(data_size * data_size * sizeof(float));
    std::fill(dists_to_water, dists_to_water + data_size * data_size, -1.0);

    std::queue<std::pair<int, float>> queue;
    for (int i = 0; i < data_size * data_size; ++i) {
        float height = heights[i];
        if (check_if_water(height)) {
            dists_to_water[i] = 0.0;
            queue.push({i, 0.0});
        }
    }

    std::pair<int, int> directions[8] = {
        {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}
    };

    while (!queue.empty()) {
        auto [i0, d0] = queue.front();
        int y0 = i0 / data_size;
        int x0 = i0 % data_size;

        queue.pop();

        for (auto [dx, dy] : directions) {
            int x1 = x0 + dx;
            int y1 = y0 + dy;
            int i1 = y1 * data_size + x1;

            if (x1 >= 0 && x1 < data_size && y1 >= 0 && y1 < data_size) {
                if (dists_to_water[i1] >= 0.0) continue;

                float height = heights[i1];
                float d1 = 0.0;
                if (!check_if_water(height)) {
                    float dd = dx == 0 || dy == 0 ? 1.0 : 1.4142;
                    d1 = d0 + dd;
                }
                dists_to_water[i1] = d1;
                queue.push({i1, d1});
            }
        }
    }

    // -------------------------------------------------------------------
    // init heights_texture
    Image image;
    image.data = heights;
    image.width = data_size;
    image.height = data_size;
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R32;
    heights_texture = LoadTextureFromImage(image);
}

int world_to_data_idx(Vector2 pos) {
    int x = pos.x * resolution;
    int y = pos.y * resolution;
    int idx = y * data_size + x;

    return idx;
}

void unload() {
    UnloadTexture(heights_texture);
}

int get_world_size() {
    return world_size;
}

Vector2 get_world_center() {
    return {world_size / 2.0f, world_size / 2.0f};
}

Rectangle get_world_rect() {
    return {
        .x = 0.0,
        .y = 0.0,
        .width = (float)world_size,
        .height = (float)world_size,
    };
}

float get_height(Vector2 pos) {
    return heights[world_to_data_idx(pos)];
}

float get_dist_to_water(Vector2 pos) {
    return dists_to_water[world_to_data_idx(pos)];
}

bool check_if_water(float h) {
    return h <= water_level;
}

bool check_if_water(Vector2 pos) {
    return check_if_water(get_height(pos));
}

void draw() {
    Shader shader = resources::terrain_shader;
    renderer::set_game_camera(shader);

    int water_level_loc = GetShaderLocation(shader, "water_level");
    SetShaderValue(shader, water_level_loc, &water_level, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(shader);
    Rectangle src = {
        .x = 0.0,
        .y = 0.0,
        .width = (float)heights_texture.width,
        .height = (float)heights_texture.height
    };
    Rectangle dst = get_world_rect();
    DrawTexturePro(heights_texture, src, dst, {0, 0}, 0, WHITE);

    EndShaderMode();
}

}  // namespace terrain

}  // namespace st
