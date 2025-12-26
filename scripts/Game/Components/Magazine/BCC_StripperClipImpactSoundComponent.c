class BCC_StripperClipImpactSoundComponentClass : ScriptComponentClass
{
	[Attribute(category: "Sound")]
	ref SCR_AudioSourceConfiguration m_RoundImpactAudioSourceConfiguration;
	
	[Attribute(category: "Sound")]
	ref SCR_AudioSourceConfiguration m_StripperImpactAudioSourceConfiguration;
	
	[Attribute("true", desc: "Set surface signal", category: "Sound")]
	bool m_bSurfaceSignal;
	
	[Attribute("true", desc: "Disable OnContact after the first contact")]
	bool m_bFirstContactOnly;
	
}


class BCC_StripperClipImpactSoundComponent : ScriptComponent
{
	bool isStripperClip = true; 
	int roundsToDrop = 1;
	
	//------------------------------------------------------------------------------------------------
	private void PlayRoundImpactSound(IEntity owner, BCC_StripperClipImpactSoundComponentClass prefabData, Contact contact)
	{
		if (!prefabData.m_RoundImpactAudioSourceConfiguration || !prefabData.m_RoundImpactAudioSourceConfiguration.IsValid())
			return;
		
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(owner.GetWorld());
		if (!soundManager)
			return;
				
		// Create audio source
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(owner, prefabData.m_RoundImpactAudioSourceConfiguration, contact.Position);	
		if (!audioSource)
			return;

		// Set surface signal
		if (prefabData.m_bSurfaceSignal)
		{
			GameMaterial material = contact.Material2;
			if (material)
			{
				audioSource.SetSignalValue(SCR_AudioSource.SURFACE_SIGNAL_NAME, material.GetSoundInfo().GetSignalValue());
			}
		}
					
		// Play a sound for each round with some random jitter
		for (int i = 0; i< roundsToDrop; i++) {
				if (i > 0){
					int jitter = Math.RandomInt(0 , i*10);
					GetGame().GetCallqueue().CallLater(PlayBulletImpactSound, jitter, false, soundManager, audioSource);
					
				} else{
					PlayBulletImpactSound(soundManager, audioSource);
				}
			}	
	}
	
	protected void PlayBulletImpactSound(SCR_SoundManagerModule soundManager, SCR_AudioSource audioSource){
		soundManager.PlayAudioSource(audioSource);		
	}
	
	//------------------------------------------------------------------------------------------------
	private void PlayStripperImpactSound(IEntity owner, BCC_StripperClipImpactSoundComponentClass prefabData, Contact contact)
	{
		if (!prefabData.m_StripperImpactAudioSourceConfiguration || !prefabData.m_StripperImpactAudioSourceConfiguration.IsValid())
			return;
		
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(owner.GetWorld());
		if (!soundManager)
			return;
				
		// Create audio source
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(owner, prefabData.m_StripperImpactAudioSourceConfiguration, contact.Position);	
		if (!audioSource)
			return;

		// Set surface signal
		if (prefabData.m_bSurfaceSignal)
		{
			GameMaterial material = contact.Material2;
			if (material)
			{
				audioSource.SetSignalValue(SCR_AudioSource.SURFACE_SIGNAL_NAME, material.GetSoundInfo().GetSignalValue());
			}
		}
				
		// Play sound	
		soundManager.PlayAudioSource(audioSource);		
	}
	
	//------------------------------------------------------------------------------------------------			
	override void OnPostInit(IEntity owner)
	{
		BCC_StripperClipImpactSoundComponentClass prefabData = BCC_StripperClipImpactSoundComponentClass.Cast(GetComponentData(owner));
		if (!prefabData)
			return;
				
		SetEventMask(owner, EntityEvent.CONTACT);
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_CONTACT_COMPONENT, "", "Show Particle Contacts", "Particles");
#endif
		
	}

	//------------------------------------------------------------------------------------------------
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		// Get prefab data
		BCC_StripperClipImpactSoundComponentClass prefabData = BCC_StripperClipImpactSoundComponentClass.Cast(GetComponentData(owner));
		if (!prefabData)
		{
			ClearEventMask(owner, EntityEvent.CONTACT);
			return;
		}
		
		// Play sound
		if (isStripperClip)
			PlayStripperImpactSound(owner, prefabData, contact);
		else
			PlayRoundImpactSound(owner, prefabData, contact);
				
		// Disable OnContact after the first contact
		if (prefabData.m_bFirstContactOnly)
			ClearEventMask(owner, EntityEvent.CONTACT);
	}
}
