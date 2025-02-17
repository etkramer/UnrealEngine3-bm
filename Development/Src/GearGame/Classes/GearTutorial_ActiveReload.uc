/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_ActiveReload extends GearTutorial_GenericPress_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

// /** Called every tick so that the tutorial has a chance to update itself */
// event Update()
// {
// 	CheckActiveReloadSweetSpot();
// 
// 	Super.Update();
// }
// 
// /** Checks for when the reload is in the sweet spot and will pause the game and force them to press the reload button */
// final function CheckActiveReloadSweetSpot()
// {
// 	local GearPC MyGearPC;
// 	local GearPawn MyGearPawn;
// 	local GearWeapon CurrWeapon;
// 	local float PreReactionWindowDuration, ReloadStartTime, SuperSweetSpotDuration, SweetSpotDuration, RelativeTimePlayerPressedButton, IncrementAmount;
// 
// 	if ( !bTutorialIsComplete )
// 	{
// 		if ( !bGameplayIsFrozen )
// 		{
// 			// Grab the current weapon
// 			MyGearPC = GearPC(Outer);
// 			if( MyGearPC != None )
// 			{
// 				MyGearPawn = MyGearPC.MyGearPawn;
// 				if( MyGearPawn != None )
// 				{
// 					CurrWeapon = MyGearPawn.MyGearWeapon;
// 				}
// 			}
// 
// 			// See if we are in the super sweet spot of the active reload
// 			if ( CurrWeapon != None )
// 			{
// 				CurrWeapon.GetActiveReloadValues( ReloadStartTime, PreReactionWindowDuration, SuperSweetSpotDuration, SweetSpotDuration );
// 				RelativeTimePlayerPressedButton = TutorialMgr.WorldInfo.TimeSeconds - CurrWeapon.AR_TimeReloadButtonWasPressed;
// 				if( (RelativeTimePlayerPressedButton >= ReloadStartTime) && (RelativeTimePlayerPressedButton <= (ReloadStartTime + SuperSweetSpotDuration)) )
// 				{
// 					// Now freeze the gameplay to force a button press
// 					ToggleFreezeGameplay( true );
// 
// 					// Set the new input handle
// 					InputHandler.SetInputButtonHandle( GB_RightBumper, OnActiveReloaded );
// 
// 					// Reinit the UIScene labels so the button press will show up
// 					InitializeUISceneLabels();
// 				}
// 			}
// 		}
// 		else
// 		{
// 			// Grab the current weapon
// 			MyGearPC = GearPC(Outer);
// 			if ( MyGearPC != None )
// 			{
// 				MyGearPawn = GearPawn(MyGearPC.Pawn);
// 				if ( MyGearPawn != None )
// 				{
// 					CurrWeapon = MyGearPawn.MyGearWeapon;
// 				}
// 
// 				if ( MyGearPC.MyGearHud != None )
// 				{
// 					MyGearPC.MyGearHud.LastWeaponInfoTime = TutorialMgr.WorldInfo.TimeSeconds;
// 				}
// 			}
// 
// 			// Force the weapon reload to stay where it is
// 			if ( CurrWeapon != None )
// 			{
// 				CurrWeapon.GetActiveReloadValues( ReloadStartTime, PreReactionWindowDuration, SuperSweetSpotDuration, SweetSpotDuration );
// 				if ( TutorialMgr.WorldInfo.TimeSeconds - CurrWeapon.AR_TimeReloadButtonWasPressed >= ReloadStartTime + SuperSweetSpotDuration/2.0f )
// 				{
// 					IncrementAmount = (TutorialMgr.WorldInfo.TimeSeconds - CurrWeapon.AR_TimeReloadButtonWasPressed) - (ReloadStartTime + SuperSweetSpotDuration/2.0f);
// 					CurrWeapon.AR_TimeReloadButtonWasPressed += IncrementAmount;
// 					CurrWeapon.ReloadStartTime += IncrementAmount;
// 				}
// 			}
// 		}
// 	}
// }
// 
// /** Initializes the strings on the opened UI scene */
// function InitializeUISceneLabels( optional bool bHideButtonText )
// {
// 	Super.InitializeUISceneLabels( !bGameplayIsFrozen );
// }
// 
// /** Delegate fired when the player active reloads */
// function OnActiveReloaded(bool bPressed, optional bool bDblClickMove)
// {
// 	// Unfreeze game
// 	ToggleFreezeGameplay( false );
// 
// 	// Pass input to the player input for reloading
// 	GearPlayerInput(InputHandler).HandleButtonInput_RightBumper( bPressed );
// 
// 	// Complete
// 	OnTutorialCompleted();
// }
// 
// /**
// * Opportunity for the tutorial to override the filtering system for the input object handling this tutorial's input
// *		@Param Filtered - whether this button should be filtered or not (non zero means true)
// *		@Return - whether we've handled the filtering or not
// */
// function bool FilterButtonInput( Name ButtonName, bool bPressed, int ButtonIdx, out int Filtered )
// {
// 	// If we haven't started the freeze let the parent handle the input
// 	if ( !bGameplayIsFrozen )
// 	{
// 		return false;
// 	}
// 
// 	// Have the tutorial's input handler handle this input
// 	if ( ButtonName == InputHandler.InputButtonDataList[GB_RightBumper].ButtonNameMapping )
// 	{
// 		Filtered = 0;
// 	}
// 	else
// 	{
// 		Filtered = 1;
// 	}
// 
// 	return true;
// }

/** Called when the tutorial's scene is finished closing */
function OnTutorialSceneClosed( UIScene DeactivatedScene )
{
	Super.OnTutorialSceneClosed( DeactivatedScene );

	// Tutorial is done after the scene closes
	OnTutorialCompleted();
}

defaultproperties
{
	TutorialType=GEARTUT_ActiveReload
//	InputHandlerClass=class'GearPlayerInputTutorialPC'
	GameButtonType=GB_A
}
