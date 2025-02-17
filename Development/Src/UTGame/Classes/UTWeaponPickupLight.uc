/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTWeaponPickupLight extends UTPickupLight
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
		Brightness=4
		LightColor=(R=251,G=244,B=200)
		Radius=128
	End Object
	LightComponent=PointLightComponent0
	Components.Add(PointLightComponent0)
}
