# AC2 VFX Presentation Contract

## Boundary

Combat and gameplay may request visual effects, but VFX never determine hits, damage, collision, invulnerability, status application, targeting, or timing authority. The gameplay/presentation boundary uses `ac2::vfx::VfxRequest` and the renderer-facing result is a stable sequence of `VfxSpawnCommand` objects.

The contract intentionally contains no SDL3 or EnTT types. The SDL3 GPU renderer will translate logical effect IDs to renderer-owned atlas resources after the general AC2 asset system exists.

## Request flow

1. Gameplay resolves an authoritative combat event.
2. Presentation extraction selects the base logical VFX ID and optional secondary impact IDs.
3. `VfxComposer` resolves base + secondary + critical + Covenant Metamorphosis presentation augmentation.
4. Commands are stable-sorted by canonical VFX layer order.
5. Readability policy clamps persistent/friendly coverage where required.
6. Renderer resolves the logical ID to atlas metadata/resources and draws the command.

Elemental, critical, and Covenant impacts are additive. They do not replace the physical contact effect unless a future authored gameplay/presentation rule explicitly selects a different base effect before composition.

## Direction semantics

- `Rotational1`: renderer rotates a direction-independent asset; `direction_index` is 0 and `rotation_degrees` preserves requested angle.
- `Cardinal4`: command direction is normalized to octant indices 0, 2, 4, or 6.
- `Octant8`: command direction is normalized to 0–7 in 45-degree sectors.

Authoring should use Octant8 whenever rotating one sprite would expose asymmetric equipment, anatomy, shadow forms, roots, sprays, or weapon geometry.

## Layer order

The enum order is the required draw order: GroundDecal → GroundEffect → RearField → CharacterShadow → Character → RearWeaponTrail → AttackCore → FrontWeaponTrail → Projectile → ImpactCore → Debris → MaterialResponse → CovenantOverlay → Status → ForegroundParticles → ScreenSpaceAccent.

## Readability

Persistent friendly effects are capped at reduced opacity after initial presentation. Effects tagged `boss_silhouette_safe` receive a stricter persistent opacity cap. Enemy telegraphs bypass friendly-opacity reduction because their gameplay warning must remain legible. Screen-space accents remain explicit and opacity-limited.

A future GPU implementation may add more conservative runtime budgets, but it must not weaken enemy telegraph visibility or make VFX authoritative for gameplay.

## Asset IDs

Authored IDs follow `AC2_VFX_[CLASS]_[ABILITY]_[COMPONENT]_[VARIANT]` or the equivalent shared-library namespace under the same `AC2_VFX_` prefix. Gameplay code should retain logical IDs; texture paths, GPU handles, atlas coordinates, and shader resources stay renderer/asset-owned.
