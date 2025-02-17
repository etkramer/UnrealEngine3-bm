/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class RB_ConstraintDrawComponent extends PrimitiveComponent
	native(Physics);

cpptext
{
	// Primitive Component interface
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
}

var()	MaterialInterface	LimitMaterial;

defaultproperties
{
}
