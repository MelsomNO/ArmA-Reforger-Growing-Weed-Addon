// GW_GrowthComponent
// Server-side timer that advances plant growth by polling the in-game clock.
// Progress is measured in accumulated in-game hours (via GetDate + GetTimeOfTheDay),
// so Game Master time-of-day changes, day-duration overrides, and manual clock
// skips all advance growth immediately.

[ComponentEditorProps(category: "GrowingWeed", description: "Advances plant growth using in-game time.")]
class GW_GrowthComponentClass : ScriptComponentClass
{
}

class GW_GrowthComponent : ScriptComponent
{
	[Attribute(defvalue: "6", desc: "In-game hours to wait before advancing to the next stage.")]
	protected float m_fGrowInGameHours;

	[Attribute(defvalue: "", desc: "Prefab to spawn when this stage completes. Empty = terminal stage.", params: "et")]
	protected ResourceName m_sNextStagePrefab;

	[Attribute(defvalue: "5", desc: "Real-time seconds between growth checks.")]
	protected float m_fPollSeconds;

	protected float m_fStartInGameHours = -1;
	protected bool m_bAdvanced;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		if (m_fGrowInGameHours <= 0 || m_sNextStagePrefab.IsEmpty())
			return;

		m_fStartInGameHours = GetCurrentInGameHours();

		int pollMs = m_fPollSeconds * 1000;
		if (pollMs < 250) pollMs = 250;

		GetGame().GetCallqueue().CallLater(PollGrowth, pollMs, true);
	}

	//------------------------------------------------------------------------------------------------
	protected float GetCurrentInGameHours()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return 0;

		ChimeraWorld cw = ChimeraWorld.CastFrom(owner.GetWorld());
		if (!cw)
			return 0;

		TimeAndWeatherManagerEntity twm = cw.GetTimeAndWeatherManager();
		if (!twm)
			return 0;

		int year, month, day;
		twm.GetDate(year, month, day);
		float timeOfDay = twm.GetTimeOfTheDay();

		// Monotonic hour counter. Uses fixed 31d/12m so it stays strictly increasing.
		return (((year * 12) + month) * 31 + day) * 24.0 + timeOfDay;
	}

	//------------------------------------------------------------------------------------------------
	// Returns 0..1 progress toward the next stage.
	float GetProgress()
	{
		if (m_fStartInGameHours < 0 || m_fGrowInGameHours <= 0)
			return 0;

		float elapsed = GetCurrentInGameHours() - m_fStartInGameHours;
		float p = elapsed / m_fGrowInGameHours;
		if (p < 0) p = 0;
		if (p > 1) p = 1;
		return p;
	}

	//------------------------------------------------------------------------------------------------
	float GetRemainingInGameHours()
	{
		float elapsed = GetCurrentInGameHours() - m_fStartInGameHours;
		float remaining = m_fGrowInGameHours - elapsed;
		if (remaining < 0) remaining = 0;
		return remaining;
	}

	//------------------------------------------------------------------------------------------------
	protected void PollGrowth()
	{
		if (m_bAdvanced)
			return;

		if (GetProgress() >= 1.0)
		{
			m_bAdvanced = true;
			GetGame().GetCallqueue().Remove(PollGrowth);
			AdvanceStage();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AdvanceStage()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		Resource res = Resource.Load(m_sNextStagePrefab);
		if (!res || !res.IsValid())
		{
			Print("[GW] AdvanceStage: failed to load next stage prefab " + m_sNextStagePrefab, LogLevel.ERROR);
			return;
		}

		vector mat[4];
		owner.GetWorldTransform(mat);

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform = mat;

		IEntity spawned = GetGame().SpawnEntityPrefab(res, owner.GetWorld(), params);
		if (!spawned)
		{
			Print("[GW] AdvanceStage: SpawnEntityPrefab returned null for " + m_sNextStagePrefab + ". Keeping current stage.", LogLevel.ERROR);
			m_bAdvanced = false;
			return;
		}

		SCR_EntityHelper.DeleteEntityAndChildren(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(PollGrowth);
	}
}
