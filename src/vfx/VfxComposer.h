#pragma once

#include "vfx/VfxCatalog.h"

#include <string>
#include <vector>

namespace ac2::vfx {

struct ComposeResult { std::vector<VfxSpawnCommand> commands; std::vector<std::string> issues; };

class VfxComposer {
public:
    explicit VfxComposer(const VfxCatalog& catalog) : catalog_(catalog) {}
    ComposeResult compose(const VfxRequest& request) const;
private:
    const VfxCatalog& catalog_;
};

} // namespace ac2::vfx
