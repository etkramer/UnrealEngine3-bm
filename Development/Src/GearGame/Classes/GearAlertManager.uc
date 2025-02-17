/**
 * GearAlertManager
 *
 * Keeps track of how/when the player should be notified about their progress
 * with regard to achievements, collectibles, etc...
 *
 * When appropriate notifies the player that they have made progress.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAlertManager extends Object
	config(Game)
	transient;

const ModuleName = "[GearAlertManager] ";

/** The GearPC to which this GearAlertManager belongs. */
var transient GearPC MyGearPC;

/** The UI Scene which actually displays the popups. */
var transient GearUIScene_Toast ToastScene;

const MAX_FREQUENCIES = 10;
// - - - - - - - Config stuff - - - - - - -
/**
 * Stores data about how to notify the player about various progresses
 *  e.g. How often to notify about kill-total based acheivements
 */
struct ProgressInfo
{
	/** Which achievement */
	var EGearAchievement GoalId;
	/** Every time that we have achieved something N times and N is a multiple of Every, we should show a progress notification. If Every is -1 we should always show progress. */
	var int Every;
	/** Whenever we have achieved something N tmies, and N is in the At array we should show a progress notification */
	var int At[MAX_FREQUENCIES];
};

/** Config how to notify about achievement progress */
var config transient array<ProgressInfo> AchievementProgressInfo;


/** The types of alerts that can be sent */
enum EAlertType
{
	eALERT_FriendAlert,
	eALERT_Progress,
	eALERT_Unlock,
	eALERT_Screenshot
};

/**
 * Represents an alert that the player will see.
 */
struct AlertEvent
{
	/** The kind of alert to show */
	var transient EAlertType Type;
	/** The title of this alert; should be an effective summary */
	var transient string Title;
	/** Optional custom text */
	var transient string SubText;
	/** The percentage to fill the progress bar; -1 to hide progress bar. */
	var transient float PercentComplete;
	/** An alternate icon to use instead of the default */
	var transient string	CustomIcon;
};

/**
 * GearAlertManager needs to know its GearPC owner in order to handle
 * achievement progress notification.
 *
 * @param InGearPC
 */
function InitGearAlertManager(GearPC InGearPC)
{
	MyGearPC = InGearPC;
}

/**
 * Initialize the Toast Alert Screen.
 */
function InitToastScreen()
{
	if (ToastScene == None)
	{
		ToastScene = OpenToastScene();
	}
}


/**
 * Queue in an alert
 *
 * @param Type	The type of alert to show.
 * @param Title	The title of an alert.
 * @param Summary The summary is only applicable to some alerts.
 * @param PercentComplete A float value between 0 and 1 for the progress bar; only applicable to some alerts.
 */
function Alert(EAlertType Type, optional string Title="", optional string Summary="", optional float PercentComplete = -1.0, optional string CustomIcon="")
{
	local AlertEvent EventToPush;

	EventToPush.Type = Type;
	EventToPush.Title = Title;
	EventToPush.SubText = Summary;
	EventToPush.PercentComplete = PercentComplete;
	EventToPush.CustomIcon = CustomIcon;

	ToastScene = OpenToastScene();
	ToastScene.PushAlert(EventToPush);
}

/**
 * Notify the alert manager that progress has been made; we may or may not queue an alert.
 *
 * @param Type	Type of progress: e.g. ePROG_Collectible, ePROG_Achievement
 * @param GoalId	The specific goal towards which progress has been made; e.g. eGA_Seriously2
 */
function MadeProgress( EProgressType Type, int GoalId )
{
	local AlertEvent EventToPush;
	local AchievementDetails AchievDetails;
	local float Current, Max;
	local GearUIDataStore_GameResource GameResourceDS;
	local string DataDumpster; //Throw away data
	/** Guests do not get any achievements, unlocks or progress */
	local bool bIsGuest;

	if ( MyGearPC.OnlineSub != None )
	{
		bIsGuest = MyGearPC.OnlineSub.PlayerInterface.IsGuestLogin( MyGearPC.GetControllerId() );
	}

	if ( !bIsGuest )
	{
		switch( Type )
		{
			case ePROG_Achievement:
				if ( !class'UIInteraction'.static.IsLoggedIn( MyGearPC.GetControllerId() ) )
				{
					// is user isn't signed into a profile, they cannot recieve achievements so don't show achievement progression
					return;
				}

				EventToPush.Type = eALERT_Progress;
				MyGearPC.OnlinePlayerData.AchievementsProvider.GetAchievementDetails(GoalId, AchievDetails);
				MyGearPC.GetAchievementProgression(GoalId, Current, Max);
				//`log( "GearAlertManager: " $ string(GoalId) $ " : " $ string(Current) $ " / " $ string(Max) );
				EventToPush.Title = AchievDetails.AchievementName;
				// Set the achievement's icon
				EventToPush.CustomIcon=MyGearPC.OnlinePlayerData.AchievementsProvider.GetAchievementIconPathName(GoalId);

				if ( int(Max) == 0 )
				{
					`warn( "GearAlertManager: THIS SHOULD NEVER HAPPEN! Showing because max is 0 : " @ string(GoalId) @ "(" $ string(Current) $ "/" $ string(Max) $ ")" );
					EventToPush.SubText = "Max == 0";
					EventToPush.PercentComplete = -1;
				}
				else
				{
					if ( !ShouldShowProgressionAlert( EGearAchievement(GoalId), int(Current), int(Max) ) )
					{
						// We don't need to actually show a progress alert.
						//`log( "GearAlertManager: Not showing: " @ string(GoalId) @ "(" $ string(Current) $ "/" $ string(Max) $ ")" );
						return;
					}

					EventToPush.SubText = string( int(Current) ) $ "/" $ string( int(Max) );
					EventToPush.PercentComplete = Current/Max;
				}
				break;
			case ePROG_Collectible:
				EventToPush.Type = eALERT_Unlock;
				GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
				GameResourceDS.GetDiscoverableDisplayStrings( EGearDiscoverableType(GoalId), EventToPush.Title, DataDumpster );
				break;
			case ePROG_Unlockable:
				EventToPush.Type = eALERT_Unlock;
				GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
				GameResourceDS.GetUnlockableDisplayStrings(EGearUnlockable(GoalId),eventToPush.SubText, DataDumpster);
				EventToPush.Title = Localize("AlertStrings", "UnlockTitle", "GearGameUI");
				break;
			default:
				`warn("GearAlertManager: Unsupported Progress Event Type.");
				break;
		}


		//`Log("Achievement Alert: " $ EventToPush.Type $ ", " $ EventToPush.Title $ ", " $ EventToPush.SubText $ ", " $ EventToPush.PercentComplete $ ", " $ EventToPush.CustomIcon $ ".")

		ToastScene = OpenToastScene();
		ToastScene.PushAlert(EventToPush);
	}
}


/**
 * Checks the Frequencies for Achievement Progress Alerts and decides if this step
 * in the progression warrants an on-screen Alert.
 *
 * Meaning of the array:
 * The 'At' array should contain positive numbers in ascending order
 * followed by negative number in ascending order.
 *   e.g. {10, 20, 100, -100, -20, -5}
 * Positive numbers mean notify when the progress equals that number;
 * in the example above notify at 10, 20, 100, and 1000.
 * Negative numbers mean notify when the progress is MAX-abs(number);
 * if we're dealing with Seriously2 which required 100,000 kills we would
 * trigger at 99,900, at 99,980 and at 99,995.
 *
 * Every time Current is a multiple of 'Every' we also display a notification.
 *
 * @param GoalId	An EGearAchievement that player made progress in.
 * @param Current	The current progress value e.g. 20 kills or 5 executions
 * @param Max		The goal number to be achieved; e.g. 100,000 kills for Seriously2
 *
 * @return true if we this progression warrants a notification; false otherwise
 */
function bool ShouldShowProgressionAlert( EGearAchievement GoalId, int Current, int Max )
{
	local int CurIdx;
	local int ProgInfoIdx;
	local ProgressInfo ProgInfo;


	// We should not alert if the achievement has been achieved because a Live popup will appear.
	if ( Current < Max )
	{
		// Look for the structure describing this achievement's notification frequency...
		ProgInfoIdx = AchievementProgressInfo.Find( 'GoalId', GoalId );
		if ( ProgInfoIdx < 0 )
		{
			// If we cannot find it then always show the notification.
			`log( "GearAlertManager: Could not find frequency info for achievement " $ string( GoalId ) $ ". Displaying notification." );
			return TRUE;
		}
		ProgInfo = AchievementProgressInfo[ProgInfoIdx];

		// If the Every field is set then check to see if we are currently on a multiple of it.
		if ( ProgInfo.Every != 0 && Current % ProgInfo.Every == 0 )
		{
			`log( "GearAlertManager: Achievement checkpoint for " $ string( GoalId ) $ " at " $ Current $ " / " $ Max $ ". (Every " $ ProgInfo.Every $ ".");
			return TRUE;
		}

		// Iterate through all the positive numbers
		for (CurIdx=0; CurIdx < MAX_FREQUENCIES; CurIdx++)
		{
			if ( ( Current > 0 && Current == ProgInfo.At[CurIdx] ) ||
				 ( Current < 0 && Max-Current == ProgInfo.At[CurIdx]) )
			{
				`log( "GearAlertManager: Achievement checkpoint for " $ string( GoalId ) $ " at " $ Current $ " / " $ Max $ ". Displaying notification." );
				return TRUE;
			}
		}

	}

	return FALSE;


}

/**
 * Opens or restarts the Toast scene (used for displaying the toast screne)
 *
 * @return the UIScene that was found or restarted
 */
function GearUIScene_Toast OpenToastScene()
{
	local GameUISceneClient GameSceneClient;
	local GearUIScene_Toast MyToastScene;

	/** The resource for our UI scene. */
	local GearUIScene_Toast ToastSceneResource;

	GameSceneClient = class'UIRoot'.static.GetSceneClient();
	if ( GameSceneClient != None )
	{
		MyToastScene = GearUIScene_Toast( GameSceneClient.FindSceneByTag('UI_HUD_Toast', LocalPlayer(MyGearPC.Player)) );
		if ( MyToastScene != None )
		{
			//`Log("!!!!!!!!!!!!!!!!!!!!!!!!! Found Scene " $ MyToastScene);
			return MyToastScene;
		}
		else
		{
			ToastSceneResource = GearUIScene_Toast(FindObject("UI_Scenes_Common.UI_HUD_Toast", class'UIScene'));
			if ( ToastSceneResource == None )
			{
				ToastSceneResource = GearUIScene_Toast(DynamicLoadObject("UI_Scenes_Common.UI_HUD_Toast", class'UIScene'));
			}

			// Open Scene returns a copy of the scene that we can actually use. Don't use ToastSceneResource!
			MyToastScene = GearUIScene_Toast( ToastSceneResource.OpenScene(ToastSceneResource, LocalPlayer(MyGearPC.Player), /*ForcedPriority*/, /*bSkipAnimation=*/true) );
			//`Log("!!!!!!!!!!!!!!!!!!!!!!!!! Openend Scene " $ MyToastScene $ " for player " $ MyGearPC.Player);
			return MyToastScene;
		}
	}
}

DefaultProperties
{
}
