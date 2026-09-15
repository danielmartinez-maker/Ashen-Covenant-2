#include "vfx/VfxComposer.h"
#include "vfx/VfxPolicy.h"

#include <algorithm>

namespace ac2::vfx {
namespace {
void appendIdsForStage(std::vector<std::string>& ids, const MetamorphosisAugments& a, MetamorphosisStage stage) {
    const auto append = [&ids](const std::vector<std::string>& values) { ids.insert(ids.end(), values.begin(), values.end()); };
    if (static_cast<int>(stage) >= 2) append(a.stage_ii);
    if (static_cast<int>(stage) >= 3) append(a.stage_iii);
    if (static_cast<int>(stage) >= 4) append(a.stage_iv);
    if (static_cast<int>(stage) >= 5) append(a.stage_v);
}
}

ComposeResult VfxComposer::compose(const VfxRequest& request) const {
    ComposeResult result;
    std::vector<std::string> ids{request.base_effect_id};
    ids.insert(ids.end(), request.secondary_effect_ids.begin(), request.secondary_effect_ids.end());
    if (!request.crit_effect_id.empty()) ids.push_back(request.crit_effect_id);
    if (const auto* base = catalog_.find(request.base_effect_id)) appendIdsForStage(ids, base->metamorphosis, request.metamorphosis_stage);

    for (const auto& id : ids) {
        const auto* definition = catalog_.find(id);
        if (definition == nullptr) { result.issues.push_back("missing VFX definition: " + id); continue; }
        VfxSpawnCommand command;
        command.effect_id = definition->id;
        command.component = definition->component;
        command.layer = definition->layer;
        command.magnitude = request.magnitude_override.value_or(definition->magnitude);
        command.role = request.role;
        command.attachment = request.attachment;
        command.world_position = request.world_position;
        command.direction_index = normalizeDirectionIndex(request.direction_degrees, definition->direction_policy);
        command.rotation_degrees = definition->direction_policy == DirectionPolicy::Rotational1 ? request.direction_degrees : 0.0F;
        command.opacity = definition->base_opacity;
        command.persistent = definition->persistent;
        command.tags = definition->tags;
        applyReadabilityPolicy(command);
        result.commands.push_back(std::move(command));
    }
    std::stable_sort(result.commands.begin(), result.commands.end(), [](const auto& a, const auto& b) { return layerRank(a.layer) < layerRank(b.layer); });
    return result;
}

} // namespace ac2::vfx
