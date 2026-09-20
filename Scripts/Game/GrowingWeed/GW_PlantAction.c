// Plants a seedling in a filled pot. Requires:
//   - A shovel-type item in the character's hands (inherited from GW_ShovelRequiredAction)
//   - The configured seed item present in the player's inventory
// On perform: consumes one seed from inventory and swaps the pot for the seedling prefab.
// Plays the build/dig-down animation (m_iAnimMode 1 default from GW_ToolAnimAction).
class GW_PlantAction : GW_ShovelRequiredAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Seedling prefab spawned in place of the pot on plant complete (typically GW_WeedPlant_*_Stage1.et).", params: "et")]
	protected ResourceName m_rSeedlingPrefab;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Seed item prefab that must be present in the player's inventory. Leave empty to allow planting without a seed requirement.", params: "et")]
	protected ResourceName m_rRequiredSeedItem;

	[Attribute(defvalue: "true", desc: "If true, consume one seed item from the player's inventory on plant complete.")]
	protected bool m_bConsumeSeed;

	override bool CanBePerformedScript(IEntity user)
	{
		if (m_rSeedlingPrefab.IsEmpty())
			return false;

		// Shovel-in-hands gate from parent
		if (!super.CanBePerformedScript(user))
			return false;

		// Seed-in-inventory gate
		if (m_rRequiredSeedItem.IsEmpty())
			return true;
		return HasSeedInInventory(user);
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		if (m_rSeedlingPrefab.IsEmpty() || !pOwnerEntity)
			return;

		if (m_bConsumeSeed && !m_rRequiredSeedItem.IsEmpty())
			ConsumeOneSeed(pUserEntity);

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

	protected bool HasSeedInInventory(IEntity user)
	{
		if (!user)
			return false;

		SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!invMgr)
			return false;

		array<IEntity> items = {};
		invMgr.GetItems(items);

		foreach (IEntity item : items)
		{
			if (IsSeedMatch(item))
				return true;
		}
		return false;
	}

	protected bool IsSeedMatch(IEntity item)
	{
		if (!item)
			return false;
		EntityPrefabData prefabData = item.GetPrefabData();
		if (!prefabData)
			return false;
		return prefabData.GetPrefabName() == m_rRequiredSeedItem;
	}

	protected void ConsumeOneSeed(IEntity user)
	{
		if (!user)
			return;

		SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!invMgr)
			return;

		array<IEntity> items = {};
		invMgr.GetItems(items);

		foreach (IEntity item : items)
		{
			if (IsSeedMatch(item))
			{
				invMgr.TryDeleteItem(item);
				return;
			}
		}
	}
}
