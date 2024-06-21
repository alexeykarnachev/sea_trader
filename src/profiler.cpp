#include "profiler.hpp"

#include "raylib/raylib.h"
#include <cstdio>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace st {
namespace profiler {

class Stage {
public:
    std::string name;
    std::string path;
    float start_time;

    Stage() = default;

    Stage(std::string name, std::string path)
        : name(name)
        , path(path)
        , start_time(GetTime()) {}

    std::string get_path() {
        if (this->path.length() == 0) {
            return this->name;
        } else {
            return this->path + "." + this->name;
        }
    }
};

class StageStats {
public:
    std::string path;
    float total_time = 0.0;
    int total_n_calls = 0;

    StageStats() = default;

    StageStats(std::string path)
        : path(path) {}

    void update(Stage stage) {
        if (this->path != stage.get_path()) {
            auto msg = "Failed to update stage stats: " + this->path
                       + " != " + stage.get_path();
            throw std::runtime_error(msg);
        }

        this->total_time += GetTime() - stage.start_time;
        this->total_n_calls += 1;
    }
};

static std::vector<Stage> STACK;
static std::unordered_map<std::string, StageStats> STATS;

void push(const std::string name) {
    if (STACK.size() != 0 && STACK.back().name == name) {
        throw std::runtime_error(
            "Failed to push profiler stage: can't profile recursive stage " + name
        );
    }

    std::string path;
    if (STACK.size() != 0) {
        path = STACK.back().get_path();
    } else {
        path = "";
    }

    STACK.emplace_back(name, path);
}

void pop() {
    if (STACK.size() == 0) {
        throw std::runtime_error("Failed pop profiler stage: stack is empty");
    }

    auto stage = STACK.back();
    auto path = stage.get_path();

    if (STATS.count(path) == 0) {
        STATS[path] = StageStats(path);
    }

    auto stats = &STATS[path];
    stats->update(stage);

    STACK.pop_back();
}

void draw() {
    // TODO: make proper ui
    for (auto &[name, stats] : STATS) {
        printf("%s", name.c_str());
        printf("\n");
        printf("    n_calls : %d\n", stats.total_n_calls);
        printf("    time    : %f\n", stats.total_time);
    }
}

}  // namespace profiler
}  // namespace st
