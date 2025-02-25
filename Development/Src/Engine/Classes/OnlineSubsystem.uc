/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class holds a set of online interfaces that game code uses to talk
 * with the platform layer's services. The set of services are implemented
 * as interface pointers so that we can mix & match services at run time.
 * This also allows licensees the ability to use part of our base services
 * and provide custom implmentations of others.
 */
class OnlineSubsystem extends Object
	native
	abstract
	inherits(FTickableObject)
	dependson(Settings)
	config(Engine);

/** The interface to use for creating and/or enumerating account information */
var OnlineAccountInterface AccountInterface;

/** The interface for accessing online player methods */
var OnlinePlayerInterface PlayerInterface;

/** The interface for accessing online player extension methods */
var OnlinePlayerInterfaceEx PlayerInterfaceEx;

/** The interface for accessing system wide network functions */
var OnlineSystemInterface SystemInterface;

/** The interface to use for creating, searching for, or destroying online games */
var OnlineGameInterface GameInterface;

/** The interface to use for online content */
var OnlineContentInterface ContentInterface;

/** The interface to use for voice communication */
var OnlineVoiceInterface VoiceInterface;

/** The interface to use for stats read/write operations */
var OnlineStatsInterface StatsInterface;

/** The interface to use for reading game specific news announcements */
var OnlineNewsInterface NewsInterface;

// BM1
var const bool bWasLastLoginAutomatic;

/** Whether to use the override or not */
var config bool bUseBuildIdOverride;

var float TimeGuideLastOpened;
var float TimeGuideLastClosed;

/** Struct that holds a transient, unique identifier for a player */
struct native UniqueNetId
{
	/** The id used by the network to uniquely identify a player */
	var QWORD Uid;

	structcpptext
	{
		/**
		 * The noinit version of the ctor
		 */
		FORCEINLINE FUniqueNetId(void)
		{
		}

		/**
		 * Zeros the data in the struct
		 *
		 * @param ignored. only present to allow zeroing of data
		 */
		FORCEINLINE FUniqueNetId(EEventParm)
		{
			Uid = (QWORD)0;
		}

		/**
		 * Zeros the data in the struct
		 *
		 * @param ignored. only present to allow zeroing of data
		 */
		FORCEINLINE FUniqueNetId(DWORD)
		{
			Uid = (QWORD)0;
		}

		/** Copy constructor */
		FORCEINLINE FUniqueNetId(const FUniqueNetId& Other)
		{
			Uid = Other.Uid;
		}

		/**
		 * Compares the IDs for equality
		 *
		 * @param Id the value to compare
		 *
		 * @returns TRUE if they are equal, FALSE otherwise
		 */
		FORCEINLINE UBOOL operator==(const FUniqueNetId& Id) const
		{
			return Uid == Id.Uid;
		}

		/**
		 * Converts a DWORD into our type in network byte order
		 *
		 * @param Id the value to assign
		 */
		FORCEINLINE FUniqueNetId& operator=(const DWORD Id)
		{
#if WITH_PANORAMA || XBOX
			check(0 && "Bad conversion loses data");
#else
			Uid = Id;
#endif
			return *this;
		}

		/**
		 * Converts the unique id to a DWORD
		 *
		 * @return unique id as a DWORD
		 */
		FORCEINLINE DWORD ToDWORD(void) const
		{
#if WITH_PANORAMA || XBOX
			check(0 && "Bad conversion loses data");
			return 0;
#else
			return Uid & 0xFFFFFFFF;
#endif
		}

		/**
		 * Accessor for determining whether this net id has been assigned a value.
		 */
		FORCEINLINE UBOOL HasValue() const
		{
			return (QWORD)0 != Uid;
		}
	}
};

/** The different login statuses for a player */
enum ELoginStatus
{
	/** Player has not logged in or chosen a local profile */
	LS_NotLoggedIn,
	/** Player is using a local profile but is not logged in */
	LS_UsingLocalProfile,
	/** Player has been validated by the platform specific authentication service */
	LS_LoggedIn
};

/** This enum indicates access to major features in the game (parent controls */
enum EFeaturePrivilegeLevel
{
	/** Parental controls have disabled this feature */
	FPL_Disabled,
	/** Parental controls allow this feature only with people on their friends list */
	FPL_EnabledFriendsOnly,
	/** Parental controls allow this feature everywhere */
	FPL_Enabled
};

/** Used to bulk query the friends list */
struct native FriendsQuery
{
	/** The unique player id to check friends status for */
	var UniqueNetId UniqueId;
	/** Out param indicating whether the player is a friend or not */
	var bool bIsFriend;
};

/** Indicates where network notifications should appear on the screen */
enum ENetworkNotificationPosition
{
	NNP_TopLeft,
	NNP_TopCenter,
	NNP_TopRight,
	NNP_CenterLeft,
	NNP_Center,
	NNP_CenterRight,
	NNP_BottomLeft,
	NNP_BottomCenter,
	NNP_BottomRight
};

/** Enum indicating the current state of the online game (in progress, ended, etc.) */
enum EOnlineGameState
{
	/** An online game has not been created yet */
	OGS_NoSession,
	/** Session has been created and the match hasn't started (pre match lobby) */
	OGS_Pending,
	/** Session has been asked to start (may take time due to communication with backend) */
	OGS_Starting,
	/** The current session has started. Matches with join in progress disabled are no longer joinable */
	OGS_InProgress,
	/** The session is still valid, but the match is no longer being played (post match lobby) */
	OGS_Ending,
	/** The session is closed and any stats committed */
	OGS_Ended
};

/** The state of an async enumeration (friends, content, etc) read request */
enum EOnlineEnumerationReadState
{
	OERS_NotStarted,
	OERS_InProgress,
	OERS_Done,
	OERS_Failed
};

/** The possible friend states for a friend entry */
enum EOnlineFriendState
{
	/** Not currently online */
	OFS_Offline,
	/** Signed in and online */
	OFS_Online,
	/** Signed in, online, and idle */
	OFS_Away,
	/** Signed in, online, and asks to be left alone */
	OFS_Busy
};

/** Holds information about a player in a friends list */
struct native OnlineFriend
{
	/** Unique identifier of the friend */
	var const UniqueNetId UniqueId;
	/** The session id that the player is in */
	var const qword SessionId;
	/** Player's nick as published to the online service */
	var const string NickName;
	/** String holding information about the player's game state (cap-ed flag, etc.) */
	var const string PresenceInfo;
	/** The current state for this friend */
	var const EOnlineFriendState FriendState;
	/** Whether the friend is online or not */
	var const bool bIsOnline;
	/** Whether the friend is playing a game or not */
	var const bool bIsPlaying;
	/** Whether the friend is playing the same game or not */
	var const bool bIsPlayingThisGame;
	/** Whether the game the friend is in is joinable or not */
	var const bool bIsJoinable;
	/** Whether the friend can chat via voice or not */
	var const bool bHasVoiceSupport;
	/** Whether the friends has an invitation outstanding from you or not*/
	var bool bHaveInvited;
	/** Whether the friends has sent an invite to you or not */
	var const bool bHasInvitedYou;

	structcpptext
	{
		/**
		 * Zeroes members
		 */
		FOnlineFriend(void)
		{
			appMemzero(this,sizeof(FOnlineFriend));
		}

		/**
		 * Copy constructor. Copies the other into this object
		 *
		 * @param Other the other structure to copy
		 */
		FOnlineFriend(const FOnlineFriend& Other)
		{
			appMemzero(this,sizeof(FOnlineFriend));
			appMemcpy(&UniqueId,&Other.UniqueId,sizeof(FUniqueNetId));
			SessionId = Other.SessionId;
			NickName = Other.NickName;
			PresenceInfo = Other.PresenceInfo;
			FriendState = Other.FriendState;
			bIsOnline = Other.bIsOnline;
			bIsPlaying = Other.bIsPlaying;
			bIsPlayingThisGame = Other.bIsPlayingThisGame;
			bIsJoinable = Other.bIsJoinable;
			bHasVoiceSupport = Other.bHasVoiceSupport;
			bHaveInvited = Other.bHaveInvited;
			bHasInvitedYou = Other.bHasInvitedYou;
		}

		/**
		 * Assignment operator. Copies the other into this object
		 *
		 * @param Other the other structure to copy
		 */
		FOnlineFriend& operator=(const FOnlineFriend& Other)
		{
			if (this != &Other)
			{
				appMemcpy(&UniqueId,&Other.UniqueId,sizeof(FUniqueNetId));
				SessionId = Other.SessionId;
				NickName = Other.NickName;
				PresenceInfo = Other.PresenceInfo;
				FriendState = Other.FriendState;
				bIsOnline = Other.bIsOnline;
				bIsPlaying = Other.bIsPlaying;
				bIsPlayingThisGame = Other.bIsPlayingThisGame;
				bIsJoinable = Other.bIsJoinable;
				bHasVoiceSupport = Other.bHasVoiceSupport;
				bHaveInvited = Other.bHaveInvited;
				bHasInvitedYou = Other.bHasInvitedYou;
			}
			return *this;
		}
	}
};

/** Holds information about a single piece of downloaded content */
struct native OnlineContent
{
	/** Optional user index that content is downloaded for (-1 means it's not associated with any user) */
	var int UserIndex;
	/** Displayable name of the content */
	var string FriendlyName;
	/** File system usable reference to the content */
	var string ContentPath;

	// BM1
	var int DeviceID;
    var int DLCPackID;

	/** List of packages in the content */
	var array<string> ContentPackages;
	/** List of all non-package files in the content */
	var array<string> ContentFiles;
};

/**
 * Indicates the connection status with the remote online servers
 */
enum EOnlineServerConnectionStatus
{
	/** Gracefully disconnected from the online servers */
	OSCS_NotConnected,
	/** Connected to the online servers just fine */
	OSCS_Connected,
	/** Connection was lost for some reason */
	OSCS_ConnectionDropped,
	/** Can't connect because of missing network connection */
	OSCS_NoNetworkConnection,
	/** Service is temporarily unavailable */
	OSCS_ServiceUnavailable,
	/** An update is required before connecting is possible */
	OSCS_UpdateRequired,
	/** Servers are too busy to handle the request right now */
	OSCS_ServersTooBusy,
	/** Disconnected due to duplicate login */
	OSCS_DuplicateLoginDetected,
	/** Can't connect because of an invalid/unknown user */
	OSCS_InvalidUser
};

/**
 * The various NAT types the player may have
 */
enum ENATType
{
	/** Unable to determine the NAT type */
	NAT_Unknown,
	/** Anyone can join without connectivity problems */
	NAT_Open,
	/** Most can join but might have problems with strict */
	NAT_Moderate,
	/** Will most likely have connectivity problems with strict/moderate */
	NAT_Strict
};

/** A player in a session */
struct native OnlineRegistrant
{
	/** Unique id of the player involved in the arbitrated session */
	var const UniqueNetId PlayerNetId;

	structcpptext
	{
		/**
		 * The noinit version of the ctor
		 */
		FORCEINLINE FOnlineRegistrant(void)
		{
		}

		/**
		 * Inits it to the player id specified
		 *
		 * @param InId sets the player id
		 */
		FORCEINLINE FOnlineRegistrant(const FUniqueNetId InId) :
			PlayerNetId(InId)
		{
		}

		/**
		 * Compares the player IDs for equality
		 *
		 * @param Id the value to compare
		 *
		 * @returns TRUE if they are equal, FALSE otherwise
		 */
		FORCEINLINE UBOOL operator==(const FOnlineRegistrant& Other) const
		{
			return PlayerNetId == Other.PlayerNetId;
		}
	}
};

/** Struct holding the information about a single arbitration registrant */
struct native OnlineArbitrationRegistrant extends OnlineRegistrant
{
	/** Unique id of the machine involved in the arbitrated session */
	var const qword MachineId;
	/** Trust level of the machine/player for the arbitrated session */
	var const int Trustworthiness;
};

/**
 * Holds a word/phrase that was recognized by the speech analyzer
 *
 * @note See VoiceInterface.h to change the native layout of this struct
 */
struct SpeechRecognizedWord
{
	/** The id of the word in the vocabulary */
	var int WordId;
	/** the actual word */
	var string WordText;
	/** How confident the analyzer was in the recognition */
	var float Confidence;
};

/** Indicates the state the LAN beacon is in */
enum ELanBeaconState
{
	/** The lan beacon is disabled */
	LANB_NotUsingLanBeacon,
	/** The lan beacon is responding to client requests for information */
	LANB_Hosting,
	/** The lan beacon is querying servers for information */
	LANB_Searching
};

/**
 * Struct holding information used when writing scoring information that is used
 * to determine a player's skill rating
 */
struct native OnlinePlayerScore
{
	/** The player that this score is for */
	var UniqueNetId PlayerId;
	/** The team that the player is on */
	var int TeamId;
	/** The score for this player */
	var int Score;
};

/** The series of status codes that the account creation method can return */
enum EOnlineAccountCreateStatus
{
	/** Created the account successfully */
	OACS_CreateSuccessful,
	/** Failed but no indication why */
	OACS_UnknownError,
	/** The user name is invalid */
	OACS_InvalidUserName,
	/** The password is invalid */
	OACS_InvalidPassword,
	/** The unique user name is invalid */
	OACS_InvalidUniqueUserName,
	/** The user name is invalid */
	OACS_UniqueUserNameInUse,
	/** The backend service is not reachable */
	OACS_ServiceUnavailable
};

/** Information about a local talker */
struct native LocalTalker
{
	/** Whether this talker is currently registered */
	var bool bHasVoice;
	/** Whether the talker should send network data */
	var bool bHasNetworkedVoice;
	/** Whether the player is trying to speak voice commands */
	var bool bIsRecognizingSpeech;
	/** Whether the local talker was speaking last frame */
	var bool bWasTalking;
	/** Whether this player was already registered with the voice interface or not */
	var bool bIsRegistered;
};

/** Information about a remote talker */
struct native RemoteTalker
{
	/** The unique id for this talker */
	var UniqueNetId TalkerId;
	/** Whether the remote talker was speaking last frame */
	var bool bWasTalking;
	/** Whether this player was already registered with the voice interface or not */
	var bool bIsRegistered;
};

/** Holds the data used in a friend message */
struct native OnlineFriendMessage
{
	/** The player that is sending the message */
	var UniqueNetId SendingPlayerId;
	/** The nick name of the player that sent the message */
	var string SendingPlayerNick;
	/** Whether this is a friend invite or just a generic message */
	var bool bIsFriendInvite;
	/** Whether this message is a game invite or not */
	var bool bIsGameInvite;
	/** Whether the invite has been accepted or not */
	var bool bWasAccepted;
	/** Whether the invite has been denied or not */
	var bool bWasDenied;
	/** The corresponding message that was sent */
	var string Message;
};

/** Holds a named object interface for dynamically bound interfaces */
struct native NamedInterface
{
	/** The name to bind this object to */
	var name InterfaceName;
	/** The object to store at this location */
	var Object InterfaceObject;
};

/** Holds the set of registered named interfaces */
var private array<NamedInterface> NamedInterfaces;

/** Holds a name to class name mapping for adding the named interfaces automatically */
struct native NamedInterfaceDef
{
	/** The name to bind this object to */
	var name InterfaceName;
	/** The class to load and create for the named interface */
	var string InterfaceClassName;
};

/** The list of named interfaces to automatically create and store */
var config array<NamedInterfaceDef> NamedInterfaceDefs;

/** Holds the data used in downloading a file asynchronously from the online service */
struct native TitleFile
{
	/** The name of the file as requested */
	var string Filename;
	/** The async state the file download is in */
	var EOnlineEnumerationReadState AsyncState;
	/** The buffer of data for the file */
	var array<byte> Data;
};

/** Community file info */
struct native CommunityContentFile
{
	/** Unique identifier for this content file */
	var int ContentId;
	/** Additional identifier for providers that need it */
	var int FileId;
	/** Game specific content type field */
	var int ContentType;
	/** Size in bytes of the payload in the file */
	var int FileSize;
	/** The original owner of the file */
	var UniqueNetId Owner;
	/** The number of times this file has been downloaded */
	var int DownloadCount;
	/** The average rating for this file from the community */
	var float AverageRating;
	/** The number of times this file has been rated */
	var int RatingCount;
	/** The last rating the user gave this content */
	var int LastRatingGiven;
	/** The path to the file if stored locally */
	var string LocalFilePath;
};

/** Community file metadata */
struct native CommunityContentMetadata
{
	/** Unique identifier for this content type */
	var int ContentType;
	/** Metadata to associate with the file */
	var array<SettingsProperty> MetadataItems;
};

/** Holds the per session information for named sessions */
struct native NamedSession
{
	/** The name of the session */
	var name SessionName;
	/** The platform specific session information */
	var const native transient pointer SessionInfo{void};
	/** The game settings associated with this session */
	var OnlineGameSettings GameSettings;
	/** List of players registered in the session */
	var array<OnlineRegistrant> Registrants;
	/** The set of registered players for arbitration */
	var array<OnlineArbitrationRegistrant> ArbitrationRegistrants;
};

/** The list of sessions currently managed by the online subsystem */
var const protected array<NamedSession> Sessions;

/** Holds the information contained in Live's achievements for this title */
struct native AchievementDetails
{
	/** The ID of the achievement */
	var const int Id;
	/** The name of the achievement */
	var const string AchievementName;
	/** The description of the achievement */
    var const string Description;
	/** The description of how to meet the requirements of the achievement */
    var const string HowTo;
    /** The image associated with the achievement */
	var Surface Image;
	/** How much this achievement is worth */
	var const int GamerPoints;
	/** Whether the achievement is secret (hidden if not achieved) or not */
	var const bool bIsSecret;
	/** Whether the achievement awarded online or not */
	var const bool bWasAchievedOnline;
	/** Whether the achievement awarded offline or not */
	var const bool bWasAchievedOffline;
};

/** The state of an async enumeration (friends, content, etc) read request */
enum EOnlineNewsType
{
	ONT_Unknown,
	ONT_GameNews,
	ONT_ContentAnnouncements,
	ONT_Misc
};

/** Used to specify a build override value */
var config int BuildIdOverride;

/** The class to create our patcher instance from */
var config string IniLocPatcherClassName;

/** The object responsible for patching downloaded INI/loc files */
var transient IniLocPatcher Patcher;

cpptext
{
// FTickableObject interface

	/**
	 * Returns whether it is okay to tick this object. E.g. objects being loaded in the background shouldn't be ticked
	 * till they are finalized and unreachable objects cannot be ticked either.
	 *
	 * @return	TRUE if tickable, FALSE otherwise
	 */
	virtual UBOOL IsTickable() const
	{
		// We cannot tick objects that are unreachable or are in the process of being loaded in the background.
		return !HasAnyFlags( RF_Unreachable | RF_AsyncLoading );
	}

	/**
	 * Used to determine if an object should be ticked when the game is paused.
	 *
	 * @return always TRUE as networking needs to be ticked even when paused
	 */
	virtual UBOOL IsTickableWhenPaused() const
	{
		return TRUE;
	}

	/**
	 * Here to complete the interface but needs to be overriden
	 *
	 * @param ignored
	 */
	virtual void Tick(FLOAT)
	{
		check(0 && "Must be overridden");
	}

	/**
	 * Generates a string representation of a UniqueNetId struct.
	 *
	 * @param	IdToConvert		the unique net id that should be converted to a string.
	 *
	 * @return	the specified UniqueNetId represented as a string.
	 */
	static FString UniqueNetIdToString( const FUniqueNetId& IdToConvert );

	/**
	 * Converts a string representing a UniqueNetId into a UniqueNetId struct.
	 *
	 * @param	UniqueNetIdString	the string containing the text representation of the unique id.
	 * @param	out_UniqueId		will receive the UniqueNetId generated from the string.
	 *
	 * @return	TRUE if the string was successfully converted into a UniqueNetId; FALSE if the string was not a valid UniqueNetId.
	 */
	static UBOOL StringToUniqueNetId( const FString& UniqueNetIdString, FUniqueNetId& out_UniqueId );

	/**
	 * @return	TRUE if the netids are the same
	 */
	static UBOOL AreUniqueNetIdsEqual( const FUniqueNetId& NetIdA, const FUniqueNetId& NetIdB );

	/**
	 * Handle downloaded content in a platform-independent way
	 *
	 * @param Content The content descriptor that describes the downloaded content files
	 *
	 * @param return TRUE is successful
	 */
	virtual UBOOL ProcessDownloadedContent(const FOnlineContent& Content);

	/**
	 * Flush downloaded content for all users, making the engine stop using the content
	 *
	 * @param MaxNumUsers Platform specific max number of users to flush (this will iterate over all users from 0 to MaxNumUsers, as well as NO_USER
	 */
	virtual void FlushAllDownloadedContent(INT MaxNumUsers);

	/**
	 * Searches the named session array for the specified session
	 *
	 * @param SessionName the name to search for
	 *
	 * @return pointer to the struct if found, NULL otherwise
	 */
	inline FNamedSession* GetNamedSession(FName SessionName)
	{
		for (INT SearchIndex = 0; SearchIndex < Sessions.Num(); SearchIndex++)
		{
			if (Sessions(SearchIndex).SessionName == SessionName)
			{
				return &Sessions(SearchIndex);
			}
		}
		return NULL;
	}

	/**
	 * Searches the named session array for the specified session
	 *
	 * @param SessionName the name to search for
	 *
	 * @return TRUE if found, FALSE otherwise
	 */
	inline UBOOL HasNamedSession(FName SessionName)
	{
		return GetNamedSession(SessionName) != NULL;
	}

	/**
	 * Adds a new named session to the list
	 *
	 * @param SessionName the name to search for
	 * @param GameSettings the game settings to add
	 *
	 * @return a pointer to the struct that was added
	 */
	inline FNamedSession* AddNamedSession(FName SessionName,UOnlineGameSettings* GameSettings)
	{
		INT AddIndex = Sessions.AddZeroed();
		Sessions(AddIndex).SessionName = SessionName;
		Sessions(AddIndex).GameSettings = GameSettings;
		return &Sessions(AddIndex);
	}

	/**
	 * Searches the named session array for the specified session and removes it
	 *
	 * @param SessionName the name to search for
	 */
	virtual void RemoveNamedSession(FName SessionName)
	{
		for (INT SearchIndex = 0; SearchIndex < Sessions.Num(); SearchIndex++)
		{
			if (Sessions(SearchIndex).SessionName == SessionName)
			{
				Sessions.Remove(SearchIndex);
				return;
			}
		}
	}
}

/**
 * Called from engine start up code to allow the subsystem to initialize
 *
 * @return TRUE if the initialization was successful, FALSE otherwise
 */
native event bool Init();

/**
 * Used to trigger any automated tasks that need to happen just after startup
 */
event bool PostInit()
{
	local class<IniLocPatcher> IniLocPatcherClass;

	// Now that the subsystem is setup, try to download any config/loc updates
	if (IniLocPatcherClassName != "")
	{
		`Log("Loading "$IniLocPatcherClassName$" for INI/Loc patching");
		IniLocPatcherClass = class<IniLocPatcher>(DynamicLoadObject(IniLocPatcherClassName,class'Class'));
		Patcher = new IniLocPatcherClass;
		if (Patcher != None)
		{
			Patcher.Init();
			// Attempt to read any files that were requested to be patched
			Patcher.DownloadFiles();
		}
		else
		{
			return false;
		}
	}
	return true;
}

/**
 * Called from the engine shutdown code to allow the subsystem to release any
 * resources that may have been allocated
 */
event Exit();

/**
 * Called from native code to assign the account interface
 *
 * @param NewInterface the object to assign as providing the account interface
 *
 * @return TRUE if the interface is valid, FALSE otherwise
 */
event bool SetAccountInterface(Object NewInterface)
{
	AccountInterface = OnlineAccountInterface(NewInterface);
	// This will return false, if the interface wasn't supported
	return AccountInterface != None;
}

/**
 * Called from native code to assign the player interface
 *
 * @param NewInterface the object to assign as providing the player interface
 *
 * @return TRUE if the interface is valid, FALSE otherwise
 */
event bool SetPlayerInterface(Object NewInterface)
{
	PlayerInterface = OnlinePlayerInterface(NewInterface);
	// This will return false, if the interface wasn't supported
	return PlayerInterface != None;
}

/**
 * Called from native code to assign the extended player interface
 *
 * @param NewInterface the object to assign as providing the player interface
 *
 * @return TRUE if the interface is valid, FALSE otherwise
 */
event bool SetPlayerInterfaceEx(Object NewInterface)
{
	PlayerInterfaceEx = OnlinePlayerInterfaceEx(NewInterface);
	// This will return false, if the interface wasn't supported
	return PlayerInterfaceEx != None;
}

/**
 * Called from native code to assign the system interface
 *
 * @param NewInterface the object to assign as providing the system interface
 *
 * @return TRUE if the interface is valid, FALSE otherwise
 */
event bool SetSystemInterface(Object NewInterface)
{
	SystemInterface = OnlineSystemInterface(NewInterface);
	// This will return false, if the interface wasn't supported
	return SystemInterface != None;
}

/**
 * Called from native code to assign the game interface
 *
 * @param NewInterface the object to assign as providing the game interface
 *
 * @return TRUE if the interface is valid, FALSE otherwise
 */
event bool SetGameInterface(Object NewInterface)
{
	GameInterface = OnlineGameInterface(NewInterface);
	// This will return false, if the interface wasn't supported
	return GameInterface != None;
}

/**
 * Called from native code to assign the content interface
 *
 * @param NewInterface the object to assign as providing the content interface
 *
 * @return TRUE if the interface is valid, FALSE otherwise
 */
event bool SetContentInterface(Object NewInterface)
{
	ContentInterface = OnlineContentInterface(NewInterface);
	// This will return false, if the interface wasn't supported
	return ContentInterface != None;
}

/**
 * Called from native code to assign the voice interface
 *
 * @param NewInterface the object to assign as providing the voice interface
 *
 * @return TRUE if the interface is valid, FALSE otherwise
 */
event bool SetVoiceInterface(Object NewInterface)
{
	VoiceInterface = OnlineVoiceInterface(NewInterface);
	// This will return false, if the interface wasn't supported
	return VoiceInterface != None;
}

/**
 * Called from native code to assign the stats interface
 *
 * @param NewInterface the object to assign as providing the stats interface
 *
 * @return TRUE if the interface is valid, FALSE otherwise
 */
event bool SetStatsInterface(Object NewInterface)
{
	StatsInterface = OnlineStatsInterface(NewInterface);
	// This will return false, if the interface wasn't supported
	return StatsInterface != None;
}

/**
 * Called from native code to assign the news interface
 *
 * @param NewInterface the object to assign as providing the news interface
 *
 * @return TRUE if the interface is valid, FALSE otherwise
 */
event bool SetNewsInterface(Object NewInterface)
{
	NewsInterface = OnlineNewsInterface(NewInterface);
	// This will return false, if the interface wasn't supported
	return NewsInterface != None;
}

/**
 * Adds the interface object to the named interface list with the specified name
 *
 * @param InterfaceName the name of the interface to add it as
 * @param NewInterface the object/interface that is being cached
 */
event SetNamedInterface(name InterfaceName,Object NewInterface)
{
	local int InterfaceIndex;

	// Determine whether to add or replace
	InterfaceIndex = NamedInterfaces.Find('InterfaceName',InterfaceName);
	if (InterfaceIndex == INDEX_NONE)
	{
		// We are adding to the list
		InterfaceIndex = NamedInterfaces.Length;
		NamedInterfaces[InterfaceIndex].InterfaceName = InterfaceName;
	}
	// Replace whatever pointer was previously there
	NamedInterfaces[InterfaceIndex].InterfaceObject = NewInterface;
}

/**
 * Returns the interface object associated with the specified name
 *
 * @param InterfaceName the name of the interface to retrieve
 *
 * @return The object that matches the interface name if found
 */
event Object GetNamedInterface(name InterfaceName)
{
	local int InterfaceIndex;

	// Find the index in the array for this interface
	InterfaceIndex = NamedInterfaces.Find('InterfaceName',InterfaceName);
	if (InterfaceIndex != INDEX_NONE)
	{
		return NamedInterfaces[InterfaceIndex].InterfaceObject;
	}
	return None;
}

/**
 * Generates a string representation of a UniqueNetId struct.
 *
 * @param	IdToConvert		the unique net id that should be converted to a string.
 *
 * @return	the specified UniqueNetId represented as a string.
 */
static final native noexportheader function string UniqueNetIdToString(const out UniqueNetId IdToConvert);

/**
 * Converts a string representing a UniqueNetId into a UniqueNetId struct.
 *
 * @param	UniqueNetIdString	the string containing the text representation of the unique id.
 * @param	out_UniqueId		will receive the UniqueNetId generated from the string.
 *
 * @return	TRUE if the string was successfully converted into a UniqueNetId; FALSE if the string was not a valid UniqueNetId.
 */
static final native noexportheader function bool StringToUniqueNetId(string UniqueNetIdString,out UniqueNetId out_UniqueId);

/**
 * @return	TRUE if the netids are the same
 */
static final native noexportheader function bool AreUniqueNetIdsEqual( const out UniqueNetId NetIdA, const out UniqueNetId NetIdB );

/**
 * Generates a unique number based off of the current script compilation
 *
 * @return the unique number from the current script compilation
 */
final native function int GetBuildUniqueId();

/**
 * Logs the session state for all sessions present
 */
function DumpSessionState()
{
	local int Index;
	local int PlayerIndex;
	local UniqueNetId NetId;
	local UniqueNetId ZeroId;

	// To make the dumb warning go away even though it is used
	NetId = ZeroId;
	ZeroId = NetId;

	`Log("Unreal online session state");
	`Log("-------------------------------------------------------------");
	`Log("");
	`Log("Number of sessions: "$Sessions.Length);

	// Iterate through the sessions listing the session plus the registrants
	for (Index = 0; Index < Sessions.Length; Index++)
	{
		`Log("  Session: "$Sessions[Index].SessionName);
		`Log("    OnlineGameSettings: "$Sessions[Index].GameSettings);
		`Log("      OwningPlayerName: "$Sessions[Index].GameSettings.OwningPlayerName);
		NetId = Sessions[Index].GameSettings.OwningPlayerId;
		`Log("      OwningPlayerId: "$UniqueNetIdToString(NetId));
		`Log("      PingInMs: "$Sessions[Index].GameSettings.PingInMs);
		`Log("      NumPublicConnections: "$Sessions[Index].GameSettings.NumPublicConnections);
		`Log("      NumOpenPublicConnections: "$Sessions[Index].GameSettings.NumOpenPublicConnections);
		`Log("      NumPrivateConnections: "$Sessions[Index].GameSettings.NumPrivateConnections);
		`Log("      bIsLanMatch: "$Sessions[Index].GameSettings.bIsLanMatch);
		`Log("      bUsesStats: "$Sessions[Index].GameSettings.bUsesStats);
		`Log("      bUsesArbitration: "$Sessions[Index].GameSettings.bUsesArbitration);
		`Log("      bShouldAdvertise: "$Sessions[Index].GameSettings.bShouldAdvertise);
		`Log("      bAllowJoinInProgress: "$Sessions[Index].GameSettings.bAllowJoinInProgress);
		`Log("      bAllowInvites: "$Sessions[Index].GameSettings.bAllowInvites);
		`Log("      bUsesPresence: "$Sessions[Index].GameSettings.bUsesPresence);
		`Log("      bAllowJoinViaPresence: "$Sessions[Index].GameSettings.bAllowJoinViaPresence);
		`Log("      bAllowJoinViaPresenceFriendsOnly: "$Sessions[Index].GameSettings.bAllowJoinViaPresenceFriendsOnly);
		`Log("      GameState: "$Sessions[Index].GameSettings.GameState);
		`Log("");

		`Log("    Number of players: "$Sessions[Index].Registrants.Length);
		// List each player in the session
		for (PlayerIndex = 0; PlayerIndex < Sessions[Index].Registrants.Length; PlayerIndex++)
		{
			NetId = Sessions[Index].Registrants[PlayerIndex].PlayerNetId;
			`Log("      Player: "$UniqueNetIdToString(NetId));
		}
		`Log("    Number of arbitrated players: "$Sessions[Index].ArbitrationRegistrants.Length);
		// List each arbitrated player in the session
		for (PlayerIndex = 0; PlayerIndex < Sessions[Index].ArbitrationRegistrants.Length; PlayerIndex++)
		{
			NetId = Sessions[Index].ArbitrationRegistrants[PlayerIndex].PlayerNetId;
			`Log("      Player: "$UniqueNetIdToString(NetId));
		}
	}
}

/**
 * Logs the list of players that are registered for voice
 */
function DumpVoiceRegistration();

/**
 * Sets the debug output level for the platform specific API (if applicable)
 *
 * @param DebugSpewLevel the level to set
 */
function SetDebugSpewLevel(int DebugSpewLevel);

function bool OnlineDebugCommand(int Command, optional int Parameter);