
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Turret_HOD extends Turret_TroikaCabal;

defaultproperties
{
	DefaultInventory(0)=class'GearWeap_HODTurret'
	TurretTurnRateScale=0.1f
	AimingTurretTurnRateScale=0.15f

	//bEnforceHardAttach=FALSE

	//bBlockActors=FALSE
	//bCollideActors=TRUE

	//// derrick sequence is tightly controlled, no need to have turret collide with world
	//// can only cause problems (already did, in fact)
	//bCollideWorld=FALSE

	//// use the cylinder for collision
	//CollisionComponent=CollisionCylinder

	//Begin Object Name=SkelMeshComponent0
	//	//PhysicsAsset=None
	//	CollideActors=FALSE
	//	BlockActors=FALSE
	//End Object
}
