/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/
class GearPawn_LocustNemaslug extends GearPawn
	config(Pawn);

var SoundCue AmbientSoundCue;


var BodyStance	BS_HitReaction;

/**
 * So for non extreme content we are not gore exploding this guy.  Which leaves this semi large mesh lying aruond with no real feedback
 * that you have killed him.  So we play this effect to "replace" the gore explosion.
 **/
var ParticleSystem PS_NonExtremeContentDeathEffect;


simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	SetTimer(RandRange(2,5),FALSE,nameof(AmbientSounds));
}

function TakeDamage
(
	 int					Damage,
	Controller			InstigatedBy,
	Vector				HitLocation,
	Vector				Momentum,
	class<DamageType>	DamageType,
	optional	TraceHitInfo		HitInfo,
	optional	Actor				DamageCauser
)
{
	// Nemaslugs not damaged by cilia in riftworm
	if( DamageType.IsA( 'GDT_Cilia') )
	{
		return;
	}

	Super.TakeDamage( Damage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );
}

simulated function AmbientSounds()
{
	PlaySound(AmbientSoundCue);
	SetTimer(RandRange(2,5),FALSE,nameof(AmbientSounds));
}

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return false;
}

reliable server function Knockdown(vector LinearVelocityImpulse, vector AngularVelocityImpulse);

function bool Died(Controller Killer, class<DamageType> DamageType, vector HitLocation)
{
	ClearTimer(nameof(AmbientSounds));
	return Super.Died(Killer,DamageType,HitLocation);
}

function ANIM_NOTIFY_DoMeleeDamage()
{
	local GearWeap_NemaSlugMelee	Wpn;
	local float Tolerance;
	local actor HitActor;
	local vector HitNormal,HitLocation;
	local vector Start, End;

	if( IsDoingSpecialMove(GSM_SwipeAttack) )
	{
		Wpn = GearWeap_NemaSlugMelee(Weapon);
		Tolerance = Controller.Enemy.GetCollisionRadius() + GetAIMeleeAttackRange();

		if( Wpn != None )
		{
			Start = Location;
			End = Start + vector(Rotation) * Tolerance;
			Start += Normal(End-Start) * GetCollisionRadius();
			HitActor = Trace(HitLocation,HitNormal,End,Start,TRUE,vect(25.f,25.f,25.f));

			if(HitActor == Controller.Enemy)
			{
				Wpn.DoMeleeDamage( Controller.Enemy, Controller.Enemy.Location, 1.f );
			}
			//`log(GetFuncName()@HitActor);
		}
	}
}

/** Play Hit Reaction animation */
simulated function bool PlayHitReactionAnimation(ImpactInfo Impact)
{
	MessagePlayer(GetFuncName());
	BS_HitReaction.AnimName[BS_FullBody] = 'NS_HitReact';
	BS_PlayByDuration(BS_HitReaction,0.5f,0.3,0.33f,true,true);
	SetTimer(0.25f,FALSE,nameof(StopHitReact));
	return true;
}

simulated function StopHitReact()
{
	BS_Stop(BS_HitReaction,0.33f);
}

/** blow up guy when he is dead **/
simulated function PlayDeath(class<GearDamageType> GearDamageType, vector HitLoc)
{
	// Abort current special move
	if( IsDoingASpecialMove() )
	{
		EndSpecialMove();
	}

	Super.PlayDeath( class'GDT_Explosive', HitLoc );

	// this is non gore so we are just going to scale the dead body out and play a particle effect to help sell the "death"
	// safety net for when somehow the create gore skeleton/goreexplosion doesn't occur correctly
	if( !bHasGoreExploded )
	{
		PlayNonGoreDeath( GearDamageType, HitLoc );
	}
}

/** This will play the non blood and gore explosion Death.  Which right now is a particle and then quickly fading out**/
simulated function PlayNonGoreDeath(class<GearDamageType> GearDamageType, vector HitLoc)
{
	local Emitter ImpactEmitter;

	// particle effect here
	ImpactEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_NonExtremeContentDeathEffect, Location, Rotation );
	ImpactEmitter.SetDrawScale( 1.0f );
	ImpactEmitter.ParticleSystemComponent.ActivateSystem();

	SetTimer( 2.0f, FALSE, nameof(GoreAroundTooLong) );
}


function float GetAIAccuracyModWhenTargetingMe(GearAI GuyTargetingMe)
{
	return 0.5f;
}
	
simulated function vector GetMeleeHitTestLocation()
{
	return Location + vect(0,0,40);
}

defaultproperties
{
	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearWeap_NemaSlugMelee'

	ControllerClass=class'GearGame.GearAI_NemaSlug'

	GoreSkeletalMesh=SkeletalMesh'Locust_Nemaslug.Mesh.Locust_Nemaslug_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Nemaslug.Mesh.Locust_Nemaslug_Physics'
	GoreBreakableJointsTest=("NC_VentralFin","RtForeArm","LtClaw","NC_Tail02","LtArmUpper")
	GoreBreakableJoints=("NC_VentralFin","RtForeArm","LtClaw","NC_Tail02","LtArmUpper")

	Begin Object Name=MyLightEnvironment
		bSynthesizeSHLight=FALSE
	End Object

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Nemaslug.Mesh.Locust_Nemaslug'
		PhysicsAsset=PhysicsAsset'Locust_Nemaslug.Mesh.Locust_Nemaslug_Physics'
		AnimTreeTemplate=AnimTree'Locust_Nemaslug.AnimTree.Locust_Nemaslug_AT'
		AnimSets(0)=AnimSet'Locust_Nemaslug.Anims.Locust_Nemaslug_Animset'
		Rotation=(Yaw=-16384)
		bHasPhysicsAssetInstance=TRUE
		bEnableFullAnimWeightBodies=TRUE
	End Object

	Begin Object Name=CollisionCylinder
		CollisionHeight=+0042.000000
	End Object

	Health=25

	bCanDoRun2Cover=FALSE
	bCanDBNO=FALSE
	bRespondToExplosions=FALSE


	HearingThreshold=6000.f
	PelvisBoneName=none

	AimAttractors(0)=(OuterRadius=96.f,InnerRadius=32.f,BoneName="NC_Head")

	AmbientSoundCue=SoundCue'Locust_Nemaslug_Efforts.Nemaslug.Nemaslug_AmbientCue'

	SpecialMoveClasses(GSM_SwipeAttack)		=class'GSM_NemaSlug_Melee'
	SpecialMoveClasses(SM_Emerge_Type1)	=class'GSM_Nemaslug_Emerge'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustNemaSlug'
	DrawScale=1.2f

	bNoDeathGUDSEvent=TRUE

	PS_NonExtremeContentDeathEffect=ParticleSystem'Effects_Gameplay.Blood.P_Blood_Crowd_Gib_Medium_NoGore'
}
