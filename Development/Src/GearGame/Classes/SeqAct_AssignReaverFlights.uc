/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class SeqAct_AssignReaverFlights extends SequenceAction;

/** If TRUE, allow Reaver to land after taking some damage. */
var()	bool	bAllowLanding;

/** If TRUE, reaver will not fire rockets in the air. */
var()	bool	bSuppressRockets;

/** Min time between rockets firing */
var()	float	MinTimeBetweenRockets;

/** Disables spawning soft gibs */
var()	bool	bDisableSoftGibs;

/** This reaver will instantly gibbing upon death, instead of ragdolling for a couple of seconds. */
var()	bool	bGibStraightAway;

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 1;
}

defaultproperties
{
	ObjName="Assign Reaver Flights"
	ObjCategory="Gear"

	InputLinks(1)=(LinkDesc="Stop")

	VariableLinks(1)=(ExpectedType=class'InterpData',LinkDesc="InitialFlight",MinVars=1,MaxVars=1)
	VariableLinks(2)=(ExpectedType=class'InterpData',LinkDesc="OtherFlights")
	VariableLinks(3)=(ExpectedType=class'SeqVar_Object',LinkDesc="LandingPoints")

	bDisableSoftGibs=TRUE
	bAllowLanding=TRUE
	bGibStraightAway=FALSE
}
