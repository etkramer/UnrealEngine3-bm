/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFE_SelectSaveSlot extends GearUISceneFrontEnd_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Structure of data per save slot */
struct SaveSlotData
{
	/** The parent button of the slot */
	var transient UIButton ParentButton;

	/** The panel within the ParentButton used for containing the widgets dealing with an empty slot */
	var transient UIPanel EmptySlotPanel;

	/** The panel within the ParentButton used for containing the widgets dealing with a used slot */
	var transient UIPanel UsedSlotPanel;

	/** Label used to display the time the game was saved */
	var transient UILabel TimeLabel;

	/** Label used to display the date the game was saved */
	var transient UILabel DateLabel;

	/** Image of the level this game was saved in */
	var transient UIImage MapImage;

	/** Image used to display the difficulty this game was save under */
	var transient UIImage DifficultyImage;

	/** Label used to display the chapter this game was saved in */
	var transient UILabel ChapterLabel;

	/** Label used to display the act this game was saved in */
	var transient UILabel ActLabel;

	/** Difficulty this checkpoint was saved with */
	var transient EDifficultyLevel DifficultyType;

	/** The chapter this is pointing to */
	var transient EChapterPoint ChapterType;

	/** The A button label for the Empty panel */
	var transient UILabel EmptyALabel;

	/** The A button label for the Used panel */
	var transient UILabel UsedALabel;

	/** The label that displays whether a slot is empty or corrupted */
	var transient UILabel EmptyString;

	/** Whether this slot is corrupted or not */
	var transient bool bIsCorrupted;
};

/** List of the save slots in the scene */
var transient array<SaveSlotData> SaveSlotList;

/** Reference to the slot button bar */
var transient UICalloutButtonPanel SlotButtonBar;

/** Whether we came through the solo campaign path or not */
var transient bool bIsSoloCampPath;

/** The index in the slot list of the last focused slot */
var transient int LastFocusedSlotIndex;

/** Hack Band-aid to prevent button mashing from breaking going into the lobby */
var transient bool bBlockInputForLobbyTravel;

/** Whether we are in the process of deleting the checkpoints or not */
var transient bool bIsDeletingCheckpoints;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
 * Called after this screen object's children have been initialized
 * Overloaded to initialized the references to this scene's widgets
 */
event PostInitialize()
{
	InitializeWidgetReferences();

	Super.PostInitialize();
}

/** Initialize the widget references */
final function InitializeWidgetReferences()
{
	local int Idx;
	local string SlotButtonName;
	local string EnterString;
	local int PlayerIndex;

	PlayerIndex = GetBestPlayerIndex();
	EnterString = GetTransitionValue("EnterSaveSlot");
	bIsSoloCampPath = (EnterString ~= "MainMenu_Solo");

	SaveSlotList.length = eGEARCAMPMEMORYSLOT_MAX;
	for ( Idx = 0; Idx < SaveSlotList.length; Idx++ )
	{
		SlotButtonName = "btnSlot" $ Idx;
		SaveSlotList[Idx].ParentButton		= UIButton(FindChild(Name(SlotButtonName), true));

		SaveSlotList[Idx].EmptySlotPanel	= UIPanel(SaveSlotList[Idx].ParentButton.FindChild('pnlNew', true));
		SaveSlotList[Idx].UsedSlotPanel		= UIPanel(SaveSlotList[Idx].ParentButton.FindChild('pnlSaveGame', true));
		SaveSlotList[Idx].TimeLabel			= UILabel(SaveSlotList[Idx].UsedSlotPanel.FindChild('lblTime', true));
		SaveSlotList[Idx].DateLabel			= UILabel(SaveSlotList[Idx].UsedSlotPanel.FindChild('lblDate', true));
		SaveSlotList[Idx].MapImage			= UIImage(SaveSlotList[Idx].UsedSlotPanel.FindChild('imgPortrait', true));
		SaveSlotList[Idx].DifficultyImage	= UIImage(SaveSlotList[Idx].UsedSlotPanel.FindChild('imgDifficulty', true));
		SaveSlotList[Idx].ChapterLabel		= UILabel(SaveSlotList[Idx].UsedSlotPanel.FindChild('lblChapter', true));
		SaveSlotList[Idx].ActLabel			= UILabel(SaveSlotList[Idx].UsedSlotPanel.FindChild('lblAct', true));

		SaveSlotList[Idx].EmptyALabel		= UILabel(SaveSlotList[Idx].EmptySlotPanel.FindChild('lblA', true));
		SaveSlotList[Idx].EmptyString		= UILabel(SaveSlotList[Idx].EmptySlotPanel.FindChild('lblNewGame', true));
		SaveSlotList[Idx].EmptyALabel.SetVisibility(false);
		SaveSlotList[Idx].UsedALabel		= UILabel(SaveSlotList[Idx].UsedSlotPanel.FindChild('lblA', true));
		SaveSlotList[Idx].UsedALabel.SetVisibility(false);
	}

	// Initialize the slot button bar
	SlotButtonBar = UICalloutButtonPanel(FindChild('btnbarSlots', true));
	if ( SlotButtonBar != None )
	{
		// Going through the campaign wizard so show different buttons then going to lobby
		if (bIsSoloCampPath)
		{
			SlotButtonBar.SetButtonCallback('SelectChapter', OnSelectChapter);
			SlotButtonBar.SetButtonCallback('NewCamp', OnNewCampaign);
		}
		else
		{
			SlotButtonBar.SetButtonInputAlias('ContinueCampaign', 'SelectSlot');
			SlotButtonBar.SetButtonCallback('SelectSlot', OnSelectSlot);
			SlotButtonBar.EnableButton('SelectChapter', PlayerIndex, false, true);
			SlotButtonBar.EnableButton('NewCamp', PlayerIndex, false, true);
		}
		SlotButtonBar.SetButtonCallback( 'GenericBack', OnCancelClicked );
	}

	// Refresh
	RefreshSlotWidgets();
	// Set the focused slot
	SetCurrentSaveSlotFocus();
}

/** Checks for corrupt saves and prompts for deletion */
function CheckForCorruptSaves()
{
	local int SlotIndex;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	for (SlotIndex = 0; SlotIndex < SaveSlotList.length; SlotIndex++)
	{
		if (SaveSlotList[SlotIndex].bIsCorrupted)
		{
			ButtonAliases.AddItem('GenericCancel');
			ButtonAliases.AddItem('AcceptOverwrite');

			// Prompt for deleting corrupted save slot
			GameSceneClient = GetSceneClient();
			GameSceneClient.ShowUIMessage(
				'NoStorageDevice',
				"<Strings:GearGameUI.MessageBoxErrorStrings.ErrorLoadingSaveGame_Title>",
				"<Strings:GearGameUI.MessageBoxErrorStrings.ErrorLoadingSaveGame_Message>",
				"<Strings:GearGameUI.MessageBoxErrorStrings.ErrorLoadingSaveGame_Question>",
				ButtonAliases,
				OnCheckpointCorrupted,
				GetPlayerOwner(GetPlayerOwnerIndex()) );
			break;
		}
	}
}

/** Return from the prompt asking if the player wanted to overwrite their saved campaign */
function bool OnCheckpointCorrupted( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local GearEngine GE;
	local GearUISceneFE_Updating SceneInstance;

	if ( SelectedInputAlias == 'AcceptOverwrite' )
	{
		GE = GetGearEngine();
		GE.PendingCheckpointAction = Checkpoint_DeleteAll;
		bIsDeletingCheckpoints = true;
		SceneInstance = OpenUpdatingScene();
		if (SceneInstance != none)
		{
			SceneInstance.InitializeUpdatingScene("DeleteSavesTitle", "DeleteSavesDesc", 1.0f);
		}
	}
	else
	{
		Sender.CloseScene(,,true);
		CloseScene(self);
		return false;
	}
	return true;
}

/**
 * Refreshes the button bar when a new slot is focused
 *
 * @param SlotIndex - the slot index of the currently focused slot
 */
final function RefreshButtonBar(int SlotIndex)
{
	local int PlayerIndex;

	PlayerIndex = GetBestPlayerIndex();

	if (bIsSoloCampPath)
	{
		if (CanContinueCampaign(SlotIndex))
		{
			SlotButtonBar.SetButtonInputAlias('SelectSlot', 'ContinueCampaign');
			SlotButtonBar.SetButtonCallback('ContinueCampaign', OnContinueCampaign);
			SlotButtonBar.EnableButton('NewCamp', PlayerIndex, true, false);
			SlotButtonBar.EnableButton('SelectChapter', PlayerIndex, true, false);
		}
		else
		{
			SlotButtonBar.SetButtonInputAlias('ContinueCampaign', 'SelectSlot');
			SlotButtonBar.SetButtonCallback('SelectSlot', OnSelectSlot);
			SlotButtonBar.EnableButton('NewCamp', PlayerIndex, true, false);
			SlotButtonBar.EnableButton('SelectChapter', PlayerIndex, false, false);
		}
	}
}

/** Whether there is a currently selected valid campaign to continue from */
function bool CanContinueCampaign(int SlotIndex)
{
	return (!SaveSlotList[SlotIndex].EmptySlotPanel.IsVisible());
}

/**
 * Called when a new UIState becomes the widget's currently active state, after all activation logic has occurred.
 *
 * @param	Sender					the widget that changed states.
 * @param	PlayerIndex				the index [into the GamePlayers array] for the player that activated this state.
 * @param	NewlyActiveState		the state that is now active
 * @param	PreviouslyActiveState	the state that used the be the widget's currently active state.
 */
function OnStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, UIState PreviouslyActiveState )
{
	local int SlotIdx;
	local int Idx;

	// Make sure the new state is focused
	if ( NewlyActiveState.IsA('UIState_Focused') )
	{
		// Find the slot that was just focused
		for ( SlotIdx = 0; SlotIdx < SaveSlotList.length; SlotIdx++ )
		{
			if ( Sender == SaveSlotList[SlotIdx].ParentButton )
			{
				RefreshButtonBar( SlotIdx );

				// Make this button's A glyps visible and the other's not
				for (Idx = 0; Idx < SaveSlotList.length; Idx++)
				{
					if (Sender == SaveSlotList[Idx].ParentButton)
					{
						// Show the A buttons
						SaveSlotList[Idx].EmptyALabel.SetVisibility(true);
						SaveSlotList[Idx].UsedALabel.SetVisibility(true);
					}
					else
					{
						// Hide the A buttons
						SaveSlotList[Idx].EmptyALabel.SetVisibility(false);
						SaveSlotList[Idx].UsedALabel.SetVisibility(false);
					}
				}
				break;
			}
		}
	}
}

/** Refresh the slot widgets */
final function RefreshSlotWidgets()
{
	local GearEngine Engine;
	local GearPC CurrGearPC;
	local int Idx, PlayerIndex;
	local CheckpointEnumerationResult EnumResult;
	local string EmptyVarString;

	PlayerIndex = GetBestPlayerIndex();

	// Prepare the checkpoint system for access
	if ( PrepareCheckpointSystem(PlayerIndex) )
	{
		CurrGearPC = GetGearPlayerOwner(PlayerIndex);
		if ( CurrGearPC != None )
		{
			Engine = GearEngine(CurrGearPC.Player.Outer);
			if ( Engine != None )
			{
				Engine.FindCheckpointData(INDEX_NONE, EnumResult);
				for (Idx = 0; Idx < eGEARCAMPMEMORYSLOT_MAX; Idx++)
				{
					if (EnumResult.bCheckpointFileExists[Idx] == 0
					||	EnumResult.bCheckpointFileContainsData[Idx] == 0
					||	EnumResult.bCheckpointFileCorrupted[Idx] != 0)
					{
						SaveSlotList[Idx].DifficultyType = DL_Casual;
						SaveSlotList[Idx].UsedSlotPanel.SetVisibility( false );
						SaveSlotList[Idx].EmptySlotPanel.SetVisibility( true );
						SaveSlotList[Idx].bIsCorrupted = (EnumResult.bCheckpointFileCorrupted[Idx] != 0);
						EmptyVarString = SaveSlotList[Idx].bIsCorrupted ? "CorruptSlot" : "EmptySlot";
						SaveSlotList[Idx].EmptyString.SetDataStoreBinding(Localize("GearUISceneSelectSaveSlot", EmptyVarString, "GearGame"));
					}
					else
					{
						SaveSlotList[Idx].bIsCorrupted = false;
						SaveSlotList[Idx].DifficultyType = EnumResult.CheckpointDifficulty[Idx];
						SaveSlotList[Idx].UsedSlotPanel.SetVisibility( true );
						SaveSlotList[Idx].EmptySlotPanel.SetVisibility( false );

						// Refresh the widgets for this checkpoint
						RefreshUsedSlotWidget( Idx, CurrGearPC, EnumResult );
					}
				}
			}
		}
	}
}

/** Refresh the widgets for the checkpoint in slot "SlotIdx" */
final function RefreshUsedSlotWidget( int SlotIdx, GearPC GPC, const out CheckpointEnumerationResult CheckpointInfo )
{
	local string StringValue, MapStringValue;
	local GearCampaignChapterData ChapterData;
	local GearCampaignActData ActData;

	SaveSlotList[SlotIdx].TimeLabel.SetDataStoreBinding( CreateTimeString(CheckpointInfo.CheckpointTimestamp[SlotIdx].SecondsSinceMidnight) );
	SaveSlotList[SlotIdx].DateLabel.SetDataStoreBinding(
		CreateDateString(CheckpointInfo.CheckpointTimestamp[SlotIdx].Day, CheckpointInfo.CheckpointTimestamp[SlotIdx].Month, CheckpointInfo.CheckpointTimestamp[SlotIdx].Year)
		);

	GPC.ProfileSettings.GetProfileSettingValue( GPC.ProfileSettings.GameIntensity, StringValue, CheckpointInfo.CheckpointDifficulty[SlotIdx] );
	SaveSlotList[SlotIdx].DifficultyImage.SetDataStoreBinding( class'GearUISceneFE_Difficulty'.default.DifficultyImagePaths[CheckpointInfo.CheckpointDifficulty[SlotIdx]] );

	ChapterData = class'GearUIDataStore_GameResource'.static.GetChapterDataProvider( CheckpointInfo.CheckpointChapter[SlotIdx]);
	if ( ChapterData != None )
	{
		StringValue = ChapterData.DisplayName;
		MapStringValue = ChapterData.ScreenshotPathName;
		SaveSlotList[SlotIdx].ChapterType = ChapterData.ChapterType;
	}
	else
	{
		StringValue = "";
		MapStringValue = "";
		SaveSlotList[SlotIdx].ChapterType = EChapterPoint(0);
	}
	SaveSlotList[SlotIdx].ChapterLabel.SetDataStoreBinding( StringValue );
	SaveSlotList[SlotIdx].MapImage.SetDataStoreBinding( MapStringValue );

	ActData = class'GearUIDataStore_GameResource'.static.GetActDataProviderUsingChapterProvider( ChapterData );
	if ( ActData != None )
	{
		StringValue = ActData.DisplayName;
	}
	else
	{
		StringValue = "";
	}
	SaveSlotList[SlotIdx].ActLabel.SetDataStoreBinding( StringValue );
}

/** Set the focused slot based on what's stored in the profile */
final function SetCurrentSaveSlotFocus()
{
	local GearPC MyGearPC;
	local int CurrSlot;

	MyGearPC = GetGearPlayerOwner( GetBestPlayerIndex() );
	if ( MyGearPC != None && MyGearPC.ProfileSettings != None )
	{
		if ( MyGearPC.ProfileSettings.GetProfileSettingValueInt(MyGearPC.ProfileSettings.LastCheckpointSlot, CurrSlot) )
		{
			CurrSlot = Max( 0, CurrSlot );
			if ( SaveSlotList[CurrSlot].ParentButton != None )
			{
				LastFocusedSlotIndex = CurrSlot;
				SaveSlotList[CurrSlot].ParentButton.SetFocus( None );
			}
		}
	}
	RefreshButtonBar(GetCurrentSaveSlotFocused());
}

/** Get the index of the slot currently focused */
final function int GetCurrentSaveSlotFocused()
{
	local int CurrSlot, Idx, PlayerIndex;

	CurrSlot = -1;
	PlayerIndex = GetBestPlayerIndex();
	for ( Idx = 0; Idx < SaveSlotList.length; Idx++ )
	{
		if ( SaveSlotList[Idx].ParentButton.IsFocused(PlayerIndex) )
		{
			CurrSlot = Idx;
			break;
		}
	}

	if ( CurrSlot == -1 )
	{
		if (LastFocusedSlotIndex == -1)
		{
			`log("GearUISceneFE_SelectSaveSlot:GetCurrentSaveSlotFocued could not find a focused slot!!!");
		}
		else
		{
			return LastFocusedSlotIndex;
		}
	}

	LastFocusedSlotIndex = CurrSlot;
	return CurrSlot;
}

/** Update the widget styles, strings, etc. */
function UpdateSaveSlotScene(float DeltaTime)
{
	local GearEngine GE;
	local UIScene MainMenuScene;
	local GameUISceneClient GameSceneClient;

	if (bIsDeletingCheckpoints)
	{
		GE = GetGearEngine();
		if (GE.PendingCheckpointAction != Checkpoint_DeleteAll)
		{
			CloseUpdatingScene();
			bIsDeletingCheckpoints = false;
			CloseScene(self);

			GameSceneClient = GetSceneClient();
			if (GameSceneClient != none)
			{
				MainMenuScene = GameSceneClient.FindSceneByTag('UI_FE_MainMenu');
				if (MainMenuScene != none)
				{
					GearUISceneFETran_MainMenu(MainMenuScene).NumEmptySaveSlots = 3;
					GearUISceneFETran_MainMenu(MainMenuScene).NumUsedSaveSlots = 0;
				}
			}
		}	
	}

	// Update the current focused index (hack because of us neededing the focused widget when a message box is up, sigh)
	GetCurrentSaveSlotFocused();
}

/**
 * Handler for the OnClick delegate for all of the save slot buttons in this scene.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool SaveSlotButtonClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local GearPC MyGearPC;
	local int Idx;

	MyGearPC = GetGearPlayerOwner( PlayerIndex );
	if ( MyGearPC != None && MyGearPC.ProfileSettings != None )
	{
		Idx = GetCurrentSaveSlotFocused();

		if ( Idx < SaveSlotList.length )
		{
			if ( MyGearPC.ProfileSettings.SetProfileSettingValueInt(MyGearPC.ProfileSettings.LastCheckpointSlot, Idx) )
			{
				// Determine if we are starting a new campaign or loading a checkpoint
				if (SaveSlotList[Idx].EmptySlotPanel.IsVisible())
				{
					// New campaign
					SetTransitionValue("ExitSaveSlot", "New");
				}
				else
				{
					// Continue campaign
					SetTransitionValue("ExitSaveSlot", "Selected");
				}

				// Set the selected difficulty
				SetTransitionValue("SelectedDifficulty", String(int(SaveSlotList[Idx].DifficultyType)));
			}
			else
			{
				`log("Could not save slot"@EGearCampaignMemorySlot(Idx)@"to profile at ID"@MyGearPC.ProfileSettings.LastCheckpointSlot);
			}
		}
		else
		{
			`log(`location@`showvar(Idx)@"Could not find save slot value because"@EventObject@"is not in the button list!");
		}
	}

	TransitionFromSaveSlotScene(PlayerIndex);
	return true;
}

/** Continuing a saved campaign */
function bool OnContinueCampaign(UIScreenObject EventObject, int PlayerIndex)
{
	local int SlotIndex;
	local GearPC MyGearPC;
	local int DiffValue;

	MyGearPC = GetGearPlayerOwner(PlayerIndex);

	if (MyGearPC != None && MyGearPC.ProfileSettings != None)
	{
		// Set the slot index and difficulty
		SlotIndex = GetCurrentSaveSlotFocused();
		if (SlotIndex < SaveSlotList.length && SlotIndex != INDEX_NONE)
		{
			DiffValue = SaveSlotList[SlotIndex].DifficultyType;
			MyGearPC.ProfileSettings.SetProfileSettingValueInt(MyGearPC.ProfileSettings.LastCheckpointSlot, SlotIndex);
			MyGearPC.ProfileSettings.SetProfileSettingValueId(MyGearPC.ProfileSettings.GameIntensity, DiffValue);
		}
		else
		{
			`log(`location@`showvar(SlotIndex)@"Could not find save slot value because GetCurrentSaveSlotFocused failed!");
		}

		// Set up the flow breadcrumbs
		SetTransitionValue("SelectedDifficulty", String(DiffValue));
		SetTransitionValue("ExitSaveSlot", "Continue");
		SetTransitionValue("UseCheckpoint", "Yes");

		// Launch the game
		LaunchSoloCampaign(PlayerIndex, true);
	}
	return true;
}

/** Selecting a slot and moving to act/chapter select */
function bool OnSelectChapter(UIScreenObject EventObject, int PlayerIndex)
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('GenericCancel');
	ButtonAliases.AddItem('AcceptOverwrite');

	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage(
		'NoStorageDevice',
		"<Strings:GearGameUI.MessageBoxStrings.SaveOverwrite_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.SaveOverwriteGeneric_Message>",
		"<Strings:GearGameUI.MessageBoxStrings.SaveOverwrite_Question>",
		ButtonAliases,
		OnCheckpointOverwriteCheckCompleteFromChapterClick,
		GetPlayerOwner(PlayerIndex) );

	return true;
}

/** Return from the prompt asking if the player wanted to overwrite their saved campaign */
function bool OnCheckpointOverwriteCheckCompleteFromChapterClick( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'AcceptOverwrite' )
	{
		Sender.OnSceneDeactivated = OnCheckpointOverwriteCheckCompleteFromChapterClick_FinishedClosing;
	}
	return true;
}

/** Called when the Checkpoint overwrite check is finished closing with a Yes answer */
function OnCheckpointOverwriteCheckCompleteFromChapterClick_FinishedClosing( UIScene ClosingScene )
{
	SaveSlotButtonClicked(none, GetBestPlayerIndex());
}

/** Selecting a slot and moving to the lobby */
function bool OnSelectSlot(UIScreenObject EventObject, int PlayerIndex)
{
	if (!bBlockInputForLobbyTravel)
	{
		SaveSlotButtonClicked(EventObject, PlayerIndex);
	}
	return true;
}

/** Cancel clicked */
function bool OnCancelClicked(UIScreenObject EventObject, int PlayerIndex)
{
	SetTransitionValue("ExitSaveSlot", "Cancel");
	TransitionFromSaveSlotScene(PlayerIndex);
	return true;
}

/** Close the scene or pushes the chapter select screen depending on where we came from to get here */
function TransitionFromSaveSlotScene(int PlayerIndex)
{
	local string EnterString, ExitString, CampaignMode;
	local UIScene SceneToOpen;
	local int SaveSlot;
	local int ChapterValue;
	local GearPC CurrGearPC;
	local int DiffValue;

	ExitString = GetTransitionValue("ExitSaveSlot");
	EnterString = GetTransitionValue("EnterSaveSlot");

	// We are canceling out of the scene
	if (ExitString == "Cancel")
	{
		CloseScene(self);
	}
	// We made a selection or are starting a new campaign
	else
	{
		SaveSlot = Max(0, GetCurrentSaveSlotFocused());
		ChapterValue = int(SaveSlotList[SaveSlot].ChapterType);
		SetTransitionValue("SelectedChapter", string(ChapterValue));

		// We are going back to the lobby after making a selection
		if (EnterString == "Lobby")
		{
			CloseScene(self);
		}
		// We are moving to either the act select screen or the difficulty screen
		else if (EnterString == "MainMenu_Solo")
		{
			// Going to the difficulty screen since we're starting a new game
			if (ExitString == "New" || ExitString == "NewQuick")
			{
				// If they have any chapters unlocked we need to allow them to go to the act screen
				CurrGearPC = GetGearPlayerOwner(PlayerIndex);
				if (CurrGearPC != none &&
					CurrGearPC.ProfileSettings != none &&
					CurrGearPC.ProfileSettings.HasUnlockedAnyChapters() &&
					ExitString == "New")
				{
					// Keep track of the checkpoint chapter so we know if they have checkpoint as an option
					// in the chapter select screen
					SetTransitionValue("SelectedChapter", "0");
					ClearTransitionValue("CheckpointChapter");
					SceneToOpen = UIScene(FindObject("UI_Scenes_FE.UI_FE_SelectAct", class'UIScene'));
				}
				else
				{
					SetTransitionValue("SelectedChapter", "0");
					SetTransitionValue("UseCheckpoint", "No");
					ClearTransitionValue("CheckpointChapter");
					SetTransitionValue("EnterDifficulty", "MainMenu_New");
					SceneToOpen = UIScene(FindObject("UI_Scenes_FE.UI_FESO_Difficulty", class'UIScene'));
				}
				OpenScene(SceneToOpen);
			}
			// Going to the act selection screen
			else
			{
				// Keep track of the checkpoint chapter so we know if they have checkpoint as an option
				// in the chapter select screen
				SetTransitionValue("CheckpointChapter", string(ChapterValue));
				SceneToOpen = UIScene(FindObject("UI_Scenes_FE.UI_FE_SelectAct", class'UIScene'));
				OpenScene(SceneToOpen);
			}
		}
		// Open the lobby
		else
		{
			CampaignMode = GetTransitionValue("Campaign");

			// Check for a network connection first since they may have lost connection since coming to this screen
			if (CampaignMode != "Split" && !class'UIInteraction'.static.HasLinkConnection())
			{
				DisplayErrorMessage("NoLinkConnection_Message", "NoLinkConnection_Title", "GenericContinue", GetPlayerOwner(PlayerIndex));
			}
			else
			{
				// Set the difficulty
				CurrGearPC = GetGearPlayerOwner(PlayerIndex);
				if (CurrGearPC != None && CurrGearPC.ProfileSettings != None)
				{
					// Set the difficulty
					SaveSlot = GetCurrentSaveSlotFocused();
					if (SaveSlot < SaveSlotList.length && SaveSlot != INDEX_NONE)
					{
						DiffValue = SaveSlotList[SaveSlot].DifficultyType;
						CurrGearPC.ProfileSettings.SetProfileSettingValueId(CurrGearPC.ProfileSettings.GameIntensity, DiffValue);
					}
				}

				bBlockInputForLobbyTravel = true;
				AdvanceToCampaignLobby(PlayerIndex);
			}
		}
	}
}

/** New Campaign clicked */
function bool OnNewCampaign(UIScreenObject EventObject, int PlayerIndex)
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local int SlotIndex;
	local bool bIsEmpty;

	SlotIndex = GetCurrentSaveSlotFocused();
	if ( SlotIndex < SaveSlotList.length && SlotIndex != INDEX_NONE )
	{
		bIsEmpty = !CanContinueCampaign(SlotIndex);
		if (!bIsEmpty)
		{
			// Make sure they want to overwrite their saved data
			ButtonAliases.AddItem('GenericCancel');
			ButtonAliases.AddItem('AcceptOverwrite');

			GameSceneClient = GetSceneClient();
			GameSceneClient.ShowUIMessage(
				'SaveOverwrite',
				"<Strings:GearGameUI.MessageBoxStrings.SaveOverwrite_Title>",
				"<Strings:GearGameUI.MessageBoxStrings.SaveOverwrite_Message>",
				"<Strings:GearGameUI.MessageBoxStrings.SaveOverwrite_Question>",
				ButtonAliases,
				OnNewCampaignCheckComplete,
				GetPlayerOwner(PlayerIndex) );
		}
		else
		{
			OnNewCampaignCheckComplete_Finished(none);
		}
	}

	return true;
}

/** Return from the prompt asking if the player wanted to select a device */
function bool OnNewCampaignCheckComplete( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'AcceptOverwrite' )
	{
		Sender.OnSceneDeactivated = OnNewCampaignCheckComplete_Finished;
	}
	return true;
}

/** Callback from OnNewCampaignCheckComplete */
function OnNewCampaignCheckComplete_Finished( UIScene ClosingScene )
{
	local int SlotIndex;
	local GearPC MyGearPC;
	local int DiffValue;
	local int PlayerIndex;

	PlayerIndex = GetBestPlayerIndex();
	MyGearPC = GetGearPlayerOwner( PlayerIndex );
	if ( MyGearPC != None && MyGearPC.ProfileSettings != None )
	{
		SlotIndex = GetCurrentSaveSlotFocused();
		if ( SlotIndex < SaveSlotList.length && SlotIndex != INDEX_NONE )
		{
			if ( !MyGearPC.ProfileSettings.SetProfileSettingValueInt(MyGearPC.ProfileSettings.LastCheckpointSlot, SlotIndex) )
			{
				`log("Could not save slot"@EGearCampaignMemorySlot(SlotIndex)@"to profile at ID"@MyGearPC.ProfileSettings.LastCheckpointSlot);
			}
		}
		else
		{
			`log(`location@`showvar(SlotIndex)@"Could not find save slot value because GetCurrentSaveSlotFocused failed!");
		}
	}

	DiffValue = 0;
	SetTransitionValue("SelectedDifficulty", String(DiffValue));
	SetTransitionValue("ExitSaveSlot", "NewQuick");
	TransitionFromSaveSlotScene(PlayerIndex);
}

/** Called when the scene is activated so we can set the difficulty strings */
function OnSceneActivatedCallback( UIScene ActivatedScene, bool bInitialActivation )
{
	if ( bInitialActivation )
	{
		// Checks for corrupt saves and prompts for deletion
		CheckForCorruptSaves();
	}
}


defaultproperties
{
	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object

	OnGearUISceneTick=UpdateSaveSlotScene
	OnSceneActivated=OnSceneActivatedCallback

	bAllowPlayerJoin=false
	bAllowSigninChanges=false
	LastFocusedSlotIndex=-1
}
