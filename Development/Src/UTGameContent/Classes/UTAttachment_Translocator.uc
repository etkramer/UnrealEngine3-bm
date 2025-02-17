/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTAttachment_Translocator extends UTWeaponAttachment;

var MaterialInterface TeamSkins[2];

simulated function SetSkin(Material NewMaterial)
{
	local int TeamIndex;

	if ( NewMaterial == None ) 	// Clear the materials
	{
		TeamIndex = Instigator.GetTeamNum();
		if ( TeamIndex == 255 )
			TeamIndex = 0;
		Mesh.SetMaterial(0,TeamSkins[TeamIndex]);
	}
	else
	{
		Super.SetSkin(NewMaterial);
	}
}

simulated function AttachTo(UTPawn OwnerPawn)
{
	Mesh.PlayAnim('WeaponIdle',, true);

	Super.AttachTo(OwnerPawn);
}

defaultproperties
{
	WeapAnimType=EWAT_Pistol

	// Weapon SkeletalMesh
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'WP_Translocator.Mesh.SK_WP_Translocator_3P_Mid'
		AnimSets(0)=AnimSet'WP_Translocator.Anims.K_WP_Translocator_3P_Base'
	End Object
	WeaponClass=class'UTWeap_Translocator_Content'


	TeamSkins[0]=MaterialInterface'WP_Translocator.Materials.M_WP_Translocator_1P'
	TeamSkins[1]=MaterialInterface'WP_Translocator.Materials.M_WP_Translocator_1PBlue'
}
