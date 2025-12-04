modded class SCR_CharacterCommandHandlerComponent : CharacterCommandHandlerComponent
{
	protected TAnimGraphTag m_TagWeaponReload_BC = -1;
	protected TAnimGraphTag m_TagIsPullingTrigger_BC = -1;
	protected TAnimGraphCommand m_CMD_BC_BoltActionReload = -1;
	protected TAnimGraphCommand m_CMD_BC_WeaponRackBolt = -1;
	protected TAnimGraphVariable m_BC_BoltActionReloadAmmoCount = -1;
	protected TAnimGraphVariable m_BC_IsStripperClipReload = -1;
	
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInit(IEntity owner) {
		super.OnInit(owner);
		
		if (m_CharacterAnimComp) {
			m_TagWeaponReload_BC 				= m_CharacterAnimComp.BindTag("TagWeaponReload");
			m_TagIsPullingTrigger_BC 			= m_CharacterAnimComp.BindTag("TagIsPullingTrigger");
			
			m_CMD_BC_BoltActionReload 			= m_CharacterAnimComp.BindCommand("CMD_BC_BoltActionReload");
			m_CMD_BC_WeaponRackBolt 			= m_CharacterAnimComp.BindCommand("CMD_BC_WeaponRackBolt");
			
			m_BC_BoltActionReloadAmmoCount 		= m_CharacterAnimComp.BindVariableInt("BC_BoltActionReloadAmmoCount");
			m_BC_IsStripperClipReload 			= m_CharacterAnimComp.BindVariableBool("BC_IsStripperClipReload");
		}
	}
	

	//------------------------------------------------------------------------------------------------
	// For handling the click "racking" on the bolt guns
	override bool HandleWeaponFire(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID) {
		
		BoltActionRackCommandActivity(pInputCtx, pDt, pCurrentCommandID);
		
		return HandleWeaponFireDefault(pInputCtx, pDt, pCurrentCommandID);
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool HandleWeaponReloading(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID)
	{
		BoltActionReloadCommandActivity(pInputCtx, pDt, pCurrentCommandID);
	
		return HandleWeaponReloadingDefault(pInputCtx, pDt, pCurrentCommandID);
	}
	
	/// ********************************* Rack Bolt ********************************* ///
	protected void BoltActionRackCommandActivity(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID) {		
		bool isPullingTrigger_BC = pInputCtx.WeaponIsPullingTrigger();
		
		if (!isPullingTrigger_BC)
			return;
		
		if (!m_CharacterAnimComp || !m_WeaponManager || m_TagIsPullingTrigger_BC == -1)
			return;

		
		bool isPullingTriggerTagActive = m_CharacterAnimComp.IsPrimaryTag(m_TagIsPullingTrigger_BC) || m_CharacterAnimComp.IsSecondaryTag(m_TagIsPullingTrigger_BC);
		
		if (isPullingTriggerTagActive)
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
		
		if (!boltAnimComp.IsClickToRackEnabled())
			return;
		
		BoltActionRackCommandHandler(boltAnimComp, weaponEntity);
	}
	
	protected void BoltActionRackCommandHandler(BCC_BoltAnimationComponent boltAnimComp, IEntity weapon) {
		
		if (!ShouldRackBolt(weapon))
			return;
		
		SendRackBolt(boltAnimComp);
	}
	
	protected void SendRackBolt(BCC_BoltAnimationComponent boltAnimComp) {
		if (!boltAnimComp)
			return;
		
		if (m_CMD_BC_WeaponRackBolt == -1)
			return;

		m_CharacterAnimComp.CallCommand(m_CMD_BC_WeaponRackBolt, 0, 0.0);
		boltAnimComp.RackBolt();
	}
	
	protected bool ShouldRackBolt(IEntity weapon) {
		return !BCC_Utils.IsWeaponEntChambered(weapon) && !BCC_Utils.IsWeaponEmpty(weapon);
	}
	
	/// ********************************* Reload********************************* ///

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
		
		BCC_BoltAnimationComponent boltAnimComp = BCC_BoltAnimationComponent.Cast(currentWeapon.FindComponent(BCC_BoltAnimationComponent));
		if (!boltAnimComp)
			return;
		
		BoltActionReloadCommandHandler(currentWeapon);
	}
	
	protected void BoltActionReloadCommandHandler(IEntity currentWeapon) {
		array<IEntity> magazines = {};
		GetCompatibleMagazines(currentWeapon, magazines);
				
		bool reloadType = false;
		bool isWeaponChambered = BCC_Utils.IsWeaponEntChambered(currentWeapon);
		int maxAmmoCount = BCC_Utils.GetMaxAmmoCountFromWeaponEnt(currentWeapon);
		int currAmmoCount = BCC_Utils.GetAmmoCountFromWeaponEnt(currentWeapon);
		int ammoCountToLoad = maxAmmoCount - currAmmoCount - isWeaponChambered;

		if (ammoCountToLoad > 0 && isWeaponChambered)
			ammoCountToLoad++;

		IEntity nextMag = GetOptimalMagazine_BCC(magazines, maxAmmoCount, currAmmoCount, true);
		if (nextMag) {
			MagazineComponent magComp = MagazineComponent.Cast(nextMag.FindComponent(MagazineComponent));
			if (magComp)
				ammoCountToLoad = Math.Min(ammoCountToLoad, magComp.GetAmmoCount());
				reloadType = GetBoltActionReloadType(currentWeapon, magComp.GetAmmoCount(), ammoCountToLoad);
		}		
		
		SendBoltActionReloadCommand_BCC(ammoCountToLoad, reloadType);
		SendMagazineToLoad_BCC(nextMag, ammoCountToLoad, reloadType);
	}
	
	protected void SendBoltActionReloadCommand_BCC(int ammoCountToLoad, bool isStripperClipReload = false) {
		
		IEntity weaponEnt = m_WeaponManager.GetCurrentWeapon().GetOwner();
		if (!weaponEnt)
			return;
		
		BCC_BoltAnimationComponent boltAnimComp = BCC_Utils.GetBoltAnimCompFromWeaponEnt(weaponEnt);
		if (!boltAnimComp)
			return;		
		
		if (ShouldRackBolt(weaponEnt)) {
			SendRackBolt(boltAnimComp);
			return;
		}
		
		if(BCC_Utils.IsWeaponInternalMagBroken(weaponEnt))
			ammoCountToLoad = -1;
		
		if (ammoCountToLoad < 1 && ammoCountToLoad != -1) // -1 explicitly means current mag is null
			return;
		
		m_CharacterAnimComp.SetVariableInt(m_BC_BoltActionReloadAmmoCount, ammoCountToLoad);
		m_CharacterAnimComp.SetVariableBool(m_BC_IsStripperClipReload, isStripperClipReload);
		m_CharacterAnimComp.CallCommand(m_CMD_BC_BoltActionReload, ammoCountToLoad, 0.0);
		
		boltAnimComp.SendBoltActionReloadCommand(ammoCountToLoad, isStripperClipReload);
	}
	
	protected bool GetBoltActionReloadType(IEntity weaponEnt, int magAmmoCount, int ammoCountToLoad) {		
		if (!weaponEnt)
			return false;
		
		if(ammoCountToLoad < 3)
			return false;
		
		BCC_BoltAnimationComponent boltAnimComp = BCC_Utils.GetBoltAnimCompFromWeaponEnt(weaponEnt);
		if (!boltAnimComp)
			return false;
		
		if (!boltAnimComp.DoesWeaponAcceptStripperClips())
			return false;
				
		if (BCC_Utils.IsScopeAttachedToWeaponEnt(weaponEnt))
			return false;
		
		return magAmmoCount == ammoCountToLoad;
	}

	protected void SendMagazineToLoad_BCC(IEntity magazine, int ammoCountToLoad, bool reloadType){
		if (!magazine)
			return;
		
		SCR_CharacterControllerComponent charController = GetScrCharacterControllerComponent_BCC();
		if (!charController)
			return;
		
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
		
		if (!currentWeapon)
			return;
		
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
	
	protected bool IsAuthority() {
		IEntity charEnt = GetCharacter();
		if (!charEnt)
			return false;
	
	    RplComponent rplComponent = RplComponent.Cast(charEnt.FindComponent(RplComponent));
	    return (rplComponent && rplComponent.Role() == RplRole.Authority);
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
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void BCC_StripperClipMagazinePredicate()
	{
		QueryComponentTypes.Insert(BaseMagazineComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		// Exclude magazines that are in weapon storage (already loaded in a weapon)
		
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
