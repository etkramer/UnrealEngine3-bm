
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_MantleDown extends GearSpecialMove;

/** Min distance required to be able to mantle down */
var() float MinMantleHeight;

/** Max distance possible to mantle down */
var() float MaxMantleHeight;

/** Mantle up animation */
var()	GearPawn.BodyStance	BS_MantleDown;


protected function bool InternalCanDoSpecialMove()
{
	if( PawnOwner.CoverType != CT_None ||
		(PawnOwner.IsDoingASpecialMove() && !PawnOwner.IsDoingMove2IdleTransition() && !PawnOwner.IsDoingSpecialMove(SM_RoadieRun)) )
	{
		return FALSE;
	}

	if(  PawnOwner.IsHumanControlled() &&
		!PawnOwner.CanPerformMantleDown(MinMantleHeight, MaxMantleHeight, PawnOwner.Rotation) )
	{
		return FALSE;
	}

	// don't let them jump down eholes
	if( GearSpawner(PawnOwner.Base) != None )
	{
		return FALSE;
	}

	return TRUE;
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Play body stance animation.
	PawnOwner.BS_Play(BS_MantleDown, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Turn on Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_MantleDown, RBA_Translate, RBA_Translate, RBA_Translate);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_MantleDown, TRUE);

	// Turn on Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
	PawnOwner.SetPhysics(PHYS_Flying);
	PawnOwner.SetBase( PawnOwner.ClampedBase );
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearPC			PC;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_MantleDown, RBA_Discard, RBA_Discard, RBA_Discard);

	PawnOwner.BS_SetAnimEndNotify(BS_MantleDown, FALSE);

	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = PawnOwner.Mesh.default.RootMotionMode;
	PawnOwner.SetPhysics(Phys_Falling);
	PawnOwner.SetBase( PawnOwner.ClampedBase );

	// check for a transition to roadie run
	PC = GearPC(PawnOwner.Controller);
	if( PC != None )
	{
		if ( PC.IsHoldingRoadieRunButton() )
		{
			if ( (PC.bUseAlternateControls || !PC.TryToRunToCover(TRUE,0.75f)) && PC.CanDoSpecialMove(SM_RoadieRun) )
			{
				PC.DoSpecialMove(SM_RoadieRun);
			}
		}
	}
}

/**
 * On server only, possibly save chained move
 * @Return TRUE if move could be chained to this one
 */
function bool CanChainMove(ESpecialMove NextMove)
{
	return (NextMove == SM_RoadieRun);
}

defaultproperties
{
	MinMantleHeight=60.f	// match max step height
	MaxMantleHeight=150.f	// MaxStepHeight + MidLevelCoverHeight (96)

	BS_MantleDown=(AnimName[BS_FullBody]="AR_Mantle_Down")

	bCanFireWeapon=FALSE
	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableCollision=TRUE
	bDisableMovement=TRUE
	bDisablePhysics=FALSE

	Action={(
			 ActionName=MantleDown,
			 ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=212,V=314,UL=35,VL=43))), // A Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=422,V=314,UL=90,VL=106)))	),
			)}
}
