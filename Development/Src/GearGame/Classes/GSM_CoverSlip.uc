
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_CoverSlip extends GearSpecialMove
	native(SpecialMoves)
	DependsOn(GearPawn);

/** GoW global macros */

/** Body Stances to play */
var()	GearPawn.BodyStance	BS_Cov_Mid_Slip;
var()	GearPawn.BodyStance	BS_Cov_Mid_Slip_Mirrored;
var()	GearPawn.BodyStance	BS_Cov_Std_Slip;
var()	GearPawn.BodyStance	BS_Cov_Std_Slip_Mirrored;

protected function bool InternalCanDoSpecialMove()
{
	local GearPC PC;
	local int SlotIdx;

	// We have to be in cover to do this special move
	if( PawnOwner.CoverType == CT_None ||
		PawnOwner.CurrentLink == None )
	{
		 return FALSE;
	}

	PC = GearPC(PawnOwner.Controller);
	// If this is a player
	if( PC != None )
	{
		// find the closest overlapping slot
		SlotIdx = PawnOwner.PickClosestCoverSlot(TRUE,0.35f);
		if (SlotIdx == -1)
		{
			// not overlapping, no coverslip!
			return FALSE;
		}
		// Make sure they are close enough to the edge to do a slip and allowed to slip in that direction
		if( ( PawnOwner.bIsMirrored && (!PawnOwner.CurrentLink.Slots[SlotIdx].bCanCoverSlip_Left  || !PawnOwner.IsAtCoverEdge(PawnOwner.bIsMirrored,FALSE)))	||
			(!PawnOwner.bIsMirrored && (!PawnOwner.CurrentLink.Slots[SlotIdx].bCanCoverSlip_Right || !PawnOwner.IsAtCoverEdge(PawnOwner.bIsMirrored,FALSE))) )
		{
			return FALSE;
		}
		// Prevent slipping from too far away
		if (VSize(PawnOwner.CurrentLink.GetSlotLocation(SlotIdx) - PawnOwner.Location) > 96.f)
		{
			return FALSE;
		}
	}
	else
	{
		// Otherwise, for AI, make sure this is an actual edge slot
		if( ( PawnOwner.bIsMirrored && (!PawnOwner.CurrentLink.Slots[PawnOwner.CurrentSlotIdx].bCanCoverSlip_Left  || !PawnOwner.CurrentLink.IsLeftEdgeSlot(PawnOwner.CurrentSlotIdx, TRUE))) ||
			(!PawnOwner.bIsMirrored && (!PawnOwner.CurrentLink.Slots[PawnOwner.CurrentSlotIdx].bCanCoverSlip_Right || !PawnOwner.CurrentLink.IsRightEdgeSlot(PawnOwner.CurrentSlotIdx, TRUE))) )
		{
			return FALSE;
		}
	}

	// make sure the player is pressing forward
	if( PC != None && PC.IsLocalPlayerController() )
	{
		// if recently entered use the raw value since that ends up being more intuitive
		if (PawnOwner.TimeSince(PawnOwner.LastCoverTime) < 0.3f)
		{
			if (PC.PlayerInput.RawJoyUp < 0.5f)
			{
				return FALSE;
			}
		}
		else
		{
			// otherwise rely on the remapped value
			if (PC.RemappedJoyUp < 0.5)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// For Heavy Weapons playing a mirror transition, we need the IK bones to be turned off...
	if( PawnOwner.bWantsToBeMirrored && PawnOwner.MirrorNode != None && PawnOwner.MyGearWeapon != None && PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
	{
		PawnOwner.MirrorNode.ForceDrivenNodesOff();
	}
}

/**
 * See if we could play a mirror transition right now.
 * Make sure it won't collide with one playing currently and produce something ugly.
 */
event bool IsMirrorTransitionSafe()
{
	if( IsIKReady() )
	{
		return Super.IsMirrorTransitionSafe();
	}

	return FALSE;
}

/** Called when ready to break from cover. Might be delayed because doing a mirror transition. */
function OnMirrorTransitionSafeNotify()
{
	StartCoverSlip();
}

/**
 * Make sure that IKs are turned off. Only matters for Heavy Weapons.
 * Work around for Heavy Weapons having a different IK chain setup, causing issues when blending with mirrored animations.
 */
function bool IsIKReady()
{
	if( PawnOwner.bWantsToBeMirrored && PawnOwner.MirrorNode != None && PawnOwner.MyGearWeapon != None && PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
	{
		return PawnOwner.MirrorNode.AreDrivenNodesTurnedOff();
	}

	return TRUE;
}

function StartCoverSlip()
{
	local float PlayRate;

	PlayRate = GetSpeedModifier();

	if( PawnOwner.bIsMirrored )
	{
		if( PawnOwner.CoverType == CT_Standing )
		{
			// Play body stance animation.
			PawnOwner.BS_Play(BS_Cov_Std_Slip_Mirrored, PlayRate, 0.1f/PlayRate, 0.2f/PlayRate, FALSE, TRUE);
		}
		else
		{
			// Play body stance animation.
			PawnOwner.BS_Play(BS_Cov_Mid_Slip_Mirrored, PlayRate, 0.1f/PlayRate, 0.2f/PlayRate, FALSE, TRUE);
		}

		// Play animation mirrored
		PawnOwner.BS_SetMirrorOptions(BS_Cov_Std_Slip_Mirrored, FALSE, TRUE, TRUE);

		// Don't trigger the notification early, as we want the IK to be switched back on after the transition animation has disapeared.
		// Heavy Weapons use a different IK setup, and we can't have them on during the transition animation.
		if( PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
		{
			PawnOwner.BS_SetEarlyAnimEndNotify(BS_Cov_Std_Slip_Mirrored, FALSE);
		}
	}
	else
	{
		if( PawnOwner.CoverType == CT_Standing )
		{
			// Play body stance animation.
			PawnOwner.BS_Play(BS_Cov_Std_Slip, PlayRate, 0.1f/PlayRate, 0.2f/PlayRate, FALSE, TRUE);
		}
		else
		{
			// Play body stance animation.
			PawnOwner.BS_Play(BS_Cov_Mid_Slip, PlayRate, 0.1f/PlayRate, 0.2f/PlayRate, FALSE, TRUE);
		}
	}

	// Turn on Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Cov_Std_Slip, RBA_Translate, RBA_Translate, RBA_Translate);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Cov_Std_Slip, TRUE);

	// Turn on Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;

	/** Trigger the cover sltip delegates in the PC */
	if( PCOwner != None )
	{
		PCOwner.TriggerGearEventDelegates( eGED_CoverSlip );
	}

	PawnOwner.SoundGroup.PlayFoleySound(PawnOwner, GearFoley_Body_CoverSlip);

}

/**
 * On server only, possibly save chained move
 * @Return TRUE if move could be chained to this one
 */
function bool CanChainMove(ESpecialMove NextMove)
{
	return (NextMove == SM_RoadieRun || NextMove == SM_Run2MidCov || NextMove == SM_Run2StdCov);
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearPC PC;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Clear timer to be safe
	PawnOwner.ClearTimer('CheckIKReady', Self);

	// Turn off mirroring
	PawnOwner.BS_SetMirrorOptions(BS_Cov_Std_Slip_Mirrored, FALSE, FALSE, FALSE);

	// Disable end of animation notification.
	PawnOwner.BS_SetAnimEndNotify(BS_Cov_Std_Slip, FALSE);

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Cov_Std_Slip, RBA_Discard, RBA_Discard, RBA_Discard);

	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Ignore;

	// Set maximum velocity on Pawn, so he keep on moving for a little while.
	PawnOwner.Velocity		= PawnOwner.Mesh.RootMotionVelocity;
	PawnOwner.Acceleration	= Normal(PawnOwner.Velocity) * PawnOwner.AccelRate;

	// check for a transition to roadie run
	PC = GearPC(PawnOwner.Controller);
	if( PC != None )
	{
		if ( PC.IsHoldingRoadieRunButton() )
		{
			if ( PC.CanDoSpecialMove(SM_RoadieRun) )
			{
				// delay for a second to allow the slip to completely finish
				PawnOwner.SetTimer( 0.01f,FALSE,nameof(PawnOwner.QueueRoadieRunMove) );
			}
			else
			{
				// force a release of the button since we're acquiring cover
				PC.ForceButtonRelease(PC.bUseAlternateControls?GB_X:GB_A,TRUE);
			}
		}
	}
}

function bool CanOverrideSpecialMove(ESpecialMove InMove)
{
	return InMove == SM_CoverRun || Super.CanOverrideSpecialMove(InMove);
}

defaultproperties
{
	BS_Cov_Mid_Slip=(AnimName[BS_FullBody]="AR_Cov_Mid_Slip")
	BS_Cov_Mid_Slip_Mirrored=(AnimName[BS_FullBody]="AR_Cov_Mid_Slip_Mirrored")
	BS_Cov_Std_Slip=(AnimName[BS_FullBody]="AR_Cov_Std_Slip")
	BS_Cov_Std_Slip_Mirrored=(AnimName[BS_FullBody]="AR_Cov_Std_Slip_Mirrored")

	bCoverExitMirrorTransition=TRUE
	bMirrorTransitionSafeNotify=TRUE
	bCanFireWeapon=TRUE
	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableMovement=TRUE

	Action={(
			 ActionName=CoverSlip,
			 ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=212,V=314,UL=35,VL=43))), // A Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=107,V=0,UL=129,VL=125)))	),
			)}
}
