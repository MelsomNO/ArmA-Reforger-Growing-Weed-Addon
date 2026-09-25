# Growing Weed — Architecture and Reference

Developer reference for the mod. Covers every prefab, script, material, and asset — where it lives, what it does, how it plugs into the growing loop, and how to tune or extend it.

For end-user gameplay docs see the root `README.md`. For the specific mechanism behind the shovel dig animation see `docs/DIGGING_ANIMATION.md`.

---

## 1. High-level design

The mod is a self-contained growing loop. No dependencies beyond the base game (`Arma Reforger Data`).

Gameplay flow:

1. **Empty pot** picked up from the world → carried in the player's backpack.
2. **Empty pot** placed on the ground → **Fill with Dirt** action (requires shovel in hands) transforms it into a **filled pot**.
3. **Filled pot** + **seed item** in inventory + shovel in hands → **Plant Sativa / Indica Seed** action consumes one seed and swaps the pot for a **Stage 1 seedling**.
4. **Growth cycle**: `GW_GrowthComponent` on each stage tracks in-game hours and, when the threshold is met, replaces the entity with the next stage's prefab. Chain: Stage 1 → Stage 2 → Stage 3 (mature). Runs server-authoritative.
5. **Mature (Stage 3)** plant → **Harvest Cannabis** action (bare-handed) yields 2–4 species-specific cannabis top items and swaps the plant for a filled pot ready to replant.
6. **Uproot Plant** action (any stage, shovel required) removes the plant entirely.
7. **Check Growth** action (any stage) pops a hint with the current progress percent and remaining in-game hours.
8. **Cannabis top** held in hands → inspect → **Roll Joint** action consumes the bud and puts a species-matched **joint** in the inventory.
9. **Joint** held in hands → inspect → **Smoke Joint** action runs the vanilla `Emote_Smoke` emote on the player and consumes the joint.

Both species (Sativa and Indica) share the same scripts and pipeline; they differ only in mesh, texture tint, and prefab references.

---

## 2. Directory layout

```
Growing Weed/
├── addon.gproj                          # Reforger project file (dependency: base game only)
├── README.md                            # Player-facing overview + credits
├── INTEGRATION.md                       # Notes for other mods integrating with this one
├── docs/
│   ├── ARCHITECTURE.md                  # This file
│   └── DIGGING_ANIMATION.md             # How the shovel dig loop is bound
├── Scripts/Game/GrowingWeed/            # All mod-owned Enforce scripts
│   ├── GW_GrowthComponent.c
│   ├── GW_CheckGrowthAction.c
│   ├── GW_ToolAnimAction.c
│   ├── GW_ShovelRequiredAction.c
│   ├── GW_FillPotAction.c
│   ├── GW_PlantAction.c
│   ├── GW_HarvestAction.c
│   ├── GW_UprootAction.c
│   ├── GW_ItemHandUtils.c
│   ├── GW_RollJointAction.c
│   ├── GW_EmoteScheduler.c
│   ├── GW_LoiterItemID.c
│   ├── GW_SmokeCommandAction.c
│   └── GW_SmokeJointAction.c
├── Prefabs/
│   ├── Items/                           # Carryable / world-placeable items
│   │   ├── GW_PotItem_Empty.et
│   │   ├── GW_PotItem_Filled.et
│   │   ├── GW_SeedsSativa.et
│   │   ├── GW_SeedsIndica.et
│   │   ├── GW_CannabisTop_Sativa.et
│   │   ├── GW_CannabisTop_Indica.et
│   │   ├── GW_Joint_Sativa.et
│   │   └── GW_Joint_Indica.et
│   └── Plants/                          # Growing plant stages
│       ├── GW_WeedPlant_Sativa_Stage1.et  # Seedling
│       ├── GW_WeedPlant_Sativa_Stage2.et  # Vegetative
│       ├── GW_WeedPlant_Sativa_Stage3.et  # Mature (harvestable)
│       ├── GW_WeedPlant_Indica_Stage1.et
│       ├── GW_WeedPlant_Indica_Stage2.et
│       └── GW_WeedPlant_Indica_Stage3.et
├── Models/
│   ├── Plants/                          # Six FBX plant meshes + shared materials
│   │   ├── Stage1{Species}.fbx          # Seedling meshes
│   │   ├── Vegetation2{Species}.fbx     # Mid-growth meshes (Stage 2)
│   │   ├── Flowering{Species}.fbx       # Mature meshes with buds baked in (Stage 3)
│   │   ├── Data/                        # Shared materials
│   │   │   ├── Weed.emat                # Cannabis leaf material (Weed_co + Weed_nohq)
│   │   │   ├── BabyLeaf.emat            # Seedling leaf material (BabyLeaf_co + …)
│   │   │   ├── BabySprout.emat          # Seedling sprout tint
│   │   │   ├── Stem.emat                # Branch / trunk (solid color placeholder)
│   │   │   ├── CannabisBud.emat         # Green bud (Sativa)
│   │   │   └── CannabisBudIndica.emat   # Purple bud (Indica)
│   │   └── _workfiles/                  # Blender source files for bud placement
│   │       ├── FloweringSativa_editable.blend
│   │       └── FloweringIndica_editable.blend
│   └── Items/                           # Pot + bud meshes
│       ├── Pot_Empty.fbx
│       ├── Pot_Filled.fbx
│       ├── CannabisBud.fbx / CannabisBud_Indica.fbx  # Standalone bud items
│       ├── Joint.fbx                    # Shared joint item mesh (96 tris)
│       └── Data/                        # Item materials
│           ├── PotClay.emat
│           ├── Plant_Pot_var1.emat      # Auto-created variant used by filled pot
│           ├── CannabisBud.emat
│           └── CannabisBudIndica.emat
├── Textures/
│   ├── Plants/                          # Cannabis leaf atlases (from CGTrader)
│   │   ├── Weed_co.tga / Weed_nohq.tga / Weed_ca.tga
│   │   └── BabyLeaf_co.tga / _nohq.tga / _ca.tga
│   ├── Items/                           # Pot + bud PBR textures
│   │   ├── PotClay_co.png / PotClay_nohq.png
│   │   ├── CannabisBud_co.png / CannabisBud_nohq.png
│   │   └── CannabisBudIndica_co.png     # Same normal as Sativa
│   └── Item icons/
│       └── weed_seeds.edds              # Inventory icon shared by both seed items
├── Configs/
│   └── EntityCatalog/                   # Faction inventory catalog entries
│       ├── CIV/InventoryItems_EntityCatalog_CIV.conf
│       ├── FIA/InventoryItems_EntityCatalog_FIA.conf
│       ├── US/InventoryItems_EntityCatalog_US.conf
│       └── USSR/InventoryItems_EntityCatalog_USSR.conf
└── testmission/
    └── weedtest.ent                     # Development test scene
```

Note: `.xob`, `.txo`, `.edds`, and `resourceDatabase.rdb` are runtime build artifacts and gitignored — Workbench regenerates them on import.

---

## 3. Scripts

All scripts live under `Scripts/Game/GrowingWeed/`. Class prefix is `GW_`.

### `GW_GrowthComponent`

Attaches to each plant stage prefab. Server-authoritative timer that advances the plant.

Attributes:

| Attribute | Type | Default | Description |
|---|---|---|---|
| `m_fGrowInGameHours` | float | 6.0 | In-game hours before this stage advances to the next. |
| `m_rNextStagePrefab` | ResourceName (.et) | "" | Prefab spawned when this stage completes. Empty = terminal (harvestable) stage. |
| `m_bDebugLogs` | bool | false | If true, prints `[GW_GrowthComponent]` lines to the console each tick. Useful while tuning. |

Runtime behavior:

- Ticks once per real-time second via `GetGame().GetCallqueue().CallLater(this.Tick, 1000, true, owner)`.
- Reads current in-game time from `ChimeraWorld.GetTimeAndWeatherManager()`. Uses continuous "absolute hour" (`day * 24 + timeOfDay`) so midnight wrap doesn't reset the timer.
- When `m_fElapsedInGameHours >= m_fGrowInGameHours`:
  - Loads `m_rNextStagePrefab` as a `Resource`.
  - Spawns it at the current entity's world transform via `GetGame().SpawnEntityPrefab`.
  - Deletes the current entity with `SCR_EntityHelper.DeleteEntityAndChildren`.
- Provides `GetProgressPercent()` and `GetRemainingInGameHours()` for other scripts (used by `GW_CheckGrowthAction`).

Growth pauses when the game is paused. Fast-forwarding time in Game Master advances growth proportionally because Reforger's Time & Weather manager continues to update.

### `GW_ToolAnimAction` (base class)

Reusable base for any action that should drive a tool animation on the character while it holds. See `docs/DIGGING_ANIMATION.md` for the mechanism.

Attributes:

| Attribute | Default | Description |
|---|---|---|
| `m_sAnimCommand` | `"CMD_Item_Action"` | Command name bound on the character animation graph. |
| `m_iAnimMode` | `1` | Command int arg. `1` = build/dig-down loop, `2` = demolition/dig-out loop. |

Lifecycle:

- `OnActionStart`: looks up the held gadget via `SCR_GadgetManagerComponent.GetHeldGadget()` (falls back to `BaseWeaponManagerComponent.GetCurrent()`), then binds `CMD_Item_Action` and calls `TryUseItemOverrideParams` with the held item as the tool entity.
- `OnActionCanceled` / `OnConfirmed` / destructor: calls `FinishItemUse(true)` on the character's command handler to end the loop cleanly.

If no matching item is held, no animation plays but the action still runs — subclasses may still gate on the held item via `GW_ShovelRequiredAction`.

### `GW_ShovelRequiredAction` extends `GW_ToolAnimAction`

Adds a gate: only allowed when the held item's prefab name matches one of the configured patterns.

Attributes:

| Attribute | Default | Description |
|---|---|---|
| `m_sShovelNamePatterns` | `"shovel;etool;entrench;spade"` | Semicolon-separated substrings, case-insensitive. Held item's prefab name must contain one. Empty = no gate. |

Used as the base for `GW_FillPotAction`, `GW_PlantAction`, and `GW_UprootAction`.

### `GW_FillPotAction` extends `GW_ShovelRequiredAction`

Attaches to `GW_PotItem_Empty`. Replaces the empty pot with a filled pot on completion.

Attributes:

| Attribute | Default | Description |
|---|---|---|
| `m_rFilledPotPrefab` | (wired) | The `.et` spawned in place of the empty pot. Points at `GW_PotItem_Filled.et`. |

Uses `m_iAnimMode 1` (build/dig-down) by default in its prefab entry.

### `GW_PlantAction` extends `GW_ShovelRequiredAction`

Attaches to `GW_PotItem_Filled`. Gates on both shovel-in-hand AND a specific seed in inventory. Consumes one seed on completion and replaces the pot with the target seedling.

Attributes:

| Attribute | Description |
|---|---|
| `m_rSeedlingPrefab` | Seedling `.et` spawned in place of the pot. |
| `m_rRequiredSeedItem` | Seed `.et` that must be present in the player's inventory. |
| `m_bConsumeSeed` | If true, one seed is deleted via `SCR_InventoryStorageManagerComponent.TryDeleteItem` on complete. Default true. |

Two `GW_PlantAction` instances are placed on the filled pot — one for each species — so both plant actions appear when the player has the matching seed. Each instance points to its own seedling and seed prefab.

### `GW_HarvestAction` (plain `ScriptedUserAction`)

Attaches to Stage 3 plants only. Bare-handed (no shovel gate) so anyone can pluck a mature bud.

Attributes:

| Attribute | Default | Description |
|---|---|---|
| `m_rYieldPrefab` | (wired) | Cannabis-top item `.et` granted to the player's inventory. |
| `m_iYieldMin` | 2 | Minimum item count granted per harvest (inclusive). |
| `m_iYieldMax` | 4 | Maximum item count granted per harvest (inclusive). Actual count is `Math.RandomIntInclusive(min, max)`. |
| `m_rReplacementPrefab` | (wired) | Prefab spawned in place of the harvested plant. Points to `GW_PotItem_Filled.et` so the player can immediately replant. |
| `m_bRequireFullyGrown` | true | If true, disallows harvesting when the growth timer hasn't finished. |

Deliberately does not inherit `GW_ToolAnimAction` — a bare-handed pluck can't drive `CMD_Item_Action` without a tool entity, and the animation editor doesn't expose a command for the "pick from ground" clip we would want.

### `GW_UprootAction` extends `GW_ShovelRequiredAction`

Attaches to every plant stage and to `GW_PotItem_Filled.et`. Deletes the owner and — if `m_rReplacementPrefab` is set — spawns that prefab at the owner's transform. `m_iAnimMode 2` (demolition/dig-out) in each prefab entry. Runs server-side only (`Replication.IsServer()` guard) so the world-spawn replicates.

The uproot chain is two-step: **plant → filled pot → empty pot**. Each step preserves the world position:
- Plant stages 1-3 set `m_rReplacementPrefab` to `GW_PotItem_Filled.et` so uprooting leaves the dirt-filled pot in place.
- `GW_PotItem_Filled.et` adds a second `GW_UprootAction` labelled "Empty Pot" with `m_rReplacementPrefab` set to `GW_PotItem_Empty.et`; the empty pot has an inventory item component so the player can pick it up.

Attributes:

| Attribute | Description |
|---|---|
| `m_rReplacementPrefab` | Prefab spawned at the owner's transform after uproot. Empty = just delete the owner. |

### `GW_CheckGrowthAction` (plain `ScriptedUserAction`)

Attaches to every plant stage. The action label itself reports the current progress — hovering the plant shows e.g. `Growth: 42%` directly in the interaction UI. Terminal stages read `Growth: fully grown`.

Implemented via `GetActionNameScript`, which the interaction UI polls each frame the player is looking at the plant, so the label stays current without any RPC or hint plumbing. `PerformAction` is a no-op — the information is the label. Marked `HasLocalEffectOnlyScript` so the poll runs on the client.

### `GW_ItemHandUtils` (static helpers)

Not an action. Three statics shared by the smoking chain, kept in their own class because its two
users sit on different inheritance branches and Enforce has no multiple inheritance:

- `GetItemInHands(user)` — gadget manager → character controller → weapon manager, first hit wins.
- `IsInHands(user, item)` — whether `item` is the one currently held.
- `DeleteOwnedItem(user, item)` — deletes through `SCR_InventoryStorageManagerComponent.TryDeleteItem`, falling back to `SCR_EntityHelper.DeleteEntityAndChildren` for an item the storage manager does not track (e.g. one lying in the world).

### `GW_RollJointAction` (plain `ScriptedUserAction`)

Attached to both cannabis top items. Turns the item it sits on into another item — the "Roll Joint"
step. Generic enough to reuse for any item-into-item conversion.

| Attribute | Default | Description |
|---|---|---|
| `m_rResultPrefab` | (wired) | Item `.et` spawned into the player's inventory. Action is hidden while empty. |
| `m_iResultCount` | 1 | How many result items per use. |
| `m_bConsumeSource` | true | Delete the source item (the bud) on success. |
| `m_bRequireInHands` | true | Only offer the action while this item is the one in hands, i.e. in the in-hand inspection list rather than on a bud lying in the world. |
| `m_bEquipResult` | true | Put the first result straight into the player's hands. |

The result is spawned **before** the source is consumed, so a full inventory makes the action a
no-op instead of destroying the bud.

Order matters for the equip: the source has to be consumed first, because it is the thing occupying
the hands. The result is also spawned and inserted in two explicit steps rather than through
`TrySpawnPrefabToStorage`, which returns only a bool — the entity reference is needed to equip it.
`CharacterControllerComponent.TryEquipRightHandItem(item, EEquipItemType.EEquipTypeUnarmedDeliberate, true)`
does the equip; `swap = true` skips the draw animation, since the hands were just emptied and there
is nothing to animate away from.

### `GW_SmokeCommandAction` (plain `ScriptedUserAction`)

**The action wired onto both joint items.** Looks the vanilla smoking emote up by name through
`SCR_CommandingManagerComponent.FindCommand()` and executes it on the smoker, then consumes the
joint.

| Attribute | Value on the joints | Description |
|---|---|---|
| `m_rEmoteConfig` | `GW_Emote_SmokeJoint.conf` | Emote config to run, instantiated straight from the resource. Takes precedence over the name lookup. |
| `m_sEmoteCommandName` | `Emote_Smoke` | Fallback only, used when `m_rEmoteConfig` is empty: the `m_sCommandName` of an already-registered emote. |
| `m_bConsumeJoint` | 1 | Delete the joint when the action completes (server-side). |
| `m_bRequireInHands` | 1 | Only offer the action while the joint is in hands. |
| `m_fEmoteDelayMs` | 250 | Milliseconds after the action's duration before the emote fires. Must stay above zero — see the table below. |
| `m_bDebugLog` | 0 | Log whether the command was found, whether it accepted execution, and whether the joint was consumed. Turn on while tuning. |

The joint is deleted in `PerformAction` behind a `Replication.IsServer()` guard. The emote is
**queued** from `OnActionStart` and run by `GW_EmoteScheduler` after this action's own duration has
elapsed.

That indirection is not decoration — three placements were tried and only the fourth works:

| Where the emote starts | Animation | Joint consumed |
|---|---|---|
| `OnActionStart`, directly | yes | **no** — `Emote_Smoke` is a *continuous* loiter command that holsters items and takes over player input, cancelling the action mid-progress, so it never reaches execution and `PerformAction` never runs (no log line at all) |
| `OnConfirmed` | **no** — the hook only fires when an action is passed somewhere as a callback, which this is not | yes |
| Queued from the action object | **no** — consuming the joint destroys the action and the pending `CallLater` with it (`joint consumed` then `scheduled emote canceled`) | yes |
| Queued from `GW_EmoteScheduler` | yes | yes |

The in-hands requirement also gates `CanBeShownScript` only, **never** `CanBePerformedScript`: the
server re-checks the latter before running `PerformAction`, and its view of what is in the
character's hands does not necessarily agree with the client's.

### Giving the emote our own prop

The smoking animation spawns a prop, and by default that prop is the vanilla cigar. Swapping it for
the joint model is done **additively** — no vanilla config or prefab is overridden. Three pieces:

**1. `GW_LoiterItemID.c` — a modded enum plus a preset entry.** How the vanilla system works, from
`SCR_LoiterItemPresets.c` and `SCR_CharacterCommandLoiter.c`:

- `Configs/Character/LoiterItemPresets.conf` (`{911269C6C40E808B}`, a GUID that only appears inside
  a `Debug.Error` format string in the script) deserialises into `SCR_LoiterItemPresets`, whose
  constructor indexes `m_aLoiterItemsData` into a `map<SCR_ELoiterItemID, SCR_LoiterItemDataBase>`
  keyed on each entry's `GetId()`.
- `SCR_LoiterItemPresets.GetData(id)` is the only read path into that map.
- `SCR_CharacterCommandLoiter.OnActivate` calls `m_customAnimData.SpawnItem(m_pCharacter)`;
  `OnDeactivate` calls `m_customAnimData.DeleteItem()`. Which preset is asked for comes from
  `SCR_LoiterCustomAnimData.m_eLoiterItemPreset`.

So `modded enum SCR_ELoiterItemID { GW_JOINT }` plus a `modded class SCR_LoiterItemPresets` that
overrides `GetData` — returning a `GW_JointLoiterItemData` for `GW_JOINT` and deferring to `super`
for everything else — registers the entry without touching the config.

> **A script-constructed preset entry must set every field.** `[Attribute]` defvalues are applied by
> the config deserialiser, not by `new`, so anything `GW_JointLoiterItemData`'s constructor leaves
> alone stays zero-initialised. Vanilla's `CIGAR` entry sets only `m_vPositionOffsetLS` and lets the
> class defaults supply the prefab and the bone; an empty `m_sBoneName` leaves the prop unparented and
> it renders at the character's origin — on the floor between their feet, with no error logged.
> `BONE_NAME` / `POSITION_OFFSET_LS` / `ANGLE_OFFSET_LS` at the top of the class are the tuning knobs.

> **`[BaseContainerProps(configRoot: true)]` must be repeated on the modded class.** A `modded class`
> does not inherit the container attributes of the class it extends. Leave it off and the engine stops
> recognising `SCR_LoiterItemPresets` as a config root, so `LoiterItemPresets.conf` fails to load with
> `Unknown class 'SCR_LoiterItemPresets' at offset 22`, the presets object is null, and every smoke
> attempt dies in `SCR_LoiterCustomAnimData.SpawnItem` with a null-pointer VM exception. The animation
> still plays, which makes it look like a model problem rather than a script one.

**2. `Prefabs/Props/GW_AnimationProp_Joint.et`** — the prop itself. Inherits vanilla
`Prefabs/Props/Civilian/Cigar/Cigar_base.et` (`{41382F04EAC2F21D}`), which already carries
`SCR_CigaretAnimationComponent` and the `ParametricMaterialInstanceComponent` that drives the
glowing tip, and overrides the mesh to `Models/Items/Joint.xob`.

The mesh override **must reuse the base component's GUID** `{69B4E2A6D81D9B20}`, which is the
`MeshObject` declared in `Prefabs/Props/Core/AnimationProp_base.et`. Minting a fresh GUID adds a
second component instead of overriding, and the prop then fails to spawn with:

```
ENTITY (E): GenericEntity component MeshObject cannot be combined with component MeshObject
```

Vanilla's `AnimationProp_ChairMilitary_US_01.et` overrides the same component the same way, which is
where that GUID came from. Note neither `Cigar_base.et` nor `AnimationProp_Cigar.et` sets a mesh at
all — the cigar model is the default `Object` on the base's `MeshObject`, so a child only ever
overrides it.

**3. `Configs/Commanding/Emotes/GW_Emote_SmokeJoint.conf`** — a mod-owned
`SCR_ContinuousLoiterCommand`, identical to vanilla's `Emote_Smoke` except
`m_eLoiterItemPreset GW_JOINT`. Vanilla builds emote variants the same way (`Emote_Sit_Chair`
inherits `Emote_Sit.conf` and only overrides its item preset).

The catch with a new emote config is that `SCR_CommandingManagerComponent.FindCommand()` only knows
commands in its own registry, which is built from a vanilla list the mod would have to override to
extend. `GW_EmoteScheduler` sidesteps it by instantiating the config directly with
`SCR_BaseContainerTools.CreateInstanceFromPrefab()` and caching the result, falling back to the
registry lookup by name when no config is set.

### `GW_EmoteScheduler` (static helper)

A session-lived singleton holding nothing but a call-queue timer, so a queued emote outlives the
item whose action requested it. `Schedule(user, commandName, delayMs, debugLog)` queues it;
`Run` does the `SCR_CommandingManagerComponent.FindCommand()` lookup and `Execute()`.

Deliberately has no cancellation: cancelling from the action's `OnActionCanceled` would also cancel
when the action object is destroyed by the joint being consumed — the exact case this class exists to
survive — so a cancelled smoke action still plays the emote a moment later.

#### Why the other two routes were abandoned

**`SCR_LoiterUserAction` cannot drive this from an inventory item.** Subclassing it looked right —
it is the class the PatrolPoint loiter posts use — but it resolves its loiter data through a sibling
AI smart-action component and warns at runtime:

```
SCRIPT (W): Invalid SmartAction id at SCR_LoiterUserAction! Components=0, id=0
```

`StartAction` and `StopAction` still fire, so the action *looks* like it works while the vanilla body
bails out before starting the emote. Two further facts make the route a dead end rather than a
missing component: no vanilla prefab anywhere uses `m_eLoiterAnimation SMOKING` (the value appears
only in the emote config), and the action has no field for an item preset, so it could never spawn
the prop. Its `m_eLoiteringType` and `m_bHolsterWeapon` are also plain protected fields with no
`[Attribute]`, so setting them in a prefab fails at load with
`Unknown keyword/data 'm_eLoiteringType'`, and it overrides `GetActionNameScript` to force the
label "Loiter here" over the action's own UIInfo.

**Item animation graphs never contain the smoking clips.** See the fallback route below.

### `GW_SmokeJointAction` extends `GW_ToolAnimAction` (fallback, not wired)

The earlier approach: plays an item-use animation while the action is held, then consumes the joint.
Inherits `m_sAnimCommand` / `m_iAnimMode` from `GW_ToolAnimAction`, so the clip comes from the
animation graph attached to the joint prefab. Superseded by `GW_SmokeCommandAction`, but kept
because it is the only route that can work in a non-standing pose.

| Attribute | Default | Description |
|---|---|---|
| `m_bConsumeJoint` | true | Delete the joint when the action completes. |
| `m_bRequireInHands` | true | Only offer the action while the joint is in hands. |
| `m_rSmokeParticle` | "" | Optional `.ptc` spawned on the smoker for the duration. **Client-local** — not replicated to other players. |
| `m_vSmokeParticleOffset` | `0 1.6 0.15` | Particle offset from the character origin, roughly mouth height. |

#### Smoking: the loiter emote is the real mechanism

Vanilla already ships a **player smoking emote**. Two configs drive it:

```
Configs/Commanding/Emotes/Emote_Smoke.conf
  SCR_ContinuousLoiterCommand {
    m_sCommandName "Emote_Smoke"
    m_eLoiterType SMOKING
    m_aAllowedStances { STAND }
    m_CustomAnimationData SCR_LoiterCustomAnimData { m_eLoiterItemPreset CIGAR }
  }

Configs/Character/LoiterItemPresets.conf
  SCR_LoiterItemPresets {
    m_aLoiterItemsData {
      SCR_LoiterItemData { m_eID CIGAR  m_vPositionOffsetLS 0 0 -0.0099 }   // prefab defaults to the cigar prop
      SCR_LoiterItemData { m_eID CHAIR  m_sItemPrefab "…/AnimationProp_ChairMilitary_US_01.et"  … }
    }
  }
```

So `ELoiteringType` has a `SMOKING` value, `SCR_ELoiterItemID` has `CIGAR`, and
`Prefabs/Props/Civilian/Cigar/AnimationProp_Cigar.et` exists precisely to be the prop that preset
spawns. The `p_smoking_*` clips under `anims/anm/NPC/male/smoking/` are reached through this
system — not through any item animation graph.

Script entry points (`SCR_CharacterCommandHandlerComponent`): `StartCommandLoitering(SCR_LoiterCustomAnimData)`,
`StopLoitering(bool)`, `IsLoitering()`. The loiter type and flags live on
`SCR_ScriptedCharacterInputContext` (`m_iLoiteringType`, `m_iLoiteringShouldHolsterWeapon`,
`m_bLoiteringShouldAlignCharacter`, `m_bLoiteringRootMotion`, `m_bLoiteringDisablePlayerInput`,
`m_mLoiteringPosition`, `m_CustomAnimData`). The character-side command is
`SCR_CharacterCommandLoiter`.

`GW_SmokeCommandAction` reaches all of this through the command registry instead of touching the
loiter API directly: `SCR_CommandingManagerComponent.GetInstance().FindCommand("Emote_Smoke")`
returns the configured `SCR_ContinuousLoiterCommand`, and `Execute()` runs it on the smoker. The
animation, the prop spawn and the character alignment are all vanilla code.

Two consequences to keep in mind:

- **SMOKING is standing-only** (`m_aAllowedStances { STAND }`), so there is no crouch or prone
  variant to fall back on.
- **The prop comes from the emote config's item preset** — see "Giving the emote our own prop"
  below. The mod ships a reference copy of vanilla's `Emote_Smoke.conf`; note its GUID
  (`{6D1E4A07F6BCAB7E}`) may be vanilla's, in which case that copy *overrides* the base-game emote
  and editing it would change the player's radial-menu emote too. The mod's own
  `GW_Emote_SmokeJoint.conf` is used instead, so nothing vanilla needs editing.
- **The prop is the vanilla cigar**, because the preset comes from the global
  `LoiterItemPresets.conf`. To make it a joint model, that config has to be overridden in the mod
  with the `CIGAR` entry's `m_sItemPrefab` repointed — which changes the prop for vanilla
  `Emote_Smoke` too, since there is one global preset table. Adding a separate id instead needs a
  `modded enum SCR_ELoiterItemID`.

#### The item-use route (`GW_SmokeJointAction`, kept as a fallback)

`GW_SmokeJointAction` extends `GW_ToolAnimAction` and drives a command against the held item's own
animation graph. It plays a *borrowed* motion rather than a real smoking one, and is kept only as a
fallback. An item's graph (`.agr`) declares which commands it answers to in
`ControlTemplate > Commands`; `BindCommand` returns `-1` for anything else and the character just
stays idle while the action completes normally:

| Graph | Commands it declares |
|---|---|
| `Canteen_US.agr` / `Canteen_Soviet.agr` | `CMD_Item_Action` |
| `MorphineInjection_01.agr` | `CMD_HealSelf`, `CMD_HealOther`, `CMD_Revive` |
| Fuel nozzle | `CMD_Nozzle_Pick`, `CMD_Nozzle_Place` |
| Weapons | `CMD_Weapon_Reload`, `CMD_Weapon_Action_Interrupt`, `CMD_Pin_Action` |

The joint prefabs keep the **morphine** graph for the *hold* pose (`AnimationAttachments`), which
is the grip that reads correctly in hand. Since that graph declares no `CMD_Item_Action`, the
item-use route is inert against it by design — set `m_bDebugAnimLog` on the action to see the
`BindCommand` failure spelled out.

## 4. Prefabs

### Item prefabs (`Prefabs/Items/`)

All items are standalone `GenericEntity` prefabs (no base game inheritance) because Reforger's inheritance system was overwriting our overrides on save during development.

The empty pot, both seed packs, and both cannabis top items are pre-wired into every faction's arsenal catalog (`Configs/EntityCatalog/{CIV,FIA,US,USSR}/InventoryItems_EntityCatalog_*.conf`), so any vanilla arsenal in the game exposes these items without additional mod-side wiring. Custom arsenals in game modes need to add the prefab GUIDs manually or inherit from the vanilla arsenal.
 Each has:

- `MeshObject` pointing at the corresponding `.xob`
- `RplComponent` (mandatory for entity init and replication)
- `RigidBody` (Dynamic, low mass; enables drop / pickup physics)
- `InventoryItemComponent` with `SCR_ItemAttributeCollection` (display name, description, weight, size slot, icon)
- `SCR_ConsumableItemComponent` with `m_bCanBeHeld 0` (not equippable in hands like a weapon)
- `ActionsManagerComponent` with:
  - One `UserActionContext` defining the interaction context ("PotContext" for pots, "ItemContext" for seeds/buds)
  - `additionalActions` containing the actions valid on this item

Prefab responsibilities:

| Prefab | Actions | Notes |
|---|---|---|
| `GW_PotItem_Empty` | `GW_FillPotAction`, `SCR_PickUpItemAction` | Wired to spawn `GW_PotItem_Filled` on fill. |
| `GW_PotItem_Filled` | `GW_PlantAction` (Sativa), `GW_PlantAction` (Indica) | No pickup — filled pots are anchored. Each plant action gates on its own seed. |
| `GW_SeedsSativa` / `GW_SeedsIndica` | `SCR_PickUpItemAction` | Uses the shared FieldDressing USSR mesh as a placeholder + custom `weed_seeds.edds` icon. |
| `GW_CannabisTop_Sativa` / `GW_CannabisTop_Indica` | `SCR_PickUpItemAction`, `GW_RollJointAction` | Custom bud mesh (4.6 cm, `scale 4` for a chunkier in-world nug). `PreviewRenderAttributes` zooms the inventory thumbnail. `AllowInspection 1` is what puts the roll action in the in-hand inspection list. |
| `GW_Joint_Sativa` / `GW_Joint_Indica` | `SCR_PickUpItemAction`, `GW_SmokeCommandAction` | Both share `Models/Items/Joint.xob` (96 tris, ~8 cm). Same hold/inspect setup as the buds. |

#### Small items need an explicit `ItemVolume`

Every carryable item should set `SizeSetupStrategy` and `ItemVolume` in its `ItemPhysAttributes`.
Leave them out and the inventory derives the item's volume from the mesh bounds, which for anything
but a very small mesh is enough to stop **quick-move** working: the item can still be dragged between
slots by hand (that uses the explicit `m_Size` grid slot) but double-click/quick-transfer silently
does nothing, because that path first has to find a storage the item fits into by volume.

The buds hit exactly this. Their mesh is ~18 cm3 of bounds against the joint's ~0.6 cm3, so joints
quick-moved fine while buds never did. Entity `scale` is a red herring here — the buds keep
`scale 4` and work fine with an explicit volume.

Vanilla overwhelmingly uses `Manual`: 237 items versus 34 `Volume` and 3 `Size`. The mod follows
`SizeSetupStrategy Manual` + `ItemVolume 1` for small items and `Volume` + a real figure for the pot.

#### Use `SCR_PickUpItemAction`, never bare `PickupAction`

Every pickable item in the mod uses vanilla's `SCR_PickUpItemAction`. Two lookalike classes are
traps:

- **`PickupAction`** is the engine base. It registers, displays and highlights normally, but the
  inventory insert lives in `SCR_PickUpItemAction.PerformActionInternal()`, so the bare base is a
  no-op — the action simply does nothing, with no error logged. Vanilla uses
  `SCR_PickUpItemAction` in ~38 prefabs and the bare base in none.
- **The mod's own `GW_PickUpAction`** (removed) gated only on the user having an inventory manager,
  which stays true while the item is already held — so "Pick Up" appeared in the in-hand inspection
  menu. `SCR_PickUpItemAction` suppresses itself in that state.

### Plant stage prefabs (`Prefabs/Plants/`)

Every stage prefab has the same skeleton:

```
GenericEntity {
 ID "..."
 components {
  RplComponent { }
  MeshObject { Object "…Models/Plants/{StageName}.xob" }
  GW_GrowthComponent {
   m_fGrowInGameHours <hours>
   m_rNextStagePrefab "…next stage .et or empty…"
   m_bDebugLogs 0
  }
  ActionsManagerComponent {
   ActionContexts { UserActionContext ContextName "CheckContext" … }
   additionalActions {
    GW_CheckGrowthAction  (parent: CheckContext)
    GW_UprootAction       (parent: CheckContext, Duration 3.0, m_iAnimMode 2)
    GW_HarvestAction      (parent: CheckContext, Duration 2.0)   // Stage 3 only
   }
  }
 }
}
```

Growth chain wiring:

| Prefab | Model | Grow hours | Next stage |
|---|---|---|---|
| `GW_WeedPlant_Sativa_Stage1` | `Stage1Sativa.xob` | 6.0 | `GW_WeedPlant_Sativa_Stage2.et` |
| `GW_WeedPlant_Sativa_Stage2` | `Vegetation2Sativa.xob` | 8.0 | `GW_WeedPlant_Sativa_Stage3.et` |
| `GW_WeedPlant_Sativa_Stage3` | `FloweringSativa.xob` | 0.0 | *(terminal — harvestable)* |
| `GW_WeedPlant_Indica_Stage1` | `Stage1Indica.xob` | 6.0 | `GW_WeedPlant_Indica_Stage2.et` |
| `GW_WeedPlant_Indica_Stage2` | `Vegetation2Indica.xob` | 8.0 | `GW_WeedPlant_Indica_Stage3.et` |
| `GW_WeedPlant_Indica_Stage3` | `FloweringIndica.xob` | 0.0 | *(terminal — harvestable)* |

Stage 3 additionally binds:

- `GW_HarvestAction.m_rYieldPrefab` to the matching `GW_CannabisTop_{Species}.et`
- `GW_HarvestAction.m_rReplacementPrefab` to `GW_PotItem_Filled.et`

---

## 5. Materials and textures

### Plant materials (`Models/Plants/Data/`)

All `MatPBRBasic`. Textures use Enfusion suffix conventions (`_co` for base color, `_nohq` for normal, `_ca` for alpha mask) so Workbench auto-detects color space and compression.

| Material | Textures | Used by |
|---|---|---|
| `Weed.emat` | `Weed_co`, `Weed_nohq`, `Weed_ca` (+ `AlphaTest 1`) | All plant meshes (fan leaves) |
| `BabyLeaf.emat` | `BabyLeaf_co`, `BabyLeaf_nohq`, `BabyLeaf_ca` (+ `AlphaTest 1`) | Stage 1 seedling leaves |
| `BabySprout.emat` | `BabyLeaf_co`, `BabyLeaf_nohq` (no alpha — solid stem) | Stage 1 seedling stem |
| `Stem.emat` | none (solid `Color 0.111 0.133 0.072`) | Placeholder for the "Material" slot on mature plants + main-stem cylinders |
| `CannabisBud.emat` | `CannabisBud_co`, `CannabisBud_nohq` | Bud clones baked into FloweringSativa mesh |
| `CannabisBudIndica.emat` | `CannabisBudIndica_co`, `CannabisBud_nohq` (shared normal) | Bud clones baked into FloweringIndica mesh |

### Item materials (`Models/Items/Data/`)

| Material | Textures | Used by |
|---|---|---|
| `PotClay.emat` | `PotClay_co`, `PotClay_nohq` | Empty pot + most plants' pot material slot |
| `Plant_Pot_var1.emat` | same as PotClay | Filled pot + FloweringIndica (auto-created variant Workbench needed) |
| `CannabisBud.emat` | `CannabisBud_co`, `CannabisBud_nohq` | `GW_CannabisTop_Sativa` inventory item |
| `CannabisBudIndica.emat` | `CannabisBudIndica_co`, `CannabisBud_nohq` | `GW_CannabisTop_Indica` inventory item |
| `Joint.emat` | `Joint_co`, `Joint_nohq` | Both joint items |

### Assets

Third-party assets (per `README.md` Credits):

- CGTrader "Cannabis Growth Stages Pack — Sativa and Indica" — plant meshes (royalty-free license, allows commercial use and remixing inside a game).
- Sketchfab "Terracotta Pots" — pot mesh.
- Sketchfab "Weed Nug" by Skyler Young (CC BY 4.0) — cannabis bud mesh. **The attribution line in `README.md` is legally required.**

---

## 6. Mesh pipeline

Each plant / pot / bud went through the same conceptual pipeline in Blender before FBX export:

1. **Import** the source model.
2. **Decimate** high-poly source (typically ratio 0.15) so the mesh fits a game-object budget (~5–20 k tris).
3. **Scale** to sensible in-game dimensions (plants ~1.2–1.5 m tall, pot ~16 cm, buds ~6 cm).
4. **Center** on the XY origin. Pot base sits at Z=0, plant stem base at Z=0.13 (slightly into the pot rim at Z=0.16 so the plant looks rooted).
5. **Combine** pot + soil disc + plant into a single mesh so it renders as one entity.
6. **Materials**: strip Blender's `.001+` suffix duplicates and remap polygons to the first slot for each base name so shared materials end up in a single slot per mesh.
7. **Export** as FBX with `apply_scale_options='FBX_SCALE_ALL'`, `-Z forward`, `Y up`.

Stage-specific extras:

- **Vegetation2 (Stage 2)**: an 8-sided cylinder (radius 2 cm) is added as the main trunk, replacing the flat card stems that read as spikes in solid view. Branch geometry (stem faces outside the central 4 cm radius) is preserved.
- **Flowering (Stage 3)**: reuses the Vegetation2 mesh as a base, then 14 decimated bud clones (~470 tris each) are placed on branches manually in Blender. The `.blend` workfiles under `Models/Plants/_workfiles/` hold the bud placements so future edits don't lose them.

### Joint mesh

Source: a purchased "Fat Joint" model at `F:\Blender\Growing Weed\Joint`. It needed almost none of
the pipeline above — 96 tris and already 9 mm x 79 mm, so no decimation and no rescale. Two things
did need fixing on the way out of Blender:

- **It ships two UV sets.** `texcoord0` is the real one (u 0.03–1.00, v 0.01–1.00); `texcoord1` is
  degenerate (u −0.07–0.01, v 1.00–1.01). Only `texcoord0` is exported, so the importer cannot pick
  the wrong one.
- **The object carried a 90° Z rotation and an off-centre mesh.** Export clears the transform and
  centres the mesh on its bounding-box centre, leaving the long axis on Blender +Y — which the
  project's `-Z forward / Y up` export settings land on Enfusion's forward axis.

The normal map ships as `normal.dds`, which the importer does not accept; it is converted to
`Textures/Items/Joint_nohq.png`. Verified as a tangent-space map (centre pixels near 128/128/255).

Both species share one mesh and one material — a joint is a joint.

### Bud material coloring

The Sativa bud uses the raw color texture from the metascan source. The Indica bud uses a saturation-boosted, red/blue-shifted variant generated via Pillow (see the reproducer at `Textures/Items/CannabisBudIndica_co.png`) so both species share the same mesh but read as different strains.

---

## 7. Configuration and tuning

Everything gameplay-facing is exposed as prefab attributes. No script edits should be needed for typical tuning.

### Growth times

Open the corresponding stage prefab and change `GW_GrowthComponent.m_fGrowInGameHours`. Defaults: 6 h → Stage 2, 8 h → Stage 3.

Growth uses the world's in-game clock, so a server that ticks time faster (accelerated day/night in Conflict, or manual jumps in Game Master) advances plants faster. Realtime players see plants grow over multiple in-game days.

### Harvest yields

On each Stage 3 prefab, the `GW_HarvestAction` exposes `m_iYieldMin` / `m_iYieldMax`. Default range 2–4. To make Indica higher-yield, bump its max to 5 or 6.

### Shovel compatibility

`GW_ShovelRequiredAction.m_sShovelNamePatterns` is a semicolon-separated substring list. Defaults cover the vanilla US ETool and Soviet MPL50. If a shovel mod ships items whose prefab names don't match, add the substring here (e.g. `"…;dsm;klappspaten"`).

### Action durations

`Duration` on each `ScriptedUserAction` instance — how long the player must hold the interaction key. Defaults: 3 s for shovel actions, 2 s for harvest.

### Seed requirements

To allow planting without a specific seed (e.g. for a testing setup), clear `GW_PlantAction.m_rRequiredSeedItem` on the filled pot prefab. To disable seed consumption, set `m_bConsumeSeed 0`.

---

## 8. Extending the mod

### Adding a new species

1. Duplicate an existing seed prefab (e.g. `GW_SeedsSativa.et` → `GW_SeedsHybrid.et`), update display name and description.
2. Duplicate an existing bud item prefab (e.g. `GW_CannabisTop_Sativa.et` → `GW_CannabisTop_Hybrid.et`).
3. Provide new meshes / textures for the three growth stages if you want a distinct visual, or reuse an existing species' meshes if you just want a scripted-only variant.
4. Duplicate the three stage prefabs (Stage 1/2/3) and update:
   - `MeshObject.Object` for each stage
   - `GW_GrowthComponent.m_rNextStagePrefab` chain
   - `GW_HarvestAction.m_rYieldPrefab` on Stage 3 → the new bud item
5. Add a third `GW_PlantAction` to `GW_PotItem_Filled.et` pointing at the new seedling and seed.

### Adding a fourth growth stage

Insert a new stage prefab between the existing ones and rewire the `m_rNextStagePrefab` chain. `GW_GrowthComponent` doesn't care how many stages there are — it just spawns whatever prefab you point at.

### Custom yield item

`GW_HarvestAction.m_rYieldPrefab` accepts any `.et` — you can point it at a joint, a bag of dried leaves, a currency token, whatever. Set `m_iYieldMin` and `m_iYieldMax` to control the drop count.

### Wiring into a game mode

Prefabs are usable directly by any game mode. Spawn `GW_PotItem_Empty.et` in a shop's stock, in a loot table, at a scenario objective, or hand it to a character on spawn — no additional wiring is needed. Same for the seeds and buds.

For criminality / heat systems: subclass `GW_HarvestAction` in your own mod and override `PerformAction` to notify your heat manager before calling `super.PerformAction`. Then create inherited variants of the Stage 3 prefabs that use your subclass.

---

## 9. Known quirks

- The `Stem.emat` placeholder is a solid olive color. Replacing it with a bark texture is on the tuning-todo list (see `MEMORY.md` in the developer's private notes if applicable).
- `Plant_Pot_var1.emat` and `PotClay.emat` are two near-identical pot materials. Both are used by different meshes because Workbench auto-created one during a specific import; consolidating them is possible but low priority since they behave the same.
- The `FloweringSativa` mesh had many free-floating leaf-tip triangles from the source model that were cleaned up by deleting island fragments with small XY extent or ≤2 faces. If a future re-export shows the same artifacts, apply the same filter (see git history for the exact script).
- `Vegetation2Indica` is Stage 2 for Indica but uses the source model's "Vegetation2" variant. The naming is a leftover from earlier iterations; changing it would require updating the `MeshObject.Object` reference and re-registering the resource. Not worth the churn.

---

## 10. Publishing checklist

Before uploading a new version to the Workshop:

1. **Confirm the Credits section in `README.md` is up to date** — every third-party asset must be attributed. The Weed Nug specifically requires the exact CC BY 4.0 attribution line.
2. **`resourceDatabase.rdb` is regenerated on load** and is gitignored — don't worry about it.
3. **Check no `.log`, `.tmp`, or `.blend1` files leaked** into the tracked tree. `.gitignore` should catch them.
4. **Remove `Scripts/WorkbenchGame/EnfusionMCP/`** if present — those are development-time handlers for the Claude Code Enfusion MCP integration and should never ship. Use `wb_cleanup` via the MCP tool or delete manually.
5. **Verify the growth loop end to end** in a Play session: pick up pot → fill → plant seed → grow through stages → harvest → replant. Do this for both species.
