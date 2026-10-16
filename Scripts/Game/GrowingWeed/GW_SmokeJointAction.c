// "Smoke Joint" -- plays an item-use animation on the character while the action is held, then
// consumes the joint.
//
// Inherits the animation plumbing from GW_ToolAnimAction (see the "Digging Animation" write-up in
// the project docs): it binds a command on the character animation graph and calls
// TryUseItemOverrideParams with the held joint as the item entity, so the clip that actually plays
// comes from the animation graph attached to the joint prefab (InventoryItemComponent >
// ItemAnimationAttributes > AnimationAttachments > AnimGraph).
//
// Vanilla has no player smoking animation: the p_smoking_* clips under anims/anm/NPC/male/smoking
// are only bound into the narrative-NPC graph, not into player_main. Until the mod ships its own
// graph, the closest stand-ins are the item-use animations of existing one-handed items -- which
// is why both the command name (m_sAnimCommand) and the graph on the prefab are left tunable.
class GW_SmokeJointAction : GW_ToolAnimAction
{
	[Attribute(defvalue: "true", desc: "If true, the joint is consumed when the action completes.")]
	protected bool m_bConsumeJoint;

	[Attribute(defvalue: "true", desc: "If true, the action is only available while this item is in the player's hands.")]
	protected bool m_bRequireInHands;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Optional smoke particle spawned on the smoker while the action runs. Local to the performing client -- it is not replicated to other players.", params: "ptc")]
	protected ResourceName m_rSmokeParticle;

	[Attribute(defvalue: "0 1.6 0.15", desc: "Offset of the smoke particle from the character's origin, in metres (roughly mouth height).")]
	protected vector m_vSmokeParticleOffset;

	protected ParticleEffectEntity m_SmokeParticle;

	override bool CanBePerformedScript(IEntity user)
	{
		if (!user || !GetOwner())
			return false;

		return !m_bRequireInHands || GW_ItemHandUtils.IsInHands(user, GetOwner());
	}

	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}

	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);
		StartSmokeParticle(pUserEntity);
	}

	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		StopSmokeParticle();
		super.OnActionCanceled(pOwnerEntity, pUserEntity);
	}

	override void OnConfirmed(IEntity pUserEntity)
	{
		StopSmokeParticle();
		super.OnConfirmed(pUserEntity);
	}

	void ~GW_SmokeJointAction()
	{
		StopSmokeParticle();
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		if (m_bConsumeJoint)
			GW_ItemHandUtils.DeleteOwnedItem(pUserEntity, pOwnerEntity);
	}

	protected void StartSmokeParticle(IEntity pUserEntity)
	{
		if (m_rSmokeParticle.IsEmpty() || !pUserEntity || m_SmokeParticle)
			return;

		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		transform[3] = m_vSmokeParticleOffset;

		ParticleEffectEntitySpawnParams params = new ParticleEffectEntitySpawnParams();
		params.TargetWorld = pUserEntity.GetWorld();
		params.Parent = pUserEntity;
		params.AutoTransform = true;
		params.TransformMode = ETransformMode.LOCAL;
		params.Transform = transform;

		m_SmokeParticle = ParticleEffectEntity.SpawnParticleEffect(m_rSmokeParticle, params);
		if (m_SmokeParticle)
			m_SmokeParticle.SetDeleteWhenStopped(true);
	}

	protected void StopSmokeParticle()
	{
		if (!m_SmokeParticle)
			return;

		m_SmokeParticle.StopEmission();
		m_SmokeParticle = null;
	}
}
