/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Key binding screen for the PC.
 */
class UTUIFrontEnd_BindKeysPC extends UTUIFrontEnd;

/** Binding list object, we need to route key presses to this first. */
var transient UTUIKeyBindingList BindingList;

/** Old bindings before making any changes. */
var transient array<PlayerInput.KeyBind> OldBindings;

/** Whether to display the crucialbind warning when closing the lose saved data screen. */
var transient bool bShowCrucialBindWarning;

/** Delegate to mark the profile as dirty. */
delegate MarkDirty();

/** Post initialize callback. */
event PostInitialize()
{
	Super.PostInitialize();

	// Find widget references
	BindingList = UTUIKeyBindingList(FindChild('lstKeys', true));
	BindingList.OnAcceptOptions=OnBindingList_AcceptOptions;

	// Store old bindings
	OldBindings = BindingList.GetPlayerInput().Bindings;
}

/** Callback to setup the buttonbar for this scene. */
function SetupButtonBar()
{
	ButtonBar.Clear();
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Accept>", OnButtonBar_Accept);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ResetToDefaults>", OnButtonBar_ResetToDefaults);
}

/** Will first check to see if there are any unbound crucial binds and close the scene if not. */
function bool CheckForCrucialBindsAndClose()
{
	local int CrucialBindIdx;
	local UTUIScene_MessageBox MessageBoxReference;
	local array<string> MessageBoxOptions;
	local string CrucialBindTitle;

	CrucialBindIdx = BindingList.GetFirstUnboundCrucialBind();
	if (CrucialBindIdx == -1)
	{
		CloseScene(self);
		return true;
	}
	else
	{
		MessageBoxReference = GetMessageBoxScene(BindingList.NonIntrusiveMessageBoxScene);

		if(MessageBoxReference != none)
		{
			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Back>");
			MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
			CrucialBindTitle = BindingList.LocalizedFriendlyNames[CrucialBindIdx];
			MessageBoxReference.Display("<Strings:UTGameUI.MessageBox.CrucialBindErrorMsg>", CrucialBindTitle);
		}

		MessageBoxReference = None;
	}

	return false;
}

/** Confirmation for the exit game dialog. */
function OnMenu_LoseChanges_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	local PlayerInput PInput;

	if(SelectedItem==0)
	{
		// restore old bindings
		PInput = BindingList.GetPlayerInput();
		PInput.Bindings = OldBindings;
		PInput.SaveConfig();
		BindingList.RefreshBindingLabels();
		CheckForCrucialBindsAndClose();
		bShowCrucialBindWarning = true;
	}
	else
	{
		bShowCrucialBindWarning = false;
	}
}

/** Called when the back dialog is closed. */
function OnBackSceneDeactivated( UIScene DeactivatedScene )
{
	if (bShowCrucialBindWarning)
	{
		CheckForCrucialBindsAndClose();
	}
}

/** Callback for when the user wants to exit this screen. */
function OnBack()
{
	local UTUIScene_MessageBox MessageBoxReference;
	local array<string> MessageBoxOptions;

	if (BindingList.BindingsHaveChanged())
	{
		MessageBoxReference = GetMessageBoxScene();

		if(MessageBoxReference != none)
		{
			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Accept>");
			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Cancel>");

			MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
			MessageBoxReference.Display("<Strings:UTGameUI.MessageBox.LoseChangesWarning_Message>", "<Strings:UTGameUI.MessageBox.LoseChangesWarning_Title>", OnMenu_LoseChanges_Confirm, 1);
		}

		MessageBoxReference = None;
	}
	else
	{
		CheckForCrucialBindsAndClose();
	}
}

/** Callback for when the user wants to save their options. */
function OnAccept()
{
	local PlayerInput PInput;
	local UTProfileSettings PlayerProfile;
	local UTPlayerController UTPC;
	local int PlayerIndex;

	PlayerIndex = GetPlayerIndex();
	PlayerProfile = GetPlayerProfile(PlayerIndex);

	PInput = BindingList.GetPlayerInput();
	PlayerProfile.StoreKeysUsingPlayerInput(PInput);

	// Save keys to INI
	PInput.SaveConfig();

	// Clear the bind key cache in the controller
	UTPC = GetUTPlayerOwner(PlayerIndex);
	if (UTPC != none)
	{
		UTPC.ClearStringAliasBindingMapCache();
	}

	// Mark the screen for saving if successfully closing and a change was made.
	if ( CheckForCrucialBindsAndClose() && BindingList.BindingsHaveChanged() )
	{
		MarkDirty();
	}
}

/** Callback for when the user wants to reset to the default set of option values. */
function OnResetToDefaults()
{
	local UTUIScene_MessageBox MessageBoxReference;
	local array<string> MessageBoxOptions;

	MessageBoxReference = GetMessageBoxScene();

	if(MessageBoxReference != none)
	{
		MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.ResetToDefaultAccept>");
		MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Cancel>");

		MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
		MessageBoxReference.Display("<Strings:UTGameUI.MessageBox.ResetToDefaults_Message>", "<Strings:UTGameUI.MessageBox.ResetToDefaults_Title>", OnMenu_ResetToDefaults_Confirm, 1);
	}

	MessageBoxReference = None;
}

/** Confirmation for the exit game dialog. */
function OnMenu_ResetToDefaults_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	if(SelectedItem==0)
	{
		BindingList.ReloadDefaults();
	}
}

/** Accept button was pressed on the option list. */
function OnBindingList_AcceptOptions(UIScreenObject InObject, int PlayerIndex)
{
	OnAccept();
}

/** Button bar callbacks. */
function bool OnButtonBar_Accept(UIScreenObject InButton, int PlayerIndex)
{
	OnAccept();

	return true;
}

function bool OnButtonBar_Back(UIScreenObject InButton, int PlayerIndex)
{
	OnBack();

	return true;
}

function bool OnButtonBar_ResetToDefaults(UIScreenObject InButton, int PlayerIndex)
{
	OnResetToDefaults();

	return true;
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

	if(EventParms.EventType==IE_Released && EventParms.InputKeyName=='XboxTypeS_A')
	{
		OnAccept();
		bResult=true;
	}

	if(bResult==false)
	{
		// Let the binding list get first chance at the input because the user may be binding a key.
		bResult=BindingList.HandleInputKey(EventParms);
	}

	if(bResult == false)
	{
		if(EventParms.EventType==IE_Released)
		{
			if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
			{
				OnBack();
				bResult=true;
			}
			else if(EventParms.InputKeyName=='XboxTypeS_LeftTrigger')
			{
				OnResetToDefaults();
				bResult=true;
			}
		}
	}

	return bResult;
}
