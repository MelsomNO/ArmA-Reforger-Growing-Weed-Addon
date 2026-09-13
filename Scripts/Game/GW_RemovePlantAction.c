// GW_RemovePlantAction
// Removes a growing/mature plant at any stage. Requires a shovel in hands.
// On completion, deletes the plant and drops an empty pot item in its place.

class GW_RemovePlantAction : GW_ShovelRequiredAction
{
	[Attribute(defvalue: "{981467EA808096D4}Prefabs/Props/GW_PotItem.et", desc: "Pot item prefab dropped in place of the removed plant.", params: "et")]
	protected ResourceName m_sReturnedPotPrefab;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Uproot Plant";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Override parent's PerformAction entirely; do not call super.
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity)
			return;

		vector mat[4];
		pOwnerEntity.GetWorldTransform(mat);
		BaseWorld world = pOwnerEntity.GetWorld();

		if (!m_sReturnedPotPrefab.IsEmpty())
		{
			Resource res = Resource.Load(m_sReturnedPotPrefab);
			if (res && res.IsValid())
			{
				EntitySpawnParams params = new EntitySpawnParams();
				params.TransformMode = ETransformMode.WORLD;
				params.Transform = mat;
				GetGame().SpawnEntityPrefab(res, world, params);
			}
		}

		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}
}
