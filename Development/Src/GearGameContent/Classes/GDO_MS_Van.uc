/**
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GDO_MS_Van extends GearDestructibleObject;


DefaultProperties
{
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_627 ObjName=StaticMeshComponent_627 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
		StaticMesh=StaticMesh'GOW_Vehicles.SM.Mesh.Van.MS_Van_SMesh'
		LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
		LightEnvironment=MyLightEnvironment
		Translation=(X=0.000000,Y=0.000000,Z=48.000000)
		ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
	End Object
	Components.Add(StaticMeshComponent_627)

	SubObjects(0)=(Mesh=StaticMeshComponent_627,DefaultHealth=1000.000000)
	SubObjectHealths(0)=1000.000000
}