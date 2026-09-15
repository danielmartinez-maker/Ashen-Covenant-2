# Ashen Covenant 2 — Foundation Architecture Design

**Status:** Approved design baseline  
**Date:** 2026-09-14  
**Target:** Native desktop action RPG  
**Language:** C++20  
**Build:** CMake  
**Platform layer:** SDL3  
**Rendering:** SDL3 GPU API  
**Entity storage:** EnTT

## 1. Purpose

This document defines the initial production architecture for Ashen Covenant 2 (AC2). AC2 is a new native C++ codebase. It carries forward the useful game-design lessons of Ashen Covenant without inheriting the original browser/Electron runtime architecture.

The first implementation milestone must establish a small, testable vertical foundation that can support the full game without prematurely implementing every planned subsystem.

The architecture is designed around six constraints:

1. Responsive action-RPG combat requires predictable timing and explicit gameplay rules.
2. Simulation logic must be testable without graphics, audio, or a window.
3. Presentation must be able to become visually extravagant through Covenant Metamorphosis without coupling rendering to gameplay state transitions.
4. Classes, skills, enemies, items, and balance values must be primarily data-authored while behavioral invariants remain native code.
5. The 300-level progression design requires validation and specialization constraints from the beginning.
6. Save compatibility, deterministic behavior where required, and regression resistance are production requirements rather than cleanup tasks.

## 2. Technology Decisions

### 2.1 C++20

C++20 is the baseline language standard. Newer language/library features may be adopted only when all supported CI compilers provide reliable support.

Rules:

- RAII owns resources.
- Raw owning pointers are prohibited.
- Exceptions do not cross core game-loop boundaries; expected failures use explicit result/error types.
- Public module interfaces avoid exposing SDL or EnTT types unless the module is explicitly an adapter for that library.
- Gameplay behavior favors explicit data and functions over deep inheritance hierarchies.

### 2.2 CMake

CMake is the canonical build system. Targets are separated by responsibility rather than compiling the entire project as one monolith.

Initial targets:

- `ac2_core` — low-level dependency-free utilities and timing primitives.
- `ac2_simulation` — headless deterministic-capable gameplay simulation.
- `ac2_game` — game-domain orchestration and authored systems.
- `ac2_platform_sdl` — SDL window, events, devices, and platform services.
- `ac2_render_sdlgpu` — GPU renderer and presentation adapters.
- `ashen_covenant_2` — desktop executable/composition root.
- `ac2_tests` — headless unit and simulation tests.

Third-party dependencies must be isolated behind targets and pinned to known versions. Dependency fetching should be reproducible.

### 2.3 SDL3

SDL3 provides:

- window/application lifecycle,
- keyboard/mouse/controller input,
- platform events,
- timing primitives where appropriate,
- GPU device/window integration,
- audio/device integration when the audio layer is introduced.

Gameplay code must not call SDL directly.

### 2.4 SDL3 GPU API

The initial renderer uses the SDL3 GPU API. The renderer receives immutable render-facing snapshots/commands rather than reading and mutating gameplay state directly.

This boundary exists so rendering can later support:

- layered 2D characters,
- 2.5D depth and parallax,
- dynamic lights,
- particles,
- post-processing,
- Covenant mutation overlays,
- off-screen rendering and scaling,

without contaminating simulation logic.

### 2.5 EnTT

EnTT supplies entity identity, component storage, and efficient iteration. EnTT is infrastructure, not the game architecture itself.

Rules:

- Components contain state; systems contain behavior.
- Major game rules must remain discoverable in named systems/services.
- No generic event soup that makes combat ordering implicit.
- Cross-domain changes use typed commands/events with documented ordering.
- Save files never serialize raw EnTT internals or transient entity identifiers.

## 3. Repository Structure

The intended root layout is:

```text
Ashen-Covenant-2/
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── LICENSE
├── .gitignore
├── cmake/
├── docs/
│   ├── architecture/
│   └── superpowers/
│       ├── specs/
│       └── plans/
├── assets/
│   ├── data/
│   ├── shaders/
│   ├── textures/
│   ├── audio/
│   └── fonts/
├── src/
│   ├── app/
│   ├── core/
│   ├── platform/
│   ├── engine/
│   ├── simulation/
│   ├── entities/
│   ├── combat/
│   ├── progression/
│   ├── covenants/
│   ├── world/
│   ├── ai/
│   ├── input/
│   ├── render/
│   ├── audio/
│   ├── assets/
│   ├── save/
│   └── game/
├── tests/
│   ├── core/
│   ├── simulation/
│   ├── combat/
│   ├── progression/
│   ├── covenants/
│   └── save/
└── tools/
```

Directories are introduced only as real code requires them. Empty speculative modules should not be created merely to match this diagram.

## 4. Dependency Direction

Dependency direction is intentionally strict.

```text
app/composition
      |
      +--> game ------------------------------+
      |      |                                |
      |      +--> simulation                  |
      |      +--> combat                      |
      |      +--> progression                 |
      |      +--> covenants                   |
      |      +--> world / ai                  |
      |                                       |
      +--> platform adapters                  |
      +--> render adapters                    |
      +--> audio adapters                     |
                                              |
core <-----------------------------------------+
```

Additional rules:

- `core` depends on the C++ standard library only unless a dependency is explicitly justified.
- gameplay domains cannot depend on renderer or windowing code.
- renderer may consume presentation DTOs/snapshots generated from game state.
- input translates device state into semantic player commands before entering simulation.
- save code serializes stable domain records, not transient engine representation.
- the executable is the composition root and is responsible for wiring concrete adapters to interfaces.

Circular target dependencies are forbidden.

## 5. Main Loop and Time Model

### 5.1 Fixed simulation

Gameplay simulation runs at **60 Hz** (`1/60 s` logical step).

The desktop loop performs:

1. pump platform events,
2. sample devices,
3. translate sampled input to semantic commands,
4. accumulate real elapsed time,
5. execute zero or more fixed simulation ticks,
6. produce presentation state,
7. render with interpolation,
8. present.

The simulation never receives arbitrary wall-clock delta time.

### 5.2 Spiral-of-death protection

The accumulator has a maximum catch-up budget. If the application stalls severely, AC2 records telemetry/debug diagnostics and prevents an unbounded tick loop. The exact production cap is tuned after profiling; tests must cover the cap behavior.

### 5.3 Pause and time scaling

Pause and gameplay time manipulation are simulation concepts. Wall-clock/platform timing does not become gameplay timing.

### 5.4 Determinism

The initial game is offline and deterministic networking is out of scope. However, reproducibility is valuable for tests, replays/debug traces, combat validation, and save regression.

Therefore:

- simulation RNG uses explicit seeded streams,
- systems do not use global random generators,
- gameplay does not read wall-clock time,
- iteration order must be explicit where ordering can change outcomes,
- floating-point determinism across unrelated architectures is not promised at milestone 1,
- deterministic test scenarios on the same supported toolchain/platform must be reproducible.

## 6. Entity and Simulation Model

### 6.1 Entity identity

Runtime entities are EnTT entities. Persistent game objects receive stable domain IDs when persistence is required.

Typical components will eventually include:

- transform,
- velocity/motion intent,
- faction/team,
- health/resource pools,
- combat statistics,
- collision shape,
- ability state,
- status effects,
- AI state,
- inventory/equipment links,
- Covenant state,
- presentation descriptor.

The milestone implements only components needed by the first vertical slice.

### 6.2 System ordering

Simulation execution order is defined centrally rather than inferred from file names or event subscribers. A representative eventual order is:

1. consume commands,
2. update state gates/status timers,
3. resolve movement intent,
4. resolve collision/spatial constraints,
5. advance abilities/attacks,
6. create hit intents,
7. resolve hits/damage/defense,
8. resolve deaths/triggers,
9. update AI/world reactions,
10. emit presentation events,
11. finalize deferred entity changes.

Actual systems are added only when implemented, but ordering remains explicit.

### 6.3 Deferred structural mutations

Creating/destroying entities or making structural changes during sensitive iterations is deferred through typed command buffers when necessary. The flush point is defined by simulation phase.

## 7. Input Architecture

SDL device events are converted into a game-owned input model.

Layers:

```text
SDL events/device state
        ↓
Platform input adapter
        ↓
Input snapshot
        ↓
Bindings / action map
        ↓
PlayerCommand frame
        ↓
Simulation
```

Simulation commands express intent such as move, aim, dodge, light attack, heavy attack, ability slot activation, interaction, or menu intent. They do not contain SDL scancodes.

This permits controller remapping, AI command injection, deterministic tests, and later replay tooling without simulating physical devices.

## 8. Combat Architecture

Combat must remain explicit because it is AC2's primary feel-critical subsystem.

The eventual combat pipeline is:

```text
Command
  → Ability request
  → Ability/state validation
  → authored attack timeline
  → active hit geometry / projectile / effect
  → HitIntent
  → defense/evasion/block/parry checks
  → DamageContext construction
  → modifiers
  → damage/resource/status resolution
  → combat events
  → presentation events
```

Key data types should distinguish intent from resolved outcomes. A hit request must not directly subtract health.

Combat invariants include:

- one authoritative place resolves final damage,
- modifier ordering is documented and testable,
- hit identity can prevent unintended repeated hits,
- animation/presentation cannot grant gameplay authority,
- invulnerability and state gates live in gameplay state,
- Covenant mutations modify defined combat hooks rather than special-casing the renderer.

Milestone 1 establishes interfaces/types only where required; it does not prematurely implement the complete combat system.

## 9. Progression Architecture

AC2 supports **300 character levels** with **one skill point per level**, subject to the final level-1 allocation rule defined by game design.

The progression model must support:

- more purchasable nodes than total obtainable points,
- prerequisites,
- rank caps,
- level gates,
- mutually exclusive choices,
- Covenant specialization requirements,
- respec validation,
- four intended viable build families per class without hard-locking players into named presets,
- deterministic validation of a build from data,
- future migration when skill graphs change between save versions.

Skill graphs are authored data. C++ validates graph structure and applies known effect primitives/behavior hooks.

A skill node definition conceptually contains:

```text
stable id
class id
cost
max rank
prerequisites
gates/exclusions
effect descriptors
tags
presentation metadata
```

Stable IDs are never derived from display names.

## 10. Covenant Architecture

Covenant Metamorphosis is a first-class domain system.

It has three separable outputs:

1. **Gameplay mutation** — ability transformations, stats, resources, triggers, rule changes.
2. **State/identity mutation** — Covenant allegiance, thresholds, specialization, progression state.
3. **Presentation mutation** — silhouette extensions, particles, spectral structures, materials, audio layers, screen effects.

The renderer receives presentation descriptors/events generated from Covenant state but cannot determine gameplay effects.

This supports the visual direction in which ordinary characters maintain recognizable class silhouettes while higher Metamorphosis increasingly extends supernatural elements outside those silhouettes.

Covenant specializations transform a class's established mechanics rather than replacing the base class with an unrelated implementation.

## 11. Data-Driven Content

Authored content includes classes, abilities, skill nodes, Covenant definitions, enemies, items, drop tables, and tuning values.

Initial data format is JSON unless implementation experience demonstrates a specific limitation. Schemas/validators are mandatory for production content categories.

Principles:

- behavior primitives and safety invariants live in C++,
- authored combinations and tuning live in data,
- stable machine IDs are separate from localized/display strings,
- data load failures report file, record ID, field, and cause,
- invalid critical gameplay data prevents entering gameplay rather than silently falling back,
- tests validate graph integrity and referential integrity.

A scripting language is explicitly deferred.

## 12. Rendering Architecture

Rendering uses a command/snapshot model.

The simulation/game layer exposes presentation-safe data such as:

- camera target information,
- sprite/layer descriptors,
- transforms and interpolation states,
- animation state IDs,
- effect spawn events,
- Covenant visual descriptors,
- debug primitives when enabled.

The renderer owns GPU resources and translation into draw work.

Rendering concerns may include logical resolution, scaling, render targets, batching, materials/pipelines, sprite atlases, particles, lighting, and post-processing. These do not become simulation dependencies.

The first renderer only needs enough capability to prove the architecture: window/device creation, clear/present, camera transform, and a minimal visible player/world primitive or sprite path.

## 13. Asset Architecture

Assets are addressed through stable logical asset IDs rather than arbitrary relative paths spread through gameplay code.

Asset ownership is centralized. GPU resources are renderer-owned; source asset metadata is game/tooling data.

Milestone 1 may use placeholder/generated visual data so architecture work is not blocked on final art.

Hot reload is desirable for authored content but is not a milestone-1 requirement.

## 14. Save Architecture

Save compatibility starts with version 1.

A save contains an explicit schema version. Loading follows:

```text
bytes/file
  → parse
  → validate envelope
  → migrate old schema(s)
  → validate current schema
  → construct stable domain records
  → instantiate runtime state
```

Rules:

- never serialize memory layouts directly,
- never store raw pointers,
- never depend on EnTT entity numeric values for persistence,
- persistent references use stable IDs,
- migrations are explicit and tested,
- save writes eventually use atomic/replace-safe semantics,
- corrupted or incompatible saves fail with actionable diagnostics.

A fully featured save system is not required in milestone 1, but the domain boundaries must not make versioned serialization difficult later.

## 15. Error Handling and Diagnostics

Errors are classified broadly as:

- programmer invariant failures,
- recoverable runtime failures,
- content validation failures,
- platform/graphics initialization failures,
- persistence failures.

Debug builds should fail loudly on violated invariants. Production failures should retain useful context instead of swallowing errors.

Logging categories will eventually include platform, render, simulation, combat, data, save, and performance. Logging must not become a hidden dependency required for correctness.

## 16. Testing Strategy

Testability is an architectural constraint.

### 16.1 Headless tests

Most gameplay tests run without SDL initialization or a GPU.

Required categories as systems arrive:

- core utilities/time,
- fixed-step accumulator behavior,
- input command mapping independent of devices,
- entity/system sequencing,
- combat resolution,
- progression graph validation,
- 300-level point-budget constraints,
- Covenant mutation rules,
- data validation,
- save round trips and migrations,
- deterministic scenario hashes/snapshots where appropriate.

### 16.2 Test-driven changes

New production behavior begins with a failing test whenever the behavior is practically testable. Platform/GPU glue receives focused integration/smoke coverage rather than forcing brittle unit mocks.

### 16.3 Smoke test

The milestone desktop smoke path must initialize, run the loop, simulate a player, render a visible frame, and shut down cleanly.

## 17. Continuous Integration

Initial CI target matrix:

- Windows — MSVC, current supported runner.
- Linux — GCC or Clang, current supported runner.

CI performs at minimum:

1. configure,
2. compile with warnings enabled,
3. run headless tests,
4. report failures without suppressing compiler/test output.

Warnings should be treated strictly for AC2-owned targets once third-party warning isolation is established.

Formatting/static analysis may be added incrementally; they must not block initial architecture on tooling churn.

## 18. Performance Principles

No optimization by intuition.

Initial principles:

- avoid per-frame heap churn in hot paths where measurement shows significance,
- use contiguous component storage appropriately,
- separate render extraction from simulation mutation,
- keep fixed-tick work bounded,
- profile before introducing caches or complex job systems,
- maintain representative performance scenarios as the project grows.

A custom multithreaded job system is explicitly out of scope for milestone 1.

## 19. First Vertical Foundation

Milestone 1 is complete when the repository contains a functioning architecture slice with:

1. reproducible CMake configuration,
2. pinned SDL3 and EnTT dependencies,
3. native desktop executable,
4. SDL application/window lifecycle,
5. SDL3 GPU initialization and basic frame presentation,
6. fixed 60 Hz simulation accumulator,
7. headless `Simulation` entry point,
8. EnTT world/registry owned behind game-domain boundaries,
9. semantic input command model,
10. player entity/state,
11. basic player movement processed by fixed ticks,
12. camera/presentation extraction,
13. renderer consuming presentation data rather than gameplay internals,
14. clean shutdown/resource destruction,
15. native automated tests for fixed-step timing and player movement,
16. CI for Windows and Linux,
17. developer README with configure/build/test/run commands.

A colored/debug primitive is acceptable for the first player rendering path. Final class artwork is outside this architecture milestone.

## 20. Explicit Non-Goals for Milestone 1

Do not build these during foundation work unless they become necessary to satisfy an item above:

- final combat implementation,
- complete class kits,
- full 300-level skill graphs,
- final Covenant specializations,
- production enemy AI,
- final physics/collision engine,
- multiplayer/networking,
- scripting VM,
- custom editor,
- custom job system,
- final asset pipeline,
- complete save UI,
- localization framework,
- mod SDK,
- final graphical effects.

Interfaces should avoid blocking these features, but speculative abstractions are not a substitute for requirements.

## 21. Architecture Acceptance Criteria

The foundation is acceptable only if:

- headless simulation compiles without SDL/GPU dependencies leaking into gameplay targets,
- the executable can create a window, simulate, render, and close cleanly,
- simulation tick rate is fixed independently of render cadence,
- tests can drive player commands without a physical keyboard/controller,
- gameplay state changes can be verified without inspecting renderer output,
- renderer consumes a defined presentation boundary,
- dependency graph has no circular module relationships,
- warnings/tests are not disabled to obtain green CI,
- architecture remains understandable from target/module interfaces without requiring knowledge of EnTT internals,
- implementation remains deliberately small enough to evolve after real combat and content constraints are discovered.

## 22. Follow-On Sequence

After milestone 1, architecture should grow through tested vertical slices rather than broad empty scaffolding. The recommended sequence is:

1. combat kernel and hit/damage context,
2. animation/state synchronization and attack timelines,
3. collision/spatial queries,
4. first complete class combat loop,
5. data validation/content loading,
6. progression graph kernel,
7. Covenant mutation kernel,
8. enemy AI/combatant loop,
9. loot/equipment,
10. versioned save implementation,
11. world/encounter state,
12. presentation depth, effects, audio, and tooling driven by demonstrated needs.

Each follow-on subsystem requires its own focused design/implementation cycle when architectural interfaces materially change.

## 23. Guiding Rule

AC2 should make gameplay rules explicit, simulation testable, content authorable, and presentation free to become spectacular without becoming authoritative. The architecture should remain smaller than the game it supports.