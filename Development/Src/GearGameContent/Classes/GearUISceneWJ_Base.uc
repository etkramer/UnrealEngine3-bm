/**
 * Abstract base class for all War Journal scenes.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_Base extends GearUISceneFrontEnd_Base
	abstract;

/** the buttonbar that holds the back button */
var	transient	UICalloutButtonPanel		btnbarMain;

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	btnbarMain = UICalloutButtonPanel(FindChild('btnbarMain',true));
}

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	btnbarMain.SetButtonCallback('WarJournalTOC', CalloutButtonClicked);
}

/**
 * Instances, initializes, and activates a scene, replacing an existing scene's location in the scene stack.  The existing scene will be deactivated and no longer part
 * of the scene stack.  The location in the scene stack for the new scene instance may be modified if its SceneStackPriority requires the scene stack to be resorted.
 *
 * @param	SceneInstanceToReplace	the scene that should be replaced.
 * @param	SceneToOpen				the scene that will replace the existing scene.  If the scene specified is contained in a content package, the scene will be duplicated and
 *									the duplicate will be added instead.
 * @param	SceneOwner				the player that should be associated with the new scene.  Will be assigned to the scene's
 *									PlayerOwner property.
 * @param	OpenedScene				the scene that was actually opened.  If Scene is located in a content package, OpenedScene will be
 *									the copy of the scene that was created.  Otherwise, OpenedScene will be the same as the scene passed in.
 * @param	ForcedPriority			overrides the scene's SceneStackPriority value to allow callers to modify where the scene is placed in the stack.
 *
 * @return TRUE if the scene was successfully activated and inserted into the scene stack (although not necessarily at the DesiredSceneIndex)
 */
static final function bool ReplaceScene( UIScene SceneInstanceToReplace, UIScene SceneToOpen, optional LocalPlayer SceneOwner, optional out UIScene OpenedScene, optional byte ForcedPriority)
{
	local GameUISceneClient GameSceneClient;

	GameSceneClient = GetSceneClient();
	return GameSceneClient.ReplaceScene(SceneInstanceToReplace, SceneToOpen, SceneOwner, OpenedScene, ForcedPriority);
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Called when the key associated with the weapon swap or player options callout button is clicked.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool CalloutButtonClicked( UIScreenObject EventObject, int PlayerIndex )
{
	local GameUISceneClient GameSceneClient;
	local UICalloutButton Sender;
	local bool bResult;

	GameSceneClient = GetSceneClient();
	if ( GameSceneClient.GetActiveScene(GetPlayerOwner(PlayerIndex), true) == Self )
	{
		Sender = UICalloutButton(EventObject);
		if ( Sender != None && Sender.InputAliasTag != 'None' )
		{
			bResult = HandleCalloutButtonClick(Sender.InputAliasTag, PlayerIndex);
		}
	}

	return bResult;
}

/**
 * Worker for CalloutButtonClicked - only called once all conditions for handling the input have been met.  Child classes should
 * override this function rather than CalloutButtonClicked, unless additional constraints are necessary.
 *
 * @param	InputAliasTag	the callout input alias associated with the input that was received
 * @param	PlayerIndex		index for the player that generated the event
 *
 * @return	TRUE if this click was processed.
 */
function bool HandleCalloutButtonClick( name InputAliasTag, int PlayerIndex )
{
	local bool bResult;

	if ( InputAliasTag == 'WarJournalTOC' || InputAliasTag == 'CloseWindow' )
	{
		if ( CloseScene() )
		{
			//@todo ronp animation
			bResult = true;
		}
	}

	return bResult;
}

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

	SetupCallbacks();
}

DefaultProperties
{
	SceneSkin=UISkin'UI_Art_WarJournal.UI_WarJournal'
	bRequiresProfile=true
	bAllowSigninChanges=false

`if(`isdefined(WJ_CHAPTER_NAVIGATION_SUPPORTED))
//@todo ronp animations - disable these animations until we add animation support to 'replacescene'
	SceneAnimation_Open=None
	SceneAnimation_Close=None
	SceneAnimation_LoseFocus=None
	SceneAnimation_RegainingFocus=None
	SceneAnimation_RegainedFocus=None
`endif
}
