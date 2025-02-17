/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_Texture: Textures
//=============================================================================

class GenericBrowserType_Texture
	extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );
	virtual void InvokeCustomCommand( INT InCommand, UObject* InObject );

	/**
	* Callback to register whether or not the object should be displayed
	* @param InObject - object that will be displayed in the GB
	* @return TRUE if should be displayed
	*/ 
	static UBOOL ShouldDisplayCallback( UObject* InObject );
}
	
defaultproperties
{
	Description="Texture"
}
