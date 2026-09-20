// Fills an empty pot with dirt. Requires a shovel in hands (inherited from
// GW_ShovelRequiredAction) and plays the build/dig-down animation (default
// m_iAnimMode 1 from GW_ToolAnimAction). Replaces the pot entity with the
// configured filled-pot prefab at the same world transform.
class GW_FillPotAction : GW_ShovelRequiredAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Filled-pot prefab spawned in place of the empty pot when the fill action completes.", params: "et")]
	protected ResourceName m_rFilledPotPrefab;

	override bool CanBePerformedScript(IEntity user)
	{
		if (m_rFilledPotPrefab.IsEmpty())
			return false;
		return super.CanBePerformedScript(user);
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		if (m_rFilledPotPrefab.IsEmpty() || !pOwnerEntity)
			return;

		Resource res = Resource.Load(m_rFilledPotPrefab);
		if (!res.IsValid())
		{
			Print("[GW_FillPotAction] Filled-pot prefab invalid: " + m_rFilledPotPrefab, LogLevel.WARNING);
			return;
		}

		vector transform[4];
		pOwnerEntity.GetWorldTransform(transform);

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform = transform;

		IEntity filled = GetGame().SpawnEntityPrefab(res, pOwnerEntity.GetWorld(), params);
		if (!filled)
		{
			Print("[GW_FillPotAction] SpawnEntityPrefab returned null", LogLevel.WARNING);
			return;
		}

		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}
}
