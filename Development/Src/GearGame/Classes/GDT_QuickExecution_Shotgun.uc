/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_QuickExecution_Shotgun extends GDT_QuickExecution_Base;

/** Curb stomp always pops off the head. */
static function bool ShouldHeadShotGib(Pawn TestPawn, Pawn Instigator)
{
	return TRUE;
}

