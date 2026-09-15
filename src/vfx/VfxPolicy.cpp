#include "vfx/VfxPolicy.h"

#include <algorithm>
#include <cmath>

namespace ac2::vfx {

std::uint8_t normalizeDirectionIndex(float degrees, DirectionPolicy policy) noexcept {
    if (policy == DirectionPolicy::Rotational1) return 0;
    float normalized = std::fmod(degrees, 360.0F);
    if (normalized < 0.0F) normalized += 360.0F;
    if (policy == DirectionPolicy::Octant8) return static_cast<std::uint8_t>(std::floor((normalized + 22.5F) / 45.0F)) % 8U;
    const auto cardinal = static_cast<std::uint8_t>(std::floor((normalized + 45.0F) / 90.0F)) % 4U;
    return static_cast<std::uint8_t>(cardinal * 2U);
}

void applyReadabilityPolicy(VfxSpawnCommand& command) noexcept {
    command.opacity = std::clamp(command.opacity, 0.0F, 1.0F);
    if (command.role == PresentationRole::EnemyTelegraph) return;
    if (command.role == PresentationRole::Friendly && command.persistent) command.opacity = std::min(command.opacity, 0.62F);
    if (command.persistent && hasTag(command.tags, "boss_silhouette_safe")) command.opacity = std::min(command.opacity, 0.45F);
    if (command.layer == VfxLayer::ScreenSpaceAccent) command.opacity = std::min(command.opacity, 0.80F);
}

} // namespace ac2::vfx
