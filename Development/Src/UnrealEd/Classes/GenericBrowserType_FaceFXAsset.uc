/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_FaceFXAsset: FaceFX Assets
//=============================================================================

class GenericBrowserType_FaceFXAsset
	extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );
	virtual UBOOL ShowObjectProperties( UObject* InObject );
	virtual UBOOL ShowObjectProperties( const TArray<UObject*>& InObjects );
	virtual void InvokeCustomCommand( INT InCommand, UObject* InObject );
}
	
defaultproperties
{
	Description="FaceFX Asset"
}