modded class SCR_WeaponInfo
{
	
	override protected void OnWeaponChanged(BaseWeaponComponent weapon, BaseWeaponComponent prevWeapon) {
		super.OnWeaponChanged(weapon, prevWeapon);
		
		UpdateCustomBoltHud();
	}
	
	override void OnAmmoCountChanged(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle, BaseMagazineComponent magazine, int ammoCount, bool isBarrelChambered) {
		super.OnAmmoCountChanged(weapon, muzzle, magazine, ammoCount, isBarrelChambered);
		
		UpdateCustomBoltHud();
	}
	
	override protected void UpdateMagazineIndicator_Textures(SCR_WeaponState state) {

		super.UpdateMagazineIndicator_Textures(state);
		if (IsBoltWeapon(state))
			UpdateCustomBoltHud();
	}
	
	override protected void UpdateMagazineIndicator_Progress(SCR_WeaponState state) {
		
		super.UpdateMagazineIndicator_Progress(state);
		if (IsBoltWeapon(state))
			UpdateCustomBoltHud();
		
	}
	
	override void UpdateFireModeIndicator(SCR_WeaponState state) {
	
		super.UpdateFireModeIndicator(state);
				if (IsBoltWeapon(state))
			UpdateCustomBoltHud();
	}
	
	protected bool IsBoltWeapon(SCR_WeaponState state)
	{
		if (!state || !state.m_Weapon)
			return false;
		
		IEntity weaponEntity = state.m_Weapon.GetOwner();
		if (!weaponEntity)
			return false;
		
		BCC_BoltAnimationComponent boltComp = BCC_BoltAnimationComponent.Cast(weaponEntity.FindComponent(BCC_BoltAnimationComponent));
		return boltComp != null;
	}
	
	protected void UpdateCustomBoltHud() {
		if (!m_Widgets || !m_WeaponState || !m_WeaponState.m_Weapon)
			return;
		
		IEntity weaponEntity = m_WeaponState.m_Weapon.GetOwner();
		if (!weaponEntity)
			return;
		
		BCC_BoltAnimationComponent boltComp = BCC_BoltAnimationComponent.Cast(weaponEntity.FindComponent(BCC_BoltAnimationComponent));
		if (!boltComp)
			return;
		
		boltComp.UpdateHud();
		Print("CALLED");
	}
	
}