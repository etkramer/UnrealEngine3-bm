/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_NPCCOGMaleWithHatAndBag extends GearPawn_NPCCOGBase
	config(Pawn);


defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=295,V=65,UL=48,VL=63)

	Begin Object Name=GearPawnMesh
	    SkeletalMesh=SkeletalMesh'Neutral_Stranded_01.NPC01_COG'
	    PhysicsAsset=PhysicsAsset'Neutral_Stranded_01.NPC01_COG_Physics'
	End Object

}
