/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_CarryCrate_Base extends GearPawn
	nativereplication
	native(Pawn)
	config(Pawn);

var		GearPawn	MarcusPawn;

var		float		MarcusStickX;
var		float		MarcusStickY;

var		vector		MarcusOldPos;

var		GearPawn	DomPawn;

var		float		DomStickX;
var		float		DomStickY;

var		vector		DomOldPos;


var()	vector		CarrierRelPos;
/** IK target, in crate component ref frame. */
var()	vector		HandleRelPos;

var()	config float	CrateFriction;
var()	config float	CrateYawFriction;

var		float		YawVelocity;
var		float		YawAcceleration;

var		repnotify float		RepVelocityX;
var		repnotify float		RepVelocityY;
var		repnotify float		RepVelocityZ;

var()	config float	CrateLinAcceleration;
var()	config float	CrateYawAcceleration;

var()	config float	CrateSteerDeadZone;


var()	config float	CrateMaxYawVel;

var()	config float	CrateCoopYawFactor;

var()	float		WalkAnimVelScale;

var()	float		CrateWalkRotFactor;
var()	float		CrateRollXTransFactor;
var()	float		CrateAmbientTransFreq;
var()	float		CrateAmbientTransMag;
var		float		AmbientTransVal;

var()	float		CrateXSpringStiffness;
var()	float		CrateXSpringDamping;
var()	float		CrateXSpringAccelFactor;
var()	float		CrateXSpringMaxDisplacement;
var		float		CrateXSpringVel;
var		float		OldForwardVel;

/** .ini editable ground speed. */
var		config float	MaxCrateSpeed;

var()		StaticMeshComponent	CrateMeshComponent;

struct CheckpointRecord
{
	var vector Location;
	var rotator Rotation;
};

replication
{
	if (bNetDirty)
		MarcusPawn, DomPawn, YawVelocity, RepVelocityX, RepVelocityY, RepVelocityZ;
}

cpptext
{
	virtual void CalcVelocity(FVector &AccelDir, FLOAT DeltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant);
	virtual void physicsRotation(FLOAT DeltaTime, FVector OldVelocity);
	virtual void TickSimulated( FLOAT DeltaSeconds );

	virtual INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
	virtual void PostNetReceive();
	virtual void PostNetReceiveLocation();

	virtual UBOOL IgnoreBlockingBy(const AActor* Other) const;
};

simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	GroundSpeed = MaxCrateSpeed;
}

simulated function ReplicatedEvent(name VarName)
{
	if(VarName == 'RepVelocityX' || VarName == 'RepVelocityY' || VarName == 'RepVelocityX')
	{
		Velocity.X = RepVelocityX;
		Velocity.Y = RepVelocityY;
		Velocity.Z = RepVelocityZ;
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

function SetPawnToCarryMode(GearPawn InPawn)
{
	InPawn.CarriedCrate = self;

	if(Role == ROLE_Authority)
	{
		InPawn.CarriedCrateStatusChanged();
	}
}

function RemovePawnFromCarryMode(GearPawn InPawn)
{
	InPawn.CarriedCrate = None;

	if(Role == ROLE_Authority)
	{
		InPawn.CarriedCrateStatusChanged();
	}
}

function BeginCarry(GearPawn InMarcusPawn, GearPawn InDomPawn)
{
	MarcusPawn = InMarcusPawn;
	SetPawnToCarryMode(MarcusPawn);

	DomPawn = InDomPawn;
	SetPawnToCarryMode(DomPawn);

	UpdatePawnsLocation(1.0);
}

function EndCarry()
{
	if(MarcusPawn != None)
	{
		RemovePawnFromCarryMode(MarcusPawn);
		MarcusPawn = None;
	}

	if(DomPawn != None)
	{
		RemovePawnFromCarryMode(DomPawn);
		DomPawn = None;
	}
}

function SetInputForPawn(GearPawn InPawn, FLOAT Forward, FLOAT Strafe)
{
	if(InPawn == MarcusPawn)
	{
		//`log("MARCUS"@Forward);
		MarcusStickX = Strafe;
		MarcusStickY = Forward;
	}
	else if(InPawn == DomPawn)
	{
		//`log("DOM"@Forward);
		DomStickX = Strafe;
		DomStickY = Forward;
	}
}

event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	// No damage to crate
}

simulated function UpdateCrateRotation(float DeltaSeconds)
{
	local float HeightDiff, ForwardVel, ForwardAcc;
	local rotator CrateRot;
	local vector CrateTrans, ForwardDir;

	// Look at phase of marcus walk cycle
	if(MarcusPawn != None)
	{
		HeightDiff += Cos(MarcusPawn.CrateWalkNode.CurrentTime * (PI/0.7));
	}

	// And Dom
	if(DomPawn != None)
	{
		HeightDiff += Cos(DomPawn.CrateWalkNode.CurrentTime * (PI/0.7));
	}

	// Take diff between their phases and rotate crate
	CrateRot = CrateMeshComponent.Rotation;
	CrateRot.Pitch = CrateWalkRotFactor * HeightDiff;
	// also translate a bit
	CrateTrans = default.CrateMeshComponent.Translation;
	//CrateTrans.Z += CrateWalkTransFactor * HeightDiff2;

	// X spring
	CrateTrans.X = CrateMeshComponent.Translation.X;

	ForwardDir = vect(1,0,0) >> Rotation;
	ForwardVel = Velocity Dot ForwardDir;
	ForwardAcc = (ForwardVel - OldForwardVel)/DeltaSeconds;
	OldForwardVel = ForwardVel;

	// Add force for external acceleration
	CrateXSpringVel += (ForwardAcc * CrateXSpringAccelFactor * DeltaSeconds);
	// stiffness force
	CrateXSpringVel += (-CrateTrans.X *  CrateXSpringStiffness);
	// damping force
	CrateXSpringVel += (-CrateXSpringVel * CrateXSpringDamping);

	// integrate to get new position
	CrateTrans.X += CrateXSpringVel * DeltaSeconds;

	// Clamp pos/vel
	if(CrateTrans.X < -CrateXSpringMaxDisplacement)
	{
		CrateTrans.X = -CrateXSpringMaxDisplacement;
		CrateXSpringVel = Max(0, CrateXSpringVel);
	}
	else if(CrateTrans.X > CrateXSpringMaxDisplacement)
	{
		CrateTrans.X = CrateXSpringMaxDisplacement;
		CrateXSpringVel = Min(0, CrateXSpringVel);
	}

	CrateRot.Roll = CrateRollXTransFactor * CrateTrans.X;

	// Random periodic Z
	AmbientTransVal += (DeltaSeconds * CrateAmbientTransFreq);
	while(AmbientTransVal > 2*PI)
	{
		AmbientTransVal -= (2*PI);
	}
	CrateTrans.Z += Cos(AmbientTransVal) * CrateAmbientTransMag * Max(Abs(ForwardVel), 10.0);

	CrateMeshComponent.SetRotation(CrateRot);
	CrateMeshComponent.SetTranslation(CrateTrans);
}

/** Move Marcus/Dom pawns to correct position relative to crate. */
simulated function UpdatePawnsLocation(float DeltaTime)
{
	local vector RelPos;
	local vector NewLocation;

	if(MarcusPawn != None)
	{
		if(MarcusPawn.Physics != PHYS_None)
		{
			MarcusPawn.SetPhysics(PHYS_None);
		}

		RelPos = CarrierRelPos;
		NewLocation = Location + (RelPos >> Rotation);

		// Use difference between old and new position to set vel
		if(MarcusOldPos != vect(0,0,0))
		{
			MarcusPawn.Velocity = (NewLocation - MarcusOldPos)/DeltaTime;
			MarcusPawn.Velocity.Z = 0.0;
		}
		MarcusOldPos = NewLocation;


		MarcusPawn.SetLocation(NewLocation);
		MarcusPawn.SetRotation(Rotation);

		RelPos = HandleRelPos;
		MarcusPawn.CrateIKLeftHand.EffectorLocation = TransformVector(CrateMeshComponent.LocalToWorld, RelPos);

		MarcusPawn.CrateWalkNode.Rate = VSize(MarcusPawn.Velocity) * WalkAnimVelScale;
	}

	if(DomPawn != None)
	{
		if(DomPawn.Physics != PHYS_None)
		{
			DomPawn.SetPhysics(PHYS_None);
		}

		RelPos = CarrierRelPos;
		RelPos.Y *= -1.0;
		NewLocation = Location + (RelPos >> Rotation);

		if(DomOldPos != vect(0,0,0))
		{
			DomPawn.Velocity = (NewLocation - DomOldPos)/DeltaTime;
			DomPawn.Velocity.Z = 0.0;
		}
		DomOldPos = NewLocation;

		DomPawn.SetLocation(NewLocation);
		DomPawn.SetRotation(Rotation);

		RelPos = HandleRelPos;
		RelPos.X *= -1.0;
		DomPawn.CrateIKRightHand.EffectorLocation = TransformVector(CrateMeshComponent.LocalToWorld, RelPos);

		DomPawn.CrateWalkNode.Rate = VSize(DomPawn.Velocity) * WalkAnimVelScale;
	}
}

simulated function Tick(float DeltaTime)
{
	local vector ForwardDir, MarcusLookDir, CrateDir;
	//local bool bMovingBackwards;
	local float MarcusHeading, CrateHeading, DeltaTargetHeading, OnCourseAmount;

	// On the server, update acceleration based on dom/marcus input
	if(Role == ROLE_Authority && (MarcusPawn != None) && (DomPawn != None))
	{
		ForwardDir = vect(1,0,0) >> Rotation;

		// CO-OP CONTROLS
		if(DomPawn.IsHumanControlled())
		{
			Acceleration = ForwardDir * CrateLinAcceleration * ((MarcusStickY + DomStickY) * 0.5);

			if (Abs(MarcusStickY - DomStickY) > 0.10)
			{
				YawAcceleration = -1.0 * CrateYawAcceleration * CrateCoopYawFactor * (MarcusStickY - DomStickY);
			}
			else
			{
				YawAcceleration = 0.0;
			}
		}
		// SP CONTROLS
		else
		{
			// Get look heading
			MarcusLookDir = vector(MarcusPawn.GetBaseAimRotation());
			MarcusHeading = GetHeadingAngle(MarcusLookDir);

			// Get crate heading
			CrateDir = vector(Rotation);
			CrateHeading = GetHeadingAngle(CrateDir);

			// Find diff between wanted and curret
			DeltaTargetHeading = FindDeltaAngle(MarcusHeading, CrateHeading);

			// Allow turn on the spot if not pushing forwards/backwards
			if(Abs(MarcusStickY) < 0.2)
			{
				YawAcceleration = CrateYawAcceleration * MarcusStickX;
			}
			// Accelerate towards desired, taking into account dead zone
			else if (DeltaTargetHeading > CrateSteerDeadZone)
			{
				YawAcceleration = (DeltaTargetHeading - CrateSteerDeadZone) * -CrateYawAcceleration * Abs(MarcusStickY);
			}
			else if (DeltaTargetHeading < -CrateSteerDeadZone)
			{
				YawAcceleration = (DeltaTargetHeading + CrateSteerDeadZone) * -CrateYawAcceleration * Abs(MarcusStickY);
			}
			else
			{
				YawAcceleration = 0.0;
			}

			// How on course we are (0.0 if facing the wrong way)
			if(Abs(DeltaTargetHeading) > 0.5*PI)
			{
				OnCourseAmount = 0.0;
			}
			else
			{
				OnCourseAmount = ((0.5*PI) - Abs(DeltaTargetHeading))/(0.5*PI);
			}

			// Only accelerate when we are pointing the right way
			Acceleration = OnCourseAmount * ForwardDir * CrateLinAcceleration * MarcusStickY;
		}
	}

	// Update crate mesh rotation to add some movement when walking
	UpdateCrateRotation(DeltaTime);

	// Update Marcus/Dom position relative to crate
	UpdatePawnsLocation(DeltaTime);
}

// Don't shoot the bomb, Dom!
simulated function bool ShouldTurnCursorRedFor(GearPC PC)
{
	return FALSE;
}

function CreateCheckpointRecord(out CheckpointRecord Record)
{
	Record.Location = Location;
	Record.Rotation = Rotation;
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
	SetLocation(Record.Location);
	SetRotation(Record.Rotation);
}

defaultproperties
{
	Begin Object Class=StaticMeshComponent Name=CrateComp
		LightEnvironment=MyLightEnvironment
	End Object
	Components.Add(CrateComp)
	CrateMeshComponent=CrateComp

	bNoEncroachCheck=true
	bNoDelete=true // mostly so checkpoint loading doesn't kill it

	bInvalidMeleeTarget=TRUE
}
