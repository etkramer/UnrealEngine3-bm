//=============================================================================
// ReplicationInfo.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class ReplicationInfo extends Info
	abstract
	native;

cpptext
{
	INT* GetOptimizedRepList( BYTE* Recent, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

defaultproperties
{
	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=True
	bStatic=False
	bNoDelete=False

	Components.Remove(Sprite)
}
