/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Character Faction selection screen for UT3
 */

class UTUIFrontEnd_CharacterFaction extends UTUIFrontEnd;

/** Preview image. */
var transient UIImage	PreviewImage;

/** Description label. */
var transient UILabel	DescriptionLabel;

/** List widget. */
var transient UTUIList FactionList;

/** Reference to the input box used to unlock characters. */
var transient UTUIScene_InputBox	InputBoxReference;

/** Character selection scene to display once the user chooses a faction. */
var string CharacterSelectionScene;

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize( )
{
	Super.PostInitialize();

	// Setup delegates
	FactionList = UTUIList(FindChild('lstFactions', true));
	if(FactionList != none)
	{
		FactionList.OnSubmitSelection = OnFactionList_SubmitSelection;
		FactionList.OnValueChanged = OnFactionList_ValueChanged;
	}

	// Store widget references
	PreviewImage = UIImage(FindChild('imgPreview1', true));
	DescriptionLabel = UILabel(FindChild('lblDescription', true));

	// Activate kismet for character selection.
	ActivateLevelEvent('CharacterFactionEnter');
}

/** Setup the scene's button bar. */
function SetupButtonBar()
{
	ButtonBar.Clear();
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Next>", OnButtonBar_Next);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.EnterUnlockCode>", OnButtonBar_EnterUnlockCode);
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
		else if(EventParms.InputKeyName=='XboxTypeS_Y')
		{
			OnEnterUnlockCode();
			bResult=true;
		}
	}

	return bResult;
}

/** Finds the faction that was selected and sets the datastore value for it. */
function OnFactionSelected()
{
	local int SelectedItem;
	local string Faction;

	SelectedItem = FactionList.GetCurrentItem();

	if(class'UTUIMenuList'.static.GetCellFieldString(FactionList, 'FactionID', SelectedItem, Faction))
	{
		`Log("Current Faction: " $ Faction);
		SetDataStoreStringValue("<UTCustomChar:Faction>", Faction);
		OpenSceneByName(CharacterSelectionScene);
	}
}

/** Called when the user wants to back out of the scene. */
function OnBack()
{
	// Activate kismet to let the level know we are exiting the character faction scene.
	ActivateLevelEvent('CharacterFactionExit');

	// Close the scene
	CloseScene(self);
}


/** Called when the user wants to enter a character unlock code. */
function OnEnterUnlockCode()
{
	InputBoxReference = GetInputBoxScene();
	InputBoxReference.InputEditbox.MaxCharacters=12;
	InputBoxReference.DisplayAcceptCancelBox(Localize("MessageBox", "EnterUnlockCode_Message", "UTGameUI"),
			Localize("MessageBox", "EnterUnlockCode_Title", "UTGameUI"), OnEnterCodeResult);
}

/**
 * Called when the user has made a selection of the choices available to them.
 */
function OnEnterCodeResult(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	local string Code;

	if(SelectedOption==0)
	{
		Code = InputBoxReference.GetValue();
		if(TryCharacterUnlock(Code)==false)	// If the unlock code fails, display a message when the input box closes.
		{
			DisplayMessageBox("<Strings:UTGameUI.Errors.InvalidUnlockCode_Message>","<Strings:UTGameUI.Errors.InvalidUnlockCode_Title>");
		}
		else
		{
			FactionList.RefreshSubscriberValue();
		}
	}

	InputBoxReference = None;
}


/**
 * Called when the user changes the current list index.
 *
 * @param	Sender	the list that is submitting the selection
 */
function OnFactionList_SubmitSelection( UIList Sender, optional int PlayerIndex=0 )
{
	OnFactionSelected();
}

/**
 * Called when the user presses Enter (or any other action bound to UIKey_SubmitListSelection) while this list has focus.
 *
 * @param	Sender	the list that is submitting the selection
 */
function OnFactionList_ValueChanged( UIObject Sender, optional int PlayerIndex=0 )
{
	local int SelectedItem;
	local string StringValue;

	// Get the map's name from the list.
	SelectedItem = FactionList.GetCurrentItem();

	// Preview Image
	if(class'UTUIMenuList'.static.GetCellFieldString(FactionList, 'PreviewImageMarkup', SelectedItem, StringValue))
	{
		PreviewImage.SetDatastoreBinding(StringValue);
	}

	// Description
	if(class'UTUIMenuList'.static.GetCellFieldString(FactionList, 'Description', SelectedItem, StringValue))
	{
		DescriptionLabel.SetDatastoreBinding(StringValue);
	}
}

/** Button bar callbacks - Accept Button */
function bool OnButtonBar_Next(UIScreenObject InButton, int InPlayerIndex)
{
	OnFactionSelected();

	return true;
}

/** Button bar callbacks - Back Button */
function bool OnButtonBar_Back(UIScreenObject InButton, int InPlayerIndex)
{
	OnBack();

	return true;
}

/** Button bar callbacks - Enter unlock code Button */
function bool OnButtonBar_EnterUnlockCode(UIScreenObject InButton, int InPlayerIndex)
{
	OnEnterUnlockCode();

	return true;
}



defaultproperties
{
	CharacterSelectionScene="UI_Scenes_ChrisBLayout.Scenes.CharacterSelection"
}
