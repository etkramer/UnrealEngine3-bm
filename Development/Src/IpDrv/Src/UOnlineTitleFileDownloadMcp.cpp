/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnIpDrv.h"

#include "HTTPDownload.h"

IMPLEMENT_CLASS(UOnlineTitleFileDownloadMcp);

/**
 * Ticks any http requests that are in flight
 *
 * @param DeltaTime the amount of time that has passed since the last tick
 */
void UOnlineTitleFileDownloadMcp::Tick(FLOAT DeltaTime)
{
	// Nothing to tick if we don't have a downloader or the index isn't valid
	if (HttpDownloader &&
		TitleFiles.IsValidIndex(CurrentIndex))
	{
		FTitleFile& TitleFile = TitleFiles(CurrentIndex);
		// Tick the task and check for timeout
		HttpDownloader->Tick(DeltaTime);
		// See if we are done
		if (HttpDownloader->GetHttpState() == HTTP_Closed)
		{
			HttpDownloader->GetBinaryData(TitleFile.Data);
			TitleFile.AsyncState = OERS_Done;
			delete HttpDownloader;
			HttpDownloader = NULL;
		}
		// Or are in error
		else if (HttpDownloader->GetHttpState() == HTTP_Error)
		{
			// Failed zero everything
			TitleFile.AsyncState = OERS_Failed;
			TitleFile.Data.Empty();
			delete HttpDownloader;
			HttpDownloader = NULL;
		}
		// Trigger the delegate for this one if done
		if (TitleFile.AsyncState != OERS_InProgress)
		{
			TriggerDelegates(&TitleFile);
			// Now we need to process the next file
			DownloadNextFile();
		}
	}
}

/**
 * Starts an asynchronous read of the specified file from the network platform's
 * title specific file store
 *
 * @param FileToRead the name of the file to read
 *
 * @return true if the calls starts successfully, false otherwise
 */
UBOOL UOnlineTitleFileDownloadMcp::ReadTitleFile(const FString& FileToRead)
{
	DWORD Result = E_FAIL;
	if (FileToRead.Len())
	{
		// See if we have added the file to be processed
		FTitleFile* TitleFile = GetTitleFile(FileToRead);
		if (TitleFile)
		{
			// Determine if it's done, in error, or in progress
			// and handle the delegate based off of that
			if (TitleFile->AsyncState == OERS_Done)
			{
				Result = ERROR_SUCCESS;
			}
			// If it hasn't started or is in progress, then mark as pending
			else if (TitleFile->AsyncState != OERS_Failed)
			{
				Result = ERROR_IO_PENDING;
			}
		}
		else
		{
			// Add this file to the list to process
			INT AddIndex = TitleFiles.AddZeroed();
			TitleFile = &TitleFiles(AddIndex);
			TitleFile->Filename = FileToRead;
			// Create the downloader object if needed
			if (HttpDownloader == NULL)
			{
				DownloadNextFile();
			}
			Result = ERROR_IO_PENDING;
		}
	}
	else
	{
		debugf(NAME_DevOnline,TEXT("ReadTitleFile() failed due to empty filename"));
	}
	if (Result != ERROR_IO_PENDING)
	{
		TriggerDelegates(GetTitleFile(FileToRead));
	}
	return Result == ERROR_SUCCESS || Result == ERROR_IO_PENDING;
}

/**
 * Copies the file data into the specified buffer for the specified file
 *
 * @param FileName the name of the file to read
 * @param FileContents the out buffer to copy the data into
 *
 * @return true if the data was copied, false otherwise
 */
UBOOL UOnlineTitleFileDownloadMcp::GetTitleFileContents(const FString& FileName,TArray<BYTE>& FileContents)
{
	// Search for the specified file and return the raw data
	FTitleFile* TitleFile = GetTitleFile(FileName);
	if (TitleFile)
	{
		FileContents = TitleFile->Data;
		return TRUE;
	}
	return FALSE;
}

/**
 * Empties the set of downloaded files if possible (no async tasks outstanding)
 *
 * @return true if they could be deleted, false if they could not
 */
UBOOL UOnlineTitleFileDownloadMcp::ClearDownloadedFiles(void)
{
	for (INT Index = 0; Index < TitleFiles.Num(); Index++)
	{
		FTitleFile& TitleFile = TitleFiles(Index);
		// If there is an async task outstanding, fail to empty
		if (TitleFile.AsyncState == OERS_InProgress)
		{
			return FALSE;
		}
	}
	// No async files being handled, so empty them all
	TitleFiles.Empty();
	delete HttpDownloader;
	HttpDownloader = NULL;
	return TRUE;
}

/**
 * Starts the next async download in the list
 */
void UOnlineTitleFileDownloadMcp::DownloadNextFile(void)
{
	// Iterates the list of files searching for ones that need to be downloaded
	for (CurrentIndex = 0; CurrentIndex < TitleFiles.Num(); CurrentIndex++)
	{
		FTitleFile& TitleFile = TitleFiles(CurrentIndex);
		if (TitleFile.AsyncState == OERS_NotStarted)
		{
			FResolveInfo* ResolveInfo = NULL;
			// Build an url from the string
			FURL Url(NULL,*BaseUrl,TRAVEL_Absolute);
			// See if we need to resolve this string
			UBOOL bIsValidIp = FInternetIpAddr::IsValidIp(*Url.Host);
			if (bIsValidIp == FALSE)
			{
				// Allocate a platform specific resolver and pass that in
				ResolveInfo = GSocketSubsystem->GetHostByName(TCHAR_TO_ANSI(*Url.Host));
			}
			const FString GetParams = FString::Printf(TEXT("TitleID=%d&PlatformID=%d&Filename=%s"),
				appGetTitleId(),
				(DWORD)appGetPlatformType(),
				*TitleFile.Filename);
			// Create the new downloader object
			HttpDownloader = new FHttpDownloadBinary(TimeOut,GetParams,ResolveInfo);
			// This is the one to start downloading
			HttpDownloader->DownloadUrl(Url);
			TitleFile.AsyncState = OERS_InProgress;
			return;
		}
	}
}

/**
 * Fires the delegates so the caller knows the file download is complete
 *
 * @param TitleFile the information for the file that was downloaded
 */
void UOnlineTitleFileDownloadMcp::TriggerDelegates(const FTitleFile* TitleFile)
{
	if (TitleFile)
	{
		OnlineTitleFileDownloadMcp_eventOnReadTitleFileComplete_Parms Parms(EC_EventParm);
		Parms.bWasSuccessful = TitleFile->AsyncState == OERS_Done ? FIRST_BITFIELD : 0;
		Parms.Filename = TitleFile->Filename;
		TriggerOnlineDelegates(this,ReadTitleFileCompleteDelegates,&Parms);
	}
}
