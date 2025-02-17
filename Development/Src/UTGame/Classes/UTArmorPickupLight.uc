/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTArmorPickupLight extends UTPickupLight
	native
	placeable;

defaultproperties
{
	Begin Object Class=PointLightComponent Name=PointLightComponent0
	    LightAffectsClassification=LAC_STATIC_AFFECTING
		CastShadows=TRUE
		CastStaticShadows=TRUE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		UseDirectLightMap=TRUE
		LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=FALSE,bInitialized=TRUE)
		Brightness=2
		LightColor=(R=251,G=239,B=155)
		Radius=128
	End Object
	LightComponent=PointLightComponent0
	Components.Add(PointLightComponent0)
}
