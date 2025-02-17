/**
 * GearTutorial_GenericPress_Base
 * 
 * This is a base class for any tutorial that wants to freeze the game and force
 * the player to press an input that will then close the screen, and then complete the tutorial
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_GenericPress_Base extends GearTutorial_Base
	config(Game)
	abstract;


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The GameButton used for this tutorial */
var EGameButtons GameButtonType;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
* Called when this tutorial's input handler is created (if this tutorial has it's own handler)
*		This is where a tutorial can add InputButtonData mappings to the handler.
*/
function InitializeInputHandler()
{
	// Must call super first so that there is button data to set
	Super.InitializeInputHandler();

	if ( InputHandler != None )
	{
		InputHandler.SetInputButtonHandle( RemapGameButton(GameButtonType), HandleButtonInput_CloseScene );
	}
}

/**
* Opportunity for the tutorial to override the filtering system for the input object handling this tutorial's input
*		@Param Filtered - whether this button should be filtered or not (non zero means true)
*		@Return - whether we've handled the filtering or not
*/
function bool FilterButtonInput( Name ButtonName, bool bPressed, int ButtonIdx, out int Filtered )
{
	// Have the tutorial's input handler handle this input
	if ( ButtonName == InputHandler.InputButtonDataList[RemapGameButton(GameButtonType)].ButtonNameMapping )
	{
		Filtered = 0;
	}
	else
	{
		Filtered = 1;
	}

	return true;
}

/** Callback for closing the UIScene */
function HandleButtonInput_CloseScene(bool bPressed, optional bool bDblClickMove)
{
	if ( bPressed )
	{
		CloseTutorialUIScene();
	}
}

/** Called when the tutorial's scene is finished closing */
function OnTutorialSceneClosed( UIScene DeactivatedScene )
{
	Super.OnTutorialSceneClosed( DeactivatedScene );

	OnTutorialCompleted();
}

defaultproperties
{
	InputHandlerClass=class'GearPlayerInputTutorialPC'
}
