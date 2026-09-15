#pragma once

#include "vfx/VfxDefinition.h"

#include <string>
#include <string_view>
#include <vector>

namespace ac2::vfx {
struct ValidationIssue { std::string field; std::string reason; };
bool isCanonicalVfxId(std::string_view id) noexcept;
bool isPowerOfTwo(std::uint32_t value) noexcept;
std::vector<ValidationIssue> validateDefinition(const VfxDefinition& definition);
} // namespace ac2::vfx
