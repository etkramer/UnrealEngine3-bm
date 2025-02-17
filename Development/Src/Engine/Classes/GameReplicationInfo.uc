//=============================================================================
// GameReplicationInfo.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//
// Every GameInfo creates a GameReplicationInfo, which is always relevant, to replicate
// important game data to clients (as the GameInfo is not replicated).
//=============================================================================
class GameReplicationInfo extends ReplicationInfo
	config(Game)
	native nativereplication;

/** Class of the server's gameinfo, assigned by GameInfo. */
var repnotify class<GameInfo> GameClass;

/** The data store instance responsible for presenting state data for the current game session. */
var	protected		CurrentGameDataStore		CurrentGameData;

var bool bStopCountDown;
var repnotify bool bMatchHasBegun;
var repnotify bool bMatchIsOver;

/**
 * Used to determine if the end of match/session clean up is needed. Game invites
 * might have already cleaned up the match/session so doing so again would break
 * the traveling to the invited game
 */
var bool bNeedsOnlineCleanup;

/** Used to determine who handles session ending */
var bool bIsArbitrated;

var databinding int  RemainingTime, ElapsedTime, RemainingMinute;
var float SecondCount;
var databinding int GoalScore;
var databinding int TimeLimit;
var databinding int MaxLives;

var databinding array<TeamInfo > Teams;

var() databinding globalconfig string ServerName;		// Name of the server, i.e.: Bob's Server.
var() databinding globalconfig string ShortName;		// Abbreviated name of server, i.e.: B's Serv (stupid example)
var() globalconfig string AdminName;		// Name of the server admin.
var() globalconfig string AdminEmail;		// Email address of the server admin.
var() globalconfig int	  ServerRegion;		// Region of the game server.

var() databinding globalconfig string MessageOfTheDay;

var Actor Winner;			// set by gameinfo when game ends

/** Array of all PlayerReplicationInfos, maintained on both server and clients (PRIs are always relevant) */
var		array<PlayerReplicationInfo> PRIArray;

/** This list mirrors the GameInfo's list of inactive PRI objects */
var		array<PlayerReplicationInfo> InactivePRIArray;

// stats

var int MatchID;
var bool bTrackStats;

cpptext
{
	// AActor interface.
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	if ( bNetDirty && (Role == ROLE_Authority) )
		bStopCountDown, Winner, bMatchHasBegun, bMatchIsOver, MatchID;

	if ( !bNetInitial && bNetDirty && (Role == ROLE_Authority) )
		RemainingMinute;

	if ( bNetInitial && (Role==ROLE_Authority) )
		GameClass,
		RemainingTime, ElapsedTime,MessageOfTheDay,
		ServerName, ShortName, AdminName,
		AdminEmail, ServerRegion, GoalScore, MaxLives, TimeLimit,
		bTrackStats, bIsArbitrated;
}


simulated event PostBeginPlay()
{
	local PlayerReplicationInfo PRI;
	local TeamInfo TI;

	if( WorldInfo.NetMode == NM_Client )
	{
		// clear variables so we don't display our own values if the server has them left blank
		ServerName = "";
		AdminName = "";
		AdminEmail = "";
		MessageOfTheDay = "";
	}

	SecondCount = WorldInfo.TimeSeconds;
	SetTimer(WorldInfo.TimeDilation, true);

	WorldInfo.GRI = self;

	// associate this GRI with the "CurrentGame" data store
	InitializeGameDataStore();

	ForEach DynamicActors(class'PlayerReplicationInfo',PRI)
	{
		AddPRI(PRI);
	}
	foreach DynamicActors(class'TeamInfo', TI)
	{
		if (TI.TeamIndex >= 0)
		{
			SetTeam(TI.TeamIndex, TI);
		}
	}

	bNeedsOnlineCleanup = true;
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'bMatchHasBegun' )
	{
		if (bMatchHasBegun)
		{
			WorldInfo.NotifyMatchStarted();
		}
	}
	else if ( VarName == 'bMatchIsOver' )
	{
		if ( bMatchIsOver )
		{
			EndGame();
		}
	}
	else if ( VarName == 'GameClass' )
	{
		ReceivedGameClass();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}


/** This is used to notify clients when they have received the GameClass so they can do things based off that. **/
simulated function ReceivedGameClass();

/* Reset()
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	Super.Reset();
	Winner = None;
}

/**
 * Called when this actor is destroyed
 */
simulated event Destroyed()
{
	Super.Destroyed();

	// de-associate this GRI with the "CurrentGame" data store
	CleanupGameDataStore();
}

simulated event Timer()
{
	if ( (WorldInfo.Game == None) || WorldInfo.Game.MatchIsInProgress() )
	{
		ElapsedTime++;
	}
	if ( WorldInfo.NetMode == NM_Client )
	{
		// sync remaining time with server once a minute
		if ( RemainingMinute != 0 )
		{
			RemainingTime = RemainingMinute;
			RemainingMinute = 0;
		}
	}
	if ( (RemainingTime > 0) && !bStopCountDown )
	{
		RemainingTime--;
		if ( WorldInfo.NetMode != NM_Client )
		{
			if ( RemainingTime % 60 == 0 )
			{
				RemainingMinute = RemainingTime;
			}
		}
	}

	if ( CurrentGameData != None )
	{
		// give the current game data store a chance to update its state
		CurrentGameData.Timer();
	}

	SetTimer(WorldInfo.TimeDilation, true);
}

/**
 * Checks to see if two actors are on the same team.
 *
 * @return	true if they are, false if they aren't
 */
simulated native function bool OnSameTeam(Actor A, Actor B);

simulated function PlayerReplicationInfo FindPlayerByID( int PlayerID )
{
    local int i;

    for( i=0; i<PRIArray.Length; i++ )
    {
	if( PRIArray[i].PlayerID == PlayerID )
	    return PRIArray[i];
    }
    return None;
}

simulated function AddPRI(PlayerReplicationInfo PRI)
{
	local int i;

	// Determine whether it should go in the active or inactive list
	if (!PRI.bIsInactive)
	{
		// make sure no duplicates
		for (i=0; i<PRIArray.Length; i++)
		{
			if (PRIArray[i] == PRI)
				return;
		}

		PRIArray[PRIArray.Length] = PRI;
	}
	else
	{
		// Add once only
		if (InactivePRIArray.Find(PRI) == INDEX_NONE)
		{
			InactivePRIArray[InactivePRIArray.Length] = PRI;
		}
	}

    if ( CurrentGameData == None )
    {
    	InitializeGameDataStore();
    }

	if ( CurrentGameData != None )
	{
		CurrentGameData.AddPlayerDataProvider(PRI);
	}
}

simulated function RemovePRI(PlayerReplicationInfo PRI)
{
    local int i;

    for (i=0; i<PRIArray.Length; i++)
    {
		if (PRIArray[i] == PRI)
		{
			if ( CurrentGameData != None )
			{
				CurrentGameData.RemovePlayerDataProvider(PRI);
			}

		    PRIArray.Remove(i,1);
			return;
		}
    }
}

/**
 * Assigns the specified TeamInfo to the location specified.
 *
 * @param	Index	location in the Teams array to place the new TeamInfo.
 * @param	TI		the TeamInfo to assign
 */
simulated function SetTeam( int Index, TeamInfo TI )
{
	//`log(GetFuncName()@`showvar(Index)@`showvar(TI));
	if ( Index >= 0 )
	{
		if ( CurrentGameData == None )
		{
    		InitializeGameDataStore();
		}

		if ( CurrentGameData != None )
		{
			if ( Index < Teams.Length && Teams[Index] != None )
			{
				// team is being replaced with another instance - see HandleSeamlessTravelPlayer
				CurrentGameData.RemoveTeamDataProvider( Teams[Index] );
			}

			if ( TI != None )
			{
				CurrentGameData.AddTeamDataProvider(TI);
			}
		}

		Teams[Index] = TI;
	}
}

simulated function GetPRIArray(out array<PlayerReplicationInfo> pris)
{
    local int i;
    local int num;

    pris.Remove(0, pris.Length);
    for (i=0; i<PRIArray.Length; i++)
    {
		if (PRIArray[i] != None)
			pris[num++] = PRIArray[i];
    }
}

/**
  * returns true if P1 should be sorted before P2
  */
simulated function bool InOrder( PlayerReplicationInfo P1, PlayerReplicationInfo P2 )
{
	local LocalPlayer LP1, LP2;

	// spectators are sorted last
    if( P1.bOnlySpectator )
    {
		return P2.bOnlySpectator;
    }
    else if ( P2.bOnlySpectator )
	{
		return true;
	}

	// sort by Score
    if( P1.Score < P2.Score )
	{
		return false;
	}
    if( P1.Score == P2.Score )
    {
		// if score tied, use deaths to sort
		if ( P1.Deaths > P2.Deaths )
			return false;

		// keep local player highest on list
		if ( (P1.Deaths == P2.Deaths) && (PlayerController(P2.Owner) != None) )
		{
			LP2 = LocalPlayer(PlayerController(P2.Owner).Player);
			if ( LP2 != None )
			{
				if ( !class'Engine'.static.IsSplitScreen() || (LP2.ViewportClient.GamePlayers[0] == LP2) )
				{
					return false;
				}
				// make sure ordering is consistent for splitscreen players
				LP1 = LocalPlayer(PlayerController(P2.Owner).Player);
				return ( LP1 != None );
			}
		}
	}
    return true;
}

simulated function SortPRIArray()
{
    local int i, j;
    local PlayerReplicationInfo P1, P2;

    for (i=0; i<PRIArray.Length-1; i++)
    {
    	P1 = PRIArray[i];
		for (j=i+1; j<PRIArray.Length; j++)
		{
			P2 = PRIArray[j];
		    if( !InOrder( P1, P2 ) )
		    {
				PRIArray[i] = P2;
				PRIArray[j] = P1;
				P1 = P2;
		    }
		}
    }
}


/**
 * Called when a variable is replicated that has the 'databinding' keyword.
 *
 * @param	VarName		the name of the variable that was replicated.
 */
simulated event ReplicatedDataBinding( name VarName )
{
	Super.ReplicatedDataBinding(VarName);

	if ( CurrentGameData != None )
	{
		CurrentGameData.RefreshSubscribers(VarName, true, CurrentGameData);
	}
}

/**
 * Creates and registers a data store for the current game session.
 */
simulated function InitializeGameDataStore()
{
	local DataStoreClient DataStoreManager;

	DataStoreManager = class'UIInteraction'.static.GetDataStoreClient();
	if ( DataStoreManager != None )
	{
		CurrentGameData = CurrentGameDataStore(DataStoreManager.FindDataStore('CurrentGame'));
		if ( CurrentGameData != None )
		{
			CurrentGameData.CreateGameDataProvider(Self);
		}
		else
		{
			`log("Primary game data store not found!");
		}
	}
}

/**
 * Unregisters the data store for the current game session.
 */
simulated function CleanupGameDataStore()
{
	`log(`location,,'DataStore');
	if ( CurrentGameData != None )
	{
		CurrentGameData.ClearDataProviders();
	}

	CurrentGameData = None;
}

/**
 * Called on the server when the match has begin
 *
 * Network - Server and Client (Via ReplicatedEvent)
 */

simulated function StartMatch()
{
	bMatchHasBegun = true;
}

/**
 * Called on the server when the match is over
 *
 * Network - Server and Client (Via ReplicatedEvent)
 */

simulated function EndGame()
{
	bMatchIsOver = true;
}

/** Is the current gametype a multiplayer game? */
simulated function bool IsMultiplayerGame()
{
	return (WorldInfo.NetMode != NM_Standalone);
}

/** Is the current gametype a coop multiplayer game? */
simulated function bool IsCoopMultiplayerGame()
{
	return FALSE;
}

/** Should players show gore? */
simulated event bool ShouldShowGore()
{
	return TRUE;
}

defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	bStopCountDown=true
	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=True
}
