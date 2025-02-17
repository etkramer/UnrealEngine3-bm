/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFELobby_Campaign extends GearUISceneFELobby_Base
	ClassRedirect(GearUISceneFEParty_Campaign)
	Config(UI);


/************************************************************************/
/* Struct, consts, enums, etc.                                          */
/************************************************************************/

struct CampStartData
{
	/** The start button label next to the Uber button */
	var transient UILabel MatchmakeButtonIcon;

	/** The uber button that is pressed to create or join a campaign */
	var transient UILabelButton StartCampButton;

	/** The panel that contains everything dealing with the uber button area (except the uber button itself) */
	var transient UIPanel ParentPanel;

	/** Image of the COG */
	var transient UIImage CogImage;
	/** Image of the COG for loading/searching (animates) */
	var transient UIImage CogLoadImage;
	/** Image of the COG for errors */
	var transient UIImage CogErrorImage;

	/** Panel containing the widgets used for showing a message and showing the join/create buttons */
	var transient UIPanel SubParentPanel;

	/** Panel containing the create/join buttons */
	var transient UIPanel ButtonPanel;
	/** Button to press for joining a game */
	var transient UILabelButton JoinGameButton;
	/** Button to press for hosting a game */
	var transient UILabelButton HostGameButton;

	/** Panel containing any messages we need to convey to the players about the state of searching */
	var transient UIPanel MessagePanel;
	/** Label to use for showing a message in the message panel */
	var transient UILabel MessageLabel;

	/** Blood splat image for when the button is pressed */
	var transient UIImage BloodSplatImage;
};

/** Struct to store the information and widgets needed for a player */
struct CampPlayerData
{
	/** Reference to the Player button (contains their name) */
	var transient UILabelButton PlayerButton;
	/** Reference to the Difficulty button label for the player */
	var transient UILabel DifficultyButtonLabel;
	/** Reference to the Difficulty label for the player */
	var transient UILabel DifficultyLabel;
	/** Reference to the Difficulty description label for the player */
	var transient UILabel DifficultyDescLabel;
	/** Reference to the chat bubble for this player */
	var transient UIImage ChatImage;
	/** Reference to the character image of the player */
	var transient UIImage CharacterImage;
	/** Reference to the rank label for this player */
	var transient UILabel RankLabel;
	/** Reference to the controller icon label */
	var transient UILabel ProfileLabel;

	/** The image of marcus */
	var transient UIImage ImageMarcus;

	/** The PRI of the player in this slot */
	var transient GearCampaignLobbyPRI PlayerPRI;
};

/** The states the message/button area can be in the uber-button area */
enum CampMessageState
{
	eCMS_None,
	eCMS_Normal,
	eCMS_Message,
	eCMS_Buttons,
};


var localized string ConnectingPlayerSlot;

/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Reference to the list of options in the scene */
var transient GearUIObjectList OptionsList;

/** Reference to the options panel (the whole left side that only the host can navigate to) */
var transient UIPanel OptionPanel;

/** Reference to the collapsing list for the acts */
var transient GearUICollapsingSelectionList ActsList;

/** Reference to the collapsing list for the chapters */
var transient GearUICollapsingSelectionList ChapterList;

/** Reference to the collapsing list for the checkpoint option */
var transient GearUICollapsingSelectionList CheckpointOption;

/** Reference to the collapsing list for the campaign mode option */
var transient GearUICollapsingSelectionList CampaignModeOption;

/** Reference to the collapsing list for the invite type option */
var transient GearUICollapsingSelectionList InviteTypeOption;

/** Widgets used for the uber start campaign button */
var transient CampStartData StartData;

/** Reference to the player button bar */
var transient UICalloutButtonPanel PlayerButtonBar;
/** Reference to the LAN button bar */
var transient UICalloutButtonPanel btnbarLAN;

/** Reference to the campaign image for the currently selected map */
var transient UIImage CampaignMapImage;

/** Reference to 'imgChapterFade'; we fade to this image when switching and then swap. */
var transient UIImage CampaignMapFadeImage;

/** The index of the Act currently chosen */
var transient int CurrentActIndex;

/** The index of the Chapter within the act currently chosen */
var transient int CurrentChapterIndex;



/** List of references to the images for whether you've completed a map on a particular difficulty */
var transient array<UIImage> DifficultyCompleteImages;
/** The background image for the difficulty images */
var transient UIImage DifficultyBGImage;

/** List of both Player datum for this scene */
var transient array<CampPlayerData> PlayerList;

/** The player panel */
var transient UIPanel PlayerPanel;

/** Reference to the currently active checkpoint */
var transient Checkpoint ActiveCheckpoint;

/** Whether the option to "load last checkpoint" is available or not */
var transient bool bShouldHaveLoadOption;

/** Whether to constrain act and chapters to the profile locking */
var config bool bConstrainChapterToProfile;

/** Reference to the SelectSaveSlot scene instance */
var transient GearUISceneFE_SelectSaveSlot SelectSaveSlotSceneInstance;

/** String to display as the player name for an empty slot in the playerlist */
var localized string EmptySlotString;
var localized string InvitablePlayerSlot;

/** Difficulty title string */
var localized String DifficultyString;
var localized string StartGame;
var localized string JoinGame;
var localized string CreateGame_Desc;
var localized string JoinGame_Desc;
var localized string PlayerClickFull_Desc;
var localized string PlayerClickEmpty_Desc;
var localized string CampModeDesc_LivePublic;
var localized string CampModeDesc_LivePrivate;
var localized string CampModeDesc_LAN;
var localized string CampModeDesc_Main;
var localized string CheckpointDesc_Main;
var localized string CheckpointDesc_Check;
var localized string CheckpointDesc_Restart;
var localized string BrowserString;

/** The current message state of the uber button area */
var transient CampMessageState CurrMessageState;

/** Local GRI cache */
var transient GearCampaignGRI CampGRI;

/** The previous value of the campaign mode */
var transient int PreviousCampMode;

/** Whether the player chose to start a LAN campaign when on the LAN screen */
var transient bool bWantsToStartLANCampaign;
/** Whether the player chose to cancel from the LAN scene or not */
var transient bool bCanceledLANScene;

/** Whether the scene has been initialized by the GRI yet (for clients) */
var transient bool bClientIsInitialized;

/** Store whether we are using a checkpoint so we can launch the game when the create online game delegate returns */
var transient bool bUsingCheckpoint;

/**
 * Indicates that a checkpoint deletion is pending (i.e. PendingCheckpointAction has been set on GearEngine)
 */
var	transient bool bIsDeletingCheckpoint;

/** Scene resource reference to the What's Up screen */
var UIScene WhatsUpSceneReference;

/** The mode the campaign is in */
var transient EGearCampaignLobbyMode CampaignMode;
/** Whether we arrived in the lobby from the game or not */
var transient bool bReturnedFromGame;

/** Whether to force the checkpoint option on the next regeneration of options */
var transient bool bForceCheckpointOnNextRegen;

/************** Animations ***************/

/** Animation for fading in */
var transient UIAnimationSeq FadeIn;

/** Animation for fading out */
var transient UIAnimationSeq FadeOut;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	local string EnterLobbyString;
	local string ExitGameString;
	local GearCampaignLobbyGame_Base CampGI;
	local int PlayerIndex;
	local GearPC PC;
	local int Value;

	Super.PostInitialize();

	if ( !IsEditor() )
	{
		// Keep track of whether we came from the game or not
		ExitGameString = GetTransitionValue("ExitGame");
		if (ExitGameString == "Yes")
		{
			bReturnedFromGame = true;
			ClearTransitionValue("ExitGame");
		}

		// Determine the lobby mode to use
		EnterLobbyString = GetTransitionValue("EnterLobby");
		CampaignMode = GetCampaignLobbyMode(EnterLobbyString);
		ClearTransitionValue("EnterLobby");
		// Set the network requirement flag
		if (CampaignMode == eGCLOBBYMODE_Split)
		{
			bRequiresNetwork = false;
		}
		else
		{
			bRequiresNetwork = true;
		}

		// Create a coop settings object if one is not present
		if (CampaignMode == eGCLOBBYMODE_Join ||
			CampaignMode == eGCLOBBYMODE_Split)
		{
			CampGI = GetCampGameInfo();
			if (CampGI.CoopGameSettings == None)
			{
				CampGI.CoopGameSettings = new class'GearCoopGameSettings';
			}
		}

		CacheGRI();

		// Get reference to the active checkpoint
		SetCurrentCheckpointReference();

		// Initialize the difficulty for this player
		PlayerIndex = GetBestPlayerIndex();
		PC = GetGearPlayerOwner(PlayerIndex);
		if (PC != None && PC.ProfileSettings != None)
		{
			PC.ProfileSettings.GetProfileSettingValueId(PC.ProfileSettings.GameIntensity, Value);
			PC.ServerSetCampaignLobbyDifficulty(EDifficultyLevel(Value));
		}

		SetupGRICallbacks();

		InitializeWidgetReferences();

		SetupCallbacks();
	}
}

/** Takes the EnterLobby string returned from the registry and returns the mode of the lobby */
static function EGearCampaignLobbyMode GetCampaignLobbyMode(string EnterLobbyString)
{
	local int LocalPlayerCount;

	if (EnterLobbyString == "MainMenu_JoinCoop")
	{
		return eGCLOBBYMODE_Join;
	}
	else if (EnterLobbyString == "MainMenu_HostCoop")
	{
		return eGCLOBBYMODE_Host;
	}
	else if (EnterLobbyString == "MainMenu_Splitscreen")
	{
		return eGCLOBBYMODE_Split;
	}
	else
	{
		LocalPlayerCount = class'UIInteraction'.static.GetLoggedInPlayerCount();
		if (LocalPlayerCount > 1)
		{
			return eGCLOBBYMODE_Split;
		}
		else
		{
			return eGCLOBBYMODE_Host;
		}
	}
}

/** Sets the uber button's string */
function SetCampaignButtonString()
{
	local string StringToUse;

	if (CampaignMode == eGCLOBBYMODE_Join)
	{
		StringToUse = JoinGame;
	}
	else
	{
		StringToUse = StartGame;
	}

	StartData.StartCampButton.SetDataStoreBinding(StringToUse);
}

/** Assigns all member variables to appropriate child widget from this scene */
function InitializeWidgetReferences()
{
	local int Idx;
	local string WidgetName;
	local GearPC GearPO;
	local int PlayerIndex;

	PlayerIndex = GetBestPlayerIndex();

	// The options panel
	OptionPanel = UIPanel(FindChild('pnlOptions', true));


	// Get reference to start button shorcut icon and hide it if necessary
	StartData.MatchmakeButtonIcon	= UILabel(FindChild('lblStart',TRUE));
	if ( GetWorldInfo().NetMode == NM_Client )
	{
		StartData.MatchmakeButtonIcon.SetVisibility(FALSE);
	}

	// Get references to the uber button widgets
	StartData.StartCampButton		= UILabelButton(OptionPanel.FindChild('btnFindGame', true));
	SetCampaignButtonString();
	StartData.ParentPanel			= UIPanel(OptionPanel.FindChild('pnlStartGame', true));
	StartData.CogImage				= UIImage(OptionPanel.FindChild('imgCOG', true));
	StartData.CogLoadImage			= UIImage(OptionPanel.FindChild('imgCOGLoading', true));
	StartData.CogErrorImage			= UIImage(OptionPanel.FindChild('imgCOGBroken', true));
	StartData.SubParentPanel		= UIPanel(StartData.ParentPanel.FindChild('pnlPartyMessage', true));
	StartData.ButtonPanel			= UIPanel(StartData.SubParentPanel.FindChild('pnlCreateJoinButton', true));
	StartData.JoinGameButton		= UILabelButton(StartData.ButtonPanel.FindChild('btnJoin', true));
	StartData.HostGameButton		= UILabelButton(StartData.ButtonPanel.FindChild('btnCreate', true));
	StartData.MessagePanel 			= UIPanel(StartData.SubParentPanel.FindChild('pnlMessage', true));
	StartData.MessageLabel			= UILabel(StartData.MessagePanel.FindChild('lblPartyMessage', true));
	StartData.BloodSplatImage		= UIImage(FindChild('imgSplat', true));

	PlayerButtonBar = UICalloutButtonPanel(FindChild('btnbarPlayers', true));
	if (PlayerButtonBar != None)
	{
		PlayerButtonBar.SetButtonCallback('Checkpoint', SaveSlotButtonClicked);
		PlayerButtonBar.EnableButton('Checkpoint', PlayerIndex, CanAccessCheckpointScene(PlayerIndex), true);
	}

	btnbarLAN = UICalloutButtonPanel(FindChild('btnbarLAN', true));
	if (btnbarLAN != none)
	{
		btnbarLAN.SetVisibility( false );
		btnbarLAN.EnableButton('LANBrowser', PlayerIndex, false, true);
	}

	CampaignMapImage = UIImage(FindChild('imgChapter', true));
	CampaignMapFadeImage = UIImage(FindChild('imgChapterFade', true));
	OptionsList = GearUIObjectList(FindChild('lstOptions', true));

	// Initialize the difficulty widgets
	DifficultyCompleteImages.length = 4;
	for ( Idx = 0; Idx < DifficultyCompleteImages.length; Idx++ )
	{
		WidgetName = "imgDifficulty" $ Idx;
		DifficultyCompleteImages[Idx] = UIImage(FindChild(Name(WidgetName), true));
		DifficultyCompleteImages[Idx].SetDataStoreBinding( class'GearChapterComboProvider'.default.DifficultyImagePaths[Idx] );
	}
	DifficultyBGImage = UIImage(FindChild('imgDifficultyBG', true));
	DifficultyBGImage.SetVisibility(false);

	// Initialize the playerlist
	PlayerPanel = UIPanel(FindChild('pnlPartyMembers', true));

	PlayerList.length = 2;
	for (Idx = 0; Idx < PlayerList.length; Idx++)
	{
		// Find the Player button
		WidgetName = "btnPlayer" $ Idx;
		PlayerList[Idx].PlayerButton = UILabelButton(FindChild(Name(WidgetName), true));
		PlayerList[Idx].PlayerButton.OnClicked = OnPlayerButtonClicked;
		// If there was a player button find the difficulty label child
		if ( PlayerList[Idx].PlayerButton != None )
		{
			PlayerList[Idx].DifficultyButtonLabel = UILabel(PlayerList[Idx].PlayerButton.FindChild('lblX', true));
			PlayerList[Idx].DifficultyLabel = UILabel(PlayerList[Idx].PlayerButton.FindChild('lblDifficulty', true));
			PlayerList[Idx].DifficultyDescLabel = UILabel(PlayerList[Idx].PlayerButton.FindChild('lblDifficultyString', true));
			PlayerList[Idx].RankLabel = UILabel(PlayerList[Idx].PlayerButton.FindChild('lblRank', true));
			PlayerList[Idx].ProfileLabel = UILabel(PlayerList[Idx].PlayerButton.FindChild('lblProfile', true));
		}

		// Find the Chat Bubble image
		WidgetName = "imgChat" $ Idx;
		PlayerList[Idx].ChatImage = UIImage(FindChild(Name(WidgetName), true));
		PlayerList[Idx].ChatImage.SetVisibility(true);
		PlayerList[Idx].ChatImage.Opacity = 0.0f;
	}

	// Handle to Marcus' icon so we can grey it out for the campaign join screen.
	PlayerList[0].ImageMarcus = UIImage(FindChild('imgMarcus', true));

	// Hack to make sure the first player is the one who gets focus when navigating over
	// and that the uber button gets focus on the host
	PlayerList[0].PlayerButton.SetFocus(None);
	if ( IsHost(PlayerIndex) )
	{
		GearPO = GetGearPlayerOwner(PlayerIndex);
		if (GearPO != None && GearPO.ProfileSettings != None)
		{
			GearPO.ProfileSettings.GetProfileSettingValueId(GearPO.ProfileSettings.const.CAMP_MODE, PreviousCampMode);
		}
		StartData.StartCampButton.SetFocus(None);
	}
	else
	{
		OptionPanel.SetEnabled(false);
	}

	// Set the option list
	SetMenuItemsInList();

	// Set the state of the uber message area
	SetMessageState( eCMS_Normal );
}

/** Whether the player can open the LAN browser or not */
function bool CanOpenLANScene(int PlayerIndex)
{
	if (GetCampaignMode() == eGCM_SystemLink &&
		GetWorldInfo().NetMode != NM_Client &&
		CampaignMode == eGCLOBBYMODE_Join)
	{
		return true;
	}
	return false;
}

/**
 * LAN Browser button pressed
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnLANBrowserClicked( UIScreenObject EventObject, int PlayerIndex )
{
	if ( GetCampaignMode() == eGCM_SystemLink &&
		 PlayerIndex == 0 &&
		 !CampGRI.bIsInputGuarding )
	{
		AttemptSystemLinkTransition();
	}

	return true;
}

/**
 * Called when the uilist selection changes (e.g. the list is open and the selection moved up or down)
 *
 * We want to crossfade the maps as the user is switching between them
 *
 * @param	Sender	the list that is submitting the selection
 * @param	PlayerIndex the player who caused this action
 */
private function ActSelectionChanged( UIObject Sender, int PlayerIndex  )
{
	local GearCampaignChapterData ChapterProvider;
	local EChapterPoint CurrChapter;
	local int CurrentHighlightedActIndex;
	local int ChapterIndexToShow;

	CurrentHighlightedActIndex = GetCurrentlySelectedAct();

	// If we are highlighting the currently selected act then show the chapter that is selected within that act.
	// Otherwise just show the first chapter within that act.
	ChapterIndexToShow = (CurrentActIndex == CurrentHighlightedActIndex) ? CurrentChapterIndex : 0;

	CurrChapter = GetChapterType(CurrentHighlightedActIndex, ChapterIndexToShow);
	ChapterProvider = class'GearUIDataStore_GameResource'.static.GetChapterDataProvider( CurrChapter );


	if ( ChapterProvider != None )
	{
		if ( CampaignMapImage != None )
		{
			CrossFade(ChapterProvider.ScreenshotPathName);
		}
	}
}



/**
 * Called when the uilist selection changes (e.g. the list is open and the selection moved up or down)
 *
 * We want to crossfade the maps as the user is switching between them
 *
 * @param	Sender	the list that is submitting the selection
 * @param	PlayerIndex the player who caused this action
 */
private function MapSelectionChanged( UIObject Sender, int PlayerIndex  )
{
	local GearCampaignChapterData ChapterProvider;
	local EChapterPoint CurrChapter;

	CurrChapter = GetCurrentSelectedChapterType();
	ChapterProvider = class'GearUIDataStore_GameResource'.static.GetChapterDataProvider( CurrChapter );


	if ( ChapterProvider != None )
	{
		if ( CampaignMapImage != None )
		{
			CrossFade(ChapterProvider.ScreenshotPathName);
		}
	}
}

/**
 * Crossfade between the current map portrait and another map portrait.
 *
 * @param NewTargetImage	The target image to fade to
 */
private function CrossFade(string NewTargetImage)
{
	local string CurrentImage;

	CampaignMapFadeImage.SetVisibility(TRUE);

	CurrentImage = CampaignMapImage.GetDataStoreBinding();

	//`Log("CrossFade: " $ CurrentImage $ "->" $ NewTargetImage);
	//ScriptTrace();

	if (CurrentImage != NewTargetImage)
	{
		CampaignMapImage.SetDataStoreBinding(NewTargetImage);
		CampaignMapImage.Opacity = 0.0f;
		CampaignMapImage.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );

		CampaignMapFadeImage.SetDataStoreBinding(CurrentImage);
		CampaignMapFadeImage.Opacity = 1.0f;
		CampaignMapFadeImage.PlayUIAnimation( '', FadeOut, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );
	}
}


/** Update the chat icon opacities */
final function UpdateChatIcons()
{
	local int Idx;
	local GearPRI CurrPRI;

	for (Idx = 0; Idx < PlayerList.length; Idx++)
	{
		if (PlayerList[Idx].ChatImage != None)
		{
			PlayerList[Idx].ChatImage.Opacity = 0.0f;
			CurrPRI = PlayerList[Idx].PlayerPRI;
			if (CurrPRI != None)
			{
				if(CurrPRI.ChatFadeValue > 0)
				{
					CurrPRI.ChatFadeValue = 1.0f - (GetWorldInfo().TimeSeconds - CurrPRI.TaccomChatFadeStart) / CurrPRI.ChatFadeTime;
					PlayerList[Idx].ChatImage.Opacity = CurrPRI.ChatFadeValue;
				}
			}
		}
	}
}


/** Assigns delegates in important child widgets to functions in this scene class */
function SetupCallbacks()
{
	if (IsHost(GetBestPlayerIndex()))
	{
		OptionsList.OnOptionChanged = OptionValueChanged;
		OptionsList.OnListOptionSubmitted = ListValueSubmitted;
		OptionsList.OnRegeneratedOptions = OnRegeneratedOptionsCallback;
		StartData.StartCampButton.OnClicked = StartGameButtonClicked;
	}
}

/** Whether the player can use the checkpoint screen or not */
function bool CanAccessCheckpointScene(int PlayerIndex)
{
	local GearEngine GE;
	local bool bResult;

	if (IsHost(PlayerIndex) &&
		PlayerIndex == 0 &&
		CampaignMode != eGCLOBBYMODE_Join)
	{
		GE = GetGearEngine();
		bResult = GE.bShouldWriteCheckpointToDisk && StorageDeviceIsReady(0);
	}

	return bResult;
}

/** Assigns delegates to the GRI class */
function SetupGRICallbacks()
{
	local GearPC OwnerPC;

	// Only the clients will use the GRI to set their values
	if (!IsHost(GetBestPlayerIndex()))
	{
		if (CampGRI != None)
		{
			CampGRI.OnCurrentChapterChanged = OnChapterReplicated;
			CampGRI.OnCheckpointChanged = OnCheckpointReplicated;
			CampGRI.OnCampaignModeChanged = OnCampaignModeReplicated;
			CampGRI.OnInviteTypeChanged = OnInviteTypeReplicated;

			// If the client hasn't initialized replicated values and the options are actually there
			if (!bClientIsInitialized && CampaignModeOption != None)
			{
				ReplicateLobbyFromGRI();
			}
		}
		else
		{
			OwnerPC = GetGearPlayerOwner(TEMP_SPLITSCREEN_INDEX);
			OwnerPC.SetTimer( 0.1f, false, nameof(self.SetupGRICallbacks), self );
		}
	}
}

/**
* Clears all delegates which have been assigned to methods in this class so that those objects
* don't hold a reference to this scene.
*/
function ClearDelegates()
{
	// Only the clients will use the GRI to set their values
	if (!IsHost(GetBestPlayerIndex()) && CampGRI != None)
	{
		CampGRI.OnCurrentChapterChanged = None;
		CampGRI.OnCheckpointChanged = None;
		CampGRI.OnCampaignModeChanged = None;
		CampGRI.OnInviteTypeChanged = None;
	}
}

/** Updates all replicated values from the GRI to the lobby */
function ReplicateLobbyFromGRI()
{
	OnChapterReplicated();
	OnCheckpointReplicated();
	OnCampaignModeReplicated();
	OnInviteTypeReplicated();
	bClientIsInitialized = true;
}

/** ReBuild the DesciptionList for displaying the descriptions for all of the host options */
function RebuildDescriptionList()
{
	// No point in doing anything if there's no description label to deal with
	if ( DescriptionLabel != None )
	{
		// Empty the description list
		DescriptionList.length = 0;

		// Add the Party options to the description list
		AddObjectListElementsToDescriptionList( OptionsList, class'UIDataProvider_MenuItem' );

		// Add the other selections to the description list
		AddMiscWidgetsToDescriptionList();
	}
}

/** Append the other selections to the description list */
function AddMiscWidgetsToDescriptionList()
{
	local int NewIdx, PlayerIdx;
	local string WidgetName;

	if ( StartData.StartCampButton != None )
	{
		NewIdx = DescriptionList.length;
		DescriptionList.length = NewIdx + 1;
		DescriptionList[NewIdx].LocalizationPath = CampaignMode == eGCLOBBYMODE_Join ? JoinGame_Desc : CreateGame_Desc;
		DescriptionList[NewIdx].WidgetToDescribeName = 'btnFindGame';
		DescriptionList[NewIdx].WidgetToDescribe = StartData.StartCampButton;
	}

	for ( PlayerIdx = 0; PlayerIdx < PlayerList.length; PlayerIdx++ )
	{
		if ( PlayerList[PlayerIdx].PlayerButton != None )
		{
			NewIdx = DescriptionList.length;
			DescriptionList.length = NewIdx + 1;
			WidgetName = "btnPlayer" $ PlayerIdx;
			DescriptionList[NewIdx].LocalizationPath = (PlayerList[PlayerIdx].PlayerPRI != None) ? PlayerClickFull_Desc : PlayerClickEmpty_Desc;
			DescriptionList[NewIdx].WidgetToDescribeName = Name(WidgetName);
			DescriptionList[NewIdx].WidgetToDescribe = PlayerList[PlayerIdx].PlayerButton;
		}
	}
}

/** Refresh the description widget */
function RefreshDescriptionWidget()
{
	local string DescribeText;

	DescribeText = GetDynamicDescriptionText( CurrentDescriptionListIndex );
	if ( DescribeText != "" &&
		DescribeText != DescriptionList[CurrentDescriptionListIndex].LocalizationPath )
	{
		DescriptionList[CurrentDescriptionListIndex].LocalizationPath = DescribeText;
		UpdateDescriptionLabel( DescriptionList[CurrentDescriptionListIndex].LocalizationPath, DescriptionList[CurrentDescriptionListIndex].WidgetToDockTo, DescriptionList[CurrentDescriptionListIndex].FaceToDockTo, DescriptionList[CurrentDescriptionListIndex].DockPadding );
	}
}

/** Called when the description goes active for the widget */
function OnDescriptionActive( int DescriptionIndex )
{
	local string DescribeText;

	DescribeText = GetDynamicDescriptionText( DescriptionIndex );
	if ( DescribeText != "" )
	{
		DescriptionList[DescriptionIndex].LocalizationPath = DescribeText;
	}
}

/** Get dynamically changing description text at a particular index */
function string GetDynamicDescriptionText( int DescriptionIndex )
{
	local int CurrentIndex;
	local int PlayerIdx;
	local string WidgetName;
	local UIObject WidgetToDescribe;
	local string DescribeString;

	WidgetToDescribe = None;
	if ( DescriptionList.length > DescriptionIndex )
	{
		WidgetToDescribe = DescriptionList[DescriptionIndex].WidgetToDescribe;
	}

	if (CampaignModeOption != none && CampaignModeOption.IsFocused())
	{
		if (CampaignModeOption.IsExpanded() || CampaignModeOption.IsExpanding())
		{
			CurrentIndex = CampaignModeOption.GetCurrentItem();
			switch (CurrentIndex)
			{
				case eGCM_LivePublic:
					DescribeString = CampModeDesc_LivePublic;
					break;
				case eGCM_LivePrivate:
					DescribeString = CampModeDesc_LivePrivate;
					break;
				case eGCM_SystemLink:
					DescribeString = CampModeDesc_LAN;
					break;
			}
		}
		else
		{
			DescribeString = CampModeDesc_Main;
		}
	}
	else if (CheckpointOption != none && CheckpointOption.IsFocused())
	{
		if (CheckpointOption.IsExpanded() || CheckpointOption.IsExpanding())
		{
			CurrentIndex = CheckpointOption.GetCurrentItem();
			switch (CurrentIndex)
			{
				case eGEARCHECKPOINT_UseLast:
					DescribeString = CheckpointDesc_Check;
					break;
				case eGEARCHECKPOINT_Restart:
					DescribeString = CheckpointDesc_Restart;
					break;
			}
		}
		else
		{
			DescribeString = CheckpointDesc_Main;
		}

	}
	else if (WidgetToDescribe != none)
	{
		for (PlayerIdx = 0; PlayerIdx < PlayerList.length; PlayerIdx++)
		{
			WidgetName = "btnPlayer" $ PlayerIdx;
			if (WidgetToDescribe.WidgetTag == Name(WidgetName))
			{
				DescribeString = (PlayerList[PlayerIdx].PlayerPRI != None) ? PlayerClickFull_Desc : PlayerClickEmpty_Desc;
				break;
			}
		}
	}

	return DescribeString;
}

/**
 * Handler for the completion of this scene's opening animation...
 *
 * @warning - if you override this in a child class, keep in mind that this function will not be called if the scene has no opening animation.
 */
function OnOpenAnimationComplete( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	Super.OnOpenAnimationComplete(Sender, AnimName, TrackTypeMask);

	if (Sender == Self)
	{
		if (GetWorldInfo().NetMode == NM_Client)
		{
			RefreshTooltip(PlayerList[0].PlayerButton);
		}
		else
		{
			RefreshTooltip(StartData.StartCampButton);
		}
	}
}

/** Update the widget styles, strings, etc. */
function UpdateCampaignLobbyScene(float DeltaTime)
{
	if ( bIsDeletingCheckpoint )
	{
		OnOverwriteCorruptedSaveGame_Completed(None);
	}

	// update the spinning cog
	if (CampGRI != None)
	{
		EnableSpinningCOG(CampGRI.bIsInputGuarding);
	}
	UpdateChatIcons();
	RefreshPlayerListData();
	RefreshDescriptionWidget();
}

/** Returns the number of valid PRIs in the GRI's PRIArray */
function int GetNumValidPRIs()
{
	local GearGRI GRI;
	local int Idx, Count;
	local UniqueNetId ZeroId;

	Count = 0;
	GRI = GetGRI();

	if ( GRI != None )
	{
		for ( Idx = 0; Idx < GRI.PRIArray.length; Idx++ )
		{
			if ( GRI.PRIArray[Idx] != None &&
				 !GRI.PRIArray[Idx].bIsInactive &&
				 // This means that have't fully replicated
				 GRI.PRIArray[Idx].UniqueId != ZeroId )
			{
				Count++;
			}
		}
	}

	return Count;
}

/** Whether this is the host or not */
final function bool IsHost( int PlayerIndex )
{
	local GearPC GearPO;

	// the current party session should be the most authoritative, so try that one first
	GearPO = GetGearPlayerOwner(PlayerIndex);
	if ( GearPO != None )
	{
		// First see that there is a proper profile and that the playerindex is 0 (which is guaranteed to be the player driving the menus)
		if ( GearPO.ProfileSettings != None && PlayerIndex == 0 )
		{
			// If there is a game object we are on the server
			if ( GetWorldInfo().Game != None )
			{
				return true;
			}
		}
	}

	return false;
}

/** Returns the campaign mode we are currently in */
final function EGearCampMode GetCampaignMode()
{
	local GearProfileSettings Profile;
	local GearCoopGameSettings CurrentSettings;
	local int SelectedContext;

	if ( IsHost(TEMP_SPLITSCREEN_INDEX) )
	{
		Profile = GetPlayerProfile(TEMP_SPLITSCREEN_INDEX);
		if ( Profile != None )
		{
			Profile.GetProfileSettingValueId(Profile.const.CAMP_MODE, SelectedContext);
		}
	}
	else
	{
		// pull the remote value
		CurrentSettings = GetCoopGameSettings();
		if ( CurrentSettings != None )
		{
			CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.CAMP_MODE, SelectedContext);
		}
	}

	return EGearCampMode(SelectedContext);
}

/** Caches the GRI so we don't have to annoyingly call a function and cast it a billion times */
function CacheGRI()
{
	local GearGRI GRI;
	local GearPC OwnerPC;

	OwnerPC = GetGearPlayerOwner(TEMP_SPLITSCREEN_INDEX);
	if (OwnerPC.PlayerReplicationInfo != None)
	{
		GRI = GetGRI();
		if (GRI != None)
		{
			CampGRI = GearCampaignGRI(GRI);
			return;
		}
	}

	OwnerPC.SetTimer( 0.1f, false, nameof(self.CacheGRI), self );
}

/** Replicates the data in the host's lobby to the GRI for the clients */
final function SetLobbyDataInGRI(optional bool bForceUpdate)
{
	local GearPC OwnerPC;
	local int SelectedContext;
	local EChapterPoint CurrChapter;
	local GearCampaignLobbyGame_Base CampGI;

	OwnerPC = GetGearPlayerOwner(TEMP_SPLITSCREEN_INDEX);

	if (CampGRI != None)
	{
		if (OwnerPC != None && OwnerPC.ProfileSettings != None)
		{
			OwnerPC.ProfileSettings.GetProfileSettingValueId(OwnerPC.ProfileSettings.CampCheckpointUsage, SelectedContext);
			CampGRI.CheckpointUsage = EGearCheckpointUsage(SelectedContext);
			OwnerPC.ProfileSettings.GetProfileSettingValueId(OwnerPC.ProfileSettings.CAMP_MODE, SelectedContext);
			CampGRI.CampaignMode = EGearCampMode(SelectedContext);
			//OwnerPC.ProfileSettings.GetProfileSettingValueId(OwnerPC.ProfileSettings.CoopInviteType, SelectedContext);
			//CampGRI.InviteType = EGearCoopInviteType(SelectedContext);
		}

		CurrChapter = GetCurrentSelectedChapterType();
		if (CurrChapter != CampGRI.CurrChapter || bForceUpdate)
		{
			CampGRI.SetCurrentChapter(CurrChapter);
			CampGI = GetCampGameInfo();
			CampGI.ChangeChapterSettings(CurrChapter);
			if (CampaignMode == eGCLOBBYMODE_Host)
			{
				// Tell live to update the advertised settings
				CampGI.UpdateCoopSettings();
			}
		}
	}
	else
	{
		OwnerPC.SetTimer( 0.1f, false, nameof(self.SetLobbyDataInGRI), self );
	}
}

/** Set the proper campaign mode list of options based on the CampaignMode in the profile */
final function SetMenuItemsInList()
{
	local int PlayerIndex, SelectedContext;
	local GearProfileSettings Profile;
	local GearCoopGameSettings CurrentSettings;

	PlayerIndex = GetBestPlayerIndex();
	if ( IsHost(PlayerIndex) )
	{
		Profile = GetPlayerProfile(PlayerIndex);
		if ( Profile != None && Profile.GetProfileSettingValueId(Profile.const.CAMP_MODE, SelectedContext) )
		{
			switch (CampaignMode)
			{
				case eGCLOBBYMODE_Host:
					if (SelectedContext == eGCM_LivePublic ||
						SelectedContext == eGCM_LivePrivate)
					{
						OptionsList.SetDataStoreBinding("<MenuItems:CampaignLobbyLive_Host>");
					}
					else if (SelectedContext == eGCM_SystemLink)
					{
						OptionsList.SetDataStoreBinding("<MenuItems:CampaignLobbyLAN_Host>");
					}
					else
					{
						`log("Error: GearUISceneFELobby_Campaign:SetMenuItemsInList: Could not resolve option list"@SelectedContext@CampaignMode);
					}
					break;
				case eGCLOBBYMODE_Join:
					if (SelectedContext == eGCM_LivePublic ||
						SelectedContext == eGCM_LivePrivate)
					{
						OptionsList.SetDataStoreBinding("<MenuItems:CampaignLobbyLive_Join>");
					}
					else if (SelectedContext == eGCM_SystemLink)
					{
						OptionsList.SetDataStoreBinding("<MenuItems:CampaignLobbyLAN_Join>");
					}
					else
					{
						`log("Error: GearUISceneFELobby_Campaign:SetMenuItemsInList: Could not resolve option list"@SelectedContext@CampaignMode);
					}
					break;
				case eGCLOBBYMODE_Split:
					OptionsList.SetDataStoreBinding("<MenuItems:CampaignLobby_Split>");
					break;
			}
		}
	}
	else
	{
		CurrentSettings = GetCoopGameSettings();
		if ( CurrentSettings != None && CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.CAMP_MODE, SelectedContext) )
		{
			switch (CampaignMode)
			{
				case eGCLOBBYMODE_Host:
					if (SelectedContext == eGCM_LivePublic ||
						SelectedContext == eGCM_LivePrivate)
					{
						OptionsList.SetDataStoreBinding("<MenuItems:CampaignLobbyLiveClient_Host>");
					}
					else if (SelectedContext == eGCM_SystemLink)
					{
						OptionsList.SetDataStoreBinding("<MenuItems:CampaignLobbyLANClient_Host>");
					}
					else
					{
						`log("Error: GearUISceneFELobby_Campaign:SetMenuItemsInList: Could not resolve option list"@SelectedContext@CampaignMode);
					}
					break;
				case eGCLOBBYMODE_Join:
					if (SelectedContext == eGCM_LivePublic ||
						SelectedContext == eGCM_LivePrivate)
					{
						OptionsList.SetDataStoreBinding("<MenuItems:CampaignLobbyLiveClient_Join>");
					}
					else if (SelectedContext == eGCM_SystemLink)
					{
						OptionsList.SetDataStoreBinding("<MenuItems:CampaignLobbyLANClient_Join>");
					}
					else
					{
						`log("Error: GearUISceneFELobby_Campaign:SetMenuItemsInList: Could not resolve option list"@SelectedContext@CampaignMode);
					}
					break;
				case eGCLOBBYMODE_Split:
					OptionsList.SetDataStoreBinding("<MenuItems:CampaignLobbyClient_Split>");
					break;
			}
		}
	}
}

/** Sets the message/button area of the uber button to its' state */
final function SetMessageState( CampMessageState MessageState )
{
	switch (MessageState)
	{
		case eCMS_Normal:
			StartData.ParentPanel.SetEnabled(false);
			StartData.CogImage.SetVisibility(true);
			StartData.CogLoadImage.SetVisibility(false);
			StartData.CogErrorImage.SetVisibility(false);
			StartData.SubParentPanel.SetEnabled(false);
			StartData.SubParentPanel.SetVisibility(false);
			StartData.MessagePanel.SetEnabled(false);
			StartData.MessagePanel.SetVisibility(false);
			StartData.BloodSplatImage.SetVisibility(true);
			StartData.BloodSplatImage.SetEnabled(true);
			if (CurrMessageState == eCMS_Buttons)
			{
				StartData.StartCampButton.SetFocus(None);
			}
			break;

		case eCMS_Message:
			StartData.ParentPanel.SetEnabled(false);
			StartData.CogImage.SetVisibility(true);
			StartData.CogLoadImage.SetVisibility(false);
			StartData.CogErrorImage.SetVisibility(false);
			StartData.SubParentPanel.SetEnabled(false);
			StartData.SubParentPanel.SetVisibility(true);
			StartData.MessagePanel.SetEnabled(false);
			StartData.MessagePanel.SetVisibility(true);
			StartData.BloodSplatImage.SetVisibility(true);
			StartData.BloodSplatImage.SetEnabled(false);
			if (CurrMessageState == eCMS_Buttons)
			{
				StartData.StartCampButton.SetFocus(None);
			}
			break;

		case eCMS_Buttons:
			StartData.ParentPanel.SetEnabled(true);
			StartData.CogImage.SetVisibility(true);
			StartData.CogLoadImage.SetVisibility(false);
			StartData.CogErrorImage.SetVisibility(false);
			StartData.SubParentPanel.SetEnabled(true);
			StartData.SubParentPanel.SetVisibility(true);
			StartData.MessagePanel.SetEnabled(false);
			StartData.MessagePanel.SetVisibility(false);
			StartData.ButtonPanel.SetEnabled(true);
			StartData.ButtonPanel.SetVisibility(true);
			StartData.JoinGameButton.SetEnabled(true);
			StartData.JoinGameButton.SetVisibility(true);
			StartData.HostGameButton.SetEnabled(true);
			StartData.HostGameButton.SetVisibility(true);
			StartData.HostGameButton.SetFocus(None);
			StartData.BloodSplatImage.SetVisibility(true);
			StartData.BloodSplatImage.SetEnabled(false);
			break;
	}

	CurrMessageState = MessageState;
}

/** Makes the COG turn in the uber-button message area */
final function EnableSpinningCOG(bool bEnable)
{
	StartData.CogImage.SetVisibility(!bEnable);
	StartData.CogLoadImage.SetVisibility(bEnable);
}

/** Called when the current chapter/act is changed */
function OnChapterReplicated()
{
	local int PlayerIndex;
	PlayerIndex = GetBestPlayerIndex();
	RefreshActListData();
	ActsList.SetIndexValue(CampGRI.CurrAct, PlayerIndex);
	UpdateChapterList(CampGRI.NormalizedChapterIndex);
	ChapterList.SetIndexValue(CampGRI.NormalizedChapterIndex, PlayerIndex);
	//`Log( "||=====================================> Replicated chapter from the host " $ string(CampGRI.CurrAct) $ " : " $ CampGRI.CurrChapter $ ", " $ CampGRI.NormalizedChapterIndex );
	RefreshChapterDisplayWidgets( PlayerIndex );
}

/** Called when the checkpoint usage option is changed */
function OnCheckpointReplicated()
{
	CheckpointOption.SetIndexValue(CampGRI.CheckpointUsage, GetBestPlayerIndex());
}

/** Called when the campaign mode option is changed */
function OnCampaignModeReplicated()
{
	CampaignModeOption.SetIndexValue(CampGRI.CampaignMode, GetBestPlayerIndex());
}

/** Called when the invite type option is changed */
function OnInviteTypeReplicated()
{
	InviteTypeOption.SetIndexValue(CampGRI.InviteType, GetBestPlayerIndex());
}

/**
 * Refreshes everything dealing with a chapter update - Checkpoint and Map display
 *
 *  @param bForceLoadCheckpoint - if the chapter matches the checkpoint this will force the checkpoint option to set to "Load Last Checkpoint"
 *  @param bOptionsRegenerated - if this is called from the options being regenerated
 */
final function RefreshChapterData( int PlayerIndex, bool bForceLoadCheckpoint, optional bool bOptionsRegenerated )
{
	if (IsHost(PlayerIndex))
	{
		RefreshOptions( PlayerIndex, bForceLoadCheckpoint, bOptionsRegenerated );
		RefreshChapterDisplayWidgets( PlayerIndex );
		SetLobbyDataInGRI();
	}
}

/**
 * Refreshes the options available in the options list
 *
 *  @param bForceLoadCheckpoint - if the chapter matches the checkpoint this will force the checkpoint option to set to "Load Last Checkpoint"
 *  @param bOptionsRegenerated - if this is called from the options being regenerated
 */
final function RefreshOptions( int PlayerIndex, bool bForceLoadCheckpoint, bool bOptionsRegenerated )
{
	local EChapterPoint SelectedChapter;
	local GearPC MyGearPC;
	local array<UIDataStore> Unused;

	// If we are forcing the load checkpoint option we need to refresh the act and chapter lists
	if ( bForceLoadCheckpoint )
	{
		if ( ActiveCheckpoint == None || ActiveCheckpoint.CheckpointIsEmpty() )
		{
			SetActAndChapterListIndexes( EChapterPoint(0) );
		}
		else
		{
			SetActAndChapterListIndexes( ActiveCheckpoint.Chapter );
		}
	}
	else if ( bOptionsRegenerated )
	{
		SetActAndChapterListIndexes( CampGRI.CurrChapter );
	}

	// See if we should show the load checkpoint option in the list
	SelectedChapter = GetCurrentSelectedChapterType();
	if ( ActiveCheckpoint != None && !ActiveCheckpoint.CheckpointIsEmpty() && ActiveCheckpoint.Chapter == SelectedChapter )
	{
		bShouldHaveLoadOption = true;
	}
	else
	{
		bShouldHaveLoadOption = false;
	}

	if ( CheckpointOption != None )
	{
		if (bShouldHaveLoadOption || bForceCheckpointOnNextRegen)
		{
			// Select the Load checkpoint option
			if (bForceLoadCheckpoint || bForceCheckpointOnNextRegen)
			{
				bForceCheckpointOnNextRegen = false;
				CheckpointOption.SetIndexValue( eGEARCHECKPOINT_UseLast, PlayerIndex );
				CheckpointOption.SaveSubscriberValue(Unused);
			}

			// Set the difficulty if there's a valid checkpoint
			if (bForceLoadCheckpoint && ActiveCheckpoint != None)
			{
				MyGearPC = GetGearPlayerOwner( PlayerIndex );
				if ( MyGearPC != None && MyGearPC.ProfileSettings != None )
				{
					MyGearPC.ProfileSettings.SetProfileSettingValueId(MyGearPC.ProfileSettings.GameIntensity, ActiveCheckpoint.Difficulty);
				}
			}
		}
		else
		{
			// Select the Restart chapter option
			CheckpointOption.SetIndexValue( eGEARCHECKPOINT_Restart, PlayerIndex );
			CheckpointOption.SaveSubscriberValue(Unused);
		}
	}
}

/** Refreshes the map image and difficulties completed images for the currently select chapter */
final function RefreshChapterDisplayWidgets( int PlayerIndex )
{
	local GearCampaignChapterData ChapterProvider;
	local int Idx;
	local GearPC PC;
	local EChapterPoint CurrChapter;
	local bool bShowedAnImage;

	CurrChapter = GetCurrentSelectedChapterType();
	ChapterProvider = class'GearUIDataStore_GameResource'.static.GetChapterDataProvider( CurrChapter );
	if ( ChapterProvider != None )
	{
		if ( CampaignMapImage != None )
		{
			CrossFade(ChapterProvider.ScreenshotPathName);
		}

		CurrentActIndex = GetCurrentlySelectedAct();
		CurrentChapterIndex = GetCurrentSelectedChapter();

		PC = GetGearPlayerOwner(PlayerIndex);
		if ( PC != None && PC.ProfileSettings != None )
		{
			for ( Idx = 0; Idx < DifficultyCompleteImages.length; Idx++ )
			{
				if ( PC.ProfileSettings.HasChapterBeenUnlocked(EChapterPoint(CurrChapter+1), EDifficultyLevel(Idx)) || !IsConsole() )
				{
					DifficultyCompleteImages[Idx].SetVisibility( true );
					bShowedAnImage = true;
				}
				else
				{
					DifficultyCompleteImages[Idx].SetVisibility( false );
				}
			}
		}
	}
	// Only show the background image for the difficulty images if one is showing
	DifficultyBGImage.SetVisibility(bShowedAnImage);
}

/** Sets the indexes of the act and chapter lists to what's in the currently active checkpoint */
final function SetActAndChapterListIndexes( EChapterPoint ChapType )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue Value, ActValue;
	local int ProviderIndex, ChapterIndex, ActIndex, PlayerIndex;
	local array<UIResourceDataProvider> ChapterProviders;
	local GearCampaignChapterData ChapterData;

	if ( ChapterList != None && ActsList != None )
	{
		GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
		if ( GameResourceDS != None )
		{
			// Grab the chapter providers
			if ( GameResourceDS.GetResourceProviders('Chapters', ChapterProviders) )
			{
				Value.PropertyTag = 'ChapterType';
				Value.PropertyType = DATATYPE_Property;
				Value.StringValue = string(ChapType);

				// Find the index to the provider with ChapType chapterpoint
				ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Chapters', 'ChapterType', Value);

				if ( ProviderIndex != INDEX_NONE )
				{
					// Get the chapter provider so we can then find the act
					ChapterData = GearCampaignChapterData(ChapterProviders[ProviderIndex]);
					if ( ChapterData != None )
					{
						ActValue.PropertyTag = 'ActType';
						ActValue.PropertyType = DATATYPE_Property;
						ActValue.StringValue = string(ChapterData.ActType);

						// Find the index to the provider with ChapterData's ActType
						ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Acts', 'ActType', ActValue);
						if ( ProviderIndex != INDEX_NONE )
						{
							// Set the act list index
							ActIndex = ProviderIndex;
							ChapterIndex = GameResourceDS.GetActChapterProviderIndexFromChapterId(ChapType);
							PlayerIndex = GetBestPlayerIndex();

							RefreshActListData();
							ActsList.SetIndexValue( ActIndex, PlayerIndex );
							UpdateChapterList();
							ChapterList.SetIndexValue( ChapterIndex, PlayerIndex );
						}
					}
				}
			}
		}
	}
}


/**
 * Allows the widget to force specific elements to be disabled.  If not implemented, or if the return value
 * is false, the list's data provider will then be given an opportunity to disable the item.
 *
 * @param	Sender			the list calling the delegate
 * @param	ElementIndex	the index [into the data store's list of items] for the item to query
 *
 * @return	TRUE if the specified element should be disabled.
 */
function bool ShouldDisableElementCheckpointOption( UIList Sender, int ElementIndex )
{
	if ( ElementIndex == eGEARCHECKPOINT_UseLast )
	{
		return !bShouldHaveLoadOption;
	}

	return false;
}

/**
 * Need to disable the Private setting for when the lobby is in "Join" mode
 * Allows the widget to force specific elements to be disabled.  If not implemented, or if the return value
 * is false, the list's data provider will then be given an opportunity to disable the item.
 *
 * @param	Sender			the list calling the delegate
 * @param	ElementIndex	the index [into the data store's list of items] for the item to query
 *
 * @return	TRUE if the specified element should be disabled.
 */
function bool ShouldDisableElementCampMode( UIList Sender, int ElementIndex )
{
	if (CampaignMode == eGCLOBBYMODE_Join &&
		ElementIndex == eGCM_LivePrivate)
	{
		return true;
	}
	return false;
}

/**
 * Allows the widget to force specific elements to be disabled.  If not implemented, or if the return value
 * is false, the list's data provider will then be given an opportunity to disable the item.
 *
 * @param	Sender			the list calling the delegate
 * @param	ElementIndex	the index [into the data store's list of items] for the item to query
 *
 * @return	TRUE if the specified element should be disabled.
 */
function bool ShouldDisableElementAct( UIList Sender, int ElementIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> ActProviders;
	local GearCampaignActData ActData;
	local int PlayerIndex;
	local GearPC GPC;

	if ( bConstrainChapterToProfile )
	{
		GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
		if ( GameResourceDS != None )
		{
			// Grab the act providers
			if ( GameResourceDS.GetResourceProviders('Acts', ActProviders) )
			{
				ActData = GearCampaignActData(ActProviders[ElementIndex]);
				if ( ActData != None )
				{
					PlayerIndex = GetBestPlayerIndex();
					GPC = GetGearPlayerOwner(PlayerIndex);
					if ( GPC != None && GPC.ProfileSettings != None )
					{
						return !GPC.ProfileSettings.HasActBeenUnlockedForAccess( ActData.ActType );
					}
				}
			}
		}
	}

	return false;
}

/**
 * Allows the widget to force specific elements to be disabled.  If not implemented, or if the return value
 * is false, the list's data provider will then be given an opportunity to disable the item.
 *
 * @param	Sender			the list calling the delegate
 * @param	ElementIndex	the index [into the data store's list of items] for the item to query
 *
 * @return	TRUE if the specified element should be disabled.
 */
function bool ShouldDisableElementChapter( UIList Sender, int ElementIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local GearCampaignChapterData ChapterData;
	local GearCampaignActData ActData;
	local int PlayerIndex;
	local GearPC GPC;

	if ( bConstrainChapterToProfile )
	{
		GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
		if ( GameResourceDS != None )
		{
			ActData = GameResourceDS.GetActDataProvider(EGearAct(GetCurrentlySelectedAct()));
			if (ActData != none)
			{
				if (ElementIndex < ActData.ChapterProviders.length)
				{
					ChapterData = ActData.ChapterProviders[ElementIndex];
					if ( ChapterData != None )
					{
						PlayerIndex = GetBestPlayerIndex();
						GPC = GetGearPlayerOwner(PlayerIndex);
						if ( GPC != None && GPC.ProfileSettings != None )
						{
							return !GPC.ProfileSettings.HasChapterBeenUnlockedForAccess( ChapterData.ChapterType );
						}
					}
				}
			}
		}
	}

	return false;
}

/** Set a local reference to the active checkpoint */
final function SetCurrentCheckpointReference()
{
	local int SaveSlot, PlayerIndex;
	local GearPC CurrGearPC;
	local GearEngine Engine;
	local CheckpointEnumerationResult EnumResult;

	PlayerIndex = GetBestPlayerIndex();
	CurrGearPC = GetGearPlayerOwner(PlayerIndex);
	if ( CurrGearPC != None && CurrGearPC.ProfileSettings != None )
	{
		CurrGearPC.ProfileSettings.GetProfileSettingValueInt(CurrGearPC.ProfileSettings.LastCheckpointSlot, SaveSlot);
		SaveSlot = Max(0, SaveSlot);

		// Prepare the checkpoint system for access so we can grab a reference to the current checkpoint
		if ( PrepareCheckpointSystem(PlayerIndex) )
		{
			Engine = GearEngine(CurrGearPC.Player.Outer);
			if ( Engine != None )
			{
				// Get the current checkpoint
				if ( Engine.FindCheckpointData(SaveSlot, EnumResult) )
				{
					`assert(EnumResult.bCheckpointFileExists[SaveSlot] == 0 || EnumResult.bCheckpointFileCorrupted[SaveSlot] == 0);
					ActiveCheckpoint = Engine.CurrentCheckpoint;
				}
				else if ( EnumResult.bCheckpointFileCorrupted[SaveSlot] != 0 )
				{
					ActiveCheckpoint = None;
				}
				else
				{
					`warn("UNHANDLED ERROR FROM FindCheckpointData for SaveSlot" @ SaveSlot);
				}
			}
		}
	}
}

/** Return from the prompt asking if the player wanted to overwrite their saved campaign */
function bool OnOverwriteCorruptedSaveGame_Selected( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local GearEngine GE;
	local GearPC GPC;

	GE = GetGearEngine();
	if ( SelectedInputAlias == 'AcceptOverwrite' )
	{
		GE.PendingCheckpointAction = Checkpoint_DeleteAll;
		Sender.OnSceneDeactivated = OnOverwriteCorruptedSaveGame_Completed;
	}
	else
	{
		// set the flag to indicate that the user chose to not save checkpoints
		GE.bShouldWriteCheckpointToDisk = false;

		// then update the profile to indicate that we can't use the last checkpoint
		GPC = GetGearPlayerOwner(PlayerIndex);
		GPC.ProfileSettings.SetProfileSettingValueId(GPC.ProfileSettings.CampCheckpointUsage, eGEARCHECKPOINT_Restart);

		// finally, unset the flag that this scene uses for determininig whether checkpoints can be used.
		bUsingCheckpoint = false;

		RefreshChapterData(PlayerIndex, false, false);

		// Stop guarding all input and making COG spin
		SetLobbyToInputGuardMode(false);
	}

	return true;
}

/**
 * Called when the 'overwrite corrupted savegame' prompt finishes closing and the corrupted savegame has been deleted.
 */
function OnOverwriteCorruptedSaveGame_Completed( UIScene ClosedScene )
{
	local GearEngine GE;
	local GearPC GPC;
	local int PlayerIndex;

	GE = GetGearEngine();
	if ( GE.PendingCheckpointAction != Checkpoint_DeleteAll )
	{
		bIsDeletingCheckpoint = false;
		PlayerIndex = GetPlayerOwnerIndex();

		// then update the profile to indicate that we can't use the last checkpoint
		GPC = GetGearPlayerOwner(PlayerIndex);
		GPC.ProfileSettings.SetProfileSettingValueId(GPC.ProfileSettings.CampCheckpointUsage, eGEARCHECKPOINT_Restart);

		RefreshChapterData(PlayerIndex, false, false);

		// Stop guarding all input and making COG spin
		SetLobbyToInputGuardMode(false);
	}
	else
	{
		// otherwise, we'll take care of it in tick
		bIsDeletingCheckpoint = true;
	}
}

/**
 * Wrapper for determining whether a player is the match host.
 */
function bool IsLobbyHost( int PlayerIndex )
{
	local GearPC GPC;

	GPC = GetGearPlayerOwner(PlayerIndex);
	if ( GPC != None )
	{
		return (GPC.Role == ROLE_Authority);
	}

	return false;
}

/** Accessor which returns the index [into the array of campaign act data providers] of the currently selected act */
function int GetCurrentlySelectedAct()
{
	return ActsList.GetCurrentItem();
}

/** Accessor which returns the index [into the currently selected act's array of chapter data providers] of the currently selected chapter */
function int GetCurrentSelectedChapter()
{
	return ChapterList.GetCurrentItem();
}

/**
 * Get the enum of the chapter.
 *
 * @param SelectedAct the index of the Act within the dropdown list
 * @param SelectedChapter the index of the Chapter within the dropdown list
 *
 * @return the type of chapter currently selected
 */
final function EChapterPoint GetChapterType( int SelectedAct, int SelectedChapter )
{
	local string ChapterProviderMarkupString, EnumString;

	ChapterProviderMarkupString = GenerateChapterProviderMarkup("ChapterType", SelectedAct, SelectedChapter);
	if ( GetDataStoreStringValue(ChapterProviderMarkupString, EnumString, Self, GetPlayerOwner()) )
	{
		return EChapterPoint(ConvertEnumStringToValue( EnumString, CHAP_MAX, enum'EChapterPoint'));
	}
	else
	{
		`log(`location @ "failed to retrieve chapter index -" @ `showvar(ChapterProviderMarkupString) @ `showvar(SelectedAct) @ `showvar(SelectedChapter));
	}

	return EChapterPoint(0);
}


/** Returns the enum of the currently selected chapter */
final function EChapterPoint GetCurrentSelectedChapterType()
{
	local int SelectedAct, SelectedChapter;
	SelectedAct = GetCurrentlySelectedAct();
	SelectedChapter = GetCurrentSelectedChapter();

	return GetChapterType(SelectedAct, SelectedChapter);
}

/**
 * Handler for the ChapterList's OnPostSceneUpdate delegate - ensures that the list's column widths are set correctly using up-to-date
 * values for the width of the list.
 */
function PostSceneUpdate( UIObject Sender )
{
	// first, disable scene updates until it's needed again
	if ( Sender == OptionsList )
	{
		OptionsList.bEnableSceneUpdateNotifications = false;
		OptionsList.OnPostSceneUpdate = None;

		UpdateListColumnSizes(ActsList);
		UpdateListColumnSizes(ChapterList);
	}
}

/**
 * Wrapper for generating a datastore markup string which can be used to access a field in a particular chapter data provider.
 *
 * @param	ChapterField	the field to access in the chapter data provider.
 * @param	DesiredAct		the index into the list of campaign act data providers for the act to access.
 * @param	DesiredChapter	the index into the list of chapter data providers for the chapter to access.
 *
 * @return	TRUE if the markup was generated successfully.
 */
function string GenerateChapterProviderMarkup( string ChapterField, int DesiredAct, optional int DesiredChapter=INDEX_NONE )
{
	local string Result;

	if ( ChapterField != "" )
	{
		if (DesiredChapter == INDEX_NONE)
		{
			Result = "<DynamicGameResource:Acts;" $ DesiredAct $ ".ChapterProviders>";
		}
		else
		{
			Result = "<DynamicGameResource:Acts;" $ DesiredAct $ ".ChapterProviders;" $ DesiredChapter $ "." $ ChapterField $ ">";
		}

	}

	return Result;
}

/**
 * Refreshes the data binding for the acts list and sets up the column bindings.
 */
function RefreshActListData()
{
	local string ColumnHeader;

	if ( ActsList != None )
	{
		ActsList.OnValueChanged = None;
		ActsList.SetDataStoreBinding("<DynamicGameResource:Acts.ListName>", class'GearUICollapsingSelectionList'.const.COLLAPSESELECTION_LIST_DATABINDING_INDEX);

		ColumnHeader = "";
		if ( !ActsList.SetCellBinding('ListName', ColumnHeader, 0) )
		{
			`log("FAILED TO SET CELLBINDING FOR COLUMN 0 on list" @ ActsList);
		}

		if ( !ActsList.SetCellBinding('DifficultyCompletedIcon', ColumnHeader, 1) )
		{
			`log("FAILED TO SET CELLBINDING FOR COLUMN 1 on list" @ ActsList);
		}

		UpdateListColumnSizes(ActsList);
		ActsList.OnValueChanged = ActSelectionChanged;
	}
}

/**
 * Repopulates the list of chapters using the currently selected act.  Optionally initializes the chapter list's selected chapter to a
 * specific value.
 *
 * @param	SelectedChapterCollectionIndex	allows the caller to specify which chapter should be set as the list's intiially selected item.
 */
function UpdateChapterList( optional int SelectedChapterCollectionIndex )
{
	local int SelectedActCollectionIndex;
	local string ChapterListMarkup, ColumnHeader;

	if ( ActsList != None )
	{
		// Disable callbacks to prevent portrait fades popping.
		ChapterList.OnValueChanged=None;

		SelectedActCollectionIndex = ActsList.GetCurrentItem();
		ChapterListMarkup = GenerateChapterProviderMarkup("ListName", SelectedActCollectionIndex, SelectedChapterCollectionIndex);
		ChapterList.SetDataStoreBinding( ChapterListMarkup, class'GearUICollapsingSelectionList'.const.COLLAPSESELECTION_LIST_DATABINDING_INDEX );

		if ( ChapterList.SetCellBinding('ListName', ColumnHeader, 0) )
		{
		}
		else
		{
			`log("FAILED TO SET CELLBINDING FOR COLUMN 0 on list" @ ChapterList);
		}

		if ( ChapterList.SetCellBinding('DifficultyCompletedIcon', ColumnHeader, 1) )
		{
			UpdateListColumnSizes(ChapterList);
		}
		else
		{
			`log("FAILED TO SET CELLBINDING FOR COLUMN 1 on list" @ ChapterList);
		}

		// Re-enable the callbacks to get portrait fading back.
		ChapterList.OnValueChanged=MapSelectionChanged;
	}
}

/**
 * Wrapper for setting the column sizes of the act and chapter lists, so that the first column fills the width of the entire list minus
 * the size of the column for the difficulty completion icons.
 */
function UpdateListColumnSizes( GearUICollapsingSelectionList TargetList )
{
	local float ListWidth;
	local float HorzPadding;
	local UIStyle AppliedStyle;
	local UIStyle_Combo ListStyleData;
	local UIStyle_Text CellStyle;

	ListWidth = TargetList.GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport);

	AppliedStyle = TargetList.GlobalCellStyle[ELEMENT_Normal].ResolvedStyle;
	if ( AppliedStyle != None )
	{
		ListStyleData = UIStyle_Combo(AppliedStyle.GetStyleForState(TargetList.GetCurrentState()));
		CellStyle = ListStyleData.GetComboTextStyle();

		HorzPadding = CellStyle.StylePadding[UIORIENT_Horizontal];
	}

//	`log(`location @ ">>>>" @ `showobj(TargetList) @ `showvar(ListWidth) @ `showvar(TargetList.CellDataComponent.GetSchemaCellSize(0),CurrentNameCellSize) @ `showvar(TargetList.CellDataComponent.GetSchemaCellSize(1),CurrentIconCellSize),,'REMOVE_ME');
	// the magic value 40 represents the width of the largest difficulty icon, in pixels
	TargetList.CellDataComponent.SetSchemaCellSize(0, ListWidth - (TargetList.ListIndent.Value + 40.f + HorzPadding));
	TargetList.CellDataComponent.SetSchemaCellSize(1, 40.f + HorzPadding);
//	`log(`location @ "<<<<" @ `showobj(TargetList) @ `showvar(ListWidth) @ `showvar(TargetList.CellDataComponent.GetSchemaCellSize(0),NewNameCellSize) @ `showvar(TargetList.CellDataComponent.GetSchemaCellSize(1),NewIconCellSize),,'REMOVE_ME');
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Handler for the OnValueChanged delegate of various children in this scene.
 *
 * @param	Sender			the UIObject whose value changed
 * @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 */
function OptionValueChanged( UIScreenObject Sender, name OptionName, int PlayerIndex )
{
//	`log(`location@`showvar(OptionName)@`showvar(PlayerIndex)@`showobj(Sender),,'RON_DEBUG');
}

/**
 * Handler for the OnSubmitSelection delegate of list children in this scene.
 *
 * @param	Sender			the list that submitted a valuewhose value changed
 * @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 */
function ListValueSubmitted( UIList Sender, name OptionName, int PlayerIndex )
{
	local GearProfileSettings Profile;
	local int SelectedContext;
	local UIDataStorePublisher Publisher;
	local array<UIDataStore> Unused;

	// always publish the value immediately
	Publisher = UIDataStorePublisher(Sender);
	if ( Publisher != None )
	{
		Publisher.SaveSubscriberValue(Unused);
	}

	switch (OptionName)
	{
		case 'ActsOption':
			UpdateChapterList();
			// If we are selecting the Act that we're already on
			if ( GetCurrentlySelectedAct() == CurrentActIndex )
			{
				ChapterList.SetIndexValue(CurrentChapterIndex, PlayerIndex);
			}
			RefreshChapterData( PlayerIndex, false );
			break;

		case 'ChapterOption':
			RefreshChapterData( PlayerIndex, false );
			break;

		case 'CampModeOption':
			Profile = GetPlayerProfile(PlayerIndex);
			if (Profile != None && Profile.GetProfileSettingValueId(Profile.const.CAMP_MODE, SelectedContext))
			{
				// Make sure to not do anything if the mode was reselected
				if (SelectedContext != PreviousCampMode)
				{
					ApplyCampaignModeUpdate(PlayerIndex);
				}
			}
			break;

		case 'CheckpointOption':
			SetLobbyDataInGRI();
			break;
	}
}

/** Publishes the current invite type to the session object */
function ApplyInviteTypeUpdate( int PlayerIndex, EGearCoopInviteType NewInviteType )
{
	local GearCampaignLobbyGame_Base CampGI;

	CampGI = GetCampGameInfo();
	if ( CampGI != None )
	{
		// Set the coop setting for the new invite policy
		CampGI.ChangeCoopInviteSetting(CampGI.CoopGameSettings, NewInviteType);

		// Update the GRI and Live
		SetLobbyDataInGRI(true);

		// Tell live to update the advertised settings
		CampGI.UpdateCoopSettings();
	}
}

/** Publishes the current campaign mode to the session object */
function ApplyCampaignModeUpdate( int PlayerIndex )
{
	local GearProfileSettings Profile;
	local int SelectedContext;

	Profile = GetPlayerProfile(PlayerIndex);
	if (Profile != None && Profile.GetProfileSettingValueId(Profile.const.CAMP_MODE, SelectedContext))
	{
		switch ( SelectedContext )
		{
		case eGCM_LivePublic:
			if (PreviousCampMode == eGCM_LivePrivate)
			{
				ApplyInviteTypeUpdate(PlayerIndex, eGCIT_Public);
				PreviousCampMode = SelectedContext;
			}
			else
			{
				AttemptLIVETransition(Profile);
			}
			break;

		case eGCM_LivePrivate:
			if (PreviousCampMode == eGCM_LivePublic)
			{
				ApplyInviteTypeUpdate(PlayerIndex, eGCIT_FriendsOnly);
				PreviousCampMode = SelectedContext;
			}
			else
			{
				AttemptLIVETransition(Profile);
			}
			break;

		case eGCM_SystemLink:
			AttemptSystemLinkTransition();
			break;
		}
	}

	SetCampaignButtonString();
}

/**
 * First checks to see if there are any local players who do not have LIVE permissions and rejects the change if so
 * Next it checks to see if there are any non-local players who need kicked and prompts for the kick
 * Next it transitions to the LIVE match mode
 */
function AttemptLIVETransition( GearProfileSettings Profile )
{
	local GearCampaignLobbyGame_Base CampGI;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local int PlayerIndex;
	local GameViewportClient VPClient;

	CampGI = GetCampGameInfo();
	PlayerIndex = GetBestPlayerIndex();
	VPClient = CampGI.GetPlayerOwnerViewportClient(PlayerIndex);

	// Return if we can't get a GameViewportClient or UI Controller
	if ( VPClient == None || VPClient.UIController == None )
	{
		return;
	}

	// Check for a network connection first
	if ( !VPClient.UIController.HasLinkConnection() )
	{
		DisplayErrorMessage("NoLinkConnection_Message", "NoLinkConnection_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
		CancelLIVECampSelection();
		return;
	}

	// Check for LIVE logins for all local players
	if ( VPClient.UIController.GetLowestLoginStatusOfControllers() != LS_LoggedIn )
	{
		DisplayErrorMessage("NeedGoldTier_Message", "NeedGoldTier_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
		CancelLIVECampSelection();
		return;
	}

	// Check for whether all local players can play online
	if ( !VPClient.UIController.CanAllPlayOnline() )
	{
		DisplayErrorMessage("NeedOnlinePermission_Message", "NeedOnlinePermission_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
		CancelLIVECampSelection();
		return;
	}

	// Check for whether the NAT is open.  If so warn them but don't fail
	if ( !CampGI.CanHostCampaign() )
	{
		DisplayErrorMessage("StrictNAT_Message", "StrictNAT_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
	}

	// If the camp mode was system link, we need to check for kicking a non-local player and then rebuilding the game
	if ( PreviousCampMode == eGCM_SystemLink )
	{
		// Prompt a warning if there are non-local players as they will be kicked
		if ( CampaignHasNonLocalPlayers() )
		{
			ButtonAliases.AddItem('GenericCancel');
			ButtonAliases.AddItem('GenericAccept');
			GameSceneClient = GetSceneClient();
			GameSceneClient.ShowUIMessage(
				'ConfirmNetworkedPlayersKicked',
				"<Strings:GearGameUI.MessageBoxStrings.NetworkPlayersKicked_Title>",
				"<Strings:GearGameUI.MessageBoxStrings.NetworkPlayersKicked_Desc>",
				"",
				ButtonAliases,
				OnKickNetworkedPlayersForLIVE_Confirm,
				GetPlayerOwner(GetBestPlayerIndex()) );
		}
		// Else go ahead and make the transition
		else
		{
			TransitionToLIVECampaign();
		}
	}
	// The match was Local so we need to rebuild the party
	else
	{
		TransitionToLIVECampaign();
	}
}

/** Make the setting go back to what it was before the host selected a LIVE seleciton */
function CancelLIVECampSelection()
{
	local GearProfileSettings Profile;

	Profile = GetPlayerProfile(GetBestPlayerIndex());
	if ( Profile != None )
	{
		// Reset the profile to the previous camp mode
		Profile.SetProfileSettingValueId( Profile.const.CAMP_MODE, PreviousCampMode );
		CampaignModeOption.RefreshSubscriberValue();
	}
}

/** Callback from asking the host if they really want to kick non-local players */
function bool OnKickNetworkedPlayersForLIVE_Confirm( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'GenericAccept' )
	{
		TransitionToLIVECampaign();
	}
	else
	{
		CancelLIVECampSelection();
	}

	return true;
}

/** Transitions to the LIVE mode set in the profile by recreating the party */
function TransitionToLIVECampaign()
{
	// Send prompts and kick non-local players
	PromptAndKickNonLocalPlayers();

	// If the previous match mode was system link we have to destroy the party
	if ( PreviousCampMode == eGCM_SystemLink )
	{
		DestroySystemLinkGameForLIVEGame();
	}
	else
	{
		CreateLIVECampaign(GetBestPlayerIndex());
	}
}

/** Finds all non-local players, prompts them that they are being booted, and then boots them */
function PromptAndKickNonLocalPlayers()
{
	local int PRIIdx;
	local PlayerReplicationInfo CurrPRI;
	local GearPC CurrController;

	// Kick non-local players
	if ( CampGRI != None )
	{
		for ( PRIIdx = 0; PRIIdx < CampGRI.PRIArray.length; PRIIdx++ )
		{
			CurrPRI = CampGRI.PRIArray[PRIIdx];
			if ( CurrPRI != None && !CurrPRI.bIsInactive )
			{
				CurrController = GearPC(CurrPRI.Owner);
				if ( CurrController != None && !CurrController.IsLocalPlayerController() )
				{
					// Send a warning to the client
					CurrController.ClientSetProgressMessage( PMT_ConnectionFailure,
						"<Strings:GearGameUI.MessageBoxStrings.KickedForPartyKill_Desc>",
						"<Strings:GearGameUI.MessageBoxStrings.KickedForPartyKill_Title>",
						true );

					// Kick the player
					CurrController.ClientReturnToMainMenu();
				}
			}
		}
	}
}

/** Destroys a system link game to build a LIVE game */
function DestroySystemLinkGameForLIVEGame()
{
	local GearCampaignLobbyGame_Base CampGI;

	CampGI = GetCampGameInfo();
	CampGI.GameInterface.AddDestroyOnlineGameCompleteDelegate(OnDestroySystemLinkForLIVEGameComplete);
	if (CampGI.GameInterface.DestroyOnlineGame('Party'))
	{
		CampGI.OpenUpdatingPartyScene();
	}
}

/**
 * Delegate fired when destroying an online game has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnDestroySystemLinkForLIVEGameComplete(name SessionName,bool bWasSuccessful)
{
	local GearCampaignLobbyGame_Base CampGI;

	CampGI = GetCampGameInfo();
	CampGI.GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroySystemLinkForLIVEGameComplete);
	CampGI.CoopGameSettings = None;
	CampGI.CloseUpdatingPartyScene();
	CreateLIVECampaign(GetBestPlayerIndex());
}

/** Creates the LIVE game and updates the UI */
function CreateLIVECampaign( int PlayerIndex )
{
	local GearCampaignLobbyGame_Base CampGI;
	local GearProfileSettings Profile;

	CampGI = GetCampGameInfo();
	// Create the LIVE game
	if (CampaignMode == eGCLOBBYMODE_Host)
	{
		if (!CampGI.RecreateCampaign(class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex), false, CampGRI.CurrChapter))
		{
			// TODO: robm show error message about network being down
		}
		// Update the GRI and Live
		SetLobbyDataInGRI();
	}

	// Get the new camp mode from the profile since a failure to create the LIVE party result in a local party
	Profile = GetPlayerProfile(PlayerIndex);
	if ( Profile != None )
	{
		Profile.GetProfileSettingValueId( Profile.const.CAMP_MODE, PreviousCampMode );
	}

	SetMessageState(eCMS_Normal);

	// Update the lobby
	SetMenuItemsInList();
}

/** Checks to see if there are any non-local players and sends a warning, else makes the transition */
function AttemptSystemLinkTransition()
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	// Prompt a warning if there are non-local players as they will be kicked
	if ( CampaignHasNonLocalPlayers() )
	{
		ButtonAliases.AddItem('GenericCancel');
		ButtonAliases.AddItem('GenericAccept');
		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage(
			'ConfirmNetworkedPlayersKicked',
			"<Strings:GearGameUI.MessageBoxStrings.NetworkPlayersKicked_Title>",
			"<Strings:GearGameUI.MessageBoxStrings.NetworkPlayersKicked_Desc>",
			"",
			ButtonAliases,
			OnKickNetworkedPlayersForSystemLink_Confirm,
			GetPlayerOwner(GetBestPlayerIndex()));
	}
	// Else go ahead and make the transition
	else
	{
		TransitionToSystemLink();
	}
}

/** Callback from asking the host if they really want to kick non-local players */
function bool OnKickNetworkedPlayersForSystemLink_Confirm( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'GenericAccept' )
	{
		TransitionToSystemLink();
	}
	else
	{
		CancelSystemLinkSelection();
	}

	return true;
}

/** Kicks non-local players, tells the gameinfo to shut the game session down, switches the invite policy, opens the LAN scene */
function TransitionToSystemLink()
{
	PromptAndKickNonLocalPlayers();

	// Kill the game session
	DestroyCampaignForSystemLink();
}

/** Destroys a campaign to transition to system link */
function DestroyCampaignForSystemLink()
{
	local GearCampaignLobbyGame_Base CampGI;

	CampGI = GetCampGameInfo();
	CampGI.GameInterface.AddDestroyOnlineGameCompleteDelegate(OnDestroyCampaignForSystemLinkComplete);
	if (CampGI.GameInterface.DestroyOnlineGame('Party'))
	{
		CampGI.OpenUpdatingPartyScene();
	}
	else
	{
		CampGI.GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroyCampaignForSystemLinkComplete);
	}
}

/**
 * Delegate fired when a destroying an online game has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnDestroyCampaignForSystemLinkComplete(name SessionName,bool bWasSuccessful)
{
	local GearCampaignLobbyGame_Base CampGI;

	CampGI = GetCampGameInfo();
	CampGI.GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroyCampaignForSystemLinkComplete);
	CampGI.CoopGameSettings = None;
	CampGI.CloseUpdatingPartyScene();

	if (CampaignMode == eGCLOBBYMODE_Host)
	{
		bWantsToStartLANCampaign = true;
		OnLANSceneClosed(none);
	}
	else
	{
		OpenLANScene();
	}
}

/** Opens the LAN scene */
function OpenLANScene()
{
	local UIScene FoundLANScene;
	local GameUISceneClient GameSceneClient;
	local UIScene SceneInstance;

	// Open the LAN screen
	GameSceneClient = GetSceneClient();
	if ( GameSceneClient != None )
	{
		FoundLANScene = UIScene(FindObject("UI_Scenes_Lobby.LANGame", class'UIScene'));
		if ( FoundLANScene != None )
		{
			SceneInstance = OpenScene(FoundLANScene, GetPlayerOwner(GetBestPlayerIndex()),,, LANSceneActivationComplete);
			if ( SceneInstance != None )
			{
				bWantsToStartLANCampaign = false;
				bCanceledLANScene = false;
				SceneInstance.OnSceneDeactivated = OnLANSceneClosed;
				GearUISceneFE_LAN(SceneInstance).OnStartLANParty = OnLANSceneCreateCampaign;
				GearUISceneFE_LAN(SceneInstance).OnCancelFromLANScene = OnCancelLANScene;
			}
		}
	}
}

/** Called when the LAN scene pops up */
function LANSceneActivationComplete( UIScene ActivatedScene, bool bInitialActivation )
{
	GearUISceneFE_LAN(ActivatedScene).bIsCoopLAN = true;
	GearUISceneFE_LAN(ActivatedScene).SceneActivationComplete(ActivatedScene, bInitialActivation);
}

/** Called if the player chose to start their own campaign from the LAN scene */
function OnLANSceneCreateCampaign()
{
	bWantsToStartLANCampaign = true;
}

/** Called if the player canceled from the LAN scene */
function OnCancelLANScene()
{
	bCanceledLANScene = true;
}

/** Called when the LAN screen is closed */
function OnLANSceneClosed( UIScene ClosedScene )
{
	local GearCampaignLobbyGame_Base CampGame;
	local GearProfileSettings Profile;

	// If the player chose to create their own party then do so
	if ( bWantsToStartLANCampaign )
	{
		CampGame = GetCampGameInfo();
		if ( CampGame != None )
		{
			if (CampaignMode == eGCLOBBYMODE_Host)
			{
				if (!CampGame.RecreateCampaign(GetBestControllerId(), true, CampGRI.CurrChapter))
				{
					// TODO: robm show error message about network being down
				}
			}

			// Get the new camp mode from the profile since a failure to create the LAN party results in a local party
			Profile = GetPlayerProfile(GetBestPlayerIndex());
			if ( Profile != None )
			{
				Profile.GetProfileSettingValueId( Profile.const.CAMP_MODE, PreviousCampMode );
			}
		}
	}
	else if ( bCanceledLANScene )
	{
		return;
	}

	// Keep track of the last camp mode
	PreviousCampMode = eGCM_SystemLink;
	SetMessageState(eCMS_Normal);

	// Update the lobby
	SetMenuItemsInList();
}

/**
 * Make the setting go back to what it was before the host selected systemlink
 */
function CancelSystemLinkSelection()
{
	local GearProfileSettings Profile;

	Profile = GetPlayerProfile(GetBestPlayerIndex());
	if ( Profile != None )
	{
		// Reset the profile to the previous camp mode
		Profile.SetProfileSettingValueId( Profile.const.CAMP_MODE, PreviousCampMode );
		CampaignModeOption.RefreshSubscriberValue();
	}
}

/** Whether there are any non-local players in the lobby */
function bool CampaignHasNonLocalPlayers()
{
	local int PRIIdx;
	local PlayerReplicationInfo CurrPRI;
	local GearPC CurrController;

	if ( CampGRI != None )
	{
		for ( PRIIdx = 0; PRIIdx < CampGRI.PRIArray.length; PRIIdx++ )
		{
			CurrPRI = CampGRI.PRIArray[PRIIdx];
			if ( CurrPRI != None && !CurrPRI.bIsInactive )
			{
				CurrController = GearPC(CurrPRI.Owner);
				if ( CurrController != None && !CurrController.IsLocalPlayerController() )
				{
					return true;
				}
			}
		}
	}

	return false;
}

/**
 * Called when the campaig mode selection changes (e.g. the list is open and the selection moved up or down)
 *
 * We want to crossfade the maps as the user is switching between them
 *
 * @param	Sender	the list that is submitting the selection
 * @param	PlayerIndex the player who caused this action
 */
private function CheckpointSelectionChanged( UIObject Sender, int PlayerIndex  )
{
}

/** Called after the options are regenerated but before the scene is updated */
function OnRegeneratedOptionsCallback(GearUIObjectList ObjectList)
{
	local int Idx, PlayerIndex;
	local int LoadCheckpointUsage;
	local GearPC CurrGearPC;
	local bool bIsHost;
	local GearUICollapsingSelectionList FirstWidgetToNavLoop;
	local GearUICollapsingSelectionList LastWidgetToNavLoop;
	local int ChildIdx;

	RebuildDescriptionList();

	if ( OptionsList != None )
	{
		bIsHost = IsHost(GetBestPlayerIndex());

		Idx = ObjectList.GetObjectInfoIndexFromName(bIsHost ? 'CampModeOption' : 'RemoteCampModeOption');
		if ( Idx != INDEX_NONE )
		{
			CampaignModeOption = GearUICollapsingSelectionList(ObjectList.GeneratedObjects[Idx].OptionObj);
			CampaignModeOption.ShouldDisableElement = ShouldDisableElementCampMode;
		}

		Idx = ObjectList.GetObjectInfoIndexFromName(bIsHost ? 'InviteOption' : 'RemoteInviteOption');
		if ( Idx != INDEX_NONE )
		{
			InviteTypeOption = GearUICollapsingSelectionList(ObjectList.GeneratedObjects[Idx].OptionObj);
		}

		Idx = ObjectList.GetObjectInfoIndexFromName(bIsHost ? 'ActsOption' : 'RemoteActsOption');
		if ( Idx != INDEX_NONE )
		{
			ActsList = GearUICollapsingSelectionList(ObjectList.GeneratedObjects[Idx].OptionObj);
			ActsList.ColumnAutoSizeMode = CELLAUTOSIZE_None;

			if (bIsHost)
			{
				ActsList.ShouldDisableElement = ShouldDisableElementAct;
			}
		}

		RefreshActListData();

		Idx = ObjectList.GetObjectInfoIndexFromName(bIsHost ? 'ChapterOption' : 'RemoteChapterOption');
		if ( Idx != INDEX_NONE )
		{
			ChapterList = GearUICollapsingSelectionList(ObjectList.GeneratedObjects[Idx].OptionObj);
			ChapterList.ColumnAutoSizeMode = CELLAUTOSIZE_None;

			// we need to adjust the column widths in the ChapterList, but we need to make sure that all positions are up to date first
			OptionsList.bEnableSceneUpdateNotifications = true;
			OptionsList.OnPostSceneUpdate = PostSceneUpdate;

			if (bIsHost)
			{
				ChapterList.ShouldDisableElement = ShouldDisableElementChapter;
			}
		}

		UpdateChapterList();

		Idx = ObjectList.GetObjectInfoIndexFromName(bIsHost ? 'CheckpointOption' : 'RemoteCheckpointOption');
		if ( Idx != INDEX_NONE )
		{
			CheckpointOption = GearUICollapsingSelectionList(ObjectList.GeneratedObjects[Idx].OptionObj);
			if (bIsHost)
			{
				CheckpointOption.ShouldDisableElement = ShouldDisableElementCheckpointOption;
			}

			// See if we should force "Load Last Checkpoint" option
			PlayerIndex = GetBestPlayerIndex();
			CurrGearPC = GetGearPlayerOwner(PlayerIndex);
			if ( CurrGearPC != None )
			{
				LoadCheckpointUsage = eGEARCHECKPOINT_Restart;
				CurrGearPC.ProfileSettings.GetProfileSettingValueId( CurrGearPC.ProfileSettings.CampCheckpointUsage, LoadCheckpointUsage );
			}
		}

		if (bIsHost)
		{
			RefreshChapterData( PlayerIndex, LoadCheckpointUsage == eGEARCHECKPOINT_UseLast, true );
		}
		else
		{
			// If the client hasn't initialized replicated values and the GRI is available
			if (!bClientIsInitialized && CampGRI != None)
			{
				ReplicateLobbyFromGRI();
			}
		}
	}

	// Set up the navigation
	if (ObjectList.Children.length > 0)
	{
		for (ChildIdx = 0; ChildIdx < ObjectList.Children.length; ChildIdx++)
		{
			FirstWidgetToNavLoop = GearUICollapsingSelectionList(ObjectList.Children[ChildIdx]);
			if (FirstWidgetToNavLoop != none)
			{
				break;
			}

		}

		for (ChildIdx = ObjectList.Children.length-1; ChildIdx >= 0; ChildIdx--)
		{
			LastWidgetToNavLoop = GearUICollapsingSelectionList(ObjectList.Children[ChildIdx]);
			if (LastWidgetToNavLoop != none)
			{
				break;
			}

		}
		if (FirstWidgetToNavLoop != none && LastWidgetToNavLoop != none)
		{
			ObjectList.SetForcedNavigationTarget(UIFACE_Bottom, none);
			ObjectList.SetForcedNavigationTarget(UIFACE_Top, none);
			StartData.StartCampButton.SetForcedNavigationTarget(UIFACE_Top, LastWidgetToNavLoop);
			StartData.StartCampButton.SetForcedNavigationTarget(UIFACE_Bottom, FirstWidgetToNavLoop);
			FirstWidgetToNavLoop.SetForcedNavigationTarget(UIFACE_Top, StartData.StartCampButton);
			LastWidgetToNavLoop.SetForcedNavigationTarget(UIFACE_Bottom, StartData.StartCampButton);
		}
	}

	RebuildDescriptionList();


	// Bind the callbacks for list selection
	ActsList.OnValueChanged=ActSelectionChanged;
	ChapterList.OnValueChanged=MapSelectionChanged;
}

/**
 * Called when the widget is no longer being pressed.  Not implemented by all widget types.
 *
 * The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
 * widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
 * when the key was released, which might not necessarily be the widget that received the key press.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnPlayerButtonClicked( UIScreenObject EventObject, int PlayerIndex )
{
	local GearPRI SelectedPRI;
	local int DataIndex;
	local array<EGearPlayerOptions> Options;
	local EGearCampMode CampMode;
	local bool bIsLiveGame;

	DataIndex = GetPlayerDataIndexFromWidget(EventObject);
	if (DataIndex != INDEX_NONE)
	{
		SelectedPRI = PlayerList[DataIndex].PlayerPRI;
		// If there is a player in this slot, go to the player options
		if ( SelectedPRI != None )
		{
			Options.AddItem(eGPOPTION_Gamercard);
			Options.AddItem(eGPOPTION_Feedback);
			if (CampGRI != None && CampGRI.Role == ROLE_Authority && !CampGRI.bIsInputGuarding)
			{
				Options.AddItem(eGPOPTION_Kick);
			}
			Options.AddItem(eGPOPTION_Cancel);

			CampMode = GetCampaignMode();
			bIsLiveGame = CampMode == eGCM_LivePrivate || CampMode == eGCM_LivePublic;
			OpenPlayerOptionsScene( PlayerIndex, SelectedPRI, Options, bIsLiveGame );
		}
		// If there is not a player in this slot, go to the what's up screen
		else
		{
			OpenScene(WhatsUpSceneReference, GetPlayerOwner(PlayerIndex));
		}
	}

	return true;
}

/** Returns the index in the PlayerList array of which playerdata contains this widget */
function int GetPlayerDataIndexFromWidget( UIScreenObject Widget )
{
	local int Idx;

	for (Idx = 0; Idx < PlayerList.length; Idx++)
	{
		if (PlayerList[Idx].PlayerButton == Widget)
		{
			return Idx;
		}
	}
	return INDEX_NONE;
}

/** @return true if a player is currently connecting */
function bool HasPendingConnection()
{
	local WorldInfo WI;
	local PlayerReplicationInfo PRI;
	local int PriIndex;
	local UniqueNetId ZeroId;

	if (CampGRI.ConnectingPlayerCount > 0)
	{
		return true;
	}

	WI = GetWorldInfo();
	if (WI != None)
	{
		// Check the PRIArray for any missing remote XUIDs
		for (PriIndex = 0; PriIndex < WI.GRI.PRIArray.Length; PriIndex++)
		{
			PRI = WI.GRI.PRIArray[PriIndex];
			if (PRI != None && PRI.UniqueId == ZeroId)
			{
				// If this PC is remote, then they are still connecting
				if (PlayerController(PRI.Owner) != None && !PlayerController(PRI.Owner).IsLocalPlayerController())
				{
					return true;
				}
			}
		}
	}
	return false;
}

/**
 * Called when the start game button is clicked.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool StartGameButtonClicked(UIScreenObject EventObject, int PlayerIndex)
{
	if (CampGRI != None && !CampGRI.bIsInputGuarding && !HasPendingConnection())
	{
		// If a coop-joining host attempts to search in system link ask them if they'd like to go to the LAN scene
		if (CampaignMode == eGCLOBBYMODE_Join &&
			GetCampaignMode() == eGCM_SystemLink &&
			IsHost(PlayerIndex))
		{
			OpenLANScene();
		}
		else
		{
			HostCampaignGame(None, PlayerIndex);
		}
	}

	return true;
}

/** Makes the COG spin and disables UI in places we don't want players to go */
final function SetLobbyToInputGuardMode(bool bGuardOn)
{
	local bool bIsHost;
	local int PlayerIndex;

	PlayerIndex = GetBestPlayerIndex();
	bIsHost = IsHost(PlayerIndex);
	CampGRI.bIsInputGuarding = bGuardOn;

	if (bIsHost)
	{
		if (bGuardOn)
		{
			OptionsList.SetEnabled(false, PlayerIndex);
			PlayerButtonBar.EnableButton('Checkpoint', PlayerIndex, false, true);
		}
		else
		{
			OptionsList.SetEnabled(true, PlayerIndex);
			PlayerButtonBar.EnableButton('Checkpoint', PlayerIndex, CanAccessCheckpointScene(PlayerIndex), true);
		}
	}
}

/** Called from the game info so we know when the joining process has failed or is finished so we can update our UI */
function JoinCampaignAttemptFinished()
{
	// Stop guarding all input and making COG spin
	SetLobbyToInputGuardMode(false);
}

/**
 * Called when the Host Campaign Button is clicked.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool HostCampaignGame(UIScreenObject EventObject, int PlayerIndex)
{
	local GearPC GPC;
	local int CurrSlot, LoadCheckpointValue;
	local GearEngine Engine;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	if (CampGRI != None && !CampGRI.bIsInputGuarding)
	{
		GPC = GetGearPlayerOwner(PlayerIndex);
		Engine = GearEngine(GPC.Player.Outer);

		// See if we're using a checkpoint
		LoadCheckpointValue = eGEARCHECKPOINT_Restart;
		GPC.ProfileSettings.GetProfileSettingValueId(GPC.ProfileSettings.CampCheckpointUsage, LoadCheckpointValue);
		bUsingCheckpoint = (LoadCheckpointValue == eGEARCHECKPOINT_UseLast);

		// Guard all input and make COG spin
		SetLobbyToInputGuardMode(true);

		// Don't need to check for checkpoint in Join-Coop mode
		if (CampaignMode != eGCLOBBYMODE_Join)
		{
			// Prepare the checkpoint system for access
			if ( PrepareCheckpointSystem(PlayerIndex) )
			{
				// Retrieve the save slot being used
				CurrSlot = -1;
				GPC.ProfileSettings.GetProfileSettingValueInt(GPC.ProfileSettings.LastCheckpointSlot, CurrSlot);
				CurrSlot = Max(0, CurrSlot);

				// Set the checkpoint to use
				Engine.FindCheckpointData( CurrSlot );

				// If the checkpoint is valid and they are restarting a chapter we will prompt
				if ( Engine.CurrentCheckpoint != None &&
					!Engine.CurrentCheckpoint.CheckpointIsEmpty() &&
					!bUsingCheckpoint )
				{
					ButtonAliases.AddItem('GenericCancel');
					ButtonAliases.AddItem('AcceptOverwrite');

					GameSceneClient = GetSceneClient();
					GameSceneClient.ShowUIMessage(
						'NoStorageDevice',
						"<Strings:GearGameUI.MessageBoxStrings.SaveOverwrite_Title>",
						"<Strings:GearGameUI.MessageBoxStrings.SaveOverwriteGeneric_Message>",
						"<Strings:GearGameUI.MessageBoxStrings.SaveOverwrite_Question>",
						ButtonAliases,
						OnCheckpointOverwriteCheckComplete,
						GetPlayerOwner(PlayerIndex) );

					return true;
				}
			}
			// Couldn't prepare the checkpoint system (no device set)
			else
			{
				GPC.ProfileSettings.SetProfileSettingValueId(GPC.ProfileSettings.CampCheckpointUsage, eGEARCHECKPOINT_Restart);
				bUsingCheckpoint = false;
				// Set the flag in the Engine for whether we are currently saving checkpoints to disk
				Engine.bShouldWriteCheckpointToDisk = false;
			}
		}

		SaveProfileAndHostCampaign();
	}
	return true;
}

/** Return from the prompt asking if the player wanted to overwrite their saved campaign */
function bool OnCheckpointOverwriteCheckComplete( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'AcceptOverwrite' )
	{
		Sender.OnSceneDeactivated = OnCheckpointOverwriteCheckComplete_FinishedClosing;
	}
	else
	{
		// Stop guarding all input and making COG spin
		SetLobbyToInputGuardMode(false);
	}
	return true;
}

/** Called when the Checkpoint overwrite check is finished closing with a Yes answer */
function OnCheckpointOverwriteCheckComplete_FinishedClosing( UIScene ClosingScene )
{
	SaveProfileAndHostCampaign();
}

function SaveProfileAndHostCampaign()
{
	local GearPC GPC;

	GPC = GetGearPlayerOwner(GetBestPlayerIndex());
	// Save profile before creating the campaign
	if ( GPC != None )
	{
		GPC.SaveProfile( StartCampaignAfterProfileWrite );
	}
	else
	{
		// Stop guarding all input and making COG spin
		SetLobbyToInputGuardMode(false);
	}
}

/** Called when the profile is done writing - builds the URL and starts the match */
function StartCampaignAfterProfileWrite(byte LocalUserNum,bool bWasSuccessful)
{
	local GearPC GPC;
	local GearMenuGame MenuGame;
	local int PlayerIndex;
	local EChapterPoint SelectedChapter;
	local String URL;
	local EGearCampMode CampMode;

	PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(LocalUserNum);
	GPC = GetGearPlayerOwner(PlayerIndex);
	GPC.ClearSaveProfileDelegate( StartCampaignAfterProfileWrite );
	MenuGame = GearMenuGame(GPC.WorldInfo.Game);

	if (MenuGame != None)
	{
		CampMode = GetCampaignMode();
		// If this is a private match or a public match in the Host-Coop mode, start the game
		if (CampaignMode == eGCLOBBYMODE_Split ||
			CampMode == eGCM_LivePrivate ||
			CampMode == eGCM_SystemLink ||
			(CampMode == eGCM_LivePublic && CampaignMode == eGCLOBBYMODE_Host))
		{
			// Get the chapter
			SelectedChapter = GetCurrentSelectedChapterType();
			// Create the URL
			URL = MenuGame.BuildCampaignURL(false, SelectedChapter, bUsingCheckpoint);
			`Log("Launching campaign with URL '"$URL$"'");
			GetWorldInfo().ServerTravel(URL);

			// Stop guarding all input and making COG spin
			SetLobbyToInputGuardMode(false);
		}
		// Otherwise, search for a session and then host/join as needed
		else
		{
			PublicCoopSearchThenJoin();
		}
	}
}

/**
 * Called when the save slot button is clicked.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool SaveSlotButtonClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local UIScene SaveSlotScene;

	// Leave breadcrumb for ui flow
	SetTransitionValue("EnterSaveSlot", "Lobby");

	SaveSlotScene = UIScene(FindObject("UI_Scenes_FE.UI_SelectSaveSlot", class'UIScene'));
	SelectSaveSlotSceneInstance = GearUISceneFE_SelectSaveSlot(SaveSlotScene.OpenScene(SaveSlotScene, LocalPlayer(GetGearPlayerOwner(PlayerIndex).Player)));
	if ( SelectSaveSlotSceneInstance != None )
	{
		SelectSaveSlotSceneInstance.OnSceneDeactivated = OnSaveSlotSceneClosedCallback;
	}

	return true;
}

/** Called when the save slot screen is closed - allows us to refresh the checkpoint option */
function OnSaveSlotSceneClosedCallback( UIScene ClosedScene )
{
	local string ExitString;

	ExitString = GetTransitionValue("ExitSaveSlot");
	if ( ExitString != "Cancel" )
	{
		// Set the current checkpoint
		SetCurrentCheckpointReference();
		RefreshSaveSlotUI();
	}

	SelectSaveSlotSceneInstance = None;
}

/** Refreshes the scene base on data gathered from the select save screen */
function RefreshSaveSlotUI()
{
	local int PlayerIndex;
	local GearPC PC;
	local int Value;

	PlayerIndex = GetBestPlayerIndex();

	// Refresh the checkpoint option which will also set the chapter and act list to their correct indexes
	// as well as the difficulty for the host
	if (IsHost(PlayerIndex))
	{
		RefreshOptions(PlayerIndex, true, false);

		PC = GetGearPlayerOwner(PlayerIndex);
		if (PC != None && PC.ProfileSettings != None && PC.PlayerReplicationInfo != None)
		{
			PC.ProfileSettings.GetProfileSettingValueId(PC.ProfileSettings.GameIntensity, Value);
			if (Value != GearCampaignLobbyPRI(PC.PlayerReplicationInfo).CampDifficulty)
			{
				PC.ServerSetCampaignLobbyDifficulty(EDifficultyLevel(Value));
			}
		}

		RefreshChapterDisplayWidgets(PlayerIndex);
		SetLobbyDataInGRI();
	}
}

/** Open the difficulty screen */
function bool OnDifficultyClicked(int PlayerIndex)
{
	local UIScene DifficultyScene, SceneInstance;
	local GearPC OwnerPC;
	local int Value;

	OwnerPC = GetGearPlayerOwner(PlayerIndex);
	if (OwnerPC != none &&
		OwnerPC.ProfileSettings != none &&
		OwnerPC.ProfileSettings.GetProfileSettingValueId(OwnerPC.ProfileSettings.GameIntensity, Value))
	{
		// Set the difficulty to be focused for the scene
		SetTransitionValue("SelectedDifficulty", string(Value));
	}

	// Leave breadcrumb for ui flow
	SetTransitionValue("EnterDifficulty", "Lobby");

	DifficultyScene = UIScene(FindObject("UI_Scenes_FE.UI_FESO_Difficulty", class'UIScene'));
	SceneInstance = DifficultyScene.OpenScene(DifficultyScene, LocalPlayer(GetGearPlayerOwner(PlayerIndex).Player));
	if (SceneInstance != None)
	{
		SceneInstance.OnSceneDeactivated = OnDifficultySceneClosedCallback;
	}

	return true;
}

/** Called when the difficulty screen is closed - allows us to refresh the difficulty UI */
function OnDifficultySceneClosedCallback( UIScene ClosedScene )
{
	local LocalPlayer LP;
	local GearPC PC;
	local int Value;

	LP = ClosedScene.PlayerOwner;
	if (LP != None)
	{
		PC = GearPC(LP.Actor);
		if (PC != None && PC.ProfileSettings != None && PC.PlayerReplicationInfo != None)
		{
			PC.ProfileSettings.GetProfileSettingValueId(PC.ProfileSettings.GameIntensity, Value);
			if (Value != GearCampaignLobbyPRI(PC.PlayerReplicationInfo).CampDifficulty)
			{
				PC.ServerSetCampaignLobbyDifficulty(EDifficultyLevel(Value));
			}
		}
	}
}

/** Called when the scene is activated so we can set the difficulty strings */
function OnSceneActivatedCallback( UIScene ActivatedScene, bool bInitialActivation )
{
	local GearPC PC;
	local WorldInfo WI;
	local int Value;

	if ( bInitialActivation )
	{
		// Set the difficulty settings for the local player in its' PRI
		WI = GetWorldInfo();
		foreach WI.AllControllers(class'GearPC', PC)
		{
			if (PC.IsLocalPlayerController() && PC.ProfileSettings != None)
			{
				if (PC.PlayerReplicationInfo != None && !PC.PlayerReplicationInfo.bIsInactive)
				{
					PC.ProfileSettings.GetProfileSettingValueId(PC.ProfileSettings.GameIntensity, Value);
					PC.ServerSetCampaignLobbyDifficulty(EDifficultyLevel(Value));
				}
			}
		}

		TrackChat( true );

		if (IsHost(GetBestPlayerIndex()))
		{
			// Initialized the chapter to display based on the active checkpoint
			if (ActiveCheckpoint != None)
			{
				CampGRI.CurrChapter = ActiveCheckpoint.Chapter;
				bForceCheckpointOnNextRegen = true;
			}

			// If we are entering the lobby in system link mode open the LAN scene, make
			if (CanOpenLANScene(GetBestPlayerIndex()) && !bReturnedFromGame)
			{
				OpenLANScene();
			}
		}
	}
}

/** Called just after this scene is removed from the active scenes array */
event SceneDeactivated()
{
	local GearPC OwnerPC;

	Super.SceneDeactivated();

	ClearDelegates();

	// Turn chat delegates off
	TrackChat( false );

	// make sure we clear the reference from the PC to us
	OwnerPC = GetGearPlayerOwner();
	if ( OwnerPC != None )
	{
		OwnerPC.ClearTimer( nameof(CacheGRI), Self );
	}

	// clear this evil evil reference to an actor!  :)
	CampGRI = None;
}

/** Set the PRI in the correct player slots */
final function SetPRIsInPlayerSlots()
{
	local int PRIIdx, SlotIdx, Idx;
	local GearCampaignLobbyPRI PRI;
	local GearPC PC;

	// First wipe the PRIs
	for (Idx = 0; Idx < PlayerList.length; Idx++)
	{
		PlayerList[Idx].PlayerPRI = None;
	}

		// Need a GRI
	if (CampGRI != None)
	{
		for (PRIIdx = 0; PRIIdx < CampGRI.PRIArray.length; PRIIdx++)
		{
			PRI = GearCampaignLobbyPRI(CampGRI.PRIArray[PRIIdx]);
			if (PRI != None && !PRI.bIsInactive)
			{
				PC = GearPC(PRI.Owner);

				// This is the host machine
				if (CampGRI.Role == ROLE_Authority)
				{
					// See if this is the host (0 == host)
					if (PC != None && PC.GetUIPlayerIndex() == 0)
					{
						// If this is a coop-join we will put the player in the Dom spot
						if (CampaignMode == eGCLOBBYMODE_Join)
						{
							SlotIdx = 1;
						}
						else
						{
							SlotIdx = 0;
						}
					}
					// This is a splitscreen coop player so they get slot 1
					else
					{
						SlotIdx = 1;
					}
				}
				// This is a networked coop player
				else
				{
					if (PC != None && PC.IsLocalPlayerController())
					{
						SlotIdx = 1;
					}
					else
					{
						SlotIdx = 0;
					}
				}

				// Set the PRI to the slot
				PlayerList[SlotIdx].PlayerPRI = PRI;
			}
		}
	}
}

/** Refreshes the player list data */
final function RefreshPlayerListData()
{
	local int Idx;
	local GearCampaignLobbyPRI PRI;
	local GearProfileSettings Profile;
	local String Value, RankString, ControllerString;
	local PlayerController PC;
	local LocalPlayer LP;
	local UniqueNetId ZeroId;

	Profile = GetGearPlayerOwner().ProfileSettings;

	// First set the PRIs
	SetPRIsInPlayerSlots();

	for (Idx = 0; Idx < PlayerList.length; Idx++)
	{
		if (PlayerList[Idx].PlayerButton != None)
		{
			PRI = PlayerList[Idx].PlayerPRI;
			// Valid PRI
			if (PRI != None && !PRI.bIsInactive)
			{
				PC = PlayerController(PRI.Owner);

				// Set the player's name and background style
				if (PRI.UniqueId != ZeroId)
				{
					PlayerList[Idx].PlayerButton.SetDataStoreBinding(PRI.PlayerName);
				}
				else
				{
					// Say "connecting..." since showing Marcus in the Dom slot seems bad
					PlayerList[Idx].PlayerButton.SetDataStoreBinding(ConnectingPlayerSlot);
				}
				PlayerList[Idx].PlayerButton.SetWidgetStyleByName('Image Style', 'CampaignFull');

				// Set the player's difficulty
				if (PRI.UniqueId != ZeroId && Profile != None && Profile.GetProfileSettingValue(Profile.GameIntensity, Value, PRI.CampDifficulty))
				{
					if (PC != None && PC.IsLocalPlayerController())
					{
						PlayerList[Idx].DifficultyButtonLabel.SetVisibility(true);
					}
					else
					{
						PlayerList[Idx].DifficultyButtonLabel.SetVisibility(false);
					}
					PlayerList[Idx].DifficultyDescLabel.SetDataStoreBinding(DifficultyString);
					PlayerList[Idx].DifficultyDescLabel.SetVisibility(true);
					PlayerList[Idx].DifficultyLabel.SetDataStoreBinding(Value);
					PlayerList[Idx].DifficultyLabel.SetVisibility(true);
				}
				else
				{
					PlayerList[Idx].DifficultyButtonLabel.SetVisibility(false);
					PlayerList[Idx].DifficultyLabel.SetVisibility(false);
					PlayerList[Idx].DifficultyDescLabel.SetVisibility(false);
				}

				// Set the player's Rank
				RankString = GetPlayerSkillString(PRI.PlayerSkill);
				PlayerList[Idx].RankLabel.SetVisibility(true);
				PlayerList[Idx].RankLabel.SetDataStoreBinding(RankString);

				// Set the player's profile icon
				ControllerString = " ";
				if (PC != None && PC.IsLocalPlayerController())
				{
					LP = LocalPlayer(PC.Player);
					if (LP != None)
					{
						ControllerString = GetControllerIconString(LP.ControllerId);
					}
				}
				PlayerList[Idx].ProfileLabel.SetVisibility(true);
				PlayerList[Idx].ProfileLabel.SetDataStoreBinding(ControllerString);
			}
			// Invalid PRI (empty)
			else
			{
				// Set the background style, set the player string to "OPEN", and hide the difficulty
				PlayerList[Idx].PlayerButton.SetWidgetStyleByName( 'Image Style', 'CampaignEmpty' );

				// Coop-join lobby should show special string and disable the widget
				if (CampaignMode == eGCLOBBYMODE_Join)
				{
					if (PlayerList[Idx].PlayerButton.IsFocused() && Idx == 0)
					{
						PlayerList[Idx].PlayerButton.KillFocus(none);
						PlayerList[1].PlayerButton.SetFocus(none);
					}
					PlayerList[Idx].PlayerButton.DisableWidget(GetBestPlayerIndex());
					PlayerList[Idx].PlayerButton.SetDataStoreBinding("");
					PlayerList[Idx].PlayerButton.SetVisibility(FALSE);
					if ( PlayerList[Idx].ImageMarcus != None )
					{
						PlayerList[Idx].ImageMarcus.SetVisibility(FALSE);
					}
				}
				// If system link, show the slot as "open"
				else if (GetCampaignMode() == eGCM_SystemLink)
				{
					PlayerList[Idx].PlayerButton.SetDataStoreBinding( EmptySlotString );
				}
				// If someone is connecting, then show them as such
				else if (CampGRI.ConnectingPlayerCount > 0)
				{
					// Say "connecting..." since showing Marcus in the Dom slot seems bad
					PlayerList[Idx].PlayerButton.SetDataStoreBinding(ConnectingPlayerSlot);
				}
				else
				{
					PlayerList[Idx].PlayerButton.SetDataStoreBinding( InvitablePlayerSlot );
				}
				PlayerList[Idx].DifficultyLabel.SetVisibility(false);
				PlayerList[Idx].DifficultyButtonLabel.SetVisibility(false);
				PlayerList[Idx].DifficultyDescLabel.SetVisibility(false);
				PlayerList[Idx].RankLabel.SetVisibility(false);
				PlayerList[Idx].ProfileLabel.SetVisibility(false);
			}
		}
	}
}

/**
 * Handler for the confirmation dialog's OnSelection delegate.  Leaves the lobby if the user confirmed.
 *
 * @param	Sender				the message box that generated this call
 * @param	SelectedInputAlias	the alias of the button that the user selected.  Should match one of the aliases passed into
 *								this message box.
 * @param	PlayerIndex			the index of the player that selected the option.
 *
 * @return	TRUE to indicate that the message box should close itself.
 */
function bool OnLeaveLobby_Confirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local OnlineGameInterface GameInterface;
	local OnlineGameSettings GameSettings;
	local GearPC GPC;
	local bool bShouldTravel;

	if ( SelectedInputAlias == 'AcceptLeave' )
	{
		GPC = GetGearPlayerOwner(PlayerIndex);
		if ( GPC != None )
		{
			GPC.bIgnoreNetworkMessages = true;
		}

		GameInterface = GetOnlineGameInterface();

		if ( GameInterface != None )
		{
			GameSettings = GameInterface.GetGameSettings('Party');
			if ( GameSettings != None )
			{
				// LeaveMatch will trigger the loading movie
				LeaveMatch();
			}
			else
			{
				bShouldTravel = true;
			}
		}
		else
		{
			bShouldTravel = true;
		}
		// Go back to the main menu if requested
		if (bShouldTravel)
		{
			if ( GPC != None )
			{
				GPC.ClientTravel("?closed", TRAVEL_Absolute);
			}
		}
	}

	return true;
}

/**
* Handler for the scene's OnProcessInputKey delegate - overrides the default handling of the CloseScene alias to issue
* a disconnect command.
*
* Called when an input key event is received which this widget responds to and is in the correct state to process.  The
* keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
*
* This delegate is called AFTER kismet is given a chance to process the input, but BEFORE any native code processes the input.
*
* @param	EventParms	information about the input event, including the name of the input alias associated with the
*						current key name (Tab, Space, etc.), event type (Pressed, Released, etc.) and modifier keys (Ctrl, Alt)
*
* @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
*/
function bool ProcessInputKey( const out SubscribedInputEventParameters EventParms )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local GearUICollapsingSelectionList ExpandedList;
	local bool bResult;
	local WorldInfo WI;

	if ( EventParms.InputAliasName == 'CloseScene' && !CampGRI.bIsInputGuarding )
	{
		if ( EventParms.EventType == IE_Released )
		{
			if ( IsEditingSelectionList(OptionsList, ExpandedList) && ExpandedList != None )
			{
				ExpandedList.Collapse();
			}
			else if ( CurrMessageState == eCMS_Buttons )
			{
				SetMessageState(eCMS_Normal);
			}
			else
			{
				ButtonAliases.AddItem('GenericCancel');
				ButtonAliases.AddItem('GenericAccept');
				GameSceneClient = GetSceneClient();
				WI = GetWorldInfo();
				if ( WI != None && WI.NetMode != NM_Client )
				{
					GameSceneClient.ShowUIMessage('ConfirmQuit',
						"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_Title>",
						"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_LeaderMessage>",
						"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_Question>",
						ButtonAliases, OnReturnToMainMenuConfirmed, GetPlayerOwner());
				}
				else
				{
					GameSceneClient.ShowUIMessage('ConfirmQuit',
						"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_Title>",
						"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_MemberMessage>",
						"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_Question>",
						ButtonAliases, OnReturnToMainMenuConfirmed, GetPlayerOwner());
				}
			}
		}

		bResult = true;
	}

	return bResult;
}

/** Callback when the user confirms leaving */
function bool OnReturnToMainMenuConfirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local WorldInfo WI;
	local GearPC PC;

	if ( SelectedInputAlias == 'GenericAccept' )
	{
		PC = GetGearPlayerOwner(0);
		WI = GetWorldInfo();
		if ( PC != None && WI != None )
		{
			PC.ReturnToMainMenu();
			// Save the profiles
			foreach WI.LocalPlayerControllers(class'GearPC', PC)
			{
				PC.SaveProfile();
			}
		}
	}
	return true;
}

/**
 * Callback function when the scene gets input
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool ProcessRawInput( out InputEventParameters EventParms )
{
	if (EventParms.InputKeyName == 'XboxTypeS_X')
	{
		if (EventParms.EventType == IE_Pressed &&
			!CampGRI.bIsInputGuarding)
		{
			OnDifficultyClicked(EventParms.PlayerIndex);
		}
		return true;
	}
	else if (EventParms.InputKeyName == 'XboxTypeS_Start' &&
		     EventParms.EventType == IE_Pressed &&
		     EventParms.PlayerIndex == 0 &&
			 IsHost(EventParms.PlayerIndex))
	{
		StartData.StartCampButton.SetFocus(none);
		StartGameButtonClicked(StartData.StartCampButton, 0);
		return true;
	}

	return false;
}

/**
 * Builds a coop search object and searches for a match
 */
function PublicCoopSearchThenJoin()
{
	local GearCoopGameSearch CoopSearch;
	local OnlineSubsystem OnlineSub;

	CoopSearch = new class'GearCoopGameSearch';
	// Set the search criteria
	CoopSearch.SetIntProperty(class'GearCoopGameSearch'.const.PROPERTY_ACTNUM,GetCurrentlySelectedAct() + 1);
	CoopSearch.SetIntProperty(class'GearCoopGameSearch'.const.PROPERTY_CHAPTERNUM,GetCurrentSelectedChapter() + 1);

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	OnlineSub.GameInterface.AddFindOnlineGamesCompleteDelegate(OnFindOnlineGamesComplete);
	// Search for a set of games that match this
	OnlineSub.GameInterface.FindOnlineGames(GetBestControllerId(),CoopSearch);
}

/**
 * Delegate fired when the search for our coop game has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnFindOnlineGamesComplete(bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameSearch CoopSearch;
	local OnlineGameSearchResult Result;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	OnlineSub.GameInterface.ClearFindOnlineGamesCompleteDelegate(OnFindOnlineGamesComplete);
	if (bWasSuccessful)
	{
		// Make sure we got a host back in the search
		CoopSearch = OnlineSub.GameInterface.GetGameSearch();
		if (CoopSearch.Results.Length > 0)
		{
			OnlineSub.GameInterface.AddJoinOnlineGameCompleteDelegate(OnJoinCoopGameComplete);
			Result = CoopSearch.Results[0];
			// Now join the party with the lowest ping (result 0)
			OnlineSub.GameInterface.JoinOnlineGame(GetBestControllerId(),'Party',Result);
		}
		else
		{
			ShowNoMatchesFoundUI();
		}
	}
}

/**
 * Delegate fired when the search for our coop game has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnJoinCoopGameComplete(name SessionName,bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local string URL;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	OnlineSub.GameInterface.ClearJoinOnlineGameCompleteDelegate(OnJoinCoopGameComplete);
	if (bWasSuccessful)
	{
		// We are joining so grab the connect string to use
		if (OnlineSub.GameInterface.GetResolvedConnectString(SessionName,URL))
		{
			ShowLoadingMovie(true);
			// Trigger a console command to connect to this url
			GetWorldInfo().ConsoleCommand("start " $ URL);
		}
	}
	else
	{
		ShowNoMatchesFoundUI();
	}
}

/** Tells the user that no matches were found and unblocks the UI upon closing */
function ShowNoMatchesFoundUI()
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('GenericAccept');
	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage(
		'ConfirmNetworkedPlayersKicked',
		"<Strings:GearGameUI.MessageBoxStrings.NoCoopGamesFound_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.NoCoopGamesFound_Desc>",
		"",
		ButtonAliases,
		OnNoMatchesFoundDismissed,
		GetPlayerOwner(GetBestPlayerIndex()) );
}

/** Unlocks the UI so they can change parameters for their searches */
function bool OnNoMatchesFoundDismissed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	SetLobbyToInputGuardMode(false);
	return true;
}

defaultproperties
{
	OnProcessInputKey=ProcessInputKey
	OnSceneActivated=OnSceneActivatedCallback
	OnGearUISceneTick=UpdateCampaignLobbyScene
	OnRawInputKey=ProcessRawInput

	WhatsUpSceneReference=UIScene'UI_Scenes_WU.UI_FE_WhatsUp'

	CurrentActIndex=0
	CurrentChapterIndex=0



	/** Animations */
	Begin Object Class=UIAnimationSeq Name=FadeIn_Template
		SeqName=FadeIn
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.3,Data=(DestAsFloat=1.0))))
	End Object
	FadeIn = FadeIn_Template

	Begin Object Class=UIAnimationSeq Name=FadeOut_Template
		SeqName=FadeOut
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.3,Data=(DestAsFloat=0.0))))
	End Object
	FadeOut = FadeOut_Template

}

