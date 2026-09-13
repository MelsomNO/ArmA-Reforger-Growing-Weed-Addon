// GW_ShovelRequiredAction
// Extends GW_ToolAnimAction with a shovel-in-hands gate. Held item's prefab path
// must contain one of the configured substrings; the ETool_ALICE and similar
// entrenching-tool mods all satisfy the default "shovel;etool;entrench;spade".

class GW_ShovelRequiredAction : GW_ToolAnimAction
{
	[Attribute(defvalue: "shovel;etool;entrench;spade", desc: "Semicolon-separated case-insensitive substrings; the held item's prefab path must contain at least one. Leave empty to accept any held item.")]
	protected string m_sShovelNamePatterns;

	//------------------------------------------------------------------------------------------------
	protected string GetHeldItemPrefabPath(IEntity user)
	{
		IEntity item = GetHeldItemEntity(user);
		if (!item)
			return string.Empty;

		EntityPrefabData pd = item.GetPrefabData();
		if (!pd)
			return string.Empty;

		return pd.GetPrefabName();
	}

	//------------------------------------------------------------------------------------------------
	protected bool PathMatchesShovel(string path)
	{
		if (path.IsEmpty())
			return false;

		if (m_sShovelNamePatterns.IsEmpty())
			return true;

		string lowerPath = path;
		lowerPath.ToLower();

		array<string> patterns = {};
		m_sShovelNamePatterns.Split(";", patterns, true);

		foreach (string pat : patterns)
		{
			pat.TrimInPlace();
			if (pat.IsEmpty())
				continue;
			string lowerPat = pat;
			lowerPat.ToLower();
			if (lowerPath.Contains(lowerPat))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		string held = GetHeldItemPrefabPath(user);
		if (PathMatchesShovel(held))
			return true;

		SetCannotPerformReason("Held item is not a shovel");
		return false;
	}
}
