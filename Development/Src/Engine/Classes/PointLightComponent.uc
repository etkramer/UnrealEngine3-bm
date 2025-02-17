/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class PointLightComponent extends LightComponent
	native
	collapsecategories
	hidecategories(Object)
	editinlinenew;

/** used to control when point light shadow mapping goes to a hack mode, the ShadowRadiusMultiplier is multiplied by the radius of object's bounding sphere */
var() interp float	ShadowRadiusMultiplier;

var() interp float	Radius;
/** Controls the radial falloff of the light */
var() interp float	FalloffExponent;
/** falloff for shadow when using LightShadow_Modulate */
var() float ShadowFalloffExponent;
/** The minimum radius at which the point light's shadow begins to attenuate. */
var() float MinShadowFalloffRadius;

var   const matrix							CachedParentToWorld; //@todo remove me please
var() const vector							Translation;

var const DrawLightRadiusComponent PreviewLightRadius;

cpptext
{
protected:
	/**
	 * Updates the light's PreviewLightRadius.
	 */
	void UpdatePreviewLightRadius();

	// UActorComponent interface.
	virtual void SetParentToWorld(const FMatrix& ParentToWorld);
	virtual void Attach();
	virtual void UpdateTransform();
public:

	// ULightComponent interface.
	virtual FLightSceneInfo* CreateSceneInfo() const;
	virtual UBOOL AffectsBounds(const FBoxSphereBounds& Bounds) const;
	virtual FVector4 GetPosition() const;
	virtual FBox GetBoundingBox() const;
	virtual FLinearColor GetDirectIntensity(const FVector& Point) const;
	virtual ELightComponentType GetLightType() const;

	// update the LocalToWorld matrix
	virtual void SetTransformedToWorld();

	/**
	 * Called after property has changed via e.g. property window or set command.
	 *
	 * @param	PropertyThatChanged	UProperty that has been changed, NULL if unknown
	 */
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	virtual void PostLoad();
}

native final function SetTranslation(vector NewTranslation);

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
	Radius=1024.0
	FalloffExponent=2
	ShadowFalloffExponent=2
	ShadowRadiusMultiplier=1.1
}
