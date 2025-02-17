/**
 * GearGameAnnex_Base
 * Gear Game Info for Annex Gametype
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameAnnex_Base extends GearGameMP_Base
	abstract
	native
	config(Game);

/// TIMER VARIABLES ///
/** Amount of time between Award calculations */
var float AwardUpdateTime;

/// COMMAND POINT VARIABLES ///
/** List of super-weapon pickup factories */
var transient array<GearWeaponPickupFactory> FactoryList;
/** List of pickup locations to avoid selecting first */
var transient array<GearWeaponPickupFactory> AvoidFirstList;
/** List of used factories */
var transient array<GearWeaponPickupFactory>	UsedList;
/** Current Command Point */
var transient GearWeaponPickupFactory		CommandPoint;
/** Particle effect at the CP location */
var transient SpawnedGearEmitter			CommandPointEffect;
/** Class of the CommandPointEffect */
var class<SpawnedGearEmitter>				CommandPointEffectClass;
/** Currently controlling team */
var transient GearTeamInfo					CommandTeam;
/** Command point control meter (100 == Max) */
var transient float							CommandMeter;
/** Max value for command meter (When CommandMeter is maxed == total control and team gets points) */
var float									CommandMeterMax;
/** Max distance player can be from CP get points for team */
var float									MaxPointDist, MaxPointDistZ;
/** Team that currently has the lead */
var transient GearTeamInfo					LeadingTeam;
// Team index that last captured the CP
var transient int							LastCapturedByTeamIndex;

/// SCORING/OTHER VARIABLES ///
/** Number of points given to team each max controlled tick - depletes CP resources */
var() config int	NumAwardPoints;
/** Amount of resource points left in the CP */
var() config int	CommandPointResources;
/** Total resource points needed to win round */
var() config int	ResourceGoal;


/// SOUND VARIABLES ///
/** Played when CP changes location */
var SoundCue		AnnexSound_CommandPointChanged;
/** Played when a new team starts to gain control */
var SoundCue		AnnexSound_CommandTeamChanged[2];
/** Played when CT control starts giving them points */
var SoundCue		AnnexSound_CommandMeterMaxed[2];
/** Played when max control has been challenged by other team */
var SoundCue		AnnexSound_CommandMeterBroken[2];
/** Played when one team takes the lead */
var SoundCue		AnnexSound_TeamLeadChanged[2];

/** Number of points a player will receive for every tick of CP aquisition when in the CP area */
var() config int	PointsPerCPTick;
/** Number of points a player will receive if in the CP area when it's capped */
var() config int	PointsPerCap;
/** Number of points a player will receive if in the CP area when a cap is broke */
var() config int	PointsPerBreak;
/** Number of points that will be distributed to players in a KOTH ring when a point is earned */
var() config int	TeamPointsPerKOTHTick;

/** List of players to join the game while in progress */
var array<PlayerController>	JoinInProgressList;

struct native AnnexMapInfo
{
	/** Name of map this item gives info for */
	var() String		MapName;
	/** List of pickup classes to use for the map */
	var() array<Name>	PickupList;
	/** List of pickup classes to avoid first */
	var() array<Name>	AvoidFirstList;
};
var() config array<AnnexMapInfo>	InfoList;

/** Are we currently in king of the hill mode? (One ring, execution forced) */
var bool bKingOfTheHillMode;

/** struct to keep track of KOTH player scoring for sitting in the circle */
struct native KOTHPlayerPointInfo
{
	/** The PRI of the player */
	var GearPRI PlayerPRI;
	/** Points earned so far - will always be less than 1 since we truncate the points to the PRI */
	var float PointsEarned;
};
/** Keeps track of the points a player has earned for sitting in the ring */
var array<KOTHPlayerPointInfo> KOTHPlayerPoints;

/**
 * Set the game rules via the online game settings
 * @return - whether the game was able to set the rules via the gamesettings object
 */
function bool SetGameRules()
{
	local int IntValue;
	local GearVersusGameSettings MyGameSettings;

	MyGameSettings = GetGameSettingsObject();

	if ( MyGameSettings != None )
	{
		// See if this is KOTH mode
		if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_VERSUSMODES, IntValue) )
		{
			if ( IntValue == eGEARMP_KOTH )
			{
				SetKOTHRules();
			}
		}

		// Resource Goal
		if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_ROUNDSCORE, IntValue) )
		{
			if ( bKingOfTheHillMode )
			{
				ResourceGoal = 60 + (IntValue * 30);
			}
			else
			{
				ResourceGoal = 120 + (IntValue * 60);
			}
		}

		return Super.SetGameRules();
	}

	return FALSE;
}

/** Initializes the game settings from the URL */
event InitGame(string Options, out string ErrorMessage)
{
	Super.InitGame(Options, ErrorMessage);

	// Parse the round score off of the URL
	if ( HasOption(Options,"RoundScore") )
	{
		ResourceGoal = int(ParseOption(Options,"RoundScore"));
	}

	// check for koth mode
	if ( HasOption(Options,"KOTH") )
	{
		SetKOTHRules();

		// if resource goal wasn't modified, use half the normal default
		if (ResourceGoal == default.ResourceGoal)
		{
			ResourceGoal = default.ResourceGoal/2.f;
		}
	}

	LastCapturedByTeamIndex = -1;
}

/** Set the game for KOTH rules */
final function SetKOTHRules()
{
	// mark the mode
	bKingOfTheHillMode = TRUE;
	bExecutionRules = TRUE;

	// force execution rules
	SetExecutionRules();

	// Change leaderboards
	OnlineStatsWriteClass=class'GearLeaderboardWriteKoth';
}

/**
 * @STATS
 * Allows each game type to add it's own set of game rules to the stats object
 */
function string GetGameOptionsForStats(string OptionStr)
{
    if (bKingOfTheHillMode)
    {
	    OptionStr $= "?KingOfTheHillMode=1";
	}
    return super.GetGameOptionsForStats(OptionStr);
}




/** Need to set some Annex specific values */
function InitGameReplicationInfo()
{
	local GearGRI MyGRI;

	Super.InitGameReplicationInfo();

	MyGRI = GetGRI();
	MyGRI.bAnnexIsKOTHRules = bKingOfTheHillMode;
	MyGRI.AnnexResourceGoal = ResourceGoal;
}

/** Team always has respawns */
function bool TeamHasRespawn( int TeamIdx )
{
	if (bKingOfTheHillMode)
	{
		return (CommandTeam == None || CommandTeam.TeamIndex != TeamIdx || CommandMeter < CommandMeterMax);
	}
	return TRUE;
}

/** Overloaded PreRound state */
state PreRound
{
	function BeginState(Name PreviousStateName)
	{
		local int Idx;

		Super.BeginState(PreviousStateName);

		for ( Idx = 0; Idx < NumTeams; Idx++ )
		{
			NumTeamRespawns[Idx] = TotalNumTeamRespawns;
			GetGRI().NumTeamRespawns[Idx] = NumTeamRespawns[Idx];
		}
		GetGRI().TotalNumTeamRespawns = TotalNumTeamRespawns;
		GetGRI().RespawnTimeInterval = RespawnTimeInterval;
	}
}

state MatchInProgress
{
	/** Allow spawning */
	function RestartPlayer(Controller NewPlayer)
	{
		`log(GetFuncName()@NewPlayer);
		GearPRI(NewPlayer.PlayerReplicationInfo).ClearDeathVariables();
		Super(GearGame).RestartPlayer(NewPlayer);
	}

	simulated function BeginState( Name PreviousStateName )
	{
		local int Idx;
		local GearGRI GRI;
		local GearPRI CurrPRI;

		Super.BeginState( PreviousStateName );

		GRI = GetGRI();

		// Clear the teams' points
		for ( Idx = 0; Idx < NumTeams; Idx++ )
		{
			GRI.GameScore[Idx] = 0;
		}

		// Select the first Command Point
		SelectCommandPoint();
		// Set timer to pick a new one every update cycle
		SetTimer( AwardUpdateTime, TRUE, nameof(AwardPoints) );

		// Reset the last captured team
		LastCapturedByTeamIndex = -1;

		// Reset the KOTH circle scoring
		KOTHPlayerPoints.length = 0;

		// Mark how many KOTH ring points the players had to begin the round
		for ( Idx = 0; Idx < GRI.PRIArray.length; Idx++ )
		{
			CurrPRI = GearPRI(GRI.PRIArray[Idx]);
			if ( CurrPRI != None )
			{
				CurrPRI.KOTHRingPointsToBeginRound = CurrPRI.GetGameScore3();
			}
		}
	}

	simulated function EndState( Name NextStateName )
	{
		local int i;

		// Stop timer updates
		ClearTimer( 'AwardPoints' );

		SetCommandPoint( None );
		if( CommandPointEffect != None )
		{
			CommandPointEffect.Destroy();
			CommandPointEffect = None;
		}


		// Write out Road Run Times/etc for this round
		for (i=0;i<GameReplicationInfo.PRIArray.Length;i++)
		{
			GearPRI(GameReplicationInfo.PRIArray[i]).RecordEndRound();
		}
		`RecordStat(STATS_LEVEL1,'RoundEnded');


		// clear this list just in case
		JoinInProgressList.length = 0;
	}

	// Called every tick
	event Tick( float DeltaTime )
	{
		local int Idx;

		// If there are any players waiting to join in progress, allow them to join
		for ( Idx = 0; Idx < JoinInProgressList.length; Idx++ )
		{
			if ( JoinInProgressList[Idx] != None && JoinInProgressList[Idx].IsInState('PlayerWaiting') )
			{
				GearPC(JoinInProgressList[Idx]).TransitionFromDeadState();
				JoinInProgressList.Remove( Idx--, 1 );
			}
		}
	}

	/** If round timer expires, take the team with the most resource points as the winner */
	function bool HandleRoundTimerExpired()
	{
		local int WinningTeamIdx, Idx;
		local GearGRI GRI;
		local float HighScore;

		GRI = GetGRI();
		WinningTeamIdx = -1;
		HighScore = -1;

		for ( Idx = 0; Idx < Teams.length; Idx++ )
		{
			if ( GRI.GameScore[Idx] > HighScore )
			{
				HighScore = GRI.GameScore[Idx];
				WinningTeamIdx = Idx;
			}
		}

		if( WinningTeamIdx >= 0 )
		{
			return HandleTeamWin( WinningTeamIdx );
		}

		return super.HandleRoundTimerExpired();
	}

	function bool PlayerIsAlive(Controller Player, Controller Killed)
	{
		// if they're waiting to respawn in KOTH
		if (bKingOfTheHillMode && CommandTeam != None && Player.GetTeamNum() == CommandTeam.TeamIndex)
		{
			// pretend they're alive
			return TRUE;
		}
		return Super.PlayerIsAlive(Player,Killed);
	}

	/** Check to see if one team has reached total goal */
	function bool CheckEndMatch( optional Controller Killed )
	{
		local bool bResult;
		local int TeamIdx;

		bResult = Super.CheckEndMatch( Killed );
		if( !bResult && (GetGRI() != None) )
		{
			for( TeamIdx = 0; TeamIdx < Teams.Length; TeamIdx++ )
			{
				if( GetGRI().GameScore[TeamIdx] >= ResourceGoal )
				{
					return HandleTeamWin( TeamIdx );
				}
			}
		}

		return bResult;
	}

	/** Make the playerstarts shuffle before broadcasting the respawn message */
	function BroadcastRespawnMessage()
	{
		ShuffleTeamPlayerStarts();
		Super.BroadcastRespawnMessage();
	}
}

/** Whether this is a weapon spawner we should consider in this gametype */
final function bool WeaponSpawnerIsUsable( GearWeaponPickupFactory Factory )
{
	if ( bKingOfTheHillMode )
	{
		return !Factory.bMP_ExcludeKOTH;
	}
	else
	{
		return !Factory.bMP_ExcludeAnnex;
	}
}

/** Pick a super weapon pickup factory to be the CP */
exec function SelectCommandPoint()
{
	local GearWeaponPickupFactory Factory;
	local int Idx, InfoIdx;
	local array<int> List;

	// If factory list is exhausted and we've used some
	if( FactoryList.Length == 0 && UsedList.Length > 0 )
	{
		// Move all the used back into the main list for reuse
		while( UsedList.Length > 0 )
		{
			FactoryList[FactoryList.Length] = UsedList[0];
			UsedList.Remove( 0, 1 );
		}
	}

	// If factory list is empty
	if( FactoryList.Length == 0 )
	{

		// Try to find current map in map info list
		InfoIdx = -1;
		for( Idx = 0; Idx < InfoList.length; Idx++ )
		{
			if( InfoList[Idx].MapName ~= GetMapFilename() )
			{
				InfoIdx = Idx;
				break;
			}
		}

		// If we found the map (and it has a valid list of pickup classes)
		if( InfoIdx >= 0 && InfoList[InfoIdx].PickupList.Length > 0 )
		{
			// Loop through each factory
			foreach AllActors( class'GearWeaponPickupFactory', Factory )
			{
				if ( WeaponSpawnerIsUsable(Factory) )
				{
					// Check each pickup class in the map's list
					for( Idx = 0; Idx < InfoList[InfoIdx].PickupList.Length; Idx++ )
					{
						// If factory pickup class matches one in the map info
						if( Factory.Name == InfoList[InfoIdx].PickupList[Idx] )
						{
							// Add the factory to valid list of CP locations
							FactoryList[FactoryList.Length] = Factory;
							break;
						}
					}

					// Make a list of factories to avoid picking at the start of a round
					for( Idx = 0; Idx < InfoList[InfoIdx].AvoidFirstList.Length; Idx++ )
					{
						if( Factory.Name == InfoList[InfoIdx].AvoidFirstList[Idx] )
						{
							AvoidFirstList[AvoidFirstList.Length] = Factory;
						}
					}
				}
			}
		}

		// If no valid CPs were found through info list
		if( FactoryList.Length == 0 )
		{
			// Build the list (only accept super weapon pickups)
			foreach AllActors( class'GearWeaponPickupFactory', Factory )
			{
				if( WeaponSpawnerIsUsable(Factory) )
				{
					// Add the factory to valid list of CP locations
					FactoryList[FactoryList.Length] = Factory;
				}
			}
		}
	}

	// If factories exist
	if( FactoryList.Length > 0 )
	{
		// Build a list of available ones (exclude the one currently selected)
		for( Idx = 0; Idx < FactoryList.Length; Idx++ )
		{
			// If factory is not current CP AND
			// not selecting the first CP or don't need to avoid this factory
			if( FactoryList[Idx] != CommandPoint &&
				(CommandPoint != None || AvoidFirstList.Find(FactoryList[Idx]) < 0))
			{
				// Add it to the list to choose from
				List[List.Length] = Idx;
			}
		}

		// Select new CP randomly
		Idx = List[Rand(List.Length)];
		Factory = FactoryList[Idx];
		if( Factory != None )
		{
			// Put new CP in the used list
			UsedList[UsedList.Length] = Factory;
			FactoryList.Remove( Idx, 1 );

			// Set the new CP
			SetCommandPoint( Factory );
		}
	}
	else
	{
		`Warn( "No GearWeaponPickupFactory in this level, unable to select Command Point" );
	}
}

/** Send a sound to all player controllers */
function BroadcastSound( SoundCue ASound )
{
	local PlayerController PC;

	foreach WorldInfo.AllControllers( class'PlayerController', PC )
	{
		PC.ClientPlaySound( ASound );
	}
}

/** Send a warning to players about the end of the game coming */
function BroadcastEndGameWarning( int TeamIndex )
{
	local GearPC PC;
	local bool bSendWarning;
	local int ScoreDiff;

	ScoreDiff = ResourceGoal - int(GetGRI().GameScore[TeamIndex]);

	if ( (ScoreDiff <= 10) && (ScoreDiff >= 0) )
	{
		bSendWarning = true;
	}
	else if ( (ScoreDiff <= 20) && (ScoreDiff > 10) )
	{
		if ( ScoreDiff % 2 == 0 )
		{
			bSendWarning = true;
		}
	}

	if ( bSendWarning )
	{
		foreach WorldInfo.AllControllers( class'GearPC', PC )
		{
			PC.ClientEndOfGameWarning( ScoreDiff, TeamIndex );
			// Check for TG tutorial
			DoTrainingGroundTutorial(GEARTUT_TRAIN_WinRnd, 0, TeamIndex);
			DoTrainingGroundTutorial(GEARTUT_TRAIN_LoseRnd, 1, TeamIndex);
		}
	}
}

/** Set new CP and do any bookkeeping that needs to be done */
function SetCommandPoint( GearWeaponPickupFactory NewCP )
{
	local GearAI_TDM Bot;
	local AICmd_MoveToGoal MoveCmd;

	//debug
	`log( GetFuncName()@NewCP@NewCP.OriginalPickupClass );

	// interrupt any bots that were moving towards the old command point
	// so that if they were only doing so because it was the game objective, they'll re-evaluate their target
	if (CommandPoint != None)
	{
		foreach WorldInfo.AllControllers(class'GearAI_TDM', Bot)
		{
			if (Bot.MoveGoal == CommandPoint)
			{
				MoveCmd = Bot.FindCommandOfClass(class'AICmd_MoveToGoal');
				if (MoveCmd != None)
				{
					`AILog_Ext("Aborting move command to" @ CommandPoint @ "because it's no longer the command point",, Bot);
					Bot.AbortCommand(MoveCmd);
				}
			}
		}
	}

	// Set new CP
	CommandPoint = NewCP;

	if( CommandPoint != None )
	{
		if( CommandPointEffect == None )
		{
			CommandPointEffect = Spawn( CommandPointEffectClass,,, CommandPoint.Location, CommandPoint.Rotation );
			CommandPointEffect.ParticleSystemComponent.ActivateSystem();
		}
		else
		{
			CommandPointEffect.SetLocation( CommandPoint.Location );
		}
	}

	// Reset resource count
	if (bKingOfTheHillMode)
	{
		// infinite resources for king of the hill
		SetCommandPointResources( -1 );
	}
	else
	{
		SetCommandPointResources( default.CommandPointResources );
	}
	// Reset command meter
	SetCommandMeter( 0.f );

	if ( GetGRI() != None )
	{
		GetGRI().CommandPoint = CommandPoint;
	}

	// Clear command team
	SetCommandTeam( None );

	if( CommandPoint != None )
	{
		// Play sound for all to hear
		BroadcastSound( AnnexSound_CommandPointChanged );
	}
}

/** Set new CT and do any bookkeeping that is necessary */
function SetCommandTeam( GearTeamInfo NewCT, optional bool bSkipSound )
{
	local GearTeamInfo OldTeam;
	local GearPC PC;

	// Set new CT
	OldTeam = CommandTeam;
	CommandTeam = NewCT;
	if( CommandTeam != None )
	{
		// Play sound for all to hear
		BroadcastSound( AnnexSound_CommandTeamChanged[0/*CommandTeam.TeamIndex*/] );
		// Check for a TG tutorial
		DoTrainingGroundTutorial(GEARTUT_TRAIN_TeamCap, 0, CommandTeam.TeamIndex);
	}

	// in KOTH notify start the respawn queue for all dead players
	if (bKingOfTheHillMode && OldTeam != None && NewCT != OldTeam)
	{
		foreach WorldInfo.AllControllers(class'GearPC',PC)
		{
			//@FIXME: IsDead() returns true when DBNO!
			if (PC.GetTeamNum() == OldTeam.TeamIndex && (PC.IsSpectating() || PC.IsDead()) && PC.MyGearPawn == None)
			{
				PC.WaitForRespawn();
			}
		}
	}

	// Update CP effect with team color
	if( CommandPointEffect != None )
	{
		SetAnnexRingColor( GetCommandTeamColor() );
	}

	if ( GetGRI() != None )
	{
		if ( CommandTeam == None )
		{
			GetGRI().CPControlTeam = -1;
		}
		else
		{
			GetGRI().CPControlTeam = CommandTeam.TeamIndex;
		}
	}
}

/** Set amount of control the command team has over the CP */
function SetCommandMeter( float Amt )
{
	// Play sound for all to hear if going from not full control to full control
	if( CommandMeter < CommandMeterMax && Amt >= CommandMeterMax )
	{
		BroadcastSound( AnnexSound_CommandMeterMaxed[0/*CommandTeam.TeamIndex*/] );
		// Check for TG tutorial
		DoTrainingGroundTutorial(GEARTUT_TRAIN_NmyCap, 1, CommandTeam.TeamIndex);
		DoTrainingGroundTutorial(GEARTUT_TRAIN_Defend, 0, CommandTeam.TeamIndex);
	}
	else
	// Otherwise, play sound if command meter is broken
	if( CommandMeter >= CommandMeterMax && Amt < CommandMeterMax && Amt > 0 )
	{
		if( CommandTeam.TeamIndex == 0 )
		{
			BroadcastSound( AnnexSound_CommandMeterBroken[0/*1*/] );
		}
		else
		{
			BroadcastSound( AnnexSound_CommandMeterBroken[0] );
		}
	}

	CommandMeter = Amt;
	SetAnnexRingProgression( CommandMeter / CommandMeterMax );
	GetGRI().CPControlPct = int(100.f * (CommandMeter / CommandMeterMax));
}

/** Set the ring progression for capturing */
function SetAnnexRingProgression( float Progression );

/** Set the annex ring's color */
function SetAnnexRingColor( Color RingColor );

/** Set amount of resources left before CP is dry */
function SetCommandPointResources( int Amt )
{
	CommandPointResources	= Amt;
	GetGRI().CPResourceLeft = Amt;
}

function Color GetCommandTeamColor()
{
	// Neutral
	if( CommandTeam == None )
	{
		return MakeColor(80,80,70,255);
	}
	else
	// COG
	if( CommandTeam.TeamIndex == 0 )
	{
		return MakeColor(0,90,120,255);
	}
	// Locust
	else
	{
		return MakeColor(140,0,0,255);
	}
}

function bool CheckResourcePoints()
{
	if( CommandPointResources <= 0 && !bKingOfTheHillMode)
	{
		SelectCommandPoint();
		// Check for TG tutorial
		DoTrainingGroundTutorial(GEARTUT_TRAIN_DrainRng);
		return TRUE;
	}

	return FALSE;
}

/** Calculate points to award the controlling team */
function AwardPoints()
{
	local GearTeamInfo T;
	local GearPawn P;
	local int TeamIdx, Idx, BestIdx;
	local array<int> NearbyCount;
	local array<int> SumTeamScore;
	local int Pts, NumPointsActuallyAdded;
	local array<Controller> NearbyPCs;
	local array<Controller> NearbyPCsToBreak;
	local bool bCPIsCurrentlyOwned;
	local int NumTeamsInCircle;
	local bool bDoInstaClear;

	// Don't do anything while the cheat detection pop up is there
	if ( bIsPausedDueToCheatDetection )
	{
		return;
	}

	// Can't award points if there is no CP
	if( CommandPoint == None || GetGRI() == None )
	{
		SetCommandTeam( None );
		return;
	}

	// Check to see if we need to switch CPs
	if( CheckResourcePoints() )
	{
		// Returns true if CP switched
		return;
	}

	// Find all the PCs inside the ring
	NearbyCount.Length  = Teams.Length;
	for( TeamIdx = 0; TeamIdx < Teams.Length; TeamIdx++ )
	{
		T = Teams[TeamIdx];

		for( Idx = 0; Idx < T.TeamMembers.Length; Idx++ )
		{
			if( T.TeamMembers[Idx] != None )
			{
				P = GearPawn(T.TeamMembers[Idx].Pawn);
				if( IsNearCommandPoint( P ) )
				{
					NearbyCount[TeamIdx]++;
					NearbyPCs[NearbyPCs.Length] = P.Controller;
				}
			}
		}

		// Keep track of how many teams are in the circle and who the last one was (for KOTH)
		if ( NearbyCount[TeamIdx] > 0 )
		{
			NumTeamsInCircle++;
			BestIdx = TeamIdx;
		}
	}

	// Go through each team and compute the ring advantage
	SumTeamScore.Length = Teams.Length;
	for( TeamIdx = 0; TeamIdx < SumTeamScore.Length; TeamIdx++ )
	{
		// Base score is how many people this team has nearby
		SumTeamScore[TeamIdx] = NearbyCount[TeamIdx];

		// Subtract the count of enemies nearby
		for( Idx = 0; Idx < SumTeamScore.Length; Idx++ )
		{
			if( TeamIdx != Idx )
			{
				SumTeamScore[TeamIdx] -= NearbyCount[Idx];
			}
		}
	}

	// Figure out the BestIdx for KOTH rules
	if ( bKingOfTheHillMode )
	{
		// No one gets credit if exactly one team is not in the ring
		if ( NumTeamsInCircle != 1 )
		{
			// No team gets credit
			BestIdx = -1;
		}
	}
	// Figure out the BestIdx for normal rules
	else
	{
		// Go through each team and pick the top score
		BestIdx = -1;
		for( TeamIdx = 0; TeamIdx < SumTeamScore.Length; TeamIdx++ )
		{
			if( SumTeamScore[TeamIdx] <= 0 )
				continue;

			if( BestIdx < 0 ||
				SumTeamScore[TeamIdx] > SumTeamScore[BestIdx] )
			{
				BestIdx = TeamIdx;
			}
		}
	}

	// determine if a team currently has control of the CP
	if ( (CommandTeam != None) && (CommandMeter >= CommandMeterMax) )
	{
		bCPIsCurrentlyOwned = true;
	}

	// If someone is in the circle
	if( BestIdx >= 0 )
	{
		// Give the best team claim if there isn't currently one
		if( CommandTeam == None )
		{
			SetCommandTeam( Teams[BestIdx] );
		}

		// If strongest team has the claim
		if( CommandTeam == Teams[BestIdx] )
		{
			// Increase their meter toward the max
			SetCommandMeter( FMin(CommandMeter + SumTeamScore[BestIdx], CommandMeterMax) );

			// see if we're still trying to cap
			if ( !bCPIsCurrentlyOwned )
			{
				// accumulate temp points
				AccumulateTeamPoints( NearbyPCs, BestIdx );

				// see if this team just took the CP or not
				if ( CommandMeter >= CommandMeterMax )
				{
					// If this is using KOTH rules, only add score when CP was captured by enemy of this is first capture
					if ( !bKingOfTheHillMode || (LastCapturedByTeamIndex != BestIdx) )
					{
						ScoreTeamCapture( NearbyPCs, BestIdx );
						LastCapturedByTeamIndex = BestIdx;
					}

					PulseHUDPlayerCPIndicators( 2, 1.f );
				}
			}
		}
		// Otherwise, the strongest team is trying to take the claim
		else
		{
			// KOTH rules - instant clear
			if ( bKingOfTheHillMode )
			{
				SetCommandMeter( 0 );
			}
			// Normal rules
			else
			{
				// Decrease current teams meter toward 0 (take away control twice as fast)
				SetCommandMeter( FMax(CommandMeter - (SumTeamScore[BestIdx] * 2), 0) );
			}

			// see if this team just stopped total control of the CP
			if ( bCPIsCurrentlyOwned && (CommandMeter < CommandMeterMax) )
			{
				ScoreTeamBreak( NearbyPCs, BestIdx );
				PulseHUDPlayerCPIndicators( 5, 0.2f );
			}

			// see if this CP was just taken back
			if ( (CommandMeter <= 0.f) && (CommandTeam != None) )
			{
				ClearAllAccumulatedPoints();
			}
		}
	}
	// Otherwise, no one is in the circle - Handled differently with KOTH rules
	else
	{
		// Normal rules
		if ( !bKingOfTheHillMode )
		{
			// If there is a controlling team and they haven't gained complete control
			if( CommandTeam != None && CommandMeter < CommandMeterMax )
			{
				// Bleed off control toward 0
				SetCommandMeter( FMax( CommandMeter - 1, 0 ) );
			}

			// see if this CP was just taken back
			if ( (CommandMeter <= 0.f) && (CommandTeam != None) )
			{
				ClearAllAccumulatedPoints();
			}
		}
		// KOTH rules
		else
		{
			bDoInstaClear = false;

			// A team has control
			if ( CommandTeam != None )
			{
				// We couldn't find a BestIdx and a team is still in control so see if another team broke the command point
				for ( TeamIdx = 0; TeamIdx < Teams.length; TeamIdx++ )
				{
					// Someone is in the ring and it's NOT the command team (another team broke the control)
					if ( (NearbyCount[TeamIdx] > 0) && (CommandTeam != Teams[TeamIdx]) )
					{
						// Mark the InstantClear flag
						bDoInstaClear = true;

						// Count all of the players in the ring and store those players so we can credit them for the break
						T = Teams[TeamIdx];
						for ( Idx = 0; Idx < T.TeamMembers.length; Idx++ )
						{
							if ( T.TeamMembers[Idx] != None )
							{
								P = GearPawn(T.TeamMembers[Idx].Pawn);
								if ( IsNearCommandPoint(P) )
								{
									NearbyCount[TeamIdx]++;
									NearbyPCsToBreak[NearbyPCsToBreak.length] = P.Controller;
								}
							}
						}

						break;
					}
				}
			}

			// See if we need to insta clear the CP (other team broke the CP
			if ( bDoInstaClear )
			{
				SetCommandMeter( 0 );
			}
			// The commanding team left the ring unattended so drain it
			else
			{
				SetCommandMeter( FMax(CommandMeter-1, 0) );
			}

			// See if this CP was JUST taken back
			if ( (CommandMeter <= 0.0f) && (CommandTeam != None) )
			{
				ClearAllAccumulatedPoints();
			}

			// If there is more than one team in the CP
			if ( NumTeamsInCircle > 0 )
			{
				if ( LastCapturedByTeamIndex != -1 )
				{
					ScoreTeamBreak( NearbyPCsToBreak, TeamIdx );
					PulseHUDPlayerCPIndicators( 5, 0.2f );

					// Play sound for all to hear
					BroadcastSound( AnnexSound_CommandMeterBroken[0] );
				}
				LastCapturedByTeamIndex = -1;
			}
		}
	}

	// If there is a team with a claim
	// Handle extremes of the command meter
	if( CommandTeam != None )
	{
		// If meter is neutral
		if( CommandMeter <= 0.f )
		{
			// Clear command team
			SetCommandTeam( None );
		}
		// Otherwise, if meter is maxed, award points to the team
		else if( CommandMeter >= CommandMeterMax )
		{
			// Get number of points to give to the team (don't allow excess drain of CP resources)
			Pts = NumAwardPoints;
			if( CommandPointResources > 0 )
			{
				Pts = Min( NumAwardPoints, CommandPointResources );
				// Deplete CP resources
				SetCommandPointResources(CommandPointResources - Pts);
			}

			NumPointsActuallyAdded = FMin( GetGRI().GameScore[CommandTeam.TeamIndex] + Pts, ResourceGoal ) - GetGRI().GameScore[CommandTeam.TeamIndex];
			GetGRI().GameScore[CommandTeam.TeamIndex] += NumPointsActuallyAdded;

			// Give points to the sorry people sitting in the circle
			if ( bKingOfTheHillMode )
			{
				ScoreKOTHResourcePoints( NumPointsActuallyAdded, NearbyPCs );
			}

			// Keep track of the leading team and play a sound if the lead team changes
			if( (LeadingTeam == None) ||
				((CommandTeam != LeadingTeam) &&
				 (GetGRI().GameScore[CommandTeam.TeamIndex] > GetGRI().GameScore[LeadingTeam.TeamIndex])) )
			{
				// New lead team, play sound
				if( LeadingTeam != None )
				{
					BroadcastSound( AnnexSound_TeamLeadChanged[0/*CommandTeam.TeamIndex*/] );
				}
				LeadingTeam = CommandTeam;
			}


			// Potentially warn players of the end of the game due to this last point adjustment
			BroadcastEndGameWarning( CommandTeam.TeamIndex );
		}
	}
}

/** Send an Annex message to all players */
function SendAnnexMessage( int TeamIndex, String AnnexMessage )
{
	local GearPC PC;
	local GearGRI GGRI;
	local PlayerReplicationInfo PRI;
	local int Idx;

	GGRI = GetGRI();

	if ( GGRI != None )
	{
		PRI = None;

		// Find a PRI who is from the team with this TeamIndex, so we can pass it into the ClientAddAnnexMessage function, anybody on the team will do.
		for ( Idx = 0; Idx < GGRI.PRIArray.Length; Idx++ )
		{
			if ( (GGRI.PRIArray[Idx] != None) && (GGRI.PRIArray[Idx].Team.TeamIndex == TeamIndex) )
			{
				PRI = GGRI.PRIArray[Idx];
				break;
			}
		}

		if ( PRI != None )
		{
			foreach WorldInfo.AllControllers( class'GearPC', PC )
			{
				if ( PC.PlayerReplicationInfo != None )
				{
					PC.ClientAddAnnexMessage( PRI, AnnexMessage );
				}
			}
		}
	}
}

// Give points to the sorry people sitting in the circle
function ScoreKOTHResourcePoints( int TeamPoints, array<Controller> NearbyPCs )
{
	local int PlayerIdx, PRIIdx, ScorePoints, InfoIdx;
	local GearPRI PlayerPRI;
	local KOTHPlayerPointInfo PlayerInfo;
	local float PointsPerPlayer;

	if ( (TeamPoints > 0) && (NearbyPCs.length > 0) )
	{
		// Determine how many points each player will receive
		PointsPerPlayer = float(TeamPointsPerKOTHTick * TeamPoints) / float(NearbyPCs.length);

		// Assign points to all the nearby players
		for ( PlayerIdx = 0; PlayerIdx < NearbyPCs.length; PlayerIdx++ )
		{
			PlayerPRI = GearPRI(NearbyPCs[PlayerIdx].PlayerReplicationInfo);
			if ( PlayerPRI != None )
			{
				// Find the player in the list
				PRIIdx = -1;
				for ( InfoIdx = 0; InfoIdx < KOTHPlayerPoints.length; InfoIdx++ )
				{
					if ( KOTHPlayerPoints[InfoIdx].PlayerPRI == PlayerPRI )
					{
						PRIIdx = InfoIdx;
						break;
					}
				}

				// No player in list so add him and the new points
				if ( PRIIdx == -1 )
				{
					PlayerInfo.PlayerPRI = PlayerPRI;
					PlayerInfo.PointsEarned = PointsPerPlayer;
					PRIIdx = KOTHPlayerPoints.AddItem( PlayerInfo );
				}
				// Found him so add his points
				else
				{
					KOTHPlayerPoints[PRIIdx].PointsEarned += PointsPerPlayer;
				}

				// Now we truncate the number adding that value to the scoring and saving the remainder
				ScorePoints = int(KOTHPlayerPoints[PRIIdx].PointsEarned);
				KOTHPlayerPoints[PRIIdx].PointsEarned -= ScorePoints;
				KOTHPlayerPoints[PRIIdx].PlayerPRI.ScoreGameSpecific3('KOTHRingPoints', "", ScorePoints, ScorePoints < 20);
			}
		}
	}
}

/** Give a capture to all nearby players on the team */
function ScoreTeamCapture(array<Controller> NearbyPCs, int ControlTeamId)
{
	local int Idx;

	for ( Idx = 0; Idx < NearbyPCs.Length; Idx++ )
	{
		if ( (NearbyPCs[Idx].PlayerReplicationInfo != None) && (GearPRI(NearbyPCs[Idx].PlayerReplicationInfo).Team.TeamIndex == ControlTeamId) )
		{
			GearPRI(NearbyPCs[Idx].PlayerReplicationInfo).ScoreGameSpecific1('AnnexCapture',"",PointsPerCap);
		}
	}

	ScoreTeamPoints( ControlTeamId );
}

/** Give a break to all nearby players on the team */
function ScoreTeamBreak(array<Controller> NearbyPCs, int ControlTeamId)
{
	local int Idx;

	for ( Idx = 0; Idx < NearbyPCs.Length; Idx++ )
	{
		if ( (NearbyPCs[Idx].PlayerReplicationInfo != None) && (GearPRI(NearbyPCs[Idx].PlayerReplicationInfo).Team.TeamIndex == ControlTeamId) )
		{
			GearPRI(NearbyPCs[Idx].PlayerReplicationInfo).ScoreGameSpecific2('AnnexBreak',"",PointsPerBreak);
		}
	}
}

/**
 * Accumulate the points being earned.  This is a temporary storarge so that real points can be added to the player once a CP is
 * officially captured or broken.
 */
function AccumulateTeamPoints(array<Controller> NearbyPCs, int ControlTeamId)
{
	local int Idx;

	for ( Idx = 0; Idx < NearbyPCs.Length; Idx++ )
	{
		if ( (NearbyPCs[Idx].PlayerReplicationInfo != None) && (GearPRI(NearbyPCs[Idx].PlayerReplicationInfo).Team.TeamIndex == ControlTeamId) )
		{
			GearPRI(NearbyPCs[Idx].PlayerReplicationInfo).PointAccumulator += PointsPerCPTick;
		}
	}
}

/** Give the accumulated points to all players who earned them */
function ScoreTeamPoints( int ControlTeamId )
{
	local GearGRI GGRI;
	local PlayerReplicationInfo PRI;
	local GearPRI GPRI;
	GGRI = GetGRI();
	if ( GGRI != None )
	{
		foreach GGRI.PRIArray(PRI)
		{
			GPRI = GearPRI(PRI);
			if (GPRI != None)
			{
				if (GPRI.Team.TeamIndex == ControlTeamId)
				{
					GPRI.ModifyScore( FMin(GPRI.PointAccumulator, CommandMeterMax*PointsPerCPTick) );
				}
				GPRI.PointAccumulator = 0;
			}
		}
	}
}

/** Clear the accumulated points to all nearby players on the team */
function ClearAllAccumulatedPoints()
{
	local int Idx;
	local GearGRI GGRI;

	GGRI = GetGRI();

	if ( GGRI != None )
	{
		for ( Idx = 0; Idx < GGRI.PRIArray.Length; Idx++ )
		{
			if ( GGRI.PRIArray[Idx] != None )
			{
				GearPRI(GGRI.PRIArray[Idx]).PointAccumulator = 0;
			}
		}
	}
}

/** Have players pulse their command point indicators */
function PulseHUDPlayerCPIndicators( int NumPulses, float LengthOfPulse = 1.f )
{
	local GearPC PC;

	foreach WorldInfo.AllControllers( class'GearPC', PC )
	{
		PC.ClientSetCommandPointIndicatorGlow( NumPulses, LengthOfPulse );
	}
}

/** Determines if a pawn is valid and close enough to the CP to get points */
function bool IsNearCommandPoint( GearPawn P )
{
	local float DistSq, DistZ;

	if( CommandPoint != None &&
		P != None &&
		(P.Health > 0 || P.IsDBNO()) )
	{
		DistSq = VSizeSq2D(P.Location - CommandPoint.Location);
		DistZ = Abs(P.Location.Z-CommandPoint.Location.Z);

		if( DistSq <= MaxPointDist*MaxPointDist &&
			DistZ <= MaxPointDistZ )
		{
			return TRUE;
		}
	}

	return FALSE;
}

//
// Log a player in.
// Fails login if you set the Error string.
// PreLogin is called before Login, but significant game time may pass before
// Login is called, especially if content is downloaded.
//
event PlayerController Login ( string Portal, string Options, out string ErrorMessage )
{
	local PlayerController NewPlayer;
	local GearPC WPC;

	// call the super class
	NewPlayer = Super.Login( Portal, Options, ErrorMessage );

	if ( NewPlayer != None )
	{
		// add the player to the join in progress list if a valid player controller exists after the game has begun
		if ( GetGRI().GameStatus == GS_RoundInProgress )
		{
			JoinInProgressList[ JoinInProgressList.length ] = NewPlayer;
		}
		else
		{
			// Set the newly created player to the neutral team position.
			WPC = GearPC(NewPlayer);
			if ( WPC != none && WPC.bDedicatedServerSpectator )
			{
				ChangeTeam( WPC,255,false );
			}
		}
	}

	return NewPlayer;
}

function Actor GetObjectivePointForAI(GearAI AI)
{
	// always take and guard command point
	return CommandPoint;
}

function bool MustStandOnObjective(GearAI AI)
{
	return (bKingOfTheHillMode || CommandTeam != AI.PlayerReplicationInfo.Team || CommandMeter < CommandMeterMax);
}

function SetAICombatMoodFor(GearAI AI)
{
	local ECombatMood NewCombatMood;

	// if someone else owns it, attack it
	if (CommandTeam != AI.PlayerReplicationInfo.Team)
	{
		NewCombatMood = AICM_Aggressive;
	}
	else
	{
		NewCombatMood = AICM_Normal;
	}

	if (AI.CombatMood != NewCombatMood)
	{
		AI.SetCombatMood(NewCombatMood);
	}
}

function AdjustEnemyRating(GearAI AI, out float out_Rating, Pawn EnemyPawn)
{
	local GearPawn P;

	P = GearPawn(EnemyPawn);
	if (P != None && IsNearCommandPoint(P))
	{
		out_Rating += 0.5;
	}
}

defaultproperties
{
	AwardUpdateTime=1.f
	CommandMeterMax=8

	MaxPointDist=256.f
	MaxPointDistZ=128.f

	OnlineStatsWriteClass=class'GearLeaderboardWriteAnnex'
}
