/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorialFTE_Execution extends GearTutorialFirstTimeEvent_Base
	config(Game);


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
		InputHandler.SetInputButtonHandle( GB_A, HandleButtonInput_MeatShield );
		InputHandler.SetInputButtonHandle( RemapGameButton(GB_X), HandleButtonInput_Execute );
		InputHandler.SetInputButtonHandle( RemapGameButton(GB_Y), HandleButtonInput_LongExecute );
		InputHandler.SetInputButtonHandle( GB_B, HandleButtonInput_QuickExecute );
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
	if ( ButtonName == InputHandler.InputButtonDataList[GB_A].ButtonNameMapping ||
		 ButtonName == InputHandler.InputButtonDataList[RemapGameButton(GB_X)].ButtonNameMapping ||
		 ButtonName == InputHandler.InputButtonDataList[RemapGameButton(GB_Y)].ButtonNameMapping ||
		 ButtonName == InputHandler.InputButtonDataList[GB_B].ButtonNameMapping )
	{
		Filtered = 0;
	}
	else
	{
		Filtered = 1;
	}

	return true;
}

/** Callback for meatshielding */
function HandleButtonInput_MeatShield(bool bPressed, optional bool bDblClickMove)
{
	if ( bPressed )
	{
		CloseTutorialUIScene();
		GearPlayerInput(InputHandler).HandleButtonInput_A(bPressed);
	}
}

/** Callback for executing */
function HandleButtonInput_Execute(bool bPressed, optional bool bDblClickMove)
{
	if ( bPressed )
	{
		CloseTutorialUIScene();
		GearPlayerInput(InputHandler).HandleButtonInput_X(bPressed);
	}
}

/** Callback for long executing */
function HandleButtonInput_LongExecute(bool bPressed, optional bool bDblClickMove)
{
	if ( bPressed )
	{
		CloseTutorialUIScene();
		GearPlayerInput(InputHandler).HandleButtonInput_Y(bPressed);
	}
}

/** Callback for quick executing */
function HandleButtonInput_QuickExecute(bool bPressed, optional bool bDblClickMove)
{
	if ( bPressed )
	{
		CloseTutorialUIScene();
		GearPlayerInput(InputHandler).HandleButtonInput_B(bPressed);
	}
}

defaultproperties
{
	TutorialType=GEARTUT_Executions
	EventDelegateType=eGED_Executions
	InputHandlerClass=class'GearPlayerInputTutorialPC'
}
