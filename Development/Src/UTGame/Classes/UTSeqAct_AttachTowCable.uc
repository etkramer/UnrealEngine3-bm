/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTSeqAct_AttachTowCable extends SequenceAction;

/** vehicle to attach to */
var UTVehicle AttachTo;

defaultproperties
{
	ObjCategory="Vehicle"
	ObjName="Attach Tow Cable"
	VariableLinks[1]=(ExpectedType=class'SeqVar_Object',LinkDesc="Attach To",PropertyName=AttachTo)
}
