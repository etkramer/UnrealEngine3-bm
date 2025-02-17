/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_FaceFXAnimSet: FaceFX AnimSets
//=============================================================================

class GenericBrowserType_FaceFXAnimSet
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
	Description="FaceFX AnimSet"
}