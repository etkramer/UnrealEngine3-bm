
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_RoadieRun extends GearSpecialMove
	DependsOn(GearPawn)
	native(SpecialMoves);

/** timer before aborting the special move, basically tracks time when velocity is less than 30% of groundspeed */
var transient float RunAbortTimer;

/** refs to currently playing looping sounds */
var array<AudioComponent> LoopingSounds;

// Double click move flags
var					bool		bWasForward;
var					bool		bWasBack;
var					bool		bWasLeft;
var					bool		bWasRight;
var					bool		bEdgeForward;
var					bool		bEdgeBack;
var					bool		bEdgeLeft;
var					bool 		bEdgeRight;

cpptext
{
	virtual void TickSpecialMove(FLOAT DeltaTime);
};

/** Takes into account boost and shield. */
native function float GetSpeedModifier();

/**
* On server only, possibly save chained move
* @Return TRUE if move could be chained to this one
*/
function bool CanChainMove(ESpecialMove NextMove)
{
	return (NextMove == SM_Run2MidCov || NextMove == SM_Run2StdCov);
}

/** Notification called when body stance animation finished playing */
simulated function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	if ( !PawnOwner.IsLocallyControlled() )
	{
		//`log("Don't animend roadierun on non-owning animend!");
		return;
	}
	super.BS_AnimEndNotify(SeqNode, PlayedTime, ExcessTime);
}

protected function bool InternalCanDoSpecialMove()
{
	// don't allow if temporary disabled (covering head)
	if( PawnOwner.bCannotRoadieRun || !PawnOwner.MyGearWeapon.bAllowsRoadieRunning )
	{
		return FALSE;
	}
	
	// don't allow if in cover
	if( PawnOwner.CoverType != CT_None && (PawnOwner.SpecialMove == SM_None || !PawnOwner.SpecialMoves[PawnOwner.SpecialMove].bBreakFromCoverOnEnd) )	
	{
		return FALSE;
	}

	// don't allow if stuck in a walk volume
	if ( WalkVolume(PawnOwner.PhysicsVolume) != None &&
		 WalkVolume(PawnOwner.PhysicsVolume).bActive )
	{
		return FALSE;
	}

	// can't RR in flagged CameraVolumes
	if ( (PawnOwner.CameraVolumes.length > 0) && PawnOwner.CameraVolumes[0].bForcePlayerToWalk )
	{
		return FALSE;
	}

	if (PawnOwner.bIsConversing || PawnOwner.bWantToConverse)
	{
		return FALSE;
	}

	if ( (PawnOwner.Health <= 0) && !PawnOwner.IsDBNO() )
	{
		// could be waiting to go dbno (bDelayedDBNO on server)
		return FALSE;
	}

	return TRUE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local rotator	NewPlayerRot;
	local float		YawDiff, CameraTurnTume, PCOwnert;
	local			GearGameplayCamera Cam;
	local Vector	Direction, X, Y, Z;
	local array<SequenceEvent> TouchEvents;
	local actor		A;
	local int		i;

	Super.SpecialMoveStarted(bForced,PrevMove);

	RunAbortTimer = 0.f;

	// for players, turn to face direction he's trying to move
	if( PCOwner != None )
	{
        // double-check the player is still holding the A button, in cases where the move was chained but the player later released the button
        if (PCOwner.IsLocalPlayerController() && !PCOwner.IsHoldingRoadieRunButton())
        {
            // make sure to call it on the playercontroller so that it's replicated to the server
            PCOwner.EndSpecialMove();
            return;
        }

		GetAxes( PCOwner.Rotation, X, Y, Z );
		Direction = Normal((PCOwner.RemappedJoyUp * X) + (PCOwner.RemappedJoyRight * Y) );

		// @fixme jf, using vel here is suboptimal, cuz it doesn't directly map to
		// player desires (like the stick does).  But using raw stick values doesn't
		// do well in multi, so coming up with something here would be good.
		// That is, if this iteration of roadie run survives. :)
		if( !IsZero(Direction) )
		{
			NewPlayerRot = Rotator(Direction);

			// figure how far we turned so we can turn the camera smoothly into the new orientation
			YawDiff = NormalizeRotAxis(PCOwner.Rotation.Yaw - NewPlayerRot.Yaw);

			// turn the player
			PCOwner.SetRotation( NewPlayerRot );

			// start executing a code-driven camera turn
			Cam = GearPlayerCamera(PCOwner.PlayerCamera).GameplayCam;
			if (Cam != None)
			{
				// some minor gymnastics here to find a function that feels good.
				// we want a snappier turn at 180, but we can tolerate a slower rate
				// closer to 0.  0.9 is the time to rotate at 180, and 0.6 is the
				// time to rotate at 0.
				PCOwnert = Abs(YawDiff) / 32768;
				CameraTurnTume = (0.9f * PCOwnert) + (0.6f * (1.f - PCOwnert));

				Cam.BeginTurn(YawDiff, 0, CameraTurnTume, 0.f);
			}
		}

		if( ( PCOwner.IsLocalPlayerController() == TRUE )
			&& ( (PawnOwner.Health / PawnOwner.DefaultHealth) < 0.40f )
			)
		{
			LoopingSounds[LoopingSounds.Length] = PawnOwner.SoundGroup.PlayFoleySoundEx(PawnOwner, GearFoley_FastHeartbeat, 2.f);
		}
	}

	// if we are not already breathing from being close to death
	if( PawnOwner.NearDeathBreathSound == none )
	{
		LoopingSounds[LoopingSounds.Length] = PawnOwner.SoundGroup.PlayEffortEx(PawnOwner, GearEffort_RoadieRunBreathe, 2.f);
	}

	PawnOwner.StartRoadieRunPSC();

	// Trigger the roadie run delegates in the PC
	if ( PCOwner != None )
	{
		PCOwner.TriggerGearEventDelegates( eGED_RoadieRun );
	}

	foreach PawnOwner.TouchingActors(class'Actor', A)
	{
		if (A.FindEventsOfClass(class'SeqEvt_GearTouch', TouchEvents))
		{
			for (i = 0; i < TouchEvents.length; i++)
			{
				SeqEvt_GearTouch(TouchEvents[i]).CheckRoadieRunTouchActivate(A,PawnOwner,true);
			}
			// clear array for next iteration
			TouchEvents.length = 0;
		}
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local int i;
	local array<SequenceEvent> TouchEvents;
	local actor		A;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	// stop all of the looped sounds
	for( i = 0; i < LoopingSounds.Length; ++i )
	{
		if (LoopingSounds[i] != None)
		{
			LoopingSounds[i].FadeOut(1.f, 0.f);
		}
	}
	LoopingSounds.Length = 0;

	if( PawnOwner.PSC_RoadieRun != none )
	{
		PawnOwner.PSC_RoadieRun.DeActivateSystem();
		PawnOwner.SetTimer( 1.0f, FALSE, nameof(PawnOwner.HideRoadieRunPSC) );
	}

	PawnOwner.RoadieRunBoostTime = -9999.f;

	foreach PawnOwner.TouchingActors(class'Actor', A)
	{
		if (A.FindEventsOfClass(class'SeqEvt_GearTouch', TouchEvents))
		{
			for (i = 0; i < TouchEvents.length; i++)
			{
				SeqEvt_GearTouch(TouchEvents[i]).CheckRoadieRunTouchActivate(A,PawnOwner,false);
			}
			// clear array for next iteration
			TouchEvents.length = 0;
		}
	}
}


simulated function PreProcessInput(GearPlayerInput Input)
{
	// Check for Double click movement.
	// Store before wiping out inputs so that we can detect double clicks for evade during RR
	bEdgeForward	= (bWasForward	^^ (Input.aBaseY  > 0));
	bEdgeBack		= (bWasBack		^^ (Input.aBaseY  < 0));
	bEdgeLeft		= (bWasLeft		^^ (Input.aStrafe < 0));
	bEdgeRight		= (bWasRight	^^ (Input.aStrafe > 0));
	bWasForward		= (Input.aBaseY	 > 0);
	bWasBack		= (Input.aBaseY	 < 0);
	bWasLeft		= (Input.aStrafe < 0);
	bWasRight		= (Input.aStrafe > 0);

	// force running forwards
	Input.aBaseY	= 1.f;

	// strafe becomes turning
	Input.aTurn		= Input.aStrafe * 0.33f;
	Input.aStrafe	= 0.f;
}

simulated function PreDoubleClickCheck( GearPlayerInput_Base Input )
{
	// Restore DClick info for player input to detect
	Input.bWasForward	= bWasForward;
	Input.bWasBack		= bWasBack;
	Input.bWasLeft		= bWasLeft;
	Input.bWasRight		= bWasRight;
	Input.bEdgeForward	= bEdgeForward;
	Input.bEdgeBack		= bEdgeBack;
	Input.bEdgeLeft		= bEdgeLeft;
	Input.bEdgeRight	= bEdgeRight;
}

defaultproperties
{
	bNoStoppingPower=FALSE
	bCanFireWeapon=FALSE

	MaxConformToFloorMeshRotation=10.0
	bConformMeshRotationToFloor=TRUE
	bConformMeshTranslationToFloor=TRUE

}
