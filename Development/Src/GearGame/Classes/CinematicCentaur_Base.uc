/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CinematicCentaur_Base extends SkeletalMeshActor
	native(Vehicle)
	abstract;


var()	actor	AimAtActor;

var()	float	WheelRadius;

var()	float	SuspensionTravel;

var()	float	AngVelSteerFactor;

var()	float	MaxSteerAngle;

var		SkelControlWheel	WheelControls[4];

var		name				WheelBoneName[4];

var		vector				WheelPosition[4];

var		GearSkelCtrl_TurretConstrained	TurretControls[2];

var		MaterialInstanceConstant		TankMIC[2];

var()	SoundCue CentaurDeathSound;
var()	ParticleSystem	CentaurDeathEffectKeepTurret;
var()	ParticleSystem	CentaurDeathEffectLoseTurret;

/** If TRUE, turret is scaled out upon death, and CentaurDeathEffectLoseTurret is used. */
var()	bool	bLoseTurretOnDeath;

/** If the size of the derrick on screen is less than this, don't update wheels */
var()	float	MinDistFactorForUpdate;

var() SoundCue CentaurCannonSound;
var() ParticleSystem CentaurCannonMuzzzleEffect;
var() ParticleSystemComponent PSC_MuzzleEffect;

/** Array of names of animations to choose from upon death */
var()	array<name>	DeathAnimNames;

/** Animation node used to play firing anim etc. */
var	AnimNodeSequence	ActionNode;

var	repnotify	int		FireCount;

var	repnotify	int		DieCount;

replication
{
	if(Role == ROLE_Authority)
		FireCount, DieCount;
}

cpptext
{
	virtual void TickSpecial(FLOAT DeltaTime);
}

simulated function ReplicatedEvent(name VarName)
{
	if(VarName == 'FireCount')
	{
		OnToggle(None);
	}
	else if(VarName == 'DieCount')
	{
		OnCauseDamage(None);
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	PSC_MuzzleEffect = new(self) class'ParticleSystemComponent';
	SkeletalMeshComponent.AttachComponentToSocket(PSC_MuzzleEffect, 'GunnerFireSocket');
	PSC_MuzzleEffect.SetTemplate(CentaurCannonMuzzzleEffect);

	TankMIC[0] = SkeletalMeshComponent.CreateAndSetMaterialInstanceConstant(0);
	TankMIC[1] = SkeletalMeshComponent.CreateAndSetMaterialInstanceConstant(1);
}

simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	local int i, BoneIndex;

	WheelControls[0] = SkelControlWheel(SkeletalMeshComponent.FindSkelControl('Rt_Rear_Tire'));
	WheelControls[1] = SkelControlWheel(SkeletalMeshComponent.FindSkelControl('Lt_Rear_Tire'));
	WheelControls[2] = SkelControlWheel(SkeletalMeshComponent.FindSkelControl('Rt_Front_Tire'));
	WheelControls[3] = SkelControlWheel(SkeletalMeshComponent.FindSkelControl('Lt_Front_Tire'));

	for(i=0; i<4; i++)
	{
		BoneIndex = SkeletalMeshComponent.MatchRefBone(WheelBoneName[i]);
		WheelPosition[i] = (SkeletalMeshComponent.GetRefPosePosition(BoneIndex) + SkeletalMeshComponent.GetRefPosePosition(0));
	}

	TurretControls[0] = GearSkelCtrl_TurretConstrained(SkeletalMeshComponent.FindSkelControl('MainTurret_Pitch'));
	TurretControls[1] = GearSkelCtrl_TurretConstrained(SkeletalMeshComponent.FindSkelControl('MainTurret_Yaw'));

	// If not aiming at anything, let turret turn at any speed to match where it wants to point
	if(AimAtActor == None)
	{
		TurretControls[0].LagDegreesPerSecond = 0;
		TurretControls[1].LagDegreesPerSecond = 0;
	}

	ActionNode = AnimNodeSequence(SkeletalMeshComponent.FindAnimNode('ActionNode'));
}

// When toggled, play firing stuff
simulated function OnToggle(SeqAct_Toggle action)
{
	// Play firing sound
	PlaySound(CentaurCannonSound);

	// Trigger firing effect
	PSC_MuzzleEffect.ActivateSystem(FALSE);

	// Play firing anim
	ActionNode.SetAnim('Fire');
	ActionNode.PlayAnim();

	if(Role == ROLE_Authority)
	{
		FireCount++;
	}
}

// When you use 'cause damage', kill vehicle.
simulated function OnCauseDamage(SeqAct_CauseDamage Action)
{
	local Emitter ExplodeEmitter;
	local ParticleSystem DeathPSys;

	PlaySound(CentaurDeathSound);

	if(bLoseTurretOnDeath)
	{
		DeathPSys = CentaurDeathEffectLoseTurret;
		// Scale away turret if desired
		TurretControls[0].BoneScale = 0.001;
		TurretControls[1].BoneScale = 0.001;
	}
	else
	{
		DeathPSys = CentaurDeathEffectKeepTurret;
	}

	ExplodeEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(DeathPSys, Location, Rotation);
	ExplodeEmitter.ParticleSystemComponent.ActivateSystem();

	// choose random anim
	ActionNode.SetAnim( DeathAnimNames[Rand(DeathAnimNames.length)] );
	ActionNode.PlayAnim();

	// Set parameter upon death
	TankMIC[0].SetScalarParameterValue('Death', 1.0);
	TankMIC[1].SetScalarParameterValue('Death', 1.0);

	if(Role == ROLE_Authority)
	{
		DieCount++;
	}
}
