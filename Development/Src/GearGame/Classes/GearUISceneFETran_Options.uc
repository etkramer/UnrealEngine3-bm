/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearUISceneFETran_Options extends GearUISceneFETran_Base
	Config(UI);

`include(Engine/Classes/UIDev.uci)

/** References to widgets in the scene */
var transient UILabelButton btnGame;
var transient UILabelButton btnVideo;
var transient UILabelButton btnAudio;
var transient UILabelButton btnVersus;
var transient UILabelButton btnController;
var transient UILabelButton btnStorage;
var transient UILabelButton btnCredits;

/** Config values for enabling/disabling the buttons */
var config bool bGameDisabled;
var config bool bAVDisabled;
var config bool bVersusDisabled;
var config bool bControllerDisabled;
var config bool bStorageDisabled;
var config bool bCreditsDisabled;

/** Whether an option screen changed and thus requires us to save the profile */
var bool bOptionsNeedSaved;

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */

/* === UIScreenObject interface === */
/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	Super.PostInitialize();

	InitializeWidgetReferences();

	// if this is not the primary player, don't allow them to modify audio/video/storage settings.
	if ( GetPlayerOwnerIndex() > 0 )
	{
		btnVideo.SetEnabled(false);
		btnAudio.SetEnabled(false);
		btnStorage.SetEnabled(false);
		// Fix the navigation
		btnController.SetForcedNavigationTarget(UIFACE_Bottom, btnGame);
		btnGame.SetForcedNavigationTarget(UIFACE_Top, btnController);
	}
}

/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	btnGame	= UILabelButton(FindChild('btnGame',true));
	btnVideo = UILabelButton(FindChild('btnVideoSettings',true));
	btnAudio = UILabelButton(FindChild('btnAV',true));
	btnVersus = UILabelButton(FindChild('btnVersus',true));
	btnController = UILabelButton(FindChild('btnController',true));
	btnStorage = UILabelButton(FindChild('btnStorage',true));
	btnCredits = UILabelButton(FindChild('btnCredits',true));

`if(`notdefined(dev_build))
	btnGame.SetEnabled( !bGameDisabled );
	btnAudio.SetEnabled( !bAVDisabled );
	btnVersus.SetEnabled( !bVersusDisabled );
	btnController.SetEnabled( !bControllerDisabled );
	btnStorage.SetEnabled( !bStorageDisabled );
	btnCredits.SetEnabled( !bCreditsDisabled );
`endif
}

/* == SequenceAction handlers == */
function OnShowDeviceSelectionUI( UIAction_ShowDeviceSelectionUI Action )
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;

	if ( Action.PlayerIndex == 0 )
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			PlayerIntEx = OnlineSub.PlayerInterfaceEx;
			if (PlayerIntEx != None)
			{
				// we need to know when the user has selected a new device.
				PlayerIntEx.AddDeviceSelectionDoneDelegate(class'UIInteraction'.static.GetPlayerControllerId(0),OnDeviceSelectionComplete);
			}
		}
	}
}

/* == Delegate handlers == */
/** Accepts the changes and closes the scene */
function bool OnBackClicked(UIScreenObject EventObject, int PlayerIndex)
{
	CloseScene(self);
	return true;
}

/**
 * Handler for this scene's GetSceneInputModeOverride delegate.  Forces INPUTMODE_Locked if in the front-end.
 */
function EScreenInputMode OverrideSceneInputMode()
{
	local EScreenInputMode Result;

	Result = GetSceneInputMode(true);
	if ( class'WorldInfo'.static.IsMenuLevel() )
	{
		Result = INPUTMODE_Locked;
	}

	return Result;
}

/** Called when the scene is closed so we can stop the music */
function OnSceneDeactivatedCallback( UIScene DeactivatedScene )
{
	local int PlayerOwnerIndex;

	PlayerOwnerIndex = GetPlayerOwnerIndex();
	if ( bOptionsNeedSaved )
	{
		SaveProfile(PlayerOwnerIndex);
	}
}

/**
 * Reads the results of the user's device choice
 *
 * @param bWasSuccessful true if the action completed without error, false if there was an error
 */
function OnDeviceSelectionComplete(bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local int ControllerId, DeviceID;

	local UIInteraction UIController;
	local GearEngine GE;
	local GearUISceneFETran_MainMenu MainMenu;
	local string UnusedDeviceName;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if (PlayerIntEx != None)
		{
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(0);
			// Unregister our callback
			PlayerIntEx.ClearDeviceSelectionDoneDelegate(ControllerId,OnDeviceSelectionComplete);

			// Don't read the information unless it was successful
			if (bWasSuccessful == true)
			{
				// Read the per user results
				DeviceID = PlayerIntEx.GetDeviceSelectionResults(ControllerId,UnusedDeviceName);
				UIController = GetCurrentUIController();
				GE = GearEngine(UIController.Outer.Outer);

				bOptionsNeedSaved = bOptionsNeedSaved || GE.GetCurrentDeviceID() != DeviceID;
				GE.SetCurrentDeviceID(DeviceID);
				GE.bShouldWriteCheckpointToDisk = true;

				MainMenu = GearUISceneFETran_MainMenu(GetPreviousScene(false, true));
				if ( MainMenu != None )
				{
					MainMenu.ClearMemorySlotAvailabilityCache();
					//@todo ronp - do I need this one?
//					MainMenu.BuildMemorySlotAvailabilityCache();
				}
			}
		}
	}
}

defaultproperties
{
	OnSceneDeactivated=OnSceneDeactivatedCallback
	bRenderParentScenes=true
	bMenuLevelRestoresScene=true
	GetSceneInputModeOverride=OverrideSceneInputMode
	SceneRenderMode=SPLITRENDER_PlayerOwner
	SceneInputMode=INPUTMODE_MatchingOnly
}

