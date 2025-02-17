/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_RockWorm extends GearAI
	native(AI);

cpptext
{
	virtual UBOOL ShouldIgnoreNavigationBlockingFor(const AActor* Other);
	virtual void ClearCrossLevelPaths(ULevel* Level);
	virtual void UpdatePawnRotation();
}

// don't care
function bool IsTooLongSinceEnemySeen( Pawn EnemyPawn );

function bool ShouldMoveToSquad( optional bool bSkipLastPositionCheck )
{
	return FALSE;
}

singular function bool StepAsideFor( Pawn ChkPawn );

function DoMeleeAttack( optional Pawn NewEnemy );

function bool SelectEnemy();
function SetEnemy( Pawn NewEnemy );

native function bool IsFriendly( Controller TestPlayer );

// no! (we set ourselves up with a magical neutral team in postbegin, so ignore setteam)
event SetTeam(int inTeamIdx);


simulated function PostBeginPlay()
{
	local gearteaminfo NewTeam;
	Super.PostBeginPlay();

	if(ROLE == ROLE_Authority)
	{
		if (PlayerReplicationInfo.Team != None)
		{
			PlayerReplicationInfo.Team.RemoveFromTeam(self);
		}
		NewTeam = Spawn( class'GearTeamInfo', self );
		NewTeam.TeamIndex = 254;
		NewTeam.AddToTeam(self);
	}
}

function bool ShouldBite( GearPawn OtherPawn )
{

	// don't chomp on AIs in the player squad
	if(OtherPawn != none && OtherPawn.MyGearAI != none && OtherPawn.MyGearAI.Squad != none && OtherPawn.MyGearAI.Squad.bPlayerSquad)
	{
		return false;
	}

	// if he's not in between me and my move target, don't worry about it
	if(MoveTarget != none  && PointDistToSegment(OtherPawn.Location,Pawn.Location,MoveTarget.Location) > OtherPawn.GetCollisionRadius())
	{
		return false;
	}

	return true;
}

function bool NotifyBump( Actor Other, vector HitNormal )
{
	local GearPawn GP;


	GP = GearPawn(Other);
	GearPawn_RockWormBase(MyGearPawn).ChompVictim = GP;
	if( (GetActiveCommand() == none || GetActiveCommand().Class != class'AICmd_Attack_RockWormMouthChomp') && GP != none && ShouldBite(GP))
	{
		class'AICmd_Attack_RockWormMouthChomp'.static.Chomp(self,GP);
	}
	else if(GP != none && GP.MyGearAI != none)
	{
		GP.MyGearAI.StepAsideFor(MyGearPawn);
	}

	return super.NotifyBump( Other, HitNormal );
}

event bool HandlePathObstruction( Actor BlockedBy )
{
	return TRUE;
}

function SetRouteMoveGoal( Route NewRouteMoveGoal, optional ERouteDirection NewRouteDirection, optional bool bInterruptable = TRUE )
{
	local RockWorm_FruitBase Fruity;

	//debug
	`AILog( "- setting route goal:"@NewRouteMoveGoal@NewRouteDirection @bInterruptable);

	Fruity = RockWorm_FruitBase(NewRouteMoveGoal.RouteList[NewRouteMoveGoal.RouteList.length-1].Actor);
	if(Fruity != none)
	{
		GearPawn_RockWormBase(MyGearPawn).MovingToFruit(Fruity);
	}

	class'AICmd_MoveToRoute_Rockworm'.static.MoveToRoute( self, NewRouteMoveGoal, NewRouteDirection, bInterruptable );
}

protected function AbortMovementCommands()
{
	Super.AbortMovementCommands();
	AbortCommand( None, class'AICmd_MoveToRoute_Rockworm'	);
}

function bool SetTether( Actor NewTetherActor, optional float NewTetherDistance, optional bool NewbDynamicTether, optional float NewTetherPersistDuration, optional bool bInteruptable, optional bool bIsValidCache )
{
	local RockWorm_FruitBase Fruity;

	// ensure we can reach our anchor
	if(Pawn.Anchor != none && !ActorReachable(Pawn.Anchor))
	{
		`AILog("Clearing anchor"$Pawn.Anchor@"because it wasn't reachable!");
		Pawn.SetAnchor(none);
	}

	Fruity = RockWorm_FruitBase(NewTetherActor);
	if(Fruity != none && ActiveRoute == none)
	{
		GearPawn_RockWormBase(MyGearPawn).MovingToFruit(Fruity);
		SetMoveGoal(Fruity,,bInteruptable,32.f,bIsValidCache);
		return true;
	}

	return Super.SetTether(NewTetherActor,NewTetherDistance,NewbDynamicTether,NewTetherPersistDuration,bInteruptable,bIsValidCache);

}

function ReachedMoveGoal()
{
	// routes take care of themselves
	if(MoveAction != none && !bMovingToRoute)
	{
		MoveAction.ReachedGoal(self);
	}

}

function bool ShouldAllowWalk()
{
	return false;
}

// we take care of our own team, so don't do anything for this...
function ApplyCheckpointTeamChanges(const out CheckpointRecord Record);

function ArrivedAtFruit()
{
	AbortMovementCommands();
	MyGearPawn.ZeroMovementVariables();
}

state Action_Idle
{
Begin:
	Pawn.Velocity = vect(0,0,0);
	Pawn.Acceleration = vect(0,0,0);
	SetDestinationPosition(Pawn.Location);
	Stop;
}

defaultproperties
{
	DefaultCommand=class'AICmd_Base_RockWorm'

	DefaultReactConditions.Empty
	DefaultReactConditionClasses.Empty
	bPreciseDestination=true
	RotationRate=(Yaw=8192)
	bCanRevive=FALSE
}

