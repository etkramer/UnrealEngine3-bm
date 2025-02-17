/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Provides a mechanism for downloading arbitrary files from the MCP server
 */
class OnlineTitleFileDownloadMcp extends MCPBase
	native
	dependson(OnlineSubsystem);

/** The list of delegates to notify when a file is read */
var private array<delegate<OnReadTitleFileComplete> > ReadTitleFileCompleteDelegates;

/** The list of title files that have been read or are being read */
var private array<TitleFile> TitleFiles;

/** The class that will communicate with backend to download the file */
var private native const pointer HttpDownloader{class FHttpDownloadBinary};

/** The index of the file in the array being processed */
var transient int CurrentIndex;

/** The base URL to use when downloading files, such that BaseUrl?TitleID=1234&FileName=MyFile.ini is the complete URL */
var config string BaseUrl;

/** The amount of time to allow for downloading of the file */
var config float TimeOut;

/**
 * Delegate fired when a file read from the network platform's title specific storage is complete
 *
 * @param bWasSuccessful whether the file read was successful or not
 * @param FileName the name of the file this was for
 */
delegate OnReadTitleFileComplete(bool bWasSuccessful,string FileName);

/**
 * Starts an asynchronous read of the specified file from the network platform's
 * title specific file store
 *
 * @param FileToRead the name of the file to read
 *
 * @return true if the calls starts successfully, false otherwise
 */
native function bool ReadTitleFile(string FileToRead);

/**
 * Adds the delegate to the list to be notified when a requested file has been read
 *
 * @param ReadTitleFileCompleteDelegate the delegate to add
 */
function AddReadTitleFileCompleteDelegate(delegate<OnReadTitleFileComplete> ReadTitleFileCompleteDelegate)
{
	if (ReadTitleFileCompleteDelegates.Find(ReadTitleFileCompleteDelegate) == INDEX_NONE)
	{
		ReadTitleFileCompleteDelegates[ReadTitleFileCompleteDelegates.Length] = ReadTitleFileCompleteDelegate;
	}
}

/**
 * Removes the delegate from the notify list
 *
 * @param ReadTitleFileCompleteDelegate the delegate to remove
 */
function ClearReadTitleFileCompleteDelegate(delegate<OnReadTitleFileComplete> ReadTitleFileCompleteDelegate)
{
	local int RemoveIndex;

	RemoveIndex = ReadTitleFileCompleteDelegates.Find(ReadTitleFileCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		ReadTitleFileCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Copies the file data into the specified buffer for the specified file
 *
 * @param FileName the name of the file to read
 * @param FileContents the out buffer to copy the data into
 *
 * @return true if the data was copied, false otherwise
 */
native function bool GetTitleFileContents(string FileName,out array<byte> FileContents);

/**
 * Determines the async state of the tile file read operation
 *
 * @param FileName the name of the file to check on
 *
 * @return the async state of the file read
 */
function EOnlineEnumerationReadState GetTitleFileState(string FileName)
{
	local int FileIndex;

	FileIndex = TitleFiles.Find('FileName',FileName);
	if (FileIndex != INDEX_NONE)
	{
		return TitleFiles[FileIndex].AsyncState;
	}
	return OERS_Failed;
}

/**
 * Empties the set of downloaded files if possible (no async tasks outstanding)
 *
 * @return true if they could be deleted, false if they could not
 */
native function bool ClearDownloadedFiles();

cpptext
{
// FTickableObject interface
	/**
	 * Ticks any outstanding async tasks that need processing
	 *
	 * @param DeltaTime the amount of time that has passed since the last tick
	 */
	virtual void Tick(FLOAT DeltaTime);

// Helpers

	/**
	 * Searches the list of files for the one that matches the filename
	 *
	 * @param FileName the file to search for
	 *
	 * @return the file details
	 */
	FORCEINLINE FTitleFile* GetTitleFile(const FString& FileName)
	{
		// Search for the specified file
		for (INT Index = 0; Index < TitleFiles.Num(); Index++)
		{
			FTitleFile* TitleFile = &TitleFiles(Index);
			if (TitleFile &&
				TitleFile->Filename == FileName)
			{
				return TitleFile;
			}
		}
		return NULL;
	}

	/**
	 * Starts the next async download in the list
	 */
	void DownloadNextFile(void);

	/**
	 * Fires the delegates so the caller knows the file download is complete
	 *
	 * @param TitleFile the information for the file that was downloaded
	 */
	void TriggerDelegates(const FTitleFile* TitleFile);
}