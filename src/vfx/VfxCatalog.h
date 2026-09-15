#pragma once

#include "vfx/VfxDefinition.h"
#include "vfx/VfxValidation.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ac2::vfx {

struct CatalogAddResult {
    std::vector<ValidationIssue> issues;
    bool ok() const noexcept { return issues.empty(); }
};

class VfxCatalog {
public:
    CatalogAddResult add(VfxDefinition definition);
    const VfxDefinition* find(std::string_view id) const noexcept;
    std::size_t size() const noexcept { return definitions_.size(); }
private:
    std::unordered_map<std::string, VfxDefinition> definitions_;
};

} // namespace ac2::vfx
