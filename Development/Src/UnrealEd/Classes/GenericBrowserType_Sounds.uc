/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_Sounds: Sounds
//=============================================================================

class GenericBrowserType_Sounds
	extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );
	virtual void InvokeCustomCommand( INT InCommand, UObject* InObject );
	virtual void DoubleClick( UObject* InObject );

	void Play( USoundCue* InSound );
	void Play( USoundNode* InSound );	
	void Stop();
}
	
defaultproperties
{
	Description="Sounds"
}
