/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 *  Our UIScenes provide PreRender and tick passes to our Widgets
 */

class UTUIScene extends UIScene
	abstract
	native(UI)
	dependson(OnlinePlayerInterface,UTGameInteraction);

`include(UTOnlineConstants.uci)

/** Possible factions for the bots. */
enum EUTBotTeam
{
	UTBotTeam_Random,
	UTBotTeam_Ironguard,
	UTBotTeam_TwinSouls,
	UTBotTeam_Krall,
	UTBotTeam_Liandri,
	UTBotTeam_Necris
};

/** Demo recording. */
enum EUTRecordDemo
{
	UTRecordDemo_No,
	UTRecordDemo_Yes,
};

var(Editor) transient bool bEditorRealTimePreview;

// hack to prevent the in-game command/taunt menu from causing the player's motion to stutter on consoles..
// @todo ronp - real fix is to toggle whether the UI controller performs axis repeat delays by looking at whether the current scene
// handles axis input, but this requires fixing all the scenes that are currently hardwired into the ProcessRawInputKey delegate.
var(Flags)	bool	bIgnoreAxisInput;

/** Global scene references, only scenes that are used in-game and in-menus should be referenced here. */
var transient UIScene MessageBoxScene;
var transient UIScene InputBoxScene;
var transient UIScene PlayerCardScene;
var transient UIScene OnlineToastScene;

/** Pending scene to open since we are waiting for the current scene's exit animation to end. */
var transient UIScene PendingOpenScene;

/** Pending scene to close since we are waiting for the current scene's exit animation to end. */
var transient UIScene PendingCloseScene;

/** Animation flags, used by the tick function to determine which update func to call. */
var transient bool bShowingScene;
var transient bool bHidingScene;

/** Whether or not to skip the kismet notify for the close scene that is pending. */
var transient bool bSkipPendingCloseSceneNotify;

/** Callback for when the scene's show animation has ended. */
delegate OnShowAnimationEnded();

/** Callback for when the scene's hide animation has ended. */
delegate OnHideAnimationEnded();

/** Callback for when a scene has opened after hiding the topmost scene. */
delegate OnSceneOpened(UIScene OpenedScene, bool bInitialActivation);

cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );
	virtual void Tick( FLOAT DeltaTime );
	virtual void TickChildren(UUIScreenObject* ParentObject, FLOAT DeltaTime);
	virtual void PreRender(FCanvas* Canvas);

	virtual UBOOL PreChildrenInputKey(INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed=1.f,UBOOL bGamepad=FALSE) { return false; }

	static void AutoPlaceChildren(UUIScreenObject *const BaseObject);

	/**
	 * Appends any command-line switches that should be carried over to the new process when starting a dedicated server instance.
	 */
	static void AppendPersistentSwitches( FString& ExtraSwitches );
}

/**
 * Sets the screen resolution.
 *
 * @param ResX			Width of the screen
 * @param ResY			Height of the screen
 * @param bFullscreen	Whether or not we are fullscreen.
 */
native function SetScreenResolution(int ResX, int ResY, bool bFullscreen);

/**
 * Get the UTPlayerController that is associated with this Hud
 */
native function UTPlayerController GetUTPlayerOwner(optional int PlayerIndex=-1);

/**
 * Returns the Pawn associated with this Hud
 */
native function Pawn GetPawnOwner();

/**
 * Returns the PRI associated with this hud
 */
native function UTPlayerReplicationInfo GetPRIOwner();

/**
 * @Returns the contents of GIsGame
 */
native function bool IsGame();

/** Starts a dedicated server and kills the current process. */
native function StartDedicatedServer(string TravelURL);

/** Retrieves all of the possible screen resolutions from the display driver. */
native function GetPossibleScreenResolutions(out array<string> OutResults);

/** Retrieves all of the possible audio devices from the audio driver. */
native function GetPossibleAudioDevices(out array<string> OutResults);

/** @return Returns the currently selected audio device. */
native function string GetCurrentAudioDevice();

/**
 * @return	TRUE if the user's machine is below the minimum required specs to play the game.
 */
native final function bool IsBelowMinSpecs() const;

/**
 * Sets the audio device to use for playback.
 *
 * @param InAudioDevice		Audio device to use.
 */
native function SetAudioDeviceToUse(string InAudioDevice);

/**
 * Tries to unlock a character using a code.
 *
 * @param UnlockCode	Code to use to unlock the character.
 *
 * @return TRUE if the unlock succeeded, FALSE otherwise.
 */
native function bool TryCharacterUnlock(string UnlockCode);

/** @return Return a reference to the UT specific version of the UI interaction. */
function UTGameInteraction GetUTInteraction()
{
	return UTGameInteraction(GetCurrentUIController());
}

/**
 * Returns whether or not the input passed in is a gamepad input event.
 *
 * @param KeyName	Key name to check
 *
 * @return Returns TRUE if the input key is from a gamepad, FALSE otherwise.
 */
event bool IsControllerInput(name KeyName)
{
	local bool bResult;
	local array<name> Keys;
	local int KeyIdx;

	bResult=false;

	Keys.length=24;

	Keys[0]='XboxTypeS_LeftThumbstick';
	Keys[1]='XboxTypeS_RightThumbstick';
	Keys[2]='XboxTypeS_Back';
	Keys[3]='XboxTypeS_Start';
	Keys[4]='XboxTypeS_A';
	Keys[5]='XboxTypeS_B';
	Keys[6]='XboxTypeS_X';
	Keys[7]='XboxTypeS_Y';
	Keys[8]='XboxTypeS_LeftShoulder';
	Keys[9]='XboxTypeS_RightShoulder';
	Keys[10]='XboxTypeS_LeftTrigger';
	Keys[11]='XboxTypeS_RightTrigger';
	Keys[12]='XboxTypeS_DPad_Up';
	Keys[13]='XboxTypeS_DPad_Down';
	Keys[14]='XboxTypeS_DPad_Right';
	Keys[15]='XboxTypeS_DPad_Left';
	Keys[16]='Gamepad_LeftStick_Up';
	Keys[17]='Gamepad_LeftStick_Down';
	Keys[18]='Gamepad_LeftStick_Right';
	Keys[19]='Gamepad_LeftStick_Left';
	Keys[20]='Gamepad_RightStick_Up';
	Keys[21]='Gamepad_RightStick_Down';
	Keys[22]='Gamepad_RightStick_Right';
	Keys[23]='Gamepad_RightStick_Left';

	for(KeyIdx=0; KeyIdx<Keys.length; KeyIdx++)
	{
		if(KeyName==Keys[KeyIdx])
		{
			bResult=true;
			break;
		}
	}

	return bResult;
}

/** Trims whitespace from the beginning and end of a string. */
static function string TrimWhitespace(string InString)
{
	local int StartIdx;
	local int EndIdx;
	local string FinalString;

	for(StartIdx=0; StartIdx<Len(InString); StartIdx++)
	{
		if(Asc(Mid(InString,StartIdx,1))!=32)
		{
			break;
		}
	}

	for(EndIdx=Len(InString)-1; EndIdx>=0; EndIdx--)
	{
		if(Asc(Mid(InString,EndIdx,1))!=32)
		{
			break;
		}
	}

	if(StartIdx<=EndIdx)
	{
		FinalString=Mid(InString,StartIdx,EndIdx-StartIdx+1);
	}

	return FinalString;
}

/**
 * Executes a console command.
 *
 * @param string Cmd	Command to execute.
 */
final function ConsoleCommand(string Cmd, optional bool bWriteToLog)
{
	if ( Cmd != "" && PlayerOwner != None )
	{
		if ( PlayerOwner.Actor != None )
		{
			PlayerOwner.Actor.ConsoleCommand(Cmd, bWriteToLog);
		}
		else if(PlayerOwner.ViewportClient != None
			&&	PlayerOwner.ViewportClient.UIController != None
			&&	PlayerOwner.ViewportClient.UIController.ViewportConsole != None)
		{
			PlayerOwner.ViewportClient.UIController.ViewportConsole.ConsoleCommand(Cmd);
		}
	}
}


/** @return Returns the player index of the player owner for this scene. */
function int GetPlayerIndex()
{
	local int PlayerIndex;
	local LocalPlayer LP;

	LP = GetPlayerOwner();
	if ( LP != None )
	{
		PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(LP.ControllerId);
	}
	else
	{
		PlayerIndex = GetBestPlayerIndex();
	}

	return PlayerIndex;
}

/**
 * Opens a UI Scene given a reference to a scene to open.
 *
 * @param SceneToOpen	Scene that we want to open.
 */
function UIScene OpenSceneByName(string SceneToOpen, bool bSkipAnimation=false, optional delegate<OnSceneActivated> SceneDelegate=None)
{
	local UIScene SceneToOpenReference;
	SceneToOpenReference = UIScene(DynamicLoadObject(SceneToOpen, class'UIScene'));

	if(SceneToOpenReference != None)
	{
		return OpenScene(SceneToOpenReference,/*LocalPlayer*/,/*ForcedPriority*/,bSkipAnimation,SceneDelegate);
	}
	else
	{
		return None;
	}
}

/**
 * Opens a UI Scene given a reference to a scene to open.
 *
 * @param	SceneToOpen		Scene that we want to open.
 * @param	bSkipAnimation	specify TRUE to indicate that opening animations should be bypassed.
 * @param	SceneDelegate	if specified, will be called when the scene has finished opening.
 */
function UIScene OpenScene(UIScene SceneToOpen, optional LocalPlayer ScenePlayerOwner=GetPlayerOwner(), optional byte UnusedForcedPriority, optional bool bSkipAnimation=false, optional delegate<OnSceneActivated> SceneDelegate=None)
{
	local UIScene Result;
	local UTUIScene CurrentScene;
	local bool bOpenNow;


	// Try to hide the current scene.
	bOpenNow = true;
	OnSceneOpened = SceneDelegate;

	// only hide the current scene if the scene we're about to open doesn't render its parent scenes
	CurrentScene = UTUIScene(GetSceneClient().GetActiveScene(ScenePlayerOwner, true));
	if ( !bSkipAnimation && SceneToOpen != None && (!SceneToOpen.bRenderParentScenes || CurrentScene.bAlwaysRenderScene) )
	{
		if(CurrentScene != None && !CurrentScene.bRenderParentScenes )
		{
			CurrentScene.OnHideAnimationEnded = OnCurrentScene_HideAnimationEnded;
			if(CurrentScene.BeginHideAnimation(false))
			{
				// Block input while animating
				GetUTInteraction().BlockUIInput(true);

				bHidingScene = true;
				PendingOpenScene = SceneToOpen;
				bOpenNow = false;
			}
			else
			{
				CurrentScene.OnHideAnimationEnded = None;
			}
		}
	}

	// If no hide animation was started, just show the scene we are opening.
	if ( bOpenNow )
	{
		Result = FinishOpenScene(SceneToOpen, bSkipAnimation, true);
	}

	return Result;
}

/** Callback for when the current scene's hide animation has completed. */
function OnCurrentScene_HideAnimationEnded()
{
	FinishOpenScene(PendingOpenScene);
}

/**
 * Finishes opening a scene, usually called when a hide animation has ended.
 *
 * @param	SceneToOpen			the scene to open
 * @param	bSkipAnimation		specify TRUE to bypass the scene's opening animation
 * @param	bSkipKismetNotify	specify TRUE to prevent the 'OpeningMenu' level event from being activated.
 *
 * @return	reference to the UIScene instance that was opened.
 */
//@todo ronp animation - bSkipKismetNotify!
function UIScene FinishOpenScene(UIScene SceneToOpen, bool bSkipAnimation=false, bool bSkipKismetNotify=false)
{
	local UIScene OpenedScene;
	local UTUIScene OpenedUTScene;
	local GameUISceneClient GameSceneClient;
	local UTUIScene UTSceneToOpen;

	// Reenable input
	GetUTInteraction().BlockUIInput(false);

	// Clear any references and delegates set
	UTSceneToOpen = UTUIScene(SceneToOpen);

	if(UTSceneToOpen != None)
	{
		UTSceneToOpen.OnHideAnimationEnded = None;
		UTSceneToOpen.OnShowAnimationEnded = None;
	}
	PendingOpenScene = None;
	OnHideAnimationEnded = None;

	OpenedScene = none;

	// Get the UI Controller and try to open the scene.
	GameSceneClient = GetSceneClient();
	if ( SceneToOpen != none && GameSceneClient != none  )
	{
		// Have the UI system look to see if the scene exists.  If it does
		// use that so that split-screen shares scenes
		OpenedScene = GameSceneClient.FindSceneByTag(SceneToOpen.SceneTag, PlayerOwner);
		if (OpenedScene == none)
		{
			// Nothing, just create a new instance
			GameSceneClient.InitializeScene(SceneToOpen, PlayerOwner, OpenedScene);
			if ( OpenedScene != None )
			{
				if ( OnSceneOpened != None )
				{
					OpenedScene.OnSceneActivated = OnSceneOpened;
				}

				GameSceneClient.OpenScene(OpenedScene, PlayerOwner, OpenedScene);
			}

			OpenedUTScene = UTUIScene(OpenedScene);
			if(OpenedUTScene != None)
			{
				if(OpenedUTScene.BeginShowAnimation(true,bSkipAnimation))
				{
					OpenedUTScene.bShowingScene=true;
				}
			}
		}
	}

	// Activate kismet for opening scene
	if ( !bSkipKismetNotify )
	{
		ActivateLevelEvent('OpeningMenu');
	}

	// Clear scene open delegate
	OnSceneOpened = None;

	return OpenedScene;
}

/** Opens a scene without any special hiding animation for previous scenes. */
static function UIScene StaticOpenScene(UIScene SceneToOpen)
{
	local GameUISceneClient GameSceneClient;
	local UIScene OpenedScene;
	local UTUIScene OpenedUTScene;

	// Get the UI Controller and try to open the scene.
	GameSceneClient = GetSceneClient();
	if ( SceneToOpen != none && GameSceneClient != none  )
	{
		// Have the UI system look to see if the scene exists.  If it does
		// use that so that split-screen shares scenes
		OpenedScene = GameSceneClient.FindSceneByTag( SceneToOpen.SceneTag );
		if (OpenedScene == none)
		{
			// Nothing, just create a new instance
			GameSceneClient.OpenScene(SceneToOpen, None, OpenedScene);

			OpenedUTScene = UTUIScene(OpenedScene);
			if(OpenedUTScene != None)
			{
				if(OpenedUTScene.BeginShowAnimation())
				{
					OpenedUTScene.bShowingScene=true;
				}
			}
		}
	}

	return OpenedUTScene;
}

/**
 * Closes a UI Scene given a reference to an previously open scene.
 *
 * @param SceneToClose			Scene that we want to close.
 * @param bSkipKismetNotify		Whether or not to close the kismet notify for the scene.
 * @param bSkipAnimation		Whether or not to skip the close animation for this scene.
 */
//function bool CloseScene( optional UIScene SceneToClose=Self, bool bSkipKismetNotify=false, bool bForceCloseImmediately=false)
function bool CloseScene( optional UIScene SceneToClose=Self, bool bCloseChildScenes=true, bool bForceCloseImmediately=false )
{
	local UTUIScene UTSceneToClose;
	local bool bResult;

	UTSceneToClose = UTUIScene(SceneToClose);
	if ( UTSceneToClose.IsSceneActive() )
	{
		if(UTSceneToClose != None && !bForceCloseImmediately && UTSceneToClose.BeginHideAnimation(true))
		{
			// Block input while animating
			GetUTInteraction().BlockUIInput(true);

			UTSceneToClose.bHidingScene = true;
			UTSceneToClose.OnHideAnimationEnded = OnPendingCloseScene_HideAnimationEnded;
			PendingCloseScene = UTSceneToClose;
			//@fixme ronp - what was this being used for?
			//bSkipPendingCloseSceneNotify = bSkipKismetNotify;
			bResult = false;
		}
		else
		{
			FinishCloseScene(SceneToClose, bForceCloseImmediately/*, bSkipKismetNotify*/);
			bResult = true;
		}
	}

	return bResult;
}

/** Callback for when the scene we are closing's hide animation has completed. */
function OnPendingCloseScene_HideAnimationEnded()
{
	FinishCloseScene(PendingCloseScene, false, bSkipPendingCloseSceneNotify);
}

/**
 * Closes a UI Scene given a reference to an previously open scene.
 *
 * @param SceneToClose	Scene that we want to close.
 */
function FinishCloseScene(UIScene SceneToClose, bool bSkipAnimations=false, bool bSkipKismetNotify=false)
{
	local UTUIScene UTSceneToClose;
	local UTUIScene TopScene;

	// Reenable input
	GetUTInteraction().BlockUIInput(false);

	UTSceneToClose = UTUIScene(SceneToClose);

	if(UTSceneToClose != None)
	{
		UTSceneToClose.bHidingScene = false;
		UTSceneToClose.OnHideAnimationEnded = None;
	}

	PendingCloseScene = None;

	if ( SceneToClose != none )
	{
		TopScene = UTUIScene(SceneToClose.GetPreviousScene());
		SceneClient.CloseScene(SceneToClose);

		// If the scene we just closed wasn't set to render its parent scenes, then begin the show animation on the topmost scene.
		if ( TopScene != None && !SceneToClose.bRenderParentScenes && !TopScene.bAlwaysRenderScene )
		{
			// Active show animation on topmost scene
			TopScene.BeginShowAnimation(false, bSkipAnimations);
		}
	}

	if( !bSkipKismetNotify )
	{
		ActivateLevelEvent('ClosingMenu');
	}
}

/**
 * Starts the show animation for the scene.
 *
 * @param	bInitialActivation	TRUE if the scene is being opened; FALSE if the another scene was closed causing this one to become the
 *								topmost scene.
 * @param	bBypassAnimation	TRUE to force all animations to their last frame, effectively bypassing animations.  This can
 *								be necessary for e.g. scenes which start out off-screen or something.
 *
 * @return TRUE if there's animation for this scene, FALSE otherwise.
 */
function bool BeginShowAnimation(bool bInitialActivation=true, bool bBypassAnimation=false)
{
	return FALSE;
}

/**
 * Starts the exit animation for the scene.
 *
 * @return TRUE if there's animation for this scene, FALSE otherwise.
 */
function bool BeginHideAnimation(bool bClosingScene=false)
{
	return FALSE;
}

/** Called when an animation on this scene has finished. */
event UIAnimationEnded( UIScreenObject AnimTarget, name AnimName, int TrackType )
{
	Super.UIAnimationEnded(AnimTarget, AnimName, TrackType);

	if ( TrackType == 0 )
	{
		if(bHidingScene)
		{
			bHidingScene = false;
			OnHideAnimationEnded();
		}
		else if(bShowingScene)
		{
			bShowingScene = false;
			OnShowAnimationEnded();
		}
	}
}

/** @return Opens the message box scene and returns a reference to it. */
function UTUIScene_MessageBox GetMessageBoxScene(optional UIScene SceneReference = None)
{
	if (SceneReference == None)
	{
		SceneReference = MessageBoxScene;
	}

	return UTUIScene_MessageBox(OpenScene(SceneReference,/*LocalPlayer*/,/*ForcedPriority*/,true));
}

/** @return Opens the input box scene and returns a reference to it. */
function UTUIScene_InputBox GetInputBoxScene()
{
	return UTUIScene_InputBox(OpenScene(InputBoxScene,/*LocalPlayer*/,/*ForcedPriority*/,true));
}


/**
 * Displays a very simple OK message box with the specified message and title.
 *
 * @param Message		Message markup for the messagebox
 * @param Title			Title markup for the messagebox
 *
 * @return	Returns a reference to the message box scene that was displayed.
 */
function UTUIScene_MessageBox DisplayMessageBox (string Message, optional string Title="")
{
	local UTUIScene_MessageBox MessageBoxReference;

	MessageBoxReference = GetMessageBoxScene();

	if(MessageBoxReference != none)
	{
		MessageBoxReference.Display(Message, Title);
	}

	return MessageBoxReference;
}

function NotifyGameSessionEnded()
{
	local int i;
	for (i=0;i<Children.Length;i++)
	{
		NotifyChildGameSessionEnded(Children[i]);
		if ( UTUI_Widget(Children[i]) != none )
		{
			UTUI_Widget(Children[i]).NotifyGameSessionEnded();
		}
	}

	Super.NotifyGameSessionEnded();
}

function NotifyChildGameSessionEnded(UIObject Child)
{
	local int i;
	for ( i=0; i<Child.Children.Length; i++ )
	{
		NotifyChildGameSessionEnded(Child.Children[i]);
		if ( UTUI_Widget(Child.Children[i]) != none )
		{
			UTUI_Widget(Child.Children[i]).NotifyGameSessionEnded();
		}
	}
}

event OnAnimationFinished(UIObject AnimTarget, name AnimName, name SeqName);

/**
 * Allows easy access to playing a sound
 *
 * @Param	InSoundCue		The Cue to play
 * @Param	SoundLocation	Where in the world to play it.  Defaults at the Player's position
 */

function PlaySound( SoundCue InSoundCue)
{
	local UTPlayerController PC;

	PC = GetUTPlayerOwner();
	if ( PC != none )
	{
		PC.ClientPlaySound(InSoundCue);
	}
}

/** @return Returns a datastore given its tag and player owner. */
static function UIDataStore FindDataStore(name DataStoreTag, optional LocalPlayer InPlayerOwner)
{
	local DataStoreClient DSClient;
	local UIDataStore Result;

	DSClient = class'UIInteraction'.static.GetDataStoreClient();
	if ( DSClient != None )
	{
		Result = DSClient.FindDataStore(DataStoreTag, InPlayerOwner);
	}

	return Result;
}

/** @return Returns the controller id of a player given its player index. */
function int GetPlayerControllerId(int PlayerIndex)
{
	return class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);;
}

/** Activates a level remote event in kismet. */
native function ActivateLevelEvent(name EventName);

/**
 *  Returns the Player Profile for a given player index.
 *
 * @param	PlayerIndex		The player who's profile you require
 * @returns the profile precast to UTProfileSettings
 */
function UTProfileSettings GetPlayerProfile(optional int PlayerIndex=GetBestPlayerIndex() )
{
	local LocalPlayer LP;
	local UTProfileSettings Profile;

	LP = GetPlayerOwner(PlayerIndex);
	if ( LP != none && LP.Actor != none )
	{
		Profile = UTProfileSettings( LP.Actor.OnlinePlayerData.ProfileProvider.Profile);
	}
	return Profile;
}

/**
 *  Returns the Player Profile for a given player index.
 *
 * @param	PC	The PlayerContorller of the profile you require.
 * @returns the profile precast to UTProfileSettings
 */
function UTProfileSettings GetPlayerProfileFromPC(PlayerController PC )
{
	local UTProfileSettings Profile;
	Profile = UTProfileSettings( PC.OnlinePlayerData.ProfileProvider.Profile);
	return Profile;
}

/**
 * Saves the profile for the specified player index.
 *
 * @param PlayerIndex	The player index of the player to save the profile for.
 */
function SavePlayerProfile(optional int PlayerIndex=GetBestPlayerIndex())
{
	local UIDataStore_OnlinePlayerData	PlayerDataStore;
	PlayerDataStore = UIDataStore_OnlinePlayerData(FindDataStore('OnlinePlayerData', GetPlayerOwner(PlayerIndex)));

	if(PlayerDataStore != none)
	{
		`Log("UTUIScene::SaveProfile() - Saving player profile for player index "$PlayerIndex);
		PlayerDataStore.SaveProfileData();
	}
}

/** @return Returns the name of the specified player if they have an alias or are logged in, or "DefaultPlayer" otherwise. */
function string GetPlayerName(int PlayerIndex=GetBestPlayerIndex())
{
	local string PlayerName;

	if(IsLoggedIn(GetPlayerControllerId(PlayerIndex)))
	{
		PlayerName=GetUTPlayerOwner(PlayerIndex).OnlinePlayerData.PlayerNick;
	}
	else
	{
		PlayerName="Player";
	}

	// Replace invalid characters
	PlayerName = Repl(PlayerName," ","_");
	PlayerName = Repl(PlayerName,"?","_");
	PlayerName = Repl(PlayerName,"=","_");

	return PlayerName;
}

/** @return string	Returns a bot faction name given its enum. */
function name GetBotTeamNameFromIndex(EUTBotTeam TeamIndex)
{
	local name Result;

	switch(TeamIndex)
	{
	case UTBotTeam_Ironguard:
		Result = 'Ironguard';
		break;
	case UTBotTeam_TwinSouls:
		Result = 'TwinSouls';
		break;
	case UTBotTeam_Krall:
		Result = 'Krall';
		break;
	case UTBotTeam_Liandri:
		Result = 'Liandri';
		break;
	case UTBotTeam_Necris:
		Result = 'Necris';
		break;
	default:
		Result = '';
		break;
	}

	return Result;
}

/** @return Checks to see if the specified player is logged in, if not an error message is shown and FALSE is returned. */
function bool CheckLoginAndError(int ControllerId=GetBestControllerId(), optional bool bMustBeLoggedInOnline=false, optional string AlternateTitle, optional string AlternateMessage)
{
	local UTUIScene_MessageBox MessageBoxReference;
	local bool bResult;

	if ( IsLoggedIn(ControllerId, bMustBeLoggedInOnline) )
	{
		bResult = true;
	}
	else
	{
		if ( AlternateTitle == "" )
		{
			AlternateTitle = "<Strings:UTGameUI.Errors.LoginRequired_Title>";
		}
		if ( AlternateMessage == "" )
		{
			AlternateMessage = "<Strings:UTGameUI.Errors.LoginRequired_Message>";
		}
		MessageBoxReference = DisplayMessageBox(AlternateMessage,AlternateTitle);
		MessageBoxReference.OnSelection=OnLoginError_Confirm;
	}

	`log(`location@`showvar(ControllerId)@`showvar(bMustBeLoggedInOnline)@`showvar(IsLoggedIn(ControllerId,bMustBeLoggedInOnline),LoggedIn),!bResult,'DevOnline');
	return bResult;
}

/** Callback for when the login required box has finished displaying. */
function OnLoginError_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex);

/** @return Checks to see if the platform is currently connected to a network. */
function bool CheckLinkConnectionAndError( optional string AlternateTitle, optional string AlternateMessage )
{
	if( HasLinkConnection() )
	{
		return true;
	}
	else
	{
		if ( AlternateTitle == "" )
		{
			AlternateTitle = "<Strings:UTGameUI.Errors.Error_Title>";
		}
		if ( AlternateMessage == "" )
		{
			AlternateMessage = "<Strings:UTGameUI.Errors.LinkDisconnected_Message>";
		}

		DisplayMessageBox(AlternateMessage,AlternateTitle);
		return false;
	}
}

/** @return Checks to see if the specified player can play online games, if not an error message is shown and FALSE is returned. */
function bool CheckOnlinePrivilegeAndError(int ControllerId=GetBestControllerId(),  optional string AlternateTitle, optional string AlternateMessage )
{
	local bool bResult;

	bResult = false;

	if(CheckLinkConnectionAndError())
	{
		if(CanPlayOnline(ControllerId))
		{
			bResult = true;
		}
		else
		{
			if ( AlternateTitle == "" )
			{
				AlternateTitle = "<Strings:UTGameUI.Errors.OnlineRequired_Title>";
			}
			if ( AlternateMessage == "" )
			{
				AlternateMessage = "<Strings:UTGameUI.Errors.OnlineRequired_Message>";
			}
			DisplayMessageBox(AlternateMessage,AlternateTitle);
		}
	}

	return bResult;
}


/** @return Checks to see if the specified player can play online games, if not an error message is shown and FALSE is returned. */
function bool CheckContentPrivilegeAndError(int ControllerId=GetBestControllerId(),  optional string AlternateTitle, optional string AlternateMessage )
{
	local bool bResult;
	local OnlinePlayerInterface PlayerInt;

	bResult = false;
	PlayerInt = GetPlayerInterface();

	if(PlayerInt != None)
	{
		if(PlayerInt.CanDownloadUserContent(ControllerId)!=FPL_Disabled)
		{
			bResult = true;
		}
		else
		{
			if ( AlternateTitle == "" )
			{
				AlternateTitle = "<Strings:UTGameUI.Errors.ContentRequired_Title>";
			}
			if ( AlternateMessage == "" )
			{
				AlternateMessage = "<Strings:UTGameUI.Errors.ContentRequired_Message>";
			}
			DisplayMessageBox(AlternateMessage,AlternateTitle);
		}
	}

	return bResult;
}


/** @return Checks to see if the specified player can play online games, if not an error message is shown and FALSE is returned. */
function bool CheckCommunicationPrivilegeAndError(int ControllerId=GetBestControllerId(),  optional string AlternateTitle, optional string AlternateMessage )
{
	local bool bResult;
	local OnlinePlayerInterface PlayerInt;

	bResult = false;
	PlayerInt = GetPlayerInterface();

	if(PlayerInt != None)
	{
		if(PlayerInt.CanCommunicate(ControllerId)!=FPL_Disabled)
		{
			bResult = true;
		}
		else
		{
			if ( AlternateTitle == "" )
			{
				AlternateTitle = "<Strings:UTGameUI.Errors.CommunicationRequired_Title>";
			}
			if ( AlternateMessage == "" )
			{
				AlternateMessage = "<Strings:UTGameUI.Errors.CommunicationRequired_Message>";
			}
			DisplayMessageBox(AlternateMessage,AlternateTitle);
		}
	}

	return bResult;
}


/**
 * Verifies that the player's NAT configuration allows them to host matches, and displays an error message if not.
 */
function bool CheckNatTypeAndDisplayError( int ControllerId=GetBestControllerId(), optional string AlternateTitle, optional string AlternateMessage )
{
	local bool bResult;

	if ( CheckLinkConnectionAndError() )
	{
		if ( GetNATType() < NAT_Strict )
		{
			bResult = true;
		}
		else
		{
			if ( AlternateTitle == "" )
			{
				AlternateTitle = "<Strings:UTGameUI.Errors.Error_Title>";
			}
			if ( AlternateMessage == "" )
			{
				AlternateMessage = "<Strings:UTGameUI.Errors.StrictNAT_Message>";
			}

			DisplayMessageBox(AlternateMessage,AlternateTitle);
		}
	}

	return bResult;
}

/** @return Returns Returns a unique demo filename. */
function string GenerateDemoFileName()
{
	return GetPlayerName();
}

/** @return Generates a set of URL options common to both instant action and host game. */
function string GetCommonOptionsURL()
{
	local int BotTeamIndex;
	local string TeamName;
	local UTUIDataStore_StringList StringListDataStore;
	local string URL;
	local string OutStringValue;

	StringListDataStore = UTUIDataStore_StringList(FindDataStore('UTStringList'));

	if ( StringListDataStore != None )
	{
		// Set Bot Faction
		BotTeamIndex = StringListDataStore.GetCurrentValueIndex('BotTeams');
		if(BotTeamIndex != UTBotTeam_Random)
		{
			TeamName = string(GetBotTeamNameFromIndex(EUTBotTeam(BotTeamIndex)));
			URL $= "?BlueFaction=" $ TeamName $ "?RedFaction=" $ TeamName;
		}

		// Set if we are recording a demo
		if(StringListDataStore.GetCurrentValueIndex('RecordDemo')==UTRecordDemo_Yes)
		{
			URL $= "?demo=" $ GenerateDemoFileName();
		}
	}

	// Set player name using the OnlinePlayerData
	// @todo: Need to add support for setting 2nd player nick.
	URL $= "?name=" $ GetPlayerName();


	// Set player alias
	if(GetDataStoreStringValue("<OnlinePlayerData:ProfileData.Alias>", OutStringValue, self, GetPlayerOwner()) && Len(OutStringValue)>0)
	{
		OutStringValue = Repl(OutStringValue," ","_");
		OutStringValue = Repl(OutStringValue,"?","_");
		OutStringValue = Repl(OutStringValue,"=","_");

		URL $= "?alias="$OutStringValue;

	}

	return URL;
}

/** @return Returns a reference to the online subsystem game interface. */
static function OnlineGameInterface GetGameInterface()
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameInterface GameInt;

	// Display the login UI
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		GameInt = OnlineSub.GameInterface;
	}
	else
	{
		`Log("UTUIScene::GetGameInterface() - Unable to find OnlineSubSystem!");
	}

	return GameInt;
}

/** @return Returns a reference to the online subsystem player interface. */
static function OnlinePlayerInterface GetPlayerInterface()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;

	// Display the login UI
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerInt = OnlineSub.PlayerInterface;
	}
	else
	{
		`Log("UTUIScene::GetPlayerInterface() - Unable to find OnlineSubSystem!");
	}

	return PlayerInt;
}

/** @return Returns a reference to the online subsystem player interface ex. */
static function OnlinePlayerInterfaceEx GetPlayerInterfaceEx()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;

	// Display the login UI
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
	}
	else
	{
		`Log("UTUIScene::GetPlayerInterfaceEx() - Unable to find OnlineSubSystem!");
	}

	return PlayerIntEx;
}


/** @return Returns a reference to the online subsystem player interface. */
static function OnlineAccountInterface GetAccountInterface()
{
	local OnlineSubsystem OnlineSub;
	local OnlineAccountInterface AccountInt;

	// Display the login UI
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		AccountInt = OnlineSub.AccountInterface;
	}
	else
	{
		`Log("UTUIScene::GetAccountInterface() - Unable to find OnlineSubSystem!");
	}

	return AccountInt;
}

/** Displays the login interface using the online subsystem. */
function bool ShowLoginUI(optional bool bOnlineOnly=false)
{
	local OnlinePlayerInterface PlayerInt;
	local bool bResult;

	PlayerInt = GetPlayerInterface();

	if(PlayerInt != none)
	{
		UTGameUISceneClient(GetSceneClient()).bDimScreen=true;

		PlayerInt.AddLoginChangeDelegate(OnLoginUI_LoginChange, GetPlayerOwner().ControllerId);
		PlayerInt.AddLoginFailedDelegate(GetPlayerOwner().ControllerId, OnLoginUI_LoginFailed);

		bResult = PlayerInt.ShowLoginUI(bOnlineOnly);
		if ( !bResult )
		{
			`Log("UTUIScene::ShowLoginUI() - Failed to show login UI!");
			UTGameUISceneClient(GetSceneClient()).bDimScreen=false;
			PlayerInt.ClearLoginChangeDelegate(OnLoginUI_LoginChange, GetPlayerOwner().ControllerId);
			PlayerInt.ClearLoginFailedDelegate(GetPlayerOwner().ControllerId, OnLoginUI_LoginFailed);
		}
	}

	return bResult;
}

/** Callback for when the login changes after showing the login UI. */
function OnLoginUI_LoginChange()
{
	local OnlinePlayerInterface PlayerInt;

	PlayerInt = GetPlayerInterface();

	if(PlayerInt != none)
	{
		PlayerInt.ClearLoginChangeDelegate(OnLoginUI_LoginChange, GetPlayerOwner().ControllerId);
		PlayerInt.ClearLoginFailedDelegate(GetPlayerOwner().ControllerId, OnLoginUI_LoginFailed);
	}

	UTGameUISceneClient(GetSceneClient()).bDimScreen=false;
}

/**
 * Delegate used in notifying the UI/game that the manual login failed after showing the login UI.
 *
 * @param LocalUserNum the controller number of the associated user
 * @param ErrorCode the async error code that occurred
 */
function OnLoginUI_LoginFailed(byte LocalUserNum,EOnlineServerConnectionStatus ErrorCode)
{
	local OnlinePlayerInterface PlayerInt;

	PlayerInt = GetPlayerInterface();

	if(PlayerInt != none)
	{
		PlayerInt.ClearLoginChangeDelegate(OnLoginUI_LoginChange, GetPlayerOwner().ControllerId);
		PlayerInt.ClearLoginFailedDelegate(GetPlayerOwner().ControllerId, OnLoginUI_LoginFailed);
	}

	UTGameUISceneClient(GetSceneClient()).bDimScreen=false;
}

/** Shows the player card for the player with the net ID specified. */
function UTUIScene_PlayerCard ShowPlayerCard(UniqueNetId InPlayerId, optional string InPlayerName, optional string InAccountName, optional bool bIncludeKickOption, optional bool bIncludeBanOption)
{
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local UTUIScene_PlayerCard PlayerCardSceneInst;

	if(IsConsole(CONSOLE_Xbox360))
	{
		PlayerIntEx = GetPlayerInterfaceEx();

		if(PlayerIntEx != None)
		{
			PlayerIntEx.ShowGamerCardUI(GetPlayerOwner().ControllerId, InPlayerId);
		}
		return none;
	}
	else
	{
		PlayerCardSceneInst = UTUIScene_PlayerCard(OpenScene(class'UTUIScene'.default.PlayerCardScene,/*LocalPlayer*/,/*ForcedPriority*/,true));

		if(PlayerCardSceneInst != None)
		{
			PlayerCardSceneInst.SetPlayer(InPlayerId, InPlayerName, InAccountName, bIncludeKickOption, bIncludeBanOption);
		}
		return PlayerCardSceneInst;
	}
}

/** Shows a toast message. */
static function ShowOnlineToast(string InMessage, optional float ToastTime=3.0f)
{
	if(!IsConsole(CONSOLE_Xbox360))
	{
		UTGameUISceneClient(GetSceneClient()).SetToastMessage(InMessage, ToastTime);
	}
}

/** Hides the toast message. */
static function HideOnlineToast()
{
	if(!IsConsole(CONSOLE_Xbox360))
	{
		UTGameUISceneClient(GetSceneClient()).FinishToast();
	}
}


/**
 * Displays a screen warning message.  This message will be displayed prominently centered in the viewport and
 * will persist until you call ClearScreenWarningMessage().  It's useful for important modal warnings, such
 * as when the controller is disconnected on a console platform.
 *
 * @param Message Message to display
 */
static function ShowScreenWarningMessage( string Message )
{
	// NOTE: Currently we don't bother drawing these on Xbox since they automatically display things like
	//   'please reconnect controller', etc.
	if( !IsConsole( CONSOLE_Xbox360 ) )
	{
		UTGameUISceneClient( GetSceneClient() ).ShowScreenWarningMessage( Message );
	}
}


/**
 * Clears the screen warning message if one was set.  It will no longer be rendered.
 */
static function ClearScreenWarningMessage()
{
	if( !IsConsole( CONSOLE_Xbox360 ) )
	{
		UTGameUISceneClient( GetSceneClient() ).ClearScreenWarningMessage();
	}
}



/**
 * Creates/Removes local players for splitscreen.
 *
 * @param bCreatePlayers	Whether we are creating or removing players.
 */
native function UpdateSplitscreenPlayers(bool bCreatePlayers);

/** Checks to see if we should be playing splitscreen or not, if so, creates a 2nd local player. */
function ConditionallyStartSplitscreen()
{
	local UTUIDataStore_StringList StringListDataStore;

	StringListDataStore = UTUIDataStore_StringList(FindDataStore('UTStringList'));
	if( StringListDataStore != None && StringListDataStore.GetCurrentValueIndex('Splitscreen') == 1 )
	{
		`Log("UTUIFrontEnd: Starting game with splitscreen: Creating 2nd LocalPlayer.");
		UpdateSplitscreenPlayers(true);
	}
	else
	{
		`Log("UTUIFrontEnd: Starting game with no splitscreen: Removing all but 1 LocalPlayer.");
		UpdateSplitscreenPlayers(false);
	}
}


/** @return Checks for 2 controllers plugged in and displays a message if the user cannot play splitscreen, returns TRUE if the game can begin, returns FALSE otherwise. */
function bool ConditionallyCheckNumControllers()
{
	local int ControllerId;
	local OnlineSubsystem OnlineSub;
	local OnlineSystemInterface SystemInterface;
	local bool bResult;
	local int TotalControllers;
	local UTUIDataStore_StringList StringListDataStore;

	StringListDataStore = UTUIDataStore_StringList(FindDataStore('UTStringList'));
	if( StringListDataStore != None && StringListDataStore.GetCurrentValueIndex('Splitscreen') == 1 )
	{
		TotalControllers=0;
		bResult = false;

		// Figure out if we have an online subsystem registered
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			SystemInterface = OnlineSub.SystemInterface;
			if (SystemInterface != None)
			{
				for(ControllerId=0; ControllerId<MAX_SUPPORTED_GAMEPADS; ControllerId++)
				{
					if(SystemInterface.IsControllerConnected(ControllerId))
					{
						TotalControllers++;

						if(TotalControllers >= 2)
						{
							bResult=true;
							break;
						}
					}
				}
			}
		}

		if(bResult==false)
		{
			DisplayMessageBox("<Strings:UTGameUI.Errors.NeedSecondController_Message>","<Strings:UTGameUI.Errors.NeedSecondController_Title>");
		}
	}
	else
	{
		bResult = true;
	}

	return bResult;
}

/**
 * Converts a 2D Screen coordiate in to 3D space
 *
 * @Param	LocalPlayerOwner		The LocalPlayer that owns the viewport where the projection occurs
 * @Param	WorldLocation			The world location to project to
 * @Param	OutScreenLocation		Returns the location in 2D space
 */
native function ViewportProject(LocalPlayer LocalPlayerOwner, vector WorldLocation, out vector OutScreenLocation);

/**
 * Converts a 2D Screen coordiate in to 3D space
 *
 * @Param	LocalPlayerOwner		The LocalPlayer that owns the viewport where the projection occurs
 * @Param	ScreenLocation			Where on the screen are we converting from
 * @Param	OutLocation				Returns the Location in world space
 * @Param	OutDirection			Returns the view direction
 */
native function ViewportDeProject(LocalPlayer LocalPlayerOwner, vector ScreenLocation, out vector OutLocation, out vector OutDirection);

/** Function that sets up a buttonbar for this scene, automatically routes the call to the currently selected tab of the scene as well. */
function SetupButtonBar();

native function DeleteDemo(string DemoName);

defaultproperties
{
	MessageBoxScene=UIScene'UI_Scenes_Common.MessageBox'
	InputBoxScene=UIScene'UI_Scenes_Common.InputBox'
	PlayerCardScene=UIScene'UI_Scenes_Common.PlayerCard'
	OnlineToastScene=UIScene'UI_Scenes_Common.OnlineToast'
}
