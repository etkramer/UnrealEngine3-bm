/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class LensFlareComponent extends PrimitiveComponent
	native(LensFlare)
	hidecategories(Object)
	hidecategories(Physics)
	hidecategories(Collision)
	editinlinenew
	dependson(LensFlare);

var()	const			LensFlare							Template;
var		const			DrawLightConeComponent				PreviewInnerCone;
var		const			DrawLightConeComponent				PreviewOuterCone;
var		const			DrawLightRadiusComponent			PreviewRadius;

struct LensFlareElementInstance
{
	// No UObject reference
};

/** If TRUE, automatically enable this flare when it is attached */
var()								bool					bAutoActivate;

/** Internal variables */
var transient						bool					bIsActive;
var	transient						bool					bHasTranslucency;
var	transient						bool					bHasUnlitTranslucency;
var	transient						bool					bHasUnlitDistortion;
var	transient						bool					bUsesSceneColor;

/** Viewing cone angles. */
var transient						float					OuterCone;
var transient						float					InnerCone;
var transient						float					ConeFudgeFactor;
var transient						float					Radius;

/** The color of the source	*/
var(Rendering)						linearcolor				SourceColor;

/** Command fence used to shut down properly */
var		native				const	pointer					ReleaseResourcesFence{class FRenderCommandFence};

native final function SetTemplate(LensFlare NewTemplate);
native		 function SetSourceColor(linearcolor InSourceColor);
native		 function SetIsActive(bool bInIsActive);

cpptext
{
	// UObject interface
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();
	virtual void FinishDestroy();
	virtual void PreEditChange(UProperty* PropertyThatWillChange);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PostLoad();

	// UActorComponent interface.
	virtual void Attach();

public:
	// UPrimitiveComponent interface
	virtual void UpdateBounds();
	
	/** Returns true if the prim is using a material with unlit distortion */
	virtual UBOOL HasUnlitDistortion() const;
	/** Returns true if the prim is using a material with unlit translucency */
	virtual UBOOL HasUnlitTranslucency() const;

	/**
	* Returns true if the prim is using a material that samples the scene color texture.
	* If true then these primitives are drawn after all other translucency
	*/
	virtual UBOOL UsesSceneColor() const;

	virtual FPrimitiveSceneProxy* CreateSceneProxy();

	// InstanceParameters interface
	void	AutoPopulateInstanceProperties();
}

defaultproperties
{
	bAutoActivate=true
	bTickInEditor=true
	TickGroup=TG_PostAsyncWork
	bAllowApproximateOcclusion=false
	bFirstFrameOcclusion=true
	bIgnoreNearPlaneIntersection=true

	SourceColor=(R=1.0,G=1.0,B=1.0,A=1.0)
}
