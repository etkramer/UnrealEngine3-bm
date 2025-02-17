/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUIScene_SaveProfile extends UTUIScene
	native(UI);

var transient float OnScreenTime;
var transient int PlayerIndexToSave;
var transient bool bProfileSaved;
var transient bool bShutdown;

var transient float MinOnScreenTime;

/** If true, then use the elapsed time to close the save message */
var transient bool bUseTimedClose;

cpptext
{
	virtual void Tick( FLOAT DeltaTime );
}

delegate OnSaveFinished();

event PostInitialize()
{
	OnRawInputKey = KillInput;
	PerformSave();
}

function bool KillInput( const out InputEventParameters EventParms )
{
	return true;
}

event PerformSave()
{
	local OnlineSubsystem OnlineSub;

	if (!bProfileSaved)
	{
		// Don't use the timed close on consoles as they are event driven
		bUseTimedClose = !IsConsole(CONSOLE_Any);
		if (!bUseTimedClose)
		{
			OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
			if (OnlineSub != None && OnlineSub.PlayerInterface != None)
			{
				// Register the call back so we can shut down the scene upon completion
				OnlineSub.PlayerInterface.AddWriteProfileSettingsCompleteDelegate(PlayerIndexToSave,OnSaveProfileComplete);
			}
			else
			{
				// Use the timed method
				bUseTimedClose = true;
			}
		}

		SavePlayerProfile(PlayerIndexToSave);
		bProfileSaved = true;
	}
}

/**
 * Called when the save has completed the async operation
 *
 * @param LocalUserNum the controller index of the player who's write just completed
 * @param bWasSuccessful whether the save worked ok or not
 */
function OnSaveProfileComplete(byte LocalUserNum,bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.PlayerInterface != None)
	{
		// Register the call back so we can shut down the scene upon completion
		OnlineSub.PlayerInterface.ClearWriteProfileSettingsCompleteDelegate(PlayerIndexToSave,OnSaveProfileComplete);
	}

	if ( !bWasSuccessful )
	{
		//@todo Amitt/JoeW -- show an error when failing to save
	}

	// set the flag indicating that the scene is ready to be closed.  the native code will make sure that we've been open
	// the required amount of time before actually closing the scene
	bShutdown = true;
}

event ShutDown()
{
	// make sure we call CloseScene BEFORE firing the OnSaveFinished delegate, as there is code that depends on the order
	CloseScene(Self);
	OnSaveFinished();
	OnSaveFinished = None;
}

defaultproperties
{
	bRenderParentScenes=true
	bCloseOnLevelChange=true
	SceneInputMode=INPUTMODE_Simultaneous
	SceneRenderMode=SPLITRENDER_Fullscreen
	MinOnScreenTime=1.5
}
