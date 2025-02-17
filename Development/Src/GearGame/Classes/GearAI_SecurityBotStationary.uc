/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearAI_SecurityBotStationary extends GearAI
	native(AI);

/** GoW global macros */

var int PatrolArcDegreesLeft,PatrolArcDegreesRight;
var() float PatrolDelay;
var() float PatrolPeriod;
var() bool bEnabled;
var Rotator InitialRot;

var Rotator TempRot;

var Gearpawn_SecurityBotStationaryBase MyBotPawn;

var GearPawn GearEnemy;
var GearTeamInfo MyTeam;

cpptext
{
	virtual void PostScriptDestroyed();
};

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	AutoAcquireEnemy();

	bForceDesiredRotation=true;
	DesiredRotation=Rotation;

}

function CheckCombatTransition();
function bool CheckInterruptCombatTransitions();
function DoMeleeAttack( optional Pawn NewEnemy );

event Possess(Pawn NewPawn, bool bVehicleTransition)
{
	Super.Possess(NewPawn,bVehicleTransition);

	InitialRot = NewPawn.Rotation;
	//`log(GetFuncName()@NewPawn@self);
	MyBotPawn = Gearpawn_SecurityBotStationaryBase(NewPawn);
	if(MyBotPawn != none)
	{
		PatrolArcDegreesRight = MyBotPawn.PatrolArcDegreesRight;
		PatrolArcDegreesLeft = MyBotPawn.PatrolArcDegreesLeft;
		PatrolDelay = MyBotPawn.PatrolArcDelay;
		PatrolPeriod = MyBotPawn.PatrolArcPeriod;
		SetDefaultRotationRate();
		MyGearPawn = MyBotPawn;
		bEnabled = MyBotPawn.bEnabled;
		if(ROLE == ROLE_Authority)
		{
			if (PlayerReplicationInfo != none && PlayerReplicationInfo.Team != None)
			{
				PlayerReplicationInfo.Team.RemoveFromTeam(self);
			}
			MyTeam = GetNeutralTeam();
			MyTeam.AddToTeam(self);
			MyTeam.JoinSquad(name,self);
		}
	}
}
event Destroyed()
{
	local GearGame Game;
	//ScriptTrace();
	//`log(GetFuncName()@">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"@self@squad);

	// if we're the last person on the neutral team, clean it up
	if(ROLE == ROLE_Authority && MyTeam != none)
	{
		if(MyTeam.TeamMembers.length < 2)
		{
			//`log(self@"was last person on neutral team, cleaning up neutral team...");
			Game = GearGame(WorldInfo.Game);
			Game.Teams.RemoveItem(MyTeam);
			MyTeam.Destroy();
			MyTeam=none;
		}		
	}

	if( Squad != None )
	{
		Squad.UnregisterSquadMember(self);
	}
	Super.Destroyed();
}

function GearTeamInfo GetNeutralTeam()
{
	local GearGame Game;
	local int i;
	local gearteaminfo team;
	Game = GearGame(WorldInfo.Game);
	if (Game.Teams.Length < Game.NumTeams)
	{
		Game.InitializeTeams();
	}

	for(i=0;i<Game.Teams.length;i++)
	{
		//`log("----------------->["$i$"]"@Game.Teams[i]@" TeamIndex:"@Game.Teams[i].TeamIndex);
		if(Game.Teams[i].TeamIndex == 254)
		{
			team = Game.Teams[i];
			break;
		}
	}	

	if(team == none)
	{
		team = Spawn( class'GearTeamInfo', self );
		team.TeamIndex = 254;
		//`log("------------------------->>>> SPAWNING NEW TEAM FOR "@self@team);
		Game.Teams.AddItem(team);
	}

	return team;
}



function bool ShouldSaveForCheckpoint()
{
	return false;
}

function bool ShouldPrintStaleWarning()
{
	return (bEnabled && (MyBotPawn == none || !MyBotPawn.bLODFrozen));
}

function bool CanFireWeapon( Weapon Wpn, byte FireModeNum )
{
	// If AI doesn't want to fire - fail
	if( !bWeaponCanFire )
	{
		//debug
		`AILog( GetFuncName()@"Fail - AI doesn't want to fire", 'Weapon' );

		return FALSE;
	}
	// If pawn can't fire weapon - fail
	if( MyGearPawn != None && !MyGearPawn.CanFireWeapon() )
	{
		//debug
		`AILog( GetFuncName()@"Fail - pawn can't fire weapon", 'Weapon' );

		return FALSE;
	}
	// If active commands prevent firing - fail
	if( CommandList != None && !CommandList.IsAllowedToFireWeapon() )
	{
		//debug
		`AILog( GetFuncName()@"Fail - active commands prevent firing", 'Weapon' );

		return FALSE;
	}

	return TRUE;
}

function SetDefaultRotationRate()
{
	MyBotPawn.SetTurretRotationRate((PatrolArcDegreesLeft+PatrolArcDegreesRight)/PatrolPeriod);
}

function SetTurretRotation(rotator Rot)
{
	//`Log(GetFuncName()@Rot);
	MyBotPawn.SetDesiredTurretRot(Rot);
}

function SetEnabled(bool bInEnabled)
{
	local Gearpawn_SecurityBotStationaryBase BotPawn;
	BotPawn = Gearpawn_SecurityBotStationaryBase(Pawn);

	bEnabled=bInEnabled;
	BotPawn.bEnabled = bEnabled;

	if(bEnabled)
	{
		if(BotPawn.CanDeploy())
		{
			bEnabled=true;
			GotoState('Action_Idle');
		}
		else //if we can't deploy at the moment, try again in a second
		{
			BotPawn.FailedDeployment();
			bEnabled=false;
			BotPawn.bEnabled=false;
			SetTimer(RandRange(2.0,5.0),FALSE,nameof(TryDeploy));
		}
	}
	else
	{
		AbortCommand(GetActiveCommand());
		StopFiring();
		GotoState('Off');
	}
}

function TryDeploy()
{
	// try to enable ourselves again
	SetEnabled(TRUE);
}

function OnToggle(SeqAct_Toggle inAction)
{
	if(inAction.InputLinks[0].bHasImpulse && !bEnabled)
	{
		SetEnabled(true);
	}
	else if(inAction.InputLinks[1].bHasImpulse)
	{
		SetEnabled(false);
	}
	else if(inAction.InputLinks[2].bHasImpulse)
	{
		SetEnabled(!bEnabled);
	}

}

event SeePlayer( Pawn Seen )
{
	local Gearpawn GP;
	GP = Gearpawn(Seen);
	// ignore DBNO pawns that are invulnerable
	if(GP == none || !Gp.IsDBNO() || GP.ShouldTakeDamageWhenDBNO())
	{
		super.SeePlayer(Seen);
	}

	if(!MyBotPawn.bDisableAIEvade && GP != none && GP.MyGearAI != none && !IsFriendly(GP.MyGearAI))
	{
		GP.MyGearAI.EvadeTowardpath();
	}
}

function OnAISetTarget(SeqAct_AISetTarget InAction)
{
	Super.OnAISetTarget(InAction);
	if(bEnabled)
	{
		FireTarget = none;
		SetSquadName('Turret');
		class'AICmd_SecurityBotStationary_TrackAndFire'.static.InitCommand(self);
	}
}

function TargetAcquired(Actor InInstigator, AIReactChannel OrigChannel)
{
	Gearpawn_SecurityBotStationaryBase(MyGearPawn).TargetAcquired(InInstigator);
}

function bool SelectTarget()
{	
	local controller C;
	// If there are targets to consider
	if( TargetList.Length > 0 )
	{
		// Randomly select one
		// @todo - sort on distance
		FireTarget = TargetList[Rand(TargetList.Length)];

		// If target is a controller, set as pawn
		C = Controller(FireTarget);
		if( C != None )
		{
			FireTarget = C.Pawn;
		}

		return (FireTarget != None);
	}

	return false;
}
function bool SelectEnemy();


function vector GetMyTargetLocation()
{
	local vector FireTargLoc;
	local vector Offset;

	FireTargLoc = GetFireTargetLocation(LT_Exact);
	if(GearEnemy == none)
	{
		return FireTargLoc;
	}

	if( GearEnemy.IsInCover() && 
		GearEnemy.CoverAction > CA_Default &&
		GearEnemy.CoverAction < CA_PeekLeft )
	{
		// Try to see the gun
		Offset = (GearEnemy.GetPhysicalFireStartLoc( vect(0.f,0.f,0.f) )+vect(0,0,-30)) - GearEnemy.Location;
	}
	else
	{
		// Otherwise, look at their head location
		Offset = (GearEnemy.GetPawnViewLocation()+vect(0,0,-40)) - GearEnemy.Location;
		Offset.X = 0.f;
		Offset.Y = 0.f;
	}
	
	return FireTargLoc + Offset;
}

function int AngleSubtract(int Angle1, int Angle2)
{
	local int Dist;
	
	Dist = Angle1 - Angle2;
	if( Dist > 32768 )
	{
		Dist -= 65536;
	}
	else if( Dist < -32768)
	{
		Dist += 65536;
	}

	return Dist;
}
function Rotator GetRotation()
{
	local rotator res;
	local int Delta;

	// transform to actor space (what skelcontrol is expecting)
	//DrawDebugLine(GetTargetLocation(),MyBotPawn.GetTurretBoneLoc(),255,255,0);
	res = Rotator(Normal(GetMyTargetLocation() - MyBotPawn.GetTurretBoneLoc()) << MyBotPawn.Rotation);

	Delta = AngleSubtract(res.yaw,0);
	if( Delta > PatrolArcDegreesRight * 182)
	{
		res.yaw = PatrolArcDegreesRight * 182;
	}
	else if ( Delta < -(PatrolArcDegreesLeft * 182) )
	{
		res.yaw = -PatrolArcDegreesLeft * 182;
	}

	res.pitch = - res.pitch;
	return res;
}


event StartFiring(optional int InBurstsToFire=-1)
{
	Super.StartFiring(InBurstsToFire);
	if(MyBotPawn != none)
	{
		MyBotPawn.StartedFiring();
	}
}


function StopFiring()
{
	Super.StopFiring();
	if(MyBotPawn != none)
	{
		MyBotPawn.StoppedFiring();
	}
}

function bool IsFireLineClear()
{
	return true;
}

state Action_Idle
{
Begin:
	Gearpawn_SecurityBotStationaryBase(Pawn).TurnOn();
	SetSquadName( 'Turret' );
	ReactionManager.UnSuppressReactionsByType(class'AIReactCond_EnemyCloseToFireLine',false);

Loop:
	if(!bEnabled)
	{
		GotoState('Off');
	}

	//`AILog("Rotating CW" $ PatrolArcDegreesRIght $" deg");
	TempRot.Yaw =  PatrolArcDegreesRight * 182;
	TempRot.Pitch = -MyBotPawn.PatrolPitchoffset * 182.f;
	SetTurretRotation(TempRot);
	FinishRotation();
	//`AILog("FinishRotation finished..");
	Sleep(PatrolDelay);
	//`AILog("Rotating CCW" $ PatrolArcDegreesLeft $" deg");
	TempRot.Yaw =  -PatrolArcDegreesLeft * 182;
	TempRot.Pitch = -MyBotPawn.PatrolPitchoffset * 182.f;
	SetTurretRotation(TempRot);
	FinishRotation();
	//`AILog("FinishRotation finished..");
	Sleep(PatrolDelay);

	Goto('Loop');
}

state Off `DEBUGSTATE
{
Begin:
	Gearpawn_SecurityBotStationaryBase(Pawn).TurnOff();
	ReactionManager.SuppressReactionsByType(class'AIReactCond_EnemyCloseToFireLine',false);
	DesiredRotation = pawn.rotation;
	Stop;
};


DefaultProperties
{
	bEnabled=true
	PatrolPeriod=7
	PatrolDelay=1.f
	DefaultCommand=class'AICmd_Base_SecurityBotStationary'

	DefaultReactConditionClasses.Empty()
	DefaultReactConditions.Empty()

	Begin object class=AIReactCond_EnemyCloseToFireLine Name=CloseToFireLine
		OutputChannelName=GoGoFire
		DistFromFireLineThresh=240.f
		bSuppressed=true
	End Object
	DefaultReactConditions.Add(CloseToFireLine)

	Begin Object class=AIReactCond_GenericPushCommand Name=Attack0
		AutoSubscribeChannels(0)=GoGoFire
		CommandClass=class'AICmd_Base_SecurityBotStationary'
		MinTimeBetweenOutputsSeconds=-1
	End Object
	DefaultReactConditions.Add(Attack0)

	Begin Object class=AIReactCond_GenericCallDelegate Name=TargetAcquired0
		AutoSubscribeChannels(0)=GoGoFire
		OutputFunction=TargetAcquired
		MinTimeBetweenOutputsSeconds=5.0f
	End Object
	DefaultReactConditions.Add(TargetAcquired0)

	bIsPlayer=false
	RotationRate=(Pitch=8000)
}
