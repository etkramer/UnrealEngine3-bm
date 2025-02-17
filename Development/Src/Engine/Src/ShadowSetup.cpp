/*=============================================================================
	ShadowSetup.cpp: Dynamic shadow setup implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

/**
 * Helper function to determine fade alpha value for shadows based on resolution. In the below ASCII art (1) is
 * the MinShadowResolution and (2) is the ShadowFadeResolution. Alpha will be 0 below the min resolution and 1
 * above the fade resolution. In between it is going to be an exponential curve with the values between (1) and (2)
 * being normalized in the 0..1 range.
 *
 *  
 *  |    /-------
 *  |  /
 *  |/
 *  1-----2-------
 *
 * @param	MaxUnclampedResolution		Requested resolution, unclamped so it can be below min
 * @param	ShadowFadeResolution		Resolution at which fade begins
 * @param	MinShadowResolution			Minimum resolution of shadow
 *
 * @return	fade value between 0 and 1
 */
static FLOAT CalculateShadowFadeAlpha( INT MaxUnclampedResolution, INT ShadowFadeResolution, INT MinShadowResolution )
{
	FLOAT FadeAlpha = 0;
	// Shadow size is above fading resolution.
	if( MaxUnclampedResolution > ShadowFadeResolution )
	{
		FadeAlpha = 1;
	}
	// Shadow size is below fading resolution but above min resolution.
	else if( MaxUnclampedResolution > MinShadowResolution )
	{
		const FLOAT SizeRatio = (FLOAT)(MaxUnclampedResolution - MinShadowResolution) / (ShadowFadeResolution - MinShadowResolution);
		FadeAlpha = appPow( SizeRatio, GSystemSettings.ShadowFadeExponent );
	}
	return FadeAlpha;
}

typedef TArray<FVector,TInlineAllocator<8> > FBoundingBoxVertexArray;

/** Stores the indices for an edge of a bounding volume. */
struct FBoxEdge
{
	WORD FirstEdgeIndex;
	WORD SecondEdgeIndex;
	FBoxEdge(WORD InFirst, WORD InSecond) :
		FirstEdgeIndex(InFirst),
		SecondEdgeIndex(InSecond)
	{}
};

typedef TArray<FBoxEdge,TInlineAllocator<12> > FBoundingBoxEdgeArray;

/**
 * Creates an array of vertices and edges for a bounding box.
 * @param Box - The bounding box
 * @param OutVertices - Upon return, the array will contain the vertices of the bounding box.
 * @param OutEdges - Upon return, will contain indices of the edges of the bounding box.
 */
static void GetBoundingBoxVertices(const FBox& Box,FBoundingBoxVertexArray& OutVertices, FBoundingBoxEdgeArray& OutEdges)
{
	OutVertices.Empty();
	for(INT X = 0;X < 2;X++)
	{
		for(INT Y = 0;Y < 2;Y++)
		{
			for(INT Z = 0;Z < 2;Z++)
			{
				OutVertices.AddItem(FVector(
					X ? Box.Min.X : Box.Max.X,
					Y ? Box.Min.Y : Box.Max.Y,
					Z ? Box.Min.Z : Box.Max.Z
					));
			}
		}
	}

	OutEdges.Empty();
	for(WORD X = 0;X < 2;X++)
	{
		WORD BaseIndex = X * 4;
		OutEdges.AddItem(FBoxEdge(BaseIndex, BaseIndex + 1));
		OutEdges.AddItem(FBoxEdge(BaseIndex + 1, BaseIndex + 3));
		OutEdges.AddItem(FBoxEdge(BaseIndex + 3, BaseIndex + 2));
		OutEdges.AddItem(FBoxEdge(BaseIndex + 2, BaseIndex));
	}
	for(WORD XEdge = 0;XEdge < 4;XEdge++)
	{
		OutEdges.AddItem(FBoxEdge(XEdge, XEdge + 4));
	}
}

/**
 * Computes the transform contains a set of bounding box vertices and minimizes the pre-transform volume inside the post-transform clip space.
 * @param ZAxis - The Z axis of the transform.
 * @param Points - The points that represent the bounding volume.
 * @param Edges - The edges of the bounding volume.
 * @param OutAspectRatio - Upon successful return, contains the aspect ratio of the AABB; the ratio of width:height.
 * @param OutTransform - Upon successful return, contains the transform.
 * @return TRUE if it successfully found a non-zero area projection of the bounding points.
 */
static UBOOL GetBestShadowTransform(const FVector& ZAxis,const FBoundingBoxVertexArray& Points, const FBoundingBoxEdgeArray& Edges, FLOAT& OutAspectRatio,FMatrix& OutTransform)
{
	// Find the axis parallel to the edge between any two boundary points with the smallest projection of the bounds onto the axis.
	FVector XAxis(0,0,0);
	FVector YAxis(0,0,0);
	FVector Translation(0,0,0);
	FLOAT BestProjectedExtent = FLT_MAX;
	UBOOL bValidProjection = FALSE;

	for(INT EdgeIndex = 0;EdgeIndex < Edges.Num();EdgeIndex++)
	{
		const FVector& Point = Points(Edges(EdgeIndex).FirstEdgeIndex);
		const FVector& OtherPoint = Points(Edges(EdgeIndex).SecondEdgeIndex);
		const FVector PointDelta = OtherPoint - Point;
		const FVector TrialXAxis = (PointDelta - ZAxis * (PointDelta | ZAxis)).SafeNormal();
		const FVector TrialYAxis = (ZAxis ^ TrialXAxis).SafeNormal();

		// Calculate the size of the projection of the bounds onto this axis and an axis orthogonal to it and the Z axis.
		FLOAT MinProjectedX = FLT_MAX;
		FLOAT MaxProjectedX = -FLT_MAX;
		FLOAT MinProjectedY = FLT_MAX;
		FLOAT MaxProjectedY = -FLT_MAX;
		for(INT ProjectedPointIndex = 0;ProjectedPointIndex < Points.Num();ProjectedPointIndex++)
		{
			const FLOAT ProjectedX = Points(ProjectedPointIndex) | TrialXAxis;
			MinProjectedX = Min(MinProjectedX,ProjectedX);
			MaxProjectedX = Max(MaxProjectedX,ProjectedX);
			const FLOAT ProjectedY = Points(ProjectedPointIndex) | TrialYAxis;
			MinProjectedY = Min(MinProjectedY,ProjectedY);
			MaxProjectedY = Max(MaxProjectedY,ProjectedY);
		}

		const FLOAT ProjectedExtentX = (MaxProjectedX - MinProjectedX);
		const FLOAT ProjectedExtentY = (MaxProjectedY - MinProjectedY);
		const FLOAT ProjectedExtent = ProjectedExtentX * ProjectedExtentY;
		if(ProjectedExtent < BestProjectedExtent)
		{
			bValidProjection = TRUE;
			BestProjectedExtent = ProjectedExtent;
			XAxis = TrialXAxis * 2.0f / ProjectedExtentX;
			YAxis = TrialYAxis * 2.0f / ProjectedExtentY;
			Translation.X = (MinProjectedX + MaxProjectedX) * 0.5f;
			Translation.Y = (MinProjectedY + MaxProjectedY) * 0.5f;
			if(ProjectedExtentY > ProjectedExtentX)
			{
				// Always make the X axis the largest one.
				Exchange(XAxis,YAxis);
				Exchange(Translation.X,Translation.Y);
				XAxis *= -1.0f;
				Translation.X *= -1.0f;
				OutAspectRatio = ProjectedExtentY / ProjectedExtentX;
			}
			else
			{
				OutAspectRatio = ProjectedExtentX / ProjectedExtentY;
			}
		}
	}

	// Only create the shadow if the projected extent of the given points has a non-zero area.
	if(bValidProjection && BestProjectedExtent > DELTA)
	{
		OutTransform = FBasisVectorMatrix(XAxis,YAxis,ZAxis,FVector(0,0,0)) * FTranslationMatrix(Translation);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/** A transform the remaps depth and potentially projects onto some plane. */
struct FShadowProjectionMatrix: FMatrix
{
	FShadowProjectionMatrix(FLOAT MinZ,FLOAT MaxZ,const FVector4& WAxis):
		FMatrix(
			FPlane(1,	0,	0,													WAxis.X),
			FPlane(0,	1,	0,													WAxis.Y),
			FPlane(0,	0,	(WAxis.Z * MaxZ + WAxis.W) / (MaxZ - MinZ),			WAxis.Z),
			FPlane(0,	0,	-MinZ * (WAxis.Z * MaxZ + WAxis.W) / (MaxZ - MinZ),	WAxis.W)
			)
	{}
};

UBOOL FProjectedShadowInitializer::CalcTransforms(
	const FVector& InPreShadowTranslation,
	const FMatrix& WorldToLight,
	const FVector& FaceDirection,
	const FBoxSphereBounds& SubjectBounds,
	const FVector4& WAxis,
	FLOAT MinLightW,
	FLOAT MaxLightW,
	UBOOL bInDirectionalLight
	)
{
	PreShadowTranslation = InPreShadowTranslation;
	bDirectionalLight = bInDirectionalLight;

	// Create an array of the extreme vertices of the subject's bounds.
	FBoundingBoxVertexArray BoundsPoints;
	FBoundingBoxEdgeArray BoundsEdges;
	GetBoundingBoxVertices(SubjectBounds.GetBox(),BoundsPoints,BoundsEdges);

	// Project the bounding box vertices.
	FBoundingBoxVertexArray ProjectedBoundsPoints;
	for(INT PointIndex = 0;PointIndex < BoundsPoints.Num();PointIndex++)
	{
		const FVector TransformedBoundsPoint = WorldToLight.TransformFVector(BoundsPoints(PointIndex));
		const FLOAT TransformedBoundsPointW = Dot4(FVector4(0,0,TransformedBoundsPoint | FaceDirection,1),WAxis);
		if(TransformedBoundsPointW >= DELTA)
		{
			ProjectedBoundsPoints.AddItem(TransformedBoundsPoint / TransformedBoundsPointW);
		}
		else
		{
			ProjectedBoundsPoints.AddItem(FVector(FLT_MAX, FLT_MAX, FLT_MAX));
		}
	}

	// Compute the transform from light-space to shadow-space.
	FMatrix LightToShadow;
	if(GetBestShadowTransform(FaceDirection.SafeNormal(),ProjectedBoundsPoints,BoundsEdges,AspectRatio,LightToShadow))
	{
		const FMatrix WorldToShadow = WorldToLight * LightToShadow;

		const FBox ShadowSubjectBounds = SubjectBounds.GetBox().TransformBy(WorldToShadow);
		const FLOAT ClampedMinSubjectZ = Max(MinLightW,ShadowSubjectBounds.Min.Z);

		PreSubjectMatrix = WorldToShadow * FShadowProjectionMatrix(MinLightW,ShadowSubjectBounds.Max.Z,WAxis);
		SubjectMatrix = WorldToShadow * FShadowProjectionMatrix(ClampedMinSubjectZ,ShadowSubjectBounds.Max.Z,WAxis);
		PostSubjectMatrix = WorldToShadow * FShadowProjectionMatrix(ClampedMinSubjectZ,MaxLightW,WAxis);

		MaxSubjectDepth = SubjectBounds.GetBox().TransformBy(SubjectMatrix).Max.Z;

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

FProjectedShadowInfo::FProjectedShadowInfo(
	FLightSceneInfo* InLightSceneInfo,
	const FPrimitiveSceneInfo* InParentSceneInfo,
	const FLightPrimitiveInteraction* const InParentInteraction,
	const FProjectedShadowInitializer& Initializer,
	UBOOL bInPreShadow,
	UINT InResolutionX,
	UINT InResolutionY,
	FLOAT InFadeAlpha
	):
	LightSceneInfo(InLightSceneInfo),
	LightSceneInfoCompact(InLightSceneInfo),
	ParentSceneInfo(InParentSceneInfo),
	ParentInteraction(InParentInteraction),
	ShadowId(INDEX_NONE),
	PreShadowTranslation(Initializer.PreShadowTranslation),
	SubjectAndReceiverMatrix(Initializer.SubjectMatrix),
	MaxSubjectDepth(Initializer.MaxSubjectDepth),
	ResolutionX(InResolutionX),
	ResolutionY(InResolutionY),
	FadeAlpha(InFadeAlpha),
	bAllocated(FALSE),
	bRendered(FALSE),
	bDirectionalLight(Initializer.bDirectionalLight),
	bPreShadow(bInPreShadow),
	bCullShadowOnBackfacesAndEmissive(
		GSystemSettings.bAllowBetterModulatedShadows &&
		InLightSceneInfo->LightShadowMode == LightShadow_ModulateBetter
		),
	bForegroundCastingOnWorld(FALSE),
	bSelfShadowOnly(InParentSceneInfo->bSelfShadowOnly)
{
	if(bPreShadow)
	{
		SubjectMatrix = Initializer.PreSubjectMatrix;
		ReceiverMatrix = Initializer.SubjectMatrix;
	}
	else
	{
		SubjectMatrix = Initializer.SubjectMatrix;
		ReceiverMatrix = Initializer.PostSubjectMatrix;
	}

	InvReceiverMatrix = ReceiverMatrix.Inverse();
	GetViewFrustumBounds(SubjectFrustum,SubjectMatrix,TRUE);
	GetViewFrustumBounds(ReceiverFrustum,ReceiverMatrix,TRUE);
}

FProjectedShadowInfo::FProjectedShadowInfo(
	FLightSceneInfo* InLightSceneInfo,
	const FProjectedShadowInitializer& Initializer,
	UINT InResolutionX,
	UINT InResolutionY,
	FLOAT InFadeAlpha
	):
	LightSceneInfo(InLightSceneInfo),
	LightSceneInfoCompact(InLightSceneInfo),
	ParentSceneInfo(NULL),
	ParentInteraction(NULL),
	ShadowId(INDEX_NONE),
	PreShadowTranslation(Initializer.PreShadowTranslation),
	SubjectAndReceiverMatrix(Initializer.SubjectMatrix),
	MaxSubjectDepth(Initializer.MaxSubjectDepth),
	ResolutionX(InResolutionX),
	ResolutionY(InResolutionY),
	FadeAlpha(InFadeAlpha),
	bAllocated(FALSE),
	bRendered(FALSE),
	bDirectionalLight(Initializer.bDirectionalLight),
	bPreShadow(FALSE),
	bCullShadowOnBackfacesAndEmissive(
		GSystemSettings.bAllowBetterModulatedShadows &&
		InLightSceneInfo->LightShadowMode == LightShadow_ModulateBetter
		),
	bForegroundCastingOnWorld(FALSE),
	bSelfShadowOnly(FALSE)
{
	SubjectMatrix = Initializer.SubjectMatrix;
	ReceiverMatrix = Initializer.PostSubjectMatrix;
	InvReceiverMatrix = ReceiverMatrix.Inverse();
	GetViewFrustumBounds(SubjectFrustum,SubjectMatrix,TRUE);
	GetViewFrustumBounds(ReceiverFrustum,ReceiverMatrix,TRUE);
}

void FProjectedShadowInfo::AddSubjectPrimitive(FPrimitiveSceneInfo* PrimitiveSceneInfo)
{
	if(!PrimitiveSceneInfo->StaticMeshes.Num())
	{
		// Add the primitive to the subject primitive list.
		SubjectPrimitives.AddItem(PrimitiveSceneInfo);
		// Logical or together all the subject's bSelfShadowOnly options
		// If any of the subjects have the option, the shadow will be rendered with it
		bSelfShadowOnly = bSelfShadowOnly || PrimitiveSceneInfo->bSelfShadowOnly;
	}
	else
	{
		// Add the primitive's static mesh elements to the draw lists.
		for(INT MeshIndex = 0;MeshIndex < PrimitiveSceneInfo->StaticMeshes.Num();MeshIndex++)
		{
			FStaticMesh* StaticMesh = &PrimitiveSceneInfo->StaticMeshes(MeshIndex);
			if ( StaticMesh->CastShadow )
			{
				const FMaterialRenderProxy* MaterialRenderProxy = StaticMesh->MaterialRenderProxy;
				const FMaterial* Material = MaterialRenderProxy->GetMaterial();
				const EBlendMode BlendMode = Material->GetBlendMode();

				if (!IsTranslucentBlendMode(BlendMode))
				{
					if (!Material->IsMasked() && !Material->IsTwoSided())
					{
						// Override with the default material for opaque materials that are not two sided
						MaterialRenderProxy = GEngine->DefaultMaterial->GetRenderProxy(FALSE);
					}


					// Add the static mesh to the shadow's subject draw list.
					SubjectMeshElements.AddMesh(
						StaticMesh,
						FShadowDepthDrawingPolicy::ElementDataType(),
						FShadowDepthDrawingPolicy(StaticMesh->VertexFactory,MaterialRenderProxy,this)
						);
				}
			}
		}
	}
}

void FProjectedShadowInfo::AddReceiverPrimitive(FPrimitiveSceneInfo* PrimitiveSceneInfo)
{
	// Add the primitive to the receiver primitive list.
	ReceiverPrimitives.AddItem(PrimitiveSceneInfo);
}

/**
 * @return TRUE if this shadow info has any casting subject prims to render
 */
UBOOL FProjectedShadowInfo::HasSubjectPrims() const
{
	return( SubjectPrimitives.Num() > 0 || SubjectMeshElements.NumMeshes() > 0 );
}

/** 
 * @param View view to check visibility in
 * @return TRUE if this shadow info has any subject prims visible in the view
 */
UBOOL FProjectedShadowInfo::SubjectsVisible(const FViewInfo& View) const
{
	for(INT PrimitiveIndex = 0;PrimitiveIndex < SubjectPrimitives.Num();PrimitiveIndex++)
	{
		const FPrimitiveSceneInfo* SubjectPrimitiveSceneInfo = SubjectPrimitives(PrimitiveIndex);
		if(View.PrimitiveVisibilityMap(SubjectPrimitiveSceneInfo->Id))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Adds current subject primitives to out array.
 *
 * @param OutSubjectPrimitives [out]	Array to add current subject primitives to.
 */
void FProjectedShadowInfo::GetSubjectPrimitives( FProjectedShadowInfo::PrimitiveArrayType& OutSubjectPrimitives )
{
	OutSubjectPrimitives += SubjectPrimitives;
}

/**
* Adds a primitive to the modulated shadow's receiver list. 
* These are rendered to mask out emissive and backface areas which shouldn't have shadows
* 
* @param PrimitiveSceneInfo - the primitive to add to the list
*/
void FProjectedShadowInfo::AddModShadowReceiverPrimitive(const FPrimitiveSceneInfo* PrimitiveSceneInfo)
{
	ModShadowReceiverPrimitives.AddItem(PrimitiveSceneInfo);
}


FProjectedShadowInfo* FSceneRenderer::CreateProjectedShadow(
	FLightPrimitiveInteraction* Interaction,
	TArray<FProjectedShadowInfo*,SceneRenderingAllocator>& OutPreShadows
	)
{
	const FPrimitiveSceneInfo* PrimitiveSceneInfo = Interaction->GetPrimitiveSceneInfo();
	FLightSceneInfo* LightSceneInfo = Interaction->GetLight();
	FProjectedShadowInfo* ProjectedShadowInfo = NULL;
	FVisibleLightInfo& VisibleLightInfo = VisibleLightInfos(LightSceneInfo->Id);

	if(!PrimitiveSceneInfo->ShadowParent)
	{
		// Check if the shadow is visible in any of the views.
		UBOOL bShadowIsPotentiallyVisibleNextFrame = FALSE;
		UBOOL bShadowIsVisibleThisFrame = FALSE;
		UBOOL bSubjectIsVisible = FALSE;
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			const FViewInfo& View = Views(ViewIndex);

			// Compute the subject primitive's view relevance.  Note that the view won't necessarily have it cached,
			// since the primitive might not be visible.
			const FPrimitiveViewRelevance ViewRelevance = PrimitiveSceneInfo->Proxy->GetViewRelevance(&View);

			// Check if the subject primitive's shadow is view relevant.
			const UBOOL bShadowIsViewRelevant = (ViewRelevance.IsRelevant() || ViewRelevance.bShadowRelevance);

			// Check if the shadow and preshadow are occluded.
			const UBOOL bShadowIsOccluded =
				!View.bIgnoreExistingQueries &&
				View.State &&
				((FSceneViewState*)View.State)->IsShadowOccluded(PrimitiveSceneInfo->Component,LightSceneInfo->LightComponent);

			// The shadow is visible if it is view relevant and unoccluded.
			bShadowIsVisibleThisFrame |= (bShadowIsViewRelevant && !bShadowIsOccluded);
			bShadowIsPotentiallyVisibleNextFrame |= bShadowIsViewRelevant;
			
			// Check if the subject is visible this frame.
			const UBOOL bSubjectIsVisibleInThisView = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);
		    bSubjectIsVisible |= bSubjectIsVisibleInThisView;
		}

		if(!bShadowIsVisibleThisFrame && !bShadowIsPotentiallyVisibleNextFrame)
		{
			// Don't setup the shadow info for shadows which don't need to be rendered or occlusion tested.
			return NULL;
		}

		// Check if this primitive is the parent of a shadow group.
		FShadowGroupSceneInfo* ShadowGroup = Scene->ShadowGroups.Find(PrimitiveSceneInfo->Component);

		// Compute the composite bounds of this group of shadow primitives.
		FBoxSphereBounds Bounds(PrimitiveSceneInfo->Bounds);
		if(ShadowGroup)
		{
			for(INT ChildIndex = 0;ChildIndex < ShadowGroup->Primitives.Num();ChildIndex++)
			{
				FPrimitiveSceneInfo* ShadowChild = ShadowGroup->Primitives(ChildIndex);
				Bounds = Bounds + ShadowChild->Bounds;
			}
		}

		// Compute the projected shadow initializer for this primitive-light pair.
		FProjectedShadowInitializer ShadowInitializer;
		if(LightSceneInfo->GetProjectedShadowInitializer(Bounds,ShadowInitializer))
		{
			// Shadowing constants.
			const UINT MinShadowResolution = (LightSceneInfo->MinShadowResolution > 0) ? LightSceneInfo->MinShadowResolution : GSystemSettings.RenderThreadSettings.MinShadowResolution;
			const UINT MaxShadowResolution = (LightSceneInfo->MaxShadowResolution > 0) ? LightSceneInfo->MaxShadowResolution : GSystemSettings.RenderThreadSettings.MaxShadowResolution;

			// Compute the maximum resolution required for the shadow by any view. Also keep track of the unclamped resolution for fading.
			UINT MaxDesiredResolution = 0;
			UINT MaxUnclampedResolution	= 0;
			for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
			{
				FViewInfo& View = Views(ViewIndex);

				// Determine the size of the subject's bounding sphere in this view.
				FVector4 ScreenPosition = View.WorldToScreen(Bounds.Origin);
				FLOAT ScreenRadius = Max(
					View.SizeX / 2.0f * View.ProjectionMatrix.M[0][0],
					View.SizeY / 2.0f * View.ProjectionMatrix.M[1][1]
					) *
					Bounds.SphereRadius /
					Max(ScreenPosition.W,1.0f);

				// Determine the amount of shadow buffer resolution needed for this view.
				const UINT ShadowBufferResolution = GSceneRenderTargets.GetShadowDepthTextureResolution();
				const UINT UnclampedResolution = appTrunc(ScreenRadius * GSystemSettings.ShadowTexelsPerPixel);
				MaxUnclampedResolution = Max( MaxUnclampedResolution, UnclampedResolution );
				MaxDesiredResolution = Max(
					MaxDesiredResolution,
					Clamp<UINT>(
						UnclampedResolution,
						Min(MinShadowResolution,ShadowBufferResolution - SHADOW_BORDER*2),
						Min(MaxShadowResolution - SHADOW_BORDER*2,ShadowBufferResolution - SHADOW_BORDER*2)
						)
					);
			}

			// Find the minimum elapsed time in any view since the subject was visible
			const FLOAT MinElapsedVisibleTime = Min(
				LightSceneInfo->ModShadowFadeoutTime,
				ViewFamily.CurrentWorldTime - PrimitiveSceneInfo->LastRenderTime
				);		
			const FLOAT MinElapsedVisChangeTime = Min(
				LightSceneInfo->ModShadowFadeoutTime,
				ViewFamily.CurrentWorldTime - PrimitiveSceneInfo->LastVisibilityChangeTime
				);

			// If the light has a positive value set for fade time, then fade the shadow based on the subject's visibility
			FLOAT ElapsedTimeFadeFraction = 1.0f;
			if (LightSceneInfo->ModShadowFadeoutTime > KINDA_SMALL_NUMBER)
			{
				if (bSubjectIsVisible)
				{
					// Fade the shadow in the longer the subject is visible
					// Invert the exponent for fading in
					const FLOAT FadeInExponent = 1.0f / Max(0.01f, LightSceneInfo->ModShadowFadeoutExponent);
					// Calculate a fade percent between 0 (completely faded out) and 1 (completely faded in) based on the light's fade time and exponent
					const FLOAT NormalizedFadePercent = appPow(MinElapsedVisChangeTime / LightSceneInfo->ModShadowFadeoutTime, FadeInExponent);
					// Convert the percent to the range [ModShadowStartFadeInPercent, 1]
					const FLOAT CurrentFade = NormalizedFadePercent * (1.0f - Interaction->ModShadowStartFadeInPercent) + Interaction->ModShadowStartFadeInPercent;
					ElapsedTimeFadeFraction = Clamp(CurrentFade, 0.0f, 1.0f);

					// Todo - fix the one-frame glitch when the subject first becomes visible, because LastVisibilityChangeTime hasn't been updated yet

					// Set the percent that fading out will start at as the current fade percent
					// This allows visibility transitions when fading hasn't completed to start from the current fade percent
					Interaction->ModShadowStartFadeOutPercent = ElapsedTimeFadeFraction;
				}
				else
				{
					// Fade the shadow out the longer the subject is not visible
					const FLOAT FadeOutExponent = Max(0.01f, LightSceneInfo->ModShadowFadeoutExponent);
					// Calculate a fade percent between 0 (completely faded in) and 1 (completely faded out) based on the light's fade time and exponent
					const FLOAT NormalizedFadePercent = appPow(MinElapsedVisibleTime / LightSceneInfo->ModShadowFadeoutTime, FadeOutExponent);
					// Convert the percent to the range [0, ModShadowStartFadeOutPercent]
					const FLOAT CurrentFade = Interaction->ModShadowStartFadeOutPercent - Interaction->ModShadowStartFadeOutPercent * NormalizedFadePercent;
					ElapsedTimeFadeFraction = Clamp(CurrentFade, 0.0f, 1.0f);

					// Set the percent that fading in will start at as the current fade percent
					// This allows visibility transitions when fading hasn't completed to start from the current fade percent
					Interaction->ModShadowStartFadeInPercent = ElapsedTimeFadeFraction;
				}
			}

			const INT ShadowFadeResolution = (LightSceneInfo->ShadowFadeResolution > 0) ? LightSceneInfo->ShadowFadeResolution : GSystemSettings.ShadowFadeResolution;
			// Combine fading based on resolution and visibility time
			const FLOAT FadeAlpha = CalculateShadowFadeAlpha( MaxUnclampedResolution, ShadowFadeResolution, MinShadowResolution ) * ElapsedTimeFadeFraction;

			// Only continue processing the shadow if it hasn't entirely faded away.
			if(FadeAlpha > 1.0f / 256.0f)
			{
				// Create a projected shadow for this interaction's shadow.
				ProjectedShadowInfo = new(GRenderingThreadMemStack,1,16) FProjectedShadowInfo(
					LightSceneInfo,
					PrimitiveSceneInfo,
					Interaction,
					ShadowInitializer,
					FALSE,
					MaxDesiredResolution,
					appTrunc(MaxDesiredResolution / ShadowInitializer.AspectRatio),
					FadeAlpha
					);
				VisibleLightInfo.ProjectedShadows.AddItem(ProjectedShadowInfo);

				// If the light is static, and the subject is visible in at least one view, create a preshadow for
				// static primitives shadowing the subject.
				FProjectedShadowInfo* ProjectedPreShadowInfo = NULL;
				if(LightSceneInfo->bStaticShadowing && bSubjectIsVisible)
				{
					// Create a projected shadow for this interaction's preshadow.
					ProjectedPreShadowInfo = new(GRenderingThreadMemStack,1,16) FProjectedShadowInfo(
						LightSceneInfo,
						PrimitiveSceneInfo,
						Interaction,
						ShadowInitializer,
						TRUE,
						MaxDesiredResolution / 2,
						appTrunc(MaxDesiredResolution / 2 / ShadowInitializer.AspectRatio),
						FadeAlpha
						);
					VisibleLightInfo.ProjectedShadows.AddItem(ProjectedPreShadowInfo);
					OutPreShadows.AddItem(ProjectedPreShadowInfo);
				}

				if(ShadowGroup)
				{
					for(INT ChildIndex = 0;ChildIndex < ShadowGroup->Primitives.Num();ChildIndex++)
					{
						FPrimitiveSceneInfo* ShadowChild = ShadowGroup->Primitives(ChildIndex);

						// Add the subject primitive to the projected shadow.
						ProjectedShadowInfo->AddSubjectPrimitive(ShadowChild);

						if(ProjectedPreShadowInfo)
						{
							// Add the subject primitive to the projected shadow as the receiver.
							ProjectedPreShadowInfo->AddReceiverPrimitive(ShadowChild);
						}
					}
				}

				// Add the subject primitive to the projected shadow.
				ProjectedShadowInfo->AddSubjectPrimitive(Interaction->GetPrimitiveSceneInfo());

				if(ProjectedPreShadowInfo)
				{
					// Add the subject primitive to the projected shadow as the receiver.
					ProjectedPreShadowInfo->AddReceiverPrimitive(Interaction->GetPrimitiveSceneInfo());
				}

				#if STATS
					// Gather dynamic shadow stats.
					if( bShouldGatherDynamicShadowStats )
					{
						FCombinedShadowStats ShadowStat;
						ShadowStat.ShadowResolution = ProjectedShadowInfo->ResolutionX;
						ProjectedShadowInfo->GetSubjectPrimitives( ShadowStat.SubjectPrimitives );
						InteractionToDynamicShadowStatsMap.Set( const_cast<FLightPrimitiveInteraction*>(Interaction), ShadowStat );
					}
				#endif
			}
		}
	}
	return ProjectedShadowInfo;
}

FProjectedShadowInfo* FSceneRenderer::CreateWholeSceneProjectedShadow(FLightSceneInfo* LightSceneInfo)
{
	FVisibleLightInfo& VisibleLightInfo = VisibleLightInfos(LightSceneInfo->Id);

	// Try to create a whole-scene projected shadow initializer for the light.
	FProjectedShadowInitializer ProjectedShadowInitializer;
	if(LightSceneInfo->GetWholeSceneProjectedShadowInitializer(ProjectedShadowInitializer))
	{
		// Shadow resolution constants.
		const UINT MinShadowResolution = (LightSceneInfo->MinShadowResolution > 0) ? LightSceneInfo->MinShadowResolution : GSystemSettings.RenderThreadSettings.MinShadowResolution;
		const UINT MaxShadowResolution = (LightSceneInfo->MaxShadowResolution > 0) ? LightSceneInfo->MaxShadowResolution : GSystemSettings.RenderThreadSettings.MaxShadowResolution;

		// Compute the maximum resolution required for the shadow by any view. Also keep track of the unclamped resolution for fading.
		UINT MaxDesiredResolution = 0;
		UINT MaxUnclampedResolution	= 0;
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			const FViewInfo& View = Views(ViewIndex);

			// Determine the size of the light's bounding sphere in this view.
			const FVector4 ScreenPosition = View.WorldToScreen(LightSceneInfo->GetOrigin());
			const FLOAT ScreenRadius = Max(
				View.SizeX / 2.0f * View.ProjectionMatrix.M[0][0],
				View.SizeY / 2.0f * View.ProjectionMatrix.M[1][1]
				) *
				LightSceneInfo->GetRadius() /
				Max(ScreenPosition.W,1.0f);

			// Determine the amount of shadow buffer resolution needed for this view.
			const UINT ShadowBufferResolution = GSceneRenderTargets.GetShadowDepthTextureResolution();
			const UINT UnclampedResolution = appTrunc(ScreenRadius * GSystemSettings.ShadowTexelsPerPixel);
			MaxUnclampedResolution = Max( MaxUnclampedResolution, UnclampedResolution );
			MaxDesiredResolution = Max(
				MaxDesiredResolution,
				Clamp<UINT>(
					UnclampedResolution,
					Min(MinShadowResolution,ShadowBufferResolution - SHADOW_BORDER*2),
					Min(MaxShadowResolution - SHADOW_BORDER*2,ShadowBufferResolution - SHADOW_BORDER*2)
					)
				);
		}

		const INT ShadowFadeResolution = (LightSceneInfo->ShadowFadeResolution > 0) ? LightSceneInfo->ShadowFadeResolution : GSystemSettings.ShadowFadeResolution;
		// Combine fading based on resolution and visibility time
		const FLOAT FadeAlpha = CalculateShadowFadeAlpha( MaxUnclampedResolution, ShadowFadeResolution, MinShadowResolution );

		// Create the projected shadow info.
		FProjectedShadowInfo* ProjectedShadowInfo = new(GRenderingThreadMemStack,1,16) FProjectedShadowInfo(
			LightSceneInfo,
			ProjectedShadowInitializer,
			MaxDesiredResolution,
			appTrunc(MaxDesiredResolution / ProjectedShadowInitializer.AspectRatio),
			FadeAlpha
			);
		VisibleLightInfo.ProjectedShadows.AddItem(ProjectedShadowInfo);

		// Add all the shadow casting primitives affected by the light to the shadow's subject primitive list.
		for(FLightPrimitiveInteraction* Interaction = LightSceneInfo->DynamicPrimitiveList;
			Interaction;
			Interaction = Interaction->GetNextPrimitive())
		{
			if(Interaction->HasShadow())
			{
				ProjectedShadowInfo->AddSubjectPrimitive(Interaction->GetPrimitiveSceneInfo());
			}
		}

		return ProjectedShadowInfo;
	}
	else
	{
		return NULL;
	}
}

void FSceneRenderer::InitProjectedShadowVisibility()
{
	// Initialize the views' ProjectedShadowVisibilityMaps and remove shadows without subjects.
	for(TSparseArray<FLightSceneInfoCompact>::TConstIterator LightIt(Scene->Lights);LightIt;++LightIt)
	{
		FVisibleLightInfo& VisibleLightInfo = VisibleLightInfos(LightIt.GetIndex());

		// Allocate the light's projected shadow visibility and view relevance maps for this view.
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			FViewInfo& View = Views(ViewIndex);
			FVisibleLightViewInfo& VisibleLightViewInfo = View.VisibleLightInfos(LightIt.GetIndex());
			VisibleLightViewInfo.ProjectedShadowVisibilityMap.Init(FALSE,VisibleLightInfo.ProjectedShadows.Num());
			VisibleLightViewInfo.ProjectedShadowViewRelevanceMap.Empty(VisibleLightInfo.ProjectedShadows.Num());
			VisibleLightViewInfo.ProjectedShadowViewRelevanceMap.AddZeroed(VisibleLightInfo.ProjectedShadows.Num());
		}

		for( INT ShadowIndex=0; ShadowIndex<VisibleLightInfo.ProjectedShadows.Num(); ShadowIndex++ )
		{
			FProjectedShadowInfo& ProjectedShadowInfo = *VisibleLightInfo.ProjectedShadows(ShadowIndex);

			// Assign the shadow its id.
			ProjectedShadowInfo.ShadowId = ShadowIndex;

			for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
			{
				FViewInfo& View = Views(ViewIndex);
				FVisibleLightViewInfo& VisibleLightViewInfo = View.VisibleLightInfos(LightIt.GetIndex());

				if(VisibleLightViewInfo.bInViewFrustum)
				{
					// Compute the subject primitive's view relevance.  Note that the view won't necessarily have it cached,
					// since the primitive might not be visible.
					FPrimitiveViewRelevance ViewRelevance;
					if(ProjectedShadowInfo.ParentSceneInfo)
					{
						ViewRelevance = ProjectedShadowInfo.ParentSceneInfo->Proxy->GetViewRelevance(&View);
					}
					else
					{
						ViewRelevance.bStaticRelevance = ViewRelevance.bDynamicRelevance = ViewRelevance.bShadowRelevance = TRUE;
						ViewRelevance.SetDPG(SDPG_World,TRUE);
					}							
					VisibleLightViewInfo.ProjectedShadowViewRelevanceMap(ShadowIndex) = ViewRelevance;

					// Check if the subject primitive's shadow is view relevant.
					const UBOOL bShadowIsViewRelevant = (ViewRelevance.IsRelevant() || ViewRelevance.bShadowRelevance);

					// Check if the shadow and preshadow are occluded.
					const UBOOL bShadowIsOccluded =
						!View.bIgnoreExistingQueries &&
						View.State &&
						((FSceneViewState*)View.State)->IsShadowOccluded(
							ProjectedShadowInfo.ParentSceneInfo ? 
								ProjectedShadowInfo.ParentSceneInfo->Component :
								NULL,
							ProjectedShadowInfo.LightSceneInfo->LightComponent
							);

					// The shadow is visible if it is view relevant and unoccluded.
					if(bShadowIsViewRelevant && !bShadowIsOccluded)
					{
						VisibleLightViewInfo.ProjectedShadowVisibilityMap(ShadowIndex) = TRUE;
					}

					// Draw the shadow frustum.
					if((ViewFamily.ShowFlags & SHOW_ShadowFrustums) && bShadowIsViewRelevant && !bShadowIsOccluded && !ProjectedShadowInfo.bPreShadow)
					{
						FViewElementPDI ShadowFrustumPDI(&Views(ViewIndex),NULL);
						ProjectedShadowInfo.RenderFrustumWireframe(&ShadowFrustumPDI);
					}
				}
			}
		}
	}
}

void FSceneRenderer::GatherShadowPrimitives(
	const TArray<FProjectedShadowInfo*,SceneRenderingAllocator>& ModulateBetterShadows,
	const TArray<FProjectedShadowInfo*,SceneRenderingAllocator>& PreShadows
	)
{
	if(ModulateBetterShadows.Num() || PreShadows.Num())
	{
		// Find primitives that are in a shadow frustum in the octree.
		for(FScenePrimitiveOctree::TConstIterator<SceneRenderingAllocator> PrimitiveOctreeIt(Scene->PrimitiveOctree);
			PrimitiveOctreeIt.HasPendingNodes();
			PrimitiveOctreeIt.Advance())
		{
			const FScenePrimitiveOctree::FNode& PrimitiveOctreeNode = PrimitiveOctreeIt.GetCurrentNode();
			const FOctreeNodeContext& PrimitiveOctreeNodeContext = PrimitiveOctreeIt.GetCurrentContext();

			// Find children of this octree node that may contain relevant primitives.
			FOREACH_OCTREE_CHILD_NODE(ChildRef)
			{
				if(PrimitiveOctreeNode.HasChild(ChildRef))
				{
					// Check that the child node is in the frustum for at least one shadow.
					const FOctreeNodeContext ChildContext = PrimitiveOctreeNodeContext.GetChildContext(ChildRef);
					UBOOL bIsInFrustum = FALSE;

					// Check for receivers of modulated shadows.
					for(INT ShadowIndex = 0;ShadowIndex < ModulateBetterShadows.Num();ShadowIndex++)
					{
						FProjectedShadowInfo* ProjectedShadowInfo = ModulateBetterShadows(ShadowIndex);
						if(ProjectedShadowInfo->ReceiverFrustum.IntersectBox(
							ChildContext.Bounds.Center + ProjectedShadowInfo->PreShadowTranslation,
							ChildContext.Bounds.Extent
							))
						{
							bIsInFrustum = TRUE;
							break;
						}
					}

					// Check for subjects of preshadows.
					if(!bIsInFrustum)
					{
						for(INT ShadowIndex = 0;ShadowIndex < PreShadows.Num();ShadowIndex++)
						{
							FProjectedShadowInfo* ProjectedShadowInfo = PreShadows(ShadowIndex);

							// Check if this primitive is in the shadow's frustum.
							if(ProjectedShadowInfo->SubjectFrustum.IntersectBox(
								ChildContext.Bounds.Center + ProjectedShadowInfo->PreShadowTranslation,
								ChildContext.Bounds.Extent
								))
							{
								bIsInFrustum = TRUE;
								break;
							}
						}
					}
					if(bIsInFrustum)
					{
						// If the child node was in the frustum of at least one preshadow or modulatebetter shadow, push it on
						// the iterator's pending node stack.
						PrimitiveOctreeIt.PushChild(ChildRef);
					}
				}
			}

			// Check all the primitives in this octree node.
			for(FScenePrimitiveOctree::ElementConstIt NodePrimitiveIt(PrimitiveOctreeNode.GetElementIt());NodePrimitiveIt;++NodePrimitiveIt)
			{
				const FPrimitiveSceneInfoCompact& PrimitiveSceneInfoCompact = *NodePrimitiveIt;
				FPrimitiveSceneInfo* PrimitiveSceneInfo = PrimitiveSceneInfoCompact.PrimitiveSceneInfo;
				const FBoxSphereBounds& PrimitiveBounds = PrimitiveSceneInfoCompact.Bounds;

				// See if it is visible (ie. not occluded) in any view
				UBOOL bIsPrimitiveVisible = FALSE;
				for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
				{
					const FViewInfo& View = Views(ViewIndex);
					if( View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id) )
					{
						bIsPrimitiveVisible = TRUE;
						break;
					}
				}

				if( bIsPrimitiveVisible 
					// Only process the primitive if it needs to mask out modulated shadows on backfaces or emissive areas
					&& (PrimitiveSceneInfo->bCullModulatedShadowOnBackfaces || PrimitiveSceneInfo->bCullModulatedShadowOnEmissive && !GModShadowsWithAlphaEmissiveBit) )
				{
					// Iterate over current shadow subjects and see if the primitive intersects 
					// any of their receiver frustums. Add to list of modulated shadow receivers 
					// for the projected shadow if it does
					for(INT ShadowIndex = 0;ShadowIndex < ModulateBetterShadows.Num();ShadowIndex++)
					{
						FProjectedShadowInfo* ProjectedShadowInfo = ModulateBetterShadows(ShadowIndex);
						if( ProjectedShadowInfo && 
							// Don't add other primitives if we are only rendering self shadowing
							!ProjectedShadowInfo->bSelfShadowOnly &&
							// Don't do a frustum intersection test for the subject primitive
							ProjectedShadowInfo->ParentSceneInfo != PrimitiveSceneInfo &&
							ProjectedShadowInfo->ReceiverFrustum.IntersectBox(
								PrimitiveBounds.Origin + ProjectedShadowInfo->PreShadowTranslation,
								PrimitiveBounds.BoxExtent
								))
						{								
							ProjectedShadowInfo->AddModShadowReceiverPrimitive(PrimitiveSceneInfo);
						}
					}
				}

				// Check if the primitive is in the subject of any preshadows.
				if(PrimitiveSceneInfoCompact.bCastDynamicShadow)
				{
					for(INT ShadowIndex = 0;ShadowIndex < PreShadows.Num();ShadowIndex++)
					{
						FProjectedShadowInfo* ProjectedShadowInfo = PreShadows(ShadowIndex);

						// Check if this primitive is in the shadow's frustum.
						if( ProjectedShadowInfo->SubjectFrustum.IntersectBox(
								PrimitiveBounds.Origin + ProjectedShadowInfo->PreShadowTranslation,
								PrimitiveBounds.BoxExtent
								) &&
							ProjectedShadowInfo->LightSceneInfoCompact.AffectsPrimitive(PrimitiveSceneInfoCompact) )
						{
							// Add this primitive to the shadow.
							ProjectedShadowInfo->AddSubjectPrimitive(PrimitiveSceneInfo);

							#if STATS
								// Add preshadow primitives to shadow stats if we're gathering.
								if( bShouldGatherDynamicShadowStats )
								{
									FCombinedShadowStats* ShadowStats = InteractionToDynamicShadowStatsMap.Find( ProjectedShadowInfo->ParentInteraction );
									check( ShadowStats );
									ShadowStats->PreShadowPrimitives.AddItem( PrimitiveSceneInfo );
								}
							#endif
						}
					}
				}
			}
		}
	}

	// Add subject primitives to each modulate better shadow's ModShadowReceiverPrimitives array
	for(INT ShadowIndex = 0;ShadowIndex < ModulateBetterShadows.Num();ShadowIndex++)
	{
		FProjectedShadowInfo* ProjectedShadowInfo = ModulateBetterShadows(ShadowIndex);
		if (ProjectedShadowInfo 
			&& ProjectedShadowInfo->ParentSceneInfo
			// Only add the subject primitive if it needs to mask out backfaces and emissive areas
			&& (ProjectedShadowInfo->ParentSceneInfo->bCullModulatedShadowOnBackfaces 
			|| ProjectedShadowInfo->ParentSceneInfo->bCullModulatedShadowOnEmissive && !GModShadowsWithAlphaEmissiveBit))
		{
			ProjectedShadowInfo->AddModShadowReceiverPrimitive(ProjectedShadowInfo->ParentSceneInfo);
		}
	}
}

void FSceneRenderer::InitDynamicShadows()
{
	SCOPE_CYCLE_COUNTER(STAT_DynamicShadowSetupTime);

	TArray<FProjectedShadowInfo*,SceneRenderingAllocator> PreShadows;
	TArray<FProjectedShadowInfo*,SceneRenderingAllocator> ModulateBetterShadows;

	// Pre-allocate for worst case.
	VisibleShadowCastingLightInfos.Empty(Scene->Lights.GetMaxIndex());

	for(TSparseArray<FLightSceneInfoCompact>::TConstIterator LightIt(Scene->Lights);LightIt;++LightIt)
	{
		const FLightSceneInfoCompact& LightSceneInfoCompact = *LightIt;
		FLightSceneInfo* LightSceneInfo = LightSceneInfoCompact.LightSceneInfo;
		FVisibleLightInfo& VisibleLightInfo = VisibleLightInfos(LightSceneInfo->Id);

		// Only consider lights that may have shadows.
		if( LightSceneInfoCompact.bCastStaticShadow || LightSceneInfoCompact.bCastDynamicShadow )
		{
			// see if the light is visible in any view
			UBOOL bIsVisibleInAnyView = FALSE;

			for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
			{
				// View frustums are only checked when lights have visible primitives or have modulated shadows,
				// so we don't need to check for that again here
				bIsVisibleInAnyView = Views(ViewIndex).VisibleLightInfos(LightSceneInfo->Id).bInViewFrustum;
				if (bIsVisibleInAnyView) 
				{
					break;
				}
			}

			if( bIsVisibleInAnyView )
			{
				const UBOOL bLightCastsDynamicShadows = !LightSceneInfoCompact.bStaticShadowing && LightSceneInfoCompact.bCastDynamicShadow;

				// Add to array of visible shadow casting lights if there is a view/ DPG that is affected.
				VisibleShadowCastingLightInfos.AddItem( LightSceneInfo );

				// If the light casts dynamic projected shadows, try creating a whole-scene shadow for it.
				UBOOL bCreatePrimitiveShadows = TRUE;
				if(	bLightCastsDynamicShadows && CreateWholeSceneProjectedShadow(LightSceneInfo) )
				{
					// If the light has a whole-scene shadow, it doesn't need individual primitive shadows.
					bCreatePrimitiveShadows = FALSE;
				}
				
				// Look for individual primitives with a dynamic shadow.
				if(bCreatePrimitiveShadows)
				{
					for(FLightPrimitiveInteraction* Interaction = LightSceneInfo->DynamicPrimitiveList;
						Interaction;
						Interaction = Interaction->GetNextPrimitive()
						)
					{
						if(Interaction->HasShadow())
						{
							FPrimitiveSceneInfo* PrimitiveSceneInfo = Interaction->GetPrimitiveSceneInfo();

							if(PrimitiveSceneInfo->bStaticShadowing)
							{
								// A static primitive should cast a shadow volume.
								VisibleLightInfo.ShadowVolumePrimitives.AddItem(PrimitiveSceneInfo);
							}
							else if(!PrimitiveSceneInfo->bStaticShadowing)
							{
								// Create projected shadow infos and add the one created for the preshadow (if used) to the PreShadows array.
								FProjectedShadowInfo* ProjectedShadowInfo = CreateProjectedShadow(Interaction,PreShadows);
								
								// Keep a list of the shadows that need a list of receiver primitives for culling their modulated effect on
								// emissive and backfaces.
								if(ProjectedShadowInfo && ProjectedShadowInfo->bCullShadowOnBackfacesAndEmissive)
								{
									ModulateBetterShadows.AddItem(ProjectedShadowInfo);
								}
							}
						}
					}
				}
			}
		}
	}

	// Calculate visibility of the projected shadows.
	InitProjectedShadowVisibility();

	// Gather the primitives used to draw the pre-shadows and modulate-better shadows.
	GatherShadowPrimitives(ModulateBetterShadows,PreShadows);
}
