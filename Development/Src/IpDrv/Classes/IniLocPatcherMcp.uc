/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class is the MCP specific version of the INI/Loc patcher. It reads from MCP
 * first and then uses the base class for fallback cases
 */
class IniLocPatcherMcp extends IniLocPatcher;

/** Used to look up the MCP downloader object by name */
var config name McpDownloaderName;

/** Cached object ref that we use for accessing the downloader */
var transient OnlineTitleFileDownloadMcp Downloader;

/**
 * Initializes the patcher, sets delegates, vars, etc.
 */
function Init()
{
	local OnlineSubsystem OnlineSub;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		// Ask for the interface by name
		Downloader = OnlineTitleFileDownloadMcp(OnlineSub.GetNamedInterface(McpDownloaderName));
		if (Downloader != None)
		{
			// Set the callback for notifications of files completing
			Downloader.AddReadTitleFileCompleteDelegate(OnReadFileComplete);
		}
		else
		{
			// Use the base class' version
			Super.Init();
		}
	}
}

/**
 * Reads the set of files from the online service
 */
function DownloadFiles()
{
	local int Index;

	// If there is online interface, then try to download the files
	if (Downloader != None)
	{
		// Iterate through files trying to download them
		for (Index = 0; Index < Files.Length; Index++)
		{
			// Kick off the read of that file if not already started or failed
			if (Files[Index].ReadState == OERS_NotStarted)
			{
				if (Downloader.ReadTitleFile(Files[Index].Filename))
				{
					Files[Index].ReadState = OERS_InProgress;
				}
				else
				{
					Files[Index].ReadState = OERS_Failed;
				}
			}
		}
	}
	else
	{
		// Use the base class' version
		Super.DownloadFiles();
	}
}

/**
 * Notifies us when the download of a file is complete
 *
 * @param bWasSuccessful true if the download completed ok, false otherwise
 * @param FileName the file that was downloaded (or failed to)
 */
function OnReadFileComplete(bool bWasSuccessful,string FileName)
{
	local int Index;
	local array<byte> FileData;

	// Iterate through files to verify that this is one that we requested
	for (Index = 0; Index < Files.Length; Index++)
	{
		if (Files[Index].Filename == FileName)
		{
			if (bWasSuccessful)
			{
				Files[Index].ReadState = OERS_Done;
				// Read the contents so that they can be processed
				if (Downloader.GetTitleFileContents(FileName,FileData))
				{
					ProcessIniLocFile(FileName,FileData);
				}
				else
				{
					Files[Index].ReadState = OERS_Failed;
				}
			}
			else
			{
				`Log("Failed to download the file ("$Files[Index].Filename$") from MCP downloader");
				Files[Index].ReadState = OERS_Failed;
			}
		}
	}
}

/**
 * Adds the specified delegate to the registered downloader. Since the file read can come from
 * different objects, this method hides that detail, but still lets callers get notifications
 *
 * @param ReadTitleFileCompleteDelegate the delegate to set
 */
function AddReadFileDelegate(delegate<OnReadTitleFileComplete> ReadTitleFileCompleteDelegate)
{
	if (Downloader != None)
	{
		// Add the delegate if not None
		if (ReadTitleFileCompleteDelegate != None)
		{
			// Set the callback for notifications of files completing
			Downloader.AddReadTitleFileCompleteDelegate(ReadTitleFileCompleteDelegate);
		}
	}
	else
	{
		Super.AddReadFileDelegate(ReadTitleFileCompleteDelegate);
	}
}

/**
 * Clears the specified delegate from any registered downloaders
 *
 * @param ReadTitleFileCompleteDelegate the delegate to remove from the downloader
 */
function ClearReadFileDelegate(delegate<OnReadTitleFileComplete> ReadTitleFileCompleteDelegate)
{
	if (Downloader != None)
	{
		Downloader.ClearReadTitleFileCompleteDelegate(ReadTitleFileCompleteDelegate);
	}
	else
	{
		Super.ClearReadFileDelegate(ReadTitleFileCompleteDelegate);
	}
}
