/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTGib_HumanArm extends UTGib_Human;

defaultproperties
{
	GibMeshesData[0]=(TheStaticMesh=None,TheSkelMesh=SkeletalMesh'CH_Gibs.Mesh.SK_CH_Gib_HandArm1',ThePhysAsset=PhysicsAsset'CH_Gibs.Mesh.SK_CH_Gib_HandArm1_Physics',DrawScale=1.3f,bUseSecondaryGibMeshMITV=TRUE)
	GibMeshesData[1]=(TheStaticMesh=None,TheSkelMesh=SkeletalMesh'CH_Gibs.Mesh.SK_CH_Gib_HandArm2',ThePhysAsset=PhysicsAsset'CH_Gibs.Mesh.SK_CH_Gib_HandArm2_Physics',DrawScale=1.3f,bUseSecondaryGibMeshMITV=TRUE)
	GibMeshesData[2]=(TheStaticMesh=StaticMesh'CH_Gibs.Mesh.S_CH_Gibs_Arm01',TheSkelMesh=None,ThePhysAsset=None,DrawScale=1.3f,bUseSecondaryGibMeshMITV=TRUE)
}
