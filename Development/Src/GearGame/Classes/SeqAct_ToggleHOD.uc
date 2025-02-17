/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ToggleHOD extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
};


/** TRUE to suppress the GUDS event that alerts the player to the change in HOD status */
var() bool bSuppressAlert;

defaultproperties
{
	bCallHandler=FALSE

	ObjName="Toggle HoD Satellite"
	ObjCategory="Gear"

	bSuppressAlert=FALSE

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")

	VariableLinks.Empty
}
