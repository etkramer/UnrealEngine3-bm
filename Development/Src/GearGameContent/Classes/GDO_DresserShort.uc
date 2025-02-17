/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GDO_DresserShort extends GearDestructibleObject;

defaultproperties
{
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_41 ObjName=StaticMeshComponent_41 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture.DresserMesh.Destruct_Dresser0a_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_41"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_41)
   Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_42 ObjName=StaticMeshComponent_42 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
      StaticMesh=StaticMesh'Destruct_SLS_Furniture.DresserMesh.Destruct_Dresser0b_Smesh'
	  LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
	  LightEnvironment=MyLightEnvironment
      Name="StaticMeshComponent_42"
      ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
   End Object
   Components.Add(StaticMeshComponent_42)
   //Begin Object Class=AudioComponent Name=AmbientSoundComponent0 ObjName=AudioComponent_0 Archetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //   Name="AudioComponent_0"
   //   ObjectArchetype=AudioComponent'GearGame.Default__GearDestructibleObject.AmbientSoundComponent0'
   //End Object
   //Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0 ObjName=StaticMeshComponent_0 Archetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //   Name="StaticMeshComponent_0"
   //   ObjectArchetype=StaticMeshComponent'GearGame.Default__GearDestructibleObject.StaticMeshComponent0'
   //End Object
   //Begin Object Class=SpriteComponent Name=Sprite ObjName=SpriteComponent_1 Archetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //   LightingChannels=(bInitialized=True,Dynamic=True)
   //   Name="SpriteComponent_1"
   //   ObjectArchetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
   //End Object
   //Begin Object Class=CylinderComponent Name=CollisionCylinder ObjName=CylinderComponent_0 Archetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //   Name="CylinderComponent_0"
   //   ObjectArchetype=CylinderComponent'GearGame.Default__GearDestructibleObject.CollisionCylinder'
   //End Object
   //AmbientSoundComponent=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_2.AudioComponent_0'
   SubObjects(0)=(Mesh=StaticMeshComponent_41,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture.DresserMesh.Destruct_Dresser1a_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Medium')),(NewMesh=StaticMesh'Destruct_SLS_Furniture.DresserMesh.Destruct_Dresser2a_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Medium'))),DefaultHealth=400.000000)
   SubObjects(1)=(Mesh=StaticMeshComponent_42,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture.DresserMesh.Destruct_Dresser1b_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Medium')),(NewMesh=StaticMesh'Destruct_SLS_Furniture.DresserMesh.Destruct_Dresser2b_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Medium'))),DefaultHealth=400.000000)
   SubObjectHealths(0)=400.000000
   SubObjectHealths(1)=400.000000
   //Components(0)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_2.StaticMeshComponent_0'
   //Components(1)=SpriteComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_2.SpriteComponent_1'
   //Components(2)=AudioComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_2.AudioComponent_0'
   //Components(3)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_2.StaticMeshComponent_41'
   //Components(4)=StaticMeshComponent'deleteme.TheWorld.PersistentLevel.GearDestructibleObject_2.StaticMeshComponent_42'
   //Tag="GearDestructibleObject"
   //Location=(X=-495.999939,Y=1184.000000,Z=0.000000)
   //Name="GearDestructibleObject_2"
   //ObjectArchetype=GearDestructibleObject'GearGame.Default__GearDestructibleObject'

}


