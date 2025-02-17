/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTScout extends Scout
	native
	transient;

var bool bRequiresDoubleJump;
var float MaxDoubleJumpHeight;

/** Square of maximum distance that can be traversed by one translocator toss */
var float MaxTranslocDistSq;
/** Translocator projectile class to use for throw tests */
var class<UTProjectile> TranslocProjClass;

/* UTScout uses the properties from this class (jump height etc.) to override UTScout default settings */
var class<UTPawn> PrototypePawnClass;

var name SpecialReachSpecsWarningLog;

var name SizePersonFindName;
cpptext
{
	virtual void AddSpecialPaths(INT NumPaths, UBOOL bOnlyChanged);
	virtual ETestMoveResult FindJumpUp(FVector Direction, FVector &CurrentPosition);
	virtual UBOOL SetHighJumpFlag();
	virtual void SetPrototype();
	void CreateTranslocatorPath(ANavigationPoint* Nav, ANavigationPoint* DestNav, FCheckResult Hit, UBOOL bOnlyChanged);
	virtual ETestMoveResult FindBestJump(FVector Dest, FVector &CurrentPosition);

	virtual void SetPathColor(UReachSpec* ReachSpec)
	{
		FVector CommonSize = GetSize(FName(TEXT("Common"),FNAME_Find));
		if ( ReachSpec->CollisionRadius >= CommonSize.X )
		{
			FVector MaxSize = GetSize(FName(TEXT("Vehicle"),FNAME_Find));
			ReachSpec->PathColorIndex = ( ReachSpec->CollisionRadius >= MaxSize.X ) ? 2 : 1;
		}
		else
		{
			ReachSpec->PathColorIndex = 0;
		}
	}

	virtual void ReviewPaths();
}

/**
SuggestJumpVelocity()
returns true if succesful jump from start to destination is possible
returns a suggested initial falling velocity in JumpVelocity
Uses GroundSpeed and JumpZ as limits
*/
native function bool SuggestJumpVelocity(out vector JumpVelocity, vector Destination, vector Start);

defaultproperties
{
	Begin Object NAME=CollisionCylinder
		CollisionRadius=+0034.000000
	End Object

	PathSizes.Empty
	PathSizes(0)=(Desc=Crouched,Radius=22,Height=29)
	PathSizes(1)=(Desc=Human,Radius=22,Height=44)
	PathSizes(2)=(Desc=Small,Radius=72,Height=44)
	PathSizes(3)=(Desc=Common,Radius=100,Height=44)
	PathSizes(4)=(Desc=Max,Radius=140,Height=100)
	PathSizes(5)=(Desc=Vehicle,Radius=260,Height=100)

	TestJumpZ=322
	TestGroundSpeed=440
	TestMaxFallSpeed=2500
	MaxStepHeight=26.0
	MaxJumpHeight=49.0
	MaxDoubleJumpHeight=85.0
	MinNumPlayerStarts=16
	WalkableFloorZ=0.78
	MaxTranslocDistSq=4000000.0
	TranslocProjClass=class'UTProj_TransDisc'

	PrototypePawnClass=class'UTGame.UTPawn'
	SpecialReachSpecsWarningLog="Adding Special Reachspecs"
	SizePersonFindName=Human
}
