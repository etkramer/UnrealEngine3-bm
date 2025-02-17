/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_Laser extends GDT_Environment;

static function bool ShouldGib(Pawn TestPawn, Pawn Instigator)
{
	return true;
}

static function bool IsScriptedDamageType()
{
	return true;
}

static function bool ShouldDBNO(Pawn TestPawn, Pawn Instigator, vector HitLocation, TraceHitInfo HitInfo)
{
	return false;
}


defaultproperties
{
	KDamageImpulse=100
	RadialDamageImpulse=400
	bRadialDamageVelChange=TRUE
	KDeathUpKick=100
}
