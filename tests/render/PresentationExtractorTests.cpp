#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "input/PlayerCommand.hpp"
#include "render/PresentationExtractor.hpp"
#include "simulation/Simulation.hpp"

TEST_CASE("Presentation extraction creates a stable player sprite") {
    ac2::simulation::Simulation simulation;
    const auto snapshot = ac2::render::extract_presentation(simulation);

    REQUIRE(snapshot.sprites.size() == 1);
    const auto& player = snapshot.sprites.front();
    REQUIRE(player.visual_id == 1);
    REQUIRE(player.previous_x == Catch::Approx(0.0F));
    REQUIRE(player.previous_y == Catch::Approx(0.0F));
    REQUIRE(player.current_x == Catch::Approx(0.0F));
    REQUIRE(player.current_y == Catch::Approx(0.0F));
}

TEST_CASE("Presentation extraction exposes previous and current simulation transforms") {
    ac2::simulation::Simulation simulation;
    ac2::input::PlayerCommand command{};
    command.move_x = 1.0F;
    simulation.tick(command);

    const auto tick_before = simulation.tick_index();
    const auto position_before = simulation.player_transform();
    const auto snapshot = ac2::render::extract_presentation(simulation);

    REQUIRE(snapshot.sprites.size() == 1);
    const auto& player = snapshot.sprites.front();
    REQUIRE(player.previous_x == Catch::Approx(0.0F));
    REQUIRE(player.current_x == Catch::Approx(0.1F));
    REQUIRE(player.previous_y == Catch::Approx(0.0F));
    REQUIRE(player.current_y == Catch::Approx(0.0F));

    REQUIRE(simulation.tick_index() == tick_before);
    const auto position_after = simulation.player_transform();
    REQUIRE(position_after.x == Catch::Approx(position_before.x));
    REQUIRE(position_after.y == Catch::Approx(position_before.y));
}
