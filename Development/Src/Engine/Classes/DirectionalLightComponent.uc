/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class DirectionalLightComponent extends LightComponent
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew;

/**
 * Trace distance for static lighting. Objects further than TraceDistance away from an object won't be taken into 
 * account for static shadowing applied to said object. This is used to work around floating point consistency
 * issues in the collision code with regard to very long traces. The old default was WORLD_MAX.
 */
var(AdvancedLighting)	float	TraceDistance;

cpptext
{
	virtual FLightSceneInfo* CreateSceneInfo() const;
	virtual FVector4 GetPosition() const;
	virtual ELightComponentType GetLightType() const;
}

/** Called from matinee code when LightColor property changes. */
function OnUpdatePropertyLightColor()
{
	UpdateColorAndBrightness();
}

/** Called from matinee code when Brightness property changes. */
function OnUpdatePropertyBrightness()
{
	UpdateColorAndBrightness();
}

defaultproperties
{
	TraceDistance=100000
}
