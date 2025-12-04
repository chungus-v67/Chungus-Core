class BCC_StripperClipImpactSoundComponentClass : ScriptComponentClass {}

class BCC_StripperClipImpactSoundComponent : ScriptComponent
{
	bool m_HasPlayed = false;
	
	[Attribute("", desc:"Sound event name to play when the clip hits the ground")]
	protected string m_sSoundEventName;

	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.CONTACT);
	}

	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (m_HasPlayed)
			return;

		if (m_sSoundEventName && m_sSoundEventName != "")
			SCR_SoundManagerModule.CreateAndPlayAudioSource(owner, m_sSoundEventName); 
		m_HasPlayed = true;
	}

	override void EOnActivate(IEntity owner)
	{
		m_HasPlayed = false;
	}
}

