/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_ParticleSystem: ParticleSystems
//=============================================================================

class GenericBrowserType_ParticleSystem
	extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );
	virtual void InvokeCustomCommand( INT InCommand, UObject* InObject );
}
	
defaultproperties
{
	Description="Particle System"
}
