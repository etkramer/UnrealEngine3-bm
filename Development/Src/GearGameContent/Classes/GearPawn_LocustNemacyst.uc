/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustNemacyst extends GearPawn_LocustNemacystBase
	config(Pawn);

/** Logging pre-processor macros */
/** GoW global macros */


/** Pct of default health before nemacyst will fall */
var() config float FallHealthPct;
var() config float DamageAmount;
var() config float DamageRadius;


/** If TRUE, Nemacyst got shot enough, that it's falling now. */
var	bool	bFellByDamage;

var float TimeOfLastAmbient;
var float NextAmbient;
var AudioComponent AmbientSound;

/** happy ink trail behind the nemacyst **/
var ParticleSystem InkTrail;
var ParticleSystemComponent PSC_InkTrail;

var bool bExploded;

singular function BaseChange();
function AddVelocity(vector NewVelocity, vector HitLocation, class<DamageType> DamageType, optional TraceHitInfo HitInfo );
function SetMovementPhysics()
{
	SetPhysics(PHYS_Flying);
}

/**
* Nemacysts don't get weapon attachments at this time
* What happens is the various actor factories get re-used and they still have the old
* inventory settings.  So IF the wretches actually had the same socket names as other pawns
* we would have Nemacysts with weapons!
*
**/
simulated function AttachWeaponToHand(GearWeapon Weap);
simulated function AttachWeaponToSlot(GearWeapon W);
simulated function AdjustWeaponDueToMirror();

/** Check on various replicated data and act accordingly. */
simulated event ReplicatedEvent(name VarName)
{
	Super.ReplicatedEvent(VarName);

	if( VarName == 'Health' )
	{
		// Replicated to client
		TookDamage();
	}
}


simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	if( AmbientSound == None )
	{
			AmbientSound = CreateAudioComponent( SoundCue'Locust_Nemecyst_Efforts.Nemecyst.Nemecyst_AmbientLoopCue', FALSE, TRUE);
			if( AmbientSound != None )
			{
				AmbientSound.bUseOwnerLocation = true;
				AmbientSound.bAutoDestroy = false;
				AttachComponent( AmbientSound );
				AmbientSound.bAllowSpatialization = true;
				AmbientSound.Play();
			}
			SetTimer( (frand()*4+3.0), TRUE, nameof(MakeRandomNoiseLoop) );
	}


	SetupInkTrail();

	// Start at full speed
	Velocity = Vector(Rotation) * AirSpeed;
}

simulated function SetupInkTrail()
{
	PSC_InkTrail = new(Outer) class'ParticleSystemComponent';
	if( PSC_InkTrail != None )
	{
		PSC_InkTrail.bAutoActivate = TRUE;
		PSC_InkTrail.SetTemplate( InkTrail );

		Mesh.AttachComponentToSocket( PSC_InkTrail, 'InkTrail' );
	}
}


simulated function MakeRandomNoiseLoop()
{
	switch(rand(4))
	{
	case 0:

		WorldInfo.PlaySound(SoundCue'Locust_Nemecyst_Efforts.Nemecyst.Nemecyst_FlyAwayCue', false, false, false, location);
		break;
	case 1:
		WorldInfo.PlaySound(SoundCue'Locust_Nemecyst_Efforts.Nemecyst.Nemecyst_AmbientCue', false, false, false, location);
		break;
	case 2:
		WorldInfo.PlaySound(SoundCue'Locust_Nemecyst_Efforts.Nemecyst.Nemecyst_CreepCue', false, false, false, location);
		break;
	case 3:
		WorldInfo.PlaySound(SoundCue'Locust_Nemecyst_Efforts.Nemecyst.Nemecyst_PainCue', false, false, false, location);
		break;
	}
}

function DoDamageEffects(float Damage, Pawn InstigatedBy, vector HitLocation, class<DamageType> DamageType, vector Momentum, TraceHitInfo HitInfo)
{
	Super.DoDamageEffects(Damage, InstigatedBy, HitLocation, DamageType, Momentum, HitInfo);

	// Call on server...
	if( Damage > 0.f )
	{
		TookDamage();
	}
}

function bool BeginAttackRun( GearPawn NewVictim ) // current call passes a NONE NewVictim!
{
	//WorldInfo.PlaySound(SoundCue'Locust_Nemecyst_Efforts.Nemecyst.Nemecyst_AttackCue', false, false, false, location);
	PlaySound(SoundCue'Locust_Nemecyst_Efforts.Nemecyst.NemaMortarIncoming_Cue',,,TRUE);
	return true;
}

simulated function TookDamage()
{
	local BodyStance	FallingAnim;

	// See if Nemacyst got enough damage to fall...
	if( !bFellByDamage && Health < FallHealthPct * HealthMax )
	{
		// Fall
		bFellByDamage = TRUE;

		SetPhysics(Phys_Falling);

		// Play falling animation
		FallingAnim.AnimName[BS_FullBody] = 'Death_A';
		BS_Play(FallingAnim, 1.f, 0.2f, -1.f, FALSE, TRUE);
		PlaySound(SoundCue'Locust_Nemecyst_Efforts.Nemecyst.Nemecyst_PainCue');
	}
}


/** Called when killed */
simulated function PlayDeath(class<GearDamageType> GearDamageType, vector HitLoc)
{
	ClearTimer('MakeRandomNoiseLoop');

	if( AmbientSound != None )
	{
		AmbientSound.FadeOut(0.1f, 0.0f);
	}

	Explosion();
}

event HitWall(vector HitNormal, Actor Wall, PrimitiveComponent WallComp)
{
	//debug
	`AILog_Ext( self@GetFuncName(), 'None', GearAI(Controller) );

	DeathFromImpact();
}

event Landed(vector HitNormal, Actor FloorActor)
{
	//debug
	`AILog_Ext( self@GetFuncName(), 'None', GearAI(Controller) );

	DeathFromImpact();
}

simulated event Bump(Actor Other, PrimitiveComponent OtherComp, Vector HitNormal)
{
	//debug
	`AILog_Ext( self@GetFuncName(), 'None', GearAI(Controller) );

	DeathFromImpact();
}

function DeathFromImpact()
{
	//debug
	`AILog_Ext( self@GetFuncName()@bPlayedDeath, 'None', GearAI(Controller));

	WorldInfo.PlaySound(SoundCue'Locust_Nemecyst_Efforts.Nemecyst.Nemecyst_ImpactFallCue',false,false,false,location);

	// Kill the nemacyst if it's not already dead.
	if( !bPlayedDeath )
	{
		Died(Controller, class'GDT_Explosive', Location);
	}
}

simulated function Explosion()
{
	local Emitter Explosion;

	//debug
	`AILog_Ext( self@GetFuncName()@bExploded, 'None', GearAI(Controller) );

	if( !bExploded )
	{
		bExploded = TRUE;

		WorldInfo.PlaySound(SoundCue'Locust_Nemecyst_Efforts.Nemecyst.Nemecyst_ExplodeCue', false, false, false, location);

		//@todo use the object pool for this
		Explosion = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( ParticleSystem'Locust_Nemacyst.Effects.P_Nemacyst_Explo', Location, rot(0,0,0) );
		Explosion.ParticleSystemComponent.ActivateSystem();

		HurtRadius( DamageAmount, DamageRadius, class'GDT_Explosive', 1.f, Location,,, TRUE );

		// Turn off physics so it doesn't keep moving
		SetPhysics( PHYS_None );

		if (PSC_InkTrail != None)
		{
			PSC_InkTrail.DeactivateSystem();
			PSC_InkTrail = None;
		}
		HideMesh();

		// Delayed kill, so death can be replicated.
		SetTimer( 2.f, FALSE, nameof(DelayedDestroy) );
	}
}

simulated function Destroyed()
{
	if (PSC_InkTrail != None)
	{
		PSC_InkTrail.DeactivateSystem();
		PSC_InkTrail = None;
	}
	Super.Destroyed();
}

simulated function DelayedDestroy()
{
	//debug
	`AILog_Ext( self@GetFuncName(), 'None', GearAI(Controller) );

	Destroy();
}

simulated function Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	//debug
	`AILog_Ext( self@GetFuncName(), 'None', GearAI(Controller) );

	// explode on unfriendly touches
	if (Pawn(Other) != None &&
		(GearAI(Controller) == None || !GearAI(Controller).IsFriendlyPawn(Pawn(Other))))
	{
		Explosion();
	}
	else
	{
		Super.Touch(Other,OtherComp,HitLocation,HitNormal);
	}
}

defaultproperties
{
	bRespondToExplosions=FALSE

	// List of BodySetups turned to physics for physical impacts
	PhysicsBodyImpactBoneList=("b_cyst_Head","b_cyst_R_Shoulder","b_cyst_R_ArmUpper","b_cyst_R_ArmLower","b_cyst_R_Claw01","b_cyst_Spine01","b_cyst_L_Shoulder","b_cyst_L_ArmUpper","b_cyst_L_ArmLower","b_cyst_L_Claw01")
	// Remap physical impulses to RigidBodies which provide better visual results.
	PhysicsImpactRBRemapTable(0)=(RB_FromName="b_cyst_Spine01",RB_ToName="b_cyst_Head")
	PhysicsImpactRBRemapTable(1)=(RB_FromName="b_cyst_Spine02",RB_ToName="b_cyst_Head")
	PhysicsHitReactionImpulseScale=1.f

	Begin Object Name=CollisionCylinder
		CollisionRadius=40.f
		CollisionHeight=40.f
	End Object

	Begin Object Name=GearPawnMesh
		//bHasPhysicsAssetInstance=TRUE	// Needed by PhysicsBodyImpact
		SkeletalMesh=SkeletalMesh'Locust_Nemacyst.Wyrm_Nemacyst'
		PhysicsAsset=PhysicsAsset'Locust_Nemacyst.Wyrm_Nemacyst_Physics'
		bIgnoreControllersWhenNotRendered=TRUE
		AnimSets(0)=AnimSet'Locust_Nemacyst.AnimSet_Wyrm_Nemacyst'
		AnimTreeTemplate=AnimTree'Locust_Nemacyst.AnimTree_Wyrm_Nemacyst'
		Rotation=(Pitch=-16384)
		Scale3D=(X=0.5,Y=0.5,Z=0.5)
	End Object
	Mesh=GearPawnMesh

	bBlockActors=FALSE	// Off for Spawning through seeder, restored later by AI.

	bCanDoRun2Cover=FALSE

	SightRadius=16384.0f
	PeripheralVision=-1.0f
	SightBoneName=SightBone
	
	AirSpeed=300.f
	RotationRate=(Pitch=25,Yaw=25,Roll=50)
	HearingThreshold=6000.f

	bBlockCamera=FALSE

	InkTrail=ParticleSystem'War_Level_Effects.ink.P_Nem_Ink_Trail'

	bUseSimplePhysicalFireStartLoc=true

	ProhibitedFindAnchorPhysicsModes=(PHYS_Interpolating)

}
