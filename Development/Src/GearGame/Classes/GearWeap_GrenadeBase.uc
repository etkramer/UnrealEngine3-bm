/**
 * GearWeap_GrenadeBase
 * Gear grenade weapon implementation
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_GrenadeBase extends GearWeapon
	config(Weapon)
	native(Weapon)
	abstract;

var vector SimulationStart, SimulationDir;

/** Focus Point to adjust camera */
var	transient	vector	FocusPoint;

var GearProj_Grenade		SimulatedGrenade;

var Emitter					EM_StartPoint;
var ParticleSystem			PS_StartPoint;

var ParticleSystemComponent PSC_Trail;
var ParticleSystem			PS_TrailCOG;
var ParticleSystem			PS_TrailLocust;

var Emitter					EM_EndPoint;
var ParticleSystem			PS_EndPointCOG;
var ParticleSystem			PS_EndPointLocust;

/** Player animations */
var()	GearPawn.BodyStance	BS_PawnThrowGrenade;

var	const	GearPawn.BodyStance	BS_PawnPrepare;
var	const	GearPawn.BodyStance	BS_PawnLoop;
var	const	GearPawn.BodyStance	BS_PawnRelease;

var const	Name				WeapAnimPrepare;
var const	Name				WeapAnimLoop;
var const	Name				WeapAnimRelease;

/** Duration of throw animation, to time firing state */
var			float				ThrowAnimLength;

/** Is player spinning grenade? */
var	transient	bool			bSpinningGrenade;

/** was the grenade thrown bolo style (TRUE) or blind fired (FALSE) */
var transient	bool			bThrownBoloStyle;

/** global grenade animation play rate. For tweaking. */
var	const		float			GlobalPlayRate;

var(AI) config float			AI_TimeTweenGrenade;
var(AI) config float			AI_ChanceToUseGrenade;

/** Sound grenade makes as it is being thrown **/
var protected const SoundCue	ThrowGrenadeSound;
/** Sound grenade make swinging, triggered from AnimNotify. */
var protected const SoundCue	GrenadeSwingSound;


/** Internal flag for determining if the arc is being simulated **/
var protected transient bool	bDisplayingArc;
/** Min/Max pitch for camera when displaying the grenade aiming arc. */
var() const vector2d			GrenadeAimingCameraPitchLimit;
/** Input sensitivity scaling when displaying the grenade aiming arc. */
var() protected const float		GrenadeAimingSensitivityMultiplier;
/** Interp speed used when enforcing GrenadeAimingCameraPitchLimit.  (Long var names ftw.) */
var() protected const float		GrenadeAimingCameraPitchCorrectionInterpSpeed;
/** How much to pitch the grenade trajectory relative to the camera pitch. */
var() protected const float		GrenadeAimingTrajectoryPitchMultiplier;
/** How much extra (i.e. additive) to pitch the grenade trajectory relative to the camera pitch. */
var() protected const int		GrenadeAimingTrajectoryPitchAdjustment;
var() protected const vector2d	GrenadeVelRange;
var() protected const vector2d	GrenadeVelRangeBlindFiring;
var() protected const float		GrenadeVelRangePow;

/** offsets for where grenade originates */
var() protected vector FireStartOffset_Base;
var() protected vector FireStartOffset_CoverLeftOrRight;
var() protected vector FireStartOffset_CoverDefault;
var() protected vector FireStartOffset_CoverLean_Low;
var() protected vector FireStartOffset_CoverLean_High;
var() protected vector FireStartOffset_CoverBlindUp;
var() protected float FireStartOffset_CoverZOffset_Low;
var() protected float FireStartOffset_CoverZOffset_Mid;
var() protected float FireStartOffset_CoverZOffset_High;

var() protected vector2d TargetingFOVRange;
var() protected vector2d TargetingFOVDistRange;

/** This particle system is played when the owner is spinning the grenade ready to throw! **/
var protected const ParticleSystem SpinningParticleEffectTemplate;
var protected const name SpinningEffectSocketName;
var protected transient ParticleSystemComponent PSC_SpinningParticleEffect;

var() protected const int MinBlindFireUpPitch;
var() protected const float BlindFireUpThrowDelay;
var() protected const float BlindFireSideThrowDelay;
var() protected const float UntargetedThrowDelay;

/** Kill off the emitters that we spawned **/
simulated function Destroyed()
{
	if( SimulatedGrenade != none )
	{
		SimulatedGrenade.Destroy();
		SimulatedGrenade = none;
	}

	if( EM_StartPoint != none )
	{
		EM_StartPoint.Destroy();
		EM_StartPoint = none;
	}

	if( EM_EndPoint != none )
	{
		EM_EndPoint.Destroy();
		EM_EndPoint = none;
	}

	Super.Destroyed();
}

/** Grenades do not set material based on team as we have color coded grenades (blue=smoke, orange=frag, green=ink)**/
simulated function SetMaterialBasedOnTeam( MeshComponent TheMesh, int TeamNum );

/** Return TRUE to prevent targeting */
simulated function bool ShouldPreventTargeting()
{
	if( !ShouldForceTargeting() && !HasAnyAmmo() )
	{
		return TRUE;
	}

	return super.ShouldPreventTargeting();
}


/** Return TRUE to force targeting */
simulated function bool ShouldForceTargeting()
{
	// Instigator is throwing grenade bolo style, force him to stand up.
	if( Instigator.FlashCount > 0 && bThrownBoloStyle )
	{
		return TRUE;
	}

	return super.ShouldForceTargeting();
}

simulated function bool IsCriticalAmmoCount()
{
	return (Super.IsCriticalAmmoCount() && GetMagazineSize() - AmmoUsedCount <= 0);
}

simulated state MeleeAttacking
{
	simulated function EndOfMeleeAttack()
	{
		`LogInv("");
		// If player is still holding melee button
		if( PendingFire(MELEE_ATTACK_FIREMODE) )
		{
			// Clear flag to avoid going back to this state
			EndFire(MELEE_ATTACK_FIREMODE);
		}
		// If ammo left, then equip next grenade.
		if( Instigator.IsHumanControlled() && HasAnyAmmo() )
		{
			// And visually equip next grenade
			GotoState('EquipNextGrenade');
		}
		else
		{
			GotoState('Active');
		}
	}
}

/** Detach grenade mesh from pawn's hand when throwing it or attaching it somewhere */
simulated function DetachGrenade()
{
	`LogInv("");
	// Use SetFlashLocation as a way to replicate this
	SetFlashLocation(VRand()*100);
}

simulated function FlashLocationUpdated(byte FiringMode, vector FlashLocation, bool bViaReplication)
{
	// we override FlashLocation to indicate melee hits
	if( !IsZero(FlashLocation) )
	{
		DetachWeapon();
	}
}

simulated function GearProj_Grenade AttachToObject(Actor Victim, vector HitLoc, vector HitNorm, TraceHitInfo HitInfo)
{
	local GearProj_Grenade NadeProj;
	local float NormOffset;

	// Get offset
	NormOffset = class<GearProj_Grenade>(WeaponProjectiles[0]).default.GrenadeAttachNormalOffset;

	NadeProj = GearProj_Grenade(Spawn(WeaponProjectiles[0],,,HitLoc + (NormOffset*HitNorm),rotator(HitNorm) + rot(16384,0,0),,TRUE));
	NadeProj.HeadFacingNormal = HitNorm; // store off the direction the head of the GrenadeProj is facing (needed for decals on walls)
	NadeProj.bNetDirty = TRUE;

	NadeProj.AttachInit(Victim);
	ConsumeAmmo(0);
	DetachGrenade();

	return NadeProj;
}

function bool MeleeAttackDestructibles()
{
	local GearDestructibleObject WDO;
	local GearPawn WP;
	local vector StartTrace, EndTrace, HitLoc, HitNorm, TraceDir;
	local TraceHitInfo HitInfo;
	local bool bHitSomething;
	local GearPC PC;
	local Actor HitActor;
	local float StartBeyondDist, TraceScale;
	local Vector TraceMatch;
	local Trigger_ChainsawInteraction Trig;

	if (!HasAnyAmmo())
	{
		return FALSE;
	}

	WP = GearPawn(Instigator);
	StartTrace	= GetMeleeStartTraceLocation();
	// Trace direction
	TraceDir = vector(GetAdjustedAim(StartTrace));

	// See how far beyond the pawn location the trace starts.
	StartBeyondDist = (StartTrace - WP.Location) Dot TraceDir;
	StartBeyondDist = Max(StartBeyondDist, 0.0); // Ensure positive

	// And back up trace so we start inside cylinder
	StartTrace -= (StartBeyondDist * TraceDir);

	//@note - use reduced distance for wall tags, but keep normal player range
	// Shorten the trace a bit if aiming at the ground, so you can't grenade tag the floor 6 feet away
	TraceMatch.X = TraceDir.X;
	TraceMatch.Y = TraceDir.Y;
	TraceScale = 0.75f * GetRangeValueByPct( vect2d(0.75f,1.f), (TraceMatch DOT TraceDir) );

	EndTrace	= StartTrace + ( TraceDir * MeleeAttackRange * TraceScale );
	bHitSomething = FALSE;
	// attempt to hit an object in the world (e.g. destructible object)
	HitActor = WP.Trace(HitLoc,HitNorm,EndTrace,StartTrace,TRUE,vect(0,0,0),HitInfo,TRACEFLAG_Bullet); // Using zero extent here to avoid grenade-tagging blocking volumes

	if( HitActor != None )
	{
		HitLoc = HitLoc + Normal(EndTrace - StartTrace) * VSize(MeleeTraceExtent) * 0.3f;
		WDO = GearDestructibleObject(HitActor);
		Trig = Trigger_ChainsawInteraction(HitActor);
		if (WDO != None)
		{
			AttachToObject(HitActor,HitLoc,HitNorm,HitInfo);
			bHitSomething = TRUE;
		}
		else if(Trig != None)
		{
			bHitSomething = Trig.GrenadeTagged(self, HitLoc, HitNorm, HitInfo);
		}
		else
		{
			if (HitActor.bWorldGeometry || HitActor == WorldInfo)
			{
				AttachToObject(HitActor,HitLoc,HitNorm,HitInfo);
				bHitSomething = TRUE;
			}
		}
	}
	if( bHitSomething )
	{
		SetFlashLocation(VRand()); // Replicate so we can detach the grenade.

		PC = GearPC(Instigator.Controller);
		if( PC != None )
		{
			PC.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.MeleeHit);
		}
		MeleeAttackEnded();
		return TRUE;
	}

	return FALSE;
}

/** Overridden to attach a timed grenade to the player */
simulated function bool MeleeAttackImpact( optional int InDamage, optional out GearPawn OutVictimPawn, optional out TraceHitInfo OutHitInfo, optional bool bTest )
{
	local GearProj_Grenade NadeProj;
	local InterpActor Interp;
	local vector HitLoc, HitNorm, Start;

	if (!HasAnyAmmo())
	{
		return FALSE;
	}

	if ( (WorldInfo.NetMode != NM_Client) && Super.MeleeAttackImpact(InDamage, OutVictimPawn, OutHitInfo, bTest) )
	{
		NadeProj = GearProj_Grenade(Spawn(WeaponProjectiles[0],,,Instigator.Location + vect(0,0,35),,,TRUE));
		if( NadeProj != None && !NadeProj.bDeleteMe )
		{
			NadeProj.AttachInit(OutVictimPawn);
			ConsumeAmmo(0);
			DetachGrenade();
		}
		MeleeAttackEnded();
		return TRUE;
	}
	else
	{
		// check for any interp actors that receive damage
		Start = GetMeleeStartTraceLocation();
		Interp = InterpActor(Instigator.Trace(HitLoc,HitNorm,Start + vector(GearPawn(Instigator).GetBaseAimRotation()) * MeleeAttackRange,Start,TRUE,vect(8,8,32),,TRACEFLAG_Bullet));
		if (Interp != None &&
			Interp.FindEventsOfClass(class'SeqEvent_TakeDamage'))
		{
			NadeProj = GearProj_Grenade(Spawn(WeaponProjectiles[0],,,Instigator.Location,,,TRUE));
			if( NadeProj != None && !NadeProj.bDeleteMe )
			{
				NadeProj.AttachInit(Interp);
				ConsumeAmmo(0);
				DetachGrenade();
			}
			MeleeAttackEnded();
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Arms the grenade for a detonation.
 */
reliable server function ArmGrenade()
{
	local GearProj_Grenade NadeProj;

	// don't allow grenade martyr in co-op if friendly because it does nothing useful and ends the game
	if (GearGameSP_Base(WorldInfo.Game) == None || Instigator.GetTeamNum() != 0)
	{
		CurrentFireMode = 0;
		if (HasAmmo(CurrentFireMode) && IsInState('Active'))
		{
			NadeProj = GearProj_Grenade(Spawn(GetProjectileClass(),,,GetPhysicalFireStartLoc()));
			if (NadeProj != None && !NadeProj.bDeleteMe)
			{
				// attach the grenade to ourselves
				NadeProj.HoldInit(Instigator);
				ConsumeAmmo(CurrentFireMode);
			}
		}
	}
}


/** @see Weapon::GetPhysicalFireStartLoc() */
simulated native event Vector GetPhysicalFireStartLoc(optional vector AimDir);


/**
 * returns grenade class to throw/spawn. Used also by simulation to get proper default values/effects.
 *
 * @return	projectile class of grenade to throw.
 */
simulated final function class<GearProj_Grenade> GetGrenadeClass()
{
	return( class<GearProj_Grenade>(WeaponProjectiles[0]) );
}


/**
 * This will make the particle system for the grenade arc be invis.
 */
simulated final function StopSimulation()
{
	if( PSC_Trail != None )
	{
		if (bDisplayingArc==true)
		{
			PSC_Trail.SetHidden(TRUE);
			PSC_Trail.DeactivateSystem();

			if (EM_StartPoint.ParticleSystemComponent != None)
			{
				EM_StartPoint.ParticleSystemComponent.DeactivateSystem();
			}
			EM_StartPoint.SetHidden(TRUE);

			if (EM_EndPoint.ParticleSystemComponent != None)
			{
				EM_EndPoint.ParticleSystemComponent.DeactivateSystem();
			}
			EM_EndPoint.SetHidden(TRUE);
		}
	}

	if (Instigator != None && Instigator.IsLocallyControlled())
	{
		bDisplayingArc = FALSE;
		ServerSetDisplayingArc(false);
	}

	if( SimulatedGrenade != None )
	{
		SimulatedGrenade.SetupForSimulation(FALSE);
	}
}

/** tells the server when the arc is being displayed so grenade trajectory can be adjusted accordingly */
reliable server final function ServerSetDisplayingArc(bool bNowDisplaying)
{
	bDisplayingArc = bNowDisplaying;
}

/**
 * Get Projectile Spawn Location and Direction.
 */
simulated function GetProjectileFirePosition(out vector out_ProjLoc, out vector out_ProjDir)
{
	local vector		StartTrace, HitLocation, HitNormal;
	local actor HitActor;

	// This is where we would start an instant trace. (what CalcWeaponFire uses)
	StartTrace	= Instigator.GetWeaponStartTraceLocation();
	out_ProjDir	= Vector(GetAdjustedAim( StartTrace ));



	// this is the location where the projectile is spawned.
	out_ProjLoc	= GetPhysicalFireStartLoc(out_ProjDir);

	// make sure not embedded in something
	HitActor = Trace(HitLocation, HitNormal, out_ProjLoc, Location, false);
	if ( HitActor != None )
	{
		out_ProjLoc = Location;
	}
}


/**
 * This is the function that will create all of the particle systems and emitters
 * needed to draw the grenade aiming arc.
 *
 * This is called each time we through and we cache the emitters and particle systems
 * so we do not have to recreate them each time.
 *
 * We want to call this set up function as we can get into the state where the last tick
 * our emitters were valid  or our virtual projectile was valid but now this tick it is not
 * (the projectile falling out of the world will cause this to occur).
 *
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
		SimulatedGrenade = GearProj_Grenade(Spawn(GetProjectileClass(),,, GetPhysicalFireStartLoc()));
		if (SimulatedGrenade == None)
		{
			return;
		}
		SimulatedGrenade.InitForSimulation();
		SimulatedGrenade.SetOwner(Self);
	}

	SimulatedGrenade.SetupForSimulation(TRUE);

	// grenade arc creation
	if( EM_StartPoint == None )
	{
		EM_StartPoint = Spawn(class'SpawnedGearEmitter',Instigator,, Location);
		EM_StartPoint.LifeSpan = 0;
		EM_StartPoint.SetOnlyOwnerSee(TRUE);
		EM_StartPoint.SetHidden(True);

		EM_StartPoint.SetTemplate(PS_StartPoint);
//		EM_StartPoint.ParticleSystemComponent.bAutoActivate=false;
	}

	EM_StartPoint.ParticleSystemComponent.ActivateSystem();
	EM_StartPoint.SetHidden(True);

	if( EM_EndPoint == None )
	{
		EM_EndPoint = Spawn(class'SpawnedGearEmitter',Instigator,, Location);
		EM_EndPoint.LifeSpan = 0;
		EM_EndPoint.SetOnlyOwnerSee(TRUE);

		if( Instigator.Controller.GetTeamNum() == 0 )
		{
			EM_EndPoint.SetTemplate( PS_EndPointCOG );
		}
		else
		{
			EM_EndPoint.SetTemplate( PS_EndPointLocust );
		}

//		EM_EndPoint.ParticleSystemComponent.bAutoActivate=false;
	}

	EM_EndPoint.ParticleSystemComponent.ActivateSystem();
	EM_EndPoint.SetHidden(True);

	if( PSC_Trail == None )
	{
		PSC_Trail = new(Outer) class'ParticleSystemComponent';
		PSC_Trail.bAutoActivate = false;
		PSC_Trail.SetOnlyOwnerSee(TRUE);
		if( Instigator.Controller.GetTeamNum() == 0 )
		{
			PSC_Trail.SetTemplate( PS_TrailCOG );
		}
		else
		{
			PSC_Trail.SetTemplate( PS_TrailLocust );
		}

		PSC_Trail.SetActorParameter('TrailActor', SimulatedGrenade);
		SimulatedGrenade.AttachComponent(PSC_Trail);
	}

	PSC_Trail.SetHidden(FALSE);
	PSC_Trail.ActivateSystem();

	if (Instigator != None && Instigator.IsLocallyControlled())
	{
		bDisplayingArc = true;
		ServerSetDisplayingArc(true);
	}
}

/** We vary toss velocity somewhat, based on the pitch of the throw. */
simulated protected function vector GetGrenadeTossVelocity(vector TossDir, GearProj_Grenade NadeProj)
{
	local float Pct;
	local vector Vel;
	local vector2d VelRange;

	VelRange = bDisplayingArc ? GrenadeVelRange : GrenadeVelRangeBlindFiring;

	Pct = FClamp(GetRangePctByValue(GrenadeAimingCameraPitchLimit, rotator(TossDir).Pitch), 0.f, 1.f);
	Pct = Pct**GrenadeVelRangePow;
	Vel = Normal(SimulationDir) * GetRangeValueByPct(VelRange, Pct);

	if ( (NadeProj != None) && NadeProj.bAddBaseVelocity)
	{
		Vel += NadeProj.InitialFrameOfRefVelocity;
	}

	return Vel;
};

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
	local Vector ImpactLoc, StartLoc, ImpactNormal;

	// purely a client-side effect, no need for server to care
	if (WorldInfo.NetMode == NM_DedicatedServer ||
		PlayerController(Instigator.Controller) == None)
	{
		return;
	}

	if (bDisplayingArc == false || SimulatedGrenade == None)
	{
		SetUpSimulation();
	}

	if( SimulatedGrenade == None || SimulatedGrenade.bDeleteMe )
	{
		return;
	}

	StartLoc = SimulationStart;
	SimulatedGrenade.SetLocation(StartLoc);
	SimulatedGrenade.Init(SimulationDir);
	SimulatedGrenade.Velocity = GetGrenadeTossVelocity(SimulationDir, None);

	SimulatedGrenade.RunPhysicsSimulationTilEnd(SimulatedGrenade.MaxSimulationTime);
	ImpactLoc	= SimulatedGrenade.Location;
	// Have the camera try to have the impact point in view
	FocusPoint	= ImpactLoc;

	ImpactNormal = Vect(0,0,1);

	// after the simulation has completed make the arc vis
	if (Instigator.IsLocallyControlled())
	{
		// this doesn't look right just yet.  need to talk to Laurent and see
		// what the camera is doing to make it looks wrong
		EM_StartPoint.SetLocation(StartLoc);
		EM_StartPoint.SetHidden(False);

		// we need to check to make certain we are landing on the ground else
		// we need to disable the end emitter

		// we are in the air
		if( IsGrenadeTargetInAir(ImpactLoc) )
		{
			EM_EndPoint.SetHidden(TRUE);
		}
		// we are on the ground
		else
		{
			EM_EndPoint.SetHidden(FALSE);
		}

		if( !EM_EndPoint.bHidden )
		{
			// we need to move the GrenadeAimingArcEnd to the target location and rotate i
			EM_EndPoint.SetLocation(ImpactLoc);
			EM_EndPoint.SetRotation(Rotator(ImpactNormal));
		}
	}
}


/**
 * This will determine if the grenade is in the air or not.  It does a small trace
 * and if it does NOT hit something then it believes the grenade's target location
 * is in the air.
 */
function bool IsGrenadeTargetInAir(Vector TargetLoc)
{
	local vector		OutHitLocation, OutHitNormal, TraceDest, TraceStart, TraceExtent;
	local TraceHitInfo	HitInfo;
	local Actor			TraceActor;

	TraceStart	= TargetLoc + (Vect(0, 0, 1) * 2);
	TraceDest	= TargetLoc - (Vect(0, 0, 1) * 5);
	TraceActor	= Trace(OutHitLocation, OutHitNormal, TraceDest, TraceStart, FALSE, TraceExtent, HitInfo);

	return (TraceActor == None);
}


/** returns focus point to adjust camera */
simulated function vector GetFocusPoint()
{
	if (SimulatedGrenade != None)
	{
		return SimulatedGrenade.FirstBounceLoc;
	}

	return vect(0,0,0);
}

/**
* Returns the location that should be the start of traces for MeleeAttackImpact()
*/
simulated function vector GetMeleeStartTraceLocation()
{
	// grenades need to come from the weapon so they can be stuck to the enemy
	return GetPhysicalFireStartLoc();
}


/**
 * Returns Aim rotation to toss the grenade. (for spawning and simulation).
 * FIXME, should work the same on both client and server.
 *
 * @param	StartFireLoc	world location when grenade would be spawned.
 * @return					Aim direction to use when spawning the grenade.
 */
simulated function vector GetGrenadeTossDirection( Vector StartFireLoc )
{
	local Rotator AimRot;
	local GearPawn InstigatorGP;


	//@fixme - controllers don't exist on clients
	AimRot = Rotation;
	// use controller rotation, and not camera, as camera rotation is adjusted to show grenade's destination.
	if (Instigator != None)
	{
		if (Instigator.Controller != None)
		{
			AimRot = Instigator.Controller.Rotation;
		}
		else
		{
			AimRot = Instigator.Rotation;
		}

		// because player input was divided by half to limit camera angle, we compensate here, so player has same throwing pitch range
		AimRot.Pitch = GrenadeAimingTrajectoryPitchMultiplier * ( NormalizeRotAxis(AimRot.Pitch) + GrenadeAimingTrajectoryPitchAdjustment );

		// clamp the pitch when blindfiring over cover
		InstigatorGP = GearPawn(Instigator);
		if (InstigatorGP != None && InstigatorGP.CoverAction == CA_BlindUp)
		{
			AimRot.Pitch = Max(MinBlindFireUpPitch, Normalize(AimRot).Pitch);
		}
	}

	return vector(AimRot);
}


simulated function bool ShouldRefire()
{
	local GearAI AI;

	AI = GearAI(Instigator.Controller);
	if( AI != None )
	{
		return FALSE;
	}

	return super.ShouldRefire();
}

simulated function HandleFinishedFiring()
{
	if (AIController != None)
	{
		// AI only ever fires one at a time, so stop now and pick a new weapon
		StopFire(CurrentFireMode);
		AIController.StopFiring();
		Super(Weapon).HandleFinishedFiring();
		GearAI(AIController).SelectWeapon();
	}
	else
	{
		Super.HandleFinishedFiring();
	}
}

/**
 * Return TRUE if grenade prediction Arc should be drawn.
 */
simulated function bool ShouldDrawTrajectoryPrediction()
{
	local GearPawn P;

	// uncomment for easier trajectory debugging
	//return TRUE;

	P = GearPawn(Instigator);
	return (P != None && P.bIsTargeting && (P.CoverType == CT_None || P.IsLeaning() || P.bDoing360Aiming) && P.IsLocallyControlled() && P.IsHumanControlled());
}


/**
 * Event called when Pawn.FiringMode has been changed.
 * bViaReplication indicates if this was the result of a replication call.
 */
simulated function FireModeUpdated(byte FiringMode, bool bViaReplication)
{
	if( FiringMode == FIREMODE_CHARGE )
	{
		PlayGrenadeReEquip();
	}

	if( bViaReplication )
	{
		// client-side grenade spinning
		if( FiringMode == FIREMODE_GRENADESPIN )
		{
			StartGrenadeSpin();
		}
		else if( FiringMode == FIREMODE_STOPGRENADESPIN )
		{
			StopGrenadeSpin();
		}
	}

	Super.FireModeUpdated(FiringMode, bViaReplication);
}


/** Active State */
simulated state Active
{
	simulated function BeginState(Name PrevStateName)
	{
		local GearPawn	P;

		// always check between throws
		SimulationStart = GetPhysicalFireStartLoc();
		SimulationDir = Normal(GetGrenadeTossDirection(SimulationStart));

		// If Pawn is targeting, start the animations...
		P = GearPawn(Instigator);
		if( P != None && P.bIsTargeting )
		{
			CheckGrenadeSpinning();
		}

		// little hack to auto reload grenade, since we don't actually "reload" it.
		if( ShouldAutoReload() )
		{
			PerformReload();
		}

		Super.BeginState(PrevStateName);
	}

	simulated function EndState(Name NextStateName)
	{
		if( NextStateName != 'ThrowingGrenade' )
		{
			StopSimulation();
		}

		Super.EndState(NextStateName);
	}

	simulated function Tick(float DeltaTime)
	{
		Super.Tick(DeltaTime);

		SimulationStart = GetPhysicalFireStartLoc();
		SimulationDir = Normal(GetGrenadeTossDirection(SimulationStart));

		// Draw grenade prediction Arc
		if( ShouldDrawTrajectoryPrediction() )
		{
			SimulateTrajectory();
		}
		else
		{
			StopSimulation();
		}
		CheckGrenadeSpinning();
	}

	/** returns focus point to adjust camera */
	//simulated function vector GetFocusPoint()
	//{
	//	if( ShouldDrawTrajectoryPrediction() )
	//	{
	//		return SimulatedGrenade.FirstBounceLoc;
	//		//return FocusPoint;
	//	}

	//	return Vect(0, 0, 0);
	//}

	/**
	 * extended to reduce player view rotation when simulating grenade trajectory, so the arc remains visible when aiming up or down.
	 * essentially, it halves the viewrotation's Pitch, with a pretty smooth interpolation.
	 */
	simulated function ProcessViewRotation(float DeltaTime, out rotator OutViewRot, out Rotator OutDeltaRot)
	{
		if( bDisplayingArc )
		{
			// limit how high camera can look and we divide player input by half
			OutDeltaRot.Pitch = NormalizeRotAxis(OutDeltaRot.Pitch) * GrenadeAimingSensitivityMultiplier;
			SClampRotAxis(DeltaTime, OutViewRot.Pitch, OutDeltaRot.Pitch, GrenadeAimingCameraPitchLimit.Y, GrenadeAimingCameraPitchLimit.X, GrenadeAimingCameraPitchCorrectionInterpSpeed);
		}
		Super.ProcessViewRotation(DeltaTime, OutViewRot, OutDeltaRot);
	}
}

simulated function FinishedThrow();

simulated state ThrowingGrenade
{
	simulated event bool IsFiring()
	{
		return TRUE;
	}

	simulated function FinishedThrow()
	{
		// If ammo left, then equip next grenade.
		if( Instigator.IsHumanControlled() && HasAnyAmmo() )
		{
			// And visually equip next grenade
			GotoState('EquipNextGrenade');
		}
		else
		{
			HandleFinishedFiring();
		}
	}

	simulated function Tick(float DeltaTime)
	{
		Super.Tick(DeltaTime);

		SimulationStart = GetPhysicalFireStartLoc();
		SimulationDir = Normal(GetGrenadeTossDirection(SimulationStart));

		// Draw grenade prediction Arc
		if( ShouldDrawTrajectoryPrediction() )
		{
			SimulateTrajectory();
		}
		else
		{
			StopSimulation();
		}


		// show it until the grenade is thrown
		//if( bDisplayingArc )
		//{
		//	// turn off the arc if moving while throwing
		//	if (VSize(SimulationStart - GetPhysicalFireStartLoc()) > 64.f)
		//	{
		//		StopSimulation();
		//	}
		//	else
		//	{
		//		SimulateTrajectory();
		//	}
		//}
	}

	simulated function BeginState(Name PreviousStateName)
	{
		local GearPawn GP;
		GP = GearPawn(Instigator);
		if (GP.CoverType == CT_Standing && GP.CoverAction == CA_Default)
		{
			SetTimer( 0.25f,FALSE,nameof(StartThrow) );
		}
		else
		{
			StartThrow();
		}
	}

	simulated function EndState(Name NextStateName)
	{
		// no longer show the arc
		StopSimulation();

		ClearFlashCount();

		NotifyWeaponFinishedFiring( CurrentFireMode );
	}

	/**
	 * extended to reduce player view rotation when simulating grenade trajectory, so the arc remains visible when aiming up or down.
	 * essentially, it halves the viewrotation's Pitch, with a pretty smooth interpolation.
	 */
	simulated function ProcessViewRotation(float DeltaTime, out rotator OutViewRot, out Rotator OutDeltaRot)
	{
		if( bDisplayingArc )
		{
			// limit how high camera can look and we divide player input by half
			OutDeltaRot.Pitch = NormalizeRotAxis(OutDeltaRot.Pitch) * GrenadeAimingSensitivityMultiplier;
			SClampRotAxis(DeltaTime, OutViewRot.Pitch, OutDeltaRot.Pitch, GrenadeAimingCameraPitchLimit.Y, GrenadeAimingCameraPitchLimit.X, GrenadeAimingCameraPitchCorrectionInterpSpeed);
		}
		Super.ProcessViewRotation(DeltaTime, OutViewRot, OutDeltaRot);
	}
}

simulated function StartThrow()
{
	// Start the throw animation
	StartGrenadeThrow();

	// Set up timing for this state, until we go back to the active state
	SetTimer( GetFireInterval(0), FALSE, 'FinishedThrow' );
}


simulated state EquipNextGrenade
{
	simulated function BeginState(Name PreviousStateName)
	{
		`LogInv("");

		// Reload
		PerformReload();

		if (Instigator != None)
		{
			Instigator.Controller.bFire = 0;
		}

		SetCurrentFireMode(FIREMODE_CHARGE);
	}

	simulated function GrenadeReEquipped()
	{
		Global.GrenadeReEquipped();

		`LogInv("");
		SetCurrentFireMode(0);
		GotoState('Active');
	}
}

/** ReEquip a new grenade. Triggered by FIREMODE_CHARGE */
simulated function PlayGrenadeReEquip()
{
	local GearPawn P;

	`LogInv("");
	P = GearPawn(Instigator);
	if( P != None )
	{
		// Stop all firing animations.
		P.BS_Stop(BS_PawnRelease, 0.2f);
		P.BS_Stop(BS_PawnThrowGrenade, 0.2f);

		P.bSwitchingWeapons = TRUE;
		P.PlaySound(WeaponEquipSound);
		SetTimer( 0.15f, FALSE, nameof(AttachNewGrenade) );
		if( EquipTime > 0.f )
		{
			P.BS_PlayByDuration(EquipShoulderRight, EquipTime, 0.1f, EquipTime * 0.5f);
			SetTimer( EquipTime, FALSE, nameof(GrenadeReEquipped) );
		}
		else
		{
			GrenadeReEquipped();
		}
	}
	else
	{
		GrenadeReEquipped();
	}
}

simulated function AttachNewGrenade()
{
	local GearPawn P;

	`LogInv("");
	P = GearPawn(Instigator);
	if( P != None )
	{
		if( P.MyGearWeapon != None )
		{
			P.AttachWeapon();
		}
		StopWeaponAnim(0.f);
	}
}

simulated function GrenadeReEquipped()
{
	local GearPawn P;

	`LogInv("");
	P = GearPawn(Instigator);
	if( P != None )
	{
		P.bSwitchingWeapons = FALSE;
	}
}

/** Called when starting to throw the grenade. When entering ThrowingGrenade state. */
simulated function StartGrenadeThrow()
{
	local GearPC MyGearPC;

	// tell remote clients that we fired, to trigger effects
	IncrementFlashCount();

	// Trigger the throw grenade event in the PC
	if (Instigator != None)
	{
		MyGearPC = GearPC(Instigator.Controller);
		if ( MyGearPC != None )
		{
			MyGearPC.TriggerGearEventDelegates( eGED_GrenadeToss );
		}
	}
}


/** ThrowGrenade Projectile */
function GearProj_Grenade ThrowGrenade()
{
	local Vector		SpawnLoc, TargetLoc;
	local GearProj_Grenade	NadeProj;
	local Vector		Toss;
	local GearAI	AI;

	if ( WorldInfo.NetMode == NM_Client )
	{
		return None;
	}

	if(Instigator == none)
	{
		return None;
	}

	ConsumeAmmo(0);
	DetachGrenade();

	SpawnLoc = GetPhysicalFireStartLoc();
	NadeProj = GearProj_Grenade(Spawn(GetProjectileClass(),,,SpawnLoc));

	if( NadeProj != None && !NadeProj.bDeleteMe )
	{
		NadeProj.Init( SimulationDir );

		AI = GearAI(Instigator.Controller);
		if( AI != None )
		{
			TargetLoc    = AI.GetFireTargetLocation();
			if(AI.Enemy != none)
			{
				TargetLoc.Z -= AI.Enemy.GetCollisionHeight();
			}

			SuggestTossVelocity( Toss, TargetLoc, SpawnLoc, GrenadeVelRange.y*1.2f, Instigator.Velocity.Z,0.45f );

			//`log("ActualToss"@Toss@"START"@SpawnLoc@"END"@TargetLoc@bRet@Instigator.Velocity.Z);
			NadeProj.Velocity = Toss;
			NotifyWeaponFired( CurrentFireMode );
		}
		else
		{
			NadeProj.Velocity = GetGrenadeTossVelocity(SimulationDir, NadeProj);
		}
	}

	return NadeProj;
}


/**
 * Grenade projectile has been thrown.
 * Match up projectile with hand attachment.
 */
simulated function GrenadeThrownNotification(GearProj_Grenade NadeProj)
{
	local GearAnim_Slot	ProjCustomAnimNode;

	if ( NadeProj != None )
	{
		ProjCustomAnimNode	= NadeProj.GetFullSlotNode();
		ProjCustomAnimNode.PlayCustomAnim('GR_Airborn_Loop', GlobalPlayRate, 0.f, 0.f, TRUE, FALSE);
	}

	// detach weapon from Pawn
	DetachWeapon();

	WeaponPlaySound(ThrowGrenadeSound);
}

simulated function NotifyGrenadeSwing()
{
	// @fixme, no need to replicate this since it's driven by the anim?
	WeaponPlaySound(GrenadeSwingSound);
}



simulated function bool ShouldDrawCrosshair()
{
	return FALSE;
}


/** Grenades don't reload */
simulated function PlayWeaponReloading()
{
	SetTimer( 0.001f, FALSE, nameof(EndOfReloadTimer) );
}

/** Notification that Targeting Mode has changed. */
simulated function TargetingModeChanged(GearPawn P)
{
	Super.TargetingModeChanged(P);

	CheckGrenadeSpinning();
}


/** Check if grenade should be spinning or not */
simulated function CheckGrenadeSpinning()
{
	local GearPawn P;


	if ( (Instigator == None) || (Instigator.Role < ROLE_AutonomousProxy) )
		return;

	P = GearPawn(Instigator);
	if( P != None && P.bIsTargeting && HasAnyAmmo() && !bWeaponPutDown && !P.bSwitchingWeapons && IsActiveWeapon() )
	{
		if( !bSpinningGrenade )
		{
			// Starts Targeting
			StartGrenadeSpin();
		}
	}
	else if( bSpinningGrenade )
	{
		// Stops Targeting
		StopGrenadeSpin();
	}
}

/** Start the grenade spinning animation */
simulated function StartGrenadeSpin()
{
	local FLOAT		PlayLength;
	local GearPawn	P;

	P = GearPawn(Instigator);

	P.BS_Play(BS_PawnPrepare, GlobalPlayRate, 0.1f, -1.f, FALSE, FALSE);
	PlayLength	= PlayWeaponAnim(WeapAnimPrepare, GlobalPlayRate, 0.1f, -1.f, FALSE, FALSE);
	SetCurrentFireMode(FIREMODE_GRENADESPIN);

	SetTimer( PlayLength, FALSE, nameof(PreparedNotify) );

	bSpinningGrenade = TRUE;

	if( PSC_SpinningParticleEffect == none )
	{
		if( SpinningParticleEffectTemplate != none )
		{
			PSC_SpinningParticleEffect = GearGRI(WorldInfo.GRI).GOP.GetImpactParticleSystemComponent( SpinningParticleEffectTemplate );
			SkeletalMeshComponent(Mesh).AttachComponentToSocket(  PSC_SpinningParticleEffect, SpinningEffectSocketName );
		}
	}
	// so now after possibly spawning any effect we check to see if we actually have one to activate
	if( PSC_SpinningParticleEffect != none )
	{
		PSC_SpinningParticleEffect.ActivateSystem();
	}
}


/** Stop the grenade spinning animation */
simulated function StopGrenadeSpin()
{
	local GearPawn	P;

	P = GearPawn(Instigator);

	P.BS_Stop(BS_PawnPrepare, 0.2f);
	P.BS_Stop(BS_PawnLoop, 0.2f);
	StopWeaponAnim(0.2f);

	ClearTimer('PreparedNotify');
	SetCurrentFireMode(FIREMODE_STOPGRENADESPIN);

	bSpinningGrenade = FALSE;

	if( PSC_SpinningParticleEffect != none )
	{
		PSC_SpinningParticleEffect.DeactivateSystem();
	}
}


/** Notification called when prepared animation is done playing */
simulated function PreparedNotify()
{
	local GearPawn	P;
	local float		ExcessTime, NewPlayRate;

	P = GearPawn(Instigator);
	if( P != None && P.bIsTargeting && !bWeaponPutDown && !P.bSwitchingWeapons && IsActiveWeapon() )
	{
		// Find out by how much we went over the ideal time.
		// By correcting the new animation, this makes a seamless transition.
		ExcessTime = GetTimerCount('PreparedNotify') - GetTimerRate('PreparedNotify');

		// Play looping animation
		P.BS_Play(BS_PawnLoop, GlobalPlayRate, 0.f, 0.f, TRUE, FALSE);
		NewPlayRate = P.BS_GetPlayRate(BS_PawnLoop);
		P.BS_SetPosition(BS_PawnLoop, ExcessTime * NewPlayRate);

		PlayWeaponAnim(WeapAnimLoop, GlobalPlayRate, 0.f, 0.f, TRUE, FALSE);
		SetWeaponAnimPosition(ExcessTime * NewPlayRate);

		bSpinningGrenade = TRUE;
	}
}

/** @see AGearWeapon::PlayFireEffects */
simulated function PlayFireEffects(byte FireModeNum, optional vector HitLocation)
{
	local GearPawn	P;
	local float Delay;

	P = GearPawn(Owner);
	if( P == None )
	{
		return;
	}

	if( bSpinningGrenade )
	{
		// Clear timer to play looping animation if it hasn't been triggered yet
		ClearTimer('PreparedNotify');

		// Play Release animation
		// Throw animation doesn't blend out to idle, so we can blend directly to the reEquip animation
		// without blending to the idle first.
		ThrowAnimLength = P.BS_Play(BS_PawnRelease, GlobalPlayRate, 0.1f, -1.f, FALSE, FALSE);
		PlayWeaponAnim(WeapAnimRelease, GlobalPlayRate, 0.1f, 0.1f, FALSE, FALSE);

		// Set Timer to throw grenade.
		SetTimer( 0.9f, FALSE, nameof(AnimNotifyThrow) );

		bSpinningGrenade	= FALSE;
		bThrownBoloStyle	= TRUE;
	}
	else
	{
		// Blind fire
		bThrownBoloStyle	= FALSE;

		// Play Throwing animation
		ThrowAnimLength = P.BS_Play(BS_PawnThrowGrenade, 1.f, 0.1f, -1.f, FALSE);

		// Set Timer to throw grenade.
		switch (P.CoverAction)
		{
		case CA_BlindUp:
			Delay = BlindFireUpThrowDelay;
			break;
		case CA_BlindLeft:
		case CA_BlindRight:
			Delay = BlindFireSideThrowDelay;
			break;
		default:
			// standard underhand throw
			Delay = UntargetedThrowDelay;
		}

		SetTimer( Delay, FALSE, nameof(AnimNotifyThrow) );
		SetTimer( Delay*0.5f, FALSE, nameof(PlayGrenadeThrowEffort) );
	}
}

simulated function float PlayGrenadeThrowAnimation()
{

}

simulated function PlayGrenadeThrowEffort()
{
	local GearPawn GP;
	GP = GearPawn(Instigator);
	if (GP != None)
	{
		GP.SoundGroup.PlayEffort( GP, GearEffort_BoloReleaseEffort );
	}
}


/** Put down current weapon. */
simulated function PutDownWeapon()
{
	local GearPawn	P;

	Super.PutDownWeapon();

	// When weapon is being put down, stop grenade spin.
	StopGrenadeSpin();

	P = GearPawn(Owner);
	if( P == None )
	{
		return;
	}

	// Stop all firing animations.
	P.BS_Stop(BS_PawnRelease, 0.2f);
	P.BS_Stop(BS_PawnThrowGrenade, 0.2f);
}


/** Time firing state by animation duration */
simulated function float GetFireInterval(byte FireModeNum)
{
	return ThrowAnimLength;
}

/** Notification from animation that grenade should be thrown */
simulated function AnimNotifyThrow()
{
	local GearProj_Grenade	NadeProj;

	NadeProj = ThrowGrenade();
	GrenadeThrownNotification(NadeProj);

    // we just tossed our grenade so we need to reset this PSC
	PSC_SpinningParticleEffect = None;

	// effort sound on throw.  unngg!
	if (bThrownBoloStyle)
	{
		PlayGrenadeThrowEffort();
	}
}

/** @return whether we should consider throwing a grenade; checks for last time we did so, random scaling, etc */
simulated function bool ShouldTryToThrowGrenade(GearAI AI, vector EnemyLocation, out int EnemiesInRange)
{
	local bool bAttackingKidnapper;
	local float ChanceScale, DmgRadius, Dist;
	local Pawn P;

	bAttackingKidnapper = (GearPawn(AI.Enemy) != None && GearPawn(AI.Enemy).IsAKidnapper());
	if ((AI.IsAtCover() || bAttackingKidnapper) && WorldInfo.TimeSeconds - AI.LastGrenadeTime > AI_TimeTweenGrenade)
	{
		ChanceScale = AI.GrenadeChanceScale;
		if (bAttackingKidnapper)
		{
			ChanceScale *= 2.0;
		}

		// count the number of enemies in range
		DmgRadius = GetGrenadeClass().static.GetAIEffectiveRadius();
		EnemiesInRange = 1; // Count our enemy
		foreach AI.Squad.AllEnemies(class'Pawn', P, AI)
		{
			if (P != AI.Enemy && (GearPawn(P) == None || !GearPawn(P).IsAHostage()))
			{
				Dist = VSize(AI.GetEnemyLocation(P) - EnemyLocation);
				if (Dist < DmgRadius * 0.75f)
				{
					EnemiesInRange++;
				}
			}
		}

		// on higher difficulties up the chance if multiple enemies may be affected
		if (AI.GetDifficultyLevel() > DL_Normal)
		{
			ChanceScale *= 1.0 + (0.5 * float(EnemiesInRange));
		}

		return (FRand() < AI_ChanceToUseGrenade * ChanceScale);
	}
	else
	{
		return false;
	}
}

simulated function float GetWeaponRating()
{
	local float Rating;
	local GearAI AI;
	local int Count;
	local Vector EnemyLocation;

	Rating = -1.0;
	if (!Instigator.IsHumanControlled() && Instigator.Controller.Enemy != None)
	{
		AI = GearAI(Instigator.Controller);
		if( AI != None && AI.Squad != None )
		{
			EnemyLocation = AI.GetEnemyLocation();
			// cap max range at approximately the best range a human can get on it on even ground
			if (VSize(Instigator.Location - EnemyLocation) < 2500.0 && ShouldTryToThrowGrenade(AI, EnemyLocation, Count))
			{
				// Select grenades
				Rating = 1.f + 0.25 * Count;

				//debug
				`AILog_Ext( self@GetFuncName()@Count@AI.Enemy, 'Weapon', AI );
			}
		}
	}

	return Rating;
}


/** Grenades don't have muzzles so we just want the hand location**/
simulated native event vector GetMuzzleLoc();


simulated event DummyFire(byte FireModeNumm, vector TargetLoc, optional Actor AttachedTo, optional float AimErrorDeg, optional Actor TargetActor)
{
	local vector SpawnLoc;
	local GearProj_Grenade NadeProj;

	SpawnLoc = Location;
	NadeProj = GearProj_Grenade(Spawn(GetProjectileClass(),,,SpawnLoc));

	if( NadeProj != None && !NadeProj.bDeleteMe )
	{
		// throw directly towards target loc
		NadeProj.Init( Normal(TargetLoc-SpawnLoc) );
	}
}

function HolderDied()
{
	local actor Proj;
	if((bSpinningGrenade || IsTimerActive('AnimNotifyThrow')) && AIController != none)
	{
		ClearTimer('AnimNotifyThrow');
		Proj = Spawn(WeaponProjectiles[0],,,AIController.Pawn.Location,,,true);
		Proj.Instigator = none;

	}
	Super.HolderDied();
}


simulated function float GetTargetingFOV(float BaseFOV)
{
	local vector FocusPt;
	local float Pct;

	FocusPt = GetFocusPoint();
	Pct = FClamp(GetRangePctByValue(TargetingFOVDistRange, VSize(FocusPt-Instigator.Location)), 0.f, 1.f);
	return GetRangeValueByPct(TargetingFOVRange, Pct);
}

/** Overridden to skip the CurrentFireMode stuff. */
simulated function class<Projectile> GetProjectileClass()
{
	return WeaponProjectiles[0];
}

simulated function PlayNeedsAmmoChatter()
{
	GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_NeedAmmoGrenade, Instigator, None);
}


function bool CanHit(vector ViewPt, vector TestLocation, rotator ViewRotation)
{
	local vector TossVelocity;
	local bool bRet;
	bRet = Instigator.SuggestTossVelocity( TossVelocity, TestLocation, GetPhysicalFireStartLoc(), GrenadeVelRange.y*1.2f,Instigator.Velocity.Z,0.45f);

	//`log("CANHITVELO"@TossVelocity@"START"@GetPhysicalFireStartLoc()@"END"@TestLocation);
	return bRet;
}

function ForceThrowGrenade( GearAI AI );


defaultproperties
{
	// we run this in the PostUpdateWork tick group to handle the preview arc while riding on moving bases.
	// this isn't ideal, since we can't guarantee tick order within this group, so the whole purpose of this
	// group is violated, but luckily, it doesn't seem to be a problem in this instance.
	TickGroup=TG_PostUpdateWork

	FiringStatesArray(0)="ThrowingGrenade"
	FiringStatesArray(RELOAD_FIREMODE)=""
	InstantHitDamageTypes(0)=class'GDT_Explosive'
	WeaponType=WT_Item
	WeaponAnimType=EWAT_Pistol

	AIRating=-1.f

	bAllowIdleStance=TRUE
	bAllowAimingStance=FALSE
	bAllowDownsightsStance=FALSE
	bWeaponCanBeReloaded=TRUE
	bUseTargetingCamera=TRUE
	bCanSelectWithoutAmmo=FALSE

	GlobalPlayRate=1.0f

	BS_PawnWeaponReload={()}  // no reload for grenades

	BS_PawnPrepare={(
		AnimName[BS_Std_Up]				="GR_Prepare",
		AnimName[BS_Std_Idle_Lower]		="GR_Prepare",
		AnimName[BS_CovStdLean_Up]		="GR_Cov_Std_Lean_Prepare",
		AnimName[BS_CovMidLean_Up]		="GR_Cov_Std_Lean_Prepare",
		AnimName[BS_CovStd_360_Upper]	="GR_Cov_Std_Back_Prepare",
		AnimName[BS_CovMid_360_Upper]	="GR_Cov_Std_Back_Prepare"
	)}
	BS_PawnLoop={(
		AnimName[BS_Std_Up]				="GR_Loop",
		AnimName[BS_Std_Idle_Lower]		="GR_Loop",
		AnimName[BS_CovStdLean_Up]		="GR_Cov_Std_Lean_Loop",
		AnimName[BS_CovMidLean_Up]		="GR_Cov_Std_Lean_Loop",
		AnimName[BS_CovStd_360_Upper]	="GR_Cov_Std_Back_Loop",
		AnimName[BS_CovMid_360_Upper]	="GR_Cov_Std_Back_Loop"
	)}
	BS_PawnRelease={(
		AnimName[BS_Std_Up]				="GR_Release",
		AnimName[BS_Std_Idle_Lower]		="GR_Release",
		AnimName[BS_CovStdLean_Up]		="GR_Cov_Std_Lean_Release",
		AnimName[BS_CovMidLean_Up]		="GR_Cov_Std_Lean_Release",
		AnimName[BS_CovStd_360_Upper]	="GR_Cov_Std_Back_Release",
		AnimName[BS_CovMid_360_Upper]	="GR_Cov_Std_Back_Release"
	)}

	WeapAnimPrepare="GR_Prepare"
	WeapAnimLoop="GR_Loop"
	WeapAnimRelease="GR_Release"

	BS_PawnThrowGrenade={(
 	    AnimName[BS_Std_Up]			    ="GR_Idle_Ready_Blind_Release",
		AnimName[BS_Std_Idle_Lower]	    ="GR_Idle_Ready_Blind_Release",
		AnimName[BS_CovMidBlindSd_Up]	="GR_Cov_Mid_Blind_Release",
		AnimName[BS_CovMidBlindUp_Up]	="GR_Cov_Mid_Blind_Up_Release",
		AnimName[BS_CovMid_360_Upper]	="GR_Cov_Mid_Back_Blind_Release",
		AnimName[BS_CovStdBlind_Up]		="GR_Cov_Std_Blind_Release",
		AnimName[BS_CovStd_360_Upper]	="GR_Cov_Std_Back_Blind_Release",
	)}

	// Weapon anim set. Grenades are on top of pistol animations
	CustomAnimSets.Empty()
	CustomAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Pistol')
	CustomAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_Overlay_Pistol')
	CustomAnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Grenade')

	AimOffsetProfileNames(0)="Pistol"
	AimOffsetProfileNames(1)="GrenadeSpin"

	BS_MeleeAttack.Empty()
	BS_MeleeAttack(0)=(AnimName[BS_Std_Up]="AR_Melee_Smack_A",AnimName[BS_Std_Idle_Lower]="AR_Melee_Smack_A")
	BS_MeleeAttack(1)=(AnimName[BS_Std_Up]="AR_Melee_Smack_B",AnimName[BS_Std_Idle_Lower]="AR_Melee_Smack_B")

	WeaponWhipSound=none
	WeaponReloadSound=none

	WeaponEquipSound=SoundCue'Weapon_Grenade.Actions.GrenadeBoloLowerCue'
	WeaponDeEquipSound=SoundCue'Weapon_Grenade.Actions.GrenadeBoloRaiseCue'

	PickupSound=SoundCue'Weapon_Grenade.Actions.GrenadePickupCue'

	ThrowGrenadeSound=SoundCue'Weapon_Grenade.Firing.GrenadeBoloThrowCue'
	GrenadeSwingSound=SoundCue'Weapon_Grenade.Actions.GrenadeBoloSwingCue'

	PS_StartPoint=ParticleSystem'GrenadeAimArcTrail.PS_GrenadeArc_Start'

	PS_TrailCOG=ParticleSystem'GrenadeAimArcTrail.PS_GrenadeAimArcTrail'
	PS_EndPointCOG=ParticleSystem'GrenadeAimArcTrail.PS_GrenadeArc_End'

	PS_TrailLocust=ParticleSystem'GrenadeAimArcTrail.PS_GrenadeAimArcTrail_Red'
	PS_EndPointLocust=ParticleSystem'GrenadeAimArcTrail.PS_GrenadeArc_End_Red'

	EquipTime=0.5f
	PutDownTime=0.5f

	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=192,V=240,UL=143,VL=46)
	IconXOffset = 0
	IconYOffset = 10

	HUDDrawData			= (DisplayCount=0)
	HUDDrawDataSuper	= (DisplayCount=0)

	bBlindFirable=TRUE
	// Don't want to play 'rifle' blind firing stance.
	bPlayDefaultBlindFireStance=FALSE
	bNoAnimDelayFiring=TRUE
	bDisableLeftHandIK=TRUE

	bDisplayingArc=false
	GrenadeAimingCameraPitchLimit=(X=-10000,Y=7000)
	GrenadeAimingSensitivityMultiplier=0.5f
	GrenadeAimingCameraPitchCorrectionInterpSpeed=4.f
	GrenadeAimingTrajectoryPitchMultiplier=1.f
	GrenadeAimingTrajectoryPitchAdjustment=2500
	GrenadeVelRange=(X=500,Y=1750)
	GrenadeVelRangeBlindFiring=(X=700,Y=1550)
	GrenadeVelRangePow=1.f

	FireStartOffset_Base=(X=0,Y=0,Z=-25)
	FireStartOffset_CoverLeftOrRight=(X=12,Y=83,Z=87)
	FireStartOffset_CoverDefault=(X=18,Y=55,Z=92)
	FireStartOffset_CoverLean_Low=(X=3,Y=15,Z=-27)
	FireStartOffset_CoverLean_High=(X=18,Y=25,Z=8)
	FireStartOffset_CoverBlindUp=(X=-24,Y=-94,Z=15)

	FireStartOffset_CoverZOffset_Low=-40.f
	FireStartOffset_CoverZOffset_Mid=0.f
	FireStartOffset_CoverZOffset_High=40.f

	bForceWalkWhenTargeting=FALSE

	bCanEquipWithShield=FALSE

	bAllowMeleeToFracture=FALSE

	TargetingFOVDistRange=(X=500.f,Y=2000.f)
	TargetingFOVRange=(X=70.f,Y=70.f)

	SpinningEffectSocketName=SpinEffect

	MinBlindFireUpPitch=2000
	BlindFireUpThrowDelay=0.36f
	BlindFireSideThrowDelay=0.45f
	UntargetedThrowDelay=0.35f
}
