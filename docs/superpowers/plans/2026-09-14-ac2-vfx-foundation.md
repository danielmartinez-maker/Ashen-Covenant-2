# AC2 Combat VFX Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a dependency-light C++20 AC2 combat-VFX domain and authored catalog that encodes the Complete Attack VFX Sprite Sheet Bible and emits deterministic renderer-facing spawn commands.

**Architecture:** Keep VFX presentation-only. `ac2_vfx` owns explicit vocabulary, validation, composition, readability and Metamorphosis presentation mutation; gameplay supplies presentation requests and the future SDL3 GPU renderer consumes spawn commands. The JSON catalog remains authored source data, with repository validation independent from GPU/runtime asset loading.

**Tech Stack:** C++20, CMake, CTest, Python 3 JSON validation, no SDL dependency in `ac2_vfx`.

**Spec:** `docs/superpowers/specs/2026-09-14-ac2-vfx-foundation-design.md`

## Global Constraints

- C++20.
- Gameplay/domain interfaces expose no SDL types.
- VFX never grants gameplay authority.
- Stable logical IDs, not arbitrary texture paths, cross gameplay/presentation boundaries.
- Final art is not fabricated; metadata and contracts must accept final sprite sheets later.
- Canonical layer order and readability rules from the user-authored bible are enforced.
- Physical impacts remain present when secondary elemental/Covenant/crit effects are composed.

---

### Task 1: Buildable VFX Domain Skeleton

**Files:**
- Create: `CMakeLists.txt`
- Create: `src/vfx/VfxTypes.h`
- Create: `src/vfx/VfxDefinition.h`
- Create: `tests/vfx/VfxTests.cpp`

**Interfaces:**
- Produces `VfxComponentType`, `DamageType`, `MagnitudeTier`, `DirectionPolicy`, `VfxLayer`, `MetamorphosisStage`, `VfxDefinition`, `VfxRequest`, and `VfxSpawnCommand`.

- [ ] Write compile-time/unit assertions for enum counts, canonical layer ranks, and valid basic construction.
- [ ] Configure CMake with `ac2_vfx` and `ac2_vfx_tests`, C++20, warnings, and CTest.
- [ ] Implement only the explicit value types needed by the tests.
- [ ] Configure/build/test with `cmake -S . -B build -DAC2_BUILD_TESTS=ON`, `cmake --build build`, and `ctest --test-dir build --output-on-failure`.
- [ ] Commit the buildable domain skeleton.

### Task 2: Validation and Canonical Naming

**Files:**
- Create: `src/vfx/VfxValidation.h`
- Create: `src/vfx/VfxValidation.cpp`
- Modify: `tests/vfx/VfxTests.cpp`

**Interfaces:**
- Produces `ValidationIssue`, `validateDefinition(const VfxDefinition&)`, `isCanonicalVfxId(std::string_view)`, `isPowerOfTwo(std::uint32_t)`.

- [ ] Add failing tests that accept `AC2_VFX_KNT_EARTHBREAKER_GROUND_A` and reject lowercase, whitespace, missing prefix, zero frames, reversed frame ranges, invalid atlas sizes, invalid dimensions, and loop metadata without persistent semantics.
- [ ] Implement minimal validation with actionable field/reason messages.
- [ ] Run the VFX test target and verify all validation tests pass.
- [ ] Commit validation.

### Task 3: Deterministic Composition and Layering

**Files:**
- Create: `src/vfx/VfxCatalog.h`
- Create: `src/vfx/VfxCatalog.cpp`
- Create: `src/vfx/VfxComposer.h`
- Create: `src/vfx/VfxComposer.cpp`
- Modify: `tests/vfx/VfxTests.cpp`

**Interfaces:**
- `VfxCatalog::add(VfxDefinition)` rejects duplicate/invalid definitions.
- `VfxCatalog::find(std::string_view)` returns a definition pointer or null.
- `VfxComposer::compose(const VfxRequest&)` returns `ComposeResult { commands, issues }`.

- [ ] Add failing tests for base+secondary additive composition, missing-ID diagnostics, canonical layer sorting, crit augmentation, and deterministic repeated composition.
- [ ] Implement catalog storage and composition without GPU/resource access.
- [ ] Run tests twice to guard stable command order.
- [ ] Commit composition.

### Task 4: Metamorphosis and Readability Policy

**Files:**
- Modify: `src/vfx/VfxDefinition.h`
- Create: `src/vfx/VfxPolicy.h`
- Create: `src/vfx/VfxPolicy.cpp`
- Modify: `src/vfx/VfxComposer.cpp`
- Modify: `tests/vfx/VfxTests.cpp`

**Interfaces:**
- `MetamorphosisAugments` stores stage II–V logical IDs.
- `applyReadabilityPolicy(VfxSpawnCommand&)` clamps friendly/persistent opacity and honors telegraph/screen-space tags.

- [ ] Add failing Stage I–V tests proving cumulative deterministic presentation mutation while Stage I emits only the base request.
- [ ] Add failing opacity/readability tests for friendly persistent fields, enemy telegraphs, and boss-silhouette-safe effects.
- [ ] Implement augmentation resolution and policy transforms.
- [ ] Run all headless tests.
- [ ] Commit policy/mutation support.

### Task 5: Complete Authored VFX Catalog

**Files:**
- Create: `assets/data/vfx/catalog.json`
- Create: `tools/validate_vfx_catalog.py`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Catalog records expose `id`, `group`, `component`, `frames`, `width`, `height`, `direction_policy`, `magnitude`, `layer`, `atlas`, `loop`, `persistent`, `tags`, and optional `damage_type`, `class_id`, `covenant_id`, `metamorphosis`.

- [ ] Author catalog entries for shared physical impacts, all damage impact families, elemental weapon loops/stretch/impact variants, every concrete class/Covenant effect named in the supplied bible, and reusable family entries where the bible defines families.
- [ ] Implement Python validation for schema fields, unique IDs, canonical names, enum values, frame/dimension bounds, power-of-two atlases, stage reference integrity, and required coverage groups.
- [ ] Add validator to CTest as `ac2_vfx_catalog_validation` when Python 3 is available.
- [ ] Run validator directly and through CTest.
- [ ] Commit catalog and validator.

### Task 6: Renderer Integration Contract

**Files:**
- Create: `src/vfx/VfxPresentation.h`
- Modify: `tests/vfx/VfxTests.cpp`
- Create: `docs/architecture/vfx-presentation-contract.md`

**Interfaces:**
- Defines presentation-only DTO fields required by a future SDL3 GPU adapter: logical effect ID, normalized direction/octant, layer, opacity, magnitude, source/target attachment intent, world/contact position placeholder type independent of SDL, and tags.

- [ ] Add compile/unit checks that DTOs contain no SDL/EnTT types and that direction normalization yields stable octants.
- [ ] Document how future renderer-owned atlas/resource lookup consumes logical IDs without becoming gameplay authority.
- [ ] Run the full suite.
- [ ] Commit integration contract.

### Task 7: Verification and Review

**Files:**
- Modify only files required by discovered verification defects.

- [ ] Run configure/build/test from a clean build directory.
- [ ] Run `python3 tools/validate_vfx_catalog.py assets/data/vfx/catalog.json`.
- [ ] Inspect git diff for generated artifacts, binary files, placeholders, weakened assertions, and accidental architecture drift.
- [ ] Verify the catalog contains all eight base class groups and all 24 Covenant specialization groups.
- [ ] Verify no final-art claim is made where only metadata exists.
- [ ] Record exact test outcomes in the PR description and leave the feature branch unmerged unless explicitly instructed.