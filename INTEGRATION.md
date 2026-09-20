# Integration Guide

For mod developers who want to plug the **Growing Weed** items (empty pot, growing plant, cannabis tops) into their own systems — economy, criminality, drug labs, faction contraband, quest rewards, etc.

This addon is intentionally self-contained: it exposes prefabs and a couple of small components you can spawn, listen to, or reference by resource. There is no bespoke API surface to learn — it uses the standard Enfusion patterns.

---

## 1. Add the dependency

In your mod's `addon.gproj`, add Growing Weed to your `Dependencies` block:

```
Dependencies {
 "6A5AB6991AF3656D"   // Growing Weed
 "65F658C80F83E459"   // Cannabis Sativa (kuka-) — required for models
 "58D0FB3206B6F859"   // Arma Reforger Data
}
```

Addon metadata:

| Field | Value |
|---|---|
| Title | Growing Weed |
| Project ID | `GrowingWeed` |
| GUID | `6A5AB6991AF3656D` |

Once the dependency is set, all prefabs, components, and actions ship with `GrowingWeed/` as their resource root and are available to your scripts and prefabs.

---

## 2. Key prefabs

Reference these by resource name (drag from the Resource Browser into your prefab, or use `Resource.Load()` in script). Exact paths as they appear under `GrowingWeed/`:

| Item | Purpose | Typical use |
|---|---|---|
| `Prefabs/Items/GW_PlantPot_Empty.et` | Empty pot, carryable item. Plant with shovel to start the cycle. | Sell at hardware store; loot table; quest starter |
| `Prefabs/Plants/GW_WeedPlant_Stage1.et` | Seedling (freshly planted) | Pre-placed grow-ops, raid targets |
| `Prefabs/Plants/GW_WeedPlant_Stage2.et` | Mid-growth plant | Pre-placed grow-ops |
| `Prefabs/Plants/GW_WeedPlant_Stage3.et` | Mature, harvestable plant | Pre-placed grow-ops, raid loot |
| `Prefabs/Items/GW_CannabisTop.et` | The harvest yield item (from Cannabis Sativa base pack, re-exposed) | Currency for dealers, processing input |

> Verify the exact filenames in the Resource Browser under `GrowingWeed/` before hard-coding them; the folder layout above matches what ships in `resourceDatabase.rdb`.

---

## 3. Spawning from script

Standard Reforger prefab spawn — nothing addon-specific required.

```csharp
// Spawn an empty pot at a vendor / lootspawner / player hand
ResourceName potRes = "{GUID_OF_PREFAB}GW_PlantPot_Empty.et";
Resource res = Resource.Load(potRes);
if (!res.IsValid())
    return;

EntitySpawnParams params = new EntitySpawnParams();
params.TransformMode = ETransformMode.WORLD;
params.Transform[3] = spawnPos;

IEntity pot = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
```

To spawn directly into a player's inventory (economy / purchase flow), use the standard inventory manager:

```csharp
SCR_InventoryStorageManagerComponent invMgr =
    SCR_InventoryStorageManagerComponent.Cast(
        player.FindComponent(SCR_InventoryStorageManagerComponent));

invMgr.TrySpawnPrefabToStorage(potRes);
```

The same pattern works for `GW_CannabisTop.et` when paying out a harvest reward from a scripted event.

---

## 4. Reacting to growth / harvest

The two components you'll interact with are:

### `GW_GrowthComponent`
Sits on each plant stage prefab. Attributes:

- `m_fGrowInGameHours` — hours (in-game clock) until this stage advances.
- Internal state tracks elapsed time; when the threshold is hit, the entity is replaced by the next stage prefab.

If you want to **observe** growth (e.g., a criminality system that flags mature plots for police AI), find the component on the entity and read its progress. Because stage transitions replace the entity, hook `EOnDelete` on the previous stage or scan periodically for the Stage 3 prefab in an area.

### `GW_HarvestAction`
Sits on the mature (Stage 3) plant. Attributes:

- `m_iYield` — number of `GW_CannabisTop` items produced (default 3).
- `m_sShovelNamePatterns` — shovel-matching pattern list.
- `Duration` — action time.

To gate harvest through your own system (e.g., only licensed players, only inside a designated zone), subclass `GW_HarvestAction` in your mod and override `CanBePerformedScript` / `PerformAction` before delegating to base. Then swap the action on the Stage 3 prefab (inherited variant) so vanilla Growing Weed players still play normally, but your gamemode enforces the extra rule.

Example gate:

```csharp
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
        MyCrimeTracker.RegisterHarvest(pUserEntity, m_iYield);
    }
}
```

---

## 5. Detecting cannabis-tops in inventory

For economy shops or police search mechanics, resolve the item by prefab resource on any storage entity:

```csharp
static const ResourceName WEED_TOP = "{GUID}GW_CannabisTop.et";

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
        if (SCR_EntityHelper.GetPrefabName(item).EndsWith("GW_CannabisTop.et"))
            count++;
    }
    return count;
}
```

Use that count directly against price tables (economy), contraband thresholds (criminality), or reputation deltas (faction).

---

## 6. Recommended integration patterns

| Your system | How to integrate |
|---|---|
| **Economy / shop** | Sell `GW_PlantPot_Empty.et` as stock; buy back `GW_CannabisTop.et` at a set price per unit. Pure prefab wiring, no code needed. |
| **Criminality / heat** | Subclass `GW_HarvestAction` to add heat on harvest; scan for Stage 3 prefabs inside patrol zones and dispatch AI. |
| **Drug processing** | Add a workbench in your mod that consumes N `GW_CannabisTop.et` and outputs a joint / edible / brick prefab you define. |
| **Quests** | Objective: "Deliver 10 tops to X" — count via prefab resource match, consume on turn-in. |
| **Raid loot** | Place Stage 3 plants in points-of-interest; players harvest with any shovel; no scripting required. |
| **Persistence** | Growth state lives on `GW_GrowthComponent` and follows the game's day/night clock, so any persistence layer that snapshots component state will round-trip correctly. |

---

## 7. Tuning without forking

All numbers live on prefab attributes. If your gamemode needs a longer grow cycle or a bigger yield, **inherit** the prefabs in your own mod rather than forking Growing Weed:

- Duplicate `GW_WeedPlant_Stage1/2/3.et` as inherited prefabs in your addon.
- Override `m_fGrowInGameHours` and `m_iYield` on the copies.
- Reference your inherited prefabs instead of the originals in your gamemode setup.

Your changes ride on top; Growing Weed updates remain drop-in compatible.

---

## 8. Compatibility notes

- **Shovel requirement** — planting and uprooting require any hand-held item whose prefab name contains one of the substrings in `m_sShovelNamePatterns` (default: `shovel;etool;entrench;spade`). If your gamemode ships a custom shovel, either name it accordingly or add your substring to the plant/uproot actions on inherited variants.
- **In-game time** — growth uses the world's day/night clock, not real time. Gamemodes with accelerated or paused time will speed up / freeze growth accordingly — usually the intended behavior for RP servers.
- **Server-authoritative** — spawn and state changes should be performed on the server (or via Game Master); clients will replicate.

---

## 9. Contact

For integration questions, open an issue on the mod repo or ping the author. When reporting integration problems, include your `addon.gproj` dependencies block and the resource path you're referencing.
