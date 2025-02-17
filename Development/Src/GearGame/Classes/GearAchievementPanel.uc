/**
 * This panel displays information about a single achievement list element for the achievements scene in Gears 2.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearAchievementPanel extends GearWJPrefabPanelBase;

/** displays the number of gamer points the achievement is worth */
var	transient		UILabel					lblGamerPoints;

/** displays instructions for how to complete the achievement */
var	transient		UILabel					lblDescription;

/** this checkbox is enabled once the achievement has been completed */
var	transient		UICheckbox				chkCompleted;

/** displays a bar depicting the current ratio for completion of the achievement */
var	transient		UIProgressBar			barCurrentProgress;

/** the label that displays the exclamation point when the player has completed a new achievement */
var	transient		UILabel					lblAttractIcon;

/**
 * Index into the list of achievements for the achievement data being displayed by this panel.
 */
var	transient		int						AchievementIndex;

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Wrapper for checking whether the achievement has been completed.
 *
 */
final function bool IsAchievementCompleted()
{
	local UIProviderFieldValue FieldValue;
	local bool bResult;

	if ( GetDataStoreFieldValue("<OnlinePlayerData:Achievements;" $ AchievementIndex $ ".IsCompleted>", FieldValue, GetScene(), GetPlayerOwner()) )
	{
		bResult = bool(FieldValue.ArrayValue[0]);
	}
	else
	{
		`log("GearAchievementPanel::IsAchievementCompleted: failed to resolve data store markup for achievement" @ AchievementIndex @ "using markup: <OnlinePlayerData:Achievements;" $ AchievementIndex $ ".IsCompleted>");
	}

	return bResult;
}

/**
 * Wrapper for checking whether the collectable associated with this panel has been unlocked but never viewed.
 */
final function bool IsNewlyCompleted()
{
	local GearUIScene_Base GearOwnerScene;
	local GearProfileSettings Profile;
	local bool bResult;

	GearOwnerScene = GearUIScene_Base(GetScene());
	Profile = GearOwnerScene.GetPlayerProfile(GetPlayerOwnerIndex());
	bResult = Profile.IsAchievementMarkedForAttract(GetAchievementId());

	return bResult;
}

/**
 * Wrapper for checking whether the achievement is secret.
 *
 */
final function bool IsAchievementSecret()
{
	// !! We don't have any secret achievements in Gears2.
	// We want to see achievement details for testing purposes. !!
	return false;
//	local UIProviderFieldValue FieldValue;
//	local bool bResult;
//
//	if ( GetDataStoreFieldValue("<OnlinePlayerData:Achievements;" $ AchievementIndex $ ".bIsSecret>", FieldValue, GetScene(), GetPlayerOwner()) )
//	{
//		bResult = bool(FieldValue.ArrayValue[0]);
//		`log(`location @ `showvar(AchievementIndex) @ `showvar(FieldValue.StringValue) @ `showvar(FieldValue.ArrayValue[0]) @ `showvar(bResult));
//	}
//	else
//	{
//		`log("GearAchievementPanel::IsAchievementCompleted: failed to resolve data store markup for achievement" @ AchievementIndex @ "using markup: <OnlinePlayerData:Achievements;" $ AchievementIndex $ ".bIsSecret>");
//	}
//
//	return bResult;
}

/**
 * Wrapper for checking whether this achievement is one that has progress (i.e. perform task 100 times, etc.)
 */
final function bool IsProgressiveAchievement()
{
	local GearUIScene_Base GearOwnerScene;
	local GearProfileSettings Profile;
	local EGearAchievement AchievementId;
	local float CurrentValue, MaxValue;
	local bool bResult;

	GearOwnerScene = GearUIScene_Base(GetScene());
	Profile = GearOwnerScene.GetPlayerProfile(GetPlayerOwnerIndex());

	if ( Profile != None )
	{
		AchievementId = GetAchievementId();
		bResult = Profile.GetAchievementProgression(AchievementId, CurrentValue, MaxValue);
	}

	return bResult;
}

/**
 * Assigns an achievement to this panel.
 *
 * @param	NewAchievementIndex		index [into the achievement data provider's list of achievements] for the achievement this panel
 *									should display.
 *
 * @return	TRUE if this is the first time the user is viewing this achievement since completing it.
 */
function bool SetAchievementIndex( int NewAchievementIndex )
{
	local int PreviousAchievementIndex;
	local bool bFirstViewSinceCompletion;

	PreviousAchievementIndex = AchievementIndex;
	AchievementIndex = NewAchievementIndex;

	bFirstViewSinceCompletion = IsNewlyCompleted();
	RefreshAchievementData(NewAchievementIndex != INDEX_NONE && AchievementIndex == PreviousAchievementIndex);

	return bFirstViewSinceCompletion;
}

/**
 * Updates the data store bindings for all controls in this panel with the markup required to access the data for the associated achievement.
 *
 * @param	bRefreshAllowed	specify TRUE to force widgets to call RefreshSubscriberValue if the current data store markup is identical
 *							to the new data store markup for this panel's achievement index.
 */
function RefreshAchievementData( bool bRefreshAllowed )
{
	local bool bIsCompleted, bIsSecret;
	local string MarkupString, MarkupStringBase;
	local int PlayerIndex;

	if ( AchievementIndex != INDEX_NONE )
	{
		SetVisibility(true);

		MarkupStringBase = "<OnlinePlayerData:Achievements;" $ AchievementIndex $ ".";

		PlayerIndex = GetPlayerOwnerIndex();
		bIsCompleted = IsAchievementCompleted();
		bIsSecret = IsAchievementSecret();
		if ( bIsCompleted || !bIsSecret )
		{
			imgIcon.EnableWidget(PlayerIndex);
			imgIcon.SetValue(None);

			lblAttractIcon.SetVisibility(IsNewlyCompleted());

			MarkupString = MarkupStringBase $ "Image>";
			AssignAchievementData(imgIcon, MarkupString, bRefreshAllowed);
			if ( !bIsCompleted )
			{
				imgIcon.DisableWidget(GetPlayerOwnerIndex());
			}

			MarkupString = MarkupStringBase $ "AchievementName>";
			AssignAchievementData(lblName, MarkupString, bRefreshAllowed);

			MarkupString = MarkupStringBase $ "GamerPoints> <Strings:GearGameUI.FrontEndStrings.GamerPointsAbbrev>";
			AssignAchievementData(lblGamerPoints, MarkupString, bRefreshAllowed);

			MarkupString = MarkupStringBase $ "ConditionalDescription>";
			AssignAchievementData(lblDescription, MarkupString, bRefreshAllowed);

			MarkupString = MarkupStringBase $ "IsCompleted>";
			AssignAchievementData(chkCompleted, MarkupString, bRefreshAllowed);

			if ( IsProgressiveAchievement() )
			{
				barCurrentProgress.SetVisibility(true);
				MarkupString = MarkupStringBase $ "ProgressRatio>";
				AssignAchievementData(barCurrentProgress, MarkupString, bRefreshAllowed);
				SetDockTarget(UIFACE_Bottom, barCurrentProgress, UIFACE_Bottom);
			}
			else
			{
				barCurrentProgress.SetVisibility(false);
				SetDockTarget(UIFACE_Bottom, lblDescription, UIFACE_Bottom);
			}

			MarkAchievementAsViewed();
		}
		else
		{
			imgIcon.DisableWidget(PlayerIndex);

			lblAttractIcon.SetVisibility(false);
			lblName.SetDataStoreBinding("<Strings:GearGameUI.FrontEndStrings.LockedAchievementName>");
			lblGamerPoints.SetDataStoreBinding("-- <Strings:GearGameUI.FrontEndStrings.GamerPointsAbbrev>");
			lblDescription.SetDataStoreBinding("<Strings:GearGameUI.FrontEndStrings.LockedAchievementDescription>");
			chkCompleted.SetDataStoreBinding("");
			chkCompleted.SetValue(false, PlayerIndex);

			barCurrentProgress.SetVisibility(false);
		}
	}
	else
	{
		SetVisibility(false);
	}
}

protected function EGearAchievement GetAchievementId()
{
	local string StringValue;
	local EGearAchievement Result;

	if ( GetDataStoreStringValue("<OnlinePlayerData:Achievements;" $ AchievementIndex $ ".Id>", StringValue, GetScene(), GetPlayerOwner()) )
	{
		Result = EGearAchievement(int(StringValue));
	}

	return Result;
}

/**
 * Wrapper for marking the collectable associated with this panel as having been viewed.
 */
function MarkAchievementAsViewed()
{
	local EGearAchievement AchievementId;
	local GearUIScene_Base GearOwnerScene;
	local GearProfileSettings Profile;
//	local GearPC PC;

	if ( IsNewlyCompleted() )
	{
		AchievementId = GetAchievementId();

		GearOwnerScene = GearUIScene_Base(GetScene());
		Profile = GearOwnerScene.GetPlayerProfile(GetPlayerOwnerIndex());

		if ( Profile != None && Profile.MarkAchievementAsViewed(AchievementId) )
		{
//			lblAttractIcon.SetVisibility(false);
//			@todo ronp - flag the profile to be saved
//			PC = GearOwnerScene.GetGearPlayerOwner(GetPlayerOwnerIndex());
//			if ( PC != None )
//			{
//				PC.SaveProfile();
//			}
		}
	}
}

/**
 * Applies achievement data to a widget in this panel, refreshing the widget's displayed value if necessary.
 *
 * @param	Subscriber			the widget to assign the markup to
 * @param	MarkupString		a string containing the data store markup representing the data that the widget should present.
 * @param	bRefreshAllowed		indicates whether RefreshSubscriberValue should be called if the widget's current data store markup is
 *								identical to the new value.
 */
protected function AssignAchievementData( UIDataStoreSubscriber Subscriber, const out string MarkupString, bool bRefreshAllowed )
{
	if ( bRefreshAllowed && MarkupString == Subscriber.GetDataStoreBinding() )
	{
		Subscriber.RefreshSubscriberValue();
	}
	else
	{
		Subscriber.SetDataStoreBinding(MarkupString);
	}
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Handler for this widget's OnProcessInputKey delegate.  Responsible for displaying this achievement's details when the correct
 * button is pressed.
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called AFTER kismet is given a chance to process the input, but BEFORE any native code processes the input.
 *
 * @param	EventParms	information about the input event, including the name of the input alias associated with the
 *						current key name (Tab, Space, etc.), event type (Pressed, Released, etc.) and modifier keys (Ctrl, Alt)
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool ProcessInputKey( const out SubscribedInputEventParameters EventParms )
{
	local bool bResult;
	local OnlinePlayerInterfaceEx PlayerIntEx;

	if ( EventParms.InputAliasName == 'ViewWJItemDetails' )
	{
		if ( EventParms.ControllerId != 255 && EventParms.EventType == IE_Released && AchievementIndex != INDEX_NONE )
		{
			PlayerIntEx = GetOnlinePlayerInterfaceEx();
			PlayerIntEx.ShowAchievementsUI(EventParms.ControllerId);
		}

		bResult = true;
	}

	return bResult || Super.ProcessInputKey(EventParms);
}

/* === GearWJPanelBase interface === */
/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	Super.InitializeWidgetReferences();

	// achievement panel's selection bar has a slightly different name.
	imgSelectionBar = UIImage(FindChild('imgSelection',true));

	lblGamerPoints = UILabel(FindChild('lblGamerPoints',true));
	lblDescription = UILabel(FindChild('lblDescription',true));
	lblAttractIcon = UILabel(FindChild('lblAttractIcon',true));

	chkCompleted = UICheckbox(FindChild('chkCompleted',true));
	barCurrentProgress = UIProgressBar(FindChild('barCurrentProgress',true));
}


/**
 * Wrapper for checking whether the child widget's enabled state should be toggled when the selection changes.
 */
function bool ShouldToggleWhenSelected( UIObject Child )
{
	return Child != imgIcon && Super.ShouldToggleWhenSelected(Child);
}

DefaultProperties
{
	WidgetTag=AchievementPanel
	AchievementIndex=INDEX_NONE
}
