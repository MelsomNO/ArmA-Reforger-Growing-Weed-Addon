# Integration Guide

For mod developers who want to plug the **Growing Weed** items (pot, plant, seeds, cannabis tops) into their own systems — economy, criminality, drug labs, faction contraband, quest rewards, arsenal loadouts, etc.

This addon is intentionally self-contained. It exposes prefabs, actions, and one gameplay component. There is no bespoke API surface to learn — it uses the standard Enfusion patterns.

---

## 1. Add the dependency

In your mod's `addon.gproj`, add Growing Weed to your `Dependencies` block:

```
Dependencies {
 "6A5AB6991AF3656D"   // Growing Weed
 "58D0FB3206B6F859"   // Arma Reforger Data
}
```

Growing Weed itself only depends on the base game — no more Cannabis Sativa mod required.

Addon metadata:

| Field | Value |
|---|---|
| Title | Growing Weed |
| Project ID | `GrowingWeed` |
| GUID | `6A5AB6991AF3656D` |

Once the dependency is set, all prefabs, components, and actions ship with `GrowingWeed/` as their resource root and are available to your scripts and prefabs.

---

## 2. Arsenal availability out of the box

The mod ships with entity-catalog configs for all four factions (USSR, US, FIA, CIV) under `Configs/EntityCatalog/`. That means every player who spawns from a vanilla arsenal can pull these items from the "Consumables" or "Misc" tab without any extra wiring on your side:

- Empty Plant Pot
- Sativa Seeds
- Indica Seeds
- Cannabis Top — Sativa
- Cannabis Top — Indica

If your game mode uses a custom arsenal loadout, either inherit the vanilla arsenal (items appear automatically) or add these prefab GUIDs to your custom catalog.

---

## 3. Key prefabs

Reference these by resource name (drag from the Resource Browser into your prefab, or use `Resource.Load()` in script).

### Items (`GrowingWeed/Prefabs/Items/`)

| Prefab | Purpose | Typical use |
|---|---|---|
| `GW_PotItem_Empty.et` | Empty clay pot, carryable, pick-uppable | Sell at hardware store; loot table; quest starter |
| `GW_PotItem_Filled.et` | Filled pot (dirt inside), world-anchored | Spawned by `GW_FillPotAction` — usually not placed directly |
| `GW_SeedsSativa.et` | Sativa seed pack (inventory item) | Vendor stock; drug lab reward |
| `GW_SeedsIndica.et` | Indica seed pack (inventory item) | Vendor stock; drug lab reward |
| `GW_CannabisTop_Sativa.et` | Sativa harvest yield (inventory item) | Currency for dealers, processing input |
| `GW_CannabisTop_Indica.et` | Indica harvest yield (inventory item) | Currency for dealers, processing input |

### Plants (`GrowingWeed/Prefabs/Plants/`)

Two species × three stages = six prefabs. Each stage prefab uses the previous naming convention:

| Species | Stage 1 (seedling) | Stage 2 (vegetative) | Stage 3 (mature, harvestable) |
|---|---|---|---|
| Sativa | `GW_WeedPlant_Sativa_Stage1.et` | `GW_WeedPlant_Sativa_Stage2.et` | `GW_WeedPlant_Sativa_Stage3.et` |
| Indica | `GW_WeedPlant_Indica_Stage1.et` | `GW_WeedPlant_Indica_Stage2.et` | `GW_WeedPlant_Indica_Stage3.et` |

Typical use: pre-place Stage 3 prefabs at raid locations for harvestable loot, or spawn Stage 1 dynamically to start a scripted grow-op.

> Verify the exact GUIDs in the Resource Browser under `GrowingWeed/` before hard-coding them in your code.

---

## 4. Spawning from script

Standard Reforger prefab spawn — nothing addon-specific required.

```enforce
// Spawn an empty pot at a vendor / lootspawner / player hand
ResourceName potRes = "{GUID_OF_PREFAB}Prefabs/Items/GW_PotItem_Empty.et";
Resource res = Resource.Load(potRes);
if (!res.IsValid())
    return;

EntitySpawnParams params = new EntitySpawnParams();
params.TransformMode = ETransformMode.WORLD;
params.Transform[3] = spawnPos;

IEntity pot = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
```

To spawn directly into a player's inventory (economy / purchase flow), use the standard inventory manager:

```enforce
SCR_InventoryStorageManagerComponent invMgr =
    SCR_InventoryStorageManagerComponent.Cast(
        player.FindComponent(SCR_InventoryStorageManagerComponent));

invMgr.TrySpawnPrefabToStorage(potRes);
```

The same pattern works for `GW_SeedsSativa/Indica.et` and `GW_CannabisTop_Sativa/Indica.et` when paying out rewards or seeding vendor stock.

---

## 5. Reacting to growth and harvest

### `GW_GrowthComponent`

Sits on each plant stage prefab. Server-authoritative timer that advances the plant through stages.

Attributes:

| Attribute | Description |
|---|---|
| `m_fGrowInGameHours` | Hours (in-game clock) until this stage advances. Defaults: Stage 1 = 6h, Stage 2 = 8h. Stage 3 is terminal (harvest-only). |
| `m_rNextStagePrefab` | Prefab spawned when this stage completes. Empty on Stage 3. |
| `m_bDebugLogs` | Enable console logging for tuning. |

Public methods you can call from outside:

- `float GetProgressPercent()` — 0 to 100.
- `float GetRemainingInGameHours()` — hours left before advancement.

If you want to **observe** growth (e.g. a criminality system that flags mature plots for police AI), find the component on the entity and read its progress. Because stage transitions replace the entity, hook `EOnDelete` on the previous stage or scan periodically for the Stage 3 prefab in an area.

### `GW_HarvestAction`

Sits on the Stage 3 plants. Bare-handed action that yields cannabis tops.

Attributes:

| Attribute | Default | Description |
|---|---|---|
| `m_rYieldPrefab` | (wired to matching species) | Item spawned into the player's inventory on harvest. |
| `m_iYieldMin` | 2 | Minimum count granted. |
| `m_iYieldMax` | 4 | Maximum count granted. Actual is uniform-random in `[min, max]`. |
| `m_rReplacementPrefab` | `GW_PotItem_Filled.et` | Prefab that replaces the plant post-harvest (default: a filled pot ready to replant). |
| `m_bRequireFullyGrown` | true | Refuses harvest if the growth timer hasn't finished. |

To gate harvest through your own system (e.g. only licensed players, only inside a designated zone), subclass `GW_HarvestAction` in your mod and override `CanBePerformedScript` / `PerformAction`:

```enforce
class MyGamemode_HarvestAction : GW_HarvestAction
{
    override bool CanBePerformedScript(IEntity user)
    {
        if (!super.CanBePerformedScript(user))
            return false;

        // Your rule: player must be in a "grow zone"
        return MyZoneManager.IsPlayerInGrowZone(user);
    }

    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
        super.PerformAction(pOwnerEntity, pUserEntity);

        // Fire your own economy / criminality event
        MyCrimeTracker.RegisterHarvest(pUserEntity);
    }
}
```

Then create inherited Stage 3 prefabs in your mod that swap in your subclass, and reference those in your game mode.

### `GW_PlantAction`

Sits on `GW_PotItem_Filled`, one instance per species. Gates on shovel-in-hand AND matching seed in inventory. Consumes one seed on complete.

Attributes:

| Attribute | Description |
|---|---|
| `m_rSeedlingPrefab` | Stage 1 prefab to spawn. |
| `m_rRequiredSeedItem` | Seed prefab that must be present. Empty = no seed check. |
| `m_bConsumeSeed` | Whether to remove one seed on complete. |

Useful for adding "hybrid" strains: add a new `GW_PlantAction` entry on the filled pot pointing at your hybrid seedling + a custom seed prefab.

---

## 6. Detecting cannabis-tops or seeds in inventory

For economy shops, police search mechanics, or turn-in quests:

```enforce
static const ResourceName WEED_TOP_SATIVA = "{GUID}Prefabs/Items/GW_CannabisTop_Sativa.et";
static const ResourceName WEED_TOP_INDICA = "{GUID}Prefabs/Items/GW_CannabisTop_Indica.et";

int CountWeedTops(IEntity player)
{
    SCR_InventoryStorageManagerComponent inv =
        SCR_InventoryStorageManagerComponent.Cast(
            player.FindComponent(SCR_InventoryStorageManagerComponent));
    if (!inv)
        return 0;

    array<IEntity> items = {};
    inv.GetItems(items);

    int count;
    foreach (IEntity item : items)
    {
        EntityPrefabData prefabData = item.GetPrefabData();
        if (!prefabData)
            continue;
        ResourceName name = prefabData.GetPrefabName();
        if (name == WEED_TOP_SATIVA || name == WEED_TOP_INDICA)
            count++;
    }
    return count;
}
```

Use that count against price tables (economy), contraband thresholds (criminality), or reputation deltas (faction).

---

## 7. Recommended integration patterns

| Your system | How to integrate |
|---|---|
| **Economy / shop** | Sell `GW_PotItem_Empty.et` and seed packs; buy back cannabis tops at a set price per unit. Pure prefab wiring, no code needed. |
| **Criminality / heat** | Subclass `GW_HarvestAction` to add heat on harvest; scan for Stage 3 prefabs inside patrol zones and dispatch AI. |
| **Drug processing** | Add a workbench in your mod that consumes N cannabis tops and outputs a joint / edible / brick prefab you define. |
| **Quests** | Objective: "Deliver 10 tops to X" — count via prefab resource match, consume on turn-in. |
| **Raid loot** | Place Stage 3 plants in points-of-interest; players harvest with any bare hands; no scripting required. |
| **Persistence** | Growth state lives on `GW_GrowthComponent` and follows the game's day/night clock, so any persistence layer that snapshots component state will round-trip correctly. |
| **Arsenal loadouts** | Vanilla arsenals already include the items via faction entity catalogs. For custom loadouts, add the item GUIDs to your `LoadoutManager` config. |

---

## 8. Tuning without forking

All gameplay numbers live on prefab attributes. If your game mode needs a longer grow cycle, a bigger yield, or different balance, **inherit** the prefabs in your own mod rather than forking Growing Weed:

- Inherit `GW_WeedPlant_{Species}_Stage{N}.et` in your addon.
- Override `m_fGrowInGameHours` on the growth component and `m_iYieldMin` / `m_iYieldMax` on the harvest action (Stage 3).
- Reference your inherited prefabs in your scripts / world placements.

Your changes ride on top; Growing Weed updates remain drop-in compatible.

---

## 9. Compatibility notes

- **Shovel requirement** — filling pots, planting seeds, and uprooting require the player to hold an item whose prefab name contains one of the substrings in `GW_ShovelRequiredAction.m_sShovelNamePatterns` (default: `shovel;etool;entrench;spade`). If your mod ships a custom shovel, either name it accordingly or add your substring on inherited action variants.
- **Harvesting is bare-handed** — no shovel required, no seed required. Only gated by `m_bRequireFullyGrown`.
- **In-game time** — growth uses the world's day/night clock, not real time. Game modes with accelerated or paused time will speed up / freeze growth accordingly — usually the intended behavior for RP servers.
- **Server-authoritative** — all state changes (growth advance, entity swaps, inventory grants) run on the server side and replicate to clients.
- **Two species** — mod ships both Sativa and Indica strains. Each has its own seed, plant meshes, and bud yield item. Add a third strain by duplicating the six prefabs and updating the `m_rNextStagePrefab` chain (see `docs/ARCHITECTURE.md` for the full recipe).

---

## 10. Contact

For integration questions, open an issue on the mod repo or ping the author. When reporting integration problems, include your `addon.gproj` dependencies block and the resource path you're referencing.
