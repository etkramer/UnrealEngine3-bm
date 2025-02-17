
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_CoverRun extends GearSpecialMove;

function bool CanChainMove(ESpecialMove NextMove)
{
	return NextMove == SM_StdLvlSwatTurn;
}

protected function bool InternalCanDoSpecialMove()
{
	// Make sure weapon allows to roadie run
	if( !PawnOwner.MyGearWeapon.bAllowsRoadieRunning )
	{
		return FALSE;
	}

	// don't allow if not in cover or is leaning or is reloading
	if( PawnOwner.CoverType == CT_None ||
		PawnOwner.CurrentLink.Slots.Length == 1 ||
		PawnOwner.IsReloadingWeapon() )
	{
		return FALSE;
	}

	return TRUE;
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearPC PC;
	PC = GearPC(PawnOwner.Controller);
	if (PC != None)
	{
		// silently release the 'A' button to prevent popping out of cover unintentionally
		PC.ForceButtonRelease(PC.bUseAlternateControls?GB_X:GB_A,TRUE);
		PC.DoubleClickDir = DCLICK_None;
	}
	Super.SpecialMoveEnded(PrevMove, NextMove);
}


defaultproperties
{
	bCanFireWeapon=FALSE
	bDisableMovement=TRUE
}