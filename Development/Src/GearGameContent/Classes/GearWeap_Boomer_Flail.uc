
/**
 * Grenade flail used by the Shield Boomer
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_Boomer_Flail extends GearWeapon
	hidedropdown;

/** Constraints to prevent flail chain from extending beyond its length. */
var()	editinline export	RB_ConstraintSetup		ChainLengthConstraintSetup;
var()	editinline export	RB_ConstraintInstance   ChainLengthConstraintInstance;
var()	editinline export	RB_BodyInstance			ChainEndBodyInstance;

var float	ChainEndSpringPctTarget;
var float	ChainEndSpringPct;
var float	ChainEndSpringBlendTimeToGo;

/** Defines the explosion. */
var() instanced editinline GearExplosion	ExplosionTemplate;

/** Amount of damage done at the epicenter. Using this instead of Projectile.Damage so we can make it config. */
var() protected const config float ExploDamage;
/** Damage range.  Using this instead of Projectile.DamageRadius so we can make it config. */
var() protected const config float ExploDamageRadius;
/** Defines how damage falls off.  High numbers cause faster falloff, lower (closer to zero) cause slower falloff.  1 is linear. */
var() protected const config float ExploDamageFalloffExp;

/** radius at which people will be knocked down/ragdolled by the projectile's explosion **/
var() protected const config float	ExploKnockDownRadius;
/** radius at which people will cringe from the explosion **/
var() protected const config float	ExploCringeRadius;


/**
 * Copy in any data we exposed here for .ini file access.
 */
simulated function PrepareExplosionTemplate()
{
	ExplosionTemplate.Damage = ExploDamage;
	ExplosionTemplate.DamageRadius = ExploDamageRadius;
	ExplosionTemplate.DamageFalloffExponent = ExploDamageFalloffExp;
	ExplosionTemplate.KnockDownRadius = ExploKnockDownRadius;
	ExplosionTemplate.CringeRadius = ExploCringeRadius;

	ExplosionTemplate.FractureMeshRadius = ExploKnockDownRadius;
}

simulated function CauseFlailExplosion(vector HitLocation, vector HitNormal)
{
	local vector NudgedHitLocation;
	local GearExplosionActor ExplosionActor;

	if (ExplosionTemplate != None)
	{
		// using a hitlocation slightly away from the impact point is nice for certain things
		NudgedHitLocation = HitLocation + (HitNormal * 4.f);

		ExplosionActor = Spawn(class'GearExplosionActor',,, NudgedHitLocation, rotator(HitNormal));
		PrepareExplosionTemplate();
		ExplosionTemplate.HitLocation = NudgedHitLocation;
		ExplosionTemplate.HitNormal = HitNormal;
		ExplosionActor.Explode(ExplosionTemplate);		// go bewm
	}
}


// hack to only allow fire in melee range
simulated function StartFire(byte FireModeNum)
{
	if( FireModeNum == MELEE_ATTACK_FIREMODE )
	{
		Super.StartFire( 0 );
	}
}

/** Base weapon refire rate on melee attack animation. */
simulated function TimeWeaponFiring( byte FireModeNum )
{
	local float				AnimTime;
	local GearPawn_Infantry	InfantryPawn;

	InfantryPawn = GearPawn_Infantry(Instigator);
	if( InfantryPawn != None )
	{
		AnimTime = 0.5f; // Set as length of attach animation + a little extra.

		//AnimTime = InfantryPawn.PlayFlailAttack();
		//`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "AnimTime:" @ AnimTime );

		// if weapon is not firing, then start timer. Firing state is responsible to stopping the timer.
		if( AnimTime > 0.f && !IsTimerActive('RefireCheckTimer') )
		{
			SetTimer( AnimTime, TRUE, nameof(RefireCheckTimer) );
			return;
		}
	}

	Super.TimeWeaponFiring(FireModeNum);
}


simulated function CustomFire()
{
	// Trigger attack effects.
	IncrementFlashCount();
}

simulated function PlayFireEffects(byte FireModeNum, optional vector HitLocation)
{
	local GearPawn_Infantry	InfantryPawn;

	Super.PlayFireEffects(FireModeNum, HitLocation);

	// Play weapon fire animation. melee attack overhand.
	PlayWeaponAnim(WeaponFireAnim, 1.f, 0.5f, 0.5f);

	// Play synchronized attack animation on pawn.
	InfantryPawn = GearPawn_Infantry(Instigator);
	if( InfantryPawn != None )
	{
		InfantryPawn.PlayFlailAttack(1.f, 0.5f, 0.5f);
	}

	if( ChainEndBodyInstance != None )
	{
		SetChainEndBoneSpringTarget(1.f, 0.15f);
		SetTimer( 0.45f, FALSE, nameof(TurnOffChainEndBoneSpring) );
	}
}

simulated function TurnOffChainEndBoneSpring()
{
	SetChainEndBoneSpringTarget(0.f, 0.1f);
}

simulated function PerformWeaponAttachment(SkeletalMeshComponent MeshCpnt, optional Name SocketName)
{
	local SkeletalMeshComponent	SkelMesh;
	local int					FlailEndIndex;
	local matrix				FlailEndMatrix;

	Super.PerformWeaponAttachment(MeshCpnt, SocketName);

	// If weapon has been successfully attached, sync both flail spin animation, with boomer spinning animation.
	AnimTreeRootNode.ForceGroupRelativePosition('FlailSpin', GearPawn(Instigator).AnimTreeRootNode.GetGroupRelativePosition('FlailSpin') );

	SkelMesh = SkeletalMeshComponent(Mesh);

	// Created physics asset instance.
	// Comment out to disable physics on flail.
	SkelMesh.SetHasPhysicsAssetInstance(TRUE);

	// Physics flail setup. Isolated for easy commenting out.
	if( SkelMesh.PhysicsAssetInstance != None )
	{
		// Setup max length constraint. To prevent chain from breaking at high velocity/torque
		ChainLengthConstraintInstance.InitConstraint(SkelMesh, SkelMesh, ChainLengthConstraintSetup, 1.f, Self, None, FALSE);

		// Add BoneSpring for FlailEnd body, so it tries to match the animation.
		SkelMesh.ForceSkelUpdate();
		FlailEndIndex	= SkelMesh.MatchRefBone('b_Flail_ChainEnd');
		FlailEndMatrix	= SkelMesh.GetBoneMatrix(FlailEndIndex);

		// Find the hip body instance
		ChainEndBodyInstance = SkelMesh.FindBodyInstanceNamed('b_Flail_ChainEnd');
		if( ChainEndBodyInstance != None )
		{
			ChainEndBodyInstance.EnableBoneSpring(TRUE, TRUE, FlailEndMatrix);
		}

		// Turn off bone spring by default.
		SetChainEndBoneSpringTarget(0.f, 0.f);
		UpdateChainEndBoneSpringValues();

		// Startup physics
		SkelMesh.PhysicsWeight = 1.f;
		SkelMesh.WakeRigidBody();
	}
}

simulated function Tick(float DeltaTime)
{
	local float Delta;

	Super.Tick(DeltaTime);

	if( ChainEndSpringPct != ChainEndSpringPctTarget )
	{
		Delta = ChainEndSpringPctTarget - ChainEndSpringPct;

		if( ChainEndSpringBlendTimeToGo > DeltaTime )
		{
			ChainEndSpringPct += (Delta / ChainEndSpringBlendTimeToGo) * DeltaTime;
			ChainEndSpringBlendTimeToGo -= DeltaTime;
		}
		else
		{
			ChainEndSpringPct = ChainEndSpringPctTarget;
			ChainEndSpringBlendTimeToGo	= 0.f;
		}

		UpdateChainEndBoneSpringValues();
	}
}

simulated function SetChainEndBoneSpringTarget(float NewTarget, float BlendTime)
{
	if( BlendTime > 0.f )
	{
		ChainEndSpringPctTarget = NewTarget;
		ChainEndSpringBlendTimeToGo = BlendTime;
	}
	else
	{
		ChainEndSpringPct = NewTarget;
		ChainEndSpringPctTarget = NewTarget;
		ChainEndSpringBlendTimeToGo = 0.f;
	}
}

simulated final function UpdateChainEndBoneSpringValues()
{
	if( ChainEndBodyInstance != None )
	{
		ChainEndBodyInstance.SetBoneSpringParams(ChainEndSpringPct, ChainEndSpringPct * 0.1f, ChainEndSpringPct, ChainEndSpringPct * 0.1f);
	}
}

function DropFrom(vector StartLocation, vector StartVelocity)
{
	// players aren't allowed to have this, so just destroy it
	Destroy();
}

defaultproperties
{
	bCanEquipWithShield=TRUE
	bWeaponCanBeReloaded=FALSE
	bIsSuppressive=TRUE
	bPreventTargeting=true

	WeaponFireTypes(0)=EWFT_Custom

	// Weapon anim set
	CustomAnimSets.Empty

	WeaponFireAnim="FL_MeleeAttack_Overhand"

	FireSound=None
	WeaponDeEquipSound=None
	WeaponEquipSound=None
	PickupSound=None
	WeaponDropSound=None
	MeleeImpactSound=None

	// Weapon Mesh Transform
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'Locust_Flail.Meshes.Locust_Flail'
		AnimTreeTemplate=AnimTree'Locust_Flail.Locust_Flail_AnimTree'
		PhysicsAsset=PhysicsAsset'Locust_Flail.Meshes.Locust_Flail_Physics'
		AnimSets(0)=AnimSet'Locust_Flail.Locust_Flail_NewChain_AnimSet'
		CollideActors=FALSE
		BlockRigidBody=TRUE
		RBChannel=RBCC_GameplayPhysics
		RBCollideWithChannels=(Default=TRUE,BlockingVolume=TRUE,EffectPhysics=TRUE)
	End Object
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	Begin Object Class=RB_ConstraintSetup Name=ChainLengthConstraintSetup0
		LinearXSetup=(bLimited=1,LimitSize=90)
		LinearYSetup=(bLimited=1,LimitSize=90)
		LinearZSetup=(bLimited=1,LimitSize=90)
		bLinearLimitSoft=FALSE
		bSwingLimited=FALSE
		ConstraintBone1="b_Flail_Chain01"
		ConstraintBone2="b_Flail_ChainEnd"
	End Object
	ChainLengthConstraintSetup=ChainLengthConstraintSetup0

	Begin Object Class=RB_ConstraintInstance Name=ChainLengthConstraintInstance0
		bSwingPositionDrive=TRUE
	End Object
	ChainLengthConstraintInstance=ChainLengthConstraintInstance0

	Begin Object Class=RB_BodyInstance Name=ChainEndBodyInstance0
	End Object
	ChainEndBodyInstance=ChainEndBodyInstance0

	FireOffset=(X=68,Y=0,Z=9)

	MagazineMesh=None
	MuzzleFlashLight=None
	MuzzleSocketName=None
	MuzFlashEmitter=None

	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=367,V=146,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=144,UL=128,VL=39)
	CrosshairIcon=None

	HUDDrawData			= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=257,UL=36,VL=8),ULPerAmmo=36)
	HUDDrawDataSuper	= (DisplayCount=1,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=257,UL=36,VL=8),ULPerAmmo=36)

	LC_EmisDefaultCOG=(R=0.5,G=3.0,B=20.0,A=1.0)
	LC_EmisDefaultLocust=(R=3.0,G=3.0,B=3.0,A=1.0)

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.600)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

	WeaponID=WC_Boomshot


	// explosion point light
    Begin Object Class=PointLightComponent Name=ExploLight0
		Radius=800.000000
		Brightness=1000.000000
		LightColor=(B=60,G=107,R=249,A=255)
		Translation=(X=16)
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bEnabled=FALSE
    End Object

	// explosion
	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_Boomer_Flail'
		MomentumTransferScale=1.f	// Scale momentum defined in DamageType

		ParticleEmitterTemplate=ParticleSystem'Locust_Flail.Effects.P_Locust_Flail_Impact_Explo_Base'
		ExplosionSound=SoundCue'Weapon_Grenade.Mauler.FBoomerGrenade_AttackImpactExploCue'
		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		bDoExploCameraAnimShake=TRUE
		ExploAnimShake=(Anim=CameraAnim'Effects_Camera.Explosions.CA_Locust_Flail_Explo')
		// ExploShakeRadius vars set in PrepareExplosionTemplate

		// FractureMeshRadius set in PrepareExplosionTemplate
		FracturePartVel=300.0

		bAllowPerMaterialFX=TRUE
		bParticleSystemIsBeingOverriddenDontUsePhysMatVersion=TRUE
	End Object
	ExplosionTemplate=ExploTemplate0
}
