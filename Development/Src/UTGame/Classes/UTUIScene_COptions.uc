/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUIScene_COptions extends UTUIFrontEnd
	config(Game);

var transient UILabel MenuLabel;
var transient UILabel SkillDesc;
var transient UIPanel LanGamePanel;
var transient UICheckBox SkillLevels[4];
var transient UICheckBox LanPlay;

var transient bool bNewGame;

/** Reference to the settings datastore that we will use to create the game. */
var transient UIDataStore_OnlineGameSettings	SettingsDataStore;

var localized string SkillDescriptions[4];

var transient string LaunchURL;

var transient int ChapterToLoad;

var transient bool bIgnoreChange;

var string ChapterURLs[5];

/**
 * @return	TRUE if the user is allowed to play internet games.
 */
function bool AllowInternetPlay()
{
	local LocalPlayer LP;

	LP = GetPlayerOwner();
	if ( LP == None )
	{
		return false;
	}

	if ( !IsLoggedIn(LP.ControllerID,true) )
	{
		`log("[Network] Not Logged in!");
		return false;
	}

	if (!CanPlayOnline(LP.ControllerID) )
	{
		`log("[Network] User Is Restricted from Online");
		return false;
	}

	// Check NAT.  If we are behind a strict nat, disable internet play
	if ( GetNATType() >= NAT_Strict )
	{
		`log("[Network] NAT configuration is incompatible");
		return false;
    }

	return true;
}

event PostInitialize( )
{
	local int i;

	Super.PostInitialize();

	for (i=0;i<4;i++)
	{
		SkillLevels[i] = UICheckBox( FindChild( name("Check"$i), true));
		SkillLevels[i].OnValueChanged = SkillLevelChanged;
	}

	SettingsDataStore = UIDataStore_OnlineGameSettings(FindDataStore('UTGameSettings'));
	SkillDesc = UILabel(FindChild('Description',true));
	LanGamePanel = UIPanel(FindChild('LanGamePanel',true));
	LanPlay = UICheckBox( FindChild('LanPlay',true));

	MenuLabel = UILabel(FindChild('Title',true));

	// this used to be set here, so we'll need to clear it in case it got serialized
	OnInitialSceneUpdate=None;

	// Created a scene closed delegate so we can clear the online delegate I added below
	OnSceneDeactivated = None;
}

/**
 * Handler for the 'show' animation completed.
 */
function OnMainRegion_Show_UIAnimEnd( UIScreenObject AnimTarget, name AnimName, int TrackTypeMask )
{
	Super.OnMainRegion_Show_UIAnimEnd(AnimTarget, AnimName, TrackTypeMask);

	if ( AnimName == 'SceneShowInitial' )
	{
		// make sure we can't choose "internet" if we aren't signed in online
		ValidateServerType();
	}
}

/**
 * Displays error messages and/or login ui to the user if the user is unable to play online.
 */
function ValidateServerType( optional bool bCheckLoginStatus=true )
{
	local int PlayerControllerID;

	PlayerControllerID = GetPlayerControllerId( GetPlayerIndex() );

	`log(`location @ `showvar(PlayerControllerId) @ `showvar(bCheckLoginStatus) @ `showvar(IsLoggedIn(PlayerControllerId,true),LoggedInOnline)
				@ `showvar(CanPlayOnline(PlayerControllerID)) @ `showenum(ENATType,GetNATType()),,'DevUI');

	if ( ((!bCheckLoginStatus && IsLoggedIn(PlayerControllerId, true)) || (bCheckLoginStatus && CheckLoginAndError(PlayerControllerId, true, , "<Strings:UTGameUI.Errors.OnlineRequiredForInternet_Message>")))
	&&	 CheckOnlinePrivilegeAndError(PlayerControllerID) )
	{
		CheckNatTypeAndDisplayError(PlayerControllerID);
	}

	UpdateNetworkChoices();
}

/**
 * Updates the visibility and enabled state of the LAN checkbox (and its owner panel) based on whether the user is
 * allowed to play online and has a valid internet connection.
 */
function UpdateNetworkChoices()
{
	local int PlayerIndex;
	local bool bAllowNetworkChoice, bShowLANOption;

	bShowLANOption = HasLinkConnection();
	bAllowNetworkChoice = bShowLANOption && AllowInternetPlay();

	PlayerIndex = GetPlayerIndex();
	if ( LanPlay != None )
	{
		LanPlay.SetVisibility(bShowLANOption);
		LanPlay.SetEnabled(bAllowNetworkChoice, PlayerIndex);
	}

	if ( LanGamePanel != None )
	{
		LanGamePanel.SetVisibility(bShowLANOption);
		LanGamePanel.SetEnabled(bAllowNetworkChoice, PlayerIndex);
	}
}

/** Callback for when the login changes after showing the login UI. */
function OnLoginUI_LoginChange()
{
	Super.OnLoginUI_LoginChange();

	ValidateServerType(false);
	SetupButtonBar();
}


/**
 * Delegate used in notifying the UI/game that the manual login failed after showing the login UI.
 *
 * @param ControllerId	the controller number of the associated user
 * @param ErrorCode		the async error code that occurred
 */
function OnLoginUI_LoginFailed( byte ControllerId,EOnlineServerConnectionStatus ErrorCode)
{
	Super.OnLoginUI_LoginFailed(ControllerId, ErrorCode);

	ValidateServerType(false);
	SetupButtonBar();
}

function SkillLevelChanged( UIObject Sender, int PlayerIndex )
{
	local int i;
	local UTProfileSettings Profile;

	Profile = GetPlayerProfile();

	if (!bIgnoreChange)
	{
		bIgnoreChange = true;
		for (i=0;i<4;i++)
		{
			if ( SkillLevels[i] == Sender )
			{
				SkillLevels[i].SetValue(true);
				SkillDesc.SetValue(SkillDescriptions[i]);
				Profile.SetCampaignSkillLevel(i);
			}
			else
			{
				SkillLevels[i].SetValue(false);
			}
		}
		bIgnoreChange = false;
	}
}


/** Sets up the button bar for the scene. */
function SetupButtonBar()
{

	if(ButtonBar != None)
	{
		ButtonBar.Clear();
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);

		if ( HasLinkConnection() )
		{
			ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.StartGame>", OnButtonBar_Start);
			ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.StartPublicGame>", OnButtonBar_StartPublic);
		}
		else
		{
			ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.StartGame>", OnButtonBar_Start);
		}

		if (!IsConsole())
		{
			ButtonBar.Buttons[1].SetFocus(none);
		}
	}
}

function Configure(bool bIsNewGame, int InChapterToLoad)
{
	local UTProfileSettings Profile;
	local int Skill;
	local string s;

	bNewGame = bIsNewGame;

	s = "[<Strings:UTGameUI.Campaign.CampaignOptionsPrefix>" @ (bIsNewGame ? "<Strings:UTGameUI.Campaign.CampaignTitleA>]" : "<Strings:UTGameUI.Campaign.CampaignTitleB>]");
    GetTitleLabel().SetDataStoreBinding(Caps(s));

	Profile = GetPlayerProfile();
	if ( Profile != None )
	{
		if ( bIsNewGame )
		{
			Profile.NewGame();
		}
		Skill = Profile.GetCampaignSkillLevel();
	}

	SkillLevels[Skill].SetValue( true );
	ChapterToLoad = InChapterToLoad;
}

function bool OnButtonBar_Start(UIScreenObject InButton, int InPlayerIndex)
{
	StartGame(InPlayerIndex, false);
	return true;
}

function bool OnButtonBar_StartPublic(UIScreenObject InButton, int InPlayerIndex)
{
	StartGame(InPlayerIndex, true);
	return true;
}

`define MaxCampaignPlayers 4

function StartGame(int InPlayerIndex, bool bPublic)
{
	local UTProfileSettings Profile;
	local int Skill;

	Profile = GetPlayerProfile();
	if ( Profile != none )
	{
		Skill = Profile.GetCampaignSkillLevel();
	}

	Skill *= 2;

	if (ChapterToLoad != INDEX_None)
	{
		LaunchURL = "open "$ChapterURLs[ChapterToLoad]$"?Difficulty="$Skill$"?MaxPlayers=" $ `MaxCampaignPlayers;
	}
	else
	{
		if ( bNewGame )
		{
			LaunchURL = "open UTM-MissionSelection?SPI=0?SPResult=1?Difficulty="$Skill$"?MaxPlayers=" $ `MaxCampaignPlayers;
		}
		else
		{
			LaunchURL = "open UTM-MissionSelection?Difficulty="$Skill$"?MaxPlayers=" $ `MaxCampaignPlayers;
		}
	}

	if ( HasLinkConnection() )
	{
		LaunchURL $= "?Listen";

		// if we're able to play online, or the lan checkbox is checked, create the match.
		CreateOnlineGame(InPlayerIndex, bPublic, LanPlay.IsChecked() || !AllowInternetPlay());
	}
	else
	{
		ConsoleCommand(LaunchURL);
	}
}


/************************ Online Game Interface **************************/

/** Creates the online game and travels to the map we are hosting a server on. */
function CreateOnlineGame(int PlayerIndex, bool bPublic, bool bIsLanMatch)
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameInterface GameInterface;
	local OnlineGameSettings GameSettings;

	// Figure out if we have an online subsystem registered
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		// Grab the game interface to verify the subsystem supports it
		GameInterface = OnlineSub.GameInterface;
		if (GameInterface != None)
		{
			// Play the startgame sound
			PlayUISound('StartGame');

			// Setup server options based on server type.
			SettingsDataStore.SetCurrentByName('UTGameSettingsCampaign');
			GameSettings = SettingsDataStore.GetCurrentGameSettings();

			`log("Starting a Campaign --- ");

			GameSettings.bUsesArbitration=false;				`log("   bUseArbitration:"@GameSettings.bUsesArbitration);
			GameSettings.bAllowJoinInProgress = true;			`log("   bAllowJoinInProgress:"@GameSettings.bAllowJoinInProgress);
			GameSettings.bAllowInvites = true;					`log("   bAllowInvites:"@GameSettings.bAllowInvites);
			GameSettings.bIsLanMatch=bIsLanMatch;				`log("   bIsLanMatch:"@GameSettings.bIsLanMatch);

			if (bPublic)
			{
				GameSettings.NumPrivateConnections = 0;
				GameSettings.NumPublicConnections = 4;
				GameSettings.bShouldAdvertise = true;
			}
			else
			{
				GameSettings.NumPrivateConnections = 4;
				GameSettings.NumPublicConnections = 0;
				GameSettings.bShouldAdvertise = false;
			}

			`log("   bShouldAdvertise:"@GameSettings.bShouldAdvertise);
			`log("   NumPrivateConnections"@GameSettings.NumPrivateConnections);
			`log("   NumPublicConnections"@GameSettings.NumPublicConnections);

			// Create the online game
			GameInterface.AddCreateOnlineGameCompleteDelegate(OnGameCreated);

			if(SettingsDataStore.CreateGame(GetPlayerControllerId(PlayerIndex))==false)
			{
				GameInterface.ClearCreateOnlineGameCompleteDelegate(OnGameCreated);
				`Log("UTUIScene_COption::CreateOnlineGame - Failed to create online game.");
			}
		}
		else
		{
			`Log("UTUIScene_COption::CreateOnlineGame - No GameInterface found.");
		}
	}
	else
	{
		`Log("UTUIScene_COption::CreateOnlineGame - No OnlineSubSystem found.");
	}
}

/** Callback for when the game is finish being created. */
function OnGameCreated(name SessionName,bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameInterface GameInterface;

	// Figure out if we have an online subsystem registered
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		// Grab the game interface to verify the subsystem supports it
		GameInterface = OnlineSub.GameInterface;
		if (GameInterface != None)
		{
			// Clear the delegate we set.
			GameInterface.ClearCreateOnlineGameCompleteDelegate(OnGameCreated);

			// If we were successful, then travel.
			if(bWasSuccessful)
			{
				ConsoleCommand(LaunchURL);
			}
			else
			{
				`Log("UTUIScene_COption::OnGameCreated - Game Creation Failed.");
			}
		}
		else
		{
			`Log("UTUIScene_COption::OnGameCreated - No GameInterface found.");
		}
	}
	else
	{
		`Log("UTUIScene_COption::OnGameCreated - No OnlineSubSystem found.");
	}
}

function bool HasPushedUp( const InputEventParameters EventParms )
{
	if( (EventParms.InputKeyName == 'Up') || (EventParms.InputKeyName == 'XboxTypeS_DPad_Up') || (EventParms.InputKeyName == 'Gamepad_LeftStick_Up') )
	{
		if ( EventParms.EventType==IE_Released )
		{
			return true;
		}
	}
	return false;
}

function bool HasPushedDown( const InputEventParameters EventParms )
{
	if( (EventParms.InputKeyName == 'Down') || (EventParms.InputKeyName == 'XboxTypeS_DPad_Down') || (EventParms.InputKeyName == 'Gamepad_LeftStick_Down') )
	{
		if ( EventParms.EventType==IE_Released )
		{
			return true;
		}
	}
	return false;
}

/**
 * Provides a hook for unrealscript to respond to input using actual input key names (i.e. Left, Tab, etc.)
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called BEFORE kismet is given a chance to process the input.
 *
 * @param	EventParms	information about the input event.
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool HandleInputKey( const out InputEventParameters EventParms )
{
	if ( HasPushedUp(EventParms) )
	{
		ChangeSkill(false);
		return true;
	}

	if ( HasPushedDown(EventParms) )
	{
		ChangeSkill(true);
		return true;
	}

    if( EventParms.InputKeyName=='XBoxTypeS_A' )
    {
		if (EventParms.EventType==IE_Released)
		{
			StartGame(EventParms.PlayerIndex,false);
		}
		return true;
	}

    if( EventParms.InputKeyName=='XboxTypeS_LeftShoulder')
    {
    	if ( AllowInternetPlay() && EventParms.EventType==IE_Released )
    	{
    		LanPlay.SetValue( !LanPlay.IsChecked() );
    	}

		return true;
	}


	if( EventParms.InputKeyName=='XboxTypeS_X' )
	{
		if ( EventParms.EventType==IE_Released && HasLinkConnection() )
		{
			StartGame(EventParms.PlayerIndex,true);
		}
		return true;
	}

	if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
	{
		if (EventParms.EventType==IE_Released)
		{
			OnBack();
		}
		return true;
	}

	return Super.HandleInputKey(EventParms);
}

function ChangeSkill( bool bSelectionDown )
{
	local UTProfileSettings Profile;
	local int Skill;

	Profile = GetPlayerProfile();
	if ( Profile != none )
	{
		Skill = Profile.GetCampaignSkillLevel() + (bSelectionDown ? 1 : -1);
		if (Skill > 3)
		{
			Skill = 0;
		}
		else if (Skill < 0)
		{
			Skill = 3;
		}

		SkillLevels[Skill].SetValue(true);
	}
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Back(UIScreenObject InButton, int PlayerIndex)
{
	OnBack();

	return true;
}

/** Callback for when the user wants to exit the scene. */
function OnBack()
{
	CloseScene(self);
}


defaultproperties
{
	ChapterURLs(0)="UTM-MissionSelection?SPI=0?SPResult=1"
	ChapterURLs(1)="UTM-MissionSelection?SPI=1?SPResult=1"
	ChapterURLs(2)="UTM-MissionSelection?SPI=15?SPResult=1"
	ChapterURLs(3)="UTM-MissionSelection?SPI=24?SPResult=1"
	ChapterURLs(4)="UTM-MissionSelection?SPI=33?SPResult=1"
}
