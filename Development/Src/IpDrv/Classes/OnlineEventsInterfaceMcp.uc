/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Provides an in game gameplay events/stats upload mechanism via the MCP backend
 */
class OnlineEventsInterfaceMcp extends MCPBase
	native
	implements(OnlineEventsInterface)
	dependson(PartyBeacon);

/** The types of events that are to be uploaded */
enum EEventUploadType
{
	EUT_GenericStats,
	EUT_ProfileData,
	EUT_HardwareData,
	EUT_MatchmakingData
};

/** Holds the configuration and instance data for event uploading */
struct native EventUploadConfig
{
	/** The type of upload this config is for */
	var const EEventUploadType UploadType;
	/** The URL to send the data to */
	var const string UploadUrl;
	/** The amount of time to wait before erroring out */
	var const float TimeOut;
	/** Whether to compress the data before sending or not */
	var const bool bUseCompression;
};

/**
 * This is the array of upload task configurations
 */
var const config array<EventUploadConfig> EventUploadConfigs;

/** List of HTTP downloader objects that are POSTing the data */
var native const array<pointer> HttpPostObjects{class FHttpDownloadString};

/** A list of upload types that are disabled (don't upload) */
var config array<EEventUploadType> DisabledUploadTypes;

/** if true, the stats data will be sent as a binary blob instead of XML */
var const config bool bBinaryStats;

cpptext
{
// FTickableObject interface
	/**
	 * Ticks any outstanding async tasks that need processing
	 *
	 * @param DeltaTime the amount of time that has passed since the last tick
	 */
	virtual void Tick(FLOAT DeltaTime);

// Event upload specific methods
	/**
	 * Finds the upload config for the type
	 *
	 * @param UploadType the type of upload that is being processed
	 *
	 * @return pointer to the config item or NULL if not found
	 */
	inline FEventUploadConfig* FindUploadConfig(BYTE UploadType)
	{
		// Make sure this config wasn't disabled
		INT ItemIndex = DisabledUploadTypes.FindItemIndex(UploadType);
		if (ItemIndex == INDEX_NONE)
		{
			for (INT EventIndex = 0; EventIndex < EventUploadConfigs.Num(); EventIndex++)
			{
				if (EventUploadConfigs(EventIndex).UploadType == UploadType)
				{
					return &EventUploadConfigs(EventIndex);
				}
			}
		}
		return NULL;
	}

	/**
	 * Common method for POST-ing a payload to an URL (determined by upload type)
	 *
	 * @param UploadType the type of upload that is happening
	 * @param Payload the data to send
	 *
	 * @return TRUE if the send started successfully, FALSE otherwise
	 */
	virtual UBOOL UploadPayload(BYTE UploadType,const FString& Payload);

	/**
	 * Common method for POST-ing a payload to an URL (determined by upload type)
	 *
	 * @param UploadType the type of upload that is happening
	 * @param Payload the data to send
	 *
	 * @return TRUE if the send started successfully, FALSE otherwise
	 */
	virtual UBOOL UploadBinaryPayload(BYTE UploadType,const TArray<BYTE>& Payload);

	/**
	 * Final method for POST-ing a payload to a URL.  At this point it is assumed to be binary data
	 *
	 * @param bWasText will be true if the original post was text data
	 * @param UploadType the type of upload that is happening
	 * @param Payload the data to send
	 *
	 * @return TRUE if the send started successfully, FALSE otherwise
	 */	
	virtual UBOOL UploadFinalPayload(UBOOL bWasText, BYTE UploadType, const TArray<BYTE>& Payload );

	/**
	 * Converts the net id to a string
	 *
	 * @param Id the net id to convert
	 *
	 * @return the string form of the id
	 */
	virtual FString FormatAsString(const FUniqueNetId& Id)
	{
		return FString::Printf(TEXT("0x%016I64X"),(QWORD&)Id);
	}
}

/**
 * Initializes the events capture interface
 *
 * @param MaxNumEvents the maximum number of events that will be sent
 */
function bool Init(int MaxNumEvents);

/**
 * Begins a logging event (creates a new log entry)
 */
function BeginLog();

/**
 * Closes a logging event
 */
function EndLog();

/**
 * Creates an event within a log
 *
 * @param EventName the name to assign the event
 */
function BeginEvent(string EventName);

/**
 * Adds a parameter to the event log
 *
 * @param ParamName the name to assign the parameter
 * @param ParamValue the value to assign the parameter
 */
function AddParamInt(string ParamName, int ParamValue);

/**
 * Adds a parameter to the event log
 *
 * @param ParamName the name to assign the parameter
 * @param ParamValue the value to assign the parameter
 */
function AddParamFloat(string ParamName, float ParamValue);

/**
 * Adds a parameter to the event log
 *
 * @param ParamName the name to assign the parameter
 * @param ParamValue the value to assign the parameter
 */
function AddParamString(string ParamName, string ParamValue);

/**
 * Closes an event within a log
 */
function EndEvent();

/**
 * Uploads a log to a remote site
 */
function UploadLog();

/**
 * Writes the log to a local storage device
 */
function SaveLog();

/**
 * Sends the profile data to the server for statistics aggregation
 *
 * @param UniqueId the unique id for the player
 * @param PlayerNick the player's nick name
 * @param ProfileSettings the profile object that is being sent
 *
 * @return true if the async task was started successfully, false otherwise
 */
native function bool UploadProfileData(UniqueNetId UniqueId,string PlayerNick,OnlineProfileSettings ProfileSettings);

/**
 * Sends the data contained within the gameplay events object to the online server for statistics
 *
 * @param Events the object that has the set of events in it
 *
 * @return true if the async send started ok, false otherwise
 */
native function bool UploadGameplayEventsData(OnlineGameplayEvents Events);

/**
 * Sends the hardware data to the online server for statistics aggregation
 *
 * @param UniqueId the unique id for the player
 * @param PlayerNick the player's nick name
 *
 * @return true if the async task was started successfully, false otherwise
 */
function bool UploadHardwareData(UniqueNetId UniqueId,string PlayerNick);

/**
 * Uploads information about the matching search
 *
 * @param UniqueId the unique id for the player uploading
 * @param Search the search object that was used
 * @param SelectedIndex the match that was chosen
 * @param Players the group of players that were used to make the search
 *
 * @return true if the async task was started successfully, false otherwise
 */
function bool UploadMatchmakingSearchData(UniqueNetId UniqueId,OnlineGameSearch Search,int SelectedIndex,const out array<PlayerReservation> Players);

/**
 * Uploads information about the match session that was found
 *
 * @param GameSettings the game settings object that was used
 * @param Players the group of players that were used to register the session
 *
 * @return true if the async task was started successfully, false otherwise
 */
function bool UploadMatchmakingSessionData(OnlineGameSettings GameSettings,const out array<PlayerReservation> Players);
