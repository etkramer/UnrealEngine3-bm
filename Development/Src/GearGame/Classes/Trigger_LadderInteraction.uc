/*
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Trigger_LadderInteraction extends Trigger
	native
	placeable;

cpptext
{
	virtual INT AddMyMarker( AActor *S );
}


/** Indicates if this trigger is hooked up to the top of the ladder */
var()	bool			bIsTopOfLadder;
/** Pointer to Ladder StaticMesh Actor */
var()	StaticMeshActor	LadderSMActor;

var()	vector			BottomLocOffset;
var()	Rotator			BottomRotOffset;

var()	vector			TopLocOffset;
var()	Rotator			TopRotOffset;

/** Top and Bottom Ladder Markers for AI movement */
var()	editconst LadderMarker	TopMarker, BottomMarker;

/** Option to override the animation used */
var() Name ClimbDownAnim, ClimbUpAnim;

simulated function OnToggle(SeqAct_Toggle Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		SetCollision(TRUE,FALSE);
	}
	else
	if (Action.InputLinks[1].bHasImpulse)
	{
		SetCollision(FALSE,FALSE);
	}
	else
	{
		SetCollision(!bCollideActors,FALSE);
	}
}

`if(`notdefined(FINAL_RELEASE))
simulated function Tick(float DeltaTime)
{
	local Vector	EntryLoc;
	local Rotator	EntryRot;

	Super.Tick(DeltaTime);

	if( LadderSMActor != None && bDebug )
	{
		// Debug for entry points
		if( bIsTopOfLadder )
		{
			GetTopEntryPoint(EntryLoc, EntryRot);
			DrawDebugCoordinateSystem(EntryLoc, EntryRot, 100.f, FALSE);
		}
		else
		{
			GetBottomEntryPoint(EntryLoc, EntryRot);
			DrawDebugCoordinateSystem(EntryLoc, EntryRot, 100.f, FALSE);
		}
	}
}
`endif

simulated native function GetTopEntryPoint( out vector out_Loc, out rotator out_Rot );
simulated native function GetBottomEntryPoint( out vector out_Loc, out rotator out_Rot );

defaultproperties
{
	Begin Object Name=CollisionCylinder
		CollisionRadius=128.f
		CollisionHeight=128.f
	End Object

	// Magic numbers to align player animation with ladder
	BottomLocOffset=(X=-73,Z=5)
	BottomRotOffset=(Yaw=-16384)
	TopLocOffset=(X=-62,Z=500.f)
	TopRotOffset=(Yaw=16384)

	Begin Object Class=LadderMeshComponent Name=LadderMesh
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
		bUsePrecomputedShadows=False
	End Object
	Components.Add(LadderMesh)


	bDebug=FALSE
}
