/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_GearExitVehicle extends SequenceAction;

var Actor TargetVehicle;

defaultproperties
{
	ObjName="Exit Vehicle"
	ObjCategory="Gear"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Vehicle",PropertyName=TargetVehicle)
}
