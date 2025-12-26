class BCC_Utils {
	
	/********************************* Magazines ************************************/
	static MagazineComponent GetMagCompFromMagEnt(IEntity magEnt) {
		if (!magEnt)
			return null;
		return MagazineComponent.Cast(magEnt.FindComponent(MagazineComponent));
	}
	
	static int GetAmmoCountFromMagEnt(IEntity magEnt) {
		if (!magEnt)
			return -1;
		BaseMagazineComponent magComp = GetMagCompFromMagEnt(magEnt);
		if (!magComp)
			return -1;
		return magComp.GetAmmoCount();
	}
	
	static int GetMaxAmmoCountFromMagEnt(IEntity magEnt) {
		if (!magEnt)
			return -1;
		BaseMagazineComponent magComp = BaseMagazineComponent.Cast(magEnt.FindComponent(BaseMagazineComponent));
		if (!magComp)
			return -1;
	
		return magComp.GetMaxAmmoCount();
	}
	
	static int GetMaxAmmoCountFromWeaponEnt(IEntity weaponEnt) {
		if (!weaponEnt)
			return -1;
		MagazineComponent magComp = GetMagCompFromWeaponEnt(weaponEnt);
		if (!magComp){
			return 0;
		}
		return magComp.GetMaxAmmoCount();
	}
	
	static int GetAmmoCountFromWeaponEnt(IEntity weaponEnt) {
		if (!weaponEnt)
			return -1;
		MagazineComponent magComp = GetMagCompFromWeaponEnt(weaponEnt);
		if (!magComp)
			return 0;
		return magComp.GetAmmoCount();
	}
	
	static MagazineComponent GetMagCompFromWeaponEnt(IEntity weaponEnt) {
		BaseWeaponComponent weaponComp = GetWeaponCompFromWeaponEnt(weaponEnt);
		if (!weaponComp)
			return null;
		return MagazineComponent.Cast(weaponComp.GetCurrentMagazine());
	}
	
	static InventoryMagazineComponent GetMagInvCompFromMagEnt(IEntity magEnt) {
		if (!magEnt)
			return null;
		return InventoryMagazineComponent.Cast(magEnt.FindComponent(InventoryMagazineComponent));
	}

	static BCC_StripperClipInventoryMagazineComponent GetMagStripperClipInvCompFromMagEnt(IEntity magEnt) {
		if (!magEnt)
			return null;
		return BCC_StripperClipInventoryMagazineComponent.Cast(magEnt.FindComponent(BCC_StripperClipInventoryMagazineComponent));
	}
	
	/********************************* Ownership ************************************/
	
	static bool IsAuthority(IEntity ent) {
		if (!ent)
			return false;
	
	    RplComponent rplComponent = RplComponent.Cast(ent.FindComponent(RplComponent));
		if (!rplComponent) {
			Print("[BCC] Called without RPLComponent!", LogLevel.ERROR);
			return false;
		}
		PrintFormat("Is Authority? %1", rplComponent.Role() == RplRole.Authority);
	    return (rplComponent && rplComponent.Role() == RplRole.Authority);
	}
	
	static bool IsOwner(IEntity ent) {
		if (!ent)
			return false;
	
	    RplComponent rplComponent = RplComponent.Cast(ent.FindComponent(RplComponent));
		if (!rplComponent) {
			Print("[BCC] Called without RPLComponent!", LogLevel.ERROR);
			return false;
		}
		PrintFormat("Is Owner? %1", rplComponent.IsOwner());
	    return (rplComponent && rplComponent.IsOwner());
	}
	
		static bool IsOwnerProxy(IEntity ent) {
		if (!ent)
			return false;
	
	    RplComponent rplComponent = RplComponent.Cast(ent.FindComponent(RplComponent));
		if (!rplComponent) {
			Print("[BCC] Called without RPLComponent!", LogLevel.ERROR);
			return false;
		}
		PrintFormat("Is OwnerProxy? %1", rplComponent.IsOwnerProxy());
	    return (rplComponent && rplComponent.IsOwnerProxy());
	}
	
	
	static bool IsMaster(IEntity ent) {
		if (!ent)
			return false;
	
	    RplComponent rplComponent = RplComponent.Cast(ent.FindComponent(RplComponent));
		if (!rplComponent) {
			Print("[BCC] Called without RPLComponent!", LogLevel.ERROR);
			return false;
		}
		PrintFormat("Is Master? %1", rplComponent.IsMaster());
	    return (rplComponent && rplComponent.IsMaster());
	}
	
	static void DebugOwnership(IEntity ent) {
		if (!ent)
			return;
	
	    RplComponent rplComponent = RplComponent.Cast(ent.FindComponent(RplComponent));
		if (!rplComponent) {
			Print("[BCC] Called without RPLComponent! Cannot debug.");
			return;
		}
		PrintFormat("Is Owner? %1", rplComponent.IsOwner());
		PrintFormat("Is Authority? %1", rplComponent.Role() == RplRole.Authority);
		PrintFormat("Is Master? %1", rplComponent.IsMaster());
	}
	
	/********************************* Weapons ************************************/
	
	static WeaponComponent GetWeaponCompFromWeaponEnt(IEntity weaponEnt) {
		if (!weaponEnt)
			return null;
		return WeaponComponent.Cast(weaponEnt.FindComponent(WeaponComponent));
	}
	
	static IEntity GetWeaponEntFromWeaponComp(WeaponComponent weaponComp) {
		if (!weaponComp)
			return null;
		return weaponComp.GetOwner();
	}
	
	static MuzzleComponent GetMuzzleCompFromWeaponEnt(IEntity weaponEnt) {
		if (!weaponEnt)
			return null;
		WeaponComponent weaponComp = GetWeaponCompFromWeaponEnt(weaponEnt);
		if (!weaponComp)
			return null;
		return MuzzleComponent.Cast(weaponComp.GetCurrentMuzzle());
	}
	
	static bool IsScopeAttachedToWeaponEnt(IEntity weaponEnt) {
		if (!weaponEnt)
			return false;
		
		BaseWeaponComponent weaponComp = BaseWeaponComponent.Cast(weaponEnt.FindComponent(BaseWeaponComponent));
		if (!weaponComp)
			return false;
		
		array<AttachmentSlotComponent> attachments = {};
		
		weaponComp.GetAttachments(attachments);
		
		foreach(AttachmentSlotComponent attachment: attachments) {
			BaseAttachmentType type = attachment.GetAttachmentSlotType();
			if (type && type.Type().IsInherited(AttachmentOptics) && attachment.GetAttachedEntity()!= null)
				return true;
		}
		
		return false;
	}

	static bool IsWeaponEntChambered(IEntity weaponEnt) {
		MuzzleComponent muzzleComp = BCC_Utils.GetMuzzleCompFromWeaponEnt(weaponEnt);
		if (!muzzleComp){
			Print("[BCC] no muzzle comp found!", LogLevel.ERROR);
			return false;
		}
		return muzzleComp.IsCurrentBarrelChambered();
	}
	
	static bool IsWeaponEmpty(IEntity weaponEnt) {
		MagazineComponent magComp = GetMagCompFromWeaponEnt(weaponEnt);
		if (!magComp)
			return true;
		return magComp.GetAmmoCount() == 0;
	}
	
	static bool IsWeaponInternalMagBroken(IEntity weaponEnt) {
		if(!weaponEnt)
			return true;
		BaseMagazineComponent magComp = GetMagCompFromWeaponEnt(weaponEnt);
		return magComp == null;
	}
	
	static BCC_BoltAnimationComponent GetBoltAnimCompFromWeaponEnt(IEntity weaponEnt) {
		return BCC_BoltAnimationComponent.Cast(weaponEnt.FindComponent(BCC_BoltAnimationComponent));
	}
	
	static BCC_StripperClipMagazineAnimationComponent GetStripperClipAnimCompFromMagEnt(IEntity magEnt) {
		return BCC_StripperClipMagazineAnimationComponent.Cast(magEnt.FindComponent(BCC_StripperClipMagazineAnimationComponent));
	}
}

 





