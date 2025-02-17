
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AnimNodeScalePlayRate extends AnimNodeBlendBase
	native(Anim)
	hidecategories(Object);

var() float	ScaleByValue;

cpptext
{
	virtual void	TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
	virtual FLOAT	GetScaleValue();
}
	
defaultproperties
{
	Children(0)=(Name="Input",Weight=1.0)
	bFixNumChildren=TRUE
	bSkipTickWhenZeroWeight=TRUE
	ScaleByValue=1
}
