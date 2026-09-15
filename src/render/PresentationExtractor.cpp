#include "render/PresentationExtractor.hpp"

#include "simulation/Simulation.hpp"

namespace ac2::render {

PresentationSnapshot extract_presentation(
    const ac2::simulation::Simulation& simulation) {
    const auto previous = simulation.player_previous_transform();
    const auto current = simulation.player_transform();

    PresentationSnapshot snapshot;
    snapshot.sprites.push_back(SpriteInstance{
        .previous_x = previous.x,
        .previous_y = previous.y,
        .current_x = current.x,
        .current_y = current.y,
        .visual_id = 1,
    });
    return snapshot;
}

}  // namespace ac2::render
