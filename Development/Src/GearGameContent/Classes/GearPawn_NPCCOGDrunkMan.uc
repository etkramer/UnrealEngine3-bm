/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_NPCCOGDrunkMan extends GearPawn_NPCCOGBase
	config(Pawn);



defaultproperties
{
	HelmetType=class'Item_Helmet_StrandedHuntingHat'

	Begin Object Name=GearPawnMesh
	    SkeletalMesh=SkeletalMesh'Neutral_Stranded_03.NPC03_COG'
	    PhysicsAsset=PhysicsAsset'Neutral_Stranded_03.NPC03_COG_Physics'
	End Object

}
