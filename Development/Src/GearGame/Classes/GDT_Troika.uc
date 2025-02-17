/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_Troika extends GDT_Ballistic;

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
	ImpactTypeBallisticID=ITB_Troika
	KilledByIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_DeathIcons',U=156,V=120,UL=77,VL=29)
	WeaponID=WC_Troika
}
