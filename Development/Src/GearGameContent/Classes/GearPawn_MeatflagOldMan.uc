/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_MeatflagOldMan extends GearPawn_MeatflagBase
	config(Pawn);



defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=147,V=0,UL=48,VL=63)

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Neutral_Stranded_10.NPC10_COG'
		PhysicsAsset=PhysicsAsset'Neutral_Stranded_10.NPC10_COG_Physics'
		Translation=(Z=-70)
	End Object

	MasterGUDBankClassNames(0)="GearGameContent.GUDData_NPCOldManMP"
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_NPCOldMan'
	FAS_ChatterNames.Add("Neutral_Stranded_10.FaceFX.Chaps_FaceFX_Chatter") 
	FAS_Efforts(0)=FaceFXAnimSet'Neutral_Stranded_10.FaceFX.Chaps_FaceFX_Efforts'
}
