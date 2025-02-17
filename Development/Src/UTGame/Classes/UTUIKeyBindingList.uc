/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Keybinding List, exposes a set of UTUIDataProvider_KeyBinding objects to the user so they can bind keys.
 */

class UTUIKeyBindingList extends UTUIOptionList
	placeable
	native(UIFrontEnd);

/** Number of buttons to display for each key binding. */
var transient int NumButtons;

/** Whether or not we should try to bind the next key. */
var transient bool bCurrentlyBindingKey;

/** Which button we are currently rebinding. */
var transient UIObject CurrentlyBindingObject;

/** Reference to the message box scene. */
var transient UTUIScene_MessageBox MessageBoxReference;

/** Global scene reference for non intrusive message box scene */
var transient UIScene NonIntrusiveMessageBoxScene;

/** Current bindings for each of the buttons, the length of this array should match the generated objects array. */
var transient array<string>	CurrentBindings;
/**
 * Stored bindings for each of the buttons, the length of this array should match the generated objects array.
 * This array stores the starting values for CurrentBindings so we know if any changes were made.
 */
var transient array<string>	StoredBindings;

/** List of friendly label names for the actions being bound to. */
var transient array<string> LocalizedFriendlyNames;

/**
 * List of bools to keep track of crucial binds that must be bound before exiting screen,
 * the length of this array should match the generated objects array.
 */
var transient array<bool> CrucialBindValues;

/** Structure to store all pertinent data when in the process of binding a key. */
struct native BindKeyData
{
	/** New key to bind to. */
	var name KeyName;
	/** Command to bind to the KeyName */
	var string Command;
	/** Player input object to bind in. */
	var PlayerInput PInput;
	/** The previous key bound to. */
	var name PreviousBinding;
	/** Whether to prompt about duplicate binds or not. */
	var bool bPromptForDuplicate;
	/** Whether the bind is the primary or secondary bind. */
	var bool bBindIsPrimary;
};

/** Key bind data to be passed from function to function and into message logic when attempting a key bind. */
var transient BindKeyData CurrKeyBindData;

/** Returns the player input object for the player that owns this widget. */
event PlayerInput GetPlayerInput()
{
	local LocalPlayer LP;
	local PlayerInput Result;

	Result = none;
	LP = GetPlayerOwner();

	if(LP != none && LP.Actor != none)
	{
		Result = LP.Actor.PlayerInput;
	}

	return Result;
}

/** Reloads default values from the default INI (PC Only). */
function ReloadDefaults()
{
	class'UTProfileSettings'.static.ResetKeysToDefault(GetPlayerOwner());
	RefreshBindingLabels();
}

/** Generates widgets for all of the options. */
native function RegenerateOptions();

/** Repositions all of the visible options. */
native function RepositionOptions();

/** Refreshes the binding labels for all of the buttons. */
native function RefreshBindingLabels();

/** Get the binding key using the command as the key and starting from a specific place in the list. */
native function String GetBindKeyFromCommand(PlayerInput PInput, String Command, out int StartIdx);

/** Post initialize, binds callbacks for all of the generated options. */
event PostInitialize()
{
	local int ObjectIdx, Idx;

	Super.PostInitialize();

	StoredBindings.length = 0;
	for (Idx = 0; Idx < CurrentBindings.length; Idx++)
	{
		StoredBindings.length = Idx + 1;
		StoredBindings[Idx] = CurrentBindings[Idx];
	}

	// Go through all of the generated object and set the OnValueChanged delegate.
	for(ObjectIdx=0; ObjectIdx < GeneratedObjects.length; ObjectIdx++)
	{
		GeneratedObjects[ObjectIdx].OptionObj.OnClicked = OnClicked;
		GeneratedObjects[ObjectIdx].OptionObj.OnRawInputKey=OnButton_InputKey;
	}
}

/** Callback for the mutator lists, captures the accept button before the mutators get to it. */
function bool OnButton_InputKey( const out InputEventParameters EventParms )
{
	local bool bResult;

	bResult = false;

	if(EventParms.EventType==IE_Released && EventParms.InputKeyName=='XboxTypeS_A')
	{
		OnAcceptOptions(self, EventParms.PlayerIndex);
		bResult = true;
	}

	return bResult;
}

/**
 * Handler for the vertical scrollbar's OnClickedScrollZone delegate.  Scrolls the list by a full page (MaxVisibleItems).
 *
 * @param	Sender			the scrollbar that was clicked.
 * @param	PositionPerc	a value from 0.0 - 1.0, representing the location of the click within the region between the increment
 *							and decrement buttons.  Values closer to 0.0 means that the user clicked near the decrement button; values closer
 *							to 1.0 are nearer the increment button.
 * @param	PlayerIndex		Player that performed the action that issued the event.
 */
function ClickedScrollZone( UIScrollbar Sender, float PositionPerc, int PlayerIndex )
{
	local int MouseX, MouseY;
	local float MarkerPosition;
	local bool bDecrement;

	if ( GetCursorPosition(MouseX, MouseY) )
	{
		// this is the position of the marker's minor side (left or top)
		MarkerPosition = Sender.GetMarkerButtonPosition();

		// determine whether the user clicked in the region above or below the marker button.
		bDecrement = (Sender.ScrollbarOrientation == UIORIENT_Vertical)
			? MouseY < MarkerPosition
			: MouseX < MarkerPosition;

		if(bDecrement)
		{
			SelectPreviousItem();
		}
		else
		{
			SelectNextItem();
		}
	}
}

/**
 * Handler for vertical scrolling activity
 * PositionChange should be a number of nudge values by which the slider was moved
 *
 * @param	Sender			the scrollbar that generated the event.
 * @param	PositionChange	indicates how many items to scroll the list by
 * @param	bPositionMaxed	indicates that the scrollbar's marker has reached its farthest available position,
 *                          unused in this function
 */
function bool ScrollVertical( UIScrollbar Sender, float PositionChange, optional bool bPositionMaxed=false )
{
	SelectItem(CurrentIndex+PositionChange*2);

	return true;
}

/** Selects the next item in the list. */
function bool SelectNextItem(optional bool bWrap=false, optional int PlayerIndex=GetBestPlayerIndex())
{
	local int TargetIndex;

	TargetIndex = CurrentIndex+2;	// This list is special since it has 2 items per row

	if(bWrap)
	{
		TargetIndex = TargetIndex%(GeneratedObjects.length);
	}

	return SelectItem(TargetIndex, PlayerIndex);
}

/** Selects the previous item in the list. */
function bool SelectPreviousItem(optional bool bWrap=false, optional int PlayerIndex=GetBestPlayerIndex())
{
	local int TargetIndex;

	TargetIndex = CurrentIndex-2;	// This list is special since it has 2 items per row

	if(bWrap && TargetIndex<0)
	{
		TargetIndex=GeneratedObjects.length-1;
	}

	return SelectItem(TargetIndex, PlayerIndex);
}

/** Returns the index in the CrucialBindValues array of the first crucial bind not bound. Returns -1 if all is good. */
function int GetFirstUnboundCrucialBind()
{
	local int Idx;

	for (Idx = 0; Idx < CrucialBindValues.length; Idx++)
	{
		if (CrucialBindValues[Idx])
		{
			if (CurrentBindings[Idx*2] == "")
			{
				return Idx;
			}
		}
	}

	return -1;
}

/** Whether any changes have been made to the bindings. */
function bool BindingsHaveChanged()
{
	local int Idx;

	for (Idx = 0; Idx < CurrentBindings.length; Idx++)
	{
		if (Name(StoredBindings[Idx]) != Name(CurrentBindings[Idx]))
		{
			return true;
		}
	}

	return false;
}

/** Whether or not a key is already bound to a command. */
function bool IsAlreadyBound(Name KeyName)
{
	local int Idx;

	if (KeyName != '')
	{
		for (Idx = 0; Idx < CurrentBindings.length; Idx++)
		{
			if (KeyName == Name(CurrentBindings[Idx]))
			{
				return true;
			}
		}
	}

	return false;
}

/** Confirmation for rebinding of an already bound key message bos. */
function OnMenu_BindOverwrite_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	if(SelectedItem == 0)
	{
		BindKey();
	}
	else
	{
		CancelKeyBind();
	}
}

/** Callback for when the warning for stomping binds dialog has finished closing. */
function OnBindStompWarning_Closed()
{
	MessageBoxReference.OnClosed = None;
	MessageBoxReference = None;
}

/** Spawns the message dialog so we can warn the player about stomping a key bind. */
function SpawnBindStompWarningMessage()
{
	local array<string> MessageBoxOptions;

	if (MessageBoxReference == None)
	{
		MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Accept>");
		MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Cancel>");

		MessageBoxReference = UTUIScene(GetScene()).GetMessageBoxScene(NonIntrusiveMessageBoxScene);
		MessageBoxReference.FadeDuration = 0.125f;
		MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
		MessageBoxReference.OnSceneDeactivated = OnStompBindDialogSceneDeactivated;
		MessageBoxReference.Display("<Strings:UTGameUI.MessageBox.StompBindKey_Message>", "<Strings:UTGameUI.MessageBox.StompBindKey_Title>", OnMenu_BindOverwrite_Confirm, 1);
	}
	else
	{
		`log("UTUIKeyBindingList::SpawnBindStompWarningMessage: Unable to prompt for bind stomp message!");
	}
}

/** Called when the bind dialog is closed. */
function OnStompBindDialogSceneDeactivated( UIScene DeactivatedScene )
{
	OnBindStompWarning_Closed();
}

/** Attempts to bind the specified key to an action. Will prompt user of bind stomping. */
function AttemptKeyBind()
{
	// Same bind, so just finish.
	if ( CurrKeyBindData.KeyName == CurrKeyBindData.PreviousBinding )
	{
		FinishKeyDialog(false);
	}
	// Already bound so ask if they want to stomp the binding or not.
	else if ( IsAlreadyBound(CurrKeyBindData.KeyName) )
	{
		// By passing false we are saying that the bind process is not over...
		// ...the dialog close delegate will open another message box asking about stomping a binding.
		FinishKeyDialog(true);
	}
	// Not bound, so just bind and finish.
	else
	{
		BindKey();
		FinishKeyDialog(false);
	}
}

/** Binds the specified key to an action. */
function BindKey()
{
	local int StartIdx;
	local KeyBind NewKeyBind;

	`Log("UTUIKeyBindingList::BindKey() - Binding key '" $ CurrKeyBindData.KeyName $ "' to command '" $ CurrKeyBindData.Command $ "'");

	// Unbind what used to be bound to this command.
	UnbindKey(CurrKeyBindData.PreviousBinding);
	// Unbind the new key to whatever it may have been bound to.
 	UnbindKey(CurrKeyBindData.KeyName);

	// Build the keybind.
	NewKeyBind.Name = CurrKeyBindData.KeyName;
	NewKeyBind.Command = CurrKeyBindData.Command;

	// Is the primary key bind.
	if (CurrKeyBindData.bBindIsPrimary)
	{
		// Is primary key bind so we can just append it to the list.
		CurrKeyBindData.PInput.Bindings[CurrKeyBindData.PInput.Bindings.length] = NewKeyBind;
	}
	// Is the secondary key bind.
	else
	{
		// Is the secondary key bind so attempt to find a primaray bind.
		StartIdx = -1;
		GetBindKeyFromCommand(CurrKeyBindData.PInput, CurrKeyBindData.Command, StartIdx);

		// Found the primary bind so place this bind in front of it in the list.
		if (StartIdx > -1)
		{
			CurrKeyBindData.PInput.Bindings.InsertItem(StartIdx, NewKeyBind);
		}
		// There is no primary bind so this bind will be the primary and place itself at the end of the list.
		else
		{
			CurrKeyBindData.PInput.Bindings[CurrKeyBindData.PInput.Bindings.length] = NewKeyBind;
		}
	}

	CurrKeyBindData.PInput.SaveConfig();

	FinishBinding();
}

/** Cancels out of an attempted bind. */
function CancelKeyBind()
{
	FinishBinding();
}

/** Unbinds the specified key. */
function UnbindKey(name BindName)
{
	local PlayerInput PInput;
	local int BindingIdx;

	PInput = GetPlayerInput();

	for(BindingIdx = 0;BindingIdx < PInput.Bindings.Length;BindingIdx++)
	{
		if(PInput.Bindings[BindingIdx].Name == BindName)
		{
			PInput.Bindings.Remove(BindingIdx, 1);
			break;
		}
	}
}

/** Callback for all of the options we generated. */
function bool OnClicked( UIScreenObject Sender, int PlayerIndex )
{
	local UILabelButton BindingButton;
	local string FinalMsg;
	local int ObjectIdx;
	local UIObject MessageBoxChild;

	// Cancel the object we were previously binding.
	if(CurrentlyBindingObject==None && MessageBoxReference==None)
	{
		// Rebind the key
		bCurrentlyBindingKey = true;
		CurrentlyBindingObject = UIObject(Sender);

		// Unbind the key that was clicked on.
		BindingButton = UILabelButton(CurrentlyBindingObject);

		if(BindingButton != none)
		{
			// Display the bind key dialog.
			ObjectIdx = GetObjectInfoIndexFromObject(CurrentlyBindingObject);

			if(ObjectIdx != INDEX_NONE)
			{
				CurrKeyBindData.PreviousBinding = name(CurrentBindings[ObjectIdx]);
				BindingButton.SetDataStoreBinding("<Strings:UTGameUI.Settings.PressKeyToBind>");

				FinalMsg = Localize("MessageBox", "BindKey_Message", "UTGameUI");
				FinalMsg = Repl(FinalMsg, "\`Binding\`", UILabel(GeneratedObjects[ObjectIdx].LabelObj).GetDataStoreBinding());

				MessageBoxReference = UTUIScene(GetScene()).GetMessageBoxScene(NonIntrusiveMessageBoxScene);

				MessageBoxChild = MessageBoxReference.FindChild('pnlScrollFrame', true);
				MessageBoxChild.KillFocus(None, PlayerIndex);
				MessageBoxChild.SetPrivateBehavior(PRIVATE_NotFocusable, true);

				MessageBoxChild = MessageBoxReference.FindChild('pnlSafeRegionLong', true);
				MessageBoxChild.KillFocus(None, PlayerIndex);
				MessageBoxChild.SetPrivateBehavior(PRIVATE_NotFocusable, true);

				MessageBoxReference.SetFocus(None, PlayerIndex);
				MessageBoxReference.OnMBInputKey = OnBindKey_InputKey;
				MessageBoxReference.FadeDuration = 0.125f;
				MessageBoxReference.DisplayModalBox(FinalMsg,"<Strings:UTGameUI.MessageBox.BindKey_Title>",0.0f);
			}
		}
	}

	return true;
}

/** Closes the key dialog */
// By passing false we are saying that the bind process is not over...
// ...the dialog close delegate will open another message box asking about stomping a binding.
function FinishKeyDialog(bool bPromptForBindStomp)
{
	if (bPromptForBindStomp)
	{
		CurrKeyBindData.bPromptForDuplicate = true;
	}
	else
	{
		FinishBinding();
	}

	// Close the modal dialog.
	MessageBoxReference.OnSceneDeactivated = OnBindDialogSceneDeactivated;
	MessageBoxReference.Close();
}

/** Finishes the binding process by clearing variables and closing the bind key dialog. */
function FinishBinding()
{
	bCurrentlyBindingKey = FALSE;
	CurrentlyBindingObject = None;

	// Clear the key bind data object.
	CurrKeyBindData.Command = "";
	CurrKeyBindData.KeyName = '';
	CurrKeyBindData.PreviousBinding = '';
	CurrKeyBindData.PInput = None;
	CurrKeyBindData.bPromptForDuplicate = false;

	RefreshBindingLabels();
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
function bool OnBindKey_InputKey( const out InputEventParameters EventParms )
{
	return HandleInputKey(EventParms);
}

/** Called when the bind dialog is closed. */
function OnBindDialogSceneDeactivated( UIScene DeactivatedScene )
{
	MessageBoxReference.OnClosed = None;
	MessageBoxReference.OnMBInputKey = None;
	MessageBoxReference=None;

	if (CurrKeyBindData.bPromptForDuplicate)
	{
		SpawnBindStompWarningMessage();
	}
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
	local PlayerInput PInput;
	local int ObjectIdx;
	local UTUIScene UTScene;
	local UTUIDataProvider_KeyBinding KeyBindingProvider;

	bResult=false;
	UTScene = UTUIScene(GetScene());

	// If we are currently binding a key, then
	if(bCurrentlyBindingKey)
	{
		if(EventParms.bShiftPressed && EventParms.InputKeyName=='Escape' && EventParms.EventType==IE_Released)
		{
			FinishKeyDialog(false);
		}
		else if(EventParms.EventType==IE_Released)
		{

			// If we're on PS3, ignore all gamepad input.
			if(!IsConsole(CONSOLE_PS3) || UTScene.IsControllerInput(EventParms.InputKeyName)==false)
			{
				// Reoslve the option name
				ObjectIdx = GetObjectInfoIndexFromObject(CurrentlyBindingObject);

				if(ObjectIdx != INDEX_NONE)
				{
					KeyBindingProvider = UTUIDataProvider_KeyBinding(GeneratedObjects[ObjectIdx].OptionProvider);

					if(KeyBindingProvider != none)
					{
						PInput = GetPlayerInput();

						if(PInput != none)
						{
							// Fill the key bind data structure and attempt to bind the key.
							CurrKeyBindData.KeyName = EventParms.InputKeyName;
							CurrKeyBindData.Command = KeyBindingProvider.Command;
							CurrKeyBindData.PInput = PInput;
							CurrKeyBindData.bBindIsPrimary = (ObjectIdx%2 == 0) ? true : false;
							AttemptKeyBind();
						}
					}
				}
			}
		}

		bResult = TRUE;
	}

	return bResult;
}

defaultproperties
{
	NumButtons=2
	NonIntrusiveMessageBoxScene=UIScene'UI_Scenes_Common.KeyBindMessageBox'
}

