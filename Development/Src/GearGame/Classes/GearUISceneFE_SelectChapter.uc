/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFE_SelectChapter extends GearUISceneFrontEnd_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The button bar for the scene */
var transient UICalloutButtonPanel ButtonBar;

/** The chapter buttons */
var transient array<UILabelButton> ChapterButtons;

/** The difficulty buttons for each chapter */
var transient array<UIImage> DifficultyImages;

/** The images used to display a portrait for the act */
var transient UIImage ChapterImage;
var transient UIImage ChapterFadeImage;

/** Animations for fading in and out */
var transient UIAnimationSeq FadeIn;
var transient UIAnimationSeq FadeOut;

/** The checkpoint chapter */
var transient int CheckpointChapter;

/** The data provider of the act this screen is displaying the chapters of */
var transient GearCampaignActData ActData;


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
	local string CheckpointString;
	local string ActString;
	local int ActId;
	local string WidgetName;
	local UILabelButton UnusedButton;
	local int FirstEnabledChapterIndex;
	local UILabelButton LastEnabledButton;
	local UILabelButton ButtonToFocus;

	ButtonToFocus = none;

	// Button bar
	ButtonBar = UICalloutButtonPanel(FindChild('pnlButtonBar', true));
	ButtonBar.SetButtonCallback('GenericBack', OnBackClicked);

	// Grab the checkpoint chapter if there is one
	CheckpointString = GetTransitionValue("CheckpointChapter");
	CheckpointChapter = (CheckpointString == "") ? -1 : int(CheckpointString);
	// Grab the act provider
	ActString = GetTransitionValue("SelectedAct");
	ActId = int(ActString);
	ActData = class'GearUIDataStore_GameResource'.static.GetActDataProvider(EGearAct(ActId));

	// Use the act provider to build the buttons
	ChapterButtons.length = ActData.ChapterProviders.length;
	if (ActData != none)
	{
		FirstEnabledChapterIndex = -1;
		// Initialize the used buttons
		for (Idx = 0; Idx < ChapterButtons.length; Idx++)
		{
			WidgetName = "btnChap" $ Idx;
			ChapterButtons[Idx] = UILabelButton(FindChild(name(WidgetName), true));
			ChapterButtons[Idx].SetDataStoreBinding(ActData.ChapterProviders[Idx].ListName);
			// If we have access to this chapter set the OnClicked delegate
			if (CanAccessChapter(Idx))
			{
				ChapterButtons[Idx].OnClicked = OnChapterClicked;
				ChapterButtons[Idx].SetEnabled(true);
				LastEnabledButton = ChapterButtons[Idx];
				if (FirstEnabledChapterIndex < 0)
				{
					FirstEnabledChapterIndex = Idx;
				}
				// See if this is the checkpoint chapter
				if (CheckpointChapter == ActData.ChapterProviders[Idx].ChapterType)
				{
					ButtonToFocus = ChapterButtons[Idx];
				}
			}
			// else disable the button
			else
			{
				ChapterButtons[Idx].SetEnabled(false);
			}

			DifficultyImages[Idx] = UIImage(ChapterButtons[Idx].FindChild('imgDifficulty', true));
			SetDifficultyImage(Idx);
		}

		// Hide the unused buttons
		for (Idx = ChapterButtons.length; Idx < 8; Idx++)
		{
			WidgetName = "btnChap" $ Idx;
			UnusedButton = UILabelButton(FindChild(name(WidgetName), true));
			UnusedButton.SetVisibility(false);
		}
	}

	// Set navigation
	if (ChapterButtons[FirstEnabledChapterIndex] != none && LastEnabledButton != none && ChapterButtons[FirstEnabledChapterIndex] != LastEnabledButton)
	{
		ChapterButtons[FirstEnabledChapterIndex].SetForcedNavigationTarget(UIFACE_Top, LastEnabledButton);
		LastEnabledButton.SetForcedNavigationTarget(UIFACE_Bottom, ChapterButtons[FirstEnabledChapterIndex]);
	}

	// Portrait images
	ChapterImage = UIImage(FindChild('imgChapter', true));
	ChapterFadeImage = UIImage(FindChild('imgChapterFade', true));

	if (ButtonToFocus != none)
	{
		ButtonToFocus.SetFocus(none);
	}
	else
	{
		ChapterButtons[FirstEnabledChapterIndex].SetFocus(none);
	}
}

/** Whether the checkpoint chapter is in the current act that was selected */
function bool CheckpointChapterIsActive()
{
	local int Idx;

	if (CheckpointChapter >= 0)
	{
		for (Idx = 0; Idx < ActData.ChapterProviders.length; Idx++)
		{
			if (ActData.ChapterProviders[Idx].ChapterType == CheckpointChapter)
			{
				return true;
			}
		}
	}
	return false;
}

/** Set the difficulty image for the chapter */
function SetDifficultyImage(int ChapterId)
{
	local int PlayerIndex;
	local GearPC GPC;
	local int Idx;
	local string ImagePath;
	local EChapterPoint ChapterType;

	ImagePath = "";
	PlayerIndex = GetBestPlayerIndex();
	GPC = GetGearPlayerOwner(PlayerIndex);
	ChapterType = ActData.ChapterProviders[ChapterId].ChapterType;

	if (GPC != none && GPC.ProfileSettings != none)
	{
		for (Idx = DL_MAX-1; Idx >= 0; Idx--)
		{
			// Check the chapter after this one since you have to have unlocked the next chapter to complete this one
			if (GPC.ProfileSettings.HasChapterBeenUnlocked(EChapterPoint(ChapterType+1), EDifficultyLevel(Idx)))
			{
				break;
			}
		}

		switch (Idx)
		{
			case DL_Casual:		ImagePath = class'GearChapterComboProvider'.default.DifficultyImagePaths[DL_Casual];	break;
			case DL_Normal:		ImagePath = class'GearChapterComboProvider'.default.DifficultyImagePaths[DL_Normal];	break;
			case DL_Hardcore:	ImagePath = class'GearChapterComboProvider'.default.DifficultyImagePaths[DL_Hardcore];	break;
			case DL_Insane:		ImagePath = class'GearChapterComboProvider'.default.DifficultyImagePaths[DL_Insane];	break;
			default:
				if (!GPC.ProfileSettings.HasChapterBeenUnlockedForAccess(ChapterType))
				{
					ImagePath = "<Images:UI_Art_WarJournal.Elements.WJ_Icon_Achieve_Lock>";
				}
				break;
		}
	}

	if (ImagePath == "")
	{
		DifficultyImages[ChapterId].SetVisibility(false);
	}
	else
	{
		DifficultyImages[ChapterId].SetVisibility(true);
		DifficultyImages[ChapterId].SetDataStoreBinding(ImagePath);
	}
}

/** Whether we have access to the chapter or not (unlocked) */
function bool CanAccessChapter(int ChapterId)
{
	local int PlayerIndex;
	local GearPC GPC;
	local EChapterPoint ChapterType;

	PlayerIndex = GetBestPlayerIndex();
	GPC = GetGearPlayerOwner(PlayerIndex);
	ChapterType = ActData.ChapterProviders[ChapterId].ChapterType;

	if (GPC != None && GPC.ProfileSettings != None)
	{
		return GPC.ProfileSettings.HasChapterBeenUnlockedForAccess(ChapterType);
	}
	return false;
}

/** Called when the checkpoint button is clicked */
function bool OnCheckpointClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local UIScene SceneToOpen;

	// Keep track of the act selected and progress to the chapter select screen
	SetTransitionValue("SelectedChapter", string(CheckpointChapter));
	SetTransitionValue("UseCheckpoint", "Yes");
	SetTransitionValue("EnterDifficulty", "ChapterSelect_Continue");
	SceneToOpen = UIScene(DynamicLoadObject("UI_Scenes_FE.UI_FESO_Difficulty", class'UIScene'));
	OpenScene(SceneToOpen);
	return true;
}

/** Called when a chapter button is clicked */
function bool OnChapterClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local int Idx;
	local UIScene SceneToOpen;
	local int ChapterType;

	// Find the matching widget so we know which act to use
	for (Idx = 0; Idx < ChapterButtons.length; Idx++)
	{
		if (EventObject == ChapterButtons[Idx])
		{
			break;
		}
	}

	ChapterType = ActData.ChapterProviders[Idx].ChapterType;

	// Keep track of the act selected and progress to the chapter select screen
	SetTransitionValue("SelectedChapter", string(ChapterType));
	SetTransitionValue("UseCheckpoint", "No");
	SetTransitionValue("EnterDifficulty", "ChapterSelect_Restart");
	SceneToOpen = UIScene(DynamicLoadObject("UI_Scenes_FE.UI_FESO_Difficulty", class'UIScene'));
	OpenScene(SceneToOpen);
	return true;
}

/** Back clicked */
function bool OnBackClicked(UIScreenObject EventObject, int PlayerIndex)
{
	CloseScene();
	return true;
}

/** Update the act portrait */
function OnStateChanged(UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, UIState PreviouslyActiveState)
{
	local int Idx;

	// Update the portrait if a new button is focused
	for (Idx = 0; Idx < ChapterButtons.length; Idx++)
	{
		if (ChapterButtons[Idx] != None &&
			ChapterButtons[Idx] == Sender &&
			NewlyActiveState.IsA('UIState_Focused'))
		{
			ChapterSelectionChanged(Idx, PlayerIndex);
			break;
		}
	}

	Super.OnStateChanged( Sender, PlayerIndex, NewlyActiveState, PreviouslyActiveState );
}

/** Called when a new chapter button takes focus */
function ChapterSelectionChanged(int ChapterId, int PlayerIndex)
{
	local GearCampaignChapterData ChapData;

	// Checkpoint
	if (ChapterId == -1)
	{
		ChapData = class'GearUIDataStore_GameResource'.static.GetChapterDataProvider(EChapterPoint(CheckpointChapter));
		CrossFade(ChapData.ScreenshotPathName);
	}
	// Chapter
	else
	{
		CrossFade(ActData.ChapterProviders[ChapterId].ScreenshotPathName);
	}
}

/**
 * Crossfade between the current act portrait and another act portrait.
 *
 * @param NewTargetImage	The target image to fade to
 */
function CrossFade(string NewTargetImage)
{
	local string CurrentImage;

	ChapterFadeImage.SetVisibility(true);
	CurrentImage = ChapterImage.GetDataStoreBinding();

	if (CurrentImage != NewTargetImage)
	{
		ChapterImage.SetDataStoreBinding(NewTargetImage);
		ChapterImage.Opacity = 0.0f;
		ChapterImage.PlayUIAnimation('', FadeIn,,,, FALSE);

		ChapterFadeImage.SetDataStoreBinding(CurrentImage);
		ChapterFadeImage.Opacity = 1.0f;
		ChapterFadeImage.PlayUIAnimation('', FadeOut,,,, FALSE);
	}
}

defaultproperties
{
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

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object

	bAllowPlayerJoin=false
	bAllowSigninChanges=false
}
