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

This document defines the initial production architecture for Ashen Covenant 2 (AC2). AC2 is a new native C++ codebase. It carries forward useful game-design lessons from Ashen Covenant without inheriting the original browser/Electron runtime architecture.

The first implementation milestone establishes a small, testable vertical foundation capable of supporting the full game without prematurely implementing every planned subsystem.

The architecture is built around six constraints:

1. Responsive action-RPG combat requires predictable timing and explicit gameplay rules.
2. Simulation logic must be testable without graphics, audio, or a window.
3. Presentation must support increasingly extravagant Covenant Metamorphosis without coupling rendering to gameplay authority.
4. Classes, skills, enemies, items, and balance values must be primarily data-authored while behavioral invariants remain native code.
5. The 300-level progression design requires validation and specialization constraints from the beginning.
6. Save compatibility, reproducibility where required, and regression resistance are production requirements.

## 2. Technology Decisions

### 2.1 C++20

C++20 is the baseline language standard.

Rules:

- RAII owns resources.
- Raw owning pointers are prohibited.
- Exceptions do not cross core game-loop boundaries; expected failures use explicit result/error types.
- Public gameplay interfaces avoid exposing SDL types.
- Public domain interfaces avoid exposing EnTT unless the interface is specifically an entity-storage adapter.
- Gameplay behavior favors explicit data and functions over deep inheritance hierarchies.

### 2.2 CMake

CMake is the canonical build system. Targets are separated by responsibility rather than compiling the project as one monolith.

Initial targets:

- `ac2_core` — dependency-light utilities and fixed-step timing primitives.
- `ac2_simulation` — headless gameplay simulation.
- `ac2_game` — game-domain orchestration.
- `ac2_platform_sdl` — SDL window, events, devices, and platform services.
- `ac2_render_sdlgpu` — GPU renderer and presentation adapters.
- `ashen_covenant_2` — desktop executable/composition root.
- `ac2_tests` — headless unit and simulation tests.

Third-party dependencies are isolated behind targets and pinned to known versions. Dependency fetching must be reproducible.

### 2.3 SDL3

SDL3 provides window/application lifecycle, keyboard/mouse/controller input, platform events, device integration, GPU window integration, and later audio-device integration. Gameplay code does not call SDL directly.

### 2.4 SDL3 GPU API

The initial renderer uses the SDL3 GPU API. It receives render-facing snapshots/commands instead of mutating gameplay state.

This boundary permits layered 2D characters, 2.5D depth/parallax, dynamic lighting, particles, post-processing, Covenant mutation overlays, and off-screen rendering without contaminating simulation logic.

### 2.5 EnTT

EnTT supplies runtime entity identity, component storage, and efficient iteration. It is infrastructure rather than the architecture itself.

Rules:

- Components contain state; systems contain behavior.
- Major game rules remain discoverable in named systems/services.
- Combat/system ordering cannot depend on an implicit event-subscriber order.
- Cross-domain changes use typed commands/events with documented ordering.
- Save files never serialize raw EnTT internals or transient entity identifiers.

## 3. Repository Structure

Intended root layout:

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

Directories are introduced only when real code requires them. Empty speculative modules are not created simply to match the diagram.

## 4. Dependency Direction

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

Rules:

- `core` depends on the C++ standard library only unless a dependency is explicitly justified.
- Gameplay domains cannot depend on renderer or windowing code.
- Renderer consumes presentation DTOs/snapshots generated from game state.
- Input translates device state into semantic player commands before simulation.
- Save code serializes stable domain records, not transient engine representation.
- The executable is the composition root and wires concrete adapters to interfaces.
- Circular target dependencies are forbidden.

## 5. Main Loop and Time Model

### 5.1 Fixed simulation

Gameplay simulation runs at **60 Hz** (`1/60 s` per logical step).

Desktop loop:

1. pump platform events,
2. sample devices,
3. translate sampled input to semantic commands,
4. accumulate real elapsed time,
5. execute zero or more fixed simulation ticks,
6. produce presentation state,
7. render with interpolation,
8. present.

Simulation never receives arbitrary wall-clock delta time.

### 5.2 Catch-up policy

The initial maximum catch-up budget is **five fixed simulation ticks per rendered frame**. When more than five ticks of accumulated time remain after a severe stall, the loop executes five ticks, discards excess accumulated whole-tick time, preserves the valid fractional remainder used for render interpolation, and records a diagnostic counter/message. This prevents the spiral of death and makes overload behavior testable.

The value may be retuned only from profiling evidence, with tests updated to preserve bounded behavior.

### 5.3 Pause and time scaling

Pause and gameplay time manipulation are simulation concepts. Wall-clock/platform timing does not become gameplay timing.

### 5.4 Reproducibility

The initial game is offline; deterministic networking is outside current scope. Reproducibility is still required for tests, combat validation, debugging traces, and save regression.

Therefore:

- simulation RNG uses explicit seeded streams,
- systems do not use global random generators,
- gameplay does not read wall-clock time,
- iteration order is explicit wherever ordering can alter an outcome,
- bit-identical floating-point determinism across unrelated architectures is not a milestone-1 promise,
- deterministic scenarios on the same supported toolchain/platform must reproduce their results.

## 6. Entity and Simulation Model

### 6.1 Entity identity

Runtime entities are EnTT entities. Persistent game objects receive stable domain IDs when persistence is required.

Potential components include transform, motion intent, faction, resources, combat statistics, collision, ability state, status effects, AI state, equipment links, Covenant state, and presentation descriptors. Milestone 1 implements only the components required by the vertical slice.

### 6.2 System ordering

Simulation execution order is defined centrally rather than inferred from file names or subscribers. Representative eventual order:

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

Only implemented systems participate, but their relative ordering is explicit.

### 6.3 Deferred structural mutations

Entity creation/destruction and structural changes during sensitive iteration are deferred through typed command buffers when needed. Flush points are defined by simulation phase.

## 7. Input Architecture

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

Simulation commands express game intent such as movement, aim, dodge, light/heavy attack, ability activation, and interaction. They never contain SDL scancodes.

This permits controller remapping, AI command injection, headless tests, and later replay tooling without emulating physical devices.

## 8. Combat Architecture

Combat remains explicit because it is AC2's primary feel-critical subsystem.

```text
Command
  → Ability request
  → Ability/state validation
  → authored attack timeline
  → active hit geometry / projectile / effect
  → HitIntent
  → defense/evasion/block/parry checks
  → DamageContext construction
  → ordered modifiers
  → damage/resource/status resolution
  → combat events
  → presentation events
```

Combat invariants:

- Hit intent and resolved outcome are different types/stages.
- One authoritative path resolves final damage.
- Modifier ordering is documented and testable.
- Hit identity can prevent unintended repeated hits.
- Animation/presentation cannot grant gameplay authority.
- Invulnerability/state gates are gameplay state.
- Covenant mutations operate through defined combat hooks rather than renderer special cases.

Milestone 1 avoids prematurely implementing the complete combat system.

## 9. Progression Architecture

AC2 supports **300 character levels and exactly one normal skill point earned at each level, including level 1**. A character reaching level 300 therefore earns **300 normal level-derived skill points** before any separately designed quest/reward bonuses. Any future bonus-point source must use an explicit separate rule and cannot silently alter this invariant.

The progression model supports:

- more purchasable nodes than obtainable points,
- prerequisites,
- rank caps,
- level gates,
- mutually exclusive choices,
- Covenant specialization requirements,
- respec validation,
- four intended viable build families per class without forcing named preset builds,
- deterministic build validation,
- save migration when graphs change.

Skill graphs are authored data. C++ validates graph structure and applies known effect primitives/behavior hooks.

A node definition conceptually contains stable ID, class ID, cost, max rank, prerequisites, gates/exclusions, effect descriptors, tags, and presentation metadata. Stable IDs are never derived from display names.

## 10. Covenant Architecture

Covenant Metamorphosis is a first-class domain system with three separable outputs:

1. **Gameplay mutation** — ability transformations, stats, resources, triggers, and rule changes.
2. **State/identity mutation** — Covenant allegiance, thresholds, specialization, and progression state.
3. **Presentation mutation** — silhouette extensions, particles, spectral structures, materials, audio layers, and screen effects.

Renderer receives presentation descriptors/events generated from Covenant state but cannot determine gameplay effects.

This supports a restrained normal class silhouette that gains supernatural extensions as Metamorphosis rises. Covenant specializations transform established class mechanics rather than replacing the base class with an unrelated implementation.

## 11. Data-Driven Content

Authored content includes classes, abilities, skill nodes, Covenant definitions, enemies, items, drop tables, and tuning values.

Initial data format is JSON. Production content categories receive validators before content depends on them at scale.

Principles:

- behavior primitives and safety invariants live in C++,
- authored combinations/tuning live in data,
- stable machine IDs are separate from display/localized strings,
- data load failures report file, record ID, field, and cause,
- invalid critical gameplay data prevents gameplay entry rather than silently falling back,
- tests validate graph and referential integrity.

A scripting language is explicitly deferred.

## 12. Rendering Architecture

Rendering uses a command/snapshot boundary. Game/simulation exposes presentation-safe data such as camera information, sprite/layer descriptors, interpolation transforms, animation state IDs, effect events, Covenant visual descriptors, and optional debug primitives.

Renderer owns GPU resources and draw translation. Logical resolution, batching, materials/pipelines, atlases, particles, lighting, render targets, and post-processing remain rendering concerns.

Milestone 1 renderer proves the architecture with window/device creation, clear/present, camera transform, and a minimal visible player/world primitive or sprite path.

## 13. Asset Architecture

Assets are addressed through stable logical asset IDs rather than arbitrary relative paths spread through gameplay code. Asset ownership is centralized. GPU resources are renderer-owned; source metadata is game/tooling data.

Milestone 1 can use placeholder/generated visual data so architecture work is independent of final art. Hot reload is deferred.

## 14. Save Architecture

Save compatibility begins at schema version 1.

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

- never serialize C++ memory layouts directly,
- never store raw pointers,
- never persist raw EnTT entity numeric values,
- persistent references use stable IDs,
- migrations are explicit and tested,
- production save writes use atomic/replace-safe semantics,
- corrupted/incompatible saves fail with actionable diagnostics.

Full save functionality is outside milestone 1, but no milestone-1 boundary may require raw runtime representation to become the persistent format.

## 15. Error Handling and Diagnostics

Errors are classified as programmer invariant failures, recoverable runtime failures, content validation failures, platform/graphics initialization failures, or persistence failures.

Debug builds fail loudly on programmer invariants. Recoverable production failures retain actionable context rather than being swallowed. Logging remains diagnostic and cannot be required for correctness.

## 16. Testing Strategy

Testability is an architectural constraint.

### 16.1 Headless tests

Gameplay tests run without SDL/GPU initialization unless they are explicitly platform/render integration tests.

Coverage grows with systems and includes:

- fixed-step accumulator and five-tick catch-up policy,
- input command mapping independent of devices,
- system sequencing,
- player movement,
- combat resolution,
- progression graph validation,
- 300-level/300-point budget rules,
- Covenant mutation rules,
- data/referential validation,
- save round trips/migrations,
- reproducible scenarios where appropriate.

### 16.2 Test-driven changes

New production behavior starts with a failing test whenever practically testable. Platform/GPU glue uses focused integration/smoke coverage instead of brittle unit mocks.

### 16.3 Desktop smoke path

Milestone desktop smoke must initialize, run the loop, simulate a player, render visible output, and shut down cleanly.

## 17. Continuous Integration

Initial matrix:

- Windows with MSVC.
- Linux with GCC or Clang.

CI performs configure, compile with warnings enabled, headless tests, and unsuppressed failure reporting. AC2-owned code moves to warnings-as-errors once third-party warning isolation is confirmed.

## 18. Performance Principles

Optimization must be measurement-driven.

Principles:

- avoid frame/tick heap churn in measured hot paths,
- use component storage appropriately,
- separate render extraction from simulation mutation,
- keep fixed-tick work bounded,
- profile before introducing caches or complex concurrency,
- retain representative performance scenarios as the game grows.

A custom job system is out of milestone-1 scope.

## 19. First Vertical Foundation

Milestone 1 is complete only when the repository has:

1. reproducible CMake configuration,
2. pinned SDL3 and EnTT dependencies,
3. native desktop executable,
4. SDL application/window lifecycle,
5. SDL3 GPU initialization/basic frame presentation,
6. fixed 60 Hz accumulator with five-tick catch-up cap,
7. headless `Simulation` entry point,
8. EnTT world/registry owned behind game-domain boundaries,
9. semantic input command model,
10. player entity/state,
11. basic player movement processed only by fixed ticks,
12. camera/presentation extraction,
13. renderer consuming presentation data instead of gameplay internals,
14. clean shutdown/resource destruction,
15. automated native tests for accumulator policy and player movement,
16. Windows and Linux CI,
17. developer README containing configure/build/test/run commands.

A debug primitive is acceptable for first player rendering. Final class artwork is outside this milestone.

## 20. Explicit Non-Goals for Milestone 1

Do not build the following unless an item above demonstrably requires it:

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

Interfaces should avoid blocking known future features, but speculative abstractions are not substitutes for requirements.

## 21. Architecture Acceptance Criteria

The foundation is acceptable only if:

- headless simulation compiles without SDL/GPU dependencies leaking into gameplay targets,
- executable creates a window, simulates, renders, and closes cleanly,
- simulation tick rate is fixed independently of render cadence,
- overload/catch-up policy is bounded and tested,
- tests drive player commands without a physical device,
- gameplay changes are testable without inspecting renderer output,
- renderer consumes a defined presentation boundary,
- target dependency graph contains no circular relationships,
- warnings/tests are never disabled to obtain green CI,
- architecture can be understood from target/module interfaces without knowledge of EnTT internals,
- implementation remains small enough to evolve after real combat/content constraints are learned.

## 22. Follow-On Sequence

After milestone 1, grow through tested vertical slices:

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

Each follow-on subsystem receives a focused design/implementation cycle when it materially changes architectural interfaces.

## 23. Guiding Rule

AC2 makes gameplay rules explicit, simulation testable, content authorable, and presentation free to become spectacular without becoming authoritative. The architecture should remain smaller than the game it supports.