/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_CurbStomp extends GDT_Execution_Base;

/** Curb stomp always pops off the head. */
static function bool ShouldHeadShotGib(Pawn TestPawn, Pawn Instigator)
{
	return TRUE;
}

defaultproperties
{
	bForceRagdollDeath=TRUE
	KilledByIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=30,UL=77,VL=29)
	KillGUDEvent=GUDEvent_KilledEnemyCurbstomp
}
