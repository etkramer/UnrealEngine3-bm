/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class GearSavedMove extends SavedMove;

var ECoverType CoverType;
var ECoverAction CoverAction;
var ECoverDirection CoverDirection;
var bool bWantsToBeMirrored;
var ECoverDirection CurrentSlotDirection;
var bool bIsInStationaryCover;

function Clear()
{
	Super.Clear();
	CoverType = CT_None;
	CoverAction = CA_Default;
	CoverDirection = CD_Default;
	bWantsToBeMirrored = FALSE;
	CurrentSlotDirection = CD_Default;
	bIsInStationaryCover = FALSE;
}

function SetMoveFor(PlayerController P, float DeltaTime, vector NewAccel, EDoubleClickDir InDoubleClick)
{
	local GearPawn WP;
	Super.SetMoveFor(P,DeltaTime,NewAccel,InDoubleClick);
	// save the cover settings
	WP = GearPawn(P.Pawn);
	if (WP != None)
	{
		CoverType = WP.CoverType;
		CoverAction = WP.CoverAction;
		CoverDirection = WP.CoverDirection;
		bWantsToBeMirrored = WP.bWantsToBeMirrored;
		CurrentSlotDirection = WP.CurrentSlotDirection;
		bIsInStationaryCover = WP.bIsInStationaryCover;
	}
}

function bool CanCombineWith(SavedMove NewMove, Pawn InPawn, float MaxDelta)
{
	local GearSavedMove NewGearMove;
	local InterpActor_GearBasePlatform Platform;

	//@hack: disallow movement combining when clamped to a base that prevents all movement
	if (InPawn != None)
	{
		Platform = InterpActor_GearBasePlatform(InPawn.Base);
		if (Platform != None && Platform.bDisallowPawnMovement)
		{
			return false;
		}
	}

	NewGearMove = GearSavedMove(NewMove);
	return (CoverType == CT_None &&			//@fixme - disable combining of cover moves for now
			CoverType == NewGearMove.CoverType &&
			CoverAction == NewGearMove.CoverAction &&
			CoverDirection == NewGearMove.CoverDirection &&
			bWantsToBeMirrored == NewGearMove.bWantsToBeMirrored &&
			CurrentSlotDirection == NewGearMove.CurrentSlotDirection &&
			bIsInStationaryCover == NewGearMove.bIsInStationaryCover &&
			Super.CanCombineWith(NewMove,InPawn,MaxDelta));
}
