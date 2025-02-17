
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Turret_MiniGun extends Turret_TroikaCabal
	config(Pawn);

defaultproperties
{
	CameraViewOffsetHigh=(X=-275,Y=0,Z=40)
	CameraViewOffsetLow=(X=-275,Y=0,Z=110)
	CameraViewOffsetMid=(X=-275,Y=0,Z=70)
	CameraTargetingViewOffsetHigh=(X=-160,Y=0,Z=36)
	CameraTargetingViewOffsetLow=(X=-160,Y=0,Z=36)
	CameraTargetingViewOffsetMid=(X=-160,Y=0,Z=36)
	ViewRotInterpSpeed=40.0

	Pivot_Latitude_BoneName	="b_mini_gun_latitude"
	LeftHandBoneHandleName  ="b_mini_handle_R"
	RightHandBoneHandleName ="b_mini_handle_L"

	YawLimit=(X=-32768,Y=+32768)

	//Begin Object Name=CollisionCylinder
	//	CollisionHeight=50.000000
	//	CollisionRadius=20.000000
	//	BlockZeroExtent=FALSE
	//	BlockNonZeroExtent=TRUE
	//	BlockActors=TRUE
	//	CollideActors=TRUE
	//	Translation=(X=70)
	//End Object

	Begin Object Name=SkelMeshComponent0
		SkeletalMesh=SkeletalMesh'COG_Minigun.COG_MinigunTurret'
		PhysicsAsset=PhysicsAsset'COG_Minigun.COG_MinigunTurret_Physics'
		AnimTreeTemplate=AnimTree'COG_Minigun.Anims.AnimTree_MiniGun'
		AnimSets(0)=AnimSet'COG_Minigun.COG_MinigunTurret_Anims'
		//BlockZeroExtent=TRUE
		//CollideActors=TRUE
		//BlockActors=TRUE
		//BlockRigidBody=TRUE
		AlwaysLoadOnClient=TRUE
		AlwaysLoadOnServer=TRUE
		Scale=1.0
		Translation=(X=0,Z=-50) 
	End Object
//	Mesh=SkelMeshComponent0
//	Components.Add(SkelMeshComponent0)

	TrackSpeed=3.f
	SearchSpeed=0.5f

//	TurretTurnRateScale=0.33
//	ExitPositions(0)=(X=-110)
//	EntryPosition=(X=-110)
//	EntryRadius=100
//	bRelativeExitPos=TRUE
//	bAttachDriver=TRUE
	PitchBone=b_mini_gun_longitude
	//BaseBone=Rt_Tripod_bone
	ViewPitchMin=-4096
	ViewPitchMax=4400
	POV=(DirOffset=(X=-9,Y=2,Z=4.5),Distance=25,fZAdjust=-100)
	CannonFireOffset=(X=160,Y=0,Z=24)

	InventoryManagerClass=class'GearInventoryManager'
	DefaultInventory(0)=class'GearWeap_MinigunTurret'
}
