/**
 * This action tells the online subsystem to show the login ui
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_ShowLoginUI extends UIAction
	native(inherit);

/** Whether the async call is done or not */
var bool bIsDone;

/** Whether to show the online only enabled profiles or not */
var() bool bShowOnlineOnly;

cpptext
{
	/**
	 * Lets script code trigger an async call to the online subsytem
	 */
	virtual void Activated(void)
	{
		bIsDone=false;
		eventShowUI();
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
		return bIsDone;
	}
}

/** Triggers the async call. */
event ShowUI()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;
	local bool bSuccess;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerInt = OnlineSub.PlayerInterface;
		if(PlayerInt != none)
		{
			// Set login delegates
			PlayerInt.AddLoginChangeDelegate(OnLoginChanged);
			PlayerInt.AddLoginCancelledDelegate(OnLoginCancelled);
			// Now display the UI
			bSuccess = PlayerInt.ShowLoginUI(bShowOnlineOnly);
			if (bSuccess == false)
			{
				// Clear delegate
				PlayerInt.ClearLoginChangeDelegate(OnLoginChanged);
				PlayerInt.ClearLoginCancelledDelegate(OnLoginCancelled);
				// Exit out early if we couldnt show the UI.
				bIsDone = true;
				`Log("Failed to show the login UI");
			}
		}
		else
		{
			bIsDone = true;
		}
	}
	else
	{
		// Exit out early if there is no OnlineSub.
		bIsDone = true;
	}

	if (bSuccess == false)
	{
		ActivateNamedOutputLink("Failure");
	}
}

/**
 * Sets the bIsDone flag to true so we unblock kismet.
 */
function OnLoginChanged()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerInt = OnlineSub.PlayerInterface;
		if (PlayerInt != None)
		{
			// Unregister our callbacks
			PlayerInt.ClearLoginChangeDelegate(OnLoginChanged);
			PlayerInt.ClearLoginCancelledDelegate(OnLoginCancelled);
		}
	}
	bIsDone = true;
	ActivateNamedOutputLink("Completed");
}

/**
 * Login was cancelled. Sets the bIsDone flag to true so we unblock kismet.
 * @todo amitt do error handling due to canceling
 */
function OnLoginCancelled()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerInt = OnlineSub.PlayerInterface;
		if (PlayerInt != None)
		{
			// Unregister our callbacks
			PlayerInt.ClearLoginChangeDelegate(OnLoginChanged);
			PlayerInt.ClearLoginCancelledDelegate(OnLoginCancelled);
		}
	}

	bIsDone = true;
	ActivateNamedOutputLink("Cancelled");
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
	return Super.GetObjClassVersion() + 1;
}

DefaultProperties
{
	ObjName="Show Login UI"
	ObjCategory="Online"
	bAutoTargetOwner=true
	bLatentExecution=true
	bAutoActivateOutputLinks=false

	OutputLinks(0)=(LinkDesc="Completed")
	OutputLinks(1)=(LinkDesc="Cancelled")
	OutputLinks(2)=(LinkDesc="Failure")
}

