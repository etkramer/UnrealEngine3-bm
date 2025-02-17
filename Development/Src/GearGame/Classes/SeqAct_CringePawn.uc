/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_CringePawn extends SequenceAction
	native(Sequence);

/** If TRUE, all pawns within CringeRadius of Target actor(s) will cringe.  If FALSE, only Target actor(s) will cringe. */
var() bool		bRadialCringe;
/** If bRadialCringe is TRUE, all pawns within this radius of the target actor(s) will cringe. */
var() float		CringeRadius;

/**
 * Optional duration for the cringe.  0.f means default duration;
 * This is time, in seconds, for the blend in and looping cringe anim to play.
 * Does not include the loop out.
 */
var() float		CringeDuration;

cpptext
{
	void Activated();
}

defaultproperties
{
	ObjName="Cringe"
	ObjCategory="Pawn"

	InputLinks(0)=(LinkDesc="Start")
	InputLinks(1)=(LinkDesc="Stop")

	bRadialCringe=FALSE
	CringeRadius=256
}
