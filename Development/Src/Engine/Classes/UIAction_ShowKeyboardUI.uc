/**
 * This action tells the online subsystem to show the keyboard input ui
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_ShowKeyboardUI extends UIAction
	native(inherit);

cpptext
{
	/**
	 * Lets script code trigger an async call to the online subsytem
	 */
	virtual void Activated(void);

	virtual void DeActivated();

	/**
	 * Polls to see if the async action is done
	 *
	 * @param ignored
	 *
	 * @return TRUE if the operation has completed, FALSE otherwise
	 */
	UBOOL UpdateOp(FLOAT);
}

/** The text to display at the top of the screen */
var() string TitleText;

/** The text to put in the edit control by default */
var() string DefaultText;

/** The text describing what is being input */
var() string DescriptionText;

/** Whether to validate the text or not */
var() bool bShouldValidate;

/** Whether to use password entry or not */
var() bool bIsPassword;

/** Will hold the resultant value the user input */
var string StringReturnValue;

/** Will hold the input read from keyboard and pass it to stringreturnvalue when ready to end action */
var string TempStringReturnValue;

/** Whether the async call is done or not */
var bool bIsDone;

/**
 * Uses the online subsystem to show the virtual keyboard for input
 */
event ReadKeyboardInput()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerInt = OnlineSub.PlayerInterface;
		if (PlayerInt != None)
		{
			// Register our callback
			PlayerInt.AddKeyboardInputDoneDelegate(OnKeyboardInputDone);
			// Show the UI
			if (PlayerInt.ShowKeyboardUI(class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex),
				TitleText,DescriptionText,bIsPassword,bShouldValidate,DefaultText) == false)
			{
				// Don't wait if there was an error
				bIsDone = true;
			}
		}
	}
}

/**
 * Reads the results of the user's keyboard input
 *
 * @param bWasSuccessful whether the call succeeded or not
 */
function OnKeyboardInputDone(bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;
	local byte bWasCancelled;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerInt = OnlineSub.PlayerInterface;
		if (PlayerInt != None)
		{
			if (bWasSuccessful == true)
			{
				TempStringReturnValue = PlayerInt.GetKeyboardInputResults(bWasCancelled);
			}
			else
			{
				`Log("Failed to read keyboard input");
			}
		}
	}
	bIsDone = true;
}

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 0;
}

defaultproperties
{
	ObjName="Get Keyboard Input"
	ObjCategory="Online"

	bLatentExecution=TRUE

	bShouldValidate=TRUE

	OutputLinks(0)=(LinkDesc="Out")
	OutputLinks(1)=(LinkDesc="Finished")

	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="Title Text",PropertyName="TitleText")
	VariableLinks(1)=(ExpectedType=class'SeqVar_String',LinkDesc="Description Text",PropertyName="DescriptionText")
	VariableLinks(2)=(ExpectedType=class'SeqVar_String',LinkDesc="Default Text",PropertyName="DefaultText")
	VariableLinks(3)=(ExpectedType=class'SeqVar_Bool',LinkDesc="Needs Validation",PropertyName="bShouldValidate")
	VariableLinks(4)=(ExpectedType=class'SeqVar_String',LinkDesc="String Return Value",PropertyName=StringReturnValue)
}
