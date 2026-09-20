class GW_CheckGrowthAction : ScriptedUserAction
{
	override bool CanBePerformedScript(IEntity user)
	{
		return GetGrowthComponent(GetOwner()) != null;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		GW_GrowthComponent gc = GetGrowthComponent(pOwnerEntity);
		if (!gc)
			return;

		float pct = gc.GetProgressPercent();
		float remaining = gc.GetRemainingInGameHours();

		string msg;
		if (remaining <= 0)
			msg = "This plant is fully grown.";
		else
			msg = string.Format("Growth: %1%% — %2 in-game hours remaining.", pct.ToString(-1, 0), remaining.ToString(-1, 1));

		SCR_HintManagerComponent hm = SCR_HintManagerComponent.GetInstance();
		if (hm)
			hm.ShowCustomHint(msg, "Growing Weed", 4);
	}

	protected GW_GrowthComponent GetGrowthComponent(IEntity owner)
	{
		if (!owner)
			return null;
		return GW_GrowthComponent.Cast(owner.FindComponent(GW_GrowthComponent));
	}
}
