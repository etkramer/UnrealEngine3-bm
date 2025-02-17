/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpTrackInstFloatMaterialParam extends InterpTrackInst
	native(Interpolation);

cpptext
{
	virtual void SaveActorState(UInterpTrack* Track);
	virtual void RestoreActorState(UInterpTrack* Track);
}

/** Saved value for restoring state when exiting Matinee. */
var	float		ResetFloat;
