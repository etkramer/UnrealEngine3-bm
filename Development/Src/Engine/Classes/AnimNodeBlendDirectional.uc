/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
// This special blend node will use the LookDir and Acceleration from the Actor that
// owns the SkeletalMeshComponent to blend the four directional animations together.

// TODO: This node could also 'lock' animation rate/phase together for its children (for cadence matching)? Should this be a general blender property?

class AnimNodeBlendDirectional extends AnimNodeBlendBase
	native(Anim)
	hidecategories(Object);

/** Allows control over how quickly the directional blend should be allowed to change. */
var()	float			DirDegreesPerSecond;

/** In radians. Between -PI and PI. 0.0 is running the way we are looking. */
var		float			DirAngle; 

/** If the LOD for the mesh is at or above this LOD level, only use a single directional animation instead of blending. */
var()	int				SingleAnimAtOrAboveLOD;

cpptext
{
	// AnimNode interface
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );

	virtual INT GetNumSliders() const { return 1; }
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);
}

defaultproperties
{
	Children(0)=(Name="Forward",Weight=1.0)
	Children(1)=(Name="Backward")
	Children(2)=(Name="Left")
	Children(3)=(Name="Right")
	bFixNumChildren=true

	DirDegreesPerSecond=360.0

	SingleAnimAtOrAboveLOD=1000
}
