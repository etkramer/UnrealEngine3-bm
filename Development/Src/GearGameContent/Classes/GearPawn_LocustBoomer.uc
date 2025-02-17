/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 */
class GearPawn_LocustBoomer extends GearPawn_LocustBoomerBase
	config(Pawn);

defaultproperties
{
	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGameContent.GearWeap_Boomshot'

	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=98,V=0,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'
	HeadSocketName="Helmet"

	ControllerClass=class'GearAI_Boomer'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustBoomer'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustBoomer"
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.Chatter")
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.Chatter_Dup")
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.boomer_FaceFX_Boomer1Chatter")
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.boomer_FaceFX_Boomer1Chatter_Dup")
	FAS_Efforts(0)=FaceFXAnimSet'Locust_Boomer.FaceFX.boomer_FaceFX_Efforts'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'Locust_Boomer.Gore.Locust_Boomer_Gore'
	GorePhysicsAsset=PhysicsAsset'LOCUST_Boomer.Mesh.Locust_Boomer_CamSkel_Physics'
	GoreBreakableJointsTest=("b_MF_Head","b_MF_Face","b_MF_Forearm_L","b_MF_Hand_L","b_MF_Forearm_R","b_MF_Hand_R","b_MF_Calf_R","b_MF_Foot_L")
	GoreBreakableJoints=("b_MF_Head","b_MF_Face","b_MF_Forearm_L","b_MF_Hand_L","b_MF_Forearm_R","b_MF_Hand_R","b_MF_Calf_R","b_MF_Foot_L")

	Begin Object Name=GearPawnMesh
		bHasPhysicsAssetInstance=TRUE	// Needed by PhysicsBodyImpact
		SkeletalMesh=SkeletalMesh'LOCUST_Boomer.Mesh.Locust_Boomer_CamSkel'
		PhysicsAsset=PhysicsAsset'LOCUST_Boomer.Mesh.Locust_Boomer_CamSkel_Physics'
		AnimSets(0)=AnimSet'LOCUST_Boomer.Animations.AnimSetBoomer_CamSkel'
		AnimTreeTemplate=AnimTree'LOCUST_Boomer.Animations.AT_LocustBoomer'
	End Object
	MeshTranslationNudgeOffset=-5.f

	PS_FootstepDust=ParticleSystem'LOCUST_Boomer.Effects.P_Locust_Boomer_Dust_Footstep'
	FootStepSound=SoundCue'Foley_Footsteps.FootSteps.BoomerFootstepsCue'

	DefaultWeapon=class'GearWeap_Boomshot'

	// regular boomer is boomerA, says "boom"
	BoomerAttackTelegraphDialogue.Empty
	BoomerAttackTelegraphDialogue(0)=SoundCue'Locust_Boomer_Chatter_Cue.BoomerSingle.BoomerChatter_Boom03Cue'
	BoomerAttackTelegraphDialogue(1)=SoundCue'Locust_Boomer_Chatter_Cue.BoomerSingle.BoomerChatter_Boom02Cue'
	BoomerAttackTelegraphDialogue(2)=SoundCue'Locust_Boomer_Chatter_Cue.BoomerSingle.BoomerChatter_Boom01Cue'
}
