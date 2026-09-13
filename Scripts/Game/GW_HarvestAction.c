// GW_HarvestAction
// Player-facing user action available on the mature plant.
// Spawns harvest items at the plant location, then respawns a fresh planter
// prefab so the bucket can grow again, then removes the mature plant.

class GW_HarvestAction : ScriptedUserAction
{
	[Attribute(defvalue: "{6A5AF1436F910600}Prefabs/Modded/Items/GW_CannabisTop.et", desc: "Prefab spawned on harvest.", params: "et")]
	protected ResourceName m_sHarvestPrefab;

	[Attribute(defvalue: "1", desc: "Number of items to spawn on harvest.")]
	protected int m_iYield;

	[Attribute(defvalue: "", desc: "Prefab to respawn in place of the harvested plant so growth can restart. Leave empty to just remove the plant.", params: "et")]
	protected ResourceName m_sReplantPrefab;

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity)
			return;

		vector mat[4];
		pOwnerEntity.GetWorldTransform(mat);
		BaseWorld world = pOwnerEntity.GetWorld();

		Resource harvestRes = Resource.Load(m_sHarvestPrefab);
		if (harvestRes && harvestRes.IsValid())
		{
			for (int i = 0; i < m_iYield; i++)
			{
				EntitySpawnParams params = new EntitySpawnParams();
				params.TransformMode = ETransformMode.WORLD;
				vector spawnMat[4];
				spawnMat[0] = mat[0];
				spawnMat[1] = mat[1];
				spawnMat[2] = mat[2];
				spawnMat[3] = mat[3] + Vector(0.15 * i, 0.6, 0);
				params.Transform = spawnMat;

				GetGame().SpawnEntityPrefab(harvestRes, world, params);
			}
		}

		if (!m_sReplantPrefab.IsEmpty())
		{
			Resource replantRes = Resource.Load(m_sReplantPrefab);
			if (replantRes && replantRes.IsValid())
			{
				EntitySpawnParams params = new EntitySpawnParams();
				params.TransformMode = ETransformMode.WORLD;
				params.Transform = mat;
				GetGame().SpawnEntityPrefab(replantRes, world, params);
			}
		}

		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Harvest";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}
}
