/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GDO_DiningTable extends GearDestructibleObject;

defaultproperties
{
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_434 ObjName=StaticMeshComponent_434 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS3.DiningtableMesh.Destruct_Diningtable0a_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Translation=(X=0.000000,Y=0.000000,Z=4.000000)
      Name="StaticMeshComponent_434"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_434)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_338 ObjName=StaticMeshComponent_338 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS3.DiningtableMesh.Destruct_Diningtable0b_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_338"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_338)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_435 ObjName=StaticMeshComponent_435 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS3.DiningtableMesh.Destruct_Diningtable0c_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Translation=(X=0.000000,Y=0.000000,Z=4.000000)
      Name="StaticMeshComponent_435"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_435)
   //Begin Object Class=AudioComponent Name=AmbientSoundComponent0 ObjName=AudioComponent_203 Archetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //   Name="AudioComponent_203"
   //   ObjectArchetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //End Object
   //Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0 ObjName=StaticMeshComponent_8 Archetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //   Name="StaticMeshComponent_8"
   //   ObjectArchetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //End Object
   //Begin Object Class=SpriteComponent Name=Sprite ObjName=SpriteComponent_9 Archetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //   LightingChannels=(bInitialized=True,Dynamic=True)
   //   Name="SpriteComponent_9"
   //   ObjectArchetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //End Object
   //Begin Object Class=CylinderComponent Name=CollisionCylinder ObjName=CylinderComponent_8 Archetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //   Name="CylinderComponent_8"
   //   ObjectArchetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //End Object
   //AmbientSoundComponent=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_14.AudioComponent_203'
   SubObjects(0)=(Mesh=StaticMeshComponent_434,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.DiningtableMesh.Destruct_Diningtable1a_Smesh'),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.DiningtableMesh.Destruct_Diningtable2a_Smesh')),DefaultHealth=400.000000)
   SubObjects(1)=(Mesh=StaticMeshComponent_338,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.DiningtableMesh.Destruct_Diningtable1b_Smesh'),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.DiningtableMesh.Destruct_Diningtable2b_Smesh')),DefaultHealth=400.000000)
   SubObjects(2)=(Mesh=StaticMeshComponent_435,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.DiningtableMesh.Destruct_Diningtable1c_Smesh'),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.DiningtableMesh.Destruct_Diningtable2c_Smesh')),DefaultHealth=400.000000)
   SubObjectHealths(0)=400.000000
   SubObjectHealths(1)=400.000000
   SubObjectHealths(2)=400.000000
   //Components(0)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_14.StaticMeshComponent_8'
   //Components(1)=SpriteComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_14.SpriteComponent_9'
   //Components(2)=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_14.AudioComponent_203'
   //Components(3)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_14.StaticMeshComponent_434'
   //Components(4)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_14.StaticMeshComponent_338'
   //Components(5)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_14.StaticMeshComponent_435'
   //Tag="GearDestructibleObject"
   //Location=(X=831.999939,Y=1296.000000,Z=0.000000)
   //Name="GearDestructibleObject_14"
   //ObjectArchetype=GearDestructibleObject'GearGame.Default__GearDestructibleObject'

}


