/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFE_Title extends GearUISceneFrontEnd_Base
	native(inherit);

cpptext
{
	/**
	 * This notification is sent to the topmost scene when a different scene is about to become the topmost scene.
	 * Provides scenes with a single location to perform any cleanup for its children.
	 *
	 * @param	NewTopScene		the scene that is about to become the topmost scene.
	 */
	virtual void NotifyTopSceneChanged( UUIScene* NewTopScene );

	/**
	 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
	 * once the scene has been completely initialized.
	 * For widgets added at runtime, called after the widget has been inserted into its parent's
	 * list of children.
	 *
	 * @param	inOwnerScene	the scene to add this widget to.
	 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
	 *							is being added to the scene's list of children.
	 */
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );
}

/**
 * Called once this screen object has been completely initialized, before it has activated its InitialState or called
 * Initialize on its children.  This event is only called the first time a widget is initialized.  If reparented, for
 * example, the widget would already be initialized so the Initialized event would not be called.
 */
event Initialized()
{
	local GearUIInteraction UIController;
	local string StringValue;

	Super.Initialized();

	if ( !IsEditor() )
	{
		if (!GetDataStoreStringValue("<Registry:HasDisplayedTitleScreen>", StringValue, Self, GetPlayerOwner())
		||	StringValue != "1" )
		{
			UIController = GearUIInteraction(GetCurrentUIController());
			if ( UIController != None )
			{
				UIController.bAttractModeAllowed = true;
			}
		}
	}
}

/* === UIScene interface === */
/**
 * Handler for the completion of this scene's opening animation...
 *
 * @warning - if you override this in a child class, keep in mind that this function will not be called if the scene has no opening animation.
 */
function OnOpenAnimationComplete( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	Super.OnOpenAnimationComplete(Sender, AnimName, TrackTypeMask);
	GetGearPlayerOwner().AlertManager.InitToastScreen();
}

/**
 * Called just after the scene is added to the ActiveScenes array, or when this scene has become the active scene as a result
 * of closing another scene.
 *
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
event SceneActivated( bool bInitialActivation )
{
	local string StringValue;

	// skip the GearUIScene_Base version
	Super(UIScene).SceneActivated(bInitialActivation);

	if ( bInitialActivation )
	{
		if (GetDataStoreStringValue("<Registry:HasDisplayedTitleScreen>", StringValue)
		&&	StringValue == "1")
		{
			// just force kill the opacity of this scene so that it doesn't flash when returning to main menu
			Opacity = 0;
		}
	}
}

/** Called just after this scene is removed from the active scenes array */
event SceneDeactivated()
{
	local GearUIInteraction UIController;

	Super.SceneDeactivated();

	if ( !IsEditor() )
	{
		UIController = GearUIInteraction(GetCurrentUIController());
		if ( UIController != None )
		{
			UIController.bAttractModeAllowed = false;
		}
	}
}

function UpdateSelectionHint( UIScreenObject Sender )
{
}

/**
 * Callback function when the scene gets input
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool MyProcessInput( out InputEventParameters EventParms )
{
	if ( EventParms.InputKeyName == 'XboxTypeS_Start' )
	{
		PlayUISound('G2UI_MenuPressStartCue');
	}

	return false;
}

defaultproperties
{
	SceneOpenedCue=None
	OnRawInputKey=MyProcessInput
	SceneAnimation_RegainingFocus=ReactivateSceneSeq
	SceneAnimation_LoseFocus=SceneLostFocusSeq
}

