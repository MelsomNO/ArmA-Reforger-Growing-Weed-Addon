// Extension of GW_ToolAnimAction that only allows the action when the character is
// holding a recognised shovel/entrenching tool (pattern-matched on prefab name).
class GW_ShovelRequiredAction : GW_ToolAnimAction
{
	[Attribute(defvalue: "shovel;etool;entrench;spade", desc: "Semicolon-separated substrings; the held item's prefab name must contain one to match.")]
	protected string m_sShovelNamePatterns;

	override bool CanBePerformedScript(IEntity user)
	{
		if (m_sShovelNamePatterns.IsEmpty())
			return true;
		return IsHoldingShovel(user);
	}

	protected bool IsHoldingShovel(IEntity user)
	{
		IEntity held = GetHeldTool(user);
		if (!held)
			return false;

		EntityPrefabData prefabData = held.GetPrefabData();
		if (!prefabData)
			return false;

		string prefabName = prefabData.GetPrefabName();
		prefabName.ToLower();

		array<string> patterns = {};
		m_sShovelNamePatterns.Split(";", patterns, false);
		foreach (string pat : patterns)
		{
			pat.Trim();
			if (pat.IsEmpty())
				continue;
			string patLower = pat;
			patLower.ToLower();
			if (prefabName.Contains(patLower))
				return true;
		}
		return false;
	}
}
