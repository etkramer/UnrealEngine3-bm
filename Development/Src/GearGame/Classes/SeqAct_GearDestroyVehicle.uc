/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_GearDestroyVehicle extends SequenceAction;

var Actor TargetVehicle;

defaultproperties
{
	ObjName="Destroy Vehicle"
	ObjCategory="Gear"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Vehicle",PropertyName=TargetVehicle)
}
