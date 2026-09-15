#pragma once

#include "vfx/VfxDefinition.h"

#include <type_traits>

namespace ac2::vfx {

// Presentation-only event suitable for extraction from gameplay into a renderer queue.
// No SDL or EnTT types are permitted in this contract.
struct VfxPresentationEvent {
    VfxRequest request;
    std::uint64_t presentation_sequence{};
};

static_assert(std::is_move_constructible_v<VfxPresentationEvent>);

} // namespace ac2::vfx
