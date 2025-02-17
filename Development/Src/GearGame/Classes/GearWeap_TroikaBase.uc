/**
 * Troika Weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_TroikaBase extends GearWeapon
	config(Weapon)
	abstract;

/** Time it takes to warm up the troika before firing */
var() protected const config float	SpinUpTime;
var() protected const config float	SpinDownTime;
var() protected const config float	SpinDownTime_ActiveCooling;
var protected transient bool		bSpinningUp;

/** Time it takes to reload troika with a loader */
var float ReloadDurationWithLoader;


/** Sound to play when ammo feed starts */
var protected const SoundCue BarrelSpinningStartCue;
/** AudioComponent for the AmmoFeedStart sound. */
var protected AudioComponent BarrelSpinningStartAC;
/** Looping SoundCue for ammo feeder. */
var protected const SoundCue BarrelSpinningLoopCue;
/** The AudioComponent that is used for the looping ammo loading sound **/
var protected AudioComponent BarrelSpinningLoopAC;
/** Sound to play when ammo feed stops */
var protected const SoundCue BarrelSpinningStopCue;


/** Looping SoundCue for firing. */
var protected const SoundCue FireLoopCue;
/** Looping SoundCue for firing. */
var protected const SoundCue FireLoopCue_Player;
/** The AudioComponent that is used for the looping shot sound **/
var protected AudioComponent FireLoopAC;


/** final shot echo, play when firing stops.  nonlooping. */
var const protected SoundCue FireStopCue;
var const protected SoundCue FireStopCue_Player;

var const protected SoundCue FireStopCue_Overheat;


/** The PS to play when reloading which is when the barrels overheat **/
var ParticleSystemComponent PSC_Reloading;


/** Longest sustained firing burst before overheating, in seconds. */
var() protected const config float	MaxFiringTimeBeforeOverheat;

/** When gun overheats, how long you must wait until the gun begins to cool down again. */
var() protected const config float	OverheatPenaltyTime;

var protected transient bool		bOverheated;

/** How long it takes the barrel to cool down from fully heated, in seconds. */
var() protected const config float	CooldownTime;
/** How long it takes the barrel to cool down from fully heated while ActiveCooling(tm) is in effect, in seconds. */
var() protected const config float	CooldownTimeActiveCooling;

var protected transient bool bActivelyCooling;
var protected const SoundCue ActiveCoolingLoopCue;
var protected const SoundCue ActiveCoolingStartCue;
var protected const SoundCue ActiveCoolingStopCue;
var protected transient AudioComponent	ActiveCoolingSteamAudioLoop;

var protected ParticleSystemComponent	PSC_ActiveCoolingSteam;


/** Smoke emitter at the muzzle */
var ParticleSystemComponent		PSC_BarrelSmoke;
/** Tracks how long this weapon has been firing, in seconds. */
var protected transient float	FiringDuration;
/** Length of time the weapon needs to fire before we can turn on the barrel smoke emiiter. */
var() protected const float		BarrelSmokeMinFiringDuration;


var protected const SoundCue HeatBuildupCue;
var protected AudioComponent HeatBuildUpAC;

var protected const SoundCue CasingImpactCue;


/** whether or not to spawn a decal **/
var bool bShouldSpawnDecal;

replication
{
	if (bNetOwner)
		bOverheated;
}

/** Internal */
simulated protected function OverheatPenaltyExpired()
{
	bOverheated = FALSE;
}



simulated event PostBeginPlay()
{
	local Turret_TroikaBase Troika;

	Troika = Turret_TroikaBase(Instigator);
	if( Troika != None )
	{
		Troika.Mesh.AttachComponentToSocket(PSC_Reloading, Troika.MuzzleSocketName);
		Troika.Mesh.AttachComponentToSocket(PSC_BarrelSmoke, Troika.MuzzleSocketName);
		Troika.Mesh.AttachComponentToSocket(PSC_ActiveCoolingSteam, Troika.MuzzleSocketName);		// @fixme, use different socket, not muzzle
	}

	super.PostBeginPlay();
}


/** Troikas do not get active reloads **/
simulated event LocalActiveReloadSuccess( bool bDidSuperSweetReload );
simulated function ActiveReloadSuccess(bool bDidSuperSweetReload);
simulated function FailedActiveReload(bool bFailedActiveReload);


simulated event AttachMuzzleEffectsComponents(SkeletalMeshComponent SkelMesh)
{
	local Turret_TroikaBase	Troika;

	if( SkelMesh != None )
	{
		Troika = Turret_TroikaBase(SkelMesh.Owner);
	}

	if( Troika != None )
	{
		// attach muzzle flash emitter, and position to fire offset
		if( MuzFlashEmitter != None && !SkelMesh.IsComponentAttached(MuzFlashEmitter) )
		{
			SkelMesh.AttachComponentToSocket(MuzFlashEmitter, Troika.MuzzleSocketName);
			HideMuzzleFlashEmitter();
		}

		// Muzzle Flash dynamic light
		if( MuzzleFlashLight != None && !SkelMesh.IsComponentAttached(MuzzleFlashLight) )
		{
			SkelMesh.AttachComponentToSocket(MuzzleFlashLight, Troika.MuzzleSocketName);
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

		// Muzzle Flash dynamic light
		if( MuzzleFlashLight != None )
		{
			AttachComponent(MuzzleFlashLight);
		}
	}
}


simulated function PerformWeaponAttachment(SkeletalMeshComponent MeshCpnt, optional Name BoneName)
{
	AttachMuzzleEffectsComponents(MeshCpnt);
}


simulated function DetachWeapon()
{
	local SkeletalMeshComponent	PawnMesh;

	Super.DetachWeapon();

	//scripttrace();

	if( Instigator != None )
	{
		PawnMesh = Instigator.Mesh;
	}

	if( PawnMesh != None )
	{
		// shell eject particle system
		if( MuzFlashEmitter != None && PawnMesh.IsComponentAttached(MuzFlashEmitter) )
		{
			PawnMesh.DetachComponent(MuzFlashEmitter);
		}

		if( MuzzleFlashLight != None && PawnMesh.IsComponentAttached(MuzzleFlashLight) )
		{
			PawnMesh.DetachComponent(MuzzleFlashLight);
		}
	}
}

simulated function FireModeUpdated(byte FiringMode, bool bViaReplication)
{
	if( FiringMode == FIREMODE_CHARGE )
	{
		ChargeStarted();
	}
	else if( FiringMode == FIREMODE_FAILED )
	{
		ChargeAborted();
	}

	Super.FireModeUpdated(FiringMode, bViaReplication);
}


simulated function ChargeStarted()
{
	// play the start-firing audio
	BarrelSpinningStartAC = GearWeaponPlaySoundLocalEx(BarrelSpinningStartCue,, BarrelSpinningStartAC);

}

simulated function ChargeAborted()
{
	EndFiringSounds();
}


simulated function EndFiringSounds()
{
	// stop any firing or spinning up sounds
	if (BarrelSpinningLoopAC != None)
	{
		BarrelSpinningLoopAC.FadeOut(0.2f, 0.0f);
	}
	if ( (FireLoopAC != None) && FireLoopAC.IsPlaying() )
	{
		FireLoopAC.FadeOut(0.2f, 0.0f);

		/** last shot echo/tail off */
		if (!bOverheated)
		{
			GearWeaponPlaySoundLocal(FireStopCue, FireStopCue_Player,, 1.f);
		}
		GearWeaponPlaySoundLocal(CasingImpactCue);
	}
	GearWeaponPlaySoundLocal(BarrelSpinningStopCue);

	if (HeatBuildUpAC != None)
	{
		HeatBuildUpAC.FadeOut(0.2f, 0.0f);
	}
	if (BarrelSpinningStartAC != None)
	{
		BarrelSpinningStartAC.FadeOut(0.2f, 0.0f);
	}
}


simulated function WeaponFired( byte FiringMode, optional vector HitLocation )
{
	//local Turret_TroikaBase Troika;
	Super.WeaponFired( FiringMode, HitLocation );

	if( !bSuppressAudio && (FireLoopAC == None || !FireLoopAC.IsPlaying()) )
	{
		FireLoopAC = GearWeaponPlaySoundLocalEx(FireLoopCue, FireLoopCue_Player, FireLoopAC);
		BarrelSpinningLoopAC = GearWeaponPlaySoundLocalEx(BarrelSpinningLoopCue,, BarrelSpinningLoopAC);
	}

	if (!PSC_BarrelSmoke.bSuppressSpawning)
	{
		PSC_BarrelSmoke.DeactivateSystem();
	}
}

/** Overridden to allow 'WeaponFiring' to be a valid fire state since it's not in the FirignStatesArray for this class **/
protected function bool IsFireState( name StateName )
{
	return (StateName == 'WeaponFiring' ||
			Super.IsFireState(StateName));
}


simulated function WeaponStoppedFiring(byte FiringMode)
{
	Super.WeaponStoppedFiring(FiringMode);
	EndFiringSounds();

	// maybe turn on barrel smoke
	if (FiringDuration > BarrelSmokeMinFiringDuration)
	{
		PSC_BarrelSmoke.ActivateSystem(TRUE);
	}
}


// no reload anim right now
simulated function PlayWeaponReloading();


/**
 * If troika has a dedicated loader, then reloading is much faster.
 */
simulated function float GetReloadDuration()
{
	if( Turret_TroikaBase(Instigator).Loader != None )
	{
//		`log("reloading with loader"@ReloadDurationWithLoader);

		return ReloadDurationWithLoader;
	}

//	`log("reloading alone"@ReloadDuration);

	return ReloadDuration;
}

/**
 *	Troika can never be reloaded
 */
simulated function bool CanReload()
{
	return FALSE;
}

/**
 * Extended so we don't relying on active reload to exit state.
 */
simulated state Reloading
{
	ignores ForceReload;

	simulated function BeginState(Name PreviousStateName)
	{
		super.BeginState(PreviousStateName);
		SetTimer( GetReloadDuration(), FALSE, nameof(EndOfReloadTimer) );

		PSC_Reloading.ActivateSystem();
	}

	simulated function EndState(Name PreviousStateName)
	{
		super.EndState(PreviousStateName);
		PSC_Reloading.DeActivateSystem();
	}
}

simulated function InstantFire()
{
	local Turret_TroikaBase	Troika;

	Troika = Turret_TroikaBase(Instigator);

	// Disabling collision on driver for weapon trace.
	if( Troika != None && Troika.Driver != None )
	{
		Troika.Driver.Mesh.SetTraceBlocking(FALSE, Troika.Driver.Mesh.BlockNonZeroExtent);
	}

	Super.InstantFire();

	if( Troika != None && Troika.Driver != None )
	{
		Troika.Driver.Mesh.SetTraceBlocking(TRUE, Troika.Driver.Mesh.BlockNonZeroExtent);
	}
}


/** Do a little clean up.  Audio Components to none!.  **/
simulated event Destroyed()
{
	if( BarrelSpinningLoopAC != none )
	{
		BarrelSpinningLoopAC.FadeOut( 0.1f, 0.0f );
		BarrelSpinningLoopAC = None;
	}

	if( FireLoopAC != none )
	{
		FireLoopAC.FadeOut( 0.1f, 0.0f );
		FireLoopAC = None;
	}

	if( BarrelSpinningStartAC != none )
	{
		BarrelSpinningStartAC.FadeOut( 0.1f, 0.0f );
		BarrelSpinningStartAC = None;
	}

	Super.Destroyed();
}


simulated function WarmedUp();

simulated state WarmingUp
{
	simulated function BeginState(Name PreviousStateName)
	{
		SetTimer( SpinUpTime, FALSE, nameof(WarmedUp) );
		SetCurrentFireMode(FIREMODE_CHARGE);
	}

	simulated function WarmedUp()
	{
		SetCurrentFireMode(0);
		GotoState('WeaponFiring');
	}

	simulated function EndState(Name NextStateName)
	{
		ClearTimer('WarmedUp');
	}

	simulated function EndFire(byte FireModeNum)
	{
		Global.EndFire(FireModeNum);

		// If fire is released, go back to active state
		GotoState('Active');
		SetCurrentFireMode(FIREMODE_FAILED);
	}
	simulated function bool IsFiring()
	{
		return FALSE;
	}
}

simulated function Tick(float DeltaTime)
{
	local Vector MuzzleLoc;
	if( Turret_TroikaBase(Instigator) != None && (FireLoopAC != None || BarrelSpinningStartAC != None || BarrelSpinningLoopAC != None))
	{
		Turret_TroikaBase(Instigator).Mesh.GetSocketWorldLocationAndRotation(Turret_TroikaBase(Instigator).MuzzleSocketName, MuzzleLoc);
		if (FireLoopAC != None)
		{
			FireLoopAC.Location = MuzzleLoc;
		}
		if (BarrelSpinningLoopAC != None)
		{
			BarrelSpinningLoopAC.Location = MuzzleLoc;
		}
		if (BarrelSpinningStartAC != None)
		{
			BarrelSpinningStartAC.Location = MuzzleLoc;
		}
	}


	// keep track of firing duration and barrel heat
	if (IsFiring())
	{
		FiringDuration += DeltaTime;
		CurrentBarrelHeat += DeltaTime / MaxFiringTimeBeforeOverheat;
		CurrentBarrelHeat = FMin(CurrentBarrelHeat, 1.f);

		if ( (CurrentBarrelHeat >= 1.f) && !bOverheated )
		{
			// overheated!  stop firing and don't allow a restart for some time
			bOverheated = TRUE;
			SetTimer( OverheatPenaltyTime,, nameof(OverheatPenaltyExpired) );
			WeaponPlaySound(FireStopCue_Overheat);
			StopFire(0);

			GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_GatlingOverheated, Instigator, None, 0.35f);
		}
	}
	else
	{
		FiringDuration = 0.f;
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

simulated function BeginFire(byte FireModeNum)
{
	if ( !bOverheated )
	{
		Super.BeginFire(FireModeNum);
	}
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


/**
 * We only spawn 1/2 decals.
 **/
simulated function SpawnImpactDecal( const class<GearDamageType> GearDmgType, const out ImpactInfo Impact, PhysicalMaterial PhysMaterial )
{
	if( bShouldSpawnDecal )
	{
		Super.SpawnImpactDecal( GearDmgType, Impact, PhysMaterial );
	}

	bShouldSpawnDecal = !bShouldSpawnDecal;
}


/** this weapon has no team based colors **/
simulated function SetMaterialBasedOnTeam( MeshComponent TheMesh, int TeamNum );

simulated function bool IsHeatLimited()
{
	return TRUE;
}

/** Returns the number of ammo icons this weapon will display in a clip */
simulated function float GetAmmoIconCount()
{
	return 0;
}

reliable server function ServerToggleActiveCooling(bool bActive)
{
	if (bActive != bActivelyCooling)
	{
		if (!bActive)
		{
			EndActiveCooling();
		}
		else
		{
			BeginActiveCooling();
		}
	}
}

simulated protected function BeginActiveCooling()
{
	bActivelyCooling = TRUE;
	ServerToggleActiveCooling(TRUE);

	// adjust to the AC spindown rate
//	CurrentBarrelRotAccel = CurrentBarrelRotRate / SpinDownTime_ActiveCooling;

	ActiveCoolingSteamAudioLoop = GearWeaponPlaySoundLocalEx(ActiveCoolingLoopCue,, ActiveCoolingSteamAudioLoop, 0.3f);
	WeaponPlaySound(ActiveCoolingStartCue);

	PSC_ActiveCoolingSteam.ActivateSystem(TRUE);
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

		// turn off effects
		PSC_ActiveCoolingSteam.DeactivateSystem();

		//// adjust to the normal spindown rate
		//CurrentBarrelRotAccel = CurrentBarrelRotRate / SpinDownTime;

		// done
		bActivelyCooling = FALSE;
		ServerToggleActiveCooling(FALSE);
	}
}


defaultproperties
{
	FiringStatesArray(0)="WarmingUp"

	bInfiniteSpareAmmo=TRUE
	bWeaponCanBeReloaded=TRUE
	bCanThrow=FALSE

	ReloadDurationWithLoader=0.1f

	NoAmmoFireSoundDelay=0.25f

	InstantHitDamageTypes(0)=class'GDT_Troika'

	FireOffset=(X=0,Y=0,Z=0)

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(R=255,G=192,B=128,A=255)
		Radius=384
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.05f
	MuzzleLightPulseFreq=60
	MuzzleLightPulseExp=1.5
	bLoopMuzzleFlashLight=TRUE

	// shell case ejection emitter
	Begin Object Class=ParticleSystemComponent Name=PSC_Muzzle0
		bAutoActivate=FALSE
		TickGroup=TG_PostUpdateWork
	End Object
	MuzFlashEmitter=PSC_Muzzle0

	Begin Object Class=ParticleSystemComponent Name=PSC_Reloading0
	    bAutoActivate=FALSE
		TickGroup=TG_PostUpdateWork
	End Object
	PSC_Reloading=PSC_Reloading0

	TracerType=WTT_MinigunFastFiring

	bKillDuringLevelTransition=TRUE

	bShouldSpawnDecal=TRUE
    WeaponID=WC_Troika

	bAllowTracers=TRUE
	bUseMuzzleLocForPhysicalFireStartLoc=TRUE

	BarrelSmokeMinFiringDuration=2.f

	Begin Object Class=ParticleSystemComponent Name=ActiveCoolingSteam0
		bAutoActivate=FALSE
		TickGroup=TG_PostUpdateWork
	End Object
	PSC_ActiveCoolingSteam=ActiveCoolingSteam0

	Begin Object Class=ParticleSystemComponent Name=PSC_BarrelSmoke0
		bAutoActivate=FALSE
		TickGroup=TG_PostUpdateWork
	End Object
	PSC_BarrelSmoke=PSC_BarrelSmoke0
}
