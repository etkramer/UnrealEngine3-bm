/**
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GDO_Cog_Truck01 extends GearDestructibleObject;


DefaultProperties
{
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_625 ObjName=StaticMeshComponent_625 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
		StaticMesh=StaticMesh'COG_Truck01_SM.SM.Mesh.S_COG_Truck01_SM'
		LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
		LightEnvironment=MyLightEnvironment
		Translation=(X=0.000000,Y=0.000000,Z=48.000000)
		ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
	End Object
	Components.Add(StaticMeshComponent_625)

	SubObjects(0)=(Mesh=StaticMeshComponent_625,DefaultHealth=1000.000000)
	SubObjectHealths(0)=1000.000000
}