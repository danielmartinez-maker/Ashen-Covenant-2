#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <vector>

#include "input/PlayerCommand.hpp"
#include "simulation/Simulation.hpp"

using ac2::input::PlayerCommand;
using ac2::simulation::Simulation;

TEST_CASE("Simulation creates one player at the origin") {
    Simulation simulation;
    const auto transform = simulation.player_transform();

    REQUIRE(transform.x == Catch::Approx(0.0F));
    REQUIRE(transform.y == Catch::Approx(0.0F));
    REQUIRE(simulation.tick_index() == 0);
}

TEST_CASE("Simulation preserves player position without movement intent") {
    Simulation simulation;
    simulation.tick(PlayerCommand{});

    const auto transform = simulation.player_transform();
    REQUIRE(transform.x == Catch::Approx(0.0F));
    REQUIRE(transform.y == Catch::Approx(0.0F));
    REQUIRE(simulation.tick_index() == 1);
}

TEST_CASE("Simulation moves player at fixed per-tick speed") {
    Simulation simulation;
    PlayerCommand command{};
    command.move_x = 1.0F;

    simulation.tick(command);

    const auto transform = simulation.player_transform();
    REQUIRE(transform.x == Catch::Approx(0.1F));
    REQUIRE(transform.y == Catch::Approx(0.0F));
}

TEST_CASE("Simulation normalizes diagonal movement") {
    Simulation simulation;
    PlayerCommand command{};
    command.move_x = 1.0F;
    command.move_y = 1.0F;

    simulation.tick(command);

    const auto transform = simulation.player_transform();
    const auto distance = std::sqrt(transform.x * transform.x + transform.y * transform.y);
    REQUIRE(distance == Catch::Approx(0.1F));
}

TEST_CASE("Simulation is reproducible for identical command streams") {
    const std::vector<PlayerCommand> commands{
        PlayerCommand{.move_x = 1.0F, .move_y = 0.0F},
        PlayerCommand{.move_x = 1.0F, .move_y = 1.0F},
        PlayerCommand{.move_x = -0.25F, .move_y = 0.5F},
        PlayerCommand{},
        PlayerCommand{.move_x = 0.0F, .move_y = -1.0F},
    };

    Simulation first;
    Simulation second;
    for (const auto& command : commands) {
        first.tick(command);
        second.tick(command);
    }

    const auto first_transform = first.player_transform();
    const auto second_transform = second.player_transform();
    REQUIRE(first_transform.x == Catch::Approx(second_transform.x));
    REQUIRE(first_transform.y == Catch::Approx(second_transform.y));
    REQUIRE(first.tick_index() == commands.size());
    REQUIRE(second.tick_index() == commands.size());
}
