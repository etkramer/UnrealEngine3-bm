
/**
 * DBNO Special Move
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_DBNO extends GearSpecialMove
	native(SpecialMoves);

/** Animations */
var	GearPawn.BodyStance		BS_StumbleAnim;
/** Blend time between dbno idle and guard animations. */
var	float					GuardToggleBlendTime;
var ParticleSystemComponent PSC_DBNO_BloodCrawlSpurt;

/** Location PawnOwner went down. */
var protected transient vector	DBNOStartLocation;
var protected transient bool	bTriggeredCrawlingGUDS;

protected function bool InternalCanDoSpecialMove()
{
	if( PawnOwner.SpecialMove != SM_None && !PawnOwner.IsDoingMove2IdleTransition() && !PawnOwner.IsDoingStumbleFromMelee() )
	{
		return FALSE;
	}

	return TRUE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearGRI	GRI;
	local Rotator	AimRot;

	// Get aim rotation before pulling pawn out of cover 
	AimRot = PawnOwner.GetBaseAimRotation();

	Super.SpecialMoveStarted(bForced, PrevMove);

	// Set the bleed out time
	PawnOwner.TimeOfDBNO = PawnOwner.WorldInfo.TimeSeconds;
	PawnOwner.DBNOTimeExtensionCnt = 0;

	PlayStumbleAnimation();

	// Reset DBNO Guard Blend Node
	if( PawnOwner.DBNOGuardBlendNode != None )
	{
		PawnOwner.DBNOGuardBlendNode.SetBlendTarget(0.f, GuardToggleBlendTime);
	}

	// turn off the near death
	PawnOwner.FadeOutAudioComponent(PawnOwner.NearDeathBreathSound,1.f);
	// everybody gets to hear you breathing hard
	PawnOwner.ReviveBreathSound = PawnOwner.SoundGroup.PlayEffortEx(PawnOwner, GearEffort_DeathBreathe);
	
	if( PawnOwner.WorldInfo.GRI.ShouldShowGore() )
	{
		GRI = GearGRI(PawnOwner.WorldInfo.GRI);
		if( GRI.IsEffectRelevant( PawnOwner, PawnOwner.Location, 3000, FALSE, GRI.CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
		{
			PSC_DBNO_BloodCrawlSpurt = GRI.GOP.GetImpactParticleSystemComponent( PawnOwner.DBNO_BloodCrawlSpurtTemplate );
			PawnOwner.Mesh.AttachComponent(PSC_DBNO_BloodCrawlSpurt, PawnOwner.MeleeDamageBoneName);
			PSC_DBNO_BloodCrawlSpurt.SetLODLevel(GRI.GetLODLevelToUse( PawnOwner.DBNO_BloodCrawlSpurtTemplate, PawnOwner.Location ));
			PSC_DBNO_BloodCrawlSpurt.ActivateSystem();
		}

		PawnOwner.StartBloodTrail('SpawnABloodTrail_DBNO');
	}

	// Owning client gets some more goodness
	if( PCOwner != None )
	{
		// Reset our active reload level
		PawnOwner.ActiveReload_CurrTier = 0;
		// Send PC controller to proper state
		PCOwner.GoToReviving();

		if( PCOwner.IsLocalPlayerController() )
		{
			PawnOwner.PlaySound(SoundCue'Interface_Audio.Interface.DeathModeEnter01Cue', true);
			PawnOwner.ReviveHeartbeatSound = PawnOwner.SoundGroup.PlayFoleySoundEx(PawnOwner, GearFoley_SlowHeartbeat);
			PawnOwner.FlatLiningSound = PawnOwner.SoundGroup.PlayFoleySoundEx(PawnOwner, GearFoley_RevivingFlatliningSound);

			// scale the camera color based on the revival time
			//GearPlayerCamera(PCOwner.PlayerCamera).SetDesiredColorScale(vect(0.8f,0.3f,0.3f), GRI.InitialRevivalTime);

			// point camera downward
			GearPlayerInput(PCOwner.PlayerInput).ForcePitchCentering(TRUE, TRUE, -6000);
		}
	}

	// Send AI to proper state.
	if( AIOwner != None )
	{
		AIOwner.GoDBNO( AimRot );
	}

	PawnOwner.AddToLocalDBNOList();
	PawnOwner.bIsCrawling = TRUE;

	DBNOStartLocation = PawnOwner.Location;
	bTriggeredCrawlingGUDS = FALSE;
	
	if (PawnOwner.ControllerWhoPutMeDBNO != None)
	{
		// Notify Kismet of DBNO
		PawnOwner.TriggerEventClass( class'SeqEvt_EnteredRevivalState', PawnOwner.ControllerWhoPutMeDBNO.Pawn, 0 );
	}
}

function PlayStumbleAnimation()
{
	// Play body stance animation.
	PawnOwner.BS_Play(BS_StumbleAnim, SpeedModifier, 0.2f/SpeedModifier, 0.2f/SpeedModifier, FALSE, TRUE);
	// Get notification when stumble animation is done playing
	PawnOwner.BS_SetAnimEndNotify(BS_StumbleAnim, TRUE);
	// Turn on root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_StumbleAnim, RBA_Translate, RBA_Translate, RBA_Translate);
	// Turn on Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = RMM_Accel;

	// Only do this where PawnOwner is locally controller.
	// So we don't get rotation mismatch between client and server.
	if( PawnOwner.IsLocallyControlled() )
	{
		CheckForClipping();
	}
}

native final function CheckForClipping();

function CleanUpStumbleAnimation()
{
	// Turn off Root motion on animation node
	PawnOwner.BS_SetRootBoneAxisOptions(BS_StumbleAnim, RBA_Discard, RBA_Discard, RBA_Discard);
	// Turn off anim end notification
	PawnOwner.BS_SetAnimEndNotify(BS_StumbleAnim, FALSE);
	// Turn off Root motion on mesh.
	PawnOwner.Mesh.RootMotionMode = PawnOwner.default.Mesh.RootMotionMode;
}

function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	// Stumble animation done playing
	if( PawnOwner.BS_SeqNodeBelongsTo(SeqNode, BS_StumbleAnim) )
	{
		CleanUpStumbleAnimation();

		// Pawn is now going to move with Root Motion on.
		PawnOwner.Mesh.RootMotionMode = RMM_Velocity;

		// Stop Pawn
		PawnOwner.Velocity		= Vect(0,0,0);
		PawnOwner.Acceleration	= Vect(0,0,0);

		// Allow player to move at this point.
		//@fixme laurent, need to be able to alter these mid-special move. w/ bLockPawnRotation.
		if( PCOwner != None )
		{
			PCOwner.IgnoreMoveInput(FALSE);
		}

		// Unlock Pawn rotation.
		SetLockPawnRotation(FALSE);

		// Detach weapon when going DBNO
		if( PawnOwner.MyGearWeapon != None )
		{
			PawnOwner.MyGearWeapon.DetachWeapon();
		}
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	// mark that we were DNBO for handling in death
	PawnOwner.bWasDBNO = TRUE;

	// cleanup extra sounds
	PawnOwner.FadeOutAudioComponent(PawnOwner.ReviveBreathSound,0.1f);
	PawnOwner.FadeOutAudioComponent(PawnOwner.FlatLiningSound,0.1f);
	PawnOwner.FadeOutAudioComponent(PawnOwner.ReviveHeartbeatSound,0.1f);

	// turn off and remove the PSC so we are not constantly ticking it
	if( PSC_DBNO_BloodCrawlSpurt != None )
	{
		PSC_DBNO_BloodCrawlSpurt.DeactivateSystem();
		PSC_DBNO_BloodCrawlSpurt = None;
	}

	// Clean up animation if stopped early
	CleanUpStumbleAnimation();

	PawnOwner.StopBloodTrail( 'SpawnABloodTrail_DBNO' );

	/*
	if( PCOwner != None )
	{
		if( PCOwner.IsLocalPlayerController() )
		{
			GearPlayerCamera(PCOwner.PlayerCamera).SetDesiredColorScale(vect(1.0f,1.0f,1.0f),1.0f);
		}
	}
	*/

	// Reset crawling variable.
	PawnOwner.bIsCrawling = PawnOwner.default.bIsCrawling;

	// reset the DBNO time
	PawnOwner.TimeOfDBNO = 0;

	// reattach weapon when leaving DBNO
	if( PawnOwner != None && PawnOwner.MyGearWeapon != None )
	{
		PawnOwner.AttachWeapon();
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

function Tick(float DeltaTime)
{
	// once pawn has crawled a certain distance, trigger event.
	if ( !bTriggeredCrawlingGUDS && (VSize(PawnOwner.Location - DBNOStartLocation) > 128.f) )
	{
		bTriggeredCrawlingGUDS = TRUE;
		if (PawnOwner.WorldInfo.Game != None)
		{
			GearGame(PawnOwner.WorldInfo.Game).TriggerGUDEvent(GUDEvent_DBNOPawnCrawlingAway, PawnOwner);
		}
	}

	super.Tick(DeltaTime);
}

simulated function PreProcessInput(GearPlayerInput Input)
{
	// Slow down turning.
	Input.aTurn	*= 0.33f;
}

defaultproperties
{
	BS_StumbleAnim=(AnimName[BS_FullBody]="AR_Injured_Drop")
	GuardToggleBlendTime=0.33f

	bShouldAbortWeaponReload=TRUE
	bCanFireWeapon=FALSE
	bCameraFocusOnPawn=FALSE
	bLockPawnRotation=TRUE
	bBreakFromCover=TRUE
	bDisableMovement=TRUE

	bConformMeshRotationToFloor=TRUE
	bConformMeshTranslationToFloor=TRUE
}
