/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_Avalanche extends GDT_Environment;

static function bool ShouldDBNO(Pawn TestPawn, Pawn Instigator, vector HitLocation,TraceHitInfo HitInfo)
{
	return FALSE;
}

static function bool ShouldGib(Pawn TestPawn, Pawn Instigator)
{
	return TRUE;
}

static function HandleDeadPlayer(GearPC Player)
{
	// death by avalanche should just move to the first battle cam
	Player.TransitionToSpectate();
}

defaultproperties
{
}
