/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GDO_Trunk extends GearDestructibleObject;

defaultproperties
{
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_434 ObjName=StaticMeshComponent_434 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ChestMesh.Destruct_Chest0a_smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Translation=(X=0.000000,Y=0.000000,Z=4.000000)
      Name="StaticMeshComponent_434"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_434)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_435 ObjName=StaticMeshComponent_435 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ChestMesh.Destruct_Chest0b_smesh'
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
   //Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0 ObjName=StaticMeshComponent_11 Archetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //   Name="StaticMeshComponent_11"
   //   ObjectArchetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //End Object
   //Begin Object Class=SpriteComponent Name=Sprite ObjName=SpriteComponent_12 Archetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //   LightingChannels=(bInitialized=True,Dynamic=True)
   //   Name="SpriteComponent_12"
   //   ObjectArchetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //End Object
   //Begin Object Class=CylinderComponent Name=CollisionCylinder ObjName=CylinderComponent_11 Archetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //   Name="CylinderComponent_11"
   //   ObjectArchetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //End Object
   //AmbientSoundComponent=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_9.AudioComponent_203'
   SubObjects(0)=(Mesh=StaticMeshComponent_434,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ChestMesh.Destruct_Chest1a_smesh'),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ChestMesh.Destruct_Chest2a_smesh')),DefaultHealth=400.000000)
   SubObjects(1)=(Mesh=StaticMeshComponent_435,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ChestMesh.Destruct_Chest1b_smesh'),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ChestMesh.Destruct_Chest2b_smesh')),DefaultHealth=400.000000)
   SubObjectHealths(0)=400.000000
   SubObjectHealths(1)=400.000000
   //Components(0)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_9.StaticMeshComponent_11'
   //Components(1)=SpriteComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_9.SpriteComponent_12'
   //Components(2)=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_9.AudioComponent_203'
   //Components(3)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_9.StaticMeshComponent_434'
   //Components(4)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_9.StaticMeshComponent_435'
   //Tag="GearDestructibleObject"
   //Location=(X=-832.000000,Y=1872.000000,Z=0.000000)
   //Name="GearDestructibleObject_9"
   //ObjectArchetype=GearDestructibleObject'GearGame.Default__GearDestructibleObject'


}


