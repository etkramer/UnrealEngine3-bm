/**
 * Event mapped to controller input.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqEvt_EnteredRevivalState extends SequenceEvent
	native(Sequence);

cpptext
{
	virtual UBOOL CheckActivate(AActor *InOriginator, AActor *InInstigator, UBOOL bTest=FALSE, TArray<INT>* ActivateIndices = NULL, UBOOL bPushTop = FALSE);
};


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
	ObjName="Entered Revival State"
	ObjCategory="Gear"

	OutputLinks[0]=(LinkDesc="DBNO")
	OutputLinks[1]=(LinkDesc="Revived")

	bPlayerOnly=FALSE
	MaxTriggerCount=0

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Pawn",bWriteable=true)
}
