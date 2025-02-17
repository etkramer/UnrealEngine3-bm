/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_FracturedStaticMesh: Fractured Static Meshes
//=============================================================================

class GenericBrowserType_FracturedStaticMesh
	extends GenericBrowserType_StaticMesh
	native;

cpptext
{
	virtual void Init();
	virtual void InvokeCustomCommand( INT InCommand, UObject* InObject );
}
	
defaultproperties
{
	Description="Fractured Static Mesh"
}
