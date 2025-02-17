/*=============================================================================
	CoverMeshComponent.cpp: CoverMeshComponent implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineAIClasses.h"
#include "DebugRenderSceneProxy.h"

IMPLEMENT_CLASS(UCoverMeshComponent);

UBOOL IsOverlapSlotSelected( ACoverLink *Link, INT InSlotIdx )
{
	for( INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++ )
	{
		if( InSlotIdx >= 0 && SlotIdx != InSlotIdx )
			continue;

		FCoverSlot& Slot = Link->Slots(SlotIdx);
		for( INT OverIdx = 0; OverIdx < Slot.OverlapClaims.Num(); OverIdx++ )
		{
			ACoverLink *OverLink = Cast<ACoverLink>(Slot.OverlapClaims(OverIdx).Actor);
			INT	OverSlotIdx = Slot.OverlapClaims(OverIdx).SlotIdx;
			if( OverLink && 
				OverLink->IsSelected() &&
				(OverSlotIdx >= 0 && OverSlotIdx < OverLink->Slots.Num()) &&
				OverLink->Slots(OverSlotIdx).bSelected )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}


/** Represents a CoverGroupRenderingComponent to the scene manager. */
class FCoverMeshSceneProxy : public FDebugRenderSceneProxy
{
	UBOOL bCreatedInGame;
public:

	/** Initialization constructor. */
	FCoverMeshSceneProxy(const UCoverMeshComponent* InComponent, UBOOL bInGame):
		FDebugRenderSceneProxy(InComponent)
	{	
		bCreatedInGame = bInGame;
		// We add all the 'path' stuff to the Debug
		{
			// draw reach specs
			ANavigationPoint *Nav = Cast<ANavigationPoint>(InComponent->GetOwner());
			if( Nav != NULL )
			{
				// draw cylinder
				if (Nav->IsSelected() && Nav->CylinderComponent != NULL)
				{
					new(Cylinders) FWireCylinder( Nav->CylinderComponent->GetOrigin(), Nav->CylinderComponent->CollisionRadius, Nav->CylinderComponent->CollisionHeight, GEngine->C_ScaleBoxHi );
				}

				if( Nav->PathList.Num() > 0 )
				{
					for (INT Idx = 0; Idx < Nav->PathList.Num(); Idx++)
					{
						UReachSpec* Reach = Nav->PathList(Idx);
						if( Reach != NULL && !Reach->bDisabled )
						{
							Reach->AddToDebugRenderProxy(this);
						}
					}
				}

				if( Nav->bBlocked )
				{
					new(Stars) FWireStar( Nav->Location + FVector(0,0,40), FColor(255,0,0), 5 );
				}
			}
		}

		ACoverLink *Link = Cast<ACoverLink>(InComponent->GetOwner());
		OwnerLink = Link; // Keep a pointer to the Actor that owns this cover.
		// only draw if selected in the editor, or if SHOW_Cover in game
		UBOOL bDrawAsSelected = Link && (Link->IsSelected() || bCreatedInGame);
		const UBOOL bOverlapped = !bDrawAsSelected;
		if( Link && !bDrawAsSelected )
		{
			bDrawAsSelected = IsOverlapSlotSelected( Link, -1 );
		}			

		if (Link != NULL) 
			// && ((GIsGame && (Context.View->ShowFlags & SHOW_Cover)) || ((Context.View->ShowFlags & SHOW_Cover) && bDrawAsSelected)))
		{
			const INT NumSlots = Link->Slots.Num();

			FVector LastLocation( 0.0f, 0.0f, 0.0f );
			for (INT SlotIdx = 0; SlotIdx < NumSlots; SlotIdx++)
			{
				if( bOverlapped )
				{
					if( !IsOverlapSlotSelected( Link, SlotIdx ) )
					{
						continue;
					}
				}

				const FVector SlotLocation = Link->GetSlotLocation(SlotIdx);
				const FRotator SlotRotation = Link->GetSlotRotation(SlotIdx);

				FCoverSlot& Slot = Link->Slots(SlotIdx);
				if( GIsGame && !Slot.bEnabled )
				{
					continue;
				}

				const FCoverMeshes &MeshSet = InComponent->Meshes((INT)Slot.CoverType);

				new(CoverArrowLines) FArrowLine(SlotLocation, SlotLocation + SlotRotation.Vector() * 64.f, FColor(0,255,0));

				// update the translation
				const FRotationTranslationMatrix SlotLocalToWorld( FRotator(SlotRotation.Quaternion() * FRotator(0, 16384, 0).Quaternion()), SlotLocation + InComponent->LocationOffset );

				// draw the base mesh
				CreateCoverMesh( MeshSet.Base, SlotIdx, SlotLocalToWorld,Slot.bSelected);

				// auto adjust
				CreateCoverMesh( Link->bAutoAdjust ? InComponent->AutoAdjustOn : InComponent->AutoAdjustOff, INDEX_NONE, SlotLocalToWorld, Slot.bSelected);

				// disabled
				if (Link->bDisabled || !Slot.bEnabled)
				{
					CreateCoverMesh( InComponent->Disabled, INDEX_NONE, SlotLocalToWorld,Slot.bSelected);
				}
				// player only
				if( Slot.bPlayerOnly )
				{
					CreateCoverMesh( MeshSet.PlayerOnly, INDEX_NONE, SlotLocalToWorld, Slot.bSelected );
				}
				// left
				if (Slot.bLeanLeft)
				{
					CreateCoverMesh( MeshSet.LeanLeft, INDEX_NONE, SlotLocalToWorld,Slot.bSelected);
				}
				// right
				if (Slot.bLeanRight)
				{
					CreateCoverMesh( MeshSet.LeanRight, INDEX_NONE, SlotLocalToWorld,Slot.bSelected);
				}
				if (Slot.bAllowCoverSlip)
				{
					// slip left
					if (Slot.bCanCoverSlip_Left)
					{
						CreateCoverMesh( MeshSet.SlipLeft, INDEX_NONE, SlotLocalToWorld,Slot.bSelected);
					}
					// slip right
					if (Slot.bCanCoverSlip_Right)
					{
						CreateCoverMesh( MeshSet.SlipRight, INDEX_NONE, SlotLocalToWorld,Slot.bSelected);
					}
				}
				if (Slot.bAllowSwatTurn)
				{
					// swat left
					if (Slot.bCanSwatTurn_Left)
					{
						CreateCoverMesh( MeshSet.SwatLeft, INDEX_NONE, SlotLocalToWorld,Slot.bSelected);
					}
					if (Slot.bCanSwatTurn_Right)
					{
						CreateCoverMesh( MeshSet.SwatRight, INDEX_NONE, SlotLocalToWorld,Slot.bSelected);
					}
				}
				// mantle
				if( Slot.bAllowMantle && Slot.bCanMantle )
				{
					CreateCoverMesh( MeshSet.Mantle, INDEX_NONE, SlotLocalToWorld,Slot.bSelected);
				}
				if( Slot.bAllowClimbUp && Slot.bCanClimbUp )
				{
					CreateCoverMesh( MeshSet.Climb, INDEX_NONE, SlotLocalToWorld,Slot.bSelected);
				}
				if( Slot.bAllowPopup && Slot.bCanPopUp )
				{
					CreateCoverMesh( MeshSet.PopUp, INDEX_NONE, SlotLocalToWorld,Slot.bSelected);
				}

				// draw a line to the slot from the last slot
				if (SlotIdx > 0)
				{
					new(CoverLines) FDebugLine(LastLocation, SlotLocation, FColor(255,0,0));
				}
				else if (Link->bLooped)
				{
					new(CoverLines) FDebugLine(Link->GetSlotLocation(NumSlots-1), SlotLocation, FColor(255,0,0));
				}

				// if this slot is selected,
				if (Slot.bSelected)
				{
					if( Link->bDebug_FireLinks )
					{
						// draw all fire links
						for( INT LinkIdx = 0; LinkIdx < Slot.FireLinks.Num(); LinkIdx++ )
						{
							FFireLink &FireLink = Slot.FireLinks(LinkIdx);
							ACoverSlotMarker* TargetMarker = Cast<ACoverSlotMarker>(~FireLink.TargetMarker);

							if( TargetMarker == NULL )
							{
								continue;
							}

							if( TargetMarker->OwningSlot.Link == NULL)
							{
								debugf(TEXT("Warning! %s Is not linked to any CoverLink!, skipping.."),*TargetMarker->GetName());
								continue;
							}

							ACoverLink*  DestLink	 = TargetMarker->OwningSlot.Link;
							INT			 DestSlotIdx = TargetMarker->OwningSlot.SlotIdx;
							const FColor DrawColor	 = FireLink.bFallbackLink ? FColor(128,0,0) : FColor(255,0,0);

							for( INT ItemIdx = 0; ItemIdx < FireLink.Items.Num(); ItemIdx++ )
							{
								FFireLinkItem& Item = FireLink.Items(ItemIdx);
								FVector SrcViewPt	= Link->GetSlotViewPoint( SlotIdx, Item.SrcType, Item.SrcAction );
								FVector DestViewPt	= DestLink->GetSlotViewPoint( DestSlotIdx, Item.DestType, Item.DestAction );

								new(CoverArrowLines) FArrowLine( SrcViewPt, DestViewPt, DrawColor );
							}
						}
						//disable forcefirelinks
						/*for (INT LinkIdx = 0; LinkIdx < Slot.ForcedFireLinks.Num(); LinkIdx++)
						{
							FFireLink &FireLink = Slot.ForcedFireLinks(LinkIdx);
							ACoverLink *TargetLink = Cast<ACoverLink>(~FireLink.TargetLink);

							if (FireLink.CoverActions.Num() > 0 &&
								TargetLink != NULL)
							{
								FVector X, Y, Z;
								FRotationMatrix(Link->Rotation + Slot.RotationOffset).GetAxes(X,Y,Z);
								FVector TargetLocation = TargetLink->GetSlotLocation(FireLink.TargetSlotIdx);
								for (INT ActionIdx = 0; ActionIdx < FireLink.CoverActions.Num(); ActionIdx++)
								{
									FVector StartLocation = Link->GetSlotViewPoint( SlotIdx, FireLink.CoverType, FireLink.CoverActions(ActionIdx) );
									const FColor DrawColor = FColor(192,0,0);
									new(CoverArrowLines) FArrowLine(StartLocation, TargetLocation, DrawColor);
								}
							}
						}*/
					}
					if( Link->bDebug_ExposedLinks )
					{
						for( INT LinkIdx = 0; LinkIdx < Slot.ExposedFireLinks.Num(); LinkIdx++ )
						{
							ACoverSlotMarker *TargetMarker = Cast<ACoverSlotMarker>(~Slot.ExposedFireLinks(LinkIdx).TargetMarker);
							if( TargetMarker )
							{
								const FColor DrawColor = FColor(128,128,128);
								new(CoverArrowLines) FArrowLine(Link->GetSlotViewPoint( SlotIdx ), TargetMarker->GetSlotLocation(), DrawColor);
							}
						}
					}
					if( Link->bDebug_DangerLinks )
					{
						for( INT DangerIdx = 0; DangerIdx < Slot.DangerLinks.Num(); DangerIdx++ )
						{
							ANavigationPoint* DangerNav = (ANavigationPoint*)~Slot.DangerLinks(DangerIdx).DangerNav;
							if( DangerNav != NULL )
							{
								new(CoverArrowLines) FArrowLine(Link->GetSlotViewPoint(SlotIdx), DangerNav->Location + FVector(0,0,20), FColor(0,0,0,255));
							}
						}
					}
				}

				// save the location for the next slot
				LastLocation = SlotLocation;
				// draw a line from the owning link
				new(CoverDashedLines) FDashedLine(Link->Location, SlotLocation, FColor(0,0,255), 32 );
			}
		}
	}

	// FPrimitiveSceneProxy interface.

	virtual HHitProxy* CreateHitProxies(const UPrimitiveComponent* Component,TArray<TRefCountPtr<HHitProxy> >& OutHitProxies)
	{
		// Create hit proxies for the cover meshes.
		for(INT MeshIndex = 0;MeshIndex < CoverMeshes.Num();MeshIndex++)
		{
			FCoverStaticMeshInfo& CoverInfo = CoverMeshes(MeshIndex);
			if(CoverInfo.SlotIndex != INDEX_NONE)
			{
				CoverInfo.HitProxy = new HActorComplex(OwnerLink,TEXT("Slot"),CoverInfo.SlotIndex);
				OutHitProxies.AddItem(CoverInfo.HitProxy);
			}
		}
		return FPrimitiveSceneProxy::CreateHitProxies(Component,OutHitProxies);
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
		PDI->SetHitProxy(NULL);
		// Draw lines and cylinders
		if ((View->Family->ShowFlags & SHOW_Paths) != 0)
		{
			FDebugRenderSceneProxy::DrawDynamicElements(PDI, View, DPGIndex, Flags);
		}

		// if in the editor (but no PIE), or SHOW_Cover is set
		const UBOOL bShowCover = ((View->Family->ShowFlags & SHOW_Cover) != 0) || !bCreatedInGame;
		if (bShowCover)
		{
			// Draw Cover Lines
			for(INT LineIdx=0; LineIdx<CoverLines.Num(); LineIdx++)
			{
				const FDebugLine& Line = CoverLines(LineIdx);
				PDI->DrawLine(Line.Start, Line.End, Line.Color, SDPG_World);
			}

			// Draw Cover Arrows
			for(INT LineIdx=0; LineIdx<CoverArrowLines.Num(); LineIdx++)
			{
				const FArrowLine& Line = CoverArrowLines(LineIdx);
				DrawLineArrow(PDI, Line.Start, Line.End, Line.Color, 8.0f);
			}


			// Draw Cover Dashed Lines
			for(INT DashIdx=0; DashIdx<CoverDashedLines.Num(); DashIdx++)
			{
				const FDashedLine& Dash = CoverDashedLines(DashIdx);
				DrawDashedLine(PDI, Dash.Start, Dash.End, Dash.Color, Dash.DashSize, SDPG_World);
			}

			// Draw all the cover meshes we want.
			for(INT i=0; i<CoverMeshes.Num(); i++)
			{
				FCoverStaticMeshInfo& CoverInfo = CoverMeshes(i);
				const UStaticMesh* StaticMesh = CoverInfo.Mesh;
				const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(0);

				PDI->SetHitProxy( CoverInfo.HitProxy );

				// Draw the static mesh elements.
				for(INT ElementIndex = 0;ElementIndex < LODModel.Elements.Num();ElementIndex++)
				{
					const FStaticMeshElement& Element = LODModel.Elements(ElementIndex);

					FMeshElement MeshElement;
					MeshElement.IndexBuffer = &LODModel.IndexBuffer;
					MeshElement.VertexFactory = &LODModel.VertexFactory;
					MeshElement.DynamicVertexData = NULL;
					MeshElement.MaterialRenderProxy = Element.Material->GetRenderProxy(CoverInfo.bSelected);
					MeshElement.LCI = NULL;
					MeshElement.LocalToWorld = CoverInfo.LocalToWorld;
					MeshElement.WorldToLocal = CoverInfo.LocalToWorld.Inverse();
					MeshElement.FirstIndex = Element.FirstIndex;
					MeshElement.NumPrimitives = Element.NumTriangles;
					MeshElement.MinVertexIndex = Element.MinVertexIndex;
					MeshElement.MaxVertexIndex = Element.MaxVertexIndex;
					MeshElement.UseDynamicData = FALSE;
					MeshElement.ReverseCulling = CoverInfo.LocalToWorld.Determinant() < 0.0f ? TRUE : FALSE;
					MeshElement.CastShadow = FALSE;
					MeshElement.Type = PT_TriangleList;
					MeshElement.DepthPriorityGroup = SDPG_World;

					PDI->DrawMesh( MeshElement );
				}
			}
		}
		PDI->SetHitProxy(NULL);
	}


	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		const UBOOL bVisible = (View->Family->ShowFlags & SHOW_Paths) || (View->Family->ShowFlags & SHOW_Cover);
		FPrimitiveViewRelevance Result;
		Result.bDynamicRelevance = IsShown(View) && bVisible;
		Result.SetDPG(SDPG_World,TRUE);
		if (IsShadowCast(View))
		{
			Result.bShadowRelevance = TRUE;
		}
		return Result;
	}

	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FDebugRenderSceneProxy::GetAllocatedSize() + CoverMeshes.GetAllocatedSize() + CoverLines.GetAllocatedSize() + CoverDashedLines.GetAllocatedSize() + CoverArrowLines.GetAllocatedSize() ); }

private:

	/** One sub-mesh to render as part of this cover setup. */
	struct FCoverStaticMeshInfo
	{
		FCoverStaticMeshInfo(UStaticMesh* InMesh, const INT InSlotIndex, const FMatrix &InLocalToWorld, const UBOOL bInSelected) :
			Mesh(InMesh),
			SlotIndex(InSlotIndex),
			LocalToWorld(InLocalToWorld),
			bSelected(bInSelected),
			HitProxy(NULL)
		{}

		UStaticMesh* Mesh;
		INT SlotIndex; // If INDEX_NONE, will not create hit proxy. Otherwise, used for hit proxy.
		FMatrix LocalToWorld;
		UBOOL bSelected;
		HHitProxy* HitProxy;
	};

	// Owning ACoverLink Actor.
	ACoverLink* OwnerLink;

	/** Cached array of information about each cover mesh. */
	TArray<FCoverStaticMeshInfo>	CoverMeshes;

	/** Cover-related lines. */
	TArray<FDebugLine>				CoverLines;

	/** Cover-related dashed lines. */
	TArray<FDashedLine>				CoverDashedLines;

	/** Cover-related arrowed lines. */
	TArray<FArrowLine>				CoverArrowLines;

	/** Creates a FCoverStaticMeshInfo with the given settings. */
	void CreateCoverMesh(UStaticMesh* InMesh, const INT InSlotIndex, const FMatrix &InLocalToWorld, const UBOOL bInSelected)
	{
		if(InMesh)
		{
			new(CoverMeshes) FCoverStaticMeshInfo(InMesh,InSlotIndex,InLocalToWorld,bInSelected);
		}
	}
};


void UCoverMeshComponent::UpdateBounds()
{
	Super::UpdateBounds();
	ACoverLink *Link = Cast<ACoverLink>(Owner);
	if (Link != NULL)
	{
		FBox BoundingBox = FBox(Link->Location,Link->Location).ExpandBy(Link->AlignDist);
		for (INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++)
		{
			FVector SlotLocation = Link->GetSlotLocation(SlotIdx);
			// create the bounds for this slot
			FBox SlotBoundingBox = FBox(SlotLocation,SlotLocation).ExpandBy(Link->StandHeight);
			// add to the component bounds
			BoundingBox += SlotBoundingBox;
			// extend the bounds to cover any firelinks
			FCoverSlot* Slot = &Link->Slots(SlotIdx);
			for( INT FireIdx = 0; FireIdx < Slot->FireLinks.Num(); FireIdx++ )
			{
				FFireLink* FL = &Slot->FireLinks(FireIdx);
				ACoverSlotMarker *TargetMarker = Cast<ACoverSlotMarker>(*FL->TargetMarker);
				if( FL && TargetMarker )
				{
					BoundingBox += TargetMarker->GetSlotLocation();
				}
			}
		}
		Bounds = Bounds + FBoxSphereBounds(BoundingBox);
	}
}

FPrimitiveSceneProxy* UCoverMeshComponent::CreateSceneProxy()
{
	UpdateMeshes();
	return new FCoverMeshSceneProxy(this,GIsGame);
}

UBOOL UCoverMeshComponent::ShouldRecreateProxyOnUpdateTransform() const
{
	// The cover mesh scene proxy caches a lot of transform dependent info, so it's easier to just recreate it when the transform changes.
	return TRUE;
}
