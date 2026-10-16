// Turns the item this action sits on into another item -- used for "Roll Joint" on a cannabis
// bud, which consumes the bud and puts a joint in the player's inventory.
//
// Attach to an inventory item that has an ActionsManagerComponent. By default the action is
// only offered while that item is the one in the player's hands, so it shows up in the in-hand
// inspection action list rather than on buds lying in the world.
//
// The result item is spawned before the source is consumed, so a full inventory makes the
// action a no-op instead of destroying the bud. With m_bEquipResult the first result is then put
// straight into the player's hands, which is why the source has to be consumed first.
class GW_RollJointAction : ScriptedUserAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Item prefab created by this action (e.g. GW_Joint_Indica.et). Required -- the action is hidden while this is empty.", params: "et")]
	protected ResourceName m_rResultPrefab;

	[Attribute(defvalue: "1", desc: "How many result items to create per use.")]
	protected int m_iResultCount;

	[Attribute(defvalue: "true", desc: "If true, the source item (the one this action sits on) is consumed on success.")]
	protected bool m_bConsumeSource;

	[Attribute(defvalue: "true", desc: "If true, the action is only available while this item is in the player's hands. Uncheck to also allow it on an item lying in the world.")]
	protected bool m_bRequireInHands;

	[Attribute(defvalue: "true", desc: "Put the first result item straight into the player's hands once the source is consumed.")]
	protected bool m_bEquipResult;

	override bool CanBePerformedScript(IEntity user)
	{
		if (m_rResultPrefab.IsEmpty() || !user || !GetOwner())
			return false;

		if (!SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent)))
			return false;

		return !m_bRequireInHands || GW_ItemHandUtils.IsInHands(user, GetOwner());
	}

	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		if (m_rResultPrefab.IsEmpty() || !pOwnerEntity || !pUserEntity)
			return;

		SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!invMgr)
			return;

		int count = m_iResultCount;
		if (count < 1)
			count = 1;

		IEntity firstResult = null;
		int created = 0;
		for (int i = 0; i < count; i++)
		{
			IEntity item = SpawnIntoInventory(invMgr, pUserEntity);
			if (!item)
				continue;

			created++;
			if (!firstResult)
				firstResult = item;
		}

		// Nothing fit -- leave the source alone so the player keeps the bud.
		if (created == 0)
			return;

		if (m_bConsumeSource)
			GW_ItemHandUtils.DeleteOwnedItem(pUserEntity, pOwnerEntity);

		// Only after the source is gone, so the hands it was occupying are free.
		if (m_bEquipResult && firstResult)
			EquipInHands(pUserEntity, firstResult);
	}

	// Spawns the result and hands it to the inventory. Done in two steps rather than through
	// TrySpawnPrefabToStorage because that returns only a bool, and the caller needs the entity
	// itself to be able to equip it.
	protected IEntity SpawnIntoInventory(SCR_InventoryStorageManagerComponent invMgr, IEntity user)
	{
		Resource res = Resource.Load(m_rResultPrefab);
		if (!res.IsValid())
			return null;

		vector transform[4];
		user.GetWorldTransform(transform);

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform = transform;

		IEntity item = GetGame().SpawnEntityPrefab(res, user.GetWorld(), params);
		if (!item)
			return null;

		if (invMgr.TryInsertItem(item))
			return item;

		// Nothing took it -- do not leave the entity lying at the player's feet.
		SCR_EntityHelper.DeleteEntityAndChildren(item);
		return null;
	}

	// swap = true equips without the draw animation. The hands were just emptied by consuming the
	// source, so there is nothing to animate away from, and skipping it avoids racing the removal.
	protected void EquipInHands(IEntity user, IEntity item)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController)
			return;

		charController.TryEquipRightHandItem(item, EEquipItemType.EEquipTypeUnarmedDeliberate, true);
	}
}
