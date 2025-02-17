/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_SetMatInstVectorParam extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
}

var() MaterialInstanceConstant	MatInst;
var() Name						ParamName;

var() LinearColor VectorValue;

defaultproperties
{
	ObjName="Set VectorParam"
	ObjCategory="Material Instance"
	VariableLinks.Empty
}
