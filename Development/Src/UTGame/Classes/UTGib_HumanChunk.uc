/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTGib_HumanChunk extends UTGib_Human;


defaultproperties
{
	GibMeshesData[0]=(TheStaticMesh=StaticMesh'CH_Gore.S_CH_Head_Chunk1',TheSkelMesh=None,ThePhysAsset=None,DrawScale=2.0)
	GibMeshesData[1]=(TheStaticMesh=StaticMesh'CH_Gore.S_CH_Head_Chunk3',TheSkelMesh=None,ThePhysAsset=None,DrawScale=2.0)
	GibMeshesData[2]=(TheStaticMesh=StaticMesh'CH_Gore.S_CH_Head_Chunk4',TheSkelMesh=None,ThePhysAsset=None,DrawScale=2.0)
	GibMeshesData[3]=(TheStaticMesh=StaticMesh'CH_Gibs.Mesh.S_CH_Gibs_Slab03',TheSkelMesh=None,ThePhysAsset=None,DrawScale=1.3f)
	GibMeshesData[4]=(TheStaticMesh=StaticMesh'CH_Gibs.Mesh.S_CH_Gibs_Slab04',TheSkelMesh=None,ThePhysAsset=None,DrawScale=1.3f)
	GibMeshesData[5]=(TheStaticMesh=StaticMesh'CH_Gibs.Mesh.S_CH_Gibs_Slab05',TheSkelMesh=None,ThePhysAsset=None,DrawScale=1.3f)
	GibMeshesData[6]=(TheStaticMesh=None,TheSkelMesh=SkeletalMesh'CH_Gibs.Mesh.SK_CH_Gib_Chunk1',ThePhysAsset=PhysicsAsset'CH_Gibs.Mesh.SK_CH_Gib_Chunk2_Physics',DrawScale=1.3f,bUseSecondaryGibMeshMITV=TRUE)
}

