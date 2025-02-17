/**
 * This scene is used for blocking input and keeping the user engaged while some action is performed.  It contains a material which
 * displays an animated cog wheel that is ticked by the render thread (thus not inhibited by blocking actions in the game thread).
 *
 * This scene remains open until closed by the caller.  It can be configured to remain open a minimum amount of time - if this value is
 * non-zero, the scene will not close until it has been open MinDisplayTime seconds even if the caller requests the scene to close by calling
 * StopUpdate().
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneFE_Updating extends GearUIScene_Base;

/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Reference to the title label */
var transient				UILabel		TitleLabel;

/** Reference to the message label */
var transient				UILabel		MessageLabel;

/** indicates that the scene should be closed as soon as it's been open the minimum amount of time */
var	transient	protected	bool		bCloseRequested;

/** Amount of time (in seconds) that this scene has been open or was last refreshed */
var transient	protected	float		TimeSinceOpen;

/** The minimum amount of time the scene must be open */
var transient	protected	float		MinDisplayTime;

/* The variable key used in GearGameUI.int in the [UpdatingStrings] for looking up the title string */
var 			protected	string		UpdateTitleKey;

/* The variable key used in GearGameUI.int in the [UpdatingStrings] for looking up the description string */
var				protected	string		UpdateDescriptionKey;

/* == Delegates == */

/* == UnrealScript == */
/** Initialize references to widgets */
function InitializeWidgetReferences()
{
	local UIPanel			Panel;

	Panel = UIPanel(FindChild('pnlUpdating', true));
	Panel.SetVisibility(true);

	TitleLabel = UILabel(Panel.FindChild('lblUpdating', true));
	MessageLabel = UILabel(Panel.FindChild('lblMessage', true));


	// make sure the other one is hidden
	Panel = UIPanel(FindChild('pnlCheckpoint',true));
	Panel.SetVisibility(false);
}

function RepositionForContentSaveWarning()
{
	local UIPanel			Panel;

	SetSceneInputMode(INPUTMODE_None);
	bFlushPlayerInput=false;

	Panel = UIPanel(FindChild('pnlCheckpoint',true));
	Panel.SetVisibility(true);

	TitleLabel = UILabel(Panel.FindChild('lblCheckpoint', true));
	MessageLabel = UILabel(Panel.FindChild('lblMessage2', true));

	Panel = UIPanel(FindChild('pnlUpdating', true));
	Panel.SetVisibility(false);
}

/**
 * Sets the scene for how we want to display the update
 *
 * TitleKey - the variable key used in GearGameUI.int in the [UpdatingStrings] for looking up the title string
 * DescriptionKey - the variable key used in GearGameUI.int in the [UpdatingStrings] for looking up the description string
 * MinTime - the minimum amount of time in seconds this scene will be displaying
 */
function InitializeUpdatingScene(optional string TitleKey, optional string DescriptionKey, optional float MinTime=default.MinDisplayTime, optional bool bRestartUpdate)
{
	UpdateTitleKey = TitleKey;
	UpdateDescriptionKey = DescriptionKey;
	RefreshUpdateStrings();

	MinDisplayTime = MinTime;

	if ( bRestartUpdate )
	{
		RestartUpdate();
	}
}

/** Set the updating strings */
protected function RefreshUpdateStrings()
{
	local string StringToShow;

	// Title
	if (UpdateTitleKey != "")
	{
		StringToShow = Localize("UpdatingStrings", UpdateTitleKey, "GearGameUI");
	}
	else
	{
		StringToShow = "";
	}
	TitleLabel.SetDataStoreBinding(StringToShow);

	// Description
	if (UpdateDescriptionKey != "")
	{
		StringToShow = Localize("UpdatingStrings", UpdateDescriptionKey, "GearGameUI");
	}
	else
	{
		StringToShow = "";
	}

	MessageLabel.SetDataStoreBinding(StringToShow);
}

/**
 * Wrapper for hooking up the scene's tick delegate handler.  This ensures that we are incrementing the counter that tells us when we
 * are allowed to close.
 */
function StartUpdate()
{
	if ( !IsEditor() )
	{
		OnGearUISceneTick = UpdateSceneCounter;
	}
}

/**
 * Resets the scene's counter back to zero and reverses any closing animations.
 */
function RestartUpdate()
{
	// clear the flag indicating that we want to be closed...
	bCloseRequested = false;

	// reset the counter back to zero
	TimeSinceOpen = 0;

	// make sure our delegate is hooked up or we won't have a counter...
	StartUpdate();

	if ( SceneAnimation_Close != '' && IsAnimating(SceneAnimation_Close) )
	{
		// if we don't have an opening animation,
		StopSceneAnimation(SceneAnimation_Close, false);
		if ( SceneAnimation_Open != '' )
		{
			//note that we do not reset TimeRemaining.
			BeginSceneOpenAnimation();
			return;
		}
		else
		{
			// the opening animation currently only interpolates opacity - if we don't have an opening animation, set opacity
			// to 1 since we were performing a closing animation.
			Opacity = 1;
		}
	}
}

/** Stops the updating scene (makes it close as once the min time is up) */
function StopUpdate()
{
	bCloseRequested = true;
}

/* == Delegate handlers == */
/** Update the widget styles, strings, etc. */
protected function UpdateSceneCounter( float DeltaTime )
{
	TimeSinceOpen += DeltaTime;

	if ( TimeSinceOpen > MinDisplayTime
	&&	(SceneAnimation_Open == '' || !IsAnimating(SceneAnimation_Open)) )
	{
		if ( bCloseRequested )
		{
			// unhook the delegate so we don't try to close the scene more than once.
			OnGearUISceneTick = None;

			`log("Closing" @ SceneTag @ "scene after" @ TimeSinceOpen @ "seconds");

			// and close the scene
			CloseScene(self);
		}
	}
}

/** === UIScreenObject interface === */
/**
 * Called after this screen object's children have been initialized
 * Overloaded to set the deactivated callback
 */
event PostInitialize()
{
	// Initialize references to widgets
	InitializeWidgetReferences();

	Super.PostInitialize();
}

defaultproperties
{
	SceneSkin=None
	OnGearUISceneTick=UpdateSceneCounter
	bNeverFocus=true
	bAllowPlayerJoin=false

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object

	SceneStackPriority=GEAR_SCENE_PRIORITY_BLOCKASYNC
	MinDisplayTime=1.0f
}
