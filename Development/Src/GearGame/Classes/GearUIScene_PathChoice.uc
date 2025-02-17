/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearUIScene_PathChoice extends UIScene
	native(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The kismet action that is initiating this scene */
var SeqAct_DisplayPathChoice PathChoiceAction;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Setup all of the labels strings */
final function InitializeScene( SeqAct_DisplayPathChoice Action, String Title, String LeftDesc, String RightDesc )
{
	// Set the action
	PathChoiceAction = Action;

	// Title
	SetLabelToString( 'lblTitle', Title );

	// Left description
	SetLabelToString( 'lblLeftDesc', LeftDesc );

	// Right description
	SetLabelToString( 'lblRightDesc', RightDesc );

	//PlayUISound('ChoicePoint');
}

/** Finds a lable with name "LabelName" and set the databind of the string to the localized VariableLookup */
final function SetLabelToString( Name LabelName, String VariableLookup )
{
	local string LocalizedString;
	local UILabel Label;

	Label = UILabel(FindChild(LabelName,true));

	if ( Label != None )
	{
		LocalizedString = Localize( "PATHCHOICE", VariableLookup, "GearGame" );
		Label.SetDataStoreBinding( LocalizedString );
	}
}

/**
* Provides a hook for unrealscript to respond to input using actual input key names (i.e. Left, Tab, etc.)
*
* Called when an input key event is received which this widget responds to and is in the correct state to process.  The
* keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
*
* This delegate is called BEFORE kismet is given a chance to process the input.
*
* @param	EventParms	information about the input event.
*
* @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
*/
function bool HandleInputKey( const out InputEventParameters EventParms )
{
	local bool bResult, bShouldAcceptInput;
	local GearPC MyGearPC;

	// See if we should accept input - only Marcus (server) should make the decision
	bShouldAcceptInput = false;
	if ( PlayerOwner != None )
	{
		MyGearPC = GearPC(PlayerOwner.Actor);
		if ( (MyGearPC != None) && (MyGearPC.Role == ROLE_Authority) )
		{
			bShouldAcceptInput = true;
		}
	}

	if( bShouldAcceptInput && ((EventParms.EventType == IE_Pressed) && (PathChoiceAction != None)) )
	{
		if ( (EventParms.InputKeyName == 'XboxTypeS_LeftTrigger') || (EventParms.InputKeyName == 'LeftMouseButton') )
		{
			PathChoiceAction.bIsLeftChoiceResult = true;
			PathChoiceAction.bIsDone = true;
			bResult = true;
		}
		else if ( (EventParms.InputKeyName == 'XboxTypeS_RightTrigger') || (EventParms.InputKeyName == 'RightMouseButton') )
		{
			PathChoiceAction.bIsLeftChoiceResult = false;
			PathChoiceAction.bIsDone = true;
			bResult = true;
		}
	}

	return bResult;
}

defaultproperties
{
	OnRawInputKey=HandleInputKey
	bDisplayCursor=false
	bPauseGameWhileActive=false

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object
}
