
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Brumak_RtGunPain extends GSM_Brumak_BasePlaySingleAnim;

simulated function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	local GearAI_Brumak	BrumakAI;

	Super.SpecialMoveStarted( bForced, PrevMove );

	// Clear Targets, abort attack
	if( Brumak != None )
	{
		Brumak.SetSideGunAimTarget(None, FALSE);
	}

	BrumakAI = GearAI_Brumak(PawnOwner.Controller);
	if( BrumakAI != None )
	{
		if( BrumakAI.RightGunAI != None ) { BrumakAI.RightGunAI.GotoState( 'Silent' ); }
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	local GearAI_Brumak	BrumakAI;

	Super.SpecialMoveEnded(PrevMove, NextMove);

	BrumakAI = GearAI_Brumak(PawnOwner.Controller);
	if( BrumakAI != None )
	{
		if( BrumakAI.RightGunAI != None ) { BrumakAI.RightGunAI.GotoState( 'Auto' ); }
	}
}

defaultproperties
{
	BS_Animation=(AnimName[BS_Additive]="Right_Gun_Small_Pain")
	BlendInTime=0.33f
	BlendOutTime=0.67f
}