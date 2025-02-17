/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This action joins an online game based upon the search object that
 * is bound to the UI list
 */
class UIAction_JoinOnlineGame extends UIAction
	native(inherit)
	dependson(OnlineGameSearch);

/** Holds the game we are currently trying to join */
var OnlineGameSearchResult PendingGameJoin;

/** Temporary var for accessing actor functions */
var WorldInfo CachedWorldInfo;

/** Whether the latent action is done */
var bool bIsDone;

/** Whether there was an error or not */
var bool bResult;

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
	UBOOL UpdateOp(FLOAT);
}

/**
 * Performs the join process by kicking off the async join for the specified game
 *
 * @param ControllerId the player joining
 * @param GameToJoin the search results for the server to join
 * @param WorldInfo the world info in case an actor is needed for iteration
 */
event JoinOnlineGame(byte ControllerId,OnlineGameSearchResult GameToJoin,WorldInfo InWorldInfo)
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameInterface GameInterface;

	if (GameToJoin.GameSettings != None)
	{
		// Figure out if we have an online subsystem registered
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			// Grab the game interface to verify the subsystem supports it
			GameInterface = OnlineSub.GameInterface;
			if (GameInterface != None)
			{
				CachedWorldInfo = InWorldInfo;
				PendingGameJoin = GameToJoin;
				// Set the delegate for notification
				GameInterface.AddJoinOnlineGameCompleteDelegate(OnJoinGameComplete);
				// Start the async task
				if (!GameInterface.JoinOnlineGame(ControllerId,'Game',GameToJoin))
				{
					OnJoinGameComplete('Game',false);
				}
			}
			else
			{
				`warn("OnlineSubsystem does not support the game interface. Can't join game");
			}
		}
		else
		{
			`warn("No OnlineSubsystem present. Can't join game");
		}
	}
	else
	{
		OnJoinGameComplete('Game',false);
	}
}

/**
 * Handles the notification that the join completed. If successful, the session
 * is joined, otherwise an error is shown
 *
 * @param SessionName the name of the session that was joined
 * @param bWasSuccessful whether the async task succeeded or not
 */
function OnJoinGameComplete(name SessionName,bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local string URL;
	local LocalPlayer LP;

	bIsDone = true;
	bResult = bWasSuccessful;
	// Figure out if we have an online subsystem registered
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.GameInterface != None)
	{
		if (bWasSuccessful)
		{
			// Get the platform specific information
			if (OnlineSub.GameInterface.GetResolvedConnectString(SessionName,URL))
			{
				// Call the game specific function to appending/changing the URL
				URL = BuildJoinURL(URL);
				`Log("Resulting url is ("$URL$")");

				LP = class'UIInteraction'.static.GetLocalPlayer(0);
				if ( LP != None && LP.Actor != None )
				{
					LP.Actor.ClientTravel(URL, TRAVEL_Relative);
				}
				else
				{
					CachedWorldInfo.ConsoleCommand("open" @ URL);
				}
			}
			else
			{
				bResult = false;
			}
		}

		// Remove the delegate from the list
		OnlineSub.GameInterface.ClearJoinOnlineGameCompleteDelegate(OnJoinGameComplete);
	}

	CachedWorldInfo = None;
}

/**
 * Builds the string needed to join a game from the resolved connection:
 *		"open 172.168.0.1"
 *
 * NOTE: Overload this method to modify the URL before exec-ing it
 *
 * @param ResolvedConnectionURL the platform specific URL information
 *
 * @return the final URL to use to open the map
 */
function string BuildJoinURL(string ResolvedConnectionURL)
{
	return ResolvedConnectionURL;
}

defaultproperties
{
	ObjName="Join Online Game"
	ObjCategory="Online"
	bAutoTargetOwner=true

	bAutoActivateOutputLinks=false
	bLatentExecution=true

	OutputLinks(0)=(LinkDesc="Failure")
	OutputLinks(1)=(LinkDesc="Success")
}
