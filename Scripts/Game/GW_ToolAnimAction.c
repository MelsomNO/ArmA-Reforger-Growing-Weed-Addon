// GW_ToolAnimAction
// Base class for actions that drive a vanilla item-use animation on the
// character (e.g., ETool dig loop) while the action's progress runs.
// Mirrors SCR_CampaignBuildingBuildUserAction.OnActionStart's mechanism:
//   BindCommand("CMD_Item_Action") → ItemUseParameters → TryUseItemOverrideParams.
// Subclasses supply PerformAction. If no item is held, animation is skipped
// but the action still runs.

class GW_ToolAnimAction : ScriptedUserAction
{
	[Attribute(defvalue: "CMD_Item_Action", desc: "Animation command to bind on the character.")]
	protected string m_sAnimCommand;

	[Attribute(defvalue: "1", desc: "Command int arg fed to the tool anim graph. Meaning depends on the tool: for ETool 1 = build (dig-down), 2 = demolition (dig-out).")]
	protected int m_iAnimMode;

	protected IEntity m_ActiveUser;

	//------------------------------------------------------------------------------------------------
	protected IEntity GetHeldItemEntity(IEntity user)
	{
		if (!user)
			return null;

		SCR_GadgetManagerComponent gm = SCR_GadgetManagerComponent.GetGadgetManager(user);
		if (gm)
		{
			IEntity gadget = gm.GetHeldGadget();
			if (gadget)
				return gadget;
		}

		BaseWeaponManagerComponent wm = BaseWeaponManagerComponent.Cast(user.FindComponent(BaseWeaponManagerComponent));
		if (wm)
		{
			BaseWeaponComponent cur = wm.GetCurrent();
			if (cur)
				return cur.GetOwner();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);
		m_ActiveUser = pUserEntity;

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController)
			return;

		CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
		if (!pAnimationComponent)
			return;

		IEntity tool = GetHeldItemEntity(pUserEntity);
		if (!tool)
			return;

		int cmdId = pAnimationComponent.BindCommand(m_sAnimCommand);

		ItemUseParameters params = new ItemUseParameters();
		params.SetEntity(tool);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(true);
		params.SetCommandID(cmdId);
		params.SetCommandIntArg(m_iAnimMode);

		charController.TryUseItemOverrideParams(params);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.OnActionCanceled(pOwnerEntity, pUserEntity);
		m_ActiveUser = null;
		FinishToolAnim(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirmed(IEntity pUserEntity)
	{
		super.OnConfirmed(pUserEntity);
		m_ActiveUser = null;
		FinishToolAnim(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void FinishToolAnim(IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController)
			return;

		CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
		if (!pAnimationComponent)
			return;

		CharacterCommandHandlerComponent handler = pAnimationComponent.GetCommandHandler();
		if (handler)
			handler.FinishItemUse(true);
	}

	//------------------------------------------------------------------------------------------------
	void ~GW_ToolAnimAction()
	{
		if (m_ActiveUser)
			FinishToolAnim(m_ActiveUser);
	}
}
