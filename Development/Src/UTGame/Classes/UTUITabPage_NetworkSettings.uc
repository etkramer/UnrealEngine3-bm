/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for a user's Network settings.
 */

class UTUITabPage_NetworkSettings extends UTUITabPage_Options
	placeable;

/** Reference to the character creation scene. */
var string	PlayerConfigScene;
var string	NewCharacterScene;

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	Super.PostInitialize();

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Settings.Player>");
}

/** Callback allowing the tabpage to setup the button bar for the current scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	if ( class'WorldInfo'.static.IsMenuLevel() )
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.CustomizePlayerButtonBar>", OnButtonBar_CustomizePlayer);
	}

	ConditionallyAppendKeyboardButton(ButtonBar);
	ConditionallyAppendDefaultsButton(ButtonBar);
}


/** @return bool Returns whether or not this user has saved character data. */
function bool HasSavedCharacterData()
{
	local bool bHaveLoadedCharData;
	local string CharacterDataStr;

	bHaveLoadedCharData = false;


	if(GetDataStoreStringValue("<OnlinePlayerData:ProfileData.CustomCharData>", CharacterDataStr, none, GetPlayerOwner()))
	{
		if(Len(CharacterDataStr) > 0)
		{
			bHaveLoadedCharData = true;
		}
	}

	return bHaveLoadedCharData;
}

/** Shows the player customization scene. */
function OnShowPlayerScene()
{
	local UTUIScene_MessageBox MessageBoxReference;
	local array<string> MessageBoxOptions;
	local array<PotentialOptionKeys> PotentialOptionKeyMappings;
	local UTUIScene UTScene;

	UTScene = UTUIScene(GetScene());

	if(UTScene != None)
	{
		if ( UTScene.GetWorldInfo().IsDemoBuild() )
		{
			UTScene.DisplayMessageBox("Custom characters are not available in the demo version of UT3.");
		}
		else
		{
			// Check to see if we have any saved char data
			if(HasSavedCharacterData())
			{
				// Pop up a message box asking the user if they want to edit their character or randomly generate one.
				MessageBoxReference = UTScene.GetMessageBoxScene();

				if(MessageBoxReference != none)
				{
					MessageBoxOptions.AddItem("<Strings:UTGameUI.CharacterCustomization.CreateNewCharacter>");
					MessageBoxOptions.AddItem("<Strings:UTGameUI.CharacterCustomization.EditExistingCharacter>");
					MessageBoxOptions.AddItem("<Strings:UTGameUI.Generic.Cancel>");

					PotentialOptionKeyMappings.length = 3;
					PotentialOptionKeyMappings[0].Keys.AddItem('XboxTypeS_X');
					PotentialOptionKeyMappings[1].Keys.AddItem('XboxTypeS_A');
					PotentialOptionKeyMappings[2].Keys.AddItem('XboxTypeS_B');
					PotentialOptionKeyMappings[2].Keys.AddItem('Escape');

					MessageBoxReference.SetPotentialOptionKeyMappings(PotentialOptionKeyMappings);
					MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
					MessageBoxReference.Display("<Strings:UTGameUI.CharacterCustomization.CreateNewChar_Message>", "<Strings:UTGameUI.CharacterCustomization.CreateNewChar_Title>", OnStartNewCharacter_Confirm);
				}
			}
			else
			{
				UTScene.OpenSceneByName(NewCharacterScene);
			}
		}
	}
}

/** Messagebox confirmation of whether or not to create a new character or edit an existing one. */
function OnStartNewCharacter_Confirm(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	local UTUIScene UTScene;
	UTScene = UTUIScene(GetScene());

	if(UTScene != None)
	{
		switch(SelectedOption)
		{
		case 0:
			UTScene.OpenSceneByName(NewCharacterScene);
			break;
		case 1:
			UTScene.OpenSceneByName(PlayerConfigScene);
			break;
		default:
			// Do nothing
		}
	}
}


function bool OnButtonBar_CustomizePlayer(UIScreenObject InButton, int PlayerIndex)
{
	OnShowPlayerScene();

	return true;
}

/** Callback for when an option is focused, by default tries to set the description label for this tab page. */
function OnOptionList_OptionFocused(UIScreenObject InObject, UIDataProvider OptionProvider)
{
	Super.OnOptionList_OptionFocused(InObject, OptionProvider);

	UTUIFrontEnd(GetScene()).SetupButtonBar();
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
		if(EventParms.InputKeyName=='XboxTypeS_Y')
		{
			OnShowPlayerScene();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_X')
		{
			OnShowKeyboard();
			bResult=true;
		}
	}

	return bResult;
}

defaultproperties
{
	bAllowResetToDefaults=true
	PlayerConfigScene="UI_Scenes_FrontEnd.Scenes.CharacterCustomization"
	NewCharacterScene="UI_Scenes_ChrisBLayout.Scenes.CharacterFaction"
};
