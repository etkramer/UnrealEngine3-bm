/*=============================================================================
 Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


/**
*
* Utility helpers. This builds on D3DX functionality on appropriate platforms, otherwise does nothing.
* Used for editor helpers.
*
*/
UBOOL GenerateLOD(UStaticMesh* SourceStaticMesh, UStaticMesh* DestStaticMesh, INT DesiredLOD, INT DesiredTriangles);

UBOOL GenerateUVs(
	UStaticMesh* StaticMesh,
	UINT LODIndex,
	UINT TexCoordIndex,
	UBOOL bKeepExistingCoordinates,
	FLOAT &MaxDesiredStretch,
	UINT &ChartsGenerated
	);

