//=============================================================================
// StaticMeshActor.
// An actor that is drawn using a static mesh(a mesh that never changes, and
// can be cached in video memory, resulting in a speed boost).
// Note that PostBeginPlay() and SetInitialState() are never called for StaticMeshActors
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================

class StaticMeshActor extends StaticMeshActorBase
	native
	placeable;

var() const editconst StaticMeshComponent	StaticMeshComponent;

cpptext
{
	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors();

protected:
	/** 
	* This function actually does the work for the GetDetailInfo and is virtual.  
	* It should only be called from GetDetailedInfo as GetDetailedInfo is safe to call on NULL object pointers
	**/
	virtual FString GetDetailedInfoInternal() const;
}

event PreBeginPlay() {}

defaultproperties
{
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		bAllowApproximateOcclusion=TRUE
		bCastDynamicShadow=FALSE
		bForceDirectLightMap=TRUE
		bUsePrecomputedShadows=TRUE
	End Object
	CollisionComponent=StaticMeshComponent0
	StaticMeshComponent=StaticMeshComponent0
	Components.Add(StaticMeshComponent0)
}
