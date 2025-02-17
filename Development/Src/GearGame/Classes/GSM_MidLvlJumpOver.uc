
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_MidLvlJumpOver extends GSM_MantleOverCoverBase
	native(SpecialMoves);

protected function bool InternalCanDoSpecialMove()
{
	if( PawnOwner.FindCoverType() != CT_MidLevel )
	{
		return FALSE;
	}

	return Super.InternalCanDoSpecialMove();
}

// Base function to handle mantle over variations.
function INT PackSpecialMoveFlags()
{
	return 0;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced, PrevMove);

	if (PCOwner != None)
	{
		PCOwner.TriggerGearEventDelegates( eGED_Mantle );
	}
}
