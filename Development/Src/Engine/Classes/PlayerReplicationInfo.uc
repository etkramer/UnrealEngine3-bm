//=============================================================================
// PlayerReplicationInfo.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//
// A PlayerReplicationInfo is created for every player on a server (or in a standalone game).
// Players are PlayerControllers, or other Controllers with bIsPlayer=true
// PlayerReplicationInfos are replicated to all clients, and contain network game relevant information about the player,
// such as playername, score, etc.
//=============================================================================
class PlayerReplicationInfo extends ReplicationInfo
	native
	nativereplication
	dependson(SoundNodeWave);

var databinding float				Score;			// Player's current score.
var databinding float				Deaths;			// Number of player's deaths.
var byte				Ping;
var Actor				PlayerLocationHint;
var 			int					NumLives;

var databinding repnotify string	PlayerName;		// Player name, or blank if none.
var databinding repnotify string 	PlayerAlias;	// The Player's current alias or blank if we are using the Player Name

var transient	ETTSSpeaker			TTSSpeaker;		// Voice to use for TTS

var string				OldName;
var int					PlayerID;		// Unique id number.
var RepNotify TeamInfo	Team;			// Player Team
var	int					SplitscreenIndex;	// -1 if not a splitscreen player, index into that player's local GamePlayers array otherwise

var bool				bAdmin;				// Player logged in as Administrator
var bool				bIsFemale;
var bool				bIsSpectator;
var bool				bOnlySpectator;
var bool				bWaitingPlayer;
var bool				bReadyToPlay;
var bool				bOutOfLives;
var bool				bBot;
var bool				bHasFlag;
var	bool				bHasBeenWelcomed;	// client side flag - whether this player has been welcomed or not

/** Means this PRI came from the GameInfo's InactivePRIArray */
var repnotify bool bIsInactive;
/** indicates this is a PRI from the previous level of a seamless travel,
 * waiting for the player to finish the transition before creating a new one
 * this is used to avoid preserving the PRI in the InactivePRIArray if the player leaves
 */
var bool bFromPreviousLevel;

/** This determines whether the user has turned on or off their Controller Vibration **/
var bool                bControllerVibrationAllowed;

var byte				PacketLoss;

// Time elapsed.
var int					StartTime;

var localized String	StringDead;
var localized String    StringSpectating;
var localized String	StringUnknown;

var databinding int		Kills;				// not replicated

var class<GameMessage>	GameMessageClass;

var float				ExactPing;

var string				SavedNetworkAddress;	/** Used to match up InactivePRI with rejoining playercontroller. */

/**
 * The id used by the network to uniquely identify a player.
 * NOTE: this property should *never* be exposed to the player as it's transient
 * and opaque in meaning (ie it might mean date/time followed by something else)
 */
var databinding repnotify UniqueNetId UniqueId;

/** Holds the player's skill for this game type */
var databinding int PlayerSkill;

/** The session that the player needs to join/remove from as it is created/leaves */
var name SessionName;


struct native AutomatedTestingDatum
{
	/** Number of matches played (maybe remove this before shipping)  This is really useful for doing soak testing and such to see how long you lasted! NOTE:  This is not replicated out to clients atm. **/
	var int NumberOfMatchesPlayed;

	/** Keeps track of the current run so when we have repeats and such we know how far along we are **/
	var int NumMapListCyclesDone;
};

var AutomatedTestingDatum AutomatedTestingData;



cpptext
{
	// AActor interface.
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}


replication
{
	// Things the server should send to the client.
	if ( bNetDirty && (Role == Role_Authority) )
		Score, Deaths, bHasFlag, PlayerLocationHint,
		PlayerName, PlayerAlias, Team, bIsFemale, bAdmin,
		bIsSpectator, bOnlySpectator, bWaitingPlayer, bReadyToPlay,
		StartTime, bOutOfLives, PlayerSkill,
		// NOTE: This needs to be replicated to the owning client so don't move it from here
		UniqueId;

	// sent to everyone except the player that belongs to this pri
	if ( bNetDirty && (Role == Role_Authority) && !bNetOwner )
		PacketLoss, Ping, SplitscreenIndex;

	if ( bNetInitial && (Role == Role_Authority) )
		PlayerID, bBot, bIsInactive;
}


/**
* Returns true if the id from the other PRI matches this PRI's id
*
* @param OtherPRI the PRI to compare IDs with
*/
native final function bool AreUniqueNetIdsEqual(PlayerReplicationInfo OtherPRI);


/**
 * Returns the alias to use for this player.  If PlayerAlias is blank, then the player name
 * is returned.
 */
native function string GetPlayerAlias();


simulated event PostBeginPlay()
{
	// register this PRI with the game's ReplicationInfo
	if ( WorldInfo.GRI != None )
		WorldInfo.GRI.AddPRI(self);

	if ( Role < ROLE_Authority )
		return;

    if (AIController(Owner) != None)
	{
		bBot = true;
	}

	StartTime = WorldInfo.GRI.ElapsedTime;
	Timer();
	SetTimer(1.5 + FRand(), true);
}

/* epic ===============================================
* ::ClientInitialize
*
* Called by Controller when its PlayerReplicationInfo is initially replicated.
* Now that
*
* =====================================================
*/
simulated function ClientInitialize(Controller C)
{
	local Actor A;
	local PlayerController PlayerOwner, FirstPlayer;
	local LocalPlayer LP;

	SetOwner(C);

	PlayerOwner = PlayerController(C);
	if ( PlayerOwner != None )
	{
		if ( PlayerOwner.IsSplitscreenPlayer() )
		{
			//@fixme ronp - not checking for none here so I get auto-error messages
			if ( PlayerOwner.NetPlayerIndex != 0 )
			{
				// we're the second player in a split-screen game - make sure the first player registered as a splitscreen
				// client
				LP = LocalPlayer(PlayerOwner.Player);
				FirstPlayer = LP.ViewportClient.GamePlayers[0].Actor;

				`assert(FirstPlayer != PlayerOwner);
				FirstPlayer.PlayerReplicationInfo.SetSplitscreenIndex(0);
			}

			SetSplitscreenIndex(PlayerOwner.NetPlayerIndex);
		}

		BindPlayerOwnerDataProvider();

		// any replicated playercontroller  must be this client's playercontroller
		if ( Team != Default.Team )
		{
			// wasnt' able to call this in ReplicatedEvent() when Team was replicated, because PlayerController did not have me as its PRI
			ForEach AllActors(class'Actor', A)
			{
				A.NotifyLocalPlayerTeamReceived();
			}
		}
	}
}

function SetPlayerTeam( TeamInfo NewTeam )
{
	bForceNetUpdate = Team != NewTeam;

	Team = NewTeam;
	UpdateTeamDataProvider();
}

/* epic ===============================================
* ::ReplicatedEvent
*
* Called when a variable with the property flag "RepNotify" is replicated
*
* =====================================================
*/
simulated event ReplicatedEvent(name VarName)
{
	local Pawn P;
	local PlayerController PC;
	local int WelcomeMessageNum;
	local Actor A;

	if ( VarName == 'Team' )
	{
		ForEach DynamicActors(class'Pawn', P)
		{
			// find my pawn and tell it
			if ( P.PlayerReplicationInfo == self )
			{
				P.NotifyTeamChanged();
				break;
			}
		}
		ForEach LocalPlayerControllers(class'PlayerController', PC)
		{
			if ( PC.PlayerReplicationInfo == self )
			{
				ForEach AllActors(class'Actor', A)
				{
					A.NotifyLocalPlayerTeamReceived();
				}

				break;
			}
		}

		ReplicatedDataBinding('Team');
	}
	else if ( VarName == 'PlayerName' )
	{
		// If the new name doesn't match what the local player thinks it should be,
		// then reupdate the name forcing it to be the unique profile name
		if (IsInvalidName())
		{
			return;
		}

		if ( WorldInfo.TimeSeconds < 2 )
		{
			bHasBeenWelcomed = true;
			OldName = PlayerName;
			return;
		}

		// new player or name change
		if ( bHasBeenWelcomed )
		{
			if( ShouldBroadCastWelcomeMessage() )
			{
				ForEach LocalPlayerControllers(class'PlayerController', PC)
				{
					PC.ReceiveLocalizedMessage( GameMessageClass, 2, self );
				}
			}
		}
		else
		{
			if ( bOnlySpectator )
				WelcomeMessageNum = 16;
			else
				WelcomeMessageNum = 1;

			bHasBeenWelcomed = true;

			if( ShouldBroadCastWelcomeMessage() )
			{
				ForEach LocalPlayerControllers(class'PlayerController', PC)
				{
					PC.ReceiveLocalizedMessage( GameMessageClass, WelcomeMessageNum, self );
				}
			}
		}
		OldName = PlayerName;
	}
	else if (VarName == 'UniqueId')
	{
		// Register the player as part of the session
		RegisterPlayerWithSession();
	}
	else if (VarName == 'bIsInactive')
	{
		// remove and re-add from the GRI so it's in the right list
		WorldInfo.GRI.RemovePRI(self);
		WorldInfo.GRI.AddPRI(self);
	}
}

/**
 * Called when a variable is replicated that has the 'databinding' keyword.
 *
 * @param	VarName		the name of the variable that was replicated.
 */
simulated event ReplicatedDataBinding( name VarName )
{
	Super.ReplicatedDataBinding(VarName);

	if ( VarName == 'Team' )
	{
		// notify the team data provider for our team that it should issue an update notification
		UpdateTeamDataProvider();
	}
	else
	{
		// If the new name doesn't match what the local player thinks it should be,
		// then reupdate the name forcing it to be the unique profile name
		if ( VarName == 'PlayerName' && IsInvalidName() )
		{
			return;
		}

		UpdatePlayerDataProvider(VarName);
	}
}

/* epic ===============================================
* ::UpdatePing
update average ping based on newly received round trip timestamp.
*/
final native function UpdatePing(float TimeStamp);

/**
 * Returns true if should broadcast player welcome/left messages.
 * Current conditions: must be a human player a network game */
simulated function bool ShouldBroadCastWelcomeMessage(optional bool bExiting)
{
	return (!bIsInactive && WorldInfo.NetMode != NM_StandAlone);
}

simulated event Destroyed()
{
	local PlayerController PC;

	if ( WorldInfo.GRI != None )
	{
		WorldInfo.GRI.RemovePRI(self);
	}

	if( ShouldBroadCastWelcomeMessage(TRUE) )
	{
		ForEach LocalPlayerControllers(class'PlayerController', PC)
		{
			PC.ReceiveLocalizedMessage( GameMessageClass, 4, self);
		}
	}

	// Remove the player from the online session
	UnregisterPlayerFromSession();

    Super.Destroyed();
}

/* Reset()
reset actor to initial state - used when restarting level without reloading.
*/
function Reset()
{
	Super.Reset();
	Score = 0;
	Kills = 0;
	Deaths = 0;
	bReadyToPlay = false;
	NumLives = 0;
	bOutOfLives = false;
	bForceNetUpdate = TRUE;
}

simulated function string GetHumanReadableName()
{
	return PlayerName;
}

simulated function string GetLocationName()
{
	local String LocationString;

    if( PlayerLocationHint == None )
		return StringSpectating;

	LocationString = PlayerLocationHint.GetLocationStringFor(self);
	return (LocationString == "") ? StringUnknown : LocationString;
}

function UpdatePlayerLocation()
{
    local Volume V, Best;
    local Pawn P;

    if( Controller(Owner) != None )
	{
		P = Controller(Owner).Pawn;
	}

    if( P == None )
	{
		PlayerLocationHint = None;
		return;
    }

    foreach P.TouchingActors( class'Volume', V )
    {
		if( V.LocationName == "" )
			continue;

		if( (Best != None) && (V.LocationPriority <= Best.LocationPriority) )
			continue;

		if( V.Encompasses(P) )
			Best = V;
	}
	PlayerLocationHint = (Best != None) ? Best : P.WorldInfo;
}

/* DisplayDebug()
list important controller attributes on canvas
*/
simulated function DisplayDebug(HUD HUD, out float YL, out float YPos)
{
	local float XS, YS;

	if ( Team == None )
		HUD.Canvas.SetDrawColor(255,255,0);
	else if ( Team.TeamIndex == 0 )
		HUD.Canvas.SetDrawColor(255,0,0);
	else
		HUD.Canvas.SetDrawColor(64,64,255);
	HUD.Canvas.SetPos(4, YPos);
    HUD.Canvas.Font	= class'Engine'.Static.GetSmallFont();
	HUD.Canvas.StrLen(PlayerName@"["$GetPlayerAlias()$"]", XS, YS);
	HUD.Canvas.DrawText(PlayerName@"["$GetPlayerAlias()$"]");
	HUD.Canvas.SetPos(4 + XS, YPos);
	HUD.Canvas.Font	= class'Engine'.Static.GetTinyFont();
	HUD.Canvas.SetDrawColor(255,255,0);
	if ( bHasFlag )
		HUD.Canvas.DrawText("   has flag ");

	YPos += YS;
	HUD.Canvas.SetPos(4, YPos);

	if ( !bBot && (PlayerController(HUD.Owner).ViewTarget != PlayerController(HUD.Owner).Pawn) )
	{
		HUD.Canvas.SetDrawColor(128,128,255);
		HUD.Canvas.DrawText("      bIsSpec:"@bIsSpectator@"OnlySpec:"$bOnlySpectator@"Waiting:"$bWaitingPlayer@"Ready:"$bReadyToPlay@"OutOfLives:"$bOutOfLives);
		YPos += YL;
		HUD.Canvas.SetPos(4, YPos);
	}
}

event Timer()
{
	UpdatePlayerLocation();
	SetTimer(1.5 + FRand(), true);
}

event SetPlayerName(string S)
{
	PlayerName = S;

	// ReplicatedEvent() won't get called by net code if we are the server
	if (WorldInfo.NetMode == NM_Standalone || WorldInfo.NetMode == NM_ListenServer)
	{
		ReplicatedEvent('PlayerName');
		ReplicatedDataBinding('PlayerName');
	}
	OldName = PlayerName;
	bForceNetUpdate = TRUE;
}

function SetWaitingPlayer(bool B)
{
	bIsSpectator = B;
	bWaitingPlayer = B;
	bForceNetUpdate = TRUE;
}

/* epic ===============================================
* ::Duplicate
Create duplicate PRI (for saving Inactive PRI)
*/
function PlayerReplicationInfo Duplicate()
{
	local PlayerReplicationInfo NewPRI;

	NewPRI = Spawn(class);
	CopyProperties(NewPRI);
	return NewPRI;
}

/* epic ===============================================
* ::OverrideWith
Get overridden properties from old PRI
*/
function OverrideWith(PlayerReplicationInfo PRI)
{
	bIsSpectator = PRI.bIsSpectator;
	bOnlySpectator = PRI.bOnlySpectator;
	bWaitingPlayer = PRI.bWaitingPlayer;
	bReadyToPlay = PRI.bReadyToPlay;
	bOutOfLives = PRI.bOutOfLives || bOutOfLives;

	Team = PRI.Team;
}

/* epic ===============================================
* ::CopyProperties
Copy properties which need to be saved in inactive PRI
*/
function CopyProperties(PlayerReplicationInfo PRI)
{
	PRI.Score = Score;
	PRI.Deaths = Deaths;
	PRI.Ping = Ping;
	PRI.NumLives = NumLives;
	PRI.PlayerName = PlayerName;
	PRI.PlayerID = PlayerID;
	PRI.StartTime = StartTime;
	PRI.Kills = Kills;
	PRI.bOutOfLives = bOutOfLives;
	PRI.SavedNetworkAddress = SavedNetworkAddress;
	PRI.Team = Team;
	PRI.UniqueId = UniqueId;
	PRI.AutomatedTestingData = AutomatedTestingData;
}

function IncrementDeaths(optional int Amt = 1)
{
	Deaths += Amt;
}

/** called by seamless travel when initializing a player on the other side - copy properties to the new PRI that should persist */
function SeamlessTravelTo(PlayerReplicationInfo NewPRI)
{
	CopyProperties(NewPRI);
	NewPRI.bOnlySpectator = bOnlySpectator;
}

/**
 * @return	a reference to the CurrentGame data store
 */
simulated function CurrentGameDataStore GetCurrentGameDS()
{
	local DataStoreClient DSClient;
	local CurrentGameDataStore Result;

	// get the global data store client
	DSClient = class'UIInteraction'.static.GetDataStoreClient();
	if ( DSClient != None )
	{
		// find the "CurrentGame" data store
		Result = CurrentGameDataStore(DSClient.FindDataStore('CurrentGame'));
		if ( Result == None )
		{
			`log(`location $ ": CurrentGame data store not found!",,'DevDataStore');
		}
	}

	return Result;
}

/**
 * Notifies the PlayerDataProvider associated with this PlayerReplicationInfo that a property's value has changed.
 *
 * @param	PropertyName	the name of the property that was changed.
 */
simulated function UpdatePlayerDataProvider( optional name PropertyName )
{
	local CurrentGameDataStore CurrentGameData;
	local PlayerDataProvider DataProvider;
	local TeamDataProvider TeamProvider;

	`log( `location@`showvar(PropertyName) @ `showvar(PlayerName),,'DevDataStore' );

	CurrentGameData = GetCurrentGameDS();
	if ( CurrentGameData != None )
	{
		DataProvider = CurrentGameData.GetPlayerDataProvider(Self);
		if ( DataProvider != None )
		{
			DataProvider.NotifyPropertyChanged(PropertyName);

			// notify the team data provider that it should refresh any lists bound to its players list
			if ( Team != None )
			{
				TeamProvider = CurrentGameData.GetTeamDataProvider(Team);
				if ( TeamProvider != None && TeamProvider.Players.Find(DataProvider) != INDEX_NONE )
				{
					TeamProvider.NotifyPropertyChanged(TeamProvider.PlayerListFieldName);
				}
			}
		}
	}
}

/**
 * Notifies the CurrentGame data store that this player's Team has changed.
 */
simulated function UpdateTeamDataProvider()
{
	local CurrentGameDataStore CurrentGameData;

	`log(`location@`showvar(PlayerName)@`showobj(Team),,'DevDataStore');

	CurrentGameData = GetCurrentGameDS();
	if ( CurrentGameData != None )
	{
		CurrentGameData.NotifyTeamChange();
	}
}

/**
 * Routes the team change notification to the CurrentGame data store.
 */
simulated function NotifyLocalPlayerTeamReceived()
{
	Super.NotifyLocalPlayerTeamReceived();

	UpdateTeamDataProvider();
}

/**
 * Finds the PlayerDataProvider that was registered with the CurrentGame data store for this PRI and links it to the
 * owning player's PlayerOwner data store.
 */
simulated function BindPlayerOwnerDataProvider()
{
	local PlayerController PlayerOwner;
	local LocalPlayer LP;
	local CurrentGameDataStore CurrentGameData;
	local PlayerDataProvider DataProvider;

	`log(">>" @ Self $ "::BindPlayerOwnerDataProvider" @ "(" $ PlayerName $ ")",,'DevDataStore');

	PlayerOwner = PlayerController(Owner);
	if ( PlayerOwner != None )
	{
		// only works if this is a local player
		LP = LocalPlayer(PlayerOwner.Player);
		if ( LP != None )
		{
			CurrentGameData = GetCurrentGameDS();
			if ( CurrentGameData != None )
			{
				// find the PlayerDataProvider that was created when this PRI was added to the GRI's PRI array.
				DataProvider = CurrentGameData.GetPlayerDataProvider(Self);
				if ( DataProvider != None )
				{
					// link it to the CurrentPlayer data provider
					PlayerOwner.SetPlayerDataProvider(DataProvider);
				}
				else
				{
					// @todo - is this an error or should we create one here?
					`log("No player data provider registered for player " $ Self @ "(" $ PlayerName $ ")",,'DevDataStore');
				}
			}
			else
			{
				`log("'CurrentGame' data store not found!",,'DevDataStore');
			}
		}
		else
		{
			`log("Non local player:" @ PlayerOwner.Player,,'DevDataStore');
		}
	}
	else
	{
		`log("Invalid owner:" @ Owner,,'DevDataStore');
	}

	`log("<<" @ Self $ "::BindPlayerOwnerDataProvider" @ "(" $ PlayerName $ ")",,'DevDataStore');
}

/** Utility for seeing if this PRI is for a locally controller player. */
simulated function bool IsLocalPlayerPRI()
{
	local PlayerController PC;
	local LocalPlayer LP;

	PC = PlayerController(Owner);
	if(PC != None)
	{
		LP = LocalPlayer(PC.Player);
		return (LP != None);
	}

	return FALSE;
}

/**
 * Set the value of SplitscreenIndex for this PRI, then send that value to the server.  Only called if the player is a splitscreen player.
 *
 * @param	PlayerIndex		the index [into the GamePlayers array] for the player that owns this PRI.
 */
simulated function SetSplitscreenIndex( byte PlayerIndex )
{
	SplitscreenIndex = PlayerIndex;
	ServerSetSplitscreenIndex(PlayerIndex);
}

/**
 * Set the value of SplitscreenIndex for this PRI on the server so it can be received by all clients.
 *
 * @param	PlayerIndex		the index [into the GamePlayers array] for the player that owns this PRI.
 */
reliable server function ServerSetSplitscreenIndex( byte PlayerIndex )
{
	SplitscreenIndex = PlayerIndex;
}

/**
 * Sets the player's unique net id on the server.
 */
simulated function SetUniqueId( UniqueNetId PlayerUniqueId )
{
	// Store the unique id, so it will be replicated to all clients
	UniqueId = PlayerUniqueId;
}

simulated native function byte GetTeamNum();

/**
 * Validates that the new name matches the profile if the player is logged in
 *
 * @return TRUE if the name doesn't match, FALSE otherwise
 */
simulated function bool IsInvalidName()
{
	local LocalPlayer LocPlayer;
	local PlayerController PC;
	local string ProfileName;
	local OnlineSubsystem OnlineSub;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PC = PlayerController(Owner);
		if (PC != None)
		{
			LocPlayer = LocalPlayer(PC.Player);
			if (LocPlayer != None &&
				OnlineSub.GameInterface != None &&
				OnlineSub.PlayerInterface != None)
			{
				// Check to see if they are logged in locally or not
				if (OnlineSub.PlayerInterface.GetLoginStatus(LocPlayer.ControllerId) == LS_LoggedIn)
				{
					// Ignore what ever was specified and use the profile's nick
					ProfileName = OnlineSub.PlayerInterface.GetPlayerNickname(LocPlayer.ControllerId);
					if (ProfileName != PlayerName)
					{
						// Force an update to the proper name
						PC.SetName(ProfileName);
						return true;
					}
				}
			}
		}
	}
	return false;
}

function SetPlayerAlias(string NewAlias)
{
	PlayerAlias = NewAlias;
}

/**
 * The base implementation registers the player with the online session so that
 * recent players list and session counts are updated.
 */
simulated function RegisterPlayerWithSession()
{
	local OnlineSubsystem Online;
	local OnlineRecentPlayersList PlayersList;

	Online = class'GameEngine'.static.GetOnlineSubsystem();
	if (Online != None &&
		Online.GameInterface != None &&
		SessionName != 'None' &&
		Online.GameInterface.GetGameSettings(SessionName) != None)
	{
		// Register the player as part of the session
		Online.GameInterface.RegisterPlayer(SessionName,UniqueId,false);
		// If this is not us, then add the player to the recent players list
		if (!bNetOwner)
		{
			PlayersList = OnlineRecentPlayersList(Online.GetNamedInterface('RecentPlayersList'));
			if (PlayersList != None)
			{
				PlayersList.AddPlayerToRecentPlayers(UniqueId);
			}
		}
	}
}

/**
 * The base implementation unregisters the player with the online session so that
 * session counts are updated.
 */
simulated function UnregisterPlayerFromSession()
{
	local OnlineSubsystem OnlineSub;
	local UniqueNetId ZeroId;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	// If there is a game and we are a client, unregister this remote player
	if (SessionName != 'None' &&
		WorldInfo.NetMode == NM_Client &&
		OnlineSub != None &&
		OnlineSub.GameInterface != None &&
		OnlineSub.GameInterface.GetGameSettings(SessionName) != None &&
		UniqueId != ZeroId)
	{
		// Remove the player from the session
		OnlineSub.GameInterface.UnregisterPlayer(SessionName,UniqueId);
	}
}

defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=True
	GameMessageClass=class'GameMessage'
	SplitscreenIndex=INDEX_NONE

	bControllerVibrationAllowed=TRUE

	// The default online session is the game one
	SessionName="Game"
	PlayerSkill=-1
}
