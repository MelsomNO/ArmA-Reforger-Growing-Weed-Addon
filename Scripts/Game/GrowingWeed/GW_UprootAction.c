// Requires a shovel (inherited from GW_ShovelRequiredAction) and drives the dig-out
// loop (inherited from GW_ToolAnimAction). On completion, deletes the owner and
// optionally spawns a replacement prefab at the same transform — used to leave a
// filled pot behind when uprooting a plant, or an empty pot when digging dirt out.
class GW_UprootAction : GW_ShovelRequiredAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Prefab spawned at the owner's transform after uproot (e.g. filled pot from a plant, or empty pot from a filled pot). Leave empty to just delete the owner.", params: "et")]
	protected ResourceName m_rReplacementPrefab;

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

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
}
