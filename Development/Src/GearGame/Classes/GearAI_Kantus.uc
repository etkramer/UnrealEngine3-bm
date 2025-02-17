/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAI_Kantus extends GearAI_Cover
	config(AI);

const ScreamThresholdRadius = 350.f;
const MinTimeBetweenReviveScreams = 5.f;
var config float MinTimeBetweenKnockDownScreams;

var float LastSuccessfulKnockdownTime;

/** knockdown range to search for targets**/
var()	config	float	KnockdownRange;
var()	config  float	KnockdownFOV;
var()	config	float	KnockdownDelay;
/** maximum distance to travel when roaming */
var()	config	float	MaxRoamDist;
/** Amount of damage to take before forcing an evade */
var()	config  float	DamageBeforeEvade;
/** once our health gets below this value spawn tickers! */
var()	config	float	TickerHealthThreshold;
var()	config	int		NumTickersToSpawn;
var()	config  float	MinionSpawnDelay;
var()	config	float	MinTimeBetweenMinionSpawns;

var		array<GearPawn> PawnsToKnockDown;

var		array<GearPawn>	SpawnedMinions;

function Possess(Pawn NewPawn, bool bVehicleTransition)
{
	local AIReactCond_DmgThreshold react;
	local AIReactCond_DmgThreshold minion_react;
	super.Possess(NewPawn,bVehicleTransition);

	react = new(self) class'AIReactCond_DmgThreshold';
	react.DamageThreshold = DamageBeforeEvade;
	react.OutputFunction = TriggerEvade;
	react.Initialize();


	if (!ClassIsChildOf(WorldInfo.GRI.GameClass,class'GearGameHorde_Base'))
	{
		`AIlog("Registering minion spawner reactions because we're not in horde...");
		minion_react = new(self) class'AIReactCond_DmgThreshold';
		minion_react.DamageThreshold = TickerHealthThreshold;
		minion_react.OutputFunction = StartMinionSpawnTimer;
		minion_react.MinTimeBetweenActivations = MinTimeBetweenMinionSpawns;
		minion_react.Initialize();
	}
	else
	{
		`AILog("No minions because we're in horde!");
	}

}

function DoEvade( ESpecialMove EvadeDir, optional bool bClearActionDamage )
{
	if(TimeSince(LastGrenadeTime) < 1.0f)
	{
		return;
	}

	if(!MyGearPawn.IsDoingASpecialMove())
	{
		// quit playing grenade anims
		MyGearPawn.BS_StopAll(0.1f);
	}

	super.DoEvade(EvadeDir,bClearActionDamage);
}

function TriggerEvade(Actor EventInstigator, AIReactChannel OrigChannel)
{
	DoEvade(GetBestEvadeDir(MyGearPawn.Location, Pawn(EventInstigator)), true);
}

function StartMinionSpawnTimer(Actor EventInstigator, AIReactChannel OrigChannel)
{
	SetTimer( MinionSpawnDelay,false,nameof(StartMinionSummoningSpecialMove) );
}

function StartMinionSummoningSpecialMove()
{
	if(MyGearPawn.IsDoingASpecialMove())
	{
		SetTimer(0.25f,FALSE,nameof(StartMinionSummoningSpecialMove));
	}
	else
	{
		LastGrenadeTime=WorldInfo.TimeSeconds;
		MyGearPawn.ServerDoSpecialMove(GSM_Kantus_SummoningScream,TRUE);
	}
}

function SummonMinions()
{
	local Goal_SpawnPoints SPGoal;
	local Vector SearchStartPt;
	local int i;
	local NavigationPoint OldAnchor;
	local NavigationPoint TempAnchor;

	local bool bOldPreparingMove;

	bOldPreparingMove = bPreparingMove;

	SearchStartPt = pawn.Location;
	SearchStartPt += Normal(SearchStartPt - Enemy.Location) * EnemyDistance_Medium;

	// need to path from enemy's location so that we spawn minions on a path network compatible with the enemy's location
	OldAnchor = Pawn.Anchor;
	TempAnchor = Enemy.Anchor;
	if(TempAnchor == none)
	{
		TempAnchor = class'NavigationPoint'.static.GetNearestNavToPoint(Enemy,Enemy.Location);
	}

	if(TempAnchor != none)
	{
		AssignAnchor(Pawn,TempAnchor);
		Pawn.bForceKeepAnchor=TRUE;
	}

	bSkipRouteCacheUpdates=TRUE;

	// move toward squad centroid, but be sure to move a bit each time
	class'Path_TowardPoint'.static.TowardPoint(Pawn,SearchStartPt);
	//Pawn.FlushPersistentDebugLines();
	//Pawn.DrawDebugCoordinateSystem(SearchStartPt,rot(0,0,0),100.f,TRUE);
	SPGoal = class'Goal_SpawnPoints'.static.FindSpawnPoints(Pawn,self,NumTickersToSpawn,768,512);
	FindPathToward( Enemy );
	AssignAnchor(Pawn,OldAnchor);
	Pawn.bForceKeepAnchor=false;
	bSkipRouteCacheUpdates=false;
	bPreparingMove=bOldPreparingMove;

	`AILog("Minion spawn point search finished.. found "@SPGoal.PickedSpawnPoints.length@"points.");
	for(i=0;i<SPGoal.PickedSpawnPoints.length;i++)
	{
		`AILog("Spawning minion at "@SPGoal.PickedSpawnPoints[i]);
		SpawnMinion(SPGoal.PickedSpawnPoints[i]);
	}
	SPGoal.ClearFoundSpawns();
	Pawn.ClearConstraints();

}

function SpawnMinion(NavigationPoint SpawnPoint)
{
	local class<Gearpawn> MinionClass;
	local GearPawn Minion;
	local GearPawn_LocustTickerBase Ticker;
	local GearAI MinionController;
	MinionClass = Gearpawn_LocustkantusBase(MyGearPawn).MinionClasses[Rand(Gearpawn_LocustkantusBase(MyGearPawn).MinionClasses.length-1)];
	Minion = Spawn(MinionClass,,,SpawnPoint.Location);
	Ticker = GearPawn_LocustTickerBase(Minion);
	//DrawDebugCoordinateSystem(SpawnPoint.Location,rot(0,0,0),100.f,TRUE);
	if( Minion != None )
	{
		if(Ticker != none)
		{
			Ticker.PlayKantusSpawnEffects();
		}

		SpawnedMinions.AddItem(Minion);
		MinionController = GearAI(Spawn(Minion.ControllerClass));
		MinionController.SetTeam(Pawn.GetTeamNum());
		MinionController.Possess(Minion, FALSE);
		MinionController.SetSquadName('Alpha');
		MinionController.BanzaiAttack();
	}


}

// kill off minions when we die

function PawnDied(Pawn inPawn)
{
	local int i;

	for(i=0;i<SpawnedMinions.length;i++)
	{
		if(SpawnedMinions[i] != none)
		{
			SpawnedMinions[i].Died(self,class'GDT_ScriptedGib',SpawnedMinions[i].Location);
		}
	}
	SpawnedMinions.length=0;

	Super.PawnDied(inPawn);
}

function TriggerAttackGUDS()
{
	// don't do attack event when throwing grenade, let telegraph lines play
	if (TimeSince(LastGrenadeTime) > 2.f)
	{
		super.TriggerAttackGUDS();
	}
}

function bool CanDoKnockdown()
{
	local GearPawn	FoundPawn;
	local vector RotDir;
	local vector MeToVictimNormal;

	if(TimeSince(LastSuccessfulKnockdownTime) < MinTimeBetweenKnockDownScreams)
	{
		return false;
	}

	//`log(self@GetFuncName()@KnockdownRange@KnockdownFOV);
	PawnsToKnockDown.length = 0;
	RotDir = vector(MyGearPawn.Rotation);
	foreach MyGearPawn.VisibleCollidingActors( class'GearPawn', FoundPawn, KnockdownRange, MyGearPawn.Location, TRUE )
	{
		//`log(self@GetFuncName()@"Potential victim:"@FoundPawn);
		if( !MyGearPawn.IsDBNO() && (MyGearPawn != FoundPawn) && !MyGearPawn.IsSameTeam(FoundPawn) && !FoundPawn.IsAHostage() && FoundPawn.bRespondToExplosions )
		{
			MeToVictimNormal = Normal(FoundPawn.Location - MyGearPawn.Location);

			// check to see if he's within our specified FOV
			if( ( MeToVictimNormal dot RotDir) >= KnockdownFOV )
			{
				//`log(self@GetFuncName()@"Found victim:"$FoundPawn);
				// Add Pawn we found on the server.
				PawnsToKnockDown.AddItem(FoundPawn);
			}
		}
	}

	return (PawnsToKnockDown.length > 0);

}

function GoReviveTeammate( GearPawn GP )
{
	if( bAllowCombatTransitions )
	{
		// pop any pause commands so we get instant reaction
		AbortCommand(none,class'AICmd_Pause');
		class'AICmd_Kantus_ReviveScream'.static.InitCommandUserActor(self,GP);
	}
}

function bool CheckInterruptCombatTransitions()
{
	local Pawn EnemyPawn;

	// If dead or reviving then don't bother
	if(  Pawn == None ||
		IsDead() ||
		(MyGearPawn != None && MyGearPawn.IsDBNO()) )
	{
		return FALSE;
	}

	// If not in combat then don't bother
	if( CommandList == None )
	{
		return FALSE;
	}

	if( HasEnemyWithinDistance(ScreamThresholdRadius, EnemyPawn, FALSE) )
	{
		ReactionManager.NudgeChannel(EnemyPawn,'Force');
	}

	return Super.CheckInterruptCombatTransitions();
}

// kantus always takes priority if he's in range
function float GetReviveRatingFor(GearPawn Other)
{
	local float Dist;

	Dist = Super.GetReviveRatingFor(Other);
	if(Dist < class'GSM_Kantus_ReviveScream'.default.ReviveSearchRadius)
	{
		return 1.f;
	}

	return Dist;
}

function SelectWeaponWrapper( Actor Inst, AIReactChannel OriginatingChannel )
{
	SelectWeapon();
}

function ForceThrowGrenade()
{
	local GearWeap_PistolBase Pistol;

	foreach Pawn.InvManager.InventoryActors( class'GearWeap_PistolBase', Pistol )
	{
		Pistol.ForceThrowGrenade( self );
		break;
	}
}

function ThrowingInkGrenade(float Delay)
{
	// reset knockdown timer so we don't knockdown close to when we threw a grenade
	LastSuccessfulKnockdownTime = WorldInfo.TimeSeconds;
	// cease fire!
	StopFiring();
	// pause for a little bit to make the grenade throw distinct
	`AILog("Waiting for "$Delay$".... ");
	DesiredRotation = rotator(Enemy.Location-pawn.location);
	SetFocalPoint(vect(0,0,0));
	Focus = Enemy;
	class'AICmd_Pause'.static.Pause(self,Delay);
}

function bool CanRevivePawn( GearPawn GP, optional bool bIgnoreDBNOCheck )
{
	if(  bCanRevive && MyGearPawn != None &&
		GP.bCanRecoverFromDBNO &&
		GP.IsDBNO() &&
		GP != Pawn &&
		Pawn.IsSameTeam(GP) &&
		!MyGearPawn.IsDBNO() &&
		!IsDead() &&
		!GP.IsAHostage() &&
		!MyGearPawn.IsAKidnapper())
	{
		return TRUE;
	}

	`if(`notdefined(FINAL_RELEASE))
		if( MyGearPawn != None && GP.IsHumanControlled() && GP.IsDBNO() && (!AllowedToMove() || GP.bCanRecoverFromDBNO) )
		{
			MessagePlayer( MyGearPawn@"Trying to revive"@GP@GP.bCanRecoverFromDBNO@"but not allowed..."@MoveIsInterruptable()@MoveAction@MoveAction.TetherDistance );
			`warn( MyGearPawn@"Can't revive player because not allowed to move"@GP@GP.bCanRecoverFromDBNO@"but not allowed..."@MoveIsInterruptable()@MoveAction@MoveAction.TetherDistance  );
			ScriptTrace();
		}
		`endif

			return FALSE;
}


defaultproperties
{
	DefaultCommand=class'AICmd_Base_Kantus'

	Begin Object Name=StoppingPowerTemplate
		ChanceToStumble=0.f
		StoppingPowerThresh=0.4f
	End Object

	// enemy close conduit to trigger the charge when an enemy is within range
	Begin Object Class=AIReactCond_Kantus_EnemyCloseEnoughToScream Name=ScreamThresh0
		AutoSubscribeChannels(0)=Sight
		AutoSubscribeChannels(1)=Hearing
		AutoSubscribeChannels(2)=Force
		DistanceThreshold=ScreamThresholdRadius
		OutputChannelName=GoGoScream
	End Object
	DefaultReactConditions.Add(ScreamThresh0)

	// generic pusher for scream command when an enemy is close
	Begin Object Class=AIReactCond_GenericPushCommand Name=TriggerKnockDownScream0
		AutoSubscribeChannels(0)=GoGoScream
		MinTimeBetweenOutputsSeconds=-1
		CommandClass=class'AICmd_Attack_KantusKnockDownScream'
	End Object
	DefaultReactConditions.Add(TriggerKnockDownScream0)


	// generic pusher for revive scream
	Begin Object Class=AIReactCond_GenericPushCommand Name=TriggerReviveScream0
		AutoSubscribeChannels(0)=FriendlyDBNO
		MinTimeBetweenOutputsSeconds=MinTimeBetweenReviveScreams
		CommandClass=class'AICmd_Kantus_ReviveScream'
	End Object
	DefaultReactConditions.Add(TriggerReviveScream0)


	// delegate call to trigger grenade throws more frequently
	Begin Object Class=AIReactCond_GenericCallDelegate Name=TriggerWeaponSwitch0
		AutoSubscribeChannels(0)=Timer
		TimerInterval=4.5f
		OutputFunction=SelectWeaponWrapper
	End Object
	DefaultReactConditions.Add(TriggerWeaponSwitch0)

	DefaultReactConditionClasses.Remove(class'AIReactCond_Targeted')
	Begin Object class=AIReactCond_Targeted Name=KantusTargetedReaction0
		ShooterRangeThreshold=9192.0
	End Object
	DefaultReactConditions.Add(KantusTargetedReaction0)

}
