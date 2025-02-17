/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class Rockworm_Fruit extends Rockworm_FruitBase
	placeable;

defaultproperties
{
	Begin Object Class=StaticMeshComponent Name=TheMesh
		StaticMesh=StaticMesh'GOW_CaveFoliage.SM.Mesh.S_GOW_CaveFoliage_WormFood_Top01'
		BlockZeroExtent=true
		CollideActors=true
		BlockRigidBody=false
	End Object
	Components.Add(TheMesh)
	FruitEatPS=ParticleSystem'Locust_Rockworm.Effects.P_Rockworm_Eating_Fruit_Looping'

	// Light component.
	Begin Object Class=PointLightComponent Name=PointLightComponent0
		LightAffectsClassification=LAC_DYNAMIC_AND_STATIC_AFFECTING
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		UseDirectLightMap=FALSE
		LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
		Radius=190.0f
		LightColor=(R=255,G=0,B=0)
	End Object
	Components.Add(PointLightComponent0)
}