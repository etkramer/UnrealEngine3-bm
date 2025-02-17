/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class InterpActor_COGDerrick extends InterpActor_COGDerrickBase
	placeable;

defaultproperties
{
	WheelActorClass=class'GearGameContent.COG_DerrickWheels'

	BlockRigidBody=TRUE

	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'COG_Derrick.Mesh.COG_Derrick_Body_TEST_SM'
		bDisableAllRigidBody=TRUE		// all rb collision will happen with the RB Collison mesh
		BlockRigidBody=FALSE
	End Object

	Begin Object Name=RBCollision0
		StaticMesh=StaticMesh'COG_Derrick.Mesh.COG_Derrick_Ragdoll_Collision'
		
		HiddenGame=TRUE

		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=TRUE
	End Object

	Begin Object Name=GlassMesh0
		StaticMesh=StaticMesh'COG_Derrick.Mesh.COG_Derrick_Glass_SM'
		bAcceptsLights=FALSE
		CastShadow=FALSE
	End Object

	Begin Object Class=SkeletalMeshComponent Name=EditorPreviewWheels0
		SkeletalMesh=SkeletalMesh'COG_Derrick.Mesh.COG_Derrick'
		PhysicsAsset=PhysicsAsset'COG_Derrick.Mesh.COG_Derrick_Physics'
		HiddenEditor=FALSE
		LightEnvironment=MyLightEnvironment
		ShadowParent=StaticMeshComponent0
	End Object
	EditorWheelsPreviewMesh=EditorPreviewWheels0
	Components.Add(EditorPreviewWheels0)

	PlayerEngineMainLoopCue=SoundCue'Vehicle_Derrick.Derrick.Derrick_EngineMain_Cue'

	StartEngineSound=SoundCue'Vehicle_Derrick.Derrick.Derrick_EngineStart_Cue'
	StopEngineSound=SoundCue'Vehicle_Derrick.Derrick.Derrick_EngineStop_Cue'
}
