/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_CraneForceUp extends SequenceAction;

/** Crane is forced to pitch above this number. */
var() float		ForceAbovePitch;

defaultproperties
{
	ObjName="Crane Force Up"
	ObjCategory="Gear"

	ForceAbovePitch=0.0
}
