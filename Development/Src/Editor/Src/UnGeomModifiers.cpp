/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EditorPrivate.h"
#include "ScopedTransaction.h"
#include "BSPOps.h"

IMPLEMENT_CLASS(UGeomModifier)
IMPLEMENT_CLASS(UGeomModifier_Edit)
IMPLEMENT_CLASS(UGeomModifier_Extrude)
IMPLEMENT_CLASS(UGeomModifier_Clip)
IMPLEMENT_CLASS(UGeomModifier_Lathe)
IMPLEMENT_CLASS(UGeomModifier_Delete)
IMPLEMENT_CLASS(UGeomModifier_Create)
IMPLEMENT_CLASS(UGeomModifier_Flip)
IMPLEMENT_CLASS(UGeomModifier_Split)
IMPLEMENT_CLASS(UGeomModifier_Triangulate)
IMPLEMENT_CLASS(UGeomModifier_Turn)
IMPLEMENT_CLASS(UGeomModifier_Weld)

/*------------------------------------------------------------------------------
	UGeomModifier
------------------------------------------------------------------------------*/

/**
 * @return		The modifier's description string.
 */
const FString& UGeomModifier::GetModifierDescription() const
{ 
	return Description;
}

/**
 * Gives the individual modifiers a chance to do something the first time they are activated.
 */
void UGeomModifier::Initialize()
{
}

/**
 * @return		TRUE if the key was handled by this editor mode tool.
 */
UBOOL UGeomModifier::InputKey(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event)
{
	return FALSE;
}

/**
 * @return		TRUE if the delta was handled by this editor mode tool.
 */
UBOOL UGeomModifier::InputDelta(FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale)
{
	if( GEditorModeTools().GetCurrentModeID() == EM_Geometry )
	{
		if( !bInitialized )
		{
			const FEdModeGeometry* CurMode = static_cast<const FEdModeGeometry*>( GEditorModeTools().GetCurrentMode() );
			const FGeometryToolSettings* Settings = static_cast<const FGeometryToolSettings*>( CurMode->GetSettings() );
			if( !Settings->bAffectWidgetOnly )
			{
				Initialize();
			}
			bInitialized = TRUE;
		}
	}

	return FALSE;
}

/**
 * Applies the modifier.  Does nothing if the editor is not in geometry mode.
 *
 * @return		TRUE if something happened.
 */
UBOOL UGeomModifier::Apply()
{
	UBOOL bResult = FALSE;
	if( GEditorModeTools().GetCurrentModeID() == EM_Geometry )
	{
		StartTrans();
		bResult = OnApply();
		EndTrans();
		EndModify();
	}
	return bResult;
}

/**
 * Implements the modifier application.
 */
UBOOL UGeomModifier::OnApply()
{
	return FALSE;
}

/**
* @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
*/
UBOOL UGeomModifier::Supports(INT InSelType)
{
	return TRUE;
}

/**
 * Interface for displaying error messages.
 *
 * @param	InErrorMsg		The error message to display.
 */
void UGeomModifier::GeomError(const FString& InErrorMsg)
{
	appMsgf( AMT_OK, *FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_Modifier"), *GetModifierDescription(), *InErrorMsg) ) );
}

/**
 * Starts the modification of geometry data.
 */
UBOOL UGeomModifier::StartModify()
{
	bInitialized = FALSE;
	return TRUE;
}

/**
 * Ends the modification of geometry data.
 */
UBOOL UGeomModifier::EndModify()
{
	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Transaction tracking.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
	/**
	 * @return		The shared transaction object used by 
	 */
	static FScopedTransaction*& StaticTransaction()
	{
		static FScopedTransaction* STransaction = NULL;
		return STransaction;
	}

	/**
	 * Ends the outstanding transaction, if one exists.
	 */
	static void EndTransaction()
	{
		delete StaticTransaction();
		StaticTransaction() = NULL;
	}

	/**
	 * Begins a new transaction, if no outstanding transaction exists.
	 */
	static void BeginTransaction(const TCHAR* SessionName)
	{
		if ( !StaticTransaction() )
		{
			StaticTransaction() = new FScopedTransaction( SessionName );
		}
	}
} // namespace

/**
 * Handles the starting of transactions against the selected ABrushes.
 */
void UGeomModifier::StartTrans()
{
	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return;
	}

	FEdModeGeometry* CurMode = static_cast<FEdModeGeometry*>( GEditorModeTools().GetCurrentMode() );

	// Record the current selection list into the selected brushes.
	for( FEdModeGeometry::TGeomObjectIterator Itor( CurMode->GeomObjectItor() ) ; Itor ; ++Itor )
	{
		FGeomObject* go = *Itor;
		go->CompileSelectionOrder();

		ABrush* Actor = go->GetActualBrush();

		Actor->SavedSelections.Empty();
		FGeomSelection* gs = NULL;

		for( INT v = 0 ; v < go->VertexPool.Num() ; ++v )
		{
			FGeomVertex* gv = &go->VertexPool(v);
			if( gv->IsSelected() )
			{
				gs = new( Actor->SavedSelections )FGeomSelection;
				gs->Type = GST_Vertex;
				gs->Index = v;
				gs->SelectionIndex = gv->GetSelectionIndex();
				gs->SelStrength = gv->GetSelStrength();
			}
		}
		for( INT e = 0 ; e < go->EdgePool.Num() ; ++e )
		{
			FGeomEdge* ge = &go->EdgePool(e);
			if( ge->IsSelected() )
			{
				gs = new( Actor->SavedSelections )FGeomSelection;
				gs->Type = GST_Edge;
				gs->Index = e;
				gs->SelectionIndex = ge->GetSelectionIndex();
				gs->SelStrength = ge->GetSelStrength();
			}
		}
		for( INT p = 0 ; p < go->PolyPool.Num() ; ++p )
		{
			FGeomPoly* gp = &go->PolyPool(p);
			if( gp->IsSelected() )
			{
				gs = new( Actor->SavedSelections )FGeomSelection;
				gs->Type = GST_Poly;
				gs->Index = p;
				gs->SelectionIndex = gp->GetSelectionIndex();
				gs->SelStrength = gp->GetSelStrength();
			}
		}
	}

	// Start the transaction.
	BeginTransaction( *FString::Printf( LocalizeSecure(LocalizeUnrealEd(TEXT("Modifier_F")), *GetModifierDescription()) ) );

	// Mark all selected brushes as modified.
	for( FEdModeGeometry::TGeomObjectIterator Itor( CurMode->GeomObjectItor() ) ; Itor ; ++Itor )
	{
		FGeomObject* go = *Itor;
		ABrush* Actor = go->GetActualBrush();

		Actor->Modify();
	}
}

/**
 * Handles the stopping of transactions against the selected ABrushes.
 */
void UGeomModifier::EndTrans()
{
	EndTransaction();
}

/*------------------------------------------------------------------------------
	UGeomModifier_Edit
------------------------------------------------------------------------------*/

/**
 * @return		TRUE if the delta was handled by this editor mode tool.
 */
UBOOL UGeomModifier_Edit::InputDelta(FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale)
{
	if( UGeomModifier::InputDelta( InViewportClient, InViewport, InDrag, InRot, InScale ) )
	{
		return TRUE;
	}

	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return FALSE;
	}

	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	FModeTool_GeometryModify* tool = (FModeTool_GeometryModify*)mode->GetCurrentTool();
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)mode->GetSettings())->SelectionType;

	TArray<FGeomVertex*> VertexList;

	/**
	 * All geometry objects can be manipulated by transforming the vertices that make
	 * them up.  So based on the type of thing we're editing, we need to dig for those
	 * vertices a little differently.
	 */

	for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
	{
		FGeomObject* go = *Itor;

		switch( seltype )
		{
			case GST_Object:
				for( INT v = 0 ; v < go->VertexPool.Num() ; ++v )
				{
					VertexList.AddUniqueItem( &go->VertexPool(v) );
				}
				break;

			case GST_Poly:
				for( INT p = 0 ; p < go->PolyPool.Num() ; ++p )
				{
					FGeomPoly* gp = &go->PolyPool(p);
					if( gp->IsSelected() )
					{
						for( INT e = 0 ; e < gp->EdgeIndices.Num() ; ++e )
						{
							FGeomEdge* ge = &go->EdgePool( gp->EdgeIndices(e) );
							VertexList.AddUniqueItem( &go->VertexPool( ge->VertexIndices[0] ) );
							VertexList.AddUniqueItem( &go->VertexPool( ge->VertexIndices[1] ) );
						}
					}
				}
				break;

			case GST_Edge:
				for( INT e = 0 ; e < go->EdgePool.Num() ; ++e )
				{
					FGeomEdge* ge = &go->EdgePool(e);
					if( ge->IsSelected() )
					{
						VertexList.AddUniqueItem( &go->VertexPool( ge->VertexIndices[0] ) );
						VertexList.AddUniqueItem( &go->VertexPool( ge->VertexIndices[1] ) );
					}
				}
				break;

			case GST_Vertex:
				for( INT v = 0 ; v < go->VertexPool.Num() ; ++v )
				{
					FGeomVertex* gv = &go->VertexPool(v);
					if( gv->IsSelected() )
					{
						VertexList.AddUniqueItem( gv );
					}
				}
				break;
		}
	}


	// If we didn't move any vertices, then tell the caller that we didn't handle the input.
	// This allows LDs to drag brushes around in geometry mode as along as no geometry
	// objects are selected.
	if( !VertexList.Num() )
	{
		return FALSE;
	}

	// If we're trying to rotate vertices, only allow that if Shift is held down.  This just makes it easier
	// to rotate brushes around while working in geometry mode, since you rarely ever want to rotate vertices
	FRotator FinalRot = InRot;
	if( seltype == GST_Vertex && !InViewportClient->Input->IsShiftPressed() )
	{
		FinalRot = FRotator( 0.0f, 0.0f, 0.0f );
	}

	if( InDrag.IsZero() && FinalRot.IsZero() && InScale.IsZero() )
	{
		// No change
		return FALSE;
	}

	/**
	 * We first generate a list of unique vertices and then transform that list
	 * in one shot.  This prevents vertices from being touched more than once (which
	 * would result in them transforming x times as fast as others).
	 */
	for( INT x = 0 ; x < VertexList.Num() ; ++x )
	{
		mode->ApplyToVertex( VertexList(x), InDrag, FinalRot, InScale );
	}
	
	return TRUE;
}

/*------------------------------------------------------------------------------
	UGeomModifier_Extrude
------------------------------------------------------------------------------*/

/**
 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
 */
UBOOL UGeomModifier_Extrude::Supports(INT InSelType)
{
	UBOOL bResult = TRUE;
	switch( InSelType )
	{
		case GST_Object:
		case GST_Edge:
		case GST_Vertex:
			bResult = FALSE;
			break;
	}

	return bResult;
}

/**
 * Gives the individual modifiers a chance to do something the first time they are activated.
 */
void UGeomModifier_Extrude::Initialize()
{
	Apply( GEditor->Constraints.GetGridSize(), 1 );
}

/**
 * Implements the modifier application.
 */
UBOOL UGeomModifier_Extrude::OnApply()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();

	// When applying via the keyboard, we force the local coordinate system.
	const ECoordSystem SaveCS = GEditorModeTools().CoordSystem;
	GEditorModeTools().CoordSystem = COORD_Local;

	mode->GetModifierWindow().FinalizePropertyWindowValues();
	Apply( Length, Segments );

	// Restore the coordinate system.
	GEditorModeTools().CoordSystem = SaveCS;

	return TRUE;
}

void UGeomModifier_Extrude::Apply(INT InLength, INT InSegments)
{
	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return;
	}

	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)mode->GetSettings())->SelectionType;

	// Force user input to be valid

	InLength = Max( 1, InLength );
	InSegments = Max( 1, InSegments );

	// Do it

	for( INT s = 0 ; s < InSegments ; ++s )
	{
		for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
		{
			FGeomObject* go = *Itor;

			switch( seltype )
			{
				case GST_Poly:
				{
					go->SendToSource();

					TArray<INT> Selections;

					for( INT p = 0 ; p < go->PolyPool.Num() ; ++p )
					{
						FGeomPoly* gp = &go->PolyPool(p);

						if( gp->IsSelected() )
						{
							Selections.AddItem( p );
							FPoly OldPoly = *gp->GetActualPoly();

							FVector Normal = mode->GetWidgetNormalFromCurrentAxis( gp );

							// Move the existing poly along the normal by InLength units.

							for( INT v = 0 ; v < gp->GetActualPoly()->Vertices.Num() ; ++v )
							{
								FVector* vtx = &gp->GetActualPoly()->Vertices(v);

								*vtx += Normal * InLength;
							}

							gp->GetActualPoly()->Base += Normal * InLength;

							// Create new polygons to connect the existing one back to the brush

							for( INT v = 0 ; v < OldPoly.Vertices.Num() ; ++v )
							{
								// Create new polygons from the edges of the old one

								FVector v0 = OldPoly.Vertices(v),
									v1 = OldPoly.Vertices( v+1 == OldPoly.Vertices.Num() ? 0 : v+1 );

								FPoly* NewPoly = new( gp->GetParentObject()->GetActualBrush()->Brush->Polys->Element )FPoly();

								NewPoly->Init();
								new(NewPoly->Vertices) FVector(v0);
								new(NewPoly->Vertices) FVector(v1);
								new(NewPoly->Vertices) FVector(v1 + (Normal * InLength));
								new(NewPoly->Vertices) FVector(v0 + (Normal * InLength));

								NewPoly->Normal = FVector(0,0,0);
								NewPoly->Base = v0;
								NewPoly->PolyFlags = OldPoly.PolyFlags;
								NewPoly->Material = OldPoly.Material;
								NewPoly->Finalize(go->GetActualBrush(),1);
							}
						}
					}

					go->FinalizeSourceData();
					go->GetFromSource();

					for( INT x = 0 ; x < Selections.Num() ; ++x )
					{
						go->PolyPool( Selections(x) ).Select(1);
					}
				}
				break;
			}
		}
	}
}

/*------------------------------------------------------------------------------
	UGeomModifier_Clip
------------------------------------------------------------------------------*/

namespace GeometryClipping {

/**
 * Iterates over all actors and assembles the set of AClipMarker actors in the world.
 */
static void GetAllClipMarkers(TArray<AClipMarker*>& OutClipMarkers)
{
	OutClipMarkers.Empty();
	for( FActorIterator It ; It ; ++It )
	{
		AClipMarker* ClipMarker = Cast<AClipMarker>( *It );
		if ( ClipMarker )
		{
			OutClipMarkers.AddItem( ClipMarker );
		}
	}
}

/**
 * Returns the number of AClipMarker actors currently in the world.
 */
static INT CountClipMarkers()
{
	TArray<AClipMarker*> ClipMarkers;
	GetAllClipMarkers( ClipMarkers );
	return ClipMarkers.Num();
}

/**
 * Destroys all AClipMarker actors in the world.
 */
static void DeleteAllClipMarkers(TArray<AClipMarker*>& ClipMarkers)
{
	for( INT ClipMarkerIndex = 0 ; ClipMarkerIndex < ClipMarkers.Num() ; ++ClipMarkerIndex )
	{
		AClipMarker* ClipMarker = ClipMarkers(ClipMarkerIndex);
		GWorld->EditorDestroyActor( ClipMarker, TRUE );
	}
	ClipMarkers.Empty();
}

/**
 * Destroys all AClipMarker actors in the world.
 */
static void DeleteAllClipMarkers()
{
	TArray<AClipMarker*> ClipMarkers;
	GetAllClipMarkers( ClipMarkers );
	DeleteAllClipMarkers( ClipMarkers );
}

/**
 * Adds an AClipMarker actor at the click location.
 */
static void AddClipMarker()
{
	TArray<AClipMarker*> ClipMarkers;
	GetAllClipMarkers( ClipMarkers );

	if( ClipMarkers.Num() > 2 )
	{
		// If there are 3 (or more) clipmarkers, already in the level delete them so the user can start fresh.
		DeleteAllClipMarkers( ClipMarkers );
	}
	else
	{
		GEditor->Exec( *FString::Printf(TEXT("ACTOR ADD CLASS=CLIPMARKER SNAP=1")) );
	}
}

/**
 * Builds a huge poly aligned with the specified plane.  This poly is
 * carved up by the calling routine and used as a capping poly following a clip operation.
 */
static FPoly BuildInfiniteFPoly(const FPlane& InPlane)
{
	FVector Axis1, Axis2;

	// Find two non-problematic axis vectors.
	InPlane.FindBestAxisVectors( Axis1, Axis2 );

	// Set up the FPoly.
	FPoly EdPoly;
	EdPoly.Init();
	EdPoly.Normal.X    = InPlane.X;
	EdPoly.Normal.Y    = InPlane.Y;
	EdPoly.Normal.Z    = InPlane.Z;
	EdPoly.Base        = EdPoly.Normal * InPlane.W;
	EdPoly.Vertices.AddItem( EdPoly.Base + Axis1*HALF_WORLD_MAX + Axis2*HALF_WORLD_MAX );
	EdPoly.Vertices.AddItem( EdPoly.Base - Axis1*HALF_WORLD_MAX + Axis2*HALF_WORLD_MAX );
	EdPoly.Vertices.AddItem( EdPoly.Base - Axis1*HALF_WORLD_MAX - Axis2*HALF_WORLD_MAX );
	EdPoly.Vertices.AddItem( EdPoly.Base + Axis1*HALF_WORLD_MAX - Axis2*HALF_WORLD_MAX );

	return EdPoly;
}

/**
 * Creates a giant brush aligned with the specified plane.
 *
 * @param	OutGiantBrush		[out] The new brush.
 * @param	InPlane				Plane with which to align the brush.
 * @param	SrcBrush			Specifies the csg operation for the new brush.
 */
static void BuildGiantBrush(ABrush& OutGiantBrush, const FPlane& InPlane, const ABrush& SrcBrush)
{
	OutGiantBrush.Location = FVector(0,0,0);
	OutGiantBrush.PrePivot = FVector(0,0,0);
	OutGiantBrush.CsgOper = SrcBrush.CsgOper;
	OutGiantBrush.SetFlags( RF_Transactional );
	if ( !SrcBrush.IsVolumeBrush() )
	{
		OutGiantBrush.SetFlags( RF_NotForClient|RF_NotForServer );
	}
	OutGiantBrush.PolyFlags = 0;

	verify( OutGiantBrush.Brush );
	verify( OutGiantBrush.Brush->Polys );

	OutGiantBrush.Brush->Polys->Element.Empty();

	// Create a list of vertices that can be used for the new brush
	FVector vtxs[8];

	FPlane Plane( InPlane );
	Plane = Plane.Flip();
	FPoly TempPoly = BuildInfiniteFPoly( Plane );
	TempPoly.Finalize(&OutGiantBrush,0);
	vtxs[0] = TempPoly.Vertices(0);
	vtxs[1] = TempPoly.Vertices(1);
	vtxs[2] = TempPoly.Vertices(2);
	vtxs[3] = TempPoly.Vertices(3);

	Plane = Plane.Flip();
	FPoly TempPoly2 = BuildInfiniteFPoly( Plane );
	vtxs[4] = TempPoly2.Vertices(0) + (TempPoly2.Normal * -(WORLD_MAX));	vtxs[5] = TempPoly2.Vertices(1) + (TempPoly2.Normal * -(WORLD_MAX));
	vtxs[6] = TempPoly2.Vertices(2) + (TempPoly2.Normal * -(WORLD_MAX));	vtxs[7] = TempPoly2.Vertices(3) + (TempPoly2.Normal * -(WORLD_MAX));

	// Create the polys for the new brush.
	FPoly newPoly;

	// TOP
	newPoly.Init();
	newPoly.Base = vtxs[0];
	newPoly.Vertices.AddItem( vtxs[0] );
	newPoly.Vertices.AddItem( vtxs[1] );
	newPoly.Vertices.AddItem( vtxs[2] );
	newPoly.Vertices.AddItem( vtxs[3] );
	newPoly.Finalize(&OutGiantBrush,0);
	new(OutGiantBrush.Brush->Polys->Element)FPoly(newPoly);

	// BOTTOM
	newPoly.Init();
	newPoly.Base = vtxs[4];
	newPoly.Vertices.AddItem( vtxs[4] );
	newPoly.Vertices.AddItem( vtxs[5] );
	newPoly.Vertices.AddItem( vtxs[6] );
	newPoly.Vertices.AddItem( vtxs[7] );
	newPoly.Finalize(&OutGiantBrush,0);
	new(OutGiantBrush.Brush->Polys->Element)FPoly(newPoly);

	// SIDES
	// 1
	newPoly.Init();
	newPoly.Base = vtxs[1];
	newPoly.Vertices.AddItem( vtxs[1] );
	newPoly.Vertices.AddItem( vtxs[0] );
	newPoly.Vertices.AddItem( vtxs[7] );
	newPoly.Vertices.AddItem( vtxs[6] );
	newPoly.Finalize(&OutGiantBrush,0);
	new(OutGiantBrush.Brush->Polys->Element)FPoly(newPoly);

	// 2
	newPoly.Init();
	newPoly.Base = vtxs[2];
	newPoly.Vertices.AddItem( vtxs[2] );
	newPoly.Vertices.AddItem( vtxs[1] );
	newPoly.Vertices.AddItem( vtxs[6] );
	newPoly.Vertices.AddItem( vtxs[5] );
	newPoly.Finalize(&OutGiantBrush,0);
	new(OutGiantBrush.Brush->Polys->Element)FPoly(newPoly);

	// 3
	newPoly.Init();
	newPoly.Base = vtxs[3];
	newPoly.Vertices.AddItem( vtxs[3] );
	newPoly.Vertices.AddItem( vtxs[2] );
	newPoly.Vertices.AddItem( vtxs[5] );
	newPoly.Vertices.AddItem( vtxs[4] );
	newPoly.Finalize(&OutGiantBrush,0);
	new(OutGiantBrush.Brush->Polys->Element)FPoly(newPoly);

	// 4
	newPoly.Init();
	newPoly.Base = vtxs[0];
	newPoly.Vertices.AddItem( vtxs[0] );
	newPoly.Vertices.AddItem( vtxs[3] );
	newPoly.Vertices.AddItem( vtxs[4] );
	newPoly.Vertices.AddItem( vtxs[7] );
	newPoly.Finalize(&OutGiantBrush,0);
	new(OutGiantBrush.Brush->Polys->Element)FPoly(newPoly);

	// Finish creating the new brush.
	OutGiantBrush.Brush->BuildBound();
}

/**
 * Clips the specified brush against the specified plane.
 *
 * @param		InPlane		The plane to clip against.
 * @param		InBrush		The brush to clip.
 * @return					The newly created brush representing the portion of the brush in the plane's positive halfspace.
 */
static ABrush* ClipBrushAgainstPlane(const FPlane& InPlane, ABrush* InBrush)
{
	ULevel* OldCurrentLevel = GWorld->CurrentLevel;

	// Create a giant brush in the level of the source brush to use in the intersection process.
	GWorld->CurrentLevel = InBrush->GetLevel();
	ABrush* GiantBrush = GWorld->SpawnBrush();

	GiantBrush->Brush = new( InBrush->GetOuter(), NAME_None, RF_NotForClient|RF_NotForServer )UModel( NULL );
	GiantBrush->BrushComponent->Brush = GiantBrush->Brush;
	BuildGiantBrush( *GiantBrush, InPlane, *InBrush );

	// Create a BSP for the brush that is being clipped.
	FBSPOps::bspBuild( InBrush->Brush, FBSPOps::BSP_Optimal, 15, 70, 1, 0 );
	FBSPOps::bspRefresh( InBrush->Brush, TRUE );
	FBSPOps::bspBuildBounds( InBrush->Brush );

	// Intersect the giant brush with the source brush's BSP.  This will give us the finished, clipping brush
	// contained inside of the giant brush.
	GEditor->bspBrushCSG( GiantBrush, InBrush->Brush, 0, CSG_Intersect, FALSE, FALSE, TRUE );
	FBSPOps::bspUnlinkPolys( GiantBrush->Brush );

	// You need at least 4 polys left over to make a valid brush.
	if( GiantBrush->Brush->Polys->Element.Num() < 4 )
	{
		GWorld->EditorDestroyActor( GiantBrush, TRUE );
		GiantBrush = NULL;
	}
	else
	{
		GiantBrush->CopyPosRotScaleFrom( InBrush );
		GiantBrush->PolyFlags = InBrush->PolyFlags;

		// Clean the brush up.
		for( INT poly = 0 ; poly < GiantBrush->Brush->Polys->Element.Num() ; poly++ )
		{
			FPoly* Poly = &(GiantBrush->Brush->Polys->Element(poly));
			Poly->iLink = poly;
			Poly->Normal = FVector(0,0,0);
			Poly->Finalize(GiantBrush,0);
		}

		// One final pass to clean the polyflags of all temporary settings.
		for( INT poly = 0 ; poly < GiantBrush->Brush->Polys->Element.Num() ; poly++ )
		{
			FPoly* Poly = &(GiantBrush->Brush->Polys->Element(poly));
			Poly->PolyFlags &= ~PF_EdCut;
			Poly->PolyFlags &= ~PF_EdProcessed;
		}

		// Move the new brush to where the new brush was to preserve brush ordering.
		ABrush* BuilderBrush = GWorld->GetBrush();
		if( InBrush == BuilderBrush )
		{
			// Special-case behaviour for the builder brush.

			// Copy the temporary brush back over onto the builder brush (keeping object flags)
			BuilderBrush->Modify();
			FBSPOps::csgCopyBrush( BuilderBrush, GiantBrush, BuilderBrush->GetFlags(), 0, 0, TRUE );
			GWorld->EditorDestroyActor( GiantBrush, FALSE );
			// Note that we're purposefully returning non-NULL here to report that the clip was successful,
			// even though the GiantBrush has been destroyed!
		}
		else
		{
			// Remove the old brush.
			const INT GiantBrushIndex = GWorld->CurrentLevel->Actors.Num() - 1;
			check( GWorld->CurrentLevel->Actors(GiantBrushIndex) == GiantBrush );
			GWorld->CurrentLevel->Actors.Remove(GiantBrushIndex);

			// Add the new brush right after the old brush.
			const INT OldBrushIndex = GWorld->CurrentLevel->Actors.FindItemIndex( InBrush );
			check( OldBrushIndex != INDEX_NONE );
			GWorld->CurrentLevel->Actors.InsertItem( GiantBrush, OldBrushIndex+1 );
		}
	}

	// Restore the current level.
	GWorld->CurrentLevel = OldCurrentLevel;

	return GiantBrush;
}

} // namespace GeometryClipping

/**
 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
 */
UBOOL UGeomModifier_Clip::Supports(INT InSelType)
{
	switch( InSelType )
	{
		case GST_Vertex:
		case GST_Poly:
		case GST_Edge:
			return FALSE;
	}

	return TRUE;
}

/**
 * Implements the modifier application.
 */
UBOOL UGeomModifier_Clip::OnApply()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();

	// When applying via the keyboard, we force the local coordinate system.
	const ECoordSystem SaveCS = GEditorModeTools().CoordSystem;
	GEditorModeTools().CoordSystem = COORD_Local;

	mode->GetModifierWindow().FinalizePropertyWindowValues();
	ApplyClip();

	// Restore the coordinate system.
	GEditorModeTools().CoordSystem = SaveCS;

	return TRUE;
}

void UGeomModifier_Clip::ApplyClip()
{
	if ( !GLastKeyLevelEditingViewportClient )
	{
		return;
	}

	// Assemble the set of selected non-builder brushes.
	TArray<ABrush*> Brushes;
	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if( Actor->IsBrush() )
		{
			Brushes.AddItem( static_cast<ABrush*>( Actor ) );
		}
	}

	// Do nothing if no brushes are selected.
	if ( Brushes.Num() == 0 )
	{
		return;
	}

	// Gather a list of all clip markers in the level.
	TArray<AActor*> ClipMarkers;
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if( Actor->IsA(AClipMarker::StaticClass()) )
		{
			ClipMarkers.AddItem( Actor );
		}
	}

	// Make sure enough clip markers have been placed.
	const UBOOL bIsOrtho = GLastKeyLevelEditingViewportClient->IsOrtho();
	if( (bIsOrtho && ClipMarkers.Num() < 2) ||
		(!bIsOrtho && ClipMarkers.Num() < 3) )
	{
		GeomError( *LocalizeUnrealEd(TEXT("Error_NotEnoughClipMarkers")) );
		return;
	}

	// Create a clipping plane based on ClipMarkers present in the level.
	const FVector vtx1 = ClipMarkers(0)->Location;
	const FVector vtx2 = ClipMarkers(1)->Location;
	FVector vtx3;

	if( ClipMarkers.Num() == 3 )
	{
		// If we have 3 points, just grab the third one to complete the plane.
		vtx3 = ClipMarkers(2)->Location;
	}
	else
	{
		// If we only have 2 points, we will assume the third based on the viewport.
		// (With just 2 points, we can only use ortho viewports)

		vtx3 = vtx1;

		if( GLastKeyLevelEditingViewportClient->IsOrtho() )
		{
			switch( GLastKeyLevelEditingViewportClient->ViewportType )
			{
			case LVT_OrthoXY:
				vtx3.Z -= 64;
				break;

			case LVT_OrthoXZ:
				vtx3.Y -= 64;
				break;

			case LVT_OrthoYZ:
				vtx3.X -= 64;
				break;
			}
		}
	}

	// Perform the clip.
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("BrushClip")) );

		GEditor->SelectNone( FALSE, TRUE );

		// Clear all clip markers.	
		GeometryClipping::DeleteAllClipMarkers();

		// Clip the brush list.
		TArray<ABrush*> NewBrushes;
		TArray<ABrush*> OldBrushes;

		for ( INT BrushIndex = 0 ; BrushIndex < Brushes.Num() ; ++BrushIndex )
		{
			ABrush* SrcBrush = Brushes( BrushIndex );

			// Compute a clipping plane in the local frame of the brush.
			const FMatrix ToBrushLocal( SrcBrush->WorldToLocal() );
			const FVector LocalVtx1( ToBrushLocal.TransformFVector( vtx1 ) );
			const FVector LocalVtx2( ToBrushLocal.TransformFVector( vtx2 ) );
			const FVector LocalVtx3( ToBrushLocal.TransformFVector( vtx3 ) );

			FVector PlaneNormal( (LocalVtx2 - LocalVtx1) ^ (LocalVtx3 - LocalVtx1) );
			if( PlaneNormal.SizeSquared() < THRESH_ZERO_NORM_SQUARED )
			{
				GeomError( *LocalizeUnrealEd(TEXT("Error_ClipUnableToComputeNormal")) );
				continue;
			}
			PlaneNormal.Normalize();

			FPlane ClippingPlane( LocalVtx1, PlaneNormal );
			if ( bFlipNormal )
			{
				ClippingPlane = ClippingPlane.Flip();
			}
			
			// Is the brush a builder brush?
			const UBOOL bIsBuilderBrush = SrcBrush->IsABuilderBrush();

			// Perform the clip.
			UBOOL bCreatedBrush = FALSE;
			ABrush* NewBrush = GeometryClipping::ClipBrushAgainstPlane( ClippingPlane, SrcBrush );
			if ( NewBrush )
			{
				// Select the src brush for builders, or the returned brush for non-builders.
				if ( !bIsBuilderBrush )
				{
					NewBrushes.AddItem( NewBrush );
				}
				else
				{
					NewBrushes.AddItem( SrcBrush );
				}
				bCreatedBrush = TRUE;
			}

			// If we're doing a split instead of just a plain clip . . .
			if( bSplit )
			{
				// Don't perform a second clip if the builder brush was already split.
				if ( !bIsBuilderBrush || !bCreatedBrush )
				{
					// Clip the brush against the flipped clipping plane.
					ABrush* NewBrush2 = GeometryClipping::ClipBrushAgainstPlane( ClippingPlane.Flip(), SrcBrush );
					if ( NewBrush2 )
					{
						// We don't add the brush to the list of new brushes, so that only new brushes
						// in the non-cleaved halfspace of the clipping plane will be selected.
						bCreatedBrush = TRUE;
					}
				}
			}

			// Destroy source brushes that aren't builders.
			if ( !bIsBuilderBrush )
			{
				OldBrushes.AddItem( SrcBrush );
			}
		}

		// Delete old brushes.
		for ( INT BrushIndex = 0 ; BrushIndex < OldBrushes.Num() ; ++BrushIndex )
		{
			ABrush* OldBrush = OldBrushes( BrushIndex );
			GWorld->EditorDestroyActor( OldBrush, TRUE );
		}

		// Select new brushes.
		for ( INT BrushIndex = 0 ; BrushIndex < NewBrushes.Num() ; ++BrushIndex )
		{
			ABrush* NewBrush = NewBrushes( BrushIndex );
			GEditor->SelectActor( NewBrush, TRUE, NULL, FALSE );
		}

		// Notify editor of new selection state.
		GEditor->NoteSelectionChange();
	}

	FEdModeGeometry* Mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	Mode->FinalizeSourceData();
	Mode->GetFromSource();
}

/**
 * @return		TRUE if the key was handled by this editor mode tool.
 */
UBOOL UGeomModifier_Clip::InputKey(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event)
{
	UBOOL bResult = FALSE;

	if( ViewportClient->IsOrtho() && Event == IE_Pressed )
	{
		const UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);
		const UBOOL bShiftDown = Viewport->KeyState(KEY_LeftShift) || Viewport->KeyState(KEY_RightShift);
		const UBOOL bAltDown = Viewport->KeyState(KEY_LeftAlt) || Viewport->KeyState(KEY_RightAlt);

		// CTRL+RightClick adds a ClipMarker to the world.
		if( bCtrlDown && !bShiftDown && !bAltDown && Key == KEY_RightMouseButton )
		{
			GeometryClipping::AddClipMarker();
			bResult = TRUE;
		}
	}

	return bResult;
}

/*------------------------------------------------------------------------------
	UGeomModifier_Lathe
------------------------------------------------------------------------------*/

/**
 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
 */
UBOOL UGeomModifier_Lathe::Supports(INT InSelType)
{
	switch( InSelType )
	{
		case GST_Object:
		case GST_Edge:
		case GST_Vertex:
			return FALSE;
	}

	return TRUE;
}

/**
 * Gives the individual modifiers a chance to do something the first time they are activated.
 */
void UGeomModifier_Lathe::Initialize()
{
	Apply( TotalSegments, Segments, (EAxis)Axis );
}

/**
 * Implements the modifier application.
 */
UBOOL UGeomModifier_Lathe::OnApply()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();

	// When applying via the keyboard, we force the local coordinate system.
	const ECoordSystem SaveCS = GEditorModeTools().CoordSystem;
	GEditorModeTools().CoordSystem = COORD_Local;

	mode->GetModifierWindow().FinalizePropertyWindowValues();
	Apply( TotalSegments, Segments, (EAxis)Axis );

	// Restore the coordinate system.
	GEditorModeTools().CoordSystem = SaveCS;

	return TRUE;
}

void UGeomModifier_Lathe::Apply(INT InTotalSegments, INT InSegments, EAxis InAxis)
{
	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return;
	}

	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)mode->GetSettings())->SelectionType;

	// Force user input to be valid

	InTotalSegments = Max( 3, InTotalSegments );
	InSegments = Clamp( InSegments, 1, InTotalSegments );

	// Do it

	for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
	{
		FGeomObject* go = *Itor;

		switch( seltype )
		{
			case GST_Poly:
			{
				go->SendToSource();

				for( INT p = 0 ; p < go->PolyPool.Num() ; ++p )
				{
					FGeomPoly* gp = &go->PolyPool(p);
					TArray<FVector> Vertices;

					if( gp->IsSelected() )
					{

						// Compute the angle step

						FLOAT Angle = 65536.f / (FLOAT)InTotalSegments;

						// If the widget is on the other side of the normal plane for this poly

						//FPlane plane(  )

						//Angle *= -1;

						FPoly OldPoly = *gp->GetActualPoly();
						for( INT s = 0 ; s <= InSegments ; ++s )
						{
							FMatrix matrix = FMatrix::Identity;
							switch( InAxis )
							{
								case AXIS_X:
									matrix = FRotationMatrix( FRotator( Angle*s, 0, 0 ) );
									break;
								case AXIS_Y:
									matrix = FRotationMatrix( FRotator( 0, Angle*s, 0 ) );
									break;
								default:
									matrix = FRotationMatrix( FRotator( 0, 0, Angle*s ) );
									break;
							}

							for( INT v = 0 ; v < OldPoly.Vertices.Num() ; ++v )
							{
								FVector Wk = go->GetActualBrush()->LocalToWorld().TransformFVector( OldPoly.Vertices(v) );

								Wk -= GEditorModeTools().SnappedLocation;
								Wk = matrix.TransformFVector( Wk );
								Wk += GEditorModeTools().SnappedLocation;

								Wk = go->GetActualBrush()->WorldToLocal().TransformFVector( Wk );

								Vertices.AddUniqueItem( Wk );
							}
						}

						// Create the new polygons

						for( INT s = 0 ; s < InSegments ; ++s )
						{
							for( INT v = 0 ; v < OldPoly.Vertices.Num() ; ++v )
							{
								FPoly* NewPoly = new( go->GetActualBrush()->Brush->Polys->Element )FPoly();
								NewPoly->Init();

								INT idx0 = (s * OldPoly.Vertices.Num()) + v;
								INT idx1 = (s * OldPoly.Vertices.Num()) + ((v+1 < OldPoly.Vertices.Num()) ? v+1 : 0);
								INT idx2 = idx1 + OldPoly.Vertices.Num();
								INT idx3 = idx0 + OldPoly.Vertices.Num();

								new(NewPoly->Vertices) FVector(Vertices( idx0 ));
								new(NewPoly->Vertices) FVector(Vertices( idx1 ));
								new(NewPoly->Vertices) FVector(Vertices( idx2 ));
								new(NewPoly->Vertices) FVector(Vertices( idx3 ));
							}
						}

						// Create a cap polygon (if we aren't doing a complete circle)

						if( InSegments < InTotalSegments )
						{
							FPoly* NewPoly = new( go->GetActualBrush()->Brush->Polys->Element )FPoly();
							NewPoly->Init();

							for( INT v = 0 ; v < OldPoly.Vertices.Num() ; ++v )
							{
								new(NewPoly->Vertices) FVector(Vertices( Vertices.Num() - OldPoly.Vertices.Num() + v ));
							}

							NewPoly->Normal = FVector(0,0,0);
							NewPoly->Base = Vertices(0);
							NewPoly->PolyFlags = PF_DefaultFlags;

							NewPoly->Finalize(go->GetActualBrush(),1);
						}

						// Mark the original polygon for deletion later

						go->GetActualBrush()->Brush->Polys->Element( gp->ActualPolyIndex ).PolyFlags |= PF_GeomMarked;
					}
				}
			}
			break;
		}

		// Remove any polys that were marked for deletion

		for( INT p = 0 ; p < go->GetActualBrush()->Brush->Polys->Element.Num() ; ++p )
		{
			if( (go->GetActualBrush()->Brush->Polys->Element( p ).PolyFlags&PF_GeomMarked) > 0 )
			{
				go->GetActualBrush()->Brush->Polys->Element.Remove( p );
				p = -1;
			}
		}
	}

	mode->FinalizeSourceData();
	mode->GetFromSource();
}

/*------------------------------------------------------------------------------
	UGeomModifier_Delete
------------------------------------------------------------------------------*/

/**
 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
 */
UBOOL UGeomModifier_Delete::Supports(INT InSelType)
{
	switch( InSelType )
	{
		case GST_Poly:
		case GST_Vertex:
			return TRUE;
	}

	return FALSE;
}

/**
 * Implements the modifier application.
 */
UBOOL UGeomModifier_Delete::OnApply()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)mode->GetSettings())->SelectionType;
	UBOOL bHandled = FALSE;

	for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
	{
		FGeomObject* go = *Itor;

		switch( seltype )
		{
			case GST_Poly:
			{
				for( INT p = 0 ; p < go->PolyPool.Num() ; ++p )
				{
					FGeomPoly* gp = &go->PolyPool(p);

					if( gp->IsSelected() )
					{
						gp->GetParentObject()->GetActualBrush()->Brush->Polys->Element( gp->ActualPolyIndex ).PolyFlags |= PF_GeomMarked;
						bHandled = 1;
					}
				}

				for( INT p = 0 ; p < go->GetActualBrush()->Brush->Polys->Element.Num() ; ++p )
				{
					if( (go->GetActualBrush()->Brush->Polys->Element( p ).PolyFlags&PF_GeomMarked) > 0 )
					{
						go->GetActualBrush()->Brush->Polys->Element.Remove( p );
						p = -1;
					}
				}
			}
			break;

			case GST_Vertex:
			{
				for( INT v = 0 ; v < go->VertexPool.Num() ; ++v )
				{
					FGeomVertex* gv = &go->VertexPool(v);

					if( gv->IsSelected() )
					{
						for( INT x = 0 ; x < gv->GetParentObject()->GetActualBrush()->Brush->Polys->Element.Num() ; ++x )
						{
							FPoly* Poly = &gv->GetParentObject()->GetActualBrush()->Brush->Polys->Element(x);
							Poly->RemoveVertex( *gv );
							bHandled = 1;
						}
					}
				}
			}
			break;
		}
	}

	mode->FinalizeSourceData();
	mode->GetFromSource();

	return bHandled;
}

/*------------------------------------------------------------------------------
	UGeomModifier_Create
------------------------------------------------------------------------------*/

/**
 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
 */
UBOOL UGeomModifier_Create::Supports(INT InSelType)
{
	switch( InSelType )
	{
		case GST_Vertex:
			return TRUE;
	}

	return FALSE;
}

/**
 * Implements the modifier application.
 */
UBOOL UGeomModifier_Create::OnApply()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)mode->GetSettings())->SelectionType;

	switch( seltype )
	{
		case GST_Vertex:
		{
			for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
			{
				FGeomObject* go = *Itor;

				go->CompileSelectionOrder();

				// Create an ordered list of vertices based on the selection order.

				TArray<FGeomVertex*> Verts;
				for( INT x = 0 ; x < go->SelectionOrder.Num() ; ++x )
				{
					FGeomBase* obj = go->SelectionOrder(x);
					if( obj->IsVertex() )
					{
						Verts.AddItem( (FGeomVertex*)obj );
					}
				}

				if( Verts.Num() > 2 )
				{
					// Create new geometry based on the selected vertices

					FPoly* NewPoly = new( go->GetActualBrush()->Brush->Polys->Element )FPoly();

					NewPoly->Init();

					for( INT x = 0 ; x < Verts.Num() ; ++x )
					{
						FGeomVertex* gv = Verts(x);

						new(NewPoly->Vertices) FVector(*gv);
					}

					NewPoly->Normal = FVector(0,0,0);
					NewPoly->Base = *Verts(0);
					NewPoly->PolyFlags = PF_DefaultFlags;
				}
			}
		}
		break;
	}

	mode->FinalizeSourceData();
	mode->GetFromSource();

	return TRUE;
}

/*------------------------------------------------------------------------------
	UGeomModifier_Flip
------------------------------------------------------------------------------*/

/**
 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
 */
UBOOL UGeomModifier_Flip::Supports(INT InSelType)
{
	switch( InSelType )
	{
		case GST_Poly:
			return TRUE;
	}

	return FALSE;
}

/**
 * Implements the modifier application.
 */
UBOOL UGeomModifier_Flip::OnApply()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)mode->GetSettings())->SelectionType;

	switch( seltype )
	{
		case GST_Poly:
		{
			for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
			{
				FGeomObject* go = *Itor;

				for( INT p = 0 ; p < go->PolyPool.Num() ; ++p )
				{
					FGeomPoly* gp = &go->PolyPool(p);

					if( gp->IsSelected() )
					{
						FPoly* Poly = &go->GetActualBrush()->Brush->Polys->Element( gp->ActualPolyIndex );
						Poly->Reverse();
					}
				}
			}
		}
		break;
	}

	mode->FinalizeSourceData();
	mode->GetFromSource();

	return TRUE;
}

/*------------------------------------------------------------------------------
	UGeomModifier_Split
------------------------------------------------------------------------------*/

/**
 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
 */
UBOOL UGeomModifier_Split::Supports(INT InSelType)
{
	switch( InSelType )
	{
		case GST_Vertex:
		case GST_Edge:
			return TRUE;
	}

	return FALSE;
}

/**
 * Implements the modifier application.
 */
UBOOL UGeomModifier_Split::OnApply()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)mode->GetSettings())->SelectionType;

	switch( seltype )
	{
		case GST_Vertex:
		{
			for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
			{
				FGeomObject* go = *Itor;

				FGeomVertex* SelectedVerts[2];

				// Make sure only 2 vertices are selected

				INT Count = 0;
				for( INT v = 0 ; v < go->VertexPool.Num() ; ++v )
				{
					FGeomVertex* gv = &go->VertexPool(v);

					if( gv->IsSelected() )
					{
						SelectedVerts[Count] = gv;
						Count++;
						if( Count == 2 )
						{
							break;
						}
					}
				}

				if( Count != 2 )
				{
					GeomError( *LocalizeUnrealEd(TEXT("Error_RequiresTwoSelectedVertices")) );
					break;
				}

				// Find the polygon we are splitting.

				FGeomPoly* PolyToSplit = NULL;
				for( INT i = 0 ; i < SelectedVerts[0]->ParentPolyIndices.Num() ; ++i )
				{
					if( SelectedVerts[1]->ParentPolyIndices.FindItemIndex( SelectedVerts[0]->ParentPolyIndices(i) ) != INDEX_NONE )
					{
						PolyToSplit = &go->PolyPool( SelectedVerts[0]->ParentPolyIndices(i) );
						break;
					}
				}

				if( !PolyToSplit )
				{
					GeomError( *LocalizeUnrealEd(TEXT("Error_SelectedVerticesMustBelongToSamePoly")) );
					break;
				}

				// Create a plane that cross the selected vertices and split the polygon with it.

				const FPoly SavePoly = *PolyToSplit->GetActualPoly();
				const FVector v0 = *SelectedVerts[0];
				const FVector v1 = *SelectedVerts[1];
				const FVector PlaneNormal( (v1 - v0).SafeNormal() );
				const FVector PlaneBase = 0.5f*(v1 + v0);

				FPoly* Front = new( PolyToSplit->GetParentObject()->GetActualBrush()->Brush->Polys->Element )FPoly();
				Front->Init();
				FPoly* Back = new( PolyToSplit->GetParentObject()->GetActualBrush()->Brush->Polys->Element )FPoly();
				Back->Init();

				INT result = PolyToSplit->GetActualPoly()->SplitWithPlane( PlaneBase, PlaneNormal, Front, Back, 1 );
				check( result == SP_Split );	// Any other result means that the splitting plane was created incorrectly

				Front->Base = SavePoly.Base;
				Front->TextureU = SavePoly.TextureU;
				Front->TextureV = SavePoly.TextureV;
				Front->Material = SavePoly.Material;

				Back->Base = SavePoly.Base;
				Back->TextureU = SavePoly.TextureU;
				Back->TextureV = SavePoly.TextureV;
				Back->Material = SavePoly.Material;

				// Remove the old polygon from the brush

				go->GetActualBrush()->Brush->Polys->Element.Remove( PolyToSplit->ActualPolyIndex );
			}
		}
		break;

		case GST_Edge:
		{
			for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
			{
				FGeomObject* go = *Itor;

				TArray<FGeomEdge> Edges;
				go->CompileUniqueEdgeArray( &Edges );

				// Check to see that the selected edges belong to the same polygon

				FGeomPoly* PolyToSplit = NULL;

				for( INT p = 0 ; p < go->PolyPool.Num() ; ++p )
				{
					FGeomPoly* gp = &go->PolyPool(p);

					UBOOL bOK = TRUE;
					for( INT s = 0 ; s < Edges.Num() ; ++s )
					{
						if( Edges(s).ParentPolyIndices.FindItemIndex( p ) == INDEX_NONE )
						{
							bOK = FALSE;
							break;
						}
					}

					if( bOK )
					{
						PolyToSplit = gp;
						break;
					}
				}

				if( !PolyToSplit )
				{
					GeomError( *LocalizeUnrealEd(TEXT("Error_SelectedEdgesMustBelongToSamePoly")) );
					break;
				}

				FPoly SavePoly = *PolyToSplit->GetActualPoly();

				// Add extra vertices to the surrounding polygons.  Any polygon that has
				// any of the selected edges as a part of it needs to have an extra vertex added.

				for( INT p = 0 ; p < go->PolyPool.Num() ; ++p )
				{
					FGeomPoly* gp = &go->PolyPool(p);

					for( INT s = 0 ; s < Edges.Num() ; ++s )
					{
						if( Edges(s).ParentPolyIndices.FindItemIndex( p ) != INDEX_NONE )
						{
							FPoly* Poly = gp->GetActualPoly();

							// Give the polygon a new set of vertices which includes the new vertex from the middle of the split edge.

							for( INT i = 0 ; i < gp->EdgeIndices.Num() ; ++i )
							{
								FGeomEdge* ge = &go->EdgePool( gp->EdgeIndices(i) );

								new(Poly->Vertices) FVector(go->VertexPool( ge->VertexIndices[0] ));

								if( ge->IsSameEdge( Edges(s) ) )
								{
									new(Poly->Vertices) FVector(ge->GetMid());
								}
							}

							Poly->Fix();
						}
					}
				}

				// If we are splitting 2 edges, we are splitting the polygon itself.

				if( Edges.Num() == 2 )
				{
					// Split the main poly.  This is the polygon that we determined earlier shares both
					// of the selected edges.  It needs to be removed and 2 new polygons created in its place.

					FVector Mid0 = Edges(0).GetMid(),
						Mid1 = Edges(1).GetMid();

					FVector PlaneBase = Mid0;
					FVector v0 = Mid0,
						v1 = Mid1,
						v2 = v1 + (PolyToSplit->GetNormal() * 16);
					FVector PlaneNormal = FPlane( v0, v1, v2 );

					FPoly* Front = new( PolyToSplit->GetParentObject()->GetActualBrush()->Brush->Polys->Element )FPoly();
					Front->Init();
					FPoly* Back = new( PolyToSplit->GetParentObject()->GetActualBrush()->Brush->Polys->Element )FPoly();
					Back->Init();

					INT result = PolyToSplit->GetActualPoly()->SplitWithPlane( PlaneBase, PlaneNormal, Front, Back, 1 );
					check( result == SP_Split );	// Any other result means that the splitting plane was created incorrectly

					Front->Base = SavePoly.Base;
					Front->TextureU = SavePoly.TextureU;
					Front->TextureV = SavePoly.TextureV;
					Front->Material = SavePoly.Material;

					Back->Base = SavePoly.Base;
					Back->TextureU = SavePoly.TextureU;
					Back->TextureV = SavePoly.TextureV;
					Back->Material = SavePoly.Material;

					// Remove the old polygon from the brush

					go->GetActualBrush()->Brush->Polys->Element.Remove( PolyToSplit->ActualPolyIndex );
				}
			}
		}
		break;
	}

	mode->FinalizeSourceData();
	mode->GetFromSource();

	return TRUE;
}

/*------------------------------------------------------------------------------
	UGeomModifier_Triangulate
------------------------------------------------------------------------------*/

/**
 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
 */
UBOOL UGeomModifier_Triangulate::Supports(INT InSelType)
{
	switch( InSelType )
	{
		case GST_Poly:
			return TRUE;
	}

	return FALSE;
}

/**
 * Implements the modifier application.
 */
UBOOL UGeomModifier_Triangulate::OnApply()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)mode->GetSettings())->SelectionType;

	switch( seltype )
	{
		case GST_Poly:
		{
			// Mark the selected polygons so we can find them in the next loop, and create
			// a local list of FPolys to triangulate later.

			for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
			{
				FGeomObject* go = *Itor;

				TArray<FPoly> PolyList;

				for( INT p = 0 ; p < go->PolyPool.Num() ; ++p )
				{
					FGeomPoly* gp = &go->PolyPool(p);

					if( gp->IsSelected() )
					{
						gp->GetParentObject()->GetActualBrush()->Brush->Polys->Element( gp->ActualPolyIndex ).PolyFlags |= PF_GeomMarked;
						PolyList.AddItem( gp->GetParentObject()->GetActualBrush()->Brush->Polys->Element( gp->ActualPolyIndex ) );
					}
				}

				// Delete existing polygons

				for( INT p = 0 ; p < go->GetActualBrush()->Brush->Polys->Element.Num() ; ++p )
				{
					if( (go->GetActualBrush()->Brush->Polys->Element( p ).PolyFlags&PF_GeomMarked) > 0 )
					{
						go->GetActualBrush()->Brush->Polys->Element.Remove( p );
						p = -1;
					}
				}

				// Triangulate the old polygons into the brush

				for( INT p = 0 ; p < PolyList.Num() ; ++p )
				{
					PolyList(p).Triangulate( go->GetActualBrush() );
				}
			}
		}
		break;
	}

	mode->FinalizeSourceData();
	mode->GetFromSource();

	return TRUE;
}

/*------------------------------------------------------------------------------
	UGeomModifier_Turn
------------------------------------------------------------------------------*/

/**
 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
 */
UBOOL UGeomModifier_Turn::Supports(INT InSelType)
{
	switch( InSelType )
	{
		case GST_Edge:
			return TRUE;
	}

	return FALSE;
}

/**
 * Implements the modifier application.
 */
UBOOL UGeomModifier_Turn::OnApply()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)mode->GetSettings())->SelectionType;

	switch( seltype )
	{
		case GST_Edge:
		{
			for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
			{
				FGeomObject* go = *Itor;

				TArray<FGeomEdge> Edges;
				go->CompileUniqueEdgeArray( &Edges );

				// Make sure that all polygons involved are triangles

				for( INT e = 0 ; e < Edges.Num() ; ++e )
				{
					FGeomEdge* ge = &Edges(e);

					for( INT p = 0 ; p < ge->ParentPolyIndices.Num() ; ++p )
					{
						FGeomPoly* gp = &go->PolyPool( ge->ParentPolyIndices(p) );
						FPoly* Poly = gp->GetActualPoly();

						if( Poly->Vertices.Num() != 3 )
						{
							GeomError( *LocalizeUnrealEd(TEXT("Error_PolygonsOnEdgeToTurnMustBeTriangles")) );
							EndTrans();
							return 0;
						}
					}
				}

				// Turn the edges, one by one

				for( INT e = 0 ; e < Edges.Num() ; ++e )
				{
					FGeomEdge* ge = &Edges(e);

					TArray<FVector> Quad;

					// Since we're doing each edge individually, they should each have exactly 2 polygon
					// parents (and each one is a triangle (verified above))

					if( ge->ParentPolyIndices.Num() == 2 )
					{
						FGeomPoly* gp = &go->PolyPool( ge->ParentPolyIndices(0) );
						FPoly* Poly = gp->GetActualPoly();
						FPoly SavePoly0 = *Poly;

						INT idx0 = Poly->GetVertexIndex( go->VertexPool( ge->VertexIndices[0] ) );
						INT idx1 = Poly->GetVertexIndex( go->VertexPool( ge->VertexIndices[1] ) );
						INT idx2 = INDEX_NONE;

						if( idx0 + idx1 == 1 )
						{
							idx2 = 2;
						}
						else if( idx0 + idx1 == 3 )
						{
							idx2 = 0;
						}
						else
						{
							idx2 = 1;
						}

						Quad.AddItem( Poly->Vertices(idx0) );
						Quad.AddItem( Poly->Vertices(idx2) );
						Quad.AddItem( Poly->Vertices(idx1) );

						gp = &go->PolyPool( ge->ParentPolyIndices(1) );
						Poly = gp->GetActualPoly();
						FPoly SavePoly1 = *Poly;

						for( INT v = 0 ; v < Poly->Vertices.Num() ; ++v )
						{
							Quad.AddUniqueItem( Poly->Vertices(v) );
						}

						// Create new polygons

						FPoly* NewPoly;

						NewPoly = new( gp->GetParentObject()->GetActualBrush()->Brush->Polys->Element )FPoly();

						NewPoly->Init();
						new(NewPoly->Vertices) FVector(Quad(2));
						new(NewPoly->Vertices) FVector(Quad(1));
						new(NewPoly->Vertices) FVector(Quad(3));

						NewPoly->Base = SavePoly0.Base;
						NewPoly->Material = SavePoly0.Material;
						NewPoly->PolyFlags = SavePoly0.PolyFlags;
						NewPoly->TextureU = SavePoly0.TextureU;
						NewPoly->TextureV = SavePoly0.TextureV;
						NewPoly->Normal = FVector(0,0,0);
						NewPoly->Finalize(go->GetActualBrush(),1);

						NewPoly = new( gp->GetParentObject()->GetActualBrush()->Brush->Polys->Element )FPoly();

						NewPoly->Init();
						new(NewPoly->Vertices) FVector(Quad(3));
						new(NewPoly->Vertices) FVector(Quad(1));
						new(NewPoly->Vertices) FVector(Quad(0));

						NewPoly->Base = SavePoly1.Base;
						NewPoly->Material = SavePoly1.Material;
						NewPoly->PolyFlags = SavePoly1.PolyFlags;
						NewPoly->TextureU = SavePoly1.TextureU;
						NewPoly->TextureV = SavePoly1.TextureV;
						NewPoly->Normal = FVector(0,0,0);
						NewPoly->Finalize(go->GetActualBrush(),1);

						// Tag the old polygons

						for( INT p = 0 ; p < ge->ParentPolyIndices.Num() ; ++p )
						{
							FGeomPoly* gp = &go->PolyPool( ge->ParentPolyIndices(p) );

							go->GetActualBrush()->Brush->Polys->Element( gp->ActualPolyIndex ).PolyFlags |= PF_GeomMarked;
						}
					}
				}

				// Delete the old polygons

				for( INT p = 0 ; p < go->GetActualBrush()->Brush->Polys->Element.Num() ; ++p )
				{
					if( (go->GetActualBrush()->Brush->Polys->Element( p ).PolyFlags&PF_GeomMarked) > 0 )
					{
						go->GetActualBrush()->Brush->Polys->Element.Remove( p );
						p = -1;
					}
				}
			}
			
		}
		break;
	}

	mode->FinalizeSourceData();
	mode->GetFromSource();

	return TRUE;
}

/*------------------------------------------------------------------------------
	UGeomModifier_Weld
------------------------------------------------------------------------------*/

/**
 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
 */
UBOOL UGeomModifier_Weld::Supports(INT InSelType)
{
	switch( InSelType )
	{
		case GST_Vertex:
			return TRUE;
	}

	return FALSE;
}

/**
 * Implements the modifier application.
 */
UBOOL UGeomModifier_Weld::OnApply()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)mode->GetSettings())->SelectionType;

	switch( seltype )
	{
		case GST_Vertex:
		{
			for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
			{
				FGeomObject* go = *Itor;

				go->CompileSelectionOrder();

				if( go->SelectionOrder.Num() > 1 )
				{
					FGeomVertex* FirstSel = (FGeomVertex*)go->SelectionOrder(0);

					// Move all selected vertices to the location of the first vertex that was selected.

					for( INT v = 1 ; v < go->SelectionOrder.Num() ; ++v )
					{
						FGeomVertex* gv = (FGeomVertex*)go->SelectionOrder(v);

						if( gv->IsSelected() )
						{
							gv->X = FirstSel->X;
							gv->Y = FirstSel->Y;
							gv->Z = FirstSel->Z;
						}
					}

					go->SendToSource();
				}
			}
			
		}
		break;
	}

	mode->FinalizeSourceData();
	mode->GetFromSource();

	return TRUE;
}
