#pragma once

#include "render/PresentationSnapshot.hpp"

namespace ac2::simulation {
class Simulation;
}

namespace ac2::render {

[[nodiscard]] PresentationSnapshot extract_presentation(
    const ac2::simulation::Simulation& simulation);

}  // namespace ac2::render
