/**
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */

/** Specialized GRI for dealing with party game informaton */
class GearPartyGRI extends GearGRI;

/** Enum of the different states in matchmaking process */
enum EMatchmakingState
{
	MMS_NotMatchmaking,
	// One or more users have requested to cancel matchmaking
	MMS_CancelingMatchmaking,
	// Reading the skill data from live so we can search using it
	MMS_ReadingSkills,
	// When you are a partial party, it searches for an exact fit for your size first
	MMS_FindingBestParty,
	// When you are a partial party and a best fit fails, it searches any party that can hold
	MMS_FindingAnyParty,
	// Once parties are full it searches for finding opponents
	MMS_FindingOpposingParty,
	// Waiting for a match as a partial party client
	MMS_WaitingForOpposingParty,
	// Past the point of no return, traveling to the party match
	MMS_ConnectingToOpposingParty,
	// The partial party host has cancelled, so we need to start looking again
	MMS_HostCancelingMatchmaking
};

/** Whether there is currently an error in advancing from the party lobby or not */
var bool bPartyLobbyErrorExists;

/** The current state of our matchmaking process */
var repnotify EMatchmakingState MatchmakingState;

/** The number of slots filled by external parties (to update the UI list with which ones are still needed) */
var repnotify int PartySlotsFilled;

/** The total number of players on a team for this game */
var repnotify int TeamSize;

/** The total number of teams invovled with this game */
var repnotify int TeamCount;

/** The number of slots filled by people that are loading into the game */
var int ConnectingPlayerCount;

replication
{
	if (bNetDirty)
		MatchmakingState, PartySlotsFilled, TeamSize, TeamCount, bPartyLobbyErrorExists, ConnectingPlayerCount;
}

/**
 * Sets the playlist that is being used and determines the team sizes
 *
 * @param NewPlaylistId the playlist to switch to
 */
function SetPlaylist(int NewPlaylistId)
{
	local OnlinePlaylistManager PlaylistMan;
	local OnlineSubsystem OnlineSub;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlaylistId = NewPlaylistId;
		// Read the playlist information
		PlaylistMan = OnlinePlaylistManager(OnlineSub.GetNamedInterface('PlaylistManager'));
		if (PlaylistMan != None)
		{
			PlaylistMan.GetTeamInfoFromPlaylist(PlaylistId,TeamSize,TeamCount);
		}
	}
}

/**
 * Does per replicated variable handling on the client
 *
 * @param VarName the variable that was just replicated
 */
simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'MatchmakingState')
	{
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

DefaultProperties
{
	PartySlotsFilled=1
}
