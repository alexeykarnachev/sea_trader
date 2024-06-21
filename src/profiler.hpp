#pragma once

#include <string>
#include <utility>

namespace st {
namespace profiler {

void push(const std::string name);
void pop();

template <typename F, typename... Args>
auto profile_function(const std::string name, F &&fn, Args &&...args)
    -> decltype(std::forward<F>(fn)(std::forward<Args>(args)...)) {

    push(name);
    auto result = std::forward<F>(fn)(std::forward<Args>(args)...);
    pop();

    return result;
}

void draw();

}  // namespace profiler
}  // namespace st
