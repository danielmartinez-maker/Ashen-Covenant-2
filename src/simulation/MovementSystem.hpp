#pragma once

#include <entt/entt.hpp>

#include "input/PlayerCommand.hpp"

namespace ac2::simulation {

void apply_movement(
    entt::registry& registry,
    entt::entity player,
    const ac2::input::PlayerCommand& command);

}  // namespace ac2::simulation
