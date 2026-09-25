// Runs a radial emote command on a character after a delay, from an object that outlives the entity
// whose user action asked for it.
//
// This exists because of a specific ordering trap. GW_SmokeCommandAction sits on the joint item and
// has to do two things: start the Emote_Smoke emote, and let the server consume the joint. Those
// cannot happen in either naive order:
//
//   * Start the emote in OnActionStart and the joint is never consumed. Emote_Smoke is a continuous
//     loiter command that holsters the character's items and takes over player input, which cancels
//     the action while it is still in progress, so it never reaches execution and PerformAction
//     never runs on the server -- confirmed by the absence of any PerformAction log line.
//   * Schedule the emote on the call queue from the action itself and the emote never fires. The
//     action object lives on the joint, so the server consuming the joint destroys the action and
//     the queued call with it ("joint consumed" immediately followed by "scheduled emote canceled").
//
// Holding the timer here, on a static instance that lives for the session, breaks the dependency:
// the action completes, the server consumes the joint, the action object dies, and this object still
// fires the emote afterwards.
//
// Note there is deliberately no cancellation. Cancelling on the action's OnActionCanceled would
// also cancel when the action object is destroyed by the joint being consumed -- the very case this
// class exists to survive -- so a cancelled smoke action still plays the emote a moment later.
class GW_EmoteScheduler
{
	protected static ref GW_EmoteScheduler s_Instance;

	static GW_EmoteScheduler GetInstance()
	{
		if (!s_Instance)
			s_Instance = new GW_EmoteScheduler();

		return s_Instance;
	}

	// Instantiated emote configs, keyed by resource name, so a config is only read once.
	protected ref map<string, ref SCR_BaseRadialCommand> m_mConfigCommands = new map<string, ref SCR_BaseRadialCommand>();

	// delayMs must exceed the requesting action's own duration, or the emote cancels the action
	// before it reaches execution. `config` takes precedence over `commandName`; either may be empty.
	void Schedule(IEntity user, ResourceName config, string commandName, float delayMs, bool debugLog)
	{
		if (!user || (config.IsEmpty() && commandName.IsEmpty()))
			return;

		GetGame().GetCallqueue().CallLater(Run, delayMs, false, user, config, commandName, debugLog);

		if (debugLog)
			Print("[GW_EmoteScheduler] emote scheduled in " + delayMs.ToString() + " ms", LogLevel.NORMAL);
	}

	void Run(IEntity user, ResourceName config, string commandName, bool debugLog)
	{
		if (!user)
			return;

		SCR_BaseRadialCommand command = ResolveCommand(config, commandName, debugLog);
		if (!command)
			return;

		// The smoking emote sets m_bTargetSelf, so the target is the smoker. The position argument
		// only matters for commands that place something in the world.
		bool executed = command.Execute(null, user, user.GetOrigin(), SCR_PlayerController.GetLocalPlayerId(), true);

		if (debugLog)
			Print("[GW_EmoteScheduler] emote executed=" + executed.ToString(), LogLevel.NORMAL);
	}

	// A mod-owned emote config is instantiated straight from its resource, which avoids having to get
	// it registered with the commanding manager -- that registry is built from a vanilla list we
	// would have to override to extend. Falling back to the registry keeps the vanilla emotes usable
	// by name.
	protected SCR_BaseRadialCommand ResolveCommand(ResourceName config, string commandName, bool debugLog)
	{
		if (!config.IsEmpty())
		{
			SCR_BaseRadialCommand cached = m_mConfigCommands.Get(config);
			if (cached)
				return cached;

			SCR_BaseRadialCommand loaded = SCR_BaseRadialCommand.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(config, true));
			if (loaded)
			{
				m_mConfigCommands.Insert(config, loaded);
				return loaded;
			}

			if (debugLog)
				Print("[GW_EmoteScheduler] could not instantiate emote config \"" + config + "\"", LogLevel.NORMAL);
		}

		if (commandName.IsEmpty())
			return null;

		SCR_CommandingManagerComponent commandingMgr = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingMgr)
		{
			if (debugLog)
				Print("[GW_EmoteScheduler] no SCR_CommandingManagerComponent -- this game mode has no commanding manager", LogLevel.NORMAL);

			return null;
		}

		SCR_BaseRadialCommand found = commandingMgr.FindCommand(commandName);
		if (!found && debugLog)
			Print("[GW_EmoteScheduler] command \"" + commandName + "\" not registered with the commanding manager", LogLevel.NORMAL);

		return found;
	}
}
