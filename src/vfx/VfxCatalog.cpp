#include "vfx/VfxCatalog.h"

namespace ac2::vfx {

CatalogAddResult VfxCatalog::add(VfxDefinition definition) {
    auto issues = validateDefinition(definition);
    if (definitions_.contains(definition.id)) issues.push_back({"id", "duplicate VFX ID"});
    if (!issues.empty()) return {std::move(issues)};
    definitions_.emplace(definition.id, std::move(definition));
    return {};
}

const VfxDefinition* VfxCatalog::find(std::string_view id) const noexcept {
    const auto it = definitions_.find(std::string{id});
    return it == definitions_.end() ? nullptr : &it->second;
}

} // namespace ac2::vfx
