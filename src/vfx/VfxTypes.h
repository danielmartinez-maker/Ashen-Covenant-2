#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace ac2::vfx {

enum class VfxComponentType : std::uint8_t { Anticipation, Trail, Core, Impact, Debris, Ground, Projectile, Muzzle, Explosion, Aura, Field, Status, Dissipation, Metamorphosis };
enum class DamageType : std::uint8_t { Slash, Pierce, Blunt, Fire, Frost, Lightning, Poison, Blood, Holy, Shadow, Void, Necrotic, Nature, Arcane, Demonic };
enum class MagnitudeTier : std::uint8_t { S, M, L, XL };
enum class DirectionPolicy : std::uint8_t { Rotational1, Cardinal4, Octant8 };
enum class MetamorphosisStage : std::uint8_t { I = 1, II = 2, III = 3, IV = 4, V = 5 };
enum class PresentationRole : std::uint8_t { Friendly, Enemy, EnemyTelegraph, Neutral };
enum class AttachmentIntent : std::uint8_t { World, Source, Target, ContactPoint };

enum class VfxLayer : std::uint8_t {
    GroundDecal,
    GroundEffect,
    RearField,
    CharacterShadow,
    Character,
    RearWeaponTrail,
    AttackCore,
    FrontWeaponTrail,
    Projectile,
    ImpactCore,
    Debris,
    MaterialResponse,
    CovenantOverlay,
    Status,
    ForegroundParticles,
    ScreenSpaceAccent
};

constexpr std::uint8_t layerRank(VfxLayer layer) noexcept { return static_cast<std::uint8_t>(layer); }

struct WorldPoint { float x{}; float y{}; float z{}; };
std::uint8_t normalizeDirectionIndex(float degrees, DirectionPolicy policy) noexcept;

inline bool hasTag(const std::vector<std::string>& tags, std::string_view tag) {
    for (const auto& value : tags) if (value == tag) return true;
    return false;
}

} // namespace ac2::vfx
