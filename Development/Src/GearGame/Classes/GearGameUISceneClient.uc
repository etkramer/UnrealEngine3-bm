/**
 * Gears2 specific version of the GameUISceneClient
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearGameUISceneClient extends GameUISceneClient
	within GearUIInteraction;


var	private	transient	int		OutstandingAsyncTasks;

/** Reference to the scene we want to open */
var GearUIMessageBox GearMessageBox;

/** Hard reference to the Updating scene used for blocking input */
var GearUISceneFE_Updating UpdatingSceneResource;

/**
 * Utility function for creating an instance of a message box.  The scene must still be activated by calling OpenScene.
 *
 * This version just loads the scene resource from a content package, rather than creating it.
 *
 * @param	SceneTag				the name to use for the message box scene.  useful for e.g. preventing multiple copies of the same message from appearing.
 * @param	CustomMessageBoxClass	allows callers to override the default message box class for custom message types.
 * @param	SceneTemplate			optional scene resource to use as the template for the new message box instance.
 *
 * @return	an instance of the message box class specified.
 */
static function UIMessageBoxBase CreateUIMessageBox( name SceneTag, optional class<UIMessageBoxBase> CustomMessageBoxClass=default.MessageBoxClass, optional UIMessageBoxBase SceneTemplate )
{
	local GearGameUISceneClient GameSceneClient;

	if ( SceneTemplate == None )
	{
		GameSceneClient = GearGameUISceneClient(class'UIRoot'.static.GetSceneClient());
		if ( GameSceneClient != None )
		{
			SceneTemplate = GameSceneClient.GearMessageBox;
		}
	}

	return Super.CreateUIMessageBox(SceneTag, CustomMessageBoxClass, SceneTemplate);
}

/**
 * Opens a generic scene that blocks input until StopUpdate is called and the scene closes
 *
 * @param	TitleString			optional string to use for the title label in the updating scene
 * @param	MessageString		optional string to use for the message label in the updating scene
 * @param	MinDisplayTime		specifies the minimum amount of time to display the updating scene
 * @param	UpdatingSceneTag	if specified, uses this as the value for the scene's SceneTag
 * @param	ForcedPriority		overrides the scene's SceneStackPriority value to allow callers to modify where the scene is placed
 *								in the stack, by default.
 *
 * @return	a reference to an instance of the scene that displays the spinning cog wheel and blocks input.
 */
static function GearUISceneFE_Updating OpenUpdatingScene(
				optional string TitleString="UpdatingTitle",
				optional string MessageString,
				optional float MinDisplayTime=1.f,
				optional name UpdatingSceneTag='Updating',
				optional byte ForcedPriority )
{
	local GearGameUISceneClient GameSceneClient;
	local UIScene SceneInstance;
	local GearUISceneFE_Updating Result;

	GameSceneClient = GearGameUISceneClient(class'UIRoot'.static.GetSceneClient());
	if ( GameSceneClient != None )
	{
		// If there is already an updating scene, just restart the timer
		SceneInstance = GameSceneClient.FindSceneByTag(UpdatingSceneTag);
		if ( SceneInstance != None )
		{
			Result = GearUISceneFE_Updating(SceneInstance);
			if ( MessageString != "" )
			{
				Result.InitializeUpdatingScene(TitleString, MessageString, MinDisplayTime);
			}

			Result.RestartUpdate();
		}
		// Open a new instance
		else if ( GameSceneClient.UpdatingSceneResource != None )
		{
			SceneInstance = GameSceneClient.UpdatingSceneResource.OpenScene(GameSceneClient.UpdatingSceneResource,,ForcedPriority);
			if ( SceneInstance != None )
			{
				SceneInstance.SceneTag = UpdatingSceneTag;
				Result = GearUISceneFE_Updating(SceneInstance);

				if ( MessageString != "" )
				{
					Result.InitializeUpdatingScene(TitleString, MessageString, MinDisplayTime);
				}
			}
		}

		// don't block input in the game
		if ( Result != None && !class'WorldInfo'.static.IsMenuLevel() )
		{
			Result.SetSceneInputMode(INPUTMODE_None);
		}
	}

	return Result;
}

/** Closes the generic scene that blocks input from OpenUpdatingScene */
static function CloseUpdatingScene( optional name UpdatingSceneTag='Updating' )
{
	local GameUISceneClient GameSceneClient;
	local GearUISceneFE_Updating ExistingScene;

	GameSceneClient = class'UIRoot'.static.GetSceneClient();
	if (GameSceneClient != None)
	{
		ExistingScene = GearUISceneFE_Updating(GameSceneClient.FindSceneByTag(UpdatingSceneTag));
		if ( ExistingScene != None )
		{
			ExistingScene.StopUpdate();
		}
	}
}

final function BeginAsyncTask()
{
	OutstandingAsyncTasks++;
}

final function bool EndAsyncTask()
{
	local bool bResult;

	bResult = true;
	if ( OutstandingAsyncTasks > 0 )
	{
		if ( --OutstandingAsyncTasks > 0 )
		{
			bResult = false;
		}
	}

	return bResult;
}

final function bool HasOutstandingAsyncTasks()
{
	return OutstandingAsyncTasks > 0;
}

/**
 * Handler for the OnOptionSelected delegate of the "too many profiles signed-in" error message.  Only allows the message box to be closed
 * if the number of signed in players is <= MAX_SPLITSCREEN_PLAYERS.
 *
 * @param	Sender				the message box that generated this call
 * @param	SelectedInputAlias	the alias of the button that the user selected.  Should match one of the aliases passed into
 *								this message box.
 * @param	PlayerIndex			the index of the player that selected the option.
 *
 * @return	TRUE to indicate that the message box should close itself.
 */
function bool OnTooManyProfilesErrorAccepted( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( class'GearUIScene_Base'.static.GetLoggedInPlayerCount() <= `MAX_SPLITSCREEN_PLAYERS )
	{
		return true;
	}

	PlayUISound('GenericError');
	return false;
}

/**
 * Synchronizes the number of local players to the number of signed-in controllers.  Games should override this method if more complex
 * logic is desired (for example, if inactive gamepads [gamepads that are signed in but not associated with a player] are allowed).
 *
 * @param	MaxPlayersAllowed			specifies the total maximum number of players that can be created; once this limit is reached, no more players
 *										will be created.  Should be set to the maximum number of split-screen players supported by the game.
 * @param	bAllowJoins					controls whether creating new player is allowed; if true, a player will be created for any gamepad
 *										that is signed into a profile and not already associated with a player (up to MaxPlayersAllowed)
 * @param	bAllowRemoval				controls whether removing players is allowed; if true, any existing players that are not signed into
 *										a profile will be removed.
 *
 * @note: if both bAllowJoins and bAllowRemoval are FALSE, only effect will be removal of extra players (i.e. player count higher than
 *	MaxPlayersAllowed)
 */
event SynchronizePlayers( optional int MaxPlayersAllowed=`MAX_SPLITSCREEN_PLAYERS, optional bool bAllowJoins=true, optional bool bAllowRemoval=true )
{
	local int Idx, PlayerIndex, NumActiveProfiles, NumPlayers, ControllerId;
	local array<int> SignedInControllerIds, LiveEnabledProfileControllerIds;
	local LocalPlayer LP;
	local string ErrorString;

	if ( IsAllowedToModifyPlayerCount() )
	{
		class'GearUIScene_Base'.static.GetLoggedInControllerIds(SignedInControllerIds);
		class'GearUIScene_Base'.static.GetLoggedInControllerIds(LiveEnabledProfileControllerIds, true);
		`assert(LiveEnabledProfileControllerIds.Length >= SignedInControllerIds.Length);

		// get the number of signed-in players
		NumActiveProfiles = SignedInControllerIds.Length;
		NumPlayers = GetPlayerCount();

		// if there are too many players signed in, show an error message
		if ( NumActiveProfiles > MaxPlayersAllowed )
		{
			// show an error message
			class'GearUIScene_Base'.static.DisplayErrorMessage("TwoPlayersOnly_Message", "TwoPlayersOnly_Title", "GenericAccept", None,OnTooManyProfilesErrorAccepted,,100);
			return;
		}

		if ( bAllowJoins )
		{
			// for each signed in profile, make sure they have a player (creating one if necessary)
			for ( Idx = 0; Idx < NumActiveProfiles; Idx++ )
			{
				ControllerId = SignedInControllerIds[Idx];
				PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(ControllerId);

				// if no player has this controllerId, create one.
				if ( PlayerIndex == INDEX_NONE )
				{
					`Assert(NumPlayers < MaxPlayersAllowed);
					`log(`location @ "attempting to create a new local player -" @ `showvar(ControllerId));
					LP = CreatePlayer(ControllerId, ErrorString, true);
				}
			}
		}

		if ( bAllowRemoval )
		{
			// for each player, if they aren't signed in, remove them unless they're the last player
			NumPlayers = GetPlayerCount();
			for ( PlayerIndex = NumPlayers - 1; PlayerIndex >= 0 && NumPlayers >  1; PlayerIndex-- )
			{
				ControllerId = GetPlayerControllerId(PlayerIndex);
				`assert(ControllerId>=0);
				`assert(ControllerId<MAX_SUPPORTED_GAMEPADS);

				// if not in the list of signed-in gamepads, remove this player
				Idx = SignedInControllerIds.Find(ControllerId);
				if ( Idx == INDEX_NONE )
				{
					LP = GamePlayers[PlayerIndex];
					if ( LP != None && RemovePlayer(LP) )
					{
						NumPlayers--;
					}
				}
			}
		}
	}
}

/**
 * Called when a gamepad (or other controller) is inserted or removed)
 *
 * @param	ControllerId	the id of the gamepad that was added/removed.
 * @param	bConnected		indicates the new status of the gamepad.
 */
function NotifyControllerChanged( int ControllerId, bool bConnected )
{
	local int PlayerIndex;
	local array<name> ButtonAliases;
	local GearUIMessageBox ReconnectMB;
	local WorldInfo WI;
	local GearGameMP_Base MPGame;
	local LocalPlayer ControllingPlayer;
	local GearPC OwnerPC;
	local bool bGameIsPauseable;
	local string ActiveMovieName;

	// we want the currently active scene to call UpdateScenePlayerState so execute the base version of this function either
	// before or after opening the message box, depending on the value of bConnected
	if ( !bConnected )
	{
		Super.NotifyControllerChanged(ControllerId, bConnected);
	}

	PlayerIndex = GetPlayerIndex(ControllerId);
	if ( PlayerIndex != INDEX_NONE )
	{
		ControllingPlayer = GetLocalPlayer(PlayerIndex);
		OwnerPC = GearPC(ControllingPlayer.Actor);

		WI = class'UIScene'.static.GetWorldInfo();
		if ( bConnected )
		{
			// only require the user to manually dismiss the prompt if the game is paused - otherwise we're just losing valuable time!  :)
			if ( class'WorldInfo'.static.IsMenuLevel() || (WI != None && WI.Pauser == None) )
			{
				OwnerPC.GetCurrentMovie(ActiveMovieName);

				if ( ActiveMovieName ~= LOADING_MOVIE )
				{
					ReconnectMB = GearUIMessageBox(FindSceneByTag('ReconnectController', ControllingPlayer));
					if ( ReconnectMB != None && ReconnectMB.TickProxy != None )
					{
						// we'll be pausing the game when the loading screen is closed, so leave the message box up
						return;
					}
				}

				ClearUIMessageScene('ReconnectController', ControllingPlayer);
			}
			else
			{
				// wait until user actually presses A
			}
		}
		else
		{
			ButtonAliases.AddItem('GenericContinue');

			ReconnectMB = GearUIMessageBox(CreateUIMessageBox('ReconnectController'));
			if ( ReconnectMB != None )
			{
				bGameIsPauseable = WI.Game != None && WI.Game.bPauseable && OwnerPC.AllowPauseForControllerRemovalHack();
				if ( bGameIsPauseable )
				{
					MPGame = GearGameMP_Base(WI.Game);
					if ( MPGame != None )
					{
						bGameIsPauseable = MPGame.AllowNetworkPause();
					}
				}

				ReconnectMB.bFlushPlayerInput = true;
				ReconnectMB.bPauseGameWhileActive = bGameIsPauseable;

				ReconnectMB.bCloseOnLevelChange = false;
				ReconnectMB.bExemptFromAutoClose = true;
				ReconnectMB.bMenuLevelRestoresScene = true;

				ReconnectMB.bSaveSceneValuesOnClose = false;

				// it's ok to allow the user to use B to close the menu
				ReconnectMB.EventProvider.DisabledEventAliases.RemoveItem('CloseScene');

				// if we're not able to pause the game, don't cover the other player's viewport
				if ( !bGameIsPauseable )
				{
					ReconnectMB.bCaptureMatchedInput = true;
					ReconnectMB.SetSceneInputMode(INPUTMODE_MatchingOnly);
					ReconnectMB.SetSceneRenderMode(SPLITRENDER_PlayerOwner);
				}
				else if ( OwnerPC != None && OwnerPC.AllowPauseForControllerRemovalHack() )
				{
					OwnerPC.GetCurrentMovie(ActiveMovieName);
					if ( ActiveMovieName ~= LOADING_MOVIE )
					{
						ReconnectMB.PauseOnLoadingMovieComplete();
					}
				}

				// setup the messagebox scene the way we like it!
				ReconnectMB = GearUIMessageBox(ReconnectMB.OpenScene(ReconnectMB, ControllingPlayer, 500));
				ReconnectMB.SetupMessageBox(
					class'GearUIScene_Base'.static.GetControllerIconString(ControllerId, true) @ "<Strings:GearGameUI.MessageBoxErrorStrings.ReconnectController_Title>",
					"<Strings:GearGameUI.MessageBoxErrorStrings.ReconnectController_Message>",
					"", ButtonAliases
				);
			}
		}
	}

	if ( bConnected )
	{
		Super.NotifyControllerChanged(ControllerId, bConnected);
	}
}

exec function RunAnimation( name AnimationName )
{
	local GearUIScene_Base ActiveScene;

	ActiveScene = GearUIScene_Base(GetActiveScene(None, true));
	if ( ActiveScene != None )
	{
		ActiveScene.PlayUIAnimation(AnimationName);
	}
}


/**
 * Called when the local player is about to travel to a new URL.  This callback should be used to perform any preparation
 * tasks, such as updating status text and such.  All cleanup should be done from NotifyGameSessionEnded, as that function
 * will be called in some cases where NotifyClientTravel is not.
 *
 * @param	TravellingPlayer	the player that received the call to ClientTravel
 * @param	TravelURL			a string containing the mapname (or IP address) to travel to, along with option key/value pairs
 * @param	TravelType			indicates whether the player will clear previously added URL options or not.
 * @param	bIsSeamlessTravel	indicates whether seamless travelling will be used.
 */
function NotifyClientTravel( PlayerController TravellingPlayer, string TravelURL, ETravelType TravelType, bool bIsSeamlessTravel )
{
	local GearPC PC;

	PC = GearPC(TravellingPlayer);
	if ( PC != None && !bIsSeamlessTravel )
	{
		PC.ClientShowLoadingMovie(true);
	}

	Super.NotifyClientTravel(TravellingPlayer, TravelURL, TravelType, bIsSeamlessTravel);
}

function NotifyLoginStatusChanged( int ControllerId, ELoginStatus NewStatus )
{
	local GearEngine GE;
	local int PlayerIndex;

	PlayerIndex = GetPlayerIndex(ControllerId);

	// if the primary player was signed out, invalidate the cached device id
	if ( PlayerIndex == 0 && NewStatus == LS_NotLoggedIn )
	{
		GE = GearEngine(Outer.Outer.Outer);
		`log(`location @ `showvar(GE.GetCurrentDeviceID()));

		GE.SetCurrentDeviceID(INDEX_NONE, true);
		SetDataStoreStringValue("<Registry:InitialProfileCheck>", "0");
	}

	Super.NotifyLoginStatusChanged(ControllerId, NewStatus);
}

/**
 * Called when a new player has been added to the list of active players (i.e. split-screen join)
 *
 * @param	PlayerIndex		the index [into the GamePlayers array] where the player was inserted
 * @param	AddedPlayer		the player that was added
 */
function NotifyPlayerAdded( int PlayerIndex, LocalPlayer AddedPlayer )
{
	Super.NotifyPlayerAdded(PlayerIndex, AddedPlayer);

	RefreshPlayerDeadZone();
}

/**
 * Called when a player has been removed from the list of active players (i.e. split-screen players)
 *
 * @param	PlayerIndex		the index [into the GamePlayers array] where the player was located
 * @param	RemovedPlayer	the player that was removed
 */
function NotifyPlayerRemoved( int PlayerIndex, LocalPlayer RemovedPlayer )
{
	local LocalPlayer PrimaryPlayer;
	local GearPC PCOwner;

	Super.NotifyPlayerRemoved( PlayerIndex, RemovedPlayer);

	if ( PlayerIndex == `PRIMARY_PLAYER_INDEX )
	{
		PrimaryPlayer = GetLocalPlayer(`PRIMARY_PLAYER_INDEX);
		if ( PrimaryPlayer != None && PrimaryPlayer != RemovedPlayer )
		{
			// a new player is the primary player so tell them to reload their profile settings as there are some global settings that
			// are only taken from the primary player's profile.
			PCOwner = GearPC(PrimaryPlayer.Actor);
			PCOwner.bProfileSettingsUpdated = true;
		}
	}

	RefreshPlayerDeadZone();
}

/**
 * Called when a storage device is inserted or removed.
 */
function NotifyStorageDeviceChanged()
{
	local GearEngine GE;

	GE = GearEngine(Outer.Outer.Outer);
	`log(`location @ `showvar(GE.GetCurrentDeviceID()));

	if ( !GE.IsCurrentDeviceValid() )
	{
		SetDataStoreStringValue("<Registry:InitialProfileCheck>", "0");

		GE.SetCurrentDeviceID(INDEX_NONE);
	}

	Super.NotifyStorageDeviceChanged();
}

/**
*  Called when local player has been added or removed
*  Refresh DeadZone for all players
*/
function RefreshPlayerDeadZone()
{
	local int Idx, NumPlayers;
	local GearPC PC;

	bUpdateSceneViewportSizes = true;

	NumPlayers = GetPlayerCount();
	for ( Idx = 0; Idx < NumPlayers; Idx++ )
	{
		PC = GearPC(GetLocalPlayer(Idx).Actor);
		if ( PC != None && PC.MyGearHUD != None )
		{
			PC.MyGearHUD.bRefreshSafeZone = true;
		}
	}
}

DefaultProperties
{
	// UI post-processing has been cut, for now
	bEnablePostProcess=false

	GearMessageBox=GearUIMessageBox'UI_Scenes_Common.UI_MessageBox'
	MessageBoxClass=class'GearGame.GearUIMessageBox'

	UpdatingSceneResource=GearUISceneFE_Updating'UI_Scenes_Common.Updating'

//	Begin Object Class=UIAnimationSeq Name=ForegroundPP_FadeInSeq
//		SeqName=ForegroundPP_FadeInSeq
//		Tracks(0)=(TrackType=EAT_PPBlurAmount,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.3,Data=(DestAsFloat=1.0))))
//	End Object
//	AnimSequencePool.Add(ForegroundPP_FadeInSeq)
//
//	Begin Object Class=UIAnimationSeq Name=ForegroundPP_FadeOutSeq
//		SeqName=ForegroundPP_FadeOutSeq
//		Tracks(0)=(TrackType=EAT_PPBlurAmount,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=1.0)),(RemainingTime=0.3,Data=(DestAsFloat=0.0))))
//	End Object
//	AnimSequencePool.Add(ForegroundPP_FadeOutSeq)


	/** For the first time the scene is open. */
	Begin Object Class=UIAnimationSeq Name=ActivateSceneSeq
		SeqName=ActivateSceneSeq
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.2,Data=(DestAsFloat=1.0))))
		//Tracks(1)=(TrackType=EAT_PPBlurAmount,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.3,Data=(DestAsFloat=1.0))))
	End Object
	AnimSequencePool.Add(ActivateSceneSeq)

	Begin Object Class=UIAnimationSeq Name=DeactivateSceneSeq
		SeqName=DeactivateSceneSeq
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.2,Data=(DestAsFloat=0.0))))
		//Tracks(1)=(TrackType=EAT_PPBlurAmount,KeyFrames=((RemainingTime=0.2,Data=(DestAsFloat=0.0))))
	End Object
	AnimSequencePool.Add(DeactivateSceneSeq)

	Begin Object Class=UIAnimationSeq Name=ReactivateSceneSeq
		SeqName=ReactivateSceneSeq
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.2,Data=(DestAsFloat=1.0))))
	End Object
	AnimSequencePool.Add(ReactivateSceneSeq)

	Begin Object Class=UIAnimationSeq Name=SceneLostFocusSeq
		SeqName=SceneLostFocusSeq
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.2,Data=(DestAsFloat=0.0))))
	End Object
	AnimSequencePool.Add(SceneLostFocusSeq)

//	Begin Object Class=UIAnimationSeq Name=ReactivateSceneSeq
//		SeqName=ReactivateSceneSeq
//		Tracks(0)=(TrackType=EAT_PPBlurAmount,KeyFrames=((RemainingTime=0.225,Data=(DestAsFloat=1.0))))
//	End Object
//	AnimSequencePool.Add(ReactivateSceneSeq)

	Begin Object Class=UIAnimationSeq Name=HintTextFadeIn_Template
		SeqName=HintTextFadeIn
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.5,Data=(DestAsFloat=1.0))))
	End Object
	AnimSequencePool.Add(HintTextFadeIn_Template)

	Begin Object Class=UIAnimationSeq Name=HintTextFadeOut_Template
		SeqName=HintTextFadeOut
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.3,Data=(DestAsFloat=0.0))))
	End Object
	AnimSequencePool.Add(HintTextFadeOut_Template)

	Begin Object Class=UIAnimationSeq Name=HintTextCrossFadeOut_Template
		SeqName=HintTextCrossFadeOut
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.3,Data=(DestAsFloat=0.0))))
	End Object
	AnimSequencePool.Add(HintTextCrossFadeOut_Template)

	Begin Object Class=UIAnimationSeq Name=HintTextCrossFadeIn_Template
		SeqName=HintTextCrossFadeIn
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.3,Data=(DestAsFloat=1.0))))
	End Object
	AnimSequencePool.Add(HintTextCrossFadeIn_Template)

}
