#pragma once

namespace ac2::simulation {

struct Transform {
    float x{};
    float y{};
};

struct PreviousTransform {
    float x{};
    float y{};
};

struct MoveSpeed {
    float units_per_second{6.0F};
};

}  // namespace ac2::simulation
