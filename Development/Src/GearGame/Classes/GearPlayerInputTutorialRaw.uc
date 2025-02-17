/**
* GearPlayerInputTutorialRaw
*
* Gear player input post processing for tutorials
* This input handler is for catching raw input values and having function get called when they happen.
* It derives straight from GearPlayerInput_Base since PlayerController data is not required or wanted.
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearPlayerInputTutorialRaw extends GearPlayerInput_Base
	transient;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Returns the tutorial who is referencing this input object */
function GearTutorial_Base GetReferencingTutorial()
{
	if ( Outer.TutorialMgr != None )
	{
		return Outer.TutorialMgr.ActiveTutorial;
	}

	return None;
}

/** Overridden so that we can allow the tutorial to have an opportunity for filtering */
function bool FilterButtonInput(Name ButtonName,bool bPressed,int ButtonIdx)
{
	local int FilterResult;
	local GearTutorial_Base RefTutorial;

	// Allow start to happen for pausing the game
	if (ButtonName == 'Start')
	{
		return false;
	}

	RefTutorial = GetReferencingTutorial();

	if ( RefTutorial != None )
	{
		if ( RefTutorial.FilterButtonInput(ButtonName, bPressed, ButtonIdx, FilterResult) )
		{
			return ((FilterResult==0) ? false : true);
		}
	}

	return Super.FilterButtonInput( ButtonName, bPressed, ButtonIdx );
}

