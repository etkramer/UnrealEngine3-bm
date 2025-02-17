
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_MantleUpBase extends GearSpecialMove
	config(Pawn)
	abstract;

/** GoW global macros */

/** Mantle up animation */
var()	GearPawn.BodyStance	BS_MantleUp, BS_MantleUpMirrored;

/** This is the angle for which we scan for cover nodes to then check for mantling up capability **/
var config float CheckForAutoMantleUpFOV;

protected function bool InternalCanDoSpecialMove()
{
	local GearPC		PC;

	if( PawnOwner.CurrentLink == None ||
		PawnOwner.LeftSlotIdx == -1 ||
		PawnOwner.RightSlotIdx == -1 )
	{
		return FALSE;
	}


	if( PawnOwner.TimeSince(PawnOwner.LastCoverTime) < 0.25f || PawnOwner.IsDoingSpecialMove(SM_MantleUpLowCover) || (PCOwner != none && PCOwner.bBreakFromCover))
	{
		return FALSE;
	}

	PC = GearPC(PawnOwner.Controller);
	if( PC != None && PC.IsLocalPlayerController() )
	{
		if( Abs(PC.RemappedJoyUp) < Abs(PC.RemappedJoyRight) || PC.RemappedJoyUp <= PC.DeadZoneThreshold )
		{
			return FALSE;
		}
	}

	// On a single slot
	if( PawnOwner.IsOnACoverSlot() )
	{
		return	PawnOwner.CurrentSlotIdx <= PawnOwner.CurrentLink.Slots.Length &&
				PawnOwner.CurrentLink.Slots[PawnOwner.CurrentSlotIdx].bCanClimbUp &&
				PawnOwner.CurrentLink.Slots[PawnOwner.CurrentSlotIdx].bCanPopUp &&
				!PawnOwner.CurrentLink.Slots[PawnOwner.CurrentSlotIdx].bCanMantle;
	}

	// in between 2 slots
	return PawnOwner.CurrentLink.Slots.Length >= PawnOwner.LeftSlotIdx
		&& PawnOwner.CurrentLink.Slots.Length >= PawnOwner.RightSlotIdx
		&& PawnOwner.CurrentLink.Slots[PawnOwner.LeftSlotIdx].bCanClimbUp
		&& PawnOwner.CurrentLink.Slots[PawnOwner.RightSlotIdx].bCanClimbUp
		&& PawnOwner.CurrentLink.Slots[PawnOwner.LeftSlotIdx].bCanPopUp
		&& PawnOwner.CurrentLink.Slots[PawnOwner.RightSlotIdx].bCanPopUp
		&& !PawnOwner.CurrentLink.Slots[PawnOwner.LeftSlotIdx].bCanMantle
		&& !PawnOwner.CurrentLink.Slots[PawnOwner.RightSlotIdx].bCanMantle;
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Disable steps smoothing
	PawnOwner.bCanDoStepsSmoothing = FALSE;

	// Play body stance animation.
	if( PawnOwner.bIsMirrored )
	{
		PawnOwner.BS_Play(BS_MantleUpMirrored, SpeedModifier, 0.1f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

		// Play animation mirrored
		PawnOwner.BS_SetMirrorOptions(BS_MantleUpMirrored, FALSE, TRUE, TRUE);

		// Don't trigger the notification early, as we want the IK to be switched back on after the transition animation has disapeared.
		// Heavy Weapons use a different IK setup, and we can't have them on during the transition animation.
		if( PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
		{
			PawnOwner.BS_SetEarlyAnimEndNotify(BS_MantleUpMirrored, FALSE);
		}
	}
	else
	{
		PawnOwner.BS_Play(BS_MantleUp, SpeedModifier, 0.1f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);
	}

	// Turn on Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_MantleUp, RBA_Translate, RBA_Translate, RBA_Translate);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_MantleUp, TRUE);

	if(PCOwner != none)
	{
		PCOwner.LeaveCover();
	}

	// Turn off collision for flappy bits when mantling up
	if( PawnOwner.Mesh.PhysicsAssetInstance != None )
	{
		PawnOwner.Mesh.PhysicsAssetInstance.SetFullAnimWeightBlockRigidBody(FALSE, PawnOwner.Mesh);
	}

	// Turn on Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;
	PawnOwner.SetPhysics(PHYS_Flying);
	PawnOwner.SetBase( PawnOwner.ClampedBase );

	PawnOwner.SoundGroup.PlayEffort(PawnOwner, GearEffort_MantleEffort, true);
	PawnOwner.SoundGroup.PlayFoleySound(PawnOwner, GearFoley_Body_ClimbUp);
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearPC	PC;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Re-enable steps smoothing
	PawnOwner.bCanDoStepsSmoothing = PawnOwner.default.bCanDoStepsSmoothing;

	// Turn off mirroring
	PawnOwner.BS_SetMirrorOptions(BS_MantleUpMirrored, FALSE, FALSE, FALSE);

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_MantleUpMirrored, RBA_Discard, RBA_Discard, RBA_Discard);

	PawnOwner.BS_SetAnimEndNotify(BS_MantleUp, FALSE);

	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = PawnOwner.Mesh.default.RootMotionMode;
	PawnOwner.SetPhysics(Phys_Falling);
	PawnOwner.SetBase( PawnOwner.ClampedBase );

	// Turn on collision again for flappy bits when mantling up
	if( PawnOwner.Mesh.PhysicsAssetInstance != None )
	{
		PawnOwner.Mesh.PhysicsAssetInstance.SetFullAnimWeightBlockRigidBody(TRUE, PawnOwner.Mesh);
	}

	// check for a transition to roadie run
	PC = GearPC(PawnOwner.Controller);
	if( PC != None && NextMove == SM_None )
	{
		if( PC.IsHoldingRoadieRunButton() )
		{
			if( PC.CanDoSpecialMove(SM_RoadieRun) )
			{
				PawnOwner.QueueRoadieRunMove(FALSE);
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
	BS_MantleUp=(AnimName[BS_FullBody]="AR_Mantle_Up")
	BS_MantleUpMirrored=(AnimName[BS_FullBody]="AR_Mantle_Up_Mirrored")

	bCoverExitMirrorTransition=TRUE
	bCanFireWeapon=FALSE
	bLockPawnRotation=TRUE
	bBreakFromCoverOnEnd=TRUE
	bDisableCollision=TRUE
	bDisableMovement=TRUE

	Action={(
			 ActionName=MantleUp,
			 ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=212,V=314,UL=35,VL=43))), // A Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=380,V=140,UL=131,VL=98)))	),
			)}
}
