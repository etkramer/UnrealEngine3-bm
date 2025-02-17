/**
 * GearVWeap_RocketCannon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearVWeap_ReaverMinigunBase extends GearVehicleWeapon
	abstract
	config(Weapon);

/** How long it takes the barrel to cool down from fully heated, in seconds. */
var() protected const config float	CooldownTime;
/** How long it takes the barrel to cool down from fully heated while ActiveCooling(tm) is in effect, in seconds. */
var() protected const config float	CooldownTimeActiveCooling;

var protected transient bool bActivelyCooling;
var protected const SoundCue ActiveCoolingLoopCue;
var protected const SoundCue ActiveCoolingStartCue;
var protected const SoundCue ActiveCoolingStopCue;
var protected transient AudioComponent	ActiveCoolingSteamAudioLoop;

var config float MaxFiringTimeBeforeOverheat, OverheatPenaltyTime;
var bool bOverheated;

var float TracerAdvance;

simulated function PlayWeaponAnimation(Name AnimName, float Rate, optional bool bLoop, optional SkeletalMeshComponent SkelMesh)
{
}

simulated function bool IsHeatLimited()
{
	return TRUE;
}

simulated function GearProj_BulletTracer GetTracer( vector SpawnLocation, rotator SpawnRotation )
{
	local GearProj_BulletTracer NewTracer;

	//`log("RMG INST"@Instigator);
	NewTracer = GearGRI(WorldInfo.GRI).GOP.GetTracer( TracerType, 0, SpawnLocation + (Instigator.Velocity * TracerAdvance), SpawnRotation );
	NewTracer.bAllowTracersMovingFromTarget = TRUE;

	return NewTracer;
}

simulated function GearProj_BulletTracer SpawnTracerEffect( vector HitLocation, float HitDistance )
{
	local GearProj_BulletTracer NewTracer;

	NewTracer = Super.SpawnTracerEffect(HitLocation, HitDistance);
	//NewTracer.Velocity += 1000.0 * vector(NewTracer.Rotation);

	return NewTracer;
}

simulated state Active
{
	/** Override BeginFire so we can stop it from firing if unpossessed */
	simulated function BeginFire(byte FireModeNum)
	{
		if ( (Instigator.Controller != None || bDummyFireWeapon) && !bOverheated )
		{
			Super.BeginFire(FireModeNum);
		}
	}

	simulated function Tick(float DeltaTime)
	{
		local GearPC GPC;
		local GearPlayerInput GPI;
		local bool bR1Pressed;

		// poll input directly, for more robustness with with respect to when the 
		// R1 button goes up/down

		GPC = (Instigator != None) ? GearPC(Instigator.Controller) : None;
		GPI = (GPC != None) ? GearPlayerInput(GPC.PlayerInput) : None;

		if (GPI != None)
		{
			bR1Pressed = GPI.IsButtonActive(GB_RightBumper);
			if (bActivelyCooling && !bR1Pressed)
			{
				EndActiveCooling();
			}
			else if ( !bActivelyCooling && bR1Pressed && (CurrentBarrelHeat > 0.f) )
			{
				BeginActiveCooling();
			}
		}

		Global.Tick(DeltaTime);
	}
}

simulated protected function BeginActiveCooling()
{
	bActivelyCooling = TRUE;

	// adjust to the AC spindown rate
//	CurrentBarrelRotAccel = CurrentBarrelRotRate / SpinDownTime_ActiveCooling;

	ActiveCoolingSteamAudioLoop = GearWeaponPlaySoundLocalEx(ActiveCoolingLoopCue,, ActiveCoolingSteamAudioLoop, 0.3f);
	WeaponPlaySound(ActiveCoolingStartCue);
}

simulated protected function OverheatPenaltyExpired()
{
	bOverheated = FALSE;
}

simulated protected function EndActiveCooling()
{
	if (bActivelyCooling)
	{
		// turn off hiss sound
		if (ActiveCoolingSteamAudioLoop != None)
		{
			ActiveCoolingSteamAudioLoop.FadeOut(0.3f, 0.f);
			ActiveCoolingSteamAudioLoop = None;

			WeaponPlaySound(ActiveCoolingStopCue);
		}
		// done
		bActivelyCooling = FALSE;
	}
}

simulated function Tick(float DeltaTime)
{
	Super.Tick(DeltaTime);
	// keep track of firing duration and barrel heat
	if (IsFiring())
	{
		CurrentBarrelHeat += DeltaTime / MaxFiringTimeBeforeOverheat;
		CurrentBarrelHeat = FMin(CurrentBarrelHeat, 1.f);

		if ( (CurrentBarrelHeat >= 1.f) && !bOverheated )
		{
			// overheated!  stop firing and don't allow a restart for some time
			bOverheated = TRUE;
			SetTimer( OverheatPenaltyTime,, nameof(OverheatPenaltyExpired) );
			//WeaponPlaySound(FireStopCue_Overheat);
			StopFire(0);

			GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_GatlingOverheated, Instigator, None, 0.35f);
		}
	}
	else
	{
		if (bActivelyCooling)
		{
			CurrentBarrelHeat -= DeltaTime / CooldownTimeActiveCooling;

			if (CurrentBarrelHeat <= 0.f)
			{
				EndActiveCooling();
			}
		}
		else
		{
			CurrentBarrelHeat -= DeltaTime / CooldownTime;
		}
		CurrentBarrelHeat = FMax(CurrentBarrelHeat, 0.f);
	}
}

simulated function PlayShellCaseEject()
{
}


simulated function PlayMuzzleFlashEffect()
{
}

simulated function bool HasInfiniteSpareAmmo()
{
	return TRUE;
}

simulated function bool HasAmmo( byte FireModeNum, optional int Amount )
{
	return TRUE;
}

simulated function float GetWeaponDistanceForEnemyTrace()
{
	return 30000.0f;
}

defaultproperties
{
	bWeaponCanBeReloaded=TRUE
	bIsSuppressive=TRUE

	TracerSmokeTrailEffectAR=None
	TracerSmokeTrailEffect=None

	bAllowTracers=TRUE
	bSuppressTracers=FALSE
	TracerType=WTT_Reaver

	WeaponFireTypes(0)=EWFT_InstantHit
	InstantHitDamageTypes(0)=class'GDT_ReaverMinigun'
	InstantHitDamage(0)=50
	InstantHitMomentum(0)=1
	bInstantHit=true

	bIgnoreDifficultyDamageScale=TRUE

	FireOffset=(X=68,Y=0,Z=9)

	// Muzzle Flash point light
	Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(B=35,G=185,R=255,A=255)
	End Object
	MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.4f
	MuzzleLightPulseFreq=10
	MuzzleLightPulseExp=1.5

	//LC_EmisDefaultCOG=(R=0.5,G=3.0,B=20.0,A=1.0)
	//LC_EmisDefaultLocust=(R=3.0,G=3.0,B=3.0,A=1.0)

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.600)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

	WeaponID=WC_Troika

	MaxFinalAimAdjustment=0.25

	bOverrideDistanceForEnemyTrace=TRUE

	TracerAdvance=0.033
}
