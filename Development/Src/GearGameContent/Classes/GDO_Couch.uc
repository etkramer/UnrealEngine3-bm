/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GDO_Couch extends GearDestructibleObject;

defaultproperties
{
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_6 ObjName=StaticMeshComponent_6 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture.FancyCouchMesh.Destruct_FancyCouch0a_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Translation=(X=0.000000,Y=0.000000,Z=48.000000)
      Name="StaticMeshComponent_6"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_6)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_7 ObjName=StaticMeshComponent_7 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture.FancyCouchMesh.Destruct_FancyCouch0b_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Translation=(X=0.000000,Y=0.000000,Z=48.000000)
      Name="StaticMeshComponent_7"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_7)
   //Begin Object Class=AudioComponent Name=AmbientSoundComponent0 ObjName=AudioComponent_0 Archetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //   Name="AudioComponent_0"
   //   ObjectArchetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //End Object
   //Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0 ObjName=StaticMeshComponent_13 Archetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //   Name="StaticMeshComponent_13"
   //   ObjectArchetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //End Object
   //Begin Object Class=SpriteComponent Name=Sprite ObjName=SpriteComponent_14 Archetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //   LightingChannels=(bInitialized=True,Dynamic=True)
   //   Name="SpriteComponent_14"
   //   ObjectArchetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //End Object
   //Begin Object Class=CylinderComponent Name=CollisionCylinder ObjName=CylinderComponent_13 Archetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //   Name="CylinderComponent_13"
   //   ObjectArchetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //End Object
   //AmbientSoundComponent=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_0.AudioComponent_0'
   SubObjects(0)=(Mesh=StaticMeshComponent_6,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture.FancyCouchMesh.Destruct_FancyCouch1a_Smesh'),(bSelfDestruct=True)),DefaultHealth=400.000000)
   SubObjects(1)=(Mesh=StaticMeshComponent_7,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture.FancyCouchMesh.Destruct_FancyCouch1b_Smesh'),(bSelfDestruct=True)),DefaultHealth=400.000000)
   SubObjectHealths(0)=400.000000
   SubObjectHealths(1)=400.000000
   //Components(0)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_0.StaticMeshComponent_13'
   //Components(1)=SpriteComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_0.SpriteComponent_14'
   //Components(2)=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_0.AudioComponent_0'
   //Components(3)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_0.StaticMeshComponent_6'
   //Components(4)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_0.StaticMeshComponent_7'
   //Tag="GearDestructibleObject"
   //Location=(X=-48.000000,Y=1008.000000,Z=0.000000)
   //Name="GearDestructibleObject_0"
   //ObjectArchetype=GearDestructibleObject'GearGame.Default__GearDestructibleObject'


}


