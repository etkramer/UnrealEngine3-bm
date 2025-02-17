/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EditorPrivate.h"
#include "SurfaceIterators.h"
#include "BSPOps.h"

/*-----------------------------------------------------------------------------
	FModeTool.
-----------------------------------------------------------------------------*/

FModeTool::FModeTool():
	ID( MT_None ),
	bUseWidget( 1 ),
	Settings( NULL )
{}

FModeTool::~FModeTool()
{
	delete Settings;
}

/*-----------------------------------------------------------------------------
	FModeTool_GeometryModify.
-----------------------------------------------------------------------------*/

FModeTool_GeometryModify::FModeTool_GeometryModify()
{
	ID = MT_GeometryModify;

	Modifiers.AddItem( ConstructObject<UGeomModifier>( UGeomModifier_Edit::StaticClass() ) );
	Modifiers.AddItem( ConstructObject<UGeomModifier>( UGeomModifier_Extrude::StaticClass() ) );
	Modifiers.AddItem( ConstructObject<UGeomModifier>( UGeomModifier_Clip::StaticClass() ) );

	Modifiers.AddItem( ConstructObject<UGeomModifier>( UGeomModifier_Create::StaticClass() ) );
	Modifiers.AddItem( ConstructObject<UGeomModifier>( UGeomModifier_Delete::StaticClass() ) );
	Modifiers.AddItem( ConstructObject<UGeomModifier>( UGeomModifier_Flip::StaticClass() ) );
	Modifiers.AddItem( ConstructObject<UGeomModifier>( UGeomModifier_Split::StaticClass() ) );
	Modifiers.AddItem( ConstructObject<UGeomModifier>( UGeomModifier_Triangulate::StaticClass() ) );
	Modifiers.AddItem( ConstructObject<UGeomModifier>( UGeomModifier_Turn::StaticClass() ) );
	Modifiers.AddItem( ConstructObject<UGeomModifier>( UGeomModifier_Weld::StaticClass() ) );

	CurrentModifier = NULL;
}

void FModeTool_GeometryModify::SelectNone()
{
	FEdModeGeometry* mode = ((FEdModeGeometry*)GEditorModeTools().GetCurrentMode());
	mode->SelectNone();
}

/** @return		TRUE if something was selected/deselected, FALSE otherwise. */
UBOOL FModeTool_GeometryModify::BoxSelect( FBox& InBox, UBOOL InSelect )
{
	UBOOL bResult = FALSE;
	if( GEditorModeTools().GetCurrentModeID() == EM_Geometry )
	{
		FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
		const EGeometrySelectionType seltype = ((FGeometryToolSettings*)mode->GetSettings())->SelectionType;

		for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
		{
			FGeomObject* go = *Itor;

			switch( seltype )
			{
				case GST_Poly:
				{
					for( INT p = 0 ; p < go->PolyPool.Num() ; ++p )
					{
						FGeomPoly& gp = go->PolyPool(p);
						if( FPointBoxIntersection( go->GetActualBrush()->LocalToWorld().TransformFVector( gp.GetMid() ), InBox ) )
						{
							gp.Select( InSelect );
							bResult = TRUE;
						}
					}
				}
				break;

				case GST_Edge:
				{
					for( INT e = 0 ; e < go->EdgePool.Num() ; ++e )
					{
						FGeomEdge& ge = go->EdgePool(e);
						if( FPointBoxIntersection( go->GetActualBrush()->LocalToWorld().TransformFVector( ge.GetMid() ), InBox ) )
						{
							ge.Select( InSelect );
							bResult = TRUE;
						}
					}
				}
				break;

				case GST_Vertex:
				{
					for( INT v = 0 ; v < go->VertexPool.Num() ; ++v )
					{
						FGeomVertex& gv = go->VertexPool(v);
						if( FPointBoxIntersection( go->GetActualBrush()->LocalToWorld().TransformFVector( gv.GetMid() ), InBox ) )
						{
							gv.Select( InSelect );
							bResult = TRUE;
						}
					}
				}
				break;
			}
		}
	}
	return bResult;
}

/**
 * @return		TRUE if the delta was handled by this editor mode tool.
 */
UBOOL FModeTool_GeometryModify::InputDelta(FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale)
{
	UBOOL bResult = FALSE;
	if( InViewportClient->Widget->GetCurrentAxis() != AXIS_None )
	{
		// Geometry mode passes the input on to the current modifier.
		if( CurrentModifier )
		{
			bResult = CurrentModifier->InputDelta( InViewportClient, InViewport, InDrag, InRot, InScale );
		}
	}
	return bResult;
}

UBOOL FModeTool_GeometryModify::StartModify()
{
	// Let the modifier do any set up work that it needs to.
	if( CurrentModifier != NULL )
	{
		CurrentModifier->StartTrans();
		return CurrentModifier->StartModify();
	}

	// No modifier to start
	return FALSE;
}

UBOOL FModeTool_GeometryModify::EndModify()
{
	// Let the modifier finish up.
	if( CurrentModifier != NULL )
	{
		FEdModeGeometry* mode = ((FEdModeGeometry*)GEditorModeTools().GetCurrentMode());

		// Update the source data to match the current geometry data.
		mode->SendToSource();

		// Make sure the source data has remained viable.
		if( mode->FinalizeSourceData() )
		{
			// If the source data was modified, reconstruct the geometry data to reflect that.
			mode->GetFromSource();
		}

		CurrentModifier->EndTrans();
		CurrentModifier->EndModify();

		// Update internals.
		for( FEdModeGeometry::TGeomObjectIterator Itor( mode->GeomObjectItor() ) ; Itor ; ++Itor )
		{
			FGeomObject* go = *Itor;
			go->ComputeData();
			FBSPOps::bspUnlinkPolys( go->GetActualBrush()->Brush );
		}
	}

	return 1;
}

void FModeTool_GeometryModify::StartTrans()
{
	if( CurrentModifier != NULL )
	{
		CurrentModifier->StartTrans();
	}
}

void FModeTool_GeometryModify::EndTrans()
{
	if( CurrentModifier != NULL )
	{
		CurrentModifier->EndTrans();
	}
}

/**
 * @return		TRUE if the key was handled by this editor mode tool.
 */
UBOOL FModeTool_GeometryModify::InputKey(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event)
{
	UBOOL bResult = FALSE;

	// Give the current modifier a chance to handle this first
	if( CurrentModifier && CurrentModifier->InputKey( ViewportClient, Viewport, Key, Event ) )
	{
		bResult = TRUE;
	}
	else
	{
		bResult = FModeTool::InputKey( ViewportClient, Viewport, Key, Event );
	}

	return bResult;
}

/*-----------------------------------------------------------------------------
	FModeTool_Texture.
-----------------------------------------------------------------------------*/

FModeTool_Texture::FModeTool_Texture()
{
	ID = MT_Texture;
	bUseWidget = 1;
}

/**
 * @return		TRUE if the delta was handled by this editor mode tool.
 */
UBOOL FModeTool_Texture::InputDelta(FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale)
{
	if( InViewportClient->Widget->GetCurrentAxis() == AXIS_None )
	{
		return FALSE;
	}

	if( !InDrag.IsZero() )
	{
		// Ensure each polygon has a unique base point index.
		FOR_EACH_UMODEL;
			for(INT SurfaceIndex = 0;SurfaceIndex < Model->Surfs.Num();SurfaceIndex++)
			{
				FBspSurf& Surf = Model->Surfs(SurfaceIndex);

				if(Surf.PolyFlags & PF_Selected)
				{
					const FVector Base = Model->Points(Surf.pBase);
					Surf.pBase = Model->Points.AddItem(Base);
				}
			}
			GEditor->polyTexPan( Model, InDrag.X, InDrag.Y, 0 );
		END_FOR_EACH_UMODEL;
	}

	if( !InRot.IsZero() )
	{
		const FRotationMatrix RotationMatrix( InRot );

		// Ensure each polygon has unique texture vector indices.
		for ( TSelectedSurfaceIterator<> It ; It ; ++It )
		{
			FBspSurf* Surf = *It;
			UModel* Model = It.GetModel();

			FVector	TextureU = Model->Vectors(Surf->vTextureU);
			FVector TextureV = Model->Vectors(Surf->vTextureV);

			TextureU = RotationMatrix.TransformFVector( TextureU );
			TextureV = RotationMatrix.TransformFVector( TextureV );

			Surf->vTextureU = Model->Vectors.AddItem(TextureU);
			Surf->vTextureV = Model->Vectors.AddItem(TextureV);

			GEditor->polyUpdateMaster( Model, It.GetSurfaceIndex(), 1 );
		}
	}

	if( !InScale.IsZero() )
	{
		FLOAT ScaleU = InScale.X / GEditor->Constraints.GetGridSize();
		FLOAT ScaleV = InScale.Y / GEditor->Constraints.GetGridSize();

		ScaleU = 1.f - (ScaleU / 100.f);
		ScaleV = 1.f - (ScaleV / 100.f);

		// Ensure each polygon has unique texture vector indices.
		FOR_EACH_UMODEL;
			for(INT SurfaceIndex = 0;SurfaceIndex < Model->Surfs.Num();SurfaceIndex++)
			{
				FBspSurf& Surf = Model->Surfs(SurfaceIndex);
				if(Surf.PolyFlags & PF_Selected)
				{
					const FVector TextureU = Model->Vectors(Surf.vTextureU);
					const FVector TextureV = Model->Vectors(Surf.vTextureV);

					Surf.vTextureU = Model->Vectors.AddItem(TextureU);
					Surf.vTextureV = Model->Vectors.AddItem(TextureV);
				}
			}
			GEditor->polyTexScale( Model, ScaleU, 0.f, 0.f, ScaleV, FALSE );
		END_FOR_EACH_UMODEL;

	}

	return TRUE;
}
