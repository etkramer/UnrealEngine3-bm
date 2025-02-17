/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for a user's Video settings.
 */

class UTUITabPage_VideoSettings extends UTUITabPage_Options
	placeable;


/** Reference to the advanced video settings scene. */
var string	AdvancedSettingsScene;

/** Struct defining a screen resoluton. */
struct ScreenResSetting
{
	var int ResX;
	var int ResY;
	var bool bFullscreen;
};

/** Struct for the old resolution for the scene. */
var transient ScreenResSetting OldResolution;

/** Struct for the new resolution we are switching to. */
var transient ScreenResSetting NewResolution;

/** When we started the resolution change. */
var transient float	StartResChangeTime;

/** Reference to the resolution changed message box. */
var transient UTUIScene_MessageBox MessageBoxReference;

/** Amount of time the user has to accept their new resolution settings. */
var transient float SettingsTimeoutTime;

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	local array<string>	PossibleOptions;
	local int FieldIdx;

	Super.PostInitialize();

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Settings.Video>");

	// Pull possible resolutionoptions
	if(!IsConsole())
	{
		// Screen Resolutions
		UTUIScene(GetScene()).GetPossibleScreenResolutions(PossibleOptions);
		StringListDataStore.Empty('ScreenResolution',PossibleOptions.Length > 0);

		for(FieldIdx=0; FieldIdx<PossibleOptions.length; FieldIdx++)
		{
			StringListDataStore.AddStr('ScreenResolution', PossibleOptions[FieldIdx], FieldIdx < Max(1, PossibleOptions.length - 1));
		}
	}

	SetupDefaults();

	OnTick=OnTickCallback;
}

/** Callback for the tick event. */
function OnTickCallback(float DeltaTime)
{
	local float TimeElapsed;
	local string FinalMsg;

	if(MessageBoxReference != None)
	{
		TimeElapsed = (UTUIScene(GetScene()).GetWorldInfo().RealTimeSeconds-StartResChangeTime);
		if(TimeElapsed > SettingsTimeoutTime)
		{
			MessageBoxReference.OptionSelected(1,0);
			UTUIScene(GetScene()).SetScreenResolution(OldResolution.ResX, OldResolution.ResY, OldResolution.bFullscreen);
		}
		else
		{
			FinalMsg=Localize("MessageBox", "KeepResolution_Message", "UTGameUI");
			FinalMsg=Repl(FinalMsg, "\`Seconds\`", ""$int(SettingsTimeoutTime-TimeElapsed));

			MessageBoxReference.SetMessage(FinalMsg);
		}
	}
}

/** Callback for when the confirm dialog closes. */
function OnConfirmDialog_Closed()
{
	MessageBoxReference.OnClosed = None;
	MessageBoxReference=None;
}

/** Callback allowing the tabpage to setup the button bar for the current scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	if(!IsConsole())
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Apply>", OnButtonBar_Apply);
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Advanced>", OnButtonBar_Advanced);
	}

	ConditionallyAppendDefaultsButton(ButtonBar);
}

/**
 * Gets the screen resolution
 */

/** Pulls default settings for screen resolution from the engine and sets the widgets appropriately. */
function SetupDefaults()
{
	local LocalPlayer LP;
	local Vector2D ViewportSize;
	local int OptionIndex;
	LP = GetPlayerOwner();

	// Screen Resolution
	LP.ViewportClient.GetViewportSize(ViewportSize);
	SetDataStoreStringValue("<UTStringList:ScreenResolution>", int(ViewportSize.X)$"x"$int(ViewportSize.Y));
	OptionIndex = OptionList.GetObjectInfoIndexFromName('ScreenResolution');
	if(OptionIndex != INDEX_NONE)
	{
		UIDataStoreSubscriber(OptionList.GeneratedObjects[OptionIndex].OptionObj).RefreshSubscriberValue();
		OldResolution.ResX=ViewportSize.X;
		OldResolution.ResY=ViewportSize.Y;

		// Full Screen
		if(LP.ViewportClient.IsFullScreenViewport())
		{
			StringListDataStore.SetCurrentValueIndex('FullScreen',0);
			OldResolution.bFullscreen = true;
		}
		else
		{
			StringListDataStore.SetCurrentValueIndex('FullScreen',1);
			OldResolution.bFullscreen = false;
		}
	}

	OptionIndex = OptionList.GetObjectInfoIndexFromName('FullScreen');
	if(OptionIndex != INDEX_NONE)
	{
		UIDataStoreSubscriber(OptionList.GeneratedObjects[OptionIndex].OptionObj).RefreshSubscriberValue();
	}
}

/** Applies the current settings to the screen, this is usually used for changing screen resolution. */
function OnApplySettings()
{
	local string NewResolutionStr;
	local int CurrentIndex;
	local int PosX;
	local array<string> MessageBoxOptions;

	if(!IsConsole())
	{
		// Apply screen resolution change.
		if(GetDataStoreStringValue("<UTStringList:ScreenResolution>", NewResolutionStr) && Len(NewResolutionStr)>0)
		{
			`Log("Screen Resolution Changed: " $ NewResolutionStr);

			// Parse out resX and resY from the string
			PosX = InStr(NewResolutionStr, "x");

			if(PosX != INDEX_NONE)
			{
				NewResolution.ResX = int(Mid(NewResolutionStr,0,PosX));
				NewResolution.ResY = int(Mid(NewResolutionStr,PosX+1));

				// Check our full screen setting.
				CurrentIndex = StringListDataStore.GetCurrentValueIndex('FullScreen');
				NewResolution.bFullscreen = (CurrentIndex==0);

				if(NewResolution != OldResolution)
				{
					StartResChangeTime=UTUIScene(GetScene()).GetWorldInfo().RealTimeSeconds;
					UTUIScene(GetScene()).SetScreenResolution(NewResolution.ResX, NewResolution.ResY, NewResolution.bFullscreen);

					// Display the timeout dialog
					MessageBoxReference = UTUIScene(GetScene()).GetMessageBoxScene();


					MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Accept>");
					MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Cancel>");

					MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
					MessageBoxReference.OnClosed = OnConfirmDialog_Closed;
					MessageBoxReference.Display("<Strings:UTGameUI.MessageBox.KeepResolution_Title>", "<Strings:UTGameUI.MessageBox.KeepResolution_Title>", OnKeepResolution_Confirm, 1);
				}
			}
		}
	}
}

/** Callback for whether or not the user decided to keep their resolution. */
function OnKeepResolution_Confirm(UTUIScene_MessageBox MessageBox, int Selection, int PlayerIndex)
{
	if(Selection==1)
	{
		UTUIScene(GetScene()).SetScreenResolution(OldResolution.ResX, OldResolution.ResY, OldResolution.bFullscreen);
	}
	else
	{
		OldResolution=NewResolution;
	}

	MessageBoxReference.OnClosed=None;
	MessageBoxReference=None;
}

/** Shows the advanced settings screen. */
function OnShowAdvancedSettings()
{
	UTUIScene(GetScene()).OpenSceneByName(AdvancedSettingsScene);
}

/** Buttonbar Callbacks. */
function bool OnButtonBar_Apply(UIScreenObject InButton, int PlayerIndex)
{
	OnApplySettings();

	return true;
}

function bool OnButtonBar_Advanced(UIScreenObject InButton, int PlayerIndex)
{
	OnShowAdvancedSettings();

	return true;
}


function OnOptionList_OptionChanged(UIScreenObject InObject, name OptionName, int PlayerIndex)
{
	local bool bSaveAndRefresh;
	local array<UIDataStore> OutDataStores;

	Super.OnOptionList_OptionChanged(InObject, OptionName, PlayerIndex);

	bSaveAndRefresh = false;

	if(OptionName=='Brightness')
	{
		bSaveAndRefresh=true;
	}

	if(bSaveAndRefresh)
	{
		UIDataStorePublisher(InObject).SaveSubscriberValue(OutDataStores);
		UTUIScene(GetScene()).GetUTPlayerOwner().UpdateVolumeAndBrightness();
	}
}

/**
 * Callback for the reset to defaults confirmation dialog box.
 *
 * @param SelectionIdx	Selected item
 * @param PlayerIndex	Index of player that performed the action.
 */
function OnResetToDefaults_Confirm(UTUIScene_MessageBox MessageBox, int SelectionIdx, int PlayerIndex)
{
	local LocalPlayer LP;
	local UTProfileSettings Profile;
	local UTUIScene UTScene;
	local int ProfileId;
	local Vector2D ViewportSize;
	local bool bFullscreen;
	local int OptionIndex;
	local int Value;


	// Call parent implementation.  This will set most of the default video settings.
	super.OnResetToDefaults_Confirm( MessageBox, SelectionIdx, PlayerIndex );


	if( !IsConsole() && SelectionIdx==0 )
	{
		// Grab current viewport size
		LP = GetPlayerOwner();
		LP.ViewportClient.GetViewportSize(ViewportSize);

		// Grab current fullscreen state
		bFullscreen = LP.ViewportClient.IsFullScreenViewport();


		UTScene = UTUIScene(GetScene());
		if(UTScene != None)
		{
			Profile = UTProfileSettings( UTScene.GetPlayerInterface().GetProfileSettings( GetPlayerOwner().ControllerId ) );

			if(Profile != None)
			{
				if(Profile.GetProfileSettingId( name( "ScreenResolutionX" ), ProfileId ) )
				{
					Profile.ResetToDefault( ProfileId );
					Profile.GetProfileSettingValueInt( ProfileId, Value );
					ViewportSize.X = float( Value );
				}

				if(Profile.GetProfileSettingId( name( "ScreenResolutionY" ), ProfileId ) )
				{
					Profile.ResetToDefault( ProfileId );
					Profile.GetProfileSettingValueInt( ProfileId, Value );
					ViewportSize.Y = float( Value );
				}

				// @todo: Should we support saving/restoring (and setting defaults for) fullscreen state?
// 				if(Profile.GetProfileSettingId( name( "Fullscreen" ), ProfileId ) )
// 				{
// 					Profile.ResetToDefault( ProfileId );
// 					Profile.GetProfileSettingValueInt( ProfileId, Value );
// 					bFullscreen = bool( Value );
// 				}
			}
		}

		// Update 'ScreenResolution' menu option
		OptionIndex = OptionList.GetObjectInfoIndexFromName( 'ScreenResolution' );
		SetDataStoreStringValue( "<UTStringList:ScreenResolution>", int( ViewportSize.X )$"x"$int( ViewportSize.Y ) );
		UIDataStoreSubscriber( OptionList.GeneratedObjects[ OptionIndex ].OptionObj ).RefreshSubscriberValue();

		// Update 'FullScreen' menu option
		OptionIndex = OptionList.GetObjectInfoIndexFromName( 'FullScreen' );
		if( bFullScreen )
		{
			StringListDataStore.SetCurrentValueIndex( 'FullScreen', 0 );	// 0 == full screen, 1 == windowed mode
		}
		else
		{
			StringListDataStore.SetCurrentValueIndex( 'FullScreen', 1 );	// 0 == full screen, 1 == windowed mode
		}
		UIDataStoreSubscriber( OptionList.GeneratedObjects[ OptionIndex ].OptionObj ).RefreshSubscriberValue();


		// Apply the changes immediately
		OnApplySettings();
	}

}


defaultproperties
{
	bAllowResetToDefaults=true
	AdvancedSettingsScene="UI_Scenes_FrontEnd.Scenes.SettingsVideoAdvanced";
	SettingsTimeoutTime=30.0
	bRequiresTick=true
}
