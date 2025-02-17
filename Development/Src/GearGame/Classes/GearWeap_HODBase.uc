
/**
 * Hammer of Dawn marker
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_HODBase extends GearWeapon
	abstract
	dependson(HOD_BeamManagerBase);

/** Sound cue to play when targeting beam begins firing */
var()	SoundCue		TargetingBeamFireSound;
/** Looped soundcue to play while beam is active */
var()	SoundCue		TargetingBeamLoopSoundTemplate;
/** Audio component for looped sound */
var		AudioComponent	TargetingBeamLoopSound;
/** How frequently to play the good/bad target feedback beep */
var()	float			TargetingBeamFeedbackTimeSec;

/** Feedback sounds */
var		private SoundCue	TargetingBeamPosFeedbackSound;
var		private SoundCue	TargetingBeamNegFeedbackSound;
var		private SoundCue	TargetingBeamLockedFeedbackSound;

/** True if targeting beam is firing, false otherwise */
var	private bool			bTargetingBeamFiring;

/** Effect indicating that the targeted location is good */
var()	ParticleSystem		PS_ValidTarget;

var	private GearEmitter		ValidTargetEmitter;
var	private ParticleSystemComponent		TargetingBeamEmitter;

/** True if this weapon has a currently valid target, false otherwise */
var private transient bool	bValidTargetEmitterIsOn;

/** Target location last tick */
var private transient vector LastTargetLoc;

/** How long player must target a single point before the main beam can fire, in seconds. */
var() config float HODTargetPaintTime;
/** If TRUE, do not have to hold weapon on one spot to target */
var() config bool bAllowTargetWhileMoving;

/** How long current target has been painted */
var private transient float CurrentTargetPaintTime;

/** How much can the target move without incurring a paint time penalty */
var() private float TargetPaintFreeMovementPerSecond;		// const

/** Keeps valid target emitter from blinking all over place every frame. */
const PaintTimeRequiredToSeeValidTargetEmitter = 0.2f;

/** how much of the invalid target grace period has elapsed */
var private transient float GracePeriodElapsedTime;

/** how long you can invalidate your target and still get it back */
var() private float InvalidTargetGracePeriod;

/** Distance to nudge target location from surface.  Helps hide imperfect collision, deals with 90 deg surfaces, etc */
var() private float NudgeDist;

var transient Actor LastTargetedEnemy;

/** last AI aim rotation, used to keep the AI steady when charging */
var rotator LastAIAim;

/** Battery life remaining, in seconds. */
var() config float	BatteryLifeSec;
/** Amount of battery life used up by initial blast */
var() config float	InitialBlastBatteryCost;

var protected const class<HOD_BeamManagerBase> BeamManagerClass;

replication
{
	if (bNetOwner)
		BatteryLifeSec;
}

simulated function TurnOnTargetingLaser()
{
	bTargetingBeamFiring = TRUE;
	bValidTargetEmitterIsOn = FALSE;			// just started firing, not valid yet
	CurrentTargetPaintTime = 0.f;
	LastTargetedEnemy = None;

	// do audio
	WeaponPlaySound(TargetingBeamFireSound, 1.f);
	if (TargetingBeamLoopSound != None)
	{
		TargetingBeamLoopSound.Play();
	}

	if ( (Instigator == None) || (GearAI(Instigator.Controller) != None) )
	{
		return;
	}

	TargetingBeamEmitter.SetHidden(FALSE);
	TargetingBeamEmitter.ActivateSystem();
}

simulated function TurnOffTargetingLaser()
{
	local HOD_BeamManagerBase BeamMgr;

	bTargetingBeamFiring = FALSE;
	bValidTargetEmitterIsOn = FALSE;

	// stop audio
	if (TargetingBeamLoopSound != None)
	{
		TargetingBeamLoopSound.Stop();
	}

	// stop effects
	if (ValidTargetEmitter != None)
	{
		ValidTargetEmitter.ParticleSystemComponent.DeActivateSystem();
	}
	TargetingBeamEmitter.DeActivateSystem();
	TargetingBeamEmitter.SetHidden(TRUE);

	// tell beam manager I have no good target
	if ( (WorldInfo.NetMode != NM_Client) && !bSlaveWeapon )
	{
		BeamMgr = GearGRI(WorldInfo.GRI).HODBeamManager;
		BeamMgr.NotifyInvalidTarget(Instigator);
		BeamMgr.NotifyStopFire(Instigator);
	}

	MuzFlashEmitter.DeActivateSystem();
}

simulated function Destroyed()
{
	if ( ValidTargetEmitter != None )
	{
		ValidTargetEmitter.Destroy();
		ValidTargetEmitter = none;
	}
	super.Destroyed();
}

simulated private function bool SpawnValidTargetEmitter(vector Loc)
{
	if (Instigator == None )
	{
		if ( ValidTargetEmitter != None )
		{
			ValidTargetEmitter.Destroy();
		}
		return false;
	}
	if ( ValidTargetEmitter != None )
	{
		// one already exists, move to proper loc and carry on
		ValidTargetEmitter.SetLocation(Loc);
		return true;
	}

	ValidTargetEmitter = Spawn(class'SpawnedGearEmitter',,, Loc);
	if ( ValidTargetEmitter == None )
	{
		return false;
	}
	ValidTargetEmitter.LifeSpan = 0;
	ValidTargetEmitter.ParticleSystemComponent.bAutoActivate = false;
	ValidTargetEmitter.SetTemplate(PS_ValidTarget);
	return true;
}

simulated event PostBeginPlay()
{
	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		TargetingBeamEmitter.DeActivateSystem();		// make sure it's off
		SkeletalMeshComponent(Mesh).AttachComponentToSocket( TargetingBeamEmitter, MuzzleSocketName );
	}

	// this begins the feedback heartbeat
	if ( (WorldInfo.NetMode != NM_Client) && !bSlaveWeapon )
	{
		TargetingBeamDoFeedback();
	}

	// here we need to check to see if the GearGRI has a manager, and it is the right class.  if it does not spawn one
	if ( WorldInfo.NetMode != NM_Client && ((GearGRI(WorldInfo.GRI).HODBeamManager == None) || (GearGRI(WorldInfo.GRI).HODBeamManager.Class != BeamManagerClass)) )
	{
		GearGRI(WorldInfo.GRI).HODBeamManager = Spawn(BeamManagerClass);
	}

	super.PostBeginPlay();
}

simulated state Inactive
{
	simulated function Tick(float DeltaTime)
	{
		if (bTargetingBeamFiring)
		{
			TurnOffTargetingLaser();
		}

		super.Tick(DeltaTime);
	}
};

simulated function Tick(float DeltaTime)
{
	local bool bHitSomething, bBeamClearToFire, bClearSky, bUsingGracePeriod, bTargetIsValid, bTargetMovedTooMuch;
	local HOD_BeamManagerBase BeamMgr;
	local ImpactInfo Impact;
	local vector BeamFireLoc;
	local float DistTargetMovedThisTick, FreeMovementDist, PvsPaintTime;
	local float	UpdatePaintTime;
	local GearPawn InstigatorWP;
	local Actor TargetedEnemy;
	local Vector NudgeLocation;

//	`log("hod state"@GetStateName()@bTargetingBeamFiring@BatteryLifeSec);

	if (!bSlaveWeapon && !IsInState('Inactive'))
	{
		InstigatorWP = GearPawn(Instigator);
		BeamMgr = GearGRI(WorldInfo.GRI).HODBeamManager;

		// fire a ray to see what we've hit
		// do this even if targeting beam isnt firing, because effects have fade and
		// can end up pointed wrong
		bHitSomething = GetTraceImpact( Impact, NudgeLocation );

		//debug
//		FlushPersistentDebugLines();
//		DrawDebugSphere( NudgeLocation, NudgeDist, 10, 255, 0, 0, TRUE );
//		bTargetingBeamFiring=TRUE;

		//debug
		`log( self@"Targeting beam hit"@Impact.HitActor, bDebug );

		if( bTargetingBeamFiring )
		{
			// consume battery (i.e. ammo) - only if beam is actually firing
			if( !HasInfiniteSpareAmmo() && BeamMgr.IsBeamFiring(Instigator) && WorldInfo.GRI.IsMultiplayerGame() )
			{
				if (WorldInfo.NetMode != NM_Client)
				{
					BatteryLifeSec -= DeltaTime;
				}
				if( BatteryLifeSec < 0.f )
				{
					StopFire(0);
				}
			}

			PvsPaintTime = CurrentTargetPaintTime;
			if( bHitSomething )
			{
				// targeting laser is pointed at a valid target
				bClearSky = BeamMgr.IsAValidHODTarget(NudgeLocation) || (Impact.HitActor != None && Impact.HitActor.Tag == 'LambentBrumak');

				bUsingGracePeriod = (!bClearSky && (GracePeriodElapsedTime < InvalidTargetGracePeriod));

				// is target valid?
				bTargetIsValid = (bClearSky || bUsingGracePeriod) && !BeamMgr.IsBeamCoolingDown(InstigatorWP);

				UpdatePaintTime = bClearSky ? DeltaTime : -DeltaTime;
				if( bTargetIsValid )
				{
					// is our target an enemy?
					if ( (Impact.HitActor.IsA('GearPawn') && (Impact.HitActor.GetTeamNum() != 0))
						  || Impact.HitActor.IsA('GearSpawner_Seeder')
						  || Impact.HitActor.Tag == 'LambentBrumak' )
					{
						TargetedEnemy = Impact.HitActor;
					}

					// A pawn running across your view doesn't break beam anymore, just updates it
					if( TargetedEnemy != LastTargetedEnemy )
					{
						LastTargetLoc = NudgeLocation;
					}

					if( TargetedEnemy == None && !bAllowTargetWhileMoving )
					{
						// check for too much movement (note this check is skipped when targeting enemies)
						DistTargetMovedThisTick = VSize2D(LastTargetLoc - NudgeLocation);
						FreeMovementDist = TargetPaintFreeMovementPerSecond * DeltaTime;
						if (DistTargetMovedThisTick > FreeMovementDist)
						{
							bTargetMovedTooMuch = TRUE;
						}
					}

					// save last targeted enemy for next time
					LastTargetedEnemy = TargetedEnemy;

					if (bTargetMovedTooMuch)
					{
						// moved too much!  use grace period?
						if (GracePeriodElapsedTime < InvalidTargetGracePeriod)
						{
							bUsingGracePeriod = TRUE;
						}
						else
						{
							bUsingGracePeriod = FALSE;
							CurrentTargetPaintTime = 0.f;
						}
					}
				}

				if (bUsingGracePeriod)
				{
					GracePeriodElapsedTime += DeltaTime;
					UpdatePaintTime = 0.f;
				}
				else if (bClearSky)
				{
					GracePeriodElapsedTime = 0.f;
				}

				// check and see if target is painted and we're ok to fire fire fire
				CurrentTargetPaintTime = FClamp(CurrentTargetPaintTime + UpdatePaintTime, 0.f, HODTargetPaintTime);
				if ( (WorldInfo.NetMode != NM_Client) && ( (CurrentTargetPaintTime >= HODTargetPaintTime) || BeamMgr.IsBeamFiring(InstigatorWP)) )
				{
					bBeamClearToFire = TRUE;
					CurrentTargetPaintTime = HODTargetPaintTime;  // paint stays during actual firing
					BeamFireLoc = Impact.HitLocation;
				}

				if (WorldInfo.NetMode != NM_Client)
				{
					// Fully painted valid target
					if ( (CurrentTargetPaintTime == HODTargetPaintTime) && (PvsPaintTime != HODTargetPaintTime) && bValidTargetEmitterIsOn )
					{
						// audio alert that firing is now available.
						PlaySound(TargetingBeamLockedFeedbackSound,,,, (Instigator != None) ? Instigator.Location : Location);
					}
				}

				//`log("CurrentTargetPaintTime"@CurrentTargetPaintTime@bBeamClearToFire);

				LastTargetLoc = NudgeLocation;

				// Keep track of when last shot was fired.  Normally this happens in Pawn::WeaponFired()
				// but that only gets called once per beam shot, regardless of shot duration.
				if (InstigatorWP != None)
				{
					InstigatorWP.LastWeaponEndFireTime = WorldInfo.TimeSeconds;
				}

				SetFlashLocation(Impact.HitLocation);
			}
			else
			{
				CurrentTargetPaintTime = 0.f;
			}

			if (WorldInfo.NetMode != NM_Client)
			{
				if (bBeamClearToFire && bClearSky)
				{
					BeamMgr.NotifyValidTarget(BeamFireLoc, Instigator);
				}
				else
				{
					BeamMgr.NotifyInvalidTarget(Instigator);
				}
			}
		}
		else
		{
			ClearFlashLocation();
			CurrentTargetPaintTime = 0.f;
			GracePeriodElapsedTime = InvalidTargetGracePeriod;
		}

		if (bHitSomething)
		{
			UpdateFX(Impact.HitLocation, bClearSky, Impact.HitNormal);
		}
		else
		{
			UpdateFX(vect(0,0,0), FALSE);
		}

		// move looping sounds with weapon's owner (since weapon itself doesn't move)
		TargetingBeamLoopSound.Location = Instigator.Location;
	}


	if ( (InstigatorWP != None) && InstigatorWP.bIsTargeting )
	{
		TargetingBeamEmitter.bIgnoreHiddenActorsMembership = TRUE;
	}
	else
	{
		TargetingBeamEmitter.bIgnoreHiddenActorsMembership = FALSE;
	}

	super.Tick(DeltaTime);
}


/** Updates weapon effects for this frame.  If NewHitNormal is not provided, it will be generated and bTargetLocIsValid derived.  */
simulated function UpdateFX(vector NewTargetLoc, optional bool bTargetLocIsValid, optional vector NewHitNormal)
{
	local vector NudgeLocation, MuzzleLoc;
	local rotator TmpRot;
	local HOD_BeamManagerBase BeamMgr;
	local ImpactInfo Impact;

	// get hit normal if one wasn't provided
	// also determine bTargetLocIsValid
	if (IsZero(NewHitNormal))
	{
		if (GetTraceImpact(Impact, NudgeLocation, NewTargetLoc))
		{
			NewHitNormal = Impact.HitNormal;
			BeamMgr = GearGRI(WorldInfo.GRI).HODBeamManager;
			bTargetLocIsValid = BeamMgr.IsAValidHODTarget(NudgeLocation);
		}
	}

	if (bValidTargetEmitterIsOn)
	{
		// transitioned from valid to invalid target, turn psc off
		if ( ((!bSlaveWeapon && CurrentTargetPaintTime <= PaintTimeRequiredToSeeValidTargetEmitter) || !bTargetLocIsValid) && (ValidTargetEmitter != None))
		{
			if ( ValidTargetEmitter != None )
			{
				ValidTargetEmitter.ParticleSystemComponent.DeActivateSystem();
			}
			bValidTargetEmitterIsOn = FALSE;
		}
	}
	else
	{
		// transitioned from invalid to valid target, turn psc on
		if ( (bSlaveWeapon || CurrentTargetPaintTime > PaintTimeRequiredToSeeValidTargetEmitter) && bTargetLocIsValid )
		{
			if ( SpawnValidTargetEmitter(NewTargetLoc) )
			{
				ValidTargetEmitter.ParticleSystemComponent.ActivateSystem();
			}
			bValidTargetEmitterIsOn = TRUE;
		}
	}

	// valid target emitter is still on after updating state, so let's update
	// it's position and orientation
	if ( bValidTargetEmitterIsOn && (ValidTargetEmitter != None) )
	{
		ValidTargetEmitter.SetLocation(NewTargetLoc);
		TmpRot = rotator(NewHitNormal);
		TmpRot.Pitch = NormalizeRotAxis(TmpRot.Pitch - 16384);
		ValidTargetEmitter.SetRotation(TmpRot);
	}

	// targeting beam setup
	// we're doing this even if it's not firing, because the effect has some cooldown time, and we
	// want it to appear attached to the weapon during that period
	if (!IsZero(NewTargetLoc))
	{
		TargetingBeamEmitter.SetVectorParameter('lock_on', NewTargetLoc);
	}
	else
	{
		// just store the endtrace somewhere and use it here, instead of recalculating?
		MuzzleLoc = GetMuzzleLoc();
		TargetingBeamEmitter.SetVectorParameter('lock_on', MuzzleLoc + Vector(GetAdjustedAim(MuzzleLoc)) * GetTraceRange());
	}
}


/**
 * Get Impact info on what the weapon is targeting at.
 * If a hit occurs, the function returns true and fills in Impact.
 * If no hit occurs, the function returns false and Impact is undefined.
 */
simulated function bool GetTraceImpact( out ImpactInfo Impact, out Vector NudgeLocation, optional vector EndTraceOverride )
{
	local vector StartTrace, EndTrace;
	local Array<ImpactInfo>	ImpactList;
	local bool bHit;

	StartTrace	= Instigator.GetWeaponStartTraceLocation();

	if (IsZero(EndTraceOverride))
	{
		EndTrace = StartTrace + Vector(GetAdjustedAim(StartTrace)) * GetTraceRange();
	}
	else
	{
		// push out a little so it hits something
		EndTrace = EndTraceOverride + Normal(EndTraceOverride - StartTrace) * 100.f;
	}
	Impact = CalcWeaponFire(StartTrace, EndTrace, ImpactList);

	bHit = (Impact.HitLocation != EndTrace);
	if( bHit )
	{
		// nudge outward a little bit
		NudgeLocation = Impact.HitLocation + (Impact.HitNormal * NudgeDist);
	}

	return bHit;
}


simulated function bool ShouldDrawCrosshair()
{
	return FALSE;
}


/** Do the MakeNoise for the targeting beam.  In a function like this so we can use it with SetTimer */
function TargetingBeamDoFeedback()
{
	local HOD_BeamManagerBase BeamMgr;
	local GearPawn InstigatorWP;

	if( bTargetingBeamFiring && Instigator != None && Instigator.IsHumanControlled() )
	{
		BeamMgr = GearGRI(WorldInfo.GRI).HODBeamManager;
		InstigatorWP = GearPawn(Instigator);
		if( InstigatorWP != None && BeamMgr != None )
		{
			if( !bValidTargetEmitterIsOn && CurrentTargetPaintTime <= 0.f )
			{

				if( !BeamMgr.IsBeamCoolingDown(InstigatorWP) )
				{
					// negative feedback sound
					PlayerController(Instigator.Controller).ClientPlaySound(TargetingBeamNegFeedbackSound);
				}
			}
			else
			if( bValidTargetEmitterIsOn && !BeamMgr.IsBeamFiring(InstigatorWP) && WorldInfo.NetMode != NM_Client )
			{
				// audio alert that firing is now available.
				PlaySound(TargetingBeamPosFeedbackSound,,,, (Instigator != None) ? Instigator.Location : Location);
			}
		}
	}

	SetTimer( TargetingBeamFeedbackTimeSec, FALSE, nameof(TargetingBeamDoFeedback) );
}

simulated function WeaponFired(byte FiringMode, optional vector HitLocation)
{
	// note this weapon doesn't do typical weapon fire-y things, so we're not going to call the super function
}


simulated function WeaponStoppedFiring(byte FiringMode)
{
	// note this weapon doesn't do typical weapon fire-y things, so we're not going to call the super function

}

simulated function SendToFiringState(byte FireModeNum)
{
	// note this weapon doesn't do typical weapon fire-y things, so we're not going to call the super function
	if (FireModeNum != 0)
	{
		Super.SendToFiringState(FireModeNum);
	}
}

simulated function FlashLocationUpdated(byte FiringMode, vector FlashLocation, bool bViaReplication)
{
	if (WorldInfo.Netmode != NM_Client || bSlaveWeapon || WorldInfo.IsPlayingDemo())
	{
		if (IsZero(FlashLocation))
		{
			TurnOffTargetingLaser();
		}
		else if ( (FiringMode == 0) && HasAmmo(0) )
		{
			if ( !bTargetingBeamFiring )
			{
				TurnOnTargetingLaser();
			}

			if (bViaReplication)
			{
				UpdateFX(FlashLocation);
			}
		}
		TargetingBeamLoopSound.Location = Instigator.Location;
	}
}

simulated function BeginFire(Byte FireModeNum)
{
	local HOD_BeamManagerBase BeamMgr;

	if ( FireModeNum == 0 )
	{
		BeamMgr = GearGRI(WorldInfo.GRI).HODBeamManager;

		if ( (BeamMgr != None) && BeamMgr.bEnabled && HasAmmo(FireModeNum) )
		{
			TurnOnTargetingLaser();

			// @STATS - Hack to record firing stats for the HOD
			if (Role == ROLE_Authority && Instigator != none && Instigator.PlayerReplicationInfo != none)
			{
				WeaponStatIndex = GearPRI(Instigator.PlayerReplicationInfo).AggWeaponFireStat(WeaponID, true, WeaponStatIndex);
			}

		}
		else
		{
			// tried to shoot laser but it was unavailable, play negative feedback sound
			if (WorldInfo.NetMode != NM_Client)
			{
				if (Instigator != None && Instigator.IsHumanControlled())
				{
					PlayerController(Instigator.Controller).ClientPlaySound(TargetingBeamNegFeedbackSound);
				}

				//if ( (BeamMgr != None) && !BeamMgr.bPlayerNotifiedOfDisabledStatus )
				//{
				//	GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_HODDisabled, Instigator);
				//	BeamMgr.bPlayerNotifiedOfDisabledStatus = TRUE;
				//}
			}
		}
	}
	super.BeginFire(FireModeNum);
}

simulated function EndFire(Byte FireModeNum)
{
	local HOD_BeamManagerBase BeamMgr;

	BeamMgr = GearGRI(WorldInfo.GRI).HODBeamManager;
	BeamMgr.NotifyInvalidTarget(Instigator);
	BeamMgr.NotifyStopFire(Instigator);

	TurnOffTargetingLaser();
	super.EndFire(FireModeNum);
}

/** Beam has just fired and done the intial blast radius - update battery life */
function NotifyBeamFired()
{
	if ( WorldInfo.GRI.IsMultiplayerGame() )
	{
		// Take off battery life for initial blast
		BatteryLifeSec -= InitialBlastBatteryCost;
	}
}

simulated function bool HasAmmo( byte FireModeNum, optional int Amount )
{
	// we can always do a melee attack
	return ( (BatteryLifeSec > 0.f) || (FireModeNum == MELEE_ATTACK_FIREMODE) );
}

function int AddAmmo( int Amount )
{
	local float OldBatteryLife;
	if( Amount == 0 )
	{
		Amount = default.BatteryLifeSec;
	}

	OldBatteryLife = BatteryLifeSec;
	BatteryLifeSec = FMin( BatteryLifeSec + Amount, default.BatteryLifeSec );

	return (BatteryLifeSec - OldBatteryLife);
}

simulated function bool CanPickupAmmo()
{
	if( BatteryLifeSec < default.BatteryLifeSec )
	{
		return TRUE;
	}
	return FALSE;
}

simulated function int GetPickupAmmoAmount()
{
	return FFloor(BatteryLifeSec);
}

simulated function int CopyAmmoAmountFromWeapon( GearWeapon InvWeapon )
{
	BatteryLifeSec = GearWeap_HODBase(InvWeapon).BatteryLifeSec;
	return GearWeap_HODBase(InvWeapon).BatteryLifeSec;
}

/** This weapon was denied the opportunity to fire because it could not blind fired.  Give any desired feedback to the player. */
simulated function DoCannotBlindfireFeedback()
{
	if (Instigator != None && Instigator.IsLocallyControlled())
	{
		Instigator.PlaySound(TargetingBeamNegFeedbackSound, true);
	}
	super.DoCannotBlindfireFeedback();
}

simulated function bool IsFiring()
{
	return bTargetingBeamFiring;
}

simulated function PlayNoAmmoFireSound()
{
	// do nothing
}

simulated function ConsumeAmmo( byte FireModeNum )
{
	// do nothing.  weapon doesn't have "ammo" to consume
}

/*
simulated function LinearColor GetWeaponEmisColor_COG()
{
	return WorldInfo.Gears_COGWeapons.HammerOfDawn;
}

simulated function LinearColor GetWeaponEmisColor_Locust()
{
	return WorldInfo.Gears_LocustWeapons.HammerOfDawn;
}
*/

simulated function float GetWeaponRating()
{
	if (!Instigator.IsHumanControlled())
	{
		if ( BatteryLifeSec <= 0.0 || GearAI_TDM(Instigator.Controller) == None || GearGRI(WorldInfo.GRI).HODBeamManager == None ||
			!GearGRI(WorldInfo.GRI).HODBeamManager.bEnabled || !GearAIController.IsStationaryFiring() ||
			GearAIController.FireTarget == None ||
			!GearGRI(WorldInfo.GRI).HODBeamManager.IsAValidHODTarget(GearAIController.FireTarget.Location) )
		{
			return -1.f;
		}
		else
		{
			return (WorldInfo.TimeSeconds - GearAIController.LastShotAtTime > 3.0) ? 1.5 : 0.25;
		}
	}
	else
	{
		return Super.GetWeaponRating();
	}
}

function int GetBurstFireCount()
{
	return -1;
}
function int GetBurstsToFire()
{
	return -1;
}

simulated function rotator GetAdjustedAim(vector StartFireLoc)
{
	// keep the AI aiming at a previous valid location while acquiring
	if ( (GearAIController != None) &&
		 (CurrentTargetPaintTime > 0.0) &&
		 !GearGRI(WorldInfo.GRI).HODBeamManager.IsBeamFiring(Instigator) )
	{
		return LastAIAim;
	}
	else
	{
		LastAIAim = Super.GetAdjustedAim(StartFireLoc);
		return LastAIAim;
	}
}

function rotator GetBaseAIAimRotation(vector StartFireLoc, vector AimLoc)
{
	local vector HitLocation, HitNormal;

	// move the aim location down so that the AI is aiming at the ground as required by the HOD
	if (Trace(HitLocation, HitNormal, AimLoc - vect(0, 0, 1000), AimLoc, false) != None)
	{
		AimLoc = HitLocation;
	}

	return Super.GetBaseAIAimRotation(StartFireLoc, AimLoc);
}

function bool CanHit(vector ViewPt, vector TestLocation, rotator ViewRotation)
{
	// say we can if we're currently targeting and the target location is still close enough to hit the intended spot with splash
	if ( CurrentTargetPaintTime > 0.0 && !GearGRI(WorldInfo.GRI).HODBeamManager.IsBeamFiring(Instigator) &&
		VSize(LastTargetLoc - TestLocation) <= class'HOD_BeamBase'.default.InitialSplashDamageRadius )
	{
		return true;
	}
	else
	{
		return Super.CanHit(ViewPt, TestLocation, ViewRotation);
	}
}

function GetAIAccuracyCone(out vector2D AccCone_Min, out vector2D AccCone_Max)
{
	Super.GetAIAccuracyCone(AccCone_Min, AccCone_Max);

	// while the beam is firing, add some additional inaccuracy so the AI is a little less effective
	// at chasing targets with the beam, particularly at lower levels
	if (GearGRI(WorldInfo.GRI).HODBeamManager.IsBeamFiring(Instigator))
	{
		AccCone_Min.X += 1.0;
		AccCone_Min.Y += 1.0;
	}
}

simulated function bool ShouldRefire()
{
	// make AI stop if HOD is deactivated
	if (AIController != None && (BatteryLifeSec <= 0.0 || !GearGRI(WorldInfo.GRI).HODBeamManager.bEnabled))
	{
		StopFire(CurrentFireMode);
		return false;
	}
	else
	{
		return Super.ShouldRefire();
	}
}

simulated function StartFire(byte FireModeNum)
{
	// don't allow AI to fire while moving around
	if (GearAIController == None || GearAIController.IsStationaryFiring())
	{
		Super.StartFire(FireModeNum);
	}
	else
	{
		GearAIController.SelectWeapon();
	}
}

simulated function bool CanDoSpecialMeleeAttack()
{
	return !bTargetingBeamFiring;
}

simulated function PlayNeedsAmmoChatter()
{
	// disable this for HOD
}

defaultproperties
{
	bSuppressImpactFX=FALSE

	AimAssistScale=0.0
	AimAssistScaleWhileTargeting=0.0

	bUsePreModCameraRotForAiming=TRUE

	Begin Object Class=AudioComponent Name=TargetingBeamLoopSound0
		bUseOwnerLocation=false
	End Object
	TargetingBeamLoopSound=TargetingBeamLoopSound0
	Components.Add(TargetingBeamLoopSound0);

	bBlindFirable=FALSE
	InstantHitDamageTypes(0)=class'GDT_HOD'
	WeaponFireTypes(0)=EWFT_Custom

	// Weapon Mesh
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	// targeting beam emitter
	Begin Object Class=ParticleSystemComponent Name=TBParticleSystemComponent0
		bAutoActivate=FALSE
		TickGroup=TG_PostUpdateWork
	End Object
	TargetingBeamEmitter=TBParticleSystemComponent0


	TargetPaintFreeMovementPerSecond=300

	bSuperSteadyCamWhileTargeting=TRUE

	InvalidTargetGracePeriod=0.25f

	NudgeDist=30

	LC_EmisDefaultCOG=(R=2.0,G=2.0,B=2.0,A=1.0)
	LC_EmisDefaultLocust=(R=50.0,G=2.0,B=0.0,A=1.0)

	WeaponID=WC_HOD

	AIRating=0.9
	bIsSuppressive=false
}
