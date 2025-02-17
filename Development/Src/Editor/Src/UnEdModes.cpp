/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EditorPrivate.h"
#include "UnTerrain.h"
#include "MouseDeltaTracker.h"
#include "ScopedTransaction.h"
#include "SurfaceIterators.h"

/*------------------------------------------------------------------------------
    Base class.
------------------------------------------------------------------------------*/

FEdMode::FEdMode():
	Desc( TEXT("N/A") ),
	BitmapOn( NULL ),
	BitmapOff( NULL ),
	ModeBar( NULL ),
	CurrentTool( NULL ),
	Settings( NULL ),
	ID( EM_None )
{
	Component = ConstructObject<UEdModeComponent>(UEdModeComponent::StaticClass());
}

FEdMode::~FEdMode()
{
	delete Settings;
}

UBOOL FEdMode::MouseMove(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,INT x, INT y)
{
	if( GetCurrentTool() )
	{
		return GetCurrentTool()->MouseMove( ViewportClient, Viewport, x, y );
	}

	return 0;
}

UBOOL FEdMode::InputKey(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event)
{
	if( GetCurrentTool() )
	{
		return GetCurrentTool()->InputKey( ViewportClient, Viewport, Key, Event );
	}

	return 0;
}

UBOOL FEdMode::InputAxis(FEditorLevelViewportClient* InViewportClient,FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime)
{
	FModeTool* Tool = GetCurrentTool();
	if (Tool)
	{
		return Tool->InputAxis(InViewportClient, Viewport, ControllerId, Key, Delta, DeltaTime);
	}

	return FALSE;
}

UBOOL FEdMode::InputDelta(FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale)
{
	if( GetCurrentTool() )
	{
		return GetCurrentTool()->InputDelta(InViewportClient,InViewport,InDrag,InRot,InScale);
	}

	return 0;
}

/**
 * Lets each tool determine if it wants to use the editor widget or not.  If the tool doesn't want to use it, it will be
 * fed raw mouse delta information (not snapped or altered in any way).
 */

UBOOL FEdMode::UsesWidget() const
{
	if( GetCurrentTool() )
	{
		return GetCurrentTool()->UseWidget();
	}

	return 1;
}

/**
 * Allows each mode/tool to determine a good location for the widget to be drawn at.
 */

FVector FEdMode::GetWidgetLocation() const
{
	//debugf(TEXT("In FEdMode::GetWidgetLocation"));
	return GEditorModeTools().PivotLocation;
}

/**
 * Lets the mode determine if it wants to draw the widget or not.
 */

UBOOL FEdMode::ShouldDrawWidget() const
{
	return (GEditor->GetSelectedActors()->GetTop<AActor>() != NULL);
}

/**
 * Allows each mode to customize the axis pieces of the widget they want drawn.
 *
 * @param	InwidgetMode	The current widget mode
 *
 * @return	A bitfield comprised of AXIS_ values
 */

INT FEdMode::GetWidgetAxisToDraw( FWidget::EWidgetMode InWidgetMode ) const
{
	return AXIS_XYZ;
}

/**
 * Lets each mode/tool handle box selection in its own way.
 *
 * @param	InBox	The selection box to use, in worldspace coordinates.
 * @return		TRUE if something was selected/deselected, FALSE otherwise.
 */
UBOOL FEdMode::BoxSelect( FBox& InBox, UBOOL InSelect )
{
	UBOOL bResult = FALSE;
	if( GetCurrentTool() )
	{
		bResult = GetCurrentTool()->BoxSelect( InBox, InSelect );
	}
	return bResult;
}

void FEdMode::SelectNone()
{
	if( GetCurrentTool() )
	{
		GetCurrentTool()->SelectNone();
	}
}

void FEdMode::Tick(FEditorLevelViewportClient* ViewportClient,FLOAT DeltaTime)
{
	if( GetCurrentTool() )
	{
		GetCurrentTool()->Tick(ViewportClient,DeltaTime);
	}
}

void FEdMode::ClearComponent()
{
	check(Component);
	Component->ConditionalDetach();
}

void FEdMode::UpdateComponent()
{
	check(Component);
	Component->ConditionalAttach(GWorld->Scene,NULL,FMatrix::Identity);
}

void FEdMode::Enter()
{
	UpdateComponent();

	// Update components for selected actors, in case the mode we just exited
	// was hijacking selection events selection and not updating components.
	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* SelectedActor = static_cast<AActor*>( *It );
		checkSlow( SelectedActor->IsA(AActor::StaticClass()) );
		SelectedActor->ForceUpdateComponents( FALSE, FALSE );
	}

	// Update the mode bar if needed
	GCallbackEvent->Send(CALLBACK_EditorModeEnter,this);
}

void FEdMode::Exit()
{
	ClearComponent();
	// Save any mode bar data if needed
	GCallbackEvent->Send(CALLBACK_EditorModeExit,this);
}

void FEdMode::SetCurrentTool( EModeTools InID )
{
	CurrentTool = FindTool( InID );
	check( CurrentTool );	// Tool not found!  This can't happen.

	CurrentToolChanged();
}

void FEdMode::SetCurrentTool( FModeTool* InModeTool )
{
	CurrentTool = InModeTool;
	check(CurrentTool);

	CurrentToolChanged();
}

FModeTool* FEdMode::FindTool( EModeTools InID )
{
	for( INT x = 0 ; x < Tools.Num() ; ++x )
	{
		if( Tools(x)->GetID() == InID )
		{
			return Tools(x);
		}
	}

	appErrorf(LocalizeSecure(LocalizeUnrealEd("Error_FailedToFindTool"),(INT)InID));
	return NULL;
}

const FToolSettings* FEdMode::GetSettings() const
{
	return Settings;
}

void FEdMode::Render(const FSceneView* View,FViewport* Viewport,FPrimitiveDrawInterface* PDI)
{
	if( GEditor->bShowBrushMarkerPolys )
	{
		// Draw translucent polygons on brushes and volumes

		for( FActorIterator It; It; ++ It )
		{
			ABrush* Brush = Cast<ABrush>( *It );

			// Brush->Brush is checked to safe from brushes that were created without having their brush members attached.
			if( Brush && Brush->Brush && (Brush->IsABuilderBrush() || Brush->IsVolumeBrush()) && GEditor->GetSelectedActors()->IsSelected(Brush) )
			{
				// Build a mesh by basically drawing the triangles of each 
				FDynamicMeshBuilder MeshBuilder;
				INT VertexOffset = 0;

				for( INT PolyIdx = 0 ; PolyIdx < Brush->Brush->Polys->Element.Num() ; ++PolyIdx )
				{
					const FPoly* Poly = &Brush->Brush->Polys->Element(PolyIdx);

					if( Poly->Vertices.Num() > 2 )
					{
						const FVector Vertex0 = Poly->Vertices(0);
						FVector Vertex1 = Poly->Vertices(1);

						MeshBuilder.AddVertex(Vertex0, FVector2D(0,0), FVector(1,0,0), FVector(0,1,0), FVector(0,0,1), FColor(255,255,255));
						MeshBuilder.AddVertex(Vertex1, FVector2D(0,0), FVector(1,0,0), FVector(0,1,0), FVector(0,0,1), FColor(255,255,255));

						for( INT VertexIdx = 2 ; VertexIdx < Poly->Vertices.Num() ; ++VertexIdx )
						{
							const FVector Vertex2 = Poly->Vertices(VertexIdx);
							MeshBuilder.AddVertex(Vertex2, FVector2D(0,0), FVector(1,0,0), FVector(0,1,0), FVector(0,0,1), FColor(255,255,255));
							MeshBuilder.AddTriangle(VertexOffset,VertexOffset + VertexIdx,VertexOffset+VertexIdx-1);
							Vertex1 = Vertex2;
						}

						// Increment the vertex offset so the next polygon uses the correct vertex indices.
						VertexOffset += Poly->Vertices.Num();
					}
				}

				// Flush the mesh triangles.
				MeshBuilder.Draw(PDI, Brush->LocalToWorld(), new(GMainThreadMemStack) FColoredMaterialRenderProxy(GEngine->EditorBrushMaterial->GetRenderProxy(FALSE),Brush->GetWireColor()), SDPG_World);
			}
		}
	}

	if( GEditorModeTools().GetCurrentModeID() == EM_Geometry )
	{
		// Clip markers

		TArray<AActor*>	ClipMarkers;

		// Gather a list of all the ClipMarkers in the level.
		//
		for( FActorIterator It; It; ++It )
		{
			AActor* Actor = *It;
			if(Actor && Actor->IsA(AClipMarker::StaticClass()) )
			{
				ClipMarkers.AddItem( Actor );
			}
		}

		if( ClipMarkers.Num() > 1 )
		{
			FVector2D Start, End;
			FPlane WkPlane;

			// Draw a connecting line between them all.
			INT x;
			for( x = 1 ; x < ClipMarkers.Num() ; x++ )
			{
				PDI->DrawLine( ClipMarkers(x - 1)->Location, ClipMarkers(x)->Location, GEngine->C_BrushWire, SDPG_World );
			}
			PDI->DrawLine( ClipMarkers(0)->Location, ClipMarkers(x - 1)->Location, GEngine->C_BrushWire, SDPG_World );

			// Draw an arrow that shows the direction of the clipping plane.  This arrow should
			// appear halfway between the first and second markers.
			//
			FVector vtx1, vtx2, vtx3;
			FPoly NormalPoly;
			UBOOL bDrawOK = 1;

			vtx1 = ClipMarkers(0)->Location;
			vtx2 = ClipMarkers(1)->Location;

			if( ClipMarkers.Num() == 3 )
			{
				// If we have 3 points, just grab the third one to complete the plane.
				//
				vtx3 = ClipMarkers(2)->Location;
			}
			else
			{
				// If we only have 2 points, we will assume the third based on the viewport.
				// (With only 2 points, we can only render into the ortho viewports)
				//
				vtx3 = vtx1;

				const FEditorLevelViewportClient* ViewportClient = static_cast<FEditorLevelViewportClient*>( Viewport->GetClient() );
				switch( ViewportClient->ViewportType )
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
					default:
						bDrawOK = FALSE;
						break;
				}
			}

			NormalPoly.Vertices.AddItem( vtx1 );
			NormalPoly.Vertices.AddItem( vtx2 );
			NormalPoly.Vertices.AddItem( vtx3 );

			if( bDrawOK && !NormalPoly.CalcNormal(1) )
			{
				FVector Start = (vtx1 + vtx2 + vtx3 ) / ClipMarkers.Num();
				if( ClipMarkers.Num() == 2 )
				{
					Start = ( vtx1 + vtx2 ) / ClipMarkers.Num();
				}

				PDI->DrawLine( Start, Start + NormalPoly.Normal * 48, GEngine->C_BrushWire, SDPG_World );

				// Draw a triangle.
				{
					FDynamicMeshBuilder MeshBuilder;

					MeshBuilder.AddVertex(vtx1, FVector2D(0,0), FVector(1,0,0), FVector(0,1,0), FVector(0,0,1), FColor(255,255,255));
					MeshBuilder.AddVertex(vtx2, FVector2D(0,0), FVector(1,0,0), FVector(0,1,0), FVector(0,0,1), FColor(255,255,255));
					MeshBuilder.AddVertex(vtx3, FVector2D(0,0), FVector(1,0,0), FVector(0,1,0), FVector(0,0,1), FColor(255,255,255));
					MeshBuilder.AddTriangle(0,2,1);

					MeshBuilder.Draw(PDI, FMatrix::Identity, new(GMainThreadMemStack) FColoredMaterialRenderProxy(GEngine->EditorBrushMaterial->GetRenderProxy(FALSE),GEngine->C_BrushWire), SDPG_World);
				}
			}
		}
	}
}

void FEdMode::DrawHUD(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,const FSceneView* View,FCanvas* Canvas)
{
	// Render the drag tool.
	ViewportClient->MouseDeltaTracker->RenderDragTool( View, Canvas );

	// If this viewport doesn't show mode widgets or the mode itself doesn't want them, leave.
	if( !(ViewportClient->ShowFlags&SHOW_ModeWidgets) || !ShowModeWidgets() )
	{
		return;
	}

	// Clear Hit proxies
	const UBOOL bIsHitTesting = Canvas->IsHitTesting();
	if ( !bIsHitTesting )
	{
		Canvas->SetHitProxy(NULL);
	}

	// Draw vertices for selected BSP brushes and static meshes if the large vertices show flag is set.
	if ( !ViewportClient->bDrawVertices )
	{
		return;
	}

	const UBOOL bLargeVertices		= View->Family->ShowFlags & SHOW_LargeVertices ? TRUE : FALSE;
	const UBOOL bShowBrushes		= (View->Family->ShowFlags & SHOW_Brushes) ? TRUE : FALSE;
	const UBOOL bShowBSP			= (View->Family->ShowFlags & SHOW_BSP) ? TRUE : FALSE;
	const UBOOL bShowBuilderBrush	= (View->Family->ShowFlags & SHOW_BuilderBrush) ? TRUE : FALSE;

	UTexture2D* VertexTexture		= GetVertexTexture();
	const FLOAT TextureSizeX		= VertexTexture->SizeX * ( bLargeVertices ? 1.0f : 0.5f );
	const FLOAT TextureSizeY		= VertexTexture->SizeY * ( bLargeVertices ? 1.0f : 0.5f );

	// Temporaries.
	TArray<FVector> Vertices;

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* SelectedActor = static_cast<AActor*>( *It );
		checkSlow( SelectedActor->IsA(AActor::StaticClass()) );

		if ( SelectedActor->IsABrush() && (bShowBrushes || bShowBSP) )
		{
			ABrush* Brush = static_cast<ABrush*>( SelectedActor );
			if ( Brush->Brush )
			{
				// Don't render builder brush vertices if the builder brush show flag is disabled.
				if( !bShowBuilderBrush && Brush->IsABuilderBrush() )
				{
					continue;
				}

				for( INT p = 0 ; p < Brush->Brush->Polys->Element.Num() ; ++p )
				{
					FPoly* poly = &Brush->Brush->Polys->Element(p);
					for( INT VertexIndex = 0 ; VertexIndex < poly->Vertices.Num() ; ++VertexIndex )
					{
						const FVector& PolyVertex	= poly->Vertices(VertexIndex);
						const FVector vtx			= Brush->LocalToWorld().TransformFVector( PolyVertex );
						FVector2D PixelLocation;
						if(View->ScreenToPixel(View->WorldToScreen(vtx),PixelLocation))
						{
							const UBOOL bOutside =
								PixelLocation.X < 0.0f || PixelLocation.X > View->SizeX ||
								PixelLocation.Y < 0.0f || PixelLocation.Y > View->SizeY;
							if ( !bOutside )
							{
								const FLOAT X = PixelLocation.X - (TextureSizeX/2);
								const FLOAT Y = PixelLocation.Y - (TextureSizeY/2);
								const FColor Color( Brush->GetWireColor() );
								if ( bIsHitTesting ) Canvas->SetHitProxy( new HBSPBrushVert(Brush,&poly->Vertices(VertexIndex)) );
								DrawTile( Canvas, X, Y, TextureSizeX, TextureSizeY, 0.f, 0.f, 1.f, 1.f, Color, VertexTexture->Resource );
								if ( bIsHitTesting ) Canvas->SetHitProxy( NULL );
							}
						}
					}
				}
			}
		}
#if !SHIPPING_PC_GAME
		// Not supported on shipped builds because PC cooking strips raw mesh data.
		else if( bLargeVertices )
		{
			// Static mesh vertices
			AStaticMeshActor* Actor = Cast<AStaticMeshActor>( SelectedActor );
			if( Actor && Actor->StaticMeshComponent && Actor->StaticMeshComponent->StaticMesh )
			{
				Vertices.Empty();
				const FStaticMeshTriangle* RawTriangleData = (FStaticMeshTriangle*) Actor->StaticMeshComponent->StaticMesh->LODModels(0).RawTriangles.Lock(LOCK_READ_ONLY);
				for( INT tri = 0 ; tri < Actor->StaticMeshComponent->StaticMesh->LODModels(0).RawTriangles.GetElementCount() ; tri++ )
				{
					const FStaticMeshTriangle* smt = &RawTriangleData[tri];
					Vertices.AddUniqueItem( Actor->LocalToWorld().TransformFVector( smt->Vertices[0] ) );
					Vertices.AddUniqueItem( Actor->LocalToWorld().TransformFVector( smt->Vertices[1] ) );
					Vertices.AddUniqueItem( Actor->LocalToWorld().TransformFVector( smt->Vertices[2] ) );
				}
				Actor->StaticMeshComponent->StaticMesh->LODModels(0).RawTriangles.Unlock();

				for( INT VertexIndex = 0 ; VertexIndex < Vertices.Num() ; ++VertexIndex )
				{				
					const FVector& Vertex = Vertices(VertexIndex);
					FVector2D PixelLocation;
					if(View->ScreenToPixel(View->WorldToScreen(Vertex),PixelLocation))
					{
						const UBOOL bOutside =
							PixelLocation.X < 0.0f || PixelLocation.X > View->SizeX ||
							PixelLocation.Y < 0.0f || PixelLocation.Y > View->SizeY;
						if ( !bOutside )
						{
							const FLOAT X = PixelLocation.X - (TextureSizeX/2);
							const FLOAT Y = PixelLocation.Y - (TextureSizeY/2);
							if ( bIsHitTesting ) Canvas->SetHitProxy( new HStaticMeshVert(Actor,Vertex) );
							DrawTile( Canvas, X, Y, TextureSizeX, TextureSizeY, 0.f, 0.f, 1.f, 1.f, FLinearColor::White, VertexTexture->Resource );
							if ( bIsHitTesting ) Canvas->SetHitProxy( NULL );
						}
					}
				}
			}
		}
#endif // SHIPPING_PC_GAME
	}
}

UBOOL FEdMode::StartTracking()
{
	UBOOL bResult = FALSE;
	if( GetCurrentTool() )
	{
		bResult = GetCurrentTool()->StartModify();
	}
	return bResult;
}

UBOOL FEdMode::EndTracking()
{
	UBOOL bResult = FALSE;
	if( GetCurrentTool() )
	{
		bResult = GetCurrentTool()->EndModify();
	}
	return bResult;
}

FVector FEdMode::GetWidgetNormalFromCurrentAxis( void* InData )
{
	// Figure out the proper coordinate system.

	FMatrix matrix = FMatrix::Identity;
	if( GEditorModeTools().CoordSystem == COORD_Local )
	{
		GetCustomDrawingCoordinateSystem( matrix, InData );
	}

	// Get a base normal from the current axis.

	FVector BaseNormal(1,0,0);		// Default to X axis
	switch( CurrentWidgetAxis )
	{
		case AXIS_Y:	BaseNormal = FVector(0,1,0);	break;
		case AXIS_Z:	BaseNormal = FVector(0,0,1);	break;
		case AXIS_XY:	BaseNormal = FVector(1,1,0);	break;
		case AXIS_XZ:	BaseNormal = FVector(1,0,1);	break;
		case AXIS_YZ:	BaseNormal = FVector(0,1,1);	break;
		case AXIS_XYZ:	BaseNormal = FVector(1,1,1);	break;
	}

	// Transform the base normal into the proper coordinate space.
	return matrix.TransformFVector( BaseNormal );
}

UBOOL FEdMode::ExecDelete()
{
	return 0;
}

/*------------------------------------------------------------------------------
    Default.
------------------------------------------------------------------------------*/

FEdModeDefault::FEdModeDefault()
{
	ID = EM_Default;
	Desc = TEXT("Default");
}

/*------------------------------------------------------------------------------
    Geometry Editing.
------------------------------------------------------------------------------*/

FEdModeGeometry::FEdModeGeometry()
{
	ID = EM_Geometry;
	Desc = TEXT("Geometry Editing");
	ModifierWindow = NULL;

	Tools.AddItem( new FModeTool_GeometryModify() );
	SetCurrentTool( MT_GeometryModify );

	Settings = new FGeometryToolSettings;
}

FEdModeGeometry::~FEdModeGeometry()
{
	for( INT i=0; i<GeomObjects.Num(); i++ )
	{
		FGeomObject* GeomObject	= GeomObjects(i);
		delete GeomObject;
	}
	GeomObjects.Empty();
}

void FEdModeGeometry::Render(const FSceneView* View,FViewport* Viewport,FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View,Viewport,PDI);

	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)GetSettings())->SelectionType;

	switch( seltype )
	{
		case GST_Object:
			RenderObject( View, PDI );
			break;
		case GST_Poly:
			RenderPoly( View, PDI );
			break;
		case GST_Edge:
			RenderEdge( View, PDI );
			break;
		case GST_Vertex:
			RenderVertex( View, PDI );
			break;
	}
}

UBOOL FEdModeGeometry::ShowModeWidgets() const
{
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)GetSettings())->SelectionType;

	switch( seltype )
	{
		case GST_Object:
		case GST_Poly:
		case GST_Edge:
			return 1;
	}

	return 0;
}

UBOOL FEdModeGeometry::ShouldDrawBrushWireframe( AActor* InActor ) const
{
	checkSlow( InActor );

	// If the actor isn't selected, we don't want to interfere with it's rendering.
	if( !GEditor->GetSelectedActors()->IsSelected( InActor ) )
	{
		return TRUE;
	}

	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)GetSettings())->SelectionType;

	switch( seltype )
	{
		case GST_Object:
		case GST_Poly:
		case GST_Edge:
		case GST_Vertex:
			return FALSE;
	}

	check(0);	// Shouldn't be here
	return FALSE;
}

UBOOL FEdModeGeometry::GetCustomDrawingCoordinateSystem( FMatrix& InMatrix, void* InData )
{
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)GetSettings())->SelectionType;

	if( seltype == GST_Object )
	{
		return 0;
	}

	if( InData )
	{
		InMatrix = FRotationMatrix( ((FGeomBase*)InData)->GetNormal().Rotation() );
	}
	else
	{
		// If we don't have a specific geometry object to get the normal from
		// use the one that was last selected.

		for( INT o = 0 ; o < GeomObjects.Num() ; ++o )
		{
			FGeomObject* go = GeomObjects(o);
			go->CompileSelectionOrder();

			if( go->SelectionOrder.Num() )
			{
				InMatrix = FRotationMatrix( go->SelectionOrder( go->SelectionOrder.Num()-1 )->GetNormal().Rotation() );
				return 1;
			}
		}
	}

	return 0;
}

UBOOL FEdModeGeometry::GetCustomInputCoordinateSystem( FMatrix& InMatrix, void* InData )
{
	return GetCustomDrawingCoordinateSystem( InMatrix, InData );
}

void FEdModeGeometry::Enter()
{
	FEdMode::Enter();

	GetFromSource();

	// Start up the tool window

	ModifierWindow = new WxGeomModifiers( (wxWindow*)GWarn->winEditorFrame, -1 );
	ModifierWindow->Show( ((FGeometryToolSettings*)GetSettings())->bShowModifierWindow == 1 );
	ModifierWindow->InitFromTool();
}

void FEdModeGeometry::Exit()
{
	FEdMode::Exit();

	for( INT i=0; i<GeomObjects.Num(); i++ )
	{
		FGeomObject* GeomObject	= GeomObjects(i);
		delete GeomObject;
	}
	GeomObjects.Empty();

	ModifierWindow->Destroy();
}

void FEdModeGeometry::ActorSelectionChangeNotify()
{
	GetFromSource();
}

void FEdModeGeometry::MapChangeNotify()
{
	// If the map changes in some major way, just refresh all the geometry data.
	GetFromSource();
}

void FEdModeGeometry::CurrentToolChanged()
{
	UpdateModifierWindow();
}

void FEdModeGeometry::UpdateModifierWindow()
{
	ModifierWindow->InitFromTool();
}

void FEdModeGeometry::Serialize( FArchive &Ar )
{
	FModeTool_GeometryModify* mtgm = (FModeTool_GeometryModify*)FindTool( MT_GeometryModify );
	for( FModeTool_GeometryModify::TModifierIterator Itor( mtgm->ModifierIterator() ) ; Itor ; ++Itor )
	{
		Ar << *Itor;
	}
}

FVector FEdModeGeometry::GetWidgetLocation() const
{
	//debugf( TEXT("FEdModeGeometry::GetWidgetLocation - do our thing here!") );
	const EGeometrySelectionType seltype = ((FGeometryToolSettings*)GetSettings())->SelectionType;
	if ( seltype != GST_Vertex )
	{
		//debugf( TEXT("GST_VERTEX") );
		return FEdMode::GetWidgetLocation();
	}
	//debugf( TEXT("GST_somethingElse") );
	return FEdMode::GetWidgetLocation();
}

// ------------------------------------------------------------------------------

/**
 * Deselects all edges, polygons, and vertices for all selected objects.
 */
void FEdModeGeometry::SelectNone()
{
	for( INT ObjectIdx = 0 ; ObjectIdx < GeomObjects.Num() ; ++ObjectIdx )
	{
		FGeomObject* GeomObject = GeomObjects(ObjectIdx);
		GeomObject->Select( 0 );

		for( int VertexIdx = 0 ; VertexIdx < GeomObject->EdgePool.Num() ; ++VertexIdx )
		{
			GeomObject->EdgePool(VertexIdx).Select( 0 );
		}
		for( int VertexIdx = 0 ; VertexIdx < GeomObject->PolyPool.Num() ; ++VertexIdx )
		{
			GeomObject->PolyPool(VertexIdx).Select( 0 );
		}
		for( int VertexIdx = 0 ; VertexIdx < GeomObject->VertexPool.Num() ; ++VertexIdx )
		{
			GeomObject->VertexPool(VertexIdx).Select( 0 );
		}

		GeomObject->SelectionOrder.Empty();
	}
}

// ------------------------------------------------------------------------------

void FEdModeGeometry::RenderSinglePoly(const FGeomPoly* InPoly, const FSceneView *InView, FPrimitiveDrawInterface* InPDI, FDynamicMeshBuilder* MeshBuilder)
{
	// Look at the edge list and create a list of vertices to render from.

	TArray<FVector> Verts;

	FVector LastPos;

	for( INT EdgeIdx = 0 ; EdgeIdx < InPoly->EdgeIndices.Num() ; ++EdgeIdx )
	{
		const FGeomEdge* GeomEdge = &InPoly->GetParentObject()->EdgePool( InPoly->EdgeIndices(EdgeIdx) );

		if( EdgeIdx == 0 )
		{
			Verts.AddItem( InPoly->GetParentObject()->VertexPool( GeomEdge->VertexIndices[0] ) );
			LastPos = InPoly->GetParentObject()->VertexPool( GeomEdge->VertexIndices[0] );
		}
		else if( InPoly->GetParentObject()->VertexPool( GeomEdge->VertexIndices[0] ) == LastPos )
		{
			Verts.AddItem( InPoly->GetParentObject()->VertexPool( GeomEdge->VertexIndices[1] ) );
			LastPos = InPoly->GetParentObject()->VertexPool( GeomEdge->VertexIndices[1] );
		}
		else
		{
			Verts.AddItem( InPoly->GetParentObject()->VertexPool( GeomEdge->VertexIndices[0] ) );
			LastPos = InPoly->GetParentObject()->VertexPool( GeomEdge->VertexIndices[0] );
		}
	}

	// Draw Polygon Triangles
	const INT VertexOffset = MeshBuilder->AddVertex(Verts(0), FVector2D(0,0), FVector(1,0,0), FVector(0,1,0), FVector(0,0,1), FColor(255,255,255));
	MeshBuilder->AddVertex(Verts(1), FVector2D(0,0), FVector(1,0,0), FVector(0,1,0), FVector(0,0,1), FColor(255,255,255));

	for( INT VertIdx = 2 ; VertIdx < Verts.Num() ; ++VertIdx )
	{
		MeshBuilder->AddVertex(Verts(VertIdx), FVector2D(0,0), FVector(1,0,0), FVector(0,1,0), FVector(0,0,1), FColor(255,255,255));
		MeshBuilder->AddTriangle( VertexOffset + VertIdx - 1, VertexOffset, VertexOffset + VertIdx);
	}

	if( ((FGeometryToolSettings*)GetSettings())->bShowNormals )
	{
		const FVector Mid = InPoly->GetParentObject()->GetActualBrush()->LocalToWorld().TransformFVector( InPoly->GetMid() );
		const FLOAT Scale = InView->Project( Mid ).W * ( 4 / (FLOAT)InView->SizeX / InView->ProjectionMatrix.M[0][0] );
		InPDI->DrawLine( Mid, Mid+(InPoly->GetNormal() * (32*Scale)), FColor(255,0,0), SDPG_Foreground );
	}
}

// ------------------------------------------------------------------------------

void FEdModeGeometry::RenderSingleEdge(const FGeomEdge* InEdge, FColor InColor, const FSceneView *InView, FPrimitiveDrawInterface* InPDI, UBOOL InShowEdgeMarkers )
{
	FVector V0 = InEdge->GetParentObject()->VertexPool( InEdge->VertexIndices[0] );
	FVector V1 = InEdge->GetParentObject()->VertexPool( InEdge->VertexIndices[1] );
	const FMatrix LocalToWorldMatrix = InEdge->GetParentObject()->GetActualBrush()->LocalToWorld();

	V0 = LocalToWorldMatrix.TransformFVector( V0 );
	V1 = LocalToWorldMatrix.TransformFVector( V1 );

	if( InShowEdgeMarkers )
	{
		const FVector Mid = InEdge->GetParentObject()->GetActualBrush()->LocalToWorld().TransformFVector( InEdge->GetMid() );
		const FLOAT Scale = InView->Project( Mid ).W * ( 4 / (FLOAT)InView->SizeX / InView->ProjectionMatrix.M[0][0] );
		InPDI->DrawSprite( Mid, 3.f * Scale, 3.f * Scale, GWorld->GetWorldInfo()->BSPVertex->Resource, InColor, SDPG_Foreground );

		if( ((FGeometryToolSettings*)GetSettings())->bShowNormals )
		{
			InPDI->DrawLine( Mid, Mid+(InEdge->GetNormal() * (32*Scale)), FColor(255,0,0), SDPG_Foreground );
		}
	}

	InPDI->DrawLine( V0, V1, InColor, SDPG_Foreground );
}

// ------------------------------------------------------------------------------

void FEdModeGeometry::RenderSinglePolyWireframe(const FGeomPoly* InPoly, FColor InColor, const FSceneView *InView, FPrimitiveDrawInterface* InPDI)
{
	for( INT EdgeIdx = 0 ; EdgeIdx < InPoly->EdgeIndices.Num() ; ++EdgeIdx )
	{
		const FGeomEdge* GeomEdge = &InPoly->GetParentObject()->EdgePool( InPoly->EdgeIndices(EdgeIdx) );
		RenderSingleEdge( GeomEdge, InColor, InView, InPDI, 0 );
	}
}


// ------------------------------------------------------------------------------

void FEdModeGeometry::RenderObject( const FSceneView* View, FPrimitiveDrawInterface* PDI )
{
	for( INT ObjectIdx = 0 ; ObjectIdx < GeomObjects.Num() ; ++ObjectIdx )
	{
		const FGeomObject* GeomObject = GeomObjects(ObjectIdx);

		RenderSingleObject(GeomObject, View, PDI);
	}
}

// ------------------------------------------------------------------------------

void FEdModeGeometry::RenderSingleObject(const FGeomObject* InObject, const FSceneView *InView, FPrimitiveDrawInterface* InPDI )
{
	
	// Anytime we draw unselected lines of an object, we need to make it use an HActor hit proxy with the 
	// brush as the parameter so that users can click on the lines of the brush and toggle selection.
	InPDI->SetHitProxy( new HActor( const_cast<ABrush*>( InObject->GetActualBrush() ) ) );
	{
		for( INT PolyIdx = 0 ; PolyIdx < InObject->PolyPool.Num() ; ++PolyIdx )
		{
			const FGeomPoly* GeomPoly = &InObject->PolyPool(PolyIdx);
			RenderSinglePolyWireframe( GeomPoly, FColor(255,255,255), InView, InPDI );
		}
	}
	InPDI->SetHitProxy(NULL);
}

// ------------------------------------------------------------------------------

void FEdModeGeometry::RenderPoly( const FSceneView* View, FPrimitiveDrawInterface* PDI )
{
	// Selected
	FMaterialRenderProxy* SelectedColorInstance = new(GMainThreadMemStack) FColoredMaterialRenderProxy(GEngine->GeomMaterial->GetRenderProxy(FALSE),FColor(255,128,64));

	for( INT ObjectIdx = 0 ; ObjectIdx < GeomObjects.Num() ; ++ObjectIdx )
	{
		const FGeomObject* GeomObject = GeomObjects(ObjectIdx);

		// Render selected wireframe polygons.
		for( INT PolyIdx = 0 ; PolyIdx < GeomObject->PolyPool.Num() ; ++PolyIdx )
		{
			const FGeomPoly* GeomPoly = &GeomObject->PolyPool(PolyIdx);
			if( GeomPoly->IsSelected() )
			{
				RenderSinglePolyWireframe( GeomPoly, FColor(255,255,255), View, PDI );
			}
		}

		// Render selected filled polygons.
		for( INT PolyIdx = 0 ; PolyIdx < GeomObject->PolyPool.Num() ; ++PolyIdx )
		{
			const FGeomPoly* GeomPoly = &GeomObject->PolyPool(PolyIdx);
			if( GeomPoly->IsSelected() )
			{
				PDI->SetHitProxy( new HGeomPolyProxy(const_cast<FGeomObject*>(GeomObject),PolyIdx) );
				{
					FDynamicMeshBuilder MeshBuilder;
					RenderSinglePoly( GeomPoly, View, PDI, &MeshBuilder );
					MeshBuilder.Draw(PDI, GeomObject->GetActualBrush()->LocalToWorld(), SelectedColorInstance, SDPG_Foreground );
				}
				PDI->SetHitProxy( NULL );
			}
		}
	}

	// Unselected

	for( INT ObjectIdx = 0 ; ObjectIdx < GeomObjects.Num() ; ++ObjectIdx )
	{
		const FGeomObject* GeomObject = GeomObjects(ObjectIdx);

		FMaterialRenderProxy* UnselectedColorInstance = new(GMainThreadMemStack) FColoredMaterialRenderProxy(GEngine->GeomMaterial->GetRenderProxy(FALSE),FLinearColor(GeomObject->GetActualBrush()->GetWireColor()) * 0.5f);

		// Render unselected wireframe polygons.
		for( INT PolyIdx = 0 ; PolyIdx < GeomObject->PolyPool.Num() ; ++PolyIdx )
		{
			const FGeomPoly* GeomPoly = &GeomObject->PolyPool(PolyIdx);
			if( !GeomPoly->IsSelected() )
			{
				RenderSinglePolyWireframe( GeomPoly, GeomObject->GetActualBrush()->GetWireColor(), View, PDI );
			}
		}

		// Render unselected filled polygons.
		for( INT PolyIdx = 0 ; PolyIdx < GeomObject->PolyPool.Num() ; ++PolyIdx )
		{
			const FGeomPoly* GeomPoly = &GeomObject->PolyPool(PolyIdx);
			if( !GeomPoly->IsSelected() )
			{
				PDI->SetHitProxy( new HGeomPolyProxy(const_cast<FGeomObject*>(GeomObject),PolyIdx) );
				{
					FDynamicMeshBuilder MeshBuilder;
					RenderSinglePoly( GeomPoly, View, PDI, &MeshBuilder );
					MeshBuilder.Draw(PDI, GeomObject->GetActualBrush()->LocalToWorld(), UnselectedColorInstance, SDPG_Foreground);
				}
				PDI->SetHitProxy( NULL );
			}
		}
	}
}

// ------------------------------------------------------------------------------

void FEdModeGeometry::RenderEdge( const FSceneView* View, FPrimitiveDrawInterface* PDI )
{
	for( INT ObjectIdx = 0 ; ObjectIdx < GeomObjects.Num() ; ++ObjectIdx )
	{
		const FGeomObject* GeometryObject = GeomObjects(ObjectIdx);
		const FColor WireColor = GeometryObject->GetActualBrush()->GetWireColor();

		// Edges
		for( INT EdgeIdx = 0 ; EdgeIdx < GeometryObject->EdgePool.Num() ; ++EdgeIdx )
		{
			const FGeomEdge* GeometryEdge = &GeometryObject->EdgePool(EdgeIdx);
			const FColor Color = GeometryEdge->IsSelected() ? FColor(255,128,64) : WireColor;

			PDI->SetHitProxy( new HGeomEdgeProxy(const_cast<FGeomObject*>(GeometryObject),EdgeIdx) );
			{
				RenderSingleEdge( GeometryEdge, Color, View, PDI, 1 );
			}
			PDI->SetHitProxy( NULL );
		}
	}
}

// ------------------------------------------------------------------------------

void FEdModeGeometry::RenderVertex( const FSceneView* View, FPrimitiveDrawInterface* PDI )
{
	for( INT ObjectIdx = 0 ; ObjectIdx < GeomObjects.Num() ; ++ObjectIdx )
	{
		const FGeomObject* GeomObject = GeomObjects(ObjectIdx);

		// Draw the outline of the object, we will draw the vertices of the object on top of it.
		// Anytime we draw unselected lines of an object, we need to make it use an HActor hit proxy with the 
		// brush as the parameter so that users can click on the lines of the brush and toggle selection.
		PDI->SetHitProxy( new HActor( const_cast<ABrush*>( GeomObject->GetActualBrush() ) ) );
		{
			for( INT PolyIdx = 0 ; PolyIdx < GeomObject->PolyPool.Num() ; ++PolyIdx )
			{
				const FGeomPoly* GeomPoly = &GeomObject->PolyPool(PolyIdx);
				RenderSinglePolyWireframe( GeomPoly, GeomObject->GetActualBrush()->GetWireColor(), View, PDI );
			}
		}
		PDI->SetHitProxy(NULL);

		// Vertices

		FColor Color;
		FLOAT Scale;
		FVector Location;

		for( INT VertIdx = 0 ; VertIdx < GeomObject->VertexPool.Num() ; ++VertIdx )
		{
			const FGeomVertex* GeomVertex = &GeomObject->VertexPool(VertIdx);

			Location = GeomObject->GetActualBrush()->LocalToWorld().TransformFVector( *GeomVertex );
			Scale = View->WorldToScreen( Location ).W * ( 4 / View->SizeX / View->ProjectionMatrix.M[0][0] );
			const FLOAT SelStrength = GeomVertex->GetSelStrength();
			Color = GeomVertex->IsSelected() ? FColor(255*SelStrength,128*SelStrength,64*SelStrength) : GeomObject->GetActualBrush()->GetWireColor();

			PDI->SetHitProxy( new HGeomVertexProxy( const_cast<FGeomObject*>(GeomObject), VertIdx) );
			PDI->DrawSprite( Location, 4.f * Scale, 4.f * Scale, GWorld->GetWorldInfo()->BSPVertex->Resource, Color, SDPG_Foreground );
			PDI->SetHitProxy( NULL );

			if( ((FGeometryToolSettings*)GetSettings())->bShowNormals )
			{
				FVector Mid = GeomObject->GetActualBrush()->LocalToWorld().TransformFVector( GeomVertex->GetMid() );
				PDI->DrawLine( Mid, Mid+(GeomVertex->GetNormal() * (32*Scale)), FColor(255,0,0), SDPG_Foreground );
			}
		}
	}
}

// Applies delta information to a single vertex.

void FEdModeGeometry::ApplyToVertex( FGeomVertex* InVtx, FVector& InDrag, FRotator& InRot, FVector& InScale )
{
	// If we are only allowed to move the widget, leave.

	if( ((FGeometryToolSettings*)GetSettings())->bAffectWidgetOnly )
	{
		return;
	}

	UBOOL bUseSoftSelection = ((FGeometryToolSettings*)GetSettings())->bUseSoftSelection;

	// Drag

	if( !InDrag.IsZero() )
	{

		// Volumes store their rotation locally, whereas normal brush rotations are always in worldspace, so we need to transform
		// the drag vector into a volume's local space before applying it.
		const ABrush* Brush = InVtx->GetParentObject()->GetActualBrush();
		
		if( Brush->IsVolumeBrush() )
		{

			const FVector Wk = Brush->WorldToLocal().TransformNormal(InDrag);
			*InVtx += Wk * (bUseSoftSelection ? InVtx->GetSelStrength() : 1.f);
		}
		else
		{
			*InVtx += InDrag * (bUseSoftSelection ? InVtx->GetSelStrength() : 1.f);
		}
	}

	// Rotate
	
	if( !InRot.IsZero() )
	{
		FRotationMatrix matrix( InRot * (bUseSoftSelection ? InVtx->GetSelStrength() : 1.f) );

		FVector Wk( InVtx->X, InVtx->Y, InVtx->Z );
		Wk = InVtx->GetParentObject()->GetActualBrush()->LocalToWorld().TransformFVector( Wk );
		Wk -= GEditorModeTools().PivotLocation;
		Wk = matrix.TransformFVector( Wk );
		Wk += GEditorModeTools().PivotLocation;
		*InVtx = InVtx->GetParentObject()->GetActualBrush()->WorldToLocal().TransformFVector( Wk );
	}

	// Scale

	if( !InScale.IsZero() )
	{
		FScaleMatrix matrix(
			FVector(
				(InScale.X * (bUseSoftSelection ? InVtx->GetSelStrength() : 1.f)) / (GEditor->Constraints.GetGridSize() * 100.f),
				(InScale.Y * (bUseSoftSelection ? InVtx->GetSelStrength() : 1.f)) / (GEditor->Constraints.GetGridSize() * 100.f), 
				(InScale.Z * (bUseSoftSelection ? InVtx->GetSelStrength() : 1.f)) / (GEditor->Constraints.GetGridSize() * 100.f)
			)
		);

		FVector Wk( InVtx->X, InVtx->Y, InVtx->Z );
		Wk = InVtx->GetParentObject()->GetActualBrush()->LocalToWorld().TransformFVector( Wk );
		Wk -= GEditorModeTools().PivotLocation;
		Wk += matrix.TransformFVector( Wk );
		Wk += GEditorModeTools().PivotLocation;
		*InVtx = InVtx->GetParentObject()->GetActualBrush()->WorldToLocal().TransformFVector( Wk );
	}
}

/**
 * Compiles geometry mode information from the selected brushes.
 */

void FEdModeGeometry::GetFromSource()
{
	for( INT i=0; i<GeomObjects.Num(); i++ )
	{
		FGeomObject* GeomObject	= GeomObjects(i);
		delete GeomObject;
	}
	GeomObjects.Empty();

	// Notify the selected actors that they have been moved.
	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( Actor->IsABrush() )
		{
			ABrush* BrushActor = static_cast<ABrush*>( Actor );
			if( BrushActor->Brush != NULL )
			{
				FGeomObject* GeomObject			= new FGeomObject();
				GeomObject->SetParentObjectIndex( GeomObjects.AddItem( GeomObject ) );
				GeomObject->ActualBrush			= BrushActor;
				GeomObject->GetFromSource();
			}
		}
	}
}

/**
 * Changes the source brushes to match the current geometry data.
 */

void FEdModeGeometry::SendToSource()
{
	for( INT o = 0 ; o < GeomObjects.Num() ; ++o )
	{
		FGeomObject* go = GeomObjects(o);

		go->SendToSource();
	}
}

UBOOL FEdModeGeometry::FinalizeSourceData()
{
	UBOOL Result = 0;

	for( INT o = 0 ; o < GeomObjects.Num() ; ++o )
	{
		FGeomObject* go = GeomObjects(o);

		if( go->FinalizeSourceData() )
		{
			Result = 1;
		}
	}

	return Result;
}

void FEdModeGeometry::PostUndo()
{
	// Rebuild the geometry data from the current brush state

	GetFromSource();
	
	// Restore selection information.

	INT HighestSelectionIndex = INDEX_NONE;

	for( INT o = 0 ; o < GeomObjects.Num() ; ++o )
	{
		FGeomObject* go = GeomObjects(o);
		ABrush* Actor = go->GetActualBrush();

		for( INT s = 0 ; s < Actor->SavedSelections.Num() ; ++s )
		{
			FGeomSelection* gs = &Actor->SavedSelections(s);

			if( gs->SelectionIndex > HighestSelectionIndex )
			{
				HighestSelectionIndex = gs->SelectionIndex;
			}

			switch( gs->Type )
			{
				case GST_Poly:
				{
					go->PolyPool( gs->Index ).ForceSelectionIndex( gs->SelectionIndex );
					GEditorModeTools().PivotLocation = GEditorModeTools().SnappedLocation = go->PolyPool( gs->Index ).GetWidgetLocation();
				}
				break;
				case GST_Edge:
				{
					go->EdgePool( gs->Index ).ForceSelectionIndex( gs->SelectionIndex );
					GEditorModeTools().PivotLocation = GEditorModeTools().SnappedLocation = go->EdgePool( gs->Index ).GetWidgetLocation();
				}
				break;
				case GST_Vertex:
				{
					go->VertexPool( gs->Index ).ForceSelectionIndex( gs->SelectionIndex );
					GEditorModeTools().PivotLocation = GEditorModeTools().SnappedLocation = go->VertexPool( gs->Index ).GetWidgetLocation();
				}
				break;
			}
		}

		go->ForceLastSelectionIndex(HighestSelectionIndex );
	}
}

UBOOL FEdModeGeometry::ExecDelete()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();

	// Find the delete modifier and execute it.

	FModeTool_GeometryModify* mtgm = (FModeTool_GeometryModify*)FindTool( MT_GeometryModify );

	for( FModeTool_GeometryModify::TModifierIterator Itor( mtgm->ModifierIterator() ) ; Itor ; ++Itor )
	{
		UGeomModifier* gm = *Itor;

		if( gm->IsA( UGeomModifier_Delete::StaticClass()) )
		{
			if( gm->Apply() )
			{
				return 1;
			}
		}
	}

	return 0;
}

/**
 * Applies soft selection to the currently selected vertices.
 */

void FEdModeGeometry::SoftSelect()
{
	FGeometryToolSettings* Settings = (FGeometryToolSettings*)GetSettings();

	if( !Settings->bUseSoftSelection )
	{
		return;
	}

	for( INT o = 0 ; o < GeomObjects.Num() ; ++o )
	{
		FGeomObject* go = GeomObjects(o);
		go->SoftSelect();
	}
}

/**
 * Called when soft selection is turned off.  This clears out any extra data
 * that the geometry data might contain related to soft selection.
 */

void FEdModeGeometry::SoftSelectClear()
{
	FGeometryToolSettings* Settings = (FGeometryToolSettings*)GetSettings();

	for( INT o = 0 ; o < GeomObjects.Num() ; ++o )
	{
		FGeomObject* go = GeomObjects(o);

		for( INT v = 0 ; v < go->VertexPool.Num() ; ++v )
		{
			FGeomVertex* gv = &go->VertexPool(v);
			
			if( !gv->IsFullySelected() )
			{
				gv->Select( 0 );
			}
		}
	}
}

void FEdModeGeometry::ShowModifierWindow(UBOOL bShouldShow)
{
	ModifierWindow->Show( bShouldShow ? true : false );
}

void FEdModeGeometry::UpdateInternalData()
{
	GetFromSource();
}

/*------------------------------------------------------------------------------
	Terrain Tools
------------------------------------------------------------------------------*/

FEdModeTerrainEditing::FEdModeTerrainEditing()
{
	ID = EM_TerrainEdit;
	Desc = TEXT("Terrain Editing");
	bPerToolSettings = FALSE;
	bShowBallAndSticks = TRUE;
	ModeColor = FColor(255,255,255);
	CurrentTerrain = NULL;

	ToolColor = FLinearColor(1.0f, 0.0f, 0.0f);
	MirrorColor = FLinearColor(1.0f, 0.0f, 1.0f);

	BallTexture = (UTexture2D*)UObject::StaticLoadObject(UTexture2D::StaticClass(), NULL, 
		TEXT("EditorMaterials.TerrainLayerBrowser.TerrainBallImage"),NULL,LOAD_None,NULL);
	
	Tools.AddItem( new FModeTool_TerrainPaint() );
	Tools.AddItem( new FModeTool_TerrainSmooth() );
	Tools.AddItem( new FModeTool_TerrainAverage() );
	Tools.AddItem( new FModeTool_TerrainFlatten() );
	Tools.AddItem( new FModeTool_TerrainNoise() );
	Tools.AddItem( new FModeTool_TerrainVisibility() );
	Tools.AddItem( new FModeTool_TerrainTexturePan() );
	Tools.AddItem( new FModeTool_TerrainTextureRotate() );
	Tools.AddItem( new FModeTool_TerrainTextureScale() );
	Tools.AddItem( new FModeTool_TerrainSplitX() );
	Tools.AddItem( new FModeTool_TerrainSplitY() );
	Tools.AddItem( new FModeTool_TerrainMerge() );
	Tools.AddItem( new FModeTool_TerrainAddRemoveSectors() );
	Tools.AddItem( new FModeTool_TerrainOrientationFlip() );
	Tools.AddItem( new FModeTool_TerrainVertexEdit() );

	SetCurrentTool( MT_TerrainPaint );

	Settings = new FTerrainToolSettings;
}

const FToolSettings* FEdModeTerrainEditing::GetSettings() const
{
	if( bPerToolSettings )
	{
		return ((FModeTool_Terrain*)GetCurrentTool())->GetSettings();
	}
	else
	{
		return Settings;
	}
}

void FEdModeTerrainEditing::DrawTool( const FSceneView* View,FViewport* Viewport, FPrimitiveDrawInterface* PDI, 
	class ATerrain* Terrain, FVector& Location, FLOAT InnerRadius, FLOAT OuterRadius, TArray<ATerrain*>& Terrains )
{
	FTerrainToolSettings* ToolSettings = (FTerrainToolSettings*)(GetSettings());

	DrawToolCircle(View, Viewport, PDI, Terrain, Location, InnerRadius, Terrains);
	if (OuterRadius != InnerRadius)
	{
		DrawToolCircle(View, Viewport, PDI, Terrain, Location, OuterRadius, Terrains);
	}
	// Draw the 'ball' at the exact vertex...
	DrawToolCircleBallAndSticks(View, Viewport, PDI, Terrain, Location, InnerRadius, OuterRadius, Terrains);

	FModeTool_Terrain* TerrainTool = (FModeTool_Terrain*)CurrentTool;
	if ((TerrainTool->SupportsMirroring() == TRUE) && (ToolSettings->MirrorSetting != FTerrainToolSettings::TTMirror_NONE))
	{
		DetermineMirrorLocation( PDI, Terrain, Location );
		FVector MirroredPosition = MirrorLocation;

		FVector MirrorWorld = Terrain->LocalToWorld().TransformFVector(MirroredPosition);
        DrawToolCircle(View, Viewport, PDI, Terrain, MirrorWorld, InnerRadius, Terrains, TRUE);
		if (OuterRadius != InnerRadius)
		{
			DrawToolCircle(View, Viewport, PDI, Terrain, MirrorWorld, OuterRadius, Terrains, TRUE);
		}
		// Draw the 'ball' at the exact vertex...
		DrawToolCircleBallAndSticks(View, Viewport, PDI, Terrain, MirrorWorld, InnerRadius, OuterRadius, Terrains);
	}
}

#define CIRCLESEGMENTS 16
void FEdModeTerrainEditing::DrawToolCircle( const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI, 
	ATerrain* Terrain, FVector& Location, FLOAT Radius, TArray<ATerrain*>& Terrains, UBOOL bMirror )
{
	FTerrainToolSettings* ToolSettings = (FTerrainToolSettings*)(GetSettings());

	FVector	Extent(0,0,0);
	FVector	LastVertex(0,0,0);
	UBOOL	LastVertexValid = 0;
    for( INT s=0;s<=CIRCLESEGMENTS;s++ )
	{
		FLOAT theta =  PI * 2.f * s / CIRCLESEGMENTS;

		FVector TraceStart = Location;
		TraceStart.X += Radius * appSin(theta);
		TraceStart.Y += Radius * appCos(theta);
		FVector TraceEnd = TraceStart;
        TraceStart.Z = HALF_WORLD_MAX;
		TraceEnd.Z = -HALF_WORLD_MAX;

		FCheckResult Result;
		Result.Actor = NULL;

		// Get list of hit actors.
		FMemMark Mark(GMainThreadMemStack);

		INT TraceFlags = TRACE_Terrain|TRACE_TerrainIgnoreHoles|TRACE_Visible;
		
		FCheckResult* FirstHit = GWorld->MultiLineCheck(
			GMainThreadMemStack,
			TraceEnd,
			TraceStart,
			FVector(0.0f),
			TRACE_Terrain|TRACE_TerrainIgnoreHoles|TRACE_Visible,
			NULL,
			NULL
		);

		Mark.Pop();
		
		Result.Component = NULL;
		FCheckResult* CheckHit = FirstHit;
		while (CheckHit && (Result.Component == NULL))
		{
			if (CheckHit->Component->IsA(UTerrainComponent::StaticClass()) == FALSE)
			{
				CheckHit = CheckHit->GetNext();
			}
			else
			{
				if (CheckHit->Component->GetOwner() == Terrain)
				{
					Result.Component = CheckHit->Component;
					Result.Actor = CheckHit->Component->GetOwner();
					Result.Location = CheckHit->Location;
				}
				else
				{
					CheckHit = CheckHit->GetNext();
				}
			}
		}

		if (Result.Component != NULL)
		{
			ATerrain* HitTerrain = CastChecked<ATerrain>(Result.Actor);
			if( (Terrains.FindItemIndex(HitTerrain) != INDEX_NONE) ||
				(((FModeTool_Terrain*)GEditorModeTools().GetCurrentTool())->TerrainIsValid(HitTerrain, ToolSettings)) )
			{
				if(LastVertexValid)
				{
					if (bMirror == FALSE)
					{
						PDI->DrawLine(LastVertex,Result.Location,ToolColor,SDPG_Foreground);
					}
					else
					{
						PDI->DrawLine(LastVertex,Result.Location,MirrorColor,SDPG_Foreground);
					}
				}
				LastVertex = Result.Location;
				LastVertexValid = 1;
				Terrains.AddUniqueItem(HitTerrain);
			}
			else
			{
				LastVertexValid = 0;
			}
		}
		else
		{
			LastVertexValid = 0;
		}
	}
}

void FEdModeTerrainEditing::DrawToolCircleBallAndSticks( const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI, 
	class ATerrain* Terrain, FVector& Location, FLOAT InnerRadius, FLOAT OuterRadius, TArray<ATerrain*>& Terrains, UBOOL bMirror )
{
	FVector OutVertex;

	for (INT TerrainIndex = 0; TerrainIndex < Terrains.Num(); TerrainIndex++)
	{
		ATerrain* CheckTerrain = Terrains(TerrainIndex);
		UBOOL bGetConstained = FALSE;
		if (bConstrained && (CheckTerrain->EditorTessellationLevel > 0))
		{
			bGetConstained = TRUE;
		}
		if (CheckTerrain->GetClosestVertex(Location, OutVertex, bGetConstained) == TRUE)
		{
			// Draw the ball and stick
			DrawBallAndStick( View, Viewport, PDI, CheckTerrain, OutVertex, -1.0f );
		}
	}
}

void FEdModeTerrainEditing::DrawBallAndStick( const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI, 
	class ATerrain* Terrain, FVector& Location, FLOAT Strength )
{
	// Draw the line
	FVector Start = Location;
	FVector End = Location + FVector(0.0f, 0.0f, 20.0f);
	FLinearColor Color(1.0f, 0.0f, 0.0f, 1.0f);
	PDI->DrawLine(Start, End, Color, SDPG_Foreground);

	// Determine the size and color for the ball
	// Calculate the view-dependent scaling factor.
	FLOAT ViewedSizeX = 10.0f;
	FLOAT ViewedSizeY = 10.0f;
	if ((View->ProjectionMatrix.M[3][3] != 1.0f))
	{
		FVector EndView = View->ViewMatrix.TransformFVector(End);
		static FLOAT BallDivisor = 40.0f;
		ViewedSizeX = EndView.Z / BallDivisor;
		ViewedSizeY = EndView.Z / BallDivisor;
	}

	if (Strength < 0.0f)
	{
		// This indicates the ball should only be rendered in a Red shade
		Color = FLinearColor(abs(Strength), 0.0f, 0.0f);
	}
	else
	{
		Color = FLinearColor(Strength, Strength, Strength);
	}
	PDI->DrawSprite(End, ViewedSizeX, ViewedSizeY, BallTexture->Resource, Color, SDPG_Foreground);
}

void FEdModeTerrainEditing::Render(const FSceneView* View,FViewport* Viewport,FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View,Viewport,PDI);

	if(!PDI->IsHitTesting())
	{
		// check the mouse cursor is inside the viewport
		FIntPoint	MousePosition;
		Viewport->GetMousePos(MousePosition);
		INT		X = MousePosition.X,
				Y = MousePosition.Y;
		if ((X >= 0) && (Y >=0) && (X < (INT)Viewport->GetSizeX()) && (Y < (INT)Viewport->GetSizeY()))
		{
			FTerrainToolSettings* CurrentSettings = (FTerrainToolSettings*)GetSettings();
			FVector HitNormal;
			ATerrain* Terrain = ((FModeTool_Terrain*)GEditorModeTools().GetCurrentTool())->TerrainTrace(Viewport,View,ToolHitLocation,HitNormal,CurrentSettings, TRUE);

			if (Terrain)
			{
				ToolTerrains.Empty(1);
				ToolTerrains.AddUniqueItem(Terrain);

				DrawTool(View,Viewport,PDI,Terrain,ToolHitLocation,CurrentSettings->RadiusMin,CurrentSettings->RadiusMax,ToolTerrains);
			}

			// Allow the tool to render anything custom.
			for (INT ToolTerrainIndex = 0; ToolTerrainIndex < ToolTerrains.Num(); ToolTerrainIndex++)
			{
				ATerrain* ToolTerrain = ToolTerrains(ToolTerrainIndex);
				((FModeTool_Terrain*)GEditorModeTools().GetCurrentTool())->Render( ToolTerrain, ToolHitLocation, View, Viewport, PDI );
			}
		}
	}
}

void FEdModeTerrainEditing::DetermineMirrorLocation( FPrimitiveDrawInterface* PDI, class ATerrain* Terrain, FVector& Location )
{
	FTerrainToolSettings* ToolSettings = (FTerrainToolSettings*)(GetSettings());
	FVector LocalPosition = Terrain->WorldToLocal().TransformFVector(Location);

	FLOAT HalfX = (FLOAT)(Terrain->NumPatchesX) / 2.0f;
	FLOAT HalfY = (FLOAT)(Terrain->NumPatchesY) / 2.0f;

	FVector MirroredPosition = LocalPosition;
	FLOAT Diff;

	switch (ToolSettings->MirrorSetting)
	{
	case FTerrainToolSettings::TTMirror_X:
		// Mirror about the X-axis
		Diff = HalfY - MirroredPosition.Y;
		MirroredPosition.Y = HalfY + Diff;
		break;
	case FTerrainToolSettings::TTMirror_Y:
		// Mirror about the Y-axis
		Diff = HalfX - MirroredPosition.X;
		MirroredPosition.X = HalfX + Diff;
		break;
	case FTerrainToolSettings::TTMirror_XY:
		// Mirror about the X and Y-axes
		Diff = HalfX - MirroredPosition.X;
		MirroredPosition.X = HalfX + Diff;
		Diff = HalfY - MirroredPosition.Y;
		MirroredPosition.Y = HalfY + Diff;
		break;
	}

	MirrorLocation = MirroredPosition;
}

INT FEdModeTerrainEditing::GetMirroredValue_Y(ATerrain* Terrain, INT InY, UBOOL bPatchOperation)
{
	INT HalfY = Terrain->NumPatchesY / 2;

	// Mirror about the X-axis
	INT Diff = HalfY - InY;
	INT ReturnY = HalfY + Diff;
	if (bPatchOperation == TRUE)
	{
		ReturnY--;
	}

	return ReturnY;
}

INT FEdModeTerrainEditing::GetMirroredValue_X(ATerrain* Terrain, INT InX, UBOOL bPatchOperation)
{
	INT HalfX = Terrain->NumPatchesX / 2;

	// Mirror about the X-axis
	INT Diff = HalfX - InX;
	INT ReturnX = HalfX + Diff;
	if (bPatchOperation == TRUE)
	{
		ReturnX--;
	}

	return ReturnX;
}

UBOOL FEdModeTerrainEditing::InputKey(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event)
{
	if( (Key == KEY_MouseScrollUp || Key == KEY_MouseScrollDown) && (ViewportClient->Input->IsShiftPressed() || ViewportClient->Input->IsAltPressed() ) )
	{
		FTerrainToolSettings* ToolSettings = (FTerrainToolSettings*)(GetSettings());
		int Delta = (Key == KEY_MouseScrollUp) ? 32 : -32;

		UBOOL bUpdate = FALSE;
		if( ViewportClient->Input->IsShiftPressed() )
		{
			ToolSettings->RadiusMax += Delta;
			bUpdate = TRUE;
		}

		if( ViewportClient->Input->IsAltPressed() )
		{
			ToolSettings->RadiusMin += Delta;
			bUpdate = TRUE;
		}

		if (bUpdate == TRUE)
		{
			ViewportClient->Invalidate( TRUE, TRUE );
			GCallbackEvent->Send(CALLBACK_RefreshEditor_TerrainBrowser);
		}

		return TRUE;
	}

	return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}

UBOOL FEdModeTerrainEditing::HandleClick(HHitProxy* HitProxy, const FViewportClick& Click)
{
	UBOOL bProcessedLocally = FALSE;

	if (HitProxy == NULL)
	{
		int dummy = 0;
	}
	else if (HitProxy->IsA(HActor::StaticGetType()))
	{
		HActor* ActorHitProxy = (HActor*)HitProxy;
		if (ActorHitProxy->Actor)
		{
			if (ActorHitProxy->Actor->IsA(ATerrain::StaticClass()) == FALSE)
			{
				// Don't process this!
				bProcessedLocally = TRUE;
			}
		}
	}
	else if (HitProxy->IsA(HBSPBrushVert::StaticGetType()))
	{
		// Don't process this!
		int dummy = 0;
		bProcessedLocally = TRUE;
	}
	else if (HitProxy->IsA(HStaticMeshVert::StaticGetType()))
	{
		// Don't process this!
		int dummy = 0;
		bProcessedLocally = TRUE;
	}
	else if (HitProxy->IsA(HGeomPolyProxy::StaticGetType()))
	{
		// Don't process this!
		int dummy = 0;
		bProcessedLocally = TRUE;
	}
	else if (HitProxy->IsA(HGeomEdgeProxy::StaticGetType()))
	{
		// Don't process this!
		int dummy = 0;
		bProcessedLocally = TRUE;
	}
	else if (HitProxy->IsA(HGeomVertexProxy::StaticGetType()))
	{
		// Don't process this!
		int dummy = 0;
		bProcessedLocally = TRUE;
	}
	else
	{
		debugf(NAME_Warning,TEXT("Unknown hit proxy type '%s' in FEdModeTerrainEditing::HandleClick()"), HitProxy->GetType()->GetName());
	}

	if (!bProcessedLocally)
	{
		return FEdMode::HandleClick(HitProxy,Click);
	}
	return TRUE;
}

/*------------------------------------------------------------------------------
	Texture
------------------------------------------------------------------------------*/

FEdModeTexture::FEdModeTexture()
	:	ScopedTransaction( NULL )
{
	ID = EM_Texture;
	Desc = TEXT("Texture Alignment");

	Tools.AddItem( new FModeTool_Texture() );
	SetCurrentTool( MT_Texture );

	Settings = new FTextureToolSettings;
}

FEdModeTexture::~FEdModeTexture()
{
	// Ensure no transaction is outstanding.
	check( !ScopedTransaction );
}

void FEdModeTexture::Enter()
{
	FEdMode::Enter();

	SaveCoordSystem = GEditorModeTools().CoordSystem;
	GEditorModeTools().CoordSystem = COORD_Local;
}

void FEdModeTexture::Exit()
{
	FEdMode::Exit();

	GEditorModeTools().CoordSystem = SaveCoordSystem;
}

FVector FEdModeTexture::GetWidgetLocation() const
{
	//debugf(TEXT("In FEdModeTexture::GetWidgetLocation"));
	for ( TSelectedSurfaceIterator<> It ; It ; ++It )
	{
		FBspSurf* Surf = *It;
		FPoly* poly = &((ABrush*)Surf->Actor)->Brush->Polys->Element( Surf->iBrushPoly );
		return Surf->Actor->LocalToWorld().TransformFVector( poly->GetMidPoint() );
	}

	return FEdMode::GetWidgetLocation();
}

UBOOL FEdModeTexture::ShouldDrawWidget() const
{
	return TRUE;
}

UBOOL FEdModeTexture::GetCustomDrawingCoordinateSystem( FMatrix& InMatrix, void* InData )
{
	FPoly* poly = NULL;
	//FMatrix LocalToWorld;

	for ( TSelectedSurfaceIterator<> It ; It ; ++It )
	{
		FBspSurf* Surf = *It;
		poly = &((ABrush*)Surf->Actor)->Brush->Polys->Element( Surf->iBrushPoly );
		//LocalToWorld = Surf->Actor->LocalToWorld();
		break;
	}

	if( !poly )
	{
		return FALSE;
	}

	InMatrix = FMatrix::Identity;

	InMatrix.SetAxis( 2, poly->Normal );
	InMatrix.SetAxis( 0, poly->TextureU );
	InMatrix.SetAxis( 1, poly->TextureV );

	InMatrix.RemoveScaling();

	return TRUE;
}

UBOOL FEdModeTexture::GetCustomInputCoordinateSystem( FMatrix& InMatrix, void* InData )
{
	return FALSE;
}

INT FEdModeTexture::GetWidgetAxisToDraw( FWidget::EWidgetMode InWidgetMode ) const
{
	switch( InWidgetMode )
	{
		case FWidget::WM_Translate:
		case FWidget::WM_ScaleNonUniform:
			return AXIS_X | AXIS_Y;
			break;

		case FWidget::WM_Rotate:
			return AXIS_Z;
			break;
	}

	return AXIS_XYZ;
}

UBOOL FEdModeTexture::StartTracking()
{
	checkMsg( !ScopedTransaction, "FEdModeTexture::StartTracking without matching FEdModeTexture::EndTracking" );
	ScopedTransaction = new FScopedTransaction( *LocalizeUnrealEd(TEXT("TextureManipulation")) );

	FOR_EACH_UMODEL;
		Model->ModifySelectedSurfs( TRUE );
	END_FOR_EACH_UMODEL;

	return TRUE;
}

UBOOL FEdModeTexture::EndTracking()
{
	checkMsg( ScopedTransaction, "FEdModeTexture::EndTracking without matching FEdModeTexture::StartTracking" );
	delete ScopedTransaction;
	ScopedTransaction = NULL;

	GWorld->MarkPackageDirty();
	GCallbackEvent->Send( CALLBACK_LevelDirtied );

	return TRUE;
}


/*------------------------------------------------------------------------------
	FEdModeCoverEdit.
------------------------------------------------------------------------------*/

FEdModeCoverEdit::FEdModeCoverEdit() : FEdMode()
{
	ID = EM_CoverEdit;
	Desc = TEXT("Cover Editing");
}

FEdModeCoverEdit::~FEdModeCoverEdit()
{
}

void FEdModeCoverEdit::Enter()
{
	bCanAltDrag = TRUE;

	// set cover rendering flag
	if (GEditor != NULL)
	{
		for (INT Idx = 0; Idx < GEditor->ViewportClients.Num(); Idx++)
		{
			if (GEditor->ViewportClients(Idx) != NULL)
			{
				GEditor->ViewportClients(Idx)->ShowFlags |= SHOW_Cover;
			}
		}
	}
	FEdMode::Enter();
}

void FEdModeCoverEdit::Exit()
{
	FEdMode::Exit();
	// clear cover rendering flag
	if (GEditor != NULL)
	{
		for (INT Idx = 0; Idx < GEditor->ViewportClients.Num(); Idx++)
		{
			if (GEditor->ViewportClients(Idx) != NULL)
			{
				GEditor->ViewportClients(Idx)->ShowFlags &= ~SHOW_Cover;
			}
		}
	}
}

UBOOL FEdModeCoverEdit::UsesWidget(FWidget::EWidgetMode CheckMode) const
{
	return (CheckMode == FWidget::WM_Translate || CheckMode == FWidget::WM_Rotate || CheckMode == FWidget::WM_Scale);
}

UBOOL FEdModeCoverEdit::Select( AActor *InActor, UBOOL bInSelected )
{
	return 0;
}

/**
 * Overridden to handle selecting individual cover link slots.
 */
UBOOL FEdModeCoverEdit::InputKey(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event)
{
	// Get some useful info about buttons being held down
	const UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);
	const UBOOL bShiftDown = Viewport->KeyState(KEY_LeftShift) || Viewport->KeyState(KEY_RightShift);
	const UBOOL bAltDown = Viewport->KeyState(KEY_LeftAlt) || Viewport->KeyState(KEY_RightAlt);
	const INT HitX = Viewport->GetMouseX(), HitY = Viewport->GetMouseY();
	// grab a list of currently selected links
	TArray<ACoverLink*> SelectedLinks;
	TArray<ACoverGroup*> SelectedGroups;
	GEditor->GetSelectedActors()->GetSelectedObjects<ACoverLink>(SelectedLinks);
	GEditor->GetSelectedActors()->GetSelectedObjects<ACoverGroup>(SelectedGroups);

	if (Key == KEY_LeftMouseButton)
	{
		switch (Event)
		{
			case IE_Released:
			{
				// sort all selected links, in case a drag changed the slot positions dramatically
				for (INT Idx = 0; Idx < SelectedLinks.Num(); Idx++)
				{
					SelectedLinks(Idx)->SortSlots();
				}
				bCanAltDrag = TRUE;
				break;
			}
			default:
			{
				break;
			}
		}
	}
	else if (Key == KEY_Delete)
	{
		if (Event == IE_Pressed)
		{
			// delete the selected slots
			UBOOL bSelectedSlots = FALSE;
			for (INT LinkIdx = 0; LinkIdx < SelectedLinks.Num(); LinkIdx++)
			{
				ACoverLink *Link = SelectedLinks(LinkIdx);
				for( INT Idx = 0; Idx < Link->Slots.Num(); Idx++ )
				{
					FCoverSlot& Slot = Link->Slots(Idx);

					if( Slot.bSelected )
					{
						// force a min of 2 slots for circular cover or 
						// a min of 1 slots for regular cover
						if( ( Link->bCircular && Link->Slots.Num() > 2) || 
							(!Link->bCircular && Link->Slots.Num() > 1)	)
						{
							// Clean up mantle targets
							ACoverLink* TargetLink	  = Cast<ACoverLink>(Slot.MantleTarget.Actor);
							INT			TargetSlotIdx = Slot.MantleTarget.SlotIdx;
							if( TargetLink )
							{
								// Clear self from target link
								if( TargetSlotIdx < TargetLink->Slots.Num() )
								{
									TargetLink->Slots(TargetSlotIdx).MantleTarget.Actor		= NULL;
									TargetLink->Slots(TargetSlotIdx).MantleTarget.SlotIdx	= -1;
								}
							}

							// Clean up swat turn targets
							for( INT TurnIdx = 0; TurnIdx < Slot.TurnTarget.Num(); TurnIdx++ )
							{
								TargetLink	  = Cast<ACoverLink>(Slot.TurnTarget(TurnIdx).Actor);
								TargetSlotIdx = Slot.TurnTarget(TurnIdx).SlotIdx;
								if( TargetLink )
								{
									// Clear self from target link
									if( TargetSlotIdx < TargetLink->Slots.Num() )
									{
										FCoverSlot& TargetSlot = TargetLink->Slots(TargetSlotIdx);
										for( INT TargIdx = 0; TargIdx < TargetSlot.TurnTarget.Num(); TargIdx++ )
										{
											if( TargetSlot.TurnTarget(TargIdx).Actor  == TargetLink	&&
												TargetSlot.TurnTarget(TargIdx).SlotIdx == TargetSlotIdx )
											{
												TargetSlot.TurnTarget.Remove( TargIdx );
												break;
											}
										}
									}
								}
							}
								
							Link->Slots.Remove(Idx--,1);
						}

						bSelectedSlots = TRUE;
					}
				}

				Link->ForceUpdateComponents(FALSE,FALSE);
			}

			// If there were slots selected - just refresh viewport
			// Otherwise, fall through to delete actors
			if( bSelectedSlots )
			{
				GEditor->RedrawAllViewports();
				return 1;
			}
		}
	}
	else if (Key == KEY_Insert)
	{
		if (Event == IE_Pressed)
		{
			// insert a new slot
			for (INT LinkIdx = 0; LinkIdx < SelectedLinks.Num(); LinkIdx++)
			{
				ACoverLink *Link = SelectedLinks(LinkIdx);
				if (Link->bCircular &&
					Link->Slots.Num() >= 2)
				{
					// don't allow more than 2 slots for circular cover
					continue;
				}
				// look for a selected slot
				UBOOL bLinkSelected = 0;
				INT NumSlots = Link->Slots.Num();
				for (INT Idx = 0; Idx < NumSlots; Idx++)
				{
					FCoverSlot &Slot = Link->Slots(Idx);
					if( Slot.bSelected )
					{
						bLinkSelected = TRUE;
						// Create one with an offset to this slot
						// and match necessary information
						FCoverSlot NewSlot = Slot;
						NewSlot.LocationOffset += FVector(0.f,64.f,0.f);
						// Select new slot and deselect old one
						NewSlot.bSelected	= TRUE;
						Slot.bSelected		= FALSE;
						// add tot he list
						Link->Slots.AddItem(NewSlot);
					}
				}

				// if no links were selected create one at the end
				if (!bLinkSelected)
				{
					const INT SlotIdx = Link->Slots.AddZeroed();
					Link->Slots(SlotIdx).LocationOffset = Link->Slots(SlotIdx-1).LocationOffset + FVector(0,64.f,0);
					Link->Slots(SlotIdx).RotationOffset = Link->Slots(SlotIdx-1).RotationOffset;
					Link->Slots(SlotIdx).bEnabled = TRUE;
				}

				Link->ForceUpdateComponents(FALSE,FALSE);
			}

			// Auto adjust groups if no links selected
			if( !SelectedLinks.Num() )
			{
				for( INT GroupIdx = 0; GroupIdx < SelectedGroups.Num(); GroupIdx++ )
				{
					SelectedGroups(GroupIdx)->ToggleGroup();
				}
			}

			GEditor->RedrawAllViewports();
		}
		return 1;
	}
	else if (Key == KEY_MiddleMouseButton)
	{
		if (Event == IE_Pressed)
		{
#if !FINAL_RELEASE
			// Clear nasty debug cylinder
			// REMOVE AFTER LDs SOLVE ISSUES
			AWorldInfo *Info = GWorld->GetWorldInfo();
			Info->FlushPersistentDebugLines();
#endif

			// auto-adjust selected slots
			for (INT LinkIdx = 0; LinkIdx < SelectedLinks.Num(); LinkIdx++)
			{
				ACoverLink *Link = SelectedLinks(LinkIdx);
				Link->SortSlots();
				Link->FindBase();
				UBOOL bLinkSelected = 0;

				for( INT Idx = 0; Idx < Link->Slots.Num(); Idx++ )
				{
					if( Link->Slots(Idx).bSelected )
					{
						bLinkSelected = 1;
						Link->AutoAdjustSlot( Idx, FALSE );
						Link->AutoAdjustSlot( Idx, TRUE );
						Link->BuildSlotInfo( Idx );
					}
				}

				// if no links were selected, then adjust all of them
				if( !bLinkSelected )
				{
					// first pass to auto position everything
					for( INT Idx = 0; Idx < Link->Slots.Num(); Idx++ )
					{
						Link->AutoAdjustSlot( Idx, FALSE );
					}
					// second pass to build the slot info
					for( INT Idx = 0; Idx < Link->Slots.Num(); Idx++ )
					{
						Link->AutoAdjustSlot( Idx, TRUE );
						Link->BuildSlotInfo( Idx );
					}
				}

				if (Link->Base != NULL && (Link->Base->GetOutermost() != Link->GetOutermost()))
				{
					Link->SetBase(NULL);
				}

				Link->ForceUpdateComponents(FALSE,FALSE);
			}

			// Auto adjust groups if no links selected
			if( !SelectedLinks.Num() )
			{
				TArray<ACoverLink*> Junk;
				for( INT GroupIdx = 0; GroupIdx < SelectedGroups.Num(); GroupIdx++ )
				{
					ACoverGroup* Group = SelectedGroups(GroupIdx);
					Group->AutoFillGroup( CGFA_Cylinder, Junk );
				}
			}

			GEditor->RedrawAllViewports();
		}
		return 1;
	}
	else if ( Key == KEY_F )
	{
		//disable forcefirelinks
		/*
		if ( Event == IE_Pressed )
		{
			if (SelectedLinks.Num() > 1)
			{
				TArray<FCoverInfo> LinkSlots;
				// grab a list of slots to link together
				for (INT Idx = 0; Idx < SelectedLinks.Num(); Idx++)
				{
					ACoverLink *Link = SelectedLinks(Idx);
					if (Link != NULL && Link->Slots.Num() > 0)
					{
						for (INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++)
						{
							FCoverSlot &Slot = Link->Slots(SlotIdx);
							if (Slot.bSelected)
							{
								const INT NewIdx = LinkSlots.AddZeroed();
								LinkSlots(NewIdx).Link = Link;
								LinkSlots(NewIdx).SlotIdx = SlotIdx;
								
							}
						}
					}
				}
				// link up all the slots
				for (INT Idx = 0; Idx < LinkSlots.Num(); Idx++)
				{
					FCoverSlot &Slot = LinkSlots(Idx).Link->Slots(LinkSlots(Idx).SlotIdx);
					for (INT OtherIdx = 0; OtherIdx < LinkSlots.Num(); OtherIdx++)
					{
						if (LinkSlots(Idx).Link != LinkSlots(OtherIdx).Link)
						{
							const INT LinkIdx = Slot.ForcedFireLinks.AddZeroed();
							Slot.ForcedFireLinks(LinkIdx).TargetLink		= LinkSlots(OtherIdx).Link;
							Slot.ForcedFireLinks(LinkIdx).TargetSlotIdx		= LinkSlots(OtherIdx).SlotIdx;
							Slot.ForcedFireLinks(LinkIdx).LastLinkLocation  = LinkSlots(OtherIdx).Link->Location;
							Slot.ForcedFireLinks(LinkIdx).CoverType			= Slot.CoverType;
							if (Slot.CoverType == CT_MidLevel)
							{
								Slot.ForcedFireLinks(LinkIdx).CoverActions.AddItem(CA_PopUp);
							}
							if (Slot.bLeanLeft)
							{
								Slot.ForcedFireLinks(LinkIdx).CoverActions.AddItem(CA_LeanLeft);
							}
							if (Slot.bLeanRight)
							{
								Slot.ForcedFireLinks(LinkIdx).CoverActions.AddItem(CA_LeanRight);
							}
						}
					}
				}
			}
		}*/
		return 1;
	}
	return FEdMode::InputKey(ViewportClient,Viewport,Key,Event);
}

UBOOL FEdModeCoverEdit::HandleClick(HHitProxy *HitProxy, const FViewportClick &Click)
{
	if (HitProxy != NULL)
	{
		if (HitProxy->IsA(HActorComplex::StaticGetType()))
		{
			HActorComplex *ComplexProxy = (HActorComplex*)HitProxy;
			ACoverLink *Link = Cast<ACoverLink>(ComplexProxy->Actor);
			if (Link != NULL)
			{
				if (Click.GetKey() == KEY_RightMouseButton)
				{
					FCoverSlot &Slot = Link->Slots(ComplexProxy->Index);
					// make sure the slot is selected
					Slot.bSelected = TRUE;
					// and show the menu
					GEditor->ShowUnrealEdContextCoverSlotMenu(Link,Slot);
				}
				else
				{
					if (!Click.IsControlDown())
					{
						// deselect all other slots
						for (INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++)
						{
							if (Link->Slots(SlotIdx).bSelected &&
								SlotIdx != ComplexProxy->Index)
							{
								Link->Slots(SlotIdx).bSelected = 0;
							}
						}
					}
				}
				// toggle selection on the clicked thing
				Link->Slots(ComplexProxy->Index).bSelected = !Link->Slots(ComplexProxy->Index).bSelected;

				// Reattach the cover link's components, to show the selection update.
				Link->ForceUpdateComponents(FALSE,FALSE);

				// note that we handled the click
				return 1;
			}
		}
	}
	// if clicked on something other than a slot, deselect all slots
	if (HitProxy == NULL ||
		!HitProxy->IsA(HActor::StaticGetType()) ||
		((HActor*)HitProxy)->Actor == NULL ||
		!((HActor*)HitProxy)->Actor->IsA(ACoverLink::StaticClass()))
	{
		// check to see if any slots are currently selected
		TArray<ACoverLink*> SelectedLinks;
		if (GEditor->GetSelectedActors()->GetSelectedObjects<ACoverLink>(SelectedLinks) > 0)
		{
			UBOOL bHadSelectedSlots = 0;
			for (INT Idx = 0; Idx < SelectedLinks.Num(); Idx++)
			{
				ACoverLink *Link = SelectedLinks(Idx);
				for (INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++)
				{
					if (Link->Slots(SlotIdx).bSelected)
					{
						bHadSelectedSlots = 1;
						// deselect the slot
						Link->Slots(SlotIdx).bSelected = 0;
					}
				}
				if (bHadSelectedSlots)
				{
					Link->ForceUpdateComponents(FALSE,FALSE);
				}
			}
			// if we deselected slots,
			if (bHadSelectedSlots)
			{
				// then claim the click and keep the links selected
				return 1;
			}
		}
	}

	return FEdMode::HandleClick(HitProxy,Click);
}

/**
 * Overridden to duplicate selected slots instead of the actor.
 */
UBOOL FEdModeCoverEdit::HandleDuplicate()
{
	UBOOL bSlotDuplicated = FALSE;
	// grab a list of currently selected links
	TArray<ACoverLink*> SelectedLinks;
	GEditor->GetSelectedActors()->GetSelectedObjects<ACoverLink>(SelectedLinks);
	// make a copy of all currently selected slots
	for (INT LinkIdx = 0; LinkIdx < SelectedLinks.Num(); LinkIdx++)
	{
		ACoverLink *Link = SelectedLinks(LinkIdx);
		const INT NumSlots = Link->Slots.Num();
		for (INT Idx = 0; Idx < NumSlots; Idx++)
		{
			FCoverSlot& Slot = Link->Slots(Idx);
			if( Slot.bSelected )
			{
				// create a copy of the slot
				FCoverSlot NewSlot = Slot;
				// offset slightly
				NewSlot.LocationOffset += FVector(0.f,64.f,0.f);
				// select new one, deselect old one
				NewSlot.bSelected = TRUE;
				Slot.bSelected	   = FALSE;
				// note that a slot was duplicated
				bSlotDuplicated = TRUE;
				// and add this to the end of the list of slots for the link
				Link->Slots.AddItem(NewSlot);
			}
		}
		Link->SortSlots();
		Link->ForceUpdateComponents(FALSE,FALSE);
	}
	return bSlotDuplicated;
}

/**
 * Overridden to drag duplicate a selected slot if possible, instead of the actor.
 */
UBOOL FEdModeCoverEdit::HandleDragDuplicate()
{
	UBOOL bSlotDuplicated = FALSE;
	// grab a list of currently selected links
	TArray<ACoverLink*> SelectedLinks;
	GEditor->GetSelectedActors()->GetSelectedObjects<ACoverLink>(SelectedLinks);

	// make a copy of all currently selected slots
	for( INT LinkIdx = 0; LinkIdx < SelectedLinks.Num(); LinkIdx++ )
	{
		ACoverLink *Link = SelectedLinks(LinkIdx);

		// Store original slot count so we don't duplicate duplicates 
		// when we select them
		const INT SlotNum = Link->Slots.Num();

		for( INT Idx = 0; Idx < SlotNum; Idx++ )
		{
			FCoverSlot &Slot = Link->Slots(Idx);
			// Only affect selected slots
			if( Slot.bSelected )
			{
				FCoverSlot NewSlot = Slot;
				// Select new slot - unselect old
				NewSlot.bSelected	= TRUE;
				Slot.bSelected		= FALSE;
				// Set return flag
				bSlotDuplicated		= TRUE;
				Link->Slots.AddItem(NewSlot);
			}
		}
		Link->ForceUpdateComponents(FALSE,FALSE);
	}
	return bSlotDuplicated;
}

/**
 * Overridden to handle dragging/rotating cover link slots.
 */
UBOOL FEdModeCoverEdit::InputDelta(FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale)
{
	// Get some useful info about buttons being held down
	UBOOL bCtrlDown		= InViewport->KeyState(KEY_LeftControl) || InViewport->KeyState(KEY_RightControl);
	UBOOL bShiftDown	= InViewport->KeyState(KEY_LeftShift)	|| InViewport->KeyState(KEY_RightShift);
	UBOOL bAltDown		= InViewport->KeyState(KEY_LeftAlt)		|| InViewport->KeyState(KEY_RightAlt);
	UBOOL bTabDown		= InViewport->KeyState(KEY_Tab);
	UBOOL bMovedObjects = 0;
	UBOOL bApplyToActor = 0;

	TArray<ACoverLink*> SelectedLinks;
	GEditor->GetSelectedActors()->GetSelectedObjects<ACoverLink>(SelectedLinks);

	// Look for a selected slot
	UBOOL bSlotsSelected = 0;
	for( INT LinkIdx = 0; LinkIdx < SelectedLinks.Num(); LinkIdx++ )
	{
		ACoverLink* Link = SelectedLinks(LinkIdx);
		for( INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++ )
		{
			FCoverSlot &Slot = Link->Slots(SlotIdx);
			if( Slot.bSelected )
			{
				bSlotsSelected = 1;
			}
		}		
	}

	if (bSlotsSelected && bAltDown && bCanAltDrag)
	{
		bCanAltDrag = FALSE;
		HandleDragDuplicate();
	}

	for( INT LinkIdx = 0; LinkIdx < SelectedLinks.Num(); LinkIdx++ )
	{
		ACoverLink* Link = SelectedLinks(LinkIdx);
		for( INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++ )
		{
			FCoverSlot &Slot = Link->Slots(SlotIdx);

			// If slots are selected
			if( bSlotsSelected )
			{
				// Move only selected slots if we grabbed widget or holding ctrl
				if( Slot.bSelected && 
					(InViewportClient->Widget->GetCurrentAxis() != AXIS_None ||
					 bCtrlDown) )
				{
					// Update slot offsets
					Slot.LocationOffset += FRotationMatrix(Link->Rotation).InverseTransformFVectorNoScale( InDrag );
					Slot.RotationOffset += InRot;

					// Limit the pitch/roll
					Slot.RotationOffset.Pitch	= 0;
					Slot.RotationOffset.Roll	= 0;

					// Set flag saying we handled all movement
					bMovedObjects = 1;
				}
			}
			else if( bTabDown )
			{
				// Apply the inverse location to keep the slots stationary in relation to the link
				Slot.LocationOffset -= FRotationMatrix(Link->Rotation).InverseTransformFVectorNoScale(InDrag);
				Slot.RotationOffset -= InRot;
				
				bMovedObjects = 1;
				bApplyToActor = 1;
			}	
		}
		Link->ForceUpdateComponents(FALSE,FALSE);
	}

	if( bApplyToActor )
	{
		InViewportClient->ApplyDeltaToActors( InDrag, InRot, InScale );
	}

	if( bMovedObjects )
	{
		// If holding shift - adjust camera with slots
		if( bShiftDown )
		{
			FVector CameraDelta( InDrag );
			if( InViewportClient->ViewportType == LVT_OrthoXY )
			{
				CameraDelta.X = -InDrag.Y;
				CameraDelta.Y =  InDrag.X;
			}
			InViewportClient->MoveViewportCamera( CameraDelta, FRotator(0,0,0) );
		}

		return 1;
	}
	else
	{
		return FEdMode::InputDelta(InViewportClient,InViewport,InDrag,InRot,InScale);
	}
}

/**
 * Returns the current selected slot location.
 */
FVector FEdModeCoverEdit::GetWidgetLocation() const
{
	//debugf(TEXT("In FEdModeCoverEdit::GetWidgetLocation"));

	// grab a list of currently selected links
	TArray<ACoverLink*> SelectedLinks;
	GEditor->GetSelectedActors()->GetSelectedObjects<ACoverLink>(SelectedLinks);
	if( SelectedLinks.Num() )
	{
		ACoverLink *Link = SelectedLinks(0);
		// check to see if a slot is selected
		for (INT Idx = 0; Idx < Link->Slots.Num(); Idx++)
		{
			if (Link->Slots(Idx).bSelected)
			{
				return (Link->Location + FRotationMatrix(Link->Rotation).TransformFVector(Link->Slots(Idx).LocationOffset));
			}
		}
		// no slot selected so use the link location
		return Link->Location;
	}

	// If another type of actor selected
	TArray<AActor*> SelectedActors;
	GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);
	if( SelectedActors.Num() )
	{
		return SelectedActors(0)->Location;
	}

	// no cover, invalid widget location
	return FVector(0.f,0.f,0.f);
}

/**
 * Draw some simple information about the currently selected links.
 */
void FEdModeCoverEdit::DrawHUD(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,const FSceneView* View,FCanvas* Canvas)
{
	FEdMode::DrawHUD(ViewportClient,Viewport,View,Canvas);
	if (!ViewportClient->IsOrtho())
	{
		// grab a list of currently selected links
		TArray<ACoverLink*> SelectedLinks;
		GEditor->GetSelectedActors()->GetSelectedObjects<ACoverLink>(SelectedLinks);
		for (INT LinkIdx = 0; LinkIdx < SelectedLinks.Num(); LinkIdx++)
		{
			ACoverLink *Link = SelectedLinks(LinkIdx);
			for (INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++)
			{
				// and draw the slot index overtop the actual slot
				FVector SlotLocation = Link->GetSlotLocation(SlotIdx);
				FVector2D DrawLocation(0,0);
				if (View->ScreenToPixel(View->WorldToScreen(SlotLocation),DrawLocation))
				{
					DrawStringCentered(Canvas,DrawLocation.X,DrawLocation.Y,*FString::Printf(TEXT("%d"),SlotIdx),GEngine->GetMediumFont(),FColor(200,200,200));
				}
			}
		}
	}
}

/*------------------------------------------------------------------------------
	FEditorModeTools.

	The master class that handles tracking of the current mode.
------------------------------------------------------------------------------*/

FEditorModeTools::FEditorModeTools()
	:	PivotShown( 0 )
	,	Snapping( 0 )
	,	CoordSystem( COORD_World )
	,	CurrentMode( NULL )
	,	WidgetMode( FWidget::WM_Translate )
	,	OverrideWidgetMode( FWidget::WM_None )
	,	bShowWidget( 1 )
	,	bMouseLock( 0 )
{}

FEditorModeTools::~FEditorModeTools()
{
	// Unregister all of our events
	GCallbackEvent->UnregisterAll(this);
}

void FEditorModeTools::Init()
{
	// Load the last used settings
	LoadConfig();

	// Editor modes
	Modes.Empty();
	Modes.AddItem( new FEdModeDefault() );
	Modes.AddItem( new FEdModeGeometry() );
	Modes.AddItem( new FEdModeTerrainEditing() );
	Modes.AddItem( new FEdModeTexture() );
	Modes.AddItem( new FEdModeCoverEdit() );

	// Register our callback for actor selection changes
	GCallbackEvent->Register(CALLBACK_SelectObject,this);
	GCallbackEvent->Register(CALLBACK_SelChange,this);
}

/**
 * Loads the state that was saved in the INI file
 */
void FEditorModeTools::LoadConfig(void)
{
	GConfig->GetBool(TEXT("FEditorModeTools"),TEXT("ShowWidget"),bShowWidget,
		GEditorUserSettingsIni);
	GConfig->GetBool(TEXT("FEditorModeTools"),TEXT("MouseLock"),bMouseLock,
		GEditorUserSettingsIni);
	INT Bogus = (INT)CoordSystem;
	GConfig->GetInt(TEXT("FEditorModeTools"),TEXT("CoordSystem"),Bogus,
		GEditorUserSettingsIni);
	CoordSystem = (ECoordSystem)Bogus;
}

/**
 * Saves the current state to the INI file
 */
void FEditorModeTools::SaveConfig(void)
{
	GConfig->SetBool(TEXT("FEditorModeTools"),TEXT("ShowWidget"),bShowWidget,
		GEditorUserSettingsIni);
	GConfig->SetBool(TEXT("FEditorModeTools"),TEXT("MouseLock"),bMouseLock,
		GEditorUserSettingsIni);
	GConfig->SetInt(TEXT("FEditorModeTools"),TEXT("CoordSystem"),(INT)CoordSystem,
		GEditorUserSettingsIni);
}

/**
 * Handles notification of an object selection change. Updates the
 * Pivot and Snapped location values based upon the selected actor
 *
 * @param InType the event that was fired
 * @param InObject the object associated with this event
 */
void FEditorModeTools::Send(ECallbackEventType InType,UObject* InObject)
{
	if (InType == CALLBACK_SelectObject || InType == CALLBACK_SelChange)
	{
		// If selecting an actor, move the pivot location.
		AActor* Actor = Cast<AActor>(InObject);
		if ( Actor != NULL)
		{
			//@fixme - why isn't this using UObject::IsSelected()?
			if ( GEditor->GetSelectedActors()->IsSelected( Actor ) )
			{
				PivotLocation = SnappedLocation = Actor->Location;
			}
		}

		// Don't do auto-switch-to-cover-editing when in Matinee, as it will exit Matinee in a bad way.
		if( GetCurrentModeID() != EM_InterpEdit )
		{
			// if already editing cover
			if ( GetCurrentModeID() == EM_CoverEdit )
			{
				// determine if we have cover links selected
				UBOOL bHasCoverLinkSelected = FALSE;
				for ( FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It )
				{
					ACoverLink *Link = Cast<ACoverLink>(*It);
					if (Link != NULL && Link->IsSelected())
					{
						bHasCoverLinkSelected = TRUE;
						break;
					}
					ACoverGroup *Group = Cast<ACoverGroup>(*It);
					if( Group != NULL && Group->IsSelected() )
					{
						bHasCoverLinkSelected = TRUE;
						break;
					}
				}
				// if no links selected
				if ( !bHasCoverLinkSelected )
				{
					// clear out cover edit mode
					SetCurrentMode(EM_Default);
				}
			}
			else
				// otherwise if selecting a coverlink
				if ( Actor != NULL && 
					 Actor->IsSelected() && 
					(Actor->IsA(ACoverLink::StaticClass()) ||
					 Actor->IsA(ACoverGroup::StaticClass())) )
				{
					// switch to cover edit
					SetCurrentMode(EM_CoverEdit);
				}
		}
	}
}

void FEditorModeTools::SetCurrentMode( EEditorMode InID )
{
	// Don't set the mode if it isn't really changing.

	if( GetCurrentMode() && GetCurrentModeID() == InID )
	{
		return;
	}

	if( GetCurrentMode() )
	{
		GetCurrentMode()->Exit();
	}

	CurrentMode = FindMode( InID );

	if( !CurrentMode )
	{
		//debugf( TEXT("FEditorModeTools::SetCurrentMode : Couldn't find mode %d.  Using default."), (INT)InID );
		CurrentMode = Modes(0);
	}

	GetCurrentMode()->Enter();

	GCallbackEvent->Send( CALLBACK_UpdateUI );
}

/**
 * Returns TRUE if the current mode is not the specified ModeID.  Also optionally warns the user.
 *
 * @param	ModeID			The editor mode to query.
 * @param	bNotifyUser		If TRUE, inform the user that the requested operation cannot be performed in the specified mode.
 * @return					TRUE if the current mode is not the specified mode.
 */
UBOOL FEditorModeTools::EnsureNotInMode(EEditorMode ModeID, UBOOL bNotifyUser) const
{
	// We're in a 'safe' mode if we're not in the specified mode.
	const UBOOL bInASafeMode = GetCurrentModeID() != ModeID;
	if( !bInASafeMode && bNotifyUser )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("EnsureNotInMode") );
	}
	return bInASafeMode;
}

FEdMode* FEditorModeTools::FindMode( EEditorMode InID )
{
	for( INT x = 0 ; x < Modes.Num() ; x++ )
	{
		if( Modes(x)->ID == InID )
		{
			return Modes(x);
		}
	}

	return NULL;
}

/**
 * Returns a coordinate system that should be applied on top of the worldspace system.
 */

FMatrix FEditorModeTools::GetCustomDrawingCoordinateSystem()
{
	FMatrix matrix = FMatrix::Identity;

	switch( CoordSystem )
	{
		case COORD_Local:
		{
			// Let the current mode have a shot at setting the local coordinate system.
			// If it doesn't want to, create it by looking at the currently selected actors list.

			if( !GetCurrentMode()->GetCustomDrawingCoordinateSystem( matrix, NULL ) )
			{
				if(GetCurrentModeID() == EM_Texture)
				{
					// Texture mode is ALWAYS in local space, so do not switch the coordinate system in that case.
					CoordSystem = COORD_Local;
				}
				else
				{
					const INT Num = GEditor->GetSelectedActors()->CountSelections<AActor>();

					// Coordinate system needs to come from the last actor selected
					if( Num > 0 )
					{
						matrix = FRotationMatrix( GEditor->GetSelectedActors()->GetBottom<AActor>()->Rotation );
					}
				}
			}
		}
		break;

		case COORD_World:
			break;

		default:
			break;
	}

	matrix.RemoveScaling();

	return matrix;
}

FMatrix FEditorModeTools::GetCustomInputCoordinateSystem()
{
	FMatrix matrix = FMatrix::Identity;

	switch( CoordSystem )
	{
		case COORD_Local:
		{
			// Let the current mode have a shot at setting the local coordinate system.
			// If it doesn't want to, create it by looking at the currently selected actors list.

			if( !GetCurrentMode()->GetCustomInputCoordinateSystem( matrix, NULL ) )
			{
				const INT Num = GEditor->GetSelectedActors()->CountSelections<AActor>();

				if( Num > 0 )
				{
					// Coordinate system needs to come from the last actor selected
					matrix = FRotationMatrix( GEditor->GetSelectedActors()->GetBottom<AActor>()->Rotation );
				}
			}
		}
		break;

		case COORD_World:
			break;

		default:
			break;
	}

	matrix.RemoveScaling();

	return matrix;
}

/**
 * Returns a good location to draw the widget at.
 */

FVector FEditorModeTools::GetWidgetLocation() const
{
	//debugf(TEXT("In FEditorModeTools::GetWidgetLocation"));
	return GetCurrentMode()->GetWidgetLocation();
}

/**
 * Changes the current widget mode.
 */

void FEditorModeTools::SetWidgetMode( FWidget::EWidgetMode InWidgetMode )
{
	WidgetMode = InWidgetMode;
}

/**
 * Allows you to temporarily override the widget mode.  Call this function again
 * with FWidget::WM_None to turn off the override.
 */

void FEditorModeTools::SetWidgetModeOverride( FWidget::EWidgetMode InWidgetMode )
{
	OverrideWidgetMode = InWidgetMode;
}

/**
 * Retrieves the current widget mode, taking overrides into account.
 */

FWidget::EWidgetMode FEditorModeTools::GetWidgetMode() const
{
	if( OverrideWidgetMode != FWidget::WM_None )
	{
		return OverrideWidgetMode;
	}

	return WidgetMode;
}

/**
 * Sets a bookmark in the levelinfo file, allocating it if necessary.
 */

void FEditorModeTools::SetBookMark( INT InIndex, FEditorLevelViewportClient* InViewportClient )
{
	AWorldInfo* Info = GWorld->GetWorldInfo();

	if( Info->BookMarks[ InIndex ] == NULL )
	{
		Info->BookMarks[ InIndex ] = ConstructObject<UBookMark>( UBookMark::StaticClass(), Info );
	}

	// Use the rotation from the first perspective viewport can find.

	FRotator Rotation(0,0,0);
	if( !InViewportClient->IsOrtho() )
	{
		Rotation = InViewportClient->ViewRotation;
	}

	Info->BookMarks[ InIndex ]->Location = InViewportClient->ViewLocation;
	Info->BookMarks[ InIndex ]->Rotation = Rotation;
}

/**
 * Retrieves a bookmark from the list.
 */

void FEditorModeTools::JumpToBookMark( INT InIndex )
{
	const AWorldInfo* Info = GWorld->GetWorldInfo();

	if( Info->BookMarks[ InIndex ] == NULL )
	{
		return;
	}

	// Set all level editing cameras to this bookmark

	for( INT v = 0 ; v < GEditor->ViewportClients.Num() ; v++ )
	{
		if ( !GEditor->ViewportClients(v)->bViewportLocked )
		{
			GEditor->ViewportClients(v)->ViewLocation = Info->BookMarks[ InIndex ]->Location;
			if( !GEditor->ViewportClients(v)->IsOrtho() )
			{
				GEditor->ViewportClients(v)->ViewRotation = Info->BookMarks[ InIndex ]->Rotation;
			}
			GEditor->ViewportClients(v)->Invalidate();
		}
	}
}

/**
 * Serializes the components for all modes.
 */

void FEditorModeTools::Serialize( FArchive &Ar )
{
	for( INT x = 0 ; x < Modes.Num() ; ++x )
	{
		Modes(x)->Serialize( Ar );
		Ar << Modes(x)->Component;
	}
}

