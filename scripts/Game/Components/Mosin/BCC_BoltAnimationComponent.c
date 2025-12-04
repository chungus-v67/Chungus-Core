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

	protected AnimationEventID m_Weapon_TriggerPulled = -1;
	protected AnimationEventID m_BC_BoltActionAdjustAmmoCount = -1;
	protected AnimationEventID m_BC_BoltActionEjectRound = -1;
	protected AnimationEventID m_emptyMagAfterReloadCheck = -1;
	protected AnimationEventID m_BC_TransitionLock	 = -1;
	protected AnimationEventID m_Weapon_Rack_Bolt = -1;
	protected AnimationEventID m_BC_InsertRound0 = -1;
	protected AnimationEventID m_BC_InsertRound1 = -1;
	protected AnimationEventID m_BC_InsertRound2 = -1;
	protected AnimationEventID m_BC_InsertRound3 = -1;
	protected AnimationEventID m_BC_InsertRound4 = -1;
	protected AnimationEventID m_BC_EjectSound = -1;

	protected TAnimGraphVariable m_BC_BoltActionReloadAmmoCount = -1;
	protected TAnimGraphVariable m_BC_IsStripperClipReload = -1;;;
	
	protected TAnimGraphCommand m_CMD_BC_BoltActionReload = -1;
	protected TAnimGraphCommand m_CMD_BC_TransitionUnlock = -1;
	protected TAnimGraphCommand m_CMD_BC_WeaponRackBolt = -1;

	protected bool wasMagEmptyAfterReload = false;
	protected bool m_bIsWorkingAction = false;
	protected bool isTransitionLocked = false;
	protected bool m_BoltActionReloadType = false;
	protected int m_boltActionReloadRoundCount = -1;

	void BCC_BoltAnimationComponent(IEntityComponentSource src, IEntity ent, IEntity parent) {
		m_Owner = ent;

		m_CMD_BC_TransitionUnlock 			= BindCommand("CMD_BC_TransitionUnlock");
		m_CMD_BC_BoltActionReload 			= BindCommand("CMD_BC_BoltActionReload");
		m_CMD_BC_WeaponRackBolt 			= BindCommand("CMD_BC_WeaponRackBolt");

		m_BC_BoltActionReloadAmmoCount 		= BindIntVariable("BC_BoltActionReloadAmmoCount");
		m_BC_IsStripperClipReload 			= BindBoolVariable("BC_IsStripperClipReload");

		m_Weapon_TriggerPulled 				= GameAnimationUtils.RegisterAnimationEvent("Weapon_TriggerPulled");
		m_Weapon_Rack_Bolt 					= GameAnimationUtils.RegisterAnimationEvent("Weapon_Rack_Bolt");
		
		m_BC_BoltActionAdjustAmmoCount 		= GameAnimationUtils.RegisterAnimationEvent("BC_BoltActionAdjustAmmoCount");
		m_BC_BoltActionEjectRound 			= GameAnimationUtils.RegisterAnimationEvent("BC_BoltActionEjectRound");
		m_emptyMagAfterReloadCheck 			= GameAnimationUtils.RegisterAnimationEvent("BoltActionEmptyMagAfterReloadCheck");
		m_BC_TransitionLock					= GameAnimationUtils.RegisterAnimationEvent("BC_TransitionLock");
		
		m_BC_InsertRound0 					= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound0");
		m_BC_InsertRound1 					= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound1");
		m_BC_InsertRound2 					= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound2");
		m_BC_InsertRound3 					= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound3");
		m_BC_InsertRound4 					= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound4");
		m_BC_EjectSound 					= GameAnimationUtils.RegisterAnimationEvent("BC_EjectSound");
		UpdateHud();
	}

	override event void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd) {
		super.OnAnimationEvent(animEventType, animUserString, intParam, timeFromStart, timeToEnd);

		switch (animEventType) {
			case m_Weapon_TriggerPulled:
				UpdateHud(false);
				break;			
			case m_Weapon_Rack_Bolt:
				RackBolt();
				GetGame().GetCallqueue().CallLater(UpdateHud, 100, false,  true, false);
				break;
			case m_BC_BoltActionAdjustAmmoCount:
				FixAmmoCount();
				break;
			case m_BC_BoltActionEjectRound:
				EjectRound();
				break;
			case m_emptyMagAfterReloadCheck:
				// TODO
				break;
			case m_BC_InsertRound0:
				InsertRound(0);
				LockTransition(0);
				UpdateHud(true, true);
				break;
			case m_BC_InsertRound1:
				InsertRound(1);
				LockTransition(1);
				UpdateHud(true, true);
				break;
			case m_BC_InsertRound2:
				InsertRound(2);
				LockTransition(2);
				UpdateHud(true, true);
				break;
			case m_BC_InsertRound3:
				InsertRound(3);
				LockTransition(3);
				UpdateHud(true, true);
				break;
			case m_BC_InsertRound4:
				InsertRound(4);
				LockTransition(4);
				UpdateHud(true, true);
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
	
	void SendBoltActionReloadCommand(int ammoCount, bool reloadType) {
		SetBoltActionAmmoCount(ammoCount);
		SetBoltActionReloadType(reloadType);
		
		if (m_CMD_BC_BoltActionReload != -1) 
			CallCommand(m_CMD_BC_BoltActionReload, ammoCount, 0.0);
	}
	
	void RackBolt() {
		if (m_CMD_BC_WeaponRackBolt != -1) 
			CallCommand(m_CMD_BC_WeaponRackBolt, 1, 0.0);
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
		
			
	    RplComponent rplComponent = RplComponent.Cast(m_Owner.FindComponent(RplComponent));
	    if (IsAuthority() && bulletIdx < m_boltActionReloadRoundCount)
	        magComp.SetAmmoCount(ammoCount + 1);
		
	}
	
	private void FixAmmoCount() {
		MagazineComponent magComp = GetMagComp();
		if (!magComp)
			return;

		if (wasMagEmptyAfterReload) {
			SetAmmoCount(magComp, 0);
			wasMagEmptyAfterReload = false;
			UpdateHud();
		}
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

	private void UpdateHud(bool shouldUpdateAmmoCount = true, bool plusOneRound = false) {
		if (!IsLocalPlayer())
			return;

		SCR_WeaponInfo weaponInfoHud = getWeaponInfoHud();
		if (!weaponInfoHud)
			return;

		int currentAmmoCount = GetCurrentAmmoCount() + plusOneRound;

		if (IsWeaponChambered()) {
			weaponInfoHud.m_Widgets.m_wFiremodeIcon.SetOpacity(1.0);
			weaponInfoHud.m_Widgets.m_wMagazineOutline.SetOpacity(1.0);
		} else {
			weaponInfoHud.m_Widgets.m_wFiremodeIcon.SetOpacity(weaponInfoHud.FADED_OPACITY);
			currentAmmoCount -= 1;
		}

		if (shouldUpdateAmmoCount) {
			if (m_sMaxAmmo == -1)
				InitMaxAmmo();

			weaponInfoHud.m_Widgets.m_wMagazineProgress.SetMaskProgress(currentAmmoCount / m_sMaxAmmo);
		}
	}
	/// ********************************* Getters & Helpers ********************************* ///
	
	bool IsClickToRackEnabled() {
		return m_bClickToRack;
	}
	
	bool DoesWeaponAcceptStripperClips() {
		return m_bAcceptsStripperClips;
	}
	
	protected int GetBoltActionAmmoCount() {
		return m_boltActionReloadRoundCount;
	}
}
		