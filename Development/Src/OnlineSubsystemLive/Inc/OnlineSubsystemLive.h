/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#ifndef __ONLINESUBSYSTEMLIVE_H__
#define __ONLINESUBSYSTEMLIVE_H__

#include "UnIpDrv.h"
#include "UnSocketLive.h"
#include "UnNetLive.h"

#if WITH_UE3_NETWORKING

#if 1//_DEBUG
	#define debugfLiveSlow debugf
#else
	#define debugfLiveSlow debugfSuppressed
#endif

/**
 * Given a Mu and Sigma value, determine the normalized skill level (0-50).
 * Note: this code returns the lower end of the range of skills the player
 * might be.
 *
 * @param Mu the skill of the player
 * @param Sigma the certainty that the skill is correct
 *
 * @return the normalized skill value
 */
inline INT CalculateConservativeSkill(DOUBLE Mu,DOUBLE Sigma)
{
	DOUBLE Intermediate = Mu - 3.0 * Sigma;
	return appTrunc(Clamp<DOUBLE>(Intermediate,0.0,6.0) * (50.0 / 6.0));
}

/**
 * Given a Mu and Sigma value, determine the normalized skill level (0-50).
 * Note: this code returns the high end of the range of skills the player
 * might be.
 *
 * @param Mu the skill of the player
 * @param Sigma the certainty that the skill is correct
 *
 * @return the normalized skill value
 */
inline INT CalculateOptimisticSkill(DOUBLE Mu,DOUBLE Sigma)
{
	DOUBLE Intermediate = Mu + 3.0 * Sigma;
	return appTrunc(Clamp<DOUBLE>(Intermediate,0.0,6.0) * (50.0 / 6.0));
}

/**
 * Given the client's skill information and the host's, determines the quality of the
 * match on a 0 to 1 scale (percentage chance of good match)
 *
 * @param ClientMu the client's skill assessment
 * @param ClientSigma the certainty that the skill is correct
 * @param ClientCount the number of players that comprise the skill rating
 * @param HostMu the host's skill assessment
 * @param HostSigma the certainty that the skill is correct
 * @param HostCount the number of players that comprise the skill rating
 */
inline FLOAT CalculateHostQuality(DOUBLE ClientMu,DOUBLE ClientSigma,DOUBLE ClientCount,DOUBLE HostMu,DOUBLE HostSigma,DOUBLE HostCount)
{
    DOUBLE ScaledClientMu = ClientMu * ClientCount;

	DOUBLE ScaledClientSigma2 = Square(ClientSigma) * ClientCount;

	DOUBLE ScaledHostMu = HostMu * HostCount;

    DOUBLE ScaledHostSigma2 = Square(HostSigma) * HostCount;

    DOUBLE TotalBeta2 = Square(0.5) * (ClientCount + HostCount);

    DOUBLE MuDifference = ScaledClientMu - ScaledHostMu;
    DOUBLE ClientWithPerfectHostVariance = TotalBeta2 + ScaledClientSigma2;
    DOUBLE ClientWithCurrentHostVariance = ClientWithPerfectHostVariance + ScaledHostSigma2;

    return appExp(-0.5 * Square(MuDifference) / ClientWithCurrentHostVariance) *
           appSqrt(ClientWithPerfectHostVariance / ClientWithCurrentHostVariance);
}

/**
 * Converts the ID into a one way hash and stringifies that
 *
 * @param Id the id to hash
 *
 * @return a string form of that hash
 */
inline FString appHashUniqueNetId(const FUniqueNetId& Id)
{
	BYTE HashResult[20];
	// One way hash the id
	FSHA1::HashBuffer((void*)&Id,sizeof(FUniqueNetId),HashResult);
	FString HashString;
	// And now turn that into a string
	for (DWORD Count = 0; Count < 20; Count++)
	{
		HashString += FString::Printf(TEXT("%02X"),(DWORD)HashResult[Count]);
	}
	return HashString;
}

/** Structure used to capture an online session */
struct FSecureSessionInfo
{
	/** The nonce for the session */
	ULONGLONG Nonce;
	/** The handle associated with the session */
	HANDLE Handle;
	/** The host address information associated with the session */
	XSESSION_INFO XSessionInfo;

	/** Default ctor that zeros everything */
	inline FSecureSessionInfo(void)
	{
		appMemzero(this,sizeof(FSecureSessionInfo));
	}
};

/**
 * Casts a base session info into a secure session info (guaranteed by platform)
 *
 * @param Session the session to get the secure session info from
 *
 * @return the secure session information for the session
 */
FORCEINLINE FSecureSessionInfo* GetSessionInfo(FNamedSession* Session)
{
	return (FSecureSessionInfo*)Session->SessionInfo;
}

/**
 * Class used to write data into packets for sending via system link
 */
class FNboSerializeToBufferXe :
	public FNboSerializeToBuffer
{
public:
	/** Default constructor zeros num bytes*/
	FNboSerializeToBufferXe(void) :
		FNboSerializeToBuffer(512)
	{
	}

	/** Constructor specifying the size to use */
	FNboSerializeToBufferXe(DWORD Size) :
		FNboSerializeToBuffer(Size)
	{
	}

	/**
	 * Adds an XNADDR to the buffer
	 */
	friend inline FNboSerializeToBufferXe& operator<<(FNboSerializeToBufferXe& Ar,XNADDR& Addr)
	{
		checkSlow(Ar.NumBytes + sizeof(XNADDR) <= Ar.GetBufferSize());
		appMemcpy(&Ar.Data(Ar.NumBytes),&Addr,sizeof(XNADDR));
		Ar.NumBytes += sizeof(XNADDR);
		return Ar;
	}

	/**
	 * Adds an XNKID to the buffer
	 */
	friend inline FNboSerializeToBufferXe& operator<<(FNboSerializeToBufferXe& Ar,XNKID& KeyId)
	{
		checkSlow(Ar.NumBytes + 8 <= Ar.GetBufferSize());
		appMemcpy(&Ar.Data(Ar.NumBytes),KeyId.ab,8);
		Ar.NumBytes += 8;
		return Ar;
	}

	/**
	 * Adds an XNKEY to the buffer
	 */
	friend inline FNboSerializeToBufferXe& operator<<(FNboSerializeToBufferXe& Ar,XNKEY& Key)
	{
		checkSlow(Ar.NumBytes + 16 <= Ar.GetBufferSize());
		appMemcpy(&Ar.Data(Ar.NumBytes),Key.ab,16);
		Ar.NumBytes += 16;
		return Ar;
	}
};

#pragma warning( push )
// Disable used without initialization warning because the reads are initializing
#pragma warning( disable : 4700 )

/**
 * Class used to write data into packets for sending via system link
 */
class FNboSerializeFromBufferXe :
	public FNboSerializeFromBuffer
{

public:
	/**
	 * Initializes the buffer, size, and zeros the read offset
	 */
	FNboSerializeFromBufferXe(BYTE* Packet,INT Length) :
		FNboSerializeFromBuffer(Packet,Length)
	{
	}

	/**
	 * Reads an XNADDR from the buffer
	 */
	friend inline FNboSerializeFromBufferXe& operator>>(FNboSerializeFromBufferXe& Ar,XNADDR& Addr)
	{
		checkSlow(Ar.CurrentOffset + (INT)sizeof(XNADDR) <= Ar.NumBytes);
		appMemcpy(&Addr,&Ar.Data[Ar.CurrentOffset],sizeof(XNADDR));
		Ar.CurrentOffset += sizeof(XNADDR);
		return Ar;
	}

	/**
	 * Reads an XNKID from the buffer
	 */
	friend inline FNboSerializeFromBufferXe& operator>>(FNboSerializeFromBufferXe& Ar,XNKID& KeyId)
	{
		checkSlow(Ar.CurrentOffset + 8 <= Ar.NumBytes);
		appMemcpy(KeyId.ab,&Ar.Data[Ar.CurrentOffset],8);
		Ar.CurrentOffset += 8;
		return Ar;
	}

	/**
	 * Reads an XNKEY from the buffer
	 */
	friend inline FNboSerializeFromBufferXe& operator>>(FNboSerializeFromBufferXe& Ar,XNKEY& Key)
	{
		checkSlow(Ar.CurrentOffset + 16 <= Ar.NumBytes);
		appMemcpy(Key.ab,&Ar.Data[Ar.CurrentOffset],16);
		Ar.CurrentOffset += 16;
		return Ar;
	}
};

#pragma warning( pop )

/**
 * Base class for all per task data that needs to be freed once a Live
 * task has completed
 */
struct FLiveAsyncTaskData
{
	/** Base destructor. Here to force proper destruction */
	virtual ~FLiveAsyncTaskData(void)
	{
	}
};

/**
 * Class that holds an async overlapped structure and the delegate to call
 * when the event is complete. This allows the Live ticking to check a set of
 * events for completion and trigger script code without knowing the specific
 * details of the event
 */
class FOnlineAsyncTaskLive :
	public FOnlineAsyncTask
{
protected:
	/** The overlapped structure to check for being done */
	XOVERLAPPED Overlapped;
	/** Any per task data that needs to be cleaned up once the task is done */
	FLiveAsyncTaskData* TaskData;

	/** Hidden on purpose */
	FOnlineAsyncTaskLive(void) {}

public:
	/**
	 * Zeros the overlapped structure and sets the delegate to call
	 *
	 * @param InScriptDelegates the delegates to fire off when complete
	 * @param InTaskData the data associated with the task. Freed when compelete
	 * @param InAsyncTaskName pointer to static data identifying the type of task
	 */
	FOnlineAsyncTaskLive(TArray<FScriptDelegate>* InScriptDelegates,FLiveAsyncTaskData* InTaskData,const TCHAR* InAsyncTaskName) :
		FOnlineAsyncTask(InScriptDelegates,InAsyncTaskName),
		TaskData(InTaskData)
	{
		appMemzero(&Overlapped,sizeof(XOVERLAPPED));
	}

	/**
	 * Frees the task data if any was specified
	 */
	virtual ~FOnlineAsyncTaskLive(void)
	{
		delete TaskData;
	}

	/**
	 * Checks the completion status of the task
	 *
	 * @return TRUE if done, FALSE otherwise
	 */
	FORCEINLINE UBOOL HasTaskCompleted(void) const
	{
		return XHasOverlappedIoCompleted(&Overlapped);
	}

	/**
	 * Returns the status code of the completed task. Assumes the task
	 * has finished.
	 *
	 * @return ERROR_SUCCESS if ok, an HRESULT error code otherwise
	 */
	virtual DWORD GetCompletionCode(void)
	{
		return XGetOverlappedExtendedError(&Overlapped);
	}

	/** Operator to use with passing the overlapped structure */
	FORCEINLINE operator PXOVERLAPPED(void)
	{
		return &Overlapped;
	}

	/**
	 * Used to route the final processing of the data to the correct subsystem
	 * function. Basically, this is a function pointer for doing final work
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem)
	{
		return TRUE;
	}
};

/**
 * Class that handles triggering of delegates that have named sessions
 */
class FOnlineAsyncTaskLiveNamedSession :
	public FOnlineAsyncTaskLive
{
protected:
	/** The name of the session the async task is for */
	const FName SessionName;

	/** Hidden on purpose */
	FOnlineAsyncTaskLiveNamedSession(void) {}

public:
	/**
	 * Initializes the class
	 *
	 * @param InSessionName the name of the session being operated on
	 * @param InScriptDelegates the delegates to fire off when complete
	 * @param InTaskData the data associated with the task. Freed when compelete
	 * @param InAsyncTaskName pointer to static data identifying the type of task
	 */
	FOnlineAsyncTaskLiveNamedSession(FName InSessionName,TArray<FScriptDelegate>* InScriptDelegates,FLiveAsyncTaskData* InTaskData,const TCHAR* InAsyncTaskName) :
		FOnlineAsyncTaskLive(InScriptDelegates,InTaskData,InAsyncTaskName),
		SessionName(InSessionName)
	{
	}

	/**
	 * Iterates the list of delegates and fires those notifications
	 *
	 * @param Object the object that the notifications are going to be issued on
	 */
	virtual void TriggerDelegates(UObject* Object)
	{
		check(Object);
		// Only fire off the events if there are some registered
		if (ScriptDelegates != NULL)
		{
			// Pass in the data that indicates whether the call worked or not
			FAsyncTaskDelegateResultsNamedSession Parms(SessionName,GetCompletionCode());
			// Use the common method to do the work
			TriggerOnlineDelegates(Object,*ScriptDelegates,&Parms);
		}
	}

#if !FINAL_RELEASE && !SHIPPING_PC_GAME
	/**
	 * Logs amount of time taken in the async task
	 */
	virtual void LogResults(void)
	{
		debugf(NAME_DevOnline,TEXT("Async task '%s' for session '%s' completed in %f seconds with 0x%08X"),
			AsyncTaskName,
			*SessionName.ToString(),
			appSeconds() - StartTime,
			GetCompletionCode());
	}
#endif
};

/**
 * Holds the buffer used in the search
 */
class FLiveAsyncTaskDataSearch :
	public FLiveAsyncTaskData
{
	/** The chunk of memory to place the search results in */
	BYTE* SearchBuffer;
	static const DWORD NumQosRequests = 50;
	/** The array of server addresses to query */
	XNADDR* ServerAddrs[NumQosRequests];
	/** The array of server session ids to use for the query */
	XNKID* ServerKids[NumQosRequests];
	/** The array of server session keys to use for the query */
	XNKEY* ServerKeys[NumQosRequests];
	/** Holds the QoS data returned by the QoS query */
	XNQOS* QosData;
	/** Holds the Live version of our property structure for the async call */
	PXUSER_PROPERTY Properties;
	/** Holds the Live array of contexts to use for the search */
	PXUSER_CONTEXT Contexts;

	/** Hidden on purpose */
	FLiveAsyncTaskDataSearch(void) {}

public:
	/**
	 * Copies the pointer passed in
	 *
	 * @param InBuffer the buffer to use to hold the data
	 * @param InSearch the search object to write the results to
	 */
	FLiveAsyncTaskDataSearch(BYTE* InBuffer) :
		SearchBuffer(InBuffer),
		QosData(NULL),
		Properties(NULL),
		Contexts(NULL)
	{
		appMemzero(ServerAddrs,sizeof(void*) * NumQosRequests);
		appMemzero(ServerKids,sizeof(void*) * NumQosRequests);
		appMemzero(ServerKeys,sizeof(void*) * NumQosRequests);
	}

	/**
	 * Allocates an internal buffer of the specified size
	 *
	 * @param InBuffer the buffer to use to hold the data
	 */
	FLiveAsyncTaskDataSearch(DWORD BufferSize) :
		QosData(NULL),
		Properties(NULL),
		Contexts(NULL)
	{
		SearchBuffer = new BYTE[BufferSize];
		appMemzero(ServerAddrs,sizeof(void*) * NumQosRequests);
		appMemzero(ServerKids,sizeof(void*) * NumQosRequests);
		appMemzero(ServerKeys,sizeof(void*) * NumQosRequests);
	}

	/** Base destructor. Here to force proper destruction */
	virtual ~FLiveAsyncTaskDataSearch(void)
	{
		delete [] SearchBuffer;
		// Free the QoS data
		if (QosData != NULL)
		{
			XNetQosRelease(QosData);
		}
		delete [] Properties;
		delete [] Contexts;
	}

	/** Operator that gives access to the data buffer */
	FORCEINLINE operator PXSESSION_SEARCHRESULT_HEADER(void)
	{
		return (PXSESSION_SEARCHRESULT_HEADER)SearchBuffer;
	}

	/** Returns the array to use for XNADDRs in the QoS query */
	FORCEINLINE XNADDR** GetXNADDRs(void)
	{
		return ServerAddrs;
	}

	/** Returns the array to use for XNKIDs in the QoS query */
	FORCEINLINE XNKID** GetXNKIDs(void)
	{
		return ServerKids;
	}

	/** Returns the array to use for XNKEYs in the QoS query */
	FORCEINLINE XNKEY** GetXNKEYs(void)
	{
		return ServerKeys;
	}

	/** Returns the pointer to the QoS data to be processed */
	FORCEINLINE XNQOS** GetXNQOS(void)
	{
		return &QosData;
	}

	/**
	 * Allocates memory for the properties array for the async task
	 *
	 * @param NumProps the number of properties that are going to be copied
	 */
	FORCEINLINE PXUSER_PROPERTY AllocateProperties(INT NumProps)
	{
		Properties = new XUSER_PROPERTY[NumProps];
		return Properties;
	}

	/**
	 * Allocates memory for the contexts array for the async task
	 *
	 * @param NumContexts the number of contexts that are going to be copied
	 */
	FORCEINLINE PXUSER_CONTEXT AllocateContexts(INT NumContexts)
	{
		Contexts = new XUSER_CONTEXT[NumContexts];
		return Contexts;
	}
};

/**
 * Handles search specific async task
 */
class FLiveAsyncTaskSearch :
	public FOnlineAsyncTaskLive
{
	/** Whether we are waiting for Live or waiting for QoS results */
	UBOOL bIsWaitingForLive;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InScriptDelegates the delegates to fire off when complete
	 * @param InTaskData the data associated with the task. Freed when compelete
	 */
	FLiveAsyncTaskSearch(TArray<FScriptDelegate>* InScriptDelegates,FLiveAsyncTaskData* InTaskData) :
		FOnlineAsyncTaskLive(InScriptDelegates,InTaskData,TEXT("XSessionSearch()")),
		bIsWaitingForLive(TRUE)
	{
	}

	/**
	 * Routes the call to the function on the subsystem for parsing search results
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

/**
 * Holds the buffer used in the read async call
 */
class FLiveAsyncTaskDataReadProfileSettings :
	public FLiveAsyncTaskData
{
	/** Which user we are reading the profile data for */
	DWORD UserIndex;
	/** Buffer to hold the data from the game specific fields in */
	BYTE* GameSettingsBuffer;
	/** Holds the IDs for the three game settings fields */
	DWORD GameSettingsIds[3];
	/** Holds the amount of data that was allocated */
	DWORD GameSettingsBufferSize;
	/** The chunk of memory to place the read results in */
	BYTE* Buffer;
	/** The number of IDs the game originally asked for */
	DWORD NumIds;
	/** Chunk of memory that holds the mapped profile setting ids */
	DWORD* ReadIds;
	/** The size of the memory needed to hold the results */
	DWORD ReadIdsSize;
	/** The action currently being performed */
	DWORD Action;
	/** The working buffer for parsing out game settings */
	BYTE WorkingBuffer[3000];
	/** The amount of space used by the working buffer */
	DWORD WorkingBufferUsed;

	/** Hidden on purpose */
	FLiveAsyncTaskDataReadProfileSettings(void) {}

public:
	/** The state the async task is in */
	enum
	{
		/** Currently fetching the settings from our 3 game settings */
		ReadingGameSettings,
		/** Reading from Live the settings that weren't in our game settings */
		ReadingLiveSettings
	};

	/**
	 * Allocates an internal buffer of the specified size
	 *
	 * @param InUserIndex the user the profile read is being done for
	 * @param InNumIds the number of ids to allocate
	 */
	FLiveAsyncTaskDataReadProfileSettings(DWORD InUserIndex,DWORD InNumIds) :
		UserIndex(InUserIndex),
		Buffer(NULL),
		NumIds(InNumIds),
		ReadIdsSize(0)
	{
		// Set the 3 fields that we are going to read
		GameSettingsIds[0] = XPROFILE_TITLE_SPECIFIC1;
		GameSettingsIds[1] = XPROFILE_TITLE_SPECIFIC2;
		GameSettingsIds[2] = XPROFILE_TITLE_SPECIFIC3;
		// Allocate where we are going to fill in the used Ids
		ReadIds = new DWORD[NumIds];
		appMemzero(ReadIds,sizeof(DWORD) * NumIds);
		GameSettingsBufferSize = 0;
		// Figure out how big a buffer is needed
		XUserReadProfileSettings(0,0,3,GameSettingsIds,&GameSettingsBufferSize,NULL,NULL);
		check(GameSettingsBufferSize > 0);
		// Now we can allocate the
		GameSettingsBuffer = new BYTE[GameSettingsBufferSize];
		appMemzero(GameSettingsBuffer,GameSettingsBufferSize);
		// We are reading our saved game settings first
		Action = ReadingGameSettings;
		// Zero the working buffer
		appMemzero(WorkingBuffer,3000);
		WorkingBufferUsed = 0;
	}

	/** Base destructor. Here to force proper destruction */
	virtual ~FLiveAsyncTaskDataReadProfileSettings(void)
	{
		delete [] Buffer;
		delete [] ReadIds;
		delete [] GameSettingsBuffer;
	}

	/** Returns the user index associated with this profile read */
	inline DWORD GetUserIndex(void) const
	{
		return UserIndex;
	}

	/** Gets the buffer that holds the results of the items to be read */
	inline PXUSER_READ_PROFILE_SETTING_RESULT GetProfileBuffer(void)
	{
		return (PXUSER_READ_PROFILE_SETTING_RESULT)Buffer;
	}

	/** Accessor to the DWORD array memory */
	inline DWORD* GetIds(void)
	{
		return ReadIds;
	}

	/** Returns the number of IDs the game asked for */
	inline DWORD GetIdsCount(void)
	{
		return NumIds;
	}

	/** Returns the size in memory of the buffer */
	inline DWORD GetIdsSize(void)
	{
		return ReadIdsSize;
	}

	/** Gets the buffer that holds the results of the game settings read */
	inline PXUSER_READ_PROFILE_SETTING_RESULT GetGameSettingsBuffer(void)
	{
		return (PXUSER_READ_PROFILE_SETTING_RESULT)GameSettingsBuffer;
	}

	/** Gets the set of IDs for the game settings */
	inline DWORD* GetGameSettingsIds(void)
	{
		return GameSettingsIds;
	}

	/** Returns the number of game settings to read */
	inline DWORD GetGameSettingsIdsCount(void)
	{
		return 3;
	}

	/** Returns the size of the buffer */
	inline DWORD GetGameSettingsSize(void)
	{
		return GameSettingsBufferSize;
	}

	/** Returns the current state the task is in */
	inline DWORD GetCurrentAction(void)
	{
		return Action;
	}

	/** Sets the current state the task is in */
	inline void SetCurrentAction(DWORD NewState)
	{
		Action = NewState;
	}

	/** Grants access to the working buffer */
	inline BYTE* GetWorkingBuffer(void)
	{
		return WorkingBuffer;
	}

	/** Grants access to the working buffer size */
	inline DWORD GetWorkingBufferSize(void)
	{
		return WorkingBufferUsed;
	}

	/**
	 * Allocates the space needed by the Live reading portion
	 *
	 * @param IdsCount the number of IDs that are actually being read
	 */
	inline void AllocateBuffer(DWORD IdsCount)
	{
		ReadIdsSize = 0;
		// Ask live for the size
		XUserReadProfileSettings(0,0,IdsCount,ReadIds,&ReadIdsSize,NULL,NULL);
		check(ReadIdsSize > 0);
		// Now do the alloc
		Buffer = new BYTE[ReadIdsSize];
	}

	/**
	 * Coalesces the game settings data into one buffer instead of 3
	 */
	void CoalesceGameSettings(void);
};

/**
 * Handles profile settings read async task
 */
class FLiveAsyncTaskReadProfileSettings :
	public FOnlineAsyncTaskLive
{
	/**
	 * Reads the online profile settings from the buffer into the specified array
	 *
	 * @param Settings the array to populate from the game settings
	 */
	void SerializeGameSettings(TArray<FOnlineProfileSetting>& Settings);

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InScriptDelegates the delegate to fire off when complete
	 * @param InTaskData the data associated with the task. Freed when compelete
	 */
	FLiveAsyncTaskReadProfileSettings(TArray<FScriptDelegate>* InScriptDelegates,
		FLiveAsyncTaskData* InTaskData) :
		FOnlineAsyncTaskLive(InScriptDelegates,InTaskData,TEXT("XUserReadProfileSettings()"))
	{
	}

	/**
	 * Routes the call to the function on the subsystem for parsing search results
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);

	/**
	 * Iterates the list of delegates and fires those notifications
	 *
	 * @param Object the object that the notifications are going to be issued on
	 */
	virtual void TriggerDelegates(UObject* Object);
};

/**
 * Holds the buffer used in the write async call
 */
class FLiveAsyncTaskDataWriteProfileSettings :
	public FLiveAsyncTaskData
{
	/** Which user we are reading the profile data for */
	DWORD UserIndex;
	/** Buffer that will hold the data during the async write */
	BYTE Buffer[3000];
	/** The three game settings that we are writing to */
	XUSER_PROFILE_SETTING GameSettings[3];

	/** Hidden on purpose */
	FLiveAsyncTaskDataWriteProfileSettings(void) {}

public:
	/**
	 * Allocates an array of profile settings for writing
	 *
	 * @param InUserIndex the user the write is being performed for
	 * @param Source the data to write in our 3 game settings slots
	 * @param Length the amount of data being written
	 */
	FLiveAsyncTaskDataWriteProfileSettings(BYTE InUserIndex,const BYTE* Source,DWORD Length) :
		UserIndex(InUserIndex)
	{
		check(Length <= 3000);
		// Clear the byte buffer
		appMemzero(Buffer,3000);
		appMemcpy(Buffer,Source,Length);
		// Clear the game settings so we can set them up
		appMemzero(GameSettings,sizeof(XUSER_PROFILE_SETTING) * 3);
		// Set the common header stuff
		GameSettings[0].dwSettingId = XPROFILE_TITLE_SPECIFIC1;
		GameSettings[0].source = XSOURCE_TITLE;
		GameSettings[0].data.type = XUSER_DATA_TYPE_BINARY;
		GameSettings[1].dwSettingId = XPROFILE_TITLE_SPECIFIC2;
		GameSettings[1].source = XSOURCE_TITLE;
		GameSettings[1].data.type = XUSER_DATA_TYPE_BINARY;
		GameSettings[2].dwSettingId = XPROFILE_TITLE_SPECIFIC3;
		GameSettings[2].source = XSOURCE_TITLE;
		GameSettings[2].data.type = XUSER_DATA_TYPE_BINARY;
		// Determine how much each game setting will hold
		DWORD BlobSize1 = Min<DWORD>(Length,1000);
		Length -= BlobSize1;
		DWORD BlobSize2 = Min<DWORD>(Length,1000);
		Length -= BlobSize2;
		DWORD BlobSize3 = Min<DWORD>(Length,1000);
		check(Length - BlobSize3 == 0);
		// Now point the blobs into the buffer
		GameSettings[0].data.binary.cbData = BlobSize1;
		GameSettings[0].data.binary.pbData = Buffer;
		GameSettings[1].data.binary.cbData = BlobSize2;
		GameSettings[1].data.binary.pbData = &Buffer[BlobSize1];
		GameSettings[2].data.binary.cbData = BlobSize3;
		GameSettings[2].data.binary.pbData = &Buffer[BlobSize1 + BlobSize2];
	}

	/** Returns the user index associated with this profile read */
	inline DWORD GetUserIndex(void) const
	{
		return UserIndex;
	}

	/** Operator that gives access to the data buffer */
	inline operator PXUSER_PROFILE_SETTING(void)
	{
		return GameSettings;
	}
};

/**
 * Handles writing profile settings asynchronously
 */
class FLiveAsyncTaskWriteProfileSettings :
	public FOnlineAsyncTaskLive
{
public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InScriptDelegates the delegate to fire off when complete
	 * @param InTaskData the data associated with the task. Freed when compelete
	 */
	FLiveAsyncTaskWriteProfileSettings(TArray<FScriptDelegate>* InScriptDelegates,
		FLiveAsyncTaskData* InTaskData) :
		FOnlineAsyncTaskLive(InScriptDelegates,InTaskData,TEXT("XUserWriteProfileSettings()"))
	{
	}

	/**
	 * Routes the call to the function on the subsystem for parsing search results
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);

	/**
	 * Iterates the list of delegates and fires those notifications
	 *
	 * @param Object the object that the notifications are going to be issued on
	 */
	virtual void TriggerDelegates(UObject* Object);
};

/** Changes the state of the game session */
class FLiveAsyncTaskSessionStateChange :
	public FOnlineAsyncTaskLiveNamedSession
{
	/** Holds the state to switch to when the async task completes */
	EOnlineGameState StateToTransitionTo;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InSessionName the name of the session being changed
	 * @param InState the state to transition the game to once the async task is complete
	 * @param InScriptDelegates the delegate to fire off when complete
	 * @param InAsyncTaskName the name of the task for debugging purposes
	 */
	FLiveAsyncTaskSessionStateChange(FName InSessionName,EOnlineGameState InState,
		TArray<FScriptDelegate>* InScriptDelegates,const TCHAR* InAsyncTaskName) :
		FOnlineAsyncTaskLiveNamedSession(InSessionName,InScriptDelegates,NULL,InAsyncTaskName),
		StateToTransitionTo(InState)
	{
	}

	/**
	 * Changes the state from the current state to whatever was specified
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

/**
 * This async task starts the session and will shrink the public size to the
 * number of active players after arbitration handshaking
 */
class FLiveAsyncTaskStartSession :
	public FLiveAsyncTaskSessionStateChange
{
	/** Whether to shrink the session size to the number of arbitrated players */
	UBOOL bUsesArbitration;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InSessionName the name of the session involved
	 * @param InbUsesArbitration true if using arbitration, false otherwise
	 * @param InScriptDelegates the delegate to fire off when complete
	 * @param InAsyncTaskName the name of the task for debugging purposes
	 */
	FLiveAsyncTaskStartSession(FName InSessionName,UBOOL InbUsesArbitration,
		TArray<FScriptDelegate>* InScriptDelegates,const TCHAR* InAsyncTaskName) :
		FLiveAsyncTaskSessionStateChange(InSessionName,OGS_InProgress,InScriptDelegates,InAsyncTaskName),
		bUsesArbitration(InbUsesArbitration)
	{
	}

	/**
	 * Checks the arbitration flag and issues a session size change if arbitrated
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

/**
 * Once the session has been created, this class automatically registers
 * the list of local players
 */
class FLiveAsyncTaskCreateSession :
	public FOnlineAsyncTaskLiveNamedSession
{
	/** The player registering the session */
	DWORD HostingPlayerNum;
	/** Whether this is a create or a join */
	UBOOL bIsCreate;
	/** Whether the join is from an invite or from a matchmaking search */
	UBOOL bIsFromInvite;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InSessionName the name of the session involved
	 * @param InHostingPlayerNum the player hosting the session
	 * @param InIsCreate whether we are creating or joining a match
	 * @param InIsFromInvite whether this join is from an invite or search
	 */
	FLiveAsyncTaskCreateSession(FName InSessionName,DWORD InHostingPlayerNum,UBOOL InIsCreate = TRUE,UBOOL InIsFromInvite = FALSE) :
		FOnlineAsyncTaskLiveNamedSession(InSessionName,NULL,NULL,TEXT("XSessionCreate()")),
		HostingPlayerNum(InHostingPlayerNum),
		bIsCreate(InIsCreate),
		bIsFromInvite(InIsFromInvite)
	{
	}

	/**
	 * Routes the call to the function on the subsystem for parsing search results
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

/**
 * Holds the buffer used to register all of the local players
 */
class FLiveAsyncTaskDataRegisterLocalPlayers :
	public FLiveAsyncTaskData
{
	/** Holds the list of indices to join the session */
	DWORD Players[4];
	/** Indicates how many local players are joining */
	DWORD Count;
	/** Private slot list */
	BOOL PrivateSlots[4];

public:
	/** Zeroes members */
	FLiveAsyncTaskDataRegisterLocalPlayers(void) :
		Count(0)
	{
		appMemzero(PrivateSlots,sizeof(BOOL) * 4);
	}

	/** Operator that gives access to the data buffer */
	const DWORD* GetPlayers(void)
	{
		return Players;
	}

	/** Returns the number of players being locally registered */
	FORCEINLINE DWORD GetCount(void)
	{
		return Count;
	}

	/** Returns aset of private slots used */
	FORCEINLINE BOOL* GetPrivateSlots(void)
	{
		return PrivateSlots;
	}

	/**
	 * Sets the number of specified slots as private
	 *
	 * @param NumSlots the number of private slots to consume
	 */
	FORCEINLINE void SetPrivateSlotsUsed(DWORD NumSlots)
	{
		for (DWORD Index = 0; Index < NumSlots; Index++)
		{
			PrivateSlots[Index] = TRUE;
		}
	}

	/**
	 * Adds a player's index to the list to register
	 */
	FORCEINLINE void AddPlayer(DWORD Index)
	{
		Players[Count++] = Index;
	}
};

/**
 * Holds the data needed to iterate over an enumeration
 */
class FLiveAsyncTaskDataEnumeration :
	public FLiveAsyncTaskData
{
	/** The handle to the enumerator */
	HANDLE Handle;
	/** Block of data used to hold the results */
	BYTE* Data;
	/** Size of the data in bytes */
	DWORD SizeNeeded;
	/** The number of items left to read */
	DWORD NumToRead;
	/** Number of items that were read */
	DWORD ReadCount;
	/** Whether all possible items have been read or not */
	UBOOL bIsAtEndOfList;
	/** The player this read is for */
	DWORD PlayerIndex;


protected:
	/** Hidden to prevent usage */
	FLiveAsyncTaskDataEnumeration() {}

public:
	/**
	 * Initializes members and allocates the buffer needed to read the enumeration
	 *
	 * @param InPlayerIndex the player we are enumerating for
	 * @param InHandle the handle of the enumeration task
	 * @param InSizeNeeded the size of the buffer to allocate
	 * @param InNumToRead the number of items to read from Live
	 */
	FLiveAsyncTaskDataEnumeration(DWORD InPlayerIndex,HANDLE InHandle,DWORD InSizeNeeded,DWORD InNumToRead) :
		Handle(InHandle),
		Data(NULL),
		SizeNeeded(InSizeNeeded),
		NumToRead(InNumToRead),
		ReadCount(0),
		bIsAtEndOfList(FALSE),
		PlayerIndex(InPlayerIndex)
	{
		// Allocate our buffer
		Data = new BYTE[SizeNeeded];
	}

	/**
	 * Frees the resources allocated by this class
	 */
	virtual ~FLiveAsyncTaskDataEnumeration(void)
	{
		delete [] Data;
		CloseHandle(Handle);
	}

	/** Accesses the handle member*/
	FORCEINLINE HANDLE GetHandle(void)
	{
		return Handle;
	}

	/** Accesses the data buffer we allocated */
	FORCEINLINE VOID* GetBuffer(void)
	{
		return (VOID*)Data;
	}

	/** Returns a pointer to the DWORD that gets the number of items returned */
	FORCEINLINE DWORD* GetReturnCountStorage(void)
	{
		return &ReadCount;
	}

	/** Returns the number of friends items that were read */
	FORCEINLINE DWORD GetNumRead(void) const
	{
		return ReadCount;
	}

	/** Updates the number of friends left to read */
	FORCEINLINE void UpdateAmountToRead(void)
	{
		check(NumToRead >= ReadCount);
		NumToRead -= ReadCount;
	}

	/** Returns the player index this read is associated with */
	FORCEINLINE DWORD GetPlayerIndex(void)
	{
		return PlayerIndex;
	}

	/** Returns the size of the buffer that was allocated */
	FORCEINLINE DWORD GetBufferSize(void)
	{
		return SizeNeeded;
	}
};

/**
 * Holds the data to enumerate over content as well as open it
 */
class FLiveAsyncTaskContent :
	public FLiveAsyncTaskDataEnumeration
{
	/** Unique drive name that maps to the content files */
	FString ContentDrive;

	/** Friendly that can be used to display the content if needed */
	FString FriendlyName;

	/** Hidden to prevent usage */
	FLiveAsyncTaskContent() {}

public:

	/**
	 * Initializes members and allocates the buffer needed to read the enumeration
	 *
	 * @param InPlayerIndex the player we are enumerating for
	 * @param InHandle the handle of the enumeration task
	 * @param InSizeNeeded the size of the buffer to allocate
	 * @param InNumToRead the number of items to read from Live
	 */
	FLiveAsyncTaskContent(DWORD InPlayerIndex,HANDLE InHandle,DWORD InSizeNeeded,DWORD InNumToRead) :
	  FLiveAsyncTaskDataEnumeration(InPlayerIndex, InHandle, InSizeNeeded, InNumToRead)
	{
	}

	/** Accesses the content drive member*/
	FORCEINLINE const FString& GetContentDrive(void)
	{
		return ContentDrive;
	}

	/** Sets the content drive member*/
	FORCEINLINE void SetContentDrive(const FString& InContentDrive)
	{
		ContentDrive = InContentDrive;
	}

	/** Accesses the friendly name member*/
	FORCEINLINE const FString& GetFriendlyName(void)
	{
		return FriendlyName;
	}

	/** Sets the friendly name member*/
	FORCEINLINE void SetFriendlyName(const FString& InFriendlyName)
	{
		FriendlyName = InFriendlyName;
	}
};

/** Common class for all enumeration tasks (handles EOF error codes) */
class FLiveAsyncTaskEnumeration :
	public FOnlineAsyncTaskLive
{
	/** Hidden on purpose */
	FLiveAsyncTaskEnumeration(void) {}

public:
	/**
	 * Forwards the call to the base class
	 *
	 * @param InScriptDelegates the delegates to fire off when complete
	 * @param InTaskData the data associated with the task. Freed when compelete
	 * @param InAsyncTaskName pointer to static data identifying the type of task
	 */
	FLiveAsyncTaskEnumeration(TArray<FScriptDelegate>* InScriptDelegates,FLiveAsyncTaskData* InTaskData,const TCHAR* InAsyncTaskName) :
		FOnlineAsyncTaskLive(InScriptDelegates,InTaskData,InAsyncTaskName)
	{
	}

	/**
	 * Returns the status code of the completed task. Assumes the task
	 * has finished.
	 *
	 * @return ERROR_SUCCESS if ok, an HRESULT error code otherwise
	 */
	virtual DWORD GetCompletionCode(void)
	{
		DWORD Result = XGetOverlappedExtendedError(&Overlapped);
		// Handle being at the end of the list as an OK thing
		return Result == 0x80070012 ? ERROR_SUCCESS : Result;
	}
};

/**
 * This class parses the results of a friends request read and continues to read
 * the list until all friends are read
 */
class FLiveAsyncTaskReadFriends :
	public FLiveAsyncTaskEnumeration
{
	/** Array of XUIDs to read the presence of */
	TArray<XUID> PresenceList;
	/** The state the async task is in */
	enum
	{
		// Reading the list of XUIDs that makes up the friends list
		ReadingFriendsXuids,
		// Reading the presence information from Live
		ReadingFriendsPresence
	} ReadState;
	/** Used to enumerate the presence information for the friends list */
	HANDLE PresenceHandle;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InScriptDelegates the delegate to fire off when complete
	 * @param InTaskData the data associated with the task. Freed when compelete
	 */
	FLiveAsyncTaskReadFriends(TArray<FScriptDelegate>* InScriptDelegates,FLiveAsyncTaskData* InTaskData) :
		FLiveAsyncTaskEnumeration(InScriptDelegates,InTaskData,TEXT("XFriendsCreateEnumerator()")),
		ReadState(ReadingFriendsXuids),
		PresenceHandle(NULL)
	{
	}

	/** Clean up the handle when done */
	virtual ~FLiveAsyncTaskReadFriends()
	{
		if (PresenceHandle != NULL)
		{
			CloseHandle(PresenceHandle);
		}
	}

	/**
	 * Routes the call to the function on the subsystem for parsing friends
	 * results. Also, continues searching as needed until there are no more
	 * friends to read
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

enum EContentTaskMode
{
	/** The task is currently enumerating content packages */
	CTM_Enumerate,
	/** The task is currently opening (creating) a content package */
	CTM_Create,
};

/**
 * This class parses the results of a content request read and continues to read
 * the list until all content is read
 */
class FLiveAsyncTaskReadContent :
	public FLiveAsyncTaskEnumeration
{
private:
	/** What the task is currently performing */
	EContentTaskMode TaskMode;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InScriptDelegates the delegate to fire off when complete
	 * @param InTaskData the data associated with the task. Freed when compelete
	 */
	FLiveAsyncTaskReadContent(TArray<FScriptDelegate>* InScriptDelegates, FLiveAsyncTaskData* InTaskData) :
		FLiveAsyncTaskEnumeration(InScriptDelegates,InTaskData,TEXT("XContentCreateEnumerator()")),
		TaskMode(CTM_Enumerate)
	{
	}

	/**
	 * Routes the call to the function on the subsystem for parsing friends
	 * results. Also, continues searching as needed until there are no more
	 * friends to read
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

/**
 * Holds the buffer used in the async keyboard input
 */
class FLiveAsyncTaskDataKeyboard :
	public FLiveAsyncTaskData
{
	/** The chunk of memory to place the string results in */
	TCHAR* KeyboardInputBuffer;
	/** Whether to use the text vetting APIs to validate the results */
	UBOOL bShouldValidate;
	/** Holds the data needed by the validation API */
	STRING_DATA ValidationInput;
	/** Holds the validation results buffer */
	BYTE* ValidationResults;
	/** The text to place in the edit box by default */
	FString DefaultText;
	/** The text to place in the title */
	FString TitleText;
	/** The description of what the user is entering */
	FString DescriptionText;

	/** Hidden on purpose */
	FLiveAsyncTaskDataKeyboard(void) {}

public:
	/**
	 * Allocates an internal buffer of the specified size and sets
	 * whether we need validation or not
	 *
	 * @param InTitleText the title text to use
	 * @param InDefaultText the default text to use
	 * @param InDescriptionText the description to show the user
	 * @param Length the number of TCHARs to hold
	 * @param bValidate whether to validate the string or not
	 */
	inline FLiveAsyncTaskDataKeyboard(const FString& InTitleText,
		const FString& InDefaultText,const FString& InDescriptionText,
		DWORD Length,UBOOL bValidate) :
		bShouldValidate(bValidate),
		ValidationResults(NULL),
		DefaultText(InDefaultText),
		TitleText(InTitleText),
		DescriptionText(InDescriptionText)
	{
		KeyboardInputBuffer = new TCHAR[Length + 1];
		// Allocate the buffer if the results should be validated
		if (bShouldValidate)
		{
			appMemzero(&ValidationInput,sizeof(STRING_DATA));
			// Allocate and zero the results buffer
			ValidationResults = new BYTE[sizeof(STRING_VERIFY_RESPONSE) + sizeof(HRESULT)];
			appMemzero(ValidationResults,sizeof(STRING_VERIFY_RESPONSE) + sizeof(HRESULT));
		}
	}

	/** Base destructor. Here to force proper destruction */
	virtual ~FLiveAsyncTaskDataKeyboard(void)
	{
		delete [] KeyboardInputBuffer;
		delete ValidationResults;
	}

	/** Operator that gives access to the string buffer */
	inline operator TCHAR*(void)
	{
		return KeyboardInputBuffer;
	}

	/** Operator that gives access to the results buffer */
	inline operator STRING_VERIFY_RESPONSE*(void)
	{
		return (STRING_VERIFY_RESPONSE*)ValidationResults;
	}

	/** Operator that gives access to the validation input buffer */
	inline operator STRING_DATA*(void)
	{
		return &ValidationInput;
	}

	/** Indicates whether we need to validate the data too */
	inline UBOOL NeedsStringValidation(void)
	{
		return bShouldValidate;
	}

	/** Returns the pointer to the default text */
	inline const TCHAR* GetDefaultText(void)
	{
		return *DefaultText;
	}

	/** Returns the pointer to the description text */
	inline const TCHAR* GetDescriptionText(void)
	{
		return *DescriptionText;
	}

	/** Returns the pointer to the title text */
	inline const TCHAR* GetTitleText(void)
	{
		return *TitleText;
	}
};

/**
 * This class parses the results of a keyboard input session and optionally
 * validates the string with the string vetting API
 */
class FLiveAsyncTaskKeyboard :
	public FOnlineAsyncTaskLive
{
	/** Whether the task is currently waiting on validation results or not */
	UBOOL bIsValidating;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InScriptDelegates the delegate to fire off when complete
	 * @param InTaskData the data associated with the task. Freed when compelete
	 */
	FLiveAsyncTaskKeyboard(TArray<FScriptDelegate>* InScriptDelegates,FLiveAsyncTaskDataKeyboard* InTaskData) :
		FOnlineAsyncTaskLive(InScriptDelegates,InTaskData,TEXT("XShowKeyboardUI()")),
		bIsValidating(FALSE)
	{
	}

	/**
	 * Copies the resulting string into subsytem buffer. Optionally, will start
	 * another async task to validate the string if requested
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

/**
 * Holds the buffer used in the read async call
 */
class FLiveAsyncTaskDataWriteAchievement :
	public FLiveAsyncTaskData
{
	XUSER_ACHIEVEMENT Achievement;

	/** Hidden on purpose */
	FLiveAsyncTaskDataWriteAchievement(void) {}

public:
	/**
	 * Sets the achievement data
	 *
	 * @param InUserIndex the user the achievement is for
	 * @param InAchievementId the achievement being written
	 */
	FLiveAsyncTaskDataWriteAchievement(DWORD InUserIndex,DWORD InAchievementId)
	{
		Achievement.dwUserIndex = InUserIndex;
		Achievement.dwAchievementId = InAchievementId;
	}

	/**
	 * Returns a pointer to the achievement buffer that will persist througout
	 * the async call
	 */
	FORCEINLINE PXUSER_ACHIEVEMENT GetAchievement(void)
	{
		return &Achievement;
	}
};

#if !WITH_PANORAMA
/**
 * Holds the buffer and user used in the read async call
 */
class FLiveAsyncTaskDataQueryDownloads :
	public FLiveAsyncTaskData
{
	/** Holds the results of the async downloadable content query */
	XOFFERING_CONTENTAVAILABLE_RESULT AvailContent;
	/** The user this is being done for */
	DWORD UserIndex;

	/** Hidden on purpose */
	FLiveAsyncTaskDataQueryDownloads(void) {}

public:
	/**
	 * Sets the user that is performing the query
	 *
	 * @param InUserIndex the user the achievement is for
	 */
	FLiveAsyncTaskDataQueryDownloads(DWORD InUserIndex)
	{
		UserIndex = InUserIndex;
	}

	/**
	 * Returns a pointer to the content availability query that will persist
	 * througout the async call
	 */
	FORCEINLINE XOFFERING_CONTENTAVAILABLE_RESULT* GetQuery(void)
	{
		return &AvailContent;
	}

	/** Returns the user index for this query */
	FORCEINLINE DWORD GetUserIndex(void)
	{
		return UserIndex;
	}
};

/**
 * This class parses the results of a keyboard input session and optionally
 * validates the string with the string vetting API
 */
class FLiveAsyncTaskQueryDownloads :
	public FOnlineAsyncTaskLive
{
public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InScriptDelegates the delegate to fire off when complete
	 * @param InTaskData the data associated with the task. Freed when compelete
	 */
	FLiveAsyncTaskQueryDownloads(TArray<FScriptDelegate>* InScriptDelegates,FLiveAsyncTaskDataQueryDownloads* InTaskData) :
		FOnlineAsyncTaskLive(InScriptDelegates,InTaskData,TEXT("XContentGetMarketplaceCounts()"))
	{
	}

	/**
	 * Copies the download query results into the per user storage on the subsystem
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};
#endif

/** The max number of views that can be written to/read from */
#define MAX_VIEWS 5

/** The max number of stat columns that can be written/read */
#define MAX_STATS 64

/** The max number of players that we can read stats for */
#define MAX_PLAYERS_TO_READ	32

/**
 * This class holds the stats data for async operation and handles the
 * notification of completion
 */
class FLiveAsyncTaskWriteStats :
	public FOnlineAsyncTaskLive
{
	/** The view data needed for async completion */
	XSESSION_VIEW_PROPERTIES Views[MAX_VIEWS];
	/** Holds the properties that are written to the views */
	XUSER_PROPERTY Stats[MAX_STATS];

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InScriptDelegates the delegate to fire off when complete
	 */
	FLiveAsyncTaskWriteStats(TArray<FScriptDelegate>* InScriptDelegates) :
		FOnlineAsyncTaskLive(InScriptDelegates,NULL,TEXT("XSessionWriteStats()"))
	{
		appMemzero(Views,sizeof(XSESSION_VIEW_PROPERTIES) * MAX_VIEWS);
		appMemzero(Stats,sizeof(XUSER_PROPERTY) * MAX_STATS);
	}

	/** Accessor that returns the buffer that needs to hold the async view data */
	FORCEINLINE XSESSION_VIEW_PROPERTIES* GetViews(void)
	{
		return Views;
	}

	/** Accessor that returns the buffer that needs to hold the async stats data */
	FORCEINLINE XUSER_PROPERTY* GetStats(void)
	{
		return Stats;
	}
};

/**
 * This class holds the stats spec data for async operation and handles the
 * notification of completion
 */
class FLiveAsyncTaskReadStats :
	public FOnlineAsyncTaskLive
{
	/** The title that is being read from (zero is current) */
	DWORD TitleId;
	/** The stats spec data that tells live what to read */
	XUSER_STATS_SPEC StatSpecs[MAX_VIEWS];
	/** The set of players that we are doing the read for */
	XUID Players[MAX_PLAYERS_TO_READ];
	/** The full set of players to read (in case paging must happen) */
	TArray<FUniqueNetId> PlayersToRead;
	/** The number of players in the read request from Live */
	DWORD NumToRead;
	/** The buffer size that was allocated */
	DWORD BufferSize;
	/** The buffer that holds the stats read results */
	BYTE* ReadResults;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InTitleId the title id to read from
	 * @param NetIds the set of people to read leaderboards for
	 * @param InScriptDelegates the delegate to fire off when complete
	 */
	FLiveAsyncTaskReadStats(DWORD InTitleId,const TArray<FUniqueNetId>& NetIds,TArray<FScriptDelegate>* InScriptDelegates) :
		FOnlineAsyncTaskLive(InScriptDelegates,NULL,TEXT("XUserReadStats()")),
		TitleId(InTitleId),
		ReadResults(NULL),
		NumToRead(0),
		BufferSize(0)
	{
		appMemzero(StatSpecs,sizeof(XUSER_STATS_SPEC) * MAX_VIEWS);
		appMemzero(Players,sizeof(XUID) * MAX_PLAYERS_TO_READ);
		// Copy the full set to read
		PlayersToRead = NetIds;
		UpdatePlayersToRead();
	}

	/** Frees any allocated memory */
	virtual ~FLiveAsyncTaskReadStats(void)
	{
		delete [] ReadResults;
	}

	/** Updates the players data and the number of items being read */
	FORCEINLINE void UpdatePlayersToRead(void)
	{
		// Figure out how many we are copying
		NumToRead = Min(PlayersToRead.Num(),MAX_PLAYERS_TO_READ);
		// Copy the first set of XUIDs over
		appMemcpy(Players,PlayersToRead.GetData(),sizeof(XUID) * NumToRead);
		// Now remove the number we copied from the full set. If there are any
		// remaining they will be read subsequently
		PlayersToRead.Remove(0,NumToRead);
	}

	/** Accessor that returns the buffer that needs to hold the async spec data */
	FORCEINLINE XUSER_STATS_SPEC* GetSpecs(void)
	{
		return StatSpecs;
	}

	/** Accessor that returns the buffer that holds the players we are reading data for */
	FORCEINLINE XUID* GetPlayers(void)
	{
		return Players;
	}

	/** Returns the number of players in the current read request */
	FORCEINLINE DWORD GetPlayerCount(void)
	{
		return NumToRead;
	}

	/** Returns the of the read buffer */
	FORCEINLINE DWORD GetBufferSize(void)
	{
		return BufferSize;
	}

	/**
	 * Allocates the space requested as the results buffer
	 *
	 * @param Size the number of bytes to allocate
	 */
	FORCEINLINE void AllocateResults(DWORD Size)
	{
		BufferSize = Size;
		ReadResults = new BYTE[Size];
	}

	/** Accessor that returns the buffer to use to hold the read results */
	FORCEINLINE XUSER_STATS_READ_RESULTS* GetReadResults(void)
	{
		return (XUSER_STATS_READ_RESULTS*)ReadResults;
	}

	/**
	 * Tells the Live subsystem to parse the results of the stats read
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

/**
 * This class holds the enumeration handle and buffers for async reads
 */
class FLiveAsyncTaskReadStatsByRank :
	public FOnlineAsyncTaskLive
{
	/** The stats spec data that tells live what to read */
	XUSER_STATS_SPEC StatSpecs[MAX_VIEWS];
	/** The handle associated with the async enumeration */
	HANDLE hEnumerate;
	/** The buffer that holds the stats read results */
	BYTE* ReadResults;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InScriptDelegates the delegate to fire off when complete
	 */
	FLiveAsyncTaskReadStatsByRank(TArray<FScriptDelegate>* InScriptDelegates) :
		FOnlineAsyncTaskLive(InScriptDelegates,NULL,TEXT("XUserCreateStatsEnumeratorByRank()")),
		hEnumerate(NULL),
		ReadResults(NULL)
	{
		appMemzero(StatSpecs,sizeof(XUSER_STATS_SPEC) * MAX_VIEWS);
	}

	/** Frees any allocated memory */
	virtual ~FLiveAsyncTaskReadStatsByRank(void)
	{
		delete [] ReadResults;
		if (hEnumerate != NULL)
		{
			CloseHandle(hEnumerate);
		}
	}

	/** Accessor that returns the buffer to use to hold the read results */
	FORCEINLINE XUSER_STATS_READ_RESULTS* GetReadResults(void)
	{
		return (XUSER_STATS_READ_RESULTS*)ReadResults;
	}

	/** Accessor that returns the buffer that needs to hold the async spec data */
	FORCEINLINE XUSER_STATS_SPEC* GetSpecs(void)
	{
		return StatSpecs;
	}

	/**
	 * Initializes the handle and buffers for the returned data
	 *
	 * @param hIn the handle for the enumerate
	 * @param Size the size of the buffer to allocate
	 */
	FORCEINLINE void Init(HANDLE hIn,DWORD Size)
	{
		hEnumerate = hIn;
		ReadResults = new BYTE[Size];
	}

	/**
	 * Tells the Live subsystem to parse the results of the stats read
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

/**
 * This class handles the steps needed to join a game via invite
 */
class FLiveAsyncTaskJoinGameInvite :
	public FOnlineAsyncTaskLive
{
	/** The user index that is joining the game */
	DWORD UserNum;
	/** The buffer that holds the game search results */
	BYTE* Results;
	/** The current state the task is in */
	DWORD State;
	/** The settings object we are working on */
	UOnlineGameSettings* InviteSettings;
	/** Holds the QoS data returned by the QoS query */
	XNQOS* QosData;
	/** The array of server addresses to query */
	XNADDR* ServerAddrs[1];
	/** The array of server session ids to use for the query */
	XNKID* ServerKids[1];
	/** The array of server session keys to use for the query */
	XNKEY* ServerKeys[1];

	enum { WaitingForSearch, QueryingQos, InviteReady };

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InUserNum the user doing the game invite
	 * @param SearchSize the amount of data needed for the results
	 * @param Delegates the delegates to fire off when complete
	 */
	FLiveAsyncTaskJoinGameInvite(DWORD InUserNum,DWORD SearchSize,TArray<FScriptDelegate>* Delegates) :
		FOnlineAsyncTaskLive(Delegates,NULL,TEXT("XInviteGetAcceptedInfo/XSessionSearchByID()")),
		UserNum(InUserNum),
		State(WaitingForSearch),
		InviteSettings(NULL),
		QosData(NULL)
	{
		Results = new BYTE[SearchSize];
	}

	/** Frees any allocated memory */
	virtual ~FLiveAsyncTaskJoinGameInvite(void)
	{
		delete [] Results;
	}

	/** Accessor to the data buffer */
	FORCEINLINE PXSESSION_SEARCHRESULT_HEADER GetResults(void)
	{
		return (PXSESSION_SEARCHRESULT_HEADER)Results;
	}

	/**
	 * Reads the qos data for the server that was sending the invite
	 *
	 * @param Search the game search to update
	 */
	void RequestQoS(UOnlineGameSearch* Search);

	/**
	 * Parses the qos data that came back from the server
	 *
	 * @param Search the game search to update
	 */
	void ParseQoS(UOnlineGameSearch* Search);

	/**
	 * Tells the Live subsystem to continue the async game invite join
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

/**
 * This class handles the async task/data for arbitration registration
 */
class FLiveAsyncTaskArbitrationRegistration :
	public FOnlineAsyncTaskLiveNamedSession
{
	/** The buffer that holds the registration results */
	BYTE* Results;

public:
	/**
	 * Allocates the result buffer and calls base initialization
	 *
	 * @param InSessionName the name of the session involved
	 * @param BufferSize the size needed for the registration results buffer
	 * @param InScriptDelegates the script delegate to call when done
	 */
	FLiveAsyncTaskArbitrationRegistration(FName InSessionName,DWORD BufferSize,TArray<FScriptDelegate>* InScriptDelegates) :
		FOnlineAsyncTaskLiveNamedSession(InSessionName,InScriptDelegates,NULL,TEXT("XSessionArbitrationRegister()"))
	{
		Results = new BYTE[BufferSize];
	}

	/** Frees any allocated memory */
	virtual ~FLiveAsyncTaskArbitrationRegistration(void)
	{
		delete [] Results;
	}

	/** Accessor to the data buffer */
	FORCEINLINE PXSESSION_REGISTRATION_RESULTS GetResults(void)
	{
		return (PXSESSION_REGISTRATION_RESULTS)Results;
	}

	/**
	 * Parses the arbitration results and stores them in the arbitration list
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

#pragma pack(push,8)

/**
 * This class holds the data used for un/registering a player in a session
 * asynchronously
 */
class FLiveAsyncPlayer :
	public FOnlineAsyncTaskLiveNamedSession
{
protected:
	/** The XUID of the player that is joining */
	XUID PlayerXuid;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InSessionName the name of the session involved
	 * @param InScriptDelegates the delegate to fire off when complete
	 * @param Type the type of player action happening
	 */
	FLiveAsyncPlayer(FName InSessionName,XUID InXuid,TArray<FScriptDelegate>* InScriptDelegates,const TCHAR* Type) :
		FOnlineAsyncTaskLiveNamedSession(InSessionName,InScriptDelegates,NULL,Type),
		PlayerXuid(InXuid)
	{
	}

	/** Accessor that returns the buffer holding the xuids */
	FORCEINLINE XUID* GetXuids(void)
	{
		return &PlayerXuid;
	}

	/**
	 * Checks to see if the match is arbitrated and shrinks it by one if it is
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

/**
 * This class holds the data used for registering a player in a session
 * asynchronously
 */
class FLiveAsyncRegisterPlayer :
	public FLiveAsyncPlayer
{
	/** The array of private invite flags */
	BOOL bPrivateInvite;
	/** Whether this is the second attempt or not */
	UBOOL bIsSecondTry;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InSessionName the name of the session involved
	 * @param InXuid the xuid of the player being registered
	 * @param bWasInvite whether to use private/public slots
	 * @param InScriptDelegates the delegate to fire off when complete
	 */
	FLiveAsyncRegisterPlayer(FName InSessionName,XUID InXuid,UBOOL bWasInvite,TArray<FScriptDelegate>* InScriptDelegates) :
		FLiveAsyncPlayer(InSessionName,InXuid,InScriptDelegates,TEXT("XSessionJoinRemote()")),
		bPrivateInvite(bWasInvite),
		bIsSecondTry(FALSE)
	{
	}

	/** Accessor that returns the buffer holding the private invite flags */
	FORCEINLINE BOOL* GetPrivateInvites(void)
	{
		return &bPrivateInvite;
	}

	/**
	 * Checks to see if the join worked. If this was an invite it may need to
	 * try private and then public.
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

#pragma pack(pop)

/**
 * This class holds the data used for handling destruction callback
 */
class FLiveAsyncDestroySession :
	public FOnlineAsyncTaskLiveNamedSession
{
	/** The handle of the session to close after deleting */
	HANDLE Handle;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InSessionName the name of the session involved
	 * @param InHandle the handle to close once the task is complete
	 * @param InScriptDelegates the delegate to fire off when complete
	 */
	FLiveAsyncDestroySession(FName InSessionName,HANDLE InHandle,TArray<FScriptDelegate>* InScriptDelegates) :
		FOnlineAsyncTaskLiveNamedSession(InSessionName,InScriptDelegates,NULL,TEXT("XSessionDelete()")),
		Handle(InHandle)
	{
	}

	/** Cleans up the handle upon deletion */
	~FLiveAsyncDestroySession()
	{
		if (Handle != NULL)
		{
			CloseHandle(Handle);
		}
	}
};

/**
 * This class holds the task data used for updating a session's skill
 */
class FLiveAsyncUpdateSessionSkill:
	public FOnlineAsyncTaskLiveNamedSession
{
	/** The list of players to use to update the skill */
	TArray <FUniqueNetId> Xuids;

public:
	/**
	 * Copies the xuids and does base initialization
	 *
	 * @param InSessionName the name of the session involved
	 * @param InPlayers the list of players to use
	 */
	FLiveAsyncUpdateSessionSkill(FName InSessionName,const TArray<FUniqueNetId>& InPlayers) :
		FOnlineAsyncTaskLiveNamedSession(InSessionName,NULL,NULL,TEXT("XSessionModifySkill()")),
		Xuids(InPlayers)
	{
	}

	/** Returns a pointer to the list of xuids for use in the async task */
	FORCEINLINE XUID* GetXuids(void)
	{
		return (XUID*)Xuids.GetData();
	}

	/** Returns the number of xuids in the array */
	FORCEINLINE DWORD GetCount(void)
	{
		return (DWORD)Xuids.Num();
	}

	/**
	 * Marks the skill in progress flag as false
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

#if !CONSOLE
/**
 * This class holds the task used for signing in users asynchronously
 */
class FLiveAsyncTaskSignin :
	public FOnlineAsyncTaskLive
{
public:
	/**
	 * Does base initialization
	 */
	FLiveAsyncTaskSignin(void) :
		FOnlineAsyncTaskLive(NULL,NULL,TEXT("XLiveSignin()"))
	{
	}

	/**
	 * Checks the results of the signin operation and shuts down the server if
	 * it fails
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

/**
 * This class holds the task used for signing out users asynchronously
 */
class FLiveAsyncTaskSignout :
	public FOnlineAsyncTaskLive
{
public:
	/**
	 * Does base initialization
	 */
	FLiveAsyncTaskSignout(void) :
		FOnlineAsyncTaskLive(NULL,NULL,TEXT("XLiveSignout()"))
	{
	}
};
#endif

/**
 * This class holds the task data used to send game invites to a series of people
 */
class FLiveAsyncTaskInviteToGame :
	public FOnlineAsyncTaskLive
{
	/** The set of people to send the invite to */
	TArray<FUniqueNetId> Invitees;
	/** The text to be sent as part of the invite */
	FString Message;

public:
	/**
	 * Does base initialization
	 */
	FLiveAsyncTaskInviteToGame(const TArray<FUniqueNetId>& InInvitees,const FString& InMessage) :
		FOnlineAsyncTaskLive(NULL,NULL,TEXT("XInviteSend()")),
		Invitees(InInvitees),
		Message(InMessage)
	{
	}

	/** Returns the number of invitees to send to */
	FORCEINLINE DWORD GetInviteeCount(void)
	{
		return (DWORD)Invitees.Num();
	}

	/** Returns the set of invitees to send to */
	FORCEINLINE XUID* GetInvitees(void)
	{
		return (XUID*)Invitees.GetTypedData();
	}

	/** Returns the message that accompanies the invite */
	FORCEINLINE const TCHAR* GetMessage(void)
	{
		return *Message;
	}
};

/** Only 50 files up to 5MB each in size */
#define MAX_TITLE_MANAGED_STORAGE_FILES 50

/**
 * This class holds the data used for reading the set of files that should
 * be downloaded and merged into the config cache
 */
class FLiveAsyncTMSRead :
	public FOnlineAsyncTaskLive
{
	/** The set of files to download from Live */
	XSTORAGE_ENUMERATE_RESULTS* FilesReturned;
	/** The amount of data allocated in the pointer above */
	DWORD AmountAllocated;
	/** The next file to read asynchronously */
	INT NextFileToRead;
	/** The buffer used to stream the file into from Live */
	BYTE* FileBuffer;
	/** The size of the file buffer */
	DWORD FileBufferSize;
	/** Holds the information about the file download */
	XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS FileDownloadResults;
	/** Keep track of the files that were read */
	TArray<FTitleFile>& FilesRead;

public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InTitleFiles the array to output title files into
	 * @param InScriptDelegates the delegates to fire when complete
	 */
	FLiveAsyncTMSRead(TArray<FTitleFile>& InTitleFiles,TArray<FScriptDelegate>* InScriptDelegates) :
		FOnlineAsyncTaskLive(InScriptDelegates,NULL,TEXT("TMS file reading")),
		FilesReturned(NULL),
		NextFileToRead(-1),
		FileBuffer(NULL),
		FileBufferSize(0),
		FilesRead(InTitleFiles)
	{
		AmountAllocated = sizeof(XSTORAGE_ENUMERATE_RESULTS) +
			(MAX_TITLE_MANAGED_STORAGE_FILES * 
			(sizeof(XSTORAGE_FILE_INFO) + (XONLINE_MAX_PATHNAME_LENGTH * sizeof(WCHAR))));
		// Allocate enough memory for the file enumerations
		FilesReturned = (XSTORAGE_ENUMERATE_RESULTS*)new BYTE[AmountAllocated];
		appMemzero(FilesReturned,AmountAllocated);
	}

	/** Cleans up the handle upon deletion */
	~FLiveAsyncTMSRead()
	{
		delete [] FilesReturned;
		delete [] FileBuffer;
	}

	/** Gets a pointer to the enumeration results buffer */
	FORCEINLINE XSTORAGE_ENUMERATE_RESULTS* GetEnumerationResults(void)
	{
		return FilesReturned;
	}

	/** Returns the amount of data that was allocated to hold the enumeration results */
	FORCEINLINE DWORD GetAllocatedSize(void)
	{
		return AmountAllocated;
	}

	/**
	 * After getting the list of files that are to be downloaded, it downloads
	 * the files and then places them in the title managed files list
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);

	/**
	 * Iterates the list of delegates and fires those notifications
	 *
	 * @param Object the object that the notifications are going to be issued on
	 */
	virtual void TriggerDelegates(UObject* Object)
	{
		check(Object);
	}
};

/**
 * Holds the data to enumerate over content as well as open it
 */
class FLiveAsyncTaskDataReadAchievements :
	public FLiveAsyncTaskDataEnumeration
{
	/** The title id that is having achievements read for */
	DWORD TitleId;

public:

	/**
	 * Initializes members and allocates the buffer needed to read the enumeration
	 *
	 * @param InPlayerIndex the player we are enumerating for
	 * @param InHandle the handle of the enumeration task
	 * @param InSizeNeeded the size of the buffer to allocate
	 * @param InNumToRead the number of items to read from Live
	 */
	FLiveAsyncTaskDataReadAchievements(DWORD InTitleId,DWORD InPlayerIndex,HANDLE InHandle,DWORD InSizeNeeded,DWORD InNumToRead) :
		FLiveAsyncTaskDataEnumeration(InPlayerIndex,InHandle,InSizeNeeded,InNumToRead),
		TitleId(InTitleId)
	{
	}

	/** Returns the title id that is being read */
	FORCEINLINE DWORD GetTitleId(void)
	{
		return TitleId;
	}

	/** Accesses the data buffer we allocated */
	FORCEINLINE XACHIEVEMENT_DETAILS* GetDetailsBuffer(void)
	{
		return (XACHIEVEMENT_DETAILS*)GetBuffer();
	}
};

/**
 * This class parses the results of a achievements read request read and
 * continues to read the list until all are read
 */
class FLiveAsyncTaskReadAchievements :
	public FLiveAsyncTaskEnumeration
{
public:
	/**
	 * Forwards the call to the base class for proper initialization
	 *
	 * @param InScriptDelegates the delegate to fire off when complete
	 * @param InTaskData the data associated with the task. Freed when complete
	 */
	FLiveAsyncTaskReadAchievements(TArray<FScriptDelegate>* InScriptDelegates,FLiveAsyncTaskData* InTaskData) :
		FLiveAsyncTaskEnumeration(InScriptDelegates,InTaskData,TEXT("XUserCreateAchievementEnumerator()"))
	{
	}

	/**
	 * Parses the read results and continues the read if needed
	 *
	 * @param LiveSubsystem the object to add the data to
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);

	/**
	 * Iterates the list of delegates and fires those notifications
	 *
	 * @param Object the object that the notifications are going to be issued on
	 */
	virtual void TriggerDelegates(UObject* Object);
};

/**
 * This class holds the task data used to display a custom players list
 */
class FLiveAsyncTaskCustomPlayersList :
	public FOnlineAsyncTaskLive
{
	/** The set of people that will show up in the blade */
	TArray<XPLAYERLIST_USER> Players;
	/** The title to use for the blade */
	FString Title;
	/** The string to use at the top of the blade */
	FString Description;

public:
	/**
	 * Does base initialization
	 */
	FLiveAsyncTaskCustomPlayersList(const TArray<FUniqueNetId>& InPlayers,const FString& InTitle,const FString& InDescription) :
		FOnlineAsyncTaskLive(NULL,NULL,TEXT("XShowCustomPlayerUI()")),
		Title(InTitle),
		Description(InDescription)
	{
		// Copy the xuids into the array that the UI
		for (INT Index = 0; Index < InPlayers.Num(); Index++)
		{
			INT AddIndex = Players.AddZeroed();
			Players(AddIndex).xuid = InPlayers(Index).Uid;
		}
	}

	/** Returns the title string to use */
	FORCEINLINE const TCHAR* GetTitle(void)
	{
		return *Title;
	}

	/** Returns the description string to use */
	FORCEINLINE const TCHAR* GetDescription(void)
	{
		return *Description;
	}

	/** Returns the number of players to display */
	FORCEINLINE DWORD GetPlayerCount(void)
	{
		return (DWORD)Players.Num();
	}

	/** Returns the set of players that will be in the list */
	FORCEINLINE XPLAYERLIST_USER* GetPlayers(void)
	{
		return (XPLAYERLIST_USER*)Players.GetTypedData();
	}
};

/**
 * This class holds the task data used for updating a session's skill
 * using a specific set of players searching
 */
class FLiveAsyncReadPlayerSkillForSearch:
	public FOnlineAsyncTaskLive
{
	/** The player initiating the search once the skill read is complete */
	DWORD LocalUserNum;
	/** The game search object to update with skill info */
	UOnlineGameSearch* Search;
	/** The buffer that holds the read results */
	BYTE* Buffer;
	/** The size of the buffer in bytes */
	DWORD BufferSize;
	/** The stat read struct filled in with the skill read request data */
	XUSER_STATS_SPEC SkillSpec;

public:
	/**
	 * Initializes the members
	 *
	 * @param InUserNum the player executing the search
	 * @param InSearch the search object to write results to
	 */
	FLiveAsyncReadPlayerSkillForSearch(DWORD InUserNum,UOnlineGameSearch* InSearch) :
		FOnlineAsyncTaskLive(NULL,NULL,TEXT("Skill read for search override")),
		LocalUserNum(InUserNum),
		Search(InSearch),
		Buffer(NULL),
		BufferSize(0)
	{
		// Init the skill specs that we use to indicate what columns to read
		appMemzero(&SkillSpec,sizeof(XUSER_STATS_SPEC));
		SkillSpec.dwViewId = Search->ManualSkillOverride.LeaderboardId;
		SkillSpec.dwNumColumnIds = 2;
		SkillSpec.rgwColumnIds[0] = X_STATS_COLUMN_SKILL_MU;
		SkillSpec.rgwColumnIds[1] = X_STATS_COLUMN_SKILL_SIGMA;
	}

	/** Deletes the buffer used to hold the read results */
	~FLiveAsyncReadPlayerSkillForSearch()
	{
		delete Buffer;
	}

	/**
	 * Allocates the space needed to hold the read results
	 *
	 * @param Size the number of bytes to allocate
	 */
	void AllocateSpace(DWORD Size)
	{
		BufferSize = Size;
		Buffer = new BYTE[BufferSize];
	}

	/** Accessor that returns the buffer that needs to hold the async spec data */
	FORCEINLINE XUSER_STATS_SPEC* GetSpecs(void)
	{
		return &SkillSpec;
	}

	/** Returns the of the read buffer */
	FORCEINLINE DWORD GetBufferSize(void)
	{
		return BufferSize;
	}

	/** Accessor that returns the buffer to use to hold the read results */
	FORCEINLINE XUSER_STATS_READ_RESULTS* GetReadBuffer(void)
	{
		return (XUSER_STATS_READ_RESULTS*)Buffer;
	}

	/**
	 * If the skill read completes successfully, it then triggers the requested search
	 * If it fails, it uses the search delegates to notify the game code
	 *
	 * @param LiveSubsystem the object to make the final call on
	 *
	 * @return TRUE if this task should be cleaned up, FALSE to keep it
	 */
	virtual UBOOL ProcessAsyncResults(class UOnlineSubsystemLive* LiveSubsystem);
};

#include "VoiceInterface.h"

#include "OnlineSubsystemLiveClasses.h"

#endif	//#if WITH_UE3_NETWORKING

#endif
