class BCC_BoltAnimationComponentClass : BCC_WeaponAnimationComponentClass {}

class BCC_BoltAnimationComponent : BCC_WeaponAnimationComponent {
	[Attribute("", UIWidgets.ResourceNamePicker, desc:"The empty casing particle effect to play when the bolt is cycled", params:"ptc")]
	protected ResourceName m_sCasingEjectionAnimation;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc:"The live round particle effect that is played when the bolt is opened", params:"ptc")]
	protected ResourceName m_sBulletEjectionAnimation;
	
	[Attribute("false", UIWidgets.CheckBox, "Does weapon accept stripper clips?")]
	protected bool m_bAcceptsStripperClips;

	[Attribute(desc:"Effect position")]
	protected ref PointInfo m_effectPosition;
	
	[Attribute("true", UIWidgets.CheckBox, "Should clicking on an empty chamber trigger the rack bolt animation?")]
	protected bool m_bClickToRack;
	
	[Attribute("true", UIWidgets.CheckBox, "Should weapon reload be interruptable by firing or jumping?")]
	protected bool m_bInterruptReload;
	
	[Attribute("true", UIWidgets.CheckBox, "Should weapon use this custom HUD or default? Default will be missing features.")]
	protected bool m_bUseCustomHUD;
	
	[Attribute("true", UIWidgets.CheckBox, "Eject live round when bolt is opened on a loaded chamber")]
	protected bool m_bEjectLiveRounds;
	
	static const ref Color COLOR_WHITE = Color.FromSRGBA(230, 230, 230, 255);
	static const ref Color COLOR_DARK = Color.FromSRGBA(0, 0, 0, 70);
	protected ref BCC_AmmoMaskMapping m_AmmoMaskMapping = new BCC_AmmoMaskMapping();

	protected AnimationEventID m_Weapon_TriggerPulled = -1;
	protected AnimationEventID m_BC_BoltActionAdjustAmmoCount = -1;
	protected AnimationEventID m_BC_BoltActionEjectRound = -1;
	protected AnimationEventID m_BC_TransitionLock	 = -1;
	protected AnimationEventID m_Weapon_Rack_Bolt = -1;
	protected AnimationEventID m_BC_InsertRound0 = -1;
	protected AnimationEventID m_BC_InsertRound1 = -1;
	protected AnimationEventID m_BC_InsertRound2 = -1;
	protected AnimationEventID m_BC_InsertRound3 = -1;
	protected AnimationEventID m_BC_InsertRound4 = -1;
	protected AnimationEventID m_BC_EjectSound = -1;
	protected AnimationEventID m_BC_ResetMagazine = -1;
	

	protected TAnimGraphVariable m_BC_BoltActionReloadAmmoCount = -1;
	protected TAnimGraphVariable m_BC_IsStripperClipReload = -1;
	protected TAnimGraphVariable m_BC_InterruptReload = -1;
	protected TAnimGraphVariable m_BC_IsCocked = -1;
	
	protected TAnimGraphCommand m_CMD_BC_BoltActionReload = -1;
	protected TAnimGraphCommand m_CMD_BC_TransitionUnlock = -1;
	protected TAnimGraphCommand m_CMD_BC_WeaponRackBolt = -1;

	protected bool wasMagEmptyAfterReload = false;
	protected bool m_bIsWorkingAction = false;
	protected bool isTransitionLocked = false;
	protected bool m_BoltActionReloadType = false;
	protected int m_boltActionReloadRoundCount = -1;
	protected bool shouldResetMagazine = false;
	protected bool plusOneRound = false;
	protected bool wasMagEmpty = false;
	protected bool isCocked = true;
	

	void BCC_BoltAnimationComponent(IEntityComponentSource src, IEntity ent, IEntity parent) {
		m_Owner = ent;

		m_CMD_BC_TransitionUnlock 			= BindCommand("CMD_BC_TransitionUnlock");
		m_CMD_BC_BoltActionReload 			= BindCommand("CMD_BC_BoltActionReload");
		m_CMD_BC_WeaponRackBolt 			= BindCommand("CMD_BC_WeaponRackBolt");

		m_BC_IsCocked 						= BindBoolVariable("BC_IsCocked");
		m_BC_BoltActionReloadAmmoCount 		= BindIntVariable("BC_BoltActionReloadAmmoCount");
		m_BC_IsStripperClipReload 			= BindBoolVariable("BC_IsStripperClipReload");
		m_BC_InterruptReload 				= BindBoolVariable("BC_InterruptReload");

		m_Weapon_TriggerPulled 				= GameAnimationUtils.RegisterAnimationEvent("Weapon_TriggerPulled");
		m_Weapon_Rack_Bolt 					= GameAnimationUtils.RegisterAnimationEvent("Weapon_Rack_Bolt");
		
		m_BC_BoltActionAdjustAmmoCount 		= GameAnimationUtils.RegisterAnimationEvent("BC_BoltActionAdjustAmmoCount");
		m_BC_BoltActionEjectRound 			= GameAnimationUtils.RegisterAnimationEvent("BC_BoltActionEjectRound");
		m_BC_TransitionLock					= GameAnimationUtils.RegisterAnimationEvent("BC_TransitionLock");
		
		m_BC_InsertRound0 					= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound0");
		m_BC_InsertRound1 					= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound1");
		m_BC_InsertRound2 					= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound2");
		m_BC_InsertRound3 					= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound3");
		m_BC_InsertRound4 					= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound4");
		m_BC_EjectSound 					= GameAnimationUtils.RegisterAnimationEvent("BC_EjectSound");
		m_BC_ResetMagazine					= GameAnimationUtils.RegisterAnimationEvent("BC_ResetMagazine");

		UpdateHud();
	}
/*	
	override event  bool OnPrepareAnimInput(IEntity owner, float ts) { 
		MagazineComponent magComp = GetMagComp();
		if (magComp) {
			Print(magComp.GetAmmoCount());
			SetBoltActionAmmoCount(magComp.GetAmmoCount());
		}
		return super.OnPrepareAnimInput(owner, ts);
		
		 }
*/
	override event void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd) {
		super.OnAnimationEvent(animEventType, animUserString, intParam, timeFromStart, timeToEnd);

		switch (animEventType) {
			case m_Weapon_TriggerPulled:
				plusOneRound = true;
				FixEmptyMag();
				UpdateHud();
				SetCockedState(false);
				break;			
			case m_Weapon_Rack_Bolt:
				plusOneRound = false;
				CheckForEmptyMag();
				SetCockedState(true);
				break;
			case m_BC_BoltActionEjectRound:
				ResetMagazine(0); // MOVE THIS LATER
				EjectRound();
				SetCockedState(false);
				break;
			case m_BC_InsertRound0:
				InsertRound(0);
				UpdateHud();
				LockTransition(0);
				break;
			case m_BC_InsertRound1:
				InsertRound(1);
				UpdateHud();
				LockTransition(1);
				break;
			case m_BC_InsertRound2:
				InsertRound(2);
				UpdateHud();
				LockTransition(2);
				break;
			case m_BC_InsertRound3:
				InsertRound(3);
				UpdateHud();
				LockTransition(3);
				break;
			case m_BC_InsertRound4:
				InsertRound(4);
				UpdateHud();
				LockTransition(4);
				GetGame().GetCallqueue().CallLater(ReleaseLock, 1000, false); // Emergency exit
				break;
			case m_BC_TransitionLock:
				LockTransition(0);
				break;
			case m_BC_EjectSound:
				PlayEjectSound(animUserString);
				break;
		}
	}
	
	/// ********************************* Commands & Variables ********************************* ///
	
	void SendBoltActionReloadCommand(int ammoCount, bool reloadType, int resetInternalMag) {
		if (resetInternalMag == -1)
			shouldResetMagazine = true;
		SetBoltActionAmmoCount(ammoCount);
		SetBoltActionReloadType(reloadType);
		SetInterruptFlag(false);
		if (m_CMD_BC_BoltActionReload != -1) {
			CallCommand(m_CMD_BC_BoltActionReload, resetInternalMag, 0.0);
		}
	}
	
	int GetRackBoltCMD(){
		return m_CMD_BC_WeaponRackBolt;
	}
	
	void RackBolt() {
		if (m_CMD_BC_WeaponRackBolt != -1)  {
			CallCommand(m_CMD_BC_WeaponRackBolt, 1, 0.0);
		}
	}
	
	void SetBoltActionReloadType(bool isStripperClipReload) {
		m_BoltActionReloadType = isStripperClipReload;
		
		if (m_BC_IsStripperClipReload != -1)	
			SetBoolVariable(m_BC_IsStripperClipReload, isStripperClipReload)
	}
	
	void SetBoltActionAmmoCount(int ammoCount) {
		m_boltActionReloadRoundCount = ammoCount;
		
		if (m_BC_BoltActionReloadAmmoCount)
			SetIntVariable(m_BC_BoltActionReloadAmmoCount, ammoCount);
	}
	
	void SetCockedState(bool state) {
		isCocked = state;
		
		if (m_BC_IsCocked != -1)	
			SetBoolVariable(m_BC_IsCocked, state)
	}
	
	
	/// ********************************* Fix Mag ******************************** ///
	protected void CheckForEmptyMag() {
		MagazineComponent magComp = GetMagComp();
		if (!magComp)
			return;
		if (!IsAuthority())
	        return;
		if (magComp.GetAmmoCount() == 0) {
			magComp.SetAmmoCount(1);
			wasMagEmpty = true;
		}
	}
	
	protected void FixEmptyMag() {
		MagazineComponent magComp = GetMagComp();
		if (!magComp)
			return;
		if (!IsAuthority())
	        return;
		if (wasMagEmpty){
			magComp.SetAmmoCount(0);
			wasMagEmpty = false;
		} 
	}
	
	/// ********************************* Interrupt ******************************** ///
	void SetInterruptFlag(bool value) {
		if (m_BC_InterruptReload != -1)
			SetBoolVariable(m_BC_InterruptReload, value);
	}
	/// ********************************* Reset Mag ******************************** ///
	protected void ResetMagazine(int retryCount) {
		if (!shouldResetMagazine)
			return;
		MagazineComponent magComp = GetMagComp();
		if (!magComp) {
			if (retryCount < 3) {
				Print("Retrying", retryCount);
				GetGame().GetCallqueue().CallLater(ResetMagazine, retryCount * 100 + 100, false, retryCount + 1);
			}
			return;
		}
		
	    if (IsAuthority())
	        magComp.SetAmmoCount(0);
		Print("Reset Magazine");
		Print(GetMagComp());
		shouldResetMagazine = false;
		
	}
	

	/// ********************************* Locking ********************************* ///
	
	protected void LockTransition(int bulletIdx) {
		isTransitionLocked = true;
		bulletIdx++;
		SCR_CharacterControllerComponent charController = GetCharController();
		if (!charController){
			SendTransitionUnlock(bulletIdx);
			return;	
		}
		if (charController.GetTransitionLock()){
			SendTransitionUnlock(bulletIdx);
		}
	}
	
	void SendTransitionUnlock(int ammoCount) {
		SCR_CharacterControllerComponent charController = GetCharController();
		if (charController)
			charController.UnlockTransition(ammoCount);
		UnlockTransition(ammoCount);
	}
	
	void UnlockTransition(int ammoCount) {
		CallCommand(m_CMD_BC_TransitionUnlock, ammoCount, 0.0);
		isTransitionLocked = false;
	}
	
	protected void ReleaseLock() {
		if(isTransitionLocked)
			SendTransitionUnlock(-1);
	}
	
	bool GetTransitionLock() {
		return isTransitionLocked;
	}
	
	
	/// ********************************* Ammo ********************************* ///
	void InsertRound(int bulletIdx) {
		MagazineComponent magComp = GetMagComp();
		if (!magComp)
			return;
		int ammoCount = magComp.GetAmmoCount();
		Print(ammoCount);
			Print(GetMagComp());
		
	    if (IsAuthority() && bulletIdx < m_boltActionReloadRoundCount)
	        magComp.SetAmmoCount(ammoCount + 1);
		
	}
	
	private void EjectRound() {
		IEntity magEnt = GetMagEnt();
		if (!magEnt)
			return;
		if (!m_sCasingEjectionAnimation.Length())
			return;

		ParticleEffectEntitySpawnParams spawnParams();
		spawnParams.Parent = magEnt;
		vector mat[4];
		spawnParams.PivotID = m_effectPosition.GetNodeId();
		m_effectPosition.GetTransform(mat);
		spawnParams.Transform[0] = mat[0];
		spawnParams.Transform[1] = mat[1];
		spawnParams.Transform[2] = mat[2];
		spawnParams.Transform[3] = mat[3];
		
		if (IsWeaponChambered()) {
			if (!m_bEjectLiveRounds)
				return;
			ClearCurrentChamber();
			ParticleEffectEntity.SpawnParticleEffect(m_sBulletEjectionAnimation, spawnParams);
		} else{
			ParticleEffectEntity.SpawnParticleEffect(m_sCasingEjectionAnimation, spawnParams);
		}
	}
	
	/// ********************************* Sounds ********************************* ///
	
	private void PlayEjectSound(AnimationEventID animUserString) {
		if (!m_Owner)
			return;
		if (!m_BoltActionReloadType)
			return;

		string soundEventName = GameAnimationUtils.GetEventString(animUserString);
		if (soundEventName.IsEmpty())
			return;

		SoundComponent soundComponent = SoundComponent.Cast(m_Owner.FindComponent(SoundComponent));
		if (soundComponent)
			soundComponent.SoundEvent(soundEventName);
	}

	/// ********************************* HUD ********************************* ///
	void UpdateHud() {
		InitMaxAmmo(); // putting this here to init
		
		if (!IsLocalPlayer())
			return;
		
		if (!IsCustomHUDEnabled())
			return;
	
		SCR_WeaponInfo weaponInfoHud = getWeaponInfoHud();
		if (!weaponInfoHud)
			return;

		weaponInfoHud.m_Widgets.m_wMagazineGlow.SetOpacity(0.0); 
		weaponInfoHud.m_Widgets.m_wMagazineBackground.SetOpacity(1.0); 
		weaponInfoHud.m_Widgets.m_wMagazineOutline.SetOpacity(0.0); 
		weaponInfoHud.m_Widgets.m_wMagazineProgress.SetOpacity(1.0); 	
		weaponInfoHud.m_Widgets.m_wFiremodeGlow.SetOpacity(0.0); 
		
		
		float width, height;
		weaponInfoHud.m_Widgets.m_wMagazineBackground.GetScreenSize(width, height);
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		float widthUnscaled = workspace.DPIUnscale(width);
		float heightUnscaled = workspace.DPIUnscale(height);
		weaponInfoHud.m_Widgets.m_wFiremodeIcon.SetSize(widthUnscaled * 0.9, heightUnscaled * 0.9);
		
		int currentAmmoCount = GetCurrentAmmoCount() - wasMagEmpty;
		Print(GetCurrentAmmoCount());

		if (IsWeaponChambered()) {
			weaponInfoHud.m_Widgets.m_wFiremodeIcon.SetColor(COLOR_WHITE);
		} else {
			weaponInfoHud.m_Widgets.m_wFiremodeIcon.SetColor(COLOR_DARK);
		}
		float maskProgress = m_AmmoMaskMapping.GetMaskProgress(currentAmmoCount);
		weaponInfoHud.m_Widgets.m_wMagazineProgress.SetMaskProgress(maskProgress); 
		
		
		
	}
	/// ********************************* Getters & Helpers ********************************* ///
	
	bool IsClickToRackEnabled() {
		return m_bClickToRack;
	}
	
	bool IsReloadInterruptEnabled() {
		return m_bInterruptReload;
	}
	
	bool IsEjectLiveRoundEnabled() {
		return m_bEjectLiveRounds;
	}
		
	bool IsCustomHUDEnabled() {
		return m_bUseCustomHUD;
	}
	
	bool DoesWeaponAcceptStripperClips() {
		return m_bAcceptsStripperClips;
	}
	
	protected int GetBoltActionAmmoCount() {
		return m_boltActionReloadRoundCount;
	}
}

class BCC_AmmoMaskMapping
{
    protected ref array<float> m_MaskValues;
    
    void BCC_AmmoMaskMapping()
    {
        m_MaskValues = {0.0, 0.6, 0.7, 0.8, 0.9, 1.0};
    }
    
    float GetMaskProgress(int ammoCount)
    {
        if (ammoCount >= 0 && ammoCount < m_MaskValues.Count())
            return m_MaskValues[ammoCount];
        return 0.0; 
    }
}
		