[EntityEditorProps(category: "GameScripted/GrowingWeed", description: "Advances a plant to the next growth stage after a configurable number of in-game hours.")]
class GW_GrowthComponentClass : ScriptComponentClass
{
}

class GW_GrowthComponent : ScriptComponent
{
	[Attribute(defvalue: "6.0", desc: "In-game hours until this stage advances to the next.", category: "Growth")]
	protected float m_fGrowInGameHours;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Prefab spawned when this stage completes. Leave empty for a terminal (harvestable) stage.", params: "et", category: "Growth")]
	protected ResourceName m_rNextStagePrefab;

	[Attribute(defvalue: "false", desc: "If true, print debug logs to Workbench console every tick.", category: "Growth")]
	protected bool m_bDebugLogs;

	protected float m_fElapsedInGameHours;
	protected float m_fLastAbsoluteHour = -1;
	protected bool m_bTicking;

	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		// Delay start slightly so world + time manager are fully up.
		GetGame().GetCallqueue().CallLater(this.StartTicking, 500, false, owner);
	}

	protected void StartTicking(IEntity owner)
	{
		if (m_bTicking || !owner)
			return;

		m_bTicking = true;
		GetGame().GetCallqueue().CallLater(this.Tick, 1000, true, owner);

		if (m_bDebugLogs)
			Print("[GW_GrowthComponent] Started ticking on " + owner);
	}

	void Tick(IEntity owner)
	{
		if (!owner)
			return;

		float absHour = GetAbsoluteWorldHour();
		if (absHour < 0)
		{
			if (m_bDebugLogs)
				Print("[GW_GrowthComponent] Tick: no world time available");
			return;
		}

		if (m_fLastAbsoluteHour < 0)
		{
			m_fLastAbsoluteHour = absHour;
			if (m_bDebugLogs)
				Print(string.Format("[GW_GrowthComponent] Baseline set at %1 hours", absHour));
			return;
		}

		float delta = absHour - m_fLastAbsoluteHour;
		m_fLastAbsoluteHour = absHour;

		if (delta < 0) delta = 0;         // clamp against weird resets
		if (delta > 24) delta = 24;       // clamp huge jumps

		m_fElapsedInGameHours += delta;

		if (m_bDebugLogs)
			Print(string.Format("[GW_GrowthComponent] +%1h -> elapsed %2 / %3", delta, m_fElapsedInGameHours, m_fGrowInGameHours));

		if (m_fElapsedInGameHours >= m_fGrowInGameHours && m_fGrowInGameHours > 0)
			Advance(owner);
	}

	protected float GetAbsoluteWorldHour()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return -1;

		TimeAndWeatherManagerEntity twm = world.GetTimeAndWeatherManager();
		if (!twm)
			return -1;

		int day = twm.GetDay();
		float tod = twm.GetTimeOfTheDay();
		return day * 24.0 + tod;
	}

	protected void Advance(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(this.Tick);
		m_bTicking = false;

		if (m_bDebugLogs)
			Print("[GW_GrowthComponent] Advancing to next stage");

		if (m_rNextStagePrefab.IsEmpty())
			return;

		Resource res = Resource.Load(m_rNextStagePrefab);
		if (!res.IsValid())
		{
			Print("[GW_GrowthComponent] Next stage resource invalid: " + m_rNextStagePrefab, LogLevel.WARNING);
			return;
		}

		vector transform[4];
		owner.GetWorldTransform(transform);

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform = transform;

		IEntity next = GetGame().SpawnEntityPrefab(res, owner.GetWorld(), params);
		if (next)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(owner);
		}
		else
		{
			Print("[GW_GrowthComponent] SpawnEntityPrefab returned null", LogLevel.WARNING);
		}
	}

	float GetProgressPercent()
	{
		if (m_fGrowInGameHours <= 0)
			return 100.0;
		return Math.Clamp((m_fElapsedInGameHours / m_fGrowInGameHours) * 100.0, 0.0, 100.0);
	}

	float GetRemainingInGameHours()
	{
		return Math.Max(0.0, m_fGrowInGameHours - m_fElapsedInGameHours);
	}
}
