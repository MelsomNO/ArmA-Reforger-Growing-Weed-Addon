// Simple "Pick Up" world-to-inventory action.
// Attach to an item entity with an InventoryItemComponent + ActionsManagerComponent + ActionContext.
// On perform: moves the owner entity into the user's inventory. If the inventory is full or refuses
// the item, the world entity stays where it is (the user gets no feedback beyond the action just
// ending — Reforger's own inventory manager handles the beep/error internally).
class GW_PickUpAction : ScriptedUserAction
{
	override bool CanBePerformedScript(IEntity user)
	{
		if (!user || !GetOwner())
			return false;
		// Only allow if user has an inventory manager
		return SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent)) != null;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		if (!pUserEntity || !pOwnerEntity)
			return;

		SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!invMgr)
			return;

		invMgr.TryInsertItem(pOwnerEntity);
	}
}
