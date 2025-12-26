class BCC_StripperClipInventoryMagazineComponentClass: InventoryMagazineComponentClass
{}


class BCC_StripperClipInventoryMagazineComponent: InventoryMagazineComponent
{
	bool m_bShouldHideStripperClip = false;
	
	
	void SetHideStripperClipInVicinity(bool vis) {
		m_bShouldHideStripperClip = vis;
	}
	
	override event bool ShouldHideInVicinity() {
		if(m_bShouldHideStripperClip)
			return m_bShouldHideStripperClip;
		
		return super.ShouldHideInVicinity();
	}
	
	
}