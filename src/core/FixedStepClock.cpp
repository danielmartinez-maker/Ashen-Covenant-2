#include "core/FixedStepClock.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace ac2::core {
namespace {

constexpr double tick_epsilon = FixedStepClock::tick_seconds * 1.0e-9;

}  // namespace

StepResult FixedStepClock::advance(double elapsed_seconds) noexcept {
    if (std::isfinite(elapsed_seconds) && elapsed_seconds > 0.0) {
        accumulator_seconds_ += elapsed_seconds;
    }

    const double available_ticks_value =
        std::floor((accumulator_seconds_ + tick_epsilon) / tick_seconds);
    const auto available_ticks = available_ticks_value > 0.0
        ? static_cast<std::uint64_t>(available_ticks_value)
        : std::uint64_t{0};

    const bool dropped_time = available_ticks > max_catch_up_ticks;
    const auto ticks = static_cast<std::uint32_t>(
        std::min<std::uint64_t>(available_ticks, max_catch_up_ticks));

    if (dropped_time) {
        accumulator_seconds_ = std::fmod(accumulator_seconds_, tick_seconds);
    } else {
        accumulator_seconds_ -= static_cast<double>(ticks) * tick_seconds;
    }

    if (std::abs(accumulator_seconds_) <= tick_epsilon) {
        accumulator_seconds_ = 0.0;
    }

    const double max_alpha = std::nextafter(1.0, 0.0);
    const double alpha = std::clamp(
        accumulator_seconds_ / tick_seconds, 0.0, max_alpha);

    return StepResult{ticks, alpha, dropped_time};
}

void FixedStepClock::reset() noexcept {
    accumulator_seconds_ = 0.0;
}

}  // namespace ac2::core
