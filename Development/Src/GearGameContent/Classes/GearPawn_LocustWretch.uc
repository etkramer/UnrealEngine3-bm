/**
 * Locust Wretch Content
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustWretch extends GearPawn_LocustWretchBase;


var SoundCue SwipeTelegraph;

/** Scream Sound **/
var SoundCue ScreamSound;

/** Swipe Attack Sound **/
var SoundCue SwipeAttack;

/** Swung arm attack impacts metal body armor or wall surface and nails scratch **/
var SoundCue SwipeAttackMissAndScrapeAcrossWall;

/** Swung arm attack impacts body flesh and rips **/
var SoundCue SwipeImpactFlesh;

/** Wretch bite attack (if needed) **/
var SoundCue BiteSound;

/** Wretch body leaps + jumps **/
var SoundCue LeapSound;

/** After leap, wretch body lands on generic surfaces **/
var SoundCue LandSound;

/** After leap, wretch body lands on wall, from horizontal to vertical **/
var SoundCue LandOnWallSound;

/** After leap, wretch body lands on human\locust body **/
var SoundCue LandOnEnemySound;

/** Wretches body explodes **/
var SoundCue BodyExplode;

/**  Wretch arm, leg, and head is severed **/
var SoundCue LimbSevered;

/** Severed wretch arm, leg, and head impacts ground **/
var SoundCue GibBounce;

/** Wretch howls while out of combat**/
var SoundCue HowlSound;

var SoundCue FootSound;
var SoundCue ArmSound;


var SoundCue PainSound;
var float NextPainSound;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// Play Howl sound at random intervals
	PlayHowlSound();
}

simulated function DeathCry()
{
	PlaySound(SoundCue'Locust_Wretch_Efforts.Wretch.WretchADeathsCue');
}

event TakeDamage( int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	Super.TakeDamage( Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);
	if(Damage > 0 && NextPainSound < WorldInfo.TimeSeconds)
	{
		NextPainSound = WorldInfo.TimeSeconds + Frand()*3 + 1;
		if(Damage > DefaultHealth*0.5)
		{
			PainSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchAPainLargesCue';
		}
		else if(Damage > DefaultHealth*0.3)
		{
			PainSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchAPainMediumsCue';
		}
		else
		{
			PainSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchAPainSmallsCue';
		}
		PlaySound(PainSound);
	}

}


/**
 * Overridden to replace the skeleton with a gore skeleton which has breakable constraints
 */

simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	PlaySound( BodyExplode );

	Super.PlayDying(DamageType, HitLoc);
}

function Landed( Vector HitNormal, Actor FloorActor )
{
	super.Landed( HitNormal, FloorActor );

	PlaySound( LandSound );
}

function PlayLandedOnWallSound()
{
	PlaySound( LandOnWallSound );
}

function PlayLandedOnEnemySound()
{
	PlaySound( LandOnEnemySound );
}

function PlayLeapSound()
{
	PlaySound( LeapSound );
	PlaySound( SwipeTelegraph );
}


function PlaySwipeAttackSound()
{
	PlaySound( SwipeAttack );
}

function PlaySwipeAttackMissAndScrapeAcrossWall()
{
	PlaySound( SwipeAttackMissAndScrapeAcrossWall );
}

function PlaySwipeAttackHitSound()
{
	PlaySound( SwipeImpactFlesh );
}

simulated function PlayScreamSound()
{
	//GearAI(Controller).PlaySoundOnPlayer( ScreamSound );
	PlaySound(ScreamSound);
}

/** Play Howling sound. Called with a timer and random intervals until dead. */
function PlayHowlSound()
{
	local GearAI_Wretch WretchAI;

	// If we're dead, then stop the timer.
	if( bPlayedDeath )
	{
		return;
	}

	// Play Howling sound if we have no enemy
	WretchAI = GearAI_Wretch(Controller);
	if( WretchAI != None )
	{
		if( WretchAI.GetNumEnemies() == 0 )
		{
			PlaySound( HowlSound );
		}

		// Set timer for next check.
		SetTimer( WorldInfo.TimeSeconds + 5 + Frand() * 10, FALSE, nameof(PlayHowlSound) );
	}
}

simulated event PlayFootStepSound( int FootDown )
{
	if( FootDown > 1 ) // arm
	{
		PlaySound(ArmSound);
	}
	else // leg
	{
		PlaySound(FootSound);
	}
	if(!bFleshTimer)
	{
		bFleshTimer = true;	
		SetTimer( FRand()*0.5f, FALSE, nameof(PlayFleshSound) );
	}	
}

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	// Don't allow attackers to melee wretch when they are on the ceiling
	if( Attacker == None || 
		(!Attacker.IsHumanControlled() && GearWallPathNode(Anchor) != None) )
	{
		return FALSE;
	}
	
	return Super.CanBeSpecialMeleeAttacked( Attacker );
}

defaultproperties
{
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustWretch'

	HostageHealthBuckets.Empty()

	GoreSkeletalMesh=SkeletalMesh'Locust_Wretch.Gore.Wretch_Gore_HardWeights'
	GorePhysicsAsset=PhysicsAsset'Locust_Wretch.Gore.Wretch_Gore_HardWeights_Physics'
	PhysicsBodyImpactBoneList=("b_W_Spine_02","b_W_Forearm_L","b_W_UpperArm_R","b_W_Forearm_R","b_W_Hand_R","b_W_Neck","b_W_Jaw","b_W_Calf_L","b_W_Foot_R")
	GoreBreakableJoints=("b_W_Spine_02","b_W_Forearm_L","b_W_UpperArm_R","b_W_Forearm_R","b_W_Hand_R","b_W_Neck","b_W_Jaw","b_W_Calf_L","b_W_Foot_R")

	NeckBoneName="b_W_Neck"
	MeleeDamageBoneName="b_W_Spine_02"


	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Wretch.Mesh.Locust_Wretch'
		PhysicsAsset=PhysicsAsset'Locust_Wretch.Mesh.Locust_Wretch_Physics'
		AnimTreeTemplate=AnimTree'Locust_Wretch.Locust_Wretch_AnimTree'
		AnimSets(0)=AnimSet'Locust_Wretch.anims.Locust_Wretch_Anims'
		RootMotionMode=RMM_Velocity
	End Object

	ScreamSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchScreamTempCue'
	SwipeAttack=SoundCue'Locust_Wretch_Efforts.Wretch.WretchAAttackSmallsCue'
	SwipeTelegraph=SoundCue'Locust_Wretch_Efforts.Wretch.WretchAAttackLargesCue'
	SwipeAttackMissAndScrapeAcrossWall=SoundCue'Locust_Wretch_Efforts.Wretch.WretchArmImpactBodyCue'
	SwipeImpactFlesh=SoundCue'Locust_Wretch_Efforts.Wretch.WretchArmImpactFleshCue'

	BiteSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchBiteCue'

	LeapSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchBodyLeapCue'

	LandSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchBodyLandCue'
	LandOnWallSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchBodyLandWallCue'
	LandOnEnemySound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchBodyLandBodyCue'

	BodyExplode=SoundCue'Locust_Wretch_Efforts.Wretch.WretchBodyExplodeCue'
	LimbSevered=SoundCue'Locust_Wretch_Efforts.Wretch.WretchLimbSeverCue'
	GibBounce=SoundCue'Foley_Flesh.Flesh.GibBodyLimbBounceCue'
	HowlSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchGroan01Cue'

	ArmSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchHandstepsCue'
	FootSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchFootStepsCue'

	PainSound=SoundCue'Locust_Wretch_Efforts.Wretch.WretchAPainMediumsCue'
	NextPainSound = 0.0f;
}
