/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GrenadeBlockingVolume extends BlockingVolume
	native;

cpptext
{
	virtual UBOOL IgnoreBlockingBy( const AActor *Other ) const;
}

defaultproperties
{
	bColored=TRUE
	BrushColor=(R=60,G=255,B=200,A=255)
	bWorldGeometry=FALSE
	bProjTarget=TRUE

	Begin Object Name=BrushComponent0
		BlockZeroExtent=TRUE
		BlockRigidBody=FALSE
		bBlockComplexCollisionTrace=TRUE
	End Object
}