class GW_CheckGrowthAction : ScriptedUserAction
{
	override bool CanBePerformedScript(IEntity user)
	{
		return GetGrowthComponent(GetOwner()) != null;
	}

	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	override bool GetActionNameScript(out string outName)
	{
		GW_GrowthComponent gc = GetGrowthComponent(GetOwner());
		if (!gc)
			return false;

		float remaining = gc.GetRemainingInGameHours();
		if (remaining <= 0)
			outName = "Growth: fully grown";
		else
			outName = string.Format("Growth: %1%%", gc.GetProgressPercent().ToString(-1, 0));

		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
	}

	protected GW_GrowthComponent GetGrowthComponent(IEntity owner)
	{
		if (!owner)
			return null;
		return GW_GrowthComponent.Cast(owner.FindComponent(GW_GrowthComponent));
	}
}
