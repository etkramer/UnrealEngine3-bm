/**
 * GearTutorial_PCAction_Base
 * 
 * This is a base class for any tutorial that wants to freeze the game and force
 * the player to press an input that will then close the screen, complete the tutorial,
 * and then have the input propogated to the playercontroller to perform the action.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_PCAction_Base extends GearTutorial_GenericPress_Base
	config(Game)
	abstract;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Callback for closing the UIScene */
function HandleButtonInput_CloseScene(bool bPressed, optional bool bDblClickMove)
{
	if ( bPressed )
	{
		CloseTutorialUIScene();
		PerformPCInputAction( bPressed );

		// We need to unfreeze input and remove the input-handler from the system early so the game can
		// receive release messages.
		ToggleFreezeGameplay(false);
		TutorialMgr.RemoveInputHandlerFromSystem();
	}
}

/** Action to perform on the player when this tutorial is completed - implemented by all subclasses */
function PerformPCInputAction( bool bPressed );
