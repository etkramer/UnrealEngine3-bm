/**
 * Base Brumak Weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_BrumakWeaponBase extends GearWeapon
	abstract;

/** This is a flag to say whether or not the brumak is still firing so we can loop the AmmoFeedLoop sound **/
var bool bNewFiringAttack;
var bool bRandomShotsFiring;

/** Time it takes to warm up before firing */
var() protected const config float	SpinUpTime;
var() protected const config float	SpinDownTime;
var() protected const config float	SpinDownTime_ActiveCooling;
var protected transient bool		bSpinningUp;


//// SOUNDS ////
var SoundCue AmmoFeedStart;
var SoundCue AmmoFeedLoop;
var SoundCue AmmoFeedStop;

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


simulated function BeginFire(byte FireModeNum)
{
	if( FireModeNum != 0 || !bOverheated )
	{
		Super.BeginFire( FireModeNum );
	}
}

simulated function WarmedUp();

simulated state WarmingUp
{
	simulated function BeginState( Name PreviousStateName )
	{
		// no spinup time for anyone
		WarmedUp();

/*		if( AIController == None )
		{
			SetTimer( SpinUpTime, FALSE, nameof(WarmedUp) );
			SetCurrentFireMode( FIREMODE_CHARGE );
		}
		else
		{
			WarmedUp();
		}
*/
	}

	simulated function WarmedUp()
	{
		SetCurrentFireMode( 0 );
		GotoState( 'WeaponFiring' );
	}

	simulated function EndState( Name NextStateName )
	{
		ClearTimer( 'WarmedUp' );
	}

	simulated function EndFire( byte FireModeNum )
	{
		Global.EndFire( FireModeNum );

		// If fire is released, go back to active state
		GotoState('Active');
		SetCurrentFireMode(FIREMODE_FAILED);
	}
	simulated function bool IsFiring()
	{
		return FALSE;
	}
}

simulated state Active
{
	simulated function BeginState( Name PreviousStateName )
	{
		local int i;

		if( Role == ROLE_Authority )
		{
			// Cache a reference to the AI controller
			AIController = AIController(Instigator.Controller);
		}

		if( (Instigator.Controller != None || bDummyFireWeapon) && !bOverheated )
		{
			if( PendingFire(RELOAD_FIREMODE) || ShouldAutoReload() )
			{
				// then check for weapon reloading
				BeginFire(RELOAD_FIREMODE);
			}
			else if( InvManager != None )
			{
				// if either of the fire modes are pending, perform them
				for( i = 0; i < InvManager.PendingFire.Length; i++ )
				{
					if( PendingFire( i ) )
					{
						BeginFire( i );
					}
				}
			}
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
	if( BarrelSpinningLoopAC != None )
	{
		BarrelSpinningLoopAC.FadeOut( 0.2f, 0.0f );
	}
	if( FireLoopAC != None && FireLoopAC.IsPlaying() )
	{
		FireLoopAC.FadeOut( 0.2f, 0.0f );

		/** last shot echo/tail off */
		if( !bOverheated )
		{
			GearWeaponPlaySoundLocal( FireStopCue, FireStopCue_Player,, 1.f );
		}
	}
	GearWeaponPlaySoundLocal( BarrelSpinningStopCue );

	if( BarrelSpinningStartAC != None )
	{
		BarrelSpinningStartAC.FadeOut(0.2f, 0.0f);
	}
}


simulated function WeaponFired( byte FiringMode, optional vector HitLocation )
{
	Super.WeaponFired( FiringMode, HitLocation );

	if( FiringMode == 0 )
	{
		if( !bSuppressAudio && (FireLoopAC == None || !FireLoopAC.IsPlaying()) )
		{
			FireLoopAC = GearWeaponPlaySoundLocalEx(FireLoopCue, FireLoopCue_Player, FireLoopAC);
			BarrelSpinningLoopAC = GearWeaponPlaySoundLocalEx(BarrelSpinningLoopCue,, BarrelSpinningLoopAC);
		}
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

	if( FiringMode == 0 )
	{
		EndFiringSounds();
	}
}


// no reload anim right now
simulated function PlayWeaponReloading();


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
	local Vector MuzzleLoc;

	Super.Tick(DeltaTime);

	if( FireLoopAC != None || BarrelSpinningStartAC != None || BarrelSpinningLoopAC != None )
	{
		MuzzleLoc = GetMuzzleLoc();
		if( FireLoopAC != None )
		{
			FireLoopAC.Location = MuzzleLoc;
		}
		if( BarrelSpinningLoopAC != None )
		{
			BarrelSpinningLoopAC.Location = MuzzleLoc;
		}
		if( BarrelSpinningStartAC != None )
		{
			BarrelSpinningStartAC.Location = MuzzleLoc;
		}
	}

	if( IsHeatLimited() )
	{
		// keep track of firing duration and barrel heat
		if( IsFiring() )
		{
			CurrentBarrelHeat += DeltaTime / MaxFiringTimeBeforeOverheat;
			CurrentBarrelHeat = FMin(CurrentBarrelHeat, 1.f);

			if ( (CurrentBarrelHeat >= 1.f) && !bOverheated )
			{
				// overheated!  stop firing and don't allow a restart for some time
				bOverheated = TRUE;
				SetTimer( OverheatPenaltyTime,, nameof(OverheatPenaltyExpired) );
				StopFire(0);

				GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_GatlingOverheated, Instigator, None, 0.35f);
			}
		}
		else
		{
			if( bActivelyCooling )
			{
				CurrentBarrelHeat -= DeltaTime / CooldownTimeActiveCooling;
				if( CurrentBarrelHeat <= 0.f )
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
}

simulated function PlayWeaponAnimation(Name AnimName, float Rate, optional bool bLoop, optional SkeletalMeshComponent SkelMesh)
{
}

/** See if weapon should refire. */
simulated function bool ShouldRefire()
{
	local GearPawn_LocustBrumak Brumak;

	// Check with Brumak. If enemy is outside of FOV, weapon will stop firing.
	Brumak = GearPawn_LocustBrumak(GetBrumak());
	if( Brumak != None && !Brumak.ShouldRefireWeapon(Self) )
	{
		return FALSE;
	}

	return Super.ShouldRefire();
}

simulated function bool CanReload()
{
	return FALSE;
}

simulated function bool ShouldAutoReload()
{
	return FALSE;
}

function int GetBurstsToFire()
{
	return 1;
}

function GearPawn_LocustBrumakDriver GetDriver()
{
	return GearPawn_LocustBrumakDriver(Instigator);
}
simulated function GearPawn_LocustBrumakBase GetBrumak()
{
	local GearPawn_LocustBrumakBase Brumak;

	Brumak = GearPawn_LocustBrumakBase(Instigator);
	if( Brumak != None )
	{
		return Brumak;
	}
	if( Instigator != None )
	{
		return GearPawn_LocustBrumakBase(Instigator.Base);
	}
	return None;
}
function GearPawn_LocustBrumak_SideGun GetSideGun()
{
	return GearPawn_LocustBrumak_SideGun(Instigator);
}

function GearPC GetPlayerOwner()
{
	if( Instigator == None )
		return None;

	if( Instigator.IsHumanControlled() )
	{
		return GearPC(Instigator.Controller);
	}
	return GearPC(Pawn(Instigator.Base).Controller);
}

simulated function Actor GetTraceOwner()
{
	local GearPawn_LocustBrumakBase Brumak;

	Brumak = GetBrumak();
	return (Brumak != None)	? Brumak : Super.GetTraceOwner();
}

simulated function SkeletalMeshComponent GetSkeletalMeshComponent()
{
	local GearPawn_LocustBrumakBase	Brumak;
	local SkeletalMeshActor			SkelMeshActor;

	Brumak = GetBrumak();
	if( Brumak != None )
	{
		return Brumak.Mesh;
	}
	else
	if( bDummyFireWeapon )
	{
		SkelMeshActor = SkeletalMeshActor(DummyFireParent);
		if( SkelMeshActor != None )
		{
			return SkelMeshActor.SkeletalMeshComponent;
		}
	}
	return None;
}


simulated function bool ShouldOverrideWeaponStartTrace()
{
	return TRUE;
}

/**
 * Performs an 'Instant Hit' shot.
 * Also, sets up replication for remote clients,
 * and processes all the impacts to deal proper damage and play effects.
 *
 * Network: Local Player and Server
 */

simulated function InstantFire()
{
	local vector			StartTrace, EndTrace;
	local Array<ImpactInfo>	ImpactList;
	local int				Idx;
	local ImpactInfo		RealImpact;
	local Rotator			Dir;

	// define range to use for CalcWeaponFire()
	StartTrace = GetMuzzleLoc();
	Dir = GetAdjustedAim(StartTrace);
	EndTrace = StartTrace + vector(Dir) * GetTraceRange();

	// Perform shot
	RealImpact = CalcWeaponFire(StartTrace, EndTrace, ImpactList);

	if (Role == ROLE_Authority || (bClientSideInstantHit && Instigator != None && Instigator.IsLocallyControlled()))
	{
/*		FlushPersistentDebugLines();
		DrawDebugSphere( StartTrace, 10, 10, 0, 255, 0, true );
		DrawDebugLine(StartTrace, EndTrace, 255, 0, 0, true);
		DrawDebugSphere( EndTrace, 10, 10, 0, 255, 0, true );
		DrawDebugSphere( RealImpact.HitLocation, 10, 10, 0, 0, 255, true );
		`log( self@GetFuncName()@Instigator@"Start"@StartTrace@"End"@EndTrace@"Dir"@Dir@"Hit"@RealImpact.HitLocation@RealImpact.HitActor );*/

		// Set flash location to trigger client side effects.
		// if HitActor == None, then HitLocation represents the end of the trace (maxrange)
		// Remote clients perform another trace to retrieve the remaining Hit Information (HitActor, HitNormal, HitInfo...)
		// Here, The final impact is replicated. More complex bullet physics (bounce, penetration...)
		// would probably have to run a full simulation on remote clients.
		SetFlashLocation(RealImpact.HitLocation);
	}

	// Process all Instant Hits on local player and server (gives damage, spawns any effects).
	for (Idx = 0; Idx < ImpactList.Length; Idx++)
	{
		ProcessInstantHit(CurrentFireMode, ImpactList[Idx]);
	}
}
/**
 * Fires a projectile.
 * Spawns the projectile, but also increment the flash count for remote client effects.
 * Network: Local Player and Server
 */

simulated function Projectile ProjectileFire()
{
	local vector		StartTrace, EndTrace, RealStartLoc, AimDir, ProjDir;
	local ImpactInfo	TestImpact;
	local Projectile	SpawnedProjectile;

	// tell remote clients that we fired, to trigger effects
	IncrementFlashCount();

	if( Role == ROLE_Authority )
	{
		// This is where we would start an instant trace. (what CalcWeaponFire uses)
		StartTrace = GetMuzzleLoc();
		AimDir = Vector(GetAdjustedAim( StartTrace ));

		// this is the location where the projectile is spawned.
		GetProjectileFirePosition(RealStartLoc, ProjDir);

		if( StartTrace != RealStartLoc )
		{
			// if projectile is spawned at different location of crosshair,
			// then simulate an instant trace where crosshair is aiming at, Get hit info.
			EndTrace = StartTrace + AimDir * GetTraceRange();
			TestImpact = CalcWeaponFire( StartTrace, EndTrace );

			// Then we realign projectile aim direction to match where the crosshair did hit.
			AimDir = Normal(TestImpact.HitLocation - RealStartLoc);
		}

		// Spawn projectile
		SpawnedProjectile = Spawn(GetProjectileClass(), Self,, RealStartLoc);
		if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
		{
			SpawnedProjectile.Init( AimDir );
		}

		// Return it up the line
		return SpawnedProjectile;
	}

	return None;
}

/** Get muzzle socket for side guns */
simulated function Name GetMuzzleSocketName( bool bIsLeftGun )
{
	local GearPawn_LocustBrumakBase	Brumak;

	Brumak = GetBrumak();
	if( Brumak != None )
	{
		if( bIsLeftGun )
		{
			return Brumak.LeftMuzzleSocketName;
		}
		return Brumak.RightMuzzleSocketName;
	}

	if( bIsLeftGun )
	{
		return class'GearPawn_LocustBrumak'.default.LeftMuzzleSocketName;
	}
	return class'GearPawn_LocustBrumak'.default.RightMuzzleSocketName;
}

simulated function PerformWeaponAttachment(SkeletalMeshComponent MeshCpnt, optional Name BoneName)
{
	//debug
	`log( self@GetFuncName()@GetBrumak()@Instigator@bDummyFireWeapon@DummyFireParent, bDebug );

	AttachMuzzleEffectsComponents( GetSkeletalMeshComponent() );
}

simulated function DetachWeapon()
{
	local SkeletalMeshComponent	PawnMesh;

	Super.DetachWeapon();

	//debug
	`log( self@GetFuncName()@GetBrumak()@Instigator@bDummyFireWeapon@DummyFireParent, bDebug );

	PawnMesh = GetSkeletalMeshComponent();
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

simulated function bool IsMuzzleFlashRelevant()
{
	return TRUE;
}

simulated state MeleeAttacking
{
	simulated function BeginState(Name PreviousStateName)
	{
		local GearPC PC;

		PC = GearPC(Instigator.Controller);
		if( PC != None )
		{
			SetCurrentFireMode( MELEE_ATTACK_FIREMODE );

			PC.DoSpecialMove( GSM_Brumak_OverlayBite );
		}
	}
}

defaultproperties
{
	MuzzleSocketName=None
	bAllowTracers=TRUE

	bUseMuzzleLocForPhysicalFireStartLoc=true
}
