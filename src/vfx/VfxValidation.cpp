#include "vfx/VfxValidation.h"

#include <algorithm>

namespace ac2::vfx {

bool isCanonicalVfxId(std::string_view id) noexcept {
    constexpr std::string_view prefix = "AC2_VFX_";
    if (!id.starts_with(prefix) || id.size() <= prefix.size()) return false;
    if (id.back() == '_' || id.find("__") != std::string_view::npos) return false;
    return std::all_of(id.begin(), id.end(), [](unsigned char c) {
        return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
    });
}

bool isPowerOfTwo(std::uint32_t value) noexcept { return value != 0 && (value & (value - 1U)) == 0U; }

std::vector<ValidationIssue> validateDefinition(const VfxDefinition& d) {
    std::vector<ValidationIssue> issues;
    if (!isCanonicalVfxId(d.id)) issues.push_back({"id", "must use canonical AC2_VFX uppercase token naming"});
    if (d.width == 0 || d.width > 4096) issues.push_back({"width", "must be between 1 and 4096"});
    if (d.height == 0 || d.height > 4096) issues.push_back({"height", "must be between 1 and 4096"});
    if (d.frame_count == 0 || d.frame_count > 64) issues.push_back({"frame_count", "must be between 1 and 64"});
    if (!isPowerOfTwo(d.atlas_size) || d.atlas_size < 256 || d.atlas_size > 4096) issues.push_back({"atlas_size", "must be a power of two between 256 and 4096"});
    if (d.base_opacity <= 0.0F || d.base_opacity > 1.0F) issues.push_back({"base_opacity", "must be in (0, 1]"});
    if (d.loop && !d.persistent) issues.push_back({"loop", "looping effects must be persistent"});
    if (hasTag(d.tags, "screen_space") && d.layer != VfxLayer::ScreenSpaceAccent) issues.push_back({"layer", "screen_space effects must use ScreenSpaceAccent"});
    return issues;
}

} // namespace ac2::vfx
