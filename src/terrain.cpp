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

static const int WORLD_SIZE = 200;
static const float RESOLUTION = 4.0;
static const float WATER_LEVEL = 0.6;
static const int DATA_SIZE = WORLD_SIZE * RESOLUTION;

static float *HEIGHTS;
static float *DISTS_TO_WATER;
static Texture HEIGHTS_TEXTURE;

void load() {
    // -------------------------------------------------------------------
    // init heights
    HEIGHTS_TEXTURE.id = 0;

    float offset_x = 0.0;
    float offset_y = 0.0;
    float scale = 2.0;

    float lacunarity = 1.4;
    float gain = 1.0;
    int octaves = 8;

    HEIGHTS = (float *)malloc(DATA_SIZE * DATA_SIZE * sizeof(float));

    float max_height = -FLT_MAX;
    float min_height = FLT_MAX;
    for (int i = 0; i < DATA_SIZE * DATA_SIZE; ++i) {
        int y = i / DATA_SIZE;
        int x = i % DATA_SIZE;

        float nx = (float)(x + offset_x) * (scale / (float)DATA_SIZE);
        float ny = (float)(y + offset_y) * (scale / (float)DATA_SIZE);
        float height = stb_perlin_fbm_noise3(nx, ny, 0.0, lacunarity, gain, octaves);

        max_height = std::max(max_height, height);
        min_height = std::min(min_height, height);
        HEIGHTS[i] = height;
    }

    for (int i = 0; i < DATA_SIZE * DATA_SIZE; ++i) {
        float *height = &HEIGHTS[i];
        *height = (*height - min_height) / (max_height - min_height);
    }

    // -------------------------------------------------------------------
    // init dists_to_water
    DISTS_TO_WATER = (float *)malloc(DATA_SIZE * DATA_SIZE * sizeof(float));
    std::fill(DISTS_TO_WATER, DISTS_TO_WATER + DATA_SIZE * DATA_SIZE, -1.0);

    std::queue<std::pair<int, float>> queue;
    for (int i = 0; i < DATA_SIZE * DATA_SIZE; ++i) {
        float height = HEIGHTS[i];
        if (check_if_water(height)) {
            DISTS_TO_WATER[i] = 0.0;
            queue.push({i, 0.0});
        }
    }

    std::pair<int, int> directions[8] = {
        {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}
    };

    while (!queue.empty()) {
        auto [i0, d0] = queue.front();
        int y0 = i0 / DATA_SIZE;
        int x0 = i0 % DATA_SIZE;

        queue.pop();

        for (auto [dx, dy] : directions) {
            int x1 = x0 + dx;
            int y1 = y0 + dy;
            int i1 = y1 * DATA_SIZE + x1;

            if (x1 >= 0 && x1 < DATA_SIZE && y1 >= 0 && y1 < DATA_SIZE) {
                if (DISTS_TO_WATER[i1] >= 0.0) continue;

                float height = HEIGHTS[i1];
                float d1 = 0.0;
                if (!check_if_water(height)) {
                    float dd = dx == 0 || dy == 0 ? 1.0 : 1.4142;
                    d1 = d0 + dd;
                }
                DISTS_TO_WATER[i1] = d1;
                queue.push({i1, d1});
            }
        }
    }

    // -------------------------------------------------------------------
    // init heights_texture
    Image image;
    image.data = HEIGHTS;
    image.width = DATA_SIZE;
    image.height = DATA_SIZE;
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R32;
    HEIGHTS_TEXTURE = LoadTextureFromImage(image);
}

int world_to_data_idx(Vector2 pos) {
    int x = pos.x * RESOLUTION;
    int y = pos.y * RESOLUTION;
    int idx = y * DATA_SIZE + x;

    return idx;
}

void unload() {
    UnloadTexture(HEIGHTS_TEXTURE);
}

int get_world_size() {
    return WORLD_SIZE;
}

Vector2 get_world_center() {
    return {WORLD_SIZE / 2.0f, WORLD_SIZE / 2.0f};
}

Rectangle get_world_rect() {
    return {
        .x = 0.0,
        .y = 0.0,
        .width = (float)WORLD_SIZE,
        .height = (float)WORLD_SIZE,
    };
}

float get_height(Vector2 pos) {
    return HEIGHTS[world_to_data_idx(pos)];
}

float get_dist_to_water(Vector2 pos) {
    return DISTS_TO_WATER[world_to_data_idx(pos)];
}

bool check_if_water(float h) {
    return h <= WATER_LEVEL;
}

bool check_if_water(Vector2 pos) {
    return check_if_water(get_height(pos));
}

void draw() {
    Shader shader = resources::TERRAIN_SHADER;
    renderer::set_game_camera(shader);

    int water_level_loc = GetShaderLocation(shader, "water_level");
    SetShaderValue(shader, water_level_loc, &WATER_LEVEL, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(shader);
    Rectangle src = {
        .x = 0.0,
        .y = 0.0,
        .width = (float)HEIGHTS_TEXTURE.width,
        .height = (float)HEIGHTS_TEXTURE.height
    };
    Rectangle dst = get_world_rect();
    DrawTexturePro(HEIGHTS_TEXTURE, src, dst, {0, 0}, 0, WHITE);

    EndShaderMode();
}

}  // namespace terrain

}  // namespace st
