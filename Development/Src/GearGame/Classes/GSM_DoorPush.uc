
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_DoorPush extends GSM_BasePlaySingleAnim
	DependsOn(GearPawn);

/** GoW global macros */

/** The special move ID for this move.  Used by GearPawn::OnDoorPush. */
var		ESpecialMove				SpecialMoveID;

protected function bool InternalCanDoSpecialMove()
{
	return TRUE;
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local float		YawDiff, CameraTurnTime, Pct;
	local GearGameplayCamera Cam;
	local rotator	NewPlayerRot;
	local GearPC		PC;
	local Trigger_DoorInteraction Door;

	Super.SpecialMoveStarted(bForced,PrevMove);

	// for players, turn to face direction of the door
	PC = GearPC(PawnOwner.Controller);
	if( PC != None && PC.DoorTriggerDatum.DoorTrigger != None )
	{
		Door = PC.DoorTriggerDatum.DoorTrigger;
		// use the align object if possible
		if (Door.AlignToObj != None)
		{
			NewPlayerRot = Door.AlignToObj.Rotation;
		}
		else
		{
`if(`notdefined(FINAL_RELEASE))
			PC.ClientMessage("WARNING: DOOR DOES NOT HAVE ALIGNTOOBJ SET PROPERLY");
`endif
			NewPlayerRot = Rotator(PC.DoorTriggerDatum.DoorTrigger.Location - PawnOwner.Location);
		}
		NewPlayerRot.Pitch = 0;

		// figure how far we turned so we can turn the camera smoothly into the new orientation
		YawDiff = NormalizeRotAxis(PC.Rotation.Yaw - NewPlayerRot.Yaw);

		// turn the player
		PC.SetRotation( NewPlayerRot );

		// start executing a code-driven camera turn
		Cam = GearPlayerCamera(PC.PlayerCamera).GameplayCam;
		if (Cam != None)
		{
			// some minor gymnastics here to find a function that feels good.
			// we want a snappier turn at 180, but we can tolerate a slower rate
			// closer to 0.  0.7 is the time to rotate at 180, and 0.35 is the
			// time to rotate at 0.
			Pct = Abs(YawDiff) / 32768;
			CameraTurnTime = (0.7f * Pct) + (0.35f * (1.f - Pct));

			Cam.BeginTurn(YawDiff, 0.f, CameraTurnTime);
		}
	}

	// Set weapon up
	PawnOwner.SetWeaponAlert(PawnOwner.AimTimeAfterFiring*2.f);

	// Trigger the door kick delegates in the GearPC
	if ( PCOwner != None )
	{
		PCOwner.TriggerGearEventDelegates( eGED_KickDoor );
	}
}

function PlayAnimation()
{
	if( PawnOwner.MyGearWeapon != None && PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
	{
		PawnOwner.SetTimer(0.25.f, FALSE, 'EndSpecialMove', Self);
	}
	else
	{
		Super.PlayAnimation();
	}
}

function EndSpecialMove()
{
	PawnOwner.EndSpecialMove();
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Discard, RBA_Discard, RBA_Discard);

	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = PawnOwner.Mesh.default.RootMotionMode;

	if (PawnOwner.Controller != None)
	{
		// kismet action may have turned this on via AlignTo parameter
		PawnOwner.Controller.bPreciseDestination	= FALSE;
		PawnOwner.Velocity = vect(0,0,0);
	}
}


defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="door_nudge")

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bLockPawnRotation=TRUE
	bDisableMovement=TRUE
	bDisableLook=TRUE
	bDisablePhysics=true

	SpecialMoveID=SM_DoorPush
}
