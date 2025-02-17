/**
 * GearPlayerInput_Base
 * Gear player input post processing base
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPlayerInput_Base extends PlayerInput within GearPC
	config(Input)
	native
	abstract;

/************************************************************************/
/* Member variables, structs, enums, etc.                               */
/************************************************************************/

/** Struct to encapsulate all data needed for a button input - indexed by EGameButtons */
struct native InputButtonData
{
	/** Name of the input */
	var Name ButtonNameMapping;
	/** Delegate to call when this input is pressed */
	var delegate<HandleButtonInput> ButtonHandler;
	/** Whether a button is pressed, after gameplay filtering */
	var byte Active;
	/** Whether a button is pressed, before gameplay filtering */
	var byte RawActive;
};

/** List of InputButtonData for mapping function handlers to input. */
var array<InputButtonData> InputButtonDataList;

/** Whether to check for kismet input events to activate or not. */
var const bool bActivateKismetInputEvents;


/**
 * Template for the handle called when button is pressed/released, used to consolidate all functionality for a button into a single function.
 * @param bDblClickMove - for function to do logic based on whether a double-tap move is being called or not
 */
delegate HandleButtonInput(bool bPressed, optional bool bDblClickMove)
{
}

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Initializes the input object */
function OnInputObjectCreate()
{
	InitializeButtonHandling();
}

/** Initializes the button handling list */
function InitializeButtonHandling()
{
	// Set the size...
	InputButtonDataList.length = GB_Max;

	// Set the button names...
	InputButtonDataList[GB_A].ButtonNameMapping					= 'A';
	InputButtonDataList[GB_X].ButtonNameMapping					= 'X';
	InputButtonDataList[GB_Y].ButtonNameMapping					= 'Y';
	InputButtonDataList[GB_B].ButtonNameMapping					= 'B';
	InputButtonDataList[GB_Start].ButtonNameMapping				= 'Start';
	InputButtonDataList[GB_Back].ButtonNameMapping				= 'Back';
	InputButtonDataList[GB_LeftTrigger].ButtonNameMapping		= 'L2';
	InputButtonDataList[GB_LeftBumper].ButtonNameMapping		= 'L1';
	InputButtonDataList[GB_RightTrigger].ButtonNameMapping		= 'R2';
	InputButtonDataList[GB_RightBumper].ButtonNameMapping		= 'R1';
	InputButtonDataList[GB_DPad_Up].ButtonNameMapping			= 'DPad_Up';
	InputButtonDataList[GB_DPad_Left].ButtonNameMapping			= 'DPad_Left';
	InputButtonDataList[GB_DPad_Down].ButtonNameMapping			= 'DPad_Down';
	InputButtonDataList[GB_DPad_Right].ButtonNameMapping		= 'DPad_Right';
	InputButtonDataList[GB_LeftStick_Push].ButtonNameMapping	= 'L3';
	InputButtonDataList[GB_RightStick_Push].ButtonNameMapping	= 'R3';

	// Set the input handlers
	InitializeButtonHandlingHandlers();
}

/** Sets the input handler for a particular button */
final function SetInputButtonHandle( EGameButtons GBType, delegate<HandleButtonInput> Handler )
{
	InputButtonDataList[GBType].ButtonHandler = Handler;
}

/** Sets the input handlers for the button handling list - set by subclasses */
event InitializeButtonHandlingHandlers();

/** Copies the raw values from one input object to this one, and will either wipe or clear the active values and PressedKey array */
function InitializeActiveButtonsFromInputObject( GearPlayerInput_Base InputObject, bool bWipeInputState )
{
	local int Idx;

	// Clears the pressed keys array for this input object
	ResetInput();

	// If there was a valid previous input object
	if ( InputObject != None )
	{
		for ( Idx = 0; Idx < InputButtonDataList.length; Idx++ )
		{
			// Copy the raw
			InputButtonDataList[Idx].RawActive = InputObject.InputButtonDataList[Idx].RawActive;

			// Wipe the active buttons
			if ( bWipeInputState )
			{
				InputButtonDataList[Idx].Active = 0;
			}
			// Copy the active values
			else
			{
				InputButtonDataList[Idx].Active = InputObject.InputButtonDataList[Idx].Active;
			}
		}

		// Copy the PressedKeys array
		if ( !bWipeInputState )
		{
			for ( Idx = 0; Idx < InputObject.PressedKeys.length; Idx++ )
			{
				PressedKeys.AddItem( InputObject.PressedKeys[Idx] );
			}
		}
	}
	// No input object
	else
	{
		for ( Idx = 0; Idx < InputButtonDataList.length; Idx++ )
		{
			// Wipe the raw values
			InputButtonDataList[Idx].RawActive = 0;

			// Wipe the active values
			InputButtonDataList[Idx].Active = 0;
		}
	}
}

/**
 * Called when this input object is pushed onto the input stack and will now be the first in line.
 *		@param bWipePrevInputState - determines whether this input object will copy the previous input object's state or wipe it
 */
function PushedOnTopOfInputStack( GearPlayerInput_Base PrevInputObject, bool bWipePrevInputState )
{
	InitializeActiveButtonsFromInputObject( PrevInputObject, bWipePrevInputState );
}

/** Called when another input object is pushed on top of this one and will now become second in line. */
function PushedBackOnInputStack();

/** Called when the top most input object is popped off of the stack and now this input object will become first in line. */
function PoppedToTopOfInputStack( GearPlayerInput_Base PrevInputObject, bool bWipePrevInputState )
{
	InitializeActiveButtonsFromInputObject( PrevInputObject, bWipePrevInputState );
}

/**
 * Called when this input object is popped off of the stack.
 *		@param bWipePrevInputState - determines whether this input object will copy the previous input object's state or wipe it
 */
function PoppedOffOfInputStack();

/**
 * Disable the input buttons
 *	@param bIsPOI - if the POI system is having us disable the input, do not disable the button responsible for POI
 */
final function DisableButtonInput( optional bool bIsPOI )
{
	local int Idx;
	for (Idx = 0; Idx < InputButtonDataList.length; Idx++)
	{
		if ( bIsPOI )
		{
			if ( Outer.bUseAlternateControls )
			{
				if (Idx == GB_RightStick_Push )
				{
					continue;
				}
			}
			else if (Idx == GB_Y)
			{
				continue;
			}
		}

		if ( IsButtonActive(EGameButtons(Idx)) )
		{
			ForceButtonRelease( EGameButtons(Idx) );
		}
		else if ( IsButtonActive(EGameButtons(Idx), TRUE) )
		{
			InputButtonDataList[Idx].RawActive = 0;
		}
	}
}

/** Enable the input buttons */
final function EnableButtonInput()
{
	local int Idx;

	// set the active set to the raw set
	for (Idx = 0; Idx < InputButtonDataList.length; Idx++)
	{
		InputButtonDataList[Idx].Active = InputButtonDataList[Idx].RawActive;
	}
	//@todo - call the handlers for all the newly active buttons?
}

/** Returns TRUE if the button is currently held. */
simulated final function bool IsButtonActive( EGameButtons Button, optional bool bRaw )
{
	return (bRaw ? InputButtonDataList[Button].RawActive == 1 : InputButtonDataList[Button].Active == 1);
}

/** Forces a button to release, optionally skipping the handler */
final function ForceButtonRelease( EGameButtons Button, optional bool bSilent )
{
	if (!bSilent)
	{
		ButtonRelease( InputButtonDataList[Button].ButtonNameMapping );
	}
	else
	{
		InputButtonDataList[Button].RawActive = 0;
		InputButtonDataList[Button].Active = 0;
	}
}

/**
 * Updates the button values for all joystick inputs, triggering registered input events as well
 */
function UpdateJoystickInput(float DeltaTime)
{
	local SeqEvt_Input InputEvt;
	local byte Idx;

	// grab the current values of the sticks
	//@fixme - find some way to avoid hard-coded values, instead read from the ini setting?
	InputButtonDataList[GB_LeftStick_Up].RawActive = (aBaseY >= 0.3f ? 1 : 0);
	InputButtonDataList[GB_LeftStick_Down].RawActive = (aBaseY <= -0.3f ? 1 : 0);
	InputButtonDataList[GB_LeftStick_Left].RawActive = (aStrafe <= -0.3f ? 1 : 0);
	InputButtonDataList[GB_LeftStick_Right].RawActive = (aStrafe >= 0.3f ? 1 : 0);
	InputButtonDataList[GB_RightStick_Up].RawActive = (aLookUp >= 0.3f ? 1 : 0);
	InputButtonDataList[GB_RightStick_Down].RawActive = (aLookUp <= -0.3f ? 1 : 0);
	InputButtonDataList[GB_RightStick_Left].RawActive = (aTurn <= -0.3f ? 1 : 0);
	InputButtonDataList[GB_RightStick_Right].RawActive = (aTurn >= 0.3f ? 1 : 0);

	// iterate through each button to see if it's state has changed
	for ( Idx = GB_LeftStick_Up; Idx <= GB_RightStick_Right; Idx++ )
	{
		// if it doesn't match the current value
		if (InputButtonDataList[Idx].RawActive != InputButtonDataList[Idx].Active)
		{
			// If we are activating kismet events check for them.
			if ( bActivateKismetInputEvents )
			{
				// check for a kismet input event activation
				foreach InputEvents(InputEvt)
				{
					InputEvt.CheckInputActivate(EGameButtons(Idx),(InputButtonDataList[Idx].RawActive == 1));
				}
			}

			// and store the new value
			InputButtonDataList[Idx].Active = InputButtonDataList[Idx].RawActive;
		}
	}
}

/**
 * Overridden to update the joystick input.
 */
function PreProcessInput(float DeltaTime)
{
	Super.PreProcessInput(DeltaTime);

	UpdateJoystickInput(DeltaTime);
}

/**
 * Jump is disabled
 */
exec function Jump();

/**
 * Checks all the cases for button input fitering, called solely by ButtonPress()/ButtonRelease().
 *
 * @return TRUE if the input should be filtered (ignored)
 */
function bool FilterButtonInput(Name ButtonName,bool bPressed,int ButtonIdx)
{
	return false;
}

/**
 * Find the index in the InputButtonDataList using the button name as the key.
 *
 * @returns the index in the InputButtonDataList of where the ButtonName was found.
 */
final function int FindInputButtonDataIndex( Name ButtonName )
{
	local int Idx;

	for ( Idx = 0; Idx < InputButtonDataList.length; Idx++ )
	{
		if ( InputButtonDataList[Idx].ButtonNameMapping == ButtonName )
		{
			return Idx;
		}
	}

	return INDEX_NONE;
}

/**
 * Entry point for button press handling.
 */
exec function ButtonPress(coerce Name ButtonName)
{
	local int ButtonIdx;
	local delegate<HandleButtonInput> ButtonHandler;

	// get the id first
	ButtonIdx = FindInputButtonDataIndex( ButtonName );

`if(`notdefined(FINAL_RELEASE))
	// assert it's a valid button name
	if ( ButtonIdx == INDEX_NONE )
	{
		`Warn("Invalid button"@ButtonName@"in"@GetFuncName());
		return;
	}
`endif

	if ( InputButtonDataList[ButtonIdx].RawActive == 0 )
	{
		// mark the raw status
		InputButtonDataList[ButtonIdx].RawActive = 1;

		// if the input shouldn't be filtered
		if ( !FilterButtonInput(ButtonName,TRUE,ButtonIdx) )
		{
			//`log("unfiltered button press:"@ButtonName);
			// mark the status
			InputButtonDataList[ButtonIdx].Active = 1;
			// and call the corresponding function
			ButtonHandler = InputButtonDataList[ButtonIdx].ButtonHandler;
			if ( ButtonHandler != None )
			{
				ButtonHandler(TRUE);
			}
		}
	}
}

/**
 * Entry point for button release handling.
 */
exec function ButtonRelease(coerce Name ButtonName)
{
	local int ButtonIdx;
	local delegate<HandleButtonInput> ButtonHandler;

	// get the id first
	ButtonIdx = FindInputButtonDataIndex( ButtonName );

`if(`notdefined(FINAL_RELEASE))
	// assert it's a valid button name
	if ( ButtonIdx == INDEX_NONE )
	{
		`Warn("Invalid button"@ButtonName@"in"@GetFuncName());
		return;
	}
`endif

	if ( InputButtonDataList[ButtonIdx].RawActive == 1 )
	{
		// mark the raw status
		InputButtonDataList[ButtonIdx].RawActive = 0;

		// if the input shouldn't be filtered
		if ( !FilterButtonInput(ButtonName,FALSE,ButtonIdx) )
		{
			//`log("unfiltered button release:"@ButtonName);
			// mark the status
			InputButtonDataList[ButtonIdx].Active = 0;
			// and call the corresponding function
			ButtonHandler = InputButtonDataList[ButtonIdx].ButtonHandler;
			if ( ButtonHandler != None )
			{
				ButtonHandler(FALSE);
			}
		}
	}
}

// check for double click move
function Actor.EDoubleClickDir CheckForDoubleClickMove( float DeltaTime )
{
	local Actor.EDoubleClickDir DoubleClickMove, OldDoubleClick;
	local GearPawn P;
	local bool bIsDoingASpecialMove;

	// Don't use double click with gamepad input
	if( bUsingGamepad || bDisableDoubleClickMovement )
	{
		DoubleClickDir = DCLICK_None;
		CurrentDoubleClickDir = DCLICK_None;
		return DCLICK_None;
	}

	P = GearPawn(Pawn);
	if( P != None )
	{
		bIsDoingASpecialMove = P.IsDoingASpecialMove();

		// Special Moves can affect input
		if( bIsDoingASpecialMove )
		{
			P.SpecialMoves[P.SpecialMove].PreDoubleClickCheck( self );
		}
	}

	// If double click was active
	if( DoubleClickDir == DCLICK_Active )
	{
		// Keep it active
		DoubleClickMove = DCLICK_Active;
	}
	else
	{
		// Otherwise, there is no double click
		DoubleClickMove = DCLICK_None;
	}

	// If accepting double clicks (time threshold must exist)
	if( DoubleClickTime > 0.0 )
	{
		// If we currently have one active
		if( DoubleClickDir == DCLICK_Active )
		{
			// Check to see if it is complete
		}
		// Otherwise, if move is not complete
		else if( DoubleClickDir != DCLICK_Done )
		{
			// Store old double click direction
			OldDoubleClick = DoubleClickDir;
			// Clear current direction
			DoubleClickDir = DCLICK_None;

			// Check to see if player was moving in one direction AND
			// has edged in that direction again
			if( bEdgeForward && bWasForward )
			{
				DoubleClickDir = DCLICK_Forward;
			}
			else if( bEdgeBack && bWasBack )
			{
				DoubleClickDir = DCLICK_Back;
			}
			else if( bEdgeLeft && bWasLeft )
			{
				DoubleClickDir = DCLICK_Left;
			}
			else if( bEdgeRight && bWasRight )
			{
				DoubleClickDir = DCLICK_Right;
			}

			//			`log( GetFuncName()@RemappedJoyUp@RemappedJoyRight@RawJoyUp@RawJoyRight, IsInCoverState() );

			// When in cover, remap double click input if facing more right/left than forward
			if( IsInCoverState() &&
				Abs(RemappedJoyRight) > Abs(RemappedJoyUp) &&
				Abs(RawJoyRight) < Abs(RawJoyUp) )
			{
				if( DoubleClickDir == DCLICK_Forward )
				{
					DoubleClickDir = (RemappedJoyRight > 0.f) ? DCLICK_Right : DCLICK_Left;
				}
				else if( DoubleClickDir == DCLICK_Back )
				{
					DoubleClickDir = (RemappedJoyRight > 0.f) ? DCLICK_Left : DCLICK_Right;
				}
				else if( DoubleClickDir == DCLICK_Left )
				{
					DoubleClickDir = (RemappedJoyRight > 0.f) ? DCLICK_Forward : DCLICK_Back;
				}
				else if( DoubleClickDir == DCLICK_Right )
				{
					DoubleClickDir = (RemappedJoyRight > 0.f) ? DCLICK_Back : DCLICK_Forward;
				}
			}

			// If player was not moving in any direction
			if( DoubleClickDir == DCLICK_None )
			{
				// Reset the old direction
				DoubleClickDir = OldDoubleClick;
			}
			// Otherwise, if the double click action changed
			else if( DoubleClickDir != OldDoubleClick )
			{
				// Set timer for the click... allow at least one frame to intercept
				DoubleClickTimer = DoubleClickTime + 0.5 * DeltaTime;
			}
			// Otherwise, click direction was the same as the old
			else
			{
				// Take it as our current double click move
				DoubleClickMove = DoubleClickDir;
			}
		}

		// If double click is done
		if( DoubleClickDir == DCLICK_Done )
		{
			// Don't reset double click until a short time after it is over
			// (prevents immediate redouble click)
			DoubleClickTimer = FMin(DoubleClickTimer-DeltaTime,0);
			if( DoubleClickTimer < -0.15 )
			{
				DoubleClickDir = DCLICK_None;
				DoubleClickTimer = DoubleClickTime;
			}
		}
		// Otherwise, if double click direction is stored
		else if( DoubleClickDir != DCLICK_None && DoubleClickDir != DCLICK_Active )
		{
			// Check to see if it has timed out before getting another click
			DoubleClickTimer -= DeltaTime;
			if( DoubleClickTimer < 0 )
			{
				DoubleClickDir = DCLICK_None;
				DoubleClickTimer = DoubleClickTime;
			}
		}
	}

	//`log(WorldInfo.TimeSeconds@"DoubleClickDir"@DoubleClickDir@"DoubleClickTimer"@DoubleClickTimer@"bWasForward"@bWasForward@"bEdgeForward"@bEdgeForward@RawJoyUp@RawJoyRight);

	// Return the current double click move
	return DoubleClickMove;
}

