/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTXRayVolume_Content extends UTXRayVolume;

defaultproperties
{
	Components.Remove(BrushComponent0)
	BrushComponent=None

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Pickups.Deployables.Mesh.S_Deployables_SlowVolume_Collision'
		BlockActors=false
		CollideActors=true
		BlockRigidBody=false
		CastShadow=false
		bUseAsOccluder=false
		HiddenGame=true
		Translation=(X=-455.0,Y=-200.0)
		Scale3D=(X=1.02,Y=1.55,Z=1.30) // this is scaled crazily so it is slightly larger than the actual mesh
	End Object
	Components.Add(StaticMeshComponent0)
	CollisionComponent=StaticMeshComponent0

	Begin Object class=AnimNodeSequence Name=MeshSequenceA
	End Object

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		ModShadowFadeoutTime=1.0
	End Object
	Components.Add(MyLightEnvironment)

	Begin Object Class=SkeletalMeshComponent Name=VisualMesh
		Animations=MeshSequenceA
		AnimSets(0)=AnimSet'Pickups.Deployables.Anims.K_Deployables_SlowVolume'
		SkeletalMesh=SkeletalMesh'Pickups.Deployables.Mesh.SK_Deployables_SlowVolume'
		BlockActors=false
		CollideActors=false
		BlockRigidBody=false
		bUseAsOccluder=false
		Translation=(X=-435.0,Y=-200.0,Z=-1.0)
		bAcceptsDynamicDecals=FALSE
		CastShadow=true
		LightEnvironment=MyLightEnvironment
		bUpdateSkelWhenNotRendered=false
	End Object
	Components.Add(VisualMesh)
	GeneratorMesh=VisualMesh;

	Begin Object Class=ParticleSystemComponent Name=VisualEffect
		Template=ParticleSystem'Pickups.Deployables.Effects.P_Deployables_SlowVolume_Spawn_Idle'
		Translation=(X=-455.0,Y=-200.0)
		Scale3D=(X=1.02,Y=1.55,Z=1.30)
		bAutoActivate=false
	End Object
	Components.Add(VisualEffect)
	SlowEffect=VisualEffect

	Begin Object Class=ParticleSystemComponent Name=VisualEffect2
		Template=ParticleSystem'Pickups.Deployables.Effects.P_Deployables_SlowVolume_Projector'
		Translation=(X=-435.0,Y=-200.0,Z=-4.0)
		bAutoActivate=false
	End Object
	Components.Add(VisualEffect2)
	GeneratorEffect=VisualEffect2

	ActivateSound=SoundCue'A_Pickups_Deployables.SlowVolume.SlowVolume_OpenCue'
	DestroySound=SoundCue'A_Pickups_Deployables.SlowVolume.SlowVolume_CloseCue'
	EnterSound=SoundCue'A_Pickups_Deployables.SlowVolume.SlowVolume_EnterCue'
	ExitSound=SoundCue'A_Pickups_Deployables.SlowVolume.SlowVolume_ExitCue'
	OutsideAmbientSound=SoundCue'A_Pickups_Deployables.SlowVolume.SlowVolume_LoopOutsideCue'
	InsideAmbientSound=SoundCue'A_Pickups_Deployables.SlowVolume.SlowVolume_LoopInsideCue'

	Begin Object Class=AudioComponent Name=AmbientAudio
		bShouldRemainActiveIfDropped=true
		bStopWhenOwnerDestroyed=true
	End Object
	AmbientSoundComponent=AmbientAudio
	Components.Add(AmbientAudio)

	//InsideCameraEffect=class'UTEmitCameraEffect_SlowVolume'
}
