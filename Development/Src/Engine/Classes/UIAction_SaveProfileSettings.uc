/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This action joins an online game based upon the search object that
 * is bound to the UI list
 */
class UIAction_SaveProfileSettings extends UIAction
	native(inherit);

cpptext
{
	/** Callback for when the event is activated. */
	virtual void Activated();

	/**
	 * Polls to see if the async action is done
	 *
	 * @param ignored
	 *
	 * @return TRUE if the operation has completed, FALSE otherwise
	 */
	UBOOL UpdateOp(FLOAT)
	{
		// Activate the output links based upon whether the call worked
		if (bWroteProfile == TRUE)
		{
			OutputLinks(1).ActivateOutputLink();
		}
		else
		{
			OutputLinks(0).ActivateOutputLink();
		}

		return bIsDone;
	}
}

/** Whether or not the latent op has finished. */
var bool	bIsDone;

/** Whether or not the profile was written to. */
var bool	bWroteProfile;

/** Registers the profile write delegate. */
event RegisterDelegate()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;
	local int ControllerId;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerInt = OnlineSub.PlayerInterface;
		if (PlayerInt != None)
		{
			bIsDone = false;
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
			if ( ControllerId != INDEX_NONE )
			{
				PlayerInt.AddWriteProfileSettingsCompleteDelegate(ControllerId,OnProfileWriteComplete);
			}
		}
	}
}

/** Clears the profile write complete delegate. */
event ClearDelegate()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;
	local int ControllerId;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerInt = OnlineSub.PlayerInterface;
		if (PlayerInt != None)
		{
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
			if ( ControllerId != INDEX_NONE )
			{
				// Unregister our callbacks
				PlayerInt.ClearWriteProfileSettingsCompleteDelegate(ControllerId,OnProfileWriteComplete);
			}
		}
	}
}

/**
 * Sets the bIsDone flag to true so we unblock kismet.
 */
function OnProfileWriteComplete(byte LocalUserNum,bool bWasSuccessful)
{
	ClearDelegate();
	bWroteProfile = bWasSuccessful;
	bIsDone = true;
}


defaultproperties
{
	ObjName="Save Profile Settings"
	ObjCategory="Online"
	bAutoTargetOwner=true
	bLatentExecution=true

	bAutoActivateOutputLinks=false
	OutputLinks(0)=(LinkDesc="Failure")
	OutputLinks(1)=(LinkDesc="Success")
}
