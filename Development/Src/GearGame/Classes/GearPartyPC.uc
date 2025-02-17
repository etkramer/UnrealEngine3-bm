/**
 * This player controller class is used for the party lobby.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPartyPC extends GearMenuPC;

/** Used to assign the gamertag as the player name for multiplayer matches */
event InitUniquePlayerId()
{
	local LocalPlayer LocPlayer;

	Super.InitUniquePlayerId();

	LocPlayer = LocalPlayer(Player);
	if (LocPlayer != None &&
		OnlineSub != None &&
		OnlineSub.PlayerInterface != None)
	{
		// See if this is a guest login and mark the PRI as being a guest
		if (OnlineSub.PlayerInterface.IsGuestLogin(LocPlayer.ControllerId))
		{
			ServerMaskAsGuest();
		}
	}
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

/** Used for a client to tell the server that the player is a guest */
reliable server function ServerMaskAsGuest()
{
	local GearPartyPRI PRI;

	PRI = GearPartyPRI(PlayerReplicationInfo);
	if (PRI != None)
	{
		PRI.bIsGuest = true;
	}
}

/** Allows a client to cancel matchmaking so they can leave */
reliable server function ServerCancelMatchmaking()
{
	local GearUISceneFELobby_Party Scene;
	local GameUISceneClient GameSceneClient;

	GameSceneClient = class'UIRoot'.static.GetSceneClient();
	// Evil hack to make it seem like the host hit the cancel button
	Scene = GearUISceneFELobby_Party(GameSceneClient.FindSceneByTag('PartyLobby',None));
	if (Scene != None)
	{
		Scene.StartMatchButtonClicked(None,0);
	}
}