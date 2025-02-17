
/**
 * Cannon prototype.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_Cannon extends GearWeapon;


/** Time it takes to reach full charge */
var	config float		MaxChargeTime;
/** Time it takes to allow projectile to stick (not bounce) */
var config float		StickChargeTime;
var float				MinChargeTime;
/** Time weapon can be held at max charge, before the player gets tired. */
var	config float		HoldTimeUntilTired;

/** Time weapon is charged */
var	transient float		ChargeTimeCount;


var		SoundCue	LaunchSound[3];
var		SoundCue	ChargeSound;


var SoundCue AimLoop;
var AudioComponent AimingSound;

var GearProj_Grenade			SimulatedGrenade;

//var Emitter					EM_StartPoint;
//var ParticleSystem			PS_StartPoint;

var ParticleSystemComponent PSC_Trail;
var ParticleSystem			PS_TrailCOG;
var ParticleSystem			PS_TrailLocust;

var ParticleSystemComponent PSC_FailedTrail;
var ParticleSystem			PS_FailedTrail;

var bool bPlayedMaxCharge;
var ForceFeedbackWaveform MaxChargeWaveform;

//
// End points particle effects
//
var Emitter			EM_EndPoint;
var ParticleSystem	PS_EndPointCOG;
var ParticleSystem	PS_EndPointLocust;


var Emitter			EM_EndPointFullCharge;
var ParticleSystem	PS_EndPointFullCharge;

var Emitter			EM_EndPointFail;
var ParticleSystem	PS_EndPointFail;

var ParticleSystemComponent PSC_TorqueBowChargeUp;
var ParticleSystem PS_TorqueBowChargeUp;

var bool			bPendingEndFire;

/** Testing of animations */
enum EAnimStyle
{
	EAS_Closed,
	EAS_Opened,
	EAS_TargetingOpens
};

var EAnimStyle	CurrentStyle;
var bool		bOpened;



/** Kill off the emitters that we spawned **/
simulated function Destroyed()
{
	if( SimulatedGrenade != none )
	{
		SimulatedGrenade.Destroy();
		SimulatedGrenade = none;
	}

// 	if( EM_StartPoint != none )
// 	{
// 		EM_StartPoint.Destroy();
// 		EM_StartPoint = none;
// 	}

	if( EM_EndPoint != none )
	{
		EM_EndPoint.Destroy();
		EM_EndPoint = none;
	}

	if( EM_EndPointFullCharge != none )
	{
		EM_EndPointFullCharge.Destroy();
		EM_EndPointFullCharge = none;
	}

	if( EM_EndPointFail != none )
	{
		EM_EndPointFail.Destroy();
		EM_EndPointFail = none;
	}


	Super.Destroyed();
}


/** Don't render crosshair for this weapon. */
simulated function DrawWeaponCrosshair( Hud H );


simulated function ToggleCharging(bool bNewCharging)
{
	if ( GearPawn(Instigator) != None )
	{
		GearPawn(instigator).bChargingBow = bNewCharging;
	}
	if ( bNewCharging )
	{
		if( PSC_TorqueBowChargeUp == none )
		{
			PSC_TorqueBowChargeUp = new(Outer) class'ParticleSystemComponent';
			PSC_TorqueBowChargeUp.SetTemplate( PS_TorqueBowChargeUp );
			SkeletalMeshComponent(Mesh).AttachComponentToSocket( PSC_TorqueBowChargeUp, 'ChargeUp' );
			PSC_TorqueBowChargeUp.SetOwnerNoSee(TRUE);

		}
		PSC_TorqueBowChargeUp.SetHidden( FALSE );
		PSC_TorqueBowChargeUp.ActivateSystem();
	}
	else
	{
		if( PSC_TorqueBowChargeUp != none )
		{
			PSC_TorqueBowChargeUp.SetHidden( TRUE );
			PSC_TorqueBowChargeUp.DeactivateSystem();
		}
	}
}

/** Player is charging weapon. */
simulated state Charge
{
	simulated function BeginState( Name PreviousStateName )
	{
		//`log( WorldInfo.TimeSeconds @ GetStateName() @ GetFuncName() );

		// Set firing mode to CHARGE.
		// This is to be replicated to trigger events/animations on all clients.
		SetCurrentFireMode(FIREMODE_CHARGE);

		ChargeTimeCount		= MinChargeTime;
		bPlayedMaxCharge	= FALSE;
		bPendingEndFire		= FALSE;

		ToggleCharging(true);

		//`log( "HIHIIHIHIH IA M CHARGING" );
	}

	simulated function EndState( Name NextStateName )
	{
		//`log( WorldInfo.TimeSeconds @ GetStateName() @ GetFuncName() );

		ClearTimer('PlayMaxCharge');
		StopSimulation();
		if(AimingSound != none)
		{
			AimingSound.FadeOut(0.5f,0.0f);
			SetTimer( 0.5f,false,nameof(NullAimSound) );
		}
		ToggleCharging(false);

		// Reset firing mode back to default
		SetCurrentFireMode(0);
	}

	simulated event bool IsFiring()
	{
		return TRUE;
	}

	simulated function Tick( float DeltaTime )
	{
		local GearPawn	P;

		P = GearPawn(Instigator);

		Super.Tick(DeltaTime);

		// See if we can fire the shot now
		// May not be safe if we're transitioning, we can hit the cover wall
		if( bPendingEndFire && CanReleaseFire() )
		{
			bPendingEndFire = FALSE;
			GotoState('Fire');
		}

		// double charge if active reloaded
		if( ActiveReload_NumBonusShots > 0 )
		{
			ChargeTimeCount += DeltaTime * 2.f;
		}
		else
		{
			ChargeTimeCount += DeltaTime;
		}

		if( ChargeTimeCount >= MaxChargeTime + HoldTimeUntilTired )
		{
			ChargeTimeCount = MaxChargeTime + HoldTimeUntilTired;

			//`log( WorldInfo.TimeSeconds @ GetStateName() @ GetFuncName() @ "MaxChargeTime, Fire!" );

			// Force fire when time expires.
			CheckFireShot();
			return;
		}
		else
		{
			if( !bPlayedMaxCharge && ChargeTimeCount > MaxChargeTime && Instigator.IsLocallyControlled() )
			{
				bPlayedMaxCharge = TRUE;
				PlayMaxCharge();
			}
		}

		// no trail when blindfiring.
		if( P.IsLocallyControlled() && P.IsHumanControlled() &&
			P.bIsTargeting && (P.CoverType == CT_None || P.IsLeaning() || P.bDoing360Aiming) )
		{
			SimulateTrajectory();
		}
		else
		{
			StopSimulation();
		}
	}

	// Override this function, to keep the player pressing fire until all animations are ready.
	simulated function EndFire(byte FireModeNum)
	{
		CheckFireShot();
		//`log( WorldInfo.TimeSeconds @ GetStateName() @ GetFuncName() );
	}
}


/**
 * See if we can fire the shot now
 * May not be safe if we're transitioning, we can hit the cover wall
 */
simulated function bool CanReleaseFire()
{
	local GearPawn	WP;
	local bool		bPawnDelayingFire;

	WP = GearPawn(Instigator);
	if( WP != None && WP.CoverType != CT_None && WP.IsBlindFiring(WP.CoverAction) )
	{
		bNoAnimDelayFiring	= FALSE;
		bPawnDelayingFire	= WP.ShouldDelayFiring();
		bNoAnimDelayFiring	= TRUE;
	}

	return !bPawnDelayingFire;
}

simulated function CheckFireShot()
{
	// See if we can fire the shot now
	// May not be safe if we're transitioning, we can hit the cover wall
	if( CanReleaseFire() )
	{
		GotoState('Fire');
	}
	else
	{
		bPendingEndFire = TRUE;
	}
}

simulated function NullAimSound()
{
	AimingSound = none;
}

/**
 * Fires a projectile.
 * Spawns the projectile, but also increment the flash count for remote client effects.
 * Network: Local Player and Server
 */
simulated function Projectile ProjectileFire()
{
	local GearProj_Cannon FiredArrow;
	local GearPC WPC;
	local GearPawn WP, HitWP;
	local vector StartTrace, TraceDir, EndTrace, CamLoc, HitLoc, HitNormal;
	local rotator CamRot;
	local GearGRI GRI;

	FiredArrow = GearProj_Cannon(Super.ProjectileFire());
	GRI = GearGRI(WorldInfo.GRI);

	// don't allow arrows to stick into non-enemy or in cover enemy pawns in MP games
	if ( (FiredArrow != None) &&
		GRI.IsMultiplayerGame() &&
		!FiredArrow.bAllowBounce )
	{
		FiredArrow.bAllowStickInPawn = false;
		WPC = GearPC(Instigator.Controller);
		WP = GearPawn(Instigator);
		if ( (WPC != None) && (WP != None) )
		{
			// start the trace from the camera location
			WPC.GetPlayerViewPoint(CamLoc,CamRot);
			StartTrace = FiredArrow.Location;
			TraceDir = normal(FiredArrow.Velocity);
			EndTrace = StartTrace + (TraceDir * 6000.0f);
			// trace out from the camera
			HitWP = GearPawn( WP.Trace(HitLoc, HitNormal, EndTrace, StartTrace, TRUE,,, TRACEFLAG_Bullet) );
			// we only want unfriendly targets for this feature
			if ( !WorldInfo.GRI.OnSameTeam(WP, HitWP) || GRI.bAllowFriendlyFire )
			{
				if ( HitWP != None && HitWP.Health >= 0 && !HitWP.bTearOff )
				{
					FiredArrow.bAllowStickInPawn = (HitWP.MultiplayerCoverDamageAdjust(TraceDir, WP, HitLoc, 100) != 0);
				}
			}
		}
	}
	return FiredArrow;
}


/** Player releases fire button, weapon fires a shot */
simulated state Fire
{
	simulated event bool IsFiring()
	{
		return TRUE;
	}

	simulated function BeginState( Name PreviousStateName )
	{
		//`log( WorldInfo.TimeSeconds @ GetStateName() @ GetFuncName() );

		// Make sure we're using the default firing mode.
		SetCurrentFireMode(0);

		// Fire the first shot right away
		FireAmmunition();
		TimeWeaponFiring(CurrentFireMode);

		// Release Fire button
		if( PendingFire(0) )
		{
			EndFire(0);
		}
	}

	simulated function RefireCheckTimer()
	{
		GotoState('Active');
	}

	simulated function EndState( Name NextStateName )
	{
		//`log( WorldInfo.TimeSeconds @ GetStateName() @ GetFuncName() );

		// Set weapon as not firing
		ClearFlashCount();
		ClearTimer( 'RefireCheckTimer' );
	}

}

simulated function PlayMaxCharge()
{
	local float Delta;
	local PlayerController PC;

	PC = PlayerController(Instigator.Controller);
	if( PC != None )
	{
		PC.ClientPlayForceFeedbackWaveform(MaxChargeWaveform);

		if( ChargeTimeCount < MaxChargeTime + HoldTimeUntilTired )
		{
			Delta = FMax((MaxChargeTime + HoldTimeUntilTired - ChargeTimeCount)/HoldTimeUntilTired, 0.1f);
			SetTimer( Delta, FALSE, nameof(PlayMaxCharge) );
		}
	}
}


simulated function RefireCheckTimer()
{
	if( bWeaponPutDown )
	{
		PutDownWeapon();
		return;
	}
}

/**
 * This will make the particle system for the grenade arc be invis.
 */
simulated function StopSimulation()
{
	if( PSC_Trail != None )
	{
		PSC_Trail.SetHidden(TRUE);
		PSC_Trail.DeactivateSystem();

		PSC_FailedTrail.SetHidden(TRUE);
		PSC_FailedTrail.DeactivateSystem();

		//EM_StartPoint.SetHidden(TRUE);
		EM_EndPoint.SetHidden(TRUE);
		EM_EndPointFullCharge.SetHidden(TRUE);
		EM_EndPointFail.SetHidden(TRUE);
	}

	if ( SimulatedGrenade != None )
	{
		SimulatedGrenade.SetHidden( TRUE );
		SimulatedGrenade.SetCollision( FALSE, FALSE, FALSE );
		SimulatedGrenade.Disable( 'Tick' );
	}
}


/**
 * This is the function that will create all of the particle systems and emitters
 * needed to draw the grenade aiming arc.
 */
simulated function SetUpSimulation()
{
	if (PlayerController(Instigator.Controller) == None)
	{
		// no arc for AI
		return;
	}

	if( SimulatedGrenade == None || SimulatedGrenade.bDeleteMe )
	{
		SimulatedGrenade = GearProj_Grenade(Spawn(WeaponProjectiles[0],,, GetPhysicalFireStartLoc()));
		if (SimulatedGrenade == None)
		{
			return;
		}
		SimulatedGrenade.RemoteRole = ROLE_None;
		SimulatedGrenade.bIsSimulating = TRUE;
		SimulatedGrenade.SetOwner(Self);
		SimulatedGrenade.LifeSpan = 0.f;
	}

	SimulatedGrenade.SetHidden( FALSE );
	SimulatedGrenade.SetCollision( TRUE, FALSE, FALSE );
	SimulatedGrenade.Enable( 'Tick' );

	// grenade arc creation
	// currently hidden inside the bow so we just comment out for now
	//if( EM_StartPoint == None )
	//{
	//	EM_StartPoint = Spawn(class'SpawnedGearEmitter',Instigator,, Location);
	//	EM_StartPoint.bOnlyOwnerSee = TRUE;
	//	EM_StartPoint.SetHidden(TRUE);

	//	EM_StartPoint.SetTemplate(PS_StartPoint);
	//	EM_StartPoint.ParticleSystemComponent.ActivateSystem();
	//	EM_StartPoint.SetHidden(TRUE);
	//}

	if( EM_EndPoint == None )
	{
		EM_EndPoint = Spawn(class'SpawnedGearEmitter',Instigator,, Location);
		EM_EndPoint.SetHidden(TRUE);
		EM_EndPoint.SetOnlyOwnerSee(TRUE);

		if( Instigator.Controller.GetTeamNum() == 0 )
		{
			EM_EndPoint.SetTemplate( PS_EndPointCOG );
		}
		else
		{
			EM_EndPoint.SetTemplate( PS_EndPointLocust );
		}

		EM_EndPoint.ParticleSystemComponent.ActivateSystem();
	}

	if( EM_EndPointFullCharge == None )
	{
		EM_EndPointFullCharge = Spawn(class'SpawnedGearEmitter',Instigator,, Location);
		EM_EndPointFullCharge.LifeSpan = 0.0f;
		EM_EndPointFullCharge.SetHidden(TRUE);
		EM_EndPointFullCharge.SetOnlyOwnerSee(TRUE);

		EM_EndPointFullCharge.SetTemplate(PS_EndPointFullCharge);
	}

	if( EM_EndPointFail == None )
	{
		EM_EndPointFail = Spawn(class'SpawnedGearEmitter',Instigator,, Location);
		EM_EndPointFail.LifeSpan = 0.0f;
		EM_EndPointFail.SetHidden(TRUE);
		EM_EndPointFail.SetOnlyOwnerSee(TRUE);

		EM_EndPointFail.SetTemplate(PS_EndPointFail);
	}

	if( PSC_Trail == None )
	{
		PSC_Trail = new(Outer) class'ParticleSystemComponent';
		if( Instigator.Controller.GetTeamNum() == 0 )
		{
			PSC_Trail.SetTemplate(PS_TrailCOG);
		}
		else
		{
			PSC_Trail.SetTemplate(PS_TrailLocust);
		}

		PSC_Trail.SetOnlyOwnerSee(TRUE);
		PSC_Trail.SetActorParameter('TrailActor', SimulatedGrenade);

		SimulatedGrenade.AttachComponent(PSC_Trail);
		PSC_Trail.SetHidden(TRUE);
	}

	if( PSC_FailedTrail == None )
	{
		PSC_FailedTrail = new(Outer) class'ParticleSystemComponent';
		PSC_FailedTrail.SetTemplate(PS_FailedTrail);
		PSC_FailedTrail.SetOnlyOwnerSee(TRUE);
		PSC_FailedTrail.SetActorParameter('TrailActor', SimulatedGrenade);

		SimulatedGrenade.AttachComponent(PSC_FailedTrail);
		PSC_FailedTrail.SetHidden(TRUE);
	}
}


/**
 * This will will simulate the grenade arc (in c++) and then tick the trail emitter
 * which causes the list of particles to grow.
 * At the end of the simulation the particle system has an entire trail of particles
 * representing the arc of the grenade toss.
 *
 * Then the arc is turned "on" so that it is visible to the client.
 */
simulated function SimulateTrajectory()
{
	local Vector	ImpactLoc, StartLoc, ImpactNormal;
	local bool		bFirstTimePenetrate;

	// purely a client-side effect, no need for server to care
	if ( (Instigator == None) || !Instigator.IsLocallyControlled() || !Instigator.IsHumanControlled() )
	{
		return;
	}

	SetUpSimulation();

	if( SimulatedGrenade == None || SimulatedGrenade.bDeleteMe )
	{
		`log("SimulatedGrenade == None || SimulatedGrenade.bDeleteMe");
		return;
	}

	// first time activation
	if( EM_EndPointFullCharge.bHidden )
	{
		bFirstTimePenetrate = TRUE;
	}
	EM_EndPointFail.SetHidden(TRUE);
	EM_EndPointFullCharge.SetHidden(FALSE);
	EM_EndPoint.SetHidden(TRUE);

	PSC_Trail.SetHidden(FALSE);
	PSC_Trail.ActivateSystem();

	StartLoc = GetPhysicalFireStartLoc();

	// unhide arrow -- it will appear in the bow
	SimulatedGrenade.SetHidden(FALSE);
	SimulatedGrenade.SetLocation(StartLoc);
	DetermineAimingArrowTrajectory(SimulatedGrenade);

	//ImpactLoc = SimulatedGrenade.RunPhysicsSimulationTilEnd(class'GearProj_Arrow'.default.LifeSpan);
	SimulatedGrenade.RunPhysicsSimulationTilEnd(0.5f);
	ImpactLoc		= SimulatedGrenade.Location;
	ImpactNormal	= GearProj_Cannon(SimulatedGrenade).ImpactNormal;
	if( IsZero(ImpactNormal) )
	{
		ImpactNormal = vect(0,0,1);
	}

	// debug impact locationl
	//DrawDebugCoordinateSystem(ImpactLoc, Rotator(ImpactNormal), 25.f);
	//EM_StartPoint.SetLocation(StartLoc);
	//EM_StartPoint.SetHidden(FALSE);

	// position end points
	if( !EM_EndPoint.bHidden )
	{
		EM_EndPoint.SetLocation(ImpactLoc);
		EM_EndPoint.SetRotation(Rotator(ImpactNormal));
	}

	if( !EM_EndPointFullCharge.bHidden )
	{
		EM_EndPointFullCharge.SetLocation(ImpactLoc);
		EM_EndPointFullCharge.SetRotation(Rotator(ImpactNormal));

		if( bFirstTimePenetrate )
		{
			EM_EndPointFullCharge.ParticleSystemComponent.ActivateSystem();
		}
	}

	if( !EM_EndPointFail.bHidden )
	{
		EM_EndPointFail.SetLocation(ImpactLoc);
		EM_EndPointFail.SetRotation(Rotator(ImpactNormal));
	}
}


simulated function GetProjectileFirePosition(out vector out_ProjLoc, out vector out_ProjDir)
{
	local GearPawn		P;
	local vector		StartTrace, SafeSpawnLoc, TestLoc;

	P = GearPawn(Instigator);

	// This is where we would start an instant trace. (what CalcWeaponFire uses)
	StartTrace	= Instigator.GetWeaponStartTraceLocation();
	out_ProjDir	= Vector(GetAdjustedAim( StartTrace ));

	// this is the location where the projectile is spawned.
	out_ProjLoc		= GetMuzzleLoc();
	SafeSpawnLoc	= out_ProjLoc - out_ProjDir * 30.f;

	//FlushPersistentDebugLines();
	//DrawDebugLine(SafeSpawnLoc, out_ProjLoc, 255, 0, 0, TRUE);

	// If can't trace to real muzzle location, use safe one
	if( !FastTrace(out_ProjLoc, SafeSpawnLoc) )
	{
		out_ProjLoc = SafeSpawnLoc;
	}

	//DrawDebugSphere(out_ProjLoc, 4.f, 8, 255, 0, 0, TRUE);

	// Special case for mid level blind fire up
	// trace further, and if cover is hit, then adjust loc and dir of projectile to prevent
	// firing into cover wall.
	if( P.CoverType == CT_MidLevel && P.CoverAction == CA_BlindUp && !P.bDoing360Aiming )
	{
		TestLoc = out_ProjLoc + out_ProjDir * 30.f;

		//DrawDebugLine(TestLoc, out_ProjLoc, 0, 255, 0, TRUE);
		if( !FastTrace(TestLoc, out_ProjLoc) )
		{
			// adust location to a safer spot
			if( out_ProjLoc != SafeSpawnLoc )
			{
				out_ProjLoc = SafeSpawnLoc;
			}

			// adjust rotation, aim parallel to cover.
			out_ProjDir.Z	= Vector(P.Rotation).Z;
			out_ProjDir		= Normal(out_ProjDir);
		}
	}
}


/**
 * This will calculate the trajectory of the arrow then init the projectile
 * correctly.
 */
simulated function DetermineAimingArrowTrajectory(GearProj_Grenade AimArrow)
{
	local Vector ProjLoc, ProjDir;

	GetProjectileFirePosition(ProjLoc, ProjDir);

	AimArrow.Init(ProjDir);
}

simulated function bool ShouldDrawCrosshair()
{
	return FALSE;
}

simulated function bool IsApplicableForARDamageBonus()
{
	return TRUE;
}


exec function SetMode( Name Style )
{
	`log( "Mode:" @ Style );

	if( Style == 'Opened' )
	{
		CurrentStyle = EAS_Opened;
	}
	else if( Style == 'Targeting' )
	{
		CurrentStyle = EAS_TargetingOpens;
	}
	else
	{
		CurrentStyle = EAS_Closed;
	}
}

simulated function UpdateBowBasedOnTargeting()
{
	local GearPawn P;

	P = GearPawn(Instigator);

	if( !bOpened && P.bIsTargeting && HasAnyAmmo() && !bWeaponPutDown && !P.bSwitchingWeapons && IsActiveWeapon() )
	{
		bOpened = TRUE;
		OpenBow();
	}
	else if( bOpened )
	{
		bOpened = FALSE;
		CloseBow();
	}
}

/** Notification that Targeting Mode has changed. */
simulated function TargetingModeChanged(GearPawn P)
{
	Super.TargetingModeChanged(P);

	if( CurrentStyle == EAS_TargetingOpens )
	{
		UpdateBowBasedOnTargeting();
	}
}

simulated function OpenBow()
{
	ClearTimer('OpenBow');
	PlayWeaponAnimByDuration('Open_Idle', EquipTime, EquipTime, -1.f, FALSE, FALSE);
}

simulated function CloseBow()
{
	PlayWeaponAnimByDuration('Close_Idle', PutDownTime, PutDownTime, -1.f, FALSE, FALSE);
}

simulated function TimeWeaponEquipping()
{
	Super.TimeWeaponEquipping();

	if( CurrentStyle == EAS_Opened )
	{
		SetTimer( 0.3f,FALSE,nameof(OpenBow) );
	}
	else if( CurrentStyle == EAS_TargetingOpens )
	{
		UpdateBowBasedOnTargeting();
	}
	else
	{
		PlayWeaponAnim('Close_Idle', 1.f, 0.05f, -1.f, FALSE, FALSE);
	}
}

simulated function TimeWeaponPutDown()
{
	Super.TimeWeaponPutDown();

	if( CurrentStyle == EAS_Opened )
	{
		CloseBow();
	}
	else if( CurrentStyle == EAS_TargetingOpens )
	{
		UpdateBowBasedOnTargeting();
	}
	else
	{
		PlayWeaponAnim('Close_Idle', 1.f, 0.05f, -1.f, FALSE, FALSE);
	}
}


/**
 * Event called when Pawn.FiringMode has been changed.
 * bViaReplication indicates if this was the result of a replication call.
 */
simulated function FireModeUpdated(byte FiringMode, bool bViaReplication)
{
	if( PreviousFiringMode != FiringMode )
	{
		// Call events that ended first
		switch( PreviousFiringMode )
		{
			case FIREMODE_CHARGE : ChargeEnded();	break;
		}

		// Then call events which started
		switch( FiringMode )
		{
			case FIREMODE_CHARGE : ChargeStarted();	break;
		}
	}

	Super.FireModeUpdated(FiringMode, bViaReplication);
}


/**
 * Event called when player starts to charge.
 * Called on all net modes.
 */
simulated function ChargeStarted()
{
	local float		AnimLength;
	if( CurrentStyle == EAS_Opened || CurrentStyle == EAS_TargetingOpens )
	{
		// Play charge animation
		AnimLength = PlayWeaponAnim('Tension_B', 1.f, 0.05f, -1.f, FALSE, FALSE);
	}
	else
	{
		// Play charge animation
		AnimLength = PlayWeaponAnim('Tension', 1.f, 0.05f, -1.f, FALSE, FALSE);
	}

	if(AimingSound == none)
	{

		AimingSound = CreateAudioComponent(AimLoop, FALSE, TRUE);
		AimingSound.bUseOwnerLocation = TRUE;
		AimingSound.bAutoDestroy = FALSE;
		AimingSound.Location = Owner.Location;
		Owner.AttachComponent(AimingSound);

	}

	AimingSound.Play();
	// Set timer to play a looping animation after the intro animation.
	SetTimer( AnimLength, FALSE, nameof(TensionIntroEnded) );
}

/** Tension intro anim is done playing, we can blend in the looping animation. */
simulated function TensionIntroEnded()
{
	if( CurrentStyle == EAS_Opened || CurrentStyle == EAS_TargetingOpens )
	{
		// Play looping animation
		PlayWeaponAnim('Tension_Hold_B', 1.f, 0.05f, -1.f, TRUE, FALSE);
	}
	else
	{
		// Play looping animation
		PlayWeaponAnim('Tension_Hold', 1.f, 0.05f, -1.f, TRUE, FALSE);
	}
}


/**
 * Event called when player stops charging.
 * Called on all net modes.
 */
simulated function ChargeEnded()
{
	// Clear TensionIntro timer in case it hasn't been called yet.
	ClearTimer('TensionIntroEnded');
}


/** Event called when weapon fired */
simulated function PlayFireEffects(byte FireModeNum, optional vector HitLocation)
{
	Super.PlayFireEffects(FireModeNum, HitLocation);

	//if( CurrentStyle == EAS_Opened || CurrentStyle == EAS_TargetingOpens )
	//{
		// Play firing animation to close bow
		PlayWeaponAnim('AR_Fire_Boomshot', 1.f, 0.05f, -1.f, FALSE, FALSE);
	//}
	//else
	//{
		// Play firing animation to close bow
		//PlayWeaponAnim('Fire', 1.f, 0.05f, -1.f, FALSE, FALSE);
	//}
}

simulated function PlayOwnedFireEffects(byte FireModeNum, vector HitLocation)
{
	Super.PlayOwnedFireEffects(FireModeNum, HitLocation);

	// Play launching sound
	// This is using non replicated variables, so replicate the resulting sound instead to remote clients.
	PlayLaunchSound();
}

/**
 * Play arrow launching sound depending on charge.
 * The longer the charge, the stronger the sound.
 */
simulated function PlayLaunchSound()
{
	WeaponPlaySound(SoundCue'Weapon_TorqueBow.Weapons.Crossbow_FireCue');
}

simulated function PlayWeaponReloading()
{
	Super.PlayWeaponReloading();

	if( CurrentStyle == EAS_TargetingOpens )
	{
		UpdateBowBasedOnTargeting();
	}
}


/** notification fired from Pawn animation when player grabs another magazine. */
simulated function Notify_AmmoGrab()
{
	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Attach magazine mesh to left hand
	SetPawnAmmoAttachment(TRUE);
}


/** notification fired from Pawn animation when player puts new magazine in weapon. */
simulated function Notify_AmmoReload()
{
	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Detach magazine mesh from instigator's left hand
	SetPawnAmmoAttachment(FALSE);
}

/*
simulated function LinearColor GetWeaponEmisColor_COG()
{
	return WorldInfo.Gears_COGWeapons.TorqueBow;
}

simulated function LinearColor GetWeaponEmisColor_Locust()
{
	return WorldInfo.Gears_LocustWeapons.TorqueBow;
}
*/


defaultproperties
{
	CurrentStyle=EAS_Opened	// default animation code is bow opened. This can be changed for easy testing.

	//FiringStatesArray(0)="Charge"
	FiringStatesArray(0)="WeaponFiring"
	WeaponFireTypes(0)=EWFT_Projectile
	WeaponProjectiles(0)=class'GearProj_Cannon'

	bNoAnimDelayFiring=TRUE	// charge weapon, so no need to delay firing because of animation transitions.
	bWeaponCanBeReloaded=TRUE

	AIRating=1.f

	InstantHitDamageTypes(0)=class'GDT_Ballistic'
	AmmoTypeClass=class'GearAmmoType_Cannon'

}

