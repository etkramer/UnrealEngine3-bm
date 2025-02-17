
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_Slot extends AnimNodeSlot
	native(Anim);

/** 
 * Forces an updated of the child weights.
 * Normally this is done in the tick function, but sometimes it is useful to enforce it.
 * For example when this node has already been ticked, and we want to force a new animation to play during the AnimEnd() event.
 */
final function AccelerateBlend(float BlendAmount)
{
	local int	i;
	local float	BlendDelta;

	for(i=0; i<Children.Length; i++)
	{
		// Amount left to blend.
		BlendDelta = TargetWeight[i] - Children[i].Weight;
		Children[i].Weight += BlendDelta * BlendAmount;
	}
}


defaultproperties
{
	BlendType=ABT_Cubic
}
