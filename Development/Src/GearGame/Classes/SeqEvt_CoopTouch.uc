/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class SeqEvt_CoopTouch extends SeqEvent_Touch
	native(Sequence);

cpptext
{
	UBOOL CheckUnTouchActivate(AActor *inOriginator, AActor *inInstigator, UBOOL bTest = FALSE);

private:
	virtual UBOOL CheckActivate(AActor *InOriginator, AActor *InInstigator, UBOOL bTest=FALSE, TArray<INT>* ActivateIndices = NULL, UBOOL bPushTop = FALSE);
}

defaultproperties
{
	OutputLinks.Empty
	OutputLinks(0)=(LinkDesc="All Touched")
	ClassProximityTypes(0)=class'GearPawn_COGMarcus'
	ClassProximityTypes(1)=class'GearPawn_COGDom'

	ObjName="Multi Touch"

	bPlayerOnly=FALSE
	bForceOverlapping=FALSE
	bAllowDeadPawns=true //@FIXME: so DBNO pawns count as they are considered dead
}
