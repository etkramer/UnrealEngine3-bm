/** this volume automatically crouches console players as there's no manual crouch on the console controls  
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTBot_Minion extends AIController
	native(AI);

var byte TeamNum;

var UTTeamAI MyTeamAI;

var UTMinionSquadAI MySquad;

var Actor DesiredGoal;

simulated native function byte GetTeamNum();

function Initialize(byte NewTeamNum, bool bDefender)
{
	local MaterialInterface TeamMaterialHead, TeamMaterialBody;
	local int ObjectiveIndex;

	`log(self$" on "$NewTeamNum@bdefender);
	TeamNum = NewTeamNum;

	// FIXME - fast version of this
	StartSpot = WorldInfo.Game.FindPlayerStart(self, TeamNum);

	Pawn = Spawn(class'UTMinion',,,StartSpot.Location,StartSpot.Rotation);
	ObjectiveIndex = bDefender? 1-TeamNum : int(TeamNum);
	MyTeamAI = UTTeamGame(WorldInfo.Game).Teams[ObjectiveIndex].AI;

	if (Pawn == None )
	{
		SetTimer(1.0, false, 'Initialize');
	}
	else
	{
		Pawn.PossessedBy(self, false);
		if (WorldInfo.NetMode != NM_DedicatedServer)
		{
			UTMinion(Pawn).CurrCharClassInfo.static.GetTeamMaterials(TeamNum, TeamMaterialHead, TeamMaterialBody);
		}
		UTMinion(Pawn).SetCharacterMeshInfo(Pawn.Mesh.SkeletalMesh, TeamMaterialHead, TeamMaterialBody);
		UTMinion(Pawn).RestartMinion(StartSpot);
	}
}

function FindNewEnemy()
{
	local Controller C;

	if ( (Enemy != None) && (Enemy.Health > 0) && LineOfSightTo(Enemy) )
	{
		return;
	}

	Enemy = None;

	ForEach WorldInfo.AllControllers(class'Controller', C)
	{
		If ( (C.Pawn != None) && !Worldinfo.GRI.OnSameTeam(self, C) && LineOfSightTo(C.Pawn) )
		{
			Enemy = C.Pawn;
			break;
		}
	}
}

function FireProjectile()
{
	local projectile SpawnedProjectile;

	SpawnedProjectile = Pawn.Spawn(class'UTProj_LinkPowerPlasma', Pawn);
	if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
	{
		SpawnedProjectile.Init( Normal(Enemy.Location - Pawn.Location) );
	}
}

function bool UpdateMoveTarget()
{
	local int i;

	if ( (MyTeamAI != None) && (MyTeamAI.AttackSquad != None) )
	{
		DesiredGoal = MyTeamAI.AttackSquad.SquadObjective;
		if ( (DesiredGoal != None) && (VSize(Pawn.Location - DesiredGoal.Location) < 1500) 
			&& LineOfSightTo(DesiredGoal) && Pawn.ValidAnchor() )
		{
			// pick random path from anchor that doesn't have jumping/swimming
			MoveTarget = Pawn.Anchor.PathList[0].GetEnd();
			for ( i=1; i<Pawn.Anchor.PathList.Length; i++ )
			{
				if ( (FRand() < 0.3) && Pawn.Anchor.PathList[i].GetEnd() != None )
				{
					MoveTarget = Pawn.Anchor.PathList[i].GetEnd();
					return true;
				}
			}
			return (MoveTarget != None);
		}
	}
	if ( (DesiredGoal != None) && Pawn.ReachedDestination(DesiredGoal) )
	{
		DesiredGoal = None;
		RouteGoal = None;
	}
	if ( DesiredGoal == None )
	{
		if ( RouteGoal != None )
		{
			DesiredGoal = RouteGoal;
		}
		else
		{
			DesiredGoal = FindRandomDest();
		}
	}

	if ( (DesiredGoal == RouteGoal) && (RouteCache.Length > 0) )
	{
		for ( i=0; i<RouteCache.Length-1; i++ )
		{
			if ( RouteCache[i] == MoveTarget )
			{
				MoveTarget = RouteCache[i+1];
				return true;
			}
		}
	}
	MoveTarget = FindPathToward(DesiredGoal);
	return true;
}

Auto State MoveTowardTarget
{
Begin:
	if ( (Pawn == None) || (Pawn.Physics == PHYS_RigidBody) )
	{
		sleep(1.0);
		Goto('Begin');
	}
	if ( UpdateMoveTarget() )
	{
		if ( (MoveTarget == None) || Pawn.ReachedDestination(MoveTarget) )
		{
			DesiredGoal = None;
			RouteGoal = None;
			Pawn.Died(self, class'DmgType_Suicided', Pawn.Location);
		}
		else
		{
			MoveToward(MoveTarget);
		}
	}

FindEnemy:
	if ( (Pawn == None) || (Pawn.Physics == PHYS_RigidBody) )
	{
		sleep(1.0);
		Goto('Begin');
	}
	Pawn.Acceleration = vect(0,0,0);
	FindNewEnemy();
	if ( Enemy != None )
	{
		Focus = Enemy;
		FinishRotation();
		FireProjectile();
		Sleep(0.5 + 0.5*FRand());
		if ( FRand() < 0.7 )
			Goto('FindEnemy');
	}
	Goto('Begin');
}


defaultproperties
{
	bIsPlayer=false
	bCanDoSpecial=false
	bStasis=false
	SightCounterInterval=1000000.0;

	RotationRate=(Pitch=30000,Yaw=30000,Roll=2048)
	RemoteRole=ROLE_None

}

