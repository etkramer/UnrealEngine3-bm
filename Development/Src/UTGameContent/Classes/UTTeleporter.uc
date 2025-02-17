/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** UT version of the teleporter with custom effects */
class UTTeleporter extends UTTeleporterBase
	hidecategories(SceneCapture);

/** component that plays the render-to-texture portal effect */
var ParticleSystemComponent PortalEffect;

/**
 * The base of the teleporter.  We hold a reference to it so that
 * it gets serialized to disk, and so we can statically light it.
 */
var StaticMeshComponent TeleporterBaseMesh;

simulated function InitializePortalEffect(Actor Dest)
{
	Super.InitializePortalEffect(Dest);
	if (Dest != None)
	{
		SceneCapture2DComponent(PortalCaptureComponent).SetCaptureParameters(TextureTarget);
		SceneCapture2DComponent(PortalCaptureComponent).SetView(Dest.Location + vector(Dest.Rotation) * 15.0, Dest.Rotation);
		PortalEffect.SetMaterialParameter('Portal', PortalMaterialInstance);
	}
}

defaultproperties
{
	Components.Remove(Sprite)

	Begin Object Class=SceneCapture2DComponent Name=SceneCapture2DComponent0
		FrameRate=15.0
		bSkipUpdateIfOwnerOccluded=true
		MaxUpdateDist=1000.0
		MaxStreamingUpdateDist=1000.0
		bUpdateMatrices=false
		NearPlane=10
		FarPlane=-1
	End Object
	PortalCaptureComponent=SceneCapture2DComponent0

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		StaticMesh=StaticMesh'GP_Onslaught.Mesh.S_GP_Ons_Conduit'
		CollideActors=true
		BlockActors=true
		CastShadow=true
		bCastDynamicShadow=false
		bForceDirectLightMap=true
		LightingChannels=(BSP=TRUE,Dynamic=FALSE,Static=TRUE,CompositeDynamic=FALSE)
		Translation=(X=0.0,Y=0.0,Z=-34.0)
		Scale=0.5
		bUseAsOccluder=false
		BlockNonZeroExtent=false
	End Object
 	Components.Add(StaticMeshComponent0)
 	TeleporterBaseMesh=StaticMeshComponent0

	Begin Object Class=ParticleSystemComponent Name=ParticleSystemComponent0
		Translation=(X=0.0,Y=0.0,Z=-40.0)
		Template=ParticleSystem'Pickups.Base_Teleporter.Effects.P_Pickups_Teleporter_Base_Idle'
	End Object
	Components.Add(ParticleSystemComponent0)

	Begin Object Class=ParticleSystemComponent Name=ParticleSystemComponent1
		Template=ParticleSystem'Pickups.Base_Teleporter.Effects.P_Pickups_Teleporter_Idle'
	End Object
	Components.Add(ParticleSystemComponent1)
	PortalEffect=ParticleSystemComponent1
	PortalMaterial=MaterialInterface'Pickups.Base_Teleporter.Material.M_T_Pickups_Teleporter_Portal_Destination'
}
