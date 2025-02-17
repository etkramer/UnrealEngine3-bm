/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GDO_ArmChair extends GearDestructibleObject;

defaultproperties
{
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_1786 ObjName=StaticMeshComponent_1786 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS3.ArmchairMesh.Destruct_Armchair0_Smesh'
      LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_1786"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_1786)
   //Begin Object Class=AudioComponent Name=AmbientSoundComponent0 ObjName=AudioComponent_2526 Archetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //   Name="AudioComponent_2526"
   //   ObjectArchetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //End Object
   //Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0 ObjName=StaticMeshComponent_6 Archetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //   Name="StaticMeshComponent_6"
   //   ObjectArchetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //End Object
   //Begin Object Class=SpriteComponent Name=Sprite ObjName=SpriteComponent_7 Archetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //   LightingChannels=(bInitialized=True,Dynamic=True)
   //   Name="SpriteComponent_7"
   //   ObjectArchetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //End Object
   //Begin Object Class=CylinderComponent Name=CollisionCylinder ObjName=CylinderComponent_6 Archetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //   Name="CylinderComponent_6"
   //   ObjectArchetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //End Object
   //AmbientSoundComponent=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_7.AudioComponent_2526'
   SubObjects(0)=(Mesh=StaticMeshComponent_1786,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.ArmchairMesh.Destruct_Armchair1_Smesh'),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.ArmchairMesh.Destruct_Armchair2_Smesh',bSelfDestruct=True)),DefaultHealth=400.000000)
   SubObjectHealths(0)=400.000000
   //Components(0)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_7.StaticMeshComponent_6'
   //Components(1)=SpriteComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_7.SpriteComponent_7'
   //Components(2)=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_7.AudioComponent_2526'
   //Components(3)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_7.StaticMeshComponent_1786'
   //Tag="GearDestructibleObject"
   //Location=(X=592.000000,Y=1712.000000,Z=0.000002)
   //Name="GearDestructibleObject_7"
   //ObjectArchetype=GearDestructibleObject'GearGame.Default__GearDestructibleObject'


//Begin Object Class=AudioComponent Name=AudioComponent_2526 ObjName=AudioComponent_2526 Archetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
//   Name="AudioComponent_2526"
//   ObjectArchetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
//End Object
//
//Begin Object Class=CylinderComponent Name=CylinderComponent_6 ObjName=CylinderComponent_6 Archetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
//   Name="CylinderComponent_6"
//   ObjectArchetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
//End Object


}


