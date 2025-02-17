
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_StdLvlSwatTurn extends GearSpecialMove
	native(SpecialMoves);

/** SWAT Turn animations */
var const	GearPawn.BodyStance	BS_Std_Start;
var const	GearPawn.BodyStance	BS_Std_End;
var const	GearPawn.BodyStance	BS_Mid_Start;
var const	GearPawn.BodyStance	BS_Mid_End;

/** Body Stance that is actually being played. */
var			GearPawn.BodyStance	BS_PlayedStance;

/** Distance travelled by the end animation. */
var	const	float				EndTurnAnimDist;
var			bool				bPlayedEndAnim;

/** SWAT Turn Target cover info */
var			CovPosInfo			SWATTurnTargetCovInfo;
var			BasedPosition		SwatTurnDestination;

/** Scale Animations independantly from movement speed */
var	config	float				AnimRateScale;

var CoverInfo					DestSlotInfo;

// C++ functions
cpptext
{
	virtual void TickSpecialMove(FLOAT DeltaTime);
}

function bool CanChainMove(ESpecialMove NextMove)
{
	return (NextMove == SM_CoverSlip || NextMove == SM_MidLvlJumpOver);
}

protected function bool InternalCanDoSpecialMove()
{
	local CoverReference	Info;
	local GearPC			PC;
	local vector			CamLoc;
	local rotator			CamRot;
	local int				CurrentSlotIdx;
	local GearPawn			P;

	if( !PawnOwner.bCanSwatTurn || !PawnOwner.IsInCover())
	{
		return FALSE;
	}

	PC = GearPC(PawnOwner.Controller);
	// If this is a player
	if( PC != None )
	{
		CurrentSlotIdx = PawnOwner.PickClosestCoverSlot(TRUE);
		if (CurrentSlotIdx == -1)
		{
			//`log(GetFuncName()@self@"FAILED because PickClosestCoverSlot returned -1");
			return FALSE;
		}
		// make sure the player is pressing to the side (except if auto swatturning from coverrun)
		if( PC.IsLocalPlayerController() && PawnOwner.SpecialMove != SM_CoverRun )
		{
			// If facing left, but not pushing left OR
			// If facing right, but not pushing right OR
			// pushing more up than sideways
			if( Abs(PC.RemappedJoyRight) < PC.DeadZoneThreshold ||
				( PawnOwner.bIsMirrored && PC.RemappedJoyRight > PC.DeadZoneThreshold) ||
				(!PawnOwner.bIsMirrored && PC.RemappedJoyRight < -PC.DeadZoneThreshold) ||
				(Abs(PC.RemappedJoyUp) > Abs(PC.RemappedJoyRight)) )
			{
				// FAIL
				//`log(GetFuncName()@self@"FAILED because player not pressing left/right");
				return FALSE;
			}

			// make sure they're facing forward, otherwise fall through to an evade
			PC.GetPlayerViewPoint(CamLoc,CamRot);
			// eliminate pitch, only care about yaw
			CamRot.Pitch = PawnOwner.Rotation.Pitch;
			if (vector(CamRot) dot vector(PawnOwner.Rotation) < 0.85f)
			{
				//`log(GetFuncName()@self@"FAILED because not facing forward");
				return FALSE;
			}
		}


		// If not swat turn target found OR
		// target isn't a valid claim
		if( !PawnOwner.CurrentLink.GetSwatTurnTarget(CurrentSlotIdx, (PawnOwner.bIsMirrored?-1:1), Info ) ||
			!CoverLink(Info.Actor).IsValidClaim(PawnOwner, Info.SlotIdx ) )
		{
			// FAIL
			//`log(GetFuncName()@self@"FAILED because no swat turn target or claim didn't work target:"@PawnOwner.CurrentLink.GetSwatTurnTarget(CurrentSlotIdx, (PawnOwner.bIsMirrored?-1:1), Info )@"ValidClaim:"@CoverLink(Info.Actor).IsValidClaim(PawnOwner, Info.SlotIdx ));
			return FALSE;
		}

		// make sure there are no obstructions
		if (!PawnOwner.FastTrace(PawnOwner.Location,CoverLink(Info.Actor).GetSlotLocation(Info.SlotIdx)))
		{
			//`log(GetFuncName()@self@"FAILED because trace failed between self and "$Info.Actor$": Slot#"@Info.SlotIdx);
			return FALSE;
		}

		// check for any pawns standing near the target location
		foreach PawnOwner.WorldInfo.AllPawns(class'GearPawn',P,CoverLink(Info.Actor).GetSlotLocation(Info.SlotIdx),PawnOwner.GetCollisionRadius())
		{
			if (P != PawnOwner && P.Health > 0)
			{
				return FALSE;
			}
		}


		// Make sure they are close enough to the edge to do a swat turn and allowed to swat turn in that direction
		if( ( PawnOwner.bIsMirrored && (!PawnOwner.CurrentLink.Slots[PawnOwner.CurrentSlotIdx].bCanSwatTurn_Left  || !PawnOwner.IsAtLeftEdgeSlot()))  ||
			(!PawnOwner.bIsMirrored && (!PawnOwner.CurrentLink.Slots[PawnOwner.CurrentSlotIdx].bCanSwatTurn_Right || !PawnOwner.IsAtRightEdgeSlot())) )
		{
			// FAIL
			//`log(GetFuncName()@self@"FAILED because not close enough to the edge IsAtLeft?"@PawnOwner.IsAtLeftEdgeSlot()@"IsAtRight?"@PawnOwner.IsAtRightEdgeSlot()@"CanTurnLeft?"@PawnOwner.CurrentLink.Slots[PawnOwner.CurrentSlotIdx].bCanSwatTurn_Left@"CanTurnRight?"@PawnOwner.CurrentLink.Slots[PawnOwner.CurrentSlotIdx].bCanSwatTurn_Right);
			return FALSE;
		}
	}
	else
	{
		// Otherwise, for AI, make sure this is an actual edge slot
		if( ( PawnOwner.bIsMirrored && (!PawnOwner.CurrentLink.Slots[PawnOwner.CurrentSlotIdx].bCanSwatTurn_Left  || !PawnOwner.CurrentLink.IsLeftEdgeSlot(PawnOwner.CurrentSlotIdx, TRUE))) ||
			(!PawnOwner.bIsMirrored && (!PawnOwner.CurrentLink.Slots[PawnOwner.CurrentSlotIdx].bCanSwatTurn_Right || !PawnOwner.CurrentLink.IsRightEdgeSlot(PawnOwner.CurrentSlotIdx, TRUE))) )
		{
			return FALSE;
		}
	}

	// don't allow if carrying a heavy weapon
	if( PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
	{
		//`log(GetFuncName()@self@"FAILED because we're carrying a heavy weapon:"$PawnOwner.MyGearWeapon);
		return FALSE;
	}

	return TRUE;
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local CoverReference SlotRef;
	local CoverInfo SlotInfo;
	local vector	Destination;
	local NavigationPoint Nav;
	local int CurrentSlotIdx;

	if (PawnOwner.CurrentLink == None)
	{
		`LogSMExt(PawnOwner,"Invalid current link in swat turn, aborting");
		PawnOwner.EndSpecialMove(SM_StdLvlSwatTurn);
		return;
	}

	CurrentSlotIdx = PawnOwner.PickClosestCoverSlot(TRUE);
	if (CurrentSlotIdx == -1)
	{
		CurrentSlotIdx = PawnOwner.CurrentSlotIdx;
	}

	// Unclaim old cover
	PawnOwner.CurrentLink.UnClaim(PawnOwner, PawnOwner.CurrentSlotIdx, TRUE);

	Super.SpecialMoveStarted(bForced,PrevMove);

	bPlayedEndAnim = FALSE;

	// Play start animation
	BS_PlayedStance = PawnOwner.CoverType == CT_Standing ? BS_Std_Start : BS_Mid_Start;
	PawnOwner.BS_Play(BS_PlayedStance, AnimRateScale, 0.1f/AnimRateScale, 0.15f/AnimRateScale, FALSE, TRUE);

	// Get SWAT Turn target SlotInfo
	if ( !PawnOwner.CurrentLink.GetSwatTurnTarget(CurrentSlotIdx, (PawnOwner.bIsMirrored?-1:1), SlotRef) &&
		PawnOwner.WorldInfo.NetMode == NM_Client && !PawnOwner.IsLocallyControlled() )
	{
		// if it fails on the client, try the reverse direction (mirroring might not have replicated yet)
		PawnOwner.CurrentLink.GetSwatTurnTarget(CurrentSlotIdx, PawnOwner.bIsMirrored ? 1 : -1, SlotRef);
	}
	Nav = NavigationPoint(SlotRef.Actor);
	SlotInfo.Link = CoverLink(Nav);
	SlotInfo.SlotIdx = SlotRef.SlotIdx;
	DestSlotInfo=SlotInfo;

	// Turn it into a CovPosInfo, so we can find out all about this cover location
	PawnOwner.SetCoverInfoFromSlotInfo(SWATTurnTargetCovInfo, SlotInfo);

	PawnOwner.SetAnchor(SlotInfo.Link.Slots[SlotInfo.SlotIdx].SlotMarker);

	Destination = SWATTurnTargetCovInfo.Location + (SWATTurnTargetCovInfo.Normal * PawnOwner.GetCollisionRadius());
	class'Controller'.static.SetBasedPosition(SwatTurnDestination, Destination, PawnOwner.Base);

	if( PawnOwner.Controller != None )
	{
		// Send player to cover location
		PawnOwner.Controller.SetDestinationPosition( Destination, TRUE );
		PawnOwner.Controller.bPreciseDestination	= TRUE;
	}

	// Acquire SWAT Turn cover
	PawnOwner.SetCovPosInfo(SWATTurnTargetCovInfo, false);
	PawnOwner.SetCoverAction(CA_Default);

	// Claim new cover
	SlotInfo.Link.Claim(PawnOwner, SlotInfo.SlotIdx);

	/** Trigger the swat turn delegates in the PC */
	if ( PCOwner != None )
	{
		PCOwner.TriggerGearEventDelegates( eGED_SwatTurn );
	}
}


simulated event PlayEndAnimation(float DistanceFromTarget)
{
	local float PlayLength, ratio;

	// effort here too?
	PawnOwner.SoundGroup.PlayFoleySound(PawnOwner, GearFoley_Body_SwatTurn);

	// Update cover type
	PawnOwner.SetCurrentCoverType();


	// Play the end animation
	// Don't blend it out automatically, because the player is moving, so it's playing move and move2idle animations
	// in the background. So we delay the blend out to minimize blending to a non idle animation.
	BS_PlayedStance = PawnOwner.CoverType == CT_Standing ? BS_Std_End : BS_Mid_End;
	PlayLength = PawnOwner.BS_Play(BS_PlayedStance, AnimRateScale, 0.1f/AnimRateScale, -1.f, FALSE, TRUE);

	// Adjust position, so animation matches the distance we have left until new cover location
	ratio = (EndTurnAnimDist - DistanceFromTarget) / EndTurnAnimDist;
	PawnOwner.BS_SetPosition(BS_PlayedStance, ratio * PlayLength);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_PlayedStance, TRUE);

	// Turn on Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_PlayedStance, RBA_Translate, RBA_Translate);

	// Turn on Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Velocity;

	// This animation is a mirror transition
	PawnOwner.BS_SetMirrorOptions(BS_PlayedStance, !PawnOwner.bIsMirrored, TRUE, FALSE);

	// Don't trigger the notification early, as we want the IK to be switched back on after the transition animation has disapeared.
	// Heavy Weapons use a different IK setup, and we can't have them on during the transition animation.
	if( PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
	{
		PawnOwner.BS_SetEarlyAnimEndNotify(BS_PlayedStance, FALSE);
	}
}


function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Blend Out animation
	PawnOwner.BS_Stop(BS_PlayedStance, 0.2f/AnimRateScale);

	// Turn off mirroring from end transition
	PawnOwner.BS_SetMirrorOptions(BS_PlayedStance, FALSE, FALSE, FALSE);

	PawnOwner.BS_SetAnimEndNotify(BS_PlayedStance, FALSE);

	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_PlayedStance, RBA_Discard, RBA_Discard, RBA_Discard);

	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Ignore;

	if( PawnOwner.Controller != None )
	{
		// turn off forced movement
		PawnOwner.Controller.bPreciseDestination = FALSE;

		// Prevent the player from doing an early evade
		// Sometimes this happens, when we would like to do another SWAT Turn instead
		// So clear evade timer.
		PawnOwner.Controller.ClearTimer('TryToRoadieRun');
	}

	//`log(GetFuncName()@VSIze(PawnOwner.Location - DestSlotInfo.Link.GetSLotLocation(DestSlotInfo.SlotIdx))@PawnOwner.IsInCover());
	// if we didn't make it to cover, unclaim our destination cover
	if(!PawnOwner.IsInCover() || VSIze(PawnOwner.Location - DestSlotInfo.Link.GetSLotLocation(DestSlotInfo.SlotIdx)) > 50.f)
	{
		// we didn't make it, clear stuff out
		DestSlotInfo.Link.UnClaim(PawnOwner, DestSlotInfo.SlotIdx,TRUE);
		PawnOwner.BreakFromCover();

	}
	// Clear movement, otherwise it triggers a move2idle transition when getting in cover.
	PawnOwner.ZeroMovementVariables();
}


defaultproperties
{
	BS_Std_Start=(AnimName[BS_FullBody]="AR_Cov_Std_SwatTurn_Start")
	BS_Std_End=(AnimName[BS_FullBody]="AR_Cov_Std_SwatTurn_End")
	BS_Mid_Start=(AnimName[BS_FullBody]="AR_Cov_Mid_SwatTurn_Start")
	BS_Mid_End=(AnimName[BS_FullBody]="AR_Cov_Mid_SwatTurn_End")

	EndTurnAnimDist=256.f

	bCoverExitMirrorTransition=TRUE
	bCanFireWeapon=FALSE
	bLockPawnRotation=TRUE
	bDisableMovement=TRUE

	Action={(
	ActionName=SwatTurn,
	ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=212,V=314,UL=35,VL=43))), // A Button
						(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=0,V=235,UL=174,VL=100)))	),
	)}

}
