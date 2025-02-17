/*=============================================================================
	SpeedTreeComponent.cpp: SpeedTreeComponent implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EnginePhysicsClasses.h"
#include "EngineMaterialClasses.h"
#include "ScenePrivate.h"
#if WITH_NOVODEX
#include "UnNovodexSupport.h"
#endif
#include "SpeedTree.h"

IMPLEMENT_CLASS(USpeedTreeComponent);

#if WITH_SPEEDTREE

extern CSpeedTreeRT::SGeometry GSpeedTreeGeometry;

/**
 * Chooses the material to render on the tree.  The InstanceMaterial is preferred, then the ArchetypeMaterial, then the default material.
 * If a material doesn't have the necessary shaders, it falls back to the next less preferred material.
 * @return The most preferred material that has the necessary shaders for rendering a SpeedTree.
 */
static UMaterialInterface* GetSpeedTreeMaterial(UMaterialInterface* InstanceMaterial,UMaterialInterface* ArchetypeMaterial,UBOOL bHasStaticLighting,FMaterialViewRelevance& MaterialViewRelevance)
{
	UMaterialInterface* Result = NULL;

	// Try the instance's material first.
	if(InstanceMaterial && InstanceMaterial->CheckMaterialUsage(MATUSAGE_SpeedTree) && (!bHasStaticLighting || InstanceMaterial->CheckMaterialUsage(MATUSAGE_StaticLighting)))
	{
		Result = InstanceMaterial;
	}

	// If that failed, try the archetype's material.
	if(!Result && ArchetypeMaterial && ArchetypeMaterial->CheckMaterialUsage(MATUSAGE_SpeedTree) && (!bHasStaticLighting || ArchetypeMaterial->CheckMaterialUsage(MATUSAGE_StaticLighting)))
	{
		Result = ArchetypeMaterial;
	}

	// If both failed, use the default material.
	if(!Result)
	{
		Result = GEngine->DefaultMaterial;
	}

	// Update the material relevance information from the resulting material.
	MaterialViewRelevance |= Result->GetViewRelevance();

	return Result;
}

/** Represents the static lighting of a SpeedTreeComponent's mesh to the scene manager. */
class FSpeedTreeLCI : public FLightCacheInterface
{
public:

	/** Initialization constructor. */
	FSpeedTreeLCI(const USpeedTreeComponent* InComponent,ESpeedTreeMeshType InMeshType):
		Component(InComponent),
		MeshType(InMeshType)
	{}

	// FLightCacheInterface
	virtual FLightInteraction GetInteraction(const class FLightSceneInfo* LightSceneInfo) const
	{
		// Check if the light is in the light-map.
		const FLightMap* LightMap = 
			ChooseByMeshType<FLightMap*>(
				MeshType,
				Component->BranchAndFrondLightMap,
				Component->BranchAndFrondLightMap,
				Component->LeafMeshLightMap,
				Component->LeafCardLightMap,
				Component->BillboardLightMap
				);
		if(LightMap)
		{
			if(LightMap->LightGuids.ContainsItem(LightSceneInfo->LightmapGuid))
			{
				return FLightInteraction::LightMap();
			}
		}

		// Check whether we have static lighting for the light.
		for(INT LightIndex = 0;LightIndex < Component->StaticLights.Num();LightIndex++)
		{
			const FSpeedTreeStaticLight& StaticLight = Component->StaticLights(LightIndex);

			if(StaticLight.Guid == LightSceneInfo->LightGuid)
			{
				const UShadowMap1D* ShadowMap = 
					ChooseByMeshType<UShadowMap1D*>(
						MeshType,
						StaticLight.BranchAndFrondShadowMap,
						StaticLight.BranchAndFrondShadowMap,
						StaticLight.LeafMeshShadowMap,
						StaticLight.LeafCardShadowMap,
						StaticLight.BillboardShadowMap
						);

				if(ShadowMap)
				{
					return FLightInteraction::ShadowMap1D(ShadowMap);
				}
				else
				{
					return FLightInteraction::Irrelevant();
				}
			}
		}

		return FLightInteraction::Uncached();
	}
	virtual FLightMapInteraction GetLightMapInteraction() const
	{
		const FLightMap* LightMap =
			ChooseByMeshType<FLightMap*>(
				MeshType,
				Component->BranchAndFrondLightMap,
				Component->BranchAndFrondLightMap,
				Component->LeafMeshLightMap,
				Component->LeafCardLightMap,
				Component->BillboardLightMap
				);
		if(LightMap)
		{
			return LightMap->GetInteraction();
		}
		else
		{
			return FLightMapInteraction();
		}
	}

private:

	const USpeedTreeComponent* const Component;
	ESpeedTreeMeshType MeshType;
};

/** Represents the SpeedTree to the scene manager in the rendering thread. */
class FSpeedTreeSceneProxy : public FPrimitiveSceneProxy
{
public:

	FSpeedTreeSceneProxy( USpeedTreeComponent* InComponent ) 
	:	FPrimitiveSceneProxy( InComponent, InComponent->SpeedTree->GetFName() )
	,	SpeedTree( InComponent->SpeedTree )
	,	Component( InComponent )
	,	Bounds( InComponent->Bounds )
	,	RotatedLocalToWorld( InComponent->RotationOnlyMatrix.Inverse() * InComponent->LocalToWorld )
	,	RotationOnlyMatrix( InComponent->RotationOnlyMatrix )
	,	LevelColor(255,255,255)
	,	PropertyColor(255,255,255)
	,	LodNearDistance(InComponent->LodNearDistance)
	,	LodFarDistance(InComponent->LodFarDistance)
	,	LodLevelOverride(InComponent->LodLevelOverride)
	,	LodFadePercent(0.15f)
	,	bUseLeaves(InComponent->bUseLeaves)
	,	bUseBranches(InComponent->bUseBranches)
	,	bUseFronds(InComponent->bUseFronds)
	,	bUseBillboards(InComponent->bUseBillboards)
	,	bCastShadow(InComponent->CastShadow)
	,	bSelected(InComponent->IsOwnerSelected())
	,	bShouldCollide(InComponent->ShouldCollide())
	,	bBlockZeroExtent(InComponent->BlockZeroExtent)
	,	bBlockNonZeroExtent(InComponent->BlockNonZeroExtent)
	,	bBlockRigidBody(InComponent->BlockRigidBody)
	,	BranchLCI(InComponent,STMT_Branches)
	,	FrondLCI(InComponent,STMT_Fronds)
	,	LeafMeshLCI(InComponent,STMT_LeafMeshes)
	,	LeafCardLCI(InComponent,STMT_LeafCards)
	,	BillboardLCI(InComponent,STMT_Billboards)
	{
		const UBOOL bHasStaticLighting = Component->BranchAndFrondLightMap != NULL || Component->StaticLights.Num();

		// Make sure applied materials have been compiled with the speed tree vertex factory.
		BranchMaterial = GetSpeedTreeMaterial(Component->BranchMaterial,SpeedTree->BranchMaterial,bHasStaticLighting,MaterialViewRelevance);
		FrondMaterial = GetSpeedTreeMaterial(Component->FrondMaterial,SpeedTree->FrondMaterial,bHasStaticLighting,MaterialViewRelevance);
		LeafMaterial = GetSpeedTreeMaterial(Component->LeafMaterial,SpeedTree->LeafMaterial,bHasStaticLighting,MaterialViewRelevance);
		BillboardMaterial = GetSpeedTreeMaterial(Component->BillboardMaterial,SpeedTree->BillboardMaterial,bHasStaticLighting,MaterialViewRelevance);
	}

	/** Determines if any collision should be drawn for this mesh. */
	UBOOL ShouldDrawCollision(const FSceneView* View)
	{
		if((View->Family->ShowFlags & SHOW_CollisionNonZeroExtent) && bBlockNonZeroExtent && bShouldCollide)
		{
			return TRUE;
		}

		if((View->Family->ShowFlags & SHOW_CollisionZeroExtent) && bBlockZeroExtent && bShouldCollide)
		{
			return TRUE;
		}	

		if((View->Family->ShowFlags & SHOW_CollisionRigidBody) && bBlockRigidBody)
		{
			return TRUE;
		}

		return FALSE;
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		FPrimitiveViewRelevance Result;
		if(View->Family->ShowFlags & SHOW_SpeedTrees)
		{
			if( IsShown(View) )
			{
#if !FINAL_RELEASE
				if(IsCollisionView(View))
				{
					Result.bDynamicRelevance = TRUE;
					Result.bForceDirectionalLightsDynamic = TRUE;
				}
				else 
#endif
				if(
#if !FINAL_RELEASE
					IsRichView(View) || 
#endif
					HasViewDependentDPG() ||
					IsMovable()	)
				{
					Result.bDynamicRelevance = TRUE;
				}
				else
				{
					Result.bStaticRelevance = TRUE;
				}

				Result.SetDPG( GetDepthPriorityGroup(View), TRUE );

#if !FINAL_RELEASE
				if(View->Family->ShowFlags & (SHOW_Bounds|SHOW_Collision))
				{
					Result.bDynamicRelevance = TRUE;
				}

				// only add to foreground DPG for debug rendering
				if(View->Family->ShowFlags & SHOW_Bounds)
				{
					Result.SetDPG(SDPG_Foreground,TRUE);
				}
#endif
			}
			if (IsShadowCast(View))
			{
				Result.bShadowRelevance = TRUE;
			}
			
			// Replicate the material relevance flags into the resulting primitive view relevance's material flags.
			MaterialViewRelevance.SetPrimitiveViewRelevance(Result);
		}
		return Result;
	}

	virtual void GetLightRelevance(FLightSceneInfo* LightSceneInfo, UBOOL& bDynamic, UBOOL& bRelevant, UBOOL& bLightMapped)
	{
		// Use the FSpeedTreeLCI to find the light's interaction type.
		// Assume that light relevance is the same for all mesh types.
		FSpeedTreeLCI SpeedTreeLCI(Component,STMT_Branches);
		const ELightInteractionType InteractionType = SpeedTreeLCI.GetInteraction(LightSceneInfo).GetType();

		// Attach the light to the primitive's static meshes.
		bDynamic = (InteractionType == LIT_Uncached);
		bRelevant = (InteractionType != LIT_CachedIrrelevant);
		bLightMapped = (InteractionType == LIT_CachedLightMap || InteractionType == LIT_CachedIrrelevant);
	}

	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* SPDI)
	{
		// Draw the tree's mesh elements.
		DrawTreeMesh(NULL,SPDI,NULL,GetStaticDepthPriorityGroup());
	}

	/** 
	 * Draw the scene proxy as a dynamic element
	 *
	 * @param	PDI - draw interface to render to
	 * @param	View - current view
	 * @param	DPGIndex - current depth priority 
	 * @param	Flags - optional set of flags from EDrawDynamicElementFlags
	 */
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags)
	{
		const UBOOL bIsCollisionView = IsCollisionView(View);
		const UBOOL bDrawCollision =
			(bIsCollisionView && ShouldDrawCollision(View)) ||
			((View->Family->ShowFlags & SHOW_Collision) && bShouldCollide);
		const UBOOL bDrawMesh =
			!bIsCollisionView &&
			(IsRichView(View) || HasViewDependentDPG() || IsMovable());

		if(bDrawMesh && GetDepthPriorityGroup(View) == DPGIndex)
		{
			// Draw the tree's mesh elements.
			DrawTreeMesh(PDI,NULL,View,DPGIndex);
		}

		if(bDrawCollision && GetDepthPriorityGroup(View) == DPGIndex)
		{
			// Draw the speedtree's collision model
			INT		NumCollision = SpeedTree->SRH->GetNumCollisionPrimitives();
			FLOAT	UniformScale = LocalToWorld.GetAxis(0).Size();

			const UMaterialInterface* CollisionMaterialParent = GEngine->ShadedLevelColorationUnlitMaterial;
			const FColoredMaterialRenderProxy CollisionMaterial(
				CollisionMaterialParent->GetRenderProxy(bSelected),
				GEngine->C_ScaleBoxHi
				);
			const UBOOL bDrawWireframeCollision = (View->Family->ShowFlags & (SHOW_Wireframe|SHOW_Collision)) != 0;

			for( INT i=0; i<NumCollision; i++ )
			{
				CSpeedTreeRT::ECollisionObjectType Type;
				FVector Pos;
				FVector Dim;
				FVector EulerAngles;
				
				SpeedTree->SRH->GetCollisionPrimitive( i, Type, Pos, Dim, EulerAngles );
				Dim *= UniformScale;

				switch(Type)
				{
				case CSpeedTreeRT::CO_CAPSULE:
					{
						EulerAngles.X = -EulerAngles.X;
						EulerAngles.Y = -EulerAngles.Y;

						FMatrix MatUnscaledL2W(RotatedLocalToWorld);
						MatUnscaledL2W.RemoveScaling( );
						MatUnscaledL2W.SetOrigin(FVector(0.0f, 0.0f, 0.0f));
						FRotationMatrix MatRotate(FRotator::MakeFromEuler(EulerAngles));
						MatRotate *= MatUnscaledL2W;

						if(bDrawWireframeCollision)
						{
							DrawWireCylinder(
								PDI, 
								RotatedLocalToWorld.TransformFVector(Pos) + MatRotate.TransformFVector(FVector(0, 0, Dim.Y * 0.5f)), 
								MatRotate.TransformNormal(FVector(1, 0, 0)), 
								MatRotate.TransformNormal(FVector(0, 1, 0)), 
								MatRotate.TransformNormal(FVector(0, 0, 1)),
								GetSelectionColor(GEngine->C_ScaleBoxHi,bSelected),
								Dim.X,
								Dim.Y * 0.5f,
								24,
								DPGIndex
								);
						}
						else
						{
							DrawCylinder(
								PDI, 
								RotatedLocalToWorld.TransformFVector(Pos) + MatRotate.TransformFVector(FVector(0, 0, Dim.Y * 0.5f)), 
								MatRotate.TransformNormal(FVector(1, 0, 0)), 
								MatRotate.TransformNormal(FVector(0, 1, 0)), 
								MatRotate.TransformNormal(FVector(0, 0, 1)),
								Dim.X,
								Dim.Y * 0.5f,
								24,
								&CollisionMaterial,
								DPGIndex
								);
						}
					}
					break;
				case CSpeedTreeRT::CO_SPHERE:
					{
						static const UINT NumSidesPerHemicircle = 6;
						const FVector WorldSphereOrigin = RotatedLocalToWorld.TransformFVector(Pos);
						if(bDrawWireframeCollision)
						{
							DrawWireSphere(
								PDI,
								WorldSphereOrigin,
								GetSelectionColor(GEngine->C_ScaleBoxHi,bSelected),
								Dim.X,
								NumSidesPerHemicircle*2,
								DPGIndex
								);
						}
						else
						{
							DrawSphere(
								PDI,
								WorldSphereOrigin,
								FVector(Dim.X,Dim.X,Dim.X),
								NumSidesPerHemicircle,
								NumSidesPerHemicircle,
								&CollisionMaterial,
								DPGIndex
								);
						}
					}	
					break;
				case CSpeedTreeRT::CO_BOX:
					// boxes not supported
					break;
				default:
					break;
				}
			}
		}
	
		// Render the bounds if the actor is selected.
		if( (View->Family->ShowFlags & SHOW_Bounds) && bSelected && DPGIndex == SDPG_Foreground )
		{
			// Draw the tree's bounding box and sphere.
			DrawWireBox( PDI, Bounds.GetBox( ), FColor(72, 72, 255), SDPG_Foreground);
			DrawCircle( PDI, Bounds.Origin, FVector(1, 0, 0), FVector(0, 1, 0), FColor(255, 255, 0), Bounds.SphereRadius, 24, SDPG_Foreground);
			DrawCircle( PDI, Bounds.Origin, FVector(1, 0, 0), FVector(0, 0, 1), FColor(255, 255, 0), Bounds.SphereRadius, 24, SDPG_Foreground);
			DrawCircle( PDI, Bounds.Origin, FVector(0, 1, 0), FVector(0, 0, 1), FColor(255, 255, 0), Bounds.SphereRadius, 24, SDPG_Foreground);
		}
	}

	virtual DWORD GetMemoryFootprint() const
	{ 
		return (sizeof(*this) + GetAllocatedSize()); 
	}
	DWORD GetAllocatedSize() const
	{ 
		return FPrimitiveSceneProxy::GetAllocatedSize(); 
	}
	virtual EMemoryStats GetMemoryStatType() const
	{ 
		return STAT_GameToRendererMallocOther; 
	}

private:

	USpeedTree*				SpeedTree;
	USpeedTreeComponent*	Component;
	FBoxSphereBounds		Bounds;

	FMatrix RotatedLocalToWorld;
	FMatrix RotationOnlyMatrix;

	const UMaterialInterface* BranchMaterial;
	const UMaterialInterface* FrondMaterial;
	const UMaterialInterface* LeafMaterial;
	const UMaterialInterface* BillboardMaterial;

	const FColor LevelColor;
	const FColor PropertyColor;

	const FLOAT LodNearDistance;
	const FLOAT LodFarDistance;
	const FLOAT LodLevelOverride;
	const FLOAT LodFadePercent;

	const BITFIELD bUseLeaves : 1;
	const BITFIELD bUseBranches : 1;
	const BITFIELD bUseFronds : 1;
	const BITFIELD bUseBillboards : 1;

	const BITFIELD bCastShadow : 1;
	const BITFIELD bSelected : 1;
	const BITFIELD bShouldCollide : 1;
	const BITFIELD bBlockZeroExtent : 1;
	const BITFIELD bBlockNonZeroExtent : 1;
	const BITFIELD bBlockRigidBody : 1;

	/** The view relevance for all the primitive's materials. */
	FMaterialViewRelevance MaterialViewRelevance;

	/** The component's billboard mesh user data. */
	TIndirectArray<FSpeedTreeVertexFactory::MeshUserDataType> MeshUserDatas;
	
	/** The static lighting cache interface for the primitive's branches. */
	FSpeedTreeLCI BranchLCI;

	/** The static lighting cache interface for the primitive's fronds. */
	FSpeedTreeLCI FrondLCI;

	/** The static lighting cache interface for the primitive's leaf meshes. */
	FSpeedTreeLCI LeafMeshLCI;

	/** The static lighting cache interface for the primitive's leaf cards. */
	FSpeedTreeLCI LeafCardLCI;

	/** The static lighting cache interface for the primitive's billboards. */
	FSpeedTreeLCI BillboardLCI;
	
	/** Computes the distance at which the primitive will be drawn with the given LOD fraction. */
	FLOAT ComputeDistanceFromLodFraction(FLOAT LodFraction) const
	{
		return LodFarDistance + (LodNearDistance - LodFarDistance) * LodFraction;
	}

	/**
	 * Computes the distance range for a given LOD fraction.
	 * @param OutMinDistance - If TRUE is returned, contains the minimum distance that the given LOD should be drawn at.
	 * @param OutMaxDistance - If TRUE is returned, contains the maximum distance that the given LOD should be drawn at.
	 * @return TRUE if the LOD should be drawn.
	 */
	UBOOL ComputeDistanceRangeFromLodFraction(FLOAT MinLodFraction,FLOAT MaxLodFraction,FLOAT& OutMinDistance,FLOAT& OutMaxDistance) const
	{
		if(LodLevelOverride == -1.0f)
		{
			// The maximum LOD fraction corresponds to the minimum distance, and the minimum LOD fraction corresponds to the maximum distance.
			OutMinDistance = Clamp(ComputeDistanceFromLodFraction(MaxLodFraction),0.0f,(FLOAT)FLT_MAX);
			OutMaxDistance = Clamp(ComputeDistanceFromLodFraction(MinLodFraction),0.0f,(FLOAT)FLT_MAX);
			return TRUE;
		}
		else
		{
			// Only draw LODs which contain the LOD level override in their LOD fraction range.
			if(MinLodFraction < LodLevelOverride && LodLevelOverride <= MaxLodFraction)
			{
				OutMinDistance = 0.0f;
				OutMaxDistance = FLT_MAX;
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}

	enum ELODBound
	{
		LODBound_Min = 0,
		LODBound_Max = 1
	};
	/**
	 * Computes the either the minimum or maximum of the LOD fraction range of an index into a set of LODs.
	 * @param LODIndex - The index of the LOD to evaluate.
	 * @param NumLODs - The total number of LODs available.
	 * @param Bound - Specified either the minimum or maximum of the LOD fraction range.
	 * @return The minimum or maximum or the LOD fraction range of the specified LOD.
	 */
	FLOAT ComputeLodFractionFromIndex(INT LODIndex,INT NumLODs,ELODBound Bound) const
	{
		if(LODIndex == 0 && Bound == LODBound_Max)
		{
			// The highest LOD doesn't have an upper bound on its LOD fraction range.
			return BIG_NUMBER;
		}
		else
		{
			return 1.0f - FLOAT(LODIndex + 1 - Bound) / FLOAT(NumLODs);
		}
	}

	/**
	 * Draws a single LOD of a mesh.
	 * @param PDI - An optional pointer to an interface to pass the dynamic mesh elements to.
	 * @param SPDI - An optional pointer to an interface to pass the static mesh elements to.  Either PDI or SPDI must be non-NULL.
	 * @param View - An optional pointer to the current view.  If provided, only the LODs relevant in that view will be drawn, otherwise all will be.
	 * @param MeshElement - The mesh to draw.
	 * @param MinLodFraction - The lower bound of the LOD fraction range.
	 * @param MaxLodFraction - The upper bound of the LOD fraction range.
	 * @param Material - The material to apply to the mesh elements.
	 * @param LCI - The static lighting cache to use for the mesh elements.
	 * @param bUseAsOccluder - TRUE if the mesh elements may be effective occluders.
	 * @param DPGIndex - The depth priority group to draw the mesh elements in.
	 * @param MeshType - The type of SpeedTree geometry this mesh is. 
	 */
	void ConditionalDrawElement(
		FPrimitiveDrawInterface* PDI,
		FStaticPrimitiveDrawInterface* SPDI,
		const FSceneView* View,
		FMeshElement& MeshElement, 
		FLOAT MinLodFraction,
		FLOAT MaxLodFraction,
		const UMaterialInterface* Material,
		const FLightCacheInterface& LCI,
		UBOOL bUseAsOccluder,
		UINT DPGIndex,
		ESpeedTreeMeshType MeshType
		)
	{
		if(MeshElement.NumPrimitives > 0)
		{
			// Compute the distance range for this LOD.
			FLOAT MinDrawDistance = 0.0f;
			FLOAT MaxDrawDistance = 0.0f;
			if(ComputeDistanceRangeFromLodFraction(MinLodFraction,MaxLodFraction,MinDrawDistance,MaxDrawDistance))
			{
				// Compute how far from the LOD's distance range to fade it in and out.
				const FLOAT LodFadeRadius = LodFadePercent * (LodFarDistance - LodNearDistance);

				// Determine whether to draw the LOD.
				UBOOL bDrawElement = FALSE;
				if(View && View->ViewOrigin.W > 0.0f)
				{
					// If a view is provided, attempt to cull LODs that aren't visible in the view.
					const FLOAT CameraDistanceSquared = ((FVector)View->ViewOrigin - Bounds.Origin).SizeSquared();

					// Only draw this LOD if the camera is within its distance range.
					if(	Square(Max(0.0f,MinDrawDistance - LodFadeRadius)) <= CameraDistanceSquared &&
						Square(MaxDrawDistance + LodFadeRadius) >= CameraDistanceSquared)
					{
						bDrawElement = TRUE;
					}
				}
				else
				{
					// If no view is provided, always draw this LOD.
					bDrawElement = TRUE;
				}

				if(bDrawElement)
				{
					// Initialize the mesh element with this primitive's parameters.
					MeshElement.LCI = &LCI;
					MeshElement.MaterialRenderProxy = Material ? Material->GetRenderProxy(bSelected) : GEngine->DefaultMaterial->GetRenderProxy(bSelected);
					MeshElement.LocalToWorld = LocalToWorld;
					MeshElement.WorldToLocal = LocalToWorld.Inverse();
					MeshElement.DepthPriorityGroup	= DPGIndex;
					MeshElement.bUseAsOccluder = bUseAsOccluder && PrimitiveSceneInfo->bUseAsOccluder;
					MeshElement.CastShadow = bCastShadow;
					MeshElement.ReverseCulling = (MeshType != STMT_LeafCards && LocalToWorldDeterminant < 0.0f) ? TRUE : FALSE;

					// Set up the mesh user data.
					FSpeedTreeVertexFactory::MeshUserDataType MeshUserDataTemp;
					MeshUserDataTemp.BoundsOrigin = Bounds.Origin;
					MeshUserDataTemp.RotationOnlyMatrix = RotationOnlyMatrix;
					MeshUserDataTemp.WindMatrixOffset = Component->WindMatrixOffset;
					MeshUserDataTemp.LodMinDistance = MinDrawDistance;
					MeshUserDataTemp.LodMaxDistance = MaxDrawDistance;
					MeshUserDataTemp.LodFadeRadius = LodFadeRadius;

					// Draw the mesh element.
					if(PDI)
					{
						// Dynamic mesh elements are drawn immediately before returning, so we can just use the user data on the stack.
						MeshElement.UserData = &MeshUserDataTemp;

						static const FLinearColor WireframeColor(0.3f,1.0f,0.3f);
						DrawRichMesh( PDI, MeshElement, WireframeColor, LevelColor, PropertyColor, PrimitiveSceneInfo, bSelected );
					}
					else if(SPDI)
					{
						// Static mesh elements aren't drawn until after this function has returned, so add the user data to a persistent array.
						MeshElement.UserData = new(MeshUserDatas) FSpeedTreeVertexFactory::MeshUserDataType(MeshUserDataTemp);

						SPDI->DrawMesh(MeshElement,MinDrawDistance - LodFadeRadius,MaxDrawDistance + LodFadeRadius);
					}
				}
			}
		}
	}

	/**
	 * Draws a set of mesh elements corresponding to separate LODs for a specific component of the tree.
	 * @param PDI - An optional pointer to an interface to pass the dynamic mesh elements to.
	 * @param SPDI - An optional pointer to an interface to pass the static mesh elements to.  Either PDI or SPDI must be non-NULL.
	 * @param View - An optional pointer to the current view.  If provided, only the LODs relevant in that view will be drawn, otherwise all will be.
	 * @param LODMeshElements - The LOD mesh elements, ordered from highest detail to lowest.
	 * @param Material - The material to apply to the mesh elements.
	 * @param LCI - The static lighting cache to use for the mesh elements.
	 * @param bUseAsOccluder - TRUE if the mesh elements may be effective occluders.
	 * @param DPGIndex - The depth priority group to draw the mesh elements in.
	 */
	void DrawMeshElementLODs(
		FPrimitiveDrawInterface* PDI,
		FStaticPrimitiveDrawInterface* SPDI,
		const FSceneView* View,
		TArray<FMeshElement>& LODMeshElements,
		const UMaterialInterface* Material,
		const FLightCacheInterface& LCI,
		UBOOL bUseAsOccluder,
		UINT DPGIndex,
		ESpeedTreeMeshType MeshType
		)
	{
		for(INT LODIndex = 0;LODIndex < LODMeshElements.Num();LODIndex++)
		{
			ConditionalDrawElement(
				PDI,
				SPDI,
				View,
				LODMeshElements(LODIndex),
				ComputeLodFractionFromIndex(LODIndex,LODMeshElements.Num(),LODBound_Min),
				ComputeLodFractionFromIndex(LODIndex,LODMeshElements.Num(),LODBound_Max),
				Material,
				LCI,
				bUseAsOccluder,
				DPGIndex,
				MeshType
				);
		}
	}

	/**
	 * Draws the tree's mesh elements, either through the static or dynamic primitive draw interface.
	 * @param PDI - An optional pointer to an interface to pass the dynamic mesh elements to.
	 * @param SPDI - An optional pointer to an interface to pass the static mesh elements to.  Either PDI or SPDI must be non-NULL.
	 * @param View - An optional pointer to the current view.  If provided, only the LODs relevant in that view will be drawn, otherwise all will be.
	 * @param DPGIndex - The depth priority group to draw the tree in.
	 */
	void DrawTreeMesh(
		FPrimitiveDrawInterface* PDI,
		FStaticPrimitiveDrawInterface* SPDI,
		const FSceneView* View,
		UINT DPGIndex
		)
	{
		check(PDI || SPDI);

		if( bUseBillboards )
		{
			// Draw the billboard mesh element.
			ConditionalDrawElement(
				PDI,
				SPDI,
				View,
				SpeedTree->SRH->BillboardElement,
				-BIG_NUMBER,
				0.0f,
				BillboardMaterial,
				BillboardLCI,
				TRUE,
				GetStaticDepthPriorityGroup(),
				STMT_Billboards
				);
		}

		if(bUseLeaves && GSystemSettings.bAllowSpeedTreeLeaves)
		{
			// Draw the leaf card mesh elements.
			DrawMeshElementLODs(PDI,SPDI,View,SpeedTree->SRH->LeafCardElements,LeafMaterial,LeafCardLCI,TRUE,DPGIndex,STMT_LeafCards);

			// Draw the leaf mesh elements.
			DrawMeshElementLODs(PDI,SPDI,View,SpeedTree->SRH->LeafMeshElements,LeafMaterial,LeafMeshLCI,TRUE,DPGIndex,STMT_LeafMeshes);
		}

		if(bUseFronds && SpeedTree->SRH->bHasFronds && GSystemSettings.bAllowSpeedTreeFronds)
		{
			// Draw the frond mesh elements.
			DrawMeshElementLODs(PDI,SPDI,View,SpeedTree->SRH->FrondElements,FrondMaterial,FrondLCI,FALSE,DPGIndex,STMT_Fronds);
		}

		if(bUseBranches && SpeedTree->SRH->bHasBranches)
		{
			// Draw the branch mesh elements.
			DrawMeshElementLODs(PDI,SPDI,View,SpeedTree->SRH->BranchElements,BranchMaterial,BranchLCI,TRUE,DPGIndex,STMT_Branches);
		}
	}
};

FPrimitiveSceneProxy* USpeedTreeComponent::CreateSceneProxy(void)
{
	return ::new FSpeedTreeSceneProxy(this);
}

void USpeedTreeComponent::UpdateBounds( )
{
	// bounds for tree
	if( SpeedTree == NULL || !SpeedTree->IsInitialized() )
	{
		if( SpeedTreeIcon != NULL )
		{
			// bounds for icon
			const FLOAT IconScale = (Owner ? Owner->DrawScale : 1.0f) * (SpeedTreeIcon ? (FLOAT)Max(SpeedTreeIcon->SizeX, SpeedTreeIcon->SizeY) : 1.0f);
			Bounds = FBoxSphereBounds(LocalToWorld.GetOrigin( ), FVector(IconScale, IconScale, IconScale), appSqrt(3.0f * Square(IconScale)));
		}
		else
		{
			Super::UpdateBounds( );
		}
	}
	else
	{
		// speedtree bounds
		Bounds = SpeedTree->SRH->GetBounds( ).TransformBy(RotationOnlyMatrix.Inverse() * LocalToWorld);
		Bounds.BoxExtent += FVector(1.0f, 1.0f, 1.0f);
		Bounds.SphereRadius += 1.0f;
	}
}

void USpeedTreeComponent::GetStreamingTextureInfo(TArray<FStreamingTexturePrimitiveInfo>& OutStreamingTextures) const
{
	check(SpeedTree);
	check(SpeedTree->SRH);
	FSpeedTreeResourceHelper* const SRH = SpeedTree->SRH;

	// If the SpeedTree doesn't have cached texel factors, compute them now.
	if(!SRH->bHasValidTexelFactors)
	{
		SRH->bHasValidTexelFactors = TRUE;

		// Process each mesh type.
		for(INT MeshType = STMT_Min;MeshType < STMT_Max;MeshType++)
		{
			FLOAT& TexelFactor = SRH->TexelFactors[MeshType];
			TexelFactor = 0.0f;

			const UBOOL bTreeHasThisMeshType = ChooseByMeshType<UBOOL>(
				MeshType,
				SRH->bHasBranches && SRH->BranchElements.Num(),
				SRH->bHasFronds && SRH->FrondElements.Num(),
				SRH->bHasLeaves && SRH->LeafMeshElements.Num(),
				SRH->bHasLeaves && SRH->LeafCardElements.Num(),
				TRUE
				);
			if(bTreeHasThisMeshType)
			{
				const TArray<WORD>& Indices = SRH->IndexBuffer.Indices;
				const TArray<FSpeedTreeVertexPosition>& VertexPositions = ChooseByMeshType<TArray<FSpeedTreeVertexPosition> >(
					MeshType,
					SRH->BranchFrondPositionBuffer.Vertices,
					SRH->BranchFrondPositionBuffer.Vertices,
					SRH->LeafMeshPositionBuffer.Vertices,
					SRH->LeafCardPositionBuffer.Vertices,
					SRH->BillboardPositionBuffer.Vertices
					);
				const FMeshElement& MeshElement = *ChooseByMeshType<FMeshElement*>(
					MeshType,
					&SRH->BranchElements(0),
					&SRH->FrondElements(0),
					&SRH->LeafMeshElements(0),
					&SRH->LeafCardElements(0),
					&SRH->BillboardElement
					);

				// Compute the maximum texel ratio of the mesh's triangles.
				for(UINT TriangleIndex = 0;TriangleIndex < MeshElement.NumPrimitives;TriangleIndex++)
				{
					const WORD I0 = Indices(MeshElement.FirstIndex + TriangleIndex * 3 + 0);
					const WORD I1 = Indices(MeshElement.FirstIndex + TriangleIndex * 3 + 1);
					const WORD I2 = Indices(MeshElement.FirstIndex + TriangleIndex * 3 + 2);
					const FLOAT L1 = (VertexPositions(I0).Position - VertexPositions(I1).Position).Size();
					const FLOAT L2 = (VertexPositions(I0).Position - VertexPositions(I2).Position).Size();
					const FLOAT T1 = (GetSpeedTreeVertexData(SRH,MeshType,I0)->TexCoord - GetSpeedTreeVertexData(SRH,MeshType,I1)->TexCoord).Size();
					const FLOAT T2 = (GetSpeedTreeVertexData(SRH,MeshType,I0)->TexCoord - GetSpeedTreeVertexData(SRH,MeshType,I2)->TexCoord).Size();
					TexelFactor = Max(TexelFactor,Max(L1 / T1,L2 / T2));
				}
			}
		}
	}

	for(INT MeshType = STMT_Min;MeshType < STMT_Max;MeshType++)
	{
		const UBOOL bComponentUsesThisMeshType = ChooseByMeshType<UBOOL>(
			MeshType,
			bUseBranches,
			bUseFronds,
			bUseLeaves,
			bUseLeaves,
			bUseBillboards
			);
		if(bComponentUsesThisMeshType)
		{
			// Determine the texel factor this mesh type in world space.
			const FLOAT WorldTexelFactor = GetMaximumAxisScale(LocalToWorld) * SRH->TexelFactors[MeshType];

			// Determine the material used by this mesh.
			FMaterialViewRelevance UnusedMaterialViewRelevance;
			UMaterialInterface* Material = GetSpeedTreeMaterial(
				ChooseByMeshType<UMaterialInterface*>(
					MeshType,
					BranchMaterial,
					FrondMaterial,
					LeafMaterial,
					LeafMaterial,
					BillboardMaterial
					),
				ChooseByMeshType<UMaterialInterface*>(
					MeshType,
					SpeedTree->BranchMaterial,
					SpeedTree->FrondMaterial,
					SpeedTree->LeafMaterial,
					SpeedTree->LeafMaterial,
					SpeedTree->BillboardMaterial
					),
				FALSE,
				UnusedMaterialViewRelevance
				);

			// Enumerate the textures used by the material.
			TArray<UTexture*> Textures;
			Material->GetTextures(Textures);

			// Add each texture to the output with the appropriate parameters.
			for(INT TextureIndex = 0;TextureIndex < Textures.Num();TextureIndex++)
			{
				FStreamingTexturePrimitiveInfo& StreamingTexture = *new(OutStreamingTextures) FStreamingTexturePrimitiveInfo;
				StreamingTexture.Bounds = Bounds.GetSphere();
				StreamingTexture.TexelFactor = WorldTexelFactor;
				StreamingTexture.Texture = Textures(TextureIndex);
			}
		}
	}
}

UBOOL USpeedTreeComponent::PointCheck(FCheckResult& Result, const FVector& Location, const FVector& Extent, DWORD TraceFlags )
{
	if( SpeedTree == NULL || !SpeedTree->IsInitialized() )
	{
		return Super::PointCheck( Result, Location, Extent, TraceFlags );
	}

	UBOOL bReturn = FALSE;

	const FMatrix RotatedLocalToWorld = RotationOnlyMatrix.Inverse() * LocalToWorld;
	INT		NumCollision = SpeedTree->SRH->GetNumCollisionPrimitives();
	FLOAT	UniformScale = LocalToWorld.GetAxis(0).Size();

	for( INT i=0; i<NumCollision && !bReturn; i++ )
	{
		// get the collision object
		CSpeedTreeRT::ECollisionObjectType Type;
		FVector Pos;
		FVector Dim;
		FVector EulerAngles(0.0f, 0.0f, 0.0f);
		SpeedTree->SRH->GetCollisionPrimitive( i, Type, Pos, Dim, EulerAngles );
		Dim *= UniformScale;

		switch( Type )
		{
		case CSpeedTreeRT::CO_CAPSULE:
			{
				EulerAngles.X = -EulerAngles.X;
				EulerAngles.Y = -EulerAngles.Y;

				FMatrix MatUnscaledL2W(RotatedLocalToWorld);
				MatUnscaledL2W.RemoveScaling( );
				MatUnscaledL2W.SetOrigin(FVector(0.0f, 0.0f, 0.0f));
				FRotationMatrix MatRotate(FRotator::MakeFromEuler(EulerAngles));
				MatRotate *= MatUnscaledL2W;
				
				FVector Transformed = RotatedLocalToWorld.TransformFVector(Pos) + MatRotate.TransformFVector(FVector(0, 0, Dim.Y * 0.5f));

				// rotate cLocation into collision object's space
				FVector NewLocation = MatRotate.TransformFVector(Location - Transformed) + Transformed;

				// *** portions taken from UCylinderComponent::LineCheck in UnActorComponent.cpp ***
				if( Square(Transformed.Z - NewLocation.Z) < Square(Dim.Y * 0.5f + Extent.Z)
				&&	Square(Transformed.X - NewLocation.X) + Square(Transformed.Y - NewLocation.Y) < Square(Dim.X + Extent.X) )
				{
					Result.Normal = (NewLocation - Transformed).SafeNormal();

					if( Result.Normal.Z < -0.5 )
					{
						Result.Location = FVector(NewLocation.X, NewLocation.Y, Transformed.Z - Extent.Z);
					}
					else if( Result.Normal.Z > 0.5 )
					{
						Result.Location = FVector(NewLocation.X, NewLocation.Y, Transformed.Z - Extent.Z);
					}
					else
					{
						Result.Location = (NewLocation - Extent.X * (Result.Normal * FVector(1, 1, 0)).SafeNormal( )) + FVector(0, 0, NewLocation.Z);
					}

					bReturn = TRUE;
				}

				// transform back into world coordinates if needed
				if( bReturn )
				{
					Result.Location = MatRotate.InverseTransformFVectorNoScale( Result.Location - Transformed ) + Transformed;
					Result.Normal	= MatRotate.InverseTransformFVectorNoScale( Result.Normal);
				}

			}
			break;
		case CSpeedTreeRT::CO_SPHERE:
			{
				// check the point in the sphere
				FVector Transformed = RotatedLocalToWorld.TransformFVector(Pos);
				if( (Location - Transformed).SizeSquared( ) < Dim.X * Dim.X)
				{
					Result.Normal	= (Location - Transformed).SafeNormal();
					Result.Location = Result.Normal * Dim.X;
					bReturn = true;
				}
			}			
			break;
		case CSpeedTreeRT::CO_BOX:
			// boxes not supported
			break;
		default:
			break;
		}
	}

	// other fcheckresult stuff
	if( bReturn )
	{
		Result.Material		= NULL;
		Result.Actor		= Owner;
		Result.Component	= this;
	}

	return !bReturn;
}

UBOOL USpeedTreeComponent::LineCheck(FCheckResult& Result, const FVector& End, const FVector& Start, const FVector& Extent, DWORD TraceFlags)
{
	if( SpeedTree == NULL || !SpeedTree->IsInitialized() )
	{
		return Super::LineCheck( Result, End, Start, Extent, TraceFlags );
	}

	UBOOL bReturn = FALSE;

	const FMatrix RotatedLocalToWorld = RotationOnlyMatrix.Inverse() * LocalToWorld;
	FLOAT	UniformScale = LocalToWorld.GetAxis(0).Size();
	FMatrix MatUnscaledL2W(RotatedLocalToWorld);
	MatUnscaledL2W.RemoveScaling( );
	MatUnscaledL2W.SetOrigin(FVector(0.0f, 0.0f, 0.0f));

	INT NumCollision = SpeedTree->SRH->GetNumCollisionPrimitives();
	for( INT i=0; i<NumCollision && !bReturn; i++ )
	{
		// get the collision primitive
		CSpeedTreeRT::ECollisionObjectType Type;
		FVector Pos;
		FVector Dim;
		FVector EulerAngles;
		SpeedTree->SRH->GetCollisionPrimitive( i, Type, Pos, Dim, EulerAngles );
		Dim *= UniformScale;

		switch( Type )
		{
		case CSpeedTreeRT::CO_CAPSULE:
			{
				EulerAngles.X = -EulerAngles.X;
				EulerAngles.Y = -EulerAngles.Y;

				FRotationMatrix MatRotate(FRotator::MakeFromEuler(EulerAngles));
				MatRotate *= MatUnscaledL2W;

				const FVector WorldCylinderOrigin = RotatedLocalToWorld.TransformFVector(Pos) + MatRotate.TransformFVector(FVector(0, 0, Dim.Y * 0.5f));

				// rotate start/end into collision object's space
				const FVector NewStart = MatRotate.InverseTransformFVectorNoScale(Start - WorldCylinderOrigin);
				const FVector NewEnd = MatRotate.InverseTransformFVectorNoScale(End - WorldCylinderOrigin);

				// *** portions taken from UCylinderComponent::LineCheck in UnActorComponent.cpp ***
				Result.Time = 1.0f;

				// Treat this actor as a cylinder.
				const FVector CylExtent(Dim.X, Dim.X, Dim.Y * 0.5f);
				const FVector NetExtent = Extent + CylExtent;

				// Quick X reject.
				const FLOAT MaxX = +NetExtent.X;
				if( NewStart.X > MaxX && NewEnd.X > MaxX )
				{
					break;
				}

				const FLOAT MinX = -NetExtent.X;
				if( NewStart.X < MinX && NewEnd.X < MinX )
				{
					break;
				}

				// Quick Y reject.
				const FLOAT MaxY = +NetExtent.Y;
				if( NewStart.Y > MaxY && NewEnd.Y > MaxY )
				{
					break;
				}

				const FLOAT MinY = -NetExtent.Y;
				if( NewStart.Y < MinY && NewEnd.Y < MinY )
				{
					break;
				}

				// Quick Z reject.
				const FLOAT TopZ = +NetExtent.Z;
				if( NewStart.Z > TopZ && NewEnd.Z > TopZ )
				{
					break;
				}

				const FLOAT BotZ = -NetExtent.Z;
				if( NewStart.Z < BotZ && NewEnd.Z < BotZ )
				{
					break;
				}

				// Clip to top of cylinder.
				FLOAT T0 = 0.0f;
				FLOAT T1 = 1.0f;
				if( NewStart.Z > TopZ && NewEnd.Z < TopZ )
				{
					FLOAT T = (TopZ - NewStart.Z) / (NewEnd.Z - NewStart.Z);
					if( T > T0 )
					{
						T0 = ::Max(T0, T);
						Result.Normal = FVector(0, 0, 1);
					}
				}
				else if( NewStart.Z < TopZ && NewEnd.Z > TopZ )
				{
					T1 = ::Min(T1, (TopZ - NewStart.Z) / (NewEnd.Z - NewStart.Z));
				}

				// Clip to bottom of cylinder.
				if( NewStart.Z < BotZ && NewEnd.Z > BotZ )
				{
					FLOAT T = (BotZ - NewStart.Z) / (NewEnd.Z - NewStart.Z);
					if( T > T0 )
					{
						T0 = ::Max(T0, T);
						Result.Normal = FVector(0, 0, -1);
					}
				}
				else if( NewStart.Z > BotZ && NewEnd.Z < BotZ )
				{
					T1 = ::Min(T1, (BotZ - NewStart.Z) / (NewEnd.Z - NewStart.Z));
				}

				// Reject.
				if (T0 >= T1)
				{
					break;
				}

				// Test setup.
				FLOAT   Kx        = NewStart.X;
				FLOAT   Ky        = NewStart.Y;

				// 2D circle clip about origin.
				FLOAT   Vx        = NewEnd.X - NewStart.X;
				FLOAT   Vy        = NewEnd.Y - NewStart.Y;
				FLOAT   A         = Vx * Vx + Vy * Vy;
				FLOAT   B         = 2.0f * (Kx * Vx + Ky * Vy);
				FLOAT   C         = Kx * Kx + Ky * Ky - Square(NetExtent.X);
				FLOAT   Discrim   = B * B - 4.0f * A * C;

				// If already inside sphere, oppose further movement inward.
				FVector LocalHitLocation;
				FVector LocalHitNormal(0,0,1);
				if( C < Square(1.0f) && NewStart.Z > BotZ && NewStart.Z < TopZ )
				{
					const FVector DirXY(
						NewEnd.X - NewStart.X,
						NewEnd.Y - NewStart.Y,
						0
						);
					FLOAT Dir = DirXY | NewStart;
					if( Dir < -0.01f )
					{
						Result.Time		= 0.0f;

						LocalHitLocation = NewStart;
						LocalHitNormal = NewStart * FVector(1, 1, 0);
							
						// transform back into world coordinates
						Result.Location = WorldCylinderOrigin + MatRotate.TransformFVector(LocalHitLocation);
						Result.Normal = MatRotate.TransformNormal(LocalHitNormal).SafeNormal();

						bReturn = TRUE;

						break;
					}
					else
					{
						break;
					}
				}

				// No intersection if discriminant is negative.
				if( Discrim < 0 )
				{
					break;
				}

				// Unstable intersection if velocity is tiny.
				if( A < Square(0.0001f) )
				{
					// Outside.
					if( C > 0 )
					{
						break;
					}
					else
					{
						LocalHitNormal = NewStart;
						LocalHitNormal.Z = 0;
					}
				}
				else
				{
					// Compute intersection times.
					Discrim = appSqrt(Discrim);
					FLOAT R2A = 0.5 / A;
					T1 = ::Min(T1, +(Discrim - B) * R2A);
					FLOAT T = -(Discrim + B) * R2A;
					if (T > T0)
					{
						T0 = T;
						LocalHitNormal = NewStart + (NewEnd - NewStart) * T0;
						LocalHitNormal.Z = 0;
					}
					if( T0 >= T1 )
					{
						break;
					}
				}
				Result.Time = Clamp(T0 - 0.001f, 0.0f, 1.0f);
				LocalHitLocation = NewStart + (NewEnd - NewStart) * Result.Time;

				// transform back into world coordinates
				Result.Location = WorldCylinderOrigin + MatRotate.TransformFVector(LocalHitLocation);
				Result.Normal = MatRotate.TransformNormal(LocalHitNormal).SafeNormal();

				bReturn = TRUE;
			}
			break;
		case CSpeedTreeRT::CO_SPHERE:
			{
				FVector Transformed = RotatedLocalToWorld.TransformFVector(Pos);

				// check the line through the sphere
				// *** portions taken from FLineSphereIntersection in UnMath.h ***

				// Check if the start is inside the sphere.
				const FVector RelativeStart = Start - Transformed;
				const FLOAT StartDistanceSquared = (RelativeStart | RelativeStart) - Square(Dim.X);
				if( StartDistanceSquared < 0.0f )
				{
					Result.Time		= 0.0f;
					Result.Location = Start;
					Result.Normal	= (Start - Transformed).SafeNormal();
					Result			= TRUE;
					break;
				}
				
				// Calculate the discriminant for the line-sphere intersection quadratic formula.
				const FVector Dir = End - Start;
				const FLOAT DirSizeSquared = Dir.SizeSquared();

				// If the line's length is very small, the intersection position will be unstable;
				// in this case we rely on the above "start is inside sphere" check.
				if(DirSizeSquared >= DELTA)
				{
					const FLOAT B = 2.0f * (RelativeStart | Dir);
					const FLOAT Discriminant = Square(B) - 4.0f * DirSizeSquared * StartDistanceSquared;

					// If the discriminant is non-negative, then the line intersects the sphere.
					if( Discriminant >= 0 )
					{
						Result.Time = (-B - appSqrt(Discriminant)) / (2.0f * DirSizeSquared);
						if( Result.Time >= 0.0f && Result.Time <= 1.0f )
						{
							Result.Location = Start + Dir * Result.Time;
							Result.Normal	= (Result.Location - Transformed).SafeNormal();
							bReturn			= TRUE;
							break;
						}
					}
				}
			}	
			break;
		case CSpeedTreeRT::CO_BOX:
			// boxes not supported
			break;
		default:
			break;
		}
	}

	// other fcheckresult stuff
	if( bReturn )
	{
		Result.Material		= NULL;
		Result.Actor		= Owner;
		Result.Component	= this;
	}

	return !bReturn;
}


#if WITH_NOVODEX
void USpeedTreeComponent::InitComponentRBPhys(UBOOL /*bFixed*/)
{
	// Don't create physics body at all if no collision (makes assumption it can't change at run time).
	if( !BlockRigidBody)
	{
		return;
	}

	if( GWorld->RBPhysScene && SpeedTree && SpeedTree->IsInitialized() )
	{
		// make novodex info
		NxActorDesc nxActorDesc;
		nxActorDesc.setToDefault( );

		NxMat33 MatIdentity;
		MatIdentity.id();

		const FMatrix RotatedLocalToWorld = RotationOnlyMatrix.Inverse() * LocalToWorld;
		INT NumPrimitives = SpeedTree->SRH->GetNumCollisionPrimitives();
		FLOAT UniformScale = LocalToWorld.GetAxis(0).Size();

		for( INT i=0; i<NumPrimitives; i++ )
		{
			CSpeedTreeRT::ECollisionObjectType Type;
			FVector Pos;
			FVector Dim;
			FVector EulerAngles;
			SpeedTree->SRH->GetCollisionPrimitive( i, Type, Pos, Dim, EulerAngles );

			switch( Type )
			{
			case CSpeedTreeRT::CO_SPHERE:
				{
					NxSphereShapeDesc* SphereDesc = new NxSphereShapeDesc;
					SphereDesc->setToDefault( );
					SphereDesc->radius = Dim.X * UniformScale * U2PScale;
					SphereDesc->localPose = NxMat34(MatIdentity, U2NPosition(RotatedLocalToWorld.TransformFVector(Pos)));
					nxActorDesc.shapes.pushBack(SphereDesc);
				}
				break;
			case CSpeedTreeRT::CO_CAPSULE:
				{
					EulerAngles.X = -EulerAngles.X;
					EulerAngles.Y = -EulerAngles.Y;
					Dim *= UniformScale;

					FMatrix MatUnscaledL2W(RotatedLocalToWorld);
					MatUnscaledL2W.RemoveScaling( );
					MatUnscaledL2W.SetOrigin(FVector(0.0f, 0.0f, 0.0f));
					FRotationMatrix MatRotate(FRotator::MakeFromEuler(EulerAngles));
					MatRotate *= MatUnscaledL2W;

					Pos = RotatedLocalToWorld.TransformFVector(Pos) + MatRotate.TransformFVector(FVector(0, 0, Dim.Y * 0.5f));
					FRotationMatrix MatPostRotate(FRotator(0, 0, 16384));
					FMatrix MatTransform = MatPostRotate * MatRotate;
					MatTransform.SetOrigin(Pos);

					NxCapsuleShapeDesc* CapsuleShape = new NxCapsuleShapeDesc;
					CapsuleShape->setToDefault( );
					CapsuleShape->radius = Dim.X * U2PScale;
					CapsuleShape->height = Dim.Y * U2PScale;
					CapsuleShape->localPose = U2NTransform(MatTransform);
					nxActorDesc.shapes.pushBack(CapsuleShape);
				}
				break;
			case CSpeedTreeRT::CO_BOX:
				// boxes not suported
				break;
			default:
				break;
			};
		}
		
		if( nxActorDesc.isValid() && nxActorDesc.shapes.size( ) > 0 && GWorld->RBPhysScene )
		{
			NxScene* NovodexScene = GWorld->RBPhysScene->GetNovodexPrimaryScene();
			check(NovodexScene);
			NxActor* nxActor = NovodexScene->createActor(nxActorDesc);
			
			if( nxActor )
			{
				BodyInstance = GWorld->InstanceRBBody(NULL);
				BodyInstance->BodyData			= (FPointer)nxActor;
				BodyInstance->OwnerComponent	= this;
				nxActor->userData				= BodyInstance;
				BodyInstance->SceneIndex		= GWorld->RBPhysScene->NovodexSceneIndex;
			}
		}
			
		while(!nxActorDesc.shapes.isEmpty())
		{
			NxShapeDesc* Shape = nxActorDesc.shapes.back();
			nxActorDesc.shapes.popBack();
			delete Shape;
		};
	}

	UpdatePhysicsToRBChannels();
}
#endif // WITH_NOVODEX

#endif // #if WITH_SPEEDTREE

void USpeedTreeComponent::SetParentToWorld(const FMatrix& ParentToWorld)
{
#if WITH_SPEEDTREE
	// Don't allow the tree to be rotated if it has billboard leaves.
	if(IsValidComponent())
	{
		// Compute a rotation-less parent to world matrix.
		RotationOnlyMatrix = ParentToWorld;
		RotationOnlyMatrix.RemoveScaling();
		RotationOnlyMatrix.SetOrigin(FVector(0,0,0));
		RotationOnlyMatrix = RotationOnlyMatrix.Inverse();
		const FMatrix RotationlessParentToWorld = RotationOnlyMatrix * ParentToWorld;

		// Pass the rotation-less matrix to UPrimitiveComponent::SetParentToWorld.
		Super::SetParentToWorld(RotationlessParentToWorld);
	}
	else
#endif
	{
		Super::SetParentToWorld(ParentToWorld);
	}
}

UBOOL USpeedTreeComponent::IsValidComponent() const
{
#if WITH_SPEEDTREE
	// Only allow the component to be attached if it has a valid SpeedTree reference.
	return SpeedTree != NULL && SpeedTree->IsInitialized() && SpeedTree->SRH != NULL && SpeedTree->SRH->bIsInitialized && Super::IsValidComponent();
#else
	return FALSE;
#endif
}

UBOOL USpeedTreeComponent::AreNativePropertiesIdenticalTo(UComponent* Other) const
{
	UBOOL bNativePropertiesAreIdentical = Super::AreNativePropertiesIdenticalTo( Other );
	USpeedTreeComponent* OtherSpeedTreeComponent = CastChecked<USpeedTreeComponent>(Other);

	if( bNativePropertiesAreIdentical )
	{
		// Components are not identical if they have lighting information.
		if( StaticLights.Num() ||
			BranchAndFrondLightMap ||
			LeafCardLightMap ||
			LeafMeshLightMap || 
			BillboardLightMap ||
			OtherSpeedTreeComponent->StaticLights.Num() ||
			OtherSpeedTreeComponent->BranchAndFrondLightMap ||
			OtherSpeedTreeComponent->LeafCardLightMap ||
			OtherSpeedTreeComponent->LeafMeshLightMap || 
			OtherSpeedTreeComponent->BillboardLightMap)
		{
			bNativePropertiesAreIdentical = FALSE;
		}
	}

	return bNativePropertiesAreIdentical;
}

void USpeedTreeComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// Serialize the component's static lighting.
	Ar << BranchAndFrondLightMap << LeafCardLightMap << BillboardLightMap;
	Ar << LeafMeshLightMap;
}

void USpeedTreeComponent::PostLoad()
{
	Super::PostLoad();

	// Randomly permute the global wind matrices for each component.
	WindMatrixOffset = (FLOAT)(appRand() % 3);

	// Initialize the light-map resources.
	if(BranchAndFrondLightMap)
	{
		BranchAndFrondLightMap->InitResources();
	}
	if(LeafMeshLightMap)
	{
		LeafMeshLightMap->InitResources();
	}
	if(LeafCardLightMap)
	{
		LeafCardLightMap->InitResources();
	}
	if(BillboardLightMap)
	{
		BillboardLightMap->InitResources();
	}
}

void USpeedTreeComponent::PostEditChange(UProperty* PropertyThatChanged)
{
#if WITH_SPEEDTREE
	// make sure Lod level is valid
	if (LodLevelOverride > 1.0f)
	{
		LodLevelOverride = 1.0f;
	}

	if (LodLevelOverride < 0.0f && LodLevelOverride != -1.0f)
	{
		LodLevelOverride = -1.0f;
	}

#endif
	Super::PostEditChange(PropertyThatChanged);
}

void USpeedTreeComponent::PostEditUndo()
{
	// Initialize the light-map resources.
	if(BranchAndFrondLightMap)
	{
		BranchAndFrondLightMap->InitResources();
	}
	if(LeafMeshLightMap)
	{
		LeafMeshLightMap->InitResources();
	}
	if(LeafCardLightMap)
	{
		LeafCardLightMap->InitResources();
	}
	if(BillboardLightMap)
	{
		BillboardLightMap->InitResources();
	}

	Super::PostEditUndo();
}

USpeedTreeComponent::USpeedTreeComponent()
{
	// Randomly permute the global wind matrices for each component.
	WindMatrixOffset = (FLOAT)(appRand() % 3);
}
