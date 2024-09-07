#include "ship.hpp"

#include "camera.hpp"
#include "cargo.hpp"
#include "components.hpp"
#include "dynamic_body.hpp"
#include "raylib/raymath.h"
#include "registry.hpp"
#include "renderer.hpp"
#include "terrain.hpp"
#include <float.h>

namespace st {
namespace ship {

Ship::Ship(entt::entity entity, ControllerType controller_type, cargo::Cargo cargo)
    : entity(entity)
    , controller_type(controller_type)
    , cargo(cargo) {}

void Ship::move(bool is_forward) {
    auto &body = registry::registry.get<dynamic_body::DynamicBody>(this->entity);
    auto transform = registry::registry.get<components::Transform>(this->entity);

    Vector2 forward = transform.get_forward();
    int sign = is_forward ? 1 : -1;

    body.apply_force(forward, sign * this->force);
}

void Ship::rotate(bool is_right) {
    auto &body = registry::registry.get<dynamic_body::DynamicBody>(this->entity);

    int sign = is_right ? 1 : -1;

    body.apply_torque(sign * this->torque);
}

// TODO: Move this somewhere else.
static Vector2 screen_to_world(Vector2 screen_position) {
    Vector2 screen_size = {(float)renderer::SCREEN_WIDTH, (float)renderer::SCREEN_HEIGHT};
    Vector2 cursor = Vector2Divide(screen_position, screen_size);
    float aspect = screen_size.x / screen_size.y;
    float view_height = camera::VIEW_WIDTH / aspect;
    Vector2 center = camera::TARGET;
    float left = center.x - 0.5 * camera::VIEW_WIDTH;
    float top = center.y - 0.5 * view_height;
    float x = left + camera::VIEW_WIDTH * cursor.x;
    float y = top + view_height * cursor.y;

    return {.x = x, .y = y};
}

void Ship::update_controller_manual() {
    auto &tr = registry::registry.get<components::Transform>(this->entity);
    auto *path = registry::registry.try_get<components::Path>(this->entity);

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        // compute new path if rmb pressed
        auto start = tr.position;
        auto end = screen_to_world(GetMousePosition());
        auto path_points = terrain::get_path_points(start, end);
        registry::registry.emplace_or_replace<components::Path>(
            this->entity, path_points
        );
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && path != NULL) {
        // remove path if lmb pressed
        registry::registry.remove<components::Path>(this->entity);
    } else if (path != NULL) {
        // follow path
        Vector2 end_point = path->points[path->points.size() - 1];
        Vector2 nearest_point = path->points[0];

        float dist_to_end_point = Vector2DistanceSqr(tr.position, end_point);
        float dist_to_nearest_point = FLT_MAX;
        for (Vector2 path_point : path->points) {
            bool is_closer = Vector2DistanceSqr(path_point, end_point)
                             <= dist_to_end_point;

            if (is_closer) {
                float dist = Vector2DistanceSqr(tr.position, path_point);
                if (dist < dist_to_nearest_point) {
                    dist_to_nearest_point = dist;
                    nearest_point = path_point;
                }
            }
        }

        Vector2 to_end_point = Vector2Subtract(end_point, tr.position);
        Vector2 to_nearest_point = Vector2Subtract(nearest_point, tr.position);

        // ---------------------------------------------------------------
        // follow path
        Vector2 direction = Vector2Normalize(to_nearest_point);

        // calculate the angle between current rotation and desired direction
        float current_angle = tr.rotation;
        float target_angle = atan2f(direction.y, direction.x);
        float angle_diff = target_angle - current_angle;

        // normalize the angle difference to be between -PI and PI
        while (angle_diff > PI) {
            angle_diff -= 2 * PI;
        }
        while (angle_diff < -PI) {
            angle_diff += 2 * PI;
        }

        // determine rotation direction and rotate
        const float rotation_threshold = 0.1f;
        if (angle_diff > rotation_threshold) {
            this->rotate(true);
        } else if (angle_diff < -rotation_threshold) {
            this->rotate(false);
        }

        // move forward if facing approximately the right direction
        const float movement_threshold = 0.1f;
        if (fabsf(angle_diff) < movement_threshold) {
            this->move(true);
        }
    } else {
        // manual movement
        if (IsKeyDown(KEY_A)) this->rotate(false);
        if (IsKeyDown(KEY_D)) this->rotate(true);

        if (IsKeyDown(KEY_W)) this->move(true);
        if (IsKeyDown(KEY_S)) this->move(false);
    }
}

void Ship::update_controller_dummy() {
    this->rotate(true);
    this->move(true);
}

void Ship::update() {
    switch (this->controller_type) {
        case ControllerType::MANUAL: this->update_controller_manual(); break;
        case ControllerType::DUMMY: this->update_controller_dummy(); break;
    }
}

}  // namespace ship
}  // namespace st
