// Base class that drives a tool animation on the character while the action is held.
// Mechanism (see the "Digging Animation" write-up in the project docs): binds CMD_Item_Action on
// the character animation graph, then calls TryUseItemOverrideParams with the held gadget as the
// tool entity.
class GW_ToolAnimAction : ScriptedUserAction
{
	[Attribute(defvalue: "CMD_Item_Action", desc: "Command name bound on the character animation graph.")]
	protected string m_sAnimCommand;

	[Attribute(defvalue: "1", desc: "Command int arg: 1 = build/dig-down loop, 2 = demolition/dig-out loop.")]
	protected int m_iAnimMode;

	[Attribute(defvalue: "false", desc: "Print what the animation call saw -- held item, command name, bound command id, and whether the engine accepted the item use. Turn on to diagnose an action that runs but plays no animation.")]
	protected bool m_bDebugAnimLog;

	protected CharacterAnimationComponent m_AnimComponentInUse;

	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);
		StartToolAnim(pUserEntity);
	}

	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		StopToolAnim();
		super.OnActionCanceled(pOwnerEntity, pUserEntity);
	}

	override void OnConfirmed(IEntity pUserEntity)
	{
		StopToolAnim();
		super.OnConfirmed(pUserEntity);
	}

	void ~GW_ToolAnimAction()
	{
		StopToolAnim();
	}

	protected void StartToolAnim(IEntity pUserEntity)
	{
		if (!pUserEntity)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController)
			return;

		CharacterAnimationComponent animComponent = charController.GetAnimationComponent();
		if (!animComponent)
			return;

		IEntity tool = GetHeldTool(pUserEntity);
		if (!tool)
		{
			DebugLog("no item in hands -- nothing to animate");
			return;
		}

		// BindCommand only resolves commands the held item's animation graph actually declares.
		// A graph that does not declare m_sAnimCommand returns < 0 and the character stays idle:
		// e.g. the canteen graph declares only CMD_Item_Action, the morphine graph only
		// CMD_HealSelf / CMD_HealOther / CMD_Revive.
		int commandId = animComponent.BindCommand(m_sAnimCommand);
		if (commandId < 0)
		{
			DebugLog("BindCommand(\"" + m_sAnimCommand + "\") failed -- the held item's animation graph does not declare that command");
			return;
		}

		ItemUseParameters params = new ItemUseParameters();
		params.SetEntity(tool);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(true);
		params.SetCommandID(commandId);
		params.SetCommandIntArg(m_iAnimMode);

		bool accepted = charController.TryUseItemOverrideParams(params);
		if (m_bDebugAnimLog)
		{
			string toolName = "<no prefab data>";
			EntityPrefabData prefabData = tool.GetPrefabData();
			if (prefabData)
				toolName = prefabData.GetPrefabName();

			DebugLog("item=" + toolName + " command=" + m_sAnimCommand + " id=" + commandId.ToString() + " intArg=" + m_iAnimMode.ToString() + " accepted=" + accepted.ToString());
		}

		m_AnimComponentInUse = animComponent;
	}

	protected void StopToolAnim()
	{
		if (!m_AnimComponentInUse)
			return;

		CharacterCommandHandlerComponent handler = m_AnimComponentInUse.GetCommandHandler();
		if (handler)
			handler.FinishItemUse(true);
		m_AnimComponentInUse = null;
	}

	protected void DebugLog(string message)
	{
		if (m_bDebugAnimLog)
			Print("[GW_ToolAnimAction] " + message, LogLevel.NORMAL);
	}

	protected IEntity GetHeldTool(IEntity user)
	{
		if (!user)
			return null;

		SCR_GadgetManagerComponent gm = SCR_GadgetManagerComponent.GetGadgetManager(user);
		if (gm)
		{
			IEntity held = gm.GetHeldGadget();
			if (held)
				return held;
		}

		BaseWeaponManagerComponent wm = BaseWeaponManagerComponent.Cast(user.FindComponent(BaseWeaponManagerComponent));
		if (wm)
		{
			BaseWeaponComponent weapon = wm.GetCurrent();
			if (weapon)
				return weapon.GetOwner();
		}

		return null;
	}
}
