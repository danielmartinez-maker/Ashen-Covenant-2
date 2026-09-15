#pragma once

#include <cstdint>

namespace ac2::core {

struct StepResult {
    std::uint32_t ticks{};
    double alpha{};
    bool dropped_time{};
};

class FixedStepClock {
public:
    static constexpr double tick_seconds = 1.0 / 60.0;
    static constexpr std::uint32_t max_catch_up_ticks = 5;

    [[nodiscard]] StepResult advance(double elapsed_seconds) noexcept;
    void reset() noexcept;

private:
    double accumulator_seconds_{};
};

}  // namespace ac2::core
