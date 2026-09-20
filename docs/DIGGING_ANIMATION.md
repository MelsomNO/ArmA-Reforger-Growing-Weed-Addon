# Digging Animation Implementation

How the vanilla ETool shovel dig animation was made to play during the plant and uproot user actions in this mod.

## Goal

When a player performs "Plant with Shovel" (on the pot) or "Uproot Plant" (on any growth stage), the character should play the vanilla Arma Reforger digging animation — the same loop used to build compositions in Conflict, Game Master, and Combat Ops. Two distinct animations are wanted:

- **Build/dig-down** — for planting (packing the soil in).
- **Demolition/dig-out** — for uprooting (breaking the soil open).

Both animation loops exist in the base game as `.anm` clips (`p_cro_supplies_buildUS_low_*`, `p_cro_supplies_demolitionUS_low_*`) and are bound into the character's animation graph whenever a shovel/entrenching-tool gadget is held.

## What did not work

Several approaches were tried before the working one:

1. **`SCR_GadgetManagerComponent.ToggleHeldGadget(true)`** on the held ETool — toggled the gadget's active state but did not transition the character into the dig animation. The dig loop is not simply gadget-toggle-on; it requires an item-use context.
2. **Inheriting the action from `SCR_CampaignBuildingBuildUserAction`** — compiled cleanly but did nothing at runtime, because the parent class exits early when its `m_LayoutComponent` (which lives on buildable compositions like sandbags) is `null`. Plants don't have that component.
3. **Refactoring the pot as a deployable inventory item** (`SCR_BaseDeployableInventoryItemComponent` + `SCR_DeployablePlaceableItemComponent` + `SCR_DeployMultiPartInventoryItemAction`) — showed a text-less "deploy" action that did nothing when triggered. The vanilla deploy flow expects the item to be equipped from inventory (not interacted with on the ground), and its UI/placement bindings come from inheritance chains that aren't reproducible without the full sample source.

None of these worked because the dig animation is not triggered by "shovel state" alone — it's triggered by a specific *character controller call* that binds an item-use command with the shovel as its tool entity.

## The working mechanism

Discovered by inspecting the source of `SCR_CampaignBuildingBuildUserAction.c` (the vanilla class that drives the dig loop when players build compositions with a shovel). Its `OnActionStart` does this:

```enforce
CharacterControllerComponent charController = character.GetCharacterController();
CharacterAnimationComponent   pAnimationComponent = charController.GetAnimationComponent();
int itemActionId = pAnimationComponent.BindCommand("CMD_Item_Action");

ItemUseParameters params = new ItemUseParameters();
params.SetEntity(GetBuildingTool(pUserEntity));   // the ETool entity
params.SetAllowMovementDuringAction(false);
params.SetKeepInHandAfterSuccess(true);
params.SetCommandID(itemActionId);
params.SetCommandIntArg(1);                        // 1 = build, 2 = demolition

charController.TryUseItemOverrideParams(params);
```

The animation is stopped on cancel/confirm by:

```enforce
CharacterCommandHandlerComponent handler = pAnimationComponent.GetCommandHandler();
handler.FinishItemUse(true);
```

Three moving pieces make this work together:

1. **`CMD_Item_Action`** — a command bound into the character's animation graph. When the character has an entrenching tool in hands, this command is wired to the shovel's animation graph module (which contains the build/demolition states).
2. **`ItemUseParameters.SetEntity`** — must be set to the actual tool entity (the ETool in the player's hand). Without it, the animation graph has no tool context and stays idle.
3. **`SetCommandIntArg`** — selects the specific animation state within the tool's module. `1` maps to the build/dig-down loop; `2` maps to the demolition/dig-out loop. Other values fall back to idle.

## Class structure

Two script classes carry this logic in the mod:

### `GW_ToolAnimAction` (base)

Reusable base class for any action that wants to drive a tool animation on the character. It has two exposed attributes:

- `m_sAnimCommand` (string, default `"CMD_Item_Action"`) — command name bound on the character.
- `m_iAnimMode` (int, default `1`) — the command int arg.

Lifecycle methods:

- `OnActionStart(pUserEntity)` — looks up the held item via `SCR_GadgetManagerComponent.GetHeldGadget()` (falling back to `BaseWeaponManagerComponent.GetCurrent()`), then makes the `BindCommand → ItemUseParameters → TryUseItemOverrideParams` call described above. If no item is held, the animation is silently skipped and the action still runs.
- `OnActionCanceled` and `OnConfirmed` — call `FinishItemUse(true)` on the character's command handler to end the loop cleanly.
- The destructor also calls `FinishItemUse` if the action is destroyed mid-use, so the character never gets stuck in the dig pose.

### `GW_ShovelRequiredAction` (extends `GW_ToolAnimAction`)

Adds a check that gates the action on the player holding a recognised shovel. The gate is a case-insensitive substring match on the held item's prefab path, driven by:

- `m_sShovelNamePatterns` (string, default `"shovel;etool;entrench;spade"`) — semicolon-separated list.

Held item prefab path is read via `IEntity.GetPrefabData().GetPrefabName()`. If no match, `CanBePerformedScript` sets a "Held item is not a shovel" cannot-perform reason and returns false.

`GW_PlantAction` and `GW_RemovePlantAction` both extend `GW_ShovelRequiredAction`. They each override `PerformAction` to spawn/delete the appropriate entities, then rely on the inherited animation lifecycle from `GW_ToolAnimAction`.

## Per-action animation configuration

The two shovel actions are configured to drive different animations by setting `m_iAnimMode` on their prefab entries:

- **`GW_PlantAction`** on `GW_PotItem.et` — uses the default `m_iAnimMode = 1` (build/dig-down). Represents the player packing the seedling into the ground.
- **`GW_RemovePlantAction`** on all three growth-stage prefabs — sets `m_iAnimMode 2` (demolition/dig-out). Represents the player breaking the plant free.

The `Duration` attribute on each action controls how long the animation loop runs before `PerformAction` fires.

## Why the harvest action has no animation

`GW_HarvestAction` deliberately does not use `GW_ToolAnimAction`. Two reasons:

1. Harvesting is designed to work bare-handed — the player shouldn't need a shovel to pick tops off a mature plant. Since `TryUseItemOverrideParams` requires a tool entity, no held tool means no animation regardless of what command is bound.
2. The clip we would want (a one-handed "pluck from ground" motion, e.g. `p_1hd_cro_pick_inv_hi.anm`) is not reachable through `CMD_Item_Action`. Whatever command it's bound to lives in a part of the character animation graph we couldn't map to a callable command string.

The harvest action therefore just runs its `Duration 1.5` progress bar and fires without playing a specific animation.

## Extending

To wire another action to a tool animation:

1. Have the action's class extend `GW_ToolAnimAction` (or `GW_ShovelRequiredAction` if you also want the shovel gate).
2. Override `PerformAction` to do whatever the action does.
3. Optionally set `m_sAnimCommand` and `m_iAnimMode` in the prefab entry to pick a different command/state combination.

Do **not** call `super.PerformAction` if inheriting from `SCR_CampaignBuildingBuildUserAction` directly — that parent's `PerformAction` targets composition network components that don't exist outside Conflict base building.
