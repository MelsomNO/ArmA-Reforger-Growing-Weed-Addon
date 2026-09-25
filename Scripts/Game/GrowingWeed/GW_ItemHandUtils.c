// Shared helpers for actions that live on an inventory item and only make sense while that
// item is the one in the character's hands (the "inspect in hand" action list).
//
// Kept as statics in their own class because the two users -- GW_RollJointAction and
// GW_SmokeJointAction -- sit on different inheritance branches (plain ScriptedUserAction vs.
// GW_ToolAnimAction) and Enforce has no multiple inheritance.
class GW_ItemHandUtils
{
	// Item currently in the character's hands, or null. An item held through
	// SCR_ConsumableItemComponent surfaces via the gadget manager; weapons via the weapon
	// manager; the character controller covers the rest -- try all three.
	static IEntity GetItemInHands(IEntity user)
	{
		if (!user)
			return null;

		SCR_GadgetManagerComponent gadgetMgr = SCR_GadgetManagerComponent.GetGadgetManager(user);
		if (gadgetMgr)
		{
			IEntity gadget = gadgetMgr.GetHeldGadget();
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

		BaseWeaponManagerComponent weaponMgr = BaseWeaponManagerComponent.Cast(user.FindComponent(BaseWeaponManagerComponent));
		if (weaponMgr)
		{
			BaseWeaponComponent weapon = weaponMgr.GetCurrent();
			if (weapon)
				return weapon.GetOwner();
		}

		return null;
	}

	// True when the given item is the one the character currently holds.
	static bool IsInHands(IEntity user, IEntity item)
	{
		return item != null && GetItemInHands(user) == item;
	}

	// Deletes an item the character owns. Prefers the storage manager (so the inventory UI and
	// replication stay in sync) and falls back to a plain entity delete, which is what happens
	// for an item the storage manager does not track -- e.g. one lying in the world. Returns
	// false only when there was no item to delete.
	static bool DeleteOwnedItem(IEntity user, IEntity item)
	{
		if (!item)
			return false;

		if (user)
		{
			SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
			if (invMgr && invMgr.TryDeleteItem(item))
				return true;
		}

		SCR_EntityHelper.DeleteEntityAndChildren(item);
		return true;
	}
}
