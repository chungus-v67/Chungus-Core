modded class SCR_CharacterCommandHandlerComponent : CharacterCommandHandlerComponent
{
	protected TAnimGraphTag m_TagWeaponReload_BC = -1;
	protected TAnimGraphTag m_TagIsPullingTrigger_BC = -1;
	protected TAnimGraphTag m_TagFireModeChange = -1;
	protected TAnimGraphTag m_BC_TagIsLoadingAmmo = -1;
	protected TAnimGraphCommand m_CMD_BC_BoltActionReload = -1;
	protected TAnimGraphCommand m_CMD_BC_WeaponRackBolt = -1;
	protected TAnimGraphVariable m_BC_BoltActionReloadAmmoCount = -1;
	protected TAnimGraphVariable m_BC_IsStripperClipReload = -1;
	protected TAnimGraphVariable m_BC_InterruptReload = -1;
	
	protected bool blockRerack;
	
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInit(IEntity owner) {
		super.OnInit(owner);
		
		if (m_CharacterAnimComp) {
			m_TagWeaponReload_BC 				= m_CharacterAnimComp.BindTag("TagWeaponReload");
			m_TagIsPullingTrigger_BC 			= m_CharacterAnimComp.BindTag("TagIsPullingTrigger");
			m_BC_TagIsLoadingAmmo				= m_CharacterAnimComp.BindTag("BC_TagIsLoadingAmmo");
			m_TagFireModeChange					= m_CharacterAnimComp.BindTag("TagFireModeChange");
			
			m_CMD_BC_BoltActionReload 			= m_CharacterAnimComp.BindCommand("CMD_BC_BoltActionReload");
			m_CMD_BC_WeaponRackBolt 			= m_CharacterAnimComp.BindCommand("CMD_BC_WeaponRackBolt");
			
			m_BC_BoltActionReloadAmmoCount 		= m_CharacterAnimComp.BindVariableInt("BC_BoltActionReloadAmmoCount");
			m_BC_IsStripperClipReload 			= m_CharacterAnimComp.BindVariableBool("BC_IsStripperClipReload");
			m_BC_InterruptReload 				= m_CharacterAnimComp.BindVariableBool("BC_InterruptReload");
		}
	}
	

	//------------------------------------------------------------------------------------------------
	// For handling the click "racking" on the bolt guns
	override bool HandleWeaponFire(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID) {
		
		BoltActionRackCommandActivity(pInputCtx, pDt, pCurrentCommandID); // Bolt rack check
		
		if( m_CharacterAnimComp.IsPrimaryTag(m_TagWeaponReload_BC)) // To prevent double firing ?
			return true;
		
		return HandleWeaponFireDefault(pInputCtx, pDt, pCurrentCommandID);
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool HandleWeaponReloading(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID) {
		BoltActionReloadCommandActivity(pInputCtx, pDt, pCurrentCommandID);
		
		return HandleWeaponReloadingDefault(pInputCtx, pDt, pCurrentCommandID);
	}
	
	/// ********************************* Rack Bolt ********************************* ///
	protected void BoltActionRackCommandActivity(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID) {		
		bool isPullingTrigger_BC = pInputCtx.WeaponIsPullingTrigger();
		
		if (!isPullingTrigger_BC) {
			blockRerack = false;
			return;
		}
		if (blockRerack)
			return;
		blockRerack = true;
		
		if (!m_CharacterAnimComp || !m_WeaponManager || m_TagIsPullingTrigger_BC == -1)
			return;
		
		bool isPullingTriggerTagActive = m_CharacterAnimComp.IsPrimaryTag(m_TagIsPullingTrigger_BC) || m_CharacterAnimComp.IsSecondaryTag(m_TagIsPullingTrigger_BC);
		
		//if (isPullingTriggerTagActive)
		//	return;

		BaseWeaponComponent currentWeapon = m_WeaponManager.GetCurrentWeapon();
		if (!currentWeapon) 
			return;
		
		IEntity weaponEntity = currentWeapon.GetOwner();
		if (!weaponEntity)
			return;
			
		BCC_BoltAnimationComponent boltAnimComp = BCC_BoltAnimationComponent.Cast(weaponEntity.FindComponent(BCC_BoltAnimationComponent));
		if (!boltAnimComp)
			return;
		
		SCR_CharacterControllerComponent charController = GetScrCharacterControllerComponent_BCC();
		if (!charController)
			return;
		
		if (!boltAnimComp.IsClickToRackEnabled())
			return;
		if (boltAnimComp.IsReloadInterruptEnabled())
			GetGame().GetInputManager().AddActionListener("CharacterFire", EActionTrigger.PRESSED, BoltActionInterruptActivity);

		BoltActionRackCommandHandler(boltAnimComp, charController, weaponEntity);
	}
	
	protected void BoltActionRackCommandHandler(BCC_BoltAnimationComponent boltAnimComp, SCR_CharacterControllerComponent charController, IEntity weapon) {
		if (!ShouldRackBolt(weapon))
			return;
		SendRackBolt(boltAnimComp, charController);
	}
	
	
	protected void SendRackBolt(BCC_BoltAnimationComponent boltAnimComp, SCR_CharacterControllerComponent charController) {
		if (!boltAnimComp)
			return;
		
		if (m_CMD_BC_WeaponRackBolt == -1)
			return;		
		
		if (!BCC_Utils.IsOwner(GetCharacter()))
			return;
		
		// Rack bolt commands sent here
		charController.ReloadWeapon();
		m_CharacterAnimComp.CallCommand(m_CMD_BC_WeaponRackBolt, 0, 0.0);
		boltAnimComp.CallCommand(boltAnimComp.GetRackBoltCMD(), 1, 0.0);
	}
	
	
	protected bool ShouldRackBolt(IEntity weapon) {
		if (!BCC_Utils.IsOwner(weapon)) // There is an issue where the authority doesn't pick up chamber status
			return false;
		return !BCC_Utils.IsWeaponEntChambered(weapon) && !BCC_Utils.IsWeaponEmpty(weapon);
	}
	
	/// ********************************* Reload ********************************* ///

	protected void BoltActionReloadCommandActivity(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID) {
		bool isStartReloading_BCC = pInputCtx.WeaponIsStartReloading();
		
		if (!isStartReloading_BCC)
			return;
		
		if (!m_CharacterAnimComp || !m_WeaponManager || m_TagWeaponReload_BC == -1 || m_CMD_BC_BoltActionReload == -1 )
			return;

		bool isReloadTagActive = m_CharacterAnimComp.IsPrimaryTag(m_TagWeaponReload_BC) || m_CharacterAnimComp.IsSecondaryTag(m_TagWeaponReload_BC);
		if (isReloadTagActive) 
			return;
		
		IEntity currentWeapon = m_WeaponManager.GetCurrentWeapon().GetOwner();
		if (!currentWeapon) 
			return;
		
		SCR_CharacterControllerComponent charController = GetScrCharacterControllerComponent_BCC();
		if (!charController)
			return;
		
		BCC_BoltAnimationComponent boltAnimComp = BCC_Utils.GetBoltAnimCompFromWeaponEnt(currentWeapon);
		if (!boltAnimComp)
			return;
		
		BoltActionReloadCommandHandler(currentWeapon, boltAnimComp, charController);
	}
	
	protected void BoltActionReloadCommandHandler(IEntity weaponEnt, BCC_BoltAnimationComponent boltAnimComp, SCR_CharacterControllerComponent charController) {
		array<IEntity> magazines = {};
		GetCompatibleMagazines(weaponEnt, magazines);
		
		if (ShouldRackBolt(weaponEnt)) {
			SendRackBolt(boltAnimComp, charController);
			return;
		}
		
		bool reloadType = false;
		bool isWeaponChambered = BCC_Utils.IsWeaponEntChambered(weaponEnt);
		bool shouldResetInternalMag = BCC_Utils.IsWeaponInternalMagBroken(weaponEnt);
		int maxAmmoCount = boltAnimComp.GetMaxAmmo();
			
		int currAmmoCount = BCC_Utils.GetAmmoCountFromWeaponEnt(weaponEnt);
		int ammoCountToLoad = maxAmmoCount - currAmmoCount - isWeaponChambered;
		
		if (ammoCountToLoad < 1 )
			return;
		
		if (ammoCountToLoad > 0 && isWeaponChambered && boltAnimComp.IsEjectLiveRoundEnabled()) // account for chambered round getting cleared
			ammoCountToLoad++;

		IEntity nextMag = GetOptimalMagazine_BCC(magazines, maxAmmoCount, currAmmoCount, true);
		if (!nextMag)
			return;

		MagazineComponent magComp = MagazineComponent.Cast(nextMag.FindComponent(MagazineComponent));
		if (!magComp)
			return;

		ammoCountToLoad = Math.Min(ammoCountToLoad, magComp.GetAmmoCount());
		reloadType = GetBoltActionReloadType(weaponEnt, boltAnimComp, magComp.GetAmmoCount(), ammoCountToLoad);
		
		SendBoltActionReloadCommand_BCC(charController, boltAnimComp, weaponEnt, nextMag, ammoCountToLoad, reloadType, shouldResetInternalMag);
	}
	
	protected void SendBoltActionReloadCommand_BCC(
		SCR_CharacterControllerComponent charController, 
		BCC_BoltAnimationComponent boltAnimComp,
		IEntity weaponEnt, 
		IEntity nextMagEnt, 
		int ammoCountToLoad, 
		bool isStripperClipReload = false, 
		bool shouldResetInternalMag = false
	) {
		charController.SetBoltActionAmmoCount(ammoCountToLoad);
		SendMagazineToLoad_BCC(nextMagEnt, charController, ammoCountToLoad, isStripperClipReload);
		if (!BCC_Utils.IsOwner(charController.GetOwner()))
			return;
		m_CharacterAnimComp.SetVariableInt(m_BC_BoltActionReloadAmmoCount, ammoCountToLoad);
		m_CharacterAnimComp.SetVariableBool(m_BC_IsStripperClipReload, isStripperClipReload);
		m_CharacterAnimComp.SetVariableBool(m_BC_InterruptReload, false);
		
		m_CharacterAnimComp.CallCommand(m_CMD_BC_BoltActionReload, -1 * shouldResetInternalMag, 0.0);
		boltAnimComp.SendBoltActionReloadCommand(ammoCountToLoad, isStripperClipReload, -1 *  shouldResetInternalMag);
	}
	
	protected bool GetBoltActionReloadType(IEntity weaponEnt, BCC_BoltAnimationComponent boltAnimComp, int magAmmoCount, int ammoCountToLoad) {		
		if(ammoCountToLoad < 3)
			return false;
		
		if (!boltAnimComp.DoesWeaponAcceptStripperClips())
			return false;
				
		if (BCC_Utils.IsScopeAttachedToWeaponEnt(weaponEnt))
			return false;
		
		return magAmmoCount == ammoCountToLoad;
	}
	
	/// ********************************* Interrupt ********************************* ///
	protected void BoltActionInterruptActivity() {		
		if (!m_CharacterAnimComp || !m_WeaponManager || m_BC_TagIsLoadingAmmo == -1)
			return;
		
		BaseWeaponComponent currentWeapon = m_WeaponManager.GetCurrentWeapon();
		if (!currentWeapon) 
			return;
		
		IEntity weaponEntity = currentWeapon.GetOwner();
		if (!weaponEntity)
			return;
			
		BCC_BoltAnimationComponent boltAnimComp = BCC_BoltAnimationComponent.Cast(weaponEntity.FindComponent(BCC_BoltAnimationComponent));
		if (!boltAnimComp)
			return;
		
		bool isLoadingAmmoTagActive = boltAnimComp.IsAnimationTag(m_BC_TagIsLoadingAmmo);
		
		if (!isLoadingAmmoTagActive)
			return;

		if (!boltAnimComp.IsReloadInterruptEnabled())
			return;
		
		SCR_CharacterControllerComponent charController = GetScrCharacterControllerComponent_BCC();
		if (!charController)
			return;
		
		BoltActionInterruptHandler(boltAnimComp, charController);
	}
	
	protected void BoltActionInterruptHandler(BCC_BoltAnimationComponent boltAnimComp, SCR_CharacterControllerComponent charController) {	
		SendBoltActionInterrupt(boltAnimComp,  charController);
	}
	
	
	protected void SendBoltActionInterrupt(BCC_BoltAnimationComponent boltAnimComp, SCR_CharacterControllerComponent charController) {
		charController.SetInterruptFlag(true);
		boltAnimComp.SetInterruptFlag(true);
		m_CharacterAnimComp.SetVariableBool(m_BC_InterruptReload, true);
		Print("[BCC] Interrupt sent");
	}
	
	
	/// ********************************* Magazine ********************************* ///
	protected void SendMagazineToLoad_BCC(IEntity magazine, SCR_CharacterControllerComponent charController, int ammoCountToLoad, bool reloadType){
		
		BCC_StripperClipMagazineAnimationComponent magAnimComp = BCC_StripperClipMagazineAnimationComponent.Cast(magazine.FindComponent(BCC_StripperClipMagazineAnimationComponent));
		if (!magAnimComp)
			return;
		
		charController.SetMagazineToLoad_BCC(magazine);
		magAnimComp.SetReloadAmmoCount(ammoCountToLoad);
		magAnimComp.SetReloadType(reloadType);
	}

	protected IEntity GetOptimalMagazine_BCC(array<IEntity> magazines, int maxAmmoCount, int currAmmoCount, bool isStripperClipReloadPossible) {
		IEntity optimalMag = null;
		int maxAmmoSoFar = -1;
		int targetAmmoCount = maxAmmoCount - currAmmoCount;
		int maxMagAmmoCount = -1; // A gun could accept more rounds than the mag has to offer. i,e, Enfield holds 10 rounds, but takes 5rnd clips
		
		if (targetAmmoCount < 1)
			return optimalMag;
		
		foreach(IEntity magazine: magazines) {
			if (maxMagAmmoCount == -1) // init this
				maxMagAmmoCount = BCC_Utils.GetMaxAmmoCountFromMagEnt(magazine);
			
			int magazineAmmoCount = BCC_Utils.GetAmmoCountFromMagEnt(magazine);
			
			if (magazineAmmoCount == targetAmmoCount) 
				return magazine;
			else if (magazineAmmoCount == maxMagAmmoCount && !isStripperClipReloadPossible)
				return magazine;
			
			if (magazineAmmoCount > maxAmmoSoFar) {
				maxAmmoSoFar = magazineAmmoCount;
				optimalMag = magazine;
			}
		}
		return optimalMag;
	}
	
	protected void GetCompatibleMagazines(IEntity currentWeapon, out array<IEntity> magazines)
	{
		magazines.Clear();

		BaseMuzzleComponent muzzle = BaseMuzzleComponent.Cast(currentWeapon.FindComponent(BaseMuzzleComponent));
		if (!muzzle)
			return;
		
		BaseMagazineWell magWell = muzzle.GetMagazineWell();
		if (!magWell)
			return;
		
		CharacterControllerComponent controller = GetControllerComponent();
		if (!controller)
			return;
		
		SCR_InventoryStorageManagerComponent invManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (!invManager)
			return;
		
		// Use existing predicate for compatible magazines
		BCC_StripperClipMagazinePredicate predicate = new BCC_StripperClipMagazinePredicate();
		predicate.magWellType = magWell.Type();
		invManager.FindItems(magazines, predicate);
	}

	/********************************* Generic helpers ************************************/
	
	protected bool IsLocalPlayer() {
	    IEntity charEnt = GetCharacter();
	    if (!charEnt)
	        return false;
	    
	    PlayerController playerController = GetGame().GetPlayerController();
	    if (!playerController)
			return false;
	
	    return charEnt == playerController.GetControlledEntity();
	}
	


	protected SCR_CharacterControllerComponent GetScrCharacterControllerComponent_BCC() {
		IEntity charEnt = GetCharacter();
		if (!charEnt)
			return null;
		return SCR_CharacterControllerComponent.Cast(charEnt.FindComponent(SCR_CharacterControllerComponent));
	}
}
	

class BCC_StripperClipMagazinePredicate : InventorySearchPredicate
{
	typename magWellType;
	
	void BCC_StripperClipMagazinePredicate()
	{
		QueryComponentTypes.Insert(BaseMagazineComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{		
		BaseMagazineComponent iMag = BaseMagazineComponent.Cast(queriedComponents[0]);
		if (!iMag)
			return false;
		
		BaseMagazineWell iMagWell = iMag.GetMagazineWell();
		if (!iMagWell)
			return false;

		BCC_StripperClipMagazineAnimationComponent weaponComp = BCC_StripperClipMagazineAnimationComponent.Cast(item.FindComponent(BCC_StripperClipMagazineAnimationComponent));
		if(!weaponComp)
			return false;
		
		return (iMagWell.IsInherited(magWellType)); // Check if magwells match
	}
}
