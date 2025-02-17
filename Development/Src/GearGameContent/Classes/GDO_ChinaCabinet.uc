/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GDO_ChinaCabinet extends GearDestructibleObject;

defaultproperties
{
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_494 ObjName=StaticMeshComponent_494 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
		StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage0a_Smesh'
		LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
		LightEnvironment=MyLightEnvironment
		Name="StaticMeshComponent_494"
		ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
	End Object
	Components.Add(StaticMeshComponent_494)
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_495 ObjName=StaticMeshComponent_495 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
		StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage0b_Smesh'
		LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
		LightEnvironment=MyLightEnvironment
		Name="StaticMeshComponent_495"
		ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
	End Object
	Components.Add(StaticMeshComponent_495)
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_496 ObjName=StaticMeshComponent_496 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
		StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage0c_Smesh'
		LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
		LightEnvironment=MyLightEnvironment
		Name="StaticMeshComponent_496"
		ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
	End Object
	Components.Add(StaticMeshComponent_496)
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_497 ObjName=StaticMeshComponent_497 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
		StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage0d_Smesh'
		LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
		LightEnvironment=MyLightEnvironment
		Name="StaticMeshComponent_497"
		ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
	End Object
	Components.Add(StaticMeshComponent_497)
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_498 ObjName=StaticMeshComponent_498 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
		StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage0e_Smesh'
		LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
		LightEnvironment=MyLightEnvironment
		Name="StaticMeshComponent_498"
		ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
	End Object
	Components.Add(StaticMeshComponent_498)
	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_499 ObjName=StaticMeshComponent_499 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
		StaticMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage0f_Smesh'
		LightingChannels=(bInitialized=True,Static=True,CompositeDynamic=TRUE)
		LightEnvironment=MyLightEnvironment
		Name="StaticMeshComponent_499"
		ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
	End Object
	Components.Add(StaticMeshComponent_499)
	//Begin Object Class=SpriteComponent Name=Sprite ObjName=SpriteComponent_140 Archetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
	//	LightingChannels=(bInitialized=True,Dynamic=True)
	//	Name="SpriteComponent_140"
	//	ObjectArchetype=SpriteComponent'GearGame.Default__GearDestructibleObject.Sprite'
	//End Object
	//Begin Object Class=StaticMeshComponent Name=StaticMeshComponent_4 ObjName=StaticMeshComponent_4 Archetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
	//	Name="StaticMeshComponent_4"
	//	ObjectArchetype=StaticMeshComponent'Engine.Default__StaticMeshComponent'
	//End Object
	//AmbientSoundComponent=AudioComponent'CoverActors.TheWorld.PersistentLevel.GearDestructibleObject_6.AudioComponent_287'
	SubObjects(0)=(SubObjName="BottomLeft",Mesh=StaticMeshComponent_494,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage1a_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Medium'),DependentSubObjs=((DependentSubObjName="TopLeft",MaxHealthToAllow=200.000000),(DependentSubObjName="TopMiddle",MaxHealthToAllow=200.000000))),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage2a_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Large'),DependentSubObjs=((DependentSubObjName="TopLeft")))),DefaultHealth=400.000000)
	SubObjects(1)=(SubObjName="BottomMiddle",Mesh=StaticMeshComponent_495,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage1b_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Medium'),DependentSubObjs=((DependentSubObjName="TopLeft",MaxHealthToAllow=200.000000),(DependentSubObjName="TopMiddle",MaxHealthToAllow=200.000000),(DependentSubObjName="TopRight",MaxHealthToAllow=200.000000))),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage2b_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Large'),DependentSubObjs=((DependentSubObjName="TopMiddle")))),DefaultHealth=400.000000)
	SubObjects(2)=(SubObjName="BottomRight",Mesh=StaticMeshComponent_496,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage1c_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Medium'),DependentSubObjs=((DependentSubObjName="TopRight",MaxHealthToAllow=200.000000),(DependentSubObjName="TopMiddle",MaxHealthToAllow=200.000000))),(NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage2c_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Large'),DependentSubObjs=((DependentSubObjName="TopRight")))),DefaultHealth=400.000000)
	SubObjects(3)=(SubObjName="TopLeft",Mesh=StaticMeshComponent_497,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage1d_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Small',SoundCue'Foley_Crashes.SoundCues.Crash_Glass_Medium')),(Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Glass_Small',SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Medium'),bSelfDestruct=True)),DefaultHealth=400.000000)
	SubObjects(4)=(SubObjName="TopMiddle",Mesh=StaticMeshComponent_498,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage1e_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Glass_Medium',SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Small')),(Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Glass_Small',SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Medium'),bSelfDestruct=True)),DefaultHealth=400.000000)
	SubObjects(5)=(SubObjName="TopRight",Mesh=StaticMeshComponent_499,DamageMods=((HealthThreshold=200.000000,NewMesh=StaticMesh'Destruct_SLS_Furniture_MS2.DocStorageMesh.Destruct_DocStorage1f_Smesh',Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Glass_Medium',SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Small')),(Sounds=(SoundCue'Foley_Crashes.SoundCues.Crash_Glass_Small',SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Medium'),bSelfDestruct=True)),DefaultHealth=400.000000)
	SubObjectHealths(0)=400.000000
	SubObjectHealths(1)=400.000000
	SubObjectHealths(2)=400.000000
	SubObjectHealths(3)=400.000000
	SubObjectHealths(4)=400.000000
	SubObjectHealths(5)=400.000000
	//AttachedCover(0)=CoverLink'CoverActors.TheWorld.PersistentLevel.CoverLink_51'
	//AttachedCover(1)=CoverLink'CoverActors.TheWorld.PersistentLevel.CoverLink_53'
	//AttachedCover(2)=CoverLink'CoverActors.TheWorld.PersistentLevel.CoverLink_52'
	//AttachedCover(3)=CoverLink'CoverActors.TheWorld.PersistentLevel.CoverLink_54'
	//Components(0)=SpriteComponent'CoverActors.TheWorld.PersistentLevel.GearDestructibleObject_6.SpriteComponent_140'
	//Components(1)=AudioComponent'CoverActors.TheWorld.PersistentLevel.GearDestructibleObject_6.AudioComponent_287'
	//Components(2)=StaticMeshComponent'CoverActors.TheWorld.PersistentLevel.GearDestructibleObject_6.StaticMeshComponent_494'
	//Components(3)=StaticMeshComponent'CoverActors.TheWorld.PersistentLevel.GearDestructibleObject_6.StaticMeshComponent_495'
	//Components(4)=StaticMeshComponent'CoverActors.TheWorld.PersistentLevel.GearDestructibleObject_6.StaticMeshComponent_496'
	//Components(5)=StaticMeshComponent'CoverActors.TheWorld.PersistentLevel.GearDestructibleObject_6.StaticMeshComponent_497'
	//Components(6)=StaticMeshComponent'CoverActors.TheWorld.PersistentLevel.GearDestructibleObject_6.StaticMeshComponent_498'
	//Components(7)=StaticMeshComponent'CoverActors.TheWorld.PersistentLevel.GearDestructibleObject_6.StaticMeshComponent_499'
	//Tag="GearDestructibleObject"
	//Location=(X=-303.999939,Y=1776.000000,Z=0.000000)
	//DrawScale3D=(X=1.000000,Y=1.250000,Z=1.000000)
	//CollisionComponent=StaticMeshComponent'CoverActors.TheWorld.PersistentLevel.GearDestructibleObject_6.StaticMeshComponent_4'
	//Name="GearDestructibleObject_6"
	//ObjectArchetype=GearDestructibleObject'GearGame.Default__GearDestructibleObject'

	
	
	
	
	
	
	
	
	
	
	
	

}


