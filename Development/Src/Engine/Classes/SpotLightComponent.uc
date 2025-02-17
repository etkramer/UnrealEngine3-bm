/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SpotLightComponent extends PointLightComponent
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew;

var() float	InnerConeAngle;
var() float OuterConeAngle;

var const DrawLightConeComponent PreviewInnerCone;
var const DrawLightConeComponent PreviewOuterCone;

cpptext
{
	// UActorComponent interface.
	virtual void Attach();

	// ULightComponent interface.
	virtual FLightSceneInfo* CreateSceneInfo() const;
	virtual UBOOL AffectsBounds(const FBoxSphereBounds& Bounds) const;
	virtual FLinearColor GetDirectIntensity(const FVector& Point) const;
	virtual ELightComponentType GetLightType() const;
	virtual void PostLoad();
}

defaultproperties
{
	InnerConeAngle=0
	OuterConeAngle=44
}
