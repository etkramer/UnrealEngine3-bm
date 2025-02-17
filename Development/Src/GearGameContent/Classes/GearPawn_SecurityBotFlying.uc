/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearPawn_SecurityBotFlying extends GearPawn_SecurityBotFlyingBase;

/** Sound that plays when an enemy is first noticed */
var SoundCue EnemyAcquiredNoise;

/** audio component playing our ambient sound */
var AudioComponent AmbientComp;

/** cached ref to gun pitch control */
var SkelControlSingleBone GunPitchControl;
/** name of the gun pitch skel control */
var() name PitchSkelControlName;

var MaterialInstanceConstant BodyMaterial;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	BodyMaterial = Mesh.CreateAndSetMaterialInstanceConstant(0);
}

simulated function OnAcquireEnemy(GearPawn Enemy)
{
	bAngryBot = TRUE;
	PlayAngryEffect();
	PlaySound(EnemyAcquiredNoise);
}

simulated function OnLostEnemy()
{
	bAngryBot = FALSE;
	PlayAngryEffect();
}

simulated function CacheAnimNodes()
{
	Super.CacheAnimNodes();

	GunPitchControl = SkelControlSingleBone(Mesh.FindSkelControl(PitchSkelControlName));
	if(GunPitchControl != none)
	{
		GunPitchControl.bApplyRotation = TRUE;
		GunPitchControl.BoneRotationSpace = BCS_WorldSpace;
		//GunPitchControl.SetSkelControlActive(TRUE);
	}
}

simulated function ClearAnimNodes()
{
	Super.ClearAnimNodes();
	GunPitchControl=none;
}

simulated event Tick(float DeltaTime)
{
	
	super.Tick(DeltaTime);	

	DesiredRotation.Pitch =0;
	if(GunPitchControl != none && Controller != none && Controller.Focus != none)
	{		
		GunPitchControl.SetSkelControlStrength(1.0f,0.25f);
		GunPitchControl.BoneRotation = Rotator(Controller.Focus.Location - Location);
		GunPitchControl.BoneRotation.Yaw = Rotation.Yaw;
		GunPitchControl.BoneRotation.Roll = Rotation.Roll;
	}
	else
	{
		GunPitchControl.SetSkelControlStrength(0.f,0.25f);
	}
	
}


simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	Super.PlayDying(DamageType,HitLoc);
	AmbientComp.Stop();
}

simulated function PlayDamageEffect()
{
	if( DamageTakenPct >= DamageEffectPct )
	{
		if( PSC_Damage == None )
		{
			PSC_Damage = new(self) class'ParticleSystemComponent';
			PSC_Damage.SetTemplate( PS_Damage );
			Mesh.AttachComponentToSocket( PSC_Damage, 'Jets' );
			PSC_Damage.SetHidden( FALSE );
			PSC_Damage.ActivateSystem();
		}
		BodyMaterial.SetScalarParameterValue( 'Damage', 1.f );
	}
	else
	{
		BodyMaterial.SetScalarParameterValue( 'Damage', DamageTakenPct );
	}
}

simulated function PlayAngryEffect()
{
	if( bAngryBot )
	{
		BodyMaterial.SetScalarParameterValue( 'SentryAngryState', 1.f );
		PSC_Eye.SetTemplate( PS_Eye_Angry );
	}
	else
	{
		BodyMaterial.SetScalarParameterValue( 'SentryAngryState', 0.f );
		PSC_Eye.SetTemplate( PS_Eye );
	}
}

simulated function StartBloodTrail( name TrailFuncName, optional float SpawnRate )
{
	// no thanks
}
/** This will leave a trail of blood when you are DBNO **/
simulated function SpawnABloodTrail_DBNO();

/** This will leave a trail of blood when you are MeatBagging **/
simulated function SpawnABloodTrail_MeatBag();

/** This will leave a trail of blood when you are in cover **/
simulated function SpawnABloodTrail_Wall();

simulated function SpawnABloodTrail_GibExplode_Ground();

/** This will unleash a blast of gore on the walls, ceiling, and floor around the character**/
simulated function SpawnABloodTrail_GibExplode_360();

/** This will leave a trail of blood on the ground when you chainsaw someone **/
simulated function SpawnABloodTrail_ChainsawSpray_Ground();

/** This will leave a trail of blood on the wall when you chainsaw someone **/
simulated function SpawnABloodTrail_ChainsawSpray_Wall();

/** This will leave a trail of blood on the wall when you chainsaw someone **/
simulated function SpawnABloodTrail_GibImpact( vector HitLoc );

/** This will leave a trail of blood from your headshot stump **/
simulated function SpawnABloodTrail_HeadShot();

/** This will leave a small splat of blood **/
simulated function SpawnABloodTrail_HitByABullet();

/** This will leave some blood as we are really hurt **/
simulated function SpawnABloodTrail_PawnIsReallyHurt();

/** This will leave some blood when one of our limbs brealks **/
simulated function SpawnABloodTrail_LimbBreak( vector InStartLocation );

defaultProperties
{
	PeripheralVision=-0.5f
	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGameContent.GearWeap_SecurityBotGunFlying'
	
	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Sentry_Bot.Meshes.Sentry_Bot'
		AnimTreeTemplate=AnimTree'Sentry_Bot.Sentry_Bot_Animtree'
		AnimSets(0)=AnimSet'Sentry_Bot.Anims.Sentry_Bot_Animset'
		PhysicsAsset=PhysicsAsset'Sentry_Bot.Meshes.Sentry_Bot_Physics'
		//Rotation=(Yaw=-16384)
	End Object

	PitchSkelControlName=Gun_Pitch
	//Begin Object Class=StaticMeshComponent Name=GearPawnMesh2
	//	StaticMesh=StaticMesh'Trap_LaserGrid.Meshes.S_FloatingDrone_01'
	//	BlockZeroExtent=true
	//	CollideActors=true
	//	BlockRigidBody=true
	//End Object
	//Components.Add(GearPawnMesh2)

	bTranslateMeshByCollisionHeight=FALSE
	Begin Object Name=CollisionCylinder
		CollisionHeight=+0030.000000
		CollisionRadius=+0030.000000
		BlockZeroExtent=TRUE
	End Object

	// Hover sound
	Begin Object Class=AudioComponent Name=AudioComponent0
		SoundCue=SoundCue'Ambient_Loops.Machines_G2.machine_medical08Cue'	
		bAutoPlay=true
		bStopWhenOwnerDestroyed=true
		bShouldRemainActiveIfDropped=true
	End Object
	AmbientComp=AudioComponent0
	Components.Add(AudioComponent0)

	EnemyAcquiredNoise=SoundCue'Ambient_NonLoop.Stereo_G2.policecar_pullaway03_stereoCue'

	Begin Object Name=ExploTemplate0
		ParticleEmitterTemplate=ParticleSystem'Sentry_Bot.Effects.P_Sentry_Bot_Explosion'
		ExplosionSound=SoundCue'Weapon_Grenade.Impacts.GrenadeBoloExplosionCue'
	End Object

	PhysicalFireLocBoneName=Muzzle
	RightHandSocketName=Muzzle

	PelvisBoneName=none

	PS_Eye=ParticleSystem'Sentry_Bot.Effects.P_Eye'
	PS_Eye_Angry=ParticleSystem'Sentry_Bot.Effects.P_Eye_Red'
	PS_Jets=ParticleSystem'Sentry_Bot.Effects.P_Sentry_Bot_Jets'
	PS_Damage=ParticleSystem'Sentry_Bot.Effects.P_Damage'

	AimAttractors(0)=(OuterRadius=60.f,InnerRadius=2.f,BoneName="b_SB_Root")
}