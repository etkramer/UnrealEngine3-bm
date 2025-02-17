/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_CameraSetFOV extends SequenceAction;

var() float DesiredFOV;

defaultproperties
{
	ObjName="Set FOV"
	ObjCategory="Camera"

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")
}
