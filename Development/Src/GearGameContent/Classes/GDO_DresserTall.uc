/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GDO_DresserTall extends GearDestructibleObject;

defaultproperties
{
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_432 ObjName=StaticMeshComponent_432 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS3.FileCabinetMesh.Destruct_FileCabinet0a_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_432"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_432)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_1765 ObjName=StaticMeshComponent_1765 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS3.FileCabinetMesh.Destruct_FileCabinet0b_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_1765"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_1765)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_1766 ObjName=StaticMeshComponent_1766 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS3.FileCabinetMesh.Destruct_FileCabinet0c_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_1766"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_1766)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_1767 ObjName=StaticMeshComponent_1767 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS3.FileCabinetMesh.Destruct_FileCabinet0d_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_1767"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_1767)
   //Begin Object Class=AudioComponent Name=AmbientSoundComponent0 ObjName=AudioComponent_202 Archetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //   Name="AudioComponent_202"
   //   ObjectArchetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //End Object
   //Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0 ObjName=StaticMeshComponent_4 Archetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //   Name="StaticMeshComponent_4"
   //   ObjectArchetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //End Object
   //Begin Object Class=SpriteComponent Name=Sprite ObjName=SpriteComponent_5 Archetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //   LightingChannels=(bInitialized=True,Dynamic=True)
   //   Name="SpriteComponent_5"
   //   ObjectArchetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //End Object
   //Begin Object Class=CylinderComponent Name=CollisionCylinder ObjName=CylinderComponent_4 Archetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //   Name="CylinderComponent_4"
   //   ObjectArchetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //End Object
   //AmbientSoundComponent=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_3.AudioComponent_202'
   SubObjects(0)=(Mesh=StaticMeshComponent_432,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.FileCabinetMesh.Destruct_FileCabinet1a_Smesh',DependentSubObjs=((DependentSubObjName="TopLeft",MaxHealthToAllow=200.000000))),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.FileCabinetMesh.Destruct_FileCabinet2a_Smesh',DependentSubObjs=((DependentSubObjName="TopLeft")))),DefaultHealth=400.000000)
   SubObjects(1)=(Mesh=StaticMeshComponent_1765,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.FileCabinetMesh.Destruct_FileCabinet1b_Smesh',DependentSubObjs=((DependentSubObjName="TopRight",MaxHealthToAllow=200.000000))),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.FileCabinetMesh.Destruct_FileCabinet2b_Smesh',DependentSubObjs=((DependentSubObjName="TopRight")))),DefaultHealth=400.000000)
   SubObjects(2)=(SubObjName="TopLeft",Mesh=StaticMeshComponent_1766,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.FileCabinetMesh.Destruct_FileCabinet1c_Smesh'),(bSelfDestruct=True)),DefaultHealth=400.000000)
   SubObjects(3)=(SubObjName="TopRight",Mesh=StaticMeshComponent_1767,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS3.FileCabinetMesh.Destruct_FileCabinet1d_Smesh'),(bSelfDestruct=True)),DefaultHealth=400.000000)
   SubObjectHealths(0)=400.000000
   SubObjectHealths(1)=400.000000
   SubObjectHealths(2)=400.000000
   SubObjectHealths(3)=400.000000
   //Components(0)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_3.StaticMeshComponent_4'
   //Components(1)=SpriteComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_3.SpriteComponent_5'
   //Components(2)=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_3.AudioComponent_202'
   //Components(3)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_3.StaticMeshComponent_432'
   //Components(4)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_3.StaticMeshComponent_1765'
   //Components(5)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_3.StaticMeshComponent_1766'
   //Components(6)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_3.StaticMeshComponent_1767'
   //Tag="GearDestructibleObject"
   //Location=(X=-768.000000,Y=1024.000000,Z=-0.000001)
   //Name="GearDestructibleObject_3"
   //ObjectArchetype=GearDestructibleObject'GearGame.Default__GearDestructibleObject'

}


