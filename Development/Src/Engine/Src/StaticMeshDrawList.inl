/*=============================================================================
	StaticMeshDrawList.inl: Static mesh draw list implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __STATICMESHDRAWLIST_INL__
#define __STATICMESHDRAWLIST_INL__

template<typename DrawingPolicyType>
void TStaticMeshDrawList<DrawingPolicyType>::FElementHandle::Remove()
{
	// Make a copy of this handle's variables on the stack, since the call to Elements.RemoveSwap deletes the handle.
	TStaticMeshDrawList* const LocalDrawList = StaticMeshDrawList;
	FDrawingPolicyLink* const LocalDrawingPolicyLink = &LocalDrawList->DrawingPolicySet(SetId);
	const INT LocalElementIndex = ElementIndex;

	checkSlow(LocalDrawingPolicyLink->SetId == SetId);

	// Unlink the mesh from this draw list.
	LocalDrawingPolicyLink->Elements(ElementIndex).Mesh->UnlinkDrawList(this);
	LocalDrawingPolicyLink->Elements(ElementIndex).Mesh = NULL;

	checkSlow(LocalDrawingPolicyLink->Elements.Num() == LocalDrawingPolicyLink->CompactElements.Num());

	// Remove this element from the drawing policy's element list.
	LocalDrawingPolicyLink->Elements.RemoveSwap(LocalElementIndex);
	LocalDrawingPolicyLink->CompactElements.RemoveSwap(LocalElementIndex);
	if (LocalElementIndex < LocalDrawingPolicyLink->Elements.Num())
	{
		// Fixup the element that was moved into the hole created by the removed element.
		LocalDrawingPolicyLink->Elements(LocalElementIndex).Handle->ElementIndex = LocalElementIndex;
	}

	// If this was the last element for the drawing policy, remove the drawing policy from the draw list.
	if(!LocalDrawingPolicyLink->Elements.Num())
	{
		LocalDrawList->OrderedDrawingPolicies.RemoveItem(LocalDrawingPolicyLink->SetId);
		LocalDrawList->DrawingPolicySet.Remove(LocalDrawingPolicyLink->SetId);
	}
}

template<typename DrawingPolicyType>
void TStaticMeshDrawList<DrawingPolicyType>::SubmitDrawCall(
	const FViewInfo& View,
	const FElement& Element,
	const FDrawingPolicyLink* DrawingPolicyLink,
	UBOOL &bDrawnShared
	) const
{
	if(!bDrawnShared)
	{
		DrawingPolicyLink->DrawingPolicy.DrawShared(&View,DrawingPolicyLink->BoundShaderState);
		bDrawnShared = TRUE;
	}

	for(INT bBackFace = 0;bBackFace < (DrawingPolicyLink->DrawingPolicy.NeedsBackfacePass() ? 2 : 1);bBackFace++)
	{
		DrawingPolicyLink->DrawingPolicy.SetMeshRenderState(
			View,
			Element.Mesh->PrimitiveSceneInfo,
			*Element.Mesh,
			bBackFace,
			Element.PolicyData
			);
		DrawingPolicyLink->DrawingPolicy.DrawMesh(*Element.Mesh);
	}
}

template<typename DrawingPolicyType>
void TStaticMeshDrawList<DrawingPolicyType>::AddMesh(
	FStaticMesh* Mesh,
	const ElementPolicyDataType& PolicyData,
	const DrawingPolicyType& InDrawingPolicy
	)
{
	// Check for an existing drawing policy matching the mesh's drawing policy.
	FDrawingPolicyLink* DrawingPolicyLink = DrawingPolicySet.Find(InDrawingPolicy);
	if(!DrawingPolicyLink)
	{
		// If no existing drawing policy matches the mesh, create a new one.
		const FSetElementId DrawingPolicyLinkId = DrawingPolicySet.Add(FDrawingPolicyLink(this,InDrawingPolicy));

		DrawingPolicyLink = &DrawingPolicySet(DrawingPolicyLinkId);
		DrawingPolicyLink->SetId = DrawingPolicyLinkId;

		// Insert the drawing policy into the ordered drawing policy list.
		INT MinIndex = 0;
		INT MaxIndex = OrderedDrawingPolicies.Num() - 1;
		while(MinIndex < MaxIndex)
		{
			INT PivotIndex = (MaxIndex + MinIndex) / 2;
			INT CompareResult = Compare(DrawingPolicySet(OrderedDrawingPolicies(PivotIndex)).DrawingPolicy,DrawingPolicyLink->DrawingPolicy);
			if(CompareResult < 0)
			{
				MinIndex = PivotIndex + 1;
			}
			else if(CompareResult > 0)
			{
				MaxIndex = PivotIndex;
			}
			else
			{
				MinIndex = MaxIndex = PivotIndex;
			}
		};
		check(MinIndex >= MaxIndex);
		OrderedDrawingPolicies.InsertItem(DrawingPolicyLinkId,MinIndex);
	}

	const INT ElementIndex = DrawingPolicyLink->Elements.Num();
	FElement* Element = new(DrawingPolicyLink->Elements) FElement(Mesh, PolicyData, this, DrawingPolicyLink->SetId, ElementIndex);
	new(DrawingPolicyLink->CompactElements) FElementCompact(Mesh->Id);
	Mesh->LinkDrawList(Element->Handle);
}

template<typename DrawingPolicyType>
void TStaticMeshDrawList<DrawingPolicyType>::RemoveAllMeshes()
{
	OrderedDrawingPolicies.Empty();
	DrawingPolicySet.Empty();
}

template<typename DrawingPolicyType>
UBOOL TStaticMeshDrawList<DrawingPolicyType>::DrawAll(const FViewInfo& View) const
{
	UBOOL bDirty = FALSE;
	for(typename TArray<FSetElementId>::TConstIterator PolicyIt(OrderedDrawingPolicies); PolicyIt; ++PolicyIt)
	{
		const FDrawingPolicyLink* DrawingPolicyLink = &DrawingPolicySet(*PolicyIt);
		UBOOL bDrawnShared = FALSE;

		for(typename TArray<FElement>::TConstIterator ElementIt(DrawingPolicyLink->Elements); ElementIt; ++ElementIt)
		{
			const FElement& Element = *ElementIt;
			FLOAT DistanceSquared = 0.0f;
			if(((FSceneView&)View).ViewOrigin.W > 0.0f)
			{
				DistanceSquared = (Element.Mesh->PrimitiveSceneInfo->Bounds.Origin - ((FSceneView&)View).ViewOrigin).SizeSquared();
			}

			//cull the mesh if it is not in range
			if(DistanceSquared >= Element.Mesh->MinDrawDistanceSquared && DistanceSquared < Element.Mesh->MaxDrawDistanceSquared)
			{
				SubmitDrawCall( View, Element, DrawingPolicyLink, bDrawnShared);
				bDirty = TRUE;
			}
		}
	}
	return bDirty;
}

template<typename DrawingPolicyType>
UBOOL TStaticMeshDrawList<DrawingPolicyType>::DrawVisible(
	const FViewInfo& View,
	const TBitArray<SceneRenderingBitArrayAllocator>& StaticMeshVisibilityMap
	) const
{
	UBOOL bDirty = FALSE;
	for(typename TArray<FSetElementId>::TConstIterator PolicyIt(OrderedDrawingPolicies); PolicyIt; ++PolicyIt)
	{
		const FDrawingPolicyLink* DrawingPolicyLink = &DrawingPolicySet(*PolicyIt);
		UBOOL bDrawnShared = FALSE;
		PREFETCH(&DrawingPolicyLink->CompactElements(0));
		const INT NumElements = DrawingPolicyLink->Elements.Num();
		PREFETCH(&((&DrawingPolicyLink->CompactElements(0))->VisibilityBitReference));
		const FElementCompact* CompactElementPtr = &DrawingPolicyLink->CompactElements(0);
		for(INT ElementIndex = 0; ElementIndex < NumElements; ElementIndex++, CompactElementPtr++)
		{
			if(StaticMeshVisibilityMap.AccessCorrespondingBit(CompactElementPtr->VisibilityBitReference))
			{
				const FElement& Element = DrawingPolicyLink->Elements(ElementIndex);
#if STATS
				if( Element.Mesh->IsDecal() )
				{
					INC_DWORD_STAT_BY(STAT_DecalTriangles,Element.Mesh->NumPrimitives);
					INC_DWORD_STAT(STAT_DecalDrawCalls);
				}
				else
				{
					INC_DWORD_STAT_BY(STAT_StaticMeshTriangles,Element.Mesh->NumPrimitives);
				}
#endif
				SubmitDrawCall( View, Element, DrawingPolicyLink, bDrawnShared);
				bDirty = TRUE;
			}
		}
	}
	return bDirty;
}

template<typename DrawingPolicyType>
INT TStaticMeshDrawList<DrawingPolicyType>::NumMeshes() const
{
	INT TotalMeshes=0;
	for(typename TArray<FSetElementId>::TConstIterator PolicyIt(OrderedDrawingPolicies); PolicyIt; ++PolicyIt)
	{
		const FDrawingPolicyLink* DrawingPolicyLink = &DrawingPolicySet(*PolicyIt);
		TotalMeshes += DrawingPolicyLink->Elements.Num();
	}
	return TotalMeshes;
}

#endif
