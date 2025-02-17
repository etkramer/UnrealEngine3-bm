/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GDT_Rockworm_Chomp extends GearDamageType;

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
}
