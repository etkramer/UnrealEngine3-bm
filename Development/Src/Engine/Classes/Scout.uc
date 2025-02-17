//=============================================================================
// Scout used for path generation.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class Scout extends Pawn
	native
	config(Game)
	notplaceable
	transient
	dependsOn(ReachSpec);

cpptext
{
	NO_DEFAULT_CONSTRUCTOR(AScout)

	virtual void InitForPathing( ANavigationPoint* Start, ANavigationPoint* End )
	{
		Physics = PHYS_Walking;
		JumpZ = TestJumpZ;
		bCanWalk = 1;
		bJumpCapable = 1;
		bCanJump = 1;
		bCanSwim = 1;
		bCanClimbLadders = 1;
		bCanFly = 0;
		GroundSpeed = TestGroundSpeed;
		MaxFallSpeed = TestMaxFallSpeed;
	}

	virtual FVector GetSize(FName desc)
	{
		for (INT idx = 0; idx < PathSizes.Num(); idx++)
		{
			if (PathSizes(idx).Desc == desc)
			{
				return FVector(PathSizes(idx).Radius,PathSizes(idx).Height,0.f);
			}
		}
		return FVector(PathSizes(0).Radius,PathSizes(0).Height,0.f);
	}

	virtual FVector GetDefaultForcedPathSize(UReachSpec* Spec)
	{
		return GetSize(FName(TEXT("Common"),FNAME_Find));
	}

	/** returns the largest size in the PathSizes list */
	FVector GetMaxSize();

	virtual void SetPathColor(UReachSpec* ReachSpec)
	{
		FVector CommonSize = GetSize(FName(TEXT("Common"),FNAME_Find));
		if ( ReachSpec->CollisionRadius >= CommonSize.X )
		{
			FVector MaxSize = GetSize(FName(TEXT("Max"),FNAME_Find));
			if ( ReachSpec->CollisionRadius >= MaxSize.X )
			{
				ReachSpec->PathColorIndex = 2;
			}
			else
			{
				ReachSpec->PathColorIndex = 1;
			}
		}
		else
		{
			ReachSpec->PathColorIndex = 0;
		}
	}

	virtual void AddSpecialPaths(INT NumPaths, UBOOL bOnlyChanged) {};
	virtual void PostBeginPlay();
	virtual void SetPrototype();
	/** updates the highest landing Z axis velocity encountered during a reach test */
	virtual void SetMaxLandingVelocity(FLOAT NewLandingVelocity)
	{
		if (-NewLandingVelocity > MaxLandingVelocity)
		{
			MaxLandingVelocity = -NewLandingVelocity;
		}
	}

	virtual UClass* GetDefaultReachSpecClass() { return DefaultReachSpecClass; }

	/**
	* Toggles collision on all actors for path building.
	*/
	virtual void SetPathCollision(UBOOL bEnabled);

	/**
	* Moves all interp actors to the path building position.
	*/
	virtual void UpdateInterpActors(UBOOL &bProblemsMoving, TArray<USeqAct_Interp*> &InterpActs);

	/**
	* Moves all updated interp actors back to their original position.
	*/
	virtual void RestoreInterpActors(TArray<USeqAct_Interp*> &InterpActs);

	/**
	* Clears all the paths and rebuilds them.
	*
	* @param	bReviewPaths	If TRUE, review paths if any were created.
	* @param	bShowMapCheck	If TRUE, conditionally show the Map Check dialog.
	*/
	virtual void DefinePaths(UBOOL bReviewPaths, UBOOL bShowMapCheck);

	/**
	* Clears all pathing information in the level.
	*/
	virtual void UndefinePaths();

	virtual void AddLongReachSpecs( INT NumPaths );

	virtual void PrunePaths(INT NumPaths);

	// interface to allow easy overides of path prune behavior (without copy+pasting ;) )
	virtual INT PrunePathsForNav(ANavigationPoint* Nav);
	// called after PrunePathsForNav is called on all pathnodes 
	virtual INT SecondPassPrunePathsForNav(ANavigationPoint* Nav){return 0;}


	virtual void ReviewPaths();

	virtual void Exec( const TCHAR* Str );
	virtual void AdjustCover( UBOOL bFromDefinePaths = FALSE );
	virtual void BuildCover(  UBOOL bFromDefinePaths = FALSE );
	virtual void FinishPathBuild();

protected:
	/**
	* Builds the per-level nav lists and then assembles the world list.
	*/
	void BuildNavLists();
};

struct native PathSizeInfo
{
	var Name		Desc;
	var	float		Radius,
					Height,
					CrouchHeight;
	var byte		PathColor;
};
var array<PathSizeInfo>			PathSizes;		// dimensions of reach specs to test for
var float						TestJumpZ,
								TestGroundSpeed,
								TestMaxFallSpeed,
								TestFallSpeed;

var const float MaxLandingVelocity;

var int MinNumPlayerStarts;

/** Specifies the default class to use when constructing reachspecs connecting NavigationPoints */
var class<ReachSpec> DefaultReachSpecClass;

simulated event PreBeginPlay()
{
	// make sure this scout has all collision disabled
	if (bCollideActors)
	{
		SetCollision(FALSE,FALSE);
	}
}

defaultproperties
{
	Components.Remove(Sprite)
	Components.Remove(Arrow)

	RemoteRole=ROLE_None
	AccelRate=+00001.000000
	bCollideActors=false
	bCollideWorld=false
	bBlockActors=false
	bProjTarget=false
	bPathColliding=true

	PathSizes(0)=(Desc=Human,Radius=48,Height=80)
	PathSizes(1)=(Desc=Common,Radius=72,Height=100)
	PathSizes(2)=(Desc=Max,Radius=120,Height=120)
	PathSizes(3)=(Desc=Vehicle,Radius=260,Height=120)

	TestJumpZ=420
	TestGroundSpeed=600
	TestMaxFallSpeed=2500
	TestFallSpeed=1200
	MinNumPlayerStarts=1
	DefaultReachSpecClass=class'Engine.Reachspec'
}
