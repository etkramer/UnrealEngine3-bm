/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class OnlineSubsystemLive extends OnlineSubsystemCommonImpl
	native
	implements(OnlinePlayerInterface,OnlinePlayerInterfaceEx,OnlineSystemInterface,OnlineGameInterface,OnlineContentInterface,OnlineVoiceInterface,OnlineStatsInterface)
	config(Engine);

/** The notification handle to use for polling events */
var const native transient pointer NotificationHandle{void};

/** The handle used with the XLocator service */
var const native transient pointer XLocatorServiceHandle{void};

/** The current game search object in use */
var const OnlineGameSearch GameSearch;

/** The current state the lan beacon is in */
var const ELanBeaconState LanBeaconState;

/** The port to tell Live to send/receive all broadcasts on */
var const config int LiveSystemLinkPort;

/** Port to listen on for LAN queries/responses */
var const config int LanAnnouncePort;

/** Unique id to keep UE3 games from seeing each others' lan packets */
var const config int LanGameUniqueId;

/** Used by a client to uniquely identify itself during lan match discovery */
var const byte LanNonce[8];

/** Mask containing which platforms can cross communicate */
var const config int LanPacketPlatformMask;

/** The amount of time before the lan query is considered done */
var float LanQueryTimeLeft;

/** The amount of time to wait before timing out a lan query request */
var config float LanQueryTimeout;

/** LAN announcement socket used to send/receive discovery packets */
var const native transient pointer LanBeacon{FLanBeacon};

/**
 * The number of simultaneous logins allowed (1, 2, or 4)
 */
var const config int NumLogins;

/**
 * Where Live notifications will be displayed on the screen
 */
var config ENetworkNotificationPosition CurrentNotificationPosition;

/**
 * This is the array of pending async tasks. Each tick these tasks are checked
 * completion. If complete, the delegate associated with them is called
 */
var native const array<pointer> AsyncTasks{FOnlineAsyncTaskLive};

/** Holds the cached state of the friends list for a single player */
struct native FriendsListCache
{
	/** The list of returned friends */
	var array<OnlineFriend> Friends;
	/** Indicates the state of the async read */
	var EOnlineEnumerationReadState ReadState;
	/** The array of delegates that notify read completion of the friends list data */
	var array<delegate<OnReadFriendsComplete> > ReadFriendsDelegates;
	/** The array of delegates that notify that the friends list has changed */
	var array<delegate<OnFriendsChange> > FriendsChangeDelegates;
};

/** Cache of friends data per player */
var FriendsListCache FriendsCache[4];

/** Holds an array of login delegates */
struct native LoginDelegates
{
	/** This is the list of requested delegates to fire */
	var array<delegate<OnLoginChange> > Delegates;
};

/** Used for per player index notification of sign in changes */
var LoginDelegates PlayerLoginDelegates[4];

/** Holds the list of delegates to fire when any login changes */
var LoginDelegates AllLoginDelegates;

/** Used to compare previous sign in masks to handle players signing out */
var const int LastSignInMask;

/** Holds the online & offline xuids for a signed in player */
struct native XuidPair
{
	/** The online xuid for the signed in player */
	var const UniqueNetId OnlineXuid;
	/** The offline xuid for the signed in player */
	var const UniqueNetId OfflineXuid;
};

/** The set of last known xuids for sign in change comparisons */
var const XuidPair LastXuids[4];

/** Holds the cached state of the content list for a single player */
struct native ContentListCache
{
	/** The list of returned content */
	var array<OnlineContent> Content;
	/** Indicates the state of the async read */
	var EOnlineEnumerationReadState ReadState;
	/** The delegate to call when the content has changed (user logged in, etc) */
	var array<delegate<OnContentChange> > ContentChangeDelegates;
	/** The delegate to call when the read is complete */
	var array<delegate<OnReadContentComplete> > ReadCompleteDelegates;
	/** The number of new downloadable content packages available */
	var int NewDownloadCount;
	/** The total number of downloadable content packages available */
	var int TotalDownloadCount;
	/** The delegate to call when the read is complete */
	var array<delegate<OnQueryAvailableDownloadsComplete> > QueryDownloadsDelegates;
};

/** Cache of content list per player */
var ContentListCache ContentCache[4];

/** The list of delegates to notify when any content changes */
var array<delegate<OnContentChange> > AnyContentChangeDelegates;

/** Holds the last keyboard input results */
var string KeyboardInputResults;

/** Whether the user canceled keyboard input or not */
var byte bWasKeyboardInputCanceled;

/** Per user cache of device id information */
struct native DeviceIdCache
{
	/** The last selected device id for this user */
	var int DeviceId;
	/** Delegate used to fire the array of events off */
	var delegate<OnDeviceSelectionComplete> DeviceSelectionMulticast;
	/** List of subscribers interested in device selection notification */
	var array<delegate<OnDeviceSelectionComplete> > DeviceSelectionDelegates;
};

/** Holds the last results of device selection */
var DeviceIdCache DeviceCache[4];

/** Holds the cached state of the profile for a single player */
struct native ProfileSettingsCache
{
	/** The profile for the player */
	var OnlineProfileSettings Profile;
	/** Used for per player index notification of profile reads completing */
	var array<delegate<OnReadProfileSettingsComplete> > ReadDelegates;
	/** Used for per player index notification of profile writes completing */
	var array<delegate<OnWriteProfileSettingsComplete> > WriteDelegates;
	/** Used to notify subscribers when the player changes their (non-game) profile */
	var array<delegate<OnProfileDataChanged> > ProfileDataChangedDelegates;
};

/** Holds the per player profile data */
var ProfileSettingsCache ProfileCache[4];

/**
 * Information about a remote talker's priority
 *
 * Zero means highest priority, < zero means muted
 */
struct native TalkerPriority
{
	/** Holds the current priority for this talker */
	var int CurrentPriority;
	/** Holds the last priority for this talker */
	var int LastPriority;
};

/** Information about a remote talker */
struct native LiveRemoteTalker extends RemoteTalker
{
	/** Holds the priorities for each of the local players */
	var TalkerPriority LocalPriorities[4];

	structcpptext
	{
		/** @return TRUE if any of the local players have this player muted, otherwise FALSE */
		inline UBOOL IsLocallyMuted(void)
		{
			for (INT Index = 0; Index < 4; Index++)
			{
				if (LocalPriorities[Index].CurrentPriority == XHV_PLAYBACK_PRIORITY_NEVER)
				{
					return TRUE;
				}
			}
			return FALSE;
		}
	}
};

/** Holds information about each of the local talkers */
var LocalTalker LocalTalkers[4];

/** Array of registered remote talkers */
var array<LiveRemoteTalker> RemoteTalkers;

/** Holds the list of delegates that are interested in receiving talking notifications */
var array<delegate<OnPlayerTalking> > TalkingDelegates;

/** Since the static array of dynamic array syntax appears to be broken */
struct native PerUserDelegateLists
{
	/** The array of delegates for notifying when speech recognition has completed for a player */
	var array<delegate<OnRecognitionComplete> > SpeechRecognitionDelegates;
	/** The array of delegates for notifying when an achievement write has completed */
	var array<delegate<OnUnlockAchievementComplete> > AchievementDelegates;
	/** The array of delegates for notifying when an achievements list read has completed */
	var array<delegate<OnReadAchievementsComplete> > AchievementReadDelegates;
};

/** Per user array of array of delegates */
var PerUserDelegateLists PerUserDelegates[4];

/** QoS packet with extra data to send to client */
var const byte QoSPacket[512];

/** The currently outstanding stats read request */
var const OnlineStatsRead CurrentStatsRead;

/** Used to track when a user cancels a sign-in request so that code waiting for a result can continue */
var const bool bIsInSignInUI;

/** This is the list of requested delegates to fire when a login is cancelled */
var array<delegate<OnLoginCancelled> > LoginCancelledDelegates;

/** This is the list of requested delegates to fire when a login fails to process */
var array<delegate<OnLoginFailed> > LoginFailedDelegates;

/** This is the list of requested delegates to fire when a logout completes */
var array<delegate<OnLogoutCompleted> > LogoutCompletedDelegates;

/** Array of delegates to multicast with for game creation notification */
var array<delegate<OnCreateOnlineGameComplete> > CreateOnlineGameCompleteDelegates;

/** Array of delegates to multicast with for game update notification */
var array<delegate<OnUpdateOnlineGameComplete> > UpdateOnlineGameCompleteDelegates;

/** Array of delegates to multicast with for game destruction notification */
var array<delegate<OnDestroyOnlineGameComplete> > DestroyOnlineGameCompleteDelegates;

/** Array of delegates to multicast with for game join notification */
var array<delegate<OnJoinOnlineGameComplete> > JoinOnlineGameCompleteDelegates;

/** Array of delegates to multicast with for game starting notification */
var array<delegate<OnStartOnlineGameComplete> > StartOnlineGameCompleteDelegates;

/** Array of delegates to multicast with for game ending notification */
var array<delegate<OnEndOnlineGameComplete> > EndOnlineGameCompleteDelegates;

/** Array of delegates to multicast with for game search notification */
var array<delegate<OnFindOnlineGamesComplete> > FindOnlineGamesCompleteDelegates;

/** Array of delegates to multicast with for game search cancellation notification */
var array<delegate<OnCancelFindOnlineGamesComplete> > CancelFindOnlineGamesCompleteDelegates;

/** Array of delegates to multicast with for player registration notification */
var array<delegate<OnRegisterPlayerComplete> > RegisterPlayerCompleteDelegates;

/** Array of delegates to multicast with for player unregistration notification */
var array<delegate<OnUnregisterPlayerComplete> > UnregisterPlayerCompleteDelegates;

/** Array of delegates to multicast with for arbitration registration notification */
var array<delegate<OnArbitrationRegistrationComplete> > ArbitrationRegistrationCompleteDelegates;

/** This is the list of delegates requesting notification when a stats read finishes */
var array<delegate<OnReadOnlineStatsComplete> > ReadOnlineStatsCompleteDelegates;

/** This is the list of delegates requesting notification when a Live UI opens/closes */
var array<delegate<OnExternalUIChange> > ExternalUIChangeDelegates;

/** This is the list of delegates requesting notification when a controller's state changes */
var array<delegate<OnControllerChange> > ControllerChangeDelegates;

/** Holds a true/false connection state for each of the possible 4 controllers */
var int LastInputDeviceConnectedMask;

/** This is the list of delegates requesting notification Live's connection state changes */
var array<delegate<OnConnectionStatusChange> > ConnectionStatusChangeDelegates;

/** This is the list of delegates requesting notification of storage device changes */
var array<delegate<OnStorageDeviceChange> > StorageDeviceChangeDelegates;

/** This is the list of delegates requesting notification of link status changes */
var array<delegate<OnConnectionStatusChange> > LinkStatusChangeDelegates;

/** The list of delegates to notify when the stats flush is complete */
var array<delegate<OnFlushOnlineStatsComplete> > FlushOnlineStatsDelegates;

/** The list of delegates to notify when the keyboard input is complete */
var array<delegate<OnKeyboardInputComplete> > KeyboardInputDelegates;

/** The list of delegates to notify when a network platform file is read */
var array<delegate<OnReadTitleFileComplete> > ReadTitleFileCompleteDelegates;

/** Holds the delegate and the last accepted invite for a player */
struct native InviteData
{
	/** The per user delegates for game invites */
	var array<delegate<OnGameInviteAccepted> > InviteDelegates;
	/** Cached invite data for the player */
	var const native transient pointer InviteData{XINVITE_INFO};
	/** Game search results associated with this invite */
	var const OnlineGameSearch InviteSearch;
};

/** The cached data for the players */
var InviteData InviteCache[4];

/** Whether to log arbitration data or not */
var config bool bShouldLogArbitrationData;

/** Whether to log stats (including true skill) data or not */
var config bool bShouldLogStatsData;

/** Holds the list of delegates that are interested in receiving mute change notifications */
var array<delegate<OnMutingChange> > MutingChangeDelegates;

/** Holds the list of delegates that are interested in receiving join friend completions */
var array<delegate<OnJoinFriendGameComplete> > JoinFriendGameCompleteDelegates;

/** The list of title managed storage files that have been read or are being read */
var array<TitleFile> TitleManagedFiles;

/** Associates the specific achievements per user and title id */
struct native CachedAchievements
{
	/** The player these are for */
	var int PlayerNum;
	/** The title id that these are for */
	var int TitleId;
	/** The list of achievements for this player and title */
	var array<AchievementDetails> Achievements;
	/** Indicates the state of the async read */
	var EOnlineEnumerationReadState ReadState;
};

/** Holds the list of achievements that have been read for players */
var array<CachedAchievements> AchievementList;

/** Whether to use MCP or not */
var config bool bShouldUseMcp;

/** The current elapsed time while waiting to signal a sign in change */
var transient float SigninCountDownCounter;

/** The value to use as the delay between sign in notification from Live and processing it */
var float SigninCountDownDelay;

/** Whether the code is ticking the count down timer or not */
var transient bool bIsCountingDownSigninNotification;

/** Sets the debug logging level for getting extra spew from Live */
var config int DebugLogLevel;

/** A bit mask indicating which players have a pending invite that was delayed at start up */
var byte DelayedInviteUserMask;

/**
 * Delegate used in login notifications
 */
delegate OnLoginChange();

/**
 * Delegate used to notify when a login request was cancelled by the user
 */
delegate OnLoginCancelled();

/**
 * Delegate used in mute list change notifications
 */
delegate OnMutingChange();

/**
 * Delegate used in friends list change notifications
 */
delegate OnFriendsChange();

/**
 * Called from engine start up code to allow the subsystem to initialize
 *
 * @return TRUE if the initialization was successful, FALSE otherwise
 */
native event bool Init();

/**
 * Called from the engine shutdown code to allow the Live to cleanup. Also, this
 * version blocks until all async tasks are complete before returning.
 */
native event Exit();

/**
 * Displays the UI that prompts the user for their login credentials. Each
 * platform handles the authentication of the user's data.
 *
 * @param bShowOnlineOnly whether to only display online enabled profiles or not
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowLoginUI(optional bool bShowOnlineOnly = false);

/**
 * Logs the player into the online service. If this fails, it generates a
 * OnLoginFailed notification
 *
 * @param LocalUserNum the controller number of the associated user
 * @param LoginName the unique identifier for the player
 * @param Password the password for this account
 * @param bWantsLocalOnly whether the player wants to sign in locally only or not
 *
 * @return true if the async call started ok, false otherwise
 */
native function bool Login(byte LocalUserNum,string LoginName,string Password,optional bool bWantsLocalOnly);

/**
 * Logs the player into the online service using parameters passed on the
 * command line. Expects -Login=<UserName> -Password=<password>. If either
 * are missing, the function returns false and doesn't start the login
 * process
 *
 * @return true if the async call started ok, false otherwise
 */
native function bool AutoLogin();

/**
 * Delegate used in notifying the UI/game that the manual login failed
 *
 * @param LocalUserNum the controller number of the associated user
 * @param ErrorCode the async error code that occurred
 */
delegate OnLoginFailed(byte LocalUserNum,EOnlineServerConnectionStatus ErrorCode);

/**
 * Sets the delegate used to notify the gameplay code that a login failed
 *
 * @param LocalUserNum the controller number of the associated user
 * @param LoginDelegate the delegate to use for notifications
 */
function AddLoginFailedDelegate(byte LocalUserNum,delegate<OnLoginFailed> LoginFailedDelegate)
{
	// Add this delegate to the array if not already present
	if (LoginFailedDelegates.Find(LoginFailedDelegate) == INDEX_NONE)
	{
		LoginFailedDelegates[LoginFailedDelegates.Length] = LoginFailedDelegate;
	}
}

/**
 * Removes the specified delegate from the notification list
 *
 * @param LocalUserNum the controller number of the associated user
 * @param LoginDelegate the delegate to use for notifications
 */
function ClearLoginFailedDelegate(byte LocalUserNum,delegate<OnLoginFailed> LoginFailedDelegate)
{
	local int RemoveIndex;

	// Remove this delegate from the array if found
	RemoveIndex = LoginFailedDelegates.Find(LoginFailedDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		LoginFailedDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Signs the player out of the online service
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
native function bool Logout(byte LocalUserNum);

/**
 * Delegate used in notifying the UI/game that the manual logout completed
 *
 * @param bWasSuccessful whether the async call completed properly or not
 */
delegate OnLogoutCompleted(bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that a logout completed
 *
 * @param LocalUserNum the controller number of the associated user
 * @param LogoutDelegate the delegate to use for notifications
 */
function AddLogoutCompletedDelegate(byte LocalUserNum,delegate<OnLogoutCompleted> LogoutDelegate)
{
	// Add this delegate to the array if not already present
	if (LogoutCompletedDelegates.Find(LogoutDelegate) == INDEX_NONE)
	{
		LogoutCompletedDelegates[LogoutCompletedDelegates.Length] = LogoutDelegate;
	}
}

/**
 * Removes the specified delegate from the notification list
 *
 * @param LocalUserNum the controller number of the associated user
 * @param LogoutDelegate the delegate to use for notifications
 */
function ClearLogoutCompletedDelegate(byte LocalUserNum,delegate<OnLogoutCompleted> LogoutDelegate)
{
	local int RemoveIndex;

	// Remove this delegate from the array if found
	RemoveIndex = LogoutCompletedDelegates.Find(LogoutDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		LogoutCompletedDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Fetches the login status for a given player
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the enum value of their status
 */
native function ELoginStatus GetLoginStatus(byte LocalUserNum);

/**
 * Determines whether the specified user is a guest login or not
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return true if a guest, false otherwise
 */
native function bool IsGuestLogin(byte LocalUserNum);

/**
 * Determines whether the specified user is a local (non-online) login or not
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return true if a local profile, false otherwise
 */
native function bool IsLocalLogin(byte LocalUserNum);

/**
 * Gets the platform specific unique id for the specified player
 *
 * @param LocalUserNum the controller number of the associated user
 * @param PlayerId the byte array that will receive the id
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
native function bool GetUniquePlayerId(byte LocalUserNum,out UniqueNetId PlayerId);

/**
 * Reads the player's nick name from the online service
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return a string containing the players nick name
 */
native function string GetPlayerNickname(byte LocalUserNum);

/**
 * Determines whether the player is allowed to play online
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
native function EFeaturePrivilegeLevel CanPlayOnline(byte LocalUserNum);

/**
 * Determines whether the player is allowed to use voice or text chat online
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
native function EFeaturePrivilegeLevel CanCommunicate(byte LocalUserNum);

/**
 * Determines whether the player is allowed to download user created content
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
native function EFeaturePrivilegeLevel CanDownloadUserContent(byte LocalUserNum);

/**
 * Determines whether the player is allowed to buy content online
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
native function EFeaturePrivilegeLevel CanPurchaseContent(byte LocalUserNum);

/**
 * Determines whether the player is allowed to view other people's player profile
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
native function EFeaturePrivilegeLevel CanViewPlayerProfiles(byte LocalUserNum);

/**
 * Determines whether the player is allowed to have their online presence
 * information shown to remote clients
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return the Privilege level that is enabled
 */
native function EFeaturePrivilegeLevel CanShowPresenceInformation(byte LocalUserNum);

/**
 * Checks that a unique player id is part of the specified user's friends list
 *
 * @param LocalUserNum the controller number of the associated user
 * @param PlayerId the id of the player being checked
 *
 * @return TRUE if a member of their friends list, FALSE otherwise
 */
native function bool IsFriend(byte LocalUserNum,UniqueNetId PlayerId);

/**
 * Checks that whether a group of player ids are among the specified player's
 * friends
 *
 * @param LocalUserNum the controller number of the associated user
 * @param Query an array of players to check for being included on the friends list
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
native function bool AreAnyFriends(byte LocalUserNum,out array<FriendsQuery> Query);

/**
 * Checks that a unique player id is on the specified user's mute list
 *
 * @param LocalUserNum the controller number of the associated user
 * @param PlayerId the id of the player being checked
 *
 * @return TRUE if the player should be muted, FALSE otherwise
 */
native function bool IsMuted(byte LocalUserNum,UniqueNetId PlayerId);

/**
 * Displays the UI that shows a user's list of friends
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowFriendsUI(byte LocalUserNum);

/**
 * Displays the UI that shows a user's list of friends
 *
 * @param LocalUserNum the controller number of the associated user
 * @param PlayerId the id of the player being invited
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowFriendsInviteUI(byte LocalUserNum,UniqueNetId PlayerId);

/**
 * Sets the delegate used to notify the gameplay code that a login changed
 *
 * @param LoginDelegate the delegate to use for notifications
 * @param LocalUserNum whether to watch for changes on a specific slot or all slots
 */
function AddLoginChangeDelegate(delegate<OnLoginChange> LoginDelegate,optional byte LocalUserNum = 255)
{
	// If this is for any login change
	if (LocalUserNum == 255)
	{
		// Add this delegate to the array if not already present
		if (AllLoginDelegates.Delegates.Find(LoginDelegate) == INDEX_NONE)
		{
			AllLoginDelegates.Delegates[AllLoginDelegates.Delegates.Length] = LoginDelegate;
		}
	}
	// Make sure it's within range
	else if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Add this delegate to the array if not already present
		if (PlayerLoginDelegates[LocalUserNum].Delegates.Find(LoginDelegate) == INDEX_NONE)
		{
			PlayerLoginDelegates[LocalUserNum].Delegates[PlayerLoginDelegates[LocalUserNum].Delegates.Length] = LoginDelegate;
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to AddLoginChangeDelegate()");
	}
}

/**
 * Removes the specified delegate from the notification list
 *
 * @param LoginDelegate the delegate to use for notifications
 * @param LocalUserNum whether to watch for changes on a specific slot or all slots
 */
function ClearLoginChangeDelegate(delegate<OnLoginChange> LoginDelegate,optional byte LocalUserNum = 255)
{
	local int RemoveIndex;

	// If this is for any login change
	if (LocalUserNum == 255)
	{
		// Remove this delegate from the array if found
		RemoveIndex = AllLoginDelegates.Delegates.Find(LoginDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			AllLoginDelegates.Delegates.Remove(RemoveIndex,1);
		}
	}
	// Make sure it's within range and remove the per player login index
	else if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Remove this delegate from the array if found
		RemoveIndex = PlayerLoginDelegates[LocalUserNum].Delegates.Find(LoginDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			PlayerLoginDelegates[LocalUserNum].Delegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearLoginChangeDelegate()");
	}
}

/**
 * Adds a delegate to the list of delegates that are fired when a login is cancelled
 *
 * @param CancelledDelegate the delegate to add to the list
 */
function AddLoginCancelledDelegate(delegate<OnLoginCancelled> CancelledDelegate)
{
	// Add this delegate to the array if not already present
	if (LoginCancelledDelegates.Find(CancelledDelegate) == INDEX_NONE)
	{
		LoginCancelledDelegates[LoginCancelledDelegates.Length] = CancelledDelegate;
	}
}

/**
 * Removes the specified delegate from the notification list
 *
 * @param CancelledDelegate the delegate to remove fromt he list
 */
function ClearLoginCancelledDelegate(delegate<OnLoginCancelled> CancelledDelegate)
{
	local int RemoveIndex;

	// Remove this delegate from the array if found
	RemoveIndex = LoginCancelledDelegates.Find(CancelledDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		LoginCancelledDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Sets the delegate used to notify the gameplay code that a muting list changed
 *
 * @param MutingDelegate the delegate to use for notifications
 */
function AddMutingChangeDelegate(delegate<OnMutingChange> MutingDelegate)
{
	if (MutingChangeDelegates.Find(MutingDelegate) == INDEX_NONE)
	{
		MutingChangeDelegates[MutingChangeDelegates.Length] = MutingDelegate;
	}
}

/**
 * Removes the delegate from the list of notifications
 *
 * @param MutingDelegate the delegate to use for notifications
 */
function ClearMutingChangeDelegate(delegate<OnMutingChange> MutingDelegate)
{
	local int RemoveIndex;

	RemoveIndex = MutingChangeDelegates.Find(MutingDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		MutingChangeDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Sets the delegate used to notify the gameplay code that a friends list changed
 *
 * @param FriendsDelegate the delegate to use for notifications
 */
function AddFriendsChangeDelegate(byte LocalUserNum,delegate<OnFriendsChange> FriendsDelegate)
{
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		if (FriendsCache[LocalUserNum].FriendsChangeDelegates.Find(FriendsDelegate) == INDEX_NONE)
		{
			FriendsCache[LocalUserNum].FriendsChangeDelegates[FriendsCache[LocalUserNum].FriendsChangeDelegates.Length] = FriendsDelegate;
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to AddFriendsChangeDelegate()");
	}
}

/**
 * Removes the delegate from the list of notifications
 *
 * @param FriendsDelegate the delegate to use for notifications
 */
function ClearFriendsChangeDelegate(byte LocalUserNum,delegate<OnFriendsChange> FriendsDelegate)
{
	local int RemoveIndex;

	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = FriendsCache[LocalUserNum].FriendsChangeDelegates.Find(FriendsDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			FriendsCache[LocalUserNum].FriendsChangeDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearReadFriendsCompleteDelegate()");
	}
}

/**
 * Displays the UI that allows a player to give feedback on another player
 *
 * @param LocalUserNum the controller number of the associated user
 * @param PlayerId the id of the player having feedback given for
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowFeedbackUI(byte LocalUserNum,UniqueNetId PlayerId);

/**
 * Displays the gamer card UI for the specified player
 *
 * @param LocalUserNum the controller number of the associated user
 * @param PlayerId the id of the player to show the gamer card of
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowGamerCardUI(byte LocalUserNum,UniqueNetId PlayerId);

/**
 * Displays the messages UI for a player
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowMessagesUI(byte LocalUserNum);

/**
 * Displays the achievements UI for a player
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowAchievementsUI(byte LocalUserNum);

/**
 * Displays the Guide UI
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowGuideUI();

/**
 * Displays the UI that shows the player list
 *
 * @param LocalUserNum the controller number of the associated user
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowPlayersUI(byte LocalUserNum);

/**
 * Displays the UI that shows the keyboard for inputing text
 *
 * @param LocalUserNum the controller number of the associated user
 * @param TitleText the title to display to the user
 * @param DescriptionText the text telling the user what to input
 * @param bIsPassword whether the entry is a password or not
 * @param bShouldValidate whether to apply the string validation API after input or not
 * @param DefaultText the default string to display
 * @param MaxResultLength the maximum length string expected to be filled in
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowKeyboardUI(byte LocalUserNum,string TitleText,
	string DescriptionText,optional bool bIsPassword = false,
	optional bool bShouldValidate = true,
	optional string DefaultText,
	optional int MaxResultLength = 256);

/**
 * Delegate used when the keyboard input request has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnKeyboardInputComplete(bool bWasSuccessful);

/**
 * Adds the delegate used to notify the gameplay code that the user has completed
 * their keyboard input
 *
 * @param InputDelegate the delegate to use for notifications
 */
function AddKeyboardInputDoneDelegate(delegate<OnKeyboardInputComplete> InputDelegate)
{
	if (KeyboardInputDelegates.Find(InputDelegate) == INDEX_NONE)
	{
		KeyboardInputDelegates[KeyboardInputDelegates.Length] = InputDelegate;
	}
}

/**
 * Clears the delegate used to notify the gameplay code that the user has completed
 * their keyboard input
 *
 * @param InputDelegate the delegate to use for notifications
 */
function ClearKeyboardInputDoneDelegate(delegate<OnKeyboardInputComplete> InputDelegate)
{
	local int RemoveIndex;
	RemoveIndex = KeyboardInputDelegates.Find(InputDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		KeyboardInputDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Fetches the results of the input
 *
 * @param bWasCanceled whether the user cancelled the input or not
 *
 * @return the string entered by the user. Note the string will be empty if it
 * fails validation
 */
function string GetKeyboardInputResults(out byte bWasCanceled)
{
	bWasCanceled = bWasKeyboardInputCanceled;
	return KeyboardInputResults;
}

/**
 * Determines if the ethernet link is connected or not
 */
native function bool HasLinkConnection();

/**
 * Delegate fired when the network link status changes
 *
 * @param bIsConnected whether the link is currently connected or not
 */
delegate OnLinkStatusChange(bool bIsConnected);

/**
 * Sets the delegate used to notify the gameplay code that link status changed
 *
 * @param LinkStatusDelegate the delegate to use for notifications
 */
function AddLinkStatusChangeDelegate(delegate<OnLinkStatusChange> LinkStatusDelegate)
{
	local int AddIndex;
	// Only add to the list once
	if (LinkStatusChangeDelegates.Find(LinkStatusDelegate) == INDEX_NONE)
	{
		AddIndex = LinkStatusChangeDelegates.Length;
		LinkStatusChangeDelegates.Length = LinkStatusChangeDelegates.Length + 1;
		LinkStatusChangeDelegates[AddIndex] = LinkStatusDelegate;
	}
}

/**
 * Removes the delegate from the notify list
 *
 * @param LinkStatusDelegate the delegate to remove
 */
function ClearLinkStatusChangeDelegate(delegate<OnLinkStatusChange> LinkStatusDelegate)
{
	local int RemoveIndex;
	// See if the specified delegate is in the list
	RemoveIndex = LinkStatusChangeDelegates.Find(LinkStatusDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		LinkStatusChangeDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Delegate fired when an external UI display state changes (opening/closing)
 *
 * @param bIsOpening whether the external UI is opening or closing
 */
delegate OnExternalUIChange(bool bIsOpening);

/**
 * Sets the delegate used to notify the gameplay code that external UI state
 * changed (opened/closed)
 *
 * @param ExternalUIDelegate the delegate to use for notifications
 */
function AddExternalUIChangeDelegate(delegate<OnExternalUIChange> ExternalUIDelegate)
{
	local int AddIndex;
	// Add this delegate to the array if not already present
	if (ExternalUIChangeDelegates.Find(ExternalUIDelegate) == INDEX_NONE)
	{
		AddIndex = ExternalUIChangeDelegates.Length;
		ExternalUIChangeDelegates.Length = ExternalUIChangeDelegates.Length + 1;
		ExternalUIChangeDelegates[AddIndex] = ExternalUIDelegate;
	}
}

/**
 * Removes the delegate from the notification list
 *
 * @param ExternalUIDelegate the delegate to remove
 */
function ClearExternalUIChangeDelegate(delegate<OnExternalUIChange> ExternalUIDelegate)
{
	local int RemoveIndex;
	RemoveIndex = ExternalUIChangeDelegates.Find(ExternalUIDelegate);
	// Verify that it is in the array
	if (RemoveIndex != INDEX_NONE)
	{
		ExternalUIChangeDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Determines the current notification position setting
 */
function ENetworkNotificationPosition GetNetworkNotificationPosition()
{
	return CurrentNotificationPosition;
}

/**
 * Sets a new position for the network notification icons/images
 *
 * @param NewPos the new location to use
 */
native function SetNetworkNotificationPosition(ENetworkNotificationPosition NewPos);

/**
 * Delegate fired when the controller becomes dis/connected
 *
 * @param ControllerId the id of the controller that changed connection state
 * @param bIsConnected whether the controller connected (true) or disconnected (false)
 */
delegate OnControllerChange(int ControllerId,bool bIsConnected);

/**
 * Sets the delegate used to notify the gameplay code that the controller state changed
 *
 * @param ControllerChangeDelegate the delegate to use for notifications
 */
function AddControllerChangeDelegate(delegate<OnControllerChange> ControllerChangeDelegate)
{
	local int AddIndex;
	// Add this delegate to the array if not already present
	if (ControllerChangeDelegates.Find(ControllerChangeDelegate) == INDEX_NONE)
	{
		AddIndex = ControllerChangeDelegates.Length;
		ControllerChangeDelegates.Length = ControllerChangeDelegates.Length + 1;
		ControllerChangeDelegates[AddIndex] = ControllerChangeDelegate;
	}
}

/**
 * Removes the delegate used to notify the gameplay code that the controller state changed
 *
 * @param ControllerChangeDelegate the delegate to remove
 */
function ClearControllerChangeDelegate(delegate<OnControllerChange> ControllerChangeDelegate)
{
	local int RemoveIndex;
	RemoveIndex = ControllerChangeDelegates.Find(ControllerChangeDelegate);
	// Verify that it is in the array
	if (RemoveIndex != INDEX_NONE)
	{
		ControllerChangeDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Determines if the specified controller is connected or not
 *
 * @param ControllerId the controller to query
 *
 * @return true if connected, false otherwise
 */
native function bool IsControllerConnected(int ControllerId);

/**
 * Delegate fire when the online server connection state changes
 *
 * @param ConnectionStatus the new connection status
 */
delegate OnConnectionStatusChange(EOnlineServerConnectionStatus ConnectionStatus);

/**
 * Adds the delegate to the list to be notified when the connection status changes
 *
 * @param ConnectionStatusDelegate the delegate to add
 */
function AddConnectionStatusChangeDelegate(delegate<OnConnectionStatusChange> ConnectionStatusDelegate)
{
	local int AddIndex;
	// Only add to the list once
	if (ConnectionStatusChangeDelegates.Find(ConnectionStatusDelegate) == INDEX_NONE)
	{
		AddIndex = ConnectionStatusChangeDelegates.Length;
		ConnectionStatusChangeDelegates.Length = ConnectionStatusChangeDelegates.Length + 1;
		ConnectionStatusChangeDelegates[AddIndex] = ConnectionStatusDelegate;
	}
}

/**
 * Removes the delegate from the notify list
 *
 * @param ConnectionStatusDelegate the delegate to remove
 */
function ClearConnectionStatusChangeDelegate(delegate<OnConnectionStatusChange> ConnectionStatusDelegate)
{
	local int RemoveIndex;
	// See if the specified delegate is in the list
	RemoveIndex = ConnectionStatusChangeDelegates.Find(ConnectionStatusDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		ConnectionStatusChangeDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Determines the NAT type the player is using
 */
native function ENATType GetNATType();


/**
 * Delegate fired when a storage device change is detected
 */
delegate OnStorageDeviceChange();

/**
 * Adds the delegate to the list to be notified when a storage device changes
 *
 * @param StorageDeviceChangeDelegate the delegate to add
 */
function AddStorageDeviceChangeDelegate(delegate<OnStorageDeviceChange> StorageDeviceChangeDelegate)
{
	// Only add to the list once
	if (StorageDeviceChangeDelegates.Find(StorageDeviceChangeDelegate) == INDEX_NONE)
	{
		StorageDeviceChangeDelegates[StorageDeviceChangeDelegates.Length] = StorageDeviceChangeDelegate;
	}
}

/**
 * Removes the delegate from the notify list
 *
 * @param StorageDeviceChangeDelegate the delegate to remove
 */
function ClearStorageDeviceChangeDelegate(delegate<OnStorageDeviceChange> StorageDeviceChangeDelegate)
{
	local int RemoveIndex;
	// See if the specified delegate is in the list
	RemoveIndex = StorageDeviceChangeDelegates.Find(StorageDeviceChangeDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		StorageDeviceChangeDelegates.Remove(RemoveIndex,1);
	}
}

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
	FileIndex = TitleManagedFiles.Find('FileName',FileName);
	if (FileIndex != INDEX_NONE)
	{
		return TitleManagedFiles[FileIndex].AsyncState;
	}
	return OERS_Failed;
}

/**
 * Delegate fired when the search for an online game has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnFindOnlineGamesComplete(bool bWasSuccessful);

/** Returns the currently set game search object */
function OnlineGameSearch GetGameSearch()
{
	return GameSearch;
}

/**
 * Creates an online game based upon the settings object specified.
 * NOTE: online game registration is an async process and does not complete
 * until the OnCreateOnlineGameComplete delegate is called.
 *
 * @param HostingPlayerNum the index of the player hosting the match
 * @param SessionName the name to use for this session so that multiple sessions can exist at the same time
 * @param NewGameSettings the settings to use for the new game session
 *
 * @return true if successful creating the session, false otherwise
 */
native function bool CreateOnlineGame(byte HostingPlayerNum,name SessionName,OnlineGameSettings NewGameSettings);

/**
 * Delegate fired when a create request has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnCreateOnlineGameComplete(name SessionName,bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the online game they
 * created has completed the creation process
 *
 * @param CreateOnlineGameCompleteDelegate the delegate to use for notifications
 */
function AddCreateOnlineGameCompleteDelegate(delegate<OnCreateOnlineGameComplete> CreateOnlineGameCompleteDelegate)
{
	if (CreateOnlineGameCompleteDelegates.Find(CreateOnlineGameCompleteDelegate) == INDEX_NONE)
	{
		CreateOnlineGameCompleteDelegates[CreateOnlineGameCompleteDelegates.Length] = CreateOnlineGameCompleteDelegate;
	}
}

/**
 * Sets the delegate used to notify the gameplay code that the online game they
 * created has completed the creation process
 *
 * @param CreateOnlineGameCompleteDelegate the delegate to use for notifications
 */
function ClearCreateOnlineGameCompleteDelegate(delegate<OnCreateOnlineGameComplete> CreateOnlineGameCompleteDelegate)
{
	local int RemoveIndex;

	RemoveIndex = CreateOnlineGameCompleteDelegates.Find(CreateOnlineGameCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		CreateOnlineGameCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Updates the localized settings/properties for the game in question. Updates
 * the QoS packet if needed (starting & restarting QoS).
 *
 * @param SessionName the name of the session to update
 * @param UpdatedGameSettings the object to update the game settings with
 * @param bShouldRefreshOnlineData whether to submit the data to the backend or not
 *
 * @return true if successful creating the session, false otherwsie
 */
native function bool UpdateOnlineGame(name SessionName,OnlineGameSettings UpdatedGameSettings,optional bool bShouldRefreshOnlineData = false);

/**
 * Delegate fired when a update request has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnUpdateOnlineGameComplete(name SessionName,bool bWasSuccessful);

/**
 * Adds a delegate to the list of objects that want to be notified
 *
 * @param UpdateOnlineGameCompleteDelegate the delegate to use for notifications
 */
function AddUpdateOnlineGameCompleteDelegate(delegate<OnUpdateOnlineGameComplete> UpdateOnlineGameCompleteDelegate)
{
	if (UpdateOnlineGameCompleteDelegates.Find(UpdateOnlineGameCompleteDelegate) == INDEX_NONE)
	{
		UpdateOnlineGameCompleteDelegates[UpdateOnlineGameCompleteDelegates.Length] = UpdateOnlineGameCompleteDelegate;
	}
}

/**
 * Removes a delegate from the list of objects that want to be notified
 *
 * @param UpdateOnlineGameCompleteDelegate the delegate to use for notifications
 */
function ClearUpdateOnlineGameCompleteDelegate(delegate<OnUpdateOnlineGameComplete> UpdateOnlineGameCompleteDelegate)
{
	local int RemoveIndex;

	RemoveIndex = UpdateOnlineGameCompleteDelegates.Find(UpdateOnlineGameCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		UpdateOnlineGameCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Returns the game settings object for the session with a matching name
 *
 * @param SessionName the name of the session to return
 *
 * @return the game settings for this session name
 */
function OnlineGameSettings GetGameSettings(name SessionName)
{
	local int SessionIndex;

	SessionIndex = Sessions.Find('SessionName',SessionName);
	if (SessionIndex != INDEX_NONE)
	{
		return Sessions[SessionIndex].GameSettings;
	}
	return None;
}

/**
 * Destroys the specified online game
 *
 * @param SessionName the name of the session to delete
 *
 * @return true if successful destroying the session, false otherwsie
 */
native function bool DestroyOnlineGame(name SessionName);

/**
 * Delegate fired when a destroying an online game has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnDestroyOnlineGameComplete(name SessionName,bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the online game they
 * destroyed has completed the destruction process
 *
 * @param DestroyOnlineGameCompleteDelegate the delegate to use for notifications
 */
function AddDestroyOnlineGameCompleteDelegate(delegate<OnDestroyOnlineGameComplete> DestroyOnlineGameCompleteDelegate)
{
	if (DestroyOnlineGameCompleteDelegates.Find(DestroyOnlineGameCompleteDelegate) == INDEX_NONE)
	{
		DestroyOnlineGameCompleteDelegates[DestroyOnlineGameCompleteDelegates.Length] = DestroyOnlineGameCompleteDelegate;
	}
}

/**
 * Removes the delegate from the notification list
 *
 * @param DestroyOnlineGameCompleteDelegate the delegate to use for notifications
 */
function ClearDestroyOnlineGameCompleteDelegate(delegate<OnDestroyOnlineGameComplete> DestroyOnlineGameCompleteDelegate)
{
	local int RemoveIndex;

	RemoveIndex = DestroyOnlineGameCompleteDelegates.Find(DestroyOnlineGameCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		DestroyOnlineGameCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Searches for games matching the settings specified
 *
 * @param SearchingPlayerNum the index of the player searching for a match
 * @param SearchSettings the desired settings that the returned sessions will have
 *
 * @return true if successful destroying the session, false otherwsie
 */
native function bool FindOnlineGames(byte SearchingPlayerNum,OnlineGameSearch SearchSettings);

/**
 * Adds the delegate used to notify the gameplay code that the search they
 * kicked off has completed
 *
 * @param FindOnlineGamesCompleteDelegate the delegate to use for notifications
 */
function AddFindOnlineGamesCompleteDelegate(delegate<OnFindOnlineGamesComplete> FindOnlineGamesCompleteDelegate)
{
	// Only add to the list once
	if (FindOnlineGamesCompleteDelegates.Find(FindOnlineGamesCompleteDelegate) == INDEX_NONE)
	{
		FindOnlineGamesCompleteDelegates[FindOnlineGamesCompleteDelegates.Length] = FindOnlineGamesCompleteDelegate;
	}
}

/**
 * Removes the delegate from the notify list
 *
 * @param FindOnlineGamesCompleteDelegate the delegate to use for notifications
 */
function ClearFindOnlineGamesCompleteDelegate(delegate<OnFindOnlineGamesComplete> FindOnlineGamesCompleteDelegate)
{
	local int RemoveIndex;
	// Find it in the list
	RemoveIndex = FindOnlineGamesCompleteDelegates.Find(FindOnlineGamesCompleteDelegate);
	// Only remove if found
	if (RemoveIndex != INDEX_NONE)
	{
		FindOnlineGamesCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Cancels the current search in progress if possible for that search type
 *
 * @return true if successful searching for sessions, false otherwise
 */
native function bool CancelFindOnlineGames();

/**
 * Delegate fired when the cancellation of a search for an online game has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnCancelFindOnlineGamesComplete(bool bWasSuccessful);

/**
 * Adds the delegate to the list to notify with
 *
 * @param CancelFindOnlineGamesCompleteDelegate the delegate to use for notifications
 */
function AddCancelFindOnlineGamesCompleteDelegate(delegate<OnCancelFindOnlineGamesComplete> CancelFindOnlineGamesCompleteDelegate)
{
	// Only add to the list once
	if (CancelFindOnlineGamesCompleteDelegates.Find(CancelFindOnlineGamesCompleteDelegate) == INDEX_NONE)
	{
		CancelFindOnlineGamesCompleteDelegates[CancelFindOnlineGamesCompleteDelegates.Length] = CancelFindOnlineGamesCompleteDelegate;
	}
}

/**
 * Removes the delegate from the notify list
 *
 * @param CancelFindOnlineGamesCompleteDelegate the delegate to use for notifications
 */
function ClearCancelFindOnlineGamesCompleteDelegate(delegate<OnCancelFindOnlineGamesComplete> CancelFindOnlineGamesCompleteDelegate)
{
	local int RemoveIndex;
	// Find it in the list
	RemoveIndex = CancelFindOnlineGamesCompleteDelegates.Find(CancelFindOnlineGamesCompleteDelegate);
	// Only remove if found
	if (RemoveIndex != INDEX_NONE)
	{
		CancelFindOnlineGamesCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Serializes the platform specific data into the provided buffer for the specified search result
 *
 * @param DesiredGame the game to copy the platform specific data for
 * @param PlatformSpecificInfo the buffer to fill with the platform specific information
 *
 * @return true if successful serializing the data, false otherwise
 */
native function bool ReadPlatformSpecificSessionInfo(const out OnlineGameSearchResult DesiredGame,out byte PlatformSpecificInfo[68]);

/**
 * Serializes the platform specific data into the provided buffer for the specified settings object.
 * NOTE: This can only be done for a session that is bound to the online system
 *
 * @param GameSettings the game to copy the platform specific data for
 * @param PlatformSpecificInfo the buffer to fill with the platform specific information
 *
 * @return true if successful reading the data for the session, false otherwise
 */
native function bool ReadPlatformSpecificSessionInfoBySessionName(name SessionName,out byte PlatformSpecificInfo[68]);

/**
 * Creates a search result out of the platform specific data and adds that to the specified search object
 *
 * @param SearchingPlayerNum the index of the player searching for a match
 * @param SearchSettings the desired search to bind the session to
 * @param PlatformSpecificInfo the platform specific information to convert to a server object
 *
 * @return true if successful searching for sessions, false otherwise
 */
native function bool BindPlatformSpecificSessionToSearch(byte SearchingPlayerNum,OnlineGameSearch SearchSettings,byte PlatformSpecificInfo[68]);

/**
 * Cleans up any platform specific allocated data contained in the search results
 *
 * @param Search the object to free search results for
 *
 * @return true if successful, false otherwise
 */
native function bool FreeSearchResults(optional OnlineGameSearch Search);

/**
 * Fetches the additional data a session exposes outside of the online service.
 * NOTE: notifications will come from the OnFindOnlineGamesComplete delegate
 *
 * @param StartAt the search result index to start gathering the extra information for
 * @param NumberToQuery the number of additional search results to get the data for
 *
 * @return true if the query was started, false otherwise
 */
function bool QueryNonAdvertisedData(int StartAt,int NumberToQuery)
{
	`Log("Ignored on Live");
	return false;
}

/**
 * Joins the game specified
 *
 * @param PlayerNum the index of the player searching for a match
 * @param SessionName the name of the session to join
 * @param DesiredGame the desired game to join
 *
 * @return true if the call completed successfully, false otherwise
 */
native function bool JoinOnlineGame(byte PlayerNum,name SessionName,const out OnlineGameSearchResult DesiredGame);

/**
 * Delegate fired when the joing process for an online game has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnJoinOnlineGameComplete(name SessionName,bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the join request they
 * kicked off has completed
 *
 * @param JoinOnlineGameCompleteDelegate the delegate to use for notifications
 */
function AddJoinOnlineGameCompleteDelegate(delegate<OnJoinOnlineGameComplete> JoinOnlineGameCompleteDelegate)
{
	if (JoinOnlineGameCompleteDelegates.Find(JoinOnlineGameCompleteDelegate) == INDEX_NONE)
	{
		JoinOnlineGameCompleteDelegates[JoinOnlineGameCompleteDelegates.Length] = JoinOnlineGameCompleteDelegate;
	}
}

/**
 * Removes the delegate from the notify list
 *
 * @param JoinOnlineGameCompleteDelegate the delegate to use for notifications
 */
function ClearJoinOnlineGameCompleteDelegate(delegate<OnJoinOnlineGameComplete> JoinOnlineGameCompleteDelegate)
{
	local int RemoveIndex;
	// Find it in the list
	RemoveIndex = JoinOnlineGameCompleteDelegates.Find(JoinOnlineGameCompleteDelegate);
	// Only remove if found
	if (RemoveIndex != INDEX_NONE)
	{
		JoinOnlineGameCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Returns the platform specific connection information for joining the match.
 * Call this function from the delegate of join completion
 *
 * @param SessionName the name of the session to fetch the connection information for
 * @param ConnectInfo the out var containing the platform specific connection information
 *
 * @return true if the call was successful, false otherwise
 */
native function bool GetResolvedConnectString(name SessionName,out string ConnectInfo);

/**
 * Registers a player with the online service as being part of the online game
 *
 * @param SessionName the name of the session the player is joining
 * @param UniquePlayerId the player to register with the online service
 * @param bWasInvited whether the player was invited to the game or searched for it
 *
 * @return true if the call succeeds, false otherwise
 */
native function bool RegisterPlayer(name SessionName,UniqueNetId PlayerId,bool bWasInvited);

/**
 * Delegate fired when the registration process has completed
 *
 * @param SessionName the name of the session the player joined or not
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnRegisterPlayerComplete(name SessionName,bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the player
 * registration request they submitted has completed
 *
 * @param RegisterPlayerCompleteDelegate the delegate to use for notifications
 */
function AddRegisterPlayerCompleteDelegate(delegate<OnRegisterPlayerComplete> RegisterPlayerCompleteDelegate)
{
	if (RegisterPlayerCompleteDelegates.Find(RegisterPlayerCompleteDelegate) == INDEX_NONE)
	{
		RegisterPlayerCompleteDelegates[RegisterPlayerCompleteDelegates.Length] = RegisterPlayerCompleteDelegate;
	}
}

/**
 * Removes the specified delegate from the notification list
 *
 * @param RegisterPlayerCompleteDelegate the delegate to use for notifications
 */
function ClearRegisterPlayerCompleteDelegate(delegate<OnRegisterPlayerComplete> RegisterPlayerCompleteDelegate)
{
	local int RemoveIndex;

	RemoveIndex = RegisterPlayerCompleteDelegates.Find(RegisterPlayerCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		RegisterPlayerCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Unregisters a player with the online service as being part of the online game
 *
 * @param SessionName the name of the session the player is leaving
 * @param PlayerId the player to unregister with the online service
 *
 * @return true if the call succeeds, false otherwise
 */
native function bool UnregisterPlayer(name SessionName,UniqueNetId PlayerId);

/**
 * Delegate fired when the unregistration process has completed
 *
 * @param SessionName the name of the session the player left
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnUnregisterPlayerComplete(name SessionName,bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the player
 * Unregistration request they submitted has completed
 *
 * @param UnregisterPlayerCompleteDelegate the delegate to use for notifications
 */
function AddUnregisterPlayerCompleteDelegate(delegate<OnUnregisterPlayerComplete> UnregisterPlayerCompleteDelegate)
{
	if (UnregisterPlayerCompleteDelegates.Find(UnregisterPlayerCompleteDelegate) == INDEX_NONE)
	{
		UnregisterPlayerCompleteDelegates[UnregisterPlayerCompleteDelegates.Length] = UnregisterPlayerCompleteDelegate;
	}
}

/**
 * Removes the specified delegate from the notification list
 *
 * @param UnregisterPlayerCompleteDelegate the delegate to use for notifications
 */
function ClearUnregisterPlayerCompleteDelegate(delegate<OnUnregisterPlayerComplete> UnregisterPlayerCompleteDelegate)
{
	local int RemoveIndex;

	RemoveIndex = UnregisterPlayerCompleteDelegates.Find(UnregisterPlayerCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		UnregisterPlayerCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Updates the current session's skill rating using the list of players' skills
 *
 * @param SessionName the name of the session to update the skill rating for
 * @param Players the set of players to use in the skill calculation
 *
 * @return true if the update succeeded, false otherwise
 */
native function bool RecalculateSkillRating(name SessionName,const out array<UniqueNetId> Players);

/**
 * Reads the online profile settings for a given user
 *
 * @param LocalUserNum the user that we are reading the data for
 * @param ProfileSettings the object to copy the results to and contains the list of items to read
 *
 * @return true if the call succeeds, false otherwise
 */
native function bool ReadProfileSettings(byte LocalUserNum,OnlineProfileSettings ProfileSettings);

/**
 * Delegate used when the last read profile settings request has completed
 *
 * @param LocalUserNum the controller index of the player who's read just completed
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnReadProfileSettingsComplete(byte LocalUserNum,bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the last read request has completed
 *
 * @param LocalUserNum which user to watch for read complete notifications
 * @param ReadProfileSettingsCompleteDelegate the delegate to use for notifications
 */
function AddReadProfileSettingsCompleteDelegate(byte LocalUserNum,delegate<OnReadProfileSettingsComplete> ReadProfileSettingsCompleteDelegate)
{
	// Make sure the user is valid
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Add this delegate to the array if not already present
		if (ProfileCache[LocalUserNum].ReadDelegates.Find(ReadProfileSettingsCompleteDelegate) == INDEX_NONE)
		{
			ProfileCache[LocalUserNum].ReadDelegates[ProfileCache[LocalUserNum].ReadDelegates.Length] = ReadProfileSettingsCompleteDelegate;
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to SetReadProfileSettingsCompleteDelegate()");
	}
}

/**
 * Searches the existing set of delegates for the one specified and removes it
 * from the list
 *
 * @param LocalUserNum which user to watch for read complete notifications
 * @param ReadProfileSettingsCompleteDelegate the delegate to find and clear
 */
function ClearReadProfileSettingsCompleteDelegate(byte LocalUserNum,delegate<OnReadProfileSettingsComplete> ReadProfileSettingsCompleteDelegate)
{
	local int RemoveIndex;

	// Make sure the user is valid
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = ProfileCache[LocalUserNum].ReadDelegates.Find(ReadProfileSettingsCompleteDelegate);
		// Remove this delegate from the array if found
		if (RemoveIndex != INDEX_NONE)
		{
			ProfileCache[LocalUserNum].ReadDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearReadProfileSettingsCompleteDelegate()");
	}
}

/**
 * Returns the online profile settings for a given user
 *
 * @param LocalUserNum the user that we are reading the data for
 *
 * @return the profile settings object
 */
function OnlineProfileSettings GetProfileSettings(byte LocalUserNum)
{
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		return ProfileCache[LocalUserNum].Profile;
	}
	return None;
}

/**
 * Writes the online profile settings for a given user to the online data store
 *
 * @param LocalUserNum the user that we are writing the data for
 * @param ProfileSettings the list of settings to write out
 *
 * @return true if the call succeeds, false otherwise
 */
native function bool WriteProfileSettings(byte LocalUserNum,OnlineProfileSettings ProfileSettings);

/**
 * Delegate used when the last write profile settings request has completed
 *
 * @param LocalUserNum the controller index of the player who's write just completed
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnWriteProfileSettingsComplete(byte LocalUserNum,bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the last read request has completed
 *
 * @param LocalUserNum which user to watch for read complete notifications
 * @param ReadProfileSettingsCompleteDelegate the delegate to use for notifications
 */
function AddWriteProfileSettingsCompleteDelegate(byte LocalUserNum,delegate<OnWriteProfileSettingsComplete> WriteProfileSettingsCompleteDelegate)
{
	// Make sure the user is valid
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Add this delegate to the array if not already present
		if (ProfileCache[LocalUserNum].WriteDelegates.Find(WriteProfileSettingsCompleteDelegate) == INDEX_NONE)
		{
			ProfileCache[LocalUserNum].WriteDelegates[ProfileCache[LocalUserNum].WriteDelegates.Length] = WriteProfileSettingsCompleteDelegate;
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to AddWriteProfileSettingsCompleteDelegate()");
	}
}

/**
 * Searches the existing set of delegates for the one specified and removes it
 * from the list
 *
 * @param LocalUserNum which user to watch for read complete notifications
 * @param ReadProfileSettingsCompleteDelegate the delegate to find and clear
 */
function ClearWriteProfileSettingsCompleteDelegate(byte LocalUserNum,delegate<OnWriteProfileSettingsComplete> WriteProfileSettingsCompleteDelegate)
{
	local int RemoveIndex;

	// Make sure the user is valid
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = ProfileCache[LocalUserNum].WriteDelegates.Find(WriteProfileSettingsCompleteDelegate);
		// Remove this delegate from the array if found
		if (RemoveIndex != INDEX_NONE)
		{
			ProfileCache[LocalUserNum].WriteDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearWriteProfileSettingsCompleteDelegate()");
	}
}

/**
 * Sets a rich presence information to use for the specified player
 *
 * @param LocalUserNum the controller number of the associated user
 * @param PresenceMode the rich presence mode to use
 * @param LocalizedStringSettings the list of localized string settings to set
 * @param Properties the list of properties to set
 */
native function SetOnlineStatus(byte LocalUserNum,int PresenceMode,
	const out array<LocalizedStringSetting> LocalizedStringSettings,
	const out array<SettingsProperty> Properties);

/**
 * Displays the invite ui
 *
 * @param LocalUserNum the local user sending the invite
 * @param InviteText the string to prefill the UI with
 */
native function bool ShowInviteUI(byte LocalUserNum,optional string InviteText);

/**
 * Displays the marketplace UI for content
 *
 * @param LocalUserNum the local user viewing available content
 * @param CategoryMask the bitmask to use to filter content by type
 * @param OfferId a specific offer that you want shown
 */
native function bool ShowContentMarketplaceUI(byte LocalUserNum,optional int CategoryMask = -1,optional int OfferId);

/**
 * Displays the marketplace UI for memberships
 *
 * @param LocalUserNum the local user viewing available memberships
 */
native function bool ShowMembershipMarketplaceUI(byte LocalUserNum);

/**
 * Displays the UI that allows the user to choose which device to save content to
 *
 * @param LocalUserNum the controller number of the associated user
 * @param SizeNeeded the size of the data to be saved in bytes
 * @param bForceShowUI true to always show the UI, false to only show the
 *		  UI if there are multiple valid choices
 * @param bManageStorage whether to allow the user to manage their storage or not
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowDeviceSelectionUI(byte LocalUserNum,int SizeNeeded,optional bool bForceShowUI,optional bool bManageStorage);

/**
 * Delegate used when the device selection request has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnDeviceSelectionComplete(bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the user has completed
 * their device selection
 *
 * @param LocalUserNum the controller number of the associated user
 * @param DeviceDelegate the delegate to use for notifications
 */
function AddDeviceSelectionDoneDelegate(byte LocalUserNum,delegate<OnDeviceSelectionComplete> DeviceDelegate)
{
	local int AddIndex;

	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Add this delegate to the array if not already present
		if (DeviceCache[LocalUserNum].DeviceSelectionDelegates.Find(DeviceDelegate) == INDEX_NONE)
		{
			AddIndex = DeviceCache[LocalUserNum].DeviceSelectionDelegates.Length;
			DeviceCache[LocalUserNum].DeviceSelectionDelegates.Length = DeviceCache[LocalUserNum].DeviceSelectionDelegates.Length + 1;
			DeviceCache[LocalUserNum].DeviceSelectionDelegates[AddIndex] = DeviceDelegate;
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to SetDeviceSelectionDoneDelegate()");
	}
}

/**
 * Removes the specified delegate from the list of callbacks
 *
 * @param LocalUserNum the controller number of the associated user
 * @param DeviceDelegate the delegate to use for notifications
 */
function ClearDeviceSelectionDoneDelegate(byte LocalUserNum,delegate<OnDeviceSelectionComplete> DeviceDelegate)
{
	local int RemoveIndex;

	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		// Find the delegate and remove it
		RemoveIndex = DeviceCache[LocalUserNum].DeviceSelectionDelegates.Find(DeviceDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			DeviceCache[LocalUserNum].DeviceSelectionDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearDeviceSelectionDoneDelegate()");
	}
}

/**
 * Fetches the results of the device selection
 *
 * @param LocalPlayerNum the player to check the results for
 * @param DeviceName out param that gets a copy of the string
 *
 * @return the ID of the device that was selected
 */
native function int GetDeviceSelectionResults(byte LocalPlayerNum,out string DeviceName);

/**
 * Checks the device id to determine if it is still valid (could be removed) and/or
 * if there is enough space on the specified device
 *
 * @param DeviceId the device to check
 * @param SizeNeeded the amount of space requested
 *
 * @return true if valid, false otherwise
 */
native function bool IsDeviceValid(int DeviceId,optional int SizeNeeded);

/**
 * Unlocks the specified achievement for the specified user
 *
 * @param LocalUserNum the controller number of the associated user
 * @param AchievementId the id of the achievement to unlock
 *
 * @return TRUE if the call worked, FALSE otherwise
 */
native function bool UnlockAchievement(byte LocalUserNum,int AchievementId);

/**
 * Delegate used when the achievement unlocking has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnUnlockAchievementComplete(bool bWasSuccessful);

/**
 * Adds the delegate used to notify the gameplay code that the achievement unlocking has completed
 *
 * @param LocalUserNum which user to watch for read complete notifications
 * @param UnlockAchievementCompleteDelegate the delegate to use for notifications
 */
function AddUnlockAchievementCompleteDelegate(byte LocalUserNum,delegate<OnUnlockAchievementComplete> UnlockAchievementCompleteDelegate)
{
	// Make sure the user is valid
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		if (PerUserDelegates[LocalUserNum].AchievementDelegates.Find(UnlockAchievementCompleteDelegate) == INDEX_NONE)
		{
			PerUserDelegates[LocalUserNum].AchievementDelegates[PerUserDelegates[LocalUserNum].AchievementDelegates.Length] = UnlockAchievementCompleteDelegate;
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to AddUnlockAchievementCompleteDelegate()");
	}
}

/**
 * Clears the delegate used to notify the gameplay code that the achievement unlocking has completed
 *
 * @param LocalUserNum which user to watch for read complete notifications
 * @param UnlockAchievementCompleteDelegate the delegate to use for notifications
 */
function ClearUnlockAchievementCompleteDelegate(byte LocalUserNum,delegate<OnUnlockAchievementComplete> UnlockAchievementCompleteDelegate)
{
	local int RemoveIndex;

	// Make sure the user is valid
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = PerUserDelegates[LocalUserNum].AchievementDelegates.Find(UnlockAchievementCompleteDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			PerUserDelegates[LocalUserNum].AchievementDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearUnlockAchievementCompleteDelegate()");
	}
}

/**
 * Unlocks a gamer picture for the local user
 *
 * @param LocalUserNum the user to unlock the picture for
 * @param PictureId the id of the picture to unlock
 */
native function bool UnlockGamerPicture(byte LocalUserNum,int PictureId);

/**
 * Called when an external change to player profile data has occured
 */
delegate OnProfileDataChanged();

/**
 * Sets the delegate used to notify the gameplay code that someone has changed their profile data externally
 *
 * @param LocalUserNum the user the delegate is interested in
 * @param ProfileDataChangedDelegate the delegate to use for notifications
 */
function AddProfileDataChangedDelegate(byte LocalUserNum,delegate<OnProfileDataChanged> ProfileDataChangedDelegate)
{
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		if (ProfileCache[LocalUserNum].ProfileDataChangedDelegates.Find(ProfileDataChangedDelegate) == INDEX_NONE)
		{
			ProfileCache[LocalUserNum].ProfileDataChangedDelegates[ProfileCache[LocalUserNum].ProfileDataChangedDelegates.Length] = ProfileDataChangedDelegate;
		}
	}
	else
	{
		`Log("Invalid user id ("$LocalUserNum$") specified for AddProfileDataChangedDelegate()");
	}
}

/**
 * Clears the delegate used to notify the gameplay code that someone has changed their profile data externally
 *
 * @param LocalUserNum the user the delegate is interested in
 * @param ProfileDataChangedDelegate the delegate to use for notifications
 */
function ClearProfileDataChangedDelegate(byte LocalUserNum,delegate<OnProfileDataChanged> ProfileDataChangedDelegate)
{
	local int RemoveIndex;

	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = ProfileCache[LocalUserNum].ProfileDataChangedDelegates.Find(ProfileDataChangedDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			ProfileCache[LocalUserNum].ProfileDataChangedDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`Log("Invalid user id ("$LocalUserNum$") specified for ClearProfileDataChangedDelegate()");
	}
}

/**
 * Marks an online game as in progress (as opposed to being in lobby or pending)
 *
 * @param SessionName the name of the session that is being started
 *
 * @return true if the call succeeds, false otherwise
 */
native function bool StartOnlineGame(name SessionName);

/**
 * Delegate fired when the online game has transitioned to the started state
 *
 * @param SessionName the name of the session the that has transitioned to started
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnStartOnlineGameComplete(name SessionName,bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the online game has
 * transitioned to the started state.
 *
 * @param StartOnlineGameCompleteDelegate the delegate to use for notifications
 */
function AddStartOnlineGameCompleteDelegate(delegate<OnStartOnlineGameComplete> StartOnlineGameCompleteDelegate)
{
	if (StartOnlineGameCompleteDelegates.Find(StartOnlineGameCompleteDelegate) == INDEX_NONE)
	{
		StartOnlineGameCompleteDelegates[StartOnlineGameCompleteDelegates.Length] = StartOnlineGameCompleteDelegate;
	}
}

/**
 * Removes the delegate from the notify list
 *
 * @param StartOnlineGameCompleteDelegate the delegate to use for notifications
 */
function ClearStartOnlineGameCompleteDelegate(delegate<OnStartOnlineGameComplete> StartOnlineGameCompleteDelegate)
{
	local int RemoveIndex;

	RemoveIndex = StartOnlineGameCompleteDelegates.Find(StartOnlineGameCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		StartOnlineGameCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Marks an online game as having been ended
 *
 * @param SessionName the name of the session the to end
 *
 * @return true if the call succeeds, false otherwise
 */
native function bool EndOnlineGame(name SessionName);

/**
 * Delegate fired when the online game has transitioned to the ending game state
 *
 * @param SessionName the name of the session the that was ended
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnEndOnlineGameComplete(name SessionName,bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the online game has
 * transitioned to the ending state.
 *
 * @param EndOnlineGameCompleteDelegate the delegate to use for notifications
 */
function AddEndOnlineGameCompleteDelegate(delegate<OnEndOnlineGameComplete> EndOnlineGameCompleteDelegate)
{
	if (EndOnlineGameCompleteDelegates.Find(EndOnlineGameCompleteDelegate) == INDEX_NONE)
	{
		EndOnlineGameCompleteDelegates[EndOnlineGameCompleteDelegates.Length] = EndOnlineGameCompleteDelegate;
	}
}

/**
 * Removes the delegate from the notify list
 *
 * @param EndOnlineGameCompleteDelegate the delegate to use for notifications
 */
function ClearEndOnlineGameCompleteDelegate(delegate<OnEndOnlineGameComplete> EndOnlineGameCompleteDelegate)
{
	local int RemoveIndex;

	RemoveIndex = EndOnlineGameCompleteDelegates.Find(EndOnlineGameCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		EndOnlineGameCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Tells the game to register with the underlying arbitration server if available
 *
 * @param SessionName the name of the session to register for arbitration with
 */
native function bool RegisterForArbitration(name SessionName);

/**
 * Delegate fired when the online game has completed registration for arbitration
 *
 * @param SessionName the name of the session the that had arbitration pending
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnArbitrationRegistrationComplete(name SessionName,bool bWasSuccessful);

/**
 * Sets the notification callback to use when arbitration registration has completed
 *
 * @param ArbitrationRegistrationCompleteDelegate the delegate to use for notifications
 */
function AddArbitrationRegistrationCompleteDelegate(delegate<OnArbitrationRegistrationComplete> ArbitrationRegistrationCompleteDelegate)
{
	if (ArbitrationRegistrationCompleteDelegates.Find(ArbitrationRegistrationCompleteDelegate) == INDEX_NONE)
	{
		ArbitrationRegistrationCompleteDelegates[ArbitrationRegistrationCompleteDelegates.Length] = ArbitrationRegistrationCompleteDelegate;
	}
}

/**
 * Removes the notification callback to use when arbitration registration has completed
 *
 * @param ArbitrationRegistrationCompleteDelegate the delegate to use for notifications
 */
function ClearArbitrationRegistrationCompleteDelegate(delegate<OnArbitrationRegistrationComplete> ArbitrationRegistrationCompleteDelegate)
{
	local int RemoveIndex;

	RemoveIndex = ArbitrationRegistrationCompleteDelegates.Find(ArbitrationRegistrationCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		ArbitrationRegistrationCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Returns the list of arbitrated players for the arbitrated session
 *
 * @param SessionName the name of the session to get the arbitration results for
 *
 * @return the list of players that are registered for this session
 */
function array<OnlineArbitrationRegistrant> GetArbitratedPlayers(name SessionName);

/**
 * Tells the online subsystem to accept the game invite that is currently pending
 *
 * @param LocalUserNum the local user accepting the invite
 * @param SessionName the name of the session this invite is to be known as
 *
 * @return true if the game invite was able to be accepted, false otherwise
 */
native function bool AcceptGameInvite(byte LocalUserNum,name SessionName);

/**
 * Starts an async task that retrieves the list of friends for the player from the
 * online service. The list can be retrieved in whole or in part.
 *
 * @param LocalUserNum the user to read the friends list of
 * @param Count the number of friends to read or zero for all
 * @param StartingAt the index of the friends list to start at (for pulling partial lists)
 *
 * @return true if the read request was issued successfully, false otherwise
 */
native function bool ReadFriendsList(byte LocalUserNum,optional int Count,optional int StartingAt);

/**
 * Delegate used when the friends read request has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnReadFriendsComplete(bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the friends read request has completed
 *
 * @param LocalUserNum the user to read the friends list of
 * @param ReadFriendsCompleteDelegate the delegate to use for notifications
 */
function AddReadFriendsCompleteDelegate(byte LocalUserNum,delegate<OnReadFriendsComplete> ReadFriendsCompleteDelegate)
{
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		if (FriendsCache[LocalUserNum].ReadFriendsDelegates.Find(ReadFriendsCompleteDelegate) == INDEX_NONE)
		{
			FriendsCache[LocalUserNum].ReadFriendsDelegates[FriendsCache[LocalUserNum].ReadFriendsDelegates.Length] = ReadFriendsCompleteDelegate;
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to AddReadFriendsCompleteDelegate()");
	}
}

/**
 * Removes the delegate from the list of notifications
 *
 * @param LocalUserNum the user to read the friends list of
 * @param ReadFriendsCompleteDelegate the delegate to use for notifications
 */
function ClearReadFriendsCompleteDelegate(byte LocalUserNum,delegate<OnReadFriendsComplete> ReadFriendsCompleteDelegate)
{
	local int RemoveIndex;

	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = FriendsCache[LocalUserNum].ReadFriendsDelegates.Find(ReadFriendsCompleteDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			FriendsCache[LocalUserNum].ReadFriendsDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearReadFriendsCompleteDelegate()");
	}
}

/**
 * Copies the list of friends for the player previously retrieved from the online
 * service. The list can be retrieved in whole or in part.
 *
 * @param LocalUserNum the user to read the friends list of
 * @param Friends the out array that receives the copied data
 * @param Count the number of friends to read or zero for all
 * @param StartingAt the index of the friends list to start at (for pulling partial lists)
 *
 * @return ERS_Done if the read has completed, otherwise one of the other states
 */
native function EOnlineEnumerationReadState GetFriendsList(byte LocalUserNum,out array<OnlineFriend> Friends,optional int Count,optional int StartingAt);

/**
 * Called when a user accepts a game invitation. Allows the gameplay code a chance
 * to clean up any existing state before accepting the invite. The invite must be
 * accepted by calling AcceptGameInvite() on the OnlineGameInterface after clean up
 * has completed
 *
 * @param InviteSettings the settings for the game we're joining via invite
 */
delegate OnGameInviteAccepted(OnlineGameSettings InviteSettings);

/**
 * Sets the delegate used to notify the gameplay code when a game invite has been accepted
 *
 * @param LocalUserNum the user to request notification for
 * @param GameInviteAcceptedDelegate the delegate to use for notifications
 */
function AddGameInviteAcceptedDelegate(byte LocalUserNum,delegate<OnGameInviteAccepted> GameInviteAcceptedDelegate)
{
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		if (InviteCache[LocalUserNum].InviteDelegates.Find(GameInviteAcceptedDelegate) == INDEX_NONE)
		{
			InviteCache[LocalUserNum].InviteDelegates[InviteCache[LocalUserNum].InviteDelegates.Length] = GameInviteAcceptedDelegate;
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to AddGameInviteAcceptedDelegate()");
	}
}

/**
 * Removes the delegate used to notify the gameplay code when a game invite has been accepted
 *
 * @param LocalUserNum the user to request notification for
 * @param GameInviteAcceptedDelegate the delegate to use for notifications
 */
function ClearGameInviteAcceptedDelegate(byte LocalUserNum,delegate<OnGameInviteAccepted> GameInviteAcceptedDelegate)
{
	local int RemoveIndex;

	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = InviteCache[LocalUserNum].InviteDelegates.Find(GameInviteAcceptedDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			InviteCache[LocalUserNum].InviteDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearGameInviteAcceptedDelegate()");
	}
}

/**
 * Delegate used in content change (add or deletion) notifications
 * for any user
 */
delegate OnContentChange();

/**
 * Sets the delegate used to notify the gameplay code that (downloaded) content changed
 *
 * @param Content Delegate the delegate to use for notifications
 * @param LocalUserNum whether to watch for changes on a specific slot or all slots
 */
function AddContentChangeDelegate(delegate<OnContentChange> ContentDelegate,optional byte LocalUserNum = 255)
{
	if (LocalUserNum == 255)
	{
		if (AnyContentChangeDelegates.Find(ContentDelegate) == INDEX_NONE)
		{
			AnyContentChangeDelegates[AnyContentChangeDelegates.Length] = ContentDelegate;
		}
	}
	// Make sure it's within range
	else if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		if (ContentCache[LocalUserNum].ContentChangeDelegates.Find(ContentDelegate) == INDEX_NONE)
		{
			ContentCache[LocalUserNum].ContentChangeDelegates[ContentCache[LocalUserNum].ContentChangeDelegates.Length] = ContentDelegate;
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to AddContentChangeDelegate()");
	}
}

/**
 * Sets the delegate used to notify the gameplay code that (downloaded) content changed
 *
 * @param Content Delegate the delegate to use for notifications
 * @param LocalUserNum whether to watch for changes on a specific slot or all slots
 */
function ClearContentChangeDelegate(delegate<OnContentChange> ContentDelegate,optional byte LocalUserNum = 255)
{
	local int RemoveIndex;

	if (LocalUserNum == 255)
	{
		RemoveIndex = AnyContentChangeDelegates.Find(ContentDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			AnyContentChangeDelegates.Remove(RemoveIndex,1);
		}
	}
	// Make sure it's within range
	else if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = ContentCache[LocalUserNum].ContentChangeDelegates.Find(ContentDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			ContentCache[LocalUserNum].ContentChangeDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearContentChangeDelegate()");
	}
}

/**
 * Delegate used when the content read request has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnReadContentComplete(bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the content read request has completed
 *
 * @param LocalUserNum the user to read the content list of
 * @param ReadContentCompleteDelegate the delegate to use for notifications
 */
function AddReadContentComplete(byte LocalUserNum,delegate<OnReadContentComplete> ReadContentCompleteDelegate)
{
	// Make sure it's within range
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		if (ContentCache[LocalUserNum].ReadCompleteDelegates.Find(ReadContentCompleteDelegate) == INDEX_NONE)
		{
			ContentCache[LocalUserNum].ReadCompleteDelegates[ContentCache[LocalUserNum].ReadCompleteDelegates.Length] = ReadContentCompleteDelegate;
		}
	}
	else
	{
		`Warn("Invalid index ("$LocalUserNum$") passed to AddReadContentComplete()");
	}
}

/**
 * Sets the delegate used to notify the gameplay code that the content read request has completed
 *
 * @param LocalUserNum the user to read the content list of
 * @param ReadContentCompleteDelegate the delegate to use for notifications
 */
function ClearReadContentComplete(byte LocalUserNum,delegate<OnReadContentComplete> ReadContentCompleteDelegate)
{
	local int RemoveIndex;

	// Make sure it's within range
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = ContentCache[LocalUserNum].ReadCompleteDelegates.Find(ReadContentCompleteDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			ContentCache[LocalUserNum].ReadCompleteDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearReadContentComplete()");
	}
}

/**
 * Starts an async task that retrieves the list of downloaded content for the player.
 *
 * @param LocalUserNum The user to read the content list of
 *
 * @return true if the read request was issued successfully, false otherwise
 */
native function bool ReadContentList(byte LocalUserNum);

/**
 * Retrieve the list of content the given user has downloaded or otherwise retrieved
 * to the local console.

 * @param LocalUserNum The user to read the content list of
 * @param ContentList The out array that receives the list of all content
 *
 * @return ERS_Done if the read has completed, otherwise one of the other states
 */
native function EOnlineEnumerationReadState GetContentList(byte LocalUserNum, out array<OnlineContent> ContentList);

/**
 * Asks the online system for the number of new and total content downloads
 *
 * @param LocalUserNum the user to check the content download availability for
 * @param CategoryMask the bitmask to use to filter content by type
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
native function bool QueryAvailableDownloads(byte LocalUserNum,optional int CategoryMask = -1);

/**
 * Called once the download query completes
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnQueryAvailableDownloadsComplete(bool bWasSuccessful);

/**
 * Sets the delegate used to notify the gameplay code that the content download query has completed
 *
 * @param LocalUserNum the user to check the content download availability for
 * @param ReadContentCompleteDelegate the delegate to use for notifications
 */
function AddQueryAvailableDownloadsComplete(byte LocalUserNum,delegate<OnQueryAvailableDownloadsComplete> QueryDownloadsDelegate)
{
	// Make sure it's within range
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		if (ContentCache[LocalUserNum].QueryDownloadsDelegates.Find(QueryDownloadsDelegate) == INDEX_NONE)
		{
			ContentCache[LocalUserNum].QueryDownloadsDelegates[ContentCache[LocalUserNum].QueryDownloadsDelegates.Length] = QueryDownloadsDelegate;
		}
	}
	else
	{
		`Warn("Invalid index ("$LocalUserNum$") passed to AddQueryAvailableDownloadsComplete()");
	}
}

/**
 * Sets the delegate used to notify the gameplay code that the content download query has completed
 *
 * @param LocalUserNum the user to check the content download availability for
 * @param ReadContentCompleteDelegate the delegate to use for notifications
 */
function ClearQueryAvailableDownloadsComplete(byte LocalUserNum,delegate<OnQueryAvailableDownloadsComplete> QueryDownloadsDelegate)
{
	local int RemoveIndex;

	// Make sure it's within range
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = ContentCache[LocalUserNum].QueryDownloadsDelegates.Find(QueryDownloadsDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			ContentCache[LocalUserNum].QueryDownloadsDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearQueryAvailableDownloadsComplete()");
	}
}

/**
 * Returns the number of new and total downloads available for the user
 *
 * @param LocalUserNum the user to check the content download availability for
 * @param NewDownloads out value of the number of new downloads available
 * @param TotalDownloads out value of the number of total downloads available
 */
function GetAvailableDownloadCounts(byte LocalUserNum,out int NewDownloads,out int TotalDownloads)
{
	// Make sure it's within range
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		NewDownloads = ContentCache[LocalUserNum].NewDownloadCount;
		TotalDownloads = ContentCache[LocalUserNum].TotalDownloadCount;
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to GetAvailableDownloadCounts()");
	}
}

/**
 * Registers the user as a talker
 *
 * @param LocalUserNum the local player index that is a talker
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
native function bool RegisterLocalTalker(byte LocalUserNum);

/**
 * Unregisters the user as a talker
 *
 * @param LocalUserNum the local player index to be removed
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
native function bool UnregisterLocalTalker(byte LocalUserNum);

/**
 * Registers a remote player as a talker
 *
 * @param PlayerId the unique id of the remote player that is a talker
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
native function bool RegisterRemoteTalker(UniqueNetId PlayerId);

/**
 * Unregisters a remote player as a talker
 *
 * @param PlayerId the unique id of the remote player to be removed
 *
 * @return TRUE if the call succeeded, FALSE otherwise
 */
native function bool UnregisterRemoteTalker(UniqueNetId PlayerId);

/**
 * Determines if the specified player is actively talking into the mic
 *
 * @param LocalUserNum the local player index being queried
 *
 * @return TRUE if the player is talking, FALSE otherwise
 */
native function bool IsLocalPlayerTalking(byte LocalUserNum);

/**
 * Determines if the specified remote player is actively talking into the mic
 * NOTE: Network latencies will make this not 100% accurate
 *
 * @param PlayerId the unique id of the remote player being queried
 *
 * @return TRUE if the player is talking, FALSE otherwise
 */
native function bool IsRemotePlayerTalking(UniqueNetId PlayerId);

/**
 * Determines if the specified player has a headset connected
 *
 * @param LocalUserNum the local player index being queried
 *
 * @return TRUE if the player has a headset plugged in, FALSE otherwise
 */
native function bool IsHeadsetPresent(byte LocalUserNum);

/**
 * Sets the relative priority for a remote talker. 0 is highest
 *
 * @param LocalUserNum the user that controls the relative priority
 * @param PlayerId the remote talker that is having their priority changed for
 * @param Priority the relative priority to use (0 highest, < 0 is muted)
 *
 * @return TRUE if the function succeeds, FALSE otherwise
 */
native function bool SetRemoteTalkerPriority(byte LocalUserNum,UniqueNetId PlayerId,int Priority);

/**
 * Mutes a remote talker for the specified local player. NOTE: This is separate
 * from the user's permanent online mute list
 *
 * @param LocalUserNum the user that is muting the remote talker
 * @param PlayerId the remote talker that is being muted
 *
 * @return TRUE if the function succeeds, FALSE otherwise
 */
native function bool MuteRemoteTalker(byte LocalUserNum,UniqueNetId PlayerId);

/**
 * Allows a remote talker to talk to the specified local player. NOTE: This call
 * will fail for remote talkers on the user's permanent online mute list
 *
 * @param LocalUserNum the user that is allowing the remote talker to talk
 * @param PlayerId the remote talker that is being restored to talking
 *
 * @return TRUE if the function succeeds, FALSE otherwise
 */
native function bool UnmuteRemoteTalker(byte LocalUserNum,UniqueNetId PlayerId);

/**
 * Called when a player is talking either locally or remote. This will be called
 * once for each active talker each frame.
 *
 * @param Player the player that is talking
 */
delegate OnPlayerTalking(UniqueNetId Player);

/**
 * Adds a talker delegate to the list of notifications
 *
 * @param TalkerDelegate the delegate to call when a player is talking
 */
function AddPlayerTalkingDelegate(delegate<OnPlayerTalking> TalkerDelegate)
{
	local int AddIndex;
	// Add this delegate to the array if not already present
	if (TalkingDelegates.Find(TalkerDelegate) == INDEX_NONE)
	{
		AddIndex = TalkingDelegates.Length;
		TalkingDelegates.Length = TalkingDelegates.Length + 1;
		TalkingDelegates[AddIndex] = TalkerDelegate;
	}
}

/**
 * Removes a talker delegate to the list of notifications
 *
 * @param TalkerDelegate the delegate to remove from the notification list
 */
function ClearPlayerTalkingDelegate(delegate<OnPlayerTalking> TalkerDelegate)
{
	local int RemoveIndex;
	RemoveIndex = TalkingDelegates.Find(TalkerDelegate);
	// Only remove if found
	if (RemoveIndex != INDEX_NONE)
	{
		TalkingDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Tells the voice layer that networked processing of the voice data is allowed
 * for the specified player. This allows for push-to-talk style voice communication
 *
 * @param LocalUserNum the local user to allow network transimission for
 */
native function StartNetworkedVoice(byte LocalUserNum);

/**
 * Tells the voice layer to stop processing networked voice support for the
 * specified player. This allows for push-to-talk style voice communication
 *
 * @param LocalUserNum the local user to disallow network transimission for
 */
native function StopNetworkedVoice(byte LocalUserNum);

/**
 * Tells the voice system to start tracking voice data for speech recognition
 *
 * @param LocalUserNum the local user to recognize voice data for
 *
 * @return true upon success, false otherwise
 */
native function bool StartSpeechRecognition(byte LocalUserNum);

/**
 * Tells the voice system to stop tracking voice data for speech recognition
 *
 * @param LocalUserNum the local user to recognize voice data for
 *
 * @return true upon success, false otherwise
 */
native function bool StopSpeechRecognition(byte LocalUserNum);

/**
 * Gets the results of the voice recognition
 *
 * @param LocalUserNum the local user to read the results of
 * @param Words the set of words that were recognized by the voice analyzer
 *
 * @return true upon success, false otherwise
 */
native function bool GetRecognitionResults(byte LocalUserNum,out array<SpeechRecognizedWord> Words);

/**
 * Called when speech recognition for a given player has completed. The
 * consumer of the notification can call GetRecognitionResults() to get the
 * words that were recognized
 */
delegate OnRecognitionComplete();

/**
 * Sets the speech recognition notification callback to use for the specified user
 *
 * @param LocalUserNum the local user to receive notifications for
 * @param RecognitionDelegate the delegate to call when recognition is complete
 */
function AddRecognitionCompleteDelegate(byte LocalUserNum,delegate<OnRecognitionComplete> RecognitionDelegate)
{
	// Make sure it's within range
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		if (PerUserDelegates[LocalUserNum].SpeechRecognitionDelegates.Find(RecognitionDelegate) == INDEX_NONE)
		{
			PerUserDelegates[LocalUserNum].SpeechRecognitionDelegates[PerUserDelegates[LocalUserNum].SpeechRecognitionDelegates.Length] = RecognitionDelegate;
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to AddRecognitionCompleteDelegate()");
	}
}

/**
 * Clears the speech recognition notification callback to use for the specified user
 *
 * @param LocalUserNum the local user to receive notifications for
 * @param RecognitionDelegate the delegate to call when recognition is complete
 */
function ClearRecognitionCompleteDelegate(byte LocalUserNum,delegate<OnRecognitionComplete> RecognitionDelegate)
{
	local int RemoveIndex;

	// Make sure it's within range
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = PerUserDelegates[LocalUserNum].SpeechRecognitionDelegates.Find(RecognitionDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			PerUserDelegates[LocalUserNum].SpeechRecognitionDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearRecognitionCompleteDelegate()");
	}
}

/**
 * Changes the vocabulary id that is currently being used
 *
 * @param LocalUserNum the local user that is making the change
 * @param VocabularyId the new id to use
 *
 * @return true if successful, false otherwise
 */
native function bool SelectVocabulary(byte LocalUserNum,int VocabularyId);

/**
 * Changes the object that is in use to the one specified
 *
 * @param LocalUserNum the local user that is making the change
 * @param SpeechRecogObj the new object use
 *
 * @param true if successful, false otherwise
 */
native function bool SetSpeechRecognitionObject(byte LocalUserNum,SpeechRecognition SpeechRecogObj);

/**
 * Reads a set of stats for the specified list of players
 *
 * @param Players the array of unique ids to read stats for
 * @param StatsRead holds the definitions of the tables to read the data from and
 *		  results are copied into the specified object
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
native function bool ReadOnlineStats(const out array<UniqueNetId> Players,OnlineStatsRead StatsRead);

/**
 * Reads a player's stats and all of that player's friends stats for the
 * specified set of stat views. This allows you to easily compare a player's
 * stats to their friends.
 *
 * @param LocalUserNum the local player having their stats and friend's stats read for
 * @param StatsRead holds the definitions of the tables to read the data from and
 *		  results are copied into the specified object
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
native function bool ReadOnlineStatsForFriends(byte LocalUserNum,OnlineStatsRead StatsRead);

/**
 * Reads stats by ranking. This grabs the rows starting at StartIndex through
 * NumToRead and places them in the StatsRead object.
 *
 * @param StatsRead holds the definitions of the tables to read the data from and
 *		  results are copied into the specified object
 * @param StartIndex the starting rank to begin reads at (1 for top)
 * @param NumToRead the number of rows to read (clamped at 100 underneath)
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
native function bool ReadOnlineStatsByRank(OnlineStatsRead StatsRead,optional int StartIndex = 1,optional int NumToRead = 100);

/**
 * Reads stats by ranking centered around a player. This grabs a set of rows
 * above and below the player's current rank
 *
 * @param LocalUserNum the local player having their stats being centered upon
 * @param StatsRead holds the definitions of the tables to read the data from and
 *		  results are copied into the specified object
 * @param NumRows the number of rows to read above and below the player's rank
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
native function bool ReadOnlineStatsByRankAroundPlayer(byte LocalUserNum,OnlineStatsRead StatsRead,optional int NumRows = 10);

/**
 * Notifies the interested party that the last stats read has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnReadOnlineStatsComplete(bool bWasSuccessful);

/**
 * Adds the delegate used to notify the gameplay code that the stats read has completed
 *
 * @param ReadOnlineStatsCompleteDelegate the delegate to use for notifications
 */
function AddReadOnlineStatsCompleteDelegate(delegate<OnReadOnlineStatsComplete> ReadOnlineStatsCompleteDelegate)
{
	local int AddIndex;
	// Used to forward the event to the list
	OnReadOnlineStatsComplete = MulticastReadOnlineStatsComplete;
	// Only add to the list once
	if (ReadOnlineStatsCompleteDelegates.Find(ReadOnlineStatsCompleteDelegate) == INDEX_NONE)
	{
		AddIndex = ReadOnlineStatsCompleteDelegates.Length;
		ReadOnlineStatsCompleteDelegates.Length = ReadOnlineStatsCompleteDelegates.Length + 1;
		ReadOnlineStatsCompleteDelegates[AddIndex] = ReadOnlineStatsCompleteDelegate;
	}
}

/**
 * Removes the delegate from the notify list
 *
 * @param ReadOnlineStatsCompleteDelegate the delegate to use for notifications
 */
function ClearReadOnlineStatsCompleteDelegate(delegate<OnReadOnlineStatsComplete> ReadOnlineStatsCompleteDelegate)
{
	local int RemoveIndex;
	// Find it in the list
	RemoveIndex = ReadOnlineStatsCompleteDelegates.Find(ReadOnlineStatsCompleteDelegate);
	// Only remove if found
	if (RemoveIndex != INDEX_NONE)
	{
		ReadOnlineStatsCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Local version of the delegate that sends to the array of subscribers
 *
 * @param bWasSuccessful whether the call completed or not
 */
function MulticastReadOnlineStatsComplete(bool bWasSuccessful)
{
	local int Index;
	local delegate<OnReadOnlineStatsComplete> Subscriber;

	// Loop through and notify all subscribed delegates
	for (Index = 0; Index < ReadOnlineStatsCompleteDelegates.Length; Index++)
	{
		Subscriber = ReadOnlineStatsCompleteDelegates[Index];
		Subscriber(bWasSuccessful);
	}
}

/**
 * Cleans up any platform specific allocated data contained in the stats data
 *
 * @param StatsRead the object to handle per platform clean up on
 */
native function FreeStats(OnlineStatsRead StatsRead);

/**
 * Writes out the stats contained within the stats write object to the online
 * subsystem's cache of stats data. Note the new data replaces the old. It does
 * not write the data to the permanent storage until a FlushOnlineStats() call
 * or a session ends. Stats cannot be written without a session or the write
 * request is ignored. No more than 5 stats views can be written to at a time
 * or the write request is ignored.
 *
 * @param SessionName the name of the session to write stats for
 * @param Player the player to write stats for
 * @param StatsWrite the object containing the information to write
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
native function bool WriteOnlineStats(name SessionName,UniqueNetId Player,OnlineStatsWrite StatsWrite);

/**
 * Commits any changes in the online stats cache to the permanent storage
 *
 * @param SessionName the name of the session that stats are being flushed for
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
native function bool FlushOnlineStats(name SessionName);

/**
 * Delegate called when the stats flush operation has completed
 *
 * @param SessionName the name of the session having stats flushed
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
delegate OnFlushOnlineStatsComplete(name SessionName,bool bWasSuccessful);

/**
 * Adds the delegate used to notify the gameplay code that the stats flush has completed
 *
 * @param FlushOnlineStatsCompleteDelegate the delegate to use for notifications
 */
function AddFlushOnlineStatsCompleteDelegate(delegate<OnFlushOnlineStatsComplete> FlushOnlineStatsCompleteDelegate)
{
	if (FlushOnlineStatsDelegates.Find(FlushOnlineStatsCompleteDelegate) == INDEX_NONE)
	{
		FlushOnlineStatsDelegates[FlushOnlineStatsDelegates.Length] = FlushOnlineStatsCompleteDelegate;
	}
}

/**
 * Clears the delegate used to notify the gameplay code that the stats flush has completed
 *
 * @param FlushOnlineStatsCompleteDelegate the delegate to use for notifications
 */
function ClearFlushOnlineStatsCompleteDelegate(delegate<OnFlushOnlineStatsComplete> FlushOnlineStatsCompleteDelegate)
{
	local int RemoveIndex;

	RemoveIndex = FlushOnlineStatsDelegates.Find(FlushOnlineStatsCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		FlushOnlineStatsDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Writes the score data for the match
 *
 * @param SessionName the name of the session to write scores for
 * @param LeaderboardId the leaderboard to write the score information to
 * @param PlayerScores the list of players, teams, and scores they earned
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
native function bool WriteOnlinePlayerScores(name SessionName,int LeaderboardId,const out array<OnlinePlayerScore> PlayerScores);

/**
 * Sends a friend invite to the specified player
 *
 * @param LocalUserNum the user that is sending the invite
 * @param NewFriend the player to send the friend request to
 * @param Message the message to display to the recipient
 *
 * @return true if successful, false otherwise
 */
function bool AddFriend(byte LocalUserNum,UniqueNetId NewFriend,optional string Message)
{
	return ShowFriendsInviteUI(LocalUserNum,NewFriend);
}

/**
 * Sends a friend invite to the specified player nick
 *
 * @param LocalUserNum the user that is sending the invite
 * @param FriendName the name of the player to send the invite to
 * @param Message the message to display to the recipient
 *
 * @return true if successful, false otherwise
 */
function bool AddFriendByName(byte LocalUserNum,string FriendName,optional string Message);

/**
 * Called when a friend invite arrives for a local player
 *
 * @param bWasSuccessful true if successfully added, false if not found or failed
 */
delegate OnAddFriendByNameComplete(bool bWasSuccessful);

/**
 * Adds the delegate used to notify the gameplay code that the user has received a friend invite
 *
 * @param LocalUserNum the user associated with the notification
 * @param FriendDelegate the delegate to use for notifications
 */
function AddAddFriendByNameCompleteDelegate(byte LocalUserNum,delegate<OnAddFriendByNameComplete> FriendDelegate);

/**
 * Removes the delegate specified from the list
 *
 * @param LocalUserNum the user associated with the notification
 * @param FriendDelegate the delegate to use for notifications
 */
function ClearAddFriendByNameCompleteDelegate(byte LocalUserNum,delegate<OnAddFriendByNameComplete> FriendDelegate);

/**
 * Used to accept a friend invite sent to this player
 *
 * @param LocalUserNum the user the invite is for
 * @param RequestingPlayer the player the invite is from
 *
 * @param true if successful, false otherwise
 */
function bool AcceptFriendInvite(byte LocalUserNum,UniqueNetId RequestingPlayer);

/**
 * Used to deny a friend request sent to this player
 *
 * @param LocalUserNum the user the invite is for
 * @param RequestingPlayer the player the invite is from
 *
 * @param true if successful, false otherwise
 */
function bool DenyFriendInvite(byte LocalUserNum,UniqueNetId RequestingPlayer);

/**
 * Removes a friend from the player's friend list
 *
 * @param LocalUserNum the user that is removing the friend
 * @param FormerFriend the player to remove from the friend list
 *
 * @return true if successful, false otherwise
 */
function bool RemoveFriend(byte LocalUserNum,UniqueNetId FormerFriend)
{
	return ShowGamerCardUI(LocalUserNum,FormerFriend);
}

/**
 * Called when a friend invite arrives for a local player
 *
 * @param LocalUserNum the user that is receiving the invite
 * @param RequestingPlayer the player sending the friend request
 * @param RequestingNick the nick of the player sending the friend request
 * @param Message the message to display to the recipient
 *
 * @return true if successful, false otherwise
 */
delegate OnFriendInviteReceived(byte LocalUserNum,UniqueNetId RequestingPlayer,string RequestingNick,string Message);

/**
 * Adds the delegate used to notify the gameplay code that the user has received a friend invite
 *
 * @param LocalUserNum the user associated with the notification
 * @param InviteDelegate the delegate to use for notifications
 */
function AddFriendInviteReceivedDelegate(byte LocalUserNum,delegate<OnFriendInviteReceived> InviteDelegate);

/**
 * Removes the delegate specified from the list
 *
 * @param LocalUserNum the user associated with the notification
 * @param InviteDelegate the delegate to use for notifications
 */
function ClearFriendInviteReceivedDelegate(byte LocalUserNum,delegate<OnFriendInviteReceived> InviteDelegate);

/**
 * Sends a message to a friend
 *
 * @param LocalUserNum the user that is sending the message
 * @param Friend the player to send the message to
 * @param Message the message to display to the recipient
 *
 * @return true if successful, false otherwise
 */
native function bool SendMessageToFriend(byte LocalUserNum,UniqueNetId Friend,string Message);

/**
 * Sends an invitation to play in the player's current session
 *
 * @param LocalUserNum the user that is sending the invite
 * @param Friend the player to send the invite to
 * @param Text the text of the message for the invite
 *
 * @return true if successful, false otherwise
 */
native function bool SendGameInviteToFriend(byte LocalUserNum,UniqueNetId Friend,optional string Text);

/**
 * Sends invitations to play in the player's current session
 *
 * @param LocalUserNum the user that is sending the invite
 * @param Friends the player to send the invite to
 * @param Text the text of the message for the invite
 *
 * @return true if successful, false otherwise
 */
native function bool SendGameInviteToFriends(byte LocalUserNum,array<UniqueNetId> Friends,optional string Text);

/**
 * Called when the online system receives a game invite that needs handling
 *
 * @param LocalUserNum the user that is receiving the invite
 * @param InviterName the nick name of the person sending the invite
 */
delegate OnReceivedGameInvite(byte LocalUserNum,string InviterName);

/**
 * Adds the delegate used to notify the gameplay code that the user has received a game invite
 *
 * @param LocalUserNum the user associated with the notification
 * @param ReceivedGameInviteDelegate the delegate to use for notifications
 */
function AddReceivedGameInviteDelegate(byte LocalUserNum,delegate<OnReceivedGameInvite> ReceivedGameInviteDelegate);

/**
 * Removes the delegate specified from the list
 *
 * @param LocalUserNum the user associated with the notification
 * @param ReceivedGameInviteDelegate the delegate to use for notifications
 */
function ClearReceivedGameInviteDelegate(byte LocalUserNum,delegate<OnReceivedGameInvite> ReceivedGameInviteDelegate);

/**
 * Allows the local player to follow a friend into a game
 *
 * @param LocalUserNum the local player wanting to join
 * @param Friend the player that is being followed
 *
 * @return true if the async call worked, false otherwise
 */
native function bool JoinFriendGame(byte LocalUserNum,UniqueNetId Friend);

/**
 * Called once the join task has completed
 *
 * @param bWasSuccessful the session was found and is joinable, false otherwise
 */
delegate OnJoinFriendGameComplete(bool bWasSuccessful);

/**
 * Sets the delegate used to notify when the join friend is complete
 *
 * @param JoinFriendGameCompleteDelegate the delegate to use for notifications
 */
function AddJoinFriendGameCompleteDelegate(delegate<OnJoinFriendGameComplete> JoinFriendGameCompleteDelegate)
{
	if (JoinFriendGameCompleteDelegates.Find(JoinFriendGameCompleteDelegate) == INDEX_NONE)
	{
		JoinFriendGameCompleteDelegates[JoinFriendGameCompleteDelegates.Length] = JoinFriendGameCompleteDelegate;
	}
}

/**
 * Removes the delegate from the list of notifications
 *
 * @param JoinFriendGameCompleteDelegate the delegate to use for notifications
 */
function ClearJoinFriendGameCompleteDelegate(delegate<OnJoinFriendGameComplete> JoinFriendGameCompleteDelegate)
{
	local int RemoveIndex;

	RemoveIndex = JoinFriendGameCompleteDelegates.Find(JoinFriendGameCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		JoinFriendGameCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Returns the list of messages for the specified player
 *
 * @param LocalUserNum the local player wanting to join
 * @param FriendMessages the set of messages cached locally for the player
 */
function GetFriendMessages(byte LocalUserNum,out array<OnlineFriendMessage> FriendMessages);

/**
 * Called when a friend invite arrives for a local player
 *
 * @param LocalUserNum the user that is receiving the invite
 * @param SendingPlayer the player sending the friend request
 * @param SendingNick the nick of the player sending the friend request
 * @param Message the message to display to the recipient
 *
 * @return true if successful, false otherwise
 */
delegate OnFriendMessageReceived(byte LocalUserNum,UniqueNetId SendingPlayer,string SendingNick,string Message);

/**
 * Adds the delegate used to notify the gameplay code that the user has received a friend invite
 *
 * @param LocalUserNum the user associated with the notification
 * @param MessageDelegate the delegate to use for notifications
 */
function AddFriendMessageReceivedDelegate(byte LocalUserNum,delegate<OnFriendMessageReceived> MessageDelegate);

/**
 * Removes the delegate specified from the list
 *
 * @param LocalUserNum the user associated with the notification
 * @param MessageDelegate the delegate to use for notifications
 */
function ClearFriendMessageReceivedDelegate(byte LocalUserNum,delegate<OnFriendMessageReceived> MessageDelegate);

/**
 * Reads the host's stat guid for synching up stats. Only valid on the host.
 *
 * @return the host's stat guid
 */
function string GetHostStatGuid();

/**
 * Registers the host's stat guid with the client for verification they are part of
 * the stat. Note this is an async task for any backend communication that needs to
 * happen before the registration is deemed complete
 *
 * @param HostStatGuid the host's stat guid
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
function bool RegisterHostStatGuid(const out string HostStatGuid);

/**
 * Called when the host stat guid registration is complete
 *
 * @param bWasSuccessful whether the registration has completed or not
 */
delegate OnRegisterHostStatGuidComplete(bool bWasSuccessful);

/**
 * Adds the delegate for notifying when the host guid registration is done
 *
 * @param RegisterHostStatGuidCompleteDelegate the delegate to use for notifications
 */
function AddRegisterHostStatGuidCompleteDelegate(delegate<OnFlushOnlineStatsComplete> RegisterHostStatGuidCompleteDelegate);

/**
 * Clears the delegate used to notify the gameplay code
 *
 * @param RegisterHostStatGuidCompleteDelegate the delegate to use for notifications
 */
function ClearRegisterHostStatGuidCompleteDelegateDelegate(delegate<OnFlushOnlineStatsComplete> RegisterHostStatGuidCompleteDelegate);

/**
 * Reads the client's stat guid that was generated by registering the host's guid
 * Used for synching up stats. Only valid on the client. Only callable after the
 * host registration has completed
 *
 * @return the client's stat guid
 */
function string GetClientStatGuid();

/**
 * Registers the client's stat guid on the host to validate that the client was in the stat.
 * Used for synching up stats. Only valid on the host.
 *
 * @param PlayerId the client's unique net id
 * @param ClientStatGuid the client's stat guid
 *
 * @return TRUE if the call is successful, FALSE otherwise
 */
function bool RegisterStatGuid(UniqueNetId PlayerId,const out string ClientStatGuid);

/**
 * Mutes all voice or all but friends
 *
 * @param LocalUserNum the local user that is making the change
 * @param bAllowFriends whether to mute everyone or allow friends
 */
function bool MuteAll(byte LocalUserNum,bool bAllowFriends);

/**
 * Allows all speakers to send voice
 *
 * @param LocalUserNum the local user that is making the change
 */
function bool UnmuteAll(byte LocalUserNum);

/**
 * Deletes a message from the list of messages
 *
 * @param LocalUserNum the user that is deleting the message
 * @param MessageIndex the index of the message to delete
 *
 * @return true if the message was deleted, false otherwise
 */
function bool DeleteMessage(byte LocalUserNum,int MessageIndex);

/**
 * Starts an async read for the achievement list
 *
 * @param LocalUserNum the controller number of the associated user
 * @param TitleId the title id of the game the achievements are to be read for
 * @param bShouldReadText whether to fetch the text strings or not
 * @param bShouldReadImages whether to fetch the image data or not
 *
 * @return TRUE if the task starts, FALSE if it failed
 */
native function bool ReadAchievements(byte LocalUserNum,optional int TitleId = 0,optional bool bShouldReadText = true,optional bool bShouldReadImages = false);

/**
 * Called when the async achievements read has completed
 *
 * @param TitleId the title id that the read was for
 */
delegate OnReadAchievementsComplete(int TitleId);

/**
 * Sets the delegate used to notify the gameplay code that the achievements read request has completed
 *
 * @param LocalUserNum the user to read the achievements list for
 * @param ReadAchievementsCompleteDelegate the delegate to use for notifications
 */
function AddReadAchievementsCompleteDelegate(byte LocalUserNum,delegate<OnReadAchievementsComplete> ReadAchievementsCompleteDelegate)
{
	// Make sure the user is valid
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		if (PerUserDelegates[LocalUserNum].AchievementReadDelegates.Find(ReadAchievementsCompleteDelegate) == INDEX_NONE)
		{
			PerUserDelegates[LocalUserNum].AchievementReadDelegates[PerUserDelegates[LocalUserNum].AchievementReadDelegates.Length] = ReadAchievementsCompleteDelegate;
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to AddReadAchievementsComplete()");
	}
}

/**
 * Clears the delegate used to notify the gameplay code that the achievements read request has completed
 *
 * @param LocalUserNum the user to read the achievements list for
 * @param ReadAchievementsCompleteDelegate the delegate to use for notifications
 */
function ClearReadAchievementsCompleteDelegate(byte LocalUserNum,delegate<OnReadAchievementsComplete> ReadAchievementsCompleteDelegate)
{
	local int RemoveIndex;

	// Make sure the user is valid
	if (LocalUserNum >= 0 && LocalUserNum < 4)
	{
		RemoveIndex = PerUserDelegates[LocalUserNum].AchievementReadDelegates.Find(ReadAchievementsCompleteDelegate);
		if (RemoveIndex != INDEX_NONE)
		{
			PerUserDelegates[LocalUserNum].AchievementReadDelegates.Remove(RemoveIndex,1);
		}
	}
	else
	{
		`warn("Invalid index ("$LocalUserNum$") passed to ClearReadAchievementsCompleteDelegate()");
	}
}

/**
 * Copies the list of achievements for the specified player and title id
 *
 * @param LocalUserNum the user to read the friends list of
 * @param Achievements the out array that receives the copied data
 * @param TitleId the title id of the game that these were read for
 *
 * @return OERS_Done if the read has completed, otherwise one of the other states
 */
native function EOnlineEnumerationReadState GetAchievements(byte LocalUserNum,out array<AchievementDetails> Achievements,optional int TitleId = 0);

/**
 * Shows a custom players UI for the specified list of players
 *
 * @param LocalUserNum the controller number of the associated user
 * @param Players the list of players to show in the custom UI
 * @param Title the title to use for the UI
 * @param Description the text to show at the top of the UI
 *
 * @return TRUE if it was able to show the UI, FALSE if it failed
 */
native function bool ShowCustomPlayersUI(byte LocalUserNum,const out array<UniqueNetId> Players,string Title,string Description);

/**
 * Calls the base version to log Unreal's state and then calls the code that dumps
 * the Live specific view of that state
 */
function DumpSessionState()
{
	Super.DumpSessionState();
	DumpLiveSessionState();
}

/**
 * Enumerates the sessions that are set and call XSessionGetDetails() on them to
 * log Live's view of the session information
 */
native function DumpLiveSessionState();

/**
 * Logs the list of players that are registered for voice
 */
native function DumpVoiceRegistration();

/**
 * Sets the debug output level for the platform specific API (if applicable)
 *
 * @param DebugSpewLevel the level to set
 */
native function SetDebugSpewLevel(int DebugSpewLevel);

defaultproperties
{
	SigninCountDownDelay=1.0
}
