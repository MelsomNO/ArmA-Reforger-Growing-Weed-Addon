// "Smoke Joint" -- runs the vanilla smoking emote on the player, then consumes the joint.
//
// Arma Reforger's smoking animation lives behind the radial emote command defined in
// Configs/Commanding/Emotes/Emote_Smoke.conf:
//
//   SCR_ContinuousLoiterCommand { m_sCommandName "Emote_Smoke"  m_eLoiterType SMOKING
//                                m_aAllowedStances { STAND }
//                                m_CustomAnimationData { m_eLoiterItemPreset CIGAR } }
//
// That command is the only thing in the game that plays ELoiteringType.SMOKING, and the only thing
// that carries the item preset which spawns the prop (the CIGAR entry of
// Configs/Character/LoiterItemPresets.conf -- what AnimationProp_Cigar.et is for). Commands are
// registered by name with SCR_CommandingManagerComponent, so it can be looked up and executed.
//
// Split of work: the emote is queued client-side from OnActionStart on the performing machine and
// run by GW_EmoteScheduler once this action has finished; deleting the joint is authoritative and
// stays in PerformAction behind a Replication.IsServer() guard.
//
// Two routes were tried and abandoned before this one:
//   * GW_SmokeJointAction (kept as a fallback) drives an item-use command against the held item's
//     own animation graph. It can only borrow another item's motion -- no animation graph in the
//     game contains the smoking clips.
//   * Subclassing SCR_LoiterUserAction fails on an inventory item: it resolves its loiter data
//     through a sibling AI smart-action component and warns "Invalid SmartAction id at
//     SCR_LoiterUserAction! Components=0, id=0" when there is none. It also has no field for an
//     item preset, so it could not spawn the prop.
class GW_SmokeCommandAction : ScriptedUserAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Emote config to run. Instantiated straight from the resource, so a mod-owned emote needs no entry in the commanding manager's registry. Takes precedence over m_sEmoteCommandName.", params: "conf")]
	protected ResourceName m_rEmoteConfig;

	[Attribute(defvalue: "Emote_Smoke", desc: "Fallback: m_sCommandName of an already-registered radial emote, as written in its .conf. Used only when m_rEmoteConfig is empty. 'Emote_Smoke' is the vanilla smoking emote, which spawns the vanilla cigar prop.")]
	protected string m_sEmoteCommandName;

	[Attribute(defvalue: "true", desc: "If true, the joint is consumed when the action completes.")]
	protected bool m_bConsumeJoint;

	[Attribute(defvalue: "true", desc: "If true, the action is only offered while this item is the one in the player's hands.")]
	protected bool m_bRequireInHands;

	[Attribute(defvalue: "250", desc: "Milliseconds after this action's own duration before the emote fires. Must stay above zero: the emote cancels the action if it starts while the action is still in progress, and the joint is then never consumed.")]
	protected float m_fEmoteDelayMs;

	[Attribute(defvalue: "false", desc: "Print whether the emote command was found, whether it accepted execution, and whether the joint was consumed. Turn on while tuning.")]
	protected bool m_bDebugLog;

	// The in-hands requirement gates *visibility* only, never CanBePerformedScript. The server
	// re-checks CanBePerformed before running PerformAction, and its view of what is in the
	// character's hands does not necessarily agree with the client's -- gating both is what made
	// the joint survive being smoked.
	override bool CanBePerformedScript(IEntity user)
	{
		return user != null && GetOwner() != null;
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (!user || !GetOwner())
			return false;

		return !m_bRequireInHands || GW_ItemHandUtils.IsInHands(user, GetOwner());
	}

	// The emote is handed to GW_EmoteScheduler rather than started here. Starting it directly makes
	// the animation play but the joint survive: Emote_Smoke cancels this action while it is still in
	// progress, so PerformAction never runs on the server. Scheduling it from the action object does
	// not work either, because consuming the joint destroys that object and the queued call with it.
	// See GW_EmoteScheduler for the full ordering trap.
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);

		// OnActionStart runs on every machine observing the action, so bail out early on the ones
		// that are not smoking. GW_EmoteScheduler repeats this check when the timer fires; this one
		// just avoids queueing a timer that would do nothing.
		if (SCR_PlayerController.GetLocalControlledEntity() != pUserEntity)
			return;

		float delay = GetActionDuration() * 1000 + m_fEmoteDelayMs;
		if (delay < m_fEmoteDelayMs)
			delay = m_fEmoteDelayMs;

		GW_EmoteScheduler.GetInstance().Schedule(pUserEntity, m_rEmoteConfig, m_sEmoteCommandName, delay, m_bDebugLog);
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
		{
			DebugLog("PerformAction reached the client -- the consume runs on the server");
			return;
		}

		if (!m_bConsumeJoint)
			return;

		if (GW_ItemHandUtils.DeleteOwnedItem(pUserEntity, pOwnerEntity))
			DebugLog("joint consumed");
		else
			DebugLog("nothing to consume -- the joint entity was already gone");
	}

	protected void DebugLog(string message)
	{
		if (m_bDebugLog)
			Print("[GW_SmokeCommandAction] " + message, LogLevel.NORMAL);
	}
}
