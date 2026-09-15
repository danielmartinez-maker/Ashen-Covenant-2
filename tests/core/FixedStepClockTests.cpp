#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/FixedStepClock.hpp"

using ac2::core::FixedStepClock;

TEST_CASE("FixedStepClock preserves sub-tick interpolation") {
    FixedStepClock clock;
    const auto result = clock.advance(FixedStepClock::tick_seconds * 0.5);

    REQUIRE(result.ticks == 0);
    REQUIRE(result.alpha == Catch::Approx(0.5));
    REQUIRE_FALSE(result.dropped_time);
}

TEST_CASE("FixedStepClock executes an exact simulation tick") {
    FixedStepClock clock;
    const auto result = clock.advance(FixedStepClock::tick_seconds);

    REQUIRE(result.ticks == 1);
    REQUIRE(result.alpha == Catch::Approx(0.0).margin(1e-12));
    REQUIRE_FALSE(result.dropped_time);
}

TEST_CASE("FixedStepClock accumulates multiple ticks and interpolation") {
    FixedStepClock clock;
    const auto result = clock.advance(FixedStepClock::tick_seconds * 2.5);

    REQUIRE(result.ticks == 2);
    REQUIRE(result.alpha == Catch::Approx(0.5));
    REQUIRE_FALSE(result.dropped_time);
}

TEST_CASE("FixedStepClock caps catch-up and discards excess whole ticks") {
    FixedStepClock clock;
    const auto result = clock.advance(FixedStepClock::tick_seconds * 10.25);

    REQUIRE(result.ticks == FixedStepClock::max_catch_up_ticks);
    REQUIRE(result.alpha == Catch::Approx(0.25));
    REQUIRE(result.dropped_time);

    const auto next = clock.advance(FixedStepClock::tick_seconds * 0.75);
    REQUIRE(next.ticks == 1);
    REQUIRE(next.alpha == Catch::Approx(0.0).margin(1e-12));
    REQUIRE_FALSE(next.dropped_time);
}

TEST_CASE("FixedStepClock clamps negative elapsed time") {
    FixedStepClock clock;
    const auto result = clock.advance(-1.0);

    REQUIRE(result.ticks == 0);
    REQUIRE(result.alpha == Catch::Approx(0.0));
    REQUIRE_FALSE(result.dropped_time);
}
