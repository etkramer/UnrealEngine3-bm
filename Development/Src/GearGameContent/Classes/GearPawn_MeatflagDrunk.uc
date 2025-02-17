/**
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearPawn_MeatflagDrunk extends GearPawn_MeatflagBase
	config(Pawn);


simulated protected function InitMPRimShader()
{
	Super.InitMPRimShader();

	if( RimShaderMaterialSpecificHead != none )
	{
		MPRimShader = new(Outer) class'MaterialInstanceConstant';
		MPRimShader.SetParent( RimShaderMaterialSpecificHead );
		Mesh.SetMaterial( 1, MPRimShader );
	}
	else
	{
		RimShaderMaterialSpecificHead = Mesh.CreateAndSetMaterialInstanceConstant(1);
	}

	SetUpRimShader( RimShaderMaterialSpecificHead );
}


defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=196,V=0,UL=48,VL=63)

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Neutral_Stranded_05.NPC05_COG'
		PhysicsAsset=PhysicsAsset'Neutral_Stranded_05.NPC05_COG_Physics'
		Translation=(Z=-70)
	End Object

	MasterGUDBankClassNames(0)="GearGameContent.GUDData_NPCDrunkMP"
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_NPCDrunk'
	FAS_ChatterNames.Add("Neutral_Stranded_05.FaceFX.NPC05_DrunkMan_COG_FaceFX_Drunk2_Chatter") 
	FAS_Efforts(0)=FaceFXAnimSet'Neutral_Stranded_05.FaceFX.NPC05_DrunkMan_COG_FaceFX_Drunk2_Efforts'
}
