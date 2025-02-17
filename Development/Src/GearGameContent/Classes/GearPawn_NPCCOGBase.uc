/**
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearPawn_NPCCOGBase extends GearPawn_Infantry
	abstract
	config(Pawn);



defaultproperties
{
	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGame.GearWeap_AssaultRifle'
	HelmetType=class'Item_Helmet_None'
	ControllerClass=class'GearAI_NPCCOG'
		
	HostageHealthBuckets.Empty()

	// HeadShot neck attachment
	Begin Object Class=StaticMeshComponent Name=GearHeadShotMesh1
		StaticMesh=StaticMesh'COG_Gore.COG_Headshot_Gore'
		CollideActors=FALSE
		LightEnvironment=MyLightEnvironment
	End Object
	HeadShotNeckGoreAttachment=GearHeadShotMesh1
	bCanPlayHeadShotDeath=TRUE

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0034.000000
		CollisionHeight=+0072.000000
	End Object

	CrouchHeight=+60.0
	CrouchRadius=+34.0
}
