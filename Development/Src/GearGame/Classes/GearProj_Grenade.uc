/**
 * GearProj_Grenade
 * Grenade Projectile Implementation
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearProj_Grenade extends GearProj_ExplosiveBase
	native(Weapon)
	abstract
	implements(AIObjectAvoidanceInterface);

/** Simulate gravity */
var() const protected bool	bPerformGravity;

/** Maximum number of bounces */
var() const protected int	MaxBounceCount;

/** Maximum number of bounces before arming */
var() const protected int	MaxBounceCountToArm;

/** TRUE if this projectile is used for trajectory simulation */
var transient bool			bIsSimulating;

/** Max time in the future the grenade can simulate when doing preview arc.  Keep this low to prevent arc-induced framerate troubles. */
var() const float			MaxSimulationTime;

/** We hit a physics volume, and should go away quietly */
var transient protected bool bHitPhysicsVolume;

/** Mesh used for rendering */
var() protected PrimitiveComponent	Mesh;

//
// Internal
//

/** Number of times grenade has bounced */
var	protected transient int	BounceCount;

/** World position of the grenade's first bounce. */
var transient vector		FirstBounceLoc;

/** Velocity is scaled by this value after each bounce */
var() protected float	VelocityDampingFactor;
/** How much grenade bounces back off wall. 1.0 = normal, 0 = slide against wall */
var() protected float	Bounciness;
/** When grenade dips below this velocity, stop simulating. */
var() protected float	StopSimulatingVelocityThreshhold;

/** sound that plays when you stick a grenade to a foe! **/
var protected const SoundCue GrenadeAttachingToFoeSound;
/** sound that plays when you stick a grenade to the world! **/
var protected const SoundCue GrenadeAttachingToWorldSound;

/** This sound plays when grenade is about to explode **/
var protected const SoundCue BeepBeepAboutToExplodeSound;

/** Sound of grenade bouncing on the ground. */
var protected const SoundCue GrenadeBounceSound;

/** Grenade turned into physics asset? */
var	protected transient bool bEnabledGrenadePhysics;

var protected transient Controller AttacheeController;

//
// Internal simulation variables.
//

/** Length of the fixed timestep for grenade simulation. */
var protected const float			SimFixedTimeStep;
/** Used by simulation code to run gravity on a fixed timestep. */
var protected transient const float	TimeTilNextGravUpdate;
/** Used by simulation code to track mid-simulation gravity advances on collisions. */
var protected const transient float SimInternalGravityAdvanceTime;




/**
 * vars to determine when to beep and blow up.
 *
 * -TimeBeforeBeep
 * -beep will play here
 * -TimeAfterBeep
 *
 * -grenades explode at:  -TimeBeforeBeep  + TimeAfterBeep
 *
 **/
var protected config float TimeBeforeBeep;
var protected config float TimeAfterBeep;

/** Used to replicate the explosion, as well as to make sure grenade can't explode more than once. */
var protected transient repnotify bool bGrenadeHasExploded;

var repnotify transient Actor MeleeVictim;

var repnotify protected transient Pawn HoldVictim;

/** Time between prox grenade being triggered and prox grenade exploding.  Serves as a warning for players. */
var() protected const config float		AttachedtoWallTriggerDelay;
/** When attached to a wall, the grenade will auto-detonate after this time.  0.f means it will never auto-detonate. */
var() protected const config float		AttachedToWallMaxLifespan;
/** When attached to a wall, the grenade will detonate if an enemy gets closer than this.  */
var() protected const config float		AttachedToWallDetonationTriggerRadius;
/** TRUE if attached to a wall and the explosion sequence is triggered.  Used to tell clients to star the sequence. */
var protected transient repnotify bool	bAttachedToWallAutoExplosionTriggered;

/** How long after a grenade tag the grenade will detonate. */
var() protected const config float		GrenadeTagExplosionDelay;

/** Amount to spawn grenade back along normal so it isn't stuck into surface. */
var() float GrenadeAttachNormalOffset;

var protected transient int TakenDamage;
var() protected const config int DamageToDetonate;

/** Damage type for Martyring */
var class<GearDamageType> MartyrDamageType;
/** Damage type for Stickying */
var class<GearDamageType> StickyDamageType;

/** This is the normal for which way the head of the grenade is facing.  This will be used for placing decals on the wall. **/
var vector HeadFacingNormal;

//
// debugging
//

/** when true, grenade is frozen. (freezes physics) */
var	bool			bFreeze;

/** Used to tell clients to trigger a beep for the auto-explosion code, since that's run on the server only. */
var repnotify protected transient byte	BeepCount;

var protected AIAvoidanceCylinder AvoidanceCylinder;

var byte TeamNum;

/** If FF is on, this is the delay after setting a grenade trap before a player can trip his own grenade. */
var() protected const float InstigatorTrapSafetyDelay;
/** True if instigator is protected from his own grenade trap. */
var protected transient bool bInstigatorCannotTriggerTrap;

/** Stats Mask Defines */

const STATS_LEVEL3	= 0x04;


cpptext
{
	virtual void processHitWall(FCheckResult const& Hit, FLOAT TimeSlice=0.f);
	virtual void physProjectile(FLOAT DeltaTime, INT Iterations);
}

replication
{
	if (bNetDirty)
		TeamNum, bGrenadeHasExploded, MeleeVictim, HoldVictim, BeepCount, bAttachedToWallAutoExplosionTriggered, HeadFacingNormal;
}

simulated event Destroyed()
{
	Super.Destroyed();
	if(AvoidanceCylinder!=none)
	{
		AvoidanceCylinder.Destroy();
		AvoidanceCylinder=none;
	}
}

simulated event byte GetTeamNum()
{
	return TeamNum;
}

simulated function ReplicatedEvent(name VarName)
{
	if (VarName == 'BeepCount')
	{
		PlayBeepSound();
	}
	else if ( VarName == 'bGrenadeHasExploded' )
	{
		bGrenadeHasExploded = false;
		DoExplosion();
	}
	else if ( VarName == 'MeleeVictim' )
	{
		if (MeleeVictim != None)
		{
			AttachInit(MeleeVictim);
		}
	}
	else if ( VarName == 'HoldVictim' )
	{
		HoldInit(HoldVictim);
	}
	else if ( VarName == 'bAttachedToWallAutoExplosionTriggered' )
	{
		TriggerAttachedToWallExplosionSequence();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated protected function EnableGrenadePhysics()
{
	local int				RootIndex;
	local matrix			RootMatrix;
	local RB_BodyInstance	RootBody;
	local GearAnim_Slot		FullSlot;
	local Name				SpringBodyName;
	local SkeletalMeshComponent	SkelMeshComp;

	SkelMeshComp = SkeletalMeshComponent(Mesh);

	if (SkelMeshComp != None)
	{
		// Blend back to idle
		FullSlot = GetFullSlotNode();
		FullSlot.PlayCustomAnim('Bolo_Extend_Idle', 1.0, 0.2, -1.0, TRUE, TRUE);
		//FullSlot.StopCustomAnim(0.2f);

		SkelMeshComp.SetHasPhysicsAssetInstance(TRUE);

		// Unfix all bodies from kinematic data.
		// So now we switch to being fully physical
		SkelMeshComp.PhysicsAssetInstance.SetAllBodiesFixed(FALSE);

		// Add Spring to prevent physics asset from moving too far from actor's location.
		SpringBodyName	= 'b_bolo_end'; //'b_bolo_root';
		RootIndex	= SkelMeshComp.MatchRefBone(SpringBodyName);
		RootMatrix	= SkelMeshComp.GetBoneMatrix(RootIndex);

		// Find the hip body instance
		RootBody	= SkelMeshComp.FindBodyInstanceNamed(SpringBodyName);

		RootBody.EnableBoneSpring(TRUE, FALSE, RootMatrix);
		RootBody.SetBoneSpringParams(100000.f, 5000.f, 0.f, 0.f);
		RootBody.bDisableOnOverExtension	= FALSE;
		RootBody.bTeleportOnOverextension	= TRUE;
		RootBody.OverextensionThreshold		= 64.f;

		// Wake up physics
		SkelMeshComp.WakeRigidBody();

		SkelMeshComp.PhysicsWeight = 1.f;
	}
}



/**
 * This function is called to have the physics simulation of the grenade til the end.
 * we are using this for the grenade arc via particle trails
 */
native function RunPhysicsSimulationTilEnd(float GrenadeLifeSpan);


simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	// gravity is handled separately in a special physProjectile for this class
	Acceleration = vect(0,0,0);

	if (Role == ROLE_Authority && Instigator != None)
	{
		TeamNum = Instigator.GetTeamNum();
	}
}

simulated function Init(vector Direction)
{
	Super.Init( Direction );

	// reset Projectile
	SetPhysics(PHYS_Projectile);

	// reset grenade
	bBounce = TRUE;
	BounceCount = 0;

	// if we are using this projectile for trajectory simulation, then set it up
	if( bIsSimulating )
	{
		LifeSpan = 0;

		if( Mesh != None )
		{
			Mesh.SetHidden( TRUE );
		}
	}
}

/**
 * Initialization function for grenades when being detonated w/o
 * throwing (when DBNO).
 *
 * NET: called by everybody, clients from HoldVictim repnotify
 */
simulated function HoldInit(Pawn NewHoldVictim)
{
	if (Role == Role_Authority)
	{
		HoldVictim = NewHoldVictim;
		AttacheeController = NewHoldVictim.Controller;
		GearPRI(HoldVictim.PlayerReplicationInfo).NoGrenadeMartyr++;
	}
	Disable('Touch');
	bIsSimulating = FALSE;
	bStasis = FALSE;
	// no physics
	SetPhysics(PHYS_None);
	bBounce = FALSE;
	// hide the mesh
	if (Mesh != None)
	{
		Mesh.SetHidden(TRUE);
	}
	// stop the flight sound
	StopInFlightAudio();
	// set the detonation timer
	if (Role == ROLE_Authority)
	{
		SetTimer( 1.7f, FALSE, nameof(DoExplosion) );
	}
	// set the beeping timers
	SetTimer( 0.5 + TimeBeforeBeep, FALSE, nameof(PlayBeepSound) );
	SetTimer( 0.5 + TimeBeforeBeep + TimeAfterBeep, FALSE, nameof(PlayBeepSound) );
	// and play one now to indicate arming
	PlayBeepSound();

	// Setup Pawn animation
	PlayMartyrAnimation(GearPawn(NewHoldVictim));


}

simulated function PlayMartyrAnimation(GearPawn MartyrPawn)
{
	/*
	local BodyStance	BS_MartyrAnim;

	// Prevent Pawn from moving
	MartyrPawn.SetPhysics(PHYS_None);

	// Have Martyr Play his animation
	BS_MartyrAnim.AnimName[BS_FullBody] = 'DBNO_Grenade_Kneel';
	MartyrPawn.BS_Play(BS_MartyrAnim, 1.f, 0.25f, -1.f);
	*/

	MartyrPawn.SoundGroup.PlayEffort(MartyrPawn, GearEffort_ChainsawDuel, true, true);
}

simulated function PrepareExplosionTemplate()
{
	Super.PrepareExplosionTemplate();
	// add the attachee so that FX are properly attached to them
	ExplosionTemplate.Attachee			 = MeleeVictim;
	ExplosionTemplate.AttacheeController = None;

	// determine which damagetype to use
	if( HoldVictim != None )
	{
		// Use martyr damagetype
		ExplosionTemplate.MyDamageType		 = MartyrDamageType;
	}
	else
	if( MeleeVictim != None && Pawn(MeleeVictim) != None )
	{
		ExplosionTemplate.AttacheeController = AttacheeController;
	}
	else
	if ( MeleeVictim != None && StickyDamageType != None )
	{
		// Use sticky damagetype for achievement
		ExplosionTemplate.MyDamageType		 = StickyDamageType;
		ExplosionTemplate.AttacheeController = Instigator != None ? Instigator.Controller : None;
	}
}

/**
 * Keeps track of this player's planted grenades to prevent more than 2 active at any time.
 */
function RegisterPlantedGrenade()
{
	local GearPRI InstigatorPRI;
	local int Idx;
	local GearProj_Grenade OtherGrenade;
	InstigatorPRI = GearPRI(Instigator.PlayerReplicationInfo);
	if (InstigatorPRI != None)
	{
		// clear out any null/deleted entries in the list
		for (Idx = 0; Idx < InstigatorPRI.PlantedGrenades.Length; Idx++)
		{
			OtherGrenade = InstigatorPRI.PlantedGrenades[Idx];
			if (OtherGrenade == None || OtherGrenade.bDeleteMe)
			{
				InstigatorPRI.PlantedGrenades.Remove(Idx--,1);
			}
		}
		// if more than one in the list
		if (InstigatorPRI.PlantedGrenades.Length > 1)
		{
			// blow up the first one and remove
			InstigatorPRI.PlantedGrenades[0].DoExplosion();
			InstigatorPRI.PlantedGrenades.Remove(0,1);
		}
		// add this one to the list
		InstigatorPRI.PlantedGrenades.AddItem(self);
	}
}


/**
 * @STATS
 * Records a grenade tag or mine planting
 *
 * @Param Attachee - The pawn that the grenade is attached to
 * NOTE Only the Frag and Smoke grenades implement this
 */

function RecordTagStat(actor Attachee);

/**
 * @STATS
 * Records the # of times someone triggers a mine
 *
 * @Param TrippedBy - The pawn that tripped the mine
 * NOTE Only the Frag and Smoke grenades implement this
 */
function RecordMineStat(actor TrippedBy);


simulated function name AttachInit(Actor Attachee)
{
	local SkeletalMeshComponent VictimMesh, NadeMesh;
	local array<Name> RootBoneName;
	local vector HitLoc, HitNorm, ToPawn;
	local TraceHitInfo HitInfo;
	local Name BoneName;
	local Pawn AttacheePawn;
	local SkeletalMeshActor_LeviathanTentacle_Base LevTentacle;
	local GearAnim_Slot	FullSlot;

	if(Role == ROLE_Authority)
	{
		//@Stats
		// Record the tag here
		RecordTagStat(Attachee);

		`RecordStat(STATS_LEVEL3,'GrenadeTag',Instigator.Controller,self);
		// Handle case of attaching to SMCA - just use WorldInfo instead.
		if( StaticMeshCollectionActor(Attachee) != None)
		{
			Attachee = WorldInfo;
		}
	}

	MeleeVictim = Attachee;

	SetPhysics(PHYS_None);
	SetCollision(TRUE,FALSE);
	bIsSimulating = FALSE;
	bBounce = FALSE;

	// After initial spawn, we will let the client handle attachment and movement.
	bReplicateMovement = FALSE;

	// Attach to pawn case
	AttacheePawn = Pawn(Attachee);
	if (AttacheePawn != None)
	{
		AttacheeController = AttacheePawn.Controller;

		PlaySound(GrenadeAttachingToFoeSound, true);
		VictimMesh = AttacheePawn.Mesh;

		// Do a trace and see if hit a bone we can attach to
		if(TraceComponent(HitLoc, HitNorm, VictimMesh, Attachee.Location + vect(0,0,45), Location,, HitInfo))
		{
			// Got a bone - set location on the bone surface and using hit normal
			SetRotation(rotator(HitNorm) + rot(16384,0,0));
			SetLocation(HitLoc + (0.5 * GrenadeAttachNormalOffset*HitNorm)); // Put nade a bit closer on pawns
			// Note the bone name to return it
			BoneName = HitInfo.BoneName;

			SetBase(Attachee,, VictimMesh, BoneName);
		}
		// No bone hit - we just attach to the pawn
		else
		{
			// Get direction nade -> Pawn
			ToPawn = Normal((Attachee.Location + vect(0,0,30)) - Location);
			// Put it kind of in the pawn
			SetLocation(Attachee.Location - (20.0 * ToPawn));
			// And pointing the way we threw it
			SetRotation(rotator(ToPawn) + rot(16384,0,0));
			// Attach to actor
			SetBase(Attachee);
		}
	}
	// Attach to non-Pawn
	else
	{
		PlaySound(GrenadeAttachingToWorldSound, true);
		if (Role == ROLE_Authority)
		{
			RegisterPlantedGrenade();
			GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_GrenadeTrapSet, Instigator);
		}

		// Special code for leviathan tentacle
		LevTentacle = SkeletalMeshActor_LeviathanTentacle_Base(Attachee);
		if(LevTentacle != None)
		{
			LevTentacle.AttachGrenadeToTentacle(self);
		}
		else
		{
			SetBase(Attachee);
		}
	}

	// dangle some shiznit
	NadeMesh = SkeletalMeshComponent(Mesh);
	if (NadeMesh != None)
	{
		FullSlot = GetFullSlotNode();
		FullSlot.PlayCustomAnim('Bolo_Extend_Idle', 1.0, 0.2, -1.0, TRUE, TRUE);

		RootBoneName.AddItem('b_bolo_end');
		NadeMesh.SetHasPhysicsAssetInstance(true);
		NadeMesh.PhysicsAssetInstance.SetNamedBodiesFixed(TRUE,RootBoneName,NadeMesh,TRUE);

		// If attached to a Pawn, disable collision, so it doesn't freak out when he does physics hit reactions.
		if( AttacheePawn != None )
		{
			NadeMesh.SetBlockRigidBody(FALSE); // Disable collision (may be inside collision volume etc)
		}

		// Wake up physics
		NadeMesh.WakeRigidBody();
		NadeMesh.PhysicsWeight = 1.f;
	}

	if (Role == ROLE_Authority)
	{
		// if attached to a player then explode quickly
		if (Pawn(Attachee) != None || Trigger_ChainsawInteraction(Attachee) != None || Attachee.Tag == 'DeckTentacle' )
		{
			SetTimer( GrenadeTagExplosionDelay, FALSE, nameof(DoExplosion) );
   		}
		else
		{
			// if we didn't hit a player then set the timer to auto-check
			SetTimer( 1.5f, FALSE, nameof(CheckAutoExplosion) );
			SetTimer( AttachedToWallMaxLifespan, FALSE, nameof(DoExplosion) );
			bInstigatorCannotTriggerTrap = TRUE;
			SetTimer( InstigatorTrapSafetyDelay, FALSE, nameof(RemoveInstigatorTrapSafety) );
			LifeSpan = 0.f;		// live indefinitely until exploded

			bProjTarget = TRUE;
		}
	}

	StopInFlightAudio();

	// if we attached to a pawn,
	if (Pawn(Attachee) != None || Trigger_ChainsawInteraction(Attachee) != None || Attachee.Tag == 'DeckTentacle' )
	{
		// kickoff some simulated beeps
		SetTimer( TimeBeforeBeep, FALSE, nameof(PlayBeepSound) );
		SetTimer( TimeBeforeBeep + TimeAfterBeep, FALSE, nameof(PlayBeepSound) );
	}
	if (TrailEmitter != None)
	{
		TrailEmitter.Destroy();
		TrailEmitter = None;
	}

	return BoneName;
}

protected function RemoveInstigatorTrapSafety()
{
	// makes instigator eligible to trigger his own grenade trap (if friendly fire is on)
	bInstigatorCannotTriggerTrap = FALSE;
}

protected function CheckAutoExplosion()
{
	local GearPawn P;
	local Actor HitActor;
	local vector HitL, HitN;
	local bool bInstigatorInRadius;

	//FlushPersistentDebugLines();

	if (!bDeleteMe && !bGrenadeHasExploded)
	{
		//DrawDebugSphere(Location, 10, 10, 255, 0, 0,TRUE);
		//DrawDebugSphere(Location + (HeadFacingNormal * 20), 10, 10, 255, 0, 255,TRUE);
		// and see if we should auto-explode
		foreach WorldInfo.AllPawns(class'GearPawn', P, Location, AttachedToWallDetonationTriggerRadius)
		{
			if (P == Instigator)
			{
				bInstigatorInRadius = TRUE;
				if (bInstigatorCannotTriggerTrap)
				{
					continue;
				}
			}

			if (GearGame(WorldInfo.Game).bAllowFriendlyFire || !WorldInfo.GRI.OnSameTeam(P,self))
			{
				if ((P.Health > 0 || P.IsDBNO()) && !P.IsAHostage() && !P.IsRespawnInvincible())
				{
					// make sure we can sort of see them
					HitActor = Trace(HitL, HitN, P.GetPawnViewLocation(), Location + (HeadFacingNormal * 20), TRUE, vect(8,8,32));
					//DrawDebugLine(P.GetPawnViewLocation(),Location + (HeadFacingNormal * 20),255,0,0,TRUE);
					if (HitActor == None || HitActor == P || !HitActor.bBlockActors)
					{

						//@STATS
						RecordMineStat(P);

						TriggerAttachedToWallExplosionSequence();
						return;
					}
					//`log("blocked by:"@`showvar(HitActor));
				}
			}
		}

		if (!bInstigatorInRadius)
		{
			RemoveInstigatorTrapSafety();
		}

		SetTimer( 0.25f, FALSE, nameof(CheckAutoExplosion) );
	}
}

simulated protected function TriggerAttachedToWallExplosionSequence()
{
	if (Role == ROLE_Authority)
	{
		// signals clients to start this as well
		bAttachedToWallAutoExplosionTriggered = TRUE;
	}

	ClearTimer('CheckAutoExplosion');
	PlayBeepSound();
	SetTimer( AttachedtoWallTriggerDelay, FALSE, nameof(DoExplosion) );
}

/************************************************************************************
 * Collision
 ***********************************************************************************/

simulated singular event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	// Ignore touches on fluid surface or physics volumes
	if(FluidSurfaceActor(Other) != None || PhysicsVolume(Other) != None)
	{
		// If simulating the arc - just change physics mode (will stop drawing arc at this point)
		if(bIsSimulating)
		{
			SetPhysics(PHYS_None);
		}
		return;
	}

	HandleCollision( Other, OtherComp, HitLocation, HitNormal );
}

simulated event Bump( Actor Other, PrimitiveComponent OtherComp, Vector HitNormal )
{
	HandleCollision( Other, OtherComp, Location, HitNormal );
}

simulated function HitWall( vector HitNormal, Actor Wall, PrimitiveComponent WallComp )
{
	HandleCollision( Wall, WallComp, Location, HitNormal );
}

/** Handle collision */
simulated function HandleCollision( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	local GearPawn_LocustLeviathanBase Levi;
	local Trigger_LeviathanMouth	   Trig;

	Levi = GearPawn_LocustLeviathanBase(Other);
	if( Levi == None )
	{
		Trig = Trigger_LeviathanMouth(Other);
		if( Trig != None )
		{
			Levi = Trig.Leviathan;
		}
	}

	if( Levi != None )
	{
		bBounce = FALSE;
		if( Role == ROLE_Authority && !bIsSimulating )
		{
			if( Levi.HandleGrenadeTouch( self, Trig ) )
			{
				Destroy();
			}
			else
			{
				DoExplosion();
			}
		}
		return;
	}

	//`log(self@"** hit!"@Other@OtherComp@HitNormal@HitLocation);
	if( Other != None && ShouldBounce(Other, HitNormal) && Physics == PHYS_Projectile )
	{
		// a bit of a hack, massage data for pawns
		if ( (Pawn(Other) != None) && (SkeletalMeshComponent(OtherComp) != None) )
		{
			HitNormal = HitLocation - Other.Location;
			HitNormal.Z = 0.f;
			HitNormal = Normal(HitNormal);
		}

		Bounce(HitLocation, HitNormal, Other);
	}
}


/** Called when grenade stops bouncing and comes to rest. */
simulated function StopBounce(vector HitNormal)
{
	local AIController C;

	bBounce = FALSE;
	StopSimulating();

	if ( (Role == ROLE_Authority) && !bIsSimulating )
	{
		// notify AI of the bounce
		foreach WorldInfo.AllControllers(class'AIController', C)
		{
			C.NotifyProjLanded( self );
		}

		ArmGrenade();
	}
}

simulated final function ArmGrenade()
{
	if (!bIsSimulating && !IsTimerActive('DoExplosion'))
	{
		if ( Pawn(Base) != None )
		{
			// :(
			TimeBeforeBeep = 1.5;
			TimeAfterBeep = 1.0;
		}
		SetTimer( TimeBeforeBeep, FALSE, nameof(PlayBeepSound) );
		SetTimer( TimeBeforeBeep+TimeAfterBeep, FALSE, nameof(DoExplosion) );
		SetTimer( TimeBeforeBeep+TimeAfterBeep * FRand(), FALSE, nameof(WarnProjExplode) );

	}
}

/** This will play the beep beep sound **/
simulated function PlayBeepSound()
{
	PlaySound( BeepBeepAboutToExplodeSound, FALSE );
}

/**
 * We do an explosion here and then tell the projectile to hide itself and prepare to be destroyed.
 * We can't destroy now as the trail emitter is still probably active.
 **/
simulated function DoExplosion()
{
	local GearPawn	Kidnapper, TaggedVictim, KamikazePawn;
	local Actor		HitActor;

	if( bGrenadeHasExploded )
	{
		return;
	}

	ClearTimer( 'PlayBeepSound' );
	ClearTimer( 'CheckAutoExplosion' );

	// If we've been attached to a Pawn, guarantee his death by ignoring modifiers (like immunity behind cover)
	if( MeleeVictim != None)
	{
		HitActor = MeleeVictim;
		TaggedVictim = GearPawn(MeleeVictim);
		if( TaggedVictim != None )
		{
			TaggedVictim.SetDontModifyDamage( TRUE );
		}
	}

	if( HoldVictim != None )
	{
		KamikazePawn = GearPawn(HoldVictim);
		if( KamikazePawn != None )
		{
			KamikazePawn.SetDontModifyDamage(TRUE);
		}

		// If hostage has been grenade tagged, then set it on the kidnapper, so he gets killed to.
		if( KamikazePawn.IsAHostage() )
		{
			Kidnapper = KamikazePawn.InteractionPawn;
			Kidnapper.SetDontModifyDamage(TRUE);
			HitActor = Kidnapper;
		}
		else
		{
			HitActor = KamikazePawn;
		}
	}


	// if we are on "something" pawn or wall
	if( MeleeVictim != None  )
	{
		TriggerExplosion(Location, HeadFacingNormal, HitActor);
	}
	// on the ground
	else
	{
		TriggerExplosion(Location, Vect(0,0,1), HitActor);
	}


	// Clear flag
	if( TaggedVictim != None )
	{
		TaggedVictim.SetDontModifyDamage( FALSE );
		SetBase(None);
		TaggedVictim.Mesh.DetachComponent(Mesh);
	}
	if( KamikazePawn != None )
	{
		KamikazePawn.SetDontModifyDamage(FALSE);
	}
	if( Kidnapper != None )
	{
		Kidnapper.SetDontModifyDamage(FALSE);
	}

	if(AvoidanceCylinder!=none)
	{
		AvoidanceCylinder.Destroy();
		AvoidanceCylinder=none;
	}
	SetHidden(TRUE);
}

function float GetAvoidanceRadius()
{
	return Max(ExploDamageRadius,KnockDownRadius);
}

function SetupAvoidanceCylinder()
{
	local float AvoidRadius;

	if( !bIsSimulating && !bGrenadeHasExploded && !bDeleteMe && AvoidanceCylinder == None )
	{
		AvoidRadius = GetAvoidanceRadius();
		AvoidanceCylinder = Spawn(class'AIAvoidanceCylinder',self,,Location,,,TRUE);
		if(AvoidanceCylinder!=none)
		{
			AvoidanceCylinder.SetBase(self);
			AvoidanceCylinder.SetCylinderSize(AvoidRadius,AvoidRadius*2.0f);
			AvoidanceCylinder.SetEnabled(true);
		}
		//DrawDebugSphere(Location,AvoidRadius,16,255,0,0,TRUE);
	}

}

/**
 ---> AIobjectAvoidanceInterface
 **/
function bool ShouldAvoid(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	//`log(GetFuncName()@AskingAI@TriggeringComponent@AskingAI.GetTeamNum()@Instigator.GetTeamNum());
	// don't avoid grenades from our team, and see if we have LOS to the AI in question, if not don't bother avoiding
	if(AskingAI.GetTeamNum() != InstigatorController.GetTeamNum() && FastTrace(Location, AskingAI.Pawn.Location,,true))
	{
		return true;
	}

	return false;
}

function bool ShouldEvade(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent);
function bool ShouldRoadieRun(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent);


/**
 * Reflect grenade off a wall
 *
 * @param	HitLocation
 * @param	HitNormal
 */
simulated function Bounce(Vector HitLocation, Vector HitNormal, Actor Other)
{
	local AIController C;

	// If we hit a mover coming towards us, on a fairly flat bit - stick to it
	//`log("HIT"@InterpActor(Other)@(Normal(Other.Velocity) Dot HitNormal)@(HitNormal.Z >= MINFLOORZ));
	if( InterpActor(Other) != None &&
		(Normal(Other.Velocity) Dot HitNormal) > 0.4 &&
		(HitNormal.Z >= MINFLOORZ) )
	{
		if( !bEnabledGrenadePhysics )
		{
			EnableGrenadePhysics();
			bEnabledGrenadePhysics = TRUE;
		}

		StopBounce(HitNormal);
		if( !bIsSimulating )
		{
			SetBase(Other);
		}
		return;
	}


	if((Normal(Velocity) Dot HitNormal) > 0.4)
	{
		return;
	}

	// on second bounce kick on avoidance cyl
	if(ROLE== ROLE_Authority && BounceCount == 1 && !bIsSimulating)
	{
		SetupAvoidanceCylinder();
	}
	if( !bBounce || BounceCount >= MaxBounceCount )
	{
		StopBounce(HitNormal);
		return;
	}
	if ( BounceCount >= MaxBounceCountToArm )
	{
		ArmGrenade();
	}

	BounceCount++;
	if (BounceCount > 1)
	{
		StopInFlightAudio();
	}

	// bounce off wall with damping
	Velocity -= (1.f + Bounciness) * HitNormal * (Velocity Dot HitNormal);
	Velocity *= VelocityDampingFactor;

	// should we stop simulating?
	if( VSize(Velocity) < StopSimulatingVelocityThreshhold )
	{
		StopBounce(HitNormal);
	}
	else
	{
		if( !bIsSimulating )
		{
			// this will do a little jitter at the end.  We can add another velocity check to stop it
			PlaySound(GrenadeBounceSound);
		}
	}

	if( !bEnabledGrenadePhysics )
	{
		EnableGrenadePhysics();
		bEnabledGrenadePhysics = TRUE;
	}

	if (BounceCount == 1)
	{
		FirstBounceLoc = HitLocation;
	}

	if ( (Role == ROLE_Authority) && !bIsSimulating )
	{
		// notify AI of the bounce
		foreach WorldInfo.AllControllers(class'AIController', C)
		{
			C.NotifyProjLanded( self );
		}
	}
}

function WarnProjExplode()
{
	local AIController C;

	foreach WorldInfo.AllControllers( class'AIController', C )
	{
		C.WarnProjExplode( self );
	}
}


/**
 * Test condition to validate a rebound
 *
 * @param	Touched, actor the grenade touched
 * @param	HitNormal, normal of the surface impacted
 * @return	true if the grenade should bounce off
 */
simulated function bool ShouldBounce(Actor Touched, vector HitNormal, optional PrimitiveComponent HitComponent)
{
	if( (Touched == None) || (Base != None) )
	{
		return FALSE;
	}

	// cannot collide with instigator until it has bounced once
	// also don't allow projectiles to collide while spawning on clients
	// because if that were accurate, the projectile would've been destroyed immediately on the server
	// and therefore it wouldn't have been replicated to the client
	if ((Touched == Instigator || (Role < ROLE_Authority && !bBegunPlay)) && BounceCount == 0)
	{
		return FALSE;
	}

	if( Touched.bWorldGeometry || Touched.bBlockActors )
	{
		return TRUE;
	}

	return FALSE;
}

simulated function TriggerExplosion(vector HitLocation, vector HitNormal, Actor HitActor)
{
	// do not do any visual work when simulating
	if( !bIsSimulating && !bGrenadeHasExploded )
	{
		super.TriggerExplosion(HitLocation, HitNormal, HitActor);
		bGrenadeHasExploded = true;
		// this hides the mesh of the grenade (other wise is sticks around after the explosion)
		Mesh.SetHidden( TRUE );
	}
}

/** Returns AnimNodeSlot to play custom animations */
simulated function GearAnim_Slot GetFullSlotNode()
{
	return GearAnim_Slot(SkeletalMeshComponent(Mesh).FindAnimNode('CustomAnim'));
}

/** @return the effective radius of this projectile's explosion for the AI */
static function float GetAIEffectiveRadius()
{
	return default.DamageRadius;
}

/** Overridden to allow explosions to blow up other grenades. */
simulated function TakeDamage(int Dmg, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	//`log("***   "@self@"took damage"@Dmg@EventInstigator@DamageType);
	if (ClassIsChildOf(DamageType, class'GDT_Explosive'))
	{
		// slight delay, then blow up
		SetTimer( 0.3f, FALSE, nameof(DoExplosion) );
	}
	else if(Base != None && Pawn(Base) == None && (EventInstigator == None || EventInstigator.Pawn == None || EventInstigator.Pawn == Instigator || !EventInstigator.Pawn.IsSameTeam(Instigator)))
	{
		TakenDamage += Dmg;
		if(TakenDamage >= DamageToDetonate)
		{
			DoExplosion();
		}
	}

	super.TakeDamage(Dmg, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);
}

/** Tells grenade to get ready to be simulating, or not. */
simulated function SetupForSimulation(bool bSimulating)
{
	if (bSimulating)
	{
		SetHidden( FALSE );
		SetCollision( TRUE, FALSE, FALSE );
//		Enable( 'Tick' );
//		SuppressAudio(FALSE);
	}
	else
	{
		SetHidden( TRUE );
		SetCollision( FALSE, FALSE, FALSE );
//		Disable( 'Tick' );
//		SuppressAudio(TRUE);
	}
}

/** This grenade will be used for simulation, get it ready. */
simulated function InitForSimulation()
{
	bIsSimulating = TRUE;

	RemoteRole = ROLE_None;

	// no audio
	SuppressAudio(TRUE);

	// get rid of unnecessary components, so we don't tick them
	// during simulation

	// hide the mesh
	Mesh.SetHidden(TRUE);
	DetachComponent(Mesh);
	Mesh = None;
}

simulated function bool CanSplash()
{
	return !bIsSimulating;
}

defaultproperties
{
//	FireDamageReductionRate=50
//	FireDamageToDetonate=300

	SimFixedTimeStep=0.05f		// 20 hz
	MaxSimulationTime=3.5f

	bKillDuringLevelTransition=TRUE

	VelocityDampingFactor=0.6f
	Bounciness=0.5f
	StopSimulatingVelocityThreshhold=40

	bPerformGravity=TRUE
	GravityScale=2.0f
	MaxBounceCount=8
	MaxBounceCountToArm=3

	bCollideComplex=TRUE	// Ignore simple collision on StaticMeshes, and collide per poly
	bStoppedByGrenadeBlockingVolume=TRUE

	bBounce=TRUE
	Physics=PHYS_Projectile
	Speed=1500
	MaxSpeed=0
	Damage=50
	MomentumTransfer=100
	MyDamageType=class'DamageType'

	bNetInitialRotation=TRUE
	RemoteRole=ROLE_SimulatedProxy
    Lifespan=10

	//RotationRate=(Pitch=-64000,Yaw=0,Roll=0)

	bNetTemporary=FALSE
	bAlwaysRelevant=TRUE

	bCollideActors=true

	GrenadeAttachNormalOffset=30.0
	TeamNum=255

	MartyrDamageType=class'GDT_FragMartyr'
	StickyDamageType=class'GDT_FragSticky'

	InstigatorTrapSafetyDelay=2.5f
}

