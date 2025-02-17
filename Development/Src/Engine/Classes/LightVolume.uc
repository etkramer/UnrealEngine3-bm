/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Used to associate lights with volumes.
 */
class LightVolume extends Volume
	native
	placeable;

cpptext
{
	/**
	 * Called after property has changed via e.g. property window or set command.
	 *
	 * @param	PropertyThatChanged	UProperty that has been changed, NULL if unknown
	 */
	virtual void PostEditChange(UProperty* PropertyThatChanged);
}

defaultproperties
{
	Begin Object Name=BrushComponent0
		CollideActors=False
		BlockActors=False
		BlockZeroExtent=False
		BlockNonZeroExtent=False
		BlockRigidBody=False
	End Object

	bCollideActors=False
	bBlockActors=False
	bProjTarget=False
	SupportedEvents.Empty
}
