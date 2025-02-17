/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


/**
 * Base class for interactions such as doors and buttons.
 */
class SeqEvt_Interaction extends SequenceEvent
	native(Sequence)
	dependson(GearTypes)
	abstract;

cpptext
{
	virtual UBOOL CheckActivate(AActor *InOriginator, AActor *InInstigator, UBOOL bTest=FALSE, TArray<INT>* ActivateIndices = NULL, UBOOL bPushTop = FALSE);
}

/** Max distance from instigator to allow interactions. */
var() bool bCheckInteractDistance;
var() float InteractDistance;

/** FOV check. */
var() bool bCheckInteractFOV;
var() float InteractFOV;

/** Action displayed on the HUD */
var ActionInfo InteractAction;

simulated function bool CanDoInteraction(GearPC PC)
{
	return bEnabled;
}

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
	return Super.GetObjClassVersion() + 9;
}

defaultproperties
{
	ObjCategory="Gear"

	bCheckInteractDistance=TRUE
	InteractDistance=200.f

	bCheckInteractFOV=TRUE
	InteractFOV=0.3
}
