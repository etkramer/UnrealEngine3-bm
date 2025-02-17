/*=============================================================================
	UnStaticMeshRenderXe.h: Xbox specific optimizations

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

//@todo joeg -- Have better reuse via templates across primitive types

/**
 * Uses intrinsics to calculate the light->triangle plane dot products.
 *
 * @param NumTriangles the number of triangles to calculate the dot product for
 * @param LightPosition the location of the light
 * @param RenderData static mesh vertex data
 * @param Indices pointer to the index buffer data
 * @param PlaneDots the out array that gets the data
 */
FORCEINLINE void GetPlaneDotsXbox(INT NumTriangles,const FVector4& LightPosition,
	const FStaticMeshRenderData& RenderData,WORD* RESTRICT Indices,
	FLOAT* RESTRICT PlaneDots)
{
#if ENABLE_VECTORINTRINSICS
	// Light position doesn't change so load once
	VectorRegister LightPos = VectorLoad( &LightPosition );
	// Determine whether the dot products are negative or not for each triangle
	for (INT TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
	{
		// Load all SIMD registers
		VectorRegister V1 = VectorLoadFloat3( &RenderData.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 0]) );
		VectorRegister V2 = VectorLoadFloat3( &RenderData.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 1]) );
		VectorRegister V3 = VectorLoadFloat3( &RenderData.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 2]) );
		// Get the face normal
		VectorRegister FaceNormal = VectorCross( VectorSubtract(V2,V3), VectorSubtract(V1,V3) );
		// Get light vector
		VectorRegister LightV1 = VectorSubtract( LightPos, VectorMultiply(V1, VectorReplicate(LightPos,3)) );
		// Dot the two
		VectorRegister PlaneDot = VectorDot3( FaceNormal, LightV1 );
		// Store the data without reading the destination address
		VectorStoreFloat1( PlaneDot, PlaneDots );
		// Move to the next dot product slot
		PlaneDots++;
	}
#else
	for(INT TriangleIndex = 0;TriangleIndex < NumTriangles;TriangleIndex++)
	{
		const FVector& V1 = RenderData.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 0]);
		const FVector& V2 = RenderData.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 1]);
		const FVector& V3 = RenderData.PositionVertexBuffer.VertexPosition(Indices[TriangleIndex * 3 + 2]);
		*PlaneDots++ = ((V2-V3) ^ (V1-V3)) | (FVector(LightPosition) - V1 * LightPosition.W);
	}
#endif
}
