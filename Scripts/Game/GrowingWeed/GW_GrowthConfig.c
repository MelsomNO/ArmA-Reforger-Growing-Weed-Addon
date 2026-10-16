//! Server-side growth settings, read from $profile:GrowingWeed/GrowthConfig.json.
//! The file is written with defaults on first load if it doesn't exist. Defaults
//! match the hours baked into the plant prefabs, so an untouched file changes nothing.
class GW_GrowthConfig
{
	static const string DIRECTORY = "$profile:GrowingWeed";
	static const string FILE_PATH = "$profile:GrowingWeed/GrowthConfig.json";

	static const float DEFAULT_SPEED_MULTIPLIER = 1.0;
	static const float DEFAULT_STAGE1_HOURS = 6.0;
	static const float DEFAULT_STAGE2_HOURS = 8.0;

	//! Divides every stage's hours. 2.0 = twice as fast, 0.5 = half speed.
	float m_fGrowthSpeedMultiplier = DEFAULT_SPEED_MULTIPLIER;
	//! In-game hours for seedling -> young plant. <= 0 falls back to the prefab value.
	float m_fStage1Hours = DEFAULT_STAGE1_HOURS;
	//! In-game hours for young plant -> mature plant. <= 0 falls back to the prefab value.
	float m_fStage2Hours = DEFAULT_STAGE2_HOURS;

	protected static ref GW_GrowthConfig s_Instance;

	//! Loaded once per session; later calls return the cached config.
	static GW_GrowthConfig Get()
	{
		if (!s_Instance)
		{
			s_Instance = new GW_GrowthConfig();
			s_Instance.Load();
		}

		return s_Instance;
	}

	//! Effective in-game hours for a stage after overrides and the speed multiplier.
	float GetStageHours(int stage, float prefabHours)
	{
		// Terminal stages (0 hours) never advance; keep them that way.
		if (prefabHours <= 0)
			return prefabHours;

		float hours = prefabHours;
		if (stage == 1 && m_fStage1Hours > 0)
			hours = m_fStage1Hours;
		else if (stage == 2 && m_fStage2Hours > 0)
			hours = m_fStage2Hours;

		return hours / m_fGrowthSpeedMultiplier;
	}

	protected void Load()
	{
		if (!FileIO.FileExists(FILE_PATH))
		{
			WriteDefaults();
			return;
		}

		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext();
		if (!ctx.LoadFromFile(FILE_PATH))
		{
			Print("[GW_GrowthConfig] Could not parse " + FILE_PATH + ", using defaults", LogLevel.WARNING);
			return;
		}

		// Missing keys keep their defaults.
		ctx.ReadValue("growthSpeedMultiplier", m_fGrowthSpeedMultiplier);
		ctx.ReadValue("stage1Hours", m_fStage1Hours);
		ctx.ReadValue("stage2Hours", m_fStage2Hours);

		if (m_fGrowthSpeedMultiplier <= 0)
		{
			Print("[GW_GrowthConfig] growthSpeedMultiplier must be > 0, using 1.0", LogLevel.WARNING);
			m_fGrowthSpeedMultiplier = DEFAULT_SPEED_MULTIPLIER;
		}

		Print(string.Format("[GW_GrowthConfig] Loaded: multiplier %1, stage1 %2h, stage2 %3h", m_fGrowthSpeedMultiplier, m_fStage1Hours, m_fStage2Hours));
	}

	//! Written by hand rather than via a save context so the file is readable for server owners.
	protected void WriteDefaults()
	{
		FileIO.MakeDirectory(DIRECTORY);

		FileHandle file = FileIO.OpenFile(FILE_PATH, FileMode.WRITE);
		if (!file)
		{
			Print("[GW_GrowthConfig] Could not create " + FILE_PATH + ", using defaults", LogLevel.WARNING);
			return;
		}

		file.WriteLine("{");
		file.WriteLine(string.Format("\t\"growthSpeedMultiplier\": %1,", DEFAULT_SPEED_MULTIPLIER.ToString(-1, 1)));
		file.WriteLine(string.Format("\t\"stage1Hours\": %1,", DEFAULT_STAGE1_HOURS.ToString(-1, 1)));
		file.WriteLine(string.Format("\t\"stage2Hours\": %1", DEFAULT_STAGE2_HOURS.ToString(-1, 1)));
		file.WriteLine("}");
		file.Close();

		Print("[GW_GrowthConfig] Wrote default config to " + FILE_PATH);
	}
}
