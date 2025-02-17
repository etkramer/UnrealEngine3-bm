/**
 * Brumak Side Gun Weapon for player driven brumak
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_BrumakSideGun_Player extends GearWeap_BrumakSideGun
	hidedropdown;

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

simulated function bool HandleButtonPress( coerce Name ButtonName )
{
	local GearPawn_LocustBrumakBase Brumak;
	local GearPC PC;

	Brumak = GetBrumak();
	// co-op player gets control if he's here
	// need to do some extra checks to make sure he didn't leave in the middle
	if (Brumak != None && (Brumak.HumanGunner == None || Brumak.HumanGunner.bDeleteMe || !Brumak.HumanGunner.IsHumanControlled()))
	{
		PC = GearPC(Brumak.Controller);
		if ( !PC.bCinematicMode &&
			((!PC.bUseAlternateControls && ButtonName == 'X') || (PC.bUseAlternateControls && ButtonName == 'Y')) )
		{
			if( HasAmmo( ALTFIRE_FIREMODE ) )
			{
				StartFire( ALTFIRE_FIREMODE );
			}
			return TRUE;
		}
	}

	return Super.HandleButtonPress( ButtonName );
}

/** Fire twice, once for each gun */
simulated function InstantFire()
{
	Super.InstantFire();

	bHandleLeftGun = TRUE;
	Super.InstantFire();
	bHandleLeftGun = FALSE;
}


simulated function CalcRemoteImpactEffects( byte FireModeNum, vector GivenHitLocation, bool bViaReplication )
{
	Super.CalcRemoteImpactEffects( FireModeNum, GivenHitLocation, bViaReplication );

	bHandleLeftGun = TRUE;
	Super.CalcRemoteImpactEffects( FireModeNum, GivenHitLocation, bViaReplication );
	bHandleLeftGun = FALSE;
}

simulated function bool IsHeatLimited()
{
	return TRUE;
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
	local GearPawn_LocustBrumakBase Brumak;
	local GearPC			PC;

	Brumak	= GetBrumak();
	PC		= GetPlayerOwner();

	Proj = GearProj_BrumakRocket( Super.ProjectileFire() );
	Proj.Instigator = (PC != None) ? PC.Pawn : Brumak;
	if( Proj != None && PC != None )
	{
		GetMuzzleInfo( StartLocation, AimRot, CurrentFireMode );
		Proj.SetupProjectile( self, PC, Brumak, StartLocation, AimRot, RocketAccelRate, (RocketVolleyCount==default.RocketVolleyCount-1) );
	}

	return Proj;
}
simulated event AttachMuzzleEffectsComponents(SkeletalMeshComponent SkelMesh)
{
	// we allowed attachment on dedicated servers, but we don't need stuff after this
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Attach muzzle flash related effects
	// if we are pending kill then don't attach things
	if( SkelMesh == None || SkelMesh.IsPendingKill() )
	{
		//ScriptTrace();
		return;
	}

	if( RocketMuzFlashEmitter != None && !SkelMesh.IsComponentAttached(RocketMuzFlashEmitter) )
	{
		RocketMuzFlashEmitter.SetScale( Instigator.DrawScale );
		SkelMesh.AttachComponentToSocket(RocketMuzFlashEmitter, RocketMuzzleSocketName);
		HideMuzzleFlashEmitter();
	}

	// attach muzzle flash emitter, and position to fire offset
	if( MuzFlashEmitter != None && !SkelMesh.IsComponentAttached(MuzFlashEmitter) )
	{
		MuzFlashEmitter.SetScale( Instigator.Base.DrawScale );
		SkelMesh.AttachComponentToSocket(MuzFlashEmitter, GetMuzzleSocketName(FALSE));
		HideMuzzleFlashEmitter();
	}
	if( MuzFlashEmitterLeft != None && !SkelMesh.IsComponentAttached(MuzFlashEmitterLeft) )
	{
		MuzFlashEmitterLeft.SetScale( Instigator.Base.DrawScale );
		SkelMesh.AttachComponentToSocket(MuzFlashEmitterLeft, GetMuzzleSocketName(TRUE));
		HideMuzzleFlashEmitter();
	}

	// Muzzle Flash dynamic light
	if( MuzzleFlashLight != None && !SkelMesh.IsComponentAttached(MuzzleFlashLight) )
	{
		SkelMesh.AttachComponentToSocket(MuzzleFlashLight, GetMuzzleSocketName(FALSE));
	}
	if( MuzzleFlashLightLeft != None && !SkelMesh.IsComponentAttached(MuzzleFlashLightLeft) )
	{
		SkelMesh.AttachComponentToSocket(MuzzleFlashLightLeft, GetMuzzleSocketName(TRUE));
	}
}

simulated protected function PlayFireSound()
{
	if( CurrentFireMode == ALTFIRE_FIREMODE )
	{
		GearWeaponPlaySoundLocal( None, RocketFireCue_Player,, 1.f );

		if( FRand() < 0.3f )
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
		Instigator.PlaySound( RocketRoarCue );
	}
}

simulated function PlayMuzzleFlashEffect()
{
	if( !bSuppressMuzzleFlash && CurrentFireMode == ALTFIRE_FIREMODE )
	{
		ClearTimer( 'HideMuzzleFlashEmitter' );
		RocketMuzFlashEmitter.SetHidden( FALSE );
		RocketMuzFlashEmitter.ActivateSystem();

		SetTimer( MuzzleLightDuration, FALSE, nameof(StopMuzzleFlashEffect) );
	}
	else
	{
		Super.PlayMuzzleFlashEffect();
	}
}

simulated function StopMuzzleFlashEffect()
{
	Super.StopMuzzleFlashEffect();

	if( RocketMuzFlashEmitter != None )
	{
		RocketMuzFlashEmitter.SetActive(FALSE);
		SetTimer( TimeToHideMuzzleFlashPS, FALSE, nameof(HideMuzzleFlashEmitter) );
	}
}

simulated function HideMuzzleFlashEmitter()
{
	Super.HideMuzzleFlashEmitter();
	RocketMuzFlashEmitter.SetHidden( TRUE );
}

simulated function Vector GetMuzzleLoc()
{
	local Vector	MuzzleLoc;
	local Rotator	MuzzleRot;

	GetMuzzleInfo( MuzzleLoc, MuzzleRot, 0 );
	return MuzzleLoc;
}

simulated function GetMuzzleInfo( out Vector MuzzleLoc, out Rotator MuzzleRot, byte FireMode )
{
	local SkeletalMeshComponent PawnMesh;

	MuzzleLoc = Location;
	MuzzleRot = Rotation;

	PawnMesh = GetSkeletalMeshComponent();
	if( PawnMesh != None )
	{
		if( FireMode == ALTFIRE_FIREMODE )
		{
			PawnMesh.GetSocketWorldLocationAndRotation( RocketMuzzleSocketName, MuzzleLoc, MuzzleRot );
		}
		else
		{
			PawnMesh.GetSocketWorldLocationAndRotation( GetMuzzleSocketName(IsLeftGun()), MuzzleLoc, MuzzleRot );
		}
	}
}

defaultproperties
{
	RocketMuzzleSocketName=RocketMuzzle

	TracerType=WTT_BrumakPlayer

	InstantHitDamageTypes(0)=class'GDT_BrumakBulletPlayer'

	// ROCKET FIREMODE
	FiringStatesArray(ALTFIRE_FIREMODE)="RocketsFiring"
	WeaponFireTypes(ALTFIRE_FIREMODE)=EWFT_Projectile
	InstantHitDamageTypes(ALTFIRE_FIREMODE)=class'GDT_BrumakCannonPlayer'
	InstantHitMomentum(ALTFIRE_FIREMODE)=1
	FireInterval(ALTFIRE_FIREMODE)=1.5
	WeaponProjectiles(ALTFIRE_FIREMODE)=class'GearProj_BrumakRocket_Player'

	Begin Object Name=PSC_Muzzle0
		bAutoActivate=FALSE
		Template=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Rideable_Gun_MuzzleFlash'
	End Object

	Begin Object Class=ParticleSystemComponent Name=PSC_Muzzle1
		bAutoActivate=FALSE
		Template=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Rideable_Gun_MuzzleFlash'
	End Object
	MuzFlashEmitterLeft=PSC_Muzzle1

	Begin Object Class=ParticleSystemComponent Name=PSC_Muzzle2
		bAutoActivate=FALSE
		Template=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Rideable_Rocket_MuzzleFlash'
	End Object
	RocketMuzFlashEmitter=PSC_Muzzle2

	ActiveCoolingStartCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamVentOpenCue'
	ActiveCoolingStopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamVentCloseCue'
	ActiveCoolingLoopCue=SoundCue'Weapon_HeavyMiniGun.Gatling.GatlingHeatSteamLoop01Cue'
	RocketFireCue_Player=SoundCue'Locust_Brumak_Efforts2.Brumak.Brumak_RocketFire_Cue'
	RocketRoarCue=SoundCue'Locust_Brumak_Efforts2.Brumak.Brumak_RoarAfterFire_Cue'

	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=146,V=193,UL=143,VL=46)
	HUDDrawData=(DisplayCount=106,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=494,UL=106,VL=7),ULPerAmmo=0)

	bKillDuringLevelTransition=FALSE
}
