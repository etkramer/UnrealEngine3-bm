/**
 * Locust Wretch Dark
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustDarkWretch extends GearPawn_LocustWretch
	config(Pawn);


/** when dark wretches get hurt they glow glow glow **/
var MaterialInstanceConstant MIC;

/** Duration before the dark wretch blows up **/
var config float TimeBeforeExplosion;

var config float DeathExplosionDamageRadius;

var config float DeathExplosionDamage;

replication
{
	if (bNetInitial)
		TimeBeforeExplosion;
}

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	if( Attacker == None || !Attacker.IsHumanControlled() )
	{
		return FALSE;
	}

	return super.CanBeSpecialMeleeAttacked( Attacker );
}

event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	Super.TakeDamage(Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);

	// here want to check to see if we should switch materials to the damaged material
	MaterialSwitchBasedOnDamage();
}


function MaterialSwitchBasedOnDamage()
{
	if( DefaultHealth * 0.7 < Health )
	{
		if( MIC == none )
		{
			MIC = new(outer) class'MaterialInstanceConstant';

			MIC.SetParent( Material'Locust_Wretch.Locust_WretchD_LP_Mat_Explo' );
			Mesh.SetMaterial( 0, MIC );
		}

		if( DefaultHealth * 0.4 < Health )
		{
			MIC.SetScalarParameterValue( 'Glow', 1.0f );
		}
	}
}

simulated state Dying
{
	simulated function BeginState(Name PreviousStateName)
	{
		EndSpecialMove();

		Super.BeginState(PreviousStateName);

		if (TimeBeforeExplosion > 0.0)
		{
			SetTimer( TimeBeforeExplosion, false, nameof(ExplodeAfterDeath) );
			if (Role == ROLE_Authority)
			{
				SetTimer( TimeBeforeExplosion * 0.5f, false, nameof(WarnOfExplosion) );
			}
		}
		else
		{
			ExplodeAfterDeath();
		}
	}


Begin:

	SpawnPreDeathExplosionParticleEffect( 'b_W_UpperArm_L' );
	Sleep(0.10f);

	SpawnPreDeathExplosionParticleEffect( 'b_W_Thigh_R' );
	Sleep(0.20f);

	SpawnPreDeathExplosionParticleEffect( 'b_W_Hand_R' );
	Sleep(0.10f);

	SpawnPreDeathExplosionParticleEffect( 'b_W_Clavicle_R_' );
	Sleep(0.20f);

	SpawnPreDeathExplosionParticleEffect( 'b_W_Calf_R' );
	Sleep(0.20f);

	SpawnPreDeathExplosionParticleEffect( 'b_W_Foot_L' );
	Sleep(0.10f);

	SpawnPreDeathExplosionParticleEffect( 'b_W_Head' );

}


function SpawnPreDeathExplosionParticleEffect( name BoneName )
{
	local vector SpawnLocation;
	local Emitter FX;

	SpawnLocation = Mesh.GetBoneLocation( BoneName, 0 );

	FX = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( ParticleSystem'Locust_Wretch.Effects.P_Wretch__InitExplo', SpawnLocation, Rotation );

	FX.SetDrawScale( 2.0 );
	FX.ParticleSystemComponent.ActivateSystem();
	PlaySound( SoundCue'Weapon_Pistol.Firing.LocustPistolFireCue' );  // temp for now, but at least hear the sound
}

simulated function OnSetPhysics( SeqAct_SetPhysics Action )
{
	if( Action.NewPhysics == PHYS_Interpolating )
	{
		bNoEncroachCheck = FALSE;
		TimeBeforeExplosion = 0.f;
	}

	super.OnSetPhysics( Action );
}

function bool Died(Controller Killer, class<DamageType> GearDamageType, vector HitLocation)
{
	local bool bResult;

	bResult = Super.Died( Killer, GearDamageType, HitLocation );

	PlaySound( SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchADeathInstantsCue' );

	return bResult;
}

simulated function DeathCry()
{
	PlaySound(SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchADeathsCue');
}
/**
 *	Warn any AI enemies in the area about the attack and make them evade
 */
function WarnOfExplosion()
{
	local GearAI AI;
	local float DistSq, RadiusSq;

	RadiusSq = DeathExplosionDamageRadius*DeathExplosionDamageRadius;
	foreach WorldInfo.AllControllers( class'GearAI', AI )
	{
		if( AI.Pawn != None &&
			AI.Pawn != self )
		{
			if( FRand() < AI.ChanceToEvadeGrenade )
			{
				DistSq = VSizeSq(Location - AI.Pawn.Location);
				if( DistSq <= RadiusSq )
				{
					AI.EvadeAwayFromPoint( Location );
				}
			}
		}
	}
}

simulated function ExplodeAfterDeath()
{
	//local GearProj_Boomshot Kaboom;
	local vector SpawnLocation;
	local Emitter FX;

	//HitDamageType = class'GDT_Explosive';
	//InitRagdoll();

	//Mesh.SetMaterial( 0, Material'Locust_Wretch.Locust_WretchD_LP_Mat_Glow_Gore' );

	SpawnLocation = Location + vect(0,0,40);

	HurtRadius( DeathExplosionDamage, DeathExplosionDamageRadius, class'GDT_Explosive', 1.f, SpawnLocation+vect(0,0,1)*3,, Controller );

	MakeNoise( 1.0 );

	PlaySound(SoundCue'Weapon_Grenade.Impacts.GrenadeExplosionCue', true);
	PlaySound(SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchBodyExplodeCue', true);

	FX = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( ParticleSystem'Locust_Wretch.Effects.P_Wretch_Explo', SpawnLocation, Rotation );

	FX.ParticleSystemComponent.ActivateSystem();
	Mesh.SetHidden( TRUE );

	SpawnAWretchGib( StaticMesh'Neutral_Stranded_01.Gore_Skull1_CP_SMesh', vect(0,10,10)*2 );

	SpawnAWretchGib( StaticMesh'Neutral_Stranded_01.Gore_Chunk5_CP_SMesh', vect(4,4,30)*2 );
	SpawnAWretchGib( StaticMesh'Neutral_Stranded_01.Gore_Chunk5_CP_SMesh', vect(10,-10,10)*2 );
	SpawnAWretchGib( StaticMesh'Neutral_Stranded_01.Gore_Chunk5_CP_SMesh', vect(20,-15,10)*2 );

	SpawnAWretchGib( StaticMesh'Neutral_Stranded_01.Gore_Torso1_CP_SMesh', vect(10,5,10)*2 );


	//Kaboom = Spawn(class'GearProj_Boomshot', self,,SpawnLocation );
	//Kaboom.Velocity = vect(3000,125,-450);
	//Kaboom.Explode( SpawnLocation, vect(0,0,1) );
}


/**
* This will spawn a helmet near the locust's head.  This is
* a quick placeholder for the real spawn which we might want to
* make it not so "jerky pop into view"
*
**/
simulated function SpawnAWretchGib( StaticMesh TheMesh, Vector ApplyImpulse )
{
	local vector SpawnLoc;
	local float Radius, Height;
	local KActor AGib;

	GetBoundingCylinder( Radius, Height );

	SpawnLoc = Location + ( Vect( 0, 0, 1) * Height );

	AGib = Spawn( class'Item_DarkWretchGib',,, SpawnLoc, );
	AGib.StaticMeshComponent.SetStaticMesh( TheMesh );
	AGib.SetHidden(True);

	AGib.CollisionComponent.AddImpulse( ApplyImpulse * 1.2, vect(0,0,100) );
	AGib.CollisionComponent.SetRBAngularVelocity( vect(0,100,300), TRUE );

}

/** Dark wretches do not spawn blood pools**/
simulated function DoSpawnABloodPool();
simulated function SpawnBloodPool();
simulated event PlayGibEffect(vector HitLoc, vector HitNorm, float SquaredForce);
simulated function PlayHeadShotDeath();
simulated function SpawnAHeadChunk( vector HeadLoc, rotator HeadRot, StaticMesh TheMesh );
simulated event ConstraintBrokenNotify( Actor ConOwner, RB_ConstraintSetup ConSetup, RB_ConstraintInstance ConInstance  );

defaultproperties
{
	bShowFatalDeathBlood=FALSE

	DefaultInventory(0)=class'GearGame.GearWeap_DarkWretchMelee'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustWretch'
	HowlSound=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchGroan01Cue'

	GoreSkeletalMesh=none
	GorePhysicsAsset=none

	Begin Object Name=GearPawnMesh
 		SkeletalMesh=SkeletalMesh'Locust_Wretch.Mesh.Locust_Wretch'
		PhysicsAsset=PhysicsAsset'Locust_Wretch.Mesh.Locust_Wretch_Physics_Dark'
		AnimTreeTemplate=AnimTree'Locust_Wretch.Locust_Wretch_AnimTree'
		AnimSets(0)=AnimSet'Locust_Wretch.anims.Locust_Wretch_Anims'
		RootMotionMode=RMM_Velocity
		Materials(0)=Material'Locust_Wretch.Locust_WretchD_LP_Mat_1'
	End Object

	ScreamSound=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchScreamTempCue'
	SwipeAttack=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchAAttackSmallsCue'
	SwipeAttackMissAndScrapeAcrossWall=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchArmImpactBodyCue'
	SwipeImpactFlesh=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchArmImpactFleshCue'
	SwipeTelegraph=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchAAttackLargesCue'

	BiteSound=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchBiteCue'

	LeapSound=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchBodyLeapCue'

	LandSound=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchBodyLandCue'
	LandOnWallSound=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchBodyLandWallCue'
	LandOnEnemySound=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchBodyLandBodyCue'

	BodyExplode=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchBodyExplodeCue'
	LimbSevered=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchLimbSeverCue'
	GibBounce=SoundCue'Foley_Flesh.Flesh.GibBodyLimbBounceCue'

	ArmSound=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchHandstepsCue'
	FootSound=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchFootStepsCue'

	PainSound=SoundCue'Locust_Wretch_Efforts.DarkWretch.DWretchAPainMediumsCue'

	GoreImpactParticle=none
}
