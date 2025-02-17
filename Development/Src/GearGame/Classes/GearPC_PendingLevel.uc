/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This player controller is used during pending level transitions and its only
 * purpose is to handle pending level errors differently than the engine's
 * player controller
 */
class GearPC_PendingLevel extends GearPC;

/**
 * Notifies the player that an attempt to connect to a remote server failed, or an existing connection was dropped.
 *
 * @param	Message		a description of why the connection was lost
 * @param	Title		the title to use in the connection failure message.
 */
function NotifyConnectionError( optional string Message=Localize("Errors", "ConnectionFailed", "Engine"), optional string Title=Localize("Errors", "ConnectionFailed_Title", "Engine") )
{
	local UIMessageBoxBase MessageScene;

	`Log("GearPC_PendingLevel connection error while traveling to game");

	// server may have paused game before disconnecting, let's reset that for cleanup
	WorldInfo.Pauser = None;

	if ( Title == "" )
	{
		Title = "<Strings:Engine.Errors.ConnectionLost>";
	}

	MessageScene = OpenUIMessageScene(Title, Message, "GenericAccept", CancelPendingConnection, 'ConnectionError', class'UIScene'.const.DEFAULT_SCENE_PRIORITY + 5);
	MessageScene.bPauseGameWhileActive = false;
	MessageScene.bExemptFromAutoClose = true;
	MessageScene.bCloseOnLevelChange = false;
	MessageScene.bMenuLevelRestoresScene = true;
}
