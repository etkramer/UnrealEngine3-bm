/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GDO_Armoire extends GearDestructibleObject;

defaultproperties
{
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_1140 ObjName=StaticMeshComponent_1140 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ArmoireMesh.Destruct_Armoire0c_Smesh'
      LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_1140"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_1140)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_1141 ObjName=StaticMeshComponent_1141 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ArmoireMesh.Destruct_Armoire0d_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_1141"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_1141)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_1142 ObjName=StaticMeshComponent_1142 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ArmoireMesh.Destruct_Armoire0a_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_1142"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_1142)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_1143 ObjName=StaticMeshComponent_1143 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ArmoireMesh.Destruct_Armoire0b_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_1143"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_1143)
   //Begin Object Class=AudioComponent Name=AmbientSoundComponent0 ObjName=AudioComponent_483 Archetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //   Name="AudioComponent_483"
   //   ObjectArchetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //End Object
   //Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0 ObjName=StaticMeshComponent_12 Archetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //   Name="StaticMeshComponent_12"
   //   ObjectArchetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //End Object
   //Begin Object Class=SpriteComponent Name=Sprite ObjName=SpriteComponent_13 Archetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //   LightingChannels=(bInitialized=True,Dynamic=True)
   //   Name="SpriteComponent_13"
   //   ObjectArchetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //End Object
   //Begin Object Class=CylinderComponent Name=CollisionCylinder ObjName=CylinderComponent_12 Archetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //   Name="CylinderComponent_12"
   //   ObjectArchetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //End Object
   //AmbientSoundComponent=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_8.AudioComponent_483'
   SubObjects(0)=(SubObjName="TopLeft",Mesh=StaticMeshComponent_1140,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ArmoireMesh.Destruct_Armoire1c_Smesh'),(bSelfDestruct=True)),DefaultHealth=400.000000)
   SubObjects(1)=(SubObjName="TopRight",Mesh=StaticMeshComponent_1141,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ArmoireMesh.Destruct_Armoire1d_Smesh'),(bSelfDestruct=True)),DefaultHealth=400.000000)
   SubObjects(2)=(SubObjName="BottomLeft",Mesh=StaticMeshComponent_1142,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ArmoireMesh.Destruct_Armoire1a_Smesh',DependentSubObjs=((DependentSubObjName="TopLeft",MaxHealthToAllow=200.000000))),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ArmoireMesh.Destruct_Armoire2a_Smesh',DependentSubObjs=((DependentSubObjName="TopLeft")))),DefaultHealth=400.000000)
   SubObjects(3)=(SubObjName="BottomRight",Mesh=StaticMeshComponent_1143,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ArmoireMesh.Destruct_Armoire1b_Smesh',DependentSubObjs=((DependentSubObjName="TopRight",MaxHealthToAllow=200.000000))),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.ArmoireMesh.Destruct_Armoire2b_Smesh',DependentSubObjs=((DependentSubObjName="TopRight")))),DefaultHealth=400.000000)
   SubObjectHealths(0)=400.000000
   SubObjectHealths(1)=400.000000
   SubObjectHealths(2)=400.000000
   SubObjectHealths(3)=400.000000
   //Components(0)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_8.StaticMeshComponent_12'
   //Components(1)=SpriteComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_8.SpriteComponent_13'
   //Components(2)=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_8.AudioComponent_483'
   //Components(3)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_8.StaticMeshComponent_1140'
   //Components(4)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_8.StaticMeshComponent_1141'
   //Components(5)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_8.StaticMeshComponent_1142'
   //Components(6)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_8.StaticMeshComponent_1143'
   //Tag="GearDestructibleObject"
   //Location=(X=-864.000000,Y=1488.000000,Z=0.000000)
   //Rotation=(Pitch=0,Yaw=32769,Roll=0)
   //Name="GearDestructibleObject_8"
   //ObjectArchetype=GearDestructibleObject'GearGame.Default__GearDestructibleObject'
}


