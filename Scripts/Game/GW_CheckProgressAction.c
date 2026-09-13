// GW_CheckProgressAction
// Player-facing user action available on growing plants. The action label itself
// reflects live growth percentage; performing it also flashes a hint with details.

class GW_CheckProgressAction : ScriptedUserAction
{
	protected IEntity m_OwnerEntity;
	protected GW_GrowthComponent m_Growth;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_OwnerEntity = pOwnerEntity;
		if (pOwnerEntity)
			m_Growth = GW_GrowthComponent.Cast(pOwnerEntity.FindComponent(GW_GrowthComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_Growth)
		{
			outName = "Check Growth";
			return true;
		}

		int percent = Math.Round(m_Growth.GetProgress() * 100);
		outName = string.Format("Check Growth (%1%%)", percent);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		GW_GrowthComponent growth = m_Growth;
		if (!growth && pOwnerEntity)
			growth = GW_GrowthComponent.Cast(pOwnerEntity.FindComponent(GW_GrowthComponent));
		if (!growth)
			return;

		int percent = Math.Round(growth.GetProgress() * 100);
		int hoursLeft = Math.Round(growth.GetRemainingInGameHours());

		string msg = string.Format("Growth: %1%%\n~%2 in-game hours remaining", percent, hoursLeft);
		SCR_HintManagerComponent.ShowCustomHint(msg, "Plant", 4);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}
}
