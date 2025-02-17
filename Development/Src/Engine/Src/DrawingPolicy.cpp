/*=============================================================================
	DrawingPolicy.cpp: Base drawing policy implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"


FMeshDrawingPolicy::FMeshDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialRenderProxy* InMaterialRenderProxy,
	UBOOL bInOverrideWithShaderComplexity,
	UBOOL bInTwoSidedOverride,
	FLOAT InDepthBias
	):
	VertexFactory(InVertexFactory),
	MaterialRenderProxy(InMaterialRenderProxy),
	bIsTwoSidedMaterial(InMaterialRenderProxy->GetMaterial()->IsTwoSided() || bInTwoSidedOverride),
	bIsWireframeMaterial(InMaterialRenderProxy->GetMaterial()->IsWireframe()),
	bNeedsBackfacePass(
		(InMaterialRenderProxy->GetMaterial()->IsTwoSided() || bInTwoSidedOverride) &&
		(InMaterialRenderProxy->GetMaterial()->GetLightingModel() != MLM_NonDirectional) && 
		(InMaterialRenderProxy->GetMaterial()->GetLightingModel() != MLM_Unlit) &&
		!CanAccessFacingRegister(GRHIShaderPlatform)
		),
	//convert from signed UBOOL to unsigned BITFIELD
	bOverrideWithShaderComplexity(bInOverrideWithShaderComplexity != FALSE),
	DepthBias(InDepthBias)
{
}

void FMeshDrawingPolicy::SetMeshRenderState(
	const FSceneView& View,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	EmitMeshDrawEvents(PrimitiveSceneInfo, Mesh);

	const FRasterizerStateInitializerRHI Initializer = {
		(Mesh.bWireframe || IsWireframe()) ? FM_Wireframe : FM_Solid,
		(IsTwoSided() && !NeedsBackfacePass()) ? CM_None :
			(XOR(XOR(View.bReverseCulling,bBackFace), Mesh.ReverseCulling) ? CM_CCW : CM_CW),
		DepthBias + Mesh.DepthBias,
		Mesh.SlopeScaleDepthBias
	};
	RHISetRasterizerStateImmediate( Initializer);

	if( Mesh.bIsDecal && 
		Mesh.DecalState && 
		// no need to use a scissor rect for triangles already clipped to the decal frustum
		!Mesh.DecalState->bUseSoftwareClip &&
		!Mesh.bWireframe )
	{
		// Calculate the screen-space scissor rect that corresponds to the decal and set it.
		FVector2D MinCorner;
		FVector2D MaxCorner;
		if( Mesh.DecalState->QuadToClippedScreenSpaceAABB( &View, MinCorner, MaxCorner, Mesh.LocalToWorld ) )
		{
			RHISetScissorRect( TRUE, appTrunc(MinCorner.X), appTrunc(MinCorner.Y), appTrunc(MaxCorner.X), appTrunc(MaxCorner.Y) );
		}
		else
		{
			// Decal was projected outside screen or clip planes
			RHISetScissorRect( TRUE, 0, 0, 0, 0 );
		}
	}
}

void FMeshDrawingPolicy::DrawMesh(const FMeshElement& Mesh) const
{
	if (Mesh.UseDynamicData)
	{
		if (Mesh.ParticleType == PET_None)
		{
			check(Mesh.DynamicVertexData);

			if (Mesh.DynamicIndexData)
			{
				//@todo.SAS. This path has not been tested yet!
				RHIDrawIndexedPrimitiveUP(
					Mesh.Type,
					Mesh.MinVertexIndex,
					Mesh.MaxVertexIndex - Mesh.MinVertexIndex + 1,
					Mesh.NumPrimitives,
					Mesh.DynamicIndexData,
					Mesh.DynamicIndexStride,
					Mesh.DynamicVertexData,
					Mesh.DynamicVertexStride
					);
			}
			else
			{
				RHIDrawPrimitiveUP(
					Mesh.Type,
					Mesh.NumPrimitives,
					Mesh.DynamicVertexData,
					Mesh.DynamicVertexStride
					);
			}
		}
		else
		if (Mesh.ParticleType == PET_Sprite)
		{
			RHIDrawSpriteParticles(
				Mesh
				);
		}
		else
		if (Mesh.ParticleType == PET_SubUV)
		{
			RHIDrawSubUVParticles( 
				Mesh
				);
		}
	}
	else
	{
		if(Mesh.IndexBuffer)
		{
			// If the RHI supports it, and the mesh requests it, draw it with pre-vertex-shader culling.
			if(Mesh.bUsePreVertexShaderCulling)
			{
				RHIDrawIndexedPrimitive_PreVertexShaderCulling(
					Mesh.IndexBuffer->IndexBufferRHI,
					Mesh.Type,
					0,
					Mesh.MinVertexIndex,
					Mesh.MaxVertexIndex - Mesh.MinVertexIndex + 1,
					Mesh.FirstIndex,
					Mesh.NumPrimitives,
					Mesh.LocalToWorld
					);
			}
			else
			{
				RHIDrawIndexedPrimitive(
					Mesh.IndexBuffer->IndexBufferRHI,
					Mesh.Type,
					0,
					Mesh.MinVertexIndex,
					Mesh.MaxVertexIndex - Mesh.MinVertexIndex + 1,
					Mesh.FirstIndex,
					Mesh.NumPrimitives
					);
			}
		}
		else
		{
			RHIDrawPrimitive(
				Mesh.Type,
				Mesh.FirstIndex,
				Mesh.NumPrimitives
				);
		}
	}

	if ( Mesh.bIsDecal && Mesh.DecalState && !Mesh.DecalState->bUseSoftwareClip )
	{
		// Restore the scissor rect.
		RHISetScissorRect( FALSE, 0, 0, 0, 0 );
	}
}

void FMeshDrawingPolicy::DrawShared(const FSceneView* View) const
{
	check(VertexFactory && VertexFactory->IsInitialized());
	VertexFactory->Set();
}

/**
* Get the decl and stream strides for this mesh policy type and vertexfactory
* @param VertexDeclaration - output decl 
* @param StreamStrides - output array of vertex stream strides 
*/
void FMeshDrawingPolicy::GetVertexDeclarationInfo(FVertexDeclarationRHIRef& VertexDeclaration, DWORD *StreamStrides) const
{
	check(VertexFactory && VertexFactory->IsInitialized());
	VertexFactory->GetStreamStrides(StreamStrides);
	VertexDeclaration = VertexFactory->GetDeclaration();
	check(IsValidRef(VertexDeclaration));
}
