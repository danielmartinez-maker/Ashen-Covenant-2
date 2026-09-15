#pragma once

#include <cstdint>
#include <vector>

namespace ac2::render {

struct SpriteInstance {
    float previous_x{};
    float previous_y{};
    float current_x{};
    float current_y{};
    std::uint32_t visual_id{};
};

struct PresentationSnapshot {
    std::vector<SpriteInstance> sprites;
};

}  // namespace ac2::render
