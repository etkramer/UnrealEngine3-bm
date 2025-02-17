/**
 * This panel displays information about a single collectable item in the Collectables section of the Gears2 War Journal.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearCollectablePanel extends GearWJPrefabPanelBase;

`include(Engine/Classes/UIDev.uci)

/** the image that displays the collectable icon */
var	transient		UIImage					imgBackgroundLocked;
var	transient		UIImage					imgBackground;

/** the label that displays the exclamation point when the player has attained a new collectible */
var	transient		UILabel					lblAttractIcon;

/**
 * Index into the list of collectables for the collectable being displayed by this panel.
 */
var	transient		int						CollectableIndex;

/**
 * Id for the collectable being displayed in this panel.
 */
//var	transient		EGearDiscoverableType	CollectableId;

/* == Delegates == */
/**
 * Called when the user requests the details for this discoverable.  Handler should open the discoverable details scene.
 *
 * @param	PlayerIndex		index of the player that generated the event.
 * @param	DiscoverableIdx	the index into the list of collectables for the collectable being displayed by this panel.
 */
delegate OnDisplayDetails( int PlayerIndex, int DiscoverableIdx );

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Wrapper for checking whether a collectible has been found.
 *
 * @param	CollectableIdx					the id for the collectible to check
 *
 */
final function bool IsCollectableUnlocked( int CollectableIdx )
{
	local UIProviderFieldValue FieldValue;
	local bool bResult;

	if ( GetDataStoreFieldValue("<DynamicGameResource:Collectables;" $ CollectableIdx $ ".HasBeenFound>", FieldValue, GetScene(), GetPlayerOwner()) )
	{
		bResult = bool(FieldValue.ArrayValue[0]);
	}
	else
	{
		`log("GearCollectablePanel::IsCollectableUnlocked: failed to resolve data store markup for collectable" @ CollectableIdx @ "using markup: <DynamicGameResource:Collectables;" $ CollectableIdx $ ".HasBeenFound>");
	}

	return bResult;
}

/**
 * Retrieves the data store markup for the collectible's various images.
 *
 * @param	CollectableIdx					the id for the collectible to retrieve the markup strings for
 * @param	out_LockedImageMarkup			receives the markup string for the collectible's "locked" image
 * @param	out_UnlockedImageMarkup			receives the markup string for the collectible's "unlocked" image
 * @param	out_BackgroundImageMarkup		receives the markup string for the collectible's "unlocked image background
 *
 * @return	TRUE if a data store markup string was successfully retrieved for the specified collectible.
 */
function bool GetCollectableImageMarkupStrings( int CollectableIdx, out string out_LockedImageMarkup, out string out_UnlockedImageMarkup, out string out_BackgroundImageMarkup )
{
	local bool bResult;
	local string MarkupString;
	local UIScene OwningScene;
	local LocalPlayer PlayerOwner;

	OwningScene = GetScene();
	PlayerOwner = OwningScene.GetPlayerOwner();

	MarkupString = "<DynamicGameResource:Collectables;" $ CollectableIdx $ ".";
	bResult	=	GetDataStoreStringValue( MarkupString $ "LockedIcon_Markup>", out_LockedImageMarkup, OwningScene, PlayerOwner)
			&&	GetDataStoreStringValue( MarkupString $ "UnlockedIcon_Markup>", out_UnlockedImageMarkup, OwningScene, PlayerOwner)
			&&	GetDataStoreStringValue( MarkupString $ "UnlockedImage_Markup>", out_BackgroundImageMarkup, OwningScene, PlayerOwner);

	return bResult;
}

/**
 * Assigns an achievement to this panel.
 *
 * @param	NewCollectableIndex		index [into the data store's list of collectables] for the collectable this panel
 *									should display.
 */
function SetCollectableIndex( int NewCollectableIndex )
{
	CollectableIndex = NewCollectableIndex;

	// update the icons and title with the appropriate values
	UpdateCollectableData();


	//@todo - CollectableId?
}

/**
 * Updates the data store bindings for all controls in this panel with the markup required to access the data for the associated collectable.
 */
function UpdateCollectableData()
{
	local string LockedImageMarkup, UnlockedImageMarkup, BackgroundImageMarkup;
	local bool bHasFoundCollectable;

	if ( CollectableIndex != INDEX_NONE )
	{
		SetVisibility(true);

		bHasFoundCollectable = IsCollectableUnlocked(CollectableIndex);

		imgBackgroundLocked.SetVisibility(!bHasFoundCollectable);
		imgBackground.SetVisibility(bHasFoundCollectable);
		imgIcon.SetVisibility(bHasFoundCollectable);
		imgSelectionBar.SetVisibility(bHasFoundCollectable && IsFocused());

		lblName.SetVisibility(bHasFoundCollectable);
		lblName.SetDataStoreBinding("<DynamicGameResource:Collectables;" $ CollectableIndex $ ".CollectableName>");
		lblAttractIcon.SetVisibility(bHasFoundCollectable && IsNewlyUnlocked());

`if(`isdefined(dev_build))
		if ( !bHasFoundCollectable )
		{
			lblName.SetVisibility(true);
			lblName.SetDataStoreBinding(string(GetCollectableIdFromCollectableIndex(CollectableIndex)));
		}
`endif

		// first clear any existing textures from the images in case there is no datastore markup for this collectable
		imgBackgroundLocked.SetValue(None);
		imgBackground.SetValue(None);
		imgIcon.SetValue(None);
		imgBackgroundLocked.SetDataStoreBinding("");
		imgBackground.SetDataStoreBinding("");
		imgIcon.SetDataStoreBinding("");

		if ( GetCollectableImageMarkupStrings(CollectableIndex, LockedImageMarkup, UnlockedImageMarkup, BackgroundImageMarkup) )
		{
//		`log(`showvar(GetCollectableIdFromCollectableIndex(CollectableIndex),CollectableIndex)
//			@ `showvar(LockedImageMarkup,LockedIcon_Markup) @ `showvar(BackgroundImageMarkup,BackgroundImage) @ `showvar(UnlockedImageMarkup,IconImage)
//			@ "(" $ imgBackgroundLocked.IsVisible() $ "," $ imgBackground.IsVisible() $ "," $ imgIcon.IsVisible() $ ")");

			imgBackgroundLocked.SetDataStoreBinding(LockedImageMarkup);
			imgBackground.SetDataStoreBinding(BackgroundImageMarkup);
			imgIcon.SetDataStoreBinding(UnlockedImageMarkup);
		}
	}
	else
	{
		SetVisibility(false);
	}
}

/**
 * Wrapper for retrieving the collectable id of a specific collectable.
 *
 * @param	CollectableIndex	the index [into the complete list of collectables] for the collectable to get the collectable id for.
 *
 * @return	the EGearDiscoverableType value for the specified collectable.
 */
static function EGearDiscoverableType GetCollectableIdFromCollectableIndex( int CollectableIdx )
{
	local EGearDiscoverableType Result;
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue FieldValue;

	GameResourceDS = GearUIDataStore_GameResource(StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS.GetProviderFieldValue('Collectables', 'CollectableId', CollectableIdx, FieldValue) )
	{
		Result = EGearDiscoverableType(class'GearUIScene_Base'.static.ConvertEnumStringToValue(FieldValue.StringValue, eDISC_MAX, enum'EGearDiscoverableType'));
	}
	else
	{
		`log("GetCollectableIdFromCollectableIndex - couldn't get field value for 'CollectableId' using CollectableIndex" @ CollectableIdx);
	}

	return Result;
}

/**
 * Wrapper for checking whether the collectable associated with this panel has been unlocked but never viewed.
 */
function bool IsNewlyUnlocked()
{
	local UIDataStore_DynamicResource DynamicResourceDS;
	local UIProviderScriptFieldValue FieldValue;
	local bool bResult;

	DynamicResourceDS = class'GearUIScene_Base'.static.GetDynamicResourceDataStore(GetPlayerOwner());
	if ( DynamicResourceDS != None && DynamicResourceDS.GetProviderFieldValue('Collectables', 'HasUpdatedData', CollectableIndex, FieldValue) )
	{
		bResult = bool(FieldValue.StringValue);
	}

	return bResult;
}

/**
 * Wrapper for marking the collectable associated with this panel as having been viewed.
 */
function MarkCollectableAsViewed()
{
	local EGearDiscoverableType CollectableId;
	local GearUIScene_Base GearOwnerScene;
	local GearProfileSettings Profile;

	if ( IsNewlyUnlocked() )
	{
		CollectableId = GetCollectableIdFromCollectableIndex(CollectableIndex);

		GearOwnerScene = GearUIScene_Base(GetScene());
		Profile = GearOwnerScene.GetPlayerProfile(GetPlayerOwnerIndex());

		if ( Profile != None && Profile.MarkDiscoverableAsViewed(CollectableId) )
		{
			lblAttractIcon.SetVisibility(false);
		}
	}
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Handler for this widget's OnProcessInputKey delegate.  Responsible for displaying this achievement's details when the correct
 * button is pressed.
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
	local bool bResult;

	if ( EventParms.InputAliasName == 'ViewWJItemDetails' )
	{
		if ( EventParms.EventType == IE_Released && CollectableIndex != INDEX_NONE )
		{
			`log(`location @ "displaying collectable details for player" @ EventParms.PlayerIndex,,'RON_DEBUG');
			OnDisplayDetails(EventParms.PlayerIndex, CollectableIndex);

			MarkCollectableAsViewed();
		}

		bResult = true;
	}

	return bResult || Super.ProcessInputKey(EventParms);
}

/* === GearWJPanelBase interface === */
/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	Super.InitializeWidgetReferences();

	imgBackground = UIImage(FindChild('imgBackground',true));
	imgBackgroundLocked = UIImage(FindChild('imgBackgroundLocked',true));
	lblAttractIcon = UILabel(FindChild('lblAttractIcon',true));
}

DefaultProperties
{
	WidgetTag=CollectablePanel
	CollectableIndex=INDEX_NONE
}

