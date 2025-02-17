/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearUIScene_Base extends UIScene
	abstract
	native(UI)
	Config(UI);

/************************************************************************/
/* Constants															*/
/************************************************************************/
const GEAR_SCENE_PRIORITY_NOTIFICATION	= 40;
const GEAR_SCENE_PRIORITY_BLOCKASYNC	= 20;
const GEAR_SCENE_PRIORITY_NORMAL		= 10;
const GEAR_SCENE_PRIORITY_GAMEOVER		= 11;
const GEAR_SCENE_PRIORITY_COOPSPECTATE	= 9;
const GEAR_SCENE_PRIORITY_DISCOVERABLES = 0;
const GEAR_SCENE_PRIORITY_MPSCENE		= 0;
const GEAR_SCENE_PRIORITY_TUTORIAL		= -10;

/** Enum of the different icons that can be added to navigation paths and menu options */
enum EGearNavigationIconType
{
	eGEARNAVICON_NewStuff,
	eGEARNAVICON_Complete,
};

/** Enum of the ways you can dock the description label to the widget it describes */
enum EGearDescriptionDockType
{
	eGEARDESCDOCK_None,
	eGEARDESCDOCK_Left,
	eGEARDESCDOCK_Right,
	eGEARDESCDOCK_Top,
	eGEARDESCDOCK_Bottom,
};

/** Enum of stat types for the scoreboard */
enum GearScoreboardStatType
{
	eGSStat_Deaths,
	eGSStat_Downs,
	eGSStat_Kills,
	eGSStat_Revives,
	eGSStat_AnnexCap,
	eGSStat_AnnexBreak,
	eGSStat_MeatflagCap,
};

/**
 * Stores the parameters from the last call to UpdateDescriptionLabel.
 * Having these parameters allows us to keep showing the old data for a while before fading to the new parameters.
 */
struct native DescriptionLabelParams
{
	var string StringToBind;
	var UIScreenObject WidgetToDockTo;
	var EGearDescriptionDockType GearDockType;
	var float DockPadding;
};

/** Struct to group the localization string path to a description to the widget it is describing */
struct native GearDescriptionData
{
	/** Localization path to the description string */
	var() string LocalizationPath;

	/** Side of the WidgetToDescribe to dock the description label to */
	var() EGearDescriptionDockType FaceToDockTo;

	/** Name of the widget the description is associated with */
	var() name WidgetToDescribeName;

	/** Name of the widget the description will dock to (if left empty, we will assume it's the WidgetToDescribeName widget) */
	var() name WidgetToDockToName;

	/** Number of pixels to pad when docking */
	var() float DockPadding;

	/** Widget a description is associated with */
	var transient UIObject WidgetToDescribe;

	/** Widget a description will dock to */
	var transient UIObject WidgetToDockTo;
};

/** Stores the parameters from the last call to UpdateDescriptionLabel */
var transient DescriptionLabelParams UpdateDescriptionLabelParams;


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Localized string for the title conjunction */
var localized string GameToMapConjunction;

var	transient UILabel SelectionHintLabel;
/** Label that any scene CAN have access to (if it is part of the scene itself) where descriptions can be placed */
var transient UILabel DescriptionLabel;
/** Label that fascilitates the fading of tooltips (descriptions) */
var transient UILabel TooltipLabelFader;
/** Background image of the DescriptionLabel used for describing widgets in the scene */
var transient UIImage DescriptionBGImage;
/** Throbber arrow image that is optionally present as part of the  */
var transient UIImage DescriptionArrowImage;

/** List of widgets that will describe themselves in the description label */
var(Controls) array<GearDescriptionData> DescriptionList;
/** Last known used index in the descriptionlist */
var transient int CurrentDescriptionListIndex;

/** Time remaining until timer runs out */
var transient float	ToolTipTimerTimeRemaining;

/** Opening animation is completed */
var transient bool bOpeningAnimationCompleted;

/**
 * Set to TRUE to indicate that the user must be signed into a valid profile in order to view this scene.  If the user is not signed into
 * a profile, the scene will automatically close and will not open until a profile exists.
 */
var(Flags)							bool	bRequiresProfile;

/**
 * Controls whether this scene is allowed to create a new local player if a sign-in notification is received.
 */
var	const	transient	private		bool	bAllowPlayerJoin;

/**
 * Controls whether sign-in changes are allowed in this scene.  Specifying FALSE will return the player to the main menu when sign-in
 * status changes are detected.
 */
var	const	transient	private		bool	bAllowSigninChanges;


/************************************************************************/
/* C++ functions                                                        */
/************************************************************************/
cpptext
{
	/** Overloaded to call our update function */
	virtual void Tick( FLOAT DeltaTime );
}

/* == Delegates == */

/** Called when the timer runs out */
delegate OnToolTipTimerExpired();

/** Called by tick to update script code if subscribed to */
delegate OnGearUISceneTick( float DeltaTime );

/**
 * Delegate called after the CheckForMemoryDevice is finished (whether or not the selection is needed)
 *		1) It will be triggered after the player selects a device OR
 *		2) It will also be called if device selection can't happen or is not needed.
 *
 *  NOTE!!!  Make sure to clear this delegate when called by using ClearDeviceMemoryCheckCompleteDelegate!!!!
 */
delegate OnDeviceMemoryCheckComplete(bool bWasSuccessful);

/* == Natives == */

/* == Events == */

/* == UnrealScript == */

/**
 * Set a timer that will call a delegate when the timer expires.
 * Setting the timer repeatedly clears previous entries.
 *
 * @param TimerTime The initial timer setting (from which to count down)
 * @param TimerExpiredHandler The delegate to call when the timer reaches zero.
 */
function SetTimer(float TimerTime, delegate<OnToolTipTimerExpired> TimerExpiredHandler)
{
	ToolTipTimerTimeRemaining = TimerTime;
	OnToolTipTimerExpired = TimerExpiredHandler;
}

/* == SequenceAction handlers == */


/**
 * Handler for the completion of this scene's opening animation...
 *
 * @warning - if you override this in a child class, keep in mind that this function will not be called if the scene has no opening animation.
 */
function OnOpenAnimationComplete( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	Super.OnOpenAnimationComplete(Sender, AnimName, TrackTypeMask);

	bOpeningAnimationCompleted = true;
	if ( TooltipLabelFader != None )
	{
		TooltipLabelFader.Opacity = 0.0;
	}
}

/**
 * Returns the PRI of the player in the list at index Idx
 * @param lstPlayers - the UIList the players are stored in
 * @param ListIdx - Will default to the currently selected list item if one is not provided,
 *					If one IS provided it is assumed that index passed in is an index into the Items array of the list
 */
final function GearPRI GetPRIFromPlayerList( UIList lstPlayers, optional int ListIdx = -1 )
{
	local WorldInfo WI;
	local string SelectedPlayerNetIdMarkupString;
	local UIProviderScriptFieldValue FieldValue;
	local int Idx;
	local UniqueNetId CurrID;

	if ( lstPlayers != None && lstPlayers.Items.length > ListIdx )
	{
		SelectedPlayerNetIdMarkupString = Left(lstPlayers.DataSource.MarkupString, Len(lstPlayers.DataSource.MarkupString) - 1) $ ";" $ lstPlayers.Items[ListIdx] $ ".UniqueId>";
		if ( GetDataStoreFieldValue(SelectedPlayerNetIdMarkupString, FieldValue, Self) )
		{
			WI = GetWorldInfo();
			if ( WI != None && WI.GRI != None )
			{
				for ( Idx = 0; Idx < WI.GRI.PRIArray.length; Idx++ )
				{
					if ( WI.GRI.PRIArray[Idx] != None )
					{
						CurrID = WI.GRI.PRIArray[Idx].UniqueId;
						if ( class'OnlineSubsystem'.static.AreUniqueNetIdsEqual(CurrID,FieldValue.NetIdValue) )
						{
							return GearPRI(WI.GRI.PRIArray[Idx]);
						}
					}
				}
			}
		}
	}

	return None;
}

/**
 * Helper function to encapsulate whether a valid storage device is set or not
 *
 * @param PlayerIndex - PlayerIndex to the player to perform this check on
 *
 * @param CurrDeviceID - returns the DeviceID that is currently being used
 *
 * @return - whether the storage device is set properly or not
 */
final function bool StorageDeviceIsReady( int PlayerIndex, optional out int CurrDeviceID )
{
	local GearPC CurrGearPC;
	local int DeviceID, ControllerId;
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local bool bResult;

	if ( IsConsole() )
	{
		CurrGearPC = GetGearPlayerOwner(PlayerIndex);

		// Get the device id from the profile
		ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
		if (CurrGearPC != None && IsLoggedIn(ControllerId) && CurrGearPC.ProfileSettings != None
		&&	CurrGearPC.ProfileSettings.GetProfileSettingValueInt(CurrGearPC.ProfileSettings.SelectedDeviceID, DeviceID) )
		{
			CurrDeviceID = DeviceID;
			// Make sure this device is valid
			OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
			if ( OnlineSub != None )
			{
				PlayerIntEx = OnlineSub.PlayerInterfaceEx;
				if ( PlayerIntEx != None )
				{
					bResult = PlayerIntEx.IsDeviceValid( DeviceID );
				}
			}
//`log(`location @ `showvar(CurrDeviceID) @ `showvar(PlayerIndex) @ `showvar(bResult));
		}
		else
		{
			CurrDeviceID = -1;
		}
	}
	else
	{
		bResult = true;
	}

//	`log(`location @ `showvar(PlayerIndex) @ `showvar(CurrDeviceId) @ `showvar(bResult),,'RON_DEBUG');
	return bResult;
}

/**
 * Helper function to encaspulate the preparing of the checkpoint system for getting checkpoints
 *
 * @return - whether the checkpoint system was able to be prepared for access or not
 */
final function bool PrepareCheckpointSystem( int PlayerIndex )
{
	local GearEngine Engine;
	local GearPC CurrGearPC;
	local int DeviceID;

	CurrGearPC = GetGearPlayerOwner(PlayerIndex);
	if ( CurrGearPC != None )
	{
		Engine = GearEngine(CurrGearPC.Player.Outer);
		if ( Engine != None )
		{
			if ( StorageDeviceIsReady(PlayerIndex, DeviceID) )
			{
				// Set the checkpoint system to look at the proper player and device
				Engine.CurrentUserID = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
				Engine.SetCurrentDeviceID(DeviceID);
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	// Must be PC
	return true;
}

/**
 * Checks to see if the profile has a valid device ID and will attempt to retrieve and store one if one does not exist
 *
 * bAlwaysShowUI - will always force the blade to show even when there is only 1 choice
 *
 */
final function CheckForMemoryDevice( delegate<OnDeviceMemoryCheckComplete> CheckCompleteCallback, optional bool bAlwaysShowUI, optional bool bManageStorage )
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local int PlayerIndex, DeviceID, ControllerId;

	PlayerIndex = `PRIMARY_PLAYER_INDEX;
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None )
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if ( PlayerIntEx != None )
		{
			// See if there's a valid device
			if ( bAlwaysShowUI || bManageStorage || !StorageDeviceIsReady(PlayerIndex, DeviceID) )
			{
				// Register our callback
				ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
				if ( IsLoggedIn(ControllerId) )
				{
					PlayerIntEx.AddDeviceSelectionDoneDelegate(ControllerId, OnDeviceSelectionComplete );
					if ( CheckCompleteCallback != None )
					{
						PlayerIntEx.AddDeviceSelectionDoneDelegate( ControllerId, CheckCompleteCallback );
					}

					// Show the UI
					if ( !PlayerIntEx.ShowDeviceSelectionUI(ControllerId, class'GearEngine'.const.MAX_DATASIZE_FOR_ALL_CHECKPOINTS, bAlwaysShowUI, bManageStorage) )
					{
						// Don't wait if there was an error
						`Log("Error occurred while trying to display device selection UI");
						PlayerIntEx.ClearDeviceSelectionDoneDelegate( ControllerId, OnDeviceSelectionComplete );
						if ( CheckCompleteCallback != None )
						{
							PlayerIntEx.ClearDeviceSelectionDoneDelegate( ControllerId, CheckCompleteCallback );
						}
					}

					// Return so that the callbacks system can take over
					return;
				}
				else
				{
					`log(`location @ "player" @ PlayerIndex @ "isn't signed in so not showing device selection UI!");
				}
			}
		}
		else
		{
			`Log("No OnlinePlayerInterfaceEx present to display the device selection UI");
		}
	}

	if ( CheckCompleteCallback != None )
	{
		CheckCompleteCallback( true );
	}
}

/**
 * Reads the results of the user's device choice
 *
 * @param bWasSuccessful true if the action completed without error, false if there was an error
 */
function OnDeviceSelectionComplete( bool bWasSuccessful )
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local string DeviceName;
	local int DeviceID;
	local GearPC CurrGearPC;
	local int PlayerIndex;

	PlayerIndex = `PRIMARY_PLAYER_INDEX;
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None )
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if (PlayerIntEx != None)
		{
			// Unregister our callback
			PlayerIntEx.ClearDeviceSelectionDoneDelegate(class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex),OnDeviceSelectionComplete);
			// Don't read the information unless it was successful
			if ( bWasSuccessful )
			{
				// Read the per user results
				DeviceID = PlayerIntEx.GetDeviceSelectionResults(class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex),DeviceName);
			}
		}
	}

	// If they selected a device
	if ( DeviceID != 0 )
	{
		CurrGearPC = GetGearPlayerOwner( PlayerIndex );
		if ( CurrGearPC != None && CurrGearPC.ProfileSettings != None )
		{
			// Set device id in the profile
			CurrGearPC.ProfileSettings.SetCurrentDeviceID( DeviceID );

			// this call to SaveProfile can happen before the profile is read in the initial sign-in case...so just don't save
			//CurrGearPC.SaveProfile();
		}
	}
}

/**
 * Clears the OnDeviceMemoryCheckComplete delegate from the online subsystem
 */
function ClearDeviceMemoryCheckCompleteDelegate( int PlayerIndex, delegate<OnDeviceMemoryCheckComplete> CheckCompleteCallback )
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None )
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if (PlayerIntEx != None)
		{
			// Unregister our callback
			PlayerIntEx.ClearDeviceSelectionDoneDelegate(class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex),CheckCompleteCallback);
		}
	}
}

/** Called when the description goes active for the widget */
function OnDescriptionActive( int DescriptionIndex );

/** Initialize the description label and the list of widgets it will describe */
function InitializeDescriptionSystem()
{
	local int Idx;

	// Find the description label
	if ( DescriptionLabel == None )
	{
		DescriptionLabel = UILabel(FindChild('lblSelectionDescription', true));
		if (TooltipLabelFader == None)
		{
			TooltipLabelFader = UILabel(FindChild('lblTooltipFader', true));
		}
	}

	// If this screen actually has a description widget
	if ( DescriptionLabel != None )
	{
		DescriptionLabel.SetVisibility( false );

		if ( DescriptionBGImage == None )
		{
			DescriptionBGImage = UIImage(FindChild('imgSelectionDescriptionBG', true));
			DescriptionArrowImage = UIImage(FindChild('imgArrow', true));
		}

		if ( DescriptionBGImage != None )
		{
			DescriptionBGImage.SetVisibility( false );
		}

		for ( Idx = 0; Idx < DescriptionList.length; Idx++ )
		{
			InitializeDescriptionElementTransientVariables( Idx );
		}
	}
}

/** Intializes the transient variables associated with a description list element */
function InitializeDescriptionElementTransientVariables( int Idx )
{
	if ( DescriptionList[Idx].WidgetToDescribeName != '' )
	{
		// Find the widget to describe
		DescriptionList[Idx].WidgetToDescribe = FindChild(DescriptionList[Idx].WidgetToDescribeName, true);

		if ( DescriptionList[Idx].WidgetToDescribe != None )
		{
			// Now setup the docking variables
			if ( DescriptionList[Idx].FaceToDockTo != eGEARDESCDOCK_None )
			{
				if ( DescriptionList[Idx].WidgetToDockToName != '' )
				{
					// Find the widget to dock to
					DescriptionList[Idx].WidgetToDockTo = FindChild(DescriptionList[Idx].WidgetToDockToName, true);
				}

				// If we couldn't find a widget to dock to, use the widget we are describing
				if ( DescriptionList[Idx].WidgetToDockTo == None )
				{
					DescriptionList[Idx].WidgetToDockTo = DescriptionList[Idx].WidgetToDescribe;
				}
			}
		}
	}
}

/**
 * Notification that one or more tracks in an animation sequence have completed.
 *
 * @param	Sender				the widget that completed animating.
 * @param	AnimName			the name of the animation sequence that completed.
 * @param	TypeMask			a bitmask indicating which animation tracks completed.  It is generated by left shifting 1 by the
 *								values of the EUIAnimType enum.
 *								A value of 0 indicates that all tracks in the animation sequence have finished.
 */
event UIAnimationEnded( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	Super.UIAnimationEnded( Sender, AnimName, TrackTypeMask );

	if ( TrackTypeMask == 0)
	{
		if (TooltipLabelFader != None && Sender == TooltipLabelFader)
		{
			UpdateBackgroundDocking();
		}
	}

}

/**
 * Docks the tooltip background to whichever is larger, the fading tooltip label or the current tooltip label
 */
function UpdateBackgroundDocking()
{

	if (TooltipLabelFader != None)
	{

		// The background area should be docked to whichever tooltip is longer to handle
		// the cases where tooltips occupy a different number of lines.
		if ( Len( TooltipLabelFader.GetValue() ) > Len( DescriptionLabel.GetValue() )
			 && TooltipLabelFader.Opacity > 0 )
		{
			DescriptionBGImage.SetDockTarget(UIFACE_Bottom, TooltipLabelFader, UIFACE_Bottom);
			DescriptionBGImage.SetDockTarget(UIFACE_Top, TooltipLabelFader, UIFACE_Top);
			DescriptionBGImage.SetDockTarget(UIFACE_Left, TooltipLabelFader, UIFACE_Left);
			DescriptionBGImage.SetDockTarget(UIFACE_Right, TooltipLabelFader, UIFACE_Right);
		}
		else
		{
			DescriptionBGImage.SetDockTarget(UIFACE_Bottom, DescriptionLabel, UIFACE_Bottom);
			DescriptionBGImage.SetDockTarget(UIFACE_Top, DescriptionLabel, UIFACE_Top);
			DescriptionBGImage.SetDockTarget(UIFACE_Left, DescriptionLabel, UIFACE_Left);
			DescriptionBGImage.SetDockTarget(UIFACE_Right, DescriptionLabel, UIFACE_Right);
		}

	}
}


/**
 * Update the hint text and related widgets. Do this with a delay depending on whether the label is stationaty.
 * If the hint text moves around to follow the currently selected widget use a 1 second delay before switching to a new label.
 */
function UpdateDescriptionLabel( string StringToBind, UIScreenObject WidgetToDockTo, EGearDescriptionDockType GearDockType, float DockPadding )
{
	local bool bCrossfade;
	bCrossfade = TooltipLabelFader != None;

	// Save the desired tooltip params for the future (after we've faded out)
	// This is necessary in the non-Crossfading case
	UpdateDescriptionLabelParams.StringToBind = StringToBind;
	UpdateDescriptionLabelParams.WidgetToDockTo = WidgetToDockTo;
	UpdateDescriptionLabelParams.GearDockType = GearDockType;
	UpdateDescriptionLabelParams.DockPadding = DockPadding;

	// Fade out current label
	if (bOpeningAnimationCompleted) //@hack playing an animation while the scene is opening has the potential to break it
	{
		if (bCrossfade)
		{
			if (TooltipLabelFader.Opacity < 0.5 )
			{
				TooltipLabelFader.SetDataStoreBinding( DescriptionLabel.GetDataStoreBinding() );
				TooltipLabelFader.StopUIAnimation( 'HintTextFadeOut' );
				TooltipLabelFader.StopUIAnimation( 'HintTextCrossFadeOut' );
				TooltipLabelFader.StopUIAnimation( 'HintTextFadeIn' );
				TooltipLabelFader.Opacity = 1.0;
				TooltipLabelFader.SetVisibility(true);
				TooltipLabelFader.PlayUIAnimation( 'HintTextCrossFadeOut',, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );
			}
		}
		else
		{
			if ( DescriptionLabel != None )
			{
				DescriptionLabel.StopUIAnimation( 'HintTextFadeOut' );
				DescriptionLabel.StopUIAnimation( 'HintTextFadeIn' );
				DescriptionLabel.PlayUIAnimation( 'HintTextFadeOut',, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );
			}
			if ( DescriptionBGImage != None )
			{
				DescriptionBGImage.StopUIAnimation( 'HintTextFadeOut',  );
				DescriptionBGImage.StopUIAnimation( 'HintTextFadeIn',  );
				DescriptionBGImage.PlayUIAnimation( 'HintTextFadeOut', , /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );
			}
		}
	}

	// If we have the little red arrow (a.la. main menu), hide it
	if ( DescriptionArrowImage != None )
	{
		DescriptionArrowImage.SetVisibility( false );
	}

	if (bCrossfade)
	{
		ShowTooltip();
	}
	else
	{
		// Tooltips take a different amount of time to show up depending on
		// whether they move with the menus or stay static in one spot.
		// "No docking" implies static tooltips.
		if ( GearDockType != eGEARDESCDOCK_None && WidgetToDockTo != None )
		{
			// Just wait long enough for the fade out to complete.
			SetTimer( 1.0, ShowTooltip );
		}
		else
		{
			// Just wait long enough for the fade out to complete. (And also long enough to not expose the input blocking bug.)
			SetTimer( 0.5, ShowTooltip );
		}
	}
}

/** Fade in the tooltip  */
function ShowTooltip()
{
	local bool bCrossfade;
	bCrossfade = TooltipLabelFader != None;

	SetDescriptionLabel(UpdateDescriptionLabelParams.StringToBind,
						UpdateDescriptionLabelParams.WidgetToDockTo,
						UpdateDescriptionLabelParams.GearDockType,
						UpdateDescriptionLabelParams.DockPadding);

	UpdateBackgroundDocking();

	if (DescriptionLabel != None)
	{
		DescriptionLabel.StopUIAnimation( 'HintTextFadeIn' );
		DescriptionLabel.StopUIAnimation( 'HintTextFadeOut' );
		DescriptionLabel.StopUIAnimation( 'HintTextCrossFadeOut' );
		DescriptionLabel.Opacity = 0.0;
		DescriptionLabel.PlayUIAnimation( 'HintTextFadeIn', , /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );
		DescriptionLabel.SetVisibility( true );
	}

	if (DescriptionBGImage != None )
	{
		DescriptionBGImage.StopUIAnimation( 'HintTextFadeIn' );
		DescriptionBGImage.StopUIAnimation( 'HintTextFadeOut' );
		if (!bCrossfade)
		{
			DescriptionBGImage.Opacity = 0.0;
		}
		DescriptionBGImage.PlayUIAnimation( 'HintTextFadeIn', , /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );
		DescriptionBGImage.SetVisibility( true );
	}

	if ( DescriptionArrowImage != None )
	{
		DescriptionArrowImage.SetVisibility( true );
	}

}

/**
 * Binds a string to the description label and sets the docking of the description label to the passed in widget
 * @param bDockToLeft - Will dock the right side of the description to the left side of the WidgetToDockTo if TRUE
 */
function SetDescriptionLabel( string StringToBind, UIScreenObject WidgetToDockTo, EGearDescriptionDockType GearDockType, float DockPadding )
{
	if ( DescriptionLabel != None )
	{
		DescriptionLabel.SetDataStoreBinding( StringToBind );

		if ( GearDockType != eGEARDESCDOCK_None && WidgetToDockTo != None )
		{
			if ( GearDockType == eGEARDESCDOCK_Left )
			{
				// We assume docking top to top
				DescriptionLabel.SetDockParameters( UIFACE_Top, WidgetToDockTo, UIFACE_Top, 0.0f );
				// Dock right side of description to left side of target
				DescriptionLabel.SetDockParameters( UIFACE_Right, WidgetToDockTo, UIFACE_Left, DockPadding );
			}
			else if ( GearDockType == eGEARDESCDOCK_Right )
			{
				// We assume docking top to top
				DescriptionLabel.SetDockParameters( UIFACE_Top, WidgetToDockTo, UIFACE_Top, 0.0f );
				// Dock left side of description to right side of target
				DescriptionLabel.SetDockParameters( UIFACE_Left, WidgetToDockTo, UIFACE_Right, DockPadding );
			}
			else if ( GearDockType == eGEARDESCDOCK_Top )
			{
				// Dock top to top
				DescriptionLabel.SetDockParameters( UIFACE_Top, WidgetToDockTo, UIFACE_Top, DockPadding );
			}
			else
			{
				// Dock bottom to bottom
				DescriptionLabel.SetDockParameters( UIFACE_Bottom, WidgetToDockTo, UIFACE_Bottom, DockPadding );
			}
		}
	}
}


///** Hides the description label */
//function ClearDescriptionLabel()
//{
//	if ( DescriptionLabel != None )
//	{
//		DescriptionLabel.PlayUIAnimation( '', HintTextFadeOut, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );
//		if ( DescriptionBGImage != None )
//		{
//			DescriptionBGImage.PlayUIAnimation( '', HintTextFadeOut, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );
//		}
//
//		if ( DescriptionArrowImage != None )
//		{
//			DescriptionArrowImage.SetVisibility( false );
//		}
//
//		if ( SelectionHintLabel != None )
//		{
//			SelectionHintLabel.SetVisibility(false);
//		}
//	}
//}

/** Helper function which will set add description list data for each widget and dataprovider in a GearUIObjectList */
function AddObjectListElementsToDescriptionList( GearUIObjectList ObjectList, class<UIResourceDataProvider> ProviderClass )
{
	local int ObjIdx, DescListIdx;

	if ( ObjectList != None )
	{
		for ( ObjIdx = 0; ObjIdx < ObjectList.GeneratedObjects.length; ObjIdx++ )
		{
			if ( ObjectList.GeneratedObjects[ObjIdx].OptionProvider != none &&
				ObjectList.GeneratedObjects[ObjIdx].OptionObj != None )
			{
				DescListIdx = DescriptionList.length;
				DescriptionList.length = DescListIdx + 1;
				DescriptionList[DescListIdx].WidgetToDescribeName = ObjectList.GeneratedObjects[ObjIdx].OptionObj.WidgetTag;

				DescriptionList[DescListIdx].LocalizationPath = GetLocalizationPathForDescriptionList( ObjectList.GeneratedObjects[ObjIdx].OptionProvider, ProviderClass );
				InitializeDescriptionElementTransientVariables( DescListIdx );
			}
		}
	}
}

/** Helper function for the AddObjectListElementsToDescriptionList() function to retrieve the localization path */
function string GetLocalizationPathForDescriptionList( UIDataProvider DataProviderInstance, class<UIResourceDataProvider> ProviderClass )
{
	if ( DataProviderInstance != None )
	{
		switch ( ProviderClass )
		{
			case class'UIDataProvider_MenuItem':
				if ( UIDataProvider_MenuItem(DataProviderInstance) != None )
				{
					return UIDataProvider_MenuItem(DataProviderInstance).DescriptionMarkup;
				}
			break;
		}
	}

	return "";
}

/** Get the enum value from a string */
static function byte ConvertEnumStringToValue( string EnumString, int MaxEnumValue, Object EnumType )
{
	local int Idx;
	local byte Result;

	if ( EnumType != None )
	{
		if ( EnumType.Class == class'Core.Enum' )
		{
			for ( Idx = 0; Idx < MaxEnumValue; Idx++ )
			{
				if ( EnumString == string(GetEnum(EnumType,Idx)) )
				{
					Result = Idx;
					break;
				}
			}
		}
		else
		{
			`log("ConvertEnumStringToByte - Invalid enum object specified!  Should pass in a literal reference to an enum (i.e. enum'ERemoteRole'):" @ EnumType);
		}
	}
	else
	{
		`log("ConvertEnumStringToByte - Null enum object specified!  Should pass in a literal reference to an enum (i.e. enum'ERemoteRole')");
	}

	return Result;
}

/**
 * Provided that the widgets necessary for showing a tooltip exist,
 * show the tooltip for the WidgetToDescribe
 *
 * @param WidgetToDescribe The widget we want to see described by the tooltip.
 */
function RefreshTooltip( UIScreenObject WidgetToDescribe )
{
	local int Idx;

	// Don't need to do anything if there's no description label
	if ( DescriptionLabel != None )
	{
		// Find the widget in our description list
		for ( Idx = 0; Idx < DescriptionList.length; Idx++ )
		{
			// Found it...
			if ( WidgetToDescribe == DescriptionList[Idx].WidgetToDescribe )
			{
				CurrentDescriptionListIndex = Idx;
				// Allows scene to manipulate the description text before setting it
				OnDescriptionActive( Idx );
				// Set the description
				UpdateDescriptionLabel( DescriptionList[Idx].LocalizationPath, DescriptionList[Idx].WidgetToDockTo, DescriptionList[Idx].FaceToDockTo, DescriptionList[Idx].DockPadding );
				break;
			}
		}
	}
}


/**
 * Called when a new UIState becomes the widget's currently active state, after all activation logic has occurred.
 *
 * @param	Sender					the widget that changed states.
 * @param	PlayerIndex				the index [into the GamePlayers array] for the player that activated this state.
 * @param	NewlyActiveState		the state that is now active
 * @param	PreviouslyActiveState	the state that used the be the widget's currently active state.
 */
function OnStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, UIState PreviouslyActiveState )
{


	// Only set the description if we've entered the "FOCUSED" state
	if ( Sender != None )
	{
		if ( NewlyActiveState.IsA('UIState_Focused') )
		{
			RefreshTooltip( Sender );
		}
	}
}

/**
 * Wrapper for displaying an error message explaining that the user was returned to the main menu due to a sign-in change.
 */
static final function DisplaySigninChangeDetected_ErrorMessage()
{
	local LocalPlayer PrimaryPlayer;

	PrimaryPlayer = class'UIInteraction'.static.GetLocalPlayer(`PRIMARY_PLAYER_INDEX);

	// make sure this error message appears on top of the "too many signed-in players" message...
	DisplayErrorMessage("IllegalSigninChange_Message", "IllegalSigninChange_Title", "GenericAccept", PrimaryPlayer,, false, 200);
}

/**
 * Safely returns the player to the main menu when sign-in status changes are received when sign-in/sign-out isn't allowed.
 *
 * @param	ControllerId	the id of the gamepad that triggered the sign-in status change.
 * @param	bForce			by default, this function only works in the front-end; specify TRUE to override this and return the player
 * 							to the front-end from the game.
 *
 * @return	TRUE if this scene handled the illegal login status change.
 */
static function bool ProcessIllegalPlayerLoginStatusChange( int ControllerId, optional bool bForce, optional ELoginStatus NewStatus=LS_NotLoggedIn )
{
	local UIScene ActiveScene, OriginalActiveScene;
	local GearUISceneFETran_MainMenu MainMenuScene;
	local GameUISceneClient GameSceneClient;
	local GearPC PrimaryPC;
	local bool bResult, bMenuLevel;

	`log("Login status change received for player using gamepad" @ ControllerId $ ".  Returning to main menu for handling...");

	bMenuLevel = class'WorldInfo'.static.IsMenuLevel();
	if ( bMenuLevel || bForce )
	{
		GameSceneClient = GetSceneClient();
		if ( GameSceneClient != None )
		{
			PrimaryPC = GearPC(GameSceneClient.GamePlayers[0].Actor);
			if ( bMenuLevel )
			{
				MainMenuScene = GearUISceneFETran_MainMenu(GameSceneClient.FindSceneByTag('UI_FE_MainMenu'));
			}

			if ( MainMenuScene != None )
			{
				// if the main menu is in the scene stack, we should always be in standalone mode; otherwise we'd need to do additional
				// cleanup
				`assert(PrimaryPC.WorldInfo.NetMode == NM_Standalone);

				ActiveScene = GameSceneClient.GetActiveScene(None, true);
				OriginalActiveScene = ActiveScene;

				while ( ActiveScene != MainMenuScene )
				{
					MainMenuScene.bPendingProfileRefresh = true;
					ActiveScene.CloseScene(ActiveScene, false, true);
					ActiveScene = GameSceneClient.GetActiveScene(None, true);
				}

				if ( ActiveScene == MainMenuScene )
				{
					bResult = true;
					if ( OriginalActiveScene != MainMenuScene )
					{
						// notify the main menu
						MainMenuScene.NotifyLoginStatusChanged(ControllerId, NewStatus);
					}
				}
			}
			else
			{
				// force stop any movies that are playing, since we disable input processing by the movie playe
				PrimaryPC.ClientStopMovie(0.f, false, true, true);

				// tell the player to return to the main menu
				PrimaryPC.ReturnToMainMenu();

				bResult = true;
			}

			if ( bResult )
			{
				DisplaySigninChangeDetected_ErrorMessage();
			}
		}
	}

	return bResult;
}

/**
 * Wrapper for getting a reference to the game engine.
 */
static final function GearEngine GetGearEngine()
{
	local UIInteraction UIController;
	local GearEngine Result;

	UIController = GetCurrentUIController();
	if ( UIController != None )
	{
		Result = GearEngine(UIController.Outer.Outer);
	}

	return Result;
}

/**
 * Returns the GearPC associated with the specified player index.
 *
 * @param	PlayerIndex		the index of the player to get a reference to; if not specified, uses the scene's player owner.
 *
 * @return	a ref to the player specified, or None if invalid.
 */
final function GearPC GetGearPlayerOwner( optional int PlayerIndex=INDEX_NONE )
{
	local LocalPlayer PO;

	PO = GetPlayerOwner(PlayerIndex);
	if ( PO != None )
	{
		return GearPC(PO.Actor);
	}

	return None;
}

/**
 * Wrapper for retrieving the index of the specified player's team.
 *
 * @param	PlayerIndex		the index of the player to get a reference to; if not specified, uses the scene's player owner.
 */
function int GetTeamIndex( optional int PlayerIndex=INDEX_NONE )
{
	local GearPC GearPlayerOwner;
	local int Result;

	Result = class'GearTeamInfo'.default.TeamIndex;

	GearPlayerOwner = GetGearPlayerOwner(PlayerIndex);
	if ( GearPlayerOwner != None )
	{
		Result = GearPlayerOwner.GetTeamNum();
	}

	return Result;
}


/**
 * Get the profile associated with the specified player.
 */
final function GearProfileSettings GetPlayerProfile( optional int PlayerIndex=INDEX_NONE )
{
	local GearProfileSettings Result;
	local GearPC GPC;

	GPC = GetGearPlayerOwner(PlayerIndex);
	if ( GPC != None )
	{
		Result = GPC.ProfileSettings;
	}

	return Result;
}

/**
 * @return	a reference to the current GameReplicationInfo object.
 */
final function GearGRI GetGRI()
{
	local WorldInfo WI;
	local GearGRI Result;

	WI = GetWorldInfo();
	if ( WI != None )
	{
		Result = GearGRI(WI.GRI);
	}

	return Result;
}

/**
 * @return	a reference to the current GameReplicationInfo object, if it's a GearPreGameGRI type.
 */
final function GearPreGameGRI GetPreGameGRI()
{
	return GearPreGameGRI(GetGRI());
}

/**
 * @return	a reference to the current gameinfo; might be none if we are a client
 */
final function GearGame GetCurrentGameInfo()
{
	local WorldInfo WI;
	local GearGame Result;

	WI = GetWorldInfo();
	if ( WI != None )
	{
		Result = GearGame(WI.Game);
	}

	return Result;
}

/**
 * Wrapper for getting a reference to the current gameinfo casted to GearMenuGame
 */
final function GearMenuGame GetMenuGameInfo()
{
	return GearMenuGame(GetCurrentGameInfo());
}

/**
 * Wrapper for getting a reference to the owning player's PRI casted to GearPartyPRI
 */
final function GearPartyPRI GetPartyPRI( int PlayerIndex=`PRIMARY_PLAYER_INDEX )
{
	local GearPC GearPO;
	local GearPartyPRI PRI;

	GearPO = GetGearPlayerOwner(PlayerIndex);
	if ( GearPO != None )
	{
		PRI = GearPartyPRI(GearPO.PlayerReplicationInfo);
	}

	return PRI;
}

/** Returns the MP game type being played */
function EGearMPTypes GetMPGameType()
{
	local GearGRI GGRI;
	GGRI = GetGRI();
	return static.GetMPGameTypeStatic(GGRI);
}

/** Returns the MP game type being played */
static function EGearMPTypes GetMPGameTypeStatic(GearGRI GGRI)
{
	local name ClassName;

	if ( GGRI == None )
	{
		return eGEARMP_Warzone;
	}

	ClassName = GGRI.GameClass.Name;

	if ( ClassName == 'GearGameKTL' )
	{
		return eGEARMP_KTL;
	}
	else if ( ClassName == 'GearGameHorde' )
	{
		return eGEARMP_CombatTrials;
	}
	else if ( ClassName == 'GearGameAnnex' )
	{
		return (GGRI.bAnnexIsKOTHRules ? eGEARMP_KOTH : eGEARMP_Annex);
	}
	else if ( ClassName == 'GearGameWingman' )
	{
		return eGEARMP_Wingman;
	}
	else if ( ClassName == 'GearGameCTM' )
	{
		return eGEARMP_CTM;
	}
	else
	{
		return (GGRI.bGameIsExecutionRules ? eGEARMP_Execution : eGEARMP_Warzone);
	}
}

/** Gets the variable name of the game type in question for finding the localized string */
final function string GetGameModeVariableString( EGearMPTypes MPType )
{
	switch ( MPType )
	{
		case eGEARMP_Execution:		return "Execution";
		case eGEARMP_KTL:           return "KTL";
		case eGEARMP_CombatTrials:	return "CT";
		case eGEARMP_Annex:         return "Annex";
		case eGEARMP_Wingman:       return "Wingman";
		case eGEARMP_KOTH:          return "KOTH";
		case eGEARMP_CTM:           return "CTM";
		default:					return "Warzone";
	}
}

/** Returns the localized string for the game type */
final function string GetGameModeString( EGearMPTypes MPType )
{
	local string ReturnString;

//	GetDataStoreStringValue("<GameResources:GameTypes;" $ GetGameModeVariableString(MPType) $ ".GameName", ReturnString, Self, GetPlayerOwner());
//	`log("###########" @ `location@`showenum(EGearMPTypes,MPType)@`showvar(ReturnString) @ "##############");

	ReturnString = Localize( "GearUIScene_Base", GetGameModeVariableString(MPType), "GearGame" );
	return ReturnString;
}

/**
 * Wrapper for getting the friendly name of a gametype, given the name of the class.
 */
static final event string GetGametypeFriendlyName( string GameName )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue Value;
	local int ProviderIndex;
	local string Result;

	Result = GameName;

	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		Value.PropertyTag = 'ClassName';
		Value.PropertyType = DATATYPE_Property;
		Value.StringValue = "GearGameContent." $ GameName;

		ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('GameTypes', 'ClassName', Value);
		if ( GameResourceDS.GetProviderFieldValue('GameTypes', 'GameName', ProviderIndex, Value) )
		{
			Result = Value.StringValue;
		}
	}

	return Result;
}

/** Returns the localized string for the map name */
static final event string GetMapNameString( String MapName )
{
	local GearGameMapSummary MapProvider;

	if (MapName == "MP_Training")
	{
		MapName = "MP_DayOne";
	}

	MapProvider = class'GearUIDataStore_GameResource'.static.GetMapSummaryFromMapName(MapName);
	if (MapProvider != none)
	{
		return MapProvider.DisplayName;
	}
	return "";
}

/**
 * Wrapper for checking whether this scene can create new players.
 */
function bool IsAllowedToCreatePlayers()
{
	local UIScene NextScene;
	local GearUIScene_Base SceneToCheck;
	local bool bResult;

	if ( bAllowPlayerJoin )
	{
		bResult = true;
		for ( NextScene = GetPreviousScene(false); NextScene != None; NextScene = NextScene.GetPreviousScene(false) )
		{
			SceneToCheck = GearUIScene_Base(NextScene);
			if ( SceneToCheck != None && !SceneToCheck.bAllowPlayerJoin )
			{
				bResult = false;
				break;
			}
		}
	}

	return bResult;
}

/**
 * Wrapper for checking whether a change in sign-in status should return the user to the main menu.
 */
function bool IsSigninChangeAllowed()
{
	local UIScene NextScene;
	local GearUIScene_Base SceneToCheck;
	local bool bResult;

	if ( bAllowSigninChanges )
	{
		bResult = true;
		for ( NextScene = GetPreviousScene(false); NextScene != None; NextScene = NextScene.GetPreviousScene(false) )
		{
			SceneToCheck = GearUIScene_Base(NextScene);
			if ( SceneToCheck != None && !SceneToCheck.bAllowSigninChanges )
			{
				bResult = false;
				break;
			}
		}
	}

	return bResult;
}

/**
 * Wrapper for checking whether the user with the specified controller is using an xbox live guest account.
 */
static final function bool IsGuestAccount( int ControllerId )
{
	local OnlineSubsystem OnlineSub;
	local bool bResult;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None && OnlineSub.PlayerInterface != None )
	{
		bResult = OnlineSub.PlayerInterface.IsGuestLogin(ControllerId);
	}

	return bResult;
}

/**
 * @return	a reference to the CurrentGame data store
 */
static final function CurrentGameDataStore GetCurrentGameDS()
{
	return CurrentGameDataStore(StaticResolveDataStore(class'CurrentGameDataStore'.default.Tag));
}

/**
 * @return	a reference to the gear-specific online game settings data store.
 */
static final function GearUIDataStore_GameSettings GetGameSettingsDataStore()
{
	return GearUIDataStore_GameSettings(StaticResolveDataStore(class'GearUIDataStore_GameSettings'.default.Tag));
}

/**
 * @return	a reference to the gear-specific online game settings data store for coop.
 */
static final function GearUIDataStore_CoopGameSettings GetCoopGameSettingsDataStore()
{
	return GearUIDataStore_CoopGameSettings(StaticResolveDataStore(class'GearUIDataStore_CoopGameSettings'.default.Tag));
}

/** Retrieves the coop game settings */
final function GearCoopGameSettings GetCoopGameSettings()
{
	local GearUIDataStore_CoopGameSettings GameSettingsDS;
	local OnlineGameSettings Result;

	GameSettingsDS = GetCoopGameSettingsDataStore();
	if ( GameSettingsDS != None )
	{
		Result = GameSettingsDS.GetCurrentGameSettings();
	}

	if ( Result != None )
	{
		return GearCoopGameSettings(Result);
	}
	return None;
}

/**
 * @return	a reference to the gear-specific menu items data store.
 */
static final function UIDataStore_MenuItems GetMenuItemsDataStore()
{
	return UIDataStore_MenuItems(StaticResolveDataStore(class'UIDataStore_MenuItems'.default.Tag));
}

/**
 * @return	a reference to the gear-specific static game resource data store.
 */
static final function GearUIDataStore_GameResource GetGameResourceDataStore()
{
	return GearUIDataStore_GameResource(StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
}

/**
 * @return	a reference to the dynamic game resource data store.
 */
static final function UIDataStore_DynamicResource GetDynamicResourceDataStore( LocalPlayer OwningPlayer )
{
	return UIDataStore_DynamicResource(StaticResolveDataStore(class'UIDataStore_DynamicResource'.default.Tag, None, OwningPlayer));
}

/**
 * @return	a reference to the screenshots data provider.
 */
static final function GearUIDataProvider_Screenshots GetScreenshotsDataProvider( LocalPlayer OwningPlayer )
{
	local GearUIDataStore_OnlinePlayerData OnlinePlayerDS;
	local GearUIDataProvider_Screenshots Result;

	OnlinePlayerDS = GearUIDataStore_OnlinePlayerData(StaticResolveDataStore('OnlinePlayerData', None, OwningPlayer));
	if ( OnlinePlayerDS != None  )
	{
		Result = OnlinePlayerDS.ScreenshotsProvider;
	}
	return Result;
}

/** Coverts the input from the controller to the input names the game expects and passes that to the GearPlayerInput */
final function PassInputToGame( InputEventParameters EventParms )
{
	local GearPC MyGearPC;
	local Name ButtonName;
	local GearPlayerInput GameInput;

	MyGearPC = GetGearPlayerOwner();
	if ( (MyGearPC != None) && (MyGearPC.PlayerInput != None) )
	{
		switch ( EventParms.InputKeyName )
		{
			case 'XboxTypeS_A':
				ButtonName = 'A';
				break;
			case 'XboxTypeS_X':
				ButtonName = 'X';
				break;
			case 'XboxTypeS_Y':
				ButtonName = 'Y';
				break;
			case 'XboxTypeS_B':
				ButtonName = 'B';
				break;
			case 'XboxTypeS_Start':
				ButtonName = 'Start';
				break;
			case 'XboxTypeS_Back':
				ButtonName = 'Back';
				break;
			case 'XboxTypeS_LeftTrigger':
				ButtonName = 'L2';
				break;
			case 'XboxTypeS_LeftShoulder':
				ButtonName = 'L1';
				break;
			case 'XboxTypeS_RightTrigger':
				ButtonName = 'R2';
				break;
			case 'XboxTypeS_RightShoulder':
				ButtonName = 'R1';
				break;
			case 'XboxTypeS_DPad_Up':
				ButtonName = 'DPad_Up';
				break;
			case 'XboxTypeS_DPad_Left':
				ButtonName = 'DPad_Left';
				break;
			case 'XboxTypeS_DPad_Down':
				ButtonName = 'DPad_Down';
				break;
			case 'XboxTypeS_DPad_Right':
				ButtonName = 'DPad_Right';
				break;
			case 'XboxTypeS_LeftThumbstick':
				ButtonName = 'L3';
				break;
			case 'XboxTypeS_RightThumbstick':
				ButtonName = 'R3';
				break;
		}

		if ( ButtonName != '' )
		{
			GameInput = GearPlayerInput(MyGearPC.PlayerInput);
			if ( GameInput != None )
			{
				if ( EventParms.EventType == IE_Pressed )
				{
					GameInput.ButtonPress( ButtonName );
				}
				else if ( EventParms.EventType == IE_Released )
				{
					GameInput.ButtonRelease( ButtonName );
				}
			}
		}
	}
}

/**
 * Hook to allow child classes to prevent the game info's online delegate subscription from being cleared when a travel
 * is pending.
 *
 * @return	TRUE if the game info's online delegates should be forcibly cleared.
 */
function bool ShouldCleanupGameDelegateSubscriptions()
{
	return true;
}

/**
 * Wrapper for displaying a movie while the game thread is blocked for loading.  All params are passed through to GearPC.ClientShowLoadingMovie().
 */
static final function ShowLoadingMovie( bool bShowMovie, optional bool bPauseAfterHide, optional float PauseDuration )
{
	local GearPC PC;
	local LocalPlayer LP;

	LP = class'UIInteraction'.static.GetLocalPlayer(`PRIMARY_PLAYER_INDEX);
	if ( LP != None )
	{
		PC = GearPC(LP.Actor);
		if ( PC != None )
		{
			PC.ClientShowLoadingMovie(bShowMovie, bPauseAfterhide, PauseDuration);
		}
		else
		{
			`warn("GearUIScene_Base::ShowLoadingMovie - player is None or not a GearPC!");
		}
	}
	else
	{
		`warn("GearUIScene_Base::ShowLoadingMovie - no valid players!");
	}
}

/**
 * Opens a generic scene that blocks input until StopUpdate is called and the scene closes
 *
 * @return	a reference to an instance of the scene that displays the spinning cog wheel and blocks input.
 */
static function GearUISceneFE_Updating OpenUpdatingScene()
{
	return class'GearGameUISceneClient'.static.OpenUpdatingScene();
}

/** Closes the generic scene that blocks input from OpenUpdatingScene */
static function CloseUpdatingScene()
{
	class'GearGameUISceneClient'.static.CloseUpdatingScene();
}

/**
 * Displays a message box for transitions between lobbies.
 *
 * @param	MessageKey				a key from the MessageBoxStrings section of the
 *									GearGameUI.int to use for the message text.
 * @param	bClearExistingMessages	specify TRUE to close all existing message boxes before displaying this error
 */
static final function DisplayLoadingScreen( string MessageKey, optional LocalPlayer OwningPlayer, optional bool bClearExistingLoadingScenes=true )
{
	local GameUISceneClient GameSceneClient;
	local array<name> Unused;
	local int ExistingLoadingSceneIndex;
	local UIMessageBoxBase LoadingScene;

	Unused.Length = 0;	// to trick compiler
	GameSceneClient = GetSceneClient();

	ExistingLoadingSceneIndex = GameSceneClient.FindSceneIndexByTag('LoadingScene', OwningPlayer);
	if ( ExistingLoadingSceneIndex != INDEX_NONE )
	{
		if ( bClearExistingLoadingScenes )
		{
			GameSceneClient.CloseSceneAtIndex(ExistingLoadingSceneIndex, false);
		}
		else
		{
			return;
		}
	}

	GameSceneClient.ShowUIMessage('LoadingScene', "<Strings:GearGameUI.MessageBoxStrings.LoadingTitle>",
		"<Strings:GearGameUI.MessageBoxStrings." $ MessageKey $ ">", "", Unused, None, OwningPlayer, LoadingScene, class'UIMessageBoxBase'.default.SceneStackPriority + 10);

	if ( LoadingScene != None )
	{
		LoadingScene.bExemptFromAutoClose = true;
	}
}

/**
 * Wrapper for displaying a message box containing an error message.
 *
 * @param	ErrorSceneTag			the tag to use for the error message.
 * @param	TitleKey				a key from the MessageBoxErrorStrings section of the GearGameUI.int to use as the message title.  If not
 *									specified, the generic error string will be used.
 * @param	MessageKey				a key from the MessageBoxErrorStrings section of the GearGameUI.int to use for the message text.
 * @param	MessageKey				a key from the MessageBoxErrorStrings section of the GearGameUI.int to use for the question text.
 * @param	ButtonAliasString		a comma-delimited string containing the buttonbar aliases to use in the message box.  The order of the
 *									button aliases should be in reverse order (i.e. the button that should be farthest right should be first)
 * @param	OwningPlayer			the player to associate with the message box (makes it so that the message box can only be dismissed by that
 *									player)
 * @param	SelectionCallback		specifies the function that should be called when the player selects an option.
 * @param	ForcedScenePriority		overrides the message box scene's default stack priority
 */
static final function UIMessageBoxBase DisplayErrorMessageWithQuestion( name ErrorSceneTag, string TitleKey, string MessageKey, string QuestionKey, string ButtonAliasString, optional LocalPlayer OwningPlayer,
										optional delegate<UIMessageBoxBase.OnOptionSelected> SelectionCallback, optional byte ForcedScenePriority )
{
	local GameUISceneClient GameSceneClient;
	local int i;
	local UIScene ExistingScene;
	local array<name> ButtonAliases;
	local array<string> ButtonAliasStringArray;
	local UIMessageBoxBase ErrorMessageScene;

	ButtonAliasStringArray = SplitString(ButtonAliasString);
	for ( i = 0; i < ButtonAliasStringArray.Length; i++ )
	{
		ButtonAliases[i] = name(ButtonAliasStringArray[i]);
	}

	GameSceneClient = GetSceneClient();
	if ( ErrorSceneTag == '' )
	{
		ErrorSceneTag = 'ErrorMessage';
	}
	if ( TitleKey == "" )
	{
		TitleKey = "ErrorTitle";
	}

	ExistingScene = GameSceneClient.FindSceneByTag(ErrorSceneTag, OwningPlayer);
	if ( ExistingScene == None )
	{
		ExistingScene = GameSceneClient.CreateUIMessageBox(ErrorSceneTag);
	}

	if ( ExistingScene != None )
	{
		ExistingScene.SceneTag = ErrorSceneTag;
		ExistingScene.bPauseGameWhileActive = true;
		ExistingScene.bExemptFromAutoClose = true;
		ExistingScene.bSaveSceneValuesOnClose = false;
		ExistingScene.bFlushPlayerInput = true;
		ExistingScene.bCaptureMatchedInput = true;
		ExistingScene.bCloseOnLevelChange = false;
		ExistingScene.bRenderParentScenes = true;

		ExistingScene = ExistingScene.OpenScene(ExistingScene, OwningPlayer, ForcedScenePriority);
		if ( ExistingScene != None )
		{
			ErrorMessageScene = UIMessageBoxBase(ExistingScene);
			ErrorMessageScene.SetupMessageBox(
				"<Strings:GearGameUI.MessageBoxErrorStrings." $ TitleKey $ ">",
				"<Strings:GearGameUI.MessageBoxErrorStrings." $ MessageKey $ ">",
				"<Strings:GearGameUI.MessageBoxErrorStrings." $ QuestionKey $ ">",
				ButtonAliases, SelectionCallback);
		}
	}

	return ErrorMessageScene;
}

/**
 * Wrapper for displaying a message box containing an error message.
 *
 * @param	MessageKey				a key from the MessageBoxStrings section of the GearGameUI.int to use for the message text.
 * @param	TitleKey				a key from the MessageBoxErrorStrings section of the GearGameUI.int to use as the message title.  If not
 *									specified, the generic error string will be used.
 * @param	ButtonAliasString		a comma-delimited string containing the buttonbar aliases to use in the message box.  The order of the
 *									button aliases should be in reverse order (i.e. the button that should be farthest right should be first)
 * @param	OwningPlayer			the player to associate with the message box (makes it so that the message box can only be dismissed by that
 *									player
 * @param	SelectionCallback		specifies the function that should be called when the player selects an option.
 * @param	bClearExistingMessages	specify TRUE to close all existing message boxes before displaying this error
 */
static final function UIMessageBoxBase DisplayErrorMessage( string MessageKey, optional string TitleKey, optional string ButtonAliasString, optional LocalPlayer OwningPlayer,
										optional delegate<UIMessageBoxBase.OnOptionSelected> SelectionCallback, optional bool bClearExistingMessages, optional byte ForcedScenePriority )
{
	local GameUISceneClient GameSceneClient;
	local int i;
	local UIMessageBoxBase MessageScene;
	local name ErrorSceneTag;
	local array<name> ButtonAliases;
	local array<string> ButtonAliasStringArray;
	local UIMessageBoxBase ErrorMessageScene;

	ButtonAliasStringArray = SplitString(ButtonAliasString);
	for ( i = 0; i < ButtonAliasStringArray.Length; i++ )
	{
		ButtonAliases[i] = name(ButtonAliasStringArray[i]);
	}

	GameSceneClient = GetSceneClient();

	if ( bClearExistingMessages )
	{
		foreach GameSceneClient.AllActiveScenes(class'UIMessageBoxBase', MessageScene, true)
		{
			MessageScene.CloseScene(MessageScene, false, true);
		}
	}

	if ( TitleKey == "" )
	{
		TitleKey = "ErrorTitle";
		ErrorSceneTag = 'ErrorMessage';
	}
	else
	{
		ErrorSceneTag = name(TitleKey);
	}

	GameSceneClient.ShowUIMessage(ErrorSceneTag, "<Strings:GearGameUI.MessageBoxErrorStrings." $ TitleKey $ ">",
		"<Strings:GearGameUI.MessageBoxErrorStrings." $ MessageKey $ ">", "", ButtonAliases, SelectionCallback, OwningPlayer, ErrorMessageScene, ForcedScenePriority);

	if ( ErrorMessageScene != None )
	{
		ErrorMessageScene.bExemptFromAutoClose = true;
		ErrorMessageScene.bCloseOnLevelChange = false;
		ErrorMessageScene.bRenderParentScenes = true;
	}

	return ErrorMessageScene;
}

/**
 * Saves the profile and updates the player controller with the new settings
 * @todo - move to scene client or uicontroller
 */
function bool SaveProfile( int PlayerIndex, optional delegate<OnlinePlayerInterface.OnWriteProfileSettingsComplete> WriteProfileSettingsCompleteDelegate=OnProfileWriteComplete )
{
	local GearPC GearPO;
	local LocalPlayer LP;
	local bool bResult;

	LP = GetPlayerOwner(PlayerIndex);
	if ( LP != None )
	{
		GearPO = GearPC(LP.Actor);
		if ( GearPO != None )
		{
			// always hook into the profile write.
			GearPO.SaveProfile(WriteProfileSettingsCompleteDelegate);
			GearPO.UploadProfileToMCP();
			bResult = true;
		}
	}

	return bResult;
}

/** Called when the profile is done writing */
function OnProfileWriteComplete(byte LocalUserNum, bool bWasSuccessful)
{
	local OnlinePlayerInterface PlayerInt;

	PlayerInt = GetOnlinePlayerInterface();
	if ( PlayerInt != None )
	{
		PlayerInt.ClearWriteProfileSettingsCompleteDelegate(LocalUserNum,OnProfileWriteComplete);
	}
}

final function bool IsProfileModeEnabled( int PlayerIndex )
{
	local GearProfileSettings Profile;
	local int Value;
	local bool bResult;

	Profile = GetPlayerProfile(PlayerIndex);
	if ( Profile != None )
	{
		bResult = Profile.GetProfileSettingValueInt(Profile.ProfileMode, Value) && Value != 0;
	}

	return bResult;
}

/**
 * Returns the string of a formatted time
 *
 * @param TimeInSeconds - time to format in seconds
 *
 * @return - formatted string in the form HH:MM
 */
final function String CreateTimeString( int TimeInSeconds )
{
	local int CurrTime, NumMinutes, NumHours;
	local String TimeString;

	CurrTime = TimeInSeconds;
	NumHours = CurrTime / 3600;
	CurrTime -= NumHours * 3600;
	NumMinutes = CurrTime / 60;
	CurrTime -= NumMinutes * 60;
	//NumSeconds = CurrTime;

	if ( NumHours <= 0 )
	{
		TimeString = "00:";
	}
	else if ( NumHours < 10 )
	{
		TimeString = "0"$NumHours$":";
	}
	else
	{
		TimeString = NumHours$":";
	}

	if ( NumMinutes <= 0 )
	{
		TimeString $= "00";
	}
	else if ( NumMinutes < 10 )
	{
		TimeString $= "0"$NumMinutes;
	}
	else
	{
		TimeString $= NumMinutes;
	}

// 	if ( NumSeconds <= 0 )
// 	{
// 		TimeString $= TimeString$"00";
// 	}
// 	else if ( NumSeconds < 10 )
// 	{
// 		TimeString $= TimeString$"0"$NumSeconds;
// 	}
// 	else
// 	{
// 		TimeString $= TimeString$NumSeconds;
// 	}

	return TimeString;
}

/**
 * Returns the string of a formatted date
 *
 * @return - formatted string in the form MM/DD/YY
 */
final function String CreateDateString( int Days, int Months, int Year )
{
	local String DateString;

	Year -= 2000;

	if ( Months < 10 )
	{
		DateString = "0"$Months$"/";
	}
	else
	{
		DateString = Months$"/";
	}

	if ( Days < 10 )
	{
		DateString $= "0"$Days$"/";
	}
	else
	{
		DateString $= Days$"/";
	}

	if ( Year < 10 )
	{
		DateString $= "0"$Year;
	}
	else
	{
		DateString $= Year;
	}

	return DateString;
}

/**
 * Adds the markup for a navigation icon to a string
 *
 * @return	TRUE if the output string contains navigation markup.
 */
final function bool AddNavigationIconMarkup( EGearNavigationIconType IconType, out string Markup )
{
	local string NavMarkup;
	local bool bResult;

	switch ( IconType )
	{
		case eGEARNAVICON_NewStuff:
			NavMarkup = `NavigationMarkup_NewStuff;
			break;

		case eGEARNAVICON_Complete:
			NavMarkup = `NavigationMarkup_Complete;
			break;
	}

	if ( NavMarkup != "" )
	{
		if ( InStr(Markup, NavMarkup) == INDEX_NONE )
		{
			Markup @= NavMarkup;
		}

		bResult = true;
	}

	return bResult;
}

/** Clears the markup for a navigation icon from a string */
final function ClearNavigationIconMarkup( EGearNavigationIconType IconType, out string Markup )
{
	local string NavMarkup;

	switch ( IconType )
	{
		case eGEARNAVICON_NewStuff:
			NavMarkup = `NavigationMarkup_NewStuff;
			break;

		case eGEARNAVICON_Complete:
			NavMarkup = `NavigationMarkup_Complete;
			break;
	}

	Markup = Repl( Markup, NavMarkup, "" );
}

/** Adds a navigation icon to the string of a UILabelButton */
final function AddNavigationIconMarkupToLabelButton( EGearNavigationIconType IconType, UILabelButton LabelButton )
{
	local string Markup;

	if ( LabelButton != None )
	{
		Markup = LabelButton.GetDataStoreBinding();
		AddNavigationIconMarkup( IconType, Markup );

		LabelButton.SetDataStoreBinding( Markup );
	}
}

/** Clears a navigation icon from the string of a UILabelButton */
final function ClearNavigationIconMarkupToLabelButton( EGearNavigationIconType IconType, UILabelButton LabelButton )
{
	local string Markup;

	if ( LabelButton != None )
	{
		Markup = LabelButton.GetDataStoreBinding();
		ClearNavigationIconMarkup( IconType, Markup );

		LabelButton.SetDataStoreBinding( Markup );
	}
}

/** Converts the default COG id to the index in the provider */
function int ConvertCharacterIdToProviderId( int CharacterId, Name ProviderTag )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue CharacterValue;
	local int ProviderId, TeamIndex;

	ProviderId = -1;
	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		CharacterValue.PropertyTag = 'ProfileId';
		CharacterValue.PropertyType = DATATYPE_Property;
		CharacterValue.ArrayValue.AddItem(CharacterId);
		TeamIndex = (ProviderTag == 'COGs') ? 0 : 1;
		CharacterValue.StringValue = string(GetEnum(GameResourceDS.GetTeamCharacterProviderIdType(TeamIndex), CharacterId));
		ProviderId = GameResourceDS.FindProviderIndexByFieldValue(ProviderTag, 'ProfileId', CharacterValue);
	}

	return ProviderId;
}

/** Converts the default weapon id to the index in the provider */
function int ConvertWeaponIdToProviderId( int WeaponId )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue WeaponValue;
	local int ProviderId;

	ProviderId = -1;
	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		WeaponValue.PropertyTag = 'WeaponId';
		WeaponValue.PropertyType = DATATYPE_Property;
		WeaponValue.StringValue = string(WeaponId);
		ProviderId = GameResourceDS.FindProviderIndexByFieldValue('Weapons', 'WeaponId', WeaponValue);
	}

	return ProviderId;
}

/**
 * Returns the string of the gear icon based on the playerskill value passed in
 *
 * @param PlayerSkill - the skill value of the player
 * @param bIncludeFontMarkup - will attach the font markup to the rank string
 */
function string GetPlayerSkillString( int PlayerSkill, optional bool bIncludeFontMarkup )
{
	local string SkillString;
	local int NormalizedSkill;

	if ( PlayerSkill > 0 )
	{
		NormalizedSkill = Clamp(PlayerSkill / 10, 0, 4);
		SkillString = bIncludeFontMarkup
			? "<Styles:Xbox360Font>" $ Chr(Asc("D") + NormalizedSkill) $ "<Styles:/>"
			: Chr(Asc("D") + NormalizedSkill);
	}

	return SkillString;
}

/**
 * Returns the string of the gear icon based on the local playerindex value passed in
 *
 * @param ControllerId - the index of the controller for this player
 * @param bIncludeFontMarkup - will attach the font markup to the controller string
 */
static function string GetControllerIconString( int ControllerId, optional bool bIncludeFontMarkup )
{
	local string ControllerString;

	// INDEX_NONE will map to a controller icon that has no quadrants highlighted
	if ( ControllerId >= INDEX_NONE && ControllerId <= 3 )
	{
		ControllerString = bIncludeFontMarkup
			? "<Styles:Xbox360Font>" $ Chr(Asc("Þ") + ControllerId) $ "<Styles:/>"
			: Chr(Asc("Þ") + ControllerId);
	}

	return ControllerString;
}

/**
 * Sets a talking delegate so we can display chat icons
 *
 * @param bTrack - Whether to Start or Stop the tracking
 */
function TrackChat( bool bTrack )
{
	local OnlineSubSystem OnlineSub;

	OnlineSub = class'GameEngine'.static.GetOnlineSubSystem();
	if ( OnlineSub != none && OnLineSub.VoiceInterface != none )
	{
		if ( bTrack )
		{
			OnlineSub.VoiceInterface.AddPlayerTalkingDelegate(OnPlayerTalking);
		}
		else
		{
			OnlineSub.VoiceInterface.ClearPlayerTalkingDelegate(OnPlayerTalking);
		}
	}
}

/**
 * Callback for when a player has talked.
 *
 * @param TalkingPlayer		UniqueNetId for the player that just talked.
 */
function OnPlayerTalking(UniqueNetId TalkingPlayer)
{
	Local GearPRI TalkerPRI;
	TalkerPRI = ResolveUniqueNetID(TalkingPlayer);
	if ( TalkerPRI != none )
	{
		TalkerPRI.ChatFadeValue = 1.0;
		TalkerPRI.TaccomChatFadeStart = GetWorldInfo().TimeSeconds;
		TalkerPRI.ChatFadeTime = 1.5;
	}
}

/**
 * Finds the PRI for the player with the NetId passed in.
 *
 * @param TestID		ID to search for.
 *
 * @return a PRI for the player with the provided NetId.
 */
function GearPRI ResolveUniqueNetID(UniqueNetId TestID)
{
	local GearGRI GRI;
	local int i;

	GRI = GearGRI(GetWorldInfo().GRI);
	if (GRI != none)
	{
		for (i=0;i<GRI.PRIArray.Length;i++)
		{
			if (GRI.PRIArray[i] != none && GRI.PRIArray[i].UniqueId == TestID)
			{
				return GearPRI(GRI.PRIArray[i]);
			}
		}
	}
	return none;
}

/**
 * Callback which provides a way to prevent the scene client from closing this scene (e.g. for performing a closing animation).
 * Handler for this scene's OnQueryCloseSceneAllowed delegate.  If this scene has a closing animation setup, prevents the scene client
 * from closing the scene immediately, so that the closing animation can run.
 *
 * @param	SceneToDeactivate	the scene that will be deactivated.
 * @param	bForcedClose		indicates whether the caller wished to force the scene to close.  Generally, if this parameter is false you
 *								should allow the scene to be closed so as not to interfere with garbage collection.
 *
 * @return	TRUE to allow the scene to be closed immediately.  FALSE to prevent the scene client from closing the scene.
 */
function bool UsesCloseSceneAnimation( UIScene SceneToDeactivate, bool bCloseChildScenes, bool bForcedClose )
{
	local bool bIsPerformingCloseAnimation;

	if ( !bForcedClose && SceneToDeactivate == Self )
	{
		bIsPerformingCloseAnimation = BeginSceneCloseAnimation(bCloseChildScenes);
	}

	return !bIsPerformingCloseAnimation;
}

/* === UIScene interface === */
/**
 * Callback for retrieving the widget that provides hints about which item is currently focused.
 *
 * @param	bQueryOnly	specify TRUE to indicate that a focus hint should not be created if it doesn't already exist.
 *
 * @return	the widget used for displaying the global focused control hint.
 */
event UIObject GetFocusHint( optional bool bQueryOnly )
{
	if ( !bQueryOnly && SelectionHintLabel == None && !IsEditor() )
	{
		SelectionHintLabel = UIFocusHint(CreateWidget(Self, class'Engine.UIFocusHint', None, 'lblSelectionHint'));
		InsertChild(SelectionHintLabel);

		SelectionHintLabel.SetWidgetStyleByName(SelectionHintLabel.StringRenderComponent.StyleResolverTag, 'cmb_ButtonLabelsXbox');
		SelectionHintLabel.SetDataStoreBinding("<Strings:GearGameUI.ControllerFont.Controller_A>");
		SelectionHintLabel.StringRenderComponent.EnableAutoSizing(UIORIENT_Horizontal);
		SelectionHintLabel.StringRenderComponent.EnableAutoSizing(UIORIENT_Vertical);
		SelectionHintLabel.SetTextAlignment(UIALIGN_Default, UIALIGN_Center);
		SelectionHintLabel.SetVisibility(false);
	}

	return SelectionHintLabel;
}

/**
 * Called when the local player is about to travel to a new URL.  This callback should be used to perform any preparation
 * tasks, such as updating status text and such.  All cleanup should be done from NotifyGameSessionEnded, as that function
 * will be called in some cases where NotifyClientTravel is not.
 *
 * @param	TravelURL		a string containing the mapname (or IP address) to travel to, along with option key/value pairs
 * @param	TravelType		indicates whether the player will clear previously added URL options or not.
 * @param	bIsSeamless		indicates whether seamless travelling will be used.
 */
function NotifyPreClientTravel( string TravelURL, ETravelType TravelType, bool bIsSeamless )
{
	local GearMenuGame MenuGI;

	Super.NotifyPreClientTravel(TravelURL, TravelType, bIsSeamless);

	if ( ShouldCleanupGameDelegateSubscriptions() )
	{
		MenuGI = GetMenuGameInfo();
		if ( MenuGI != None )
		{
			// instruct the menu
			MenuGI.ClearOnlineDelegates();
		}
	}
}

/**
 * Called when a gamepad (or other controller) is inserted or removed)
 *
 * @param	ControllerId	the id of the gamepad that was added/removed.
 * @param	bConnected		indicates the new status of the gamepad.
 */
function NotifyControllerStatusChanged( int ControllerId, bool bConnected )
{
	local int i;
	local array<bool> Overrides;

	if ( IsSceneActive(true) )
	{
		for ( i = 0; i < MAX_SUPPORTED_GAMEPADS; i++ )
		{
			if ( i == ControllerId )
			{
				Overrides[i] = bConnected;
			}
			else
			{
				Overrides[i] = IsGamepadConnected(i);
			}
		}

//		`log(`location @ `showvar(ControllerId) @ `showvar(bConnected) @ `showvar(IsSceneActive(true),TopmostScene) @
//			"(" $ Overrides[0] $ "," $ Overrides[1] $ "," $ Overrides[2] $ "," $ Overrides[3] $ ")",,'RON_DEBUG');

		UpdateScenePlayerState(Overrides);
	}

	RequestSceneInputMaskUpdate();
	Super.NotifyControllerStatusChanged(ControllerId, bConnected);
}

/**
 * Notification that the login status a player has changed.
 *
 * @param	ControllerId	the id of the gamepad for the player that changed login status
 * @param	NewStatus		the value for the player's current login status
 *
 * @return	TRUE if this scene wishes to handle the event and prevent further processing.
 */
function bool NotifyLoginStatusChanged( int ControllerId, ELoginStatus NewStatus )
{
	local UIScene ParentScene;
	local bool bResult;

`if(`isdefined(FIXING_SIGNIN_ISSUES))
`log(">>>>" @ `location @ `showvar(ControllerId) @ `showvar(NewStatus) @ `showvar(IsSigninChangeAllowed()));
ScriptTrace();
`endif

	if ( !IsSigninChangeAllowed() && ProcessIllegalPlayerLoginStatusChange(ControllerId,,NewStatus) )
	{
		bResult = true;
	}
	else if ( NewStatus == LS_NotLoggedIn || NewStatus == LS_UsingLocalProfile )
	{
		ParentScene = GetPreviousScene(false, true);
		if ((bRequiresOnlineService || (bRequiresProfile && NewStatus == LS_NotLoggedIn))
		&&	(PlayerOwner == None || PlayerOwner.ControllerId == ControllerId) )
		{
			//@todo - should we always force the scene closed without allowing it to perform closing animations?
			// seems like we'd only want to do this if the this is not the last scene being closed.  but then again,
			// we will usually be displaying a message box or something to notify the user that the network status changed
			// so perhaps it's best to skip all animations
			CloseScene( Self, true, /*ParentScene == None || !ParentScene.bRequiresOnlineService*/true );
			bResult = true;
		}

		if ( ParentScene != None && ParentScene.NotifyLoginStatusChanged(ControllerId, NewStatus) )
		{
			bResult = true;
		}
	}
	else
	{
		bResult = Super.NotifyLoginStatusChanged(ControllerId, NewStatus);
	}

	if ( IsSceneActive(false) )
	{
		if ( !bResult && IsSceneActive(true) )
		{
			UpdateScenePlayerState();
		}

`if(`isdefined(FIXING_SIGNIN_ISSUES))
`log("<<<<" @ `location @ `showvar(ControllerId) @ `showvar(NewStatus) @ `showvar(IsSigninChangeAllowed()) @ `showvar(bResult) @ `showvar(IsSceneActive(true)));
`endif

		RequestSceneInputMaskUpdate();
	}

	return bResult;
}

/**
 * Handler for the OnOptionSelected delegate of the "too many profiles signed-in" error message.  Only allows the message box to be closed
 * if the number of signed in players is <= MAX_SPLITSCREEN_PLAYERS.
 *
 * @param	Sender				the message box that generated this call
 * @param	SelectedInputAlias	the alias of the button that the user selected.  Should match one of the aliases passed into
 *								this message box.
 * @param	PlayerIndex			the index of the player that selected the option.
 *
 * @return	TRUE to indicate that the message box should close itself.
 */
function bool OnTooManyProfilesErrorAccepted( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( GetLoggedInPlayerCount() <= `MAX_SPLITSCREEN_PLAYERS )
	{
		return true;
	}

	PlayUISound('GenericError');
	return false;
}

/**
 * Generic, light-weight function for updating the state of various widgets in the scene.
 *
 * @param	ControllerConnectionStatusOverrides		see UpdateProfileLabels.
 */
function UpdateScenePlayerState( optional array<bool> ControllerConnectionStatusOverrides )
{
	local int Idx, PlayerIndex, NumActiveProfiles, NumPlayers, ControllerId;
	local array<int> SignedInControllerIds, LiveEnabledProfileControllerIds;

	local GameUISceneClient GameSceneClient;
	local LocalPlayer LP;
	local string ErrorString;

	if ( !IsEditor() )
	{
		GameSceneClient = GetSceneClient();
		GetLoggedInControllerIds(SignedInControllerIds);
		GetLoggedInControllerIds(LiveEnabledProfileControllerIds, true);
		`assert(SignedInControllerIds.Length >= LiveEnabledProfileControllerIds.Length);

		// get the number of signed-in players
		NumActiveProfiles = SignedInControllerIds.Length;
		NumPlayers = GameSceneClient.GetPlayerCount();

		// if there are too many players signed in, show an error message
		if ( NumActiveProfiles > `MAX_SPLITSCREEN_PLAYERS )
		{
			// show an error message
			DisplayErrorMessage("TwoPlayersOnly_Message", "TwoPlayersOnly_Title", "GenericAccept", None,OnTooManyProfilesErrorAccepted,,100);
			return;
		}

		if ( IsAllowedToCreatePlayers() && class'WorldInfo'.static.IsMenuLevel() )
		{
			// for each signed in profile, make sure they have a player (creating one if necessary)
			for ( Idx = 0; Idx < NumActiveProfiles; Idx++ )
			{
				ControllerId = SignedInControllerIds[Idx];
				PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(ControllerId);

				// if no player has this controllerId, create one.
				if ( PlayerIndex == INDEX_NONE )
				{
					`Assert(NumPlayers < `MAX_SPLITSCREEN_PLAYERS);
					`log(`location @ "attempting to create a new local player -" @ `showvar(ControllerId));
					LP = GameSceneClient.CreatePlayer(ControllerId, ErrorString, true);
				}
			}

			// for each player, if they aren't signed in, remove them unless they're the last player
			NumPlayers = GameSceneClient.GetPlayerCount();
			for ( PlayerIndex = NumPlayers - 1; PlayerIndex >= 0 && NumPlayers > 1; PlayerIndex-- )
			{
				ControllerId = GameSceneClient.GetPlayerControllerId(PlayerIndex);
				`assert(ControllerId>=0);
				`assert(ControllerId<MAX_SUPPORTED_GAMEPADS);

				// if not in the list of signed-in gamepads, remove this player
				Idx = SignedInControllerIds.Find(ControllerId);
				if ( Idx == INDEX_NONE )
				{
					LP = GameSceneClient.GamePlayers[PlayerIndex];

`if(`isdefined(FIXING_SIGNIN_ISSUES))
					`if(`isdefined(dev_build))
					ScriptTrace();
					`endif
					`log(`location @ "attempting to remove a local player:" @ LP @ "at index" @ PlayerIndex @ "with ControllerId of" @ ControllerId,,'PlayerManagement');
`endif
					if ( LP != None && GameSceneClient.RemovePlayer(LP) )
					{
						NumPlayers--;
					}
				}
			}
		}
	}
}

/** Called just after this scene is removed from the active scenes array */
event SceneDeactivated()
{
	Super.SceneDeactivated();
}

/* === UIScreenObject interface == */
/**
 * Called after this screen object's children have been initialized
 * Overloaded look for a description label
 */
event PostInitialize()
{
	InitializeDescriptionSystem();

	Super.PostInitialize();
}

/**
 * Wrapper for setting a string style only if the style isn't already set.
 */
final function SetLabelStyle( UILabel Label, name StyleName )
{
	local UIStyle_Combo StringStyle;

	StringStyle = Label.StringRenderComponent.GetAppliedStringStyle();
	if (StringStyle == None
		||	UIStyle(StringStyle.Outer).StyleTag != StyleName)
	{
		Label.SetWidgetStyleByName('String Style', StyleName);
	}
}

/**
 * Wrapper for setting an image style only if the style isn't already set
 */
final function SetImageStyle( UIImage Image, name StyleName )
{
	local UIStyle_Image ImageStyle;

	ImageStyle = Image.ImageComponent.GetAppliedImageStyle();
	if (ImageStyle == None
		||	UIStyle(ImageStyle.Outer).StyleTag != StyleName)
	{
		Image.SetWidgetStyleByName('Image Style', StyleName);
	}
}

/**
 * Wrapper for setting a string style only if the style isn't already set.
 */
final function SetLabelButtonBGStyle( UILabelButton LabelButton, name StyleName )
{
	local UIStyle_Image ImageStyle;

	ImageStyle = LabelButton.BackgroundImageComponent.GetAppliedImageStyle();
	if (ImageStyle == None
		||	UIStyle(ImageStyle.Outer).StyleTag != StyleName)
	{
		LabelButton.SetWidgetStyleByName('Background Image Style', StyleName);
	}
}

/** Sets the value of a scene transition data */
static function SetTransitionValue(string VariableName, string NewValue)
{
	local string VarName;

	VarName = "<Registry:" $ VariableName $ ">";
	SetDataStoreStringValue(VarName, NewValue);
}

/** Gets the value of a scene transition data */
static function string GetTransitionValue(string VariableName)
{
	local string VarName;
	local string StringValue;

	VarName = "<Registry:" $ VariableName $ ">";
	GetDataStoreStringValue(VarName, StringValue);
	return StringValue;
}

/** Clears the value of a transition value */
static function ClearTransitionValue(string VariableName)
{
	local string VarName;

	VarName = "<Registry:" $ VariableName $ ">";
	SetDataStoreStringValue(VarName, "");
}

/** Clear all the main menu transition values */
static function ClearMainMenuTransitionValues()
{
	ClearTransitionValue("EnterSaveSlot");
	ClearTransitionValue("ExitSaveSlot");
	ClearTransitionValue("EnterDifficulty");
	ClearTransitionValue("CanSaveSlot");
	ClearTransitionValue("ExitDifficulty");
	ClearTransitionValue("DifficultyPlayer");
	ClearTransitionValue("SelectedAct");
	ClearTransitionValue("SelectedChapter");
	ClearTransitionValue("SelectedDifficulty");
	ClearTransitionValue("UseCheckpoint");
	ClearTransitionValue("CheckpointChapter");
	ClearTransitionValue("Campaign");
	ClearTransitionValue("CompletedTraining");
}

/** Clear all the main menu transition values */
static function ClearAllTransitionValues()
{
	ClearMainMenuTransitionValues();

	ClearTransitionValue("EnterLobby");
	ClearTransitionValue("ExitGame");
}

DefaultProperties
{
	NotifyActiveStateChanged=OnStateChanged
	SceneStackPriority=GEAR_SCENE_PRIORITY_NORMAL
	SceneRenderMode=SPLITRENDER_Fullscreen
	ScenePostProcessGroup=UIPostProcess_Dynamic
	bDisplayCursor=false
	bPauseGameWhileActive=false
	SceneSkin=UISkin'UI_Skin.GearsUI'
//	OnQueryCloseSceneAllowed=UsesCloseSceneAnimation
	CurrentBackgroundSettings=(bEnableBloom=true,bEnableDOF=true,bEnableMotionBlur=false,bEnableSceneEffect=false,bAllowAmbientOcclusion=false,DOF_BlurKernelSize=16,Bloom_Scale=0,Scene_Desaturation=0,Scene_Shadows=(X=0,Y=0,Z=0),Scene_HighLights=(X=2,Y=2,Z=2))
	CurrentForegroundSettings=(bEnableBloom=true,bEnableDOF=true,bEnableMotionBlur=false,bEnableSceneEffect=false,bAllowAmbientOcclusion=false,DOF_BlurKernelSize=16)

	SceneAnimation_Open=ActivateSceneSeq
	SceneAnimation_Close=DeactivateSceneSeq
//	SceneAnimation_LoseFocus=ForegroundPP_FadeInSeq
//	SceneAnimation_RegainingFocus=ForegroundPP_FadeOutSeq
//	SceneAnimation_RegainedFocus=ReactivateSceneSeq

	bAllowSigninChanges=true
	bAllowPlayerJoin=true

	bOpeningAnimationCompleted=false
}

