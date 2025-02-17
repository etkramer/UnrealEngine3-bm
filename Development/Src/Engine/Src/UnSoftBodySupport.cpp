/*=============================================================================
	UnSoftBodySupport.cpp: SoftBody support
	Copyright © 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EnginePhysicsClasses.h"

#if WITH_NOVODEX && !NX_DISABLE_SOFTBODY

#include "UnNovodexSupport.h"
#include "UnSoftBodySupport.h"

void ScaleSoftBodyTetras(const TArray<FVector>& InTetraVerts, TArray<FVector>& OutTetraVerts, FLOAT InScale)
{
	OutTetraVerts.Empty();
	OutTetraVerts.AddZeroed(InTetraVerts.Num());

	for(INT i=0; i<InTetraVerts.Num(); i++)
	{
		OutTetraVerts(i) = InTetraVerts(i) * InScale;
	}
}

#if WITH_NOVODEX && !NX_DISABLE_SOFTBODY

NxSoftBodyMesh* USkeletalMesh::GetSoftBodyMeshForScale(FLOAT InScale)
{
	check(CachedSoftBodyMeshes.Num() == CachedSoftBodyMeshScales.Num());

	// Look to see if we already have this mesh at this scale.
	for(INT i=0; i<CachedSoftBodyMeshes.Num(); i++)
	{
		if( Abs(InScale - CachedSoftBodyMeshScales(i)) < KINDA_SMALL_NUMBER )
		{
			return (NxSoftBodyMesh*)CachedSoftBodyMeshes(i);
		}
	}

	if(SoftBodySurfaceToGraphicsVertMap.Num() == 0)
	{
		debugf(TEXT("Cannot instantiate soft-body mesh, no soft-body vertices present."));
		return NULL;
	}

	TArray<FVector> TetraVerts;
	ScaleSoftBodyTetras(SoftBodyTetraVertsUnscaled, TetraVerts, InScale);

	check((SoftBodyTetraIndices.Num() % 4) == 0);

	NxSoftBodyMeshDesc SoftBodyMeshDesc;

	SoftBodyMeshDesc.numVertices = TetraVerts.Num();
	SoftBodyMeshDesc.numTetrahedra = SoftBodyTetraIndices.Num() / 4;
	SoftBodyMeshDesc.vertexStrideBytes = sizeof(NxVec3);
	SoftBodyMeshDesc.tetrahedronStrideBytes = sizeof(NxU32) * 4;
	SoftBodyMeshDesc.vertices = TetraVerts.GetData(); 
	SoftBodyMeshDesc.tetrahedra = SoftBodyTetraIndices.GetData();
	SoftBodyMeshDesc.flags = 0;
	SoftBodyMeshDesc.vertexMassStrideBytes = 0;
	SoftBodyMeshDesc.vertexFlagStrideBytes = 0;
	SoftBodyMeshDesc.vertexMasses = NULL;
	SoftBodyMeshDesc.vertexFlags = NULL;

	check(SoftBodyMeshDesc.isValid());

	TArray<BYTE> TempData;
	FNxMemoryBuffer Buffer(&TempData);
	bool bSuccess = GNovodexCooking->NxCookSoftBodyMesh(SoftBodyMeshDesc, Buffer);
	check(bSuccess);

	NxSoftBodyMesh* NewSoftBodyMesh = GNovodexSDK->createSoftBodyMesh(Buffer);
	check(NewSoftBodyMesh);

	CachedSoftBodyMeshes.AddItem( NewSoftBodyMesh );
	CachedSoftBodyMeshScales.AddItem( InScale );

	return NewSoftBodyMesh;
}

#endif //WITH_NOVODEX && !NX_DISABLE_SOFTBODY

/** Reset the store of cooked SoftBody meshes. Need to make sure you are not actually using any when you call this. */
void USkeletalMesh::ClearSoftBodyMeshCache()
{
#if WITH_NOVODEX && !NX_DISABLE_SOFTBODY
	for (INT i = 0; i < CachedSoftBodyMeshes.Num(); i++)
	{
		NxSoftBodyMesh* SM = (NxSoftBodyMesh*)CachedSoftBodyMeshes(i);
		check(SM);
		GNovodexPendingKillSoftBodyMesh.AddItem(SM);
	}
	CachedSoftBodyMeshes.Empty();
	CachedSoftBodyMeshScales.Empty();

#endif // WITH_NOVODEX && !NX_DISABLE_SOFTBODY
}

#endif // WITH_NOVODEX
