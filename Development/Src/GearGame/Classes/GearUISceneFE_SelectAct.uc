/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFE_SelectAct extends GearUISceneFrontEnd_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The button bar for the scene */
var transient UICalloutButtonPanel ButtonBar;

/** The button labels used to display the act name */
var transient array<UILabelButton> ActButtons;

/** The images that display the highest difficulty they beat an act in */
var transient array<UIImage> DifficultyImages;

/** The images used to display a portrait for the act */
var transient UIImage ActImage;
var transient UIImage ActFadeImage;

/** Animations for fading in and out */
var transient UIAnimationSeq FadeIn;
var transient UIAnimationSeq FadeOut;


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
	local string WidgetName;
	local int ChapterValue;
	local GearCampaignActData ActData;
	local UILabelButton LastEnabledButton;

	// Button bar
	ButtonBar = UICalloutButtonPanel(FindChild('pnlButtonBar', true));
	ButtonBar.SetButtonCallback('GenericBack', OnBackClicked);

	// Act buttons and diff images
	ActButtons.length = 5;
	DifficultyImages.length = ActButtons.length;
	for (Idx = 0; Idx < ActButtons.length; Idx++)
	{
		WidgetName = "btnAct" $ Idx;
		ActButtons[Idx] = UILabelButton(FindChild(name(WidgetName), true));
		// If we have access to this act set the OnClicked delegate
		if (CanAccessAct(Idx))
		{
			ActButtons[Idx].OnClicked = OnActButtonClicked;
			ActButtons[Idx].SetEnabled(true);
			LastEnabledButton = ActButtons[Idx];
		}
		// else disable the button
		else
		{
			ActButtons[Idx].SetEnabled(false);
		}

		DifficultyImages[Idx] = UIImage(ActButtons[Idx].FindChild('imgDifficulty', true));
		SetDifficultyImage(Idx);
	}

	// Set navigation
	if (ActButtons[0] != LastEnabledButton && ActButtons[0] != none && LastEnabledButton != none)
	{
		ActButtons[0].SetForcedNavigationTarget(UIFACE_Top, LastEnabledButton);
		LastEnabledButton.SetForcedNavigationTarget(UIFACE_Bottom, ActButtons[0]);
	}

	// Portrait images
	ActImage = UIImage(FindChild('imgAct', true));
	ActFadeImage = UIImage(FindChild('imgActFade', true));

	// Set the focus based on the selected chapter
	ChapterValue = int(GetTransitionValue("SelectedChapter"));
	ActData = class'GearUIDataStore_GameResource'.static.GetActDataProviderUsingChapter(EChapterPoint(ChapterValue));
	if (ActData != none)
	{
		ActButtons[ActData.ActType].SetFocus(none);
	}
}

/** Set the difficulty image for the act */
function SetDifficultyImage(int ActId)
{
	local int PlayerIndex;
	local GearPC GPC;
	local int Idx;
	local string ImagePath;

	ImagePath = "";
	PlayerIndex = GetBestPlayerIndex();
	GPC = GetGearPlayerOwner(PlayerIndex);

	if (GPC != none && GPC.ProfileSettings != none)
	{
		for (Idx = DL_MAX-1; Idx >= 0; Idx--)
		{
			if (GPC.ProfileSettings.HasActBeenCompleted(EGearAct(ActId), EDifficultyLevel(Idx)))
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
				if (!GPC.ProfileSettings.HasActBeenUnlockedForAccess(EGearAct(ActId)))
				{
					ImagePath = "<Images:UI_Art_WarJournal.Elements.WJ_Icon_Achieve_Lock>";
				}
				break;
		}
	}

	if (ImagePath == "")
	{
		DifficultyImages[ActId].SetVisibility(false);
	}
	else
	{
		DifficultyImages[ActId].SetVisibility(true);
		DifficultyImages[ActId].SetDataStoreBinding(ImagePath);
	}
}

/** Whether the act should be enabled or not */
function bool CanAccessAct(int ActId)
{
	local int PlayerIndex;
	local GearPC GPC;

	PlayerIndex = GetBestPlayerIndex();
	GPC = GetGearPlayerOwner(PlayerIndex);
	if (GPC != None && GPC.ProfileSettings != None)
	{
		return GPC.ProfileSettings.HasActBeenUnlockedForAccess(EGearAct(ActId));
	}
	return false;
}

/** Called when an act button is clicked */
function bool OnActButtonClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local int Idx;
	local UIScene SceneToOpen;

	// Find the matching widget so we know which act to use
	for (Idx = 0; Idx < ActButtons.length; Idx++)
	{
		if (EventObject == ActButtons[Idx])
		{
			break;
		}
	}

	// Keep track of the act selected and progress to the chapter select screen
	SetTransitionValue("SelectedAct", string(Idx));
	SceneToOpen = UIScene(DynamicLoadObject("UI_Scenes_FE.UI_FE_SelectChapter", class'UIScene'));
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
	for (Idx = 0; Idx < ActButtons.length; Idx++)
	{
		if (ActButtons[Idx] != None &&
			ActButtons[Idx] == Sender &&
			NewlyActiveState.IsA('UIState_Focused'))
		{
			ActSelectionChanged(Idx, PlayerIndex);
			break;
		}
	}

	Super.OnStateChanged( Sender, PlayerIndex, NewlyActiveState, PreviouslyActiveState );
}

/** Called when a new act button takes focus */
function ActSelectionChanged(int ActId, int PlayerIndex)
{
	local GearCampaignActData ActProvider;

	ActProvider = class'GearUIDataStore_GameResource'.static.GetActDataProvider(EGearAct(ActId));
	if (ActProvider != none)
	{
		CrossFade(ActProvider.ScreenshotPathName);
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

	ActFadeImage.SetVisibility(true);
	CurrentImage = ActImage.GetDataStoreBinding();

	if (CurrentImage != NewTargetImage)
	{
		ActImage.SetDataStoreBinding(NewTargetImage);
		ActImage.Opacity = 0.0f;
		ActImage.PlayUIAnimation('', FadeIn,,,, FALSE);

		ActFadeImage.SetDataStoreBinding(CurrentImage);
		ActFadeImage.Opacity = 1.0f;
		ActFadeImage.PlayUIAnimation('', FadeOut,,,, FALSE);
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
