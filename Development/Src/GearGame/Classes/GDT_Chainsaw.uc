/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_Chainsaw extends GDT_Melee;

static function bool ShouldGib(Pawn TestPawn, Pawn Instigator)
{
	return TRUE;
}

static function bool ShouldDBNO(Pawn TestPawn, Pawn Instigator, vector HitLocation,TraceHitInfo HitInfo)
{
	return FALSE;
}

defaultproperties
{
	KillGUDEvent=GUDEvent_KilledEnemyChainsaw
	KilledByIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=210,UL=77,VL=29)
}
