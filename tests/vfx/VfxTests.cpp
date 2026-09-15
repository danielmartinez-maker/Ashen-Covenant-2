#include "vfx/VfxCatalog.h"
#include "vfx/VfxComposer.h"
#include "vfx/VfxPolicy.h"
#include "vfx/VfxPresentation.h"
#include "vfx/VfxValidation.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
int failures = 0;
#define CHECK(expr) do { if (!(expr)) { std::cerr << "FAIL " << __FILE__ << ':' << __LINE__ << ": " #expr "\n"; ++failures; } } while(false)
using namespace ac2::vfx;

VfxDefinition def(std::string id, VfxComponentType component, VfxLayer layer) {
    VfxDefinition d; d.id=std::move(id); d.component=component; d.width=256; d.height=256; d.frame_count=6; d.atlas_size=1024; d.direction_policy=DirectionPolicy::Octant8; d.magnitude=MagnitudeTier::M; d.layer=layer; return d;
}
void testCanonicalLayerOrder(){ CHECK(layerRank(VfxLayer::GroundDecal)<layerRank(VfxLayer::Character)); CHECK(layerRank(VfxLayer::AttackCore)<layerRank(VfxLayer::ImpactCore)); CHECK(layerRank(VfxLayer::Status)<layerRank(VfxLayer::ScreenSpaceAccent)); }
void testValidation(){ auto d=def("AC2_VFX_KNT_EARTHBREAKER_GROUND_A",VfxComponentType::Ground,VfxLayer::GroundEffect); CHECK(isCanonicalVfxId(d.id)); CHECK(validateDefinition(d).empty()); CHECK(!isCanonicalVfxId("ac2_vfx_knt_bad")); CHECK(!isCanonicalVfxId("AC2 VFX BAD")); CHECK(!isCanonicalVfxId("VFX_KNT_BAD")); d.frame_count=0; CHECK(!validateDefinition(d).empty()); d.frame_count=6; d.atlas_size=1500; CHECK(!validateDefinition(d).empty()); }
void testAdditiveCompositionAndLayerSorting(){ VfxCatalog catalog; CHECK(catalog.add(def("AC2_VFX_SHARED_SLASH_IMP_M",VfxComponentType::Impact,VfxLayer::ImpactCore)).ok()); auto fire=def("AC2_VFX_SHARED_FIRE_IMP_M",VfxComponentType::Impact,VfxLayer::MaterialResponse); fire.damage_type=DamageType::Fire; CHECK(catalog.add(fire).ok()); auto trail=def("AC2_VFX_KNT_LIGHT_01_TRAIL_A",VfxComponentType::Trail,VfxLayer::FrontWeaponTrail); CHECK(catalog.add(trail).ok()); VfxRequest req; req.base_effect_id=trail.id; req.secondary_effect_ids={fire.id,"AC2_VFX_SHARED_SLASH_IMP_M"}; req.direction_degrees=91.0F; VfxComposer composer(catalog); const auto result=composer.compose(req); CHECK(result.issues.empty()); CHECK(result.commands.size()==3); CHECK(result.commands[0].effect_id==trail.id); CHECK(result.commands[1].effect_id=="AC2_VFX_SHARED_SLASH_IMP_M"); CHECK(result.commands[2].effect_id==fire.id); CHECK(result.commands[0].direction_index==2); }
void testCritAugmentsRatherThanReplaces(){ VfxCatalog catalog; auto base=def("AC2_VFX_ROG_BACKSTAB_IMP_A",VfxComponentType::Impact,VfxLayer::ImpactCore); auto crit=def("AC2_VFX_SHARED_PHY_CRIT_A",VfxComponentType::Impact,VfxLayer::CovenantOverlay); CHECK(catalog.add(base).ok()); CHECK(catalog.add(crit).ok()); VfxRequest req; req.base_effect_id=base.id; req.crit_effect_id=crit.id; VfxComposer composer(catalog); CHECK(composer.compose(req).commands.size()==2); }
void testMetamorphosisIsCumulativePresentationMutation(){ VfxCatalog catalog; auto base=def("AC2_VFX_KNT_LIGHT_01_TRAIL_A",VfxComponentType::Trail,VfxLayer::FrontWeaponTrail); base.metamorphosis.stage_ii={"AC2_VFX_KNT_BLOODKNIGHT_LIGHT_IMP_S2"}; base.metamorphosis.stage_iii={"AC2_VFX_KNT_BLOODKNIGHT_LIGHT_TRAIL_S3"}; base.metamorphosis.stage_iv={"AC2_VFX_KNT_BLOODKNIGHT_LIGHT_SILHOUETTE_S4"}; base.metamorphosis.stage_v={"AC2_VFX_KNT_BLOODKNIGHT_LIGHT_META_S5"}; CHECK(catalog.add(base).ok()); for(const auto& id:{base.metamorphosis.stage_ii[0],base.metamorphosis.stage_iii[0],base.metamorphosis.stage_iv[0],base.metamorphosis.stage_v[0]}) CHECK(catalog.add(def(id,VfxComponentType::Metamorphosis,VfxLayer::CovenantOverlay)).ok()); VfxComposer composer(catalog); VfxRequest req; req.base_effect_id=base.id; req.metamorphosis_stage=MetamorphosisStage::I; CHECK(composer.compose(req).commands.size()==1); req.metamorphosis_stage=MetamorphosisStage::III; CHECK(composer.compose(req).commands.size()==3); req.metamorphosis_stage=MetamorphosisStage::V; CHECK(composer.compose(req).commands.size()==5); }
void testReadabilityPolicy(){ VfxSpawnCommand friendly; friendly.persistent=true; friendly.role=PresentationRole::Friendly; friendly.opacity=1.0F; applyReadabilityPolicy(friendly); CHECK(friendly.opacity<=0.62F); VfxSpawnCommand telegraph; telegraph.persistent=true; telegraph.role=PresentationRole::EnemyTelegraph; telegraph.opacity=1.0F; applyReadabilityPolicy(telegraph); CHECK(std::abs(telegraph.opacity-1.0F)<0.001F); VfxSpawnCommand bossSafe; bossSafe.persistent=true; bossSafe.role=PresentationRole::Friendly; bossSafe.opacity=0.9F; bossSafe.tags={"boss_silhouette_safe"}; applyReadabilityPolicy(bossSafe); CHECK(bossSafe.opacity<=0.45F); }
void testDirectionNormalization(){ CHECK(normalizeDirectionIndex(0.0F,DirectionPolicy::Octant8)==0); CHECK(normalizeDirectionIndex(44.0F,DirectionPolicy::Octant8)==1); CHECK(normalizeDirectionIndex(91.0F,DirectionPolicy::Octant8)==2); CHECK(normalizeDirectionIndex(-90.0F,DirectionPolicy::Octant8)==6); CHECK(normalizeDirectionIndex(91.0F,DirectionPolicy::Cardinal4)==2); CHECK(normalizeDirectionIndex(179.0F,DirectionPolicy::Cardinal4)==4); CHECK(normalizeDirectionIndex(271.0F,DirectionPolicy::Cardinal4)==6); CHECK(normalizeDirectionIndex(271.0F,DirectionPolicy::Rotational1)==0); }
void testMissingIdsProduceDiagnostics(){ VfxCatalog catalog; VfxComposer composer(catalog); VfxRequest req; req.base_effect_id="AC2_VFX_KNT_MISSING_CORE_A"; const auto result=composer.compose(req); CHECK(result.commands.empty()); CHECK(result.issues.size()==1); }
void testDuplicateCatalogIdsAreRejected(){ VfxCatalog catalog; auto d=def("AC2_VFX_MAG_ARCANE_BOLT_CORE_A",VfxComponentType::Core,VfxLayer::AttackCore); CHECK(catalog.add(d).ok()); CHECK(!catalog.add(d).ok()); }
}
int main(){ testCanonicalLayerOrder(); testValidation(); testAdditiveCompositionAndLayerSorting(); testCritAugmentsRatherThanReplaces(); testMetamorphosisIsCumulativePresentationMutation(); testReadabilityPolicy(); testDirectionNormalization(); testMissingIdsProduceDiagnostics(); testDuplicateCatalogIdsAreRejected(); if(failures!=0){ std::cerr<<failures<<" VFX test assertion(s) failed\n"; return EXIT_FAILURE; } std::cout<<"AC2 VFX tests passed\n"; return EXIT_SUCCESS; }
