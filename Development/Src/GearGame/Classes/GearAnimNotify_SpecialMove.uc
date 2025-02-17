
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Send a notification to the current SpecialMove in place
 */
class GearAnimNotify_SpecialMove extends AnimNotify_Scripted;

/** Name to pass in some information */
var()	Name	Info;

event Notify(Actor Owner, AnimNodeSequence AnimSeqInstigator)
{
	local GearPawn	P;

	P = GearPawn(Owner);
	if( P != None && P.SpecialMove != SM_None && P.SpecialMoves[P.SpecialMove] != None )
	{
		P.SpecialMoves[P.SpecialMove].AnimNotify(AnimSeqInstigator, Self);
	}
}