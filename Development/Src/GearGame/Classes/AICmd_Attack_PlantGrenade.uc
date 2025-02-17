class AICmd_Attack_PlantGrenade extends AICommand
	native;

/** min and max distance from current location to consider planting the grenade */
var int MinDistance, MaxDistance;

/** list of traces to find the wall to plant the grenade on after reaching the desired NavigationPoint */
var array<vector> WallTraceDirs;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool PlantGrenade(GearAI AI, optional int InMinDistance, optional int InMaxDistance)
{
	local AICmd_Attack_PlantGrenade Cmd;

	if (AI.Pawn.FindInventoryType(class'GearWeap_GrenadeBase', true) != None)
	{
		Cmd = new(AI) default.Class;
		Cmd.MinDistance = InMinDistance;
		Cmd.MaxDistance = InMaxDistance;
		AI.PushCommand(Cmd);
		return true;
	}
	else
	{
		return false;
	}
}

function Popped()
{
	Super.Popped();

	StopMeleeAttack();
	EndOfMeleeAttackNotification();

	// reinit FireTarget since we cleared it to tag the wall
	SelectTarget();
	// and choose best weapon
	SelectWeapon();
}

/** @return NavigationPoint in the best area to plant a grenade within our distance constraints */
native final function NavigationPoint GetBestPlantPoint();

/** traces to nearby walls to find the final spot to place the grenade and sets DestinationPosition to that location */
final function SetFinalPlantLocation()
{
	local int i;
	local float Dist, BestDist;
	local vector BestLoc, HitLocation, HitNormal;

	BestDist = 100000.0;
	for (i = 0; i < WallTraceDirs.length; i++)
	{
		if (Trace(HitLocation, HitNormal, Pawn.Location + WallTraceDirs[i], Pawn.Location, false, vect(1,1,1)) != None)
		{
			Dist = VSize(Pawn.Location - HitLocation);
			if (Dist < BestDist)
			{
				BestLoc = HitLocation - (Normal(WallTraceDirs[i]) * Pawn.CylinderComponent.CollisionRadius);
				BestDist = Dist;
				SetFocalPoint(BestLoc + WallTraceDirs[i], false);
			}
		}
	}

	if (IsZero(BestLoc))
	{
		`AILog("Failed to find valid plant location from" @ Pawn.Location);
		Status = 'Failure';
		PopCommand(self);
	}
	else
	{
		FindSpot(Pawn.GetCollisionExtent(), BestLoc);
		SetDestinationPosition(BestLoc, false);
	}
}

function bool ShouldSelectTarget()
{
	return (Status != 'None'); // keep AI looking at plant location
}

auto state DoPlant
{
Begin:
	StopFiring();
	FireTarget = None;
	// switch to grenades
	GearInventoryManager(Pawn.InvManager).SetWeaponFromSlot(EASlot_Belt);
	// figure out where to go
	SetMoveGoal(GetBestPlantPoint());
	//@note: lack of failure check is intentional here - if we couldn't get there, just plant grenades as close as we got

	// find the wall to plant on
	SetFinalPlantLocation();
	MoveTo(GetDestinationPosition());

	// if we still don't have grenades out, block here until we do
	GearInventoryManager(Pawn.InvManager).SetWeaponFromSlot(EASlot_Belt);
	while (IsSwitchingWeapons())
	{
		Sleep(0.25);
	}

	if (MyGearPawn != None && GearWeap_GrenadeBase(MyGearPawn.MyGearWeapon) != None)
	{
		// do it
		MyGearPawn.MyGearWeapon.bHitWallThisAttack = false;
		MyGearPawn.MyGearWeapon.NumMeleeAttackAttempts = 0;
		bAllowedToFireWeapon = true;
		StartMeleeAttack();
		// wait for anim before finishing
		Sleep(1.0);
		Status = 'Success';
		PopCommand(self);
	}
	else
	{
		`AILog("Failed to switch to grenades");
		Status = 'Failure';
		PopCommand(self);
	}
}

defaultproperties
{
	bAllowedToFireWeapon=false

	WallTraceDirs[0]=(X=1000.0)
	WallTraceDirs[1]=(Y=1000.0)
	WallTraceDirs[2]=(X=-1000.0)
	WallTraceDirs[3]=(Y=-1000.0)
}
