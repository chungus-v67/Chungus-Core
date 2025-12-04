class BCC_StripperClipMagazineAnimationComponentClass: MagazineAnimationComponentClass
{
}

class BCC_StripperClipMagazineAnimationComponent: MagazineAnimationComponent
{
	[Attribute("", desc:"Name of stripper clip mesh to show/hide")]
	protected string m_sClipMesh;
	
	[Attribute("", desc:"Array of bullet mesh names filtered. Similar to 'Bullet Meshes to hide', which is incompatible here.")]
	protected ref array<string> m_aBulletMeshes;
	
	[Attribute(desc:"Point and vector used to control the ejection direction. Keep COM in mind")]
	protected ref PointInfo m_stripEjectPoint;
	
	[Attribute("2.5", desc:"Force applied to the ejected magazine")]
	protected float m_fStripperEjectForce;
	
	protected IEntity m_Owner;
	protected int m_ammoReloadCount = -1;
	protected bool m_reloadType = 0;
	
	protected AnimationEventID m_BC_InsertRound0 = -1;
	protected AnimationEventID m_BC_InsertRound1 = -1;
	protected AnimationEventID m_BC_InsertRound2 = -1;
	protected AnimationEventID m_BC_InsertRound3 = -1;
	protected AnimationEventID m_BC_InsertRound4 = -1;
	protected AnimationEventID m_BC_EjectStripperClip = -1;
	
	protected TAnimGraphCommand m_CMD_BC_BoltActionReload = -1;
	
	protected TAnimGraphVariable m_BC_ReturnToIdle = -1;
	protected TAnimGraphVariable m_BC_IsStripperClipReload = -1;
	
	void BCC_StripperClipMagazineAnimationComponent(IEntityComponentSource src, IEntity ent, IEntity parent) {
		m_Owner = ent;
		m_CMD_BC_BoltActionReload 		= BindCommand("CMD_BC_BoltActionReload");
		m_BC_ReturnToIdle 				= BindBoolVariable("BC_ReturnToIdle");
		m_BC_IsStripperClipReload 		= BindBoolVariable("BC_IsStripperClipReload");
		
		m_BC_InsertRound0				= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound0");
		m_BC_InsertRound1 				= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound1");
		m_BC_InsertRound2 				= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound2");
		m_BC_InsertRound3 				= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound3");
		m_BC_InsertRound4 				= GameAnimationUtils.RegisterAnimationEvent("BC_InsertRound4");
	}
	
	override event void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd) {
		super.OnAnimationEvent(animEventType, animUserString, intParam, timeFromStart, timeToEnd);
		
		switch (animEventType) {
			case m_BC_InsertRound0:
				InsertRound(0);
				break;
			case m_BC_InsertRound1:
				InsertRound(1);
				break;
			case m_BC_InsertRound2:
				InsertRound(2);
				break;
			case m_BC_InsertRound3:
				InsertRound(3);
				break;
			case m_BC_InsertRound4:
				InsertRound(4);
				break;
		}
	}
	
	void SetReloadAmmoCount(int ammoCount){
		m_ammoReloadCount = ammoCount;
	}
	
	int GetReloadAmmoCount(){
		return m_ammoReloadCount;
	}

	protected void InsertRound(int bulletIdx) {
		MagazineComponent magComp = GetMagComp();
		if (!magComp)
			return;
		int ammoCount = magComp.GetAmmoCount() -1;
					
	    RplComponent rplComponent = RplComponent.Cast(m_Owner.FindComponent(RplComponent));
	    if (rplComponent && rplComponent.Role() == RplRole.Authority){
	        magComp.SetAmmoCount(ammoCount);
		}
		int offset = GetStripperIdxOffset();
		if (bulletIdx < m_ammoReloadCount + offset) {
			HideRoundMesh(bulletIdx);
		}
		
		// Return to idle
		if (bulletIdx == m_ammoReloadCount + offset -1) {
			SetBoolVariable(m_BC_ReturnToIdle, true);
		}
	}
	
	// TODO: Add Replication
	void EjectStripperClip() {    
		if (!m_Owner)
			return;

		IEntity weapon = m_Owner.GetParent();
		if (!weapon)
			return;

		MagazineComponent magComponent = GetMagComp();
		if (!magComponent)
			return;

		IEntity magEntity = magComponent.GetOwner();
		if (!magEntity)
			return;

		BCC_StripperClipImpactSoundComponent clipSound = BCC_StripperClipImpactSoundComponent.Cast(magEntity.FindComponent(BCC_StripperClipImpactSoundComponent));
		if (clipSound) clipSound.m_HasPlayed = false;

		Physics magPhysics = magEntity.GetPhysics();
		if (!magPhysics)
			return;

		IEntity magParent = magEntity.GetParent();
		if (magParent)
			magParent.RemoveChild(magEntity, true);

		vector magTransform[4];
		magEntity.GetTransform(magTransform);

		vector worldEjectPoint = magTransform[3];
		vector ejectDirection = magTransform[2];
		vector stripPointWorldTransform[4];
		vector stripPointLocalTransform[4];
		if (m_stripEjectPoint) {
			m_stripEjectPoint.GetWorldTransform(stripPointWorldTransform);
			worldEjectPoint = stripPointWorldTransform[3];
			
			m_stripEjectPoint.GetLocalTransform(stripPointLocalTransform);
			vector localDirection = stripPointLocalTransform[2];
			ejectDirection = localDirection.Multiply3(magTransform);
		}
		float dirLength = ejectDirection.Length();
		if (dirLength > 0.001) {
			ejectDirection /= dirLength;
		} else {
			ejectDirection = magTransform[2];
		}

		magPhysics.ChangeSimulationState(SimulationState.SIMULATION);
		magPhysics.SetInteractionLayer(EPhysicsLayerPresets.Debris);
		magPhysics.EnableGravity(true);

		vector weaponVelocity = vector.Zero;
		Physics weaponPhysics = weapon.GetPhysics();
		if (weaponPhysics)
			weaponVelocity = weaponPhysics.GetVelocity();
		magPhysics.SetVelocity(weaponVelocity);

		vector impulse = ejectDirection * magPhysics.GetMass() * m_fStripperEjectForce;
		magPhysics.ApplyImpulseAt(worldEjectPoint, impulse);
		magPhysics.SetAngularVelocity("0 0 6");		
	} 

	protected void HideRoundMesh(int bulletIdx) {
		int meshIdx = GameAnimationUtils.FindMeshIndex(m_Owner, m_aBulletMeshes[bulletIdx]);
		if (!meshIdx)
			return;

		GameAnimationUtils.ShowMesh(m_Owner, meshIdx, false);
	}
	
	void SetReloadType(bool reloadType){
		m_reloadType = reloadType;
	}
	
	bool GetReloadType() {
		return m_reloadType;
	}
	
	protected MagazineComponent GetMagComp(){
		if (!m_Owner)
			return null;
		return MagazineComponent.Cast(m_Owner.FindComponent(MagazineComponent));
	}
	
	void StartReload() {
		SetBoolVariable(m_BC_ReturnToIdle, false);
		SetBoolVariable(m_BC_IsStripperClipReload, m_reloadType);
		CallCommand(m_CMD_BC_BoltActionReload, 0, 0.0);
		
		Rpc(RPC_SendReloadCommand, m_reloadType); 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server-side RPC for rack bolt command
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_SendReloadCommand(bool reloadType)
	{		
		Rpc(RPC_DoSendReloadCommand, reloadType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Broadcast RPC for rack bolt command
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoSendReloadCommand(bool reloadType)
	{
		if (m_BC_IsStripperClipReload != -1)
			SetBoolVariable(m_BC_IsStripperClipReload, reloadType);
		
		if (m_CMD_BC_BoltActionReload != -1)
			CallCommand(m_CMD_BC_BoltActionReload, 1, 0.0);
	}
	
	protected int GetStripperIdxOffset() {
		if (!m_reloadType)
			return 0;
		MagazineComponent magComp = GetMagComp();
		if (!magComp)
			return 0;
		return magComp.GetMaxAmmoCount() - m_ammoReloadCount;
	}
	
	void SetBulletVisibility() {		
		if (!m_Owner)
			return;
		
		if (!m_ammoReloadCount)
			return;
		
		int offset = GetStripperIdxOffset();
		
		// Show stripper
		if (m_reloadType) {
			int meshIdx = -1;
			if (m_sClipMesh.Length())
				meshIdx = GameAnimationUtils.FindMeshIndex(m_Owner, m_sClipMesh);
			if (meshIdx != -1)
				GameAnimationUtils.ShowMesh(m_Owner, meshIdx , true);
		}
			
		
		for (int i = 0; i< m_ammoReloadCount; i++) {
			int meshIdx = -1;
			if (i < m_aBulletMeshes.Count()) {
				string bulletMesh = m_aBulletMeshes[i + offset];
			if (bulletMesh.Length())
				meshIdx = GameAnimationUtils.FindMeshIndex(m_Owner, bulletMesh);
			if (meshIdx != -1)
				GameAnimationUtils.ShowMesh(m_Owner, meshIdx , true);				
			}
		}
		
	}
	
	protected InventoryMagazineComponent GetMagInvComp(){
		return InventoryMagazineComponent.Cast(m_Owner.FindComponent(InventoryMagazineComponent));
	}
	
	
	void HideAllBulletMesh() {
		int meshIdx = -1;
		foreach(string bulletMesh: m_aBulletMeshes) {
			if (bulletMesh.Length())
				meshIdx = GameAnimationUtils.FindMeshIndex(m_Owner, bulletMesh);
			if (meshIdx != -1)
				GameAnimationUtils.ShowMesh(m_Owner, meshIdx , false);
		}
		
		meshIdx = -1;
		if (m_sClipMesh.Length())
			meshIdx = GameAnimationUtils.FindMeshIndex(m_Owner, m_sClipMesh);
		if (meshIdx != -1)
			GameAnimationUtils.ShowMesh(m_Owner, meshIdx , false);
	}
}
