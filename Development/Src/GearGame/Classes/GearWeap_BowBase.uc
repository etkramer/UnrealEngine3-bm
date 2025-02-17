
/**
 * Torque Bow.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_BowBase extends GearWeapon
	abstract
	config(Weapon);

/** Time it takes to reach full charge */
var	config float		MaxChargeTime;
/** Time it takes to allow projectile to stick (not bounce) */
var config float		StickChargeTime;
var float				MinChargeTime;
/** Time weapon can be held at max charge, before the player gets tired. */
var	config float		HoldTimeUntilTired;

/** Time weapon is charged */
var	transient float		ChargeTimeCount;


//var		SoundCue	LaunchSound[3];
var		SoundCue		ChargeSound;


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

/** set during charging after guessing the shooter's target and sending a warning */
var bool bGuessedTarget;

var StaticMeshComponent	ArrowMesh;

var protected const SoundCue LaunchSound;

var protected const SoundCue BowMeleeSwingSound;

simulated function float GetWeaponRating()
{
	local float Rating;

	Rating = Super.GetWeaponRating();

	// AI prefers bow at long range
	if ( GearAI(Instigator.Controller) != None && Instigator.Controller.Enemy != None &&
		!GearAI(Instigator.Controller).IsShortRange(Instigator.Controller.Enemy.Location) )
	{
		Rating += 0.5;
	}

	return Rating;
}

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
	// when we're charging, stop firing and wait a bit before evading so we don't fire the bow while evading
	function float AIDelayBeforeEvade(GearAI AI)
	{
		AI.StopFiring();
		return 0.785f;
	}

	simulated function BeginState( Name PreviousStateName )
	{
		//`log( WorldInfo.TimeSeconds @ GetStateName() @ GetFuncName() );

		// Set firing mode to CHARGE.
		// This is to be replicated to trigger events/animations on all clients.
		SetCurrentFireMode(FIREMODE_CHARGE);

		ChargeTimeCount		= MinChargeTime;
		bPlayedMaxCharge	= FALSE;
		bPendingEndFire		= FALSE;
		bGuessedTarget = false;

		ToggleCharging(true);

		//`log( "HIHIIHIHIH IA M CHARGING" );
	}

	simulated function EndState( Name NextStateName )
	{
		//`log( WorldInfo.TimeSeconds @ GetStateName() @ GetFuncName() );

		ClearTimer('PlayMaxCharge');
		StopSimulation();
		if (AimingSound != None)
		{
			AimingSound.FadeOut(0.5f,0.0f);
			SetTimer(0.5f, false, nameof(NullAimSound));
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
		local vector StartTrace, EndTrace, HitLocation, HitNormal;
		local Pawn HitPawn;

		P = GearPawn(Instigator);

		Super.Tick(DeltaTime);

		// See if we can fire the shot now
		// May not be safe if we're transitioning, we can hit the cover wall
		if( bPendingEndFire && CanReleaseFire() && !P.PawnCommitToFiring(0) )
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

		// AI checks if it wants to release fire here
		if ( GearAI_TDM(GearAIController) != None && GearAIController.FireTarget != None && ChargeTimeCount >= MaxChargeTime &&
			GearAIController.IsFireLineClear() )
		{
			CheckFireShot();
			return;
		}

		// attempt to notify an AI being targeted by the bow as it charges
		// only guess a target once per shot so that if the player is good enough to be able to
		// successfully switch targets mid-charge, they get rewarded by the AI being unable to react
		if (Role == ROLE_Authority && !bGuessedTarget && ChargeTimeCount > 0.75)
		{
			StartTrace = GetMuzzleLoc();
			EndTrace = StartTrace + vector(GetAdjustedAim(StartTrace)) * 5000.0;
			HitPawn = Pawn(Trace(HitLocation, HitNormal, EndTrace, StartTrace, true,,, TRACEFLAG_Bullet));
			if (HitPawn != None && !WorldInfo.GRI.OnSameTeam(HitPawn, Instigator))
			{
				bGuessedTarget = true;
				if (GearAI(HitPawn.Controller) != None)
				{
					GearAI(HitPawn.Controller).ReceiveChargeTargetWarning(Instigator);
				}
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

	/** Returns TRUE if weapon wants to define focus loc, false otherwise. */
	simulated function bool GetCameraDOFFocusLocation(out vector FocusLoc)
	{
		if (SimulatedGrenade != None)
		{
			FocusLoc = SimulatedGrenade.Location;
			return TRUE;
		}
		return Super.GetCameraDOFFocusLocation(FocusLoc);
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
	if( WP != None && WP.CoverType != CT_None && (WP.IsBlindFiring(WP.CoverAction) || WP.CoverAction == CA_Default) )
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

/** Player releases fire button, weapon fires a shot */
simulated state Fire
{
	simulated event bool IsFiring()
	{
		return TRUE;
	}

	simulated function BeginState( Name PreviousStateName )
	{
		local gearpawn GP;
		//`log( WorldInfo.TimeSeconds @ GetStateName() @ GetFuncName() );

		// Make sure we're using the default firing mode.
		SetCurrentFireMode(0);

		GP = GearPawn(Instigator);
		if(GP != none && GP.MyGearAI != none)
		{
			// if we're being fired from an AI who is dead in the process of dying, reset charge time to min so it droops
			if(GP.Health <= 0)
			{
				ChargeTimeCount=MinChargeTime;
			}
		}
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
			SetTimer(Delta, FALSE, nameof(PlayMaxCharge));
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
		EM_EndPointFullCharge.ParticleSystemComponent.DeactivateSystem();
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
		SimulatedGrenade.bAddBaseVelocity = FALSE;		// no base vel compensation for simulation, since that happens all in one frame
		if (SimulatedGrenade.TrailEmitter != None)
		{
			SimulatedGrenade.TrailEmitter.Destroy();
		}
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
		EM_EndPoint.LifeSpan = 0;
		EM_EndPoint.SetHidden(TRUE);
		EM_EndPoint.SetOnlyOwnerSee(TRUE);
//		EM_EndPoint.ParticleSystemComponent.SetTickGroup(TG_PostUpdateWork);

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
		EM_EndPointFullCharge.LifeSpan = 0;
		EM_EndPointFullCharge.SetHidden(TRUE);
		EM_EndPointFullCharge.SetOnlyOwnerSee(TRUE);
//		EM_EndPointFullCharge.ParticleSystemComponent.SetTickGroup(TG_PostUpdateWork);

		EM_EndPointFullCharge.SetTemplate(PS_EndPointFullCharge);
	}

	if( EM_EndPointFail == None )
	{
		EM_EndPointFail = Spawn(class'SpawnedGearEmitter',Instigator,, Location);
		EM_EndPointFail.LifeSpan = 0;
		EM_EndPointFail.SetHidden(TRUE);
		EM_EndPointFail.SetOnlyOwnerSee(TRUE);
//		EM_EndPointFail.ParticleSystemComponent.SetTickGroup(TG_PostUpdateWork);

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
	ImpactNormal	= GearProj_ArrowBase(SimulatedGrenade).ImpactNormal;
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
		SetTimer(0.3f, FALSE, nameof(OpenBow));
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
	SetTimer(AnimLength, FALSE, nameof(TensionIntroEnded));
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
	ClearTimer(nameof(TensionIntroEnded));
}


/**
 * Event called when player starts being tired.
 * Called on all net modes.
 */
simulated function TiredStarted()
{
	if( CurrentStyle == EAS_Opened || CurrentStyle == EAS_TargetingOpens )
	{
		// Play firing animation to close bow
		PlayWeaponAnim('Fire_B', 1.f, 0.05f, -1.f, FALSE, FALSE);
	}
	else
	{
		// Play firing animation to close bow
		PlayWeaponAnim('Fire', 1.f, 0.05f, -1.f, FALSE, FALSE);
	}
}


/**
 * Event called when player stops being tired.
 * Called on all net modes.
 */
simulated function TiredEnded()
{
}


/** Event called when weapon fired */
simulated function PlayFireEffects(byte FireModeNum, optional vector HitLocation)
{
	Super.PlayFireEffects(FireModeNum, HitLocation);

	if( CurrentStyle == EAS_Opened || CurrentStyle == EAS_TargetingOpens )
	{
		// Play firing animation to close bow
		PlayWeaponAnim('Fire_B', 1.f, 0.05f, -1.f, FALSE, FALSE);
	}
	else
	{
		// Play firing animation to close bow
		PlayWeaponAnim('Fire', 1.f, 0.05f, -1.f, FALSE, FALSE);
	}

	// Hide Arrow Mesh.
	if( ArrowMesh != None )
	{
		ArrowMesh.SetHidden(TRUE);
	}
}

simulated function PlayOwnedFireEffects(byte FireModeNum, vector HitLocation)
{
	Super.PlayOwnedFireEffects(FireModeNum, HitLocation);

	// Play launching sound
	// This is using non replicated variables, so replicate the resulting sound instead to remote clients.
	WeaponPlaySound(LaunchSound);
}


simulated function PlayWeaponReloading()
{
	Super.PlayWeaponReloading();

	if( CurrentStyle == EAS_TargetingOpens )
	{
		UpdateBowBasedOnTargeting();
	}
}

// Don't want to stop weapon animations on torque bow. Post reload causes issues.
simulated function StopWeaponAnim(optional float BlendOutTime);

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

	if( ArrowMesh != None )
	{
		ArrowMesh.SetHidden(FALSE);
	}
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

protected function bool IsFireState( name StateName )
{
	return ( (StateName == 'Charge') || (StateName == 'Fire') );
}

simulated function PerformWeaponAttachment(SkeletalMeshComponent MeshCpnt, optional Name SocketName)
{
	local SkeletalMeshComponent WeaponMesh;

	Super.PerformWeaponAttachment(MeshCpnt, SocketName);

	WeaponMesh = SkeletalMeshComponent(Mesh);
	if( ArrowMesh != None && WeaponMesh != None && !WeaponMesh.IsComponentAttached(ArrowMesh) )
	{
		WeaponMesh.AttachComponentToSocket(ArrowMesh, 'Arrow');

		// Set Shadow parent and light environment.
		ArrowMesh.SetShadowParent(MeshCpnt);
		ArrowMesh.SetLightEnvironment(GearPawn(Instigator).LightEnvironment);
	}
}


simulated function DetachWeapon()
{
	Super.DetachWeapon();

	if( ArrowMesh != None )
	{
		// remove shadow parent and light environment, as this causes a nice little crash when garbage collection is performed.
		ArrowMesh.SetShadowParent(None);
		ArrowMesh.SetLightEnvironment(None);
	}
}

/** Have a chance to not play the active reload failed animation */
simulated function bool ShouldPlayFailedActiveReload()
{
	// If we're more than half way through and the pawn is not holding the arrow anymore, then don't play the failed animation.
	if( (GetTimerCount('EndOfReloadTimer') / GetTimerRate('EndOfReloadTimer')) > 0.5f
		&& Instigator.Mesh != None && !Instigator.Mesh.IsComponentAttached(MagazineMesh) )
	{
		return FALSE;
	}

	return TRUE;
}

simulated function PlayMeleeAttack()
{
	super.PlayMeleeAttack();
	GearWeaponPlaySoundLocal(BowMeleeSwingSound);
}

defaultproperties
{
	CurrentStyle=EAS_Opened	// default animation code is bow opened. This can be changed for easy testing.

	FiringStatesArray(0)="Charge"
	WeaponFireTypes(0)=EWFT_Projectile

	bNoAnimDelayFiring=TRUE	// charge weapon, so no need to delay firing because of animation transitions.
	bWeaponCanBeReloaded=TRUE
	bAutoSwitchWhenOutOfAmmo=FALSE

	AIRating=1.f

	InstantHitDamageTypes(0)=class'GDT_TorqueBow_Impact'
	AmmoTypeClass=class'GearAmmoType_TorqueBow'

	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	FireOffset=(X=35,Y=0,Z=0)

	Begin Object Class=StaticMeshComponent Name=MagazineMesh0
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bAcceptsDynamicDecals=FALSE
		MotionBlurScale=0.0
	End Object
	MagazineMesh=MagazineMesh0

	Begin Object Class=StaticMeshComponent Name=ArrowMesh0
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bAcceptsDynamicDecals=FALSE
		MotionBlurScale=0.0
	End Object
	ArrowMesh=ArrowMesh0

	// muzzle flash emitter
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(R=192,G=180,B=144,A=255)
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.2f
	MuzzleLightPulseFreq=20
	MuzzleLightPulseExp=1.5

	DamageTypeClassForUI=class'GDT_TorqueBow_Impact'

	bIsSuppressive=false
	bSniping=true
	bCanNegateMeatShield=true
	bIgnoresExecutionRules=true

	Recoil_Hand={(
					LocAmplitude=(X=-8,Y=2,Z=-2),
					LocFrequency=(X=15,Y=10,Z=10),
					LocParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
					RotAmplitude=(X=1500,Y=200,Z=0),
					RotFrequency=(X=8,Y=5,Z=0),
					RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
					TimeDuration=0.5f
					)}

	Recoil_Spine={(
					RotAmplitude=(X=1000,Y=300,Z=0),
					RotFrequency=(X=10,Y=10,Z=0),
					RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
					TimeDuration=1.0f
					)}

	MinChargeTime=0.3

	LC_EmisDefaultCOG=(R=0.0,G=25.0,B=100.0,A=1.0)
    LC_EmisDefaultLocust=(R=30.0,G=30.0,B=30.0,A=1.0)

    WeaponID=WC_TorqueBow

	// CQC Long Executions
	CQC_Long_KillerAnim="CTRL_torquebow"
	CQC_Long_VictimAnim="DBNO_torquebow"
	CQC_Long_CameraAnims.Empty
	CQC_Long_bHolsterWeapon=FALSE
	CQC_Long_VictimDeathTime=1.6f
	CQC_Long_VictimRotStartTime=0.f
	CQC_Long_VictimRotInterpSpeed=0.f
	CQC_Long_DamageType=class'GDT_LongExecution_Bow'
	CQC_Long_EffortID=GearEffort_TorquebowLongExecutionEffort

	CQC_Quick_KillerAnim="CTRL_Quick_Torquebow"
	CQC_Quick_VictimDeathTime=0.3f
	CQC_Quick_EffortID=GearEffort_TorquebowQuickExecutionEffort
	CQC_Quick_DamageType=class'GDT_QuickExecution_Bow'

	// we run this in the PostUpdateWork tick group to handle the preview arc while riding on moving bases.
	// this isn't ideal, since we can't guarantee tick order within this group, so the whole purpose of this
	// group is violated, but luckily, it doesn't seem to be a problem in this instance.
	TickGroup=TG_PostUpdateWork
}

