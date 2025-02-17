
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_StumbleGoDownFromExplosion extends GSM_StumbleGoDown
	config(Pawn);

/** How long the the pawn will be down from this special move **/
var() config float KnockDownDuration;

protected function bool InternalCanDoSpecialMove()
{
	if( !Super.InternalCanDoSpecialMove() || PawnOwner.IsInCover() )
	{
		return FALSE;
	}

	return TRUE;
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Timer to auto get up pawn
	if( PawnOwner.WorldInfo.NetMode != NM_Client )
	{
		PawnOwner.SetTimer( KnockDownDuration, FALSE, nameof(PawnOwner.GetBackUpFromKnockDown) );
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	// Clear Timer
	if( PawnOwner.WorldInfo.NetMode != NM_Client )
	{
		PawnOwner.ClearTimer('GetBackUpFromKnockDown');
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

defaultproperties
{
	BS_DropAnimation=(AnimName[BS_FullBody]="ar_injuredc_drop")
	BS_Idle=(AnimName[BS_FullBody]="ar_injuredc_idle")
}
