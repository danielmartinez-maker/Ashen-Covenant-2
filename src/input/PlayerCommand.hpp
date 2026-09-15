#pragma once

namespace ac2::input {

struct PlayerCommand {
    float move_x{};
    float move_y{};
    bool dodge_pressed{};
    bool light_attack_pressed{};
    bool heavy_attack_pressed{};
};

}  // namespace ac2::input
