modded class SCR_CharacterControllerComponent {
	protected AnimationEventID m_Weapon_Rack_Bolt = -1;
	protected AnimationEventID m_emptyMagAfterReloadCheck = -1;
	protected AnimationEventID m_TransitionLock = -1;
	protected AnimationEventID m_BC_SpawnStripperMagazine = -1;
	protected AnimationEventID m_BC_InsertRound0 = -1;
	protected AnimationEventID m_BC_InsertRound1 = -1;
	protected AnimationEventID m_BC_InsertRound2 = -1;
	protected AnimationEventID m_BC_InsertRound3 = -1;
	protected AnimationEventID m_BC_InsertRound4 = -1;
	protected AnimationEventID m_BC_EjectStripperClip = -1;
	protected AnimationEventID m_BC_DropRounds = -1;
	
	protected TAnimGraphCommand m_CMD_BC_TransitionUnlock = -1;
		
	protected bool isWeaponStillReloading = false; // Used for emergency exit
	protected IEntity m_magazineToLoad = null;
	protected bool isTransitionLocked = false;
	protected bool m_BoltActionReloadType = false;
	protected int m_boltActionReloadRoundCount = -1;
	protected int m_boltActionAmmoCountLoadedSoFar = -1;
	protected bool wasReloadInterrupted = false;
	
    override protected void OnInit(IEntity owner) {
		super.OnInit(owner);
		m_Weapon_Rack_Bolt 				= GameAnimationUtils.RegisterAnimationEvent("Weapon_Rack_Bolt");
		m_emptyMagAfterReloadCheck 		= GameAnimationUtils.RegisterAnimationEvent("BoltActionEmptyMagAfterReloadCheck");
		m_TransitionLock 				= GameAnimationUtils.RegisterAnimationEvent("BC_TransitionLock");
		m_BC_SpawnStripperMagazine 		= GameAnimationUtils.RegisterAnimationEvent("BC_SpawnStripperMagazine");
		m_BC_InsertRound0 				= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound0");
		m_BC_InsertRound1 				= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound1");
		m_BC_InsertRound2 				= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound2");
		m_BC_InsertRound3 				= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound3");
		m_BC_InsertRound4 				= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound4");
		m_BC_EjectStripperClip 			= GameAnimationUtils.RegisterAnimationEvent("BC_EjectStripperClip");
		m_BC_DropRounds 				= GameAnimationUtils.RegisterAnimationEvent("BC_DropRounds");
		
		CharacterAnimationComponent charAnimComp = GetCharAnimComp_BCC();
		if (charAnimComp) {
			m_CMD_BC_TransitionUnlock = charAnimComp.BindCommand("CMD_BC_TransitionUnlock");
		}
    }

    override protected void OnAnimationEvent(AnimationEventID animEventType,AnimationEventID animUserString,int intParam,float timeFromStart,float timeToEnd) {
        super.OnAnimationEvent(animEventType,animUserString,intParam,timeFromStart,timeToEnd);
		
		switch (animEventType) {
			case m_Weapon_Rack_Bolt:
				//PerformCleanup_BCC();
				break;
			case m_BC_SpawnStripperMagazine:
				AttachMagToHand_BCC();
				break;	
			case m_BC_InsertRound0:
				InsertRound(0);
				LockTransition(0);
				break;
			case m_BC_InsertRound1:
				InsertRound(1);
				LockTransition(1);
				break;
			case m_BC_InsertRound2:
				InsertRound(2);
				LockTransition(2);
				break;
			case m_BC_InsertRound3:
				InsertRound(3);
				LockTransition(3);
				break;
			case m_BC_InsertRound4:
				InsertRound(4);
				LockTransition(4);
				GetGame().GetCallqueue().CallLater(ReleaseLock, 1000, false);
				break;
			case m_BC_EjectStripperClip:
				EjectStripperClip();
				break;
			case m_BC_DropRounds:
				//DropRounds();
				break;
		}					
    }
	
	void SetBoltActionAmmoCount(int ammoCount) {
		m_boltActionReloadRoundCount = ammoCount;
	}
	
	void SetInterruptFlag(bool value) {
		wasReloadInterrupted = true;
	}
	
	protected void InsertRound(int bulletIdx) {
		m_boltActionAmmoCountLoadedSoFar = bulletIdx + 1; // Tracking for interrupts
		if (wasReloadInterrupted && m_boltActionReloadRoundCount - m_boltActionAmmoCountLoadedSoFar > 0) {
			DropRounds(m_boltActionReloadRoundCount - m_boltActionAmmoCountLoadedSoFar);
		}
			
	}
	
	protected void DropRounds(int roundsToDrop) {
		if (!m_magazineToLoad)
			return;
		
		BCC_StripperClipMagazineAnimationComponent magAnimComp = BCC_Utils.GetStripperClipAnimCompFromMagEnt(m_magazineToLoad);
		if (!magAnimComp)
			return;
		
		magAnimComp.DropRounds(roundsToDrop);
		GetGame().GetCallqueue().CallLater(ReturnMagToInv_BCC,2000,false);
	}

	
	protected void EjectStripperClip() {
		if (!m_magazineToLoad || !m_BoltActionReloadType)
			return;
		BCC_StripperClipMagazineAnimationComponent magAnimComp = BCC_Utils.GetStripperClipAnimCompFromMagEnt(m_magazineToLoad);
		if (!magAnimComp)
			return;
		magAnimComp.EjectStripperClip();
		
		GetGame().GetCallqueue().CallLater(TryDeleteMagToLoad, 4000);
	}

	// Just a helper to call commands from here
	protected void CallCommand(TAnimGraphCommand pCmdIndex, int intParam, float floatParam) {
		CharacterAnimationComponent charAnimComp = GetCharAnimComp_BCC();
		if (!charAnimComp)
			return;
		charAnimComp.CallCommand(pCmdIndex, intParam, floatParam);
	}
	
	void SetMagazineToLoad_BCC(IEntity magazine) {
		wasReloadInterrupted = false;
		if (magazine)
			m_magazineToLoad = magazine;
	}

	/********************************* Mag to hand ************************************/
	
	private void AttachMagToHand_BCC() {
		if (!m_magazineToLoad) 
			return;

		RplComponent magRplComp = RplComponent.Cast(m_magazineToLoad.FindComponent(RplComponent));
		if (!magRplComp)
			return;
		
		EntitySlotInfo leftHandSlot = GetLeftHandPointInfo();
		if (!leftHandSlot)
			return;
		
		BCC_StripperClipMagazineAnimationComponent magAnimComp = BCC_Utils.GetStripperClipAnimCompFromMagEnt(m_magazineToLoad);
		int ammoCount = 0;
		int reloadType = false;
		if (magAnimComp) {
			ammoCount = magAnimComp.GetReloadAmmoCount();
			reloadType = magAnimComp.GetReloadType();
		}
	
		// Hide inventory visibility while reloading
		BCC_StripperClipInventoryMagazineComponent stripperMagInvComp = BCC_Utils.GetMagStripperClipInvCompFromMagEnt(m_magazineToLoad);
		if (stripperMagInvComp)
			stripperMagInvComp.SetHideStripperClipInVicinity(true);
		
		m_BoltActionReloadType = reloadType;
		HideAllStripperClipMesh_BCC(m_magazineToLoad);
		Rpc(RPC_AttachMagToHand, magRplComp.Id(), ammoCount, reloadType);
		
		SetStripperClipVisibility_BCC(m_magazineToLoad, reloadType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server-side RPC to handle magazine attachment to hand
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_AttachMagToHand(RplId magazineRplId, int ammoCount, bool reloadType)
	{
		RplComponent magRplComp = RplComponent.Cast(Replication.FindItem(magazineRplId));
		if (!magRplComp)
			return;
		
		IEntity magazineEntity = magRplComp.GetEntity();
		if (!magazineEntity)
			return;
		
		InventoryItemComponent magInventory = InventoryItemComponent.Cast(magazineEntity.FindComponent(InventoryItemComponent));
		if (!magInventory)
			return;
		
		InventoryStorageSlot parentSlot = magInventory.GetParentSlot();
		if(!parentSlot)
			return;
		
		BaseInventoryStorageComponent magStorage = parentSlot.GetStorage();
		if (!magStorage)
			return;
		
		SCR_InventoryStorageManagerComponent invManager = GetInvMan_BCC();
		if (!invManager)
			return;

		invManager.TryRemoveItemFromStorage(magazineEntity, magStorage);
		EntitySlotInfo leftHandSlot = GetLeftHandPointInfo();
		if (!leftHandSlot)
			return;
		
		leftHandSlot.AttachEntity(m_magazineToLoad);
		Rpc(RPC_DoAttachMagToHand, magazineRplId, ammoCount, reloadType);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoAttachMagToHand(RplId magazineRplId, int ammoCount, bool reloadType)
	{
		RplComponent magRplComp = RplComponent.Cast(Replication.FindItem(magazineRplId));
		if (!magRplComp)
			return;
		
		IEntity magazineEntity = magRplComp.GetEntity();
		if (!magazineEntity)
			return;
		
		EntitySlotInfo leftHandSlot = GetLeftHandPointInfo();
		if (!leftHandSlot)
			return;
		
		BCC_StripperClipMagazineAnimationComponent magAnimComp = BCC_StripperClipMagazineAnimationComponent.Cast(magazineEntity.FindComponent(BCC_StripperClipMagazineAnimationComponent));
		if (magAnimComp) {
			magAnimComp.SetReloadAmmoCount(ammoCount);
		}
		m_BoltActionReloadType = reloadType;
		HideAllStripperClipMesh_BCC(magazineEntity);
		leftHandSlot.AttachEntity(magazineEntity);
		SetStripperClipVisibility_BCC(magazineEntity, reloadType);
	}

	/********************************* Mag to inv ************************************/
	
	private void ReturnMagToInv_BCC() {
		// Allow stripper local vis
		if(m_magazineToLoad) {
			BCC_StripperClipInventoryMagazineComponent stripperMagInvComp = BCC_Utils.GetMagStripperClipInvCompFromMagEnt(m_magazineToLoad);
			if (stripperMagInvComp)
				stripperMagInvComp.SetHideStripperClipInVicinity(false);
		}
		Rpc(RPC_ReturnMagToInv);
	}

	//------------------------------------------------------------------------------------------------
	//! Server-side RPC to return entity from left hand to inventory
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_ReturnMagToInv()
	{
		EntitySlotInfo leftHandSlot = GetLeftHandPointInfo();
		if (!leftHandSlot)
			return;

		IEntity attachedEntity = leftHandSlot.GetAttachedEntity();
		if (!attachedEntity)
			return;

		SCR_InventoryStorageManagerComponent invManager = GetInvMan_BCC();
		if (!invManager)
			return;

		leftHandSlot.DetachEntity();

		invManager.TryInsertItem(attachedEntity);
		
		Rpc(RPC_DoReturnMagToInv);
	}

	//------------------------------------------------------------------------------------------------
	//! Client-side RPC to synchronize visual detachment
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoReturnMagToInv()
	{
		if (Replication.IsServer())
			return;

		EntitySlotInfo leftHandSlot = GetLeftHandPointInfo();
		if (!leftHandSlot)
			return;

		leftHandSlot.DetachEntity();
	}
	
	/// ********************************* Locking ********************************* ///
	
	protected void LockTransition(int bulletIdx) {
		isTransitionLocked = true;
		bulletIdx++;
		BCC_BoltAnimationComponent boltAnimComp = GetBoltAnimComp_BCC();
		if (!boltAnimComp)  {
			SendTransitionUnlock(bulletIdx);
			return;
		}
		if (boltAnimComp.GetTransitionLock()){
			SendTransitionUnlock(bulletIdx);
		}
	}

	protected void SendTransitionUnlock(int ammoCount) {
		BCC_BoltAnimationComponent boltAnimComp = GetBoltAnimComp_BCC();
		if (boltAnimComp) 
			boltAnimComp.UnlockTransition(ammoCount);
		UnlockTransition(ammoCount);
	}
	
	void UnlockTransition(int ammoCount) {
		CharacterAnimationComponent charAnimComp = GetCharAnimComp_BCC();
		if (charAnimComp)
			charAnimComp.CallCommand(m_CMD_BC_TransitionUnlock, ammoCount, 0.0);
		isTransitionLocked = false;
	}
	
	protected void ReleaseLock() {
		SendTransitionUnlock(-1);
	}
	
	bool GetTransitionLock() {
		return isTransitionLocked;
	}
	
	/// ********************************* Mesh ********************************* ///
	
	protected void HideAllStripperClipMesh_BCC(IEntity magazine) {
		if (!magazine)
			return;
		
		BCC_StripperClipMagazineAnimationComponent magAnimComp = BCC_StripperClipMagazineAnimationComponent.Cast(magazine.FindComponent(BCC_StripperClipMagazineAnimationComponent));
		
		if (!magAnimComp)
			return;
		magAnimComp.HideAllBulletMesh();
	}
	

	protected void SetStripperClipVisibility_BCC(IEntity magazine, bool reloadType) {
		if (!magazine)
			return;
		
		BCC_StripperClipMagazineAnimationComponent magAnimComp = BCC_StripperClipMagazineAnimationComponent.Cast(magazine.FindComponent(BCC_StripperClipMagazineAnimationComponent));
		
		if (!magAnimComp)
			return;
		magAnimComp.SetReloadType(reloadType); // Doing this for Rpc
		magAnimComp.SetBulletVisibility();
		magAnimComp.StartReload();
	
	}
	
	/********************************* Cleanup ************************************/
	
	protected void TryDeleteMagToLoad() {
		MagazineComponent magComp = BCC_Utils.GetMagCompFromMagEnt(m_magazineToLoad);
		if (!magComp)
			return;
			
		if (magComp.GetAmmoCount() == 0) {
			SCR_InventoryStorageManagerComponent invMan = GetInvMan_BCC();
			invMan.AskServerToDeleteEntity(m_magazineToLoad);
			m_magazineToLoad = null;
		}
	}
	
	private void PerformCleanup_BCC(){
		TryDeleteMagToLoad();
		
		if (m_magazineToLoad)
			ReturnMagToInv_BCC();
	}

	
	/// ********************************* Generics ********************************* ///
	
	private CharacterAnimationComponent GetCharAnimComp_BCC() {
        IEntity playerEnt = GetOwner();
        if(!playerEnt)
            return null;
        return CharacterAnimationComponent.Cast(playerEnt.FindComponent(CharacterAnimationComponent));
    }
	
	private void CompleteReload_BCC() {
		isWeaponStillReloading = false;
    }
	
	protected BCC_BoltAnimationComponent GetBoltAnimComp_BCC(){
		IEntity weaponEnt = GetWeaponEnt_BCC();
		if (!weaponEnt)
			return null;
		return BCC_BoltAnimationComponent.Cast(weaponEnt.FindComponent(BCC_BoltAnimationComponent));
	}


    private WeaponComponent GetWeaponComp_BCC() {
        BaseWeaponManagerComponent weaponManager = GetWeaponManagerComponent();
        if(!weaponManager)
            return null;
        return WeaponComponent.Cast(weaponManager.GetCurrentWeapon());
    }
	
	private IEntity GetWeaponEnt_BCC() {
        WeaponComponent weaponComp = GetWeaponComp_BCC();
        if(!weaponComp)
            return null;
        return weaponComp.GetOwner();
    }
	
    private MagazineComponent GetMagComp_BCC() {
        WeaponComponent weapon = GetWeaponComp_BCC();
        if(!weapon)
            return null;
        return MagazineComponent.Cast(weapon.GetCurrentMagazine());
    }

    private typename GetMagWellType_BCC() {
		IEntity weaponEnt = GetWeaponEnt_BCC();
		if (!weaponEnt)
			return typename.Empty;
		MuzzleComponent muzzleComp = MuzzleComponent.Cast(weaponEnt.FindComponent(MuzzleComponent));
		if (!muzzleComp)
			return typename.Empty;
		
		BaseMagazineWell weaponMagWell = muzzleComp.GetMagazineWell();
		if (!weaponMagWell)
			return typename.Empty;
		
		return weaponMagWell.Type();
    }

    private SCR_InventoryStorageManagerComponent GetInvMan_BCC() {
        return SCR_InventoryStorageManagerComponent.Cast(GetInventoryStorageManager());
    }

    private SCR_MagazinePredicate MakeMagPredicate_BCC(typename magWellType) {
        SCR_MagazinePredicate predicate = new SCR_MagazinePredicate();
        predicate.magWellType = magWellType;
        return predicate;
    }

    private IEntity FindMatchingMagInInventory_BCC() {
        SCR_InventoryStorageManagerComponent invMan = GetInvMan_BCC();
        if(!invMan)
            return null;

        typename magWellType = GetMagWellType_BCC();
        SCR_MagazinePredicate predicate = MakeMagPredicate_BCC(magWellType);

        return invMan.FindItem(predicate);
    }

    private EntitySlotInfo GetLeftHandSlot_BCC() {
        return GetLeftHandPointInfo();
    }
	
	private bool IsLocalPlayer_BCC(){
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return false;
		IEntity playerControllerEnt = playerController.GetControlledEntity();
		if (!playerControllerEnt)
			return false;	
		IEntity weapon = GetWeaponEnt_BCC();
		if (!weapon)
			return false;
		IEntity characterEnt = GetOwner();
		if (!characterEnt)
			return false;

		return playerControllerEnt == characterEnt;
	}
};
