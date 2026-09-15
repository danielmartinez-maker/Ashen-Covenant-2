#pragma once

#include <cstdint>
#include <memory>

#include "input/PlayerCommand.hpp"
#include "simulation/Components.hpp"

namespace ac2::simulation {

class Simulation {
public:
    Simulation();
    ~Simulation();

    Simulation(Simulation&&) noexcept;
    Simulation& operator=(Simulation&&) noexcept;
    Simulation(const Simulation&) = delete;
    Simulation& operator=(const Simulation&) = delete;

    void tick(const ac2::input::PlayerCommand& command);

    [[nodiscard]] Transform player_transform() const;
    [[nodiscard]] Transform player_previous_transform() const;
    [[nodiscard]] std::uint64_t tick_index() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace ac2::simulation
