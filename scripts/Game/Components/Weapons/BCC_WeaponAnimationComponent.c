class BCC_WeaponAnimationComponentClass : WeaponAnimationComponentClass {}

// This class is just for holding generics
class BCC_WeaponAnimationComponent : WeaponAnimationComponent {
	protected IEntity m_Owner;
	protected typename magWellType;
	protected int m_sMaxAmmo = -1;
	
	void BCC_WeaponAnimationComponent(IEntityComponentSource src, IEntity ent, IEntity parent) {
		m_Owner = ent;
		InitMaxAmmo();
	}
	
	
	
	/* ************************* GENERICS ********************************/
	protected SCR_WeaponInfo getWeaponInfoHud() {
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (!hudManager)
			return null;
		return SCR_WeaponInfo.Cast(hudManager.FindInfoDisplay(SCR_WeaponInfo));
	}
	
	protected bool IsAuthority(){
	    RplComponent rplComponent = RplComponent.Cast(m_Owner.FindComponent(RplComponent));
	    return (rplComponent && rplComponent.Role() == RplRole.Authority);
	}

	protected IEntity GetMagFromInv() {
		SCR_InventoryStorageManagerComponent invMan = GetPlayerInvMan();
		if (!invMan)
			return null;

		if (!magWellType && GetMagComp())
			magWellType = GetMagwellType();

		SCR_MagazinePredicate predicate = new SCR_MagazinePredicate();
		predicate.magWellType = magWellType;

		IEntity magEntity = invMan.FindItem(predicate);
		return magEntity;
	}

	protected bool IsLocalPlayer() {
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return false;
		IEntity playerControllerEnt = playerController.GetControlledEntity();
		if (!playerControllerEnt)
			return false;
		IEntity weapon = GetOwner();
		if (!weapon)
			return false;
		IEntity characterEnt = weapon.GetParent();
		if (!characterEnt)
			return false;
		return playerControllerEnt == characterEnt;
	}

	protected int GetCurrentAmmoCount() {
		MagazineComponent magComp = GetMagComp();
		if (!magComp)
			return 0;
		return magComp.GetAmmoCount();
	}

	protected void InitMaxAmmo() {
		if (m_sMaxAmmo != -1)
			return;
		MagazineComponent mag = GetMagComp();
		if (!mag)
			return;
		m_sMaxAmmo = mag.GetMaxAmmoCount();
	}

	int GetMaxAmmo(){
		if (m_sMaxAmmo == -1)
			InitMaxAmmo();
		return m_sMaxAmmo;
	}
	
	protected WeaponComponent GetWeaponComp() {
		if (!m_Owner)
			return null;
		return WeaponComponent.Cast(m_Owner.FindComponent(WeaponComponent));
	}

	protected ChimeraCharacter GetChimeraChar() {
		if (!m_Owner)
			return null;
		return ChimeraCharacter.Cast(m_Owner.GetParent());
	}

	protected SCR_CharacterControllerComponent GetCharController() {
		ChimeraCharacter chimeraChar = GetChimeraChar();
		if (!chimeraChar)
			return null;
		return SCR_CharacterControllerComponent.Cast(chimeraChar.GetCharacterController());
	}

	protected IEntity GetCharEnt() {
		if (!m_Owner)
			return null;
		return m_Owner.GetParent();
	}

	protected CharacterAnimationComponent GetCharAnimComp() {
		IEntity playerEnt = GetCharEnt();
		if (!playerEnt)
			return null;
		return CharacterAnimationComponent.Cast(playerEnt.FindComponent(CharacterAnimationComponent));
	}
	
	protected SCR_CharacterCommandHandlerComponent GetCharCommandHandlerComp() {
		IEntity playerEnt = GetCharEnt();
		if (!playerEnt)
			return null;
		return SCR_CharacterCommandHandlerComponent.Cast(playerEnt.FindComponent(SCR_CharacterCommandHandlerComponent));
	}

	protected SCR_InventoryStorageManagerComponent GetPlayerInvMan() {
		SCR_CharacterControllerComponent charController = GetCharController();
		if (!charController)
			return null;
		return SCR_InventoryStorageManagerComponent.Cast(charController.GetInventoryStorageManager());
	}

	protected MagazineComponent GetMagComp() {
		WeaponComponent weaponComp = GetWeaponComp();
		if (!weaponComp)
			return null;
		return MagazineComponent.Cast(weaponComp.GetCurrentMagazine());
	}

	protected IEntity GetMagEnt() {
		MagazineComponent magComp = GetMagComp();
		if (!magComp)
			return null;
		return magComp.GetOwner();
	}

	protected typename GetMagwellType() {
		MagazineComponent magComp = GetMagComp();
		if (!magComp)
			return typename.Empty;
		BaseMagazineWell magWell = magComp.GetMagazineWell();
		if (!magWell)
			return typename.Empty;
		return magWell.Type();
	}

	int GetMagSlotIdx() {
		IEntity magEnt = GetMagEnt();
		if (!magEnt)
			return -1;
		InventoryMagazineComponent magInvComp = InventoryMagazineComponent.Cast(magEnt.FindComponent(InventoryMagazineComponent));
		if (!magInvComp)
			return -1;
		InventoryStorageSlot magParentSlot = magInvComp.GetParentSlot();
		if (!magParentSlot)
			return -1;
		return magParentSlot.GetID();
	}

	protected MuzzleComponent GetCurrentMuzzle() {
		WeaponComponent weaponComp = GetWeaponComp();
		if (!weaponComp)
			return null;
		return MuzzleComponent.Cast(weaponComp.GetCurrentMuzzle());
	}
	
	protected void ClearCurrentChamber() {
		MuzzleComponent muzzleComp = GetCurrentMuzzle();
		if (!muzzleComp)
			return;
		int muzzleCompIdx = muzzleComp.GetCurrentBarrelIndex();
	
		if (muzzleCompIdx < 0)
			return;
		muzzleComp.ClearChamber(muzzleCompIdx);
	}

	protected bool IsWeaponChambered() {
		MuzzleComponent muzzleComp = GetCurrentMuzzle();
		if (!muzzleComp)
			return false;
		return muzzleComp.IsCurrentBarrelChambered();
	}

	protected bool IsWeaponEmpty() {
		MagazineComponent magComp = GetMagComp();
		if (!magComp)
			return true;
		return magComp.GetAmmoCount() == 0;
	}

	protected void SetAnimBoolVars(TAnimGraphVariable playerVar, TAnimGraphVariable weaponVar, bool value) {
		if (playerVar != -1) {
			CharacterAnimationComponent charAnimComp = GetCharAnimComp();
			if (charAnimComp)
				charAnimComp.SetVariableBool(playerVar, value);
		}
		SetBoolVariable(weaponVar, value);
	}

	protected bool HasAttachedMagazine() {
		if (!m_Owner)
			return false;
		IEntity child = m_Owner.GetChildren();
		while (child) {
			if (child.FindComponent(MagazineComponent))
				return true;
			child = child.GetSibling();
		}
		return false;
	}

	protected void SetAmmoCount(MagazineComponent magazine, int ammoCount) {
		if (!magazine)
			return;
		IEntity magEntity = magazine.GetOwner();
		if (!magEntity)
			return;
		RplComponent rplComponent = RplComponent.Cast(magEntity.FindComponent(RplComponent));
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
			magazine.SetAmmoCount(ammoCount);
	}
}
		