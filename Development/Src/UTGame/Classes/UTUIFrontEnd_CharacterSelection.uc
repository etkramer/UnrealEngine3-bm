/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Character selection screen for UT3.
 */
class UTUIFrontEnd_CharacterSelection extends UTUIFrontEnd;

/** Preview image. */
var transient UIImage	PreviewImage;

/** Description label. */
var transient UILabel	DescriptionLabel;

/** List widget. */
var transient UTUIList CharacterList;

/** Character customization scene to display once the user chooses a character type. */
var string CharacterCustomizationScene;

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize( )
{
	Super.PostInitialize();

	// Setup delegates
	CharacterList = UTUIList(FindChild('lstCharacters', true));
	if(CharacterList != none)
	{
		CharacterList.OnSubmitSelection = OnCharacterList_SubmitSelection;
		CharacterList.OnValueChanged = OnCharacterList_ValueChanged;
	}

	// Store widget references
	PreviewImage = UIImage(FindChild('imgPreview1', true));
	DescriptionLabel = UILabel(FindChild('lblDescription', true));

	// Activate kismet for entering scene
	ActivateLevelEvent('CharacterSelectionEnter');
}

/** Setup the scene's button bar. */
function SetupButtonBar()
{
	ButtonBar.Clear();
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Next>", OnButtonBar_Next);
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
	local bool bResult;

	bResult=false;

	if(EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
		{
			OnBack();
			bResult=true;
		}
	}

	return bResult;
}

/** Updates the datastore with the currently selected character's family ID and shows the part selection screen. */
function OnCharacterSelected()
{
	local int SelectedItem;
	local string CharID;
	local LocalPlayer LP;

	LP = GetPlayerOwner();

	SelectedItem = CharacterList.GetCurrentItem();

	if(class'UTUIMenuList'.static.GetCellFieldString(CharacterList, 'CharacterID', SelectedItem, CharID))
	{
		`Log("Current CharacterID: " $ CharID);
		SetDataStoreStringValue("<UTCustomChar:Character>", CharID);

		// @todo: Maybe there's a better way to do this, but clear saved char data.  So the character customization screen uses the default character.
		SetDataStoreStringValue("<OnlinePlayerData:ProfileData.CustomCharData>", "", none, LP);

		OpenSceneByName(CharacterCustomizationScene);
	}
}

/** Called when the user wants to back out of the scene. */
function OnBack()
{
	// Activate kismet for exiting scene
	ActivateLevelEvent('CharacterSelectionExit');

	CloseScene(self);
}

/** Button bar callbacks - Accept Button */
function bool OnButtonBar_Next(UIScreenObject InButton, int InPlayerIndex)
{
	OnCharacterSelected();

	return true;
}

/** Button bar callbacks - Back Button */
function bool OnButtonBar_Back(UIScreenObject InButton, int InPlayerIndex)
{
	OnBack();

	return true;
}


/**
 * Called when the user submits the current list index.
 *
 * @param	Sender	the list that is submitting the selection
 */
function OnCharacterList_SubmitSelection( UIList Sender, optional int PlayerIndex=0 )
{
	OnCharacterSelected();
}

/**
 * Called when the user presses Enter (or any other action bound to UIKey_SubmitListSelection) while this list has focus.
 *
 * @param	Sender	the list that is submitting the selection
 */
function OnCharacterList_ValueChanged( UIObject Sender, optional int PlayerIndex=0 )
{
	local int SelectedItem;
	local string StringValue;

	SelectedItem = CharacterList.GetCurrentItem();

	// Preview Image
	if(class'UTUIMenuList'.static.GetCellFieldString(CharacterList, 'PreviewImageMarkup', SelectedItem, StringValue))
	{
		PreviewImage.SetDatastoreBinding(StringValue);
	}

	// Description
	if(class'UTUIMenuList'.static.GetCellFieldString(CharacterList, 'Description', SelectedItem, StringValue))
	{
		DescriptionLabel.SetDatastoreBinding(StringValue);
	}
}


defaultproperties
{
	CharacterCustomizationScene="UI_Scenes_FrontEnd.Scenes.CharacterCustomization"
}