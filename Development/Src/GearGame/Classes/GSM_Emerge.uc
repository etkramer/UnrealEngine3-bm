
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Emerge extends GearSpecialMove;

var Array<GearPawn.BodyStance>	StanceArray;
var int							VariationIndex;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	VariationIndex	= PawnOwner.SpecialMove - SM_Emerge_Type1;

	// Play body stance animation.
	PawnOwner.BS_Play(StanceArray[VariationIndex], SpeedModifier, 0.0f, 0.2f/SpeedModifier, FALSE, TRUE );

	// Turn on Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(StanceArray[VariationIndex], RBA_Translate, RBA_Translate, RBA_Translate);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(StanceArray[VariationIndex], TRUE);

	// Turn on Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Translate;

	PawnOwner.SetCollision(PawnOwner.bCollideActors, FALSE);
	PawnOwner.bCollideWorld = FALSE;
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(StanceArray[VariationIndex], RBA_Discard, RBA_Discard, RBA_Discard);

	PawnOwner.BS_SetAnimEndNotify(StanceArray[VariationIndex], FALSE);

	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;

	// restore collision
	PawnOwner.SetCollision(PawnOwner.default.bCollideActors, PawnOwner.default.bBlockActors);
	PawnOwner.bCollideWorld = TRUE;

	PawnOwner.FitCollision();
}

defaultproperties
{
	StanceArray(0)=(AnimName[BS_FullBody]="Emerge")
	StanceArray(1)=(AnimName[BS_FullBody]="Emerge")

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bCameraFocusOnPawn=TRUE
	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableCollision=FALSE
	bDisableMovement=TRUE
	bDisablePhysics=TRUE
	bDisableAI=TRUE
}
