/*=============================================================================
 Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
#include "MeshUtils.h"

#if _WINDOWS
#include "../../D3D9Drv/Inc/D3D9Drv.h"
#endif

UBOOL GenerateLOD(UStaticMesh* SourceStaticMesh, UStaticMesh* DestStaticMesh, INT DesiredLOD, INT DesiredTriangles)
{
#if _WINDOWS
	FD3DMeshUtilities MeshUtils;
	return MeshUtils.GenerateLOD( SourceStaticMesh, DestStaticMesh, DesiredLOD, DesiredTriangles);
#endif
	return FALSE;
}

UBOOL GenerateUVs(
	UStaticMesh* StaticMesh,
	UINT LODIndex,
	UINT TexCoordIndex,
	UBOOL bKeepExistingCoordinates,
	FLOAT &MaxDesiredStretch,
	UINT &ChartsGenerated
	)
{
#if _WINDOWS
	FD3DMeshUtilities MeshUtils;
	return MeshUtils.GenerateUVs(StaticMesh, LODIndex, TexCoordIndex, bKeepExistingCoordinates, MaxDesiredStretch, ChartsGenerated);
#endif
	return FALSE;
}

