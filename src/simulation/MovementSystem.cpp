#include "simulation/MovementSystem.hpp"

#include <cmath>

#include "simulation/Components.hpp"

namespace ac2::simulation {

void apply_movement(
    entt::registry& registry,
    entt::entity player,
    const ac2::input::PlayerCommand& command) {
    auto& transform = registry.get<Transform>(player);
    const auto& speed = registry.get<MoveSpeed>(player);

    float move_x = command.move_x;
    float move_y = command.move_y;
    const float magnitude_squared = move_x * move_x + move_y * move_y;
    if (magnitude_squared > 1.0F) {
        const float inverse_magnitude = 1.0F / std::sqrt(magnitude_squared);
        move_x *= inverse_magnitude;
        move_y *= inverse_magnitude;
    }

    constexpr float fixed_tick_seconds = 1.0F / 60.0F;
    transform.x += move_x * speed.units_per_second * fixed_tick_seconds;
    transform.y += move_y * speed.units_per_second * fixed_tick_seconds;
}

}  // namespace ac2::simulation
