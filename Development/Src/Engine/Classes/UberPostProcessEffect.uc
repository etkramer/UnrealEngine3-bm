/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Uber post process effect
 *
 */
class UberPostProcessEffect extends DOFBloomMotionBlurEffect
	native;

/** */
var() vector SceneShadows;
/** */
var() vector SceneHighLights;
/** */
var() vector SceneMidTones;
/** */
var() float  SceneDesaturation;

cpptext
{
	// UPostProcessEffect interface

	/**
	 * Creates a proxy to represent the render info for a post process effect
	 * @param WorldSettings - The world's post process settings for the view.
	 * @return The proxy object.
	 */
	virtual class FPostProcessSceneProxy* CreateSceneProxy(const FPostProcessSettings* WorldSettings);

	// UObject interface

	/**
	* Called after this instance has been serialized.  UberPostProcessEffect should only
	* ever exists in the SDPG_PostProcess scene
	*/
	virtual void PostLoad();
	
	/**
	 * Called when properties change.  UberPostProcessEffect should only
	 * ever exists in the SDPG_PostProcess scene
	 */
	virtual void PostEditChange(UProperty* PropertyThatChanged);
}

//
// The UberPostProcessingEffect performs DOF, Bloom, Material (Sharpen/Desaturate) and Tone Mapping
//
// For the DOF and Bloom parameters see DOFAndBloomEffect.uc.  The Material parameters are used as
// follows:
//
// Color0 = ((InputColor - SceneShadows) / SceneHighLights) ^ SceneMidTones
// Color1 = Luminance(Color0)
//
// OutputColor = Color0 * (1 - SceneDesaturation) + Color1 * SceneDesaturation
//

defaultproperties
{
    SceneShadows=(X=0.0,Y=0.0,Z=-0.003);
    SceneHighLights=(X=0.8,Y=0.8,Z=0.8);
    SceneMidTones=(X=1.3,Y=1.3,Z=1.3);
    SceneDesaturation=0.4; 
}
