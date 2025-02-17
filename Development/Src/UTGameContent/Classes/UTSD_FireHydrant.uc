/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTSD_FireHydrant extends UTSimpleDestroyable
	placeable;


defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'HU_Deco3.SM.Mesh.S_HU_Deco_SM_FireHydrant01'
	End Object

	MeshOnDestroy=StaticMesh'HU_Deco3.SM.Mesh.S_HU_Deco_SM_FireHydrantCap01'
	SoundOnDestroy=SoundCue'A_Gameplay.FireHydrant.FireHydrant_BreakSpewCue'
	ParticlesOnDestroy=ParticleSystem'Envy_Level_Effects.Water.P_Water_BrokenHydrant'

	SpawnPhysMesh=StaticMesh'HU_Deco3.SM.Mesh.S_HU_Deco_SM_FireHydrant01'
	SpawnPhysMeshLinearVel=(Z=1000.0)
	SpawnPhysMeshAngularVel=(Y=10.0)

	bDestroyOnPlayerTouch=FALSE
}
