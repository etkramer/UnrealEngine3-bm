
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearAnim_BlendByReaverLean extends AnimNodeBlendBase
	native(Anim);

var()	float	LeanYawFactor;

var()	float	LeanStrafeFactor;

var()	float	LeanThreshold;

var		float	LeanAmountHistory[10];
var		int		LeanAmountSlot;

cpptext
{
	virtual void	TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
};

defaultproperties
{
	LeanYawFactor=1.0
	LeanStrafeFactor=1.0

	Children(0)=(Name="Left",Weight=0.0)
	Children(1)=(Name="Center",Weight=1.0)
	Children(2)=(Name="Right",Weight=0.0)
	bFixNumChildren=TRUE
}