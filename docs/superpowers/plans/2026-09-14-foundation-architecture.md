# Ashen Covenant 2 Foundation Architecture Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Produce the first native AC2 vertical foundation: a reproducible C++20 build, headless fixed-step simulation, semantic input, player movement, SDL3 desktop shell, SDL3 GPU presentation path, and Windows/Linux CI.

**Architecture:** The executable is a composition root. Simulation is headless and owns gameplay state; SDL adapters translate device/platform concerns into game-owned commands and immutable presentation snapshots. The first milestone proves dependency direction and deterministic timing without prematurely implementing combat, progression, Covenant rules, saves, audio, or a content editor.

**Tech Stack:** C++20, CMake 3.28+, SDL3, SDL3 GPU API, EnTT, Catch2 v3, GitHub Actions.

**Spec:** `docs/superpowers/specs/2026-09-14-ac2-foundation-design.md`

## Global Constraints

- Gameplay simulation runs at exactly 60 Hz.
- A rendered frame may execute at most five catch-up simulation ticks; excess accumulated time is discarded and reported.
- Level progression contract remains 300 levels and 300 normally earned skill points; progression behavior is outside this milestone.
- Gameplay code cannot depend on SDL or rendering headers.
- SDL and EnTT are isolated behind project targets; EnTT runtime IDs are never persistence IDs.
- Simulation receives semantic commands, never SDL scancodes or wall-clock delta time.
- Rendering consumes presentation data and cannot mutate gameplay authority.
- No scripting VM, networking, custom physics engine, job system, editor, or premature combat/progression/save implementation.
- New testable production behavior follows RED -> GREEN -> REFACTOR.
- Do not merge this branch as part of this plan.

---

## File Map

- `CMakeLists.txt`: root project, dependency pins, target graph, test registration.
- `CMakePresets.json`: canonical developer/CI configure and build presets.
- `.gitignore`: generated CMake/build artifacts only.
- `README.md`: bootstrap/build/test/run instructions and milestone scope.
- `src/core/FixedStepClock.hpp/.cpp`: 60 Hz accumulator with five-tick catch-up cap.
- `src/input/PlayerCommand.hpp`: game-owned command DTO with normalized movement intent.
- `src/simulation/Components.hpp`: milestone state components only.
- `src/simulation/Simulation.hpp/.cpp`: headless world, stable player handle, explicit tick ordering.
- `src/simulation/MovementSystem.hpp/.cpp`: consumes PlayerCommand and updates player position per fixed tick.
- `src/render/PresentationSnapshot.hpp`: immutable render-facing DTOs.
- `src/render/PresentationExtractor.hpp/.cpp`: converts simulation state into render-facing snapshot.
- `src/platform/sdl/SdlPlatform.hpp/.cpp`: SDL lifecycle, window, event pump, semantic input adapter.
- `src/render/sdlgpu/SdlGpuRenderer.hpp/.cpp`: GPU device/swapchain, frame acquire/clear/present and snapshot draw boundary.
- `src/app/main.cpp`: composition root and accumulator-driven application loop.
- `tests/core/FixedStepClockTests.cpp`: accumulator/cap/interpolation behavior.
- `tests/simulation/SimulationTests.cpp`: deterministic creation/movement behavior.
- `tests/render/PresentationExtractorTests.cpp`: render snapshot extraction without SDL/GPU.
- `.github/workflows/ci.yml`: Linux and Windows configure/build/test matrix.

---

### Task 1: Reproducible Build and Test Harness

**Files:**
- Create: `CMakeLists.txt`
- Create: `CMakePresets.json`
- Create: `.gitignore`
- Create: `README.md`
- Create: `tests/smoke/BootstrapTests.cpp`

**Interfaces:**
- Consumes: approved architecture spec only.
- Produces: CMake targets `ac2_core`, `ac2_input`, `ac2_simulation`, `ac2_render_model`, `ac2_platform_sdl`, `ac2_render_sdlgpu`, `ashen_covenant_2`, `ac2_tests`; Catch2 test discovery; pinned SDL3/EnTT/Catch2 dependencies.

- [ ] **Step 1: Write the failing bootstrap test**

```cpp
#include <catch2/catch_test_macros.hpp>

TEST_CASE("AC2 test harness boots") {
    REQUIRE(2 + 2 == 4);
}
```

- [ ] **Step 2: Verify RED**

Run: `cmake --preset dev && cmake --build --preset dev && ctest --preset dev`

Expected: configure/build fails because the project targets and dependency graph do not exist yet.

- [ ] **Step 3: Add the minimal build graph**

Use CMake `FetchContent` with explicit tags, require C++20 with extensions off, create focused static/interface targets, enable testing, link Catch2 to `ac2_tests`, and use `catch_discover_tests(ac2_tests)`. Keep SDL linked only to SDL adapter targets and EnTT linked only to simulation.

Canonical dependency intent:

```cmake
add_library(ac2_core STATIC)
add_library(ac2_input INTERFACE)
add_library(ac2_simulation STATIC)
add_library(ac2_render_model STATIC)
add_library(ac2_platform_sdl STATIC)
add_library(ac2_render_sdlgpu STATIC)
add_executable(ashen_covenant_2)
add_executable(ac2_tests tests/smoke/BootstrapTests.cpp)
```

`CMakePresets.json` must provide `dev` configure/build/test presets with an out-of-source `build/dev` directory and `CMAKE_BUILD_TYPE=Debug` on single-config generators.

- [ ] **Step 4: Verify GREEN**

Run: `cmake --preset dev && cmake --build --preset dev && ctest --preset dev --output-on-failure`

Expected: bootstrap test passes and there are no AC2-owned compiler warnings.

- [ ] **Step 5: Commit**

Commit message: `build: bootstrap native AC2 C++ project`

---

### Task 2: Fixed-Step Simulation Clock

**Files:**
- Create: `src/core/FixedStepClock.hpp`
- Create: `src/core/FixedStepClock.cpp`
- Create: `tests/core/FixedStepClockTests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces:
  - `struct ac2::core::StepResult { std::uint32_t ticks; double alpha; bool dropped_time; };`
  - `class ac2::core::FixedStepClock`
  - `static constexpr double tick_seconds = 1.0 / 60.0;`
  - `static constexpr std::uint32_t max_catch_up_ticks = 5;`
  - `StepResult advance(double elapsed_seconds);`
  - `void reset() noexcept;`

- [ ] **Step 1: Write failing tests**

Cover: less than one tick produces zero ticks and interpolation alpha; exactly one tick produces one tick; 2.5 ticks produces two ticks plus alpha 0.5; large stalls produce exactly five ticks, set `dropped_time=true`, and discard excess backlog; negative elapsed time is clamped to zero.

Representative assertion:

```cpp
ac2::core::FixedStepClock clock;
auto result = clock.advance(ac2::core::FixedStepClock::tick_seconds * 2.5);
REQUIRE(result.ticks == 2);
REQUIRE(result.alpha == Catch::Approx(0.5));
REQUIRE_FALSE(result.dropped_time);
```

- [ ] **Step 2: Verify RED**

Run: `ctest --preset dev -R FixedStepClock --output-on-failure`

Expected: compilation fails because `FixedStepClock` does not exist.

- [ ] **Step 3: Implement minimal accumulator**

`advance()` accumulates non-negative elapsed time, computes whole fixed ticks, caps to five, drops excess whole-tick backlog when capped, and returns fractional interpolation alpha in `[0,1)`. No SDL timing types are permitted.

- [ ] **Step 4: Verify GREEN**

Run: `cmake --build --preset dev && ctest --preset dev -R FixedStepClock --output-on-failure`

Expected: all clock tests pass.

- [ ] **Step 5: Commit**

Commit message: `feat: add fixed-step simulation clock`

---

### Task 3: Semantic Player Commands and Headless Movement

**Files:**
- Create: `src/input/PlayerCommand.hpp`
- Create: `src/simulation/Components.hpp`
- Create: `src/simulation/MovementSystem.hpp`
- Create: `src/simulation/MovementSystem.cpp`
- Create: `src/simulation/Simulation.hpp`
- Create: `src/simulation/Simulation.cpp`
- Create: `tests/simulation/SimulationTests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces:
  - `struct ac2::input::PlayerCommand { float move_x; float move_y; bool dodge_pressed; bool light_attack_pressed; bool heavy_attack_pressed; };`
  - `struct ac2::simulation::Transform { float x; float y; };`
  - `struct ac2::simulation::MoveSpeed { float units_per_second; };`
  - `class ac2::simulation::Simulation`
  - `void tick(const ac2::input::PlayerCommand& command);`
  - `Transform player_transform() const;`
  - `std::uint64_t tick_index() const noexcept;`

- [ ] **Step 1: Write failing simulation tests**

Tests must prove: a new simulation has one player at origin; no command preserves position; `(1,0)` moves exactly `speed/60` units in one tick; diagonal input is normalized so `(1,1)` is not faster; identical command sequences create identical final position and tick index.

- [ ] **Step 2: Verify RED**

Run: `ctest --preset dev -R Simulation --output-on-failure`

Expected: compilation fails because the simulation API is absent.

- [ ] **Step 3: Implement minimal EnTT-backed simulation**

Create one player entity in the constructor with `Transform{0,0}` and `MoveSpeed{6.0F}`. `MovementSystem` clamps/normalizes movement magnitude to one and integrates with a compile-time fixed `1/60` tick. `Simulation::tick()` executes explicit ordering and increments a monotonic tick index after movement.

- [ ] **Step 4: Verify GREEN**

Run: `cmake --build --preset dev && ctest --preset dev -R Simulation --output-on-failure`

Expected: all simulation tests pass headlessly.

- [ ] **Step 5: Commit**

Commit message: `feat: add headless player simulation`

---

### Task 4: Immutable Presentation Extraction

**Files:**
- Create: `src/render/PresentationSnapshot.hpp`
- Create: `src/render/PresentationExtractor.hpp`
- Create: `src/render/PresentationExtractor.cpp`
- Create: `tests/render/PresentationExtractorTests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces:
  - `struct ac2::render::SpriteInstance { float previous_x; float previous_y; float current_x; float current_y; std::uint32_t visual_id; };`
  - `struct ac2::render::PresentationSnapshot { std::vector<SpriteInstance> sprites; };`
  - `PresentationSnapshot extract_presentation(const ac2::simulation::Simulation& simulation);`
  - Simulation exposes read-only previous/current player transform data sufficient for interpolation.

- [ ] **Step 1: Write failing extraction tests**

Verify a snapshot contains exactly the player, visual ID is stable, extraction does not alter simulation tick/position, and previous/current positions bracket the latest movement tick.

- [ ] **Step 2: Verify RED**

Run: `ctest --preset dev -R Presentation --output-on-failure`

Expected: compilation fails because render-model types do not exist.

- [ ] **Step 3: Implement minimal extraction**

Store previous transform before each movement tick. Extract by value into presentation DTOs; expose no EnTT registry and no mutable gameplay references.

- [ ] **Step 4: Verify GREEN**

Run: `cmake --build --preset dev && ctest --preset dev -R Presentation --output-on-failure`

Expected: presentation tests pass and simulation tests remain green.

- [ ] **Step 5: Commit**

Commit message: `feat: add immutable presentation snapshots`

---

### Task 5: SDL3 Platform and Semantic Input Adapter

**Files:**
- Create: `src/platform/sdl/SdlPlatform.hpp`
- Create: `src/platform/sdl/SdlPlatform.cpp`
- Create: `tests/input/InputMappingTests.cpp`
- Create: `src/input/ActionMap.hpp`
- Create: `src/input/ActionMap.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces:
  - SDL-independent `ac2::input::DigitalInputState` and `map_to_player_command(...)` in `ac2_input`.
  - `class ac2::platform::SdlPlatform` owning SDL initialization and `SDL_Window*` through RAII.
  - `bool pump_events();` returning false after quit.
  - `ac2::input::PlayerCommand sample_player_command() const;`
  - `SDL_Window* window() const noexcept;` only in the SDL adapter API.

- [ ] **Step 1: RED for mapping behavior**

Headless tests verify W/A/S/D directional composition, opposing directions cancel, and edge-triggered actions appear only in the command frame in which they are pressed. These tests call the SDL-independent action map.

- [ ] **Step 2: Verify RED**

Run: `ctest --preset dev -R InputMapping --output-on-failure`

Expected: compile failure because action mapping is absent.

- [ ] **Step 3: GREEN action map, then SDL glue**

Implement action mapping first. Then add minimal RAII SDL initialization/window/event adapter and translate SDL key/device state into `DigitalInputState` before calling the tested mapper. Gameplay targets must remain SDL-free.

- [ ] **Step 4: Verify GREEN**

Run: `cmake --build --preset dev && ctest --preset dev --output-on-failure`

Expected: all headless tests pass. Platform target compiles.

- [ ] **Step 5: Commit**

Commit message: `feat: add SDL platform and semantic input adapter`

---

### Task 6: SDL3 GPU Renderer Boundary

**Files:**
- Create: `src/render/sdlgpu/SdlGpuRenderer.hpp`
- Create: `src/render/sdlgpu/SdlGpuRenderer.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: `SDL_Window*`, `const PresentationSnapshot&`, interpolation alpha.
- Produces: RAII `SdlGpuRenderer`, `render(const PresentationSnapshot&, double alpha)` and a clean failure result for device/swapchain initialization.

- [ ] **Step 1: Establish testable interpolation helper RED**

Put pure interpolation in the SDL-independent render model:

```cpp
float interpolate(float previous, float current, double alpha);
```

Test alpha `0`, `0.5`, `1` boundaries before implementation.

- [ ] **Step 2: Verify RED**

Run: `ctest --preset dev -R Presentation --output-on-failure`

Expected: compile/link failure because interpolation helper is missing.

- [ ] **Step 3: GREEN helper and add GPU adapter**

Implement the pure helper. Add SDL GPU device/window claim, acquire command buffer/swapchain texture, clear the frame, consume the snapshot boundary, submit and present. The first renderer may draw the player with a minimal generated primitive/path; no asset pipeline is required yet.

- [ ] **Step 4: Verify GREEN/build integration**

Run: `cmake --build --preset dev && ctest --preset dev --output-on-failure`

Expected: all headless tests pass and GPU adapter compiles on supported SDL3 platforms.

- [ ] **Step 5: Commit**

Commit message: `feat: add SDL GPU presentation adapter`

---

### Task 7: Composition Root and Real Application Loop

**Files:**
- Create: `src/app/main.cpp`
- Modify: `CMakeLists.txt`
- Modify: `README.md`

**Interfaces:**
- Consumes: `SdlPlatform`, `SdlGpuRenderer`, `FixedStepClock`, `Simulation`, `extract_presentation`.
- Produces: desktop executable `ashen_covenant_2`.

- [ ] **Step 1: Specify loop contract in a testable coordinator RED**

Create a small SDL-independent `src/app/FrameCoordinator.hpp/.cpp` used by `main`: `advance(elapsed, command)` calls `FixedStepClock`, executes exactly `StepResult.ticks` simulation ticks, returns the presentation snapshot, alpha, and dropped-time flag. Test that a 2.5-tick elapsed frame advances simulation exactly twice and reports alpha 0.5.

- [ ] **Step 2: Verify RED**

Run: `ctest --preset dev -R FrameCoordinator --output-on-failure`

Expected: compile failure because coordinator is absent.

- [ ] **Step 3: Implement coordinator and composition root**

`main.cpp` owns all concrete objects, measures elapsed time with `std::chrono::steady_clock`, pumps events, samples one semantic command frame, advances the coordinator, logs catch-up drops to stderr in debug/developer builds, renders the returned snapshot, and shuts down via RAII.

- [ ] **Step 4: Verify GREEN**

Run: `cmake --build --preset dev && ctest --preset dev --output-on-failure`

Expected: all tests pass and executable links.

Manual desktop smoke criterion: window initializes, player movement is visible, quit exits cleanly.

- [ ] **Step 5: Commit**

Commit message: `feat: wire AC2 vertical foundation loop`

---

### Task 8: Windows/Linux CI and Architecture Guardrails

**Files:**
- Create: `.github/workflows/ci.yml`
- Create: `tests/architecture/BoundaryTests.cpp`
- Modify: `README.md`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces: CI matrix for Ubuntu and Windows; build/test commands identical to documented presets where possible.

- [ ] **Step 1: Add architecture guard RED**

Add a test/helper that scans project-owned gameplay headers/sources and fails if SDL includes appear outside `src/platform/sdl`, `src/render/sdlgpu`, or `src/app`; also fail if render adapter headers appear in `src/simulation`, `src/input`, or `src/core`.

- [ ] **Step 2: Verify RED intentionally**

Before adding allowlist-aware implementation, run the architecture test and confirm it fails because the boundary scanner is missing.

- [ ] **Step 3: Implement guard and CI**

CI triggers on pushes and pull requests, checks out recursively if needed, configures, builds, and runs `ctest --output-on-failure` on `ubuntu-latest` and `windows-latest`. Cache only fetched/build dependencies if cache correctness is straightforward; caching is optional.

- [ ] **Step 4: Full verification**

Run locally when possible:

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev --output-on-failure
```

Then verify both GitHub Actions matrix jobs are green. Do not weaken warnings/tests to achieve green.

- [ ] **Step 5: Commit**

Commit message: `ci: verify AC2 foundation on Windows and Linux`

---

## Plan Self-Review

- Spec coverage for this milestone: build separation, C++20, CMake, SDL isolation, EnTT simulation, 60 Hz fixed step, five-tick cap, semantic input, explicit headless tick, immutable presentation extraction, SDL GPU adapter, composition root, headless tests, Windows/Linux CI.
- Explicitly deferred per spec: full combat pipeline, 300-level skill graph implementation, Covenant mechanics, save migrations, content schemas, audio, editor/tooling, performance job systems. Their architectural boundaries are preserved but they are not fake-scaffolded in milestone 1.
- No production gameplay behavior is introduced before a failing test in its task. Platform/GPU glue is kept behind tested pure boundaries and compile/smoke integration coverage.
- Naming/type consistency checked across all tasks: `PlayerCommand`, `Simulation`, `FixedStepClock`, `StepResult`, `PresentationSnapshot`, `SdlPlatform`, `SdlGpuRenderer`, and `FrameCoordinator` are used consistently.
