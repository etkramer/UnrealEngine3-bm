/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_MaterialInstance: Material Instances
//=============================================================================

class GenericBrowserType_MaterialInstanceTimeVarying
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
	Description="Material Instance Time Varying"
}
