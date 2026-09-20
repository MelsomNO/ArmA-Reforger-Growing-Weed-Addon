// Removes the plant with the demolition dig animation. Requires a shovel in hands
// (inherited from GW_ShovelRequiredAction) and drives CMD_Item_Action mode 2
// (inherited from GW_ToolAnimAction) so the character plays the dig-out loop.
class GW_UprootAction : GW_ShovelRequiredAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Prefab spawned into the player's inventory on uproot (typically the empty pot). Leave empty to just remove the plant.", params: "et")]
	protected ResourceName m_rReturnedItemPrefab;

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		if (!m_rReturnedItemPrefab.IsEmpty() && pUserEntity)
		{
			SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
			if (invMgr)
				invMgr.TrySpawnPrefabToStorage(m_rReturnedItemPrefab);
		}

		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}
}
