#include "terrain.hpp"

#include "constants.hpp"
#include "raylib/raylib.h"
#include "renderer.hpp"
#include "resources.hpp"
#include "stb/stb_perlin.h"
#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <vector>

namespace st {
namespace terrain {

static std::pair<int, int> DIRECTIONS[8] = {
    {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}
};

// world parameters
static constexpr int WORLD_SIZE = 200;
static constexpr float RESOLUTION = 4.0;
static constexpr float WATER_LEVEL = 0.6;
static constexpr int DATA_SIZE = WORLD_SIZE * RESOLUTION;

// data pointers
static float *HEIGHTS;
static float *DISTS_TO_WATER;
static float *DISTS_TO_GROUND;
static Texture HEIGHTS_TEXTURE;

// pathfinding parameters
static constexpr int PATH_STEP = 3;

std::pair<int, int> data_idx_to_xy(int idx) {
    int x = idx % DATA_SIZE;
    int y = idx / DATA_SIZE;
    return std::make_pair(x, y);
}

int xy_to_data_idx(int x, int y) {
    if (x < 0 || x >= DATA_SIZE || y < 0 || y >= DATA_SIZE) return -1;
    return y * DATA_SIZE + x;
}

int world_to_data_idx(Vector2 pos) {
    int x = pos.x * RESOLUTION;
    int y = pos.y * RESOLUTION;
    int idx = y * DATA_SIZE + x;

    if (idx < 0 || idx >= (DATA_SIZE * DATA_SIZE)) {
        return -1;
    }

    return idx;
}

Vector2 data_idx_to_world(int idx) {
    auto [x, y] = data_idx_to_xy(idx);
    Vector2 pos = {x / RESOLUTION, y / RESOLUTION};
    return pos;
}

float *get_distances(bool (*fn)(float)) {
    float *data = (float *)malloc(DATA_SIZE * DATA_SIZE * sizeof(float));
    std::fill(data, data + DATA_SIZE * DATA_SIZE, -1.0);

    std::queue<int> queue;
    for (int i = 0; i < DATA_SIZE * DATA_SIZE; ++i) {
        if (fn(HEIGHTS[i])) {
            data[i] = 0.0;
            queue.push(i);
        }
    }

    while (!queue.empty()) {
        auto i0 = queue.front();
        auto [x0, y0] = data_idx_to_xy(i0);
        queue.pop();

        for (auto [dx, dy] : DIRECTIONS) {
            int i1 = xy_to_data_idx(x0 + dx, y0 + dy);
            if (i1 < 0 || data[i1] >= 0.0) continue;

            float d1 = 0.0;
            if (!fn(HEIGHTS[i1])) {
                float dd = dx == 0 || dy == 0 ? 1.0 : SQRT2;
                d1 = data[i0] + dd;
            }
            data[i1] = d1;
            queue.push(i1);
        }
    }

    return data;
}

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
        auto [x, y] = data_idx_to_xy(i);

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
    // init heights_texture
    Image image;
    image.data = HEIGHTS;
    image.width = DATA_SIZE;
    image.height = DATA_SIZE;
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R32;
    HEIGHTS_TEXTURE = LoadTextureFromImage(image);

    // -------------------------------------------------------------------
    // init distances
    DISTS_TO_WATER = get_distances(check_if_water);
    DISTS_TO_GROUND = get_distances(check_if_ground);
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
    int idx = world_to_data_idx(pos);
    if (idx < 0) return FLT_MAX;
    return HEIGHTS[idx];
}

float get_dist_to_water(Vector2 pos) {
    int idx = world_to_data_idx(pos);
    if (idx < 0) return FLT_MAX;
    return DISTS_TO_WATER[idx];
}

bool check_if_water(float h) {
    return h <= WATER_LEVEL;
}

bool check_if_water(Vector2 pos) {
    return check_if_water(get_height(pos));
}

bool check_if_ground(float h) {
    return h > WATER_LEVEL;
}

bool check_if_ground(Vector2 pos) {
    return check_if_ground(get_height(pos));
}

// -----------------------------------------------------------------------
// astar path finding
struct Node {
    int idx;
    int parent_idx;
    float g_cost;
    float f_cost;
};

struct CompareNode {
    bool operator()(const Node &n1, const Node &n2) {
        return n1.f_cost > n2.f_cost;
    }
};

float get_h_cost(int idx1, int idx2) {
    // euclidian cost
    auto [x1, y1] = data_idx_to_xy(idx1);
    auto [x2, y2] = data_idx_to_xy(idx2);
    int dx = x2 - x1;
    int dy = y2 - y1;
    float euclidian_cost = std::sqrt(dx * dx + dy * dy);

    // distance to ground cost
    float dist_to_ground_cost = -DISTS_TO_GROUND[idx1];

    // compound cost
    float h_cost = euclidian_cost + dist_to_ground_cost;

    return h_cost;
}

std::vector<Vector2> get_path(Vector2 start, Vector2 end) {
    static std::array<Node, DATA_SIZE * DATA_SIZE> nodes;
    std::memset(nodes.data(), 0, sizeof(nodes));

    std::priority_queue<Node, std::vector<Node>, CompareNode> queue;
    std::vector<Vector2> path = {};

    int start_idx = world_to_data_idx(start);
    int end_idx = world_to_data_idx(end);
    if (start_idx < 0 || end_idx < 0) return path;
    auto [end_x, end_y] = data_idx_to_xy(end_idx);

    Node start_node = {start_idx, -1, 0, get_h_cost(start_idx, end_idx)};
    nodes[start_idx] = start_node;
    queue.push(start_node);

    while (!queue.empty()) {
        Node current = queue.top();
        queue.pop();

        if (current.idx == end_idx) {
            while (current.parent_idx != -1) {
                path.push_back(data_idx_to_world(current.idx));
                current = nodes[current.parent_idx];
            }
            std::reverse(path.begin(), path.end());
            break;
        }

        auto [current_x, current_y] = data_idx_to_xy(current.idx);

        int dx = end_x - current_x;
        int dy = end_y - current_y;
        float d = std::sqrt(dx * dx + dy * dy);
        int step = d <= PATH_STEP * SQRT2 ? 1 : PATH_STEP;

        for (auto &dir : DIRECTIONS) {
            int new_x = current_x + dir.first * step;
            int new_y = current_y + dir.second * step;
            int new_idx = xy_to_data_idx(new_x, new_y);
            if (new_idx < 0 || !check_if_water(HEIGHTS[new_idx])) continue;

            float d_cost = (dir.first == 0 || dir.second == 0) ? step : step * SQRT2;
            float g_cost = current.g_cost + d_cost;
            float h_cost = get_h_cost(new_idx, end_idx);
            float f_cost = g_cost + h_cost;

            if (nodes[new_idx].idx == 0 || f_cost < nodes[new_idx].f_cost) {
                nodes[new_idx] = {new_idx, current.idx, g_cost, f_cost};
                queue.push(nodes[new_idx]);
            }
        }
    }

    return path;
}

// -----------------------------------------------------------------------
// draw
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
