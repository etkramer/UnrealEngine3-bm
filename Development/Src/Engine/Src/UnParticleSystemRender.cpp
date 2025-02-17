/*=============================================================================
	UnParticleSystemRender.cpp: Particle system rendering functions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
#include "EngineParticleClasses.h"
#include "EngineMaterialClasses.h"

#include "UnParticleHelper.h"

#include "ParticleInstancedMeshVertexFactory.h"

//@todo.SAS. Remove this once the Trail packing bug is corrected.
#include "ScenePrivate.h"


#define LOG_DETAILED_PARTICLE_RENDER_STATS 0

#if LOG_DETAILED_PARTICLE_RENDER_STATS 
/** Global detailed update stats. */
static FDetailedTickStats GDetailedParticleRenderStats( 20, 10, 1, 4, TEXT("rendering") );
#define TRACK_DETAILED_PARTICLE_RENDER_STATS(Object) FScopedDetailTickStats DetailedTickStats(GDetailedParticleRenderStats,Object);
#else
#define TRACK_DETAILED_PARTICLE_RENDER_STATS(Object)
#endif

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_COMPARE_CONSTREF(FParticleOrder,UnParticleComponents,{ return A.Z < B.Z ? 1 : -1; });

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FDynamicSpriteEmitterDataBase::RenderDebug(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, UBOOL bCrosses)
{
	check(SceneProxy);

	const FDynamicSpriteEmitterReplayData& SpriteSource =
		static_cast< const FDynamicSpriteEmitterReplayData& >( GetSource() );

	const FMatrix& LocalToWorld = SpriteSource.bUseLocalSpace ? SceneProxy->GetLocalToWorld() : FMatrix::Identity;

	FMatrix CameraToWorld = View->ViewMatrix.Inverse();
	FVector CamX = CameraToWorld.TransformNormal(FVector(1,0,0));
	FVector CamY = CameraToWorld.TransformNormal(FVector(0,1,0));

	FLinearColor EmitterEditorColor = FLinearColor(1.0f,1.0f,0);

	for (INT i = 0; i < SpriteSource.ActiveParticleCount; i++)
	{
		DECLARE_PARTICLE(Particle, SpriteSource.ParticleData.GetData() + SpriteSource.ParticleStride * SpriteSource.ParticleIndices(i));

		FVector DrawLocation = LocalToWorld.TransformFVector(Particle.Location);
		if (bCrosses)
		{
			FVector Size = Particle.Size * SpriteSource.Scale;
			PDI->DrawLine(DrawLocation - (0.5f * Size.X * CamX), DrawLocation + (0.5f * Size.X * CamX), EmitterEditorColor, DPGIndex);
			PDI->DrawLine(DrawLocation - (0.5f * Size.Y * CamY), DrawLocation + (0.5f * Size.Y * CamY), EmitterEditorColor, DPGIndex);
		}
		else
		{
			PDI->DrawPoint(DrawLocation, EmitterEditorColor, 2, DPGIndex);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	ParticleMeshEmitterInstance
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	FDynamicSpriteEmitterData
///////////////////////////////////////////////////////////////////////////////

/** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
void FDynamicSpriteEmitterData::Init( UBOOL bInSelected )
{
	bSelected = bInSelected;

	bUsesDynamicParameter = FALSE;
	if( Source.MaterialInterface->GetMaterialResource() != NULL )
	{
		bUsesDynamicParameter =
			Source.MaterialInterface->GetMaterialResource()->GetUsesDynamicParameter();
	}

	MaterialResource =
		Source.MaterialInterface->GetRenderProxy( bSelected );

	// We won't need this on the render thread
	Source.MaterialInterface = NULL;
}


UBOOL FDynamicSpriteEmitterData::GetVertexAndIndexData(void* VertexData, void* FillIndexData, TArray<FParticleOrder>* ParticleOrder)
{
	INT ParticleCount = Source.ActiveParticleCount;
	// 'clamp' the number of particles actually drawn
	//@todo.SAS. If sorted, we really want to render the front 'N' particles...
	// right now it renders the back ones. (Same for SubUV draws)
	if ((Source.MaxDrawCount >= 0) && (ParticleCount > Source.MaxDrawCount))
	{
		ParticleCount = Source.MaxDrawCount;
	}

	// Pack the data
	INT	ParticleIndex;
	INT	ParticlePackingIndex = 0;
	INT	IndexPackingIndex = 0;

	INT VertexStride = sizeof(FParticleSpriteVertex);
	if (bUsesDynamicParameter)
	{
		VertexStride = sizeof(FParticleSpriteVertexDynamicParameter);
	}
	BYTE* TempVert = (BYTE*)VertexData;
	WORD* Indices = (WORD*)FillIndexData;
	FParticleSpriteVertex* FillVertex;
	FParticleSpriteVertexDynamicParameter* DynFillVertex;

	UBOOL bSorted = ParticleOrder ? TRUE : FALSE;

	for (INT i = 0; i < ParticleCount; i++)
	{
		if (bSorted)
		{
			ParticleIndex	= (*ParticleOrder)(i).ParticleIndex;
		}
		else
		{
			ParticleIndex	= i;
		}

		DECLARE_PARTICLE(Particle, Source.ParticleData.GetData() + Source.ParticleStride * Source.ParticleIndices(ParticleIndex));

		if (i + 1 < ParticleCount)
		{
			INT NextIndex = bSorted ? (*ParticleOrder)(i+1).ParticleIndex : i+1;
			DECLARE_PARTICLE(NextParticle, Source.ParticleData.GetData() + Source.ParticleStride * Source.ParticleIndices(NextIndex));
			PREFETCH(&NextParticle);
		}

		FVector Size = Particle.Size * Source.Scale;
		if (Source.ScreenAlignment == PSA_Square)
		{
			Size.Y = Size.X;
		}

		FOrbitChainModuleInstancePayload* LocalOrbitPayload = NULL;
		FVector OrbitOffset(0.0f, 0.0f, 0.0f);
		FVector PrevOrbitOffset(0.0f, 0.0f, 0.0f);

		if (Source.OrbitModuleOffset != 0)
		{
			INT CurrentOffset = Source.OrbitModuleOffset;
			const BYTE* ParticleBase = (const BYTE*)&Particle;
			PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);
			OrbitOffset = OrbitPayload.Offset;

			if (Source.bUseLocalSpace == FALSE)
			{
				OrbitOffset = SceneProxy->GetLocalToWorld().TransformNormal(OrbitOffset);
			}
			PrevOrbitOffset = OrbitPayload.PreviousOffset;

			LocalOrbitPayload = &OrbitPayload;
		}

		// 0
		FillVertex = (FParticleSpriteVertex*)TempVert;
		FillVertex->Position	= Particle.Location + OrbitOffset;
		FillVertex->OldPosition	= Particle.OldLocation + PrevOrbitOffset;
		FillVertex->Size		= Size;
		FillVertex->Tex_U		= 0.f;
		FillVertex->Tex_V		= 0.f;
		FillVertex->Rotation	= Particle.Rotation;
		FillVertex->Color		= Particle.Color;
		if (bUsesDynamicParameter)
		{
			DynFillVertex = (FParticleSpriteVertexDynamicParameter*)TempVert;
			if (Source.DynamicParameterDataOffset > 0)
			{
				FEmitterDynamicParameterPayload* DynPayload = ((FEmitterDynamicParameterPayload*)((BYTE*)(&Particle) + Source.DynamicParameterDataOffset));
				DynFillVertex->DynamicValue = DynPayload->DynamicParameterValue;
			}
			else
			{
				DynFillVertex->DynamicValue = FVector4(1.0f, 1.0f, 1.0f, 1.0f);;
			}
		}
		TempVert += VertexStride;
		// 1
		FillVertex = (FParticleSpriteVertex*)TempVert;
		FillVertex->Position	= Particle.Location + OrbitOffset;
		FillVertex->OldPosition	= Particle.OldLocation + PrevOrbitOffset;
		FillVertex->Size		= Size;
		FillVertex->Tex_U		= 0.f;
		FillVertex->Tex_V		= 1.f;
		FillVertex->Rotation	= Particle.Rotation;
		FillVertex->Color		= Particle.Color;
		if (bUsesDynamicParameter)
		{
			DynFillVertex = (FParticleSpriteVertexDynamicParameter*)TempVert;
			if (Source.DynamicParameterDataOffset > 0)
			{
				FEmitterDynamicParameterPayload* DynPayload = ((FEmitterDynamicParameterPayload*)((BYTE*)(&Particle) + Source.DynamicParameterDataOffset));
				DynFillVertex->DynamicValue = DynPayload->DynamicParameterValue;
			}
			else
			{
				DynFillVertex->DynamicValue = FVector4(1.0f, 1.0f, 1.0f, 1.0f);;
			}
		}
		TempVert += VertexStride;
		// 2
		FillVertex = (FParticleSpriteVertex*)TempVert;
		FillVertex->Position	= Particle.Location + OrbitOffset;
		FillVertex->OldPosition	= Particle.OldLocation + PrevOrbitOffset;
		FillVertex->Size		= Size;
		FillVertex->Tex_U		= 1.f;
		FillVertex->Tex_V		= 1.f;
		FillVertex->Rotation	= Particle.Rotation;
		FillVertex->Color		= Particle.Color;
		if (bUsesDynamicParameter)
		{
			DynFillVertex = (FParticleSpriteVertexDynamicParameter*)TempVert;
			if (Source.DynamicParameterDataOffset > 0)
			{
				FEmitterDynamicParameterPayload* DynPayload = ((FEmitterDynamicParameterPayload*)((BYTE*)(&Particle) + Source.DynamicParameterDataOffset));
				DynFillVertex->DynamicValue = DynPayload->DynamicParameterValue;
			}
			else
			{
				DynFillVertex->DynamicValue = FVector4(1.0f, 1.0f, 1.0f, 1.0f);;
			}
		}
		TempVert += VertexStride;
		// 3
		FillVertex = (FParticleSpriteVertex*)TempVert;
		FillVertex->Position	= Particle.Location + OrbitOffset;
		FillVertex->OldPosition	= Particle.OldLocation + PrevOrbitOffset;
		FillVertex->Size		= Size;
		FillVertex->Tex_U		= 1.f;
		FillVertex->Tex_V		= 0.f;
		FillVertex->Rotation	= Particle.Rotation;
		FillVertex->Color		= Particle.Color;
		if (bUsesDynamicParameter)
		{
			DynFillVertex = (FParticleSpriteVertexDynamicParameter*)TempVert;
			if (Source.DynamicParameterDataOffset > 0)
			{
				FEmitterDynamicParameterPayload* DynPayload = ((FEmitterDynamicParameterPayload*)((BYTE*)(&Particle) + Source.DynamicParameterDataOffset));
				DynFillVertex->DynamicValue = DynPayload->DynamicParameterValue;
			}
			else
			{
				DynFillVertex->DynamicValue = FVector4(1.0f, 1.0f, 1.0f, 1.0f);;
			}
		}
		TempVert += VertexStride;

		if (Indices)
		{
			*Indices++ = (i * 4) + 0;
			*Indices++ = (i * 4) + 2;
			*Indices++ = (i * 4) + 3;
			*Indices++ = (i * 4) + 0;
			*Indices++ = (i * 4) + 1;
			*Indices++ = (i * 4) + 2;
		}

		if (LocalOrbitPayload)
		{
			LocalOrbitPayload->PreviousOffset = OrbitOffset;
		}
	}

	return TRUE;
}

void FDynamicSpriteEmitterData::Render(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_SpriteRenderingTime);

	if (bValid == FALSE)
	{
		return;
	}

	if (Source.EmitterRenderMode == ERM_Normal)
	{
		// Don't render if the material will be ignored
		if (PDI->IsMaterialIgnored(MaterialResource) && !(View->Family->ShowFlags & SHOW_Wireframe))
		{
			return;
		}
		FMatrix LocalToWorld = Proxy->GetLocalToWorld();

		VertexFactory->SetScreenAlignment(Source.ScreenAlignment);
		VertexFactory->SetLockAxesFlag(Source.LockAxisFlag);
		if (Source.LockAxisFlag != EPAL_NONE)
		{
			FVector Up, Right;
			Proxy->GetAxisLockValues((FDynamicSpriteEmitterDataBase*)this, Up, Right);
			VertexFactory->SetLockAxes(Up, Right);
		}

		INT ParticleCount = Source.ActiveParticleCount;

		UBOOL bSorted = FALSE;
		if( Source.bRequiresSorting )
		{
			// If material is using unlit translucency and the blend mode is translucent or 
			// if it is using unlit distortion then we need to sort (back to front)
			const FMaterial* Material = MaterialResource->GetMaterial();
			if (Material && 
				Material->GetLightingModel() == MLM_Unlit && 
				(Material->GetBlendMode() == BLEND_Translucent || Material->IsDistorted())
				)
			{
				SCOPE_CYCLE_COUNTER(STAT_SortingTime);

				ParticleOrder.Empty(ParticleCount);

				// Take UseLocalSpace into account!
				UBOOL bLocalSpace = Source.bUseLocalSpace;

				for (INT ParticleIndex = 0; ParticleIndex < ParticleCount; ParticleIndex++)
				{
					DECLARE_PARTICLE(Particle, Source.ParticleData.GetData() + Source.ParticleStride * Source.ParticleIndices(ParticleIndex));
					FLOAT InZ;
					if (bLocalSpace)
					{
						InZ = View->ViewProjectionMatrix.TransformFVector(LocalToWorld.TransformFVector(Particle.Location)).Z;
					}
					else
					{
						InZ = View->ViewProjectionMatrix.TransformFVector(Particle.Location).Z;
					}
					new(ParticleOrder)FParticleOrder(ParticleIndex, InZ);
				}
				Sort<USE_COMPARE_CONSTREF(FParticleOrder,UnParticleComponents)>(&(ParticleOrder(0)),ParticleOrder.Num());
				bSorted	= TRUE;
			}
		}

		FMeshElement Mesh;

		Mesh.UseDynamicData			= TRUE;
		Mesh.IndexBuffer			= NULL;
		Mesh.VertexFactory			= VertexFactory;
		Mesh.DynamicVertexData		= this;
		if (bUsesDynamicParameter == FALSE)
		{
			Mesh.DynamicVertexStride	= sizeof(FParticleSpriteVertex);
		}
		else
		{
			Mesh.DynamicVertexStride	= sizeof(FParticleSpriteVertexDynamicParameter);
		}
		Mesh.DynamicIndexData		= NULL;
		Mesh.DynamicIndexStride		= 0;
		if (bSorted == TRUE)
		{
			Mesh.DynamicIndexData	= (void*)(&(ParticleOrder));
		}
		Mesh.LCI = NULL;
		if (Source.bUseLocalSpace == TRUE)
		{
			Mesh.LocalToWorld = LocalToWorld;
			Mesh.WorldToLocal = Proxy->GetWorldToLocal();
		}
		else
		{
			Mesh.LocalToWorld = FMatrix::Identity;
			Mesh.WorldToLocal = FMatrix::Identity;
		}
		Mesh.FirstIndex				= 0;
		Mesh.MinVertexIndex			= 0;
		Mesh.MaxVertexIndex			= (ParticleCount * 4) - 1;
		Mesh.ParticleType			= PET_Sprite;
		Mesh.ReverseCulling			= Proxy->GetLocalToWorldDeterminant() < 0.0f ? TRUE : FALSE;
		Mesh.CastShadow				= Proxy->GetCastShadow();
		Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;

		Mesh.MaterialRenderProxy	= MaterialResource;
		Mesh.NumPrimitives			= ParticleCount;
		Mesh.Type					= PT_TriangleList;

		DrawRichMesh(
			PDI, 
			Mesh, 
			FLinearColor(1.0f, 0.0f, 0.0f),	//WireframeColor,
			FLinearColor(1.0f, 1.0f, 0.0f),	//LevelColor,
			FLinearColor(1.0f, 1.0f, 1.0f),	//PropertyColor,		
			Proxy->GetPrimitiveSceneInfo(),
			Proxy->GetSelected()
			);
	}
	else
	if (Source.EmitterRenderMode == ERM_Point)
	{
		RenderDebug(PDI, View, DPGIndex, FALSE);
	}
	else
	if (Source.EmitterRenderMode == ERM_Cross)
	{
		RenderDebug(PDI, View, DPGIndex, TRUE);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	FDynamicSubUVEmitterData
///////////////////////////////////////////////////////////////////////////////

/** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
void FDynamicSubUVEmitterData::Init( UBOOL bInSelected )
{
	bSelected = bInSelected;

	bUsesDynamicParameter = FALSE;
	if( Source.MaterialInterface->GetMaterialResource() != NULL )
	{
		bUsesDynamicParameter =
			Source.MaterialInterface->GetMaterialResource()->GetUsesDynamicParameter();
	}

	MaterialResource =
		Source.MaterialInterface->GetRenderProxy( bSelected );

	// We won't need this on the render thread
	Source.MaterialInterface = NULL;
}


UBOOL FDynamicSubUVEmitterData::GetVertexAndIndexData(void* VertexData, void* FillIndexData, TArray<FParticleOrder>* ParticleOrder)
{
	INT ParticleCount = Source.ActiveParticleCount;
	// 'clamp' the number of particles actually drawn
	//@todo.SAS. If sorted, we really want to render the front 'N' particles...
	// right now it renders the back ones. (Same for SubUV draws)
	if ((Source.MaxDrawCount >= 0) && (ParticleCount > Source.MaxDrawCount))
	{
		ParticleCount = Source.MaxDrawCount;
	}

	// Pack the data
	INT	ParticleIndex;
	INT	ParticlePackingIndex = 0;
	INT	IndexPackingIndex = 0;

	INT			SIHorz			= Source.SubImages_Horizontal;
	INT			SIVert			= Source.SubImages_Vertical;
	INT			iTotalSubImages = SIHorz * SIVert;
	FLOAT		baseU			= (1.0f / (FLOAT)SIHorz);
	FLOAT		baseV			= (1.0f / (FLOAT)SIVert);
	FLOAT		U;
	FLOAT		V;

	INT VertexStride = sizeof(FParticleSpriteSubUVVertex);
	if (bUsesDynamicParameter)
	{
		VertexStride = sizeof(FParticleSpriteSubUVVertexDynamicParameter);
	}
	BYTE* TempVert = (BYTE*)VertexData;
	WORD* Indices = (WORD*)FillIndexData;
	FParticleSpriteSubUVVertex* FillVertex;
	FParticleSpriteSubUVVertexDynamicParameter* DynFillVertex;

	UBOOL bSorted = ParticleOrder ? TRUE : FALSE;
	for (INT i = 0; i < ParticleCount; i++)
	{
		if (bSorted)
		{
			ParticleIndex	= (*ParticleOrder)(i).ParticleIndex;
		}
		else
		{
			ParticleIndex	= i;
		}

		DECLARE_PARTICLE(Particle, Source.ParticleData.GetData() + Source.ParticleStride * Source.ParticleIndices(ParticleIndex));

		if (i + 1 < ParticleCount)
		{
			INT NextIndex = bSorted ? (*ParticleOrder)(i+1).ParticleIndex : i+1;
			DECLARE_PARTICLE(NextParticle, Source.ParticleData.GetData() + Source.ParticleStride * Source.ParticleIndices(NextIndex));
			PREFETCH(&NextParticle);
		}

		FVector Size = Particle.Size * Source.Scale;
		if (Source.ScreenAlignment == PSA_Square)
		{
			Size.Y = Size.X;
		}

		FOrbitChainModuleInstancePayload* LocalOrbitPayload = NULL;
		FVector OrbitOffset(0.0f, 0.0f, 0.0f);
		FVector PrevOrbitOffset(0.0f, 0.0f, 0.0f);

		if (Source.OrbitModuleOffset != 0)
		{
			INT CurrentOffset = Source.OrbitModuleOffset;
			const BYTE* ParticleBase = (const BYTE*)&Particle;
			PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);
			OrbitOffset = OrbitPayload.Offset;

			if (Source.bUseLocalSpace == FALSE)
			{
				OrbitOffset = SceneProxy->GetLocalToWorld().TransformNormal(OrbitOffset);
			}
			PrevOrbitOffset = OrbitPayload.PreviousOffset;

			LocalOrbitPayload = &OrbitPayload;
		}

		FFullSubUVPayload* PayloadData = (FFullSubUVPayload*)(((BYTE*)&Particle) + Source.SubUVDataOffset);

		// 0
		FillVertex = (FParticleSpriteSubUVVertex*)TempVert;
		FillVertex->Position	= Particle.Location + OrbitOffset;
		FillVertex->OldPosition	= Particle.OldLocation + PrevOrbitOffset;
		FillVertex->Size		= Size;
		FillVertex->Rotation	= Particle.Rotation;
		FillVertex->Color		= Particle.Color;
		FillVertex->Interp		= PayloadData->ImageHVInterp_UVOffset.Z;
		if (Source.bDirectUV)
		{
			U	= baseU * (PayloadData->ImageHVInterp_UVOffset.X + 0);
			V	= baseV * (PayloadData->ImageHVInterp_UVOffset.Y + 0);
    		FillVertex->Tex_U	= U;
	    	FillVertex->Tex_V	= V;
			FillVertex->Tex_U2	= U;
			FillVertex->Tex_V2	= V;
		}
		else
		{
    		FillVertex->Tex_U	= baseU * (appTruncFloat(PayloadData->ImageHVInterp_UVOffset.X) + 0.0f);
	    	FillVertex->Tex_V	= baseV * (appTruncFloat(PayloadData->ImageHVInterp_UVOffset.Y) + 0.0f);
			FillVertex->Tex_U2	= baseU * (appTruncFloat(PayloadData->Image2HV_UV2Offset.X) + 0.0f);
			FillVertex->Tex_V2	= baseV * (appTruncFloat(PayloadData->Image2HV_UV2Offset.Y) + 0.0f);
		}
		FillVertex->SizeU		= 0.f;
		FillVertex->SizeV		= 0.f;
		if (bUsesDynamicParameter)
		{
			DynFillVertex = (FParticleSpriteSubUVVertexDynamicParameter*)TempVert;
			if (Source.DynamicParameterDataOffset > 0)
			{
				FEmitterDynamicParameterPayload* DynPayload = ((FEmitterDynamicParameterPayload*)((BYTE*)(&Particle) + Source.DynamicParameterDataOffset));
				DynFillVertex->DynamicValue = DynPayload->DynamicParameterValue;
			}
			else
			{
				DynFillVertex->DynamicValue = FVector4(1.0f, 1.0f, 1.0f, 1.0f);;
			}
		}
		TempVert += VertexStride;
		// 1
		FillVertex = (FParticleSpriteSubUVVertex*)TempVert;
		FillVertex->Position	= Particle.Location + OrbitOffset;
		FillVertex->OldPosition	= Particle.OldLocation + PrevOrbitOffset;
		FillVertex->Size		= Size;
		FillVertex->Rotation	= Particle.Rotation;
		FillVertex->Color		= Particle.Color;
		FillVertex->Interp		= PayloadData->ImageHVInterp_UVOffset.Z;
		if (Source.bDirectUV)
		{
			U	= baseU * (PayloadData->ImageHVInterp_UVOffset.X + 0);
			V	= baseV * (PayloadData->ImageHVInterp_UVOffset.Y + PayloadData->Image2HV_UV2Offset.Y);
			FillVertex->Tex_U	= U;
			FillVertex->Tex_V	= V;
			FillVertex->Tex_U2	= U;
			FillVertex->Tex_V2	= V;
		}
		else
		{
			FillVertex->Tex_U	= baseU * (appTruncFloat(PayloadData->ImageHVInterp_UVOffset.X) + 0.0f);
			FillVertex->Tex_V	= baseV * (appTruncFloat(PayloadData->ImageHVInterp_UVOffset.Y) + 1.0f);
			FillVertex->Tex_U2	= baseU * (appTruncFloat(PayloadData->Image2HV_UV2Offset.X) + 0.0f);
			FillVertex->Tex_V2	= baseV * (appTruncFloat(PayloadData->Image2HV_UV2Offset.Y) + 1.0f);
		}
		FillVertex->SizeU		= 0.f;
		FillVertex->SizeV		= 1.f;
		if (bUsesDynamicParameter)
		{
			DynFillVertex = (FParticleSpriteSubUVVertexDynamicParameter*)TempVert;
			if (Source.DynamicParameterDataOffset > 0)
			{
				FEmitterDynamicParameterPayload* DynPayload = ((FEmitterDynamicParameterPayload*)((BYTE*)(&Particle) + Source.DynamicParameterDataOffset));
				DynFillVertex->DynamicValue = DynPayload->DynamicParameterValue;
			}
			else
			{
				DynFillVertex->DynamicValue = FVector4(1.0f, 1.0f, 1.0f, 1.0f);;
			}
		}
		TempVert += VertexStride;
		// 2
		FillVertex = (FParticleSpriteSubUVVertex*)TempVert;
		FillVertex->Position	= Particle.Location + OrbitOffset;
		FillVertex->OldPosition	= Particle.OldLocation + PrevOrbitOffset;
		FillVertex->Size		= Size;
		FillVertex->Rotation	= Particle.Rotation;
		FillVertex->Color		= Particle.Color;
		FillVertex->Interp		= PayloadData->ImageHVInterp_UVOffset.Z;
		if (Source.bDirectUV)
		{
			U	= baseU * (PayloadData->ImageHVInterp_UVOffset.X + PayloadData->Image2HV_UV2Offset.X);
			V	= baseV * (PayloadData->ImageHVInterp_UVOffset.Y + PayloadData->Image2HV_UV2Offset.Y);
    		FillVertex->Tex_U	= U;
	    	FillVertex->Tex_V	= V;
			FillVertex->Tex_U2	= U;
			FillVertex->Tex_V2	= V;
		}
		else
		{
    		FillVertex->Tex_U	= baseU * (appTruncFloat(PayloadData->ImageHVInterp_UVOffset.X) + 1.0f);
	    	FillVertex->Tex_V	= baseV * (appTruncFloat(PayloadData->ImageHVInterp_UVOffset.Y) + 1.0f);
			FillVertex->Tex_U2	= baseU * (appTruncFloat(PayloadData->Image2HV_UV2Offset.X) + 1.0f);
			FillVertex->Tex_V2	= baseV * (appTruncFloat(PayloadData->Image2HV_UV2Offset.Y) + 1.0f);
		}
		FillVertex->SizeU		= 1.f;
		FillVertex->SizeV		= 1.f;
		if (bUsesDynamicParameter)
		{
			DynFillVertex = (FParticleSpriteSubUVVertexDynamicParameter*)TempVert;
			if (Source.DynamicParameterDataOffset > 0)
			{
				FEmitterDynamicParameterPayload* DynPayload = ((FEmitterDynamicParameterPayload*)((BYTE*)(&Particle) + Source.DynamicParameterDataOffset));
				DynFillVertex->DynamicValue = DynPayload->DynamicParameterValue;
			}
			else
			{
				DynFillVertex->DynamicValue = FVector4(1.0f, 1.0f, 1.0f, 1.0f);;
			}
		}
		TempVert += VertexStride;
		// 3
		FillVertex = (FParticleSpriteSubUVVertex*)TempVert;
		FillVertex->Position	= Particle.Location + OrbitOffset;
		FillVertex->OldPosition	= Particle.OldLocation + PrevOrbitOffset;
		FillVertex->Size		= Size;
		FillVertex->Rotation	= Particle.Rotation;
		FillVertex->Color		= Particle.Color;
		FillVertex->Interp		= PayloadData->ImageHVInterp_UVOffset.Z;
		if (Source.bDirectUV)
		{
			U	= baseU * (PayloadData->ImageHVInterp_UVOffset.X + PayloadData->Image2HV_UV2Offset.X);
			V	= baseV * (PayloadData->ImageHVInterp_UVOffset.Y + 0);
    		FillVertex->Tex_U	= U;
	    	FillVertex->Tex_V	= V;
			FillVertex->Tex_U2	= U;
			FillVertex->Tex_V2	= V;
		}
		else
		{
    		FillVertex->Tex_U	= baseU * (appTruncFloat(PayloadData->ImageHVInterp_UVOffset.X) + 1.0f);
	    	FillVertex->Tex_V	= baseV * (appTruncFloat(PayloadData->ImageHVInterp_UVOffset.Y) + 0.0f);
			FillVertex->Tex_U2	= baseU * (appTruncFloat(PayloadData->Image2HV_UV2Offset.X) + 1.0f);
			FillVertex->Tex_V2	= baseV * (appTruncFloat(PayloadData->Image2HV_UV2Offset.Y) + 0.0f);
		}
		FillVertex->SizeU		= 1.f;
		FillVertex->SizeV		= 0.f;
		if (bUsesDynamicParameter)
		{
			DynFillVertex = (FParticleSpriteSubUVVertexDynamicParameter*)TempVert;
			if (Source.DynamicParameterDataOffset > 0)
			{
				FEmitterDynamicParameterPayload* DynPayload = ((FEmitterDynamicParameterPayload*)((BYTE*)(&Particle) + Source.DynamicParameterDataOffset));
				DynFillVertex->DynamicValue = DynPayload->DynamicParameterValue;
			}
			else
			{
				DynFillVertex->DynamicValue = FVector4(1.0f, 1.0f, 1.0f, 1.0f);;
			}
		}
		TempVert += VertexStride;

		if (Indices)
		{
			*Indices++ = (i * 4) + 0;
			*Indices++ = (i * 4) + 2;
			*Indices++ = (i * 4) + 3;
			*Indices++ = (i * 4) + 0;
			*Indices++ = (i * 4) + 1;
			*Indices++ = (i * 4) + 2;
		}

		if (LocalOrbitPayload)
		{
			LocalOrbitPayload->PreviousOffset = OrbitOffset;
		}
	}

	return TRUE;
}

void FDynamicSubUVEmitterData::Render(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_SpriteRenderingTime);

	if (bValid == FALSE)
	{
		return;
	}

	if (Source.EmitterRenderMode == ERM_Normal)
	{
		// Don't render if the material will be ignored
		if (PDI->IsMaterialIgnored(MaterialResource) && !(View->Family->ShowFlags & SHOW_Wireframe))
		{
			return;
		}

		FMatrix LocalToWorld = Proxy->GetLocalToWorld();

		VertexFactory->SetScreenAlignment(Source.ScreenAlignment);
		VertexFactory->SetLockAxesFlag(Source.LockAxisFlag);
		if (Source.LockAxisFlag != EPAL_NONE)
		{
			FVector Up, Right;
			Proxy->GetAxisLockValues((FDynamicSpriteEmitterDataBase*)this, Up, Right);
			VertexFactory->SetLockAxes(Up, Right);
		}

		INT ParticleCount = Source.ActiveParticleCount;

		UBOOL bSorted = FALSE;
		if( Source.bRequiresSorting )
		{
			// If material is using unlit translucency and the blend mode is translucent or 
			// if it is using unlit distortion then we need to sort (back to front)
			const FMaterial* Material = MaterialResource->GetMaterial();
			if (Material && 
				Material->GetLightingModel() == MLM_Unlit && 
				(Material->GetBlendMode() == BLEND_Translucent || Material->IsDistorted())
				)
			{
				SCOPE_CYCLE_COUNTER(STAT_SortingTime);

				ParticleOrder.Empty(ParticleCount);

				// Take UseLocalSpace into account!
				UBOOL bLocalSpace = Source.bUseLocalSpace;

				for (INT ParticleIndex = 0; ParticleIndex < ParticleCount; ParticleIndex++)
				{
					DECLARE_PARTICLE(Particle, Source.ParticleData.GetData() + Source.ParticleStride * Source.ParticleIndices(ParticleIndex));
					FLOAT InZ;
					if (bLocalSpace)
					{
						InZ = View->ViewProjectionMatrix.TransformFVector(LocalToWorld.TransformFVector(Particle.Location)).Z;
					}
					else
					{
						InZ = View->ViewProjectionMatrix.TransformFVector(Particle.Location).Z;
					}
					new(ParticleOrder)FParticleOrder(ParticleIndex, InZ);
				}
				Sort<USE_COMPARE_CONSTREF(FParticleOrder,UnParticleComponents)>(&(ParticleOrder(0)),ParticleOrder.Num());
				bSorted	= TRUE;
			}
		}

		FMeshElement Mesh;

		Mesh.UseDynamicData			= TRUE;
		Mesh.IndexBuffer			= NULL;
		Mesh.VertexFactory			= VertexFactory;
		Mesh.DynamicVertexData		= this;
		if (bUsesDynamicParameter == FALSE)
		{
			Mesh.DynamicVertexStride	= sizeof(FParticleSpriteSubUVVertex);
		}
		else
		{
			Mesh.DynamicVertexStride	= sizeof(FParticleSpriteSubUVVertexDynamicParameter);
		}
		Mesh.DynamicIndexData		= NULL;
		Mesh.DynamicIndexStride		= 0;
		if (bSorted == TRUE)
		{
			Mesh.DynamicIndexData	= (void*)(&(ParticleOrder));
		}
		Mesh.LCI = NULL;
		if (Source.bUseLocalSpace == TRUE)
		{
			Mesh.LocalToWorld = Proxy->GetLocalToWorld();
			Mesh.WorldToLocal = Proxy->GetWorldToLocal();
		}
		else
		{
			Mesh.LocalToWorld = FMatrix::Identity;
			Mesh.WorldToLocal = FMatrix::Identity;
		}
		Mesh.FirstIndex				= 0;
		Mesh.MinVertexIndex			= 0;
		Mesh.MaxVertexIndex			= (ParticleCount * 4) - 1;
		Mesh.ParticleType			= PET_SubUV;
		Mesh.ReverseCulling			= Proxy->GetLocalToWorldDeterminant() < 0.0f ? TRUE : FALSE;
		Mesh.CastShadow				= Proxy->GetCastShadow();
		Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;

		Mesh.MaterialRenderProxy		= MaterialResource;
		Mesh.NumPrimitives			= ParticleCount;
		Mesh.Type					= PT_TriangleList;

		DrawRichMesh(
			PDI, 
			Mesh, 
			FLinearColor(1.0f, 0.0f, 0.0f),	//WireframeColor,
			FLinearColor(1.0f, 1.0f, 0.0f),	//LevelColor,
			FLinearColor(1.0f, 1.0f, 1.0f),	//PropertyColor,		
			Proxy->GetPrimitiveSceneInfo(),
			Proxy->GetSelected()
			);
	}
	else
	if (Source.EmitterRenderMode == ERM_Point)
	{
		RenderDebug(PDI, View, DPGIndex, FALSE);
	}
	else
	if (Source.EmitterRenderMode == ERM_Cross)
	{
		RenderDebug(PDI, View, DPGIndex, TRUE);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	FDynamicMeshEmitterData
///////////////////////////////////////////////////////////////////////////////

FDynamicMeshEmitterData::FDynamicMeshEmitterData()
	: LastFramePreRendered(-1)
	, StaticMesh( NULL )
	, LODs()
	, InstancedMaterialInterface( NULL )
	, InstanceBuffer( NULL )
	, InstancedVertexFactory( NULL )
	, bUseNxFluid( FALSE )
	, bInstBufIsVB(FALSE)
	, InstBuf(NULL)
	, NumInst(0)
	, InstBufDesc(NULL)
	, MEMatInstRes()
{
}

FDynamicMeshEmitterData::~FDynamicMeshEmitterData()
{
	if (bUseNxFluid)
	{
		if (InstBuf)
		{
			if (bInstBufIsVB)
			{
				InstBufDesc->ReleaseVB((FVertexBufferRHIRef *)InstBuf);
			}
			else
			{
				appFree(InstBuf);
			}
		}
		if (InstBufDesc)
		{
			InstBufDesc->DecRefCount();
		}
	}

	if(InstancedVertexFactory)
	{
		InstancedVertexFactory->ReleaseResource();
		delete InstancedVertexFactory;
	}
	if(InstanceBuffer)
	{
		InstanceBuffer->ReleaseResource();
		delete InstanceBuffer;
	}
}

/** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
void FDynamicMeshEmitterData::Init( UBOOL bInSelected,
									const FParticleMeshEmitterInstance* InEmitterInstance,
									UStaticMesh* InStaticMesh,
									const UStaticMeshComponent* InStaticMeshComponent,
									UBOOL UseNxFluid )
{
	bSelected = bInSelected;

	// @todo: For replays, currently we're assuming the original emitter instance is bound to the same mesh as
	//        when the replay was generated (safe), and various mesh/material indices are intact.  If
	//        we ever support swapping meshes/material on the fly, we'll need cache the mesh
	//        reference and mesh component/material indices in the actual replay data.

	StaticMesh = InStaticMesh;
	bUseNxFluid = UseNxFluid;

	check(Source.ActiveParticleCount < 16 * 1024);	// TTP #33375
	check(Source.ParticleStride < 2 * 1024);	// TTP #3375


	// Build the proxy's LOD data.
	// @todo: We only use the highest LOD for the moment
#if PARTICLES_USE_DOUBLE_BUFFERING
	// Don't do this...
	// Add a function to update the FLODInfo
	INT LODIndex = 0;
//	LODs.Empty();
	if (LODs.Num() == 0)
	{
		new(LODs) FLODInfo(InStaticMeshComponent, InEmitterInstance, LODIndex, bSelected);
	}
	else
	{
		LODs(0).Update(InStaticMeshComponent, InEmitterInstance, LODIndex, bSelected);
	}
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	LODs.Empty();
	for (INT LODIndex = 0; LODIndex < 1; LODIndex++)
	{
		new(LODs) FLODInfo(InStaticMeshComponent, InEmitterInstance, LODIndex, bSelected);
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}


// This can be safely called from the game thread -- there are no calls to D3D
UBOOL FDynamicMeshEmitterData::FInstanceBufferDesc::GetNextBuffer(UINT NumInst, FVertexBufferRHIRef** vb, void** ptr)
{
	bool ret = FALSE;
	Lock();
	if (NextNumInstances >= NumInst)
	{
		for (int i = 0; i < F_INSTANCE_BUFFER_MAX; i++)
		{
			if (NextVertexBuffer[i])
			{
				*vb = NextVertexBuffer[i];
				*ptr = NextInstanceBuffer[i];
				NextVertexBuffer[i] = NULL;
				NextInstanceBuffer[i] = NULL;
				ret = TRUE;
				break;
			}
		}
	}
	UnLock();
	return ret;
}

// This can only be called from the render thread -- it makes calls to D3D...
#define ADDITIONAL_PARTICLES_IN_VB	50
void FDynamicMeshEmitterData::UpdateInstBufDesc(UINT NewNumInst)
{
	if (!bUseNxFluid)
		return;

	InstBufDesc->Lock();

	// Delete any VB's that are too small...
	if (InstBufDesc->NextNumInstances < NewNumInst)
	{
		InstBufDesc->ReleaseAll();
		InstBufDesc->NextNumInstances = NewNumInst + ADDITIONAL_PARTICLES_IN_VB;
	}

	// If the old size is too big, make new allocations smaller from now on
	if (InstBufDesc->NextNumInstances > (NewNumInst + (ADDITIONAL_PARTICLES_IN_VB * 3))) 
		InstBufDesc->NextNumInstances = NewNumInst + ADDITIONAL_PARTICLES_IN_VB;

	// Allocate new VB's
	InstBufDesc->AllocateAll(InstBufDesc->NextNumInstances);

	InstBufDesc->UnLock();

	InstBufDesc->EmptyGarbage();

}

void FDynamicMeshEmitterData::FInstanceBufferDesc::AllocateAll(UINT InParticleCount)
{
	// Must be called from render thread, with the critical section already locked
	for (int i = 0; i < F_INSTANCE_BUFFER_MAX; i++)
	{
		if (NextVertexBuffer[i] == NULL)
		{
			NextVertexBuffer[i] = AllocateVB(InParticleCount);
			NextInstanceBuffer[i] = LockVB(NextVertexBuffer[i], InParticleCount);
		}
	}
}

void FDynamicMeshEmitterData::FInstanceBufferDesc::ReleaseAll()
{
	// Must be called from render thread, with the critical section already locked
	for (int i = 0; i < F_INSTANCE_BUFFER_MAX; i++)
	{
		if (NextVertexBuffer[i])
		{
			UnlockVB(NextVertexBuffer[i]);
			ReleaseVB(NextVertexBuffer[i]);
			NextVertexBuffer[i] = NULL;
			NextInstanceBuffer[i] = NULL;
		}
	}
}

void FDynamicMeshEmitterData::FInstanceBufferDesc::ReleaseDynamicRHI()
{
	// Called from render thread
	// Unlock and release all VB's in NextVertexBuffer
	Lock();
	ReleaseAll();
	UnLock();
}

void FDynamicMeshEmitterData::FInstanceBufferDesc::InitDynamicRHI()
{
	// Called from render thread
	// Allocate and lock new VB's in NextVertexBuffer
	Lock();
	AllocateAll(32);
	UnLock();
}

TArray<struct FDynamicMeshEmitterData::FInstanceBufferDesc *>	FDynamicMeshEmitterData::FInstanceBufferDesc::Garbage;
FCriticalSection	FDynamicMeshEmitterData::FInstanceBufferDesc::GarbageCritSect;

void FDynamicMeshEmitterData::FInstanceBufferDesc::IncRefCount()
{
	CritSect.Lock();
	RefCount++;
	CritSect.Unlock();
}

void FDynamicMeshEmitterData::FInstanceBufferDesc::DecRefCount()
{
	CritSect.Lock();
	RefCount--;
	if (RefCount == 0)
	{   
		GarbageCritSect.Lock();
		if ((NextNumInstances > 0) || !IsInRenderingThread())
			Garbage.Push(this);
		else
		{
			GarbageCritSect.Unlock();
			CritSect.Unlock();
			ReleaseResource();
			delete this;
			return;
		}
		GarbageCritSect.Unlock();
	}
	CritSect.Unlock();
}

struct FDynamicMeshEmitterData::FInstanceBufferDesc *FDynamicMeshEmitterData::FInstanceBufferDesc::GetNextGarbage()
{
	return Garbage.Num() ? Garbage.Pop() : NULL;
}

void FDynamicMeshEmitterData::FInstanceBufferDesc::EmptyGarbage()
{
	// Do garbage collection of the FInstanceBufferDesc structures...
    GarbageCritSect.Lock();
	while (FInstanceBufferDesc *p = GetNextGarbage())
	{
		p->ReleaseResource();
		delete p;
	}
	GarbageCritSect.Unlock();
}

/** Information used by the proxy about a single LOD of the mesh. */
FDynamicMeshEmitterData::FLODInfo::FLODInfo(const UStaticMeshComponent* InStaticMeshComponent,
	const FParticleMeshEmitterInstance* MeshEmitInst, INT LODIndex, UBOOL bSelected)
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	Update(InStaticMeshComponent, MeshEmitInst, LODIndex, bSelected);
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	check(InStaticMeshComponent);
	// Gather the materials applied to the LOD.
	const FStaticMeshRenderData& LODModel = InStaticMeshComponent->StaticMesh->LODModels(LODIndex);
	Elements.Empty(LODModel.Elements.Num());
	for (INT MaterialIndex = 0; MaterialIndex < LODModel.Elements.Num(); MaterialIndex++)
	{
		FElementInfo ElementInfo;

		ElementInfo.MaterialInterface = NULL;

		// Determine the material applied to this element of the LOD.
		UMaterialInterface* MatInst = NULL;

		// The emitter instance Materials array will be filled in with entries from the 
		// MeshMaterial module, if present.
		if (MaterialIndex < MeshEmitInst->CurrentMaterials.Num())
		{
			MatInst = MeshEmitInst->CurrentMaterials(MaterialIndex);
		}

		if (MatInst == NULL)
		{
			// Next, check the static mesh component itself
			if (MaterialIndex < InStaticMeshComponent->Materials.Num())
			{
				MatInst = InStaticMeshComponent->Materials(MaterialIndex);
			}
		}

		// Safety catch - no material at this point? Use the default engine material.
		if (MatInst == NULL)
		{
			MatInst = GEngine->DefaultMaterial;
		}

		// We better have a material by now!
		check(MatInst);

		// Set up the element info entry, and add it to the component
		// tracker to prevent garbage collection.
		ElementInfo.MaterialInterface = MatInst;
		MeshEmitInst->Component->SMMaterialInterfaces.AddUniqueItem(MatInst);

		// Store the element info.
		Elements.AddItem(ElementInfo);
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

void FDynamicMeshEmitterData::FLODInfo::Update(const UStaticMeshComponent* InStaticMeshComponent, const FParticleMeshEmitterInstance* MeshEmitInst, INT LODIndex, UBOOL bSelected)
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	check(InStaticMeshComponent);
	// Gather the materials applied to the LOD.
	const FStaticMeshRenderData& LODModel = InStaticMeshComponent->StaticMesh->LODModels(LODIndex);
	if (Elements.Num() < LODModel.Elements.Num())
	{
		Elements.Empty(LODModel.Elements.Num());
		Elements.AddZeroed(LODModel.Elements.Num());
	}
	for (INT MaterialIndex = 0; MaterialIndex < LODModel.Elements.Num(); MaterialIndex++)
	{
		FElementInfo& ElementInfo = Elements(MaterialIndex);

		ElementInfo.MaterialInterface = NULL;
		// Determine the material applied to this element of the LOD.
		UMaterialInterface* MatInst = NULL;
		// The emitter instance Materials array will be filled in with entries from the 
		// MeshMaterial module, if present.
		if (MaterialIndex < MeshEmitInst->CurrentMaterials.Num())
		{
			MatInst = MeshEmitInst->CurrentMaterials(MaterialIndex);
		}

		if (MatInst == NULL)
		{
			// Next, check the static mesh component itself
			if (MaterialIndex < InStaticMeshComponent->Materials.Num())
			{
				MatInst = InStaticMeshComponent->Materials(MaterialIndex);
			}
		}

		// Safety catch - no material at this point? Use the default engine material.
		if (MatInst == NULL)
		{
			MatInst = GEngine->DefaultMaterial;
		}

		// We better have a material by now!
		check(MatInst);

		// Set up the element info entry, and add it to the component
		// tracker to prevent garbage collection.
		ElementInfo.MaterialInterface = MatInst;
		MeshEmitInst->Component->SMMaterialInterfaces.AddUniqueItem(MatInst);
	}
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	checkf(0, TEXT("FLODInfo::Update> Should only be called if DOUBLE_BUFFERING!"));
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

void FDynamicMeshEmitterData::Render(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_MeshRenderingTime);

	if (bValid == FALSE)
	{
		return;
	}

	if (Source.EmitterRenderMode == ERM_Normal)
	{
		if (bUseNxFluid)
		{
			if (InstanceBuffer && InstancedVertexFactory)
			{
				RenderNxFluidInstanced(Proxy, PDI, View, DPGIndex);
			}
			else
			{
				RenderNxFluidNonInstanced(Proxy, PDI, View, DPGIndex);
			}
			return;
		}

		if(InstancedVertexFactory)
		{
			RenderInstanced(Proxy, PDI, View, DPGIndex);
			return;
		}
		
		const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(0);
		FDynamicMeshEmitterData::FLODInfo* LODInfo = &(LODs(0));
		TArray<INT> ValidElementIndices;

		UBOOL bNoValidElements = TRUE;
		for (INT LODIndex = 0; LODIndex < 1; LODIndex++)
		{
			for (INT ElementIndex = 0; ElementIndex < LODModel.Elements.Num(); ElementIndex++)
			{
				FMeshEmitterMaterialInstanceResource* MIRes = &(MEMatInstRes(ElementIndex));
				check(MIRes);

				// If the material is ignored by the PDI (or not there at all...), 
				// do not add it to the list of valid elements.
				if ((MIRes->Parent && !PDI->IsMaterialIgnored(MIRes->Parent)) || (View->Family->ShowFlags & SHOW_Wireframe))
				{
					ValidElementIndices.AddItem(ElementIndex);
					bNoValidElements = FALSE;
				}
				else
				{
					ValidElementIndices.AddItem(-1);
				}
			}
		}

		if (bNoValidElements == TRUE)
		{
			// No valid materials... quick out
			return;
		}

		EParticleSubUVInterpMethod eSubUVMethod = (EParticleSubUVInterpMethod)(Source.SubUVInterpMethod);

		UBOOL bWireframe = ((View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials));

		FMatrix kMat(FMatrix::Identity);
		// Reset velocity and size.

		INT ParticleCount = Source.ActiveParticleCount;
		if ((Source.MaxDrawCount >= 0) && (ParticleCount > Source.MaxDrawCount))
		{
			ParticleCount = Source.MaxDrawCount;
		}

		FMatrix Local2World;

		FTranslationMatrix kTransMat(FVector(0.0f));
		FScaleMatrix kScaleMat(FVector(1.0f));
		FVector Location;
		FVector ScaledSize;
		FVector	DirToCamera;
		FVector	LocalSpaceFacingAxis;
		FVector	LocalSpaceUpAxis;

		FMeshElement Mesh;
		Mesh.DynamicVertexData = NULL;
		Mesh.LCI = NULL;
		Mesh.UseDynamicData = FALSE;
		Mesh.ReverseCulling = Proxy->GetLocalToWorldDeterminant() < 0.0f ? TRUE : FALSE;
		Mesh.CastShadow = Proxy->GetCastShadow();
		Mesh.DepthPriorityGroup = (ESceneDepthPriorityGroup)DPGIndex;

		for (INT i = ParticleCount - 1; i >= 0; i--)
		{
			const INT	CurrentIndex	= Source.ParticleIndices(i);
			const BYTE* ParticleBase	= Source.ParticleData.GetData() + CurrentIndex * Source.ParticleStride;
			FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);

			if (Particle.RelativeTime < 1.0f)
			{
				kTransMat.M[3][0] = Particle.Location.X;
				kTransMat.M[3][1] = Particle.Location.Y;
				kTransMat.M[3][2] = Particle.Location.Z;
				ScaledSize = Particle.Size * Source.Scale;
				kScaleMat.M[0][0] = ScaledSize.X;
				kScaleMat.M[1][1] = ScaledSize.Y;
				kScaleMat.M[2][2] = ScaledSize.Z;

				FRotator kRotator;
				Local2World = Proxy->GetLocalToWorld();
				if (Source.ScreenAlignment == PSA_TypeSpecific)
				{
					Location = Particle.Location;
					if (Source.bUseLocalSpace)
					{
						Location = Local2World.TransformFVector(Location);
						kTransMat.SetOrigin(Location);
						Local2World.SetIdentity();
					}
					DirToCamera	= View->ViewOrigin - Location;
					DirToCamera.Normalize();
					if (DirToCamera.SizeSquared() <	0.5f)
					{
						// Assert possible if DirToCamera is not normalized
						DirToCamera	= FVector(1,0,0);
					}

					LocalSpaceFacingAxis = FVector(1,0,0); // facing axis is taken to be the local x axis.	
					LocalSpaceUpAxis = FVector(0,0,1); // up axis is taken to be the local z axis

					if (Source.MeshAlignment == PSMA_MeshFaceCameraWithLockedAxis)
					{
						// TODO: Allow an arbitrary	vector to serve	as the locked axis

						// For the locked axis behavior, only rotate to	face the camera	about the
						// locked direction, and maintain the up vector	pointing towards the locked	direction
						// Find	the	rotation that points the localupaxis towards the targetupaxis
						FQuat PointToUp	= FQuatFindBetween(LocalSpaceUpAxis, Source.LockedAxis);

						// Add in rotation about the TargetUpAxis to point the facing vector towards the camera
						FVector	DirToCameraInRotationPlane = DirToCamera - ((DirToCamera | Source.LockedAxis)*Source.LockedAxis);
						DirToCameraInRotationPlane.Normalize();
						FQuat PointToCamera	= FQuatFindBetween(PointToUp.RotateVector(LocalSpaceFacingAxis), DirToCameraInRotationPlane);

						// Set kRotator	to the composed	rotation
						FQuat MeshRotation = PointToCamera*PointToUp;
						kRotator = FRotator(MeshRotation);
					}
					else
					if (Source.MeshAlignment == PSMA_MeshFaceCameraWithSpin)
					{
						// Implement a tangent-rotation	version	of point-to-camera.	 The facing	direction points to	the	camera,
						// with	no roll, and has addtional sprite-particle rotation	about the tangential axis
						// (c.f. the roll rotation is about	the	radial axis)

						// Find	the	rotation that points the facing	axis towards the camera
						FRotator PointToRotation = FRotator(FQuatFindBetween(LocalSpaceFacingAxis, DirToCamera));

						// When	constructing the rotation, we need to eliminate	roll around	the	dirtocamera	axis,
						// otherwise the particle appears to rotate	around the dircamera axis when it or the camera	moves
						PointToRotation.Roll = 0;

						// Add in the tangential rotation we do	want.
						FVector	vPositivePitch = FVector(0,0,1); //	this is	set	by the rotator's yaw/pitch/roll	reference frame
						FVector	vTangentAxis = vPositivePitch^DirToCamera;
						vTangentAxis.Normalize();
						if (vTangentAxis.SizeSquared() < 0.5f)
						{
							vTangentAxis = FVector(1,0,0); // assert is	possible if	FQuat axis/angle constructor is	passed zero-vector
						}

						FQuat AddedTangentialRotation =	FQuat(vTangentAxis,	Particle.Rotation);

						// Set kRotator	to the composed	rotation
						FQuat MeshRotation = AddedTangentialRotation*PointToRotation.Quaternion();
						kRotator = FRotator(MeshRotation);
					}
					else
					//if (MeshAlignment == PSMA_MeshFaceCameraWithRoll)
					{
						// Implement a roll-rotation version of	point-to-camera.  The facing direction points to the camera,
						// with	no roll, and then rotates about	the	direction_to_camera	by the spriteparticle rotation.

						// Find	the	rotation that points the facing	axis towards the camera
						FRotator PointToRotation = FRotator(FQuatFindBetween(LocalSpaceFacingAxis, DirToCamera));

						// When	constructing the rotation, we need to eliminate	roll around	the	dirtocamera	axis,
						// otherwise the particle appears to rotate	around the dircamera axis when it or the camera	moves
						PointToRotation.Roll = 0;

						// Add in the roll we do want.
						FQuat AddedRollRotation	= FQuat(DirToCamera, Particle.Rotation);

						// Set kRotator	to the composed	rotation
						FQuat MeshRotation = AddedRollRotation*PointToRotation.Quaternion();
						kRotator = FRotator(MeshRotation);
					}
				}
				else
				if (Source.bMeshRotationActive)
				{
					FMeshRotationPayloadData* PayloadData = (FMeshRotationPayloadData*)((BYTE*)&Particle + Source.MeshRotationOffset);
					kRotator = FRotator::MakeFromEuler(PayloadData->Rotation);
				}
				else
				{
					FLOAT fRot = Particle.Rotation * 180.0f / PI;
					FVector kRotVec = FVector(fRot, fRot, fRot);
					kRotator = FRotator::MakeFromEuler(kRotVec);
				}

				FRotationMatrix kRotMat(kRotator);
				kMat = kScaleMat * kRotMat * kTransMat;

				FVector OrbitOffset(0.0f, 0.0f, 0.0f);
				if (Source.OrbitModuleOffset != 0)
				{
					INT CurrentOffset = Source.OrbitModuleOffset;
					PARTICLE_ELEMENT(FOrbitChainModuleInstancePayload, OrbitPayload);
					OrbitOffset = OrbitPayload.Offset;
					if (Source.bUseLocalSpace == FALSE)
					{
						OrbitOffset = SceneProxy->GetLocalToWorld().TransformNormal(OrbitOffset);
					}

					FTranslationMatrix OrbitMatrix(OrbitOffset);
					kMat *= OrbitMatrix;
				}

				if (Source.bUseLocalSpace)
				{
					kMat *= Local2World;
				}

				UBOOL bBadParent = FALSE;

				//@todo. Handle LODs...
				//for (INT LODIndex = 0; LODIndex < MeshData->LODs.Num(); LODIndex++)
				for (INT LODIndex = 0; LODIndex < 1; LODIndex++)
				{
					for (INT ValidIndex = 0; ValidIndex < ValidElementIndices.Num(); ValidIndex++)
					{
						INT ElementIndex = ValidElementIndices(ValidIndex);
						if (ElementIndex == -1)
						{
							continue;
						}

						FMeshEmitterMaterialInstanceResource* MIRes = &(MEMatInstRes(ElementIndex));
						const FStaticMeshElement& Element = LODModel.Elements(ElementIndex);
						if ((Element.NumTriangles == 0) || (MIRes == NULL) || (MIRes->Parent == NULL))
						{
							//@todo. This should never happen... but it does.
							continue;
						}

						MIRes->Param_MeshEmitterVertexColor = Particle.Color;
						if (Source.SubUVInterpMethod != PSUVIM_None)
						{
							FFullSubUVPayload* SubUVPayload = (FFullSubUVPayload*)(((BYTE*)&Particle) + Source.SubUVDataOffset);

							MIRes->Param_TextureOffsetParameter = 
								FLinearColor(SubUVPayload->ImageHVInterp_UVOffset.X, SubUVPayload->ImageHVInterp_UVOffset.Y, 0.0f, 0.0f);

							if (Source.bScaleUV)
							{
								MIRes->Param_TextureScaleParameter = 
									FLinearColor((1.0f / (FLOAT)Source.SubImages_Horizontal),
									(1.0f / (FLOAT)Source.SubImages_Vertical),
									0.0f, 0.0f);
							}
							else
							{
								MIRes->Param_TextureScaleParameter = 
									FLinearColor(1.0f, 1.0f, 0.0f, 0.0f);
							}
						}

						// Draw the static mesh elements.
						Mesh.VertexFactory = &LODModel.VertexFactory;
						Mesh.LocalToWorld = kMat;
						Mesh.WorldToLocal = kMat.InverseSafe();
						//@todo motionblur
						Mesh.FirstIndex = Element.FirstIndex;
						Mesh.MinVertexIndex = Element.MinVertexIndex;
						Mesh.MaxVertexIndex = Element.MaxVertexIndex;
						if( bWireframe && LODModel.WireframeIndexBuffer.IsInitialized() )
						{
							Mesh.bWireframe = TRUE;
							Mesh.IndexBuffer = &LODModel.WireframeIndexBuffer;
							Mesh.MaterialRenderProxy = Proxy->GetDeselectedWireframeMatInst();
							Mesh.Type = PT_LineList;
							Mesh.NumPrimitives = LODModel.WireframeIndexBuffer.Indices.Num() / 2;
						}
						else
						{
							Mesh.bWireframe = FALSE;
							Mesh.IndexBuffer = &LODModel.IndexBuffer;
							Mesh.MaterialRenderProxy = MIRes;
							Mesh.Type = PT_TriangleList;
							Mesh.NumPrimitives = Element.NumTriangles;
						}
						PDI->DrawMesh(Mesh);
					}
				}
			}
			else
			{
				// Remove it from the scene???
			}
		}
	}
	else
	if (Source.EmitterRenderMode == ERM_Point)
	{
		RenderDebug(PDI, View, DPGIndex, FALSE);
	}
	else
	if (Source.EmitterRenderMode == ERM_Cross)
	{
		RenderDebug(PDI, View, DPGIndex, TRUE);
	}
}

/**
 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
 *  Only called for primitives that are visible and have bDynamicRelevance
 *
 *	@param	Proxy			The 'owner' particle system scene proxy
 *	@param	ViewFamily		The ViewFamily to pre-render for
 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
 *	@param	FrameNumber		The frame number of this pre-render
 */
void FDynamicMeshEmitterData::PreRenderView(FParticleSystemSceneProxy* Proxy, const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber)
{
	if (bValid == FALSE)
	{
		return;
	}

	// Mesh emitters don't actually care about the view...
	// They just need to setup their material instances.
	if (LastFramePreRendered != FrameNumber)
	{
		if (Source.EmitterRenderMode == ERM_Normal)
		{
			// The material setup only needs to be done once per-frame as it is view-independent.
			const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(0);
			FDynamicMeshEmitterData::FLODInfo* LODInfo = &(LODs(0));
			//@todo. Handle LODs...
			for (INT LODIndex = 0; LODIndex < 1; LODIndex++)
			{
				for (INT ElementIndex = 0; ElementIndex < LODModel.Elements.Num(); ElementIndex++)
				{
					FMeshEmitterMaterialInstanceResource* NewMIRes;
					if (ElementIndex < MEMatInstRes.Num())
					{
						NewMIRes = &(MEMatInstRes(ElementIndex));
					}
					else
					{
						NewMIRes = new(MEMatInstRes) FMeshEmitterMaterialInstanceResource();
					}
					check(NewMIRes);

					// Set the parent of our mesh material instance constant...
					NewMIRes->Parent = NULL;

					// If it has been stored off when we generated the dynamic data, use that
					if (ElementIndex < LODInfo->Elements.Num())
					{
						const FDynamicMeshEmitterData::FLODInfo::FElementInfo& Info = LODInfo->Elements(ElementIndex);
						if (LODInfo->Elements(ElementIndex).MaterialInterface)
						{
							NewMIRes->Parent = LODInfo->Elements(ElementIndex).MaterialInterface->GetRenderProxy(bSelected);
						}
					}

					// Otherwise, try grabbing it from the mesh itself.
					if (NewMIRes->Parent == NULL)
					{
						UMaterialInterface* MatIF = LODModel.Elements(ElementIndex).Material;
						NewMIRes->Parent = MatIF ? MatIF->GetRenderProxy(bSelected) : NULL;
					}
				}
			}
		}
		LastFramePreRendered = FrameNumber;
	}
}

void FDynamicMeshEmitterData::RenderDebug(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, UBOOL bCrosses)
{
	if (!bUseNxFluid)
	{
		FDynamicSpriteEmitterDataBase::RenderDebug(PDI, View, DPGIndex, bCrosses);
		return;
	}

	if (!InstBuf)
		return;

	if (bInstBufIsVB)
	{
		// this shouldn't happen
		InstBufDesc->UnlockVB((FVertexBufferRHIRef *)InstBuf);
		InstBufDesc->ReleaseVB((FVertexBufferRHIRef *)InstBuf);
		InstBuf = NULL;
		return;
	}

	FParticleInstancedMeshInstance* SrcInstance = (FParticleInstancedMeshInstance*)InstBuf;

	check(SceneProxy);

	const FMatrix& LocalToWorld = Source.bUseLocalSpace ? SceneProxy->GetLocalToWorld() : FMatrix::Identity;

	FMatrix CameraToWorld = View->ViewMatrix.Inverse();
	FVector CamX = CameraToWorld.TransformNormal(FVector(1,0,0));
	FVector CamY = CameraToWorld.TransformNormal(FVector(0,1,0));

	FLinearColor EmitterEditorColor = FLinearColor(1.0f,1.0f,0);

	for (INT i = 0; i < Source.ActiveParticleCount; i++)
	{
		FVector DrawLocation = LocalToWorld.TransformFVector(SrcInstance->Location);
		SrcInstance++;
		if (bCrosses)
		{
			FVector Size(1.0f); /* Particle.Size * Scale; */
			PDI->DrawLine(DrawLocation - (0.5f * Size.X * CamX), DrawLocation + (0.5f * Size.X * CamX), EmitterEditorColor, DPGIndex);
			PDI->DrawLine(DrawLocation - (0.5f * Size.Y * CamY), DrawLocation + (0.5f * Size.Y * CamY), EmitterEditorColor, DPGIndex);
		}
		else
		{
			PDI->DrawPoint(DrawLocation, EmitterEditorColor, 2, DPGIndex);
		}
	}
	appFree(InstBuf);
	InstBuf = NULL;
}

/** Render using hardware instancing. */
void FDynamicMeshEmitterData::RenderInstanced(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	check(InstancedMaterialInterface);
	check(InstanceBuffer);
	check(InstancedVertexFactory);
	
	INT ParticleCount = Source.ActiveParticleCount;
	
	if(ParticleCount == 0)
	{
		return; // Nothing to do...
	}
	
	const FStaticMeshRenderData&       LODModel = StaticMesh->LODModels(0);
	FDynamicMeshEmitterData::FLODInfo* LODInfo  = &(LODs(0));
	
	InitInstancedResources(ParticleCount);
	
	FParticleInstancedMeshInstance* DestInstance =
		(FParticleInstancedMeshInstance*)InstanceBuffer->CreateAndLockInstances(ParticleCount);

	if (DestInstance == NULL)
	{
		return;
	}

	for(INT i=0; i<ParticleCount; i++)
	{
		const INT	CurrentIndex  = Source.ParticleIndices(i);
		const BYTE* ParticleBase  = Source.ParticleData.GetData() + CurrentIndex * Source.ParticleStride;
		FBaseParticle& Particle   = *((FBaseParticle*) ParticleBase);
		
		FScaleMatrix kScaleMat(Particle.Size * Source.Scale);
		FRotator kRotator(0,0,0);
		
		if(Source.bMeshRotationActive)
		{
			FMeshRotationPayloadData* PayloadData = (FMeshRotationPayloadData*)((BYTE*)&Particle + Source.MeshRotationOffset);
			kRotator = FRotator::MakeFromEuler(PayloadData->Rotation);
		}
		
		FRotationMatrix kRotMat(kRotator);
		FMatrix kMat = kScaleMat * kRotMat;

		FParticleInstancedMeshInstance &Instance = DestInstance[i];
		Instance.Location = Particle.Location;
		kMat.GetAxes(Instance.XAxis, Instance.YAxis, Instance.ZAxis);
	}
	
	InstanceBuffer->UnlockInstances();

	FMeshElement MeshElement;
	MeshElement.IndexBuffer   = &LODModel.IndexBuffer;
	MeshElement.VertexFactory = InstancedVertexFactory;
	
	MeshElement.MaterialRenderProxy = InstancedMaterialInterface->GetMaterial()->GetRenderProxy(FALSE);
	
	MeshElement.LocalToWorld = FMatrix::Identity;
	MeshElement.WorldToLocal = FMatrix::Identity;
	
	MeshElement.FirstIndex     = 0;
	MeshElement.NumPrimitives  = LODModel.IndexBuffer.Indices.Num() / 3;
	MeshElement.MinVertexIndex = 0;
	MeshElement.MaxVertexIndex = LODModel.NumVertices - 1;
	
	MeshElement.Type               = PT_TriangleList;
	MeshElement.DepthPriorityGroup = (ESceneDepthPriorityGroup)DPGIndex;
	
	PDI->DrawMesh(MeshElement);
}

/** Render NxFluid without hardware instancing, but using the fast-path through the particle system */
void FDynamicMeshEmitterData::RenderNxFluidNonInstanced(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	INT ParticleCount = Source.ActiveParticleCount;
	
	if(ParticleCount == 0)
	{
		return; // Nothing to do...
	}
	
	const FStaticMeshRenderData&       LODModel = StaticMesh->LODModels(0);
	const FStaticMeshElement&          Element  = LODModel.Elements(0);

	const FDynamicMeshEmitterData::FLODInfo* LODInfo  = &(LODs(0));
	const FDynamicMeshEmitterData::FLODInfo::FElementInfo& Info = LODInfo->Elements(0);

	FMeshEmitterMaterialInstanceResource	AutoMIRes;
	FMeshEmitterMaterialInstanceResource*	MIRes = & AutoMIRes;

	MIRes->Param_MeshEmitterVertexColor = FLinearColor(1.0f, 1.0f, 1.0f);
	MIRes->Parent = Info.MaterialInterface->GetRenderProxy(bSelected);
	if (MaterialResource)
	{
		MIRes->Parent = (FMaterialRenderProxy*)(MaterialResource);
	}
	else
	if (MIRes->Parent == NULL)
	{
		if (Element.Material)
		{
			MIRes->Parent = Element.Material->GetRenderProxy(bSelected);
		}
		else
		{
			MIRes->Parent = GEngine->DefaultMaterial->GetRenderProxy(bSelected);
		}
	}

	UBOOL bWireframe = ((View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials));

	if (InstBuf)
	{
		if (bInstBufIsVB)
		{
			// This should normally not happen, unless we switch from
			// instanced to non-instanced NxFluid on the fly.  In that
			// case, a few VB's could still be filled in by the game
			// thread before running out of VB's.
			InstBufDesc->UnlockVB((FVertexBufferRHIRef *)InstBuf);
			InstBufDesc->ReleaseVB((FVertexBufferRHIRef *)InstBuf);
			InstBuf = NULL;
			return;
		}

		FParticleInstancedMeshInstance* SrcInstance =
				(FParticleInstancedMeshInstance*)InstBuf;
			
		FMeshElement MeshElement;

		MeshElement.VertexFactory = &LODModel.VertexFactory;
		MeshElement.DynamicVertexData = NULL;
		MeshElement.LCI = NULL;

		MeshElement.FirstIndex     = Element.FirstIndex;
		MeshElement.MinVertexIndex = Element.MinVertexIndex;
		MeshElement.MaxVertexIndex = Element.MaxVertexIndex;
		MeshElement.UseDynamicData = FALSE;
		MeshElement.ReverseCulling = Proxy->GetLocalToWorldDeterminant() < 0.0f ? TRUE : FALSE;
		MeshElement.CastShadow     = Proxy->GetCastShadow();
		MeshElement.DepthPriorityGroup = (ESceneDepthPriorityGroup)DPGIndex;
		MeshElement.bWireframe = bWireframe;

		if (bWireframe && LODModel.WireframeIndexBuffer.IsInitialized())
		{
			MeshElement.IndexBuffer    = &LODModel.WireframeIndexBuffer;
			MeshElement.MaterialRenderProxy = Proxy->GetDeselectedWireframeMatInst();
			MeshElement.Type           = PT_LineList;
			MeshElement.NumPrimitives  = LODModel.WireframeIndexBuffer.Indices.Num() / 2;
		}
		else
		{
			MeshElement.IndexBuffer    = &LODModel.IndexBuffer;
			MeshElement.MaterialRenderProxy = MIRes;
			MeshElement.Type           = PT_TriangleList;
			MeshElement.NumPrimitives  = LODModel.IndexBuffer.Indices.Num() / 3;
		}

		for (INT i = ParticleCount - 1; i >= 0; i--)
		{
			MeshElement.LocalToWorld = FMatrix(SrcInstance->XAxis, SrcInstance->YAxis, SrcInstance->ZAxis, SrcInstance->Location);
			MeshElement.WorldToLocal = MeshElement.LocalToWorld.Inverse();
			SrcInstance++;

			PDI->DrawMesh(MeshElement);
		}
	}
}

/** Render using NxFluid hardware instancing, with pre-filled VB's */
void FDynamicMeshEmitterData::RenderNxFluidInstanced(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	check(InstancedMaterialInterface);
	check(InstanceBuffer);
	check(InstancedVertexFactory);

	INT ParticleCount = Source.ActiveParticleCount;
	
	if(ParticleCount == 0)
	{
		return; // Nothing to do...
	}
	
	const FStaticMeshRenderData&       LODModel = StaticMesh->LODModels(0);
	FDynamicMeshEmitterData::FLODInfo* LODInfo  = &(LODs(0));
	
	InitInstancedResources(ParticleCount);
	
	UpdateInstBufDesc(ParticleCount);		// Allocate more vertex buffers for game thread...

	if (InstBuf)
	{
		if (bInstBufIsVB)
		{
			InstBufDesc->UnlockVB((FVertexBufferRHIRef *)InstBuf);
			InstanceBuffer->VertexBufferRHI = *(FVertexBufferRHIRef *)InstBuf;
		}
		else
		{
			const UINT BufferSize = sizeof(FParticleInstancedMeshInstance) * ParticleCount;

			InstanceBuffer->VertexBufferRHI = RHICreateVertexBuffer(BufferSize, NULL, RUF_Dynamic);

			FParticleInstancedMeshInstance* DstInstance =
					(FParticleInstancedMeshInstance*)RHILockVertexBuffer(InstanceBuffer->VertexBufferRHI, 0, BufferSize, FALSE);

			FParticleInstancedMeshInstance* SrcInstance =
					(FParticleInstancedMeshInstance*)InstBuf;
			
			for (INT i = ParticleCount - 1; i >= 0; i--)
			{
				*DstInstance++ = *SrcInstance++;
			}

			RHIUnlockVertexBuffer(InstanceBuffer->VertexBufferRHI);

			appFree(InstBuf);
 			InstBuf = ::new FVertexBufferRHIRef(InstanceBuffer->VertexBufferRHI);
 			bInstBufIsVB = TRUE;
		}

		FMeshElement MeshElement;
		MeshElement.IndexBuffer   = &LODModel.IndexBuffer;
		MeshElement.VertexFactory = InstancedVertexFactory;
		
		MeshElement.MaterialRenderProxy = InstancedMaterialInterface->GetMaterial()->GetRenderProxy(FALSE);
		
		MeshElement.LocalToWorld = FMatrix::Identity;
		MeshElement.WorldToLocal = FMatrix::Identity;
		
		MeshElement.FirstIndex     = 0;
		MeshElement.NumPrimitives  = LODModel.IndexBuffer.Indices.Num() / 3;
		MeshElement.MinVertexIndex = 0;
		MeshElement.MaxVertexIndex = LODModel.NumVertices - 1;
		
		MeshElement.Type               = PT_TriangleList;
		MeshElement.DepthPriorityGroup = (ESceneDepthPriorityGroup)DPGIndex;
		
		PDI->DrawMesh(MeshElement);
	}

}

/** Initialized the vertex factory for a specific number of instances. */
void FDynamicMeshEmitterData::InitInstancedResources(UINT NumInstances)
{
	// Initialize the instance buffer.
	InstanceBuffer->InitResource();
	
	const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(0);
	FParticleInstancedMeshVertexFactory::DataType VertexFactoryData;
	
	VertexFactoryData.PositionComponent = FVertexStreamComponent(
		&LODModel.PositionVertexBuffer,
		STRUCT_OFFSET(FPositionVertex,Position),
		LODModel.PositionVertexBuffer.GetStride(),
		VET_Float3
		);
	VertexFactoryData.TangentBasisComponents[0] = FVertexStreamComponent(
		&LODModel.VertexBuffer,
		STRUCT_OFFSET(FStaticMeshFullVertex,TangentX),
		LODModel.VertexBuffer.GetStride(),
		VET_PackedNormal
		);
	VertexFactoryData.TangentBasisComponents[1] = FVertexStreamComponent(
		&LODModel.VertexBuffer,
		STRUCT_OFFSET(FStaticMeshFullVertex,TangentZ),
		LODModel.VertexBuffer.GetStride(),
		VET_PackedNormal
		);
	
	if( !LODModel.VertexBuffer.GetUseFullPrecisionUVs() )
	{
		VertexFactoryData.TextureCoordinateComponent = FVertexStreamComponent(
			&LODModel.VertexBuffer,
			STRUCT_OFFSET(TStaticMeshFullVertexFloat16UVs<MAX_TEXCOORDS>,UVs[0]),
			LODModel.VertexBuffer.GetStride(),
			VET_Half2
			);
	}
	else
	{
		VertexFactoryData.TextureCoordinateComponent = FVertexStreamComponent(
			&LODModel.VertexBuffer,
			STRUCT_OFFSET(TStaticMeshFullVertexFloat32UVs<MAX_TEXCOORDS>,UVs[0]),
			LODModel.VertexBuffer.GetStride(),
			VET_Float2
			);
	}
	
	VertexFactoryData.InstanceOffsetComponent = FVertexStreamComponent(
		InstanceBuffer,
		STRUCT_OFFSET(FParticleInstancedMeshInstance,Location),
		sizeof(FParticleInstancedMeshInstance),
		VET_Float3,
		TRUE
		);
	VertexFactoryData.InstanceAxisComponents[0] = FVertexStreamComponent(
		InstanceBuffer,
		STRUCT_OFFSET(FParticleInstancedMeshInstance,XAxis),
		sizeof(FParticleInstancedMeshInstance),
		VET_Float3,
		TRUE
		);
	VertexFactoryData.InstanceAxisComponents[1] = FVertexStreamComponent(
		InstanceBuffer,
		STRUCT_OFFSET(FParticleInstancedMeshInstance,YAxis),
		sizeof(FParticleInstancedMeshInstance),
		VET_Float3,
		TRUE
		);
	VertexFactoryData.InstanceAxisComponents[2] = FVertexStreamComponent(
		InstanceBuffer,
		STRUCT_OFFSET(FParticleInstancedMeshInstance,ZAxis),
		sizeof(FParticleInstancedMeshInstance),
		VET_Float3,
		TRUE
		);
	
	VertexFactoryData.NumVerticesPerInstance = LODModel.NumVertices;
	VertexFactoryData.NumInstances = NumInstances;

	InstancedVertexFactory->SetData(VertexFactoryData);
	InstancedVertexFactory->InitResource();
}

/**
 * Allocate and lock a vertex buffer for storing instance transforms.
 */
void* FDynamicMeshEmitterData::FParticleInstancedMeshInstanceBuffer::CreateAndLockInstances(const UINT NumInstances)
{
	check(NumInstances > 0);

	const UINT BufferSize = sizeof(FParticleInstancedMeshInstance) * NumInstances;
	
	// Create the vertex buffer.
	VertexBufferRHI = RHICreateVertexBuffer(BufferSize, NULL, RUF_Dynamic);
	
	// Lock the vertex buffer.
	void* VertexBufferData = RHILockVertexBuffer(VertexBufferRHI, 0, BufferSize, FALSE);

	return VertexBufferData;
}

/**
 * Unlock a vertex buffer for storing instance transforms.
 */
void FDynamicMeshEmitterData::FParticleInstancedMeshInstanceBuffer::UnlockInstances()
{
	// Unlock the vertex buffer.
	RHIUnlockVertexBuffer(VertexBufferRHI);
}

///////////////////////////////////////////////////////////////////////////////
//	FDynamicBeam2EmitterData
///////////////////////////////////////////////////////////////////////////////


/** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
void FDynamicBeam2EmitterData::Init( UBOOL bInSelected )
{
	bSelected = bInSelected;

	check(Source.ActiveParticleCount < (MaxBeams));	// TTP #33330 - Max of 2048 beams from a single emitter
	check(Source.ParticleStride < 
		((MaxInterpolationPoints + 2) * (sizeof(FVector) + sizeof(FLOAT))) + 
		(MaxNoiseFrequency * (sizeof(FVector) + sizeof(FVector) + sizeof(FLOAT) + sizeof(FLOAT)))
		);	// TTP #33330 - Max of 10k per beam (includes interpolation points, noise, etc.)

	bUsesDynamicParameter = FALSE;
	if( Source.MaterialInterface->GetMaterialResource() != NULL )
	{
		bUsesDynamicParameter =
			Source.MaterialInterface->GetMaterialResource()->GetUsesDynamicParameter();
	}

	MaterialResource =
		Source.MaterialInterface->GetRenderProxy( bSelected );

	// We won't need this on the render thread
	Source.MaterialInterface = NULL;
}


void FDynamicBeam2EmitterData::Render(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_BeamRenderingTime);
	INC_DWORD_STAT(STAT_BeamParticlesRenderCalls);

	if (bValid == FALSE)
	{
		return;
	}

	UBOOL bMaterialIgnored = PDI->IsMaterialIgnored(MaterialResource);
	if (bMaterialIgnored && (!(View->Family->ShowFlags & SHOW_Wireframe)))
	{
		return;
	}

	if (GIsEditor || !(GEngine->GameViewport && (GEngine->GameViewport->GetCurrentSplitscreenType() == eSST_NONE)))
	{
		VertexFactory->SetScreenAlignment(Source.ScreenAlignment);
		// Beams/trails do not support LockAxis
		VertexFactory->SetLockAxesFlag(EPAL_NONE);

		// Allocate and generate the data...
		// Determine the required particle count
		if ((VertexData == NULL) || (VertexCount < Source.VertexCount))
		{
			VertexData = (FParticleSpriteVertex*)appRealloc(VertexData, Source.VertexCount * sizeof(FParticleSpriteVertex));
			VertexCount = Source.VertexCount;
			check(VertexData);
		}

		TrianglesToRender = FillIndexData(Proxy, PDI, View, DPGIndex);
		if (Source.bLowFreqNoise_Enabled)
		{
			FillData_Noise(Proxy, PDI, View, DPGIndex);
		}
		else
		{
			FillVertexData_NoNoise(Proxy, PDI, View, DPGIndex);
		}
	}

	if (TrianglesToRender > 0)
	{
		if (!bMaterialIgnored || View->Family->ShowFlags & SHOW_Wireframe)
		{
			FMeshElement Mesh;

			Mesh.IndexBuffer			= NULL;
			Mesh.VertexFactory			= VertexFactory;
			Mesh.DynamicVertexData		= VertexData;
			Mesh.DynamicVertexStride	= sizeof(FParticleSpriteVertex);
			Mesh.DynamicIndexData		= IndexData;
			Mesh.DynamicIndexStride		= Source.IndexStride;
			Mesh.LCI					= NULL;
			if (Source.bUseLocalSpace == TRUE)
			{
				Mesh.LocalToWorld = Proxy->GetLocalToWorld();
				Mesh.WorldToLocal = Proxy->GetWorldToLocal();
			}
			else
			{
				Mesh.LocalToWorld = FMatrix::Identity;
				Mesh.WorldToLocal = FMatrix::Identity;
			}
			Mesh.FirstIndex				= 0;
			if ((TrianglesToRender % 2) != 0)
			{
				TrianglesToRender--;
			}
			Mesh.NumPrimitives			= TrianglesToRender;
			Mesh.MinVertexIndex			= 0;
			Mesh.MaxVertexIndex			= Source.VertexCount - 1;
			Mesh.UseDynamicData			= TRUE;
			Mesh.ReverseCulling			= Proxy->GetLocalToWorldDeterminant() < 0.0f ? TRUE : FALSE;
			Mesh.CastShadow				= Proxy->GetCastShadow();
			Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;

			if ((View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials))
			{
				Mesh.MaterialRenderProxy		= Proxy->GetDeselectedWireframeMatInst();
			}
			else
			{
				Mesh.MaterialRenderProxy		= MaterialResource;
			}
			Mesh.Type = PT_TriangleStrip;

			DrawRichMesh(
				PDI,
				Mesh,
				FLinearColor(1.0f, 0.0f, 0.0f),
				FLinearColor(1.0f, 1.0f, 0.0f),
				FLinearColor(1.0f, 1.0f, 1.0f),
				Proxy->GetPrimitiveSceneInfo(),
				Proxy->GetSelected()
				);

			INC_DWORD_STAT_BY(STAT_BeamParticlesTrianglesRendered, Mesh.NumPrimitives);
		}

		if (Source.bRenderDirectLine == TRUE)
		{
			RenderDirectLine(Proxy, PDI, View, DPGIndex);
		}

		if ((Source.bRenderLines == TRUE) ||
			(Source.bRenderTessellation == TRUE))
		{
			RenderLines(Proxy, PDI, View, DPGIndex);
		}
	}
}

/**
 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
 *  Only called for primitives that are visible and have bDynamicRelevance
 *
 *	@param	Proxy			The 'owner' particle system scene proxy
 *	@param	ViewFamily		The ViewFamily to pre-render for
 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
 *	@param	FrameNumber		The frame number of this pre-render
 */
void FDynamicBeam2EmitterData::PreRenderView(FParticleSystemSceneProxy* Proxy, const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber)
{
	if (GIsEditor || (GEngine->GameViewport == NULL))
	{
		return;
	}

	if (bValid == FALSE)
	{
		return;
	}

	if (GEngine->GameViewport->GetCurrentSplitscreenType() == eSST_NONE)
	{
		// Find the first view w/ the visiblity flag set...
		INT ValidView = -1;

		for (INT ViewIndex = 0; ViewIndex < ViewFamily->Views.Num(); ViewIndex++)
		{
			if (VisibilityMap(ViewIndex) == TRUE)
			{
				ValidView = ViewIndex;
				break;
			}
		}

		if (ValidView == -1)
		{
			return;
		}

		const FSceneView* View = ViewFamily->Views(ValidView);

		// We only need to do this once for each view...
		if (LastFramePreRendered != FrameNumber)
		{
			VertexFactory->SetScreenAlignment(Source.ScreenAlignment);
			// Beams/trails do not support LockAxis
			VertexFactory->SetLockAxesFlag(EPAL_NONE);

			// Allocate and generate the data...
			// Determine the required particle count
			if ((VertexData == NULL) || (VertexCount < Source.VertexCount))
			{
				VertexData = (FParticleSpriteVertex*)appRealloc(VertexData, Source.VertexCount * sizeof(FParticleSpriteVertex));
				VertexCount = Source.VertexCount;
				check(VertexData);
			}

			// Fill the index and vertex data
			TrianglesToRender = FillIndexData(Proxy, NULL, View, -1);
			if (Source.bLowFreqNoise_Enabled)
			{
				FillData_Noise(Proxy, NULL, View, -1);
			}
			else
			{
				FillVertexData_NoNoise(Proxy, NULL, View, -1);
			}

			// Set the frame tracker
			LastFramePreRendered = FrameNumber;
		}
	}
}

void FDynamicBeam2EmitterData::RenderDirectLine(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
{
	for (INT Beam = 0; Beam < Source.ActiveParticleCount; Beam++)
	{
		DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * Beam);

		FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
		FVector*				InterpolatedPoints	= NULL;
		FLOAT*					NoiseRate			= NULL;
		FLOAT*					NoiseDelta			= NULL;
		FVector*				TargetNoisePoints	= NULL;
		FVector*				NextNoisePoints		= NULL;
		FLOAT*					TaperValues			= NULL;

		BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + Source.BeamDataOffset);
		if (BeamPayloadData->TriangleCount == 0)
		{
			continue;
		}

		DrawWireStar(PDI, BeamPayloadData->SourcePoint, 20.0f, FColor(0,255,0),DPGIndex);
		DrawWireStar(PDI, BeamPayloadData->TargetPoint, 20.0f, FColor(255,0,0),DPGIndex);
		PDI->DrawLine(BeamPayloadData->SourcePoint, BeamPayloadData->TargetPoint, FColor(255,255,0),DPGIndex);
	}
}

void FDynamicBeam2EmitterData::RenderLines(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
{
	if (Source.bLowFreqNoise_Enabled)
	{
		INT	TrianglesToRender = 0;

		FMatrix WorldToLocal = Proxy->GetWorldToLocal();
		FMatrix LocalToWorld = Proxy->GetLocalToWorld();
		FMatrix CameraToWorld = View->ViewMatrix.Inverse();
		FVector	ViewOrigin = CameraToWorld.GetOrigin();

		Source.Sheets = (Source.Sheets > 0) ? Source.Sheets : 1;

		// Frequency is the number of noise points to generate, evenly distributed along the line.
		Source.Frequency = (Source.Frequency > 0) ? Source.Frequency : 1;

		// NoiseTessellation is the amount of tessellation that should occur between noise points.
		INT	TessFactor	= Source.NoiseTessellation ? Source.NoiseTessellation : 1;
		FLOAT	InvTessFactor	= 1.0f / TessFactor;
		INT		i;

		// The last position processed
		FVector	LastPosition, LastDrawPosition, LastTangent;
		// The current position
		FVector	CurrPosition, CurrDrawPosition;
		// The target
		FVector	TargetPosition, TargetDrawPosition;
		// The next target
		FVector	NextTargetPosition, NextTargetDrawPosition, TargetTangent;
		// The interperted draw position
		FVector InterpDrawPos;
		FVector	InterimDrawPosition;

		FVector	Size;

		FVector Location;
		FVector EndPoint;
		FVector Offset;
		FVector LastOffset;
		FLOAT	fStrength;
		FLOAT	fTargetStrength;

		INT	 VertexCount	= 0;

		// Tessellate the beam along the noise points
		for (i = 0; i < Source.ActiveParticleCount; i++)
		{
			DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * i);

			// Retrieve the beam data from the particle.
			FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
			FVector*				InterpolatedPoints	= NULL;
			FLOAT*					NoiseRate			= NULL;
			FLOAT*					NoiseDelta			= NULL;
			FVector*				TargetNoisePoints	= NULL;
			FVector*				NextNoisePoints		= NULL;
			FLOAT*					TaperValues			= NULL;
			FLOAT*					NoiseDistanceScale	= NULL;

			BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + Source.BeamDataOffset);
			if (BeamPayloadData->TriangleCount == 0)
			{
				continue;
			}
			if (Source.InterpolatedPointsOffset != -1)
			{
				InterpolatedPoints = (FVector*)((BYTE*)Particle + Source.InterpolatedPointsOffset);
			}
			if (Source.NoiseRateOffset != -1)
			{
				NoiseRate = (FLOAT*)((BYTE*)Particle + Source.NoiseRateOffset);
			}
			if (Source.NoiseDeltaTimeOffset != -1)
			{
				NoiseDelta = (FLOAT*)((BYTE*)Particle + Source.NoiseDeltaTimeOffset);
			}
			if (Source.TargetNoisePointsOffset != -1)
			{
				TargetNoisePoints = (FVector*)((BYTE*)Particle + Source.TargetNoisePointsOffset);
			}
			if (Source.NextNoisePointsOffset != -1)
			{
				NextNoisePoints = (FVector*)((BYTE*)Particle + Source.NextNoisePointsOffset);
			}
			if (Source.TaperValuesOffset != -1)
			{
				TaperValues = (FLOAT*)((BYTE*)Particle + Source.TaperValuesOffset);
			}
			if (Source.NoiseDistanceScaleOffset != -1)
			{
				NoiseDistanceScale = (FLOAT*)((BYTE*)Particle + Source.NoiseDistanceScaleOffset);
			}

			FLOAT NoiseDistScale = 1.0f;
			if (NoiseDistanceScale)
			{
				NoiseDistScale = *NoiseDistanceScale;
			}

			FVector* NoisePoints	= TargetNoisePoints;
			FVector* NextNoise		= NextNoisePoints;

			FLOAT NoiseRangeScaleFactor = Source.NoiseRangeScale;
			//@todo. How to handle no noise points?
			// If there are no noise points, why are we in here?
			if (NoisePoints == NULL)
			{
				continue;
			}

			// Pin the size to the X component
			Size = FVector(Particle->Size.X * Source.Scale.X);

			check(TessFactor > 0);

			// Setup the current position as the source point
			CurrPosition		= BeamPayloadData->SourcePoint;
			CurrDrawPosition	= CurrPosition;

			// Setup the source tangent & strength
			if (Source.bUseSource)
			{
				// The source module will have determined the proper source tangent.
				LastTangent	= BeamPayloadData->SourceTangent;
				fStrength	= BeamPayloadData->SourceStrength;
			}
			else
			{
				// We don't have a source module, so use the orientation of the emitter.
				LastTangent	= WorldToLocal.GetAxis(0);
				fStrength	= Source.NoiseTangentStrength;
			}
			LastTangent.Normalize();
			LastTangent *= fStrength;
			fTargetStrength	= Source.NoiseTangentStrength;

			// Set the last draw position to the source so we don't get 'under-hang'
			LastPosition		= CurrPosition;
			LastDrawPosition	= CurrDrawPosition;

			UBOOL	bLocked	= BEAM2_TYPEDATA_LOCKED(BeamPayloadData->Lock_Max_NumNoisePoints);

			FVector	UseNoisePoint, CheckNoisePoint;
			FVector	NoiseDir;

			// Reset the texture coordinate
			LastPosition		= BeamPayloadData->SourcePoint;
			LastDrawPosition	= LastPosition;

			// Determine the current position by stepping the direct line and offsetting with the noise point. 
			CurrPosition		= LastPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;

			if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
			{
				NoiseDir		= NextNoise[0] - NoisePoints[0];
				NoiseDir.Normalize();
				CheckNoisePoint	= NoisePoints[0] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
				if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[0].X) < Source.NoiseLockRadius) &&
					(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[0].Y) < Source.NoiseLockRadius) &&
					(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[0].Z) < Source.NoiseLockRadius))
				{
					NoisePoints[0]	= NextNoise[0];
				}
				else
				{
					NoisePoints[0]	= CheckNoisePoint;
				}
			}

			CurrDrawPosition	= CurrPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[0] * NoiseDistScale);

			// Determine the offset for the leading edge
			Location	= LastDrawPosition;
			EndPoint	= CurrDrawPosition;

			// 'Lead' edge
			DrawWireStar(PDI, Location, 15.0f, FColor(0,255,0), DPGIndex);

			for (INT StepIndex = 0; StepIndex < BeamPayloadData->Steps; StepIndex++)
			{
				// Determine the current position by stepping the direct line and offsetting with the noise point. 
				CurrPosition		= LastPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;

				if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
				{
					NoiseDir		= NextNoise[StepIndex] - NoisePoints[StepIndex];
					NoiseDir.Normalize();
					CheckNoisePoint	= NoisePoints[StepIndex] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
					if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[StepIndex].X) < Source.NoiseLockRadius) &&
						(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[StepIndex].Y) < Source.NoiseLockRadius) &&
						(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[StepIndex].Z) < Source.NoiseLockRadius))
					{
						NoisePoints[StepIndex]	= NextNoise[StepIndex];
					}
					else
					{
						NoisePoints[StepIndex]	= CheckNoisePoint;
					}
				}

				CurrDrawPosition	= CurrPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[StepIndex] * NoiseDistScale);

				// Prep the next draw position to determine tangents
				UBOOL bTarget = FALSE;
				NextTargetPosition	= CurrPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;
				if (bLocked && ((StepIndex + 1) == BeamPayloadData->Steps))
				{
					// If we are locked, and the next step is the target point, set the draw position as such.
					// (ie, we are on the last noise point...)
					NextTargetDrawPosition	= BeamPayloadData->TargetPoint;
					if (Source.bTargetNoise)
					{
						if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
						{
							NoiseDir		= NextNoise[Source.Frequency] - NoisePoints[Source.Frequency];
							NoiseDir.Normalize();
							CheckNoisePoint	= NoisePoints[Source.Frequency] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
							if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Source.Frequency].X) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Source.Frequency].Y) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Source.Frequency].Z) < Source.NoiseLockRadius))
							{
								NoisePoints[Source.Frequency]	= NextNoise[Source.Frequency];
							}
							else
							{
								NoisePoints[Source.Frequency]	= CheckNoisePoint;
							}
						}

						NextTargetDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[Source.Frequency] * NoiseDistScale);
					}
					TargetTangent = BeamPayloadData->TargetTangent;
					fTargetStrength	= BeamPayloadData->TargetStrength;
				}
				else
				{
					// Just another noise point... offset the target to get the draw position.
					if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
					{
						NoiseDir		= NextNoise[StepIndex + 1] - NoisePoints[StepIndex + 1];
						NoiseDir.Normalize();
						CheckNoisePoint	= NoisePoints[StepIndex + 1] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
						if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[StepIndex + 1].X) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[StepIndex + 1].Y) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[StepIndex + 1].Z) < Source.NoiseLockRadius))
						{
							NoisePoints[StepIndex + 1]	= NextNoise[StepIndex + 1];
						}
						else
						{
							NoisePoints[StepIndex + 1]	= CheckNoisePoint;
						}
					}

					NextTargetDrawPosition	= NextTargetPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[StepIndex + 1] * NoiseDistScale);

					TargetTangent = ((1.0f - Source.NoiseTension) / 2.0f) * (NextTargetDrawPosition - LastDrawPosition);
				}
				TargetTangent.Normalize();
				TargetTangent *= fTargetStrength;

				InterimDrawPosition = LastDrawPosition;
				// Tessellate between the current position and the last position
				for (INT TessIndex = 0; TessIndex < TessFactor; TessIndex++)
				{
					InterpDrawPos = CubicInterp(
						LastDrawPosition, LastTangent,
						CurrDrawPosition, TargetTangent,
						InvTessFactor * (TessIndex + 1));

					Location	= InterimDrawPosition;
					EndPoint	= InterpDrawPos;

					FColor StarColor(255,0,255);
					if (TessIndex == 0)
					{
						StarColor = FColor(0,0,255);
					}
					else
					if (TessIndex == (TessFactor - 1))
					{
						StarColor = FColor(255,255,0);
					}

					// Generate the vertex
					DrawWireStar(PDI, EndPoint, 15.0f, StarColor, DPGIndex);
					PDI->DrawLine(Location, EndPoint, FLinearColor(1.0f,1.0f,0.0f), DPGIndex);
					InterimDrawPosition	= InterpDrawPos;
				}
				LastPosition		= CurrPosition;
				LastDrawPosition	= CurrDrawPosition;
				LastTangent			= TargetTangent;
			}

			if (bLocked)
			{
				// Draw the line from the last point to the target
				CurrDrawPosition	= BeamPayloadData->TargetPoint;
				if (Source.bTargetNoise)
				{
					if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
					{
						NoiseDir		= NextNoise[Source.Frequency] - NoisePoints[Source.Frequency];
						NoiseDir.Normalize();
						CheckNoisePoint	= NoisePoints[Source.Frequency] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
						if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Source.Frequency].X) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Source.Frequency].Y) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Source.Frequency].Z) < Source.NoiseLockRadius))
						{
							NoisePoints[Source.Frequency]	= NextNoise[Source.Frequency];
						}
						else
						{
							NoisePoints[Source.Frequency]	= CheckNoisePoint;
						}
					}

					CurrDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[Source.Frequency] * NoiseDistScale);
				}

				if (Source.bUseTarget)
				{
					TargetTangent = BeamPayloadData->TargetTangent;
				}
				else
				{
					NextTargetDrawPosition	= CurrPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;
					TargetTangent = ((1.0f - Source.NoiseTension) / 2.0f) * 
						(NextTargetDrawPosition - LastDrawPosition);
				}
				TargetTangent.Normalize();
				TargetTangent *= fTargetStrength;

				// Tessellate this segment
				InterimDrawPosition = LastDrawPosition;
				for (INT TessIndex = 0; TessIndex < TessFactor; TessIndex++)
				{
					InterpDrawPos = CubicInterp(
						LastDrawPosition, LastTangent,
						CurrDrawPosition, TargetTangent,
						InvTessFactor * (TessIndex + 1));

					Location	= InterimDrawPosition;
					EndPoint	= InterpDrawPos;

					FColor StarColor(255,0,255);
					if (TessIndex == 0)
					{
						StarColor = FColor(255,255,255);
					}
					else
					if (TessIndex == (TessFactor - 1))
					{
						StarColor = FColor(255,255,0);
					}

					// Generate the vertex
					DrawWireStar(PDI, EndPoint, 15.0f, StarColor, DPGIndex);
					PDI->DrawLine(Location, EndPoint, FLinearColor(1.0f,1.0f,0.0f), DPGIndex);
					VertexCount++;
					InterimDrawPosition	= InterpDrawPos;
				}
			}
		}
	}

	if (Source.InterpolationPoints > 1)
	{
		FMatrix CameraToWorld = View->ViewMatrix.Inverse();
		FVector	ViewOrigin = CameraToWorld.GetOrigin();
		INT TessFactor = Source.InterpolationPoints ? Source.InterpolationPoints : 1;

		if (TessFactor <= 1)
		{
			for (INT i = 0; i < Source.ActiveParticleCount; i++)
			{
				DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * i);
				FBeam2TypeDataPayload* BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + Source.BeamDataOffset);
				if (BeamPayloadData->TriangleCount == 0)
				{
					continue;
				}

				FVector EndPoint	= Particle->Location;
				FVector Location	= BeamPayloadData->SourcePoint;

				DrawWireStar(PDI, Location, 15.0f, FColor(255,0,0), DPGIndex);
				DrawWireStar(PDI, EndPoint, 15.0f, FColor(255,0,0), DPGIndex);
				PDI->DrawLine(Location, EndPoint, FColor(255,255,0), DPGIndex);
			}
		}
		else
		{
			for (INT i = 0; i < Source.ActiveParticleCount; i++)
			{
				DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * i);

				FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
				FVector*				InterpolatedPoints	= NULL;

				BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + Source.BeamDataOffset);
				if (BeamPayloadData->TriangleCount == 0)
				{
					continue;
				}
				if (Source.InterpolatedPointsOffset != -1)
				{
					InterpolatedPoints = (FVector*)((BYTE*)Particle + Source.InterpolatedPointsOffset);
				}

				FVector Location;
				FVector EndPoint;

				check(InterpolatedPoints);	// TTP #33139

				Location	= BeamPayloadData->SourcePoint;
				EndPoint	= InterpolatedPoints[0];

				DrawWireStar(PDI, Location, 15.0f, FColor(255,0,0), DPGIndex);
				for (INT StepIndex = 0; StepIndex < BeamPayloadData->InterpolationSteps; StepIndex++)
				{
					EndPoint = InterpolatedPoints[StepIndex];
					DrawWireStar(PDI, EndPoint, 15.0f, FColor(255,0,0), DPGIndex);
					PDI->DrawLine(Location, EndPoint, FColor(255,255,0), DPGIndex);
					Location = EndPoint;
				}
			}
		}
	}
}

void FDynamicBeam2EmitterData::RenderDebug(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, UBOOL bCrosses)
{
}

INT FDynamicBeam2EmitterData::FillIndexData(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_BeamFillIndexTime);

	INT	TrianglesToRender = 0;

	// Beam2 polygons are packed and joined as follows:
	//
	// 1--3--5--7--9-...
	// |\ |\ |\ |\ |\...
	// | \| \| \| \| ...
	// 0--2--4--6--8-...
	//
	// (ie, the 'leading' edge of polygon (n) is the trailing edge of polygon (n+1)
	//
	// NOTE: This is primed for moving to tri-strips...
	//
	INT TessFactor	= Source.InterpolationPoints ? Source.InterpolationPoints : 1;
	if (Source.Sheets <= 0)
	{
		Source.Sheets = 1;
	}

	//	UBOOL bWireframe = ((View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials));
	UBOOL bWireframe = FALSE;

	INT TempIndexCount = 0;
	for (INT ii = 0; ii < Source.TrianglesPerSheet.Num(); ii++)
	{
		INT Triangles = Source.TrianglesPerSheet(ii);
		if (bWireframe)
		{
			TempIndexCount += (8 * Triangles + 2) * Source.Sheets;
		}
		else
		{
			if (TempIndexCount == 0)
			{
				TempIndexCount = 2;
			}
			TempIndexCount += Triangles * Source.Sheets;
			TempIndexCount += 4 * (Source.Sheets - 1);	// Degenerate indices between sheets
			if ((ii + 1) < Source.TrianglesPerSheet.Num())
			{
				TempIndexCount += 4;	// Degenerate indices between beams
			}
		}
	}

	if ((IndexData == NULL) || (IndexCount < Source.IndexCount))
	{
		IndexData = appRealloc(IndexData, Source.IndexCount * Source.IndexStride);
		check(IndexData);
		IndexCount = Source.IndexCount;
	}

	if (Source.IndexStride == sizeof(WORD))
	{
		WORD*	Index				= (WORD*)IndexData;
		WORD	VertexIndex			= 0;
		WORD	StartVertexIndex	= 0;

		for (INT Beam = 0; Beam < Source.ActiveParticleCount; Beam++)
		{
			DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * Beam);

			FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
			FVector*				InterpolatedPoints	= NULL;
			FLOAT*					NoiseRate			= NULL;
			FLOAT*					NoiseDelta			= NULL;
			FVector*				TargetNoisePoints	= NULL;
			FVector*				NextNoisePoints		= NULL;
			FLOAT*					TaperValues			= NULL;

			BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + Source.BeamDataOffset);
			if (BeamPayloadData->TriangleCount == 0)
			{
				continue;
			}

			if (bWireframe)
			{
				for (INT SheetIndex = 0; SheetIndex < Source.Sheets; SheetIndex++)
				{
					VertexIndex = 0;

					// The 'starting' line
					TrianglesToRender += 1;
					*(Index++) = StartVertexIndex + 0;
					*(Index++) = StartVertexIndex + 1;

					// 4 lines per quad
					INT TriCount = Source.TrianglesPerSheet(Beam);
					INT QuadCount = TriCount / 2;
					TrianglesToRender += TriCount * 2;

					for (INT i = 0; i < QuadCount; i++)
					{
						*(Index++) = StartVertexIndex + VertexIndex + 0;
						*(Index++) = StartVertexIndex + VertexIndex + 2;
						*(Index++) = StartVertexIndex + VertexIndex + 1;
						*(Index++) = StartVertexIndex + VertexIndex + 2;
						*(Index++) = StartVertexIndex + VertexIndex + 1;
						*(Index++) = StartVertexIndex + VertexIndex + 3;
						*(Index++) = StartVertexIndex + VertexIndex + 2;
						*(Index++) = StartVertexIndex + VertexIndex + 3;

						VertexIndex += 2;
					}

					StartVertexIndex += TriCount + 2;
				}
			}
			else
			{
				// 
				if (Beam == 0)
				{
					*(Index++) = VertexIndex++;	// SheetIndex + 0
					*(Index++) = VertexIndex++;	// SheetIndex + 1
				}

				for (INT SheetIndex = 0; SheetIndex < Source.Sheets; SheetIndex++)
				{
					// 2 triangles per tessellation factor
					TrianglesToRender += BeamPayloadData->TriangleCount;

					// Sequentially step through each triangle - 1 vertex per triangle
					for (INT i = 0; i < BeamPayloadData->TriangleCount; i++)
					{
						*(Index++) = VertexIndex++;
					}

					// Degenerate tris
					if ((SheetIndex + 1) < Source.Sheets)
					{
						*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
						*(Index++) = VertexIndex;		// First vertex of the next sheet
						*(Index++) = VertexIndex++;		// First vertex of the next sheet
						*(Index++) = VertexIndex++;		// Second vertex of the next sheet

						TrianglesToRender += 4;
					}
				}
				if ((Beam + 1) < Source.ActiveParticleCount)
				{
					*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
					*(Index++) = VertexIndex;		// First vertex of the next sheet
					*(Index++) = VertexIndex++;		// First vertex of the next sheet
					*(Index++) = VertexIndex++;		// Second vertex of the next sheet

					TrianglesToRender += 4;
				}
			}
		}
	}
	else
	{
		check(!TEXT("Rendering beam with > 5000 vertices!"));
		DWORD*	Index		= (DWORD*)IndexData;
		DWORD	VertexIndex	= 0;
		for (INT Beam = 0; Beam < Source.ActiveParticleCount; Beam++)
		{
			DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * Beam);

			FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
			BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + Source.BeamDataOffset);
			if (BeamPayloadData->TriangleCount == 0)
			{
				continue;
			}

			// 
			if (Beam == 0)
			{
				*(Index++) = VertexIndex++;	// SheetIndex + 0
				*(Index++) = VertexIndex++;	// SheetIndex + 1
			}

			for (INT SheetIndex = 0; SheetIndex < Source.Sheets; SheetIndex++)
			{
				// 2 triangles per tessellation factor
				TrianglesToRender += BeamPayloadData->TriangleCount;

				// Sequentially step through each triangle - 1 vertex per triangle
				for (INT i = 0; i < BeamPayloadData->TriangleCount; i++)
				{
					*(Index++) = VertexIndex++;
				}

				// Degenerate tris
				if ((SheetIndex + 1) < Source.Sheets)
				{
					*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
					*(Index++) = VertexIndex;		// First vertex of the next sheet
					*(Index++) = VertexIndex++;		// First vertex of the next sheet
					*(Index++) = VertexIndex++;		// Second vertex of the next sheet
					TrianglesToRender += 4;
				}
			}
			if ((Beam + 1) < Source.ActiveParticleCount)
			{
				*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
				*(Index++) = VertexIndex;		// First vertex of the next sheet
				*(Index++) = VertexIndex++;		// First vertex of the next sheet
				*(Index++) = VertexIndex++;		// Second vertex of the next sheet
				TrianglesToRender += 4;
			}
		}
	}

	return TrianglesToRender;
}

INT FDynamicBeam2EmitterData::FillVertexData_NoNoise(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_BeamFillVertexTime);

	INT	TrianglesToRender = 0;

	FParticleSpriteVertex* Vertex = (FParticleSpriteVertex*)VertexData;
	FMatrix CameraToWorld = View->ViewMatrix.InverseSafe();
	FVector	ViewOrigin = CameraToWorld.GetOrigin();
	FVector ViewDirection = View->ViewMatrix.GetAxis(0);
	INT TessFactor = Source.InterpolationPoints ? Source.InterpolationPoints : 1;

	if (Source.Sheets <= 0)
	{
		Source.Sheets = 1;
	}

	FVector	Offset(0.0f), LastOffset(0.0f);
	FVector	Size;

	INT PackedCount = 0;

	if (TessFactor <= 1)
	{
		for (INT i = 0; i < Source.ActiveParticleCount; i++)
		{
			DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * i);

			FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
			FVector*				InterpolatedPoints	= NULL;
			FLOAT*					NoiseRate			= NULL;
			FLOAT*					NoiseDelta			= NULL;
			FVector*				TargetNoisePoints	= NULL;
			FVector*				NextNoisePoints		= NULL;
			FLOAT*					TaperValues			= NULL;

			BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + Source.BeamDataOffset);
			if (BeamPayloadData->TriangleCount == 0)
			{
				continue;
			}
			if (Source.InterpolatedPointsOffset != -1)
			{
				InterpolatedPoints = (FVector*)((BYTE*)Particle + Source.InterpolatedPointsOffset);
			}
			if (Source.NoiseRateOffset != -1)
			{
				NoiseRate = (FLOAT*)((BYTE*)Particle + Source.NoiseRateOffset);
			}
			if (Source.NoiseDeltaTimeOffset != -1)
			{
				NoiseDelta = (FLOAT*)((BYTE*)Particle + Source.NoiseDeltaTimeOffset);
			}
			if (Source.TargetNoisePointsOffset != -1)
			{
				TargetNoisePoints = (FVector*)((BYTE*)Particle + Source.TargetNoisePointsOffset);
			}
			if (Source.NextNoisePointsOffset != -1)
			{
				NextNoisePoints = (FVector*)((BYTE*)Particle + Source.NextNoisePointsOffset);
			}
			if (Source.TaperValuesOffset != -1)
			{
				TaperValues = (FLOAT*)((BYTE*)Particle + Source.TaperValuesOffset);
			}

			// Pin the size to the X component
			Size	= FVector(Particle->Size.X * Source.Scale.X);

			FVector EndPoint	= Particle->Location;
			FVector Location	= BeamPayloadData->SourcePoint;
			FVector Right, Up;
			FVector WorkingUp;

			Right = Location - EndPoint;
			Right.Normalize();
			if (((Source.UpVectorStepSize == 1) && (i == 0)) || (Source.UpVectorStepSize == 0))
			{
				//Up = Right ^ ViewDirection;
				Up = Right ^ (Location - ViewOrigin);
				if (!Up.Normalize())
				{
					Up = CameraToWorld.GetAxis(1);
				}
			}

			FLOAT	fUEnd;
			FLOAT	Tiles		= 1.0f;
			if (Source.TextureTileDistance > KINDA_SMALL_NUMBER)
			{
				FVector	Direction	= BeamPayloadData->TargetPoint - BeamPayloadData->SourcePoint;
				FLOAT	Distance	= Direction.Size();
				Tiles				= Distance / Source.TextureTileDistance;
			}
			fUEnd		= Tiles;

			if (BeamPayloadData->TravelRatio > KINDA_SMALL_NUMBER)
			{
				fUEnd	= Tiles * BeamPayloadData->TravelRatio;
			}

			// For the direct case, this isn't a big deal, as it will not require much work per sheet.
			for (INT SheetIndex = 0; SheetIndex < Source.Sheets; SheetIndex++)
			{
				if (SheetIndex)
				{
					FLOAT	Angle		= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
					FQuat	QuatRotator	= FQuat(Right, Angle);
					WorkingUp			= QuatRotator.RotateVector(Up);
				}
				else
				{
					WorkingUp	= Up;
				}

				FLOAT	Taper	= 1.0f;
				if (Source.TaperMethod != PEBTM_None)
				{
					check(TaperValues);
					Taper	= TaperValues[0];
				}

				Offset.X		= WorkingUp.X * Size.X * Taper;
				Offset.Y		= WorkingUp.Y * Size.Y * Taper;
				Offset.Z		= WorkingUp.Z * Size.Z * Taper;

				// 'Lead' edge
				Vertex->Position	= Location + Offset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= 0.0f;
				Vertex->Tex_V		= 0.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				PackedCount++;

				Vertex->Position	= Location - Offset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= 0.0f;
				Vertex->Tex_V		= 1.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				PackedCount++;

				if (Source.TaperMethod != PEBTM_None)
				{
					check(TaperValues);
					Taper	= TaperValues[1];
				}

				Offset.X		= WorkingUp.X * Size.X * Taper;
				Offset.Y		= WorkingUp.Y * Size.Y * Taper;
				Offset.Z		= WorkingUp.Z * Size.Z * Taper;

				//
				Vertex->Position	= EndPoint + Offset;
				Vertex->OldPosition	= Particle->OldLocation;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fUEnd;
				Vertex->Tex_V		= 0.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				PackedCount++;

				Vertex->Position	= EndPoint - Offset;
				Vertex->OldPosition	= Particle->OldLocation;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fUEnd;
				Vertex->Tex_V		= 1.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				PackedCount++;
			}
		}
	}
	else
	{
		FLOAT	fTextureIncrement	= 1.0f / Source.InterpolationPoints;;

		for (INT i = 0; i < Source.ActiveParticleCount; i++)
		{
			DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * i);

			FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
			FVector*				InterpolatedPoints	= NULL;
			FLOAT*					NoiseRate			= NULL;
			FLOAT*					NoiseDelta			= NULL;
			FVector*				TargetNoisePoints	= NULL;
			FVector*				NextNoisePoints		= NULL;
			FLOAT*					TaperValues			= NULL;

			BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + Source.BeamDataOffset);
			if (BeamPayloadData->TriangleCount == 0)
			{
				continue;
			}
			if (Source.InterpolatedPointsOffset != -1)
			{
				InterpolatedPoints = (FVector*)((BYTE*)Particle + Source.InterpolatedPointsOffset);
			}
			if (Source.NoiseRateOffset != -1)
			{
				NoiseRate = (FLOAT*)((BYTE*)Particle + Source.NoiseRateOffset);
			}
			if (Source.NoiseDeltaTimeOffset != -1)
			{
				NoiseDelta = (FLOAT*)((BYTE*)Particle + Source.NoiseDeltaTimeOffset);
			}
			if (Source.TargetNoisePointsOffset != -1)
			{
				TargetNoisePoints = (FVector*)((BYTE*)Particle + Source.TargetNoisePointsOffset);
			}
			if (Source.NextNoisePointsOffset != -1)
			{
				NextNoisePoints = (FVector*)((BYTE*)Particle + Source.NextNoisePointsOffset);
			}
			if (Source.TaperValuesOffset != -1)
			{
				TaperValues = (FLOAT*)((BYTE*)Particle + Source.TaperValuesOffset);
			}

			if (Source.TextureTileDistance > KINDA_SMALL_NUMBER)
			{
				FVector	Direction	= BeamPayloadData->TargetPoint - BeamPayloadData->SourcePoint;
				FLOAT	Distance	= Direction.Size();
				FLOAT	Tiles		= Distance / Source.TextureTileDistance;
				fTextureIncrement	= Tiles / Source.InterpolationPoints;
			}

			// Pin the size to the X component
			Size	= FVector(Particle->Size.X * Source.Scale.X);

			FLOAT	Angle;
			FQuat	QuatRotator(0, 0, 0, 0);

			FVector Location;
			FVector EndPoint;
			FVector Right;
			FVector Up;
			FVector WorkingUp;
			FLOAT	fU;

			check(InterpolatedPoints);	// TTP #33139
			// For the direct case, this isn't a big deal, as it will not require much work per sheet.
			for (INT SheetIndex = 0; SheetIndex < Source.Sheets; SheetIndex++)
			{
				fU			= 0.0f;
				Location	= BeamPayloadData->SourcePoint;
				EndPoint	= InterpolatedPoints[0];
				Right		= Location - EndPoint;
				Right.Normalize();
				if (Source.UpVectorStepSize == 0)
				{
					//Up = Right ^ ViewDirection;
					Up = Right ^ (Location - ViewOrigin);
					if (!Up.Normalize())
					{
						Up = CameraToWorld.GetAxis(1);
					}
				}

				if (SheetIndex)
				{
					Angle		= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
					QuatRotator	= FQuat(Right, Angle);
					WorkingUp	= QuatRotator.RotateVector(Up);
				}
				else
				{
					WorkingUp	= Up;
				}

				FLOAT	Taper	= 1.0f;

				if (Source.TaperMethod != PEBTM_None)
				{
					check(TaperValues);
					Taper	= TaperValues[0];
				}

				Offset.X	= WorkingUp.X * Size.X * Taper;
				Offset.Y	= WorkingUp.Y * Size.Y * Taper;
				Offset.Z	= WorkingUp.Z * Size.Z * Taper;

				// 'Lead' edge
				Vertex->Position	= Location + Offset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fU;
				Vertex->Tex_V		= 0.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				PackedCount++;

				Vertex->Position	= Location - Offset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fU;
				Vertex->Tex_V		= 1.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				PackedCount++;

				for (INT StepIndex = 0; StepIndex < BeamPayloadData->Steps; StepIndex++)
				{
					EndPoint	= InterpolatedPoints[StepIndex];
					if (Source.UpVectorStepSize == 0)
					{
						//Up = Right ^ ViewDirection;
						Up = Right ^ (Location - ViewOrigin);
						if (!Up.Normalize())
						{
							Up = CameraToWorld.GetAxis(1);
						}
					}

					if (SheetIndex)
					{
						WorkingUp	= QuatRotator.RotateVector(Up);
					}
					else
					{
						WorkingUp	= Up;
					}

					if (Source.TaperMethod != PEBTM_None)
					{
						check(TaperValues);
						Taper	= TaperValues[StepIndex + 1];
					}

					Offset.X		= WorkingUp.X * Size.X * Taper;
					Offset.Y		= WorkingUp.Y * Size.Y * Taper;
					Offset.Z		= WorkingUp.Z * Size.Z * Taper;

					//
					Vertex->Position	= EndPoint + Offset;
					Vertex->OldPosition	= EndPoint;
					Vertex->Size		= Size;
					Vertex->Tex_U		= fU + fTextureIncrement;
					Vertex->Tex_V		= 0.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= Particle->Color;
					Vertex++;
				PackedCount++;

					Vertex->Position	= EndPoint - Offset;
					Vertex->OldPosition	= EndPoint;
					Vertex->Size		= Size;
					Vertex->Tex_U		= fU + fTextureIncrement;
					Vertex->Tex_V		= 1.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= Particle->Color;
					Vertex++;
				PackedCount++;

					Location			 = EndPoint;
					fU					+= fTextureIncrement;
				}

				if (BeamPayloadData->TravelRatio > KINDA_SMALL_NUMBER)
				{
					//@todo.SAS. Re-implement partial-segment beams
				}
			}
		}
	}

	check(PackedCount <= Source.VertexCount);

	return TrianglesToRender;
}

INT FDynamicBeam2EmitterData::FillData_Noise(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_BeamFillVertexTime);

	INT	TrianglesToRender = 0;

	if (Source.InterpolationPoints > 0)
	{
		return FillData_InterpolatedNoise(Proxy, PDI, View, DPGIndex);
	}

	FParticleSpriteVertex* Vertex = (FParticleSpriteVertex*)VertexData;
	FMatrix CameraToWorld = View->ViewMatrix.InverseSafe();
	FVector ViewDirection	= View->ViewMatrix.GetAxis(0);

	if (Source.Sheets <= 0)
	{
		Source.Sheets = 1;
	}

	FVector	ViewOrigin	= CameraToWorld.GetOrigin();

	// Frequency is the number of noise points to generate, evenly distributed along the line.
	if (Source.Frequency <= 0)
	{
		Source.Frequency = 1;
	}

	// NoiseTessellation is the amount of tessellation that should occur between noise points.
	INT	TessFactor	= Source.NoiseTessellation ? Source.NoiseTessellation : 1;
	
	FLOAT	InvTessFactor	= 1.0f / TessFactor;
	INT		i;

	// The last position processed
	FVector	LastPosition, LastDrawPosition, LastTangent;
	// The current position
	FVector	CurrPosition, CurrDrawPosition;
	// The target
	FVector	TargetPosition, TargetDrawPosition;
	// The next target
	FVector	NextTargetPosition, NextTargetDrawPosition, TargetTangent;
	// The interperted draw position
	FVector InterpDrawPos;
	FVector	InterimDrawPosition;

	FVector	Size;

	FLOAT	Angle;
	FQuat	QuatRotator;

	FVector Location;
	FVector EndPoint;
	FVector Right;
	FVector Up;
	FVector WorkingUp;
	FVector LastUp;
	FVector WorkingLastUp;
	FVector Offset;
	FVector LastOffset;
	FLOAT	fStrength;
	FLOAT	fTargetStrength;

	FLOAT	fU;
	FLOAT	TextureIncrement	= 1.0f / (((Source.Frequency > 0) ? Source.Frequency : 1) * TessFactor);	// TTP #33140/33159

	INT	 CheckVertexCount	= 0;

	FVector THE_Up = FVector(0.0f);

	FMatrix WorldToLocal = Proxy->GetWorldToLocal();
	FMatrix LocalToWorld = Proxy->GetLocalToWorld();

	// Tessellate the beam along the noise points
	for (i = 0; i < Source.ActiveParticleCount; i++)
	{
		DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * i);

		// Retrieve the beam data from the particle.
		FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
		FVector*				InterpolatedPoints	= NULL;
		FLOAT*					NoiseRate			= NULL;
		FLOAT*					NoiseDelta			= NULL;
		FVector*				TargetNoisePoints	= NULL;
		FVector*				NextNoisePoints		= NULL;
		FLOAT*					TaperValues			= NULL;
		FLOAT*					NoiseDistanceScale	= NULL;

		BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + Source.BeamDataOffset);
		if (BeamPayloadData->TriangleCount == 0)
		{
			continue;
		}
		if (Source.InterpolatedPointsOffset != -1)
		{
			InterpolatedPoints = (FVector*)((BYTE*)Particle + Source.InterpolatedPointsOffset);
		}
		if (Source.NoiseRateOffset != -1)
		{
			NoiseRate = (FLOAT*)((BYTE*)Particle + Source.NoiseRateOffset);
		}
		if (Source.NoiseDeltaTimeOffset != -1)
		{
			NoiseDelta = (FLOAT*)((BYTE*)Particle + Source.NoiseDeltaTimeOffset);
		}
		if (Source.TargetNoisePointsOffset != -1)
		{
			TargetNoisePoints = (FVector*)((BYTE*)Particle + Source.TargetNoisePointsOffset);
		}
		if (Source.NextNoisePointsOffset != -1)
		{
			NextNoisePoints = (FVector*)((BYTE*)Particle + Source.NextNoisePointsOffset);
		}
		if (Source.TaperValuesOffset != -1)
		{
			TaperValues = (FLOAT*)((BYTE*)Particle + Source.TaperValuesOffset);
		}
		if (Source.NoiseDistanceScaleOffset != -1)
		{
			NoiseDistanceScale = (FLOAT*)((BYTE*)Particle + Source.NoiseDistanceScaleOffset);
		}

		FLOAT NoiseDistScale = 1.0f;
		if (NoiseDistanceScale)
		{
			NoiseDistScale = *NoiseDistanceScale;
		}

		FVector* NoisePoints	= TargetNoisePoints;
		FVector* NextNoise		= NextNoisePoints;

		FLOAT NoiseRangeScaleFactor = Source.NoiseRangeScale;
		//@todo. How to handle no noise points?
		// If there are no noise points, why are we in here?
		if (NoisePoints == NULL)
		{
			continue;
		}

		// Pin the size to the X component
		Size	= FVector(Particle->Size.X * Source.Scale.X);

		if (TessFactor <= 1)
		{
			// Setup the current position as the source point
			CurrPosition		= BeamPayloadData->SourcePoint;
			CurrDrawPosition	= CurrPosition;

			// Setup the source tangent & strength
			if (Source.bUseSource)
			{
				// The source module will have determined the proper source tangent.
				LastTangent	= BeamPayloadData->SourceTangent;
				fStrength	= BeamPayloadData->SourceStrength;
			}
			else
			{
				// We don't have a source module, so use the orientation of the emitter.
				LastTangent	= WorldToLocal.GetAxis(0);
				fStrength	= Source.NoiseTangentStrength;
			}
			LastTangent.Normalize();
			LastTangent *= fStrength;

			fTargetStrength	= Source.NoiseTangentStrength;

			// Set the last draw position to the source so we don't get 'under-hang'
			LastPosition		= CurrPosition;
			LastDrawPosition	= CurrDrawPosition;

			UBOOL	bLocked	= BEAM2_TYPEDATA_LOCKED(BeamPayloadData->Lock_Max_NumNoisePoints);

			FVector	UseNoisePoint, CheckNoisePoint;
			FVector	NoiseDir;

			for (INT SheetIndex = 0; SheetIndex < Source.Sheets; SheetIndex++)
			{
				// Reset the texture coordinate
				fU					= 0.0f;
				LastPosition		= BeamPayloadData->SourcePoint;
				LastDrawPosition	= LastPosition;

				// Determine the current position by stepping the direct line and offsetting with the noise point. 
				CurrPosition		= LastPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;

				if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
				{
					NoiseDir		= NextNoise[0] - NoisePoints[0];
					NoiseDir.Normalize();
					CheckNoisePoint	= NoisePoints[0] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
					if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[0].X) < Source.NoiseLockRadius) &&
						(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[0].Y) < Source.NoiseLockRadius) &&
						(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[0].Z) < Source.NoiseLockRadius))
					{
						NoisePoints[0]	= NextNoise[0];
					}
					else
					{
						NoisePoints[0]	= CheckNoisePoint;
					}
				}

				CurrDrawPosition	= CurrPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[0] * NoiseDistScale);

				// Determine the offset for the leading edge
				Location	= LastDrawPosition;
				EndPoint	= CurrDrawPosition;
				Right		= Location - EndPoint;
				Right.Normalize();
				if (((Source.UpVectorStepSize == 1) && (i == 0)) || (Source.UpVectorStepSize == 0))
				{
					//LastUp = Right ^ ViewDirection;
					LastUp = Right ^ (Location - ViewOrigin);
					if (!LastUp.Normalize())
					{
						LastUp = CameraToWorld.GetAxis(1);
					}
					THE_Up = LastUp;
				}
				else
				{
					LastUp = THE_Up;
				}

				if (SheetIndex)
				{
					Angle			= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
					QuatRotator		= FQuat(Right, Angle);
					WorkingLastUp	= QuatRotator.RotateVector(LastUp);
				}
				else
				{
					WorkingLastUp	= LastUp;
				}

				FLOAT	Taper	= 1.0f;

				if (Source.TaperMethod != PEBTM_None)
				{
					check(TaperValues);
					Taper	= TaperValues[0];
				}

				LastOffset.X	= WorkingLastUp.X * Size.X * Taper;
				LastOffset.Y	= WorkingLastUp.Y * Size.Y * Taper;
				LastOffset.Z	= WorkingLastUp.Z * Size.Z * Taper;

				// 'Lead' edge
				Vertex->Position	= Location + LastOffset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fU;
				Vertex->Tex_V		= 0.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				CheckVertexCount++;

				Vertex->Position	= Location - LastOffset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fU;
				Vertex->Tex_V		= 1.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				CheckVertexCount++;

				fU	+= TextureIncrement;

				for (INT StepIndex = 0; StepIndex < BeamPayloadData->Steps; StepIndex++)
				{
					// Determine the current position by stepping the direct line and offsetting with the noise point. 
					CurrPosition		= LastPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;

					if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
					{
						NoiseDir		= NextNoise[StepIndex] - NoisePoints[StepIndex];
						NoiseDir.Normalize();
						CheckNoisePoint	= NoisePoints[StepIndex] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
						if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[StepIndex].X) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[StepIndex].Y) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[StepIndex].Z) < Source.NoiseLockRadius))
						{
							NoisePoints[StepIndex]	= NextNoise[StepIndex];
						}
						else
						{
							NoisePoints[StepIndex]	= CheckNoisePoint;
						}
					}

					CurrDrawPosition	= CurrPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[StepIndex] * NoiseDistScale);

					// Prep the next draw position to determine tangents
					UBOOL bTarget = FALSE;
					NextTargetPosition	= CurrPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;
					if (bLocked && ((StepIndex + 1) == BeamPayloadData->Steps))
					{
						// If we are locked, and the next step is the target point, set the draw position as such.
						// (ie, we are on the last noise point...)
						NextTargetDrawPosition	= BeamPayloadData->TargetPoint;
						if (Source.bTargetNoise)
						{
							if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
							{
								NoiseDir		= NextNoise[Source.Frequency] - NoisePoints[Source.Frequency];
								NoiseDir.Normalize();
								CheckNoisePoint	= NoisePoints[Source.Frequency] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
								if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Source.Frequency].X) < Source.NoiseLockRadius) &&
									(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Source.Frequency].Y) < Source.NoiseLockRadius) &&
									(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Source.Frequency].Z) < Source.NoiseLockRadius))
								{
									NoisePoints[Source.Frequency]	= NextNoise[Source.Frequency];
								}
								else
								{
									NoisePoints[Source.Frequency]	= CheckNoisePoint;
								}
							}

							NextTargetDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[Source.Frequency] * NoiseDistScale);
						}
						TargetTangent = BeamPayloadData->TargetTangent;
						fTargetStrength	= BeamPayloadData->TargetStrength;
					}
					else
					{
						// Just another noise point... offset the target to get the draw position.
						if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
						{
							NoiseDir		= NextNoise[StepIndex + 1] - NoisePoints[StepIndex + 1];
							NoiseDir.Normalize();
							CheckNoisePoint	= NoisePoints[StepIndex + 1] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
							if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[StepIndex + 1].X) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[StepIndex + 1].Y) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[StepIndex + 1].Z) < Source.NoiseLockRadius))
							{
								NoisePoints[StepIndex + 1]	= NextNoise[StepIndex + 1];
							}
							else
							{
								NoisePoints[StepIndex + 1]	= CheckNoisePoint;
							}
						}

						NextTargetDrawPosition	= NextTargetPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[StepIndex + 1] * NoiseDistScale);

						TargetTangent = ((1.0f - Source.NoiseTension) / 2.0f) * (NextTargetDrawPosition - LastDrawPosition);
					}
					TargetTangent.Normalize();
					TargetTangent *= fTargetStrength;

					InterimDrawPosition = LastDrawPosition;
					// Tessellate between the current position and the last position
					for (INT TessIndex = 0; TessIndex < TessFactor; TessIndex++)
					{
						InterpDrawPos = CubicInterp(
							LastDrawPosition, LastTangent,
							CurrDrawPosition, TargetTangent,
							InvTessFactor * (TessIndex + 1));

						Location	= InterimDrawPosition;
						EndPoint	= InterpDrawPos;
						Right		= Location - EndPoint;
						Right.Normalize();
						if (Source.UpVectorStepSize == 0)
						{
							//Up = Right ^  (Location - CameraToWorld.GetOrigin());
							Up = Right ^ (Location - ViewOrigin);
							if (!Up.Normalize())
							{
								Up = CameraToWorld.GetAxis(1);
							}
						}
						else
						{
							Up = THE_Up;
						}

						if (SheetIndex)
						{
							Angle		= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
							QuatRotator	= FQuat(Right, Angle);
							WorkingUp	= QuatRotator.RotateVector(Up);
						}
						else
						{
							WorkingUp	= Up;
						}

						if (Source.TaperMethod != PEBTM_None)
						{
							check(TaperValues);
							Taper	= TaperValues[StepIndex * TessFactor + TessIndex];
						}

						Offset.X	= WorkingUp.X * Size.X * Taper;
						Offset.Y	= WorkingUp.Y * Size.Y * Taper;
						Offset.Z	= WorkingUp.Z * Size.Z * Taper;

						// Generate the vertex
						Vertex->Position	= InterpDrawPos + Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 0.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						CheckVertexCount++;

						Vertex->Position	= InterpDrawPos - Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 1.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						CheckVertexCount++;

						fU	+= TextureIncrement;
						InterimDrawPosition	= InterpDrawPos;
					}
					LastPosition		= CurrPosition;
					LastDrawPosition	= CurrDrawPosition;
					LastTangent			= TargetTangent;
				}

				if (bLocked)
				{
					// Draw the line from the last point to the target
					CurrDrawPosition	= BeamPayloadData->TargetPoint;
					if (Source.bTargetNoise)
					{
						if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
						{
							NoiseDir		= NextNoise[Source.Frequency] - NoisePoints[Source.Frequency];
							NoiseDir.Normalize();
							CheckNoisePoint	= NoisePoints[Source.Frequency] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
							if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Source.Frequency].X) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Source.Frequency].Y) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Source.Frequency].Z) < Source.NoiseLockRadius))
							{
								NoisePoints[Source.Frequency]	= NextNoise[Source.Frequency];
							}
							else
							{
								NoisePoints[Source.Frequency]	= CheckNoisePoint;
							}
						}

						CurrDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[Source.Frequency] * NoiseDistScale);
					}

					if (Source.bUseTarget)
					{
						TargetTangent = BeamPayloadData->TargetTangent;
					}
					else
					{
						NextTargetDrawPosition	= CurrPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;
						TargetTangent = ((1.0f - Source.NoiseTension) / 2.0f) * 
							(NextTargetDrawPosition - LastDrawPosition);
					}
					TargetTangent.Normalize();
					TargetTangent *= fTargetStrength;

					// Tessellate this segment
					InterimDrawPosition = LastDrawPosition;
					for (INT TessIndex = 0; TessIndex < TessFactor; TessIndex++)
					{
						InterpDrawPos = CubicInterp(
							LastDrawPosition, LastTangent,
							CurrDrawPosition, TargetTangent,
							InvTessFactor * (TessIndex + 1));

						Location	= InterimDrawPosition;
						EndPoint	= InterpDrawPos;
						Right		= Location - EndPoint;
						Right.Normalize();
						if (Source.UpVectorStepSize == 0)
						{
							//Up = Right ^  (Location - CameraToWorld.GetOrigin());
							Up = Right ^ (Location - ViewOrigin);
							if (!Up.Normalize())
							{
								Up = CameraToWorld.GetAxis(1);
							}
						}
						else
						{
							Up = THE_Up;
						}

						if (SheetIndex)
						{
							Angle		= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
							QuatRotator	= FQuat(Right, Angle);
							WorkingUp	= QuatRotator.RotateVector(Up);
						}
						else
						{
							WorkingUp	= Up;
						}

						if (Source.TaperMethod != PEBTM_None)
						{
							check(TaperValues);
							Taper	= TaperValues[BeamPayloadData->Steps * TessFactor + TessIndex];
						}

						Offset.X	= WorkingUp.X * Size.X * Taper;
						Offset.Y	= WorkingUp.Y * Size.Y * Taper;
						Offset.Z	= WorkingUp.Z * Size.Z * Taper;

						// Generate the vertex
						Vertex->Position	= InterpDrawPos + Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 0.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						CheckVertexCount++;

						Vertex->Position	= InterpDrawPos - Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 1.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						CheckVertexCount++;

						fU	+= TextureIncrement;
						InterimDrawPosition	= InterpDrawPos;
					}
				}
			}
		}
		else
		{
			// Setup the current position as the source point
			CurrPosition		= BeamPayloadData->SourcePoint;
			CurrDrawPosition	= CurrPosition;

			// Setup the source tangent & strength
			if (Source.bUseSource)
			{
				// The source module will have determined the proper source tangent.
				LastTangent	= BeamPayloadData->SourceTangent;
				fStrength	= BeamPayloadData->SourceStrength;
			}
			else
			{
				// We don't have a source module, so use the orientation of the emitter.
				LastTangent	= WorldToLocal.GetAxis(0);
				fStrength	= Source.NoiseTangentStrength;
			}
			LastTangent.Normalize();
			LastTangent *= fStrength;

			// Setup the target tangent strength
			fTargetStrength	= Source.NoiseTangentStrength;

			// Set the last draw position to the source so we don't get 'under-hang'
			LastPosition		= CurrPosition;
			LastDrawPosition	= CurrDrawPosition;

			UBOOL	bLocked	= BEAM2_TYPEDATA_LOCKED(BeamPayloadData->Lock_Max_NumNoisePoints);

			FVector	UseNoisePoint, CheckNoisePoint;
			FVector	NoiseDir;

			for (INT SheetIndex = 0; SheetIndex < Source.Sheets; SheetIndex++)
			{
				// Reset the texture coordinate
				fU					= 0.0f;
				LastPosition		= BeamPayloadData->SourcePoint;
				LastDrawPosition	= LastPosition;

				// Determine the current position by stepping the direct line and offsetting with the noise point. 
				CurrPosition		= LastPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;

				if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
				{
					NoiseDir		= NextNoise[0] - NoisePoints[0];
					NoiseDir.Normalize();
					CheckNoisePoint	= NoisePoints[0] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
					if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[0].X) < Source.NoiseLockRadius) &&
						(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[0].Y) < Source.NoiseLockRadius) &&
						(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[0].Z) < Source.NoiseLockRadius))
					{
						NoisePoints[0]	= NextNoise[0];
					}
					else
					{
						NoisePoints[0]	= CheckNoisePoint;
					}
				}

				CurrDrawPosition	= CurrPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[0] * NoiseDistScale);

				// Determine the offset for the leading edge
				Location	= LastDrawPosition;
				EndPoint	= CurrDrawPosition;
				Right		= Location - EndPoint;
				Right.Normalize();
				if (((Source.UpVectorStepSize == 1) && (i == 0)) || (Source.UpVectorStepSize == 0))
				{
					//LastUp = Right ^ ViewDirection;
					LastUp = Right ^ (Location - ViewOrigin);
					if (!LastUp.Normalize())
					{
						LastUp = CameraToWorld.GetAxis(1);
					}
					THE_Up = LastUp;
				}
				else
				{
					LastUp == THE_Up;
				}

				if (SheetIndex)
				{
					Angle			= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
					QuatRotator		= FQuat(Right, Angle);
					WorkingLastUp	= QuatRotator.RotateVector(LastUp);
				}
				else
				{
					WorkingLastUp	= LastUp;
				}

				FLOAT	Taper	= 1.0f;

				if (Source.TaperMethod != PEBTM_None)
				{
					check(TaperValues);
					Taper	= TaperValues[0];
				}

				LastOffset.X	= WorkingLastUp.X * Size.X * Taper;
				LastOffset.Y	= WorkingLastUp.Y * Size.Y * Taper;
				LastOffset.Z	= WorkingLastUp.Z * Size.Z * Taper;

				// 'Lead' edge
				Vertex->Position	= Location + LastOffset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fU;
				Vertex->Tex_V		= 0.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				CheckVertexCount++;

				Vertex->Position	= Location - LastOffset;
				Vertex->OldPosition	= Location;
				Vertex->Size		= Size;
				Vertex->Tex_U		= fU;
				Vertex->Tex_V		= 1.0f;
				Vertex->Rotation	= Particle->Rotation;
				Vertex->Color		= Particle->Color;
				Vertex++;
				CheckVertexCount++;

				fU	+= TextureIncrement;

				for (INT StepIndex = 0; StepIndex < BeamPayloadData->Steps; StepIndex++)
				{
					// Determine the current position by stepping the direct line and offsetting with the noise point. 
					CurrPosition		= LastPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;

					if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
					{
						NoiseDir		= NextNoise[StepIndex] - NoisePoints[StepIndex];
						NoiseDir.Normalize();
						CheckNoisePoint	= NoisePoints[StepIndex] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
						if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[StepIndex].X) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[StepIndex].Y) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[StepIndex].Z) < Source.NoiseLockRadius))
						{
							NoisePoints[StepIndex]	= NextNoise[StepIndex];
						}
						else
						{
							NoisePoints[StepIndex]	= CheckNoisePoint;
						}
					}

					CurrDrawPosition	= CurrPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[StepIndex] * NoiseDistScale);

					// Prep the next draw position to determine tangents
					UBOOL bTarget = FALSE;
					NextTargetPosition	= CurrPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;
					if (bLocked && ((StepIndex + 1) == BeamPayloadData->Steps))
					{
						// If we are locked, and the next step is the target point, set the draw position as such.
						// (ie, we are on the last noise point...)
						NextTargetDrawPosition	= BeamPayloadData->TargetPoint;
						if (Source.bTargetNoise)
						{
							if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
							{
								NoiseDir		= NextNoise[Source.Frequency] - NoisePoints[Source.Frequency];
								NoiseDir.Normalize();
								CheckNoisePoint	= NoisePoints[Source.Frequency] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
								if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Source.Frequency].X) < Source.NoiseLockRadius) &&
									(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Source.Frequency].Y) < Source.NoiseLockRadius) &&
									(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Source.Frequency].Z) < Source.NoiseLockRadius))
								{
									NoisePoints[Source.Frequency]	= NextNoise[Source.Frequency];
								}
								else
								{
									NoisePoints[Source.Frequency]	= CheckNoisePoint;
								}
							}

							NextTargetDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[Source.Frequency] * NoiseDistScale);
						}
						TargetTangent = BeamPayloadData->TargetTangent;
						fTargetStrength	= BeamPayloadData->TargetStrength;
					}
					else
					{
						// Just another noise point... offset the target to get the draw position.
						if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
						{
							NoiseDir		= NextNoise[StepIndex + 1] - NoisePoints[StepIndex + 1];
							NoiseDir.Normalize();
							CheckNoisePoint	= NoisePoints[StepIndex + 1] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
							if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[StepIndex + 1].X) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[StepIndex + 1].Y) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[StepIndex + 1].Z) < Source.NoiseLockRadius))
							{
								NoisePoints[StepIndex + 1]	= NextNoise[StepIndex + 1];
							}
							else
							{
								NoisePoints[StepIndex + 1]	= CheckNoisePoint;
							}
						}

						NextTargetDrawPosition	= NextTargetPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[StepIndex + 1] * NoiseDistScale);

						TargetTangent = ((1.0f - Source.NoiseTension) / 2.0f) * (NextTargetDrawPosition - LastDrawPosition);
					}
					TargetTangent.Normalize();
					TargetTangent *= fTargetStrength;

					InterimDrawPosition = LastDrawPosition;
					// Tessellate between the current position and the last position
					for (INT TessIndex = 0; TessIndex < TessFactor; TessIndex++)
					{
						InterpDrawPos = CubicInterp(
							LastDrawPosition, LastTangent,
							CurrDrawPosition, TargetTangent,
							InvTessFactor * (TessIndex + 1));

						CONSOLE_PREFETCH(Vertex+2);

						Location	= InterimDrawPosition;
						EndPoint	= InterpDrawPos;
						Right		= Location - EndPoint;
						Right.Normalize();
						if (Source.UpVectorStepSize == 0)
						{
							//Up = Right ^  (Location - CameraToWorld.GetOrigin());
							Up = Right ^ (Location - ViewOrigin);
							if (!Up.Normalize())
							{
								Up = CameraToWorld.GetAxis(1);
							}
						}
						else
						{
							Up = THE_Up;
						}

						if (SheetIndex)
						{
							Angle		= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
							QuatRotator	= FQuat(Right, Angle);
							WorkingUp	= QuatRotator.RotateVector(Up);
						}
						else
						{
							WorkingUp	= Up;
						}

						if (Source.TaperMethod != PEBTM_None)
						{
							check(TaperValues);
							Taper	= TaperValues[StepIndex * TessFactor + TessIndex];
						}

						Offset.X	= WorkingUp.X * Size.X * Taper;
						Offset.Y	= WorkingUp.Y * Size.Y * Taper;
						Offset.Z	= WorkingUp.Z * Size.Z * Taper;

						// Generate the vertex
						Vertex->Position	= InterpDrawPos + Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 0.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						CheckVertexCount++;

						Vertex->Position	= InterpDrawPos - Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 1.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						CheckVertexCount++;

						fU	+= TextureIncrement;
						InterimDrawPosition	= InterpDrawPos;
					}
					LastPosition		= CurrPosition;
					LastDrawPosition	= CurrDrawPosition;
					LastTangent			= TargetTangent;
				}

				if (bLocked)
				{
					// Draw the line from the last point to the target
					CurrDrawPosition	= BeamPayloadData->TargetPoint;
					if (Source.bTargetNoise)
					{
						if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
						{
							NoiseDir		= NextNoise[Source.Frequency] - NoisePoints[Source.Frequency];
							NoiseDir.Normalize();
							CheckNoisePoint	= NoisePoints[Source.Frequency] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
							if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Source.Frequency].X) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Source.Frequency].Y) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Source.Frequency].Z) < Source.NoiseLockRadius))
							{
								NoisePoints[Source.Frequency]	= NextNoise[Source.Frequency];
							}
							else
							{
								NoisePoints[Source.Frequency]	= CheckNoisePoint;
							}
						}

						CurrDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[Source.Frequency] * NoiseDistScale);
					}

					if (Source.bUseTarget)
					{
						TargetTangent = BeamPayloadData->TargetTangent;
					}
					else
					{
						NextTargetDrawPosition	= CurrPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;
						TargetTangent = ((1.0f - Source.NoiseTension) / 2.0f) * 
							(NextTargetDrawPosition - LastDrawPosition);
					}
					TargetTangent.Normalize();
					TargetTangent *= fTargetStrength;

					// Tessellate this segment
					InterimDrawPosition = LastDrawPosition;
					for (INT TessIndex = 0; TessIndex < TessFactor; TessIndex++)
					{
						InterpDrawPos = CubicInterp(
							LastDrawPosition, LastTangent,
							CurrDrawPosition, TargetTangent,
							InvTessFactor * (TessIndex + 1));

						Location	= InterimDrawPosition;
						EndPoint	= InterpDrawPos;
						Right		= Location - EndPoint;
						Right.Normalize();
						if (Source.UpVectorStepSize == 0)
						{
							//Up = Right ^  (Location - CameraToWorld.GetOrigin());
							Up = Right ^ (Location - ViewOrigin);
							if (!Up.Normalize())
							{
								Up = CameraToWorld.GetAxis(1);
							}
						}
						else
						{
							Up = THE_Up;
						}

						if (SheetIndex)
						{
							Angle		= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
							QuatRotator	= FQuat(Right, Angle);
							WorkingUp	= QuatRotator.RotateVector(Up);
						}
						else
						{
							WorkingUp	= Up;
						}

						if (Source.TaperMethod != PEBTM_None)
						{
							check(TaperValues);
							Taper	= TaperValues[BeamPayloadData->Steps * TessFactor + TessIndex];
						}

						Offset.X	= WorkingUp.X * Size.X * Taper;
						Offset.Y	= WorkingUp.Y * Size.Y * Taper;
						Offset.Z	= WorkingUp.Z * Size.Z * Taper;

						// Generate the vertex
						Vertex->Position	= InterpDrawPos + Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 0.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						CheckVertexCount++;

						Vertex->Position	= InterpDrawPos - Offset;
						Vertex->OldPosition	= InterpDrawPos;
						Vertex->Size		= Size;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 1.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= Particle->Color;
						Vertex++;
						CheckVertexCount++;

						fU	+= TextureIncrement;
						InterimDrawPosition	= InterpDrawPos;
					}
				}
				else
				if (BeamPayloadData->TravelRatio > KINDA_SMALL_NUMBER)
				{
					//@todo.SAS. Re-implement partial-segment beams
				}
			}
		}
	}

	check(CheckVertexCount <= Source.VertexCount);

	return TrianglesToRender;
}

INT FDynamicBeam2EmitterData::FillData_InterpolatedNoise(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	INT	TrianglesToRender = 0;

	check(Source.InterpolationPoints > 0);
	check(Source.Frequency > 0);

	FParticleSpriteVertex* Vertex = (FParticleSpriteVertex*)VertexData;
	FMatrix CameraToWorld = View->ViewMatrix.Inverse();
	FVector ViewDirection	= View->ViewMatrix.GetAxis(0);
	
	if (Source.Sheets <= 0)
	{
		Source.Sheets = 1;
	}

	FVector	ViewOrigin	= CameraToWorld.GetOrigin();

	// Frequency is the number of noise points to generate, evenly distributed along the line.
	if (Source.Frequency <= 0)
	{
		Source.Frequency = 1;
	}

	// NoiseTessellation is the amount of tessellation that should occur between noise points.
	INT	TessFactor	= Source.NoiseTessellation ? Source.NoiseTessellation : 1;
	
	FLOAT	InvTessFactor	= 1.0f / TessFactor;
	INT		i;

	// The last position processed
	FVector	LastPosition, LastDrawPosition, LastTangent;
	// The current position
	FVector	CurrPosition, CurrDrawPosition;
	// The target
	FVector	TargetPosition, TargetDrawPosition;
	// The next target
	FVector	NextTargetPosition, NextTargetDrawPosition, TargetTangent;
	// The interperted draw position
	FVector InterpDrawPos;
	FVector	InterimDrawPosition;

	FVector	Size;

	FLOAT	Angle;
	FQuat	QuatRotator;

	FVector Location;
	FVector EndPoint;
	FVector Right;
	FVector Up;
	FVector WorkingUp;
	FVector LastUp;
	FVector WorkingLastUp;
	FVector Offset;
	FVector LastOffset;
	FLOAT	fStrength;
	FLOAT	fTargetStrength;

	FLOAT	fU;
	FLOAT	TextureIncrement	= 1.0f / (((Source.Frequency > 0) ? Source.Frequency : 1) * TessFactor);	// TTP #33140/33159

	FVector THE_Up = FVector(0.0f);

	INT	 CheckVertexCount	= 0;

	FMatrix WorldToLocal = Proxy->GetWorldToLocal();
	FMatrix LocalToWorld = Proxy->GetLocalToWorld();

	// Tessellate the beam along the noise points
	for (i = 0; i < Source.ActiveParticleCount; i++)
	{
		DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * i);

		// Retrieve the beam data from the particle.
		FBeam2TypeDataPayload*	BeamPayloadData		= NULL;
		FVector*				InterpolatedPoints	= NULL;
		FLOAT*					NoiseRate			= NULL;
		FLOAT*					NoiseDelta			= NULL;
		FVector*				TargetNoisePoints	= NULL;
		FVector*				NextNoisePoints		= NULL;
		FLOAT*					TaperValues			= NULL;
		FLOAT*					NoiseDistanceScale	= NULL;

		BeamPayloadData = (FBeam2TypeDataPayload*)((BYTE*)Particle + Source.BeamDataOffset);
		if (BeamPayloadData->TriangleCount == 0)
		{
			continue;
		}
		if (Source.InterpolatedPointsOffset != -1)
		{
			InterpolatedPoints = (FVector*)((BYTE*)Particle + Source.InterpolatedPointsOffset);
		}
		if (Source.NoiseRateOffset != -1)
		{
			NoiseRate = (FLOAT*)((BYTE*)Particle + Source.NoiseRateOffset);
		}
		if (Source.NoiseDeltaTimeOffset != -1)
		{
			NoiseDelta = (FLOAT*)((BYTE*)Particle + Source.NoiseDeltaTimeOffset);
		}
		if (Source.TargetNoisePointsOffset != -1)
		{
			TargetNoisePoints = (FVector*)((BYTE*)Particle + Source.TargetNoisePointsOffset);
		}
		if (Source.NextNoisePointsOffset != -1)
		{
			NextNoisePoints = (FVector*)((BYTE*)Particle + Source.NextNoisePointsOffset);
		}
		if (Source.TaperValuesOffset != -1)
		{
			TaperValues = (FLOAT*)((BYTE*)Particle + Source.TaperValuesOffset);
		}
		if (Source.NoiseDistanceScaleOffset != -1)
		{
			NoiseDistanceScale = (FLOAT*)((BYTE*)Particle + Source.NoiseDistanceScaleOffset);
		}

		FLOAT NoiseDistScale = 1.0f;
		if (NoiseDistanceScale)
		{
			NoiseDistScale = *NoiseDistanceScale;
		}

		INT Freq = BEAM2_TYPEDATA_FREQUENCY(BeamPayloadData->Lock_Max_NumNoisePoints);
		FLOAT InterpStepSize = (FLOAT)(BeamPayloadData->InterpolationSteps) / (FLOAT)(BeamPayloadData->Steps);
		FLOAT InterpFraction = appFractional(InterpStepSize);
		//UBOOL bInterpFractionIsZero = (Abs(InterpFraction) < KINDA_SMALL_NUMBER) ? TRUE : FALSE;
		UBOOL bInterpFractionIsZero = FALSE;
		INT InterpIndex = appTrunc(InterpStepSize);

		FVector* NoisePoints	= TargetNoisePoints;
		FVector* NextNoise		= NextNoisePoints;

		FLOAT NoiseRangeScaleFactor = Source.NoiseRangeScale;
		//@todo. How to handle no noise points?
		// If there are no noise points, why are we in here?
		if (NoisePoints == NULL)
		{
			continue;
		}

		// Pin the size to the X component
		Size	= FVector(Particle->Size.X * Source.Scale.X);

		// Setup the current position as the source point
		CurrPosition		= BeamPayloadData->SourcePoint;
		CurrDrawPosition	= CurrPosition;

		// Setup the source tangent & strength
		if (Source.bUseSource)
		{
			// The source module will have determined the proper source tangent.
			LastTangent	= BeamPayloadData->SourceTangent;
			fStrength	= Source.NoiseTangentStrength;
		}
		else
		{
			// We don't have a source module, so use the orientation of the emitter.
			LastTangent	= WorldToLocal.GetAxis(0);
			fStrength	= Source.NoiseTangentStrength;
		}
		LastTangent *= fStrength;

		// Setup the target tangent strength
		fTargetStrength	= Source.NoiseTangentStrength;

		// Set the last draw position to the source so we don't get 'under-hang'
		LastPosition		= CurrPosition;
		LastDrawPosition	= CurrDrawPosition;

		UBOOL	bLocked	= BEAM2_TYPEDATA_LOCKED(BeamPayloadData->Lock_Max_NumNoisePoints);

		FVector	UseNoisePoint, CheckNoisePoint;
		FVector	NoiseDir;

		for (INT SheetIndex = 0; SheetIndex < Source.Sheets; SheetIndex++)
		{
			// Reset the texture coordinate
			fU					= 0.0f;
			LastPosition		= BeamPayloadData->SourcePoint;
			LastDrawPosition	= LastPosition;

			// Determine the current position by finding it along the interpolated path and 
			// offsetting with the noise point. 
			if (bInterpFractionIsZero)
			{
				CurrPosition = InterpolatedPoints[InterpIndex];
			}
			else
			{
				CurrPosition = 
					(InterpolatedPoints[InterpIndex + 0] * InterpFraction) + 
					(InterpolatedPoints[InterpIndex + 1] * (1.0f - InterpFraction));
			}

			if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
			{
				NoiseDir		= NextNoise[0] - NoisePoints[0];
				NoiseDir.Normalize();
				CheckNoisePoint	= NoisePoints[0] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
				if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[0].X) < Source.NoiseLockRadius) &&
					(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[0].Y) < Source.NoiseLockRadius) &&
					(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[0].Z) < Source.NoiseLockRadius))
				{
					NoisePoints[0]	= NextNoise[0];
				}
				else
				{
					NoisePoints[0]	= CheckNoisePoint;
				}
			}

			CurrDrawPosition	= CurrPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[0] * NoiseDistScale);

			// Determine the offset for the leading edge
			Location	= LastDrawPosition;
			EndPoint	= CurrDrawPosition;
			Right		= Location - EndPoint;
			Right.Normalize();
			if (((Source.UpVectorStepSize == 1) && (i == 0)) || (Source.UpVectorStepSize == 0))
			{
				//LastUp = Right ^ ViewDirection;
				LastUp = Right ^ (Location - ViewOrigin);
				if (!LastUp.Normalize())
				{
					LastUp = CameraToWorld.GetAxis(1);
				}
				THE_Up = LastUp;
			}
			else
			{
				LastUp = THE_Up;
			}

			if (SheetIndex)
			{
				Angle			= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
				QuatRotator		= FQuat(Right, Angle);
				WorkingLastUp	= QuatRotator.RotateVector(LastUp);
			}
			else
			{
				WorkingLastUp	= LastUp;
			}

			FLOAT	Taper	= 1.0f;

			if (Source.TaperMethod != PEBTM_None)
			{
				check(TaperValues);
				Taper	= TaperValues[0];
			}

			LastOffset.X	= WorkingLastUp.X * Size.X * Taper;
			LastOffset.Y	= WorkingLastUp.Y * Size.Y * Taper;
			LastOffset.Z	= WorkingLastUp.Z * Size.Z * Taper;

			// 'Lead' edge
			Vertex->Position	= Location + LastOffset;
			Vertex->OldPosition	= Location;
			Vertex->Size		= Size;
			Vertex->Tex_U		= fU;
			Vertex->Tex_V		= 0.0f;
			Vertex->Rotation	= Particle->Rotation;
			Vertex->Color		= Particle->Color;
			Vertex++;
			CheckVertexCount++;

			Vertex->Position	= Location - LastOffset;
			Vertex->OldPosition	= Location;
			Vertex->Size		= Size;
			Vertex->Tex_U		= fU;
			Vertex->Tex_V		= 1.0f;
			Vertex->Rotation	= Particle->Rotation;
			Vertex->Color		= Particle->Color;
			Vertex++;
			CheckVertexCount++;

			fU	+= TextureIncrement;

			check(InterpolatedPoints);
			for (INT StepIndex = 0; StepIndex < BeamPayloadData->Steps; StepIndex++)
			{
				// Determine the current position by finding it along the interpolated path and 
				// offsetting with the noise point. 
				if (bInterpFractionIsZero)
				{
					CurrPosition = InterpolatedPoints[StepIndex  * InterpIndex];
				}
				else
				{
					if (StepIndex == (BeamPayloadData->Steps - 1))
					{
						CurrPosition = 
							(InterpolatedPoints[StepIndex * InterpIndex] * (1.0f - InterpFraction)) + 
							(BeamPayloadData->TargetPoint * InterpFraction);
					}
					else
					{
						CurrPosition = 
							(InterpolatedPoints[StepIndex * InterpIndex + 0] * (1.0f - InterpFraction)) + 
							(InterpolatedPoints[StepIndex * InterpIndex + 1] * InterpFraction);
					}
				}


				if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
				{
					NoiseDir		= NextNoise[StepIndex] - NoisePoints[StepIndex];
					NoiseDir.Normalize();
					CheckNoisePoint	= NoisePoints[StepIndex] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
					if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[StepIndex].X) < Source.NoiseLockRadius) &&
						(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[StepIndex].Y) < Source.NoiseLockRadius) &&
						(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[StepIndex].Z) < Source.NoiseLockRadius))
					{
						NoisePoints[StepIndex]	= NextNoise[StepIndex];
					}
					else
					{
						NoisePoints[StepIndex]	= CheckNoisePoint;
					}
				}

				CurrDrawPosition	= CurrPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[StepIndex] * NoiseDistScale);

				// Prep the next draw position to determine tangents
				UBOOL bTarget = FALSE;
				NextTargetPosition	= CurrPosition + BeamPayloadData->Direction * BeamPayloadData->StepSize;
				// Determine the current position by finding it along the interpolated path and 
				// offsetting with the noise point. 
				if (bInterpFractionIsZero)
				{
					if (StepIndex == (BeamPayloadData->Steps - 2))
					{
						NextTargetPosition = BeamPayloadData->TargetPoint;
					}
					else
					{
						NextTargetPosition = InterpolatedPoints[(StepIndex + 2) * InterpIndex + 0];
					}
				}
				else
				{
					if (StepIndex == (BeamPayloadData->Steps - 1))
					{
						NextTargetPosition = 
							(InterpolatedPoints[(StepIndex + 1) * InterpIndex + 0] * InterpFraction) + 
							(BeamPayloadData->TargetPoint * (1.0f - InterpFraction));
					}
					else
					{
						NextTargetPosition = 
							(InterpolatedPoints[(StepIndex + 1) * InterpIndex + 0] * InterpFraction) + 
							(InterpolatedPoints[(StepIndex + 1) * InterpIndex + 1] * (1.0f - InterpFraction));
					}
				}
				if (bLocked && ((StepIndex + 1) == BeamPayloadData->Steps))
				{
					// If we are locked, and the next step is the target point, set the draw position as such.
					// (ie, we are on the last noise point...)
					NextTargetDrawPosition	= BeamPayloadData->TargetPoint;
					if (Source.bTargetNoise)
					{
						if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
						{
							NoiseDir		= NextNoise[Source.Frequency] - NoisePoints[Source.Frequency];
							NoiseDir.Normalize();
							CheckNoisePoint	= NoisePoints[Source.Frequency] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
							if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Source.Frequency].X) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Source.Frequency].Y) < Source.NoiseLockRadius) &&
								(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Source.Frequency].Z) < Source.NoiseLockRadius))
							{
								NoisePoints[Source.Frequency]	= NextNoise[Source.Frequency];
							}
							else
							{
								NoisePoints[Source.Frequency]	= CheckNoisePoint;
							}
						}

						NextTargetDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[Source.Frequency] * NoiseDistScale);
					}
					TargetTangent = BeamPayloadData->TargetTangent;
					fTargetStrength	= Source.NoiseTangentStrength;
				}
				else
				{
					// Just another noise point... offset the target to get the draw position.
					if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
					{
						NoiseDir		= NextNoise[StepIndex + 1] - NoisePoints[StepIndex + 1];
						NoiseDir.Normalize();
						CheckNoisePoint	= NoisePoints[StepIndex + 1] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
						if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[StepIndex + 1].X) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[StepIndex + 1].Y) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[StepIndex + 1].Z) < Source.NoiseLockRadius))
						{
							NoisePoints[StepIndex + 1]	= NextNoise[StepIndex + 1];
						}
						else
						{
							NoisePoints[StepIndex + 1]	= CheckNoisePoint;
						}
					}

					NextTargetDrawPosition	= NextTargetPosition + NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[StepIndex + 1] * NoiseDistScale);

					TargetTangent = ((1.0f - Source.NoiseTension) / 2.0f) * (NextTargetDrawPosition - LastDrawPosition);
				}
				TargetTangent = ((1.0f - Source.NoiseTension) / 2.0f) * (NextTargetDrawPosition - LastDrawPosition);
				TargetTangent.Normalize();
				TargetTangent *= fTargetStrength;

				InterimDrawPosition = LastDrawPosition;
				// Tessellate between the current position and the last position
				for (INT TessIndex = 0; TessIndex < TessFactor; TessIndex++)
				{
					InterpDrawPos = CubicInterp(
						LastDrawPosition, LastTangent,
						CurrDrawPosition, TargetTangent,
						InvTessFactor * (TessIndex + 1));

					Location	= InterimDrawPosition;
					EndPoint	= InterpDrawPos;
					Right		= Location - EndPoint;
					Right.Normalize();
					if (Source.UpVectorStepSize == 0)
					{
						//Up = Right ^  (Location - CameraToWorld.GetOrigin());
						Up = Right ^ (Location - ViewOrigin);
						if (!Up.Normalize())
						{
							Up = CameraToWorld.GetAxis(1);
						}
					}
					else
					{
						Up = THE_Up;
					}

					if (SheetIndex)
					{
						Angle		= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
						QuatRotator	= FQuat(Right, Angle);
						WorkingUp	= QuatRotator.RotateVector(Up);
					}
					else
					{
						WorkingUp	= Up;
					}

					if (Source.TaperMethod != PEBTM_None)
					{
						check(TaperValues);
						Taper	= TaperValues[StepIndex * TessFactor + TessIndex];
					}

					Offset.X	= WorkingUp.X * Size.X * Taper;
					Offset.Y	= WorkingUp.Y * Size.Y * Taper;
					Offset.Z	= WorkingUp.Z * Size.Z * Taper;

					// Generate the vertex
					Vertex->Position	= InterpDrawPos + Offset;
					Vertex->OldPosition	= InterpDrawPos;
					Vertex->Size		= Size;
					Vertex->Tex_U		= fU;
					Vertex->Tex_V		= 0.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= Particle->Color;
					Vertex++;
					CheckVertexCount++;

					Vertex->Position	= InterpDrawPos - Offset;
					Vertex->OldPosition	= InterpDrawPos;
					Vertex->Size		= Size;
					Vertex->Tex_U		= fU;
					Vertex->Tex_V		= 1.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= Particle->Color;
					Vertex++;
					CheckVertexCount++;

					fU	+= TextureIncrement;
					InterimDrawPosition	= InterpDrawPos;
				}
				LastPosition		= CurrPosition;
				LastDrawPosition	= CurrDrawPosition;
				LastTangent			= TargetTangent;
			}

			if (bLocked)
			{
				// Draw the line from the last point to the target
				CurrDrawPosition	= BeamPayloadData->TargetPoint;
				if (Source.bTargetNoise)
				{
					if ((Source.NoiseLockTime >= 0.0f) && Source.bSmoothNoise_Enabled)
					{
						NoiseDir		= NextNoise[Source.Frequency] - NoisePoints[Source.Frequency];
						NoiseDir.Normalize();
						CheckNoisePoint	= NoisePoints[Source.Frequency] + NoiseDir * Source.NoiseSpeed * *NoiseRate;
						if ((Abs<FLOAT>(CheckNoisePoint.X - NextNoise[Source.Frequency].X) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Y - NextNoise[Source.Frequency].Y) < Source.NoiseLockRadius) &&
							(Abs<FLOAT>(CheckNoisePoint.Z - NextNoise[Source.Frequency].Z) < Source.NoiseLockRadius))
						{
							NoisePoints[Source.Frequency]	= NextNoise[Source.Frequency];
						}
						else
						{
							NoisePoints[Source.Frequency]	= CheckNoisePoint;
						}
					}

					CurrDrawPosition += NoiseRangeScaleFactor * LocalToWorld.TransformNormal(NoisePoints[Source.Frequency] * NoiseDistScale);
				}

				NextTargetDrawPosition	= BeamPayloadData->TargetPoint;
				if (Source.bUseTarget)
				{
					TargetTangent = BeamPayloadData->TargetTangent;
				}
				else
				{
					TargetTangent = ((1.0f - Source.NoiseTension) / 2.0f) * 
						(NextTargetDrawPosition - LastDrawPosition);
					TargetTangent.Normalize();
				}
				TargetTangent *= fTargetStrength;

				// Tessellate this segment
				InterimDrawPosition = LastDrawPosition;
				for (INT TessIndex = 0; TessIndex < TessFactor; TessIndex++)
				{
					InterpDrawPos = CubicInterp(
						LastDrawPosition, LastTangent,
						CurrDrawPosition, TargetTangent,
						InvTessFactor * (TessIndex + 1));

					Location	= InterimDrawPosition;
					EndPoint	= InterpDrawPos;
					Right		= Location - EndPoint;
					Right.Normalize();
					if (Source.UpVectorStepSize == 0)
					{
						//Up = Right ^  (Location - CameraToWorld.GetOrigin());
						Up = Right ^ (Location - ViewOrigin);
						if (!Up.Normalize())
						{
							Up = CameraToWorld.GetAxis(1);
						}
					}
					else
					{
						Up = THE_Up;
					}

					if (SheetIndex)
					{
						Angle		= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
						QuatRotator	= FQuat(Right, Angle);
						WorkingUp	= QuatRotator.RotateVector(Up);
					}
					else
					{
						WorkingUp	= Up;
					}

					if (Source.TaperMethod != PEBTM_None)
					{
						check(TaperValues);
						Taper	= TaperValues[BeamPayloadData->Steps * TessFactor + TessIndex];
					}

					Offset.X	= WorkingUp.X * Size.X * Taper;
					Offset.Y	= WorkingUp.Y * Size.Y * Taper;
					Offset.Z	= WorkingUp.Z * Size.Z * Taper;

					// Generate the vertex
					Vertex->Position	= InterpDrawPos + Offset;
					Vertex->OldPosition	= InterpDrawPos;
					Vertex->Size		= Size;
					Vertex->Tex_U		= fU;
					Vertex->Tex_V		= 0.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= Particle->Color;
					Vertex++;
					CheckVertexCount++;

					Vertex->Position	= InterpDrawPos - Offset;
					Vertex->OldPosition	= InterpDrawPos;
					Vertex->Size		= Size;
					Vertex->Tex_U		= fU;
					Vertex->Tex_V		= 1.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= Particle->Color;
					Vertex++;
					CheckVertexCount++;

					fU	+= TextureIncrement;
					InterimDrawPosition	= InterpDrawPos;
				}
			}
			else
			if (BeamPayloadData->TravelRatio > KINDA_SMALL_NUMBER)
			{
				//@todo.SAS. Re-implement partial-segment beams
			}
		}
	}

	check(CheckVertexCount <= Source.VertexCount);

	return TrianglesToRender;
}

///////////////////////////////////////////////////////////////////////////////
//	FDynamicTrail2EmitterData
///////////////////////////////////////////////////////////////////////////////


/** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
void FDynamicTrail2EmitterData::Init( UBOOL bInSelected )
{
	bSelected = bInSelected;

	check(Source.ActiveParticleCount < (16 * 1024));	// TTP #33330
	check(Source.ParticleStride < (2 * 1024));	// TTP #33330

	bUsesDynamicParameter = FALSE;
	if( Source.MaterialInterface->GetMaterialResource() != NULL )
	{
		bUsesDynamicParameter =
			Source.MaterialInterface->GetMaterialResource()->GetUsesDynamicParameter();
	}

	MaterialResource =
		Source.MaterialInterface->GetRenderProxy( bSelected );

	// We won't need this on the render thread
	Source.MaterialInterface = NULL;
}


void FDynamicTrail2EmitterData::Render(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_TrailRenderingTime);
	INC_DWORD_STAT(STAT_TrailParticlesRenderCalls);

	if (bValid == FALSE)
	{
		return;
	}

	check(PDI);
	if ((Source.VertexCount <= 0) || (Source.ActiveParticleCount <= 0) || (Source.IndexCount < 3))
	{
		return;
	}

	// Don't render if the material will be ignored
	if (PDI->IsMaterialIgnored(MaterialResource) && (!(View->Family->ShowFlags & SHOW_Wireframe)))
	{
		return;
	}

	if (!(GEngine->GameViewport && (GEngine->GameViewport->GetCurrentSplitscreenType() == eSST_NONE)))
	{
		VertexFactory->SetScreenAlignment(Source.ScreenAlignment);

		// Beams/trails do not support LockAxis
		VertexFactory->SetLockAxesFlag(EPAL_NONE);

		// Allocate and generate the data...
		// Determine the required particle count
		INT	TrianglesToRender	= 0;

		if ((VertexData == NULL) || (VertexCount < Source.VertexCount))
		{
			VertexData = (FParticleSpriteVertex*)appRealloc(VertexData, Source.VertexCount * sizeof(FParticleSpriteVertex));
			VertexCount = Source.VertexCount;
		}
		check(VertexData);

		TriCountIndex = FillIndexData(Proxy, PDI, View, DPGIndex);
		INT TriCountVertex = FillVertexData(Proxy, PDI, View, DPGIndex);
	}

	if (TriCountIndex == 0)
	{
		return;
	}

	FMeshElement Mesh;

	Mesh.IndexBuffer			= NULL;
	Mesh.VertexFactory			= VertexFactory;
	Mesh.DynamicVertexData		= VertexData;
	Mesh.DynamicVertexStride	= sizeof(FParticleSpriteVertex);
	Mesh.DynamicIndexData		= IndexData;
	Mesh.DynamicIndexStride		= Source.IndexStride;
	Mesh.LCI					= NULL;
	if (Source.bUseLocalSpace == TRUE)
	{
		Mesh.LocalToWorld = Proxy->GetLocalToWorld();
		Mesh.WorldToLocal = Proxy->GetWorldToLocal();
	}
	else
	{
		Mesh.LocalToWorld = FMatrix::Identity;
		Mesh.WorldToLocal = FMatrix::Identity;
	}
	Mesh.FirstIndex				= 0;
	Mesh.NumPrimitives			= TriCountIndex;
	Mesh.MinVertexIndex			= 0;
	Mesh.MaxVertexIndex			= Source.VertexCount - 1;
	Mesh.UseDynamicData			= TRUE;
	Mesh.ReverseCulling			= Proxy->GetLocalToWorldDeterminant() < 0.0f ? TRUE : FALSE;
	Mesh.CastShadow				= Proxy->GetCastShadow();
	Mesh.DepthPriorityGroup		= (ESceneDepthPriorityGroup)DPGIndex;

	if ((View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials))
	{
		Mesh.MaterialRenderProxy = Proxy->GetDeselectedWireframeMatInst();
	}
	else
	{
		check(TriCountIndex == Source.PrimitiveCount);
		Mesh.MaterialRenderProxy = MaterialResource;
	}
	Mesh.Type = PT_TriangleStrip;

	DrawRichMesh(
		PDI,
		Mesh,
		FLinearColor(1.0f, 0.0f, 0.0f),
		FLinearColor(1.0f, 1.0f, 0.0f),
		FLinearColor(1.0f, 1.0f, 1.0f),
		Proxy->GetPrimitiveSceneInfo(),
		Proxy->GetSelected()
		);

	INC_DWORD_STAT_BY(STAT_TrailParticlesTrianglesRendered, Mesh.NumPrimitives);
}

/**
 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
 *  Only called for primitives that are visible and have bDynamicRelevance
 *
 *	@param	Proxy			The 'owner' particle system scene proxy
 *	@param	ViewFamily		The ViewFamily to pre-render for
 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
 *	@param	FrameNumber		The frame number of this pre-render
 */
void FDynamicTrail2EmitterData::PreRenderView(FParticleSystemSceneProxy* Proxy, const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber)
{
	if (bValid == FALSE)
	{
		return;
	}

	UBOOL bIsSingleViewGame = (GEngine && GEngine->GameViewport && (GEngine->GameViewport->GetCurrentSplitscreenType() == eSST_NONE)) ? TRUE : FALSE;
	if (bIsSingleViewGame)
	{
		// Only need to do this once per-view
		if (LastFramePreRendered < FrameNumber)
		{
			// Find the first view w/ the visiblity flag set...
			INT ValidView = -1;

			for (INT ViewIndex = 0; ViewIndex < ViewFamily->Views.Num(); ViewIndex++)
			{
				if (VisibilityMap(ViewIndex) == TRUE)
				{
					ValidView = ViewIndex;
					break;
				}
			}

			if (ValidView == -1)
			{
				return;
			}

			const FSceneView* View = ViewFamily->Views(ValidView);

			VertexFactory->SetScreenAlignment(Source.ScreenAlignment);
			VertexFactory->SetLockAxesFlag(EPAL_NONE);

			// Allocate and generate the data...
			// Determine the required particle count
			if ((VertexData == NULL) || (VertexCount < Source.VertexCount))
			{
				VertexData = (FParticleSpriteVertex*)appRealloc(VertexData, Source.VertexCount * sizeof(FParticleSpriteVertex));
				VertexCount = Source.VertexCount;
			}
			check(VertexData);

			// Fill in the index and vertex data
			TriCountIndex = FillIndexData(Proxy, NULL, View, -1);
			FillVertexData(Proxy, NULL, View, -1);

			// Set the frame tracker
			LastFramePreRendered = FrameNumber;
		}
	}
}

void FDynamicTrail2EmitterData::RenderDebug(FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex, UBOOL bCrosses)
{
}

INT FDynamicTrail2EmitterData::FillIndexData(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_TrailFillIndexTime);

	INT	TrianglesToRender = 0;

	// Trail2 polygons are packed and joined as follows:
	//
	// 1--3--5--7--9-...
	// |\ |\ |\ |\ |\...
	// | \| \| \| \| ...
	// 0--2--4--6--8-...
	//
	// (ie, the 'leading' edge of polygon (n) is the trailing edge of polygon (n+1)
	//
	// NOTE: This is primed for moving to tri-strips...
	//

	INT	Sheets = 1;
	Source.TessFactor = Max<INT>(Source.TessFactor, 1);

	FMatrix LocalToWorld = Proxy->GetLocalToWorld();

	UBOOL bWireframe = ((View->Family->ShowFlags & SHOW_Wireframe) && !(View->Family->ShowFlags & SHOW_Materials));

	if ((IndexData == NULL) || (IndexCount < Source.IndexCount))
	{
		if ((UINT)Source.IndexCount > 65535)
		{
			FString TemplateName = TEXT("*** UNKNOWN PSYS ***");
			UParticleSystemComponent* PSysComp = Cast<UParticleSystemComponent>(Proxy->GetPrimitiveSceneInfo()->Component);
			if (PSysComp)
			{
				if (PSysComp->Template)
				{
					TemplateName = PSysComp->Template->GetName();
				}
			}

			FString ErrorOut = FString::Printf(
				TEXT("*** PLEASE SUBMIT IMMEDIATELY ***%s")
				TEXT("Trail Index Error			- %s%s")
				TEXT("\tPosition				- %s%s")
				TEXT("\tPrimitiveCount			- %d%s")
				TEXT("\tVertexCount				- %d%s")
				TEXT("\tVertexData				- 0x%08x%s"),
				LINE_TERMINATOR,
				*TemplateName, LINE_TERMINATOR,
				*LocalToWorld.GetOrigin().ToString(), LINE_TERMINATOR,
				Source.PrimitiveCount, LINE_TERMINATOR,
				Source.VertexCount, LINE_TERMINATOR,
				VertexData, LINE_TERMINATOR
				);
			ErrorOut += FString::Printf(
				TEXT("\tIndexCount				- %d%s")
				TEXT("\tIndexStride				- %d%s")
				TEXT("\tIndexData				- 0x%08x%s")
				TEXT("\tVertexFactory			- 0x%08x%s"),
				Source.IndexCount, LINE_TERMINATOR,
				Source.IndexStride, LINE_TERMINATOR,
				IndexData, LINE_TERMINATOR,
				VertexFactory, LINE_TERMINATOR
				);
			ErrorOut += FString::Printf(
				TEXT("\tTrailDataOffset			- %d%s")
				TEXT("\tTaperValuesOffset		- %d%s")
				TEXT("\tParticleSourceOffset	- %d%s")
				TEXT("\tTrailCount				- %d%s"),
				Source.TrailDataOffset, LINE_TERMINATOR,
				Source.TaperValuesOffset, LINE_TERMINATOR,
				Source.ParticleSourceOffset, LINE_TERMINATOR,
				Source.TrailCount, LINE_TERMINATOR
				);
			ErrorOut += FString::Printf(
				TEXT("\tSheets					- %d%s")
				TEXT("\tTessFactor				- %d%s")
				TEXT("\tTessStrength			- %d%s")
				TEXT("\tTessFactorDistance		- %f%s")
				TEXT("\tActiveParticleCount		- %d%s"),
				Source.Sheets, LINE_TERMINATOR,
				Source.TessFactor, LINE_TERMINATOR,
				Source.TessStrength, LINE_TERMINATOR,
				Source.TessFactorDistance, LINE_TERMINATOR,
				Source.ActiveParticleCount, LINE_TERMINATOR
				);

			appErrorf(*ErrorOut);
		}
		IndexData = appRealloc(IndexData, Source.IndexCount * Source.IndexStride);
		check(IndexData);
		IndexCount = Source.IndexCount;
	}

	INT	CheckCount	= 0;

	WORD*	Index		= (WORD*)IndexData;
	WORD	VertexIndex	= 0;

	for (INT Trail = 0; Trail < Source.ActiveParticleCount; Trail++)
	{
		DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * Source.ParticleIndices(Trail));

		INT	CurrentOffset = Source.TrailDataOffset;

		FTrail2TypeDataPayload* TrailPayload = (FTrail2TypeDataPayload*)((BYTE*)Particle + CurrentOffset);
		CurrentOffset += sizeof(FTrail2TypeDataPayload);
		if (TRAIL_EMITTER_IS_START(TrailPayload->Flags) == FALSE)
		{
			continue;
		}

		INT LocalTrianglesToRender = TrailPayload->TriangleCount;
		if (LocalTrianglesToRender == 0)
		{
			continue;
		}

		FLOAT* TaperValues = (FLOAT*)((BYTE*)Particle + CurrentOffset);
		CurrentOffset += sizeof(FLOAT);

		for (INT SheetIndex = 0; SheetIndex < Sheets; SheetIndex++)
		{
			// 2 triangles per tessellation factor
			if (SheetIndex == 0)
			{
				// Only need the starting two for the first sheet
				*(Index++) = VertexIndex++;	// SheetIndex + 0
				*(Index++) = VertexIndex++;	// SheetIndex + 1

				CheckCount += 2;
			}

			// Sequentially step through each triangle - 1 vertex per triangle
			for (INT i = 0; i < LocalTrianglesToRender; i++)
			{
				*(Index++) = VertexIndex++;
				CheckCount++;
				TrianglesToRender++;
			}

			// Degenerate tris
			if ((SheetIndex + 1) < Sheets)
			{
				*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
				*(Index++) = VertexIndex;		// First vertex of the next sheet
				*(Index++) = VertexIndex++;		// First vertex of the next sheet
				*(Index++) = VertexIndex++;		// Second vertex of the next sheet
				TrianglesToRender += 4;
				CheckCount += 4;
			}
		}

		if ((Trail + 1) < Source.TrailCount)
		{
			*(Index++) = VertexIndex - 1;	// Last vertex of the previous sheet
			*(Index++) = VertexIndex;		// First vertex of the next sheet
			*(Index++) = VertexIndex++;		// First vertex of the next sheet
			*(Index++) = VertexIndex++;		// Second vertex of the next sheet
			TrianglesToRender += 4;
			CheckCount += 4;
		}
	}

	return TrianglesToRender;
}

INT FDynamicTrail2EmitterData::FillVertexData(FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DPGIndex)
{
	SCOPE_CYCLE_COUNTER(STAT_TrailFillVertexTime);
	check(Proxy);

	INT	TrianglesToRender = 0;
	FParticleSpriteVertex* Vertex = (FParticleSpriteVertex*)(VertexData);
	FMatrix CameraToWorld = View->ViewMatrix.Inverse();

	Source.TessFactor = Max<INT>(Source.TessFactor, 1);
	Source.Sheets = Max<INT>(Source.Sheets, 1);

	FLOAT	InvTessFactor	= 1.0f / (FLOAT)Source.TessFactor;
	FVector	InterpDrawPos;

	FVector	ViewOrigin	= CameraToWorld.GetOrigin();

	FVector	Offset, LastOffset;
	FLOAT	TextureIncrement;
	FLOAT	fU;
	FLOAT	Angle;
	FQuat	QuatRotator(0, 0, 0, 0);
	FVector	CurrPosition, CurrTangent;
	FLOAT CurrSize;
	FVector EndPoint, Location, Right;
	FVector Up, WorkingUp, NextUp, WorkingNextUp;
	FVector	NextPosition, NextTangent;
	FLOAT NextSize;
	FVector	TempDrawPos;
	FLinearColor CurrLinearColor, NextLinearColor, InterpLinearColor;

	FVector	TessDistCheck;
	INT		SegmentTessFactor;
#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
	FLOAT	TessRatio;
#endif	//#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)

	FMatrix LocalToWorld = Proxy->GetLocalToWorld();

	INT		PackedVertexCount	= 0;
	for (INT i = 0; i < Source.ActiveParticleCount; i++)
	{
		DECLARE_PARTICLE_PTR(Particle, Source.ParticleData.GetData() + Source.ParticleStride * Source.ParticleIndices(i));

		INT	CurrentOffset = Source.TrailDataOffset;

		FTrail2TypeDataPayload* TrailPayload = (FTrail2TypeDataPayload*)((BYTE*)Particle + CurrentOffset);
		CurrentOffset += sizeof(FTrail2TypeDataPayload);

		if (TRAIL_EMITTER_IS_START(TrailPayload->Flags))
		{
			FLOAT* TaperValues = (FLOAT*)((BYTE*)Particle + CurrentOffset);
			CurrentOffset += sizeof(FLOAT);

			// Pin the size to the X component
			CurrSize	= Particle->Size.X * Source.Scale.X;
			CurrLinearColor	= Particle->Color;

			//@todo. This will only work for a single trail!
			TextureIncrement	= 1.0f / (Source.TessFactor * Source.ActiveParticleCount + 1);
			UBOOL	bFirstInSheet	= TRUE;
			for (INT SheetIndex = 0; SheetIndex < Source.Sheets; SheetIndex++)
			{
				if (SheetIndex)
				{
					Angle		= ((FLOAT)PI / (FLOAT)Source.Sheets) * SheetIndex;
					QuatRotator	= FQuat(Right, Angle);
				}

				fU	= 0.0f;

				// Set the current position to the source...
				/***
				if (TrailSource)
				{
				//					TrailSource->ResolveSourcePoint(Owner, *Particle, *TrailData, CurrPosition, CurrTangent);
				}
				else
				***/
				{
					FVector	Dir = LocalToWorld.GetAxis(0);
					Dir.Normalize();
					CurrTangent	=  Dir * Source.TessStrength;
				}

				CurrPosition	= Source.SourcePosition(TrailPayload->TrailIndex);

				NextPosition	= Particle->Location;
				NextSize		= Particle->Size.X * Source.Scale.X;
				NextTangent		= TrailPayload->Tangent * Source.TessStrength;
				NextLinearColor	= Particle->Color;
				TempDrawPos		= CurrPosition;

				SegmentTessFactor	= Source.TessFactor;
#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
				if (TrailTypeData->TessellationFactorDistance > KINDA_SMALL_NUMBER)
				{
					TessDistCheck		= (CurrPosition - NextPosition);
					TessRatio			= TessDistCheck.Size() / Source.TessFactorDistance;
					if (TessRatio <= 0.0f)
					{
						SegmentTessFactor	= 1;
					}
					else
					if (TessRatio < 1.0f)
					{
						SegmentTessFactor	= appTrunc((Source.TessFactor + 1) * TessRatio);
					}
				}
#endif	//#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
				// Tessellate the current to next...
#if !defined(_TRAIL2_TESSELLATE_TO_SOURCE_)
				SegmentTessFactor = 1;
#endif	//#if !defined(_TRAIL2_TESSELLATE_TO_SOURCE_)
				InvTessFactor	= 1.0f / SegmentTessFactor;

				for (INT TessIndex = 0; TessIndex < SegmentTessFactor; TessIndex++)
				{
					InterpDrawPos = CubicInterp(
						CurrPosition, CurrTangent,
						NextPosition, NextTangent,
						InvTessFactor * (TessIndex + 1));
					InterpLinearColor = Lerp<FLinearColor>(
						CurrLinearColor, NextLinearColor, InvTessFactor * (TessIndex + 1));

					EndPoint	= InterpDrawPos;
					Location	= TempDrawPos;
					Right		= Location - EndPoint;
					Right.Normalize();

					if (bFirstInSheet)
					{
						Up	= Right ^  (Location - ViewOrigin);
						if (!Up.Normalize())
						{
							Up = CameraToWorld.GetAxis(1);
						}
						if (SheetIndex)
						{
							WorkingUp	= QuatRotator.RotateVector(Up);
						}
						else
						{
							WorkingUp	= Up;
						}

						if (WorkingUp.IsNearlyZero())
						{
							WorkingUp	= CameraToWorld.GetAxis(2);
							WorkingUp.Normalize();
						}

						// Setup the lead verts
						Vertex->Position	= Location + WorkingUp * CurrSize;
						Vertex->OldPosition	= Location;
						Vertex->Size.X		= CurrSize;
						Vertex->Size.Y		= CurrSize;
						Vertex->Size.Z		= CurrSize;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 0.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= CurrLinearColor;
						Vertex++;
						PackedVertexCount++;

						Vertex->Position	= Location - WorkingUp * CurrSize;
						Vertex->OldPosition	= Location;
						Vertex->Size.X		= CurrSize;
						Vertex->Size.Y		= CurrSize;
						Vertex->Size.Z		= CurrSize;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 1.0f;
						Vertex->Rotation	= Particle->Rotation;
						Vertex->Color		= CurrLinearColor;
						Vertex++;
						PackedVertexCount++;

						fU	+= TextureIncrement;
						bFirstInSheet	= FALSE;
					}

					// Setup the next verts
					NextUp	= Right ^  (EndPoint - ViewOrigin);
					if (!NextUp.Normalize())
					{
						NextUp = CameraToWorld.GetAxis(1);
					}
					if (SheetIndex)
					{
						WorkingNextUp	= QuatRotator.RotateVector(NextUp);
					}
					else
					{
						WorkingNextUp	= NextUp;
					}

					if (WorkingNextUp.IsNearlyZero())
					{
						WorkingNextUp	= CameraToWorld.GetAxis(2);
						WorkingNextUp.Normalize();
					}
					Vertex->Position	= EndPoint + WorkingNextUp * NextSize;
					Vertex->OldPosition	= EndPoint;
					Vertex->Size.X		= NextSize;
					Vertex->Size.Y		= NextSize;
					Vertex->Size.Z		= NextSize;
					Vertex->Tex_U		= fU;
					Vertex->Tex_V		= 0.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= InterpLinearColor;
					Vertex++;
					PackedVertexCount++;

					Vertex->Position	= EndPoint - WorkingNextUp * NextSize;
					Vertex->OldPosition	= EndPoint;
					Vertex->Size.X		= NextSize;
					Vertex->Size.Y		= NextSize;
					Vertex->Size.Z		= NextSize;
					Vertex->Tex_U		= fU;
					Vertex->Tex_V		= 1.0f;
					Vertex->Rotation	= Particle->Rotation;
					Vertex->Color		= InterpLinearColor;
					Vertex++;
					PackedVertexCount++;

					fU	+= TextureIncrement;

					TempDrawPos = InterpDrawPos;
				}

				CurrPosition	= NextPosition;
				CurrTangent		= NextTangent;
				CurrSize		= NextSize;
				CurrLinearColor	= NextLinearColor;

				UBOOL bDone = TRAIL_EMITTER_IS_ONLY(TrailPayload->Flags);
				while (!bDone)
				{
					// Grab the next particle
					INT	NextIndex	= TRAIL_EMITTER_GET_NEXT(TrailPayload->Flags);

					DECLARE_PARTICLE_PTR(NextParticle, Source.ParticleData.GetData() + Source.ParticleStride * NextIndex);

					CurrentOffset = Source.TrailDataOffset;

					TrailPayload = (FTrail2TypeDataPayload*)((BYTE*)NextParticle + CurrentOffset);
					CurrentOffset += sizeof(FTrail2TypeDataPayload);

					TaperValues = (FLOAT*)((BYTE*)NextParticle + CurrentOffset);
					CurrentOffset += sizeof(FLOAT);

					NextPosition	= NextParticle->Location;
					NextTangent		= TrailPayload->Tangent * Source.TessStrength;
					NextSize		= NextParticle->Size.X * Source.Scale.X;
					NextLinearColor	= NextParticle->Color;

					TempDrawPos	= CurrPosition;

					SegmentTessFactor	= Source.TessFactor;
#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
					if (TrailTypeData->TessellationFactorDistance > KINDA_SMALL_NUMBER)
					{
						TessDistCheck		= (CurrPosition - NextPosition);
						TessRatio			= TessDistCheck.Size() / Source.TessFactorDistance;
						if (TessRatio <= 0.0f)
						{
							SegmentTessFactor	= 1;
						}
						else
						if (TessRatio < 1.0f)
						{
							SegmentTessFactor	= appTrunc((TessFactor + 1) * TessRatio);
						}
					}
#endif	//#if defined(_TRAIL2_TESSELLATE_SCALE_BY_DISTANCE_)
					InvTessFactor	= 1.0f / SegmentTessFactor;

					for (INT TessIndex = 0; TessIndex < SegmentTessFactor; TessIndex++)
					{
						InterpDrawPos = CubicInterp(
							CurrPosition, CurrTangent,
							NextPosition, NextTangent,
							InvTessFactor * (TessIndex + 1));
						InterpLinearColor = Lerp<FLinearColor>(
							CurrLinearColor, NextLinearColor, InvTessFactor * (TessIndex + 1));

						EndPoint	= InterpDrawPos;
						Location	= TempDrawPos;
						Right		= Location - EndPoint;
						Right.Normalize();

						// Setup the next verts
						NextUp	= Right ^  (EndPoint - ViewOrigin);
						if (!NextUp.Normalize())
						{
							NextUp = CameraToWorld.GetAxis(1);
						}
						if (SheetIndex)
						{
							WorkingNextUp	= QuatRotator.RotateVector(NextUp);
						}
						else
						{
							WorkingNextUp	= NextUp;
						}

						if (WorkingNextUp.IsNearlyZero())
						{
							WorkingNextUp	= CameraToWorld.GetAxis(2);
							WorkingNextUp.Normalize();
						}
						Vertex->Position	= EndPoint + WorkingNextUp * NextSize;
						Vertex->OldPosition	= EndPoint;
						Vertex->Size.X		= NextSize;
						Vertex->Size.Y		= NextSize;
						Vertex->Size.Z		= NextSize;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 0.0f;
						Vertex->Rotation	= NextParticle->Rotation;
						Vertex->Color		= InterpLinearColor;
						Vertex++;
						PackedVertexCount++;

						Vertex->Position	= EndPoint - WorkingNextUp * NextSize;
						Vertex->OldPosition	= EndPoint;
						Vertex->Size.X		= NextSize;
						Vertex->Size.Y		= NextSize;
						Vertex->Size.Z		= NextSize;
						Vertex->Tex_U		= fU;
						Vertex->Tex_V		= 1.0f;
						Vertex->Rotation	= NextParticle->Rotation;
						Vertex->Color		= InterpLinearColor;
						Vertex++;
						PackedVertexCount++;

						fU	+= TextureIncrement;

						TempDrawPos	= InterpDrawPos;
					}

					CurrPosition	= NextPosition;
					CurrTangent		= NextTangent;
					CurrSize		= NextSize;
					CurrLinearColor	= NextLinearColor;

					if (TRAIL_EMITTER_IS_END(TrailPayload->Flags) ||
						TRAIL_EMITTER_IS_ONLY(TrailPayload->Flags))
					{
						bDone = TRUE;
					}
				}
			}
		}
	}

	return TrianglesToRender;
}

///////////////////////////////////////////////////////////////////////////////
//	ParticleDynamicBufferedData
///////////////////////////////////////////////////////////////////////////////
void FParticleDynamicBufferedData::UpdateTemplate(UParticleSystemComponent* InPSysComp)
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	check(InPSysComp);

	if ((InPSysComp->Template != Template) || (bForceTemplateReset == TRUE) || (LODLevel != InPSysComp->LODLevel))
	{
		if (DynamicData != NULL)
		{
			InPSysComp->HandleDynamicDataDeletion(DynamicData, NULL);
		}
		Template = InPSysComp->Template;
		if (Template)
		{
			DynamicData = new FParticleDynamicData();
			check(DynamicData);
			DynamicData->DynamicEmitterDataArray.Empty(InPSysComp->EmitterInstances.Num());
			DynamicData->DynamicEmitterDataArray.AddZeroed(InPSysComp->EmitterInstances.Num());
		}
		else
		{
			DynamicData = NULL;
		}

		bForceTemplateReset = FALSE;
		LODLevel = InPSysComp->LODLevel;
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

void FParticleDynamicBufferedData::ReleaseResources()
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	if (DynamicData)
	{
		for (INT EmitterIndex = 0; EmitterIndex < DynamicData->DynamicEmitterDataArray.Num(); EmitterIndex++)
		{
			FDynamicEmitterDataBase* EmitterData = DynamicData->DynamicEmitterDataArray(EmitterIndex);
			if (EmitterData)
			{
				EmitterData->ReleaseResource();
			}
		}
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

///////////////////////////////////////////////////////////////////////////////
//	ParticleSystemSceneProxy
///////////////////////////////////////////////////////////////////////////////
/** Initialization constructor. */
FParticleSystemSceneProxy::FParticleSystemSceneProxy(const UParticleSystemComponent* Component):
FPrimitiveSceneProxy(Component, Component->Template ? Component->Template->GetFName() : NAME_None)
	, Owner(Component->GetOwner())
	, bSelected(Component->IsOwnerSelected())
	, CullDistance(Component->CachedMaxDrawDistance > 0 ? Component->CachedMaxDrawDistance : WORLD_MAX)
	, bCastShadow(Component->CastShadow)
	, MaterialViewRelevance(
		((Component->LODLevel >= 0) && (Component->LODLevel < Component->CachedViewRelevanceFlags.Num())) ?
			Component->CachedViewRelevanceFlags(Component->LODLevel) :
		((Component->LODLevel == -1) && (Component->CachedViewRelevanceFlags.Num() >= 1)) ?
			Component->CachedViewRelevanceFlags(0) :
			FMaterialViewRelevance()
		)
	, DynamicData(NULL)
#if PARTICLES_USE_DOUBLE_BUFFERING
#else //#if PARTICLES_USE_DOUBLE_BUFFERING
	, LastDynamicData(NULL)
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
	, SelectedWireframeMaterialInstance(
		GEngine->WireframeMaterial->GetRenderProxy(FALSE),
		GetSelectionColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f),TRUE)
		)
	, DeselectedWireframeMaterialInstance(
		GEngine->WireframeMaterial->GetRenderProxy(FALSE),
		GetSelectionColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f),FALSE)
		)
	, PendingLODDistance(0.0f)
	, LODOrigin(0.0f, 0.0f, 0.0f)
	, LODHasNearClippingPlane(FALSE)
	, LastFramePreRendered(-1)
{
	LODMethod = Component->LODMethod;
}

FParticleSystemSceneProxy::~FParticleSystemSceneProxy()
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	// The dynamic data is owned by the game thread in this case...
#else
	delete DynamicData;
	DynamicData = NULL;
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

// FPrimitiveSceneProxy interface.

/** 
* Draw the scene proxy as a dynamic element
*
* @param	PDI - draw interface to render to
* @param	View - current view
* @param	DPGIndex - current depth priority 
* @param	Flags - optional set of flags from EDrawDynamicElementFlags
*/
void FParticleSystemSceneProxy::DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags)
{
	if (View->Family->ShowFlags & SHOW_Particles)
	{
#if LOG_DETAILED_PARTICLE_RENDER_STATS
		static QWORD LastFrameCounter = 0;
		if( LastFrameCounter != GFrameCounter )
		{
			GDetailedParticleRenderStats.DumpStats();
			GDetailedParticleRenderStats.Reset();
			LastFrameCounter = GFrameCounter;
		}

		UParticleSystemComponent* ParticleSystemComponent = CastChecked<UParticleSystemComponent>(PrimitiveSceneInfo->Component);
#endif

		SCOPE_CYCLE_COUNTER(STAT_ParticleRenderingTime);
		TRACK_DETAILED_PARTICLE_RENDER_STATS(ParticleSystemComponent->Template);

		// Determine the DPG the primitive should be drawn in for this view.
		if (GetDepthPriorityGroup(View) == DPGIndex)
		{
			if (DynamicData != NULL)
			{
				for (INT Index = 0; Index < DynamicData->DynamicEmitterDataArray.Num(); Index++)
				{
					FDynamicEmitterDataBase* Data =	DynamicData->DynamicEmitterDataArray(Index);
					if ((Data == NULL) || (Data->bValid != TRUE))
					{
						continue;
					}
					// only allow rendering of mesh data based on static or dynamic vertex usage 
					// particles are rendered in two passes - one for static and one for dynamic
					if( (Flags&DontAllowStaticMeshElementData && !Data->HasDynamicMeshElementData()) ||
						(Flags&DontAllowDynamicMeshElementData && Data->HasDynamicMeshElementData()) )
					{
						continue;
					}

					Data->SceneProxy = this;
					Data->Render(this, PDI, View, DPGIndex);
				}
			}
		}

		if ((DPGIndex == SDPG_Foreground) && (View->Family->ShowFlags & SHOW_Bounds) && (View->Family->ShowFlags & SHOW_Particles) && (GIsGame || !Owner || Owner->IsSelected()))
		{
			// Draw the static mesh's bounding box and sphere.
			DrawWireBox(PDI,PrimitiveSceneInfo->Bounds.GetBox(), FColor(72,72,255),SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,1,0),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(1,0,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
			DrawCircle(PDI,PrimitiveSceneInfo->Bounds.Origin,FVector(0,1,0),FVector(0,0,1),FColor(255,255,0),PrimitiveSceneInfo->Bounds.SphereRadius,32,SDPG_Foreground);
		}
	}
}

/**
 *	Called when the rendering thread adds the proxy to the scene.
 *	This function allows for generating renderer-side resources.
 */
UBOOL FParticleSystemSceneProxy::CreateRenderThreadResources()
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	//checkf(0, TEXT("CreateRenderThreadResource> NOT USED IN DOUBLE BUFFERING!"));
	return TRUE;
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	// 
	if (DynamicData == NULL)
	{
		return FALSE;
	}

	for (INT Index = 0; Index < DynamicData->DynamicEmitterDataArray.Num(); Index++)
	{
		FDynamicEmitterDataBase* Data =	DynamicData->DynamicEmitterDataArray(Index);
		if (Data == NULL)
		{
			continue;
		}

		switch (Data->GetSource().eEmitterType)
		{
		case DET_Sprite:
			{
				FDynamicSpriteEmitterData* SpriteData = (FDynamicSpriteEmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (SpriteData->VertexFactory == NULL)
				{
					if (SpriteData->bUsesDynamicParameter == FALSE)
					{
						SpriteData->VertexFactory = new FParticleVertexFactory();
					}
					else
					{
						SpriteData->VertexFactory = new FParticleDynamicParameterVertexFactory();
					}
					check(SpriteData->VertexFactory);
					SpriteData->VertexFactory->InitResource();
				}
			}
			break;
		case DET_SubUV:
			{
				FDynamicSubUVEmitterData* SubUVData = (FDynamicSubUVEmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (SubUVData->VertexFactory == NULL)
				{
					if (SubUVData->bUsesDynamicParameter == FALSE)
					{
						SubUVData->VertexFactory = new FParticleSubUVVertexFactory();
					}
					else
					{
						SubUVData->VertexFactory = new FParticleSubUVDynamicParameterVertexFactory();
					}
					check(SubUVData->VertexFactory);
					SubUVData->VertexFactory->InitResource();
				}
			}
			break;
		case DET_Mesh:
			{
                FDynamicMeshEmitterData* MeshData = (FDynamicMeshEmitterData*)Data;
                check(MeshData);
                if(GSupportsVertexInstancing && MeshData)
                {
					if(!MeshData->InstancedMaterialInterface && MeshData->StaticMesh)
					{
						FStaticMeshLODInfo    &MeshLODInfo    = MeshData->StaticMesh->LODInfo(0);
						MeshData->InstancedMaterialInterface = MeshData->StaticMesh->LODModels(0).Elements(0).Material;
					}
                }
                if(MeshData->InstancedMaterialInterface)
                {
					const FMaterialRenderProxy* MaterialResource = MeshData->InstancedMaterialInterface->GetRenderProxy(FALSE);
					const FMaterial* Material = MaterialResource ? MaterialResource->GetMaterial() : 0;
					check(Material);
					if(Material && Material->IsUsedWithInstancedMeshParticles())
					{
						MeshData->InstanceBuffer = new FDynamicMeshEmitterData::FParticleInstancedMeshInstanceBuffer(*MeshData);
						MeshData->InstancedVertexFactory = new FParticleInstancedMeshVertexFactory;
					}
                }
			}
			break;
		case DET_Beam2:
			{
				FDynamicBeam2EmitterData* BeamData = (FDynamicBeam2EmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (BeamData->VertexFactory == NULL)
				{
					BeamData->VertexFactory = new FParticleBeamTrailVertexFactory();
					check(BeamData->VertexFactory);
					BeamData->VertexFactory->InitResource();
				}
			}
			break;
		case DET_Trail2:
			{
				FDynamicTrail2EmitterData* TrailData = (FDynamicTrail2EmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (TrailData->VertexFactory == NULL)
				{
					TrailData->VertexFactory = new FParticleBeamTrailVertexFactory();
					check(TrailData->VertexFactory);
					TrailData->VertexFactory->InitResource();
				}
			}
			break;
		default:
			break;
		}
	}

	return TRUE;
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

/**
 *	Called when the rendering thread removes the dynamic data from the scene.
 */
UBOOL FParticleSystemSceneProxy::ReleaseRenderThreadResources()
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	//checkf(0, TEXT("ReleaseRenderThreadResources> NOT USED IN DOUBLE BUFFERING!"));
	return TRUE;
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	// 
	if (DynamicData == NULL)
	{
		return FALSE;
	}

	for (INT Index = 0; Index < DynamicData->DynamicEmitterDataArray.Num(); Index++)
	{
		FDynamicEmitterDataBase* Data =	DynamicData->DynamicEmitterDataArray(Index);
		if (Data == NULL)
		{
			continue;
		}

		switch (Data->GetSource().eEmitterType)
		{
		case DET_Sprite:
			{
				FDynamicSpriteEmitterData* SpriteData = (FDynamicSpriteEmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (SpriteData->VertexFactory)
				{
					SpriteData->VertexFactory->ReleaseResource();
				}
			}
			break;
		case DET_SubUV:
			{
				FDynamicSubUVEmitterData* SubUVData = (FDynamicSubUVEmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (SubUVData->VertexFactory)
				{
					SubUVData->VertexFactory->ReleaseResource();
				}
			}
			break;
		case DET_Mesh:
			{
				FDynamicMeshEmitterData* MeshData = (FDynamicMeshEmitterData*)Data;
			}
			break;
		case DET_Beam2:
			{
				FDynamicBeam2EmitterData* BeamData = (FDynamicBeam2EmitterData*)Data;
				// Create the vertex factory...
				//@todo. Cache these??
				if (BeamData->VertexFactory)
				{
					BeamData->VertexFactory->ReleaseResource();
				}
			}
			break;
		case DET_Trail2:
			{
				FDynamicTrail2EmitterData* TrailData = (FDynamicTrail2EmitterData*)Data;
				if (TrailData->VertexFactory)
				{
					TrailData->VertexFactory->ReleaseResource();
				}
			}
			break;
		default:
			break;
		}
	}

	return TRUE;
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

void FParticleSystemSceneProxy::UpdateData(FParticleDynamicData* NewDynamicData)
{
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		ParticleUpdateDataCommand,
		FParticleSystemSceneProxy*, Proxy, this,
		FParticleDynamicData*, NewDynamicData, NewDynamicData,
		{
			Proxy->UpdateData_RenderThread(NewDynamicData);
		}
		);
}

void FParticleSystemSceneProxy::UpdateData_RenderThread(FParticleDynamicData* NewDynamicData)
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	DynamicData = NewDynamicData;
#else //#if PARTICLES_USE_DOUBLE_BUFFERING
	ReleaseRenderThreadResources();
	if (DynamicData != NewDynamicData)
	{
		delete DynamicData;
	}
	DynamicData = NewDynamicData;
	CreateRenderThreadResources();
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

void FParticleSystemSceneProxy::UpdateViewRelevance(FMaterialViewRelevance& NewViewRelevance)
{
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		ParticleUpdateViewRelevanceCommand,
		FParticleSystemSceneProxy*, Proxy, this,
		FMaterialViewRelevance, ViewRel, NewViewRelevance,
		{
			Proxy->UpdateViewRelevance_RenderThread(ViewRel);
		}
		);		
}

void FParticleSystemSceneProxy::UpdateViewRelevance_RenderThread(FMaterialViewRelevance& NewViewRelevance)
{
	MaterialViewRelevance = NewViewRelevance;
}

void FParticleSystemSceneProxy::DetermineLODDistance(const FSceneView* View, INT FrameNumber)
{
	INT	LODIndex = -1;

	if (LODMethod == PARTICLESYSTEMLODMETHOD_Automatic)
	{
		// Default to the highest LOD level
		FVector	CameraPosition	= View->ViewOrigin;
		FVector	CompPosition	= LocalToWorld.GetOrigin();
		FVector	DistDiff		= CompPosition - CameraPosition;
		FLOAT	Distance		= DistDiff.Size() * View->LODDistanceFactor;

		if (FrameNumber != LastFramePreRendered)
		{
			// First time in the frame - then just set it...
			PendingLODDistance = Distance;
			LODOrigin = CameraPosition;
			LODHasNearClippingPlane = View->bHasNearClippingPlane;
			LODNearClippingPlane = View->NearClippingPlane;
			LastFramePreRendered = FrameNumber;
		}
		else
		if (Distance < PendingLODDistance)
		{
			// Not first time in the frame, then we compare and set if closer
			PendingLODDistance = Distance;
			LODOrigin = CameraPosition;
			LODHasNearClippingPlane = View->bHasNearClippingPlane;
			LODNearClippingPlane = View->NearClippingPlane;
		}
	}
}

UBOOL FParticleSystemSceneProxy::GetNearClippingPlane(FPlane& OutNearClippingPlane) const
{
	if(LODHasNearClippingPlane)
	{
		OutNearClippingPlane = LODNearClippingPlane;
	}
	return LODHasNearClippingPlane;
}

/**
 *	Retrieve the appropriate camera Up and Right vectors for LockAxis situations
 *
 *	@param	DynamicData		The emitter dynamic data the values are being retrieved for
 *	@param	CameraUp		OUTPUT - the resulting camera Up vector
 *	@param	CameraRight		OUTPUT - the resulting camera Right vector
 */
void FParticleSystemSceneProxy::GetAxisLockValues(FDynamicSpriteEmitterDataBase* DynamicData, FVector& CameraUp, FVector& CameraRight)
{
	const FDynamicSpriteEmitterReplayData& SpriteSource =
		static_cast< const FDynamicSpriteEmitterReplayData& >( DynamicData->GetSource() );
	const FMatrix& AxisLocalToWorld = SpriteSource.bUseLocalSpace ? LocalToWorld: FMatrix::Identity;

	switch (SpriteSource.LockAxisFlag)
	{
	case EPAL_X:
		CameraUp		=  AxisLocalToWorld.GetAxis(2);
		CameraRight	=  AxisLocalToWorld.GetAxis(1);
		break;
	case EPAL_Y:
		CameraUp		=  AxisLocalToWorld.GetAxis(2);
		CameraRight	= -AxisLocalToWorld.GetAxis(0);
		break;
	case EPAL_Z:
		CameraUp		=  AxisLocalToWorld.GetAxis(0);
		CameraRight	= -AxisLocalToWorld.GetAxis(1);
		break;
	case EPAL_NEGATIVE_X:
		CameraUp		=  AxisLocalToWorld.GetAxis(2);
		CameraRight	= -AxisLocalToWorld.GetAxis(1);
		break;
	case EPAL_NEGATIVE_Y:
		CameraUp		=  AxisLocalToWorld.GetAxis(2);
		CameraRight	=  AxisLocalToWorld.GetAxis(0);
		break;
	case EPAL_NEGATIVE_Z:
		CameraUp		=  AxisLocalToWorld.GetAxis(0);
		CameraRight	=  AxisLocalToWorld.GetAxis(1);
		break;
	}
}

/**
* @return Relevance for rendering the particle system primitive component in the given View
*/
FPrimitiveViewRelevance FParticleSystemSceneProxy::GetViewRelevance(const FSceneView* View)
{
	FPrimitiveViewRelevance Result;
	const EShowFlags ShowFlags = View->Family->ShowFlags;
	if (IsShown(View) && (ShowFlags & SHOW_Particles))
	{
		Result.bDynamicRelevance = TRUE;
		Result.bNeedsPreRenderView = TRUE;
		Result.SetDPG(GetDepthPriorityGroup(View),TRUE);
		if (!(View->Family->ShowFlags & SHOW_Wireframe) && (View->Family->ShowFlags & SHOW_Materials))
		{
			MaterialViewRelevance.SetPrimitiveViewRelevance(Result);
		}
		if (View->Family->ShowFlags & SHOW_Bounds)
		{
			Result.SetDPG(SDPG_Foreground,TRUE);
			Result.bOpaqueRelevance = TRUE;
		}
		// see if any of the emitters use dynamic vertex data
		if (DynamicData != NULL)
		{
			for (INT Index = 0; Index < DynamicData->DynamicEmitterDataArray.Num(); Index++)
			{
				FDynamicEmitterDataBase* Data =	DynamicData->DynamicEmitterDataArray(Index);
				if (Data == NULL)
				{
					continue;
				}
				if( Data->HasDynamicMeshElementData() )
				{
					Result.bUsesDynamicMeshElementData = TRUE;
				}
			}
		}
		else
		{
			// In order to get the LOD distances to update,
			// we need to force a call to DrawDynamicElements...
			Result.bOpaqueRelevance = TRUE;
		}
	}

	if (IsShadowCast(View))
	{
		Result.bShadowRelevance = TRUE;
	}

	return Result;
}

/**
 *	Helper function for calculating the tessellation for a given view.
 *
 *	@param	View		The view of interest.
 *	@param	FrameNumber		The frame number being rendered.
 */
void FParticleSystemSceneProxy::ProcessPreRenderView(const FSceneView* View, INT FrameNumber)
{
	const FSceneView* LocalView = View;
	if (View->ParentViewFamily)
	{
		if ((View->ParentViewIndex != -1) && (View->ParentViewIndex <= View->ParentViewFamily->Views.Num()))
		{
			// If the ParentViewIndex is set to a valid index, use that View
			LocalView = View->ParentViewFamily->Views(View->ParentViewIndex);
		}
		else
		if (View->ParentViewIndex == -1)
		{
			// Iterate over all the Views in the ParentViewFamily
			FSceneView TempView(
				View->Family,
				View->State,
				-1,
				View->ParentViewFamily,
				View->ActorVisibilityHistory,
				View->ViewActor,
				View->PostProcessChain,
				View->PostProcessSettings,
				View->PostProcessMask,
				View->Drawer,
				View->X,
				View->Y,
				View->SizeX,
				View->SizeY,
				View->ViewMatrix,
				View->ProjectionMatrix,
				View->BackgroundColor,
				View->OverlayColor,
				View->ColorScale,
				View->HiddenPrimitives,
				View->LODDistanceFactor
				);
			for (INT ViewIdx = 0; ViewIdx < View->ParentViewFamily->Views.Num(); ViewIdx++)
			{
				TempView.ParentViewIndex = ViewIdx;
				ProcessPreRenderView(&TempView, FrameNumber);
			}
			return;
		}
	}

	if (DynamicData && DynamicData->bNeedsLODDistanceUpdate)
	{
		DetermineLODDistance(LocalView, FrameNumber);
	}
}

/**
 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
 *  Only called for primitives that are visible and have bDynamicRelevance
 *
 *	@param	ViewFamily		The ViewFamily to pre-render for
 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
 *	@param	FrameNumber		The frame number of this pre-render
 */
void FParticleSystemSceneProxy::PreRenderView(const FSceneViewFamily* ViewFamily, const TBitArray<FDefaultBitArrayAllocator>& VisibilityMap, INT FrameNumber)
{
	for (INT ViewIndex = 0; ViewIndex < ViewFamily->Views.Num(); ViewIndex++)
	{
		ProcessPreRenderView(ViewFamily->Views(ViewIndex), FrameNumber);
	}

	if (DynamicData != NULL)
	{
		for (INT EmitterIndex = 0; EmitterIndex < DynamicData->DynamicEmitterDataArray.Num(); EmitterIndex++)
		{
			FDynamicEmitterDataBase* DynamicEmitterData = DynamicData->DynamicEmitterDataArray(EmitterIndex);
			if (DynamicEmitterData)
			{
				DynamicEmitterData->PreRenderView(this, ViewFamily, VisibilityMap, FrameNumber);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//	ParticleSystemComponent
///////////////////////////////////////////////////////////////////////////////

FPrimitiveSceneProxy* UParticleSystemComponent::CreateSceneProxy()
{
	FParticleSystemSceneProxy* NewProxy = NULL;

	//@fixme EmitterInstances.Num() check should be here to avoid proxies for dead emitters but there are some edge cases where it happens for emitters that have just activated...
	if ((bIsActive == TRUE)/** && (EmitterInstances.Num() > 0)*/)
	{
		if (EmitterInstances.Num() > 0)
		{
			CacheViewRelevanceFlags(NULL);
		}
		NewProxy = ::new FParticleSystemSceneProxy(this);
		check (NewProxy);
	}
	
	// 
	return NewProxy;
}

////////////////////////////////////////////////////////////////////////////////
//	Helper functions
///////////////////////////////////////////////////////////////////////////////
void PS_DumpBeamDataInformation(TCHAR* Message, 
	UParticleSystemComponent* PSysComp, FParticleSystemSceneProxy* Proxy, 
	FParticleDynamicData* NewPSDynamicData, FParticleDynamicData* OldPSDynamicData, 
	FDynamicBeam2EmitterData* NewBeamData, FDynamicBeam2EmitterData* OldBeamData)
{
#if defined(_DEBUG_BEAM_DATA_)
	INT	Spaces = 0;
	if (Message)
	{
		appOutputDebugString(Message);
		Spaces = appStrlen(Message);
	}

	while (Spaces < 72)
	{
		appOutputDebugString(TEXT(" "));
		Spaces++;
	}
	appOutputDebugString(TEXT("    "));

	FString DebugOut = FString::Printf(TEXT("PSysComp     0x%08x - SceneProxy   0x%08x - DynamicData  0x%08x - OldDynData   0x%08x - BeamData     0x%08x - OldBeamData  0x%08x"),
		PSysComp, Proxy, NewPSDynamicData, OldPSDynamicData, NewBeamData, OldBeamData);
	appOutputDebugString(*DebugOut);
	appOutputDebugString(TEXT("\n"));
#endif
}

void PS_DumpTrailDataInformation(TCHAR* Message, 
	UParticleSystemComponent* PSysComp, FParticleSystemSceneProxy* Proxy, 
	FParticleDynamicData* NewPSDynamicData, FParticleDynamicData* OldPSDynamicData, 
	FDynamicTrail2EmitterData* NewTrailData, FDynamicTrail2EmitterData* OldTrailData)
{
#if defined(_DEBUG_TRAIL_DATA_)
	INT	Spaces = 0;
	if (Message)
	{
		appOutputDebugString(Message);
		Spaces = appStrlen(Message);
	}

	while (Spaces < 48)
	{
		appOutputDebugString(TEXT(" "));
		Spaces++;
	}
	appOutputDebugString(TEXT("    "));

	FString DebugOut = FString::Printf(TEXT("PSysComp     0x%08x - SceneProxy   0x%08x - DynamicData  0x%08x - OldDynData   0x%08x - TrailData     0x%08x - OldTrailData  0x%08x"),
		PSysComp, Proxy, NewPSDynamicData, OldPSDynamicData, NewTrailData, OldTrailData);
	appOutputDebugString(*DebugOut);
	appOutputDebugString(TEXT("\n"));
#endif
}
