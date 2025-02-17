/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTVehicle_Cicada extends UTAirVehicle
	native(Vehicle)
	abstract;

cpptext
{
	virtual void TickSpecial(FLOAT DeltaTime);
	virtual UBOOL HasRelevantDriver();
}

var repnotify vector TurretFlashLocation;
var repnotify rotator TurretWeaponRotation;
var	repnotify byte TurretFlashCount;
var repnotify byte TurretFiringMode;


var bool bFreelanceStart;

var array<int> JetEffectIndices;

var ParticleSystem TurretBeamTemplate;

var UTSkelControl_JetThruster JetControl;

var name JetScalingParam;

replication
{
	if (bNetDirty)
		TurretFlashLocation;

	if (!IsSeatControllerReplicationViewer(1))
		TurretFlashCount, TurretFiringMode, TurretWeaponRotation;
}

simulated event RigidBodyCollision( PrimitiveComponent HitComponent, PrimitiveComponent OtherComponent,
					const out CollisionImpactData RigidCollisionData, int ContactIndex )
{
	Super.RigidBodyCollision(HitComponent, OtherComponent, RigidCollisionData, ContactIndex);
	if(!IsTimerActive('ResetTurningSpeed'))
	{
		SetTimer(0.7f,false,'ResetTurningSpeed');
		MaxSpeed = default.MaxSpeed/2.0;
		MaxAngularVelocity = default.MaxAngularVelocity/4.0;
		if( UTVehicleSimChopper(SimObj) != none)
			UTVehicleSimChopper(SimObj).bRecentlyHit = true;
		//UTVehicleSimChopper(SimObj).TurnDamping = 0.8;
	}
}

simulated function ResetTurningSpeed()
{ // this is safe since this only gets called above after checking SimObj is a chopper.
	MaxSpeed = default.MaxSpeed;
	MaxAngularVelocity = default.MaxAngularVelocity;
	if( UTVehicleSimChopper(SimObj) != none)
		UTVehicleSimChopper(SimObj).bRecentlyHit = false;

	//UTVehicleSimChopper(SimObj).TurnDamping = UTVehicleSimChopper(default.SimObj).TurnDamping;
}
// AI hint
function bool ImportantVehicle()
{
	return !bFreelanceStart;
}

function bool DriverEnter(Pawn P)
{
	local UTBot B;

	if ( !Super.DriverEnter(P) )
		return false;

	B = UTBot(Controller);
	bFreelanceStart = (B != None && B.Squad != None && B.Squad.bFreelance);
	return true;
}

function DriverLeft()
{
	Super.DriverLeft();

	SetDriving(NumPassengers() > 0);
}

function PassengerLeave(int SeatIndex)
{
	Super.PassengerLeave(SeatIndex);

	SetDriving(NumPassengers() > 0);
}

function bool PassengerEnter(Pawn P, int SeatIndex)
{
	if ( !Super.PassengerEnter(P, SeatIndex) )
		return false;

	SetDriving(true);
	return true;
}

//Switching seats during altfire doesn't replicate because
//bDriving is the same at the end of the tick as the beginning.  Tell the client
//to stop firing the weapon it left behind
simulated function SitDriver( UTPawn UTP, int SeatIndex)
{
	if (Role<ROLE_Authority && SeatIndex == 1)
	{
		Seats[0].Gun.ForceEndFire();
	}

	Super.SitDriver(UTP, SeatIndex);
}


/* FIXME:
function Vehicle FindEntryVehicle(Pawn P)
{
	local Bot B, S;

	B = Bot(P.Controller);
	if ( (B == None) || !IsVehicleEmpty() || (WeaponPawns[0].Driver != None) )
		return Super.FindEntryVehicle(P);

	for ( S=B.Squad.SquadMembers; S!=None; S=S.NextSquadMember )
	{
		if ( (S != B) && (S.RouteGoal == self) && S.InLatentExecution(S.LATENT_MOVETOWARD)
			&& ((S.MoveTarget == self) || (Pawn(S.MoveTarget) == None)) )
			return WeaponPawns[0];
	}
	return Super.FindEntryVehicle(P);
}
*/

function bool RecommendLongRangedAttack()
{
	return true;
}

/* FIXME:
function float RangedAttackTime()
{
	local ONSDualACSideGun G;

	G = ONSDualACSideGun(Weapons[0]);
	if ( G.LoadedShotCount > 0 )
		return (0.05 + (G.MaxShotCount - G.LoadedShotCount) * G.FireInterval);
	return 1;
}
*/

simulated function VehicleWeaponImpactEffects(vector HitLocation, int SeatIndex)
{
	local ParticleSystemComponent E;

	Super.VehicleWeaponImpactEffects(HitLocation, SeatIndex);

	if (SeatIndex == 1 && !IsZero(HitLocation))
	{
		E = WorldInfo.MyEmitterPool.SpawnEmitter(TurretBeamTemplate, GetEffectLocation(SeatIndex));
		E.SetVectorParameter('ShockBeamEnd', HitLocation);
	}
}

/**
 * We override GetCameraStart for the Belly Turret so that it just uses the Socket Location
 */
simulated function vector GetCameraStart(int SeatIndex)
{
	local vector CamStart;

	if (SeatIndex == 1 && Seats[SeatIndex].CameraTag != '')
	{
		if (Mesh.GetSocketWorldLocationAndRotation(Seats[SeatIndex].CameraTag, CamStart) )
		{
			return CamStart;
		}
	}

	return Super.GetCameraStart(SeatIndex);
}

/**
 * We override VehicleCalcCamera for the Belly Turret so that it just uses the Socket Location
 */
simulated function VehicleCalcCamera(float DeltaTime, int SeatIndex, out vector out_CamLoc, out rotator out_CamRot, out vector CamStart, optional bool bPivotOnly)
{
	if (SeatIndex == 1)
	{
		out_CamLoc = GetCameraStart(SeatIndex);
		out_CamRot = Seats[SeatIndex].SeatPawn.GetViewRotation();
	CamStart = out_CamLoc;
	}
	else
	{
		Super.VehicleCalcCamera(DeltaTime, SeatIndex, out_CamLoc, out_CamRot, CamStart, bPivotOnly);
	}
}

simulated function bool ShouldClamp()
{
	return false;
}

defaultproperties
{
	Begin Object Class=UTVehicleSimChopper Name=SimObject
		MaxThrustForce=700.0
		MaxReverseForce=700.0
		LongDamping=0.6
		MaxStrafeForce=680.0
		LatDamping=0.7
		MaxRiseForce=1000.0
		UpDamping=0.7
		TurnTorqueFactor=7000.0
		TurnTorqueMax=10000.0
		TurnDamping=1.2
		MaxYawRate=1.8
		PitchTorqueFactor=450.0
		PitchTorqueMax=60.0
		PitchDamping=0.3
		RollTorqueTurnFactor=700.0
		RollTorqueStrafeFactor=100.0
		RollTorqueMax=300.0
		RollDamping=0.1
		MaxRandForce=30.0
		RandForceInterval=0.5
		StopThreshold=100
		bShouldCutThrustMaxOnImpact=true
	End Object
	SimObj=SimObject
	Components.Add(SimObject)

	COMOffset=(X=-40,Z=-50.0)

	BaseEyeheight=30
	Eyeheight=30
	bRotateCameraUnderVehicle=false
	CameraLag=0.05
	LookForwardDist=290.0
	bLimitCameraZLookingUp=true

	AirSpeed=2000.0
	GroundSpeed=1600.0

	UprightLiftStrength=30.0
	UprightTorqueStrength=30.0

	bStayUpright=true
	StayUprightRollResistAngle=5.0
	StayUprightPitchResistAngle=5.0
	StayUprightStiffness=1200
	StayUprightDamping=20

	SpawnRadius=180.0
	RespawnTime=45.0

	PushForce=50000.0
	HUDExtent=140.0

	HornIndex=0
}
