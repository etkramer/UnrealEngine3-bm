/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_OverrideRedReticleTraceDistance extends SequenceAction;

/** How far from the muzzle to trace to detect enemies when trying to turn the crosshair red. */
var() float TraceDistance;

defaultproperties
{
	ObjName="Override Reticle Trace Dist"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Set")
	InputLinks(1)=(LinkDesc="Restore Default")
}
