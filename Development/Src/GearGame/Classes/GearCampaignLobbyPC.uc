/**
 * This player controller class is used for the campaign lobby.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearCampaignLobbyPC extends GearMenuPC;

/**
 * Sets the rich presence to the default value (in the menu)
 */
reliable client function ClientSetOnlineStatus()
{
	SetRichPresenceString(CONTEXT_PRESENCE_CAMPAIGNPARTYPRESENCE);
}

/**
 * Registers the unique id of the player with the server so it can be replicated
 * to all clients.
 *
 * @param UniqueId the buffer that holds the unique id
 * @param bWasInvited whether the player was invited to play or is joining via search
 */
reliable server function ServerSetUniquePlayerId(UniqueNetId UniqueId,bool bWasInvited)
{
	local UniqueNetId ZeroId;

	if (!bPendingDestroy && !bReceivedUniqueId)
	{
		// Store the unique id, so it will be replicated to all clients
		PlayerReplicationInfo.SetUniqueId(UniqueId);

		if (OnlineSub != None &&
			OnlineSub.GameInterface != None &&
			ZeroId != UniqueId)
		{
			// Go ahead and register the player as part of the session
			OnlineSub.GameInterface.RegisterPlayer(PlayerReplicationInfo.SessionName,PlayerReplicationInfo.UniqueId,bWasInvited);
		}
		// Notify the game that we can now be muted and mute others
		if (WorldInfo.NetMode != NM_Client)
		{
			WorldInfo.Game.UpdateGameplayMuteList(self);
			// Now that the unique id is replicated, this player can contribute to skill
			WorldInfo.Game.RecalculateSkillRating();
		}

		bReceivedUniqueId = true;
	}
}

/**
 * Displays a message when the physical network is lost during a networked game
 *
 * @param bIsConnected the new connection state
 */
function OnLinkStatusChange(bool bIsConnected)
{
	// Ignore this change if standalone
	if (!bIsConnected && WorldInfo.NetMode != NM_Standalone)
	{
		ShowMessageBox("<Strings:GearGameUI.MessageBoxErrorStrings.LostLink_Title>",
			"<Strings:GearGameUI.MessageBoxErrorStrings.LostLink_Message>",
			'ConnectionError');
		ReturnToMainMenu();
	}
}

