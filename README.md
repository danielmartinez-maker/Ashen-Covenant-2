# Ashen Covenant 2

Ashen Covenant 2 is a native C++20 2D/2.5D action RPG under active development.

## Foundation stack

- C++20
- CMake 3.28+
- SDL 3.4.12
- SDL3 GPU API
- EnTT 4.0.0
- Catch2 3.15.3

The first milestone establishes a headless fixed-step simulation and a thin SDL desktop/presentation layer. Full combat, progression, Covenants, saves, audio, and authored content pipelines are intentionally developed as later vertical systems rather than empty scaffolding.

## Configure, build, and test

Requirements: CMake 3.28+, Ninja, a C++20 compiler, Git.

```sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev --output-on-failure
```

Run after building:

```sh
./build/dev/ashen_covenant_2
```

On Windows with Ninja the executable is `build/dev/ashen_covenant_2.exe`.

## Architecture

The approved architecture is documented in `docs/superpowers/specs/2026-09-14-ac2-foundation-design.md`. The current implementation plan is in `docs/superpowers/plans/2026-09-14-foundation-architecture.md`.

Core rule: gameplay simulation stays independent of SDL and rendering. Platform adapters produce semantic commands; rendering consumes immutable presentation state.
