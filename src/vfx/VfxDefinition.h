#pragma once

#include "vfx/VfxTypes.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ac2::vfx {

struct MetamorphosisAugments {
    std::vector<std::string> stage_ii;
    std::vector<std::string> stage_iii;
    std::vector<std::string> stage_iv;
    std::vector<std::string> stage_v;
};

struct VfxDefinition {
    std::string id;
    VfxComponentType component{VfxComponentType::Core};
    std::uint16_t width{};
    std::uint16_t height{};
    std::uint16_t frame_count{};
    std::uint16_t atlas_size{};
    DirectionPolicy direction_policy{DirectionPolicy::Rotational1};
    MagnitudeTier magnitude{MagnitudeTier::S};
    VfxLayer layer{VfxLayer::AttackCore};
    bool loop{};
    bool persistent{};
    float base_opacity{1.0F};
    std::optional<DamageType> damage_type;
    std::string class_id;
    std::string covenant_id;
    std::vector<std::string> tags;
    MetamorphosisAugments metamorphosis;
};

struct VfxRequest {
    std::string base_effect_id;
    std::vector<std::string> secondary_effect_ids;
    std::string crit_effect_id;
    float direction_degrees{};
    PresentationRole role{PresentationRole::Friendly};
    MetamorphosisStage metamorphosis_stage{MetamorphosisStage::I};
    std::optional<MagnitudeTier> magnitude_override;
    AttachmentIntent attachment{AttachmentIntent::ContactPoint};
    WorldPoint world_position{};
};

struct VfxSpawnCommand {
    std::string effect_id;
    VfxComponentType component{VfxComponentType::Core};
    VfxLayer layer{VfxLayer::AttackCore};
    MagnitudeTier magnitude{MagnitudeTier::S};
    PresentationRole role{PresentationRole::Friendly};
    AttachmentIntent attachment{AttachmentIntent::ContactPoint};
    WorldPoint world_position{};
    std::uint8_t direction_index{};
    float rotation_degrees{};
    float opacity{1.0F};
    bool persistent{};
    std::vector<std::string> tags;
};

} // namespace ac2::vfx
