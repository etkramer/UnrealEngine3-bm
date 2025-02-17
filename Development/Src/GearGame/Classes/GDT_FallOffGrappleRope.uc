/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_FallOffGrappleRope extends GearDamageType;

/** Should the Pawn transition to DBNO when killed with this damage type? */
static function bool ShouldDBNO(Pawn TestPawn, Pawn Instigator, vector HitLocation,TraceHitInfo HitInfo)
{
	return FALSE;
}

static function bool ShouldGib(Pawn TestPawn, Pawn Instigator)
{
	return FALSE;
}

defaultproperties
{
	bForceRagdollDeath=TRUE
	KillGUDEvent=GUDEvent_KilledEnemyGeneric
	DeathScreamEffortID=GearEffort_FellOffGrapplingHook
}
