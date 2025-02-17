
/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GSM_LadderClimbDown extends GearSpecialMove;

/** Animation to play */
var()	GearPawn.BodyStance			BS_Animation;
var		vector						LadderEntryLoc;
var		rotator						LadderEntryRot;
var		Array<CameraBoneAnimation>	LadderCameraBoneAnims;


protected function bool InternalCanDoSpecialMove()
{
	local GearPawn P;

	if( PawnOwner == None || PawnOwner.CoverType != CT_None || PawnOwner.LadderTrigger == None || !PawnOwner.LadderTrigger.bCollideActors || !IsRightLadderTrigger() || !Super.InternalCanDoSpecialMove() )
	{
		return FALSE;
	}

	// Location for entry point of ladder
	GetLadderEntryPoint(LadderEntryLoc, LadderEntryRot);

	// Pawn has to be looking at ladder.
	if( Vector(PawnOwner.Rotation) dot Vector(LadderEntryRot) < 0.1f )
	{
		return FALSE;
	}

	// Don't allow climbing ladder another pawn is already climbing
	foreach PawnOwner.WorldInfo.AllPawns(class'GearPawn', P)
	{
		if ( P != PawnOwner && P.LadderTrigger != None && P.LadderTrigger.LadderSMActor == PawnOwner.LadderTrigger.LadderSMActor &&
			(P.IsDoingSpecialMove(SM_LadderClimbDown) || P.IsDoingSpecialMove(SM_LadderClimbUp)) )
		{
			return FALSE;
		}
	}

	return TRUE;
}


simulated function GetLadderEntryPoint(out Vector Out_EntryLoc, out Rotator Out_EntryRot)
{
	PawnOwner.LadderTrigger.GetTopEntryPoint(Out_EntryLoc, Out_EntryRot);
}

simulated function bool IsRightLadderTrigger()
{
	return PawnOwner.LadderTrigger.bIsTopOfLadder;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Location for entry point of ladder
	GetLadderEntryPoint(LadderEntryLoc, LadderEntryRot);

	SetReachPreciseDestination(LadderEntryLoc);
	SetFacePreciseRotation(LadderEntryRot, 0.2f);

	PawnOwner.SetTimer( 2.f, FALSE, nameof(self.ReachLadderTimeOut), self );

	// Trigger the ladder climb down delegates in the GearPC
	if ( PCOwner != None )
	{
		PCOwner.TriggerGearEventDelegates( eGED_ClimbDownLadder );

		PawnOwner.LadderTrigger.UsedBy( PawnOwner );
	}

	// if carrying a heavy weapon, drop it
	if( PawnOwner.WorldInfo.NetMode != NM_Client )
	{
		if( PawnOwner.IsCarryingAHeavyWeapon() )
		{
			PawnOwner.ThrowActiveWeapon();
		}
	}
}

simulated function ReachLadderTimeOut()
{
	PawnOwner.EndSpecialMove();
}

simulated function CheckOverrideAnim()
{
	if (PawnOwner.LadderTrigger.ClimbDownAnim != '')
	{
		BS_Animation.AnimName[BS_FullBody] = PawnOwner.LadderTrigger.ClimbDownAnim;
	}
	else
	{
		BS_Animation.AnimName[BS_FullBody] = default.BS_Animation.AnimName[BS_FullBody];
	}
}

simulated event ReachedPrecisePosition()
{
	local vector	StartLoc;
	local GearPC		PC;

	Super.ReachedPrecisePosition();

	PawnOwner.ClearTimer('ReachLadderTimeOut', Self);

	// check for an override in the trigger
	CheckOverrideAnim();

	// Play body stance animation.
	PawnOwner.BS_Play(BS_Animation, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);

	// Turn on Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Translate, RBA_Translate, RBA_Translate);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, TRUE);

	// Climbing ladder requires RMM_Translate,
	// because other modes lag a frame behind, and it's too noticeable.
	PawnOwner.SetPhysics(Phys_None);
	PawnOwner.Mesh.RootMotionMode = RMM_Translate;
	TogglePawnCollision(PawnOwner, FALSE);

	// Keep Z same has current Pawn location
	StartLoc	= LadderEntryLoc;
	StartLoc.Z	= PawnOwner.Location.Z;

	// Make sure Pawn starts at the right position, for perfect synchronization.
	PawnOwner.SetLocation(StartLoc);
	PawnOwner.SetRotation(LadderEntryRot);

	PC = GearPC(PawnOwner.Controller);
	// Start camera animation
	if( PC != None && LadderCameraBoneAnims.Length > 0 )
	{
		PC.PlayRandomCameraBoneAnim(LadderCameraBoneAnims, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier,,, TRUE, TRUE, TRUE);
	}
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_Animation, RBA_Discard, RBA_Discard, RBA_Discard);

	// Disable end of animation notification.
	PawnOwner.BS_SetAnimEndNotify(BS_Animation, FALSE);

	// Stop animation in case we've been aborted.
	PawnOwner.BS_Stop(BS_Animation, 0.2f/SpeedModifier);

	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = PawnOwner.Mesh.default.RootMotionMode;

	// Reset physics
	PawnOwner.SetPhysics(PHYS_Falling);

	// Restore Pawn collision
	TogglePawnCollision(PawnOwner, TRUE);

	PawnOwner.ClearTimer('ReachLadderTimeOut', Self);
}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="AR_Ladder_Down")

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bLockPawnRotation=TRUE
	bDisableMovement=TRUE
	bDisableLook=TRUE
	bForcePrecisePosition=TRUE

	Action={(
	ActionName=ClimbDown,
	ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
						(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=235,V=367,UL=48,VL=106)))	),
	)}

	LadderCameraBoneAnims(0)=(AnimName="Camera_Ladder_Down")

	bDisablePOIs=TRUE
}
