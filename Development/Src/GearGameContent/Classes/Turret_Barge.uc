class Turret_Barge extends Turret_TroikaCabal
	config(Pawn);

var() float NewDriverCylRad;
var transient vector BackedUpDriverCollisionSize;

simulated function DrivingStatusChanged()
{
	super.DrivingStatusChanged();

	if (bDriving)
	{
		// shrink radius of drivers cylinder
		// this is a hack to make him able to spin freely around the turret's fake collision
		BackedUpDriverCollisionSize = Driver.GetCollisionExtent();
		Driver.SetCollisionSize(NewDriverCylRad, BackedUpDriverCollisionSize.Z);
	}
	else
	{
		Driver.SetCollisionSize(BackedUpDriverCollisionSize.X, BackedUpDriverCollisionSize.Z);
	}
}

function bool PlaceExitingDriver(optional Pawn Who)
{
	local vector HitLocation, HitNormal, NewActorPos;
	local Actor HitActor;
	local vector TurretFwd, StartTrace, EndTrace, PawnExtent;
	local vector TraceUp;

	if (Who == None)
	{
		Who = Driver;
	}

	if (Who.Base == Base)
	{
		// try and place straight back
		PawnExtent = BackedUpDriverCollisionSize;

		TurretFwd = vector(Rotation);
		TurretFwd.Z = 0.f;
		TurretFwd = Normal(TurretFwd);

		TraceUp.Z = PawnExtent.Z*2.f;

		StartTrace = Location - (TurretFwd * (PawnExtent.X * 1.5f)) + TraceUp;
		EndTrace = StartTrace - TraceUp*2.f;

		HitActor = Trace(HitLocation, HitNormal, EndTrace, StartTrace, true, PawnExtent);
		if (HitActor != None)
		{
//			FlushPersistentDebugLines();
//			DrawDebugBox(HitLocation, vect(10,10,10), 255, 255, 0, TRUE);
//			DrawDebugBox(HitLocation, PawnExtent, 255, 0, 0, TRUE);

			NewActorPos = HitLocation + vect(0,0,2.f);

			// back to local pos
			NewActorPos = (NewActorPos - Base.Location) << Base.Rotation;

			// assuming based on derrick
			if (Who.SetRelativeLocation(NewActorPos))
			{
				return TRUE;
			}
		}
	}

	// fall back to default behavior
	return Super.PlaceExitingDriver(Who);
}

function bool DriverEnter(Pawn P)
{
	local vector ExitPos;
	local bool bRet;

	if (P.Base == Base)
	{
		// store relative to base
		//ExitPos = P.RelativeLocation + vect(0,0,16);
		ExitPos = (P.Location - Base.Location) << Base.Rotation;

	}
	else
	{
		ExitPos = P.Location + vect(0,0,16);
	}

	bRet = super.DriverEnter(P);

	`log("saved world pos"@P.Location@"saved local pos"@ExitPos);
	ExitPositions[0] = ExitPos;

	return bRet;
}

defaultproperties
{
	NewDriverCylRad=10

		ViewPitchMin=-5000
		ViewPitchMax=6500		// Pawn arm IK breaks down if more than this

		TurretTurnRateScale=0.66

		CameraViewOffsetHigh=(X=-300,Y=60,Z=25)
		CameraViewOffsetLow=(X=-250,Y=60,Z=80)
		CameraViewOffsetMid=(X=-300,Y=60,Z=60)
		CameraTargetingViewOffsetHigh=(X=-185,Y=0,Z=40)
		CameraTargetingViewOffsetLow=(X=-185,Y=0,Z=40)
		CameraTargetingViewOffsetMid=(X=-185,Y=0,Z=40)

		bEnforceHardAttach=FALSE

		bBlockActors=FALSE
		bCollideActors=TRUE

		// derrick sequence is tightly controlled, no need to have turret collide with world
		// can only cause problems (already did, in fact)
		bCollideWorld=FALSE
		bNoEncroachCheck=TRUE

		// use the cylinder for collision
		//CollisionComponent=CollisionCylinder

		Begin Object Name=SkelMeshComponent0
		PhysicsAsset=None
		CollideActors=FALSE
		BlockActors=FALSE
		End Object
}