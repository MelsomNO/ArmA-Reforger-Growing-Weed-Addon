// Plants a seedling in a filled pot. Requires:
//   - The configured seed item held in the character's hands (not a shovel, and not merely
//     carried in inventory -- the matching seed pack must be the item currently in hands)
// On perform: consumes the held seed and swaps the pot for the seedling prefab.
//
// Extends GW_ToolAnimAction (not GW_ShovelRequiredAction) so the tool-animation plumbing
// stays available -- note the animation played comes from the held item's animation graph,
// so with a seed pack in hands this no longer plays the shovel dig loop.
class GW_PlantAction : GW_ToolAnimAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Seedling prefab spawned in place of the pot on plant complete (typically GW_WeedPlant_*_Stage1.et).", params: "et")]
	protected ResourceName m_rSeedlingPrefab;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Seed item prefab that must be held in the player's hands. Leave empty to allow planting without a seed requirement.", params: "et")]
	protected ResourceName m_rRequiredSeedItem;

	[Attribute(defvalue: "true", desc: "If true, consume the held seed item on plant complete.")]
	protected bool m_bConsumeSeed;

	[Attribute(defvalue: "false", desc: "Print what the action sees in the player's hands to the console. Turn on to diagnose a locked action.")]
	protected bool m_bDebugLog;

	override bool CanBePerformedScript(IEntity user)
	{
		if (m_rSeedlingPrefab.IsEmpty())
			return false;

		// No seed configured -> no gate
		if (m_rRequiredSeedItem.IsEmpty())
			return true;

		// Seed-in-hands gate
		IEntity held = GetItemInHands(user);

		if (m_bDebugLog)
		{
			string heldName = "<null>";
			if (held)
			{
				EntityPrefabData pd = held.GetPrefabData();
				if (pd)
					heldName = pd.GetPrefabName();
				else
					heldName = "<no prefab data>";
			}
			Print("[GW_PlantAction] in hands: " + heldName + " | required: " + m_rRequiredSeedItem, LogLevel.NORMAL);
		}

		return IsSeedMatch(held);
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		if (m_rSeedlingPrefab.IsEmpty() || !pOwnerEntity)
			return;

		if (m_bConsumeSeed && !m_rRequiredSeedItem.IsEmpty())
			ConsumeHeldSeed(pUserEntity);

		Resource res = Resource.Load(m_rSeedlingPrefab);
		if (!res.IsValid())
			return;

		vector transform[4];
		pOwnerEntity.GetWorldTransform(transform);

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform = transform;

		IEntity seedling = GetGame().SpawnEntityPrefab(res, pOwnerEntity.GetWorld(), params);
		if (!seedling)
			return;

		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}

	// Item currently in the character's hands. A seed pack is held through
	// SCR_ConsumableItemComponent, so it can surface via the gadget manager, the character
	// controller, or (for weapons) the weapon manager -- try all three.
	protected IEntity GetItemInHands(IEntity user)
	{
		if (!user)
			return null;

		SCR_GadgetManagerComponent gm = SCR_GadgetManagerComponent.GetGadgetManager(user);
		if (gm)
		{
			IEntity gadget = gm.GetHeldGadget();
			if (gadget)
				return gadget;
		}

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (character)
		{
			CharacterControllerComponent charController = character.GetCharacterController();
			if (charController)
			{
				IEntity inHands = charController.GetCurrentItemInHands();
				if (inHands)
					return inHands;
			}
		}

		BaseWeaponManagerComponent wm = BaseWeaponManagerComponent.Cast(user.FindComponent(BaseWeaponManagerComponent));
		if (wm)
		{
			BaseWeaponComponent weapon = wm.GetCurrent();
			if (weapon)
				return weapon.GetOwner();
		}

		return null;
	}

	// Tolerant prefab comparison: GetPrefabName() and the configured ResourceName should both
	// read "{GUID}Path/To/Prefab.et", but match on either half alone so a differing GUID
	// prefix or path spelling does not silently lock the action.
	protected bool IsSeedMatch(IEntity item)
	{
		if (!item)
			return false;

		EntityPrefabData prefabData = item.GetPrefabData();
		if (!prefabData)
			return false;

		string held = prefabData.GetPrefabName();
		string required = m_rRequiredSeedItem;

		if (held == required)
			return true;

		string heldGuid = SplitGuid(held);
		if (!heldGuid.IsEmpty() && heldGuid == SplitGuid(required))
			return true;

		string heldPath = SplitPath(held);
		return !heldPath.IsEmpty() && heldPath == SplitPath(required);
	}

	// "{GUID}Path.et" -> "GUID" (empty when there is no GUID prefix)
	protected string SplitGuid(string resName)
	{
		int close = resName.IndexOf("}");
		if (close <= 0 || resName.IndexOf("{") != 0)
			return "";
		return resName.Substring(1, close - 1);
	}

	// "{GUID}Path.et" -> "Path.et"
	protected string SplitPath(string resName)
	{
		int close = resName.IndexOf("}");
		if (close < 0)
			return resName;
		return resName.Substring(close + 1, resName.Length() - close - 1);
	}

	protected void ConsumeHeldSeed(IEntity user)
	{
		IEntity held = GetItemInHands(user);
		if (!IsSeedMatch(held))
			return;

		SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (invMgr && invMgr.TryDeleteItem(held))
			return;

		// Fallback: the item may not be tracked by the storage manager while in hands
		SCR_EntityHelper.DeleteEntityAndChildren(held);
	}
}
