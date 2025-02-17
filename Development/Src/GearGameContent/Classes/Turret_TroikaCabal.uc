/**
 * Troika Cabal turret
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Turret_TroikaCabal extends Turret_TroikaBase
	config(Pawn);

defaultproperties
{
	MuzzleSocketName		="Muzzle"
	PlayerRefSocketName		="PlayerRef"
	Pivot_Latitude_BoneName	="b_troika_gun_latitude"
	LeftHandBoneHandleName  ="b_troika_handle_R"
	RightHandBoneHandleName ="b_troika_handle_L"

	Begin Object Name=SkelMeshComponent0
		SkeletalMesh=SkeletalMesh'Locust_TroikaCabal.TroikaTurret'
		PhysicsAsset=PhysicsAsset'Locust_TroikaCabal.TroikaTurret_Physics'
		AnimTreeTemplate=AnimTree'Locust_TroikaCabal.AnimTree_TroikaCabal'
		AnimSets(0)=AnimSet'Locust_TroikaCabal.TroikaTurretAnims'
		Scale=1.f
		Translation=(X=0,Z=-50)
	End Object

	DefaultInventory(0)=class'GearWeap_Troika'
}
