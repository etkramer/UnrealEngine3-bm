/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustBloodMountRiderless extends GearPawn_LocustBloodMount;

defaultproperties
{
	DefaultInventory(0)=class'GearWeap_BloodMountMelee'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_Bloodmount'

	Begin Object Name=CollisionCylinder
		CollisionHeight=+60.0
	End Object

	bCanPlayPhysicsHitReactions=FALSE
	
	GoreSkeletalMesh=SkeletalMesh'Locust_Bloodmount.Locust_Bloodmount_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Bloodmount.Locust_Blood_Mount_Physics'
	GoreBreakableJointsTest=("b_handlebars","b_face1","b_face2","Jaw","Hips","Lft_Clav","Lft_UpperArm","Lft_Hand","Lft_Toe","Lft_Foot","Rt_Finger_07","Rt_Hand","Rt_ForeArm","Rt_Thigh","Rt_Foot","Rt_Toe")
	GoreBreakableJoints=("b_handlebars","b_face1","b_face2","Jaw","Hips","Lft_Clav","Lft_UpperArm","Lft_Hand","Lft_Toe","Lft_Foot","Rt_Finger_07","Rt_Hand","Rt_ForeArm","Rt_Thigh","Rt_Foot","Rt_Toe")

	NeckBoneName="Neck"
	MeleeDamageBoneName="Spine2"

	Begin Object Name=GearPawnMesh
	    bHasPhysicsAssetInstance=TRUE
		SkeletalMesh=SkeletalMesh'Locust_Bloodmount.Locust_Blood_Mount'
		PhysicsAsset=PhysicsAsset'Locust_Bloodmount.Locust_Blood_Mount_Physics'
		AnimSets(0)=AnimSet'Locust_Bloodmount.Locust_Bloodmount_Animset'
		AnimTreeTemplate=AnimTree'Locust_Bloodmount.BloodMount_AnimTree'
		Translation=(Z=-60)
	End Object

	DriverClass=none
	AttackSound=SoundCue'Locust_Bloodmount_Efforts.BloodMount.BloodMount_BodyArmImpactCue'
	HeadShotCue=SoundCue'Locust_Bloodmount_Efforts.BloodMount.BloodMount_HelmetRipOffCue'
	DriverGoneCue=SoundCue'Locust_Bloodmount_Efforts.BloodMount.BloodMount_GrowlACue'
	EnemyAcquiredCue=SoundCue'Locust_Bloodmount_Efforts.BloodMount.BloodMount_AlertCue'	
	AmbientCue=SoundCue'Locust_Bloodmount_Efforts.BloodMount.BloodMount_AmbientCue'
	CombatAmbientCue=SoundCue'Locust_Bloodmount_Efforts.BloodMount.BloodMount_BreathFastCue'
	ChargeCue=SoundCue'Locust_Bloodmount_Efforts.BloodMount.BloodMount_GrowlACue'
	BodyFallSound=SoundCue'Locust_Bloodmount_Efforts.BloodMount.BloodMount_BodyFallCue'
	HelmetItem=class'Item_Helmet_LocustBloodmount'
}
