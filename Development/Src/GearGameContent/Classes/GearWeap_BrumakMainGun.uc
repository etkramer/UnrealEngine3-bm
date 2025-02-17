/**
 * Brumak Main Gun Weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_BrumakMainGun extends GearWeap_BrumakWeaponBase;

/** Name of sockets for weapon muzzle flash */
var			Name	MainMuzzleSocketName;
/** Name of socket for rocket fire */
var			Name	RocketMuzzleSocketName;
/** Muzzle Flash particle system */
var()	editinline	ParticleSystemComponent		RocketMuzFlashEmitter;
/** Rocket fire soundcue */
var const protected SoundCue RocketFireCue_Player;
/** Brumak roar soundcue */
var const protected SoundCue RocketRoarCue;

/** Number of rockets in a volley */
var() config Byte	RocketVolleyCount;
/** How fast should the rockets accelerate towards it's cross or away */
var() config float	RocketAccelRate;
var() config float	NextPlayerRocketFireTime;

simulated function Vector GetMuzzleLoc()
{
	local Vector	MuzzleLoc;
	local Rotator	MuzzleRot;

	GetMuzzleInfo( MuzzleLoc, MuzzleRot, 0 );
	return MuzzleLoc;
}

simulated function GetMuzzleInfo( out Vector MuzzleLoc, out Rotator MuzzleRot, byte FireMode )
{
	local GearPawn_LocustBrumakBase	Brumak;
	local SkeletalMeshActor		SkelMeshActor;

	MuzzleLoc = Location;
	MuzzleRot = Rotation;

	Brumak = GetBrumak();
	if( Brumak != None && Brumak.Mesh != None )
	{
		if( FireMode == ALTFIRE_FIREMODE )
		{
			Brumak.Mesh.GetSocketWorldLocationAndRotation( RocketMuzzleSocketName, MuzzleLoc, MuzzleRot );
		}
		else
		{
			Brumak.Mesh.GetSocketWorldLocationAndRotation( MuzzleSocketName, MuzzleLoc, MuzzleRot );
		}
	}
	else
	if( bDummyFireWeapon )
	{
		SkelMeshActor = SkeletalMeshActor(DummyFireParent);
		if( SkelMeshActor != None )
		{
			if( FireMode == ALTFIRE_FIREMODE )
			{
				SkelMeshActor.SkeletalMeshComponent.GetSocketWorldLocationAndRotation( RocketMuzzleSocketName, MuzzleLoc, MuzzleRot );
			}
			else
			{
				SkelMeshActor.SkeletalMeshComponent.GetSocketWorldLocationAndRotation( MuzzleSocketName, MuzzleLoc, MuzzleRot );
			}
		}
	}
}

simulated function bool HasAmmo( byte FireModeNum, optional int Amount )
{
	local GearPawn_LocustBrumakBase Brumak;

	if( Instigator.IsHumanControlled() )
	{
		if( FireModeNum == ALTFIRE_FIREMODE )
		{
			Brumak = GetBrumak();
			if( Brumak != None )
			{
				return Brumak.bCanFireRockets;
			}
			return FALSE;
		}
	}

	return TRUE;
}

simulated function ConsumeAmmo( byte FireModeNum )
{
	if( FireModeNum == ALTFIRE_FIREMODE )
	{
		RocketVolleyCount--;
	}
}

simulated function bool ShouldRefire()
{
	if( CurrentFireMode == ALTFIRE_FIREMODE )
	{
		return (RocketVolleyCount>0);
	}
	return Super.ShouldRefire();
}

simulated function bool HandleButtonPress( coerce Name ButtonName )
{
	local GearPC PC;

	PC = GearPC(Instigator.Controller);
	if ( !PC.bCinematicMode &&
		((!PC.bUseAlternateControls && ButtonName == 'X') || (PC.bUseAlternateControls && ButtonName == 'Y')) )
	{
		if( HasAmmo( ALTFIRE_FIREMODE ) )
		{
			StartFire( ALTFIRE_FIREMODE );
		}
		return TRUE;
	}

	return Super.HandleButtonPress( ButtonName );
}

simulated function StartFire( byte FireModeNum )
{
	if( !IsFiring() && !HasAmmo( FireModeNum ) )
		return;

	Super.StartFire( FireModeNum );
}

function SetEnableRocketFire( bool bEnable )
{
	local GearPawn_LocustBrumakBase Brumak;

	Brumak = GetBrumak();
	if( Brumak != None )
	{
		Brumak.bCanFireRockets = bEnable;
	}
}

function AllowRocketFire()
{
	SetEnableRocketFire( TRUE );
}

simulated state RocketsFiring extends WeaponFiring
{
	simulated event BeginState( Name PreviousStateName )
	{
		SetEnableRocketFire( FALSE );
		RocketVolleyCount = default.RocketVolleyCount;
		Super.BeginState( PreviousStateName );
	}

	simulated event EndState( Name NextStateName )
	{
		StopFire( CurrentFireMode );
		SetTimer( NextPlayerRocketFireTime, FALSE, nameof(AllowRocketFire) );

		Super.EndState( NextStateName );
	}
}

simulated function float GetFireInterval( byte FireModeNum )
{
	if( FireModeNum == ALTFIRE_FIREMODE )
	{
		return 0.15f;
	}
	return Super.GetFireInterval( FireModeNum );
}

simulated function GetProjectileFirePosition(out vector out_ProjLoc, out vector out_ProjDir)
{
	local Rotator Rot;
	GetMuzzleInfo( out_ProjLoc, Rot, CurrentFireMode );
	out_ProjDir = Vector(Rot);
}

simulated function Projectile ProjectileFire()
{
	local GearProj_BrumakRocket Proj;
	local Vector StartLocation;
	local rotator AimRot;
	local GearAI_BrumakDriver AI;
	local GearPawn_LocustBrumakBase Brumak;
	local GearPC			PC;

	Brumak	= GetBrumak();
	AI		= GearAI_BrumakDriver(Instigator.Controller);
	PC		= GetPlayerOwner();

	Proj = GearProj_BrumakRocket( Super.ProjectileFire() );
	if (Proj != None)
	{
		Proj.Instigator = (PC != None) ? PC.Pawn : Brumak;
		if (AI != None || PC != None)
		{
			GetMuzzleInfo( StartLocation, AimRot, CurrentFireMode );
			if( AI != None )
			{
				Proj.SetupProjectile( self, AI, Brumak, StartLocation, AimRot, RocketAccelRate );
			}
			else if( PC != None )
			{
				Proj.SetupProjectile( self, PC, Brumak, StartLocation, AimRot, RocketAccelRate, (RocketVolleyCount==default.RocketVolleyCount-1) );
			}
		}
	}

	return Proj;
}

simulated event Projectile ProjectileFireSimple( optional float AimErrorDeg )
{
	local GearProj_BrumakRocket Proj;
	local Vector AccelAdjustment, StartLocation, CrossVec;
	local rotator AimRot;
	local Vector LockedTargetVect, AimDir;
	local Projectile SpawnedProjectile;
	local float AimErrorUnr;

	// fire a projectile without an instigator.  nothing fancy, just straight
	if( WorldInfo.Netmode != NM_Client )
	{
		AimRot = Rotation;
		GetMuzzleInfo( StartLocation, AimRot, CurrentFireMode );

		// modify Aim to add player aim error
		// and convert aim error from degrees to Unreal Units ( * 65536 / 360 )
		if (AimErrorDeg != 0.f)
		{
			AimErrorUnr = AimErrorDeg * 182.044;
			AimRot.Pitch += AimErrorUnr * (0.5 - FRand());
			AimRot.Yaw	 += AimErrorUnr * (0.5 - FRand());
		}
		AimDir = vector(AimRot);
		SpawnedProjectile = Spawn(GetProjectileClass(), self,, StartLocation,,, TRUE);
		if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
		{
			SpawnedProjectile.Init( AimDir );
			GearProjectile(SpawnedProjectile).bSuppressAudio = bSuppressAudio;
		}

		Proj = GearProj_BrumakRocket( SpawnedProjectile );
		Proj.Instigator = None;
		if( Proj != None )
		{
			AimRot = Rotation;

			Proj.Target		 = DummyFireTargetLoc;
			Proj.Target.X	+= RandRange( -128.f, 128.f );
			Proj.Target.Y	+= RandRange( -128.f, 128.f );
			LockedTargetVect = Proj.Target;

			// Set their initial velocity
			CrossVec	= Vect(0,0,1);
			CrossVec   *= (FRand()>0.5f ? 1 : -1);

			AccelAdjustment = (vector(AimRot) Cross CrossVec) * RocketAccelRate;
			AccelAdjustment.Z += ((300.0 * FRand()) - 100.0) * ( FRand() * 2.f);

			Proj.KillRange			= 1024;
			Proj.bFinalTarget		= false;
			Proj.SecondTarget		= LockedTargetVect;
			Proj.SwitchTargetTime	= 0.5;

			Proj.ArmMissile(AccelAdjustment, AimDir * (700 + VSize(DummyFireParent.Velocity)) );
		}
	}

	return SpawnedProjectile;
}

simulated event AttachMuzzleEffectsComponents(SkeletalMeshComponent SkelMesh)
{
	if( Instigator != None )
	{
		// attach muzzle flash emitter, and position to fire offset
		if( MuzFlashEmitter != None && !SkelMesh.IsComponentAttached(MuzFlashEmitter) )
		{
			MuzFlashEmitter.SetScale( Instigator.DrawScale );
			SkelMesh.AttachComponentToSocket(MuzFlashEmitter, MuzzleSocketName);
			HideMuzzleFlashEmitter();
		}

		if( RocketMuzFlashEmitter != None && !SkelMesh.IsComponentAttached(RocketMuzFlashEmitter) )
		{
			RocketMuzFlashEmitter.SetScale( Instigator.DrawScale );
			SkelMesh.AttachComponentToSocket(RocketMuzFlashEmitter, MuzzleSocketName);
			HideMuzzleFlashEmitter();
		}

		// Muzzle Flash dynamic light
		if( MuzzleFlashLight != None && !SkelMesh.IsComponentAttached(MuzzleFlashLight) )
		{
			SkelMesh.AttachComponentToSocket(MuzzleFlashLight, MuzzleSocketName);
		}
	}
	else
	{
		// attach muzzle flash emitter, and position to fire offset
		if( MuzFlashEmitter != None )
		{
			AttachComponent(MuzFlashEmitter);
			HideMuzzleFlashEmitter();
		}

		if( RocketMuzFlashEmitter != None )
		{
			AttachComponent(RocketMuzFlashEmitter);
			HideMuzzleFlashEmitter();
		}

		// Muzzle Flash dynamic light
		if( MuzzleFlashLight != None )
		{
			AttachComponent(MuzzleFlashLight);
		}
	}
}

simulated function WeaponFired(byte FiringMode, optional vector HitLocation)
{
	local GearPawn_LocustBrumakBase Brumak;

	Brumak = GetBrumak();

	// Start firing animation for machine gun
	if( Brumak != None && FiringMode == 0 )
	{
		if( Brumak.MainGunFireAdditiveBlend.Child2WeightTarget != 1.f )
		{
			Brumak.MainGunFireAdditiveBlend.SetBlendTarget(1.f, 0.f);
		}
		//`log(WorldInfo.TimeSeconds @ class @ GetFuncName() @ "MainGunFireAdditiveBlend ON");
	}

	Super.WeaponFired(FiringMode, HitLocation);
}

simulated function WeaponStoppedFiring(byte FiringMode)
{
	local GearPawn_LocustBrumakBase Brumak;

	Brumak = GetBrumak();

	// Start firing animation for machine gun
	if( Brumak != None && FiringMode == 0 )
	{
		if( Brumak.MainGunFireAdditiveBlend.Child2WeightTarget != 0.f )
		{
			Brumak.MainGunFireAdditiveBlend.SetBlendTarget(0.f, 0.15f);
		}
		//`log(WorldInfo.TimeSeconds @ class @ GetFuncName() @ "MainGunFireAdditiveBlend OFF");
	}

	Super.WeaponStoppedFiring(FiringMode);
}

simulated protected function PlayFireSound()
{
	local GearPawn_LocustBrumakBase Brumak;

	if( CurrentFireMode == ALTFIRE_FIREMODE )
	{
		GearWeaponPlaySoundLocal( RocketFireCue_Player, RocketFireCue_Player,, 1.f );
		Brumak = GetBrumak();
		if( Brumak.IsHumanControlled() && FRand() < 0.3f )
		{
			SetTimer( 0.75f, FALSE, 'PlayRoarSound' );
		}
	}
	else
	{
		Super.PlayFireSound();
	}
}

function PlayRoarSound()
{
	if( Role == ROLE_Authority )
	{
		GetBrumak().PlaySound( RocketRoarCue );
	}
}

simulated function PlayMuzzleFlashEffect()
{
	if( !bSuppressMuzzleFlash && CurrentFireMode == ALTFIRE_FIREMODE )
	{
		ClearTimer( 'HideMuzzleFlashEmitter' );
		RocketMuzFlashEmitter.SetHidden( FALSE );
		RocketMuzFlashEmitter.ActivateSystem();

		if( MuzzleFlashLight != None && !GearGRI(WorldInfo.GRI).ShouldDisableEffectsDueToFramerate(Instigator) && (!bLoopMuzzleFlashLight || !MuzzleFlashLight.bEnabled) )
		{
			MuzzleFlashLight.CastDynamicShadows = bDynamicMuzzleFlashes;
			MuzzleFlashLight.SetEnabled(TRUE);

			// Reset Pulse time to zero, so we start at the beginning.
			MuzzleLightPulseTime = 0.f;
		}

		SetTimer( MuzzleLightDuration, FALSE, nameof(StopMuzzleFlashEffect) );
	}
	else
	{
		Super.PlayMuzzleFlashEffect();
	}
}

simulated function StopMuzzleFlashEffect()
{
	if( MuzFlashEmitter != None )
	{
		MuzFlashEmitter.SetActive(FALSE);
		SetTimer( TimeToHideMuzzleFlashPS, FALSE, nameof(HideMuzzleFlashEmitter) );
	}

	if( RocketMuzFlashEmitter != None )
	{
		RocketMuzFlashEmitter.SetActive(FALSE);
		SetTimer( TimeToHideMuzzleFlashPS, FALSE, nameof(HideMuzzleFlashEmitter) );
	}

	// turn off muzzle flash light only if it's non looping
	// Looping ones are turned off when the weapon stops firing
	// Used by rapid fire weapons
	if( !bLoopMuzzleFlashLight && MuzzleFlashLight != None )
	{
		MuzzleFlashLight.SetEnabled( FALSE );
	}
}

simulated function HideMuzzleFlashEmitter()
{
	MuzFlashEmitter.SetHidden( TRUE );
	RocketMuzFlashEmitter.SetHidden( TRUE );
}

defaultproperties
{
	AmmoTypeClass=class'GearAmmoType_AssaultRifle'
	bInfiniteSpareAmmo=TRUE
	bWeaponCanBeReloaded=TRUE
	bCanThrow=FALSE

	MuzzleSocketName=MainMuzzle
	RocketMuzzleSocketName=RocketMuzzle

	// MAIN FIREMODE
	FiringStatesArray(0)="WarmingUp"
	InstantHitDamageTypes(0)=class'GDT_BrumakBullet'
	FireInterval(0)=0.1

	// ROCKET FIREMODE
	FiringStatesArray(ALTFIRE_FIREMODE)="RocketsFiring"
	WeaponFireTypes(ALTFIRE_FIREMODE)=EWFT_Projectile
	InstantHitDamageTypes(ALTFIRE_FIREMODE)=class'GDT_BrumakCannon'
	InstantHitMomentum(ALTFIRE_FIREMODE)=1
	FireInterval(ALTFIRE_FIREMODE)=1.5
	WeaponProjectiles(ALTFIRE_FIREMODE)=class'GearProj_BrumakRocket'

	FireOffset=(X=0,Y=0,Z=0)

//@reimport
	WeaponWhipSound=SoundCue'Weapon_AssaultRifle.Firing.CogRifleWhipCue'
//	PS_DefaultImpactEffect=ParticleSystem'Locust_Brumak.Effects.P_Geist_Brumak_SideGun_Impact'

	Mesh=None
	DroppedPickupMesh=None

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
	    LightColor=(R=255,G=192,B=128,A=255)
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.05f
	MuzzleLightPulseFreq=60
	MuzzleLightPulseExp=1.5
	bLoopMuzzleFlashLight=TRUE

	// MF flash
	Begin Object Class=ParticleSystemComponent Name=PSC_Muzzle0
		bAutoActivate=FALSE
		Template=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Gun_MuzzleFlash'
	End Object
	MuzFlashEmitter=PSC_Muzzle0

	Begin Object Class=ParticleSystemComponent Name=PSC_Muzzle1
		bAutoActivate=FALSE
		Template=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Rocket_MuzzleFlash'
	End Object
	RocketMuzFlashEmitter=PSC_Muzzle1

	TracerType=WTT_Brumak

	bKillDuringLevelTransition=TRUE
	bInstantHit=TRUE

	PutDownTime=0.0

	FireLoopCue=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_FireLoopCue_Cue'
	FireLoopCue_Player=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_FireLoopCue_Cue'
	FireStopCue=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_FireStopCue_Cue'
	FireStopCue_Player=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_FireStopCue_Cue'
	BarrelSpinningStartCue=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_AmmoFeederStartCue_Cue'
	BarrelSpinningStopCue=SoundCue'Locust_Brumak_Efforts2.Weapon.BrumakTroika_AmmoFeederStopCue_Cue'
	RocketFireCue_Player=SoundCue'Locust_Brumak_Efforts2.Brumak.Brumak_RocketFire_Cue'
	RocketRoarCue=SoundCue'Locust_Brumak_Efforts2.Brumak.Brumak_RoarAfterFire_Cue'

	bNewFiringAttack=TRUE
}
