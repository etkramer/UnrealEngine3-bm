/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTGib_HumanTorso extends UTGib_Human;

defaultproperties
{
	GibMeshesData[0]=(TheStaticMesh=None,TheSkelMesh=SkeletalMesh'CH_Gibs.Mesh.SK_CH_Gib_Torso',ThePhysAsset=PhysicsAsset'CH_Gibs.Mesh.SK_CH_Gib_Torso_Physics',DrawScale=1.3f,bUseSecondaryGibMeshMITV=TRUE)
	GibMeshesData[1]=(TheStaticMesh=None,TheSkelMesh=SkeletalMesh'CH_Gibs.Mesh.SK_CH_Gib_Ribs',ThePhysAsset=PhysicsAsset'CH_Gibs.Mesh.SK_CH_Gib_Ribs_Physics',DrawScale=1.3f,bUseSecondaryGibMeshMITV=TRUE)
	HitSound=SoundCue'A_Character_BodyImpacts.BodyImpacts.A_Character_BodyImpact_GibLarge_Cue'
}
