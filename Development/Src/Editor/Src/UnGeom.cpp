/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EditorPrivate.h"
#include "..\..\UnrealEd\Inc\Utils.h"
#include "..\..\UnrealEd\Inc\ResourceIDs.h"
#include "..\..\UnrealEd\Inc\Properties.h"

///////////////////////////////////////////////////////////////////////////////
//
// FGeomBase
//
///////////////////////////////////////////////////////////////////////////////

FGeomBase::FGeomBase()
{
	SelectionIndex = INDEX_NONE;
	Normal = FVector(0,0,0);
	ParentObjectIndex = INDEX_NONE;
	SelStrength = 1.0f;
}

void FGeomBase::Select( UBOOL InSelect )
{
	FEditorModeTools& Tools = GEditorModeTools();
	check( Tools.GetCurrentModeID() == EM_Geometry );

	if( InSelect )
	{
		SelectionIndex = GetParentObject()->GetNewSelectionIndex();
		SelStrength = 1.f;
	}
	else
	{
		SelectionIndex = INDEX_NONE;
		SelStrength = 0.f;
	}

	// If something is selected, move the pivot and snap locations to the widget location.
	if( IsSelected() )
	{
		//debugf(TEXT("FGeomBase::Select() - calling GetWidgetLocation"));
		Tools.PivotLocation = Tools.SnappedLocation = GetWidgetLocation();
	}

	GetParentObject()->DirtySelectionOrder();

	// Apply soft selection.
	FEdModeGeometry* mode = (FEdModeGeometry*)Tools.GetCurrentMode();
	mode->SoftSelect();
}

FGeomObject* FGeomBase::GetParentObject()
{
	check( GEditorModeTools().GetCurrentModeID() == EM_Geometry );
	check( ParentObjectIndex > INDEX_NONE );

	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	return mode->GetGeomObject( ParentObjectIndex );
}

const FGeomObject* FGeomBase::GetParentObject() const
{
	check( GEditorModeTools().GetCurrentModeID() == EM_Geometry );
	check( ParentObjectIndex > INDEX_NONE );

	const FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	return mode->GetGeomObject( ParentObjectIndex );
}

///////////////////////////////////////////////////////////////////////////////
//
// FGeomVertex
//
///////////////////////////////////////////////////////////////////////////////

FGeomVertex::FGeomVertex()
{
	X = Y = Z = 0;
	ParentObjectIndex = INDEX_NONE;
}

FVector FGeomVertex::GetWidgetLocation()
{
	//debugf(TEXT("FGeomVertex::GetWidgetLocation()"));
	return GetParentObject()->GetActualBrush()->LocalToWorld().TransformFVector( *this );
}

FVector FGeomVertex::GetMidPoint()
{
	return *this;
}

FVector* FGeomVertex::GetActualVertex( FPolyVertexIndex& InPVI )
{
	return &(GetParentObject()->GetActualBrush()->Brush->Polys->Element( InPVI.PolyIndex ).Vertices( InPVI.VertexIndex ));
}

///////////////////////////////////////////////////////////////////////////////
//
// FGeomEdge
//
///////////////////////////////////////////////////////////////////////////////

FGeomEdge::FGeomEdge()
{
	ParentObjectIndex = INDEX_NONE;
	VertexIndices[0] = INDEX_NONE;
	VertexIndices[1] = INDEX_NONE;
}

// Returns true if InEdge matches this edge, independant of winding.
UBOOL FGeomEdge::IsSameEdge( const FGeomEdge& InEdge ) const
{
	return (( VertexIndices[0] == InEdge.VertexIndices[0] && VertexIndices[1] == InEdge.VertexIndices[1] ) ||
			( VertexIndices[0] == InEdge.VertexIndices[1] && VertexIndices[1] == InEdge.VertexIndices[0] ));
}

FVector FGeomEdge::GetWidgetLocation()
{
	//debugf(TEXT("FGeomEdge::GetWidgetLocation()"));
	FVector dir = (GetParentObject()->VertexPool( VertexIndices[1] ) - GetParentObject()->VertexPool( VertexIndices[0] ));
	const FLOAT dist = dir.Size() / 2;
	dir.Normalize();
	const FVector loc = GetParentObject()->VertexPool( VertexIndices[0] ) + (dir * dist);
	return GetParentObject()->GetActualBrush()->LocalToWorld().TransformFVector( loc );
}

FVector FGeomEdge::GetMidPoint()
{
	FGeomVertex* wk0 = &(GetParentObject()->VertexPool( VertexIndices[0] ));
	FGeomVertex* wk1 = &(GetParentObject()->VertexPool( VertexIndices[1] ));

	const FVector v0( wk0->X, wk0->Y, wk0->Z );
	const FVector v1( wk1->X, wk1->Y, wk1->Z );

	return (v0 + v1) / 2;
}

///////////////////////////////////////////////////////////////////////////////
//
// FGeomPoly
//
///////////////////////////////////////////////////////////////////////////////

FGeomPoly::FGeomPoly()
{
	ParentObjectIndex = INDEX_NONE;
}

FVector FGeomPoly::GetWidgetLocation()
{
	//debugf(TEXT("FGeomPoly::GetWidgetLocation()"));
	const FPoly* Poly = GetActualPoly();
	FVector Wk(0.f,0.f,0.f);
	if ( Poly->Vertices.Num() > 0 )
	{
		for( INT VertIndex = 0 ; VertIndex < Poly->Vertices.Num() ; ++VertIndex )
		{
			Wk += Poly->Vertices(VertIndex);
		}
		Wk = Wk / Poly->Vertices.Num();
	}

	return GetParentObject()->GetActualBrush()->LocalToWorld().TransformFVector( Wk );
}

FVector FGeomPoly::GetMidPoint()
{
	FVector Wk(0,0,0);
	INT Count = 0;

	for( INT e = 0 ; e < EdgeIndices.Num() ; ++e )
	{
		const FGeomEdge* ge = &GetParentObject()->EdgePool( EdgeIndices(e) );
		Wk += GetParentObject()->VertexPool( ge->VertexIndices[0] );
		Count++;
		Wk += GetParentObject()->VertexPool( ge->VertexIndices[1] );
		Count++;
	}

	check( Count );
	return Wk / Count;
}

FPoly* FGeomPoly::GetActualPoly()
{
	return &( GetParentObject()->GetActualBrush()->Brush->Polys->Element(ActualPolyIndex) );
}

///////////////////////////////////////////////////////////////////////////////
//
// FGeomObject
//
///////////////////////////////////////////////////////////////////////////////

FGeomObject::FGeomObject():
	LastSelectionIndex( INDEX_NONE )
{
	DirtySelectionOrder();
	ActualBrush = NULL;
}

FVector FGeomObject::GetWidgetLocation()
{
	//debugf(TEXT("FGeomObject::GetWidgetLocation()"));
	return GetActualBrush()->Location;
}

INT FGeomObject::AddVertexToPool( INT InObjectIndex, INT InParentPolyIndex, INT InPolyIndex, INT InVertexIndex )
{
	FGeomVertex* gv = NULL;
	FVector CmpVtx = GetActualBrush()->Brush->Polys->Element( InPolyIndex ).Vertices(InVertexIndex);

	// See if the vertex is already in the pool
	for( INT x = 0 ; x < VertexPool.Num() ; ++x )
	{
		if( FPointsAreNear( VertexPool(x), CmpVtx, THRESH_POINTS_ARE_NEAR ) )
		{
			gv = &VertexPool(x);
			gv->ActualVertexIndices.AddUniqueItem( FPolyVertexIndex( InPolyIndex, InVertexIndex ) );
			gv->ParentPolyIndices.AddUniqueItem( InParentPolyIndex );
			return x;
		}
	}

	// If not, add it...
	if( gv == NULL )
	{
		new( VertexPool )FGeomVertex();
		gv = &VertexPool( VertexPool.Num()-1 );
		*gv = CmpVtx;
	}

	gv->ActualVertexIndices.AddUniqueItem( FPolyVertexIndex( InPolyIndex, InVertexIndex ) );
	gv->SetParentObjectIndex( InObjectIndex );
	gv->ParentPolyIndices.AddUniqueItem( InParentPolyIndex );

	return VertexPool.Num()-1;
}

INT FGeomObject::AddEdgeToPool( FGeomPoly* InPoly, INT InParentPolyIndex, INT InVectorIdxA, INT InVectorIdxB )
{
	const INT idx0 = AddVertexToPool( InPoly->GetParentObjectIndex(), InParentPolyIndex, InPoly->ActualPolyIndex, InVectorIdxA );
	const INT idx1 = AddVertexToPool( InPoly->GetParentObjectIndex(), InParentPolyIndex, InPoly->ActualPolyIndex, InVectorIdxB );

	// See if the edge is already in the pool.  If so, leave.
	FGeomEdge* ge = NULL;
	for( INT e = 0 ; e < EdgePool.Num() ; ++e )
	{
		if( EdgePool(e).VertexIndices[0] == idx0 && EdgePool(e).VertexIndices[1] == idx1 )
		{
			EdgePool(e).ParentPolyIndices.AddItem( InPoly->GetParentObject()->PolyPool.FindItemIndex( *InPoly ) );
			return e;
		}
	}

	// Add a new edge to the pool and set it up.
	new( EdgePool )FGeomEdge();
	ge = &EdgePool( EdgePool.Num()-1 );

	ge->VertexIndices[0] = idx0;
	ge->VertexIndices[1] = idx1;

	ge->ParentPolyIndices.AddItem( InPoly->GetParentObject()->PolyPool.FindItemIndex( *InPoly ) );
	ge->SetParentObjectIndex( InPoly->GetParentObjectIndex() );

	return EdgePool.Num()-1;
}

/**
 * Removes all geometry data and reconstructs it from the source brushes.
 */

void FGeomObject::GetFromSource()
{
	PolyPool.Empty();
	EdgePool.Empty();
	VertexPool.Empty();

	for( INT p = 0 ; p < GetActualBrush()->Brush->Polys->Element.Num() ; ++p )
	{
		FPoly* poly = &(GetActualBrush()->Brush->Polys->Element(p));

		new( PolyPool )FGeomPoly();
		FGeomPoly* gp = &PolyPool( PolyPool.Num()-1 );
		gp->SetParentObjectIndex( GetObjectIndex() );
		gp->ActualPolyIndex = p;

		for( int v = 1 ; v <= poly->Vertices.Num() ; ++v )
		{
			int idx = (v == poly->Vertices.Num()) ? 0 : v,
				previdx = v-1;

			INT eidx = AddEdgeToPool( gp, PolyPool.Num()-1, previdx, idx );
			gp->EdgeIndices.AddItem( eidx );
		}
	}

	ComputeData();
}

INT FGeomObject::GetObjectIndex()
{
	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return INDEX_NONE;
	}

	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();

	for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
	{
		if( *Itor == this )
		{
			return Itor.GetIndex();
		}
	}

	check(0);	// Should never happen
	return INDEX_NONE;
}

/**
 * Sends the vertex data that we have back to the source vertices.
 */

void FGeomObject::SendToSource()
{
	for( INT v = 0 ; v < VertexPool.Num() ; ++v )
	{
		FGeomVertex* gv = &VertexPool(v);

		for( INT x = 0 ; x < gv->ActualVertexIndices.Num() ; ++x )
		{
			FVector* vtx = gv->GetActualVertex( gv->ActualVertexIndices(x) );
			vtx->X = gv->X;
			vtx->Y = gv->Y;
			vtx->Z = gv->Z;
		}
	}
}

/**
 * Finalizes the source geometry by checking for invalid polygons,
 * updating components, etc. - anything that needs to be done
 * before the engine will accept the resulting brushes/polygons
 * as valid.
 */

UBOOL FGeomObject::FinalizeSourceData()
{
	if( GEditorModeTools().GetCurrentModeID() != EM_Geometry )
	{
		return 0;
	}

	ABrush* brush = GetActualBrush();
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	UBOOL Ret = 0;

	// Remove invalid polygons from the brush

	for( INT x = 0 ; x < brush->Brush->Polys->Element.Num() ; ++x )
	{
		FPoly* Poly = &brush->Brush->Polys->Element(x);

		if( Poly->Vertices.Num() < 3 )
		{
			brush->Brush->Polys->Element.Remove( x );
			x = -1;
		}
	}

	for( INT p = 0 ; p < brush->Brush->Polys->Element.Num() ; ++p )
	{
		FPoly* Poly = &(brush->Brush->Polys->Element(p));
		Poly->iLink = p;
		INT SaveNumVertices = Poly->Vertices.Num();

		if( !Poly->IsCoplanar() )
		{
			// If the polygon is no longer coplanar, break it up into separate triangles.

			FPoly WkPoly = *Poly;
			brush->Brush->Polys->Element.Remove( p );
			if( WkPoly.Triangulate( brush ) > 0 )
			{
				p = -1;
				Ret = 1;
			}
		}
		else
		{
			INT FixResult = Poly->Fix();
			if( FixResult != SaveNumVertices )
			{
				// If the polygon collapses after running "Fix" against it, it needs to be
				// removed from the brushes polygon list.

				if( FixResult == 0 )
				{
					brush->Brush->Polys->Element.Remove( p );
				}

				p = -1;
				Ret = 1;
			}
			else
			{
				// If we get here, the polygon is valid and needs to be kept.  Finalize its internals.

				Poly->Finalize(brush,1);
			}
		}
	}

	brush->ClearComponents();
	brush->ConditionalUpdateComponents();

	return Ret;
}

/**
 * Recomputes data specific to the geometry data (i.e. normals, mid points, etc)
 */

void FGeomObject::ComputeData()
{
	INT Count;
	FVector Wk;

	// Polygons

	FGeomPoly* poly;
	for( INT p = 0 ; p < PolyPool.Num() ; ++p )
	{
		poly = &PolyPool(p);

		poly->SetNormal( poly->GetActualPoly()->Normal );
		poly->SetMid( poly->GetMidPoint() );

	}

	// Vertices (= average normal of all the polygons that touch it)

	FGeomVertex* gv;
	for( INT v = 0 ; v < VertexPool.Num() ; ++v )
	{
		gv = &VertexPool(v);
		Count = 0;
		Wk = FVector(0,0,0);

		for( INT e = 0 ; e < EdgePool.Num() ; ++e )
		{
			FGeomEdge* ge = &EdgePool(e);

			FGeomVertex *v0 = &VertexPool( ge->VertexIndices[0] ),
				*v1 = &VertexPool( ge->VertexIndices[1] );

			if( gv == v0 || gv == v1 )
			{
				for( INT p = 0 ; p < ge->ParentPolyIndices.Num() ; ++ p )
				{
					FGeomPoly* poly = &PolyPool( ge->ParentPolyIndices(p) );

					Wk += poly->GetNormal();
					Count++;
				}
			}
		}

		gv->SetNormal( Wk / Count );
		gv->SetMid( gv->GetMidPoint() );
	}

	// Edges (= average normal of all the polygons that touch it)

	FGeomEdge* ge;
	for( INT e = 0 ; e < EdgePool.Num() ; ++e )
	{
		ge = &EdgePool(e);
		Count = 0;
		Wk = FVector(0,0,0);

		for( INT e2 = 0 ; e2 < EdgePool.Num() ; ++e2 )
		{
			FGeomEdge* ge2 = &EdgePool(e2);

			if( ge->IsSameEdge( *ge2 ) )
			{
				for( INT p = 0 ; p < ge2->ParentPolyIndices.Num(); ++p )
				{
					FGeomPoly* gp = &PolyPool( ge2->ParentPolyIndices(p) );

					Wk += gp->GetActualPoly()->Normal;
					Count++;

				}
			}
		}

		ge->SetNormal( Wk / Count );
		ge->SetMid( ge->GetMidPoint() );
	}
}

void FGeomObject::ClearData()
{
	EdgePool.Empty();
	VertexPool.Empty();
}

FVector FGeomObject::GetMidPoint()
{
	return GetActualBrush()->Location;
}

INT FGeomObject::GetNewSelectionIndex()
{
	LastSelectionIndex++;
	return LastSelectionIndex;
}

void FGeomObject::SelectNone()
{
	Select( 0 );

	for( int i = 0 ; i < EdgePool.Num() ; ++i )
	{
		EdgePool(i).Select( 0 );
	}
	for( int i = 0 ; i < PolyPool.Num() ; ++i )
	{
		PolyPool(i).Select( 0 );
	}
	for( int i = 0 ; i < VertexPool.Num() ; ++i )
	{
		VertexPool(i).Select( 0 );
	}

	LastSelectionIndex = INDEX_NONE;
}

/**
 * Compiles the selection order array by putting every geometry object
 * with a valid selection index into the array, and then sorting it.
 */

static int CDECL SelectionIndexCompare(const void *InA, const void *InB)
{
	const INT A = (*(FGeomBase**)InA)->GetSelectionIndex();
	const INT B = (*(FGeomBase**)InB)->GetSelectionIndex();
	return A - B;
}

void FGeomObject::CompileSelectionOrder()
{
	// Only compile the array if it's dirty.

	if( !bSelectionOrderDirty )
	{
		return;
	}

	SelectionOrder.Empty();

	for( int i = 0 ; i < EdgePool.Num() ; ++i )
	{
		FGeomEdge* ge = &EdgePool(i);
		if( ge->GetSelectionIndex() > INDEX_NONE )
		{
			SelectionOrder.AddItem( ge );
		}
	}
	for( int i = 0 ; i < PolyPool.Num() ; ++i )
	{
		FGeomPoly* gp = &PolyPool(i);
		if( gp->GetSelectionIndex() > INDEX_NONE )
		{
			SelectionOrder.AddItem( gp );
		}
	}
	for( int i = 0 ; i < VertexPool.Num() ; ++i )
	{
		FGeomVertex* gv = &VertexPool(i);
		if( gv->GetSelectionIndex() > INDEX_NONE )
		{
			SelectionOrder.AddItem( gv );
		}
	}

	if( SelectionOrder.Num() )
	{
		appQsort( &SelectionOrder(0), SelectionOrder.Num(), sizeof(FGeomBase*), (QSORT_COMPARE)SelectionIndexCompare );
	}

	bSelectionOrderDirty = 0;
}

/**
 * Compiles a list of unique edges.  This runs through the edge pool
 * and only adds edges into the pool that aren't already there (the
 * difference being that this routine counts edges that share the same
 * vertices, but are wound backwards to each other, as being equal).
 *
 * @param	InEdges	The edge array to fill up
 */

void FGeomObject::CompileUniqueEdgeArray( TArray<FGeomEdge>* InEdges )
{
	InEdges->Empty();

	// Fill the array with any selected edges

	for( INT e = 0 ; e < EdgePool.Num() ; ++e )
	{
		FGeomEdge* ge = &EdgePool(e);

		if( ge->IsSelected() )
		{
			InEdges->AddItem( *ge );
		}
	}

	// Gather up any other edges that share the same position

	for( INT e = 0 ; e < EdgePool.Num() ; ++e )
	{
		FGeomEdge* ge = &EdgePool(e);

		// See if this edge is in the array already.  If so, add its parent
		// polygon indices into the transient edge arrays list.

		for( int x = 0 ; x < InEdges->Num() ; ++x )
		{
			FGeomEdge* geWk = &((*InEdges)(x));

			if( geWk->IsSameEdge( *ge ) )
			{
				// The parent polygon indices of both edges must be combined so that the resulting
				// item in the edge array will point to the complete list of polygons that share that edge.

				for( INT p = 0 ; p < ge->ParentPolyIndices.Num() ; ++p )
				{
					geWk->ParentPolyIndices.AddUniqueItem( ge->ParentPolyIndices(p) );
				}

				break;
			}
		}
	}
}

/**
 * Soft selects based on current selections.
 */

void FGeomObject::SoftSelect()
{
	check( GEditorModeTools().GetCurrentModeID() == EM_Geometry );

	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	FGeometryToolSettings* Settings = (FGeometryToolSettings*)mode->GetSettings();
	EGeometrySelectionType seltype = Settings->SelectionType;

	if( !Settings->bUseSoftSelection )
	{
		return;
	}

	switch( seltype )
	{
		case GST_Vertex:
		{
			TArray<FGeomVertex*> FullStrengthVerts;

			// Figure out which vertices are fully selected (1.0 strength) and zero out the others.

			for( INT v = 0 ; v < VertexPool.Num() ; ++v )
			{
				FGeomVertex* gv = &VertexPool(v);

				if( gv->IsFullySelected() )
				{
					FullStrengthVerts.AddItem( gv );
				}
				else
				{
					gv->SetSelStrength( 0.f );
					gv->ForceSelectionIndex( INDEX_NONE );
				}
			}

			// Set the selection strengths for any vertex that is is close enough to a full strength vertex

			for( INT v = 0 ; v < VertexPool.Num() ; ++v )
			{
				FGeomVertex* gv = &VertexPool(v);

				if( !gv->IsSelected() )
				{
					// Find the closest selected vertex to this one and set it's selection
					// strength based on the distance between them.

					FGeomVertex* Closest = NULL;
					FLOAT ClosestDist = WORLD_MAX;

					for( INT x = 0 ; x < FullStrengthVerts.Num() ; ++x )
					{
						FLOAT Dist = FDist( *FullStrengthVerts(x), *gv );
						if( Dist < ClosestDist )
						{
							ClosestDist = Dist;
							Closest = FullStrengthVerts(x);
						}
					}

					if( Closest )
					{
						gv->SetSelStrength( ClosestDist / (FLOAT)Settings->SoftSelectionRadius );
						gv->SetSelStrength( 1.0f - Clamp<FLOAT>( gv->GetSelStrength(), 0.f, 1.f ) );

						if( gv->GetSelStrength() > 0.f )
						{
							gv->ForceSelectionIndex( 999 );
						}
					}
				}
			}
		}
		break;

		case GST_Object:
		case GST_Poly:
		case GST_Edge:
		{
			for( INT v = 0 ; v < VertexPool.Num() ; ++v )
			{
				FGeomVertex* gv = &VertexPool(v);

				gv->SetSelStrength( 1.f );
			}
		}
		break;
	}
}

void FGeomObject::Serialize(FArchive& Ar)
{
	if( !Ar.IsLoading() && !Ar.IsSaving() )
	{
		Ar << ActualBrush;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// WxGeomModifiers
//
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_CLASS(WxGeomModifiers,wxFrame);

BEGIN_EVENT_TABLE( WxGeomModifiers, wxFrame )
	EVT_SIZE( WxGeomModifiers::OnSize )
	EVT_CLOSE( WxGeomModifiers::OnClose )
	EVT_COMMAND_RANGE( ID_GEOMMODIFIER_START, ID_GEOMMODIFIER_END, wxEVT_COMMAND_RADIOBUTTON_SELECTED, WxGeomModifiers::OnModifierSelected )
	EVT_COMMAND_RANGE( ID_GEOMMODIFIER_START, ID_GEOMMODIFIER_END, wxEVT_COMMAND_BUTTON_CLICKED , WxGeomModifiers::OnModifierClicked )
	EVT_UPDATE_UI_RANGE( ID_GEOMMODIFIER_START, ID_GEOMMODIFIER_END, WxGeomModifiers::OnUpdateUI )
	EVT_BUTTON( ID_GEOMMODIFIER_ACTION, WxGeomModifiers::OnActionClicked )
END_EVENT_TABLE()

WxGeomModifiers::WxGeomModifiers( wxWindow* Parent, wxWindowID id )
	: wxFrame( Parent, id, *LocalizeUnrealEd(TEXT("GeometryModifiers")), wxDefaultPosition, wxDefaultSize, (Parent ? wxFRAME_FLOAT_ON_PARENT : wxDIALOG_NO_PARENT ) | wxDEFAULT_FRAME_STYLE | wxFRAME_TOOL_WINDOW | wxFRAME_NO_TASKBAR )
	,	ModifierBox(NULL)
	,	PropertyWindow(NULL)
{
	FWindowUtil::LoadPosSize( TEXT("FloatingGeomModifiers"), this, 256,64,300,350 );
	SetBackgroundColour( wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE) );
	ModifierBox = new wxStaticBox( this, -1, TEXT("Modifiers") );
	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create( this, NULL );
	ActionButton = new wxButton( this, ID_GEOMMODIFIER_ACTION, TEXT("Apply") );
}

WxGeomModifiers::~WxGeomModifiers()
{
	FWindowUtil::SavePosSize( TEXT("FloatingGeomModifiers"), this );
}

void WxGeomModifiers::OnClose( wxCloseEvent& In )
{
	GCallbackEvent->Send( CALLBACK_ChangeEditorMode, EM_Default );
	In.Veto();
}

void WxGeomModifiers::OnSize( wxSizeEvent& In )
{
	PositionChildControls();
}

/**
 * Initializes the controls inside the window based on the
 * current tool in geometry mode.
 */
void WxGeomModifiers::InitFromTool()
{
	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	if( !mode ) return;		// can be null while the mode is constructing
	FModeTool* tool = mode->GetCurrentTool();

	// Clean up old buttons

	for( INT x = 0 ; x < RadioButtons.Num() ; ++x )
	{
		RadioButtons(x)->Destroy();
	}
	for( INT x = 0 ; x < PushButtons.Num() ; ++x )
	{
		PushButtons(x)->Destroy();
	}

	// Create new modifier buttons

	switch( tool->GetID() )
	{
		case MT_GeometryModify:
		{
			FModeTool_GeometryModify* mtgm = (FModeTool_GeometryModify*)tool;
			INT id = ID_GEOMMODIFIER_START;
			for( FModeTool_GeometryModify::TModifierConstIterator Itor( mtgm->ModifierConstIterator() ) ; Itor ; ++Itor )
			{
				wxButton* button;
				const UGeomModifier* modifier = *Itor;
				if( modifier->bPushButton )
				{
					button = (wxButton*)(new wxButton( this, id, *modifier->GetModifierDescription() ));
					PushButtons.AddItem( button );
				}
				else
				{
					button = (wxButton*)(new wxRadioButton( this, id, *modifier->GetModifierDescription() ));
					RadioButtons.AddItem( (wxRadioButton*)button );
				}

				id++;
				button->Show();
			}
			PositionChildControls();

			// The first button/modifier is assumed to be "Edit"

			RadioButtons(0)->SetValue( 1 );
			FModeTool_GeometryModify::TModifierIterator Itor2( mtgm->ModifierIterator() );
			PropertyWindow->SetObject( *Itor2, 1, 1, 1 );
			mtgm->SetCurrentModifier( *Itor2 );
		}
		break;
	}
}

void WxGeomModifiers::OnUpdateUI( wxUpdateUIEvent& In )
{
	if ( GEditorModeTools().GetCurrentModeID() == EM_Geometry )
	{
		FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
		FModeTool_GeometryModify* tool = (FModeTool_GeometryModify*)mode->GetCurrentTool();
		const INT idx = In.GetId() - ID_GEOMMODIFIER_START;
		UGeomModifier* modifier = tool->GetModifier(idx);

		// Enable/Disable modifier buttons based on the modifier supporting the current selection type.
		if( modifier->Supports( ((FGeometryToolSettings*)mode->GetSettings())->SelectionType ) )
		{
			In.Enable( 1 );
		}
		else
		{
			In.Enable( 0 );

			// If this selection type doesn't support this modifier, and this modifier is currently
			// selected, select the first modifier in the list instead.  The first modifier is always
			// "Edit" and it supports everything by design.
			if( tool->GetCurrentModifier() == modifier )
			{
				FModeTool_GeometryModify::TModifierIterator Itor( tool->ModifierIterator() );
				tool->SetCurrentModifier( *Itor );
				PropertyWindow->SetObject( *Itor, TRUE, TRUE, TRUE );

				In.Check( tool->GetCurrentModifier() == modifier );
			}
		}

	}
}

/**
 * Figures out proper locations for all child controls.
 */
void WxGeomModifiers::PositionChildControls()
{
	if( !ModifierBox )
	{
		return;
	}

	wxRect rc = GetClientRect();
	INT Padding = 4;
	INT ButtonHeight = 24;
	INT Top = 4;

	// Modifier box

	ModifierBox->SetSize( Padding/2,Padding/2, rc.GetWidth()-Padding,(ButtonHeight*((RadioButtons.Num()+1)/2))+ButtonHeight+(Padding*4) );
	wxRect rcBox = ModifierBox->GetClientRect();
	INT ButtonWidth = (rcBox.GetWidth()-(Padding*3)) / 2;

	Top += (Padding*2);

	// Radio buttons

	for( INT x = 0 ; x < RadioButtons.Num() ; ++x )
	{
		INT Column = x % 2;
		INT XPos = Padding + (Column*(ButtonWidth+Padding));

		RadioButtons(x)->SetSize( XPos, Top, ButtonWidth, ButtonHeight );

		if( Column )
		{
			Top += ButtonHeight;
		}
	}

	Top += Padding/2;

	if( RadioButtons.Num() % 2 )
	{
		Top += ButtonHeight;
	}

	// Action Button

	ActionButton->SetSize( (rc.GetWidth()-ButtonWidth)/2, Top, ButtonWidth, ButtonHeight );

	Top += ButtonHeight + (Padding*2);

	// Property window

	INT Height = rc.GetHeight() - Top - (Padding*2) - (ButtonHeight * ((PushButtons.Num()+1)/2));
	PropertyWindow->SetSize( rcBox.GetX(), Top, rcBox.GetWidth(), Height );

	Top += Height + Padding;

	// Push buttons

	for( INT x = 0 ; x < PushButtons.Num() ; ++x )
	{
		INT Column = x % 2;
		INT XPos = Padding + (Column*(ButtonWidth+Padding));

		PushButtons(x)->SetSize( XPos, Top, ButtonWidth, ButtonHeight );

		if( Column )
		{
			Top += ButtonHeight;
		}
	}
}

void WxGeomModifiers::OnModifierSelected( wxCommandEvent& In )
{
	check( GEditorModeTools().GetCurrentModeID() == EM_Geometry );

	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	FModeTool_GeometryModify* tool = (FModeTool_GeometryModify*)mode->GetCurrentTool();

	const INT idx = In.GetId() - ID_GEOMMODIFIER_START;

	PropertyWindow->SetObject( tool->GetModifier(idx), 1, 1, 1 );
	tool->SetCurrentModifier( tool->GetModifier(idx) );
}

void WxGeomModifiers::OnModifierClicked( wxCommandEvent& In )
{
	check( GEditorModeTools().GetCurrentModeID() == EM_Geometry );

	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	FModeTool_GeometryModify* tool = (FModeTool_GeometryModify*)mode->GetCurrentTool();

	const INT idx = In.GetId() - ID_GEOMMODIFIER_START;

	tool->GetModifier(idx)->Apply();
}

/**
 * Takes any input the user has made to the Keyboard section of the current modifier and causes the modifier activate.
 */
void WxGeomModifiers::OnActionClicked( wxCommandEvent& In )
{
	check( GEditorModeTools().GetCurrentModeID() == EM_Geometry );

	FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
	FModeTool_GeometryModify* tool = (FModeTool_GeometryModify*)mode->GetCurrentTool();

	tool->GetCurrentModifier()->Apply();
}

void WxGeomModifiers::FinalizePropertyWindowValues()
{
	PropertyWindow->FinalizeValues();
}
