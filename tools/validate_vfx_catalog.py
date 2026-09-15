#!/usr/bin/env python3
import json,re,sys
from pathlib import Path
COMPONENTS={'ANT','TRAIL','CORE','IMP','DEBRIS','GROUND','PROJ','MUZZLE','EXP','AURA','FIELD','STATUS','DISS','META'}
DIRECTIONS={'rotational1','cardinal4','octant8'}; MAGNITUDES={'S','M','L','XL'}
LAYERS={'ground_decal','ground_effect','rear_field','character_shadow','character','rear_weapon_trail','attack_core','front_weapon_trail','projectile','impact_core','debris','material_response','covenant_overlay','status','foreground_particles','screen_space_accent'}
DAMAGE={'slash','pierce','blunt','fire','frost','lightning','poison','blood','holy','shadow','void','necrotic','nature','arcane','demonic'}
ID_RE=re.compile(r'^AC2_VFX_[A-Z0-9]+(?:_[A-Z0-9]+)*$')
REQUIRED_GROUPS={'Shared/Physical','Shared/Impacts','Shared/WeaponOverlays','KNT/Base','KNT/BASTION','KNT/BLACKGUARD','KNT/BLOODKNIGHT','ROG/Base','ROG/NIGHTBLADE','ROG/TRICKSTER','ROG/REAPER','RNG/Base','RNG/BEASTMASTER','RNG/WRAITHHUNTER','RNG/PLAGUESTALKER','MAG/Base','MAG/PYROMANCER','MAG/VOIDWEAVER','MAG/SPELLBLADE','CLR/Base','CLR/PALADIN','CLR/EXORCIST','CLR/MARTYR','WRL/Base','WRL/NECROMANCER','WRL/DEMONOLOGIST','WRL/BLOODBINDER','MNK/Base','MNK/IRONFIST','MNK/ASTRALHAND','MNK/PENITENT','DRU/Base','DRU/WILDHEART','DRU/GROVEKEEPER','DRU/ROTCALLER'}
def power2(n): return isinstance(n,int) and n>0 and (n&(n-1))==0
def atlas_for(size): return 1024 if size<=256 else 2048 if size<=512 else 4096
def expand(doc,errors):
 dmap=doc.get('codes',{}).get('direction',{}); lmap=doc.get('codes',{}).get('layer',{}); effects=[]; groups=set()
 for gi,group in enumerate(doc.get('groups',[])):
  if not isinstance(group,dict): errors.append(f'groups[{gi}] must be object'); continue
  name,prefix,rows=group.get('name'),group.get('prefix'),group.get('effects')
  if not isinstance(name,str) or not isinstance(prefix,str) or not isinstance(rows,list): errors.append(f'groups[{gi}] malformed'); continue
  groups.add(name)
  for ri,row in enumerate(rows):
   if not isinstance(row,list) or not 8<=len(row)<=10: errors.append(f'{name}.effects[{ri}] must have 8..10 fields'); continue
   suffix,component,frames,size,dcode,mag,lcode,flags,*optional=row; damage=optional[0] if optional else ''; tags=optional[1] if len(optional)>1 else []
   effects.append({'id':prefix+suffix,'group':name,'component':component,'frames':frames,'width':size,'height':size,'direction_policy':dmap.get(dcode),'magnitude':mag,'layer':lmap.get(lcode),'atlas':atlas_for(size) if isinstance(size,int) else 0,'loop':isinstance(flags,str) and len(flags)>0 and flags[0]=='L','persistent':isinstance(flags,str) and len(flags)>1 and flags[1]=='P','damage_type':damage or None,'tags':tags})
 families=doc.get('families',{}); impacts=families.get('damage_impacts',{})
 if impacts:
  groups.add('Shared/Impacts'); prefix=impacts.get('prefix','')
  for damage in impacts.get('damage',[]):
   for tier,shape in impacts.get('tiers',{}).items():
    if not isinstance(shape,list) or len(shape)!=2: errors.append(f'damage_impacts {tier} must be [size,frames]'); continue
    size,frames=shape; effects.append({'id':f'{prefix}{damage}_IMP_{tier}','group':'Shared/Impacts','component':'IMP','frames':frames,'width':size,'height':size,'direction_policy':'octant8' if damage in {'SLASH','PIERCE'} else 'rotational1','magnitude':tier,'layer':'material_response','atlas':atlas_for(size),'loop':False,'persistent':False,'damage_type':damage.lower(),'tags':['additive_impact']})
 overlays=families.get('weapon_overlays',{})
 if overlays:
  groups.add('Shared/WeaponOverlays'); prefix=overlays.get('prefix','')
  for element in overlays.get('elements',[]):
   for variant in overlays.get('variants',[]):
    if variant=='LOOP': comp,frames,size,direction,mag,layer,loop,persistent='AURA',12,256,'octant8','S','covenant_overlay',True,True
    elif variant=='ATTACK_STRETCH': comp,frames,size,direction,mag,layer,loop,persistent='TRAIL',8,384,'octant8','M','front_weapon_trail',False,False
    elif variant=='IMPACT': comp,frames,size,direction,mag,layer,loop,persistent='IMP',8,256,'rotational1','M','material_response',False,False
    else: errors.append(f'unknown weapon overlay variant {variant}'); continue
    effects.append({'id':f'{prefix}{element}_{variant}','group':'Shared/WeaponOverlays','component':comp,'frames':frames,'width':size,'height':size,'direction_policy':direction,'magnitude':mag,'layer':layer,'atlas':atlas_for(size),'loop':loop,'persistent':persistent,'damage_type':element.lower(),'tags':['weapon_overlay']+(['additive_impact'] if variant=='IMPACT' else [])})
 return effects,groups
def validate(path):
 errors=[]
 try: doc=json.loads(Path(path).read_text())
 except Exception as exc: return [f'cannot parse catalog: {exc}'],0,0
 if doc.get('schema_version')!=2: errors.append('schema_version must be 2')
 effects,groups=expand(doc,errors); ids=set()
 for e in effects:
  eid=e['id']
  if not isinstance(eid,str) or not ID_RE.fullmatch(eid): errors.append(f'noncanonical id {eid!r}')
  if eid in ids: errors.append(f'duplicate id {eid}')
  ids.add(eid)
  if e['component'] not in COMPONENTS: errors.append(f'{eid}: invalid component {e["component"]!r}')
  if e['direction_policy'] not in DIRECTIONS: errors.append(f'{eid}: invalid direction_policy {e["direction_policy"]!r}')
  if e['magnitude'] not in MAGNITUDES: errors.append(f'{eid}: invalid magnitude')
  if e['layer'] not in LAYERS: errors.append(f'{eid}: invalid layer {e["layer"]!r}')
  if e.get('damage_type') is not None and e['damage_type'] not in DAMAGE: errors.append(f'{eid}: invalid damage_type {e["damage_type"]!r}')
  if not isinstance(e['frames'],int) or not 1<=e['frames']<=64: errors.append(f'{eid}: frames must be 1..64')
  for field in ('width','height'):
   if not isinstance(e[field],int) or not 1<=e[field]<=4096: errors.append(f'{eid}: {field} must be 1..4096')
  if not power2(e['atlas']) or not 256<=e['atlas']<=4096: errors.append(f'{eid}: invalid atlas')
  if e['loop'] and not e['persistent']: errors.append(f'{eid}: loop requires persistent')
  if not isinstance(e['tags'],list) or not all(isinstance(t,str) and t for t in e['tags']): errors.append(f'{eid}: invalid tags')
  if 'screen_space' in e['tags'] and e['layer']!='screen_space_accent': errors.append(f'{eid}: screen_space tag requires screen_space_accent')
 missing=sorted(REQUIRED_GROUPS-groups)
 if missing: errors.append(f'missing coverage groups: {missing}')
 for source,stages in doc.get('metamorphosis',{}).items():
  if source not in ids: errors.append(f'metamorphosis source missing: {source}')
  if set(stages)-{'II','III','IV','V'}: errors.append(f'{source}: invalid metamorphosis stage')
  for refs in stages.values():
   if not isinstance(refs,list): errors.append(f'{source}: metamorphosis refs must be arrays'); continue
   for ref in refs:
    if ref not in ids: errors.append(f'{source}: missing metamorphosis ref {ref}')
 for damage in [d.upper() for d in DAMAGE]:
  for tier in MAGNITUDES:
   if f'AC2_VFX_SHARED_{damage}_IMP_{tier}' not in ids: errors.append(f'missing {damage} {tier} impact')
 for elem in ['FIRE','FROST','LIGHTNING','POISON','BLOOD','HOLY','SHADOW','VOID','NECROTIC','NATURE']:
  for variant in ['LOOP','ATTACK_STRETCH','IMPACT']:
   if f'AC2_VFX_SHARED_WEAPON_{elem}_{variant}' not in ids: errors.append(f'missing {elem} weapon {variant}')
 return errors,len(effects),len(groups)
def main(argv):
 if len(argv)!=2: print('usage: validate_vfx_catalog.py <catalog.json>',file=sys.stderr); return 2
 errors,count,groups=validate(argv[1])
 if errors:
  for error in errors: print(f'ERROR: {error}',file=sys.stderr)
  return 1
 print(f'validated {count} AC2 VFX definitions across {groups} groups'); return 0
if __name__=='__main__': raise SystemExit(main(sys.argv))
