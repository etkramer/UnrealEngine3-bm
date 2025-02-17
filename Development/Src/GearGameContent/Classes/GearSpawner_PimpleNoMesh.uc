/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*
*/
class GearSpawner_PimpleNoMesh extends GearSpawner_Pimple
	DependsOn(GearPawn)
	placeable;


simulated function PlayOpenEffects();

function Open()
{
	bOpen=true;
}


simulated function PostBeginPlay()
{
	Super(GearSpawner).PostBeginPlay();
}

defaultproperties
{
	Components.Remove(PimpleMesh)
	Mesh=none

	Components.Remove(CollisionCylinder)
	CollisionComponent=none

	Components.Remove(ClosedMesh0)
	ClosedSM=none


	Begin Object Name=SpawnEffect0
		Translation=(z=0)
	End Object

	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=TRUE
		AlwaysLoadOnClient=False
		AlwaysLoadOnServer=False
	End Object
	Components.Add(Sprite)
}
