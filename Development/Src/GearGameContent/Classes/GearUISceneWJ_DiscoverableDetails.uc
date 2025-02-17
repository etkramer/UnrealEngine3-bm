/**
 * This class diplays more in-depth information about a single discoverable item, such as detailed description, the date the item was found, etc.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_DiscoverableDetails extends GearUISceneWJ_PopupBase;

var	transient				UIPanel							pnlContent;
var	transient				UILabel							lblActName;
var	transient				UILabel							lblChapterName;
var	transient				UILabel							lblDiscoverableName;
var	transient				UILabel							lblDateFound;

/** displays instructions for how to complete the achievement */
var	transient				UILabel							lblDescription;

var	transient				UIImage							imgBackground;

/** animations for showing / hiding detail text */
var							UIAnimationSeq					HideTextAnimation, ShowTextAnimation;


/**
 * Retrieves the data store markup for the collectible's various images.
 *
 * @param	CollectableIdx				the id for the collectible to retrieve the markup strings for
 * @param	out_UnlockedImageMarkup		receives the markup string for the collectible's high-res unlocked image
 *
 * @return	TRUE if a data store markup string was successfully retrieved for the specified collectible.
 */
function bool GetCollectableImageMarkupStrings( int CollectableIdx, out string out_UnlockedImageMarkup )
{
	return GetDataStoreStringValue("<DynamicGameResource:Collectables;" $ CollectableIdx $ "." $ "UnlockedImage_Markup>", out_UnlockedImageMarkup, Self, GetPlayerOwner());
}

/**
 * Sets the index of the collectable/discoverable this scene will show details for.  Sets the appropriate data store bindings for
 * all widgets in the scene
 *
 * @param	CollectableIndex	the index [into the list of collectable dynamic data providers] for the discoverable/collectable
 *								to show info for.
 */
function SetCollectableIndex( int CollectableIndex )
{
	local EGearAct ActId;
	local EChapterPoint ChapterId;
	local int ActProviderIndex, ChapterProviderIndex;
	local string MarkupString, UnlockedImageMarkup;

	if ( CollectableIndex != INDEX_NONE )
	{
		ActId = class'GearUISceneWJ_Discoverable'.static.GetActIdFromCollectableIndex(CollectableIndex);
		ActProviderIndex = class'GearUIDataStore_GameResource'.static.GetActProviderIndexFromActId(ActId);

		ChapterId = class'GearUISceneWJ_Discoverable'.static.GetChapterIdFromCollectableIndex(CollectableIndex);
		ChapterProviderIndex = class'GearUIDataStore_GameResource'.static.GetChapterProviderIndexFromChapterId(ChapterId);

		lblActName.SetDataStoreBinding("<DynamicGameResource:Acts;" $ ActProviderIndex $ ".DisplayName>");
		lblChapterName.SetDataStoreBinding("<DynamicGameResource:Chapters;" $ ChapterProviderIndex $ ".DisplayName>");

		MarkupString = "<DynamicGameResource:Collectables;" $ CollectableIndex $ ".";
		lblDiscoverableName.SetDataStoreBinding(MarkupString $ "CollectableName>");
		lblDescription.SetDataStoreBinding(MarkupString $ "CollectableDetails>");
		lblDateFound.SetDataStoreBinding(MarkupString $ "DateFound>");
		if ( GetCollectableImageMarkupStrings(CollectableIndex, UnlockedImageMarkup) )
		{
			imgBackground.SetDataStoreBinding(UnlockedImageMarkup);
		}
	}
}

/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	Super.InitializeWidgetReferences();

	pnlContent			=	UIPanel(FindChild('pnlContent', true));
	pnlContent.SetVisibility(true);

	lblActName			=	UILabel(FindChild('lblActName',true));
	lblChapterName		=	UILabel(FindChild('lblChapterName',true));
	lblDiscoverableName	=	UILabel(FindChild('lblName',true));
	lblDateFound		=	UILabel(FindChild('lblDateFound',true));
	lblDescription		=	UILabel(FindChild('lblDescription',true));
	imgBackground		=	UIImage(FindChild('imgPreview',true));

	if ( !btnbarMain.ContainsButton('Generic_A') )
	{
		`assert(btnbarMain.FindChild('btnToggleText',true) == none);
		btnbarMain.CreateCalloutButton('Generic_A', 'btnToggleText');
	}

	// we forgot to add tracking for the date found, and now it's too late, so just hide the label   :(
	lblDateFound.SetVisibility(false);
}


/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	Super.SetupCallbacks();

	btnbarMain.SetButtonCallback('Generic_A', CalloutButtonClicked);
	btnbarMain.SetButtonCaption('Generic_A', "<Strings:GearGameUI.CalloutButtonText.CollectibleDetail_HideText>");
}

/* == Delegate handlers == */
/**
 * Worker for CalloutButtonClicked - only called once all conditions for handling the input have been met.  Child classes should
 * override this function rather than CalloutButtonClicked, unless additional constraints are necessary.
 *
 * @param	InputAliasTag	the callout input alias associated with the input that was received
 * @param	PlayerIndex		index for the player that generated the event
 *
 * @return	TRUE if this click was processed.
 */
function bool HandleCalloutButtonClick( name InputAliasTag, int PlayerIndex )
{
	local bool bResult;

	if ( InputAliasTag == 'Generic_A' )
	{
		if ( pnlContent.Opacity == 1.0 || pnlContent.IsAnimating('ShowTextAnimation') )
		{
			pnlContent.StopUIAnimation('',ShowTextAnimation,false);
			pnlContent.PlayUIAnimation('',HideTextAnimation,,,,false);
			btnbarMain.SetButtonCaption('Generic_A', "<Strings:GearGameUI.CalloutButtonText.CollectibleDetail_ShowText>");
		}
		else
		{
			pnlContent.StopUIAnimation('',HideTextAnimation,false);
			pnlContent.PlayUIAnimation('',ShowTextAnimation,,,,false);
			btnbarMain.SetButtonCaption('Generic_A', "<Strings:GearGameUI.CalloutButtonText.CollectibleDetail_HideText>");
		}

		bResult = true;
	}

	return bResult || Super.HandleCalloutButtonClick(InputAliasTag, PlayerIndex);
}

DefaultProperties
{
	bEnableScenePostProcessing=true

	Begin Object class=UIAnimationSeq Name=ShowTextAnimationTemplate
		SeqName=ShowTextAnimation
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=1.0))))
		//Tracks(1)=(TrackType=EAT_PPBlurAmount,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=0.0))))
	End Object
	ShowTextAnimation=ShowTextAnimationTemplate

	Begin Object class=UIAnimationSeq Name=HideTextAnimationTemplate
		SeqName=HideTextAnimation
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=0.0))))
		//Tracks(1)=(TrackType=EAT_PPBlurAmount,KeyFrames=((RemainingTime=0.125,Data=(DestAsFloat=1.0))))
	End Object
	HideTextAnimation=HideTextAnimationTemplate
}
