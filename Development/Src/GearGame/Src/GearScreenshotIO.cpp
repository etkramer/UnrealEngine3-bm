/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

#include "GearGame.h"

#if _XBOX

#include "UnIpDrv.h"

#define RANDOM_SCREENSHOT_VALUES 0

namespace
{
	/** Root name used for saving/loading files.  This is just a symbolic name for whichever device we are using for IO. */
	static const ANSICHAR* RootName = "screenshot";

	/** Names of the files we store for each screenshot. */
	static const ANSICHAR* ImageFileName = "screenshot:\\image.jpg";
	static const ANSICHAR* InfoFileName = "screenshot:\\info";

	/** These are for the content files that contain the actual files. */
	static const ANSICHAR* FileNamePrefix = "shot_";
	static const FString   FileNamePrefixString(FileNamePrefix);
	static const FString   DisplayNameSeparator(", ");

	/** This is the number of screenshots to get at a time when enumerating. */
	static const DWORD     EnumerationCount = 1000;

	/** This is the NULL of DeviceIDs. */
	static const DWORD     InvalidDeviceID = (DWORD)-1;

	/** This is the size set aside for info being dumped to disk. */
	//TODO: is this a good estimate?  larger/smaller?  make the archive class automatically resize?
	static const DWORD     MaxInfoSize = (16 * 1024);

	/** This is the version number for the screenshot info. */
	static const DWORD     InfoVersion = 5;

	/** Content type of saved content */
	static const DWORD     ContentType = XCONTENTTYPE_SAVEDGAME;
}

static FString& GetRatingSuffix()
{
	static FString RatingSuffix;
	if(RatingSuffix.Len() == 0)
	{
		RatingSuffix = TEXT(" ");
		RatingSuffix += Localize(TEXT("Screenshots"), TEXT("Score"), TEXT("GearGame"));
	}
	return RatingSuffix;
}

static FString& GetDisplayNamePrefix()
{
	static FString DisplayNamePrefix;
	if(DisplayNamePrefix.Len() == 0)
	{
		DisplayNamePrefix = Localize(TEXT("Screenshots"), TEXT("Photo"), TEXT("GearGame"));
	}
	return DisplayNamePrefix;
}

static FString& GetGameToMapConjunction()
{
	static FString GameToMapConjunction;
	if(GameToMapConjunction.Len() == 0)
	{
		GameToMapConjunction = TEXT(" ");
		GameToMapConjunction += Localize(TEXT("GearUIScene_Base"), TEXT("GameToMapConjunction"), TEXT("GearGame"));
		GameToMapConjunction += TEXT(" ");
	}
	return GameToMapConjunction;
}

/**
 * This class is used to time some of what we do.
 * The functionality is similar to FScopeSecondsCounter.
 * But this class doesn't require you to provide a DOUBLE, and it automatically prints the results.
 */
class Stopwatch
{
public:
	Stopwatch(FString InName) :
		Name(InName),
		StartTime(appSeconds()),
		bIsStopped(FALSE)
	{
	}
	void Stop()
	{
		debugf(TEXT("%s: %fs"), *Name, appSeconds() - StartTime);
		bIsStopped = TRUE;
	}
	~Stopwatch()
	{
		if(!bIsStopped)
		{
			Stop();
		}
	}
private:
	FString Name;
	DOUBLE StartTime;
	UBOOL bIsStopped;
};

/**
 * Helper class for screenshot IO.
 */
class ScreenshotIOHelper
{
public:
	/**
	* Writes an individual data file to disk.
	*/
	static UBOOL WriteFile(const ANSICHAR* FileName, DWORD Size, BYTE* Data)
	{
		UBOOL Result = FALSE;
		// Open the file within the virtual device
		HANDLE FileHandle = CreateFile(FileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,NULL);
		if (FileHandle != INVALID_HANDLE_VALUE)
		{
			DWORD BytesWritten;
			// Write our data to the file
			if (::WriteFile(FileHandle,Data,Size,&BytesWritten,NULL) != 0 &&
				Size == BytesWritten)
			{
				Result = TRUE;
			}
			else
			{
				debugf(TEXT("WriteFile(%s): WriteFile failed. Error = 0x%08X"),
					FileName, GetLastError());
			}
			CloseHandle(FileHandle);
		}
		else
		{
			debugf(TEXT("WriteFile(%s): CreateFile failed. Error = 0x%08X"),
				FileName, GetLastError());
		}
		return Result;
	}

	/** Reads a file from disk. */
	static UBOOL ReadFile(const ANSICHAR* FileName, TArray<BYTE>& Buffer)
	{
		UBOOL Result = FALSE;
		HANDLE FileHandle = CreateFile( FileName, GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if( FileHandle != INVALID_HANDLE_VALUE )
		{
			// Get the file's size.
			DWORD FileSize = GetFileSize(FileHandle, NULL);
			if(FileSize != -1)
			{
				// Allocate enough bytes to hold the file.
				Buffer.Empty(FileSize);
				Buffer.Add(FileSize);

				// Read data from the file.
				DWORD BytesRead;
				if( ::ReadFile( FileHandle, (VOID*)Buffer.GetData(), FileSize, &BytesRead, NULL) == 0 )
				{
					debugf( TEXT("ScreenshotIOHelper::ReadFile: ReadFile failed. Error = %08x\n"), GetLastError() );
				}
				else
				{
					// Make sure we read the entire file.
					check(FileSize==BytesRead);
					Result = TRUE;
				}
			}
			else
			{
				debugf( TEXT("ScreenshotIOHelper::ReadFile: GetFileSize failed. Error = %08x\n"), GetLastError() );
			}

			CloseHandle( FileHandle );
		}
		else
		{
			debugf( TEXT("ScreenshotIOHelper::ReadFile: CreateFile failed. Error = %08x"), GetLastError());
		}
		return Result;
	}

#if RANDOM_SCREENSHOT_VALUES
	static const FString& RandomString()
	{
		const FString CharList = TEXT("abcdefghijklmnopqrstuvwxyz");
		const INT MinLen = 5;
		const INT MaxLen = 12;
		static FString OutString;
		OutString.Empty(MaxLen);
		INT Len = MinLen + INT(appFrand() * (MaxLen - MinLen));
		for(INT Index = 0; Index < Len; Index++)
		{
			OutString += CharList[appFrand() * CharList.Len()];
		}
		return OutString;
	}
#endif
private:
	ScreenshotIOHelper() {}
};

/**
* Performs a screenshot save in an async fashion using the thread pool
*/
class FAsyncScreenshotSave : public FQueuedWork
{
	/** Holds the descriptor data to use for saving */
	XCONTENT_DATA ContentData;
	/** Which player is saving the data */
	DWORD PlayerId;
	/** How large the image is in bytes */
	DWORD ImageSize;
	/** Holds a copy of the image that was asked to be written */
	BYTE* Image;
	/** How large the thumbnail is in bytes */
	DWORD ThumbnailSize;
	/** Holds a copy of the thumbnail */
	BYTE* Thumbnail;
	/** Holds a copy of the info that was asked to be written */
	FNboSerializeToBuffer InfoArchive;
	/** Holds the state of the save */
	volatile INT* SaveState;

public:
	/**
	* Allocates and copies the information needed to do the save
	*
	* @param InContentData the descriptor to copy with
	* @param InPlayerId the player that the save is happening for
	* @param InImageSize the size of the image to write
	* @param InImage the image to copy and later write
	* @param InThumbnailSize the size of the thumbnail
	* @param InThumbnail the thumbnail to copy
	* @param InInfo the info to copy and later write
	* @param InSaveState the state of the save attempt
	*/
	FAsyncScreenshotSave(const XCONTENT_DATA* InContentData,DWORD InPlayerId,
		DWORD InImageSize,const BYTE* InImage,
		DWORD InThumbnailSize,const BYTE* InThumbnail,
		const FScreenshotInfo& InInfo,
		volatile INT* InSaveState) :
		PlayerId(InPlayerId),
		ImageSize(InImageSize), Image(NULL),
		ThumbnailSize(InThumbnailSize), Thumbnail(NULL),
		InfoArchive(MaxInfoSize),
		SaveState(InSaveState)
	{
		appMemcpy(&ContentData,InContentData,sizeof(XCONTENT_DATA));
		Image = new BYTE[ImageSize];
		appMemcpy(Image,InImage,ImageSize);
		Thumbnail = new BYTE[ThumbnailSize];
		appMemcpy(Thumbnail,InThumbnail,ThumbnailSize);
		SerializeInfo(InInfo);
	}

	/** Frees the copied data */
	~FAsyncScreenshotSave()
	{
		delete Image;
		delete Thumbnail;
	}

	//FQueuedWork interface

	/**
	* Performs the async save game write
	*/
	virtual void DoWork(void)
	{
		Stopwatch Timer(TEXT("SaveScreenshot"));
		eAsyncTaskResult SaveResult = ATR_Failed;
		// Mount the device specified
		DWORD Result = XContentCreate(PlayerId,RootName,&ContentData,
			XCONTENTFLAG_CREATENEW | XCONTENTFLAG_NOPROFILE_TRANSFER,NULL,NULL,NULL);
		if (Result == ERROR_SUCCESS)
		{
			UBOOL WroteBothFiles = FALSE;
			if(ScreenshotIOHelper::WriteFile(InfoFileName, InfoArchive.GetByteCount(), (BYTE*)InfoArchive.GetBuffer().GetData()))
			{
				if(ScreenshotIOHelper::WriteFile(ImageFileName, ImageSize, Image))
				{
					WroteBothFiles = TRUE;
				}
			}
			Result = XContentClose(RootName,NULL);
			if (Result != ERROR_SUCCESS)
			{
				debugf(TEXT("FAsyncScreenshotSave::DoWork(): XContentClose() failed with 0x%08X"),
					Result);
			}
			if(WroteBothFiles)
			{
				// try to set a thumbnail
				if(Thumbnail && (ThumbnailSize > 0))
				{
					Result = XContentSetThumbnail(PlayerId,&ContentData,Thumbnail, ThumbnailSize, NULL);
					if(Result != ERROR_SUCCESS)
					{
						debugf(TEXT("FAsyncScreenshotSave::DoWork(): XContentSetThumbnail() failed with 0x%08X"),
							Result);
					}
				}
				SaveResult = ATR_Succeeded;
				debugf(TEXT("FAsyncScreenshotSave successfully saved the screenshot"));
			}
			else
			{
				// we didn't successfully write both files, so delete this content
				XContentDelete(PlayerId,&ContentData,NULL);
			}
		}
		else
		{
			debugf(TEXT("FAsyncScreenshotSave::DoWork(): XContentCreate() failed with 0x%08X"),
				Result);
		}
		appInterlockedExchange(SaveState, SaveResult);
	}

	/**
	* Ignored
	*/
	virtual void Abandon(void)
	{
	}

	/**
	* Cleans up the object
	*/ 
	virtual void Dispose(void)
	{
		delete this;
	}

private:
	/**
	 * Stores the screenshot's info in an archive.
	 * If anything in here changes, the InfoVersion number should be changed.
	 */
	void SerializeInfo(const FScreenshotInfo& Info)
	{
		InfoArchive << InfoVersion;
		InfoArchive << Info.WorldTime;
		InfoArchive << Info.Description;
		InfoArchive << Info.Rating;
		InfoArchive << Info.GameType;
		InfoArchive << Info.GameTypeFriendly;
		InfoArchive << Info.GameTypeId;
		InfoArchive << Info.MatchType;
		InfoArchive << Info.MapName;
		InfoArchive << Info.MapNameFriendly;
		InfoArchive << Info.Realtime.A << Info.Realtime.B;
		InfoArchive << Info.MatchID.A << Info.MatchID.B << Info.MatchID.C << Info.MatchID.D;
		InfoArchive << Info.CamLoc.X << Info.CamLoc.Y << Info.CamLoc.Z;
		InfoArchive << Info.CamRot.Pitch << Info.CamRot.Yaw << Info.CamRot.Roll;
		InfoArchive << Info.Width;
		InfoArchive << Info.Height;
		INT NumPlayers = Info.Players.Num();
		InfoArchive << NumPlayers;
		for(INT Index = 0 ; Index < NumPlayers; Index++)
		{
			const FScreenshotPlayerInfo& Player = Info.Players(Index);
			InfoArchive << Player.Nick;
			InfoArchive << Player.Xuid.Uid;
			InfoArchive << (DWORD)Player.IsVisible;
			InfoArchive << Player.ScreenSpaceBoundingBox.Min.X << Player.ScreenSpaceBoundingBox.Min.Y;
			InfoArchive << Player.ScreenSpaceBoundingBox.Max.X << Player.ScreenSpaceBoundingBox.Max.Y;
			InfoArchive << Player.Location.X << Player.Location.Y << Player.Location.Z;
		}
	}
};

/**
* Performs a screenshot load in an async fashion using the thread pool
*/
class FAsyncScreenshotLoad : public FQueuedWork
{
public:
	/** Holds the descriptor data to use for loading */
	XCONTENT_DATA ContentData;
	/** The player loading the screenshot */
	DWORD PlayerId;
	/** The ID of the screenshot to load */
	FGuid ID;
	/** The screenshot's JPG will be stored here */
	TArray<BYTE>* Image;
	/** The screenshot's info will be stored here */
	FScreenshotInfo* Info;
	/** Holds the state of the load */
	volatile INT* LoadState;

	/**
	* Allocates and copies the information needed to do the load
	*/
	FAsyncScreenshotLoad(const XCONTENT_DATA* InContentData, DWORD InPlayerId, const FGuid& InID,
		TArray<BYTE>* InImage, FScreenshotInfo* InInfo,
		volatile INT* InLoadState) :
		PlayerId(InPlayerId), ID(InID),
		Image(InImage), Info(InInfo),
		LoadState(InLoadState)
	{
		appMemcpy(&ContentData,InContentData,sizeof(XCONTENT_DATA));
	}

	//FQueuedWork interface

	/**
	* Performs the async load
	*/
	virtual void DoWork(void)
	{
		check(Image && Info);
		eAsyncTaskResult LoadResult = ATR_Failed;
		Stopwatch Timer(TEXT("LoadScreenshot"));

		// Mount the device specified.
		DWORD Result = XContentCreate( PlayerId, RootName, &ContentData,
			XCONTENTFLAG_OPENEXISTING | XCONTENTFLAG_NOPROFILE_TRANSFER, NULL, NULL, NULL );
		if(Result == ERROR_SUCCESS)
		{
			// Read the info.
			TArray<BYTE> InfoBuffer;
			if(ScreenshotIOHelper::ReadFile(InfoFileName, InfoBuffer))
			{
				FNboSerializeFromBuffer InfoReader(InfoBuffer.GetData(), InfoBuffer.Num());
				if(SerializeInfo(InfoReader, *Info))
				{
					// Read the image.
					if(ScreenshotIOHelper::ReadFile(ImageFileName, *Image))
					{
						LoadResult = ATR_Succeeded;
					}
					else
					{
						debugf(NAME_Error,TEXT("FAsyncScreenshotLoad::DoWork(): ReadFile(%s) failed: %d"), ImageFileName, Result);
					}
				}
				else
				{
					debugf(NAME_Error,TEXT("FAsyncScreenshotLoad::DoWork(): SerializeInfo() failed: %d"), Result);
				}
			}
			else
			{
				debugf(NAME_Error,TEXT("FAsyncScreenshotLoad::DoWork(): ReadFile(%s) failed: %d"), InfoFileName, Result);
			}
			XContentClose( RootName, NULL );
		}
		else
		{
			debugf(NAME_Error,TEXT("FAsyncScreenshotLoad::DoWork(): XContentCreate() failed: %d"), Result);
		}

		appInterlockedExchange(LoadState, LoadResult);
	}

	/**
	* Ignored
	*/
	virtual void Abandon(void)
	{
	}

	/**
	* Cleans up the object
	*/ 
	virtual void Dispose(void)
	{
		delete this;
	}

private:
	/**
	 * Reads a screenshot's info from a buffer.
	 * If anything in here changes, the InfoVersion number should be changed.
	 */
	static UBOOL SerializeInfo(FNboSerializeFromBuffer& Buffer, FScreenshotInfo& Info)
	{
		appMemzero(&Info, sizeof(FScreenshotInfo));
		DWORD Version;
		Buffer >> Version;
		if(Version != InfoVersion)
		{
			return FALSE;
		}
		Buffer >> Info.WorldTime;
		Buffer >> Info.Description;
		Buffer >> Info.Rating;
		Buffer >> Info.GameType;
		Buffer >> Info.GameTypeFriendly;
		Buffer >> Info.GameTypeId;
		Buffer >> Info.MatchType;
		Buffer >> Info.MapName;
		Buffer >> Info.MapNameFriendly;
		Buffer >> Info.Realtime.A >> Info.Realtime.B;
		Buffer >> Info.MatchID.A >> Info.MatchID.B >> Info.MatchID.C >> Info.MatchID.D;
		Buffer >> Info.CamLoc.X >> Info.CamLoc.Y >> Info.CamLoc.Z;
		Buffer >> Info.CamRot.Pitch >> Info.CamRot.Yaw >> Info.CamRot.Roll;
		Buffer >> Info.Width;
		Buffer >> Info.Height;
		INT NumPlayers = Info.Players.Num();
		check(NumPlayers >= 0 && NumPlayers < 100);
		Buffer >> NumPlayers;
		Info.Players.Empty(NumPlayers);
		Info.Players.AddZeroed(NumPlayers);
		for(INT Index = 0 ; Index < NumPlayers; Index++)
		{
			FScreenshotPlayerInfo& Player = Info.Players(Index);
			Buffer >> Player.Nick;
			Buffer >> Player.Xuid.Uid;
			DWORD IsVisible;
			Buffer >> IsVisible;
			Player.IsVisible = IsVisible;
			Buffer >> Player.ScreenSpaceBoundingBox.Min.X >> Player.ScreenSpaceBoundingBox.Min.Y;
			Buffer >> Player.ScreenSpaceBoundingBox.Max.X >> Player.ScreenSpaceBoundingBox.Max.Y;
			Buffer >> Player.Location.X >> Player.Location.Y >> Player.Location.Z;
		}
		return TRUE;
	}
};

/**
* Performs a screenshot enumeration in an async fashion using the thread pool
*/
class FAsyncScreenshotEnumerate : public FQueuedWork
{
public:
	/** The player enumerating screenshots */
	DWORD PlayerId;
	/** The screenshots' info will be stored here */
	TArray<FSavedScreenshotInfo>* Screenshots;
	/** Holds the state of the enumeration */
	volatile INT* EnumerationState;

	/**
	* Sets up the enumeration
	*/
	FAsyncScreenshotEnumerate(DWORD InPlayerId, TArray<FSavedScreenshotInfo>* InScreenshots, volatile INT* InEnumerationState) :
		PlayerId(InPlayerId), Screenshots(InScreenshots), EnumerationState(InEnumerationState)
	{
	}

	//FQueuedWork interface

	/**
	* Performs the async enumeration
	*/
	virtual void DoWork(void)
	{
		check(Screenshots);
		eAsyncTaskResult EnumerationResult = ATR_Failed;
		Stopwatch Timer(TEXT("EnumerateScreenshots"));

		Screenshots->Empty();

		DWORD DataSize;
		HANDLE EnumHandle;
		DWORD Result = XContentCreateEnumerator(PlayerId,
			XCONTENTDEVICE_ANY,
			ContentType,
			0,
			EnumerationCount,
			&DataSize,
			&EnumHandle);
		if(Result == ERROR_SUCCESS)
		{
			check(DataSize % sizeof(XCONTENT_DATA) == 0);
			XCONTENT_DATA* Data = (XCONTENT_DATA*)appMalloc(DataSize);
			DWORD NumReturned;
			do
			{
				Result = XEnumerate(EnumHandle, Data, DataSize, &NumReturned, NULL);
				if(Result == ERROR_SUCCESS)
				{
					debugf(TEXT("FAsyncScreenshotEnumerate::DoWork(): XEnumerate returned %d items."), NumReturned);
					Screenshots->Reserve(Screenshots->Num() + NumReturned);
					for(DWORD Index = 0; Index < NumReturned; Index++)
					{
						// need to do this because the filename they give us might not be NUL-terminated
						char FileNameBuffer[XCONTENT_MAX_FILENAME_LENGTH+1];
						memcpy(FileNameBuffer, Data[Index].szFileName, XCONTENT_MAX_FILENAME_LENGTH);
						FileNameBuffer[XCONTENT_MAX_FILENAME_LENGTH] = 0;

						FString FileName = FString(FileNameBuffer);
						if(FileName.StartsWith(FileNamePrefixString))
						{
							FSavedScreenshotInfo Screenshot;
							appMemzero(&Screenshot, sizeof(Screenshot));
							FGuid& ID = Screenshot.Id;
							// read in the unique ID for this shot
							INT Offset = FileNamePrefixString.Len();
							appSSCANF(*FileName + Offset, TEXT("%08X%08X%08X%08X"), &ID.A, &ID.B, &ID.C, &ID.D);
							// save the device
							Screenshot.DeviceID = (DWORD)Data[Index].DeviceID;
							// split the display name into its substrings
							FString DisplayName = FString(Data[Index].szDisplayName);
							TArray<FString> SubStrings;
							INT NumElements = DisplayName.ParseIntoArray(&SubStrings, *DisplayNameSeparator, TRUE);
							if(NumElements == 4)
							{
								// split the gametype and mapname
								if(SubStrings(1).Split(GetGameToMapConjunction(), &Screenshot.GameType, &Screenshot.MapName))
								{
									// split the rating from its suffix
									FString Rating;
									if(SubStrings(2).Split(GetRatingSuffix(), &Rating, NULL))
									{
										// convert the rating to a numerical value
										Screenshot.Rating = appAtoi(*Rating);
										// date in string format
										Screenshot.Date = SubStrings(3);
										// parsed date
										ParseDate(Screenshot.Date, Screenshot.Year, Screenshot.Month, Screenshot.Day);
										// add it to the list
										Screenshots->AddItem(Screenshot);
									}
								}
							}
						}
					}
					EnumerationResult = ATR_Succeeded;
				}
				else if(Result == ERROR_NO_MORE_FILES)
				{
					EnumerationResult = ATR_Succeeded;
					debugf(TEXT("FAsyncScreenshotEnumerate::DoWork(): XEnumerate found no more files."));
				}
				else
				{
					EnumerationResult = ATR_Failed;
					debugf(NAME_Error,TEXT("FAsyncScreenshotEnumerate::DoWork(): XEnumerate() failed: %d"), Result);
				}
			}
			while(Result == ERROR_SUCCESS);
			appFree(Data);
			CloseHandle(EnumHandle);
		}
		else
		{
			debugf(NAME_Error,TEXT("FAsyncScreenshotEnumerate::DoWork(): XContentCreateEnumerator() failed: %d"), Result);
		}

		appInterlockedExchange(EnumerationState, EnumerationResult);
	}

	/**
	* Ignored
	*/
	virtual void Abandon(void)
	{
	}

	/**
	* Cleans up the object
	*/ 
	virtual void Dispose(void)
	{
		delete this;
	}

private:
	/**
	 * Converts a m[m]/d[d]/yyyy string into numbers.
	 */
	void ParseDate(const FString& Date, INT& Year, BYTE& Month, BYTE& Day)
	{
		TArray<FString> Parts;
		INT NumParts = Date.ParseIntoArray(&Parts, TEXT("/"), TRUE);
		if(NumParts == 3)
		{
			Month = (BYTE)appAtoi(*Parts(0));
			Day = (BYTE)appAtoi(*Parts(1));
			Year = appAtoi(*Parts(2));
		}
		else
		{
			Month = (BYTE)0;
			Day = (BYTE)0;
			Year = 0;
		}
	}
};

#endif

/**
 * Starts an async task to save screenshots to disk.
 *
 * @param PlayerID The player requesting the save.
 * @param Image The JPG to save to disk.
 * @param Thumbnail The PNG to use as a thumbnail.
 * @param Info The screenshot's info.
 * @param InSaveCompleteDelegates The delegates to call when the attempt completes.
 *
 * @return True if the async task is started successfully.
 */
UBOOL FGearScreenshotIO::Save(INT PlayerID, const TArray<BYTE>& Image, const TArray<BYTE>& Thumbnail, const FScreenshotInfo& Info, TArray<FScriptDelegate>* InSaveCompleteDelegates)
{
	UBOOL Return = FALSE;
#if _XBOX
	// verify that there's no other IO activity
	if(IsBusy())
	{
		debugf(TEXT("Attempted to save a screenshot while the IO object was busy."));
		return FALSE;
	}
	appInterlockedExchange(&SaveScreenshotTask.Result, ATR_Working);
	SaveScreenshotTask.PlayerID = PlayerID;
	SaveScreenshotTask.CompleteDelegates = InSaveCompleteDelegates;

	XCONTENT_DATA ContentData;
	FGuid ID = appCreateGuid();
	if(GetXContentData(PlayerID, ID, InvalidDeviceID, ContentData))
	{
		// Create a display name
		FString DisplayName;
		GetContentDisplayName(Info, DisplayName);
		check(DisplayName.Len() < XCONTENT_MAX_DISPLAYNAME_LENGTH);
		appStrcpy(ContentData.szDisplayName, *DisplayName);
		// Do the save on another thread
		GThreadPool->AddQueuedWork(new FAsyncScreenshotSave(&ContentData, PlayerID,
			Image.Num(), Image.GetData(),
			Thumbnail.Num(), Thumbnail.GetData(),
			Info,
			&SaveScreenshotTask.Result));
		Return = TRUE;
		// Make sure this object ticks.
		if(!TickableObjects.ContainsItem(this))
		{
			TickableObjects.AddItem( this );
		}
	}
	SaveScreenshotTask.bActive = Return;
#endif
	return Return;
}

/**
 * Loads a screenshot from disk.
 *
 * @param PlayerID The player requesting the screenshot to be loaded.
 * @param ID The ID of the screenshot to load.
 * @param DeviceID The device from which to load the screenshot.
 * @param Image The screenshot's JPG will be stored in this array.
 * @param Info The screenshot's info will be stored in this parameter.
 * @param InLoadCompleteDelegates The delegates to call when the attempt completes.
 *
 * @return True if the async task is started successfully.
 */
UBOOL FGearScreenshotIO::Load(INT PlayerID, const FGuid& ID, INT DeviceID, TArray<BYTE>& Image, FScreenshotInfo& Info, TArray<FScriptDelegate>* InLoadCompleteDelegates)
{
	UBOOL Return = FALSE;
#if _XBOX
	// verify that there's no other IO activity
	if(IsBusy())
	{
		debugf(TEXT("Attempted to load a screenshot while the IO object was busy."));
		return FALSE;
	}
	appInterlockedExchange(&LoadScreenshotTask.Result, ATR_Working);
	LoadScreenshotTask.PlayerID = PlayerID;
	LoadScreenshotTask.CompleteDelegates = InLoadCompleteDelegates;

	XCONTENT_DATA ContentData;
	if(GetXContentData(PlayerID, ID, DeviceID, ContentData))
	{
		GThreadPool->AddQueuedWork(new FAsyncScreenshotLoad(&ContentData, PlayerID, ID, &Image, &Info, &LoadScreenshotTask.Result));
		Return = TRUE;
		// Make sure this object ticks.
		if(!TickableObjects.ContainsItem(this))
		{
			TickableObjects.AddItem( this );
		}
	}
	else
	{
		debugf(TEXT("FGearScreenshotIO::Load(): GetXContentData failed"));
	}
	LoadScreenshotTask.bActive = Return;
#endif
	return Return;
}

/**
 * Deletes a screenshot from disk.
 *
 * @param PlayerID The player requesting the delete.
 * @param ID The unique ID of the screenshot to delete.
 * @param DeviceID The device from which to delete the screenshot.
 *
 * @return True if the screenshot was successfully deleted.
 */
UBOOL FGearScreenshotIO::Delete(INT PlayerID, const FGuid& ID, INT DeviceID)
{
	UBOOL Return = FALSE;
#if _XBOX
	Stopwatch Timer(TEXT("DeleteScreenshot"));
	XCONTENT_DATA ContentData;
	if(GetXContentData(PlayerID, ID, DeviceID, ContentData))
	{
		DWORD Result = XContentDelete(PlayerID, &ContentData, NULL);
		if(SUCCEEDED(Result))
		{
			Return = TRUE;
		}
		else
		{
			debugf(NAME_Error,TEXT("Failed to delete screenshot"));
		}
	}
#endif
	return Return;
}

/**
 * Gets a list of screenshots on disk.
 *
 * @param PlayerID The player requesting the enumeration.
 * @param Screenshots The list of screenshots will be stored in this array.
 * @param InEnumerateCompleteDelegates The delegates to call when the enumeration completes.
 *
 * @return True if the async task is started successfully.
 */
UBOOL FGearScreenshotIO::Enumerate(INT PlayerID, TArray<FSavedScreenshotInfo>& Screenshots, TArray<FScriptDelegate>* InEnumerateCompleteDelegates)
{
#if _XBOX
	// verify that there's no other IO activity
	if(IsBusy())
	{
		debugf(TEXT("Attempted to enumerate screenshots while the IO object was busy."));
		return FALSE;
	}
	appInterlockedExchange(&EnumerateScreenshotTask.Result, ATR_Working);
	EnumerateScreenshotTask.PlayerID = PlayerID;
	EnumerateScreenshotTask.CompleteDelegates = InEnumerateCompleteDelegates;
	GThreadPool->AddQueuedWork(new FAsyncScreenshotEnumerate(PlayerID, &Screenshots, &EnumerateScreenshotTask.Result));
	// Make sure this object ticks.
	if(!TickableObjects.ContainsItem(this))
	{
		TickableObjects.AddItem( this );
	}
	EnumerateScreenshotTask.bActive = TRUE;
#endif
	return TRUE;
}

/**
 * Checks if the object is busy doing IO.
 * IO attempts while the object is busy will fail.
 *
 * @return True if the object is busy, false if the object is free to perform a new operations.
 */
UBOOL FGearScreenshotIO::IsBusy() const
{
	return SaveScreenshotTask.bActive || LoadScreenshotTask.bActive || EnumerateScreenshotTask.bActive;
}

/**
* Used to determine whether an object is ready to be ticked.
*
* @return	TRUE if tickable, FALSE otherwise
*/
UBOOL FGearScreenshotIO::IsTickable() const
{
#if _XBOX
	return IsBusy();
#else
	return FALSE;
#endif
}

/**
* Used to determine if an object should be ticked when the game is paused.
*
* @return always TRUE as networking needs to be ticked even when paused
*/
UBOOL FGearScreenshotIO::IsTickableWhenPaused() const
{
#if _XBOX
	return TRUE;
#else
	return FALSE;
#endif
}

void FGearScreenshotIO::Tick(FLOAT DeltaTime)
{
	TickTask(&SaveScreenshotTask);
	TickTask(&LoadScreenshotTask);
	TickTask(&EnumerateScreenshotTask);
}

void FGearScreenshotIO::TickTask(AsyncTaskState* Task)
{
#if _XBOX
	if(Task->bActive)
	{
		if(Task->Result != ATR_Working)
		{
			if(Task->CompleteDelegates != NULL)
			{
				const INT PlayerIndex = UUIInteraction::GetPlayerIndex(Task->PlayerID);

				check(GEngine->GamePlayers.Num()>0);
				check(GEngine->GamePlayers.IsValidIndex(PlayerIndex));
				AGearPC *PC = CastChecked<AGearPC>(GEngine->GamePlayers(PlayerIndex)->Actor);

				GearScreenshotManager_eventOnSaveScreenshotComplete_Parms Parms(EC_EventParm);
				Parms.bWasSuccessful = (Task->Result == ATR_Succeeded) ? FIRST_BITFIELD : 0;
				// copy off the delegates before clearing the active flag, to be safe
				TArray<FScriptDelegate> CompleteDelegates = *Task->CompleteDelegates;
				// clear the flag so that operations can be done from the delegates
				Task->bActive = FALSE;
				TriggerOnlineDelegates(PC, CompleteDelegates, &Parms);
			}
			else
			{
				Task->bActive = FALSE;
			}
		}
	}
#endif
}

#if _XBOX

/** Utility for getting a display name for a screenshot. */
void FGearScreenshotIO::GetContentDisplayName(const FScreenshotInfo& Info, FString& DisplayName)
{
	SYSTEMTIME SystemTime;
	appMemzero(&SystemTime, sizeof(SYSTEMTIME));
	BOOL bGotSystemTime = FALSE;
	// Load the UTC time.
	FILETIME UTCFileTime;
	UTCFileTime.dwLowDateTime = Info.Realtime.A;
	UTCFileTime.dwHighDateTime = Info.Realtime.B;
	// Convert to local time.
	FILETIME LocalFileTime;
	if(FileTimeToLocalFileTime(&UTCFileTime, &LocalFileTime))
	{
		// Convert to something we can read useful info out of.
		bGotSystemTime = FileTimeToSystemTime(&LocalFileTime, &SystemTime);
	}
	if(!bGotSystemTime)
	{
		// Make sure it is zeroed.
		appMemzero(&SystemTime, sizeof(SYSTEMTIME));
	}
#if RANDOM_SCREENSHOT_VALUES
	FString RandomGameType = ScreenshotIOHelper::RandomString();
	FString RandomMapName = ScreenshotIOHelper::RandomString();
#endif
	// FAsyncScreenshotEnumerate::ParseDate() depends on the date format used here.
	DisplayName = FString::Printf(TEXT("%s%s%s%s%s%s%d%s%s%d/%d/%d"),
		*GetDisplayNamePrefix(),
		*DisplayNameSeparator,
#if !RANDOM_SCREENSHOT_VALUES
		*Info.GameTypeFriendly, *GetGameToMapConjunction(), *Info.MapNameFriendly,
#else
		*RandomGameType, *GetGameToMapConjunction(), *RandomMapName,
#endif
		*DisplayNameSeparator,
		Info.Rating, *GetRatingSuffix(),
		*DisplayNameSeparator,
		SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear);
}

/**
 * Utility for setting up an XCONTENT_DATA.
 * If DeviceID is InvalidDeviceID (-1), the engine's current device will be used.
 * If the engine doesn't have a current device, the device selector will pop out.
 */
UBOOL FGearScreenshotIO::GetXContentData(DWORD PlayerID, const FGuid& ID, INT DeviceID, XCONTENT_DATA& ContentData)
{
	UBOOL bResult = FALSE;
	appMemzero(&ContentData, sizeof(ContentData));
	GetContentFileName(ID, ContentData.szFileName);
	ContentData.dwContentType = ContentType;
	ContentData.DeviceID = DeviceID;
	if(DeviceID != InvalidDeviceID)
	{
		bResult = TRUE;
	}
	else
	{
		// get the device to use
		if(GetDeviceID(PlayerID, ContentData.DeviceID))
		{
			bResult = TRUE;
		}
		else
		{
			debugf(TEXT("FGearScreenshotIO::GetXContentData: GetDeviceID failed."));
		}
	}
	return bResult;
}

/** Utility for getting a filename from an ID. */
void FGearScreenshotIO::GetContentFileName(const FGuid& ID, ANSICHAR FileName[XCONTENT_MAX_FILENAME_LENGTH])
{
	appMemzero(FileName,XCONTENT_MAX_FILENAME_LENGTH);
	sprintf_s(FileName,XCONTENT_MAX_FILENAME_LENGTH,"%s%08X%08X%08X%08X", FileNamePrefix, ID.A, ID.B, ID.C, ID.D);
}

/** Gets the deviceID to use for access the disk. */
UBOOL FGearScreenshotIO::GetDeviceID(INT PlayerID, DWORD& OutDeviceID)
{
	UGearEngine* Engine = Cast<UGearEngine>(GEngine);
	if ( Engine == NULL )
	{
		return FALSE;
	}

	XCONTENTDEVICEID DeviceID        = static_cast<XCONTENTDEVICEID>(Engine->GetCurrentDeviceID());
	ULARGE_INTEGER   FileSize        = {0};
	INT              MaxSize         = 400 * 1024;  //TODO: should we calculate, or is this big enough?

	FileSize.QuadPart = XContentCalculateSize(MaxSize, 1);

	// determine if we should show the selector, and if so the content flags to use
	const UBOOL bDeviceHasSpace = Engine->eventIsCurrentDeviceValid((DWORD)FileSize.QuadPart);
	if ( bDeviceHasSpace )
	{
		OutDeviceID = (DWORD)DeviceID;
		return TRUE;
	}
	debugf(TEXT("The user doesn't have or didn't select a valid device"));
	return FALSE;
}

#endif

