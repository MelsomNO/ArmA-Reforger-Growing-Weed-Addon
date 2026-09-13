modded class JWK_AddonManagerComponent
{
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!m_Config)
			m_Config = new JWK_AddonsIntegrationsConfig();
		if (!m_Config.m_aAddons)
			m_Config.m_aAddons = {};

		JWK_AddonConfig growingWeed = new JWK_AddonConfig();
		growingWeed.m_sGUID = "6A5AF1436F91E4A1";
		growingWeed.m_sLabel = "FF_GrowingWeed_Integration";
		growingWeed.m_sVersion = "1.0.0";
		growingWeed.m_bEnabled = true;
		growingWeed.m_rConfig = "{6A5AF1436F910000}Configs/Addons/FF_GrowingWeed_Integration.conf";
		m_Config.m_aAddons.Insert(growingWeed);
	}
}
