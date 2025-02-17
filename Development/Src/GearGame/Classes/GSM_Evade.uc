
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_Evade extends GearSpecialMove;

/** Animations to play */
var()	GearPawn.BodyStance	BS_EvadeStart, BS_EvadeEnd;

/** how much steering is allowed when evading. */
var()	float	EvadeTurnPct;
/** acceleration scale */
var()	float	AccelScale;

/** how much camera rotation is allowed when evading. */
var()	float	CamRotationPct;

/** Flag set when playing start animation */
var		bool	bPlayStartAnimation;

/**
 * This does all of the work for the various Types of Evades (e.g. fwd, bwk, right, left)
 * The testing whether you can do the evade is basically the same for all evade types, so
 * we push the functionality here and then pass in the type of move and do the specific checks
 * needed for that evade.
 */
protected function bool InternalCanDoSpecialMoveEvade_Worker(ESpecialMove TheMove)
{
	local WalkVolume	V;
	local rotator CheckRot;

	// Can't evade in WalkVolumes
	if( PawnOwner.PhysicsVolume != None )
	{
		V = WalkVolume(PawnOwner.PhysicsVolume);
		if( V != None && V.bActive )
		{
			//`log( "InternalCanDoSpecialMove FALSE 0"  );
			return FALSE;
		}
	}

	// Can't if doing a special move other than a move2idle transition
	if( PawnOwner.IsDoingASpecialMove() && !PawnOwner.IsDoingMove2IdleTransition() && !PawnOwner.IsDoingSpecialMove(SM_PushOutOfCover) )
	{
		//`log( "InternalCanDoSpecialMove FALSE 1"  );
		return FALSE;
	}

	// can't evade in flagged CameraVolumes
	if ( (PawnOwner.CameraVolumes.length > 0) && PawnOwner.CameraVolumes[0].bForcePlayerToWalk )
	{
		return FALSE;
	}

	if( PawnOwner.MyGearWeapon.WeaponType == WT_Heavy )
	{
		return FALSE;
	}

	// make sure there is no geometry directly in the way
	CheckRot = PawnOwner.Rotation;
	if (TheMove == SM_EvadeRt)
	{
		CheckRot.Yaw += 16384;
	}
	else
	if (TheMove == SM_EvadeLt)
	{
		CheckRot.Yaw -= 16834;
	}
	else
	if (TheMove == SM_EvadeBwd)
	{
		CheckRot.Yaw -= 32768;
	}
	return (PawnOwner.CoverType == CT_None || PawnOwner.FastTrace(PawnOwner.Location + vector(CheckRot) * 128.f,PawnOwner.Location));
}


function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// if crouching when starting an evade, stop it
	PawnOwner.ShouldCrouch(FALSE);

	PawnOwner.PlaySound( SoundCue'Foley_BodyMoves.BodyMoves.CogBodyLunge_Cue' ); // add to soundgroup?
	PawnOwner.SoundGroup.PlayEffort(PawnOwner, GearEffort_EvadeEffort, true);
	PawnOwner.MakeNoise( 0.75f );

	bPlayStartAnimation = TRUE;

	// Play Evade Start Animation.
	PawnOwner.BS_Play(BS_EvadeStart, SpeedModifier, 0.1f/SpeedModifier, -1.f, FALSE, TRUE);

	// Enable end of animation notification. This is going to call SpecialMoveEnded()
	PawnOwner.BS_SetAnimEndNotify(BS_EvadeStart, TRUE);

	// Turn on Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_EvadeStart, RBA_Translate, RBA_Translate, RBA_Translate);

	PawnOwner.Mesh.RootMotionMode = RMM_Accel;

	/** Trigger the playercontroller delegates for when the player evades */
	if ( PCOwner != None )
	{
		PCOwner.TriggerGearEventDelegates( eGED_Evade );
	}
}


/** Notification called when body stance animation finished playing */
simulated function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	local FLOAT CheckNextMoveTime;

	if( bPlayStartAnimation )
	{
		bPlayStartAnimation = FALSE;

		PawnOwner.BS_Stop(BS_EvadeStart);
		PawnOwner.BS_SetAnimEndNotify(BS_EvadeStart, FALSE);

		// Set Evade End animation.
		CheckNextMoveTime = PawnOwner.BS_Play(BS_EvadeEnd, SpeedModifier, 0.f, -1.f, FALSE, TRUE);
		if( PCOwner != None )
		{
			PawnOwner.SetTimer( CheckNextMoveTime-0.2f, FALSE, nameof(self.CheckTransitionToAnotherMove), self );
		}

		// Set correct position for transition.
		PawnOwner.BS_SetPosition(BS_EvadeEnd, ExcessTime * PawnOwner.BS_GetPlayRate(BS_EvadeEnd));

		// Enable end of animation notification. This is going to call SpecialMoveEnded()
		PawnOwner.BS_SetAnimEndNotify(BS_EvadeEnd, TRUE);

		// Turn on Root motion on animation node
		PawnOwner.BS_SetRootBoneAxisOptions(BS_EvadeEnd, RBA_Translate, RBA_Translate, RBA_Translate);

		// Notification that we play the landed animation
		PawnOwner.PlayEvadeAnimLandedNotify();
	}
	else
	{
		PawnOwner.EndSpecialMove();
	}
}

/**
 * On server only, possibly save chained move
 * @Return TRUE if move could be chained to this one
 */
function bool CanChainMove(ESpecialMove NextMove)
{
	return (NextMove == SM_RoadieRun || NextMove == SM_Run2MidCov || NextMove == SM_Run2StdCov);
}

/**
 * Can a new special move override this one before it is finished?
 * This is only if CanDoSpecialMove() == TRUE && !bForce when starting it.
 */
function bool CanOverrideMoveWith(ESpecialMove NewMove)
{
	return (NewMove == SM_RoadieRun || NewMove == SM_Run2MidCov || NewMove == SM_Run2StdCov);
}

function CheckTransitionToAnotherMove()
{
	if( PCOwner != None && PawnOwner != None && !PawnOwner.bWantToUseCommLink )
	{
		// check to see if we've pressed the 'a' button
		if( LocalPlayer(PCOwner.Player) != None && IsHoldingRoadieRunButton(PCOwner) )
		{
			// either run to cover or start roadie running (if not already waiting for a roadie run)
			if( (PCOwner.bUseAlternateControls || !PCOwner.TryToRunToCover(TRUE,0.75f)) && PCOwner.CanDoSpecialMove(SM_RoadieRun) )
			{
				PawnOwner.QueueRoadieRunMove(FALSE,TRUE);
			}
			else
			{
				// otherwise clear the button so that we don't autoslip/mantle
				PCOwner.ForceButtonRelease(PCOwner.bUseAlternateControls?GB_X:GB_A,TRUE);
			}
		}
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	PawnOwner.ClearTimer('CheckTransitionToAnotherMove', Self);

	// Stop stances
	PawnOwner.BS_Stop(BS_EvadeStart, 0.25f);
	PawnOwner.BS_Stop(BS_EvadeEnd, 0.25f);

	// Disable end of animation notification.
	PawnOwner.BS_SetAnimEndNotify(BS_EvadeStart, FALSE);
	PawnOwner.BS_SetAnimEndNotify(BS_EvadeEnd, FALSE);

	// Restore original root motion
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;

	PawnOwner.LastEvadeTime = PawnOwner.WorldInfo.TimeSeconds;
	PawnOwner.ClearTimer('PlayEvadeCameraShake');

	if( PCOwner != None )
	{
		// notify our playercontroller that the evade has finished
		PCOwner.NotifyEvadeFinished();

		// If we're not transitioning to another move, see if we can do one.
		if( NextMove == SM_None )
		{
			CheckTransitionToAnotherMove();
		}
	}
}

// Only allow roadie run if holding A button or if evaded forward and is still holding forward
simulated function bool IsHoldingRoadieRunButton( GearPC PC )
{
	return (PC.IsButtonActive(PC.bUseAlternateControls?GB_X:GB_A) || (PC.CurrentDoubleClickDir == DCLICK_Forward && PC.PlayerInput.RawJoyUp > 0.f));
}

simulated function PreProcessInput(GearPlayerInput Input)
{
	// when rotating the camera let's slow the rotation down a bit
	Input.aTurn = Input.aTurn * CamRotationPct;
}

defaultproperties
{
	bNoStoppingPower=FALSE
	bBreakFromCover=TRUE
	bCanFireWeapon=FALSE
	bCameraFocusOnPawn=TRUE
	EvadeTurnPct=0.33f
	AccelScale=100.f
	bLockPawnRotation=TRUE
	CamRotationPct=0.5f
}
