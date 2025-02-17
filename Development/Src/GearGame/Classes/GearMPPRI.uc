/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Special PRI used to avoid the party/pregame from inheriting certain functionality
 */
class GearMPPRI extends GearPRI;

/**
 * This version unregisters both from the game and party sessions so that
 * players that quit to the main menu remove themselves from any party slots
 * so the host can invite people back in
 */
simulated function UnregisterPlayerFromSession()
{
	local OnlineSubsystem OnlineSub;
	local UniqueNetId ZeroId;

	Super.UnregisterPlayerFromSession();

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	// Make sure the player gets unregistered so that the session can replace them later
	if (OnlineSub != None &&
		OnlineSub.GameInterface != None &&
		UniqueId != ZeroId)
	{
		// Remove the player from the session
		OnlineSub.GameInterface.UnregisterPlayer('Party',UniqueId);
	}
}
