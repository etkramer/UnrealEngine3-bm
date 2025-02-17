/**
 * Base class for locust drones
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustDroneBase extends GearPawn_LocustBase
	abstract
	config(Pawn);

var protected const GearSoundGroup	SoundGroupB;

simulated function PostBeginPlay()
{
	// these need to go there due to this uncrealscipt compiler assertion:
	//Assertion failed: OverrideComponent==BaseTemplate [File:D:\depot_epicgames\code\UnrealEngine3\Development\Src\Editor\Src\UnEdObject.cpp] [Line: 663] OverrideComponent: 'DynamicLightEnvironmentComponent GearGame.Default__GearPawn_LocustBase:MyLightEnvironment' BaseTemplate: 'DynamicLightEnvironmentComponent GearGame.Default__GearPawn_Infantry:MyLightEnvironment' Stack: Address = 0x746f7065 (filename not found)
	LightEnvironment.SetTickGroup( TG_DuringAsyncWork );

	Super.PostBeginPlay();
}

/** Grunts just explode when shot by ride reaver or brumak - like crowd members - save ragdoll perf */
simulated function bool ShouldUseSimpleEffectDeath(class<GearDamageType> GearDamageType)
{
	if( ClassIsChildOf(GearDamageType, class'GearGame.GDT_RideReaverCannon') ||
		ClassIsChildOf(GearDamageType, class'GearGame.GDT_ReaverMinigun') ||
		ClassIsChildOf(GearDamageType, class'GearGame.GDT_BrumakBulletPlayer') ||
		ClassIsChildOf(GearDamageType, class'GearGame.GDT_BrumakCannonPlayer') ||
		ClassIsChildOf(GearDamageType, class'GearGame.GDT_ReaverCannonCheap') )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

defaultproperties
{
	CharacterFootStepType=CFST_Locust_Drone

	DefaultInventory(0)=class'GearGame.GearWeap_LocustAssaultRifle'
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=343,V=0,UL=48,VL=63)
	HelmetType=None
	ShoulderPadLeftType=None
	ShoulderPadRightType=None

	ControllerClass=class'GearAI_Locust'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustDrone'
	SoundGroupB=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustDroneB'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustDrone"
	MasterGUDBankClassNames(1)="GearGameContent.GUDData_LocustDroneB"
	FAS_ChatterNames.Add("Locust_Grunt.FaceFX.Chatter")
	FAS_ChatterNames.Add("Locust_Grunt.FaceFX.Chatter_Dup")
	FAS_Efforts(0)=FaceFXAnimSet'Locust_Grunt.FaceFX.Drone_Efforts'

	PhysHRMotorStrength=(X=200,Y=0)

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'Locust_Grunt.Mesh.Locust_Grunt_Gore_SoftWeights'
	GorePhysicsAsset=PhysicsAsset'Locust_Grunt.PhysicsAsset.Locust_Grunt_CamSkel_Physics'
	GoreBreakableJointsTest=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_Armor_Crotch","b_MF_Armor_Sho_R","b_MF_UpperArm_R","b_MF_Hand_R","b_MF_Calf_R","b_MF_Calf_L")
	GoreBreakableJoints=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_Armor_Crotch","b_MF_Armor_Sho_R","b_MF_UpperArm_R","b_MF_Hand_R","b_MF_Calf_R","b_MF_Calf_L")
	
	Begin Object Name=GearPawnMesh
	    SkeletalMesh=SkeletalMesh'Locust_Grunt.Mesh.Locust_Grunt_CamSkel'
		PhysicsAsset=PhysicsAsset'Locust_Grunt.PhysicsAsset.Locust_Grunt_CamSkel_Physics'
	End Object

	
	SpecialMoveClasses(SM_MidLvlJumpOver)=class'GSM_MantleOverLocust'

	NoticedGUDSEvent=GUDEvent_NoticedDrone
	NoticedGUDSPriority=20

}
