/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Key binding screen for the 360.
 */
class UTUIFrontEnd_BindKeys360 extends UTUIFrontEnd;

/** Preset button configurations */
enum ButtonPresets
{
	BP_Normal,
	BP_Legacy
};

/** Preset stick configurations */
enum StickPresets
{
	SP_Normal,
	SP_Legacy,
	SP_SouthPaw,
	SP_LegacySouthPaw
};

/** References to all of the controller binding widgets in the scene. */
var transient array<UTUIOptionButton>	ControllerBindings;

/** Widget names for the controller binding widgets. */
var transient array<name>	ControllerBindingNames;

/** Delegate to mark the profile as dirty. */
delegate MarkDirty();

/** Post initialize callback. */
event PostInitialize()
{
	local int BindingIdx;
	local UTUIOptionButton BindingWidget;
	Super.PostInitialize();

	// Find widget references
	for(BindingIdx=0; BindingIdx<ControllerBindingNames.length; BindingIdx++)
	{
		BindingWidget = UTUIOptionbutton(FindChild(ControllerBindingNames[BindingIdx],true));

		if(BindingWidget != none)
		{
			ControllerBindings.AddItem(BindingWidget);
		}
	}
}

/** Callback to setup the buttonbar for this scene. */
function SetupButtonBar()
{
	ButtonBar.Clear();
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Accept>", OnButtonBar_Accept);
}

/** Updates the labels for the buttons on the 360 controller diagram. */
function UpdateButtonLabels()
{
	
}

/** Callback for when the user wants to exit this screen. */
function OnBack()
{
	CloseScene(self);
}

/** Callback for when the user wants to save their options. */
function OnAccept()
{
	local int PlayerIndex;
	local UTPlayerController UTPC;

	PlayerIndex = GetPlayerIndex();

	// Force widgets to save their new values back to the profile.
	SaveSceneDataValues(FALSE);

	// Clear the bind key cache in the controller
	UTPC = GetUTPlayerOwner(PlayerIndex);
	if (UTPC != none)
	{
		UTPC.ClearStringAliasBindingMapCache();
	}

	MarkDirty();

	CloseScene(self);
}

/** @return Finds which keybinding widget currently has focus and returns its index and INDEX_NONE if none have focus. */
function int FindFocusedBinding()
{
	local int BindingIdx;
	local int Result;

	Result = INDEX_NONE;

	for(BindingIdx=0; BindingIdx<ControllerBindings.length; BindingIdx++)
	{
		if(ControllerBindings[BindingIdx].IsFocused())
		{
			Result = BindingIdx;
			break;
		}
	}

	return Result;
}

/** Sets focus to the next key binding widget. */
function OnFocusNextBinding()
{
	local int CurrentBinding;

	CurrentBinding = FindFocusedBinding();
	CurrentBinding=(CurrentBinding+1)%ControllerBindings.length;

	`Log("focus next"@CurrentBinding);
	ControllerBindings[CurrentBinding].SetFocus(none,GetPlayerIndex());
}

/** Sets focus to the previous key binding widget. */
function OnFocusPreviousBinding()
{
	local int CurrentBinding;

	CurrentBinding = FindFocusedBinding();

	if(CurrentBinding==INDEX_NONE)
	{
		CurrentBinding = 0;
	}
	else
	{
		if(CurrentBinding==0)
		{
			CurrentBinding=ControllerBindings.length-1;
		}
		else
		{
			CurrentBinding--;
		}
	}

	`Log("focus prev"@CurrentBinding);
	ControllerBindings[CurrentBinding].SetFocus(none,GetPlayerIndex());
}

/** Callback for when the user wants to reset to the default set of option values. */
function OnResetToDefaults()
{
	
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

	bResult = false;


	if(EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
		{
			OnBack();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_A')
		{
			OnAccept();
			bResult=true;
		}
	}

	if(EventParms.EventType==IE_Released || EventParms.EventType==IE_Repeat)
	{
		if(EventParms.InputKeyName=='XboxTypeS_Dpad_Up' || EventParms.InputKeyName=='Gamepad_LeftStick_Up' || EventParms.InputKeyName=='Gamepad_RightStick_Up')	
		{
			OnFocusPreviousBinding();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_Dpad_Down' || EventParms.InputKeyName=='Gamepad_LeftStick_Down' || EventParms.InputKeyName=='Gamepad_RightStick_Down')	
		{
			OnFocusNextBinding();
			bResult=true;
		}
	}

	return bResult;
}

defaultproperties
{
	// The order of this array controls the focus order of the controls.
	ControllerBindingNames=("butLeftTrigger", "butRightTrigger", "butLeftBumper", "butRightBumper", "butY", "butX", "butB", "butA", "butLeftStick", "butRightStick", "butDPadUp",  "butDPadLeft", "butDPadRight", "butDPadDown", "butBack", "butStart",  "butLeftStickDown", "butRightStickDown"  )
}