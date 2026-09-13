// GW_PlantAction
// Continuous user action on a dropped pot. Requires a shovel in hands.
// On completion, replaces the pot with a Stage 1 seedling that begins growing.

class GW_PlantAction : GW_ShovelRequiredAction
{
	[Attribute(defvalue: "{8289DBAA61AF2EB9}Prefabs/Props/GW_Stage1_Seedling.et", desc: "Seedling prefab spawned in place of the pot.", params: "et")]
	protected ResourceName m_sSeedlingPrefab;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Plant with Shovel";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Override parent's PerformAction entirely; do not call super (parent would
	// try to build a composition on this entity).
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity)
			return;

		Resource res = Resource.Load(m_sSeedlingPrefab);
		if (res && res.IsValid())
		{
			vector mat[4];
			pOwnerEntity.GetWorldTransform(mat);

			EntitySpawnParams params = new EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform = mat;
			GetGame().SpawnEntityPrefab(res, pOwnerEntity.GetWorld(), params);
		}

		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}
}
