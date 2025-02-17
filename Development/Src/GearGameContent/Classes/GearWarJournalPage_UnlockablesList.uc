/**
 * This panel displays the list of unlockable content.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearWarJournalPage_UnlockablesList extends GearWarJournalPage_Base;

var	transient		UILabel				lblPageTitle;
var	transient		UIProgressBar		barCompletion;
var	transient		UILabel				lblCompletionValue;
var	transient		UIList				lstUnlockables;
var	transient		UIPanel				pnlCheckboxes;
var	transient		array<UICheckbox>	chkLockedIcons;

var	transient		UILabel				lblDescription;
var	transient		UIImage				imgPreview;

var	const			string				LockedImageMarkup;

/** indicates that we altered something in the profile which requires it to be saved */
var	transient		bool				bSaveProfile;

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Wrapper for grabbing the UnlockableId for the unlockable at the specified index in the data store
 */
function EGearUnlockable GetUnlockableId( int UnlockableIndex )
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local UIProviderScriptFieldValue FieldValue;
	local EGearUnlockable Result;

	DynamicResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(GetPlayerOwner());
	if ( DynamicResourceDS != None && DynamicResourceDS.GetProviderFieldValue('Unlockables', 'UnlockableId', UnlockableIndex, FieldValue) )
	{
		Result = EGearUnlockable(class'GearUIScene_Base'.static.ConvertEnumStringToValue(FieldValue.StringValue, eUNLOCK_MAX, enum'EGearUnlockable'));
	}

	return Result;
}

/**
 * Wrapper for checking whether an unlockable has been unlocked.
 */
function bool IsUnlocked( int UnlockableIndex )
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local UIProviderScriptFieldValue FieldValue;
	local bool bResult;

	DynamicResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(GetPlayerOwner());
	if ( DynamicResourceDS != None && DynamicResourceDS.GetProviderFieldValue('Unlockables', 'IsUnlocked', UnlockableIndex, FieldValue) )
	{
		bResult = bool(FieldValue.StringValue);
	}

	return bResult;
}

/**
 * Wrapper for checking whether an unlockable has been unlocked but never viewed.
 *
 * @return	TRUE if the unlockable has been unlocked but not viewed.
 */
function bool IsNewlyUnlocked( int UnlockableIndex )
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local UIProviderScriptFieldValue FieldValue;
	local bool bResult;

	DynamicResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(GetPlayerOwner());
	if ( DynamicResourceDS != None && DynamicResourceDS.GetProviderFieldValue('Unlockables', 'HasUpdatedData', UnlockableIndex, FieldValue) )
	{
		bResult = bool(FieldValue.StringValue);
	}

	return bResult;
}

/**
 * Wrapper for checking whether the player has downloaded specific downloadable content.
 *
 * @param	UnlockableIndex	the index for the unlockable to check for
 *
 * @return	TRUE if the unlockable specified is DLC and the player has downloaded it.
 */
function bool HasDownloadedContent( int UnlockableIndex )
{
	local bool bResult;

	if ( UnlockableIndex >= eUNLOCK_Character_DLC1 && UnlockableIndex < eUNLOCK_Character_None )
	{
		//@todo ?
		bResult = false;
	}

	return bResult;
}

/**
 * Calculates the percentage of unlockable content that has been unlocked in the current profile, then updates the progress bar with this value.
 */
function CalculateCompletionProgress()
{
	local int UnlockableIndex, NumUnlockedItems, TotalItems;
	local float Ratio;

	TotalItems = lstUnlockables.GetItemCount();
	`assert(chkLockedIcons.Length >= TotalItems);

	for ( UnlockableIndex = 0; UnlockableIndex < TotalItems; UnlockableIndex++ )
	{
		if ( IsUnlocked(UnlockableIndex) )
		{
			NumUnlockedItems++;
		}
		else if ( UnlockableIndex >= eUNLOCK_Character_DLC1 )
		{
			// if this is a checkbox for DLC and it hasn't been downloaded, don't include it in the progress
			if ( !HasDownloadedContent(UnlockableIndex) )
			{
				TotalItems = UnlockableIndex;
				break;
			}
		}
	}

	barCompletion.ProgressBarValue.MinValue = 0;
	barCompletion.ProgressBarValue.MaxValue = TotalItems;
	barCompletion.SetValue(NumUnlockedItems, false);

	Ratio = (float(NumUnlockedItems) / float(TotalItems)) * 100;
	lblCompletionValue.SetValue(string(int(Ratio)) @ "%");
}

/**
 * Retrieves the data store markup for the unlockable's "unlocked" image.
 *
 * @param	UnlockableIndex		the id for the unlockable to retrieve the markup string for
 * @param	MarkupString		receives the value of the unlockabe's image markup string
 *
 * @return	TRUE if a data store markup string was successfully retrieved for the specified unlockable.
 */
function bool GetUnlockableImageMarkup( int UnlockableIndex, out string MarkupString )
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local UIProviderScriptFieldValue FieldValue;
	local bool bResult;

	if ( UnlockableIndex >= 0 && UnlockableIndex < eUNLOCK_Character_None )
	{
		DynamicResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(GetPlayerOwner());
		if ( DynamicResourceDS != None && DynamicResourceDS.GetProviderFieldValue('Unlockables', 'UnlockableImage', UnlockableIndex, FieldValue) )
		{
			MarkupString = FieldValue.StringValue;
			bResult = true;
		}
	}

	return bResult;
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Handler for OnValueChanged delegate of the list of unlockables.  Updates the description text and preview image with the data for the selected
 * unlockable content.
 *
 * @param	Sender			the UIObject whose value changed
 * @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 */
function UnlockableIndexChanged( UIObject Sender, int PlayerIndex )
{
	local int SelectedItem;
	local string MarkupString;
	local GearProfileSettings Profile;
	local GearUIScene_Base GearOwnerScene;
	local EGearUnlockable Unlockable;

	SelectedItem = lstUnlockables.GetCurrentItem();
	Unlockable = GetUnlockableId(SelectedItem);

	//@todo - should the description be hidden until it's unlocked?
	lblDescription.SetDataStoreBinding("<DynamicGameResource:Unlockables;" $ SelectedItem $ ".UnlockableDescription>");
	if ( IsUnlocked(SelectedItem) && GetUnlockableImageMarkup(SelectedItem, MarkupString) )
	{
		imgPreview.SetDataStoreBinding(MarkupString);
	}
	else
	{
		imgPreview.SetDataStoreBinding(LockedImageMarkup);
	}

	if ( IsNewlyUnlocked(SelectedItem) )
	{
		GearOwnerScene = GearUIScene_Base(GetScene());
		Profile = GearOwnerScene.GetPlayerProfile(PlayerIndex);

		if ( Profile != None )
		{
			bSaveProfile = Profile.MarkUnlockableAsViewed(Unlockable) || bSaveProfile;
			if ( !IsNewlyUnlocked(SelectedItem) )
			{
				//@todo ronp - technically, this should happen as a result of calling Profile.MarkUnlockableAsViewed (which would need to
				// notify the data provider that the 'needs viewing' value had changed; data provider would then tell its subscribers
				// to refresh) but this is a bit easier
				lstUnlockables.RefreshSubscriberValue();
				lstUnlockables.SetIndex(lstUnlockables.Items.Find(SelectedItem));
			}
		}
	}
}


/* == GearWarJournalPage_Base interface == */
/**
 * Assigns all member variables to the appropriate child widgets in this panel.
 */
function InitializePageReferences()
{
	local int i, UnlockableIndex;
	local UIScene SceneOwner;
	local UICheckbox checkbox;

	lblPageTitle = UILabel(FindChild('lblPageTitle',true));

	barCompletion = UIProgressBar(FindChild('barCompletion',true));
	lblCompletionValue = UILabel(FindChild('lblCompletionValue',true));

	lstUnlockables = UIList(FindChild('lstUnlockables',true));
	lstUnlockables.OnValueChanged = UnlockableIndexChanged;

	//@todo ronp - do we need an OnClick handler...for example to show the UnlockabesInfo scene?

	pnlCheckboxes = UIPanel(FindChild('pnlCheckboxes',true));
	for ( i = pnlCheckboxes.Children.Length - 1; i >= 0; i-- )
	{
		checkbox = UICheckBox(pnlCheckboxes.Children[i]);
		if ( checkbox != None )
		{
			UnlockableIndex = chkLockedIcons.Length;
			chkLockedIcons[UnlockableIndex] = checkbox;

			if ( UnlockableIndex >= eUNLOCK_Character_DLC1 && UnlockableIndex < eUNLOCK_Character_None )
			{
				checkbox.SetVisibility(HasDownloadedContent(UnlockableIndex));
			}
		}
	}

	SceneOwner = GetScene();
	lblDescription = UILabel(SceneOwner.FindChild('lblDescription',true));
	imgPreview = UIImage(SceneOwner.FindChild('imgPreview', true));

	Super.InitializePageReferences();
}

/**
 * Changes the index for this panel when the user "flips" to a new journal page.
 *
 * @param	NewPageIndex	the index to assign to this page.
 */
function SetPageIndex( int NewPageIndex )
{
	Super.SetPageIndex(NewPageIndex);

	CalculateCompletionProgress();
}

DefaultProperties
{
	LockedImageMarkup="<Images:UI_Art_WarJournal.Unlock.WJ_Unlock_Locked>"
}
