/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTAnimNodeFramePlayer extends AnimNodeSequence
	native(Animation);

native function SetAnimation(name Sequence, float RateScale);
native function SetAnimPosition(float Perc);

defaultproperties
{
}
