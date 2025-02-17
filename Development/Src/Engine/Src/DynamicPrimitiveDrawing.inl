/*=============================================================================
	DynamicPrimitiveDrawing.inl: Dynamic primitive drawing implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DYNAMICPRIMITIVEDRAWING_INL__
#define __DYNAMICPRIMITIVEDRAWING_INL__

template<typename DrawingPolicyFactoryType>
TDynamicPrimitiveDrawer<DrawingPolicyFactoryType>::~TDynamicPrimitiveDrawer()
{
	if(View)
	{
		// Draw the batched elements.
		BatchedElements.Draw(
			View->ViewProjectionMatrix,
			appTrunc(View->SizeX),
			appTrunc(View->SizeY),
			(View->Family->ShowFlags & SHOW_HitProxies) != 0
			);
	}

	// Cleanup the dynamic resources.
	for(INT ResourceIndex = 0;ResourceIndex < DynamicResources.Num();ResourceIndex++)
	{
		//release the resources before deleting, they will delete themselves
		DynamicResources(ResourceIndex)->ReleasePrimitiveResource();
	}
}

template<typename DrawingPolicyFactoryType>
void TDynamicPrimitiveDrawer<DrawingPolicyFactoryType>::SetPrimitive(const FPrimitiveSceneInfo* NewPrimitiveSceneInfo)
{
	PrimitiveSceneInfo = NewPrimitiveSceneInfo;
	HitProxyId = PrimitiveSceneInfo->DefaultDynamicHitProxyId;
}

template<typename DrawingPolicyFactoryType>
UBOOL TDynamicPrimitiveDrawer<DrawingPolicyFactoryType>::IsHitTesting()
{
	return bIsHitTesting;
}

template<typename DrawingPolicyFactoryType>
void TDynamicPrimitiveDrawer<DrawingPolicyFactoryType>::SetHitProxy(HHitProxy* HitProxy)
{
	if(HitProxy)
	{
		// Only allow hit proxies from CreateHitProxies.
		checkMsg(PrimitiveSceneInfo->HitProxies.FindItemIndex(HitProxy) != INDEX_NONE,"Hit proxy used in DrawDynamicElements which wasn't created in CreateHitProxies");
		HitProxyId = HitProxy->Id;
	}
	else
	{
		HitProxyId = FHitProxyId();
	}
}

template<typename DrawingPolicyFactoryType>
void TDynamicPrimitiveDrawer<DrawingPolicyFactoryType>::RegisterDynamicResource(FDynamicPrimitiveResource* DynamicResource)
{
	// Add the dynamic resource to the list of resources to cleanup on destruction.
	DynamicResources.AddItem(DynamicResource);

	// Initialize the dynamic resource immediately.
	DynamicResource->InitPrimitiveResource();
}

template<typename DrawingPolicyFactoryType>
UBOOL TDynamicPrimitiveDrawer<DrawingPolicyFactoryType>::IsMaterialIgnored(const FMaterialRenderProxy* MaterialRenderProxy) const
{
	return DrawingPolicyFactoryType::IsMaterialIgnored(MaterialRenderProxy);
}

template<typename DrawingPolicyFactoryType>
INT TDynamicPrimitiveDrawer<DrawingPolicyFactoryType>::DrawMesh(const FMeshElement& Mesh)
{
	INT NumPassesRendered=0;
	if( Mesh.DepthPriorityGroup == DPGIndex )
	{
		const FMaterial* Material = Mesh.MaterialRenderProxy->GetMaterial();
		const EMaterialLightingModel LightingModel = Material->GetLightingModel();
		const UBOOL bNeedsBackfacePass =
			Material->IsTwoSided() &&
			(LightingModel != MLM_NonDirectional) &&
			(LightingModel != MLM_Unlit) &&
			!CanAccessFacingRegister(GRHIShaderPlatform);
		const UINT NumBackfacePasses = bNeedsBackfacePass ? 2 : 1;
		for(UINT bBackFace = 0;bBackFace < NumBackfacePasses;bBackFace++)
		{
			const UBOOL DrawDirty = DrawingPolicyFactoryType::DrawDynamicMesh(
				*View,
				DrawingContext,
				Mesh,
				bBackFace,
				bPreFog,
				PrimitiveSceneInfo,
				HitProxyId
				);
			bDirty |= DrawDirty;

			NumPassesRendered += DrawDirty;						
		}
	}
	return NumPassesRendered;
}

template<typename DrawingPolicyFactoryType>
void TDynamicPrimitiveDrawer<DrawingPolicyFactoryType>::DrawSprite(
	const FVector& Position,
	FLOAT SizeX,
	FLOAT SizeY,
	const FTexture* Sprite,
	const FLinearColor& Color,
	BYTE DepthPriorityGroup
	)
{
	if(DepthPriorityGroup == DPGIndex && DrawingPolicyFactoryType::bAllowSimpleElements)
	{
		BatchedElements.AddSprite(Position,SizeX,SizeY,Sprite,Color,HitProxyId);
		bDirty = TRUE;
	}
}

template<typename DrawingPolicyFactoryType>
void TDynamicPrimitiveDrawer<DrawingPolicyFactoryType>::DrawLine(
	const FVector& Start,
	const FVector& End,
	const FLinearColor& Color,
	BYTE DepthPriorityGroup
	)
{
	if(DepthPriorityGroup == DPGIndex && DrawingPolicyFactoryType::bAllowSimpleElements)
	{
		BatchedElements.AddLine(Start,End,Color,HitProxyId);
		bDirty = TRUE;
	}
}

template<typename DrawingPolicyFactoryType>
void TDynamicPrimitiveDrawer<DrawingPolicyFactoryType>::DrawPoint(
	const FVector& Position,
	const FLinearColor& Color,
	FLOAT PointSize,
	BYTE DepthPriorityGroup
	)
{
	if(DepthPriorityGroup == DPGIndex && DrawingPolicyFactoryType::bAllowSimpleElements)
	{
		BatchedElements.AddPoint(Position,PointSize,Color,HitProxyId);
		bDirty = TRUE;
	}
}

template<class DrawingPolicyFactoryType>
UBOOL DrawViewElements(
	const FViewInfo& View,
	const typename DrawingPolicyFactoryType::ContextType& DrawingContext,
	UINT DPGIndex,
	UBOOL bPreFog
	)
{
	// Draw the view's mesh elements.
	for(INT MeshIndex = 0;MeshIndex < View.ViewMeshElements[DPGIndex].Num();MeshIndex++)
	{
		const FHitProxyMeshPair& Mesh = View.ViewMeshElements[DPGIndex](MeshIndex);
		const UBOOL bIsTwoSided = Mesh.MaterialRenderProxy->GetMaterial()->IsTwoSided();
		const UBOOL bIsNonDirectionalLighting = Mesh.MaterialRenderProxy->GetMaterial()->GetLightingModel() == MLM_NonDirectional;
		const UINT NumBackfacePasses = (bIsTwoSided && !bIsNonDirectionalLighting) ? 2 : 1;
		for(UINT bBackFace = 0;bBackFace < NumBackfacePasses;bBackFace++)
		{
			DrawingPolicyFactoryType::DrawDynamicMesh(
				View,
				DrawingContext,
				Mesh,
				bBackFace,
				bPreFog,
				NULL,
				Mesh.HitProxyId
				);
		}
	}

	return View.ViewMeshElements[DPGIndex].Num() != 0;
}

template<class DrawingPolicyFactoryType>
UBOOL DrawDynamicPrimitiveSet(
	const FViewInfo& View,
	const typename DrawingPolicyFactoryType::ContextType& DrawingContext,
	UINT DPGIndex,
	UBOOL bPreFog,
	UBOOL bIsHitTesting
	)
{
	// Draw the view's elements.
	UBOOL bDrewViewElements = DrawViewElements<DrawingPolicyFactoryType>(View,DrawingContext,DPGIndex,bPreFog);

	// Draw the elements of each dynamic primitive.
	TDynamicPrimitiveDrawer<DrawingPolicyFactoryType> Drawer(&View,DPGIndex,DrawingContext,bPreFog,bIsHitTesting);
	for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
	{
		const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);

		if(!View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id))
		{
			continue;
		}

		if(!View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id).GetDPG(DPGIndex))
		{
			continue;
		}

		Drawer.SetPrimitive(PrimitiveSceneInfo);
		PrimitiveSceneInfo->Proxy->DrawDynamicElements(
			&Drawer,
			&View,
			DPGIndex
			);
	}

	return bDrewViewElements || Drawer.IsDirty();
}

inline FViewElementPDI::FViewElementPDI(FViewInfo* InViewInfo,FHitProxyConsumer* InHitProxyConsumer):
	FPrimitiveDrawInterface(InViewInfo),
	ViewInfo(InViewInfo),
	HitProxyConsumer(InHitProxyConsumer)
{}

inline UBOOL FViewElementPDI::IsHitTesting()
{
	return HitProxyConsumer != NULL;
}
inline void FViewElementPDI::SetHitProxy(HHitProxy* HitProxy)
{
	// Change the current hit proxy.
	CurrentHitProxy = HitProxy;

	if(HitProxyConsumer && HitProxy)
	{
		// Notify the hit proxy consumer of the new hit proxy.
		HitProxyConsumer->AddHitProxy(HitProxy);
	}
}

inline void FViewElementPDI::RegisterDynamicResource(FDynamicPrimitiveResource* DynamicResource)
{
	ViewInfo->DynamicResources.AddItem(DynamicResource);
}

inline void FViewElementPDI::DrawSprite(
	const FVector& Position,
	FLOAT SizeX,
	FLOAT SizeY,
	const FTexture* Sprite,
	const FLinearColor& Color,
	BYTE DepthPriorityGroup
	)
{
	ViewInfo->BatchedViewElements[DepthPriorityGroup].AddSprite(
		Position,
		SizeX,
		SizeY,
		Sprite,
		Color,
		CurrentHitProxy ? CurrentHitProxy->Id : FHitProxyId()
		);
}

inline void FViewElementPDI::DrawLine(
	const FVector& Start,
	const FVector& End,
	const FLinearColor& Color,
	BYTE DepthPriorityGroup
	)
{
	ViewInfo->BatchedViewElements[DepthPriorityGroup].AddLine(Start,End,Color,CurrentHitProxy ? CurrentHitProxy->Id : FHitProxyId());
}

inline void FViewElementPDI::DrawPoint(
	const FVector& Position,
	const FLinearColor& Color,
	FLOAT PointSize,
	BYTE DepthPriorityGroup
	)
{
	ViewInfo->BatchedViewElements[DepthPriorityGroup].AddPoint(
		Position,
		PointSize,
		Color,
		CurrentHitProxy ? CurrentHitProxy->Id : FHitProxyId()
		);
}

inline INT FViewElementPDI::DrawMesh(const FMeshElement& Mesh)
{
	const UINT DepthPriorityGroup = Mesh.DepthPriorityGroup < SDPG_MAX_SceneRender ? Mesh.DepthPriorityGroup : SDPG_World;

	// Keep track of view mesh elements whether that have transluceny or distortion.
	ViewInfo->bHasTranslucentViewMeshElements |= (Mesh.IsTranslucent() << DepthPriorityGroup);
	ViewInfo->bHasDistortionViewMeshElements |= (Mesh.IsDistortion() << DepthPriorityGroup);

	new(ViewInfo->ViewMeshElements[DepthPriorityGroup]) FHitProxyMeshPair(
		Mesh,
		CurrentHitProxy ? CurrentHitProxy->Id : FHitProxyId()
		);

	return 1;
}

#endif
