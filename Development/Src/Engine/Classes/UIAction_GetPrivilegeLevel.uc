/**
 * This action retrieves the privilege level for a user.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_GetPrivilegeLevel extends UIAction
	native(inherit);


cpptext
{
	/**
	 * Activated event handler.
	 */
	virtual void Activated(void);
}

/** Different actions that each have their own privilege level. */
enum EFeaturePrivilegeMode
{
	FPM_Online,
	FPM_Chat,
	FPM_DownloadUserContent,
	FPM_PurchaseContent
};

/** Player to get privilege level for. */
var() int PlayerID;

/** Which action to get the level for. */
var() EFeaturePrivilegeMode PrivMode;

/** @return Returns the player's current login status enum using the online subsystem. */
event EFeaturePrivilegeLevel GetPrivilegeLevel(int ControllerID)
{
	local EFeaturePrivilegeLevel Result;
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInterface;

	Result = FPL_Disabled;

	// Figure out if we have an online subsystem registered
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		// Grab the player interface to verify the subsystem supports it
		PlayerInterface = OnlineSub.PlayerInterface;
		if (PlayerInterface != None)
		{
			switch(PrivMode)
			{
			case FPM_Online:
				Result = PlayerInterface.CanPlayOnline(ControllerID);
				break;
			case FPM_Chat:
				Result = PlayerInterface.CanCommunicate(ControllerID);
				break;
			case FPM_DownloadUserContent:
				Result = PlayerInterface.CanDownloadUserContent(ControllerID);
				break;
			case FPM_PurchaseContent:
				Result = PlayerInterface.CanPurchaseContent(ControllerID);
				break;	
			}
		}
	}

	return Result;
}

defaultproperties
{
	ObjName="Get Privilege Level"
	ObjCategory="Online"

	bAutoActivateOutputLinks=false

	OutputLinks(0)=(LinkDesc="Disabled")
	OutputLinks(1)=(LinkDesc="Enabled for Friends Only")
	OutputLinks(2)=(LinkDesc="Enabled")
}
