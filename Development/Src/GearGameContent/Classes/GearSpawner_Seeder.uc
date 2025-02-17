/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Defines an seeder for Nemacysts to shoot out of
 */
class GearSpawner_Seeder extends GearSpawner
	DependsOn(GearPawn)
	placeable;

/** Static mesh component */
var	SkeletalMeshComponent Mesh;
/** cached custom anim node sequence reference */
var GearAnim_Slot FullBodySlot;
/** Number of Hammer of Dawn hits it takes to kill this sucker */
var() int NumHODHitsForKill;
/** Amount of health the seeder has */
var float SeederHealth;
/** The names of the animations for the seeder */
var Array<Name> SeederAnimNames;
/** Is the seeder actively spitting? */
var bool bSpawning;
/** Whether it's time to spit a Nemacyst out of our ass yet */
var bool bAllowSpawn;
/** Camera shake data */
var ScreenShakeStruct CamShake1, CamShake2, CamShake3, CamShake4, CamShake5;

/** This is the max distance the camera shakes will occur at **/
var() float CameraShakeDistance;

/** Footstep sound */
var SoundCue FootstepSound;

var float LastDamageScreamTime;
/**
 * Foot impacts effects: the leg order (if you're facing the seeder's butt)
 * is back-left to the front and back to the back-right
 */
var array<ParticleSystemComponent> PSC_FeetImpacts[8];
var array<Name> FeetSocketNames[8];

/** Butt effects */
var ParticleSystemComponent PSC_ButtSpit, PSC_ButtFrontSpit, PSC_ButtSideSpit, PSC_BodyDust, PSC_HeavyFoot2, PSC_HeavyFoot3;

/** Time we will wait between damage until we'll stop the hit reaction anim */
var const float StopHitReactionDelta;

/** Used for keeping track of the collision cylinder we have **/
var protected CylinderComponent CollisionCylinderComponent;


/** enum of animations for the seeder */
enum ESeederAnim
{
	SA_Idle,
	SA_Spit,
	SA_Death_Quick,
	SA_Death_Uber,
	SA_Death_QuickB,
	SA_HitReactionHOD,
	SA_Death_Hide,
	SA_Count,
	SA_HitReactionSmall,
	SA_HitReactionBig,
};

/** The current animation playing */
var repnotify ESeederAnim CurrentAnimation;
/** used when receiving animation on the client to know what the previous anim was */
var ESeederAnim LastReplicatedAnimation;

/** enum of death animations for the editor to use */
enum ESeederDeathAnim
{
	SDA_Death_Quick,
	SDA_Death_Uber,
	SDA_Death_QuickB,
	SDA_Death_Hide,
};
/** The death anim to play when the seeder dies. */
var() ESeederDeathAnim DeathAnim;
var bool bPlayedDeathScream;
var SoundCue DeathScream;

/** Played when the seeder has been cooked to death by the HOD!**/
var SpawnedGearEmitter DeathSmoke;

replication
{
	if (bNetDirty)
		CurrentAnimation;
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// Set the seeder's health so that it takes 2 hits from the HOD_.
	SeederHealth = (float(NumHODHitsForKill) - 0.5f) * (class'HOD_Beam'.default.FireDurationSec * class'HOD_Beam'.default.BaseDamagePerSecond);

	// Setup the spawn slot
	SpawnSlots.Length = 1;

	SetupEffectComponents();

	// Set up the Animations
	FullBodySlot = GearAnim_Slot(Mesh.FindAnimNode('Custom_FullBody'));
	CurrentAnimation = SA_Idle;
}

simulated event ReplicatedEvent(name VarName)
{
	local ESeederAnim NewAnimation;

	if (VarName == 'CurrentAnimation')
	{
		NewAnimation = CurrentAnimation;
		CurrentAnimation = LastReplicatedAnimation;
		if (CurrentAnimation == SA_HitReactionHOD)
		{
			StopHitReactionHOD();
		}
		switch (NewAnimation)
		{
			case SA_Idle:
				break;
			case SA_Death_Quick:
			case SA_Death_Uber:
			case SA_Death_QuickB:
			case SA_Death_Hide:
				PlayDeathAnimation();
				break;
			case SA_Spit:
				PlaySpitAnimation();
				break;
			case SA_HitReactionHOD:
				StartHitReactionHOD();
				break;
			case SA_HitReactionBig:
			case SA_HitReactionSmall:
				PlayHitReactionAnim();
				break;
			default:
				`Warn("Unhandled animation type" @ NewAnimation);
				break;
		}
		CurrentAnimation = NewAnimation;
		LastReplicatedAnimation = NewAnimation;
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/**
 *  Setup up the effect components
 */
simulated function SetupEffectComponents()
{
	local int Idx;

	if ( Mesh != None )
	{
		for ( Idx = 0; Idx < 8; Idx++ )
		{
			PSC_FeetImpacts[Idx] = new(Outer) class'ParticleSystemComponent';
			PSC_FeetImpacts[Idx].bAutoActivate = FALSE;
			PSC_FeetImpacts[Idx].SetTemplate( ParticleSystem'Locust_Seeder.Particles.P_Seeder_Soft_Footstep' );
			Mesh.AttachComponentToSocket( PSC_FeetImpacts[Idx], FeetSocketNames[Idx] );
		}

		PSC_ButtSpit = new(Outer) class'ParticleSystemComponent';
		PSC_ButtSpit.bAutoActivate = FALSE;
		PSC_ButtSpit.SetTemplate( ParticleSystem'Locust_Seeder.Particles.P_Seeder_Fire_end' );
		Mesh.AttachComponentToSocket( PSC_ButtSpit, 'Spit' );

		PSC_ButtFrontSpit = new(Outer) class'ParticleSystemComponent';
		PSC_ButtFrontSpit.bAutoActivate = FALSE;
		PSC_ButtFrontSpit.SetTemplate( ParticleSystem'Locust_Seeder.Particles.P_Seeder_Pre_Fire_Front' );
		Mesh.AttachComponentToSocket( PSC_ButtFrontSpit, 'Spit' );

		PSC_ButtSideSpit = new(Outer) class'ParticleSystemComponent';
		PSC_ButtSideSpit.bAutoActivate = FALSE;
		PSC_ButtSideSpit.SetTemplate( ParticleSystem'Locust_Seeder.Particles.P_Seeder_Pre_Fire_Side' );
		Mesh.AttachComponentToSocket( PSC_ButtSideSpit, 'Spit' );

		PSC_BodyDust = new(Outer) class'ParticleSystemComponent';
		PSC_BodyDust.bAutoActivate = FALSE;
		PSC_BodyDust.SetTemplate( ParticleSystem'Locust_Seeder.Particles.P_Seeder_BellyDust' );
		Mesh.AttachComponentToSocket( PSC_BodyDust, 'Body_Dust' );

		PSC_HeavyFoot2 = new(Outer) class'ParticleSystemComponent';
		PSC_HeavyFoot2.bAutoActivate = FALSE;
		PSC_HeavyFoot2.SetTemplate( ParticleSystem'Locust_Seeder.Particles.P_Seeder_Heavy_Footstep' );
		Mesh.AttachComponentToSocket( PSC_HeavyFoot2, FeetSocketNames[2] );

		PSC_HeavyFoot3 = new(Outer) class'ParticleSystemComponent';
		PSC_HeavyFoot3.bAutoActivate = FALSE;
		PSC_HeavyFoot3.SetTemplate( ParticleSystem'Locust_Seeder.Particles.P_Seeder_Heavy_Footstep' );
		Mesh.AttachComponentToSocket( PSC_HeavyFoot3, FeetSocketNames[3] );
	}
}

/**
* Overridden to time the spawning with the spit animation.
*/
event bool GetSpawnSlot(out int out_SpawnSlotIdx, out vector out_SpawnLocation, out rotator out_SpawnRotation)
{
// 	`DLog("bActive" @ bActive @ "bSpawning:" @ bSpawning @ "bAllowSpawn:" @ bAllowSpawn @ "CurrentAnimation:" @ CurrentAnimation);
	if ( bActive )
	{
		// if already in the spawning process
		if ( bSpawning )
		{
			// and ready to spawn
			if ( bAllowSpawn )
			{
				// turn off the spawn
				bAllowSpawn = FALSE;
				// and return the slot
				out_SpawnSlotIdx = 0;
				Mesh.GetSocketWorldLocationAndRotation('NemaSpawnReally',out_SpawnLocation,out_SpawnRotation);
				// force them to face up
				//out_SpawnRotation = rotator(vect(0,0,1));
				return TRUE;
			}
			// otherwise keep waiting
		}
		else
		{
			// start the spawning process
			bSpawning = TRUE;
			bAllowSpawn = FALSE;
			// play the animation
			PlaySpitAnimation();
		}
	}
	return FALSE;
}

/**
 * Play the spit anim and set the timer for the next one
 */
simulated final function PlaySpitAnimation()
{
	local float AnimTime;

	// play the animation
	CurrentAnimation = SA_Spit;
	AnimTime = FullBodySlot.PlayCustomAnim( SeederAnimNames[CurrentAnimation], 1.f, 0.2f, 0.2f, FALSE );

// 	`DLog("CurrentAnimation:" @ CurrentAnimation);

	// set the timer to indicate the end of this cycle
	SetTimer( AnimTime + 2.5f + FRand() * 4.f, FALSE, nameof(SpawningFinished) );
}

/** Turns of the spawning cycle so that the seeder can start again. */
final function SpawningFinished()
{
	if( CurrentAnimation == SA_Spit )
	{
		CurrentAnimation = SA_Idle; // necessary for replication
	}
	bSpawning = FALSE;
}

/**
 * Seeder is dying, so play it's death anim, but clear any spit timers that might be active, and tell parent.
 */
simulated final function PlayDeathAnimation()
{
	local float AnimLength;

	StartingToDie();
	CurrentAnimation = GetDeathAnimationIndex();
// 	`DLog("CurrentAnimation:" @ CurrentAnimation);
	AnimLength = FullBodySlot.PlayCustomAnim( SeederAnimNames[CurrentAnimation], 1.f, 0.2f, -1.f, FALSE );
	if( !bPlayedDeathScream && (CurrentAnimation == SA_Death_Uber) )
	{
		PlaySound(DeathScream);
		bPlayedDeathScream = TRUE;
	}
	SetTimer(AnimLength, FALSE, nameof(FinallyDead));
}

/**
 * Seeder is finally dead, so tell parent.
 */
simulated final function FinallyDead()
{
	FinishedDying();

	DeathSmoke = Spawn( class'SpawnedGearEmitter' );
	DeathSmoke.SetTemplate( ParticleSystem'Locust_Seeder.Particles.P_Seeder_Death_Smoking' );
	DeathSmoke.ParticleSystemComponent.ActivateSystem();

	CollisionComponent=CollisionCylinderComponent;
	SetCollisionType( COLLIDE_NoCollision );
}


/** kill off our smoke **/
simulated event Destroyed()
{
	DeathSmoke.ParticleSystemComponent.DeactivateSystem();
	DeathSmoke.LifeSpan = 3.0f;

	Super.Destroyed();
}


/** Convert death anim index into normal anim index. */
simulated final function ESeederAnim GetDeathAnimationIndex()
{
	switch ( DeathAnim )
	{
		case SDA_Death_Quick:	return SA_Death_Quick;
		case SDA_Death_Uber:	return SA_Death_Uber;
		case SDA_Death_QuickB:	return SA_Death_QuickB;
		case SDA_Death_Hide:	return SA_Death_Hide;
	}

	return SA_Death_Quick;
}

/**
 * Take damage, but only from HOD_
 */
event TakeDamage( int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
// 	`DLog("PRE Health:" @ SeederHealth @ "Damage:" @ Damage @ "DamageType:" @ DamageType @ "CurrentAnimation:" @ CurrentAnimation);

	if( (SeederHealth > 0) && (ClassIsChildOf(DamageType, class'GDT_HOD') || ClassIsChildOf(DamageType, class'GDT_BrumakBullet') || ClassIsChildOf(DamageType, class'GDT_BrumakCannon')) )
	{
		SeederHealth -= Damage;
		Killer = EventInstigator.Pawn;
		if( SeederHealth <= 0 )
		{
			StopHitReactionHOD();
			HitReactionAnimFinished();
			PlayDeathAnimation();
			bActive = FALSE;
			DeActivated();
		}
		else
		{
			// start the hit reaction anim
			if( ClassIsChildOf(DamageType, class'GDT_HOD') )
			{
				if( (CurrentAnimation != SA_HitReactionHOD) )//&& ((CurrentAnimation != SA_Spit) || (bAllowSpawn == false)) )
				{
					StartHitReactionHOD();
				}

				// set the timer for stopping the hitreaction animation
				if ( CurrentAnimation == SA_HitReactionHOD )
				{
					SetTimer( StopHitReactionDelta, FALSE, nameof(self.StopHitReactionHOD));
				}
			}
			else if( ClassIsChildOf(DamageType, class'GDT_BrumakCannon') )
			{
				if( CurrentAnimation != SA_HitReactionBig )
				{
					CurrentAnimation = SA_HitReactionBig;
					PlayHitReactionAnim();
				}
			}
			else
			{
				if( CurrentAnimation != SA_HitReactionSmall )
				{
					CurrentAnimation = SA_HitReactionSmall;
					PlayHitReactionAnim();
				}
			}
		}

		Super.TakeDamage(Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);
	}

// 	`DLog("POST Health:" @ SeederHealth @ "Damage" @ Damage @ "DamageType:" @ DamageType @ "CurrentAnimation:" @ CurrentAnimation);
}

simulated function PlayHitReactionAnim()
{
	local float AnimTime;

// 	`DLog("CurrentAnimation:" @ CurrentAnimation);
	AnimTime = FullBodySlot.PlayCustomAnim(SeederAnimNames[CurrentAnimation], 1.f, 0.25f, 0.25f);
	SetTimer(AnimTime, FALSE, nameof(self.HitReactionAnimFinished));
	// prevent any spawns
	ClearTimer(nameof(SpawningFinished));
}

simulated function HitReactionAnimFinished()
{
	ClearTimer(nameof(HitReactionAnimFinished));
	if( CurrentAnimation == SA_HitReactionBig || CurrentAnimation == SA_HitReactionSmall )
	{
		CurrentAnimation = SA_Idle;

		// if stopped in mid-spit, start it up again
		if( bSpawning )
		{
			// play the animation
			PlaySpitAnimation();
		}
	}
}

simulated function StartHitReactionHOD()
{
	if (Role == ROLE_Authority && LastDamageScreamTime + 5.0 < WorldInfo.TimeSeconds)
	{
		PlaySound(SoundCue'Locust_Seeder_Efforts.SeederEfforts.Seeder_SnortCue');
		PlaySound(SoundCue'Locust_Seeder_Efforts.SeederEfforts.Seeder_GrowlAngryCue');
		LastDamageScreamTime = WorldInfo.TimeSeconds;
	}
	FullBodySlot.PlayCustomAnim(SeederAnimNames[SA_HitReactionHOD], 1.f, 0.2f, 0.2f, true);
	CurrentAnimation = SA_HitReactionHOD;
	// prevent any spawns
	ClearTimer(nameof(SpawningFinished));
}

simulated function StopHitReactionHOD()
{
	ClearTimer(nameof(StopHitReactionHOD));
	if( CurrentAnimation == SA_HitReactionHOD )
	{
		FullBodySlot.StopCustomAnim( 0.2f );
		CurrentAnimation = SA_Idle;

		// if stopped in mid-spit, start it up again
		if ( bSpawning )
		{
			// play the animation
			PlaySpitAnimation();
		}
	}
}

event HandleSpawn(GearPawn NewSpawn, int SlotIdx)
{
	local GearAI_Nemacyst Nem;
	`log(GetFuncName()@NewSpawn@NewSpawn.Mesh@NewSpawn.bDeleteMe@NewSpawn.Controller);
	// clear the claim immediately as there is no reason to queue this way
	SpawnSlots[SlotIdx].SpawningPawn = None;
	// tell the Nem to shoot out
	Nem = GearAI_Nemacyst(NewSpawn.Controller);
	if (Nem != None)
	{
		Nem.DoLaunch();
	}
}

simulated function DoFootStep( int FootDown, bool bHardStep, ParticleSystemComponent PSComponent )
{
	local vector SoundLocation;
	local rotator SoundRotation;

	// Hard step sounds are taken care of in a different notify
	if (Role == ROLE_Authority && !bHardStep)
	{
		Mesh.GetSocketWorldLocationAndRotation( FeetSocketNames[FootDown], SoundLocation, SoundRotation );
		PlaySound( FootstepSound, false, false, false, SoundLocation );
	}

	// do the effect
	PSComponent.ActivateSystem();
}

/**
 *  Set flag so we spawn namacyst.
 */
event ANIM_NOTIFY_SpawnNemacyst()
{
	bAllowSpawn = TRUE;
}

simulated event ANIM_NOTIFY_EffectSideSpit()
{
	PSC_ButtSideSpit.ActivateSystem();
}

simulated event ANIM_NOTIFY_EffectFrontSpit()
{
	PSC_ButtFrontSpit.ActivateSystem();
}

simulated event ANIM_NOTIFY_EffectSpit()
{
	PSC_ButtSpit.ActivateSystem();
}

simulated event ANIM_NOTIFY_EffectBodyDust()
{
	PSC_BodyDust.ActivateSystem();
}

/** Helper function to do camera shakes with a max distance to be relevant **/
simulated function DoCameraShake( const out ScreenShakeStruct CamShakeToDo )
{
	local GearPC PC;

	foreach WorldInfo.AllControllers( class'GearPC', PC )
	{
		if(	( PC.Pawn != none && VSize(PC.Pawn.Location - Location) < CameraShakeDistance ) )
		{
			PC.ClientPlayCameraShake( CamShakeToDo, TRUE );
		}
	}
}

simulated event ANIM_NOTIFY_CameraShake1()
{
	DoCameraShake( CamShake1 );
}

simulated event ANIM_NOTIFY_CameraShake2()
{
	DoCameraShake( CamShake2 );
}

simulated event ANIM_NOTIFY_CameraShake3()
{
	DoCameraShake( CamShake3 );
}

simulated event ANIM_NOTIFY_CameraShake4()
{
	DoCameraShake( CamShake4 );
}

simulated event ANIM_NOTIFY_CameraShake5()
{
	DoCameraShake( CamShake5 );
}

simulated event ANIM_NOTIFY_FootStep0()
{
	DoFootStep( 0, false, PSC_FeetImpacts[0] );
}

simulated event ANIM_NOTIFY_FootStep1()
{
	DoFootStep( 1, false, PSC_FeetImpacts[1] );
}

simulated event ANIM_NOTIFY_FootStep2()
{
	DoFootStep( 2, false, PSC_FeetImpacts[2] );
}

simulated event ANIM_NOTIFY_FootStep3()
{
	DoFootStep( 3, false, PSC_FeetImpacts[3] );
}

simulated event ANIM_NOTIFY_FootStep4()
{
	DoFootStep( 4, false, PSC_FeetImpacts[4] );
}

simulated event ANIM_NOTIFY_FootStep5()
{
	DoFootStep( 5, false, PSC_FeetImpacts[5] );
}

simulated event ANIM_NOTIFY_FootStep6()
{
	DoFootStep( 6, false, PSC_FeetImpacts[6] );
}

simulated event ANIM_NOTIFY_FootStep7()
{
	DoFootStep( 7, false, PSC_FeetImpacts[7] );
}

simulated event ANIM_NOTIFY_FootStepHeavy2()
{
	DoFootStep( 2, false, PSC_HeavyFoot2 );
}

simulated event ANIM_NOTIFY_FootStepHeavy3()
{
	DoFootStep( 3, false, PSC_HeavyFoot3 );
}

defaultproperties
{
	bEdShouldSnap=TRUE
	bBlockActors=TRUE
	bCollideActors=TRUE
	bWorldGeometry=FALSE
	bGameRelevant=TRUE
	bProjTarget=TRUE
	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=true

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
	    LightShadowMode=LightShadow_Modulate
		bSynthesizeSHLight=FALSE
		bDynamic=FALSE
		bCastShadows=FALSE
		TickGroup=TG_DuringAsyncWork
		MinTimeBetweenFullUpdates=1.0f
		InvisibleUpdateTime=5.0f
	End Object
	Components.Add(MyLightEnvironment)

	Begin Object Class=SkeletalMeshComponent Name=SeederSkelMesh
		SkeletalMesh=SkeletalMesh'Locust_Seeder.Locust_Seeder'
		AnimSets=(AnimSet'Locust_Seeder.Locust_Seeder_Anims')
		AnimTreeTemplate=AnimTree'Locust_Seeder.Locust_Seeder_AnimTree'
		PhysicsAsset=PhysicsAsset'Locust_Seeder.Locust_Seeder_Physics'
		CollideActors=TRUE
		BlockActors=TRUE
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=TRUE
		bCastDynamicShadow=TRUE
		LightEnvironment=MyLightEnvironment
	End Object
	Mesh=SeederSkelMesh
	CollisionComponent=SeederSkelMesh
	Components.Add(SeederSkelMesh)

	Begin Object Class=CylinderComponent Name=CollisionCylinder
		CollisionRadius=+0500.000000
		CollisionHeight=+0500.000000
		BlockNonZeroExtent=TRUE
		BlockZeroExtent=FALSE
		BlockActors=TRUE
		CollideActors=TRUE
	End Object
	CollisionCylinderComponent=CollisionCylinder
	Components.Add(CollisionCylinder)

	SeederAnimNames(SA_Idle)="Idle"
	SeederAnimNames(SA_Spit)="Fire"
	SeederAnimNames(SA_Death_Quick)="Death_Quick"
	SeederAnimNames(SA_Death_Uber)="Death"
	SeederAnimNames(SA_Death_QuickB)="Death_Quick_B"
	SeederAnimNames(SA_HitReactionHOD)="HitReaction_HOD"
	SeederAnimNames(SA_Death_Hide)="Death_Quick_C"
	SeederAnimNames(SA_HitReactionBig)="HitReaction_Big"
	SeederAnimNames(SA_HitReactionSmall)="HitReaction_Small"

	NumHODHitsForKill=2
	DeathAnim=SDA_Death_Quick

	CameraShakeDistance=1024.0f

	CamShake1=(TimeDuration=0.300000,RotAmplitude=(X=100,Y=100,Z=200),RotFrequency=(X=10,Y=10,Z=25),LocAmplitude=(X=0,Y=3,Z=5),LocFrequency=(X=1,Y=10,Z=20),FOVAmplitude=2,FOVFrequency=5)
	CamShake2=(TimeDuration=0.400000,RotAmplitude=(X=100,Y=100,Z=200),RotFrequency=(X=10,Y=10,Z=25),LocAmplitude=(X=0,Y=3,Z=5),LocFrequency=(X=1,Y=10,Z=20),FOVAmplitude=2,FOVFrequency=5)
	CamShake3=(TimeDuration=0.200000,RotAmplitude=(X=100,Y=100,Z=200),RotFrequency=(X=10,Y=10,Z=25),LocAmplitude=(X=0,Y=3,Z=5),LocFrequency=(X=1,Y=10,Z=20),FOVAmplitude=2,FOVFrequency=5)
	CamShake4=(TimeDuration=0.100000,RotAmplitude=(X=100,Y=100,Z=200),RotFrequency=(X=10,Y=10,Z=25),LocAmplitude=(X=0,Y=3,Z=5),LocFrequency=(X=1,Y=10,Z=20),FOVAmplitude=2,FOVFrequency=5)
	CamShake5=(TimeDuration=0.100000,RotAmplitude=(X=100,Y=100,Z=200),RotFrequency=(X=10,Y=10,Z=25),LocAmplitude=(X=0,Y=3,Z=5),LocFrequency=(X=1,Y=10,Z=20),FOVAmplitude=2,FOVFrequency=5)

	FootstepSound=SoundCue'Locust_Corpser_Efforts.Corpser.CorpserFootstepCue'
	FeetSocketNames(0)="L_06"
	FeetSocketNames(1)="L_09"
	FeetSocketNames(2)="L_12"
	FeetSocketNames(3)="R_12"
	FeetSocketNames(4)="R_09"
	FeetSocketNames(5)="R_06"
	FeetSocketNames(6)="R_03"
	FeetSocketNames(7)="L_03"

	StopHitReactionDelta=0.5f

	bPlayedDeathScream=false;
	DeathScream=SoundCue'Locust_Seeder_Efforts.SeederEfforts.Seeder_RoarHugeCue';

}
