class GW_HarvestAction : ScriptedUserAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Prefab spawned into the player's inventory on harvest (e.g. a cannabis-top item). Leave empty to grant no inventory yield.", params: "et")]
	protected ResourceName m_rYieldPrefab;

	[Attribute(defvalue: "1", desc: "How many yield items to grant per harvest.")]
	protected int m_iYieldCount;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Prefab spawned in place of the plant on harvest (typically the filled pot, so the player can replant). Leave empty to just delete the plant.", params: "et")]
	protected ResourceName m_rReplacementPrefab;

	[Attribute(defvalue: "true", desc: "If true, only allow harvest when this stage has finished growing (elapsed >= grow hours). Uncheck to allow harvest at any time.")]
	protected bool m_bRequireFullyGrown;

	override bool CanBePerformedScript(IEntity user)
	{
		if (m_bRequireFullyGrown)
		{
			GW_GrowthComponent gc = GetGrowth(GetOwner());
			if (gc && gc.GetRemainingInGameHours() > 0)
				return false;
		}
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		if (!m_rYieldPrefab.IsEmpty() && pUserEntity)
		{
			SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
			if (invMgr)
			{
				for (int i = 0; i < m_iYieldCount; i++)
					invMgr.TrySpawnPrefabToStorage(m_rYieldPrefab);
			}
		}

		if (!m_rReplacementPrefab.IsEmpty() && pOwnerEntity)
		{
			Resource res = Resource.Load(m_rReplacementPrefab);
			if (res.IsValid())
			{
				vector transform[4];
				pOwnerEntity.GetWorldTransform(transform);

				EntitySpawnParams params = new EntitySpawnParams();
				params.TransformMode = ETransformMode.WORLD;
				params.Transform = transform;

				GetGame().SpawnEntityPrefab(res, pOwnerEntity.GetWorld(), params);
			}
		}

		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}

	protected GW_GrowthComponent GetGrowth(IEntity owner)
	{
		if (!owner) return null;
		return GW_GrowthComponent.Cast(owner.FindComponent(GW_GrowthComponent));
	}
}
