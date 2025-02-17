/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearVehicleWeapon extends GearWeapon
	abstract
	native(Weapon)
	nativereplication
	dependson(GearPhysicalMaterialProperty);

/** Holds a link in to the Seats array in MyVehicle that represents this weapon */
var int SeatIndex;

/** Holds a link to the parent vehicle */
var RepNotify GearVehicle	MyVehicle;

/** Triggers that should be activated when a weapon fires */
var array<name>	FireTriggerTags, AltFireTriggerTags;

/** sound that is played when the bullets go whizzing past your head */
var SoundCue BulletWhip;

/** last time aim was correct, used for looking up GoodAimColor */
var float LastCorrectAimTime;

/** last time aim was incorrect, used for looking up GoodAimColor */
var float LastInCorrectAimTime;

var float CurrentCrosshairScaling;

/** cached max range of the weapon used for aiming traces */
var float AimTraceRange;

/** actors that the aiming trace should ignore */
var array<Actor> AimingTraceIgnoredActors;

/** This value is used to cap the maximum amount of "automatic" adjustment that will be made to a shot
so that it will travel at the crosshair.  If the angle between the barrel aim and the player aim is
less than this angle, it will be adjusted to fire at the crosshair.  The value is in radians */
var float MaxFinalAimAdjustment;

var bool bPlaySoundFromSocket;

/** used for client to tell server when zooming, as that is clientside
* but on console it affects the controls so the server needs to know
*/
var bool bCurrentlyZoomed;

/**
* If the weapon is attached to a socket that doesn't pitch with
* player view, and should fire at the aimed pitch, then this should be enabled.
*/
var bool bIgnoreSocketPitchRotation;
/**
* Same as above, but only allows for downward direction, for vehicles with 'bomber' like behavior.
*/
var bool bIgnoreDownwardPitch;

/** Vehicle class used for drawing kill icons */
var class<GearVehicle> VehicleClass;

/** for debugging turret aiming */
var bool bDebugTurret;

/** if we fail an AR, add this much time to the reload */
var config float AR_FailReloadPenanltySeconds;

cpptext
{
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	if (Role == ROLE_Authority && bNetInitial && bNetOwner)
		SeatIndex, MyVehicle;
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	AimTraceRange = MaxRange();
}

/** checks if the weapon is actually capable of hitting the desired target, including trace test (used by crosshair)
* if false because trace failed, RealAimPoint is set to what the trace hit
*/
simulated function bool CanHitDesiredTarget(vector SocketLocation, rotator SocketRotation, vector DesiredAimPoint, Actor TargetActor, out vector RealAimPoint)
{
	local int i;
	local array<Actor> IgnoredActors;
	local Actor HitActor;
	local vector HitLocation, HitNormal;
	local bool bResult;

	if ((Normal(DesiredAimPoint - SocketLocation) dot Normal(RealAimPoint - SocketLocation)) >= GetMaxFinalAimAdjustment())
	{
		// turn off bProjTarget on Actors we should ignore for the aiming trace
		for (i = 0; i < AimingTraceIgnoredActors.length; i++)
		{
			if (AimingTraceIgnoredActors[i] != None && AimingTraceIgnoredActors[i].bProjTarget)
			{
				AimingTraceIgnoredActors[i].bProjTarget = false;
				IgnoredActors[IgnoredActors.length] = AimingTraceIgnoredActors[i];
			}
		}
		// perform the trace
		HitActor = GetTraceOwner().Trace(HitLocation, HitNormal, DesiredAimPoint, SocketLocation, true,,, TRACEFLAG_Bullet);
		if (HitActor == None || HitActor == TargetActor)
		{
			bResult = true;
		}
		else
		{
			RealAimPoint = HitLocation;
		}
		// restore bProjTarget on Actors we turned it off for
		for (i = 0; i < IgnoredActors.length; i++)
		{
			IgnoredActors[i].bProjTarget = true;
		}
	}

	return bResult;
}

/** GetDesiredAimPoint - Returns the desired aim given the current controller
* @param TargetActor (out) - if specified, set to the actor being aimed at
* @return The location the controller is aiming at
*/
simulated event vector GetDesiredAimPoint(optional out Actor TargetActor)
{
	local vector CameraLocation, HitLocation, HitNormal, DesiredAimPoint;
	local rotator CameraRotation;
	local Controller C;
	local PlayerController PC;

	C = MyVehicle.Seats[SeatIndex].SeatPawn.Controller;

	PC = PlayerController(C);
	if (PC != None)
	{
		PC.GetPlayerViewPoint(CameraLocation, CameraRotation);
		DesiredAimPoint = CameraLocation + Vector(CameraRotation) * GetTraceRange();
		TargetActor = GetTraceOwner().Trace(HitLocation, HitNormal, DesiredAimPoint, CameraLocation);
		if (TargetActor != None)
		{
			DesiredAimPoint = HitLocation;
		}
	}
	else if ( C != None )
	{
		DesiredAimPoint = C.GetFocalPoint();
		TargetActor = C.Focus;
	}
	return DesiredAimPoint;
}

/** returns the location and rotation that the weapon's fire starts at */
simulated function GetFireStartLocationAndRotation(out vector StartLocation, out rotator StartRotation)
{
	if ( MyVehicle.Seats[SeatIndex].GunSocket.Length>0 )
	{
		MyVehicle.GetBarrelLocationAndRotation(SeatIndex, StartLocation, StartRotation);
	}
	else
	{
		StartLocation = MyVehicle.Location;
		StartRotation = MyVehicle.Rotation;
	}
}

/**
* IsAimCorrect - Returns true if the turret associated with a given seat is aiming correctly
*
* @return TRUE if we can hit where the controller is aiming
*/
simulated event bool IsAimCorrect()
{
	local vector SocketLocation, DesiredAimPoint;
	local rotator SocketRotation;
	local float	DotP, MaxAdjust;
	//debug
	//local Vector RealAimPoint;

	DesiredAimPoint = GetDesiredAimPoint();

	GetFireStartLocationAndRotation(SocketLocation, SocketRotation);

	DotP = Normal(DesiredAimPoint - SocketLocation) dot Vector(SocketRotation);
	MaxAdjust = GetMaxFinalAimAdjustment();

	/*
	//debug
	RealAimPoint = SocketLocation + Vector(SocketRotation) * GetTraceRange();
	FlushPersistentDebugLines();
	DrawDebugCoordinateSystem( SocketLocation, SocketRotation, 256.f, TRUE );
	DrawDebugBox( DesiredAimPoint, vect(25,25,25), 0, 255, 0, TRUE );
	DrawDebugLine( SocketLocation, DesiredAimPoint, 0, 255, 0, TRUE );
	DrawDebugBox( RealAimPoint, vect(25,25,25), 255, 0, 0, TRUE );
	DrawDebugLine( SocketLocation, RealAimPoint, 255, 0, 0, TRUE );
	`AILog_Ext( GetFuncName()@DotP@MaxAdjust@"--"@DesiredAimPoint@"--"@RealAimPoint@(DotP>=MaxAdjust), '', GearAI(AIController) );
*/

	return (DotP >= MaxAdjust);
}


simulated static function Name GetFireTriggerTag(int BarrelIndex, int FireMode)
{
	if (FireMode==0)
	{
		if (default.FireTriggerTags.Length > BarrelIndex)
		{
			return default.FireTriggerTags[BarrelIndex];
		}
	}
	else
	{
		if (default.AltFireTriggerTags.Length > BarrelIndex)
		{
			return default.AltFireTriggerTags[BarrelIndex];
		}
	}
	return '';
}

simulated function AttachWeaponTo( SkeletalMeshComponent MeshCpnt, optional Name SocketName );
simulated function DetachWeapon();

simulated function Activate()
{
	// don't reactivate if already firing
	if (!IsFiring())
	{
		GotoState('Active');
	}
}

simulated function PutDownWeapon()
{
	GotoState('Inactive');
}

simulated native event Vector GetPhysicalFireStartLoc(optional vector AimDir);

simulated function BeginFire(byte FireModeNum)
{
	local GearVehicle V;

	// allow the vehicle to override the call
	V = GearVehicle(Instigator);
	if (V == None || !V.OverrideBeginFire(FireModeNum))
	{
		Super.BeginFire(FireModeNum);
	}
}

simulated function EndFire(byte FireModeNum)
{
	local GearVehicle V;

	// allow the vehicle to override the call
	V = GearVehicle(Instigator);
	if (V == None || !V.OverrideEndFire(FireModeNum))
	{
		Super.EndFire(FireModeNum);
	}
}

simulated function Rotator GetAdjustedAim( vector StartFireLoc )
{
	// Start the chain, see Pawn.GetAdjustedAimFor()
	// @note we don't add in spread here because UTVehicle::GetWeaponAim() assumes
	// that a return value of Instigator.Rotation or Instigator.Controller.Rotation means 'no adjustment', which spread interferes with
	return Instigator.GetAdjustedAimFor( Self, StartFireLoc );
}

/**
* Create the projectile, but also increment the flash count for remote client effects.
*/
simulated function Projectile ProjectileFire()
{
	local Projectile SpawnedProjectile;

	IncrementFlashCount();

	if (Role==ROLE_Authority)
	{
		SpawnedProjectile = Spawn(GetProjectileClass(),,, MyVehicle.GetPhysicalFireStartLoc(self));

		if ( SpawnedProjectile != None )
		{
			SpawnedProjectile.Init( vector(MyVehicle.GetWeaponAim(self)) );
		}
	}
	return SpawnedProjectile;
}

/**
* Overriden to use vehicle starttrace/endtrace locations
* @returns position of trace start for instantfire()
*/
simulated function vector InstantFireStartTrace()
{
	return MyVehicle.GetPhysicalFireStartLoc(self);
}

/**
* @returns end trace position for instantfire()
*/
simulated function vector InstantFireEndTrace(vector StartTrace)
{
	return StartTrace + vector(AddSpread(MyVehicle.GetWeaponAim(self))) * GetTraceRange();;
}

simulated function Actor GetTraceOwner()
{
	return (MyVehicle != None) ? MyVehicle : Super.GetTraceOwner();
}

simulated native function float GetMaxFinalAimAdjustment();

simulated function WeaponPlaySound( SoundCue Sound, optional float NoiseLoudness )
{
	local int Barrel;
	local name Pivot;
	local vector Loc;
	local rotator Rot;
	if(bPlaySoundFromSocket && MyVehicle != none && MyVehicle.Mesh != none)
	{
		if( Sound == None || Instigator == None )
		{
			return;
		}
		Barrel = MyVehicle.GetBarrelIndex(SeatIndex);
		Pivot = MyVehicle.Seats[SeatIndex].GunSocket[Barrel];
		MyVehicle.Mesh.GetSocketWorldLocationAndRotation(Pivot, Loc, Rot);
		Instigator.PlaySound(Sound, false, true,false,Loc);
	}
	else
		super.WeaponPlaySound(Sound,NoiseLoudness);
}

simulated function StartFire(byte FireModeNum)
{
	// AI doesn't fire if vehicle turret can't hit target
	if (AIController == None || AIController.bDeleteMe || AIController != Instigator.Controller || IsAimCorrect())
	{
		Super.StartFire(FireModeNum);
	}
}

simulated function bool ShouldRefire()
{
	return ((AIController == None || IsAimCorrect()) && (GearAIController == None || GearAIController.IsFireLineClear()) && Super.ShouldRefire());
}

simulated function Vector GetMuzzleLoc()
{
	local vector FireLocation;
	local rotator FireRotation;

	MyVehicle.GetBarrelLocationAndRotation(SeatIndex, FireLocation, FireRotation);

	return FireLocation;
}

simulated function bool CanReload()
{

	return(	bWeaponCanBeReloaded	&&	// Weapon can be reloaded
		AmmoUsedCount > 0		&&	// we fired at least a shot
		HasSpareAmmo()				// and we have more ammo to fill current magazine
		);
}

/** Is the weapon currently being reloaded? */
simulated function bool IsReloading()
{
	return Instigator != None && IsTimerActive('EndOfReloadTimer');
}

/** Active Reload failed, ouch... penalty follows. */
simulated function PlayActiveReloadFailed()
{
	local Pawn		P;
	local float		NewTimerRate;

	P = (Instigator);
	// Need a Pawn doing a reload to proceed
	if( P == None || !IsTimerActive('EndOfReloadTimer') )
	{
		return;
	}

	NewTimerRate = Max(ReloadDuration - TimeSince(AR_TimeReloadButtonWasPressed),0) + AR_FailReloadPenanltySeconds;

	// Adjust end of reload timer based on new animation
	if( NewTimerRate > 0.f )
	{
		SetTimer( NewTimerRate, FALSE, nameof(EndOfReloadTimer) );
	}
	else
	{
		`warn(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "Instigator:" @ Instigator @ "NewTimerRate:" @ NewTimerRate @ "!!! Calling EndOfReloadTimer() right away.");
		EndOfReloadTimer();
	}
}

defaultproperties
{
	TickGroup=TG_PostAsyncWork

	// ~ 5 Degrees
	MaxFinalAimAdjustment=0.995;
}
