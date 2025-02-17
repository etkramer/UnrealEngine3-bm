/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_PostProcess: Animation Blend Trees
//=============================================================================

class GenericBrowserType_PostProcess
	extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );
}
	
defaultproperties
{
	Description="PostProcesss"
}
