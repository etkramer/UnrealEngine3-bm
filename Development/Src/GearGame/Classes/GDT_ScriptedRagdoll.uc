/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_ScriptedRagdoll extends GDT_Environment;

static function bool ShouldGib(Pawn TestPawn, Pawn Instigator)
{
	return FALSE;
}

static function bool ShouldDBNO(Pawn TestPawn, Pawn Instigator, vector HitLocation,TraceHitInfo HitInfo)
{
	return FALSE;
}

static function bool IsScriptedDamageType()
{
	return TRUE;
}

