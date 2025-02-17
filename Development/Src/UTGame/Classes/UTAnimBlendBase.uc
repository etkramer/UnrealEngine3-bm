/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAnimBlendBase extends AnimNodeBlendList
	native(Animation);

/** How fast show a given child blend in. */
var(Animation) float BlendTime;

/** Also allow for Blend Overrides */
var(Animation) array<float> ChildBlendTimes;

cpptext
{
	// AnimTree editor interface
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
}

native final function float GetBlendTime(int ChildIndex, optional bool bGetDefault);

/** If child is an AnimNodeSequence, find its duration at current play rate. */
native final function float GetAnimDuration(int ChildIndex);

defaultproperties
{
	BlendTime=0.25
}
