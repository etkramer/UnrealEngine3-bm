/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class DirectionalLight extends Light
	native
	placeable;

cpptext
{
public:
	/**
	 * This will determine which icon should be displayed for this light.
	 **/
	virtual void DetermineAndSetEditorIcon();
}


defaultproperties
{
	Begin Object Name=Sprite
		Sprite=Texture2D'EditorResources.LightIcons.Light_Directional_Stationary_DynamicsAndStatics'
	End Object
	
	Begin Object Class=DirectionalLightComponent Name=DirectionalLightComponent0
	    LightAffectsClassification=LAC_DYNAMIC_AND_STATIC_AFFECTING

	    CastShadows=TRUE
	    CastStaticShadows=TRUE
	    CastDynamicShadows=TRUE
	    bForceDynamicLight=FALSE
	    UseDirectLightMap=TRUE

	    LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
	End Object
	LightComponent=DirectionalLightComponent0
	Components.Add(DirectionalLightComponent0)

	Begin Object Class=ArrowComponent Name=ArrowComponent0
		ArrowColor=(R=150,G=200,B=255)
		bTreatAsASprite=True
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(ArrowComponent0)

	Rotation=(Pitch=-16384,Yaw=0,Roll=0)
	DesiredRotation=(Pitch=-16384,Yaw=0,Roll=0)
}
