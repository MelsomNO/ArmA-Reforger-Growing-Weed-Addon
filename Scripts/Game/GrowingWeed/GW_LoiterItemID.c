// Registers a loiter-item preset for the joint prop, so a smoking emote can spawn our own model
// instead of the vanilla cigar. Purely additive -- no vanilla config or prefab is overridden.
//
// How the vanilla system works (from SCR_LoiterItemPresets.c and SCR_CharacterCommandLoiter.c):
//
//   * Configs/Character/LoiterItemPresets.conf ({911269C6C40E808B}) deserialises into
//     SCR_LoiterItemPresets, whose constructor indexes its m_aLoiterItemsData array into a
//     map<SCR_ELoiterItemID, SCR_LoiterItemDataBase> keyed on each entry's GetId().
//   * SCR_LoiterItemPresets.GetData(id) is the only read path into that map.
//   * SCR_CharacterCommandLoiter.OnActivate calls m_customAnimData.SpawnItem(m_pCharacter), and
//     the matching OnDeactivate calls m_customAnimData.DeleteItem(). The preset an emote asks for
//     is SCR_LoiterCustomAnimData.m_eLoiterItemPreset.
//
// So overriding GetData is enough to add an entry without touching the config: vanilla ids still
// resolve through super, and GW_JOINT resolves to the data built here.

// Appends to the enum, leaving vanilla's NONE / CIGAR / CHAIR / TEST_EXAMPLE untouched.
modded enum SCR_ELoiterItemID
{
	GW_JOINT,
}

// Built in script rather than read from a config, which means **every** field has to be set
// explicitly: [Attribute] defvalues are applied by the config deserialiser, not by `new`, so
// anything left alone here stays zero-initialised. Vanilla's CIGAR entry gets away with
//   SCR_LoiterItemData { m_eID CIGAR  m_vPositionOffsetLS 0 0 -0.0099 }
// because the class defaults fill in the prefab and the bone; an empty m_sBoneName leaves the prop
// unparented and it renders at the character's origin, i.e. on the floor between their feet.
//
// Bone and offsets are the tuning knobs.
//
// Use the rig's dedicated *prop* socket, not the hand bone. Attaching to "RightHand" puts the prop
// at that bone's origin, which is the wrist -- the joint then renders inside the sleeve. The rig
// carries prop sockets positioned where a held item should actually sit: the vanilla morphine
// animation workspace attaches its item to "LeftHandProp". That is also why vanilla's CIGAR entry
// needs only a ~1 cm tweak rather than a hand-length offset.
//
// Known-good bone names, if this one needs changing: "RightHand" / "LeftHand" (vanilla's
// TEST_EXAMPLE preset puts a bottle in one and bread in the other) and "LeftHandProp" (morphine).
// A name the rig does not have leaves the prop unparented at the character's origin -- if the joint
// reappears on the floor, the bone name is wrong rather than the offset.
[BaseContainerProps()]
class GW_JointLoiterItemData : SCR_LoiterItemData
{
	static const string BONE_NAME = "RightHandProp";
	static const vector POSITION_OFFSET_LS = Vector(0, 0, -0.0099);
	static const vector ANGLE_OFFSET_LS = vector.Zero;

	void GW_JointLoiterItemData()
	{
		m_eID = SCR_ELoiterItemID.GW_JOINT;
		m_sItemPrefab = "{6A9C40C012000000}Prefabs/Props/GW_AnimationProp_Joint.et";
		m_sBoneName = BONE_NAME;
		m_vPositionOffsetLS = POSITION_OFFSET_LS;
		m_vAngleOffsetLS = ANGLE_OFFSET_LS;
	}
}

// [BaseContainerProps(configRoot: true)] must be repeated here. A modded class does not inherit the
// container attributes of the class it extends, and without it the engine no longer recognises
// SCR_LoiterItemPresets as a config root: loading LoiterItemPresets.conf fails with
// "Unknown class 'SCR_LoiterItemPresets'", the presets object comes back null, and
// SCR_LoiterCustomAnimData.SpawnItem null-pointers.
[BaseContainerProps(configRoot: true)]
modded class SCR_LoiterItemPresets
{
	protected ref GW_JointLoiterItemData m_GwJointData;

	override SCR_LoiterItemDataBase GetData(SCR_ELoiterItemID id)
	{
		if (id != SCR_ELoiterItemID.GW_JOINT)
			return super.GetData(id);

		if (!m_GwJointData)
			m_GwJointData = new GW_JointLoiterItemData();

		return m_GwJointData;
	}
}
