
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustBoomerMechanic extends GearPawn_LocustBoomerButcher
	config(Pawn);

defaultproperties
{
	HelmetType=None
	HeldItem=class'Item_BoomerWrench'


	GoreSkeletalMesh=SkeletalMesh'Locust_Boomer.Gore.Locust_BoomerMechanic_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Boomer.Mesh.Locust_Mechanic_Boomer_CamSkel_Physics'
	GoreBreakableJointsTest=("b_MF_Head","b_MF_Face","b_MF_Forearm_L","b_MF_Hand_L","b_MF_Forearm_R","b_MF_Hand_R","b_MF_Calf_R","b_MF_Foot_L","b_Boomer_Hook1")
	GoreBreakableJoints=("b_MF_Head","b_MF_Face","b_MF_Forearm_L","b_MF_Hand_L","b_MF_Forearm_R","b_MF_Hand_R","b_MF_Calf_R","b_MF_Foot_L","b_Boomer_Hook1")

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Boomer.Mesh.Locust_Mechanic_Boomer'
		PhysicsAsset=PhysicsAsset'Locust_Boomer.Mesh.Locust_Mechanic_Boomer_CamSkel_Physics'
	End Object

	// wrench boomer says "pound"
	BoomerAttackTelegraphDialogue.Empty
	BoomerAttackTelegraphDialogue(0)=SoundCue'Locust_Boomer2_Chatter_Cue.DupedRefsForCode.Boomer2Chatter_Pound_Loud01Cue_Code'
	BoomerAttackTelegraphDialogue(1)=SoundCue'Locust_Boomer1_Chatter_Cue.AttackingEnemy.Boomer1Chatter_Pound_Loud01Cue'

}
