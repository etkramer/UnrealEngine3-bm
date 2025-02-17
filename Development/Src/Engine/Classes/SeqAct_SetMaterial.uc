/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_SetMaterial extends SequenceAction
	native(Sequence);

/** Material to apply to target when action is activated. */
var()	MaterialInterface	NewMaterial;

/** Index in the Materials array to replace with NewMaterial when this action is activated. */
var()	INT					MaterialIndex;

cpptext
{
	// UObject interface.
	virtual void PostEditChange(UProperty* PropertyThatChanged);
}

defaultproperties
{
	ObjName="Set Material"
	ObjCategory="Actor"
}