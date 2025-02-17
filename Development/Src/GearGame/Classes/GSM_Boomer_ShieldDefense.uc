
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Boomer_ShieldDefense extends GearSpecialMove;

/** Cached pointed to AnimNodeBlend, set by boomer. */
var transient AnimNodeBlend	CoverBlend;

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	// Force Boomer into ducking stance.
	CoverBlend.SetBlendTarget(1.f, 0.67f);

	if( PawnOwner.EquippedShield != None )
	{
		PawnOwner.EquippedShield.Expand();
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	// Boomer can get up now.
	CoverBlend.SetBlendTarget(0.f, 0.67f);

	Super.SpecialMoveEnded(PrevMove, NextMove);

	if( PawnOwner.EquippedShield != None )
	{
		PawnOwner.EquippedShield.Retract();
	}
}

defaultproperties
{
	bCanFireWeapon=FALSE
}


