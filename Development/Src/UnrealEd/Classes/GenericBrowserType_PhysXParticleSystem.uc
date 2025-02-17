class GenericBrowserType_PhysXParticleSystem
	extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor(UObject *InObject);
}

defaultproperties
{
  	Description = "PhysX Particle System"
}
