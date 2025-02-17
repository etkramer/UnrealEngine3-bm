/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class holds the set of playlists that the game exposes, handles
 * downloading updates to the playlists via MCP/TitleFiles, and creates the
 * game settings objects that make up a playlist
 */
class OnlinePlaylistManager extends Object
	native
	config(Playlist);

/** Contains a game settings class name to load and instance using the specified URL to override defaults */
struct native ConfiguredGameSetting
{
	/** The unique (within the playlist) id for this game setting */
	var int GameSettingId;
	/** The name of the class to load and instance */
	var string GameSettingsClassName;
	/** The URL to use to replace settings with (see UpdateFromURL()) */
	var string Url;
	/** Holds the object that was created for this entry in a playlist */
	var transient OnlineGameSettings GameSettings;
};

/** A playlist contains 1 or more game configurations that players can choose between */
struct native Playlist
{
	/** Holds the list of game configurations that are part of this playlist */
	var array<ConfiguredGameSetting> ConfiguredGames;
	/** The unique id for this playlist */
	var int PlaylistId;
	/** The string to use to lookup the display name for this playlist */
	var string LocalizationString;
	/** The set of content/maps (or DLC bundles) that must be present in order to play on this playlist */
	var array<int> ContentIds;
	/** The number of players per team if different from the defaults */
	var int TeamSize;
	/** The number of teams per match if different from the defaults */
	var int TeamCount;
	/** The string to use in the UI for this playlist */
	var string Name;
};

/** This is the complete set of playlists available to choose from */
var config array<Playlist> Playlists;

/** The file names to request when downloading a playlist from MCP/TMS/etc */
var array<string> PlaylistFileNames;

/** The set of UIDataStore_GameResource objects to refresh once the download has completed */
var config array<name> DatastoresToRefresh;

/** Used to know when we should finalize the objects */
var int DownloadCount;

/** Incremented when successful to determine whether to update at all */
var int SuccessfulCount;

/**
 * Delegate fired when the playlist has been downloaded and processed
 */
delegate OnReadPlaylistComplete();

/**
 * Reads the playlist from either MCP or from some form of title storage
 */
function DownloadPlaylist()
{
	local OnlineSubsystem OnlineSub;
	local int FileIndex;

	if (SuccessfulCount == 0)
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None &&
			OnlineSub.Patcher != None)
		{
			if (PlaylistFileNames.Length == 0)
			{
				// Request notification of file downloads
				OnlineSub.Patcher.AddReadFileDelegate(OnReadTitleFileComplete);
				// Reset our counts since we might be re-downloading
				DownloadCount = 0;
				SuccessfulCount = 0;
				// Don't rebuild the list of files when already there
				if (PlaylistFileNames.Length == 0)
				{
					DetermineFilesToDownload();
				}
				// Iterate the files that we read and request them
				for (FileIndex = 0; FileIndex < PlaylistFileNames.Length; FileIndex++)
				{
					OnlineSub.Patcher.AddFileToDownload(PlaylistFileNames[FileIndex]);
				}
			}
			else
			{
				// Notify the completion
				OnReadPlaylistComplete();
			}
		}
		else
		{
			`Log("No online layer present, using defaults for playlist");
		}
	}
}

/** Uses the current loc setting and game ini name to build the download list */
native function DetermineFilesToDownload();

/**
 * Notifies us when the download of the playlist file is complete
 *
 * @param bWasSuccessful true if the download completed ok, false otherwise
 * @param FileName the file that was downloaded (or failed to)
 */
function OnReadTitleFileComplete(bool bWasSuccessful,string FileName)
{
	local OnlineSubsystem OnlineSub;
	local int FileIndex;

	for (FileIndex = 0; FileIndex < PlaylistFileNames.Length; FileIndex++)
	{
		if (PlaylistFileNames[FileIndex] == FileName)
		{
			// Increment how many we've downloaded
			DownloadCount++;
			SuccessfulCount += int(bWasSuccessful);
			// If they have all been downloaded, rebuild the playlist
			if (DownloadCount == PlaylistFileNames.Length)
			{
				if (SuccessfulCount != DownloadCount)
				{
					`Log("PlaylistManager: not all files downloaded correctly, using defaults where applicable");
				}
				// Rebuild the playlist and update any objects/ui
				FinalizePlaylistObjects();
				// Notify our requester
				OnReadPlaylistComplete();

				// Remove the delegates since we are done
				OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
				if (OnlineSub != None &&
					OnlineSub.Patcher != None)
				{
					OnlineSub.Patcher.ClearReadFileDelegate(OnReadTitleFileComplete);
				}
			}
		}
	}
}

/**
 * Uses the configuration data to create the requested objects and then applies any
 * specific game settings changes to them
 */
native function FinalizePlaylistObjects();

/**
 * Finds the game settings object associated with this playlist and game settings id
 *
 * @param PlaylistId the playlist we are searching
 * @param GameSettingsId the game settings id being searched for
 *
 * @return the game settings specified or None if not found
 */
function OnlineGameSettings GetGameSettings(int PlaylistId,int GameSettingsId)
{
	local int PlaylistIndex;
	local int GameIndex;

	// Find the matching playlist
	for (PlaylistIndex = 0; PlaylistIndex < Playlists.Length; PlaylistIndex++)
	{
		if (Playlists[PlaylistIndex].PlaylistId == PlaylistId)
		{
			// Search through the registered games for this playlist
			for (GameIndex = 0; GameIndex < Playlists[PlaylistIndex].ConfiguredGames.Length; GameIndex++)
			{
				if (Playlists[PlaylistIndex].ConfiguredGames[GameIndex].GameSettingId == GameSettingsId)
				{
					return Playlists[PlaylistIndex].ConfiguredGames[GameIndex].GameSettings;
				}
			}
		}
	}
	return None;
}

/**
 * Finds the team information for the specified playlist and returns it in the out vars
 *
 * @param PlaylistId the playlist being searched for
 * @param TeamSize out var getting the number of players per team
 * @param TeamCount out var getting the number of teams per match
 */
function GetTeamInfoFromPlaylist(int PlaylistId,out int TeamSize,out int TeamCount)
{
	local int PlaylistIndex;

	// Find the matching playlist
	for (PlaylistIndex = 0; PlaylistIndex < Playlists.Length; PlaylistIndex++)
	{
		if (Playlists[PlaylistIndex].PlaylistId == PlaylistId)
		{
			TeamSize = Playlists[PlaylistIndex].TeamSize;
			TeamCount = Playlists[PlaylistIndex].TeamCount;
			return;
		}
	}
	// Not found so set to invalid numbers
	TeamSize = 0;
	TeamCount = 0;
}

/**
 * Finds the specified playlist and return the content ids in the out var
 *
 * @param PlaylistId the playlist being searched for
 * @param ContentIds the list to set the content ids in
 */
function GetContentIdsFromPlaylist(int PlaylistId,out array<int> ContentIds)
{
	local int PlaylistIndex, ContentIdx;

	// Find the matching playlist
	for (PlaylistIndex = 0; PlaylistIndex < Playlists.Length; PlaylistIndex++)
	{
		if (Playlists[PlaylistIndex].PlaylistId == PlaylistId)
		{
			for (ContentIdx = 0; ContentIdx < Playlists[PlaylistIndex].ContentIds.length; ContentIdx++)
			{
				ContentIds.AddItem(Playlists[PlaylistIndex].ContentIds[ContentIdx]);
			}
			return;
		}
	}
}
