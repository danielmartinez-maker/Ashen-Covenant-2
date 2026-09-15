#include "simulation/Simulation.hpp"

#include <memory>

#include <entt/entt.hpp>

#include "simulation/MovementSystem.hpp"

namespace ac2::simulation {

struct Simulation::Impl {
    entt::registry registry;
    entt::entity player{entt::null};
    std::uint64_t tick_index{};
};

Simulation::Simulation()
    : impl_(std::make_unique<Impl>()) {
    impl_->player = impl_->registry.create();
    impl_->registry.emplace<Transform>(impl_->player, Transform{});
    impl_->registry.emplace<PreviousTransform>(impl_->player, PreviousTransform{});
    impl_->registry.emplace<MoveSpeed>(impl_->player, MoveSpeed{6.0F});
}

Simulation::~Simulation() = default;
Simulation::Simulation(Simulation&&) noexcept = default;
Simulation& Simulation::operator=(Simulation&&) noexcept = default;

void Simulation::tick(const ac2::input::PlayerCommand& command) {
    const auto& current = impl_->registry.get<Transform>(impl_->player);
    auto& previous = impl_->registry.get<PreviousTransform>(impl_->player);
    previous.x = current.x;
    previous.y = current.y;

    apply_movement(impl_->registry, impl_->player, command);
    ++impl_->tick_index;
}

Transform Simulation::player_transform() const {
    return impl_->registry.get<Transform>(impl_->player);
}

Transform Simulation::player_previous_transform() const {
    const auto& previous = impl_->registry.get<PreviousTransform>(impl_->player);
    return Transform{previous.x, previous.y};
}

std::uint64_t Simulation::tick_index() const noexcept {
    return impl_->tick_index;
}

}  // namespace ac2::simulation
