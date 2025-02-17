/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class Gearpawn_SecurityBotStationaryBase extends GearPawn
	abstract
	native(Pawn);

/** GoW global macros */

/** degree arc of the patrol rotation, ignored when patrolarcdegreesleft and patrolarcdegreesright is non negative */
var() int PatrolArcDegrees;
/** if this is non negative, will be treated as degrees left of forward for patrol arc  (PatrolArcDegrees is ignored when this is non neg)*/
var() int PatrolArcDegreesLeft;
/** if this is non negative, will be treated as degrees right of forward for patrol arc (PatrolArcDegrees is ignored when this is non neg)*/
var() int PatrolArcDegreesRight;
var() float PatrolArcDelay;
var() float PatrolArcPeriod;
var() float PatrolPitchoffset;
var() repnotify bool bEnabled;
var() bool bInvulnerable;
/** multiplier to be applied to all outbound damage */
var() float DamageMultiplier;
/** range of the laser/detection */
var() float DetectionRange;
/** Speed (in degrees per second) that the turret should be able to track its target once acquired */
var() int TargetTrackingSpeed;
/** time after acquiring an enemy before we start firing */
var() float StartFireDelay;
/** time after losing track of an enemy before we stop firing */
var() float StopFireDelay;
/** time after losing an enemy before we stop trying to track and go back to patrolling */
var() float ResetDelay;
/** DEGREES angle of cone describing the accuracy of this turret */
var() float AccuracyConeDegrees;
/** Bool to enable AI evading when targeted by this turret */
var() bool bDisableAIEvade;

var rotator DesiredTurretRot;
var repnotify float   TurretRotationSpeed;

/** cached ref to gun rotation control */
var SkelControlSingleBone GunRotControl;
var name BarrelRotationSkelControlName;
var name BarrelRotationBoneName;
var rotator BoneRotOffset;


/** Mommy, Where do bullets come from? */
var name PhysicalFireLocBoneName;

var repnotify bool bTrackingAnEnemy; // controls material swaps, etc..
var repnotify bool bFiring;
var repnotify bool bRotating;
/** whether or not we are fully deployed and ready to check for enemies */
var	bool bDeployed;


/** offset for second trace done in tick (to check for wretches and stuff below the beam we want to cheat with :) ) */
var() vector SecondPassOffset;
var() array<class<Actor> >		SecondTracePassValidClasses<AllowAbstract>;


var	transient vector LaserHitPoint;
/** when this is enabled the beam will stop when it hits geometry (this is more expensive and shouldn't be on unless neccesary) */
var() bool bConformLaserToGeometry;

/** enabled when we haven't been rendered in a long time */
var bool bLODFrozen;
var() float TimeNotRenderedUntilLODFreeze;

struct CheckpointRecord
{
	var bool bEnabled;
};

cpptext
{
	virtual UBOOL ReachedDesiredRotation();
	virtual UBOOL Tick( FLOAT DeltaSeconds, ELevelTick TickType );
	void GetBaseAimDir(FVector& out_AimDir);
	virtual void BeginDestroy();
	virtual UBOOL NeedsTick();
};

replication
{
	if (Role == ROLE_Authority && bNetDirty)
		bEnabled,DetectionRange,DesiredTurretRot,TurretRotationSpeed,bFiring,bTrackingAnEnemy,bRotating;
}


// sounds played via animnotify
simulated function ANIMNOTIFY_PlayShutDownSound1();
simulated function ANIMNOTIFY_PlayShutDownSound2();
simulated function ANIMNOTIFY_PlayShutDownSound3();
simulated function ANIMNOTIFY_PlayStartUpSound1();
simulated function ANIMNOTIFY_PlayStartUpSound2();
simulated function ANIMNOTIFY_PlayStartUpSound3();
simulated function ANIMNOTIFY_PlayStartUpSound4();




simulated function bool ShouldTurnCursorRedFor(GearPC PC)
{
	return !bInvulnerable;
}

function CreateCheckpointRecord(out CheckpointRecord Record)
{
	Record.bEnabled = bEnabled;
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
	local GearAI_SecurityBotStationary StatCont;
	StatCont = GearAI_SecurityBotStationary(Controller);
	if(StatCont != none)
	{
		StatCont.SetEnabled(Record.bEnabled);
	}
}

native function bool CanDeploy();

simulated function PostBeginPlay()
{
	
	// set up our patrol arc values
	// if one of the directional settings is set then set the other with the difference
	if(PatrolArcDegreesRight < 0)
	{
		if(PatrolArcDegreesLeft>=0)
		{
			PatrolArcDegreesRight = PatrolArcDegrees-PatrolArcDegreesLeft;
		}
		else
		{
			PatrolArcDegreesRight = PatrolArcDegrees/2.0f;
		}
	}

	if(PatrolArcDegreesLeft < 0)
	{
		if(PatrolArcDegreesRight>=0)
		{
			PatrolArcDegreesLeft = PatrolArcDegrees - PatrolArcDegreesRight;
		}
		else
		{
			PatrolArcDegreesLeft = PatrolArcDegrees/2.0f;
		}
	}
	//`log("Left"@PatrolArcDegreesLeft@"Right"@PatrolArcDegreesRight);

	Super.PostBeginPlay();
	SuperDamageMultiplier=DamageMultiplier;
	AddDefaultInventory();
	SetPhysics(PHYS_None);

	if(bEnabled)
	{
		TurnOn();
	}
	else
	{
		TurnOff();
	}
}

simulated function GetPissed();
simulated function GetHappy();
function FailedDeployment();

event NotifyDoneRotating()
{
	StoppedRotating();
}

simulated function StartedFiring()
{
	bFiring=true;
}

simulated function StoppedFiring(optional bool bFromRep)
{
	bFiring=false;
}

simulated function StartedRotating()
{
	bRotating=true;
}

simulated function StoppedRotating()
{
	bRotating=false;
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'bEnabled')
	{
		if(bEnabled)
		{
			TurnOn();
		}
		else
		{
			TurnOff();
		}
	}
	else if(VarName == 'TurretRotationSpeed')
	{
		SetTurretRotationRate(TurretRotationSpeed);
	}
	else if(VarName == 'bTrackingAnEnemy')
	{
		if(bTrackingAnEnemy)
		{
			GetPissed();
		}
		else
		{
			GetHappy();
		}
	}
	else if(VarName == 'bFiring')
	{
		if(bFiring)
		{
			StartedFiring();
		}
		else
		{
			StoppedFiring(TRUE);
		}
	}
	else if(VarName == nameof(bRotating))
	{
		if(bRotating)
		{
			StartedRotating();
		}
		else
		{
			StoppedRotating();
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}



simulated function SetDesiredTurretRot(rotator DesiredRot)
{
	StartedRotating();
	DesiredRot.Roll=0;
	DesiredTurretRot = DesiredRot;
}

simulated function SetTurretRotationRate(float DegsPerSec)
{
	TurretRotationSpeed = DegsPerSec;
}

simulated function CacheAnimNodes()
{
	Super.CacheAnimNodes();

	GunRotControl = SkelControlSingleBone(Mesh.FindSkelControl(BarrelRotationSkelControlName));
	if(GunRotControl != none)
	{
		GunRotControl.bApplyRotation = TRUE;
		GunRotControl.bAddRotation = FALSE;
		GunRotControl.BoneRotationSpace = BCS_ActorSpace;
	}
}

simulated function ClearAnimNodes()
{
	Super.ClearAnimNodes();
	GunRotControl=none;
}

function TargetAcquired(Actor Target);


function SpawnDefaultController()
{
	Super(Pawn).SpawnDefaultController();
}

simulated function InitSkelControl()
{
	GunRotControl.SetSkelControlActive(TRUE);
	GunRotControl.SetSkelControlStrength(1.0f,0.25f);
	GunRotControl.BoneRotation = BoneRotOffset;
}

simulated function TurnOn();
simulated function TurnOff()
{
	GunRotControl.SetSkelControlStrength(0.0f,0.25f);
}

simulated event Destroyed()
{
	super.Destroyed();
	if(Controller != none)
	{
		Controller.Destroy();
	}
}
simulated native event Vector GetPhysicalFireStartLoc( vector FireOffset );

simulated native event Vector GetPawnViewLocation();


reliable server function Knockdown(vector LinearVelocityImpulse, vector AngularVelocityImpulse);

/** Overidden to take more damage from grenades, and less damage otherwise */
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
	local class<GearDamageType> GDT;
	if(bInvulnerable)
	{
		GDT = class<GearDamageType>(DamageType);
		// play any associated fx/sounds
		DoDamageEffects(Damage, (InstigatedBy != None) ? InstigatedBy.Pawn : None, HitLocation, GDT, Momentum, HitInfo);

		// allow the damage type to add any extra fx/etc
		GDT.static.HandleDamagedPawn(self, (InstigatedBy != None) ? InstigatedBy.Pawn : None, Damage, Momentum);

		Damage = 0;

	}
	

	//`log(GetFuncName()@Damage@DamageType);
	if(ClassIsChildOf(DamageType,class'GDT_Explosive'))
	{
		Damage *= 2; // just to be sure ;p
	}
	else
	{
		Damage /= 3;
	}

	Super.TakeDamage(Damage,InstigatedBy,HitLocation,Momentum,DamageType,HitInfo,DamageCauser);
}

simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
 return FALSE;
}


function vector GetTurretBoneLoc()
{
	return Mesh.GetBoneLocation(BarrelRotationBoneName) + (vect(0,0,25.f) >> Rotation);
}

function StatsLocationUpdate();

function SetMovementPhysics();
DefaultProperties
{
	ControllerClass=class'GearAI_SecurityBotStationary'
	PeripheralVision=0.4f
	SightRadius=0
	SightBoneName=Dummy_Muzzle
	bAllowDamageModification=false
	bTranslateMeshByCollisionHeight=false

	PatrolArcDegrees=120
	PatrolArcDelay=2
	PatrolArcPeriod=8
	bEnabled=true
	bCanDBNO=false
	DetectionRange=770.0f
	DamageMultiplier=1.0f
	TargetTrackingSpeed=20.f // degrees per second
	BoneRotOffset=(Yaw=32768,Roll=16384) // compensate for misaligned bones in the skeleton
	AllowedYawError=500
	SecondPassOffset=(z=-96)

	SecondTracePassValidClasses(0)=class'GearPawn_LocustWretchBase'
	PatrolArcDegreesRight=-1
	PatrolArcDegreesLeft=-1

	Begin Object Class=SecurityBotStationaryRenderingComponent Name=ArcRenderer
		HiddenGame=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(ArcRenderer)

	StartFireDelay=0.25f
	StopFireDelay=5.0f
	ResetDelay=5.0f
	AccuracyConeDegrees=5.0f
	bIgnoreForces=true
	TimeNotRenderedUntilLODFreeze=10.0f
	bBlockCamera=false
}
