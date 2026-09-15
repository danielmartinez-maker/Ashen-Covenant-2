# Ashen Covenant 2 — Combat VFX Foundation Design

**Status:** Approved from the user-authored Complete Attack VFX Sprite Sheet Bible  
**Date:** 2026-09-14  
**Target:** AC2 C++20 presentation/VFX domain

## 1. Purpose

This design converts the user-authored AC2 Attack VFX Sprite Sheet Bible into an executable, data-authored VFX foundation. It does not fabricate final art. It defines the contracts, authored catalog, composition rules, validation, and renderer-facing commands that final sprite sheets will use.

The implementation must preserve AC2's foundation architecture: gameplay authority stays outside rendering; authored presentation data is separate from behavior; SDL types do not leak into domain interfaces; stable logical asset/effect IDs are used instead of arbitrary paths.

## 2. Scope

This slice implements:

- modular VFX component types: ANT, TRAIL, CORE, IMP, DEBRIS, GROUND, PROJ, MUZZLE, EXP, AURA, FIELD, STATUS, DISS, META;
- damage types and additive secondary impacts;
- S/M/L/XL hit magnitude tiers;
- 1-direction rotational, 4-direction, and 8-direction policies;
- the canonical combat draw-layer order;
- stable AC2 effect naming validation;
- an authored catalog covering the shared physical library, all eight base classes, all 24 listed Covenant specializations, elemental weapon overlays, and reusable damage-type families from the user's bible;
- effect composition into renderer-facing spawn commands;
- Metamorphosis Stage I–V mutation rules without granting gameplay authority to VFX;
- readability policies for opacity, persistence, telegraph separation, boss-silhouette protection, and screen-space accents;
- atlas recommendations and metadata (1024, 2048, 4096 power-of-two families);
- deterministic validation and tests.

Final PNG sprite sheets, GPU texture upload, SDL3 GPU pipelines, particles, material shaders, screen post-processing, and content hot reload are not fabricated in this slice because those systems/art assets do not yet exist in the repository. The interfaces are designed so those pieces can attach without changing combat rules.

## 3. Architecture

### 3.1 Domain boundary

`ac2_vfx` is a dependency-light library. It owns VFX vocabulary, authored definitions, composition rules, validation, and presentation commands. It depends only on C++20 standard library code in this slice.

Gameplay emits presentation-safe requests containing stable IDs and context such as direction, magnitude, friendly/enemy ownership, and Metamorphosis stage. `VfxComposer` resolves authored definitions into `VfxSpawnCommand` records. A future SDL3 GPU renderer consumes those records and owns texture/atlas resources.

The composer never applies damage, hit stun, status logic, targeting, collision, or invulnerability.

### 3.2 Authored source and generated/runtime representation

`assets/data/vfx/catalog.json` is the machine-readable production catalog. A repository tool validates catalog structure and naming. C++ types mirror the schema so gameplay/render code has explicit contracts.

The initial C++ tests use constructed definitions directly. A runtime JSON asset loader is deferred until the general AC2 asset/data-loading layer exists; adding an isolated JSON parser here would violate the foundation architecture.

## 4. Core Types

- `VfxComponentType`: Anticipation, Trail, Core, Impact, Debris, Ground, Projectile, Muzzle, Explosion, Aura, Field, Status, Dissipation, Metamorphosis.
- `DamageType`: Slash, Pierce, Blunt, Fire, Frost, Lightning, Poison, Blood, Holy, Shadow, Void, Necrotic, Nature, Arcane, Demonic.
- `MagnitudeTier`: S, M, L, XL.
- `DirectionPolicy`: Rotational1, Cardinal4, Octant8.
- `VfxLayer`: GroundDecal, GroundEffect, RearField, CharacterShadow, Character, RearWeaponTrail, AttackCore, FrontWeaponTrail, Projectile, ImpactCore, Debris, MaterialResponse, CovenantOverlay, Status, ForegroundParticles, ScreenSpaceAccent.
- `MetamorphosisStage`: I–V.
- `VfxDefinition`: stable effect ID plus component, dimensions, frame range, loop/persistence, direction policy, magnitude, layer, tags, opacity policy, atlas recommendation, and optional class/Covenant/damage identity.
- `VfxRequest`: base effect ID, optional secondary effect IDs, direction, friendly/enemy presentation role, magnitude override, and Metamorphosis stage.
- `VfxSpawnCommand`: fully presentation-safe resolved command.

Dimensions and frame counts are metadata. Large attacks should be assembled from modular effects rather than one giant 4096 frame.

## 5. Naming

Canonical authored attack assets use:

`AC2_VFX_[CLASS]_[ABILITY]_[COMPONENT]_[VARIANT]`

Shared libraries use the same `AC2_VFX_` prefix with stable category tokens. IDs are uppercase ASCII tokens separated by single underscores. Catalog validation rejects whitespace, lowercase characters, duplicate IDs, unknown component names, invalid direction counts, non-power-of-two atlas recommendations, invalid frame ranges, and unsupported dimensions.

## 6. Composition Rules

A resolved attack is a stack, not a replacement effect.

For an elemental proc, the physical attack remains present and the elemental impact is appended at the same contact event. Example: a sword strike can emit both a slash impact and a small fire impact. Critical-hit VFX augment the existing attack in the same way.

Commands are stable-sorted by canonical VFX layer order so authored insertion order cannot accidentally place ground decals over characters or debris under impacts.

Direction-independent effects may be rotated dynamically. Octant8 assets require authored directional variants when asymmetry would be visible.

## 7. Metamorphosis

Metamorphosis is presentation mutation applied after the base class effect family is selected:

- Stage I: base/physical presentation only.
- Stage II: small Covenant impact augmentation.
- Stage III: Covenant trails and persistent secondary effects.
- Stage IV: attack silhouettes visibly mutate.
- Stage V: basic attacks can resolve to Covenant-supernatural expressions while retaining recognizable action identity.

The catalog stores stage augmentation IDs as presentation references. The composer appends them in deterministic stage order. It never changes damage values or attack timing.

## 8. Readability

Friendly VFX outer opacity is capped below full opacity for large/persistent effects. Persistent player fields receive a reduced steady-state opacity after cast. Enemy telegraphs are tagged and rendered on their own presentation path. Effects tagged `boss_silhouette_safe` may not request opaque persistent foreground coverage. Screen-space accents require an explicit `screen_space` tag and default to rare/short-lived use.

The catalog validator enforces structural readability constraints where possible; aesthetic validation remains an art-review responsibility.

## 9. Catalog Coverage

The initial catalog groups effects beneath:

- Shared: physical hits, damage impacts, elemental weapon overlays.
- Knight: Base, Bastion, Blackguard, BloodKnight.
- Rogue: Base, Nightblade, Trickster, Reaper.
- Ranger: Base, Beastmaster, WraithHunter, PlagueStalker.
- Mage: Base, Pyromancer, Voidweaver, Spellblade.
- Cleric: Base, Paladin, Exorcist, Martyr.
- Warlock: Base, Necromancer, Demonologist, Bloodbinder.
- Monk: Base, IronFist, AstralHand, Penitent.
- Druid: Base, Wildheart, Grovekeeper, Rotcaller.
- Metamorphosis and Ultimates.

The catalog records concrete named effects from the supplied bible and reusable family templates where the bible describes a family rather than one single attack asset. This makes omissions machine-detectable while avoiding fake final art.

## 10. Testing

Headless tests verify:

1. canonical layer ordering;
2. additive secondary impact composition;
3. crit augmentation behavior;
4. Metamorphosis stage augmentation ordering;
5. deterministic direction normalization;
6. friendly persistent-opacity policy;
7. canonical naming acceptance/rejection;
8. magnitude and atlas constraints.

The Python catalog validator is also run by CTest when Python 3 is available, so malformed authored data fails CI.

## 11. Success Criteria

The slice is complete when AC2 can represent and validate the entire supplied VFX vocabulary and authored catalog, compose a base attack plus damage/crit/Covenant overlays into deterministic renderer-facing commands, preserve the required layer/readability rules, and build/test headlessly without SDL or GPU initialization.