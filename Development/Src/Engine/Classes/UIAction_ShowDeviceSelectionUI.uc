/**
 * This action tells the online subsystem to show the marketplace ui for content
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_ShowDeviceSelectionUI extends UIAction
	native(inherit);

cpptext
{
	/**
	 * Lets script code trigger an async call to the online subsytem
	 */
	virtual void Activated(void)
	{
		Super::Activated();

		bIsDone=false;
		bResult=false;
		eventSelectDevice();
	}

	/**
	 * Polls to see if the async action is done
	 *
	 * @param ignored
	 *
	 * @return TRUE if the operation has completed, FALSE otherwise
	 */
	UBOOL UpdateOp(FLOAT)
	{
		if(bIsDone)
		{
			if(bResult)
			{
				OutputLinks(1).ActivateOutputLink();
			}
			else
			{
				OutputLinks(0).ActivateOutputLink();
			}
		}
		return bIsDone;
	}
}

/** The amount of data that is going to be written */
var() int SizeNeeded;

/** Whether the UI should always be displayed or not */
var() bool bForceShowUI;

/** Will hold the resultant value the user input */
var() int DeviceID;

/** The name of the device that was selected */
var() string DeviceName;

/** Whether the async call is done or not */
var bool bIsDone;

/** Whether the user chose a device or cancelled the dialog. */
var bool bResult;

/** Writes the current DeviceID to all connected variables. */
native function WriteToVariables();

/**
 * Uses the online subsystem to show the device selection UI
 */
event SelectDevice()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if (PlayerIntEx != None)
		{
			// Register our callback
			PlayerIntEx.AddDeviceSelectionDoneDelegate(GamepadID,OnDeviceSelectionComplete);
			// Show the UI
			if (PlayerIntEx.ShowDeviceSelectionUI(GamepadID,SizeNeeded,bForceShowUI) == false)
			{
				// Don't wait if there was an error
				`Log("Error occured while trying to display device selection UI");
				PlayerIntEx.ClearDeviceSelectionDoneDelegate(GamepadID,OnDeviceSelectionComplete);
				bIsDone = true;
			}
		}
		else
		{
			// Don't wait if there was an error
			bIsDone = true;
			`Log("No OnlinePlayerInterfaceEx present to display the device selection UI");
		}
	}
	// if we do not have an Online SubSystem then we can just return true
	else
	{
		bIsDone = true;
	}
}

/**
 * Reads the results of the user's device choice
 *
 * @param bWasSuccessful true if the action completed without error, false if there was an error
 */
function OnDeviceSelectionComplete(bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if (PlayerIntEx != None)
		{
			// Unregister our callback
			PlayerIntEx.ClearDeviceSelectionDoneDelegate(GamepadID,OnDeviceSelectionComplete);
			// Don't read the information unless it was successful
			if (bWasSuccessful == true)
			{
				// Read the per user results
				DeviceID = PlayerIntEx.GetDeviceSelectionResults(GamepadID,DeviceName);
				WriteToVariables();
				bResult = true;
			}
		}
	}

	`log("Device Result: " $ bWasSuccessful);
	bIsDone = true;

}

defaultproperties
{
	ObjName="Select Media Device"
	ObjCategory="Online"

	bAutoActivateOutputLinks=false
	bLatentExecution=true

	OutputLinks(0)=(LinkDesc="Failure")
	OutputLinks(1)=(LinkDesc="Success")

	VariableLinks(0)=(ExpectedType=class'SeqVar_String',LinkDesc="Device Name",PropertyName=DeviceName,bWriteable=true)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="DeviceID",PropertyName=DeviceID,bWriteable=true)
}
