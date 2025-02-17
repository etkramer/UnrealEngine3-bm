/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTWeap_RocketLauncher extends UTWeapon
	native;

var enum ERocketFireMode1
{
	RFM_Spread,
	RFM_Spiral,
	RFM_Grenades,
	RFM_Max
} LoadedFireMode;	

/** Class of the rocket to use when seeking */
var class<UTProjectile> SeekingRocketClass;

/** Class of the rocket to use when multiple rockets loaded */
var class<UTProjectile> LoadedRocketClass;

/** Class of the grenade */
var class<UTProjectile> GrenadeClass;

/** How much spread on Grenades */
var int GrenadeSpreadDist;

/** The sound to play when alt-fire mode is changed */
var SoundCue AltFireModeChangeSound;

/** The sound to play when a rocket is loaded */
var SoundCue RocketLoadedSound;
/** sound to play when firing grenades */
var SoundCue GrenadeFireSound;

/** Skeletal mesh component for the 1st person view of the rocketlauncher */
var UTSkeletalMeshComponent SkeletonFirstPersonMesh;

/** Have we hidden any of the ammo sockets from player's view */
var bool bIsAnyAmmoHidden;

/*********************************************************************************************
 * Weapon lock on support
 ********************************************************************************************* */

/** The frequency with which we will check for a lock */
var(Locking) float		LockCheckTime;

/** How far out should we be considering actors for a lock */
var float		LockRange;

/** How long does the player need to target an actor to lock on to it*/
var(Locking) float		LockAcquireTime;

/** Once locked, how long can the player go without painting the object before they lose the lock */
var(Locking) float		LockTolerance;

/** When true, this weapon is locked on target */
var bool 				bLockedOnTarget;

/** What "target" is this weapon locked on to */
var Actor 				LockedTarget;

var PlayerReplicationInfo LockedTargetPRI;

/** What "target" is current pending to be locked on to */
var Actor				PendingLockedTarget;

/** How long since the Lock Target has been valid */
var float  				LastLockedOnTime;

/** When did the pending Target become valid */
var float				PendingLockedTargetTime;

/** When was the last time we had a valid target */
var float				LastValidTargetTime;

/** angle for locking for lock targets */
var float 				LockAim;

/** angle for locking for lock targets when on Console */
var float 				ConsoleLockAim;

/** Sound Effects to play when Locking */
var SoundCue 			LockAcquiredSound;
var SoundCue			LockLostSound;

/** If true, weapon will try to lock onto targets */
var bool bTargetLockingActive;

/** Last time target lock was checked */
var float LastTargetLockCheckTime;

/** This holds the current number of shots queued up */
var int LoadedShotCount;

/** This holds the maximum number of shots that can be queued */
var int MaxLoadCount;

var Soundcue WeaponLoadedSnd;

// LoadedWeapons supported multiple animation paths depending on
// how many rockets have been loaded.  These next few variables
// are used to time those paths.

/** Holds how long it takes to que a given shot */
var array<float> AltFireQueueTimes;

/** Holds how long it takes to fire a given shot */
var array<float> AltFireLaunchTimes;

/** How long does it takes after a shot is fired for the weapon to reset and return to active */
var array<float> AltFireEndTimes;

/** Holds a collection of sound cues that define what sound to play at each loaded level */
var array<SoundCue> AltFireSndQue;

/** Allow for multiple muzzle flash sockets **FIXME: will become offsets */
var array<name> MuzzleFlashSocketList;

/** Holds a list of emitters that make up the muzzle flash */
var array<UTParticleSystemComponent> MuzzleFlashPSCList;

/** How much distance should be between each shot */
var int	SpreadDist;

/** How much grace at the end of loading should be given before the weapon auto-fires */
var float GracePeriod;

/** How much of the load-up timer needs to pass before the weapon will wait to fire on release */
var float WaitToFirePct;

var localized string GrenadeString, SpiralString;

/** Textures which show multiple loaded rockets */
var TextureCoordinates LoadedIconCoords[3];

/** list of anims to play for loading the RL */
var name LoadUpAnimList[3];

var name WeaponAltFireLaunch[3];
var name WeaponAltFireLaunchEnd[3];

cpptext
{
	virtual void TickSpecial( FLOAT DeltaSeconds );
}

replication
{
	// Server->Client properties
	if (Role == ROLE_Authority)
		bLockedOnTarget, LockedTarget;
}

/**
  * Adjust weapon equip and fire timings so they match between PC and console
  * This is important so the sounds match up.
  */
simulated function AdjustWeaponTimingForConsole()
{
	local int i;

	Super.AdjustWeaponTimingForConsole();

	For ( i=0; i<AltFireQueueTimes.Length; i++ )
	{
		AltFireQueueTimes[i] = AltFireQueueTimes[i]/1.1;
	}
	For ( i=0; i<AltFireEndTimes.Length; i++ )
	{
		AltFireEndTimes[i] = AltFireEndTimes[i]/1.1;
	}
	For ( i=0; i<AltFireLaunchTimes.Length; i++ )
	{
		AltFireLaunchTimes[i] = AltFireLaunchTimes[i]/1.1;
	}
}

/**
 * Returns interval in seconds between each shot, for the firing state of FireModeNum firing mode.
 * We override it here to support the queue!
 *
 * @param	FireModeNum	fire mode
 * @return	Period in seconds of firing mode
 */
simulated function float GetFireInterval( byte FireModeNum )
{
	if (FireModeNum != 1 || UTPawn(Owner) == None || LoadedShotCount == MaxLoadCount)
	{
		return super.GetFireInterval(FireModeNum);
	}
	else
	{
		return AltFireQueueTimes[LoadedShotCount] * UTPawn(Owner).FireRateMultiplier;
	}
}


/*********************************************************************************************
 * Target Locking
 *********************************************************************************************/

/**
 * The function checks to see if we are locked on a target
 */
native function CheckTargetLock();

native function UpdateLockTarget(Actor NewLockTarget);

simulated function GetWeaponDebug( out Array<String> DebugInfo )
{
	Super.GetWeaponDebug(DebugInfo);

	DebugInfo[DebugInfo.Length] = "Locked: "@bLockedOnTarget@LockedTarget@LastLockedontime@(WorldInfo.TimeSeconds-LastLockedOnTime);
	DebugInfo[DebugInfo.Length] = "Pending:"@PendingLockedTarget@PendingLockedTargetTime@WorldInfo.TimeSeconds;
}

simulated function FireAmmunition()
{
	Super.FireAmmunition();

	if ( Role == ROLE_Authority )
	{
		AdjustLockTarget( none );
		LastValidTargetTime = 0;
		PendingLockedTarget = None;
		LastLockedOnTime = 0;
		PendingLockedTargetTime = 0;
	}
}

/**
 *  This function is used to adjust the LockTarget.
 *  Called by UpdateLockTarget() only when NewLockTarget is different from LockTarget
 */
event AdjustLockTarget(actor NewLockTarget)
{
	if (NewLockTarget == None)
	{
		// Clear the lock
		if (bLockedOnTarget)
		{
			LockedTarget = None;

			bLockedOnTarget = false;

			if (LockLostSound != None && Instigator != None && Instigator.IsHumanControlled() )
			{
				PlayerController(Instigator.Controller).ClientPlaySound(LockLostSound);
			}
		}
	}
	else
	{
		// Set the lcok
		bLockedOnTarget = true;
		LockedTarget = NewLockTarget;
		LockedTargetPRI = (Pawn(NewLockTarget) != None) ? Pawn(NewLockTarget).PlayerReplicationInfo : None;
		if ( LockAcquiredSound != None && Instigator != None  && Instigator.IsHumanControlled() )
		{
			PlayerController(Instigator.Controller).ClientPlaySound(LockAcquiredSound);
		}
	}
}

/**
 * Given an actor (TA) determine if we can lock on to it.  By default only allow locking on
 * to pawns.  Some weapons may want to be able to lock on to other actors.
 */
native function bool CanLockOnTo(Actor TA);

auto simulated state Inactive
{
	simulated function BeginState(name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);

		if ( Role == ROLE_Authority )
		{
			bTargetLockingActive = false;
			AdjustLockTarget(None);
		}
	}

	simulated function EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);

		if ( Role == ROLE_Authority )
		{
			bTargetLockingActive = true;
		}
	}
}

simulated event Destroyed()
{
	AdjustLockTarget(none);
	super.Destroyed();
}

/**
 * Access to HUD and Canvas.
 * Event always called when the InventoryManager considers this Inventory Item currently "Active"
 * (for example active weapon)
 *
 * @param	HUD			- HUD with canvas to draw on
 */
simulated function ActiveRenderOverlays( HUD H )
{
	local UTPlayerController PC;

	PC = UTPlayerController(Instigator.Controller);
	if ( PC == None )
	{
		return;
	}
	if ( PC.bNoCrosshair )
	{
		return;
	}
	if ( PC.bSimpleCrosshair )
	{
		CrossHairCoordinates = SimpleCrosshairCoordinates;
		super.DrawWeaponCrosshair( H );
	}
	else
	{
		CrossHairCoordinates = default.CrosshairCoordinates;
		DrawWeaponCrosshair( H );
	}
	if ( bLockedOnTarget && (LockedTarget != None) && (Instigator != None) )
	{
		if ( (LocalPlayer(PC.Player) == None) || !LocalPlayer(PC.Player).GetActorVisibility(LockedTarget) )
		{
			return;
		}
		DrawLockedOn( H );
	}
	else
	{
		bWasLocked = false;
	}

}

//*********************************************************************************************
/**
 * Turns the MuzzleFlashlight off
 */
simulated event MuzzleFlashTimer()
{
	local int i, length;

	if (CurrentFireMode != 1)
	{
		Super.MuzzleFlashTimer();
	}
	else
	{
	    //Always deactivate all three
		length = MuzzleFlashPSCList.length;
		for (i=0;i<length;i++)
		{
		    if (MuzzleFlashPSCList[i] != None)
			{
			   MuzzleFlashPSCList[i].DeactivateSystem();
			}
		}
	}
}

/**
 * Quickly turns off an active muzzle flash
 */
simulated event StopMuzzleFlash()
{
	ClearTimer('MuzzleFlashTimer');

	//Makes call to shut off all PSC muzzle flashes
	MuzzleFlashTimer();

	if ( MuzzleFlashPSC != none )
	{
		MuzzleFlashPSC.DeactivateSystem();
	}
}


/**
 * Causes the muzzle flashlight to turn on
 */
simulated event CauseMuzzleFlashLight()
{
	local UTExplosionLight NewMuzzleFlashLight;

	Super.CauseMuzzleFlashLight();

	if ( WorldInfo.bDropDetail )
		return;

	if ( LoadedShotCount == 3 )
	{
		NewMuzzleFlashLight = new(Outer) MuzzleFlashLightClass;
		SkeletalMeshComponent(Mesh).AttachComponentToSocket(NewMuzzleFlashLight,MuzzleFlashSocketList[2]);
	}
}

simulated function AttachMuzzleFlash()
{
	local UTSkeletalMeshComponent SKMesh;
	local int i;

	bMuzzleFlashAttached = true;
	SKMesh = UTSkeletalMeshComponent(Mesh);

	// Attach the Muzzle Flash
	if ( SKMesh != none )
	{
		if (MuzzleFlashPSCTemplate != none)
		{
			for (i=0;i<3;i++)
			{
				MuzzleFlashPSCList[i] = new(Outer) class'UTParticleSystemComponent';
				MuzzleFlashPSCList[i].bAutoActivate = false;
				MuzzleFlashPSCList[i].SetTemplate(MuzzleFlashPSCTemplate);
				MuzzleFlashPSCList[i].SetDepthPriorityGroup(SDPG_Foreground);
				MuzzleFlashPSCList[i].SetColorParameter('MuzzleFlashColor',MuzzleFlashColor);
				MuzzleFlashPSCList[i].SetVectorParameter('MFlashScale', vect(0.5,0.5,0.5));
				MuzzleFlashPSCList[i].SetFOV(SKMesh.FOV);
				SKMesh.AttachComponentToSocket(MuzzleFlashPSCList[i], MuzzleFlashSocketList[i]);
			}
		}
	}
}

/**
 * Remove/Detach the muzzle flash components
 */
simulated function DetachMuzzleFlash()
{
	local SkeletalMeshComponent SKMesh;
	local int i;

	bMuzzleFlashAttached = false;

	SKMesh = SkeletalMeshComponent(Mesh);

	if ( SKMesh != none )
	{
	    if (MuzzleFlashPSC != none)
	    {
			SKMesh.DetachComponent(MuzzleFlashPSC);
			MuzzleFlashPSC = None;
		}

		if ( MuzzleFlashPSCList.Length == 0 )
			return;
		for (i=0;i<3;i++)
		{
			if (MuzzleFlashPSCList[i] != none)
			{
				SKMesh.DetachComponent( MuzzleFlashPSCList[i] );
				MuzzleFlashPSCList[i] = None;
			}
		}
	}
}

/**
 * Causes the muzzle flashlight to turn on and setup a time to
 * turn it back off again.
 */
simulated event CauseMuzzleFlash()
{
	local int i;
	local UTPawn P;

	if ( WorldInfo.NetMode != NM_Client )
	{
		P = UTPawn(Instigator);
		if ( (P == None) || !P.bUpdateEyeHeight )
		{
			return;
		}
	}

    //AltFire grenades don't cause a muzzle flash
	if (CurrentFireMode == 1 && LoadedFireMode == RFM_Grenades)
	{
		return;
	}
	if ( !bMuzzleFlashAttached )
	{
		AttachMuzzleFlash();
	}

    //Add a light to the scene
	CauseMuzzleFlashLight();

	if (CurrentFireMode != 1)
	{
	    //Regular fire mode, activate the top muzzle flash
		if (MuzzleFlashPSCList.length > 0 && MuzzleFlashPSCList[0] != None)
		{
			MuzzleFlashPSCList[0].ActivateSystem();
		}
	}
	else
	{
	    //AltFire mode, activate LoadedShotCount muzzleflashes
		for (i = 0; i < LoadedShotCount && i < MuzzleFlashPSCList.length; i++)
		{
			if (MuzzleFlashPSCList[i] != None)
			{
				MuzzleFlashPSCList[i].ActivateSystem();
			}
		}
	}

	// Set when to turn it off.
	SetTimer(MuzzleFlashDuration,false,'MuzzleFlashTimer');
}

/*********************************************************************************************
 * Hud/Crosshairs
 *********************************************************************************************/

simulated function DrawLFMData(HUD Hud)
{
	local string s;
	local vector2d CrosshairSize;
	local float XL, YL, x,y,PickupScale, ScreenX, ScreenY;
	local UTHUD	H;
	local int DrawLoaded;
	local rotator CrosshairRotation;
	local float TimeRemaining;

	if ( LoadedShotCount == 0 )
	{
		Global.DrawWeaponCrosshair(HUD);
		return;
	}

	H = UTHUD(HUD);
	if ( H == None )
		return;

	// Apply pickup scaling
	if ( H.LastPickupTime > WorldInfo.TimeSeconds - 0.3 )
	{
		if ( H.LastPickupTime > WorldInfo.TimeSeconds - 0.15 )
		{
			PickupScale = (1 + 5 * (WorldInfo.TimeSeconds - H.LastPickupTime));
		}
		else
		{
			PickupScale = (1 + 5 * (H.LastPickupTime + 0.3 - WorldInfo.TimeSeconds));
		}
	}
	else
	{
		PickupScale = 1.0;
	}

	DrawLoaded = Min(LoadedShotCount-1, 3);
 	CrosshairSize.Y = H.ConfiguredCrosshairScaling * CrosshairScaling * LoadedIconCoords[DrawLoaded].VL * PickupScale * H.Canvas.ClipY/720;
  	CrosshairSize.X = CrosshairSize.Y * LoadedIconCoords[DrawLoaded].UL/LoadedIconCoords[DrawLoaded].VL;

	X = H.Canvas.ClipX * 0.5;
	Y = H.Canvas.ClipY * 0.5;
	ScreenX = X - (CrosshairSize.X * 0.5);
	ScreenY = Y - (CrosshairSize.Y * 0.5);

	TimeRemaining = GetTimerCount('RefireCheckTimer');
	if ( LoadedShotCount < 3 )
	{
		CrosshairRotation.Yaw = 21845.3333 * (LoadedShotCount - 1.0 + FMin(1.0,2.0*TimeRemaining/GetFireInterval(1)));
	}
	H.Canvas.DrawColor = H.BlackColor;
	H.Canvas.SetPos(ScreenX, ScreenY);
	H.Canvas.DrawRotatedTile(CrosshairImage, CrosshairRotation, CrosshairSize.X, CrosshairSize.Y, LoadedIconCoords[DrawLoaded].U, LoadedIconCoords[DrawLoaded].V, LoadedIconCoords[DrawLoaded].UL,LoadedIconCoords[DrawLoaded].VL);

	H.Canvas.DrawColor = UTHUD(HUD).bGreenCrosshair ? UTHUD(HUD).Default.LightGreenColor : Default.CrosshairColor;
	H.Canvas.SetPos(ScreenX, ScreenY);
	H.Canvas.DrawRotatedTile(CrosshairImage, CrosshairRotation, CrosshairSize.X, CrosshairSize.Y, LoadedIconCoords[DrawLoaded].U, LoadedIconCoords[DrawLoaded].V, LoadedIconCoords[DrawLoaded].UL,LoadedIconCoords[DrawLoaded].VL);

	if (LoadedFireMode != RFM_Spread)
	{
		S = (LoadedFireMode == RFM_Spiral) ? SpiralString : GrenadeString;

		H.Canvas.Font = class'UTHUD'.static.GetFontSizeIndex(0);
		H.Canvas.StrLen(S,XL,YL);
		H.Canvas.SetPos( 0.5*H.Canvas.ClipX - 0.5*XL, 0.5*H.Canvas.ClipY + 0.71*CrosshairSize.Y);
		H.Canvas.DrawText(s);
	}
}


/*********************************************************************************************
 * AI Interface
 *********************************************************************************************/

function float SuggestAttackStyle()
{
	local float EnemyDist;

	if (Instigator.Controller.Enemy != None)
	{
		// recommend backing off if target is too close
		EnemyDist = VSize(Instigator.Controller.Enemy.Location - Owner.Location);
		if ( EnemyDist < 750 )
		{
			return (EnemyDist < 500) ? -1.5 : -0.7;
		}
		else if (EnemyDist > 1600)
		{
			return 0.5;
		}
	}

	return -0.1;
}

// tell bot how valuable this weapon would be to use, based on the bot's combat situation
// also suggest whether to use regular or alternate fire mode
function float GetAIRating()
{
	local UTBot B;
	local float EnemyDist, Rating, ZDiff;
	local vector EnemyDir;

	B = UTBot(Instigator.Controller);
	if ( (B == None) || (B.Enemy == None) )
		return AIRating;

	// if standing on a lift, make sure not about to go around a corner and lose sight of target
	// (don't want to blow up a rocket in bot's face)
	if ( (Instigator.Base != None) && (Instigator.Base.Velocity != vect(0,0,0))
		&& !B.CheckFutureSight(0.1) )
		return 0.1;

	EnemyDir = B.Enemy.Location - Instigator.Location;
	EnemyDist = VSize(EnemyDir);
	Rating = AIRating;

	// don't pick rocket launcher if enemy is too close
	if ( EnemyDist < 360 )
	{
		if ( Instigator.Weapon == self )
		{
			// don't switch away from rocket launcher unless really bad tactical situation
			if ( (EnemyDist > 250) || ((Instigator.Health < 50) && (Instigator.Health < B.Enemy.Health - 30)) )
				return Rating;
		}
		return 0.05 + EnemyDist * 0.001;
	}

	// rockets are good if higher than target, bad if lower than target
	ZDiff = Instigator.Location.Z - B.Enemy.Location.Z;
	if ( ZDiff > 120 )
		Rating += 0.25;
	else if ( ZDiff < -160 )
		Rating -= 0.35;
	else if ( ZDiff < -80 )
		Rating -= 0.05;
	if ( (B.Enemy.Weapon != None) && B.Enemy.Weapon.bMeleeWeapon && (EnemyDist < 2500) )
		Rating += 0.25;

	return Rating;
}

/* BestMode()
choose between regular or alt-fire
*/
function byte BestMode()
{
	local UTBot B;

	if (IsFiring())
	{
		return CurrentFireMode;
	}

	B = UTBot(Instigator.Controller);
	if (B == None || B.Enemy == None)
	{
		return 0;
	}

	return (FRand() < 0.3 && !B.IsStrafing() && Instigator.Physics != PHYS_Falling) ? 1 : 0;
}

/*********************************************************************************************
 * Utility Functions.
 *********************************************************************************************/


/**
 * @Returns 	The amount of spread
 */

simulated function int GetSpreadDist()
{
	if ( LoadedFireMode == RFM_Grenades )
	{
		return GrenadeSpreadDist;
	}
	else
	{
		return SpreadDist;
	}
}

simulated function WeaponFireLoad()
{
	SetCurrentFireMode(1);
	IncrementFlashCount();

	if (Role == ROLE_Authority)
	{
		FireLoad();
	}

	PlayFiringSound();
	InvManager.OwnerEvent('FiredWeapon');
	GotoState('WeaponPlayingFire');

	//We've expended all our ammo, make sure the ammo geometry is hidden from view
	if (AmmoCount == 0 && Instigator.IsHumanControlled() && Instigator.IsLocallyControlled())
	{
		HideRocket('MackRocketScale', true);
		bIsAnyAmmoHidden = true;
	}
}

/**
 * Fire off a load of rockets.
 *
 * Network: Server Only
 */
function FireLoad()
{
	local int i,j,k;
	local vector SpreadVector;
	local rotator Aim;
	local float theta;
   	local vector Firelocation, RealStartLoc, X,Y,Z;
	local Projectile	SpawnedProjectile;
	local UTProj_LoadedRocket FiredRockets[4];
	local bool bCurl;
	local byte FlockIndex;

	// this is to get the location of the "popped up" rocket launcher tube
	if(LoadedShotCount == 1 && CurrentFireMode == 1)
	{
		RealStartLoc = GetPhysicalFireStartLoc() + vect(0,0,3);
	}
	else
	{
		// this is the location where the projectile is spawned
		RealStartLoc = GetPhysicalFireStartLoc();
	}


	Aim = GetAdjustedAim( RealStartLoc );			// get fire aim direction

	GetViewAxes(X,Y,Z);

	for (i = 0; i < LoadedShotCount; i++)
	{
		if (LoadedFireMode == RFM_Grenades || LoadedFireMode == RFM_Spread)
		{
			// Give them some gradual spread.
			theta = GetSpreadDist() * PI / 32768.0 * (i - float(LoadedShotCount - 1) / 2.0);
			SpreadVector.X = Cos(theta);
			SpreadVector.Y = Sin(theta);
			SpreadVector.Z = 0.0;

			SpawnedProjectile = Spawn(GetProjectileClass(),,, RealStartLoc, Rotator(SpreadVector >> Aim));
			if ( SpawnedProjectile != None )
			{
				if (LoadedFireMode == RFM_Grenades)
				{
					UTProjectile(SpawnedProjectile).TossZ += (frand() * 200 - 100);
				}
				SpawnedProjectile.Init(SpreadVector >> Aim);
			}
		}
		else
		{
			Firelocation = RealStartLoc - 2 * (       (Sin(i * 2 * PI / MaxLoadCount) * 8 - 7) * Y - (Cos(i * 2 * PI / MaxLoadCount) * 8 - 7) * Z    ) - X * 8 * FRand();
			SpawnedProjectile = Spawn(GetProjectileClass(),,, FireLocation, Aim);
			if ( SpawnedProjectile != None )
			{
				SpawnedProjectile.Init(vector(Aim));
				FiredRockets[i] = UTProj_LoadedRocket(SpawnedProjectile);
			}
		}

		if (LoadedFireMode != RFM_Grenades && bLockedOnTarget && UTProj_SeekingRocket(SpawnedProjectile) != None)
		{
			UTProj_SeekingRocket(SpawnedProjectile).Seeking = LockedTarget;
		}
	}

	// Initialize the rockets so they flock towards each other
	if ( !bLockedOnTarget && (LoadedFireMode == RFM_Spiral) )
	{
		FlockIndex++;
		bCurl = false;

		// To get crazy flying, we tell each projectile in the flock about the others.
		for ( i = 0; i < LoadedShotCount; i++ )
		{
			if ( FiredRockets[i] != None )
			{
				FiredRockets[i].bCurl = bCurl;
				FiredRockets[i].FlockIndex = FlockIndex;

				j=0;
				for ( k=0; k<LoadedShotCount; k++ )
				{
					if ( (i != k) && (FiredRockets[k] != None) )
					{
						FiredRockets[i].Flock[j] = FiredRockets[k];
						j++;
					}
				}
				bCurl = !bCurl;
				if ( WorldInfo.NetMode != NM_DedicatedServer )
				{
					FiredRockets[i].SetTimer(0.1, true, 'FlockTimer');
				}
			}
		}
	}
}

/**
 * If we are locked on, we need to set the Seeking projectiles LockedTarget.
 */

simulated function Projectile ProjectileFire()
{
	local Projectile SpawnedProjectile;

	SpawnedProjectile = super.ProjectileFire();
	if (bLockedOnTarget && UTProj_SeekingRocket(SpawnedProjectile) != None)
	{
		UTProj_SeekingRocket(SpawnedProjectile).Seeking = LockedTarget;
	}

	return SpawnedProjectile;
}

/**
 * We override GetProjectileClass to swap in a Seeking Rocket if we are locked on.
 */

function class<Projectile> GetProjectileClass()
{
	if (CurrentFireMode == 1 && LoadedFireMode == RFM_Grenades)
	{
		return GrenadeClass;
	}
	else if (bLockedOnTarget)
	{
		return SeekingRocketClass;
	}
	else if ( LoadedShotCount > 1 )
	{
		return LoadedRocketClass;
	}
	else
	{
		return WeaponProjectiles[CurrentFireMode];
	}
}

simulated function PlayFiringSound()
{
	if (CurrentFireMode == 1 && LoadedFireMode == RFM_Grenades)
	{
		MakeNoise(1.0);
		WeaponPlaySound(GrenadeFireSound);
	}
	else
	{
		Super.PlayFiringSound();
	}
}

simulated function HideRocketAmmo()
{
	HideRocket('RocketAmmoScale', true);
}

//Give the name of a skelcontrol, turn the control on or off which sets BoneScale to 0 or 1
simulated function HideRocket(name RocketName, bool ShouldHide)
{
	local SkelControlSingleBone SkelControl;
	//Choose to hide or unhide a rocket by name
	SkelControl = SkelControlSingleBone(SkeletonFirstPersonMesh.FindSkelControl(RocketName));
	if(SkelControl != none)
	{
		//`log(ShouldHide?"Hiding"@RocketName:"Unhiding"@RocketName);
		SkelControl.SetSkelControlStrength(ShouldHide?1.0f:0.0f, 0.0f);
	}
}

//Properly removes from view any rocket geometry that shouldn't be there based on ammo count
simulated function UpdateAmmoVisibility()
{
	if (Instigator.IsHumanControlled() && Instigator.IsLocallyControlled())
	{
		if (AmmoCount > 2)
		{
			ClearTimer('HideRocketAmmo');
			HideRocket('RocketAmmoScale', false);
			if (bIsAnyAmmoHidden)
			{
				//Unhide all rockets
				HideRocket('SpikeRocketScale', false);
				HideRocket('BerthaRocketScale', false);
				HideRocket('MackRocketScale', false);
				bIsAnyAmmoHidden = false;
			}
		}
		else
		{
			if (CurrentFireMode == 0)
			{
				if (AmmoCount == 1)
				{
					//Hide the rocket after it loads the last bullet
					SetTimer(0.48, false, 'HideRocketAmmo');
				}
				else if (AmmoCount == 0)
				{
					//Hide all rockets
					HideRocket('SpikeRocketScale', true);
					HideRocket('BerthaRocketScale', true);
					HideRocket('MackRocketScale', true);
					bIsAnyAmmoHidden = true;
				}
			}
			else if (!bIsAnyAmmoHidden)
			{
				//This code is called once during BeginState of WeaponLoadAmmo
				if ((LoadedShotCount + AmmoCount) == 3)
				{
					//We've done our check, mark this true even if we didn't hide anything
					//prevents subsequent calls to this function during loading
					bIsAnyAmmoHidden = true;
				}
				else if ((LoadedShotCount + AmmoCount) == 2)
				{
					//Hide 'Spike' rocket
					HideRocket('SpikeRocketScale', true);
					bIsAnyAmmoHidden = true;
				}
				else if ((LoadedShotCount + AmmoCount) == 1)
				{
					//Hide 'Bertha' and 'Spike' rockets
					HideRocket('SpikeRocketScale', true);
					HideRocket('BerthaRocketScale', true);
					bIsAnyAmmoHidden = true;
				}
				else if ((LoadedShotCount + AmmoCount) == 0)
				{
					//Hide all rockets
					HideRocket('SpikeRocketScale', true);
					HideRocket('BerthaRocketScale', true);
					HideRocket('MackRocketScale', true);
					bIsAnyAmmoHidden = true;
				}
			}
		}
	}
}

//Overridden to update the ammo visibility in single fire mode
function int AddAmmo( int Amount )
{
	local int NewAmmoCount;
	NewAmmoCount = super.AddAmmo(Amount);

	UpdateAmmoVisibility();
	return NewAmmoCount;
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'AmmoCount')
	{
		UpdateAmmoVisibility();
	}

	Super.ReplicatedEvent(VarName);
}


/*********************************************************************************************
 * States
 *********************************************************************************************/


/*********************************************************************************************
 * State WeaponLoadAmmo
 * In this state, ammo will continue to load up until MaxLoadCount has been reached.  It's
 * similar to the firing state
 *********************************************************************************************/

simulated state WeaponLoadAmmo
{
	//Overridden here to not call UpdateAmmoVisibility() because it was already handled in BeginState()
	function int AddAmmo( int Amount )
	{
		return Super.AddAmmo(Amount);
	}

	/**
	 * We override BeginFire to detect a normal fire press and switch in to flocked mode
	 */
   	simulated function BeginFire(byte FireModeNum)
	{
		if (FireModeNum == 0)
		{
			LoadedFireMode = ERocketFireMode1((int(LoadedFireMode) + 1) % RFM_Max);
			WeaponPlaySound(AltFireModeChangeSound);
		}

		Global.BeginFire(FireModeNum);
	}

	simulated function TimeWeaponFiring( byte FireModeNum )
	{
		SetTimer( GetFireInterval(1) , false, 'RefireCheckTimer' );
	}

	simulated function WeaponEmpty()
	{
		if ( Instigator.IsLocallyControlled() )
		{
			StopWeaponAnimation();
			GotoState('WeaponWaitingForFire');
		}
		else
		{
			Global.WeaponEmpty();
		}
	}

	/**
	 * Adds a projectile to the queue and uses up some ammo.  In Addition, it plays
	 * a sound so that other pawns in the world can hear it.
	 */
	simulated function AddProjectile()
	{
		WeaponPlaySound(AltFireSndQue[Clamp(LoadedShotCount, 0, AltFireSndQue.Length - 1)]);

		// Play the que animation
		if ( Instigator.IsFirstPerson() && (LoadedShotCount < 3) )
		{
			PlayWeaponAnimation( LoadUpAnimList[LoadedShotCount], AltFireQueueTimes[LoadedShotCount]);

		}
		TimeWeaponFiring(1);
	}

	simulated function RefireCheckTimer()
	{
		local UTBot B;

		// AI checks which of the extra fire modes it wants here
		B = UTBot(Instigator.Controller);
		if (B != None && B.Enemy != None)
		{
			// when retreating, we want grenades
			// otherwise, if close, spiral; if not, spread
			if (B.IsRetreating() || B.IsInState('StakeOut'))
			{
				LoadedFireMode = RFM_Grenades;
			}
			else if (VSize(B.Pawn.Location - B.Enemy.Location) < 1000.0)
			{
				LoadedFireMode = RFM_Spiral;
			}
			else
			{
				LoadedFireMode = RFM_Spread;
			}
		}

		//Check if we need to hide the rocket model
		if(AmmoCount == 2 && Instigator.IsHumanControlled() && Instigator.IsLocallyControlled())
		{
			//If there is only one rocket left, we're about to take it, hide the ammo
			HideRocket('RocketAmmoScale', true);
		}

		// If we have ammo, load a shot
		if ( HasAmmo(CurrentFireMode) && (LoadedShotCount < 3) )
		{
			// Play Loading Sound
			MakeNoise(0.1);

			// Add the Rocket
			WeaponPlaySound(WeaponLoadedSnd);
			WeaponPlaySound(RocketLoadedSound);
			LoadedShotCount++;
			ConsumeAmmo(CurrentFireMode);
		}

		// Figure out what to do now

		// If we have released AltFire - Just fire the load
		if ( !StillFiring(1) )
		{
			if ( LoadedShotCount > 0 )
			{
				// We have to check to insure we are in the proper state here.  StillFiring() may cause
				// bots to end their firing sequence.  This will cause and EndFire() call to be made which will
				// also fire off the load and switch to the animation state prematurely.

				if ( IsInState('WeaponLoadAmmo') )
				{
					WeaponFireLoad();
				}
			}
			else
			{
				GotoState('Active');
			}
		}
		else if ( !HasAmmo(CurrentFireMode) || LoadedShotCount >= MaxLoadCount )
		{
			GotoState('WeaponWaitingForFire');
		}
		else
		{
			AddProjectile();
		}
	}

	/**
	 * We need to override EndFire so that we can correctly fire off the
	 * current load if we have any.
	 */
	simulated function EndFire(byte FireModeNum)
	{
		local float MinTimerTarget, TimerCount;
		// Pass along to the global to handle everything
		Global.EndFire(FireModeNum);

		if (FireModeNum == 1 && LoadedShotCount>0)
		{
			MinTimerTarget = GetTimerRate('RefireCheckTimer') * WaitToFirePct;
			TimerCount = GetTimerCount('RefireCheckTimer');
			if (TimerCount < MinTimerTarget)
			{
				WeaponFireLoad();
			}
		}
	}

	/**
	 * Insure that the LoadedShotCount is 0 when we leave this state
	 */
	simulated function EndState(Name NextStateName)
	{
		ClearTimer('RefireCheckTimer');
		super.EndState(NextStateName);
	}

	simulated function bool IsFiring()
	{
		return true;
	}

	simulated function bool TryPutdown()
	{
		bWeaponPutDown = true;
		return true;
	}
	simulated function DrawWeaponCrosshair( Hud HUD )
	{
		DrawLFMData(HUD);
	}

	/**
	 * Initialize the alt fire state
	 */
	simulated function BeginState(Name PreviousStateName)
	{
		LoadedFireMode = RFM_Spread;
		//Before we begin this mode, hide/show any ammo
		UpdateAmmoVisibility();
		Super.BeginState(PreviousStateName);
	}


	/** You can run around loading up rockets ready to fire them! **/
	simulated function bool CanViewAccelerationWhenFiring()
	{
		return TRUE;
	}


Begin:
	AddProjectile();
}

/*********************************************************************************************
 * State WeaponWaitingForFire
 * After the weapon has fully loaded, it will enter this state and wait for a short period of
 * time before auto-firing
 *********************************************************************************************/

simulated state WeaponWaitingForFire
{
	simulated function DrawWeaponCrosshair( Hud HUD )
	{
		DrawLFMData(HUD);
	}

	simulated function WeaponEmpty()
	{}

	/**
	 * We override BeginFire to detect a normal fire press and switch in to flocked mode
	 */
   	simulated function BeginFire(byte FireModeNum)
	{
		if (FireModeNum == 0)
		{
			LoadedFireMode = ERocketFireMode1((int(LoadedFireMode) + 1) % RFM_Max);
			WeaponPlaySound(AltFireModeChangeSound);
		}
		global.BeginFire(FireModeNum);
	}

	/**
	 * Insure that the LoadedShotCount is 0 when we leave this state
	 */
	simulated function EndState(Name NextStateName)
	{
		ClearTimer('ForceFireTimer');
		super.EndState(NextStateName);
	}

	/**
	 * Set the Grace Period
	 */
	simulated function BeginState(Name PrevStateName)
	{
		SetTimer(GracePeriod, false, 'ForceFireTimer');
		Super.BeginState(PrevStateName);
	}

	/**
	 * If we get an EndFire - Exit Immediately
	 */
	simulated function EndFire(byte FireModeNum)
	{
		if (FireModeNum == 1 && LoadedShotCount>0)
		{
			WeaponFireLoad();
		}
		Global.EndFire(FireModeNum);
	}

	/**
	 * Fire the load
	 */

	simulated function ForceFireTimer()
	{
		WeaponFireLoad();
	}

	simulated function bool IsFiring()
	{
		return true;
	}

	simulated function bool TryPutdown()
	{
		bWeaponPutDown = true;
		return true;
	}
}

/*********************************************************************************************
 * State WeaponPlayingFire
 * In this state, the weapon will have already spawned the projectiles and is just playing out
 * the firing animations.  When finished, it returns to the Active state
 *
 * We use 2 animations, one for firing and one of spin down in order to allow better tweaking of
 * the timing.
 *********************************************************************************************/

simulated State WeaponPlayingFire
{
	/**
	 * Choose the proper "Firing" animation to play depending on the number of shots loaded.
	 */
	simulated function PlayFiringAnim()
	{
		local int index;

		index = max(LoadedShotCount,1);
		PlayWeaponAnimation( WeaponAltFireLaunch[Index-1], AltFireLaunchTimes[Index-1]);
		SetTimer(AltFireLaunchTimes[Index-1],false,'FireAnimDone');
	}

	/**
	 * Choose the proper "We are done firing, reset" animation to play depending on the number of shots loaded
	 */
	simulated function PlayFiringEndAnim()
	{
		local int index;

		index = max(LoadedShotCount,1);
		PlayWeaponAnimation( WeaponAltFireLaunchEnd[Index-1], AltFireEndTimes[Index-1]);
		SetTimer(AltFireEndTimes[Index-1],false,'FireAnimEnded');
	}

	/**
	 * When the Fire animation is done chain the ending animation.
	 */
	simulated function FireAnimDone()
	{
		PlayFiringEndAnim();

	}
	/**
	 * When the End-Fire animation is done return to the active state
	 */
	simulated function FireAnimEnded()
	{
		// if switching to another weapon, abort firing and put down right away
		if( bWeaponPutDown )
		{
			PutDownWeapon();
			return;
		}

		// if out of ammo, then call weapon empty notification
		if( !HasAnyAmmo() )
		{
			//Switches to 'Active' state and then on to 'WeaponPutDown'
			WeaponEmpty();
		}
		else
		{
		    Gotostate('Active');
		}
	}

	/**
	 * Clean up the weapon.  Reset the shot count, etc
	 */
	simulated function EndState(Name NextStateName)
	{
		LoadedShotCount = 0;
		ClearFlashCount();
		ClearFlashLocation();

		// Clear out the other 2 timers
		ClearTimer('FireAnimDone');
		ClearTimer('FireAnimEnded');

		// Reset the flocked flag
		LoadedFireMode = RFM_Spread;
	}

	simulated function bool IsFiring()
	{
		return true;
	}

Begin:
	WeaponPlaySound(WeaponLoadedSnd);
	PlayFiringAnim();
}


simulated state WeaponFiring
{
	simulated event ReplicatedEvent(name VarName)
	{
		//Overridden here to update the ammo visibility (UTWeapon::WeaponFiring::ReplicatedEvent just returns)
		if ( VarName == 'AmmoCount' && !HasAnyAmmo() )
		{
			UpdateAmmoVisibility();
		}

		Super.ReplicatedEvent(VarName);
	}
}

simulated state Active
{
	simulated event BeginState(name PreviousStateName)
	{
		// reset firemode on Instigator so that it will replicate again if we start another load
		// (attachment uses this to play load anim)
		if (Instigator != None)
		{
			Instigator.SetFiringMode(0);
		}

		Super.BeginState(PreviousStateName);
	}
}

defaultproperties
{
	WeaponColor=(R=255,G=0,B=0,A=255)
	FireInterval(0)=+1.0
	FireInterval(1)=+1.05
	PlayerViewOffset=(X=0.0,Y=0.0,Z=0.0)

	Begin Object Name=FirstPersonMesh
		SkeletalMesh=SkeletalMesh'WP_RocketLauncher.Mesh.SK_WP_RocketLauncher_1P'
		PhysicsAsset=None
		AnimTreeTemplate=AnimTree'WP_RocketLauncher.Anims.AT_WP_RocketLauncher_1P_Base'
		AnimSets(0)=AnimSet'WP_RocketLauncher.Anims.K_WP_RocketLauncher_1P_Base'
		Translation=(X=0,Y=0,Z=0)
		Rotation=(Yaw=0)
		scale=1.0
		FOV=60.0
	End Object
	SkeletonFirstPersonMesh = FirstPersonMesh;
	AttachmentClass=class'UTGame.UTAttachment_RocketLauncher'

	// Pickup staticmesh
	Begin Object Name=PickupMesh
		SkeletalMesh=SkeletalMesh'WP_RocketLauncher.Mesh.SK_WP_RocketLauncher_3P'
	End Object

	WeaponLoadedSnd=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_Load_Cue'
	WeaponFireSnd[0]=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_Fire_Cue'
	WeaponFireSnd[1]=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_Fire_Cue'
	WeaponEquipSnd=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_Raise_Cue'
	AltFireModeChangeSound=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_AltModeChange_Cue'
	RocketLoadedSound=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_RocketLoaded_Cue'
	GrenadeFireSound=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_GrenadeFire_Cue'

	AltFireSndQue(0)=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_AltFireQueue1_Cue'
	AltFireSndQue(1)=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_AltFireQueue2_Cue'
	AltFireSndQue(2)=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_AltFireQueue3_Cue'

	LockAcquiredSound=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_SeekLock_Cue'
	LockLostSound=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_SeekLost_Cue'

	WeaponProjectiles(0)=class'UTProj_Rocket'
	WeaponProjectiles(1)=class'UTProj_Rocket'
	LoadedRocketClass=class'UTProj_LoadedRocket'

	GrenadeClass=class'UTProj_Grenade'

	FireOffset=(X=20,Y=12,Z=-5)

	MaxDesireability=0.78
	AIRating=+0.78
	CurrentRating=+0.78
	bInstantHit=false
	bSplashJump=true
	bRecommendSplashDamage=true
	bSniping=false
	ShouldFireOnRelease(0)=0
	ShouldFireOnRelease(1)=1
	InventoryGroup=8
	GroupWeight=0.5

	PickupSound=SoundCue'A_Pickups.Weapons.Cue.A_Pickup_Weapons_Rocket_Cue'

	AmmoCount=9
	LockerAmmoCount=18
	MaxAmmoCount=30

	SeekingRocketClass=class'UTProj_SeekingRocket'

	AltFireQueueTimes(0)=0.40
	AltFireQueueTimes(1)=0.96
	AltFireQueueTimes(2)=0.96
	AltFireLaunchTimes(0)= 0.51
	AltFireLaunchTimes(1)= 0.51
	AltFireLaunchTimes(2)= 0.51
	AltFireEndTimes(0)=0.44
	AltFireEndTimes(1)=0.44
	AltFireEndTimes(2)=0.44

	MaxLoadCount=3
	SpreadDist=1000
	FiringStatesArray(1)=WeaponLoadAmmo
	WeaponFireTypes(0)=EWFT_Projectile
	WeaponFireTypes(1)=EWFT_Projectile
	WaitToFirePct=0.85
	GracePeriod=0.96

	MuzzleFlashSocket=MuzzleFlashSocketA
	MuzzleFlashSocketList(0)=MuzzleFlashSocketA
	MuzzleFlashSocketList(1)=MuzzleFlashSocketC
	MuzzleFlashSocketList(2)=MuzzleFlashSocketB

	MuzzleFlashPSCTemplate=WP_RocketLauncher.Effects.P_WP_RockerLauncher_Muzzle_Flash
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'UTGame.UTRocketMuzzleFlashLight'


	ConsoleLockAim=0.992
	LockRange=8000
	LockAim=0.997
	LockChecktime=0.1
	LockAcquireTime=1.1
	LockTolerance=0.2

	IconX=460
	IconY=34
	IconWidth=51
	IconHeight=38

	EquipTime=+0.6

	GrenadeSpreadDist=300

	JumpDamping=0.75

	LockerRotation=(pitch=0,yaw=0,roll=-16384)
	IconCoordinates=(U=131,V=379,UL=129,VL=50)
	CrossHairCoordinates=(U=128,V=64,UL=64,VL=64)
	WeaponPutDownSnd=SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_Lower_Cue'

	QuickPickGroup=2
	QuickPickWeight=0.9

	LoadedIconCoords[0]=(U=0,V=384,UL=63,VL=63)
	LoadedIconCoords[1]=(U=63,V=384,UL=63,VL=63)
	LoadedIconCoords[2]=(U=126,V=384,UL=63,VL=63)

	LoadUpAnimList[0]=WeaponAltFireQueue1
	LoadUpAnimList[1]=WeaponAltFireQueue2
	LoadUpAnimList[2]=WeaponAltFireQueue3

	WeaponAltFireLaunch[0]=WeaponAltFireLaunch1
	WeaponAltFireLaunch[1]=WeaponAltFireLaunch2
	WeaponAltFireLaunch[2]=WeaponAltFireLaunch3

	WeaponAltFireLaunchEnd[0]=WeaponAltFireLaunch1End
	WeaponAltFireLaunchEnd[1]=WeaponAltFireLaunch2End
	WeaponAltFireLaunchEnd[2]=WeaponAltFireLaunch3End

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=90,RightAmplitude=50,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.200)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1
}
