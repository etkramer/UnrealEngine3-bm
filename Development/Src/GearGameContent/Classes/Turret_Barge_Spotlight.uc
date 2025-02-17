class Turret_Barge_Spotlight extends Turret_Spotlight
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
	local vector HitLocation, HitNormal, ExitWorldPos, Slice, NewActorPos;
	local float ColRad, ColHeight;
	local Actor HitActor;

	if (Who == None)
	{
		Who = Driver;
	}

	if (Who.Base == Base)
	{
		ColRad = BackedUpDriverCollisionSize.X;
		ColHeight = BackedUpDriverCollisionSize.Z;

		// convert back to world space, in case base moved while we were driving
		ExitWorldPos = Who.Base.Location + (ExitPositions[0] >> Who.Base.Rotation);

		Slice = ColRad * vect(1,1,0);
		Slice.Z = 2;

		HitActor = Trace(HitLocation, HitNormal, ExitWorldPos - ColHeight*vect(0,0,5), ExitWorldPos, true, Slice);
		if (HitActor != None)
		{
			NewActorPos = HitLocation + (ColHeight+Who.MaxStepHeight)*vect(0,0,1);

			// back to local pos
			NewActorPos = (NewActorPos - Base.Location) << Base.Rotation;

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
	CollisionComponent=CollisionCylinder

	Begin Object Name=SkelMeshComponent0
		PhysicsAsset=None
		CollideActors=FALSE
		BlockActors=FALSE
	End Object
}

