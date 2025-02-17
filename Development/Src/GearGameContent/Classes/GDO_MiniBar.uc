/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GDO_MiniBar extends GearDestructibleObject;

defaultproperties
{
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_494 ObjName=StaticMeshComponent_494 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar0a_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Translation=(X=0.000000,Y=0.000000,Z=102.000000)
      Name="StaticMeshComponent_494"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_494)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_495 ObjName=StaticMeshComponent_495 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar0b_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Translation=(X=0.000000,Y=0.000000,Z=102.000000)
      Name="StaticMeshComponent_495"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_495)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_496 ObjName=StaticMeshComponent_496 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar0c_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Translation=(X=0.000000,Y=0.000000,Z=102.000000)
      Name="StaticMeshComponent_496"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_496)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_497 ObjName=StaticMeshComponent_497 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar0d_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Translation=(X=0.000000,Y=0.000000,Z=102.000000)
      Name="StaticMeshComponent_497"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_497)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_498 ObjName=StaticMeshComponent_498 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar0e_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Translation=(X=0.000000,Y=0.000000,Z=102.000000)
      Name="StaticMeshComponent_498"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_498)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_499 ObjName=StaticMeshComponent_499 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar0f_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Translation=(X=0.000000,Y=0.000000,Z=102.000000)
      Name="StaticMeshComponent_499"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_499)
   //Begin Object Class=AudioComponent Name=AmbientSoundComponent0 ObjName=AudioComponent_287 Archetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //   Name="AudioComponent_287"
   //   ObjectArchetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //End Object
   //Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0 ObjName=StaticMeshComponent_15 Archetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //   Name="StaticMeshComponent_15"
   //   ObjectArchetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //End Object
   //Begin Object Class=SpriteComponent Name=Sprite ObjName=SpriteComponent_16 Archetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //   LightingChannels=(bInitialized=True,Dynamic=True)
   //   Name="SpriteComponent_16"
   //   ObjectArchetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //End Object
   //Begin Object Class=CylinderComponent Name=CollisionCylinder ObjName=CylinderComponent_14 Archetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //   Name="CylinderComponent_14"
   //   ObjectArchetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //End Object
   //AmbientSoundComponent=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_10.AudioComponent_287'
   SubObjects(0)=(SubObjName="BottomLeft",Mesh=StaticMeshComponent_494,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar1a_Smesh',DependentSubObjs=((DependentSubObjName="TopLeft",MaxHealthToAllow=200.000000),(DependentSubObjName="TopMiddle",MaxHealthToAllow=200.000000))),(NewMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar2a_Smesh',DependentSubObjs=((DependentSubObjName="TopLeft")))),DefaultHealth=400.000000)
   SubObjects(1)=(SubObjName="BottomMiddle",Mesh=StaticMeshComponent_495,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar1b_Smesh',DependentSubObjs=((DependentSubObjName="TopLeft",MaxHealthToAllow=200.000000),(DependentSubObjName="TopMiddle",MaxHealthToAllow=200.000000),(DependentSubObjName="TopRight",MaxHealthToAllow=200.000000))),(NewMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar2b_Smesh',DependentSubObjs=((DependentSubObjName="TopMiddle")))),DefaultHealth=400.000000)
   SubObjects(2)=(SubObjName="BottomRight",Mesh=StaticMeshComponent_496,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar1c_Smesh',DependentSubObjs=((DependentSubObjName="TopRight",MaxHealthToAllow=200.000000),(DependentSubObjName="TopMiddle",MaxHealthToAllow=200.000000))),(NewMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar2c_Smesh',DependentSubObjs=((DependentSubObjName="TopRight")))),DefaultHealth=400.000000)
   SubObjects(3)=(SubObjName="TopLeft",Mesh=StaticMeshComponent_497,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar1d_Smesh'),(bSelfDestruct=True)),DefaultHealth=400.000000)
   SubObjects(4)=(SubObjName="TopMiddle",Mesh=StaticMeshComponent_498,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar1e_Smesh'),(bSelfDestruct=True)),DefaultHealth=400.000000)
   SubObjects(5)=(SubObjName="TopRight",Mesh=StaticMeshComponent_499,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture.MiniBarMesh.Destruct_Minibar1f_Smesh'),(bSelfDestruct=True)),DefaultHealth=400.000000)
   SubObjectHealths(0)=400.000000
   SubObjectHealths(1)=400.000000
   SubObjectHealths(2)=400.000000
   SubObjectHealths(3)=400.000000
   SubObjectHealths(4)=400.000000
   SubObjectHealths(5)=400.000000
   //Components(0)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_10.StaticMeshComponent_15'
   //Components(1)=SpriteComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_10.SpriteComponent_16'
   //Components(2)=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_10.AudioComponent_287'
   //Components(3)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_10.StaticMeshComponent_494'
   //Components(4)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_10.StaticMeshComponent_495'
   //Components(5)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_10.StaticMeshComponent_496'
   //Components(6)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_10.StaticMeshComponent_497'
   //Components(7)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_10.StaticMeshComponent_498'
   //Components(8)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_10.StaticMeshComponent_499'
   //Tag="GearDestructibleObject"
   //Location=(X=640.000000,Y=2656.000000,Z=0.000000)
   //Name="GearDestructibleObject_10"
   //ObjectArchetype=GearDestructibleObject'GearGame.Default__GearDestructibleObject'

}


