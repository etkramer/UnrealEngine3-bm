/*=============================================================================
	FoliageComponent.cpp: Foliage rendering implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "FoliageRendering.h"

IMPLEMENT_CLASS(UFoliageComponent);

/** Base class for implementing the hooks used by the common foliage rendering code to build the render data for a FoliageComponent. */
class FFoliageFactoryRenderDataPolicy
{
public:
	/** Initialization constructor. */
	FFoliageFactoryRenderDataPolicy(UFoliageComponent* InComponent):
		Component(InComponent),
		StaticMeshRenderData(InComponent->InstanceStaticMesh->LODModels(0)),
		MinTransitionRadiusSquared(Square(InComponent->MinTransitionRadius)),
		InvTransitionSize(1.0f / Max(InComponent->MaxDrawRadius - InComponent->MinTransitionRadius,0.0f))
	{
	}

	virtual ~FFoliageFactoryRenderDataPolicy() {}

	/** @return The maximum number of instances which will be rendered. */
	INT GetMaxInstances() const
	{
		return Component->Instances.Num();
	}

	/** @return The static mesh render data for a single foliage mesh. */
	const FStaticMeshRenderData& GetMeshRenderData() const
	{
		return StaticMeshRenderData;
	}

	/**
	 * These functions are implemented by TFoliageRenderData.
	 */

	virtual void UpdateInstances(const TArray<INT>& VisibleFoliageInstances,const FSceneView* View) {}

	/** Accesses the vertex factory used to render the foliage. */
	virtual const FVertexFactory* GetVertexFactory()
	{
		return NULL;
	}

	/** Accesses the vertex buffer containing the foliage static lighting. */
	virtual const FVertexBuffer* GetStaticLightingVertexBuffer() const
	{
		return NULL;
	}

	/** Accesses the index buffer containing the foliage mesh indices. */
	virtual const FIndexBuffer* GetIndexBuffer() const
	{
		return NULL;
	}

protected:

	/** The foliage component which is being rendered. */
	UFoliageComponent* Component;

	/** The static mesh render data for a single foliage mesh. */
	const FStaticMeshRenderData& StaticMeshRenderData;

	/** The squared distance that foliage instances begin to scale down at. */
	const FLOAT MinTransitionRadiusSquared;

	/** The inverse difference between the distance where foliage instances begin to scale down, and the distance where they disappear. */
	const FLOAT InvTransitionSize;
};

/** Implements the hooks used by the common foliage rendering code for a certain FoliageInstanceType to build the render data for a FoliageComponent. */
template<class FoliageInstanceType>
class TFoliageFactoryRenderDataPolicy : public FFoliageFactoryRenderDataPolicy
{
public:

	typedef FoliageInstanceType InstanceType;
	typedef INT InstanceReferenceType;

	/** Initialization constructor. */
	TFoliageFactoryRenderDataPolicy(UFoliageComponent* InComponent) :
		FFoliageFactoryRenderDataPolicy(InComponent)
	{}

	/**
	* Accesses an instance for a specific view.  The instance is scaled according to the view.
	* @param InstanceIndex - The instance index to access.
	* @param View - The view to access the instance for.
	* @param OutInstance - Upon return, contains the instance data.
	* @param OutWindScale - Upon return, contains the amount to scale the wind skew by.
	*/
	void GetInstance(INT InstanceIndex,const FSceneView* View,InstanceType& OutInstance,FLOAT& OutWindScale) const
	{
		const FGatheredFoliageInstance& Instance = Component->Instances(InstanceIndex);

		// Calculate the transition scale for this instance.
		const FLOAT DistanceSquared = ((FVector)View->ViewOrigin - Instance.Location).SizeSquared() * Instance.DistanceFactorSquared;
		const FLOAT TransitionScale =
			DistanceSquared < MinTransitionRadiusSquared ?
			1.0f :
		(1.0f - (appSqrt(DistanceSquared) - Component->MinTransitionRadius) * InvTransitionSize);

		OutInstance.Location = Instance.Location + View->PreViewTranslation;
		OutInstance.XAxis = Instance.XAxis * TransitionScale;
		OutInstance.YAxis = Instance.YAxis * TransitionScale;
		OutInstance.ZAxis = Instance.ZAxis * TransitionScale;
		for(UINT CoefficientIndex = 0; CoefficientIndex < InstanceType::NumCoefficients; CoefficientIndex++)
		{
			OutInstance.StaticLighting[CoefficientIndex] = Instance.StaticLighting[CoefficientIndex + InstanceType::CoefficientOffset];
		}

		OutWindScale = Component->SwayScale * TransitionScale;
	}

	/**
	* Sets up the foliage vertex factory for the instance's static lighting data.
	* @param InstanceBuffer - The vertex buffer containing the instance data.
	* @param OutData - Upon return, the static lighting components are set for the terrain foliage rendering.
	*/
	void InitVertexFactoryStaticLighting(FVertexBuffer* InstanceBuffer,FFoliageVertexFactory::DataType& OutData)
	{
		OutData.LightMapStream.Offset = STRUCT_OFFSET(InstanceType,StaticLighting);
		OutData.LightMapStream.DirectionalStride = sizeof(FDirectionalFoliageInstance);
		OutData.LightMapStream.SimpleStride = sizeof(FSimpleFoliageInstance);
		OutData.LightMapStream.bUseInstanceIndex = TRUE;
	}
};

/** Represents foliage in the rendering thread. */
class FFoliageSceneProxy : public FPrimitiveSceneProxy
{
public:

	FFoliageSceneProxy(UFoliageComponent* InComponent):
		FPrimitiveSceneProxy(InComponent),
		Component(InComponent),
		Material(NULL),
		RenderResources(NULL),
		bSelected(InComponent->IsOwnerSelected()),
		LevelColor(255,255,255),
		PropertyColor(255,255,255)
	{
		// Try to find a color for level coloration.
		if (Component->GetOwner())
		{
			ULevel* Level = Component->GetOwner()->GetLevel();
			ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel(Level);
			if (LevelStreaming)
			{
				LevelColor = LevelStreaming->DrawColor;
			}
		}

		// Get a color for property coloration.
		GEngine->GetPropertyColorationColor( (UObject*)Component, PropertyColor );

		// Find the material to use for the foliage.
		if(Component->Material)
		{
			Material = Component->Material;
		}
		else if(Component->InstanceStaticMesh->LODModels(0).Elements.Num() && Component->InstanceStaticMesh->LODModels(0).Elements(0).Material)
		{
			Material = Component->InstanceStaticMesh->LODModels(0).Elements(0).Material;
		}

		// Ensure the material is valid for foliage rendering with static lighting.
		if(!Material || !Material->CheckMaterialUsage(MATUSAGE_Foliage) || !Material->CheckMaterialUsage(MATUSAGE_StaticLighting))
		{
			Material = GEngine->DefaultMaterial;
		}

		MaterialViewRelevance = Material->GetViewRelevance();

		// Set the foliage mesh's bUsedWithInstancing flag if it isn't set yet, and this is in the editor.
		if(!Component->InstanceStaticMesh->bUsedForInstancing && GIsEditor)
		{
			Component->InstanceStaticMesh->PreEditChange(NULL);
			Component->InstanceStaticMesh->bUsedForInstancing = TRUE;
			Component->InstanceStaticMesh->PostEditChange(NULL);
			Component->InstanceStaticMesh->MarkPackageDirty();
		}
	}

	~FFoliageSceneProxy()
	{
		delete RenderResources;
	}

	// FPrimitiveSceneProxy interface.

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);
	virtual void GetLightRelevance(FLightSceneInfo* LightSceneInfo, UBOOL& bDynamic, UBOOL& bRelevant, UBOOL& bLightMapped);
	
	/** 
	* Draw the scene proxy as a dynamic element
	*
	* @param	PDI - draw interface to render to
	* @param	View - current view
	* @param	DPGIndex - current depth priority 
	* @param	Flags - optional set of flags from EDrawDynamicElementFlags
	*/
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags);
	

	virtual EMemoryStats GetMemoryStatType( void ) const { return STAT_GameToRendererMallocOther; }
	virtual DWORD GetMemoryFootprint( void ) const { return 0; }

private:

	/** The component. */
	UFoliageComponent* Component;

	/** The material to render on the foliage. */
	UMaterialInterface* Material;

	/** The render resources for the foliage component.  These are created and destroyed as the foliage enters the view. */
	FFoliageFactoryRenderDataPolicy* RenderResources;

	/** Whether the component's actor is selected. */
	BITFIELD bSelected : 1;

	/** The foliage material's view relevance. */
	FMaterialViewRelevance MaterialViewRelevance;

	FColor LevelColor;
	FColor PropertyColor;
};


/** Implementation of the interface between the foliage static lighting and the renderer. */
class FFoliageLCI : public FLightCacheInterface
{
public:

	/** Initialization constructor. */
	FFoliageLCI(const UFoliageComponent* InComponent,const FVertexBuffer* InVertexBuffer):
		Component(InComponent),
		VertexBuffer(InVertexBuffer)
	{}

	// FLightCacheInterface
	virtual FLightInteraction GetInteraction(const FLightSceneInfo* LightSceneInfo) const
	{
		if(Component->StaticallyRelevantLights.ContainsItem(LightSceneInfo->LightmapGuid))
		{
			return FLightInteraction::LightMap();
		}
		else if(Component->StaticallyIrrelevantLights.ContainsItem(LightSceneInfo->LightmapGuid))
		{
			return FLightInteraction::Irrelevant();
		}
		else
		{
			return FLightInteraction::Uncached();
		}
	}
	virtual FLightMapInteraction GetLightMapInteraction() const
	{
		const FVector4 DirectionalCoefficientScale = FVector4(
			Component->DirectionalStaticLightingScale[0],
			Component->DirectionalStaticLightingScale[1],
			Component->DirectionalStaticLightingScale[2]
			);
		const FVector4 SimpleCoefficientScale = FVector4(
			Component->SimpleStaticLightingScale[0],
			Component->SimpleStaticLightingScale[1],
			Component->SimpleStaticLightingScale[2]
			);
		const FVector4 CoefficientScales[NUM_GATHERED_LIGHTMAP_COEF] = { DirectionalCoefficientScale, DirectionalCoefficientScale, DirectionalCoefficientScale, SimpleCoefficientScale };

		return FLightMapInteraction::Vertex(VertexBuffer,CoefficientScales,GSystemSettings.bAllowDirectionalLightMaps);
	}

private:

	/** The foliage component which is being rendered. */
	const UFoliageComponent* Component;

	/** The vertex buffer containing the foliage vertices being rendered. */
	const FVertexBuffer* VertexBuffer;
};

FPrimitiveViewRelevance FFoliageSceneProxy::GetViewRelevance(const FSceneView* View)
{
	FPrimitiveViewRelevance Result;
	if(View->Family->ShowFlags & SHOW_Foliage)
	{
		if(IsShown(View))
		{
			Result.bDynamicRelevance = TRUE;
			Result.SetDPG(GetDepthPriorityGroup(View),TRUE);
		}

		if (IsShadowCast(View))
		{
			Result.bShadowRelevance = TRUE;
			Result.SetDPG(GetDepthPriorityGroup(View),TRUE);
		}

		MaterialViewRelevance.SetPrimitiveViewRelevance(Result);
	}

	return Result;
}

void FFoliageSceneProxy::GetLightRelevance(FLightSceneInfo* LightSceneInfo, UBOOL& bDynamic, UBOOL& bRelevant, UBOOL& bLightMapped)
{
	FFoliageLCI FoliageLCI(Component,NULL);

	const ELightInteractionType InteractionType = FoliageLCI.GetInteraction(LightSceneInfo).GetType();

	// Attach the light to the primitive's static meshes.
	bDynamic = (InteractionType == LIT_Uncached);
	bRelevant = (InteractionType != LIT_CachedIrrelevant);
	bLightMapped = (InteractionType == LIT_CachedLightMap || InteractionType == LIT_CachedIrrelevant);
}

/** 
* Draw the scene proxy as a dynamic element
*
* @param	PDI - draw interface to render to
* @param	View - current view
* @param	DPGIndex - current depth priority 
* @param	Flags - optional set of flags from EDrawDynamicElementFlags
*/
void FFoliageSceneProxy::DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags)
{
	// Only draw foliage in perspective viewports, not for hit testing, and on hardware that supports vertex instancing.
	const UBOOL bIsPerspectiveViewport = View->ProjectionMatrix.M[3][3] < 1.0f;
	if(	!PDI->IsHitTesting() && 
		bIsPerspectiveViewport && 
		(GSupportsVertexInstancing || GSupportsEmulatedVertexInstancing) &&
		(GetViewRelevance(View).GetDPG(DPGIndex) == TRUE))
	{
		const FStaticMeshRenderData& FoliageMeshRenderData = Component->InstanceStaticMesh->LODModels(0);

		// Check that the material isn't ignored in this context before doing any of the work to render it.
		const FMaterialRenderProxy* MaterialRenderProxy = Material->GetRenderProxy(bSelected);
		if(!PDI->IsMaterialIgnored(MaterialRenderProxy))
		{
			if(!RenderResources)
			{
				if (GSystemSettings.bAllowDirectionalLightMaps)
				{
					TFoliageFactoryRenderDataPolicy<FDirectionalFoliageInstance> NewFoliageFactoryPolicy(Component);
					RenderResources = new TFoliageRenderData<TFoliageFactoryRenderDataPolicy<FDirectionalFoliageInstance> >(NewFoliageFactoryPolicy);
				}
				else
				{
					TFoliageFactoryRenderDataPolicy<FSimpleFoliageInstance> NewFoliageFactoryPolicy(Component);
					RenderResources = new TFoliageRenderData<TFoliageFactoryRenderDataPolicy<FSimpleFoliageInstance> >(NewFoliageFactoryPolicy);
				}
			}

			// Find the foliage instances visible in this view.
			TArray<INT> VisibleFoliageInstances;
			VisibleFoliageInstances.Empty(Component->Instances.Num());
			const FLOAT DrawRadiusSquared = Square(Component->MaxDrawRadius);
			const FBoxSphereBounds MeshLocalBounds = Component->InstanceStaticMesh->Bounds;
			const FLOAT MeshLocalRadius = MeshLocalBounds.Origin.Size() + MeshLocalBounds.SphereRadius;
			const FLOAT MeshMaxScale = Max(Component->MaxScale.X,Max(Component->MaxScale.Y,Component->MaxScale.Z));
			const FLOAT MeshRadius = MeshLocalRadius * MeshMaxScale;
			for(INT InstanceIndex = 0;InstanceIndex < Component->Instances.Num();InstanceIndex++)
			{
				const FGatheredFoliageInstance& Instance = Component->Instances(InstanceIndex);
				if(	(Instance.Location - View->ViewOrigin).SizeSquared() * Instance.DistanceFactorSquared < DrawRadiusSquared &&
					View->ViewFrustum.IntersectSphere(Instance.Location,MeshRadius))
				{
					VisibleFoliageInstances.AddItem(InstanceIndex);
				}
			}

			INC_DWORD_STAT_BY(STAT_TerrainFoliageInstances,VisibleFoliageInstances.Num());

			if(VisibleFoliageInstances.Num() > 1 && FoliageMeshRenderData.NumVertices && FoliageMeshRenderData.IndexBuffer.Indices.Num())
			{
				RenderResources->UpdateInstances(VisibleFoliageInstances,View);

				FFoliageLCI FoliageLCI(Component,RenderResources->GetStaticLightingVertexBuffer());
				FMeshElement MeshElement;
				MeshElement.IndexBuffer   = &FoliageMeshRenderData.IndexBuffer;
				MeshElement.VertexFactory = RenderResources->GetVertexFactory();
				MeshElement.MaterialRenderProxy = MaterialRenderProxy;
				MeshElement.LCI = &FoliageLCI;
				MeshElement.LocalToWorld = FMatrix::Identity;
				MeshElement.WorldToLocal = FMatrix::Identity;
				MeshElement.FirstIndex = 0;
				MeshElement.NumPrimitives = FoliageMeshRenderData.IndexBuffer.Indices.Num() / 3;
				MeshElement.MinVertexIndex = 0;
				MeshElement.MaxVertexIndex = FoliageMeshRenderData.NumVertices - 1;
				MeshElement.Type = PT_TriangleList;
				MeshElement.DepthPriorityGroup = DPGIndex;
				DrawRichMesh(PDI,MeshElement,FLinearColor(0,1,0),LevelColor,PropertyColor,PrimitiveSceneInfo,bSelected);
			}
		}
	}
}

void UFoliageComponent::UpdateBounds()
{
	// Find the bounding box of all the component's instance origins.
	FBox BoundingBox(0);
	for(INT InstanceIndex = 0;InstanceIndex < Instances.Num();InstanceIndex++)
	{
		BoundingBox += Instances(InstanceIndex).Location;
	}

	if(InstanceStaticMesh)
	{
		// Expand the bounding box to account for the size of each instance.
		BoundingBox.ExpandBy(InstanceStaticMesh->Bounds.SphereRadius);
	}

	Bounds = FBoxSphereBounds(BoundingBox);
}

FPrimitiveSceneProxy* UFoliageComponent::CreateSceneProxy()
{
	// Verify that the foliage mesh is valid before using it.
	const UBOOL bFoliageMeshIsValid = 
		InstanceStaticMesh &&
		InstanceStaticMesh->LODModels(0).NumVertices > 0 && 
		InstanceStaticMesh->LODModels(0).IndexBuffer.Indices.Num() > 0;

	if(bFoliageMeshIsValid)
	{
		return ::new FFoliageSceneProxy(this);
	}
	else
	{
		return NULL;
	}
}

/** The number of light samples along each axis of a foliage instance. */
#define NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS	2

/** The total number of light samples for a foliage instance. */
#define NUM_LIGHT_SAMPLES_PER_INSTANCE		(NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS * NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS * NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS)

/** Represents foliage instances to the static lighting system. */
class FFoliageStaticLightingMesh : public FStaticLightingMesh
{
public:

	/** Initialization constructor. */
	FFoliageStaticLightingMesh(const UFoliageComponent* InPrimitive,const TArray<ULightComponent*>& InRelevantLights):
		FStaticLightingMesh(
			InPrimitive->Instances.Num() * NUM_LIGHT_SAMPLES_PER_INSTANCE,
			InPrimitive->Instances.Num() * NUM_LIGHT_SAMPLES_PER_INSTANCE,
			FALSE,
			FALSE,
			TRUE,
			InRelevantLights,
			InPrimitive->Bounds.GetBox()
			),
		Primitive(InPrimitive)
	{
	}

	// FStaticLightingMesh interface.

	virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2) const
	{
		const INT InstanceIndex = TriangleIndex / NUM_LIGHT_SAMPLES_PER_INSTANCE;
		const INT SampleIndex = TriangleIndex % NUM_LIGHT_SAMPLES_PER_INSTANCE;
		const FGatheredFoliageInstance& Instance = Primitive->Instances(InstanceIndex);

		// Determine which sample this is.
		const INT SampleX = SampleIndex % NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS;
		const INT SampleY = (SampleIndex / NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS) % NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS;
		const INT SampleZ = (SampleIndex / NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS / NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS) % NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS;
		const FLOAT FracSampleX = Lerp(-1.0f,+1.0f,(SampleX + 0.5f) / (FLOAT)NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS);
		const FLOAT FracSampleY = Lerp(-1.0f,+1.0f,(SampleY + 0.5f) / (FLOAT)NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS);
		const FLOAT FracSampleZ = Lerp(-1.0f,+1.0f,(SampleZ + 0.5f) / (FLOAT)NUM_LIGHT_SAMPLES_PER_INSTANCE_AXIS);

		const FVector LocalPosition = Primitive->InstanceStaticMesh->Bounds.Origin +
			Primitive->InstanceStaticMesh->Bounds.BoxExtent * FVector(FracSampleX,FracSampleY,FracSampleZ);

		OutV0.WorldPosition = Instance.Location +
			Instance.XAxis * LocalPosition.X +
			Instance.YAxis * LocalPosition.Y +
			Instance.ZAxis * LocalPosition.Z;
		OutV0.WorldTangentX = Instance.XAxis;
		OutV0.WorldTangentY = Instance.YAxis;
		OutV0.WorldTangentZ = Instance.ZAxis;
		
		OutV1 = OutV2 = OutV0;
	}

	virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
	{
		OutI0 = TriangleIndex;
		OutI1 = TriangleIndex;
		OutI2 = TriangleIndex;
	}

	virtual UBOOL ShouldCastShadow(ULightComponent* Light,const FStaticLightingMapping* Receiver) const
	{
		return FALSE;
	}

	virtual UBOOL IsUniformShadowCaster() const
	{
		return FALSE;
	}

	virtual FLightRayIntersection IntersectLightRay(const FVector& Start,const FVector& End,UBOOL bFindNearestIntersection) const
	{
		return FLightRayIntersection::None();
	}

private:
	
	/** The primitive this mesh represents. */
	const UFoliageComponent* const Primitive;
};

/** Represents the mapping from static lighting data to foliage instances. */
class FFoliageStaticLightingVertexMapping : public FStaticLightingVertexMapping
{
public:

	/** Initialization constructor. */
	FFoliageStaticLightingVertexMapping(UFoliageComponent* InComponent,FStaticLightingMesh* InMesh,const TArray<ULightComponent*>& InRelevantLights):
		FStaticLightingVertexMapping(
			InMesh,
			InComponent,
			InComponent->bForceDirectLightMap,
			0.0f,
			TRUE
			),
		Component(InComponent),
		RelevantLights(InRelevantLights)
	{}

	// FStaticLightingTextureMapping interface
	virtual void Apply(FLightMapData1D* LightMapData,const TMap<ULightComponent*,FShadowMapData1D*>& ShadowMapData)
	{
		// FGatheredFoliageInstance::StaticLighting needs to be adjusted if NUM_GATHERED_LIGHTMAP_COEF changes.
		checkSlow(NUM_GATHERED_LIGHTMAP_COEF == 4);

		if(LightMapData)
		{
			// Calculate the maximum lighting value for each color component.
			FLOAT MaxDirectionalLightValue[3] = { 0.0f, 0.0f, 0.0f };
			FLOAT MaxSimpleLightValue[3] = { 0.0f, 0.0f, 0.0f };
			for(INT SampleIndex = 0;SampleIndex < LightMapData->GetSize();SampleIndex++)
			{
				const FLightSample& LightSample = (*LightMapData)(SampleIndex);				
				for(INT CoefficientIndex = 0;CoefficientIndex < NUM_DIRECTIONAL_LIGHTMAP_COEF;CoefficientIndex++)
				{
					for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
					{
						MaxDirectionalLightValue[ColorIndex] = Max(MaxDirectionalLightValue[ColorIndex],LightSample.Coefficients[CoefficientIndex][ColorIndex]);
					}
				}

				for(INT CoefficientIndex = SIMPLE_LIGHTMAP_COEF_INDEX;CoefficientIndex < SIMPLE_LIGHTMAP_COEF_INDEX + NUM_SIMPLE_LIGHTMAP_COEF;CoefficientIndex++)
				{
					for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
					{
						MaxSimpleLightValue[ColorIndex] = Max(MaxSimpleLightValue[ColorIndex],LightSample.Coefficients[CoefficientIndex][ColorIndex]);
					}
				}
			}

			for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
			{
				Component->DirectionalStaticLightingScale[ColorIndex] = MaxDirectionalLightValue[ColorIndex];
				Component->SimpleStaticLightingScale[ColorIndex] = MaxSimpleLightValue[ColorIndex];
			}

			// Quantize the static lighting.
			for(INT InstanceIndex = 0;InstanceIndex < Component->Instances.Num();InstanceIndex++)
			{
				// Average the instance's samples.
				FLightSample AverageSample;
				for(INT SampleIndex = 0;SampleIndex < NUM_LIGHT_SAMPLES_PER_INSTANCE;SampleIndex++)
				{
					AverageSample.AddWeighted(
						(*LightMapData)(InstanceIndex * NUM_LIGHT_SAMPLES_PER_INSTANCE + SampleIndex),
						1.0f / (FLOAT)NUM_LIGHT_SAMPLES_PER_INSTANCE
						);
				}

				for(INT CoefficientIndex = 0;CoefficientIndex < NUM_DIRECTIONAL_LIGHTMAP_COEF;CoefficientIndex++)
				{
					Component->Instances(InstanceIndex).StaticLighting[CoefficientIndex] = FColor(
						(BYTE)Clamp<INT>(appTrunc(appPow(AverageSample.Coefficients[CoefficientIndex][0] / MaxDirectionalLightValue[0],1.0f / 2.2f) * 255.0f),0,255),
						(BYTE)Clamp<INT>(appTrunc(appPow(AverageSample.Coefficients[CoefficientIndex][1] / MaxDirectionalLightValue[1],1.0f / 2.2f) * 255.0f),0,255),
						(BYTE)Clamp<INT>(appTrunc(appPow(AverageSample.Coefficients[CoefficientIndex][2] / MaxDirectionalLightValue[2],1.0f / 2.2f) * 255.0f),0,255),
						0
						);
				}

				for(INT CoefficientIndex = SIMPLE_LIGHTMAP_COEF_INDEX;CoefficientIndex < SIMPLE_LIGHTMAP_COEF_INDEX + NUM_SIMPLE_LIGHTMAP_COEF;CoefficientIndex++)
				{
					Component->Instances(InstanceIndex).StaticLighting[CoefficientIndex] = FColor(
						(BYTE)Clamp<INT>(appTrunc(appPow(AverageSample.Coefficients[CoefficientIndex][0] / MaxSimpleLightValue[0],1.0f / 2.2f) * 255.0f),0,255),
						(BYTE)Clamp<INT>(appTrunc(appPow(AverageSample.Coefficients[CoefficientIndex][1] / MaxSimpleLightValue[1],1.0f / 2.2f) * 255.0f),0,255),
						(BYTE)Clamp<INT>(appTrunc(appPow(AverageSample.Coefficients[CoefficientIndex][2] / MaxSimpleLightValue[2],1.0f / 2.2f) * 255.0f),0,255),
						0
						);
				}
			}
			
		}

		// Build the list of relevant and irrelevant light GUIDs.
		Component->StaticallyRelevantLights.Empty();
		Component->StaticallyIrrelevantLights.Empty();
		for(INT LightIndex = 0;LightIndex < RelevantLights.Num();LightIndex++)
		{
			ULightComponent* Light = RelevantLights(LightIndex);
			const UBOOL bIsRelevantLight = LightMapData && LightMapData->Lights.ContainsItem(Light);

			if(bIsRelevantLight)
			{
				Component->StaticallyRelevantLights.AddItem(Light->LightmapGuid);
			}
			else
			{
				Component->StaticallyIrrelevantLights.AddItem(Light->LightmapGuid);
			}
		}
	}

private:

	/** The foliage component this mapping represents. */
	UFoliageComponent* const Component;

	/** The lights relevant to the foliage component. */
	TArray<ULightComponent*> RelevantLights;
};

void UFoliageComponent::GetStaticLightingInfo(FStaticLightingPrimitiveInfo& OutPrimitiveInfo,const TArray<ULightComponent*>& InRelevantLights,const FLightingBuildOptions& Options)
{
	OutPrimitiveInfo.Meshes.AddItem(new FFoliageStaticLightingMesh(this,InRelevantLights));
	OutPrimitiveInfo.Mappings.AddItem(new FFoliageStaticLightingVertexMapping(this,OutPrimitiveInfo.Meshes(0),InRelevantLights));
}

void UFoliageComponent::InvalidateLightingCache()
{
	// Detach the component while the data shared with the rendering thread scene proxy is being updated.
	FComponentReattachContext ReattachContext(this);
	FlushRenderingCommands();

	// Reset the static lighting.
	StaticallyRelevantLights.Empty();
	StaticallyIrrelevantLights.Empty();
	for(INT InstanceIndex = 0;InstanceIndex < Instances.Num();InstanceIndex++)
	{
		for(INT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
		{
			Instances(InstanceIndex).StaticLighting[CoefficientIndex] = FColor(0,0,0);
		}
	}
}
