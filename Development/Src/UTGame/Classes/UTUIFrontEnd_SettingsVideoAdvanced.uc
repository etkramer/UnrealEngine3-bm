/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Advanced video settings for the PC.
 */
class UTUIFrontEnd_SettingsVideoAdvanced extends UTUIFrontEnd
	native(UIFrontEnd);

/** Possible settings enum. */
enum EPossibleVideoSettings
{
	PVS_ScreenPercentage,
	PVS_TextureDetail,
	PVS_WorldDetail,
	PVS_FXDetail,
	PVS_DecalQuality,
	PVS_LightingQuality,
	PVS_ShadowQuality,
	PVS_PostProcessQuality,
	PVS_VSyncValue,
	PVS_SmoothFramerate,
	PVS_PlayerFOV,
};

/** Array of setting types to widget names. */
var transient array<name>	SettingWidgetMapping;

/** Pointer to the options page. */
var transient UTUITabPage_Options	OptionsPage;

/** Reference to the messagebox scene. */
var transient UTUIScene_MessageBox MessageBoxReference;

/**
 * Sets the value of the video setting.
 *
 * @param Setting	Setting to set the value of
 * @param Value		New value for the setting
 */
native function SetVideoSettingValue(EPossibleVideoSettings Setting, int Value);

/**
 * Sets the value of multiple video settings at once.
 *
 * @param Setting	Array of settings to set the value of
 * @param Value		New values for teh settings
 */
native function SetVideoSettingValueArray(array<EPossibleVideoSettings> Settings, array<int> Values);


/**
 * @param	Setting		Setting to get the value of
 * @return				Returns the current value of the specified setting.
 */
native function int GetVideoSettingValue(EPossibleVideoSettings Setting);

/** Post initialize callback. */
event PostInitialize()
{
	local int SettingIdx;
	local int WidgetIdx;
	local int CurrentValue;

	Super.PostInitialize();

	// Find widget references
	OptionsPage = UTUITabPage_Options(FindChild('pnlOptions', true));
	OptionsPage.OnAcceptOptions=OnAcceptOptions;
	OptionsPage.OnOptionChanged=OnOptionChanged;

	// Set all of the default values
	for(SettingIdx=0; SettingIdx<SettingWidgetMapping.length; SettingIdx++)
	{
		CurrentValue = GetVideoSettingValue(EPossibleVideoSettings(SettingIdx));
		SetDataStoreStringValue("<Registry:" $ SettingWidgetMapping[SettingIdx] $ ">", string(CurrentValue));

		// Refresh the widget
		WidgetIdx = OptionsPage.OptionList.GetObjectInfoIndexFromName(SettingWidgetMapping[SettingIdx]);
		UIDataStoreSubscriber(OptionsPage.OptionList.GeneratedObjects[WidgetIdx].OptionObj).RefreshSubscriberValue();
	}
}


/** Callback to setup the buttonbar for this scene. */
function SetupButtonBar()
{
	ButtonBar.Clear();
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Accept>", OnButtonBar_Accept);
}

/** Called when one of our options changes. */
function OnOptionChanged(UIScreenObject InObject, name OptionName, int PlayerIndex)
{
	/*
	local int SettingIdx;
	local int CurrentValue;

	// Find the setting idx for this widget
	SettingIdx = SettingWidgetMapping.Find(OptionName);

	// Set the global settings using the new datastore value.
	if(SettingIdx != INDEX_NONE)
	{
		// Force the widget to update its datastore value
		if(UISlider(InObject)!=None)
		{
			CurrentValue=UISlider(InObject).GetValue();
		}
		else if(UICheckbox(InObject)!=None)
		{
			CurrentValue=UICheckbox(InObject).IsChecked() ? 1 : 0;
		}

		// Save out the option
		SetVideoSettingValue(EPossibleVideoSettings(SettingIdx), CurrentValue);
	}
	*/
}

/** Callback for when the user wants to exit this screen. */
function OnBack()
{
	CloseScene(self);
}

/** Callback for when the user wants to save their options. */
function OnAccept()
{
	local int SettingIdx;
	local int WidgetIdx;
	local int CurrentValue;
	local UIObject InObject;
	local array<EPossibleVideoSettings> SettingArray;
	local array<int> ValueArray;

	// Save all out settings using the datastore value
	for(SettingIdx=0; SettingIdx<SettingWidgetMapping.length; SettingIdx++)
	{
		WidgetIdx = OptionsPage.OptionList.GetObjectInfoIndexFromName(SettingWidgetMapping[SettingIdx]);
		InObject = OptionsPage.OptionList.GeneratedObjects[WidgetIdx].OptionObj;

		// Force the widget to update its datastore value
		if(UISlider(InObject)!=None)
		{
			CurrentValue=UISlider(InObject).GetValue();
		}
		else if(UICheckbox(InObject)!=None)
		{
			CurrentValue=UICheckbox(InObject).IsChecked() ? 1 : 0;
		}

		SettingArray.length=SettingArray.length+1;
		SettingArray[SettingArray.length-1]=EPossibleVideoSettings(SettingIdx);

		ValueArray.length=ValueArray.length+1;
		ValueArray[ValueArray.length-1]=CurrentValue;
	}

	// Save out the options
	SetVideoSettingValueArray(SettingArray, ValueArray);

	MessageBoxReference  = DisplayMessageBox("<Strings:UTGameUI.Errors.SomeChangesMayNotBeApplied_Message>", "<Strings:UTGameUI.Errors.SomeChangesMayNotBeApplied_Title>");
	MessageBoxReference.OnClosed = WarningMessage_Closed;
}

/** Callback for when the warning message has closed. */
function WarningMessage_Closed()
{
	MessageBoxReference.OnClosed = None;
	MessageBoxReference = None;
	CloseScene(self);
}

/** Callback for when the user accepts the options list. */
function OnAcceptOptions(UIScreenObject InScreenObject, int InPlayerIndex)
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

	// Let the binding list get first chance at the input because the user may be binding a key.
	bResult=OptionsPage.HandleInputKey(EventParms);

	if(bResult == false)
	{
		if(EventParms.EventType==IE_Released)
		{
			if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
			{
				OnBack();
				bResult=true;
			}
		}
	}

	return bResult;
}

event SetPlayerFOV(int NewFOV)
{
	local UTPlayerController PC;
	PC = GetUTPlayerOwner();
	if ( PC != none )
	{
		PC.FOV(NewFOV);
	}
}

DefaultProperties
{
	SettingWidgetMapping(PVS_ScreenPercentage)="ScreenPercentage";
	SettingWidgetMapping(PVS_TextureDetail)="TextureDetail";
	SettingWidgetMapping(PVS_WorldDetail)="WorldDetail";
	SettingWidgetMapping(PVS_FXDetail)="FXDetail";
	SettingWidgetMapping(PVS_DecalQuality)="DecalQuality";
	SettingWidgetMapping(PVS_LightingQuality)="LightingQuality";
	SettingWidgetMapping(PVS_ShadowQuality)="ShadowQuality";
	SettingWidgetMapping(PVS_PostProcessQuality)="PostProcessQuality";
	SettingWidgetMapping(PVS_VSyncValue)="VSyncValue";
	SettingWidgetMapping(PVS_SmoothFramerate)="SmoothFramerate";
	SettingWidgetMapping(PVS_PlayerFOV)="PlayerFOV";
}
