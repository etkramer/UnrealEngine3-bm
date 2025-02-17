/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class COG_DerrickWheels extends COG_DerrickWheelsBase;

simulated protected function SkeletalMeshComponent CreateGrindliftMesh(Name AttachName)
{
	local SkeletalMeshComponent GrindliftMesh;
	GrindliftMesh = new(self) class'SkeletalMeshComponent';

	GrindliftMesh.AnimSets[0] = AnimSet'COG_Derrick.Anims.COG_Grindlift_Animset';
	GrindliftMesh.SetSkeletalMesh(SkeletalMesh'COG_Derrick.Mesh.COG_GrindLift');
	GrindliftMesh.SetAnimTreeTemplate(AnimTree'COG_Derrick.Mesh.COG_Grindlift_AnimTree');

	GrindliftMesh.bUpdateSkelWhenNotRendered = FALSE;

	GrindliftMesh.SetShadowParent(DerrickBody.StaticMeshComponent);
	GrindliftMesh.SetLightEnvironment(DerrickBody.LightEnvironment);

	Mesh.AttachComponent(GrindliftMesh, AttachName);

	return GrindliftMesh;
}

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'COG_Derrick.Mesh.COG_Derrick'
		AnimTreeTemplate=AnimTree'COG_Derrick.Mesh.COG_Derrick_AnimTree'
		AnimSets(0)=AnimSet'COG_Derrick.Anims.COG_Derrick_Animset'
		PhysicsAsset=PhysicsAsset'COG_Derrick.Mesh.COG_Derrick_Physics'
	End Object

	// audio for spatialized engine loops and suspension audio
	EngineAudio_Player_LeftCue=SoundCue'Vehicle_Derrick.Derrick.Derrick_Engine01_Cue'
	EngineAudio_Player_RightCue=SoundCue'Vehicle_Derrick.Derrick.Derrick_Engine02_Cue'
	SuspensionAudio_Player_FrontRightCue=SoundCue'Vehicle_Derrick.Derrick.Derrick_SuspensionA_Cue'
	SuspensionAudio_Player_FrontLeftCue=SoundCue'Vehicle_Derrick.Derrick.Derrick_SuspensionA_Cue'

	DerrickAudio_NonPlayerCue=SoundCue'Vehicle_Derrick.Derrick.Derrick_EngineMainMono_Cue'

	// wheels
	Wheels(0)={(
		BoneName=b_Rt_Front_Tire,SkelControlName=Rt_Front_Wheel,
		WheelRadius=175.f,
		FreeSpinningFrictionDecel=50.f,
		WheelRayBaseOffset=(Y=128),
		)}
	Wheels(1)={(
		BoneName=b_Rt_Mid_Tire,SkelControlName=Rt_Mid_Wheel,
		WheelRadius=145.f,
		FreeSpinningFrictionDecel=50.f,
		WheelRayBaseOffset=(Y=128)
		)}
	Wheels(2)={(
		BoneName=b_Rt_Rear_Tire,SkelControlName=Rt_Rear_Wheel,
		WheelRadius=145.f,
		FreeSpinningFrictionDecel=50.f,
		WheelRayBaseOffset=(Y=128),
		)}
	Wheels(3)={(
		BoneName=b_Lt_Front_Tire,SkelControlName=Lt_Front_Wheel,
		WheelRadius=175.f,
		FreeSpinningFrictionDecel=50.f,
		WheelRayBaseOffset=(Y=-128),
		)}
	Wheels(4)={(
		BoneName=b_Lt_Mid_Tire,SkelControlName=Lt_Mid_Wheel,
		WheelRadius=145.f,
		FreeSpinningFrictionDecel=50.f,
		WheelRayBaseOffset=(Y=-128)
		)}
	Wheels(5)={(
		BoneName=b_Lt_Rear_Tire,SkelControlName=Lt_Rear_Wheel,
		WheelRadius=145.f,
		FreeSpinningFrictionDecel=50.f,
		WheelRayBaseOffset=(Y=-128),
		)}

}

