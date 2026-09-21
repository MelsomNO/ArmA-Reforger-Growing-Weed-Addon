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
│   └── GW_PickUpAction.c
├── Prefabs/
│   ├── Items/                           # Carryable / world-placeable items
│   │   ├── GW_PotItem_Empty.et
│   │   ├── GW_PotItem_Filled.et
│   │   ├── GW_SeedsSativa.et
│   │   ├── GW_SeedsIndica.et
│   │   ├── GW_CannabisTop_Sativa.et
│   │   └── GW_CannabisTop_Indica.et
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

Attaches to every plant stage. Removes the plant entirely. `m_iAnimMode 2` (demolition/dig-out) in each prefab entry.

Attributes:

| Attribute | Description |
|---|---|
| `m_rReturnedItemPrefab` | Optional prefab granted to the player on uproot. Currently unwired (empty) since the design chose to keep uprooting destructive rather than reversible. Set this to `GW_PotItem_Empty.et` if you want players to get the pot back. |

### `GW_CheckGrowthAction` (plain `ScriptedUserAction`)

Attaches to every plant stage. Shows a hint with progress percent and remaining hours.

Uses `SCR_HintManagerComponent.GetInstance().ShowCustomHint(msg, "Growing Weed", 4)`. Terminal stages report "This plant is fully grown."

### `GW_PickUpAction` (plain `ScriptedUserAction`)

Attached to items on the ground: empty pot, both seed packs, both cannabis top items. Moves the entity into the player's inventory via `SCR_InventoryStorageManagerComponent.TryInsertItem`.

`CanBePerformedScript` returns false if the user lacks an inventory manager (rare, but avoids no-op action showing up for spectators/vehicles).

---

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
| `GW_PotItem_Empty` | `GW_FillPotAction`, `GW_PickUpAction` | Wired to spawn `GW_PotItem_Filled` on fill. |
| `GW_PotItem_Filled` | `GW_PlantAction` (Sativa), `GW_PlantAction` (Indica) | No pickup — filled pots are anchored. Each plant action gates on its own seed. |
| `GW_SeedsSativa` / `GW_SeedsIndica` | `GW_PickUpAction` | Uses the shared FieldDressing USSR mesh as a placeholder + custom `weed_seeds.edds` icon. |
| `GW_CannabisTop_Sativa` / `GW_CannabisTop_Indica` | `GW_PickUpAction` | Custom bud mesh. `PreviewRenderAttributes` with `CameraDistanceToItem 0.08` zooms the inventory thumbnail because the bud is small (~6 cm). |

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
