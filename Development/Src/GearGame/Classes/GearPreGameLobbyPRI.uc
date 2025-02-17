/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPreGameLobbyPRI extends GearPartyPRI;

/** Whether this player has requested a team change or not */
var		bool		bRequestedTeamChange;

/**
 * tracks whether this player has opened the lobby scene;  the countdown only starts once all active players have opened the lobby scene
 */
var		bool		bLobbySceneOpened;

/**
 * Tracks whether this player has submitted their map vote.
 */
var		bool		bSubmittedMapVote;

/**
 * Tracks whether this player has submitted their game vote.
 */
var		bool		bSubmittedGameVote;

/**
 * Tracks whether the player is happy with their selection of character and/or weapon (a.k.a ready)
 */
var		bool		bReady;


/** Value of GetTeamNum() the last time the PreGameLobby checked for it */
var int LastCheckedTeamNum;


replication
{
	if ( Role == ROLE_Authority && bNetDirty )
		bRequestedTeamChange;
}

/**
 * Entry point for submitting a map vote to the match host.
 *
 * @param	MapIndex	0 to vote for the first map, 1 to vote for the second map.
 */
simulated function SubmitMapVote( int MapIndex )
{
	if ( !bSubmittedMapVote )
	{
		if ( MapIndex == 0 || MapIndex == 1 )
		{
			bSubmittedMapVote = true;
			ServerSubmitMapVote(MapIndex);
		}
	}
}

/**
 * Entry point for submitting a map vote to the match host.
 *
 * @param	MapIndex	0 to vote for the first game, 1 to vote for the second game.
 */
simulated function SubmitGameVote( int GameIndex )
{
	if ( !bSubmittedGameVote )
	{
		if ( GameIndex == 0 || GameIndex == 1 )
		{
			bSubmittedGameVote = true;
			ServerSubmitGameVote(GameIndex);
		}
	}
}

/**
 * Entry point for readying up during character selection.
 */
simulated function SubmitReady()
{
	if ( !bReady )
	{
		bReady = true;
		ServerSubmitReady();
	}
}

/**
 * RPC for SubmitMapVote.  Executes on the server and submits the player's choice to
 * the host's GameReplicationInfo
 *
 * @param	MapIndex	0 to vote for the first map, 1 to vote for the second map.
 */
reliable server protected function ServerSubmitMapVote( int MapIndex )
{
	local GearPreGameGRI PartyGRI;

	PartyGRI = GearPreGameGRI(WorldInfo.GRI);
	if ( PartyGRI != None )
	{
		PartyGRI.SubmitMapVote(Self, MapIndex);
	}
}

/**
 * RPC for SubmitGameVote.  Executes on the server and submits the player's choice to
 * the host's GameReplicationInfo
 *
 * @param	GameIndex	0 to vote for the first game, 1 to vote for the second game.
 */
reliable server protected function ServerSubmitGameVote( int GameIndex )
{
	local GearPreGameGRI PartyGRI;

	PartyGRI = GearPreGameGRI(WorldInfo.GRI);
	if ( PartyGRI != None )
	{
		PartyGRI.SubmitGameVote(Self, GameIndex);
	}
}

/**
 * RPC for SubmitReady.  Executes on the server and submits the ready status to the 
 * host's GameReplication Info
 */
reliable server protected function ServerSubmitReady()
{
	local GearPreGameGRI PartyGRI;

	PartyGRI = GearPreGameGRI(WorldInfo.GRI);
	if ( PartyGRI != None )
	{
		PartyGRI.SubmitReady(Self);
	}
}

/**
 * Notifies the server that this player has opened the lobby scene.
 */
reliable server function ServerNotifyLobbySceneOpened()
{
	bLobbySceneOpened = true;
}

/* === PlayerReplicationInfo interface === */
/**
 * Copy properties from a player's initially spawned PRI into a PRI pulled from the list of inactive PRIs.
 *
 * @param	PRI	the PRI the player originally spawned with.
 */
function OverrideWith(PlayerReplicationInfo PRI)
{
	local GearPreGameLobbyPRI LobbyPRI;

	Super.OverrideWith(PRI);

	LobbyPRI = GearPreGameLobbyPRI(PRI);
	if ( LobbyPRI != None )
	{
		bLobbySceneOpened = LobbyPRI.bLobbySceneOpened;
	}
}

function bool IsFullyInitialized()
{
	local UniqueNetId ZeroId;

	return bLobbySceneOpened && !class'OnlineSubsystem'.static.AreUniqueNetIdsEqual(UniqueId, ZeroId);
}

/**
 * The base implementation registers the player with the online session so that
 * recent players list and session counts are updated.
 */
simulated function RegisterPlayerWithSession()
{
	local OnlineSubsystem OnlineSub;
	local UniqueNetId ZeroId;

	Super.RegisterPlayerWithSession();

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	// If this is a system link session then re-register with the party session
	if (OnlineSub != None &&
		OnlineSub.GameInterface != None &&
		UniqueId != ZeroId &&
		OnlineSub.GameInterface.GetGameSettings('Party') != None &&
		OnlineSub.GameInterface.GetGameSettings('Party').bIsLanMatch)
	{
		OnlineSub.GameInterface.RegisterPlayer('Party',UniqueId,false);
	}
}

/**
 * This version ignores the PRI destruction if the player leaves after arbitration
 * has completed since the PRI from travelling will handle that
 */
simulated function UnregisterPlayerFromSession()
{
}


simulated function bool ShouldBroadCastWelcomeMessage(optional bool bExiting)
{
	return FALSE;
}

defaultproperties
{
	// This class operates as if the "game" session is the one being manipulated
	SessionName="Game"

	LastCheckedTeamNum=255
}
