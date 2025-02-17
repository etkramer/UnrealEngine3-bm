/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "EditorPrivate.h"

static const INT AXIS_ARROW_RADIUS		= 8;
static const FLOAT AXIS_CIRCLE_RADIUS	= 48.0f;
static const INT AXIS_CIRCLE_SIDES		= 24;

FWidget::FWidget()
{
	// Compute and store sample vertices for drawing the axis arrow heads

	AxisColorX = FColor(255,0,0);
	AxisColorY = FColor(0,255,0);
	AxisColorZ = FColor(0,0,255);
	CurrentColor = FColor(255,255,0);

	AxisMaterialX = (UMaterial*)UObject::StaticLoadObject( UMaterial::StaticClass(),NULL,TEXT("EditorMaterials.WidgetMaterial_X"),NULL,LOAD_None,NULL );
	AxisMaterialY = (UMaterial*)UObject::StaticLoadObject( UMaterial::StaticClass(),NULL,TEXT("EditorMaterials.WidgetMaterial_Y"),NULL,LOAD_None,NULL );
	AxisMaterialZ = (UMaterial*)UObject::StaticLoadObject( UMaterial::StaticClass(),NULL,TEXT("EditorMaterials.WidgetMaterial_Z"),NULL,LOAD_None,NULL );
	CurrentMaterial = (UMaterial*)UObject::StaticLoadObject( UMaterial::StaticClass(),NULL,TEXT("EditorMaterials.WidgetMaterial_Current"),NULL,LOAD_None,NULL );

	CurrentAxis = AXIS_None;

	CustomCoordSystem = FMatrix::Identity;
}

void FWidget::Render( const FSceneView* View,FPrimitiveDrawInterface* PDI )
{
	const UBOOL bDrawModeSupportsWidgetDrawing = GEditorModeTools().GetCurrentMode()->ShouldDrawWidget();
	const UBOOL bShowFlagsSupportsWidgetDrawing = (View->Family->ShowFlags & SHOW_ModeWidgets) ? TRUE : FALSE;
	const UBOOL bEditorModeToolsSupportsWidgetDrawing = GEditorModeTools().GetShowWidget();
	UBOOL bDrawWidget;

	// Because the movement routines use the widget axis to determine how to transform mouse movement into
	// editor object movement, we need to still run through the Render routine even though widget drawing may be
	// disabled.  So we keep a flag that is used to determine whether or not to actually render anything.  This way
	// we can still update the widget axis' based on the Context's transform matrices, even though drawing is disabled.
	if(bDrawModeSupportsWidgetDrawing && bShowFlagsSupportsWidgetDrawing && bEditorModeToolsSupportsWidgetDrawing)
	{
		bDrawWidget = TRUE;

		// See if there is a custom coordinate system we should be using, only change it if we are drawing widgets.
		CustomCoordSystem = GEditorModeTools().GetCustomDrawingCoordinateSystem();
	}
	else
	{
		bDrawWidget = FALSE;
	}

	// If the current mode doesn't want to use the widget, don't draw it.

	if( !GEditorModeTools().GetCurrentMode()->UsesWidget() )
	{
		CurrentAxis = AXIS_None;
		return;
	}

	FVector Loc = GEditorModeTools().GetWidgetLocation();
	if(!View->ScreenToPixel(View->WorldToScreen(Loc),Origin))
	{
		// GEMINI_TODO: This case wasn't handled before.  Was that intentional?
		Origin.X = Origin.Y = 0;
	}

	switch( GEditorModeTools().GetWidgetMode() )
	{
		case WM_Translate:
			Render_Translate( View, PDI, Loc, bDrawWidget );
			break;

		case WM_Rotate:
			Render_Rotate( View, PDI, Loc, bDrawWidget );
			break;

		case WM_Scale:
			Render_Scale( View, PDI, Loc, bDrawWidget );
			break;

		case WM_ScaleNonUniform:
			Render_ScaleNonUniform( View, PDI, Loc, bDrawWidget );
			break;

		default:
			break;
	}
};

static const FLOAT CUBE_SCALE = 4.0f;
#define CUBE_VERT(x,y,z) MeshBuilder.AddVertex( FVector((x)*CUBE_SCALE+52,(y)*CUBE_SCALE,(z)*CUBE_SCALE), FVector2D(0,0), FVector(0,0,0), FVector(0,0,0), FVector(0,0,0), FColor(255,255,255) )
#define CUBE_FACE(i,j,k) MeshBuilder.AddTriangle( CubeVerts[(i)],CubeVerts[(j)],CubeVerts[(k)] )

/**
 * Draws an arrow head line for a specific axis.
 */
void FWidget::Render_Axis( const FSceneView* View, FPrimitiveDrawInterface* PDI, EAxis InAxis, FMatrix& InMatrix, UMaterialInterface* InMaterial, FColor* InColor, FVector2D& OutAxisEnd, FLOAT InScale, UBOOL bDrawWidget, UBOOL bCubeHead )
{
	FMatrix ArrowToWorld = FScaleMatrix(FVector(InScale,InScale,InScale)) * InMatrix;

	if( bDrawWidget )
	{
		PDI->SetHitProxy( new HWidgetAxis(InAxis) );

		PDI->DrawLine( InMatrix.TransformFVector( FVector(8,0,0) * InScale ), InMatrix.TransformFVector( FVector(48,0,0) * InScale ), *InColor, SDPG_UnrealEdForeground );

		FDynamicMeshBuilder MeshBuilder;

		if ( bCubeHead )
		{
			// Compute cube vertices.
			INT CubeVerts[8];
			CubeVerts[0] = CUBE_VERT( 1, 1, 1 );
			CubeVerts[1] = CUBE_VERT( 1, -1, 1 );
			CubeVerts[2] = CUBE_VERT( 1, -1, -1 );
			CubeVerts[3] = CUBE_VERT( 1, 1, -1 );
			CubeVerts[4] = CUBE_VERT( -1, 1, 1 );
			CubeVerts[5] = CUBE_VERT( -1, -1, 1 );
			CubeVerts[6] = CUBE_VERT( -1, -1, -1 );
			CubeVerts[7] = CUBE_VERT( -1, 1, -1 );
			CUBE_FACE(0,1,2);
			CUBE_FACE(0,2,3);
			CUBE_FACE(4,5,6);
			CUBE_FACE(4,6,7);

			CUBE_FACE(4,0,7);
			CUBE_FACE(0,7,3);
			CUBE_FACE(1,6,5);
			CUBE_FACE(1,6,2);

			CUBE_FACE(5,4,0);
			CUBE_FACE(5,0,1);
			CUBE_FACE(3,7,6);
			CUBE_FACE(3,6,2);
		}
		else
		{
			// Compute the arrow cone vertices.
			INT ArrowVertices[AXIS_ARROW_SEGMENTS];
			for(INT VertexIndex = 0;VertexIndex < AXIS_ARROW_SEGMENTS;VertexIndex++)
			{
				FLOAT theta =  PI * 2.f * VertexIndex / AXIS_ARROW_SEGMENTS;
				ArrowVertices[VertexIndex] = MeshBuilder.AddVertex(
					FVector(40,AXIS_ARROW_RADIUS * appSin( theta ) * 0.5f,AXIS_ARROW_RADIUS * appCos( theta ) * 0.5f),
					FVector2D(0,0),
					FVector(0,0,0),
					FVector(0,0,0),
					FVector(0,0,0),
					FColor(255,255,255)
					);
			}
			INT RootArrowVertex = MeshBuilder.AddVertex(FVector(64,0,0),FVector2D(0,0),FVector(0,0,0),FVector(0,0,0),FVector(0,0,0), FColor(255,255,255));

			// Build the arrow mesh.
			for( INT s = 0 ; s < AXIS_ARROW_SEGMENTS ; s++ )
			{
				MeshBuilder.AddTriangle(RootArrowVertex,ArrowVertices[s],ArrowVertices[(s+1)%AXIS_ARROW_SEGMENTS]);
			}
		}

		// Draw the arrow mesh.
		MeshBuilder.Draw(PDI,ArrowToWorld,InMaterial->GetRenderProxy(FALSE),SDPG_UnrealEdForeground);

		PDI->SetHitProxy( NULL );
	}

	if(!View->ScreenToPixel(View->WorldToScreen(ArrowToWorld.TransformFVector(FVector(64,0,0))),OutAxisEnd))
	{
		// GEMINI_TODO: This case previously left the contents of OutAxisEnd unmodified.  Is that desired?
		OutAxisEnd.X = OutAxisEnd.Y = 0;
	}
}

/**
 * Draws the translation widget.
 */
void FWidget::Render_Translate( const FSceneView* View, FPrimitiveDrawInterface* PDI, const FVector& InLocation, UBOOL bDrawWidget )
{
	FLOAT Scale = View->WorldToScreen( InLocation ).W * ( 4 / View->SizeX / View->ProjectionMatrix.M[0][0] );

	// Figure out axis colors

	FColor *XColor = &( CurrentAxis&AXIS_X ? CurrentColor : AxisColorX );
	FColor *YColor = &( CurrentAxis&AXIS_Y ? CurrentColor : AxisColorY );
	FColor *ZColor = &( CurrentAxis&AXIS_Z ? CurrentColor : AxisColorZ );

	// Figure out axis materials

	UMaterial* XMaterial = ( CurrentAxis&AXIS_X ? CurrentMaterial : AxisMaterialX );
	UMaterial* YMaterial = ( CurrentAxis&AXIS_Y ? CurrentMaterial : AxisMaterialY );
	UMaterial* ZMaterial = ( CurrentAxis&AXIS_Z ? CurrentMaterial : AxisMaterialZ );

	// Figure out axis matrices

	FMatrix XMatrix = CustomCoordSystem * FTranslationMatrix( InLocation );
	FMatrix YMatrix = FRotationMatrix( FRotator(0,16384,0) ) * CustomCoordSystem * FTranslationMatrix( InLocation );
	FMatrix ZMatrix = FRotationMatrix( FRotator(16384,0,0) ) * CustomCoordSystem * FTranslationMatrix( InLocation );

	UBOOL bIsPerspective = ( View->ProjectionMatrix.M[3][3] < 1.0f );

	// For local space widgets, we always want to draw all three axis, since they may not be aligned with
	// the orthographic projection anyway.
	UBOOL bIsLocalSpace = ( GEditorModeTools().CoordSystem == COORD_Local );

	INT DrawAxis = GEditorModeTools().GetCurrentMode()->GetWidgetAxisToDraw( GEditorModeTools().GetWidgetMode() );

	// Draw the axis lines with arrow heads
	if( DrawAxis&AXIS_X && (bIsPerspective || bIsLocalSpace || View->ViewMatrix.M[0][2] != 1.f) )
	{
		Render_Axis( View, PDI, AXIS_X, XMatrix, XMaterial, XColor, XAxisEnd, Scale, bDrawWidget );
	}

	if( DrawAxis&AXIS_Y && (bIsPerspective || bIsLocalSpace || View->ViewMatrix.M[0][0] != 1.f) )
	{
		Render_Axis( View, PDI, AXIS_Y, YMatrix, YMaterial, YColor, YAxisEnd, Scale, bDrawWidget );
	}

	if( DrawAxis&AXIS_Z && (bIsPerspective || bIsLocalSpace || View->ViewMatrix.M[0][1] != 1.f) )
	{
		Render_Axis( View, PDI, AXIS_Z, ZMatrix, ZMaterial, ZColor, ZAxisEnd, Scale, bDrawWidget );
	}

	// Draw the grabbers
	if( bDrawWidget )
	{
		if( bIsPerspective || bIsLocalSpace || View->ViewMatrix.M[0][1] == 1.f )
		{
			if( (DrawAxis&(AXIS_X|AXIS_Y)) == (AXIS_X|AXIS_Y) ) 
			{
				PDI->SetHitProxy( new HWidgetAxis(AXIS_XY) );
				{
					PDI->DrawLine( XMatrix.TransformFVector(FVector(16,0,0) * Scale), XMatrix.TransformFVector(FVector(16,16,0) * Scale), *XColor, SDPG_UnrealEdForeground );
					PDI->DrawLine( XMatrix.TransformFVector(FVector(16,16,0) * Scale), XMatrix.TransformFVector(FVector(0,16,0) * Scale), *YColor, SDPG_UnrealEdForeground );
				}
				PDI->SetHitProxy( NULL );
			}
		}

		if( bIsPerspective || bIsLocalSpace || View->ViewMatrix.M[0][0] == 1.f )
		{
			if( (DrawAxis&(AXIS_X|AXIS_Z)) == (AXIS_X|AXIS_Z) ) 
			{
				PDI->SetHitProxy( new HWidgetAxis(AXIS_XZ) );
				{
					PDI->DrawLine( XMatrix.TransformFVector(FVector(16,0,0) * Scale), XMatrix.TransformFVector(FVector(16,0,16) * Scale), *XColor, SDPG_UnrealEdForeground );
					PDI->DrawLine( XMatrix.TransformFVector(FVector(16,0,16) * Scale), XMatrix.TransformFVector(FVector(0,0,16) * Scale), *ZColor, SDPG_UnrealEdForeground );
				}
				PDI->SetHitProxy( NULL );
			}
		}

		if( bIsPerspective || bIsLocalSpace || View->ViewMatrix.M[0][0] == 0.f )
		{
			if( (DrawAxis&(AXIS_Y|AXIS_Z)) == (AXIS_Y|AXIS_Z) ) 
			{
				PDI->SetHitProxy( new HWidgetAxis(AXIS_YZ) );
				{
					PDI->DrawLine( XMatrix.TransformFVector(FVector(0,16,0) * Scale), XMatrix.TransformFVector(FVector(0,16,16) * Scale), *YColor, SDPG_UnrealEdForeground );
					PDI->DrawLine( XMatrix.TransformFVector(FVector(0,16,16) * Scale), XMatrix.TransformFVector(FVector(0,0,16) * Scale), *ZColor, SDPG_UnrealEdForeground );
				}
				PDI->SetHitProxy( NULL );
			}
		}
	}
}

/**
 * Draws the rotation widget.
 */
void FWidget::Render_Rotate( const FSceneView* View,FPrimitiveDrawInterface* PDI, const FVector& InLocation, UBOOL bDrawWidget )
{
	FLOAT Scale = View->WorldToScreen( InLocation ).W * ( 4 / View->SizeX / View->ProjectionMatrix.M[0][0] );

	// Figure out axis colors

	FColor *XColor = &( CurrentAxis&AXIS_X ? CurrentColor : AxisColorX );
	FColor *YColor = &( CurrentAxis&AXIS_Y ? CurrentColor : AxisColorY );
	FColor *ZColor = &( CurrentAxis&AXIS_Z ? CurrentColor : AxisColorZ );

	// Figure out axis matrices

	FMatrix XMatrix = FRotationMatrix( FRotator(0,16384,0) ) * FTranslationMatrix( InLocation );
	FMatrix YMatrix = FRotationMatrix( FRotator(0,16384,0) ) * FTranslationMatrix( InLocation );
	FMatrix ZMatrix = FTranslationMatrix( InLocation );

	INT DrawAxis = GEditorModeTools().GetCurrentMode()->GetWidgetAxisToDraw( GEditorModeTools().GetWidgetMode() );

	// Draw a circle for each axis

	if( DrawAxis&AXIS_X )
	{
		if( bDrawWidget )
		{
			PDI->SetHitProxy( new HWidgetAxis(AXIS_X) );
			{
				DrawCircle( PDI, InLocation, CustomCoordSystem.TransformFVector( FVector(0,1,0) ), CustomCoordSystem.TransformFVector( FVector(0,0,1) ), *XColor, AXIS_CIRCLE_RADIUS*Scale, AXIS_CIRCLE_SIDES, SDPG_UnrealEdForeground );
			}
			PDI->SetHitProxy( NULL );
		}

		
		// Update Axis by projecting the axis vector to screenspace.
		View->ScreenToPixel(View->WorldToScreen( XMatrix.TransformFVector( FVector(96,0,0) ) ), XAxisEnd);
	}

	if( DrawAxis&AXIS_Y )
	{
		if( bDrawWidget )
		{	
			PDI->SetHitProxy( new HWidgetAxis(AXIS_Y) );
			{
				DrawCircle( PDI, InLocation, CustomCoordSystem.TransformFVector( FVector(1,0,0) ), CustomCoordSystem.TransformFVector( FVector(0,0,1) ), *YColor, AXIS_CIRCLE_RADIUS*Scale, AXIS_CIRCLE_SIDES, SDPG_UnrealEdForeground );
			}
			PDI->SetHitProxy( NULL );
		}

		// Update Axis by projecting the axis vector to screenspace.
		View->ScreenToPixel(View->WorldToScreen( XMatrix.TransformFVector( FVector(0,96,0) ) ), YAxisEnd);	
}

	if( DrawAxis&AXIS_Z )
	{
		if( bDrawWidget )
		{	
			PDI->SetHitProxy( new HWidgetAxis(AXIS_Z) );
			{
				DrawCircle( PDI, InLocation, CustomCoordSystem.TransformFVector( FVector(1,0,0) ), CustomCoordSystem.TransformFVector( FVector(0,1,0) ), *ZColor, AXIS_CIRCLE_RADIUS*Scale, AXIS_CIRCLE_SIDES, SDPG_UnrealEdForeground );
			}
			PDI->SetHitProxy( NULL );
		}

		// Update Axis by projecting the axis vector to screenspace.
		View->ScreenToPixel(View->WorldToScreen( XMatrix.TransformFVector( FVector(96,0,0) ) ), ZAxisEnd);	
}
}

/**
 * Draws the scaling widget.
 */
void FWidget::Render_Scale( const FSceneView* View,FPrimitiveDrawInterface* PDI, const FVector& InLocation, UBOOL bDrawWidget )
{
	FLOAT Scale = View->WorldToScreen( InLocation ).W * ( 4 / View->SizeX / View->ProjectionMatrix.M[0][0] );

	// Figure out axis colors

	FColor *Color = &( CurrentAxis != AXIS_None ? CurrentColor : AxisColorX );

	// Figure out axis materials

	UMaterial* Material = ( CurrentAxis&AXIS_X ? CurrentMaterial : AxisMaterialX );

	// Figure out axis matrices

	FMatrix XMatrix = CustomCoordSystem * FTranslationMatrix( InLocation );
	FMatrix YMatrix = FRotationMatrix( FRotator(0,16384,0) ) * CustomCoordSystem * FTranslationMatrix( InLocation );
	FMatrix ZMatrix = FRotationMatrix( FRotator(16384,0,0) ) * CustomCoordSystem * FTranslationMatrix( InLocation );

	// Draw the axis lines with cube heads

	Render_Axis( View, PDI, AXIS_X, XMatrix, Material, Color, XAxisEnd, Scale, bDrawWidget, TRUE );
	Render_Axis( View, PDI, AXIS_Y, YMatrix, Material, Color, YAxisEnd, Scale, bDrawWidget, TRUE );
	Render_Axis( View, PDI, AXIS_Z, ZMatrix, Material, Color, ZAxisEnd, Scale, bDrawWidget, TRUE );

	PDI->SetHitProxy( NULL );
}

/**
 * Draws the non-uniform scaling widget.
 */
void FWidget::Render_ScaleNonUniform( const FSceneView* View,FPrimitiveDrawInterface* PDI, const FVector& InLocation, UBOOL bDrawWidget )
{
	FLOAT Scale = View->WorldToScreen( InLocation ).W * ( 4 / View->SizeX / View->ProjectionMatrix.M[0][0] );

	// Figure out axis colors

	FColor *XColor = &( CurrentAxis&AXIS_X ? CurrentColor : AxisColorX );
	FColor *YColor = &( CurrentAxis&AXIS_Y ? CurrentColor : AxisColorY );
	FColor *ZColor = &( CurrentAxis&AXIS_Z ? CurrentColor : AxisColorZ );

	// Figure out axis materials

	UMaterial* XMaterial = ( CurrentAxis&AXIS_X ? CurrentMaterial : AxisMaterialX );
	UMaterial* YMaterial = ( CurrentAxis&AXIS_Y ? CurrentMaterial : AxisMaterialY );
	UMaterial* ZMaterial = ( CurrentAxis&AXIS_Z ? CurrentMaterial : AxisMaterialZ );

	// Figure out axis matrices

	FMatrix XMatrix = CustomCoordSystem * FTranslationMatrix( InLocation );
	FMatrix YMatrix = FRotationMatrix( FRotator(0,16384,0) ) * CustomCoordSystem * FTranslationMatrix( InLocation );
	FMatrix ZMatrix = FRotationMatrix( FRotator(16384,0,0) ) * CustomCoordSystem * FTranslationMatrix( InLocation );

	INT DrawAxis = GEditorModeTools().GetCurrentMode()->GetWidgetAxisToDraw( GEditorModeTools().GetWidgetMode() );

	// Draw the axis lines with cube heads

	if( DrawAxis&AXIS_X )	Render_Axis( View, PDI, AXIS_X, XMatrix, XMaterial, XColor, XAxisEnd, Scale, bDrawWidget, TRUE );
	if( DrawAxis&AXIS_Y )	Render_Axis( View, PDI, AXIS_Y, YMatrix, YMaterial, YColor, YAxisEnd, Scale, bDrawWidget, TRUE );
	if( DrawAxis&AXIS_Z )	Render_Axis( View, PDI, AXIS_Z, ZMatrix, ZMaterial, ZColor, ZAxisEnd, Scale, bDrawWidget, TRUE );

	// Draw grabber handles
	if ( bDrawWidget )
	{
		if( ((DrawAxis&(AXIS_X|AXIS_Y)) == (AXIS_X|AXIS_Y)) )
		{
			PDI->SetHitProxy( new HWidgetAxis(AXIS_XY) );
			{
				PDI->DrawLine( XMatrix.TransformFVector(FVector(16,0,0) * Scale), XMatrix.TransformFVector(FVector(8,8,0) * Scale), *XColor, SDPG_UnrealEdForeground );
				PDI->DrawLine( XMatrix.TransformFVector(FVector(8,8,0) * Scale), XMatrix.TransformFVector(FVector(0,16,0) * Scale), *YColor, SDPG_UnrealEdForeground );
			}
			PDI->SetHitProxy( NULL );
		}

		if( ((DrawAxis&(AXIS_X|AXIS_Z)) == (AXIS_X|AXIS_Z)) )
		{
			PDI->SetHitProxy( new HWidgetAxis(AXIS_XZ) );
			{
				PDI->DrawLine( XMatrix.TransformFVector(FVector(16,0,0) * Scale), XMatrix.TransformFVector(FVector(8,0,8) * Scale), *XColor, SDPG_UnrealEdForeground );
				PDI->DrawLine( XMatrix.TransformFVector(FVector(8,0,8) * Scale), XMatrix.TransformFVector(FVector(0,0,16) * Scale), *ZColor, SDPG_UnrealEdForeground );
			}
			PDI->SetHitProxy( NULL );
		}

		if( ((DrawAxis&(AXIS_Y|AXIS_Z)) == (AXIS_Y|AXIS_Z)) )
		{
			PDI->SetHitProxy( new HWidgetAxis(AXIS_YZ) );
			{
				PDI->DrawLine( XMatrix.TransformFVector(FVector(0,16,0) * Scale), XMatrix.TransformFVector(FVector(0,8,8) * Scale), *YColor, SDPG_UnrealEdForeground );
				PDI->DrawLine( XMatrix.TransformFVector(FVector(0,8,8) * Scale), XMatrix.TransformFVector(FVector(0,0,16) * Scale), *ZColor, SDPG_UnrealEdForeground );
			}
			PDI->SetHitProxy( NULL );
		}
	}
}

/**
 * Converts mouse movement on the screen to widget axis movement/rotation.
 */
void FWidget::ConvertMouseMovementToAxisMovement( FEditorLevelViewportClient* InViewportClient, const FVector& InLocation, const FVector& InDiff, FVector& InDrag, FRotator& InRotation, FVector& InScale )
{
	FSceneViewFamilyContext ViewFamily(InViewportClient->Viewport,InViewportClient->GetScene(),InViewportClient->ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL);
	InViewportClient->CalcSceneView(&ViewFamily);
	FPlane Wk;
	FVector2D AxisEnd;
	FVector Diff = InDiff;

	InDrag = FVector(0,0,0);
	InRotation = FRotator(0,0,0);
	InScale = FVector(0,0,0);

	// Get the end of the axis (in screen space) based on which axis is being pulled

	switch( CurrentAxis )
	{
		case AXIS_X:	AxisEnd = XAxisEnd;		break;
		case AXIS_Y:	AxisEnd = YAxisEnd;		break;
		case AXIS_Z:	AxisEnd = ZAxisEnd;		break;
		case AXIS_XY:	AxisEnd = Diff.X != 0 ? XAxisEnd : YAxisEnd;		break;
		case AXIS_XZ:	AxisEnd = Diff.X != 0 ? XAxisEnd : ZAxisEnd;		break;
		case AXIS_YZ:	AxisEnd = Diff.X != 0 ? YAxisEnd : ZAxisEnd;		break;
		case AXIS_XYZ:	AxisEnd = Diff.X != 0 ? YAxisEnd : ZAxisEnd;		break;
		default:
			break;
	}

	// Screen space Y axis is inverted

	Diff.Y *= -1;

	// Get the directions of the axis (on the screen) and the mouse drag direction (in screen space also).

	FVector2D AxisDir = AxisEnd - Origin;
	AxisDir.Normalize();

	FVector2D DragDir( Diff.X, Diff.Y );
	DragDir.Normalize();

	// Use the most dominant axis the mouse is being dragged along

	INT idx = 0;
	if( Abs(Diff.X) < Abs(Diff.Y) )
	{
		idx = 1;
	}

	FLOAT Val = Diff[idx];

	// If the axis dir is negative, it is pointing in the negative screen direction.  In this situation, the mouse
	// drag must be inverted so that you are still dragging in the right logical direction.
	//
	// For example, if the X axis is pointing left and you drag left, this will ensure that the widget moves left.

	if( AxisDir[idx] < 0 )
	{
		Val *= -1;
	}

	// Honor INI option to invert Z axis movement on the widget

	if( idx == 1 && (CurrentAxis&AXIS_Z) && GEditor->InvertwidgetZAxis &&
		// Don't apply this if the origin and the AxisEnd are the same
		AxisDir.IsNearlyZero() == FALSE)
	{
		Val *= -1;
	}

	FMatrix InputCoordSystem = GEditorModeTools().GetCustomInputCoordinateSystem();

	switch( GEditorModeTools().GetWidgetMode() )
	{
		case WM_Translate:
			switch( CurrentAxis )
			{
				case AXIS_X:	InDrag = FVector( Val, 0, 0 );	break;
				case AXIS_Y:	InDrag = FVector( 0, Val, 0 );	break;
				case AXIS_Z:	InDrag = FVector( 0, 0, -Val );	break;
				case AXIS_XY:	InDrag = ( InDiff.X != 0 ? FVector( 0, Val, 0 ) : FVector( -Val, 0, 0 ) );	break;
				case AXIS_XZ:	InDrag = ( InDiff.X != 0 ? FVector( Val, 0, 0 ) : FVector( 0, 0, -Val ) );	break;
				case AXIS_YZ:	InDrag = ( InDiff.X != 0 ? FVector( 0, Val, 0 ) : FVector( 0, 0, -Val ) );	break;
			}

			InDrag = InputCoordSystem.TransformFVector( InDrag );
			break;

		case WM_Rotate:
			{
				FVector Axis;
				switch( CurrentAxis )
				{
					case AXIS_X:	Axis = FVector( -1, 0, 0 );	break;
					case AXIS_Y:	Axis = FVector( 0, -1, 0 );	break;
					case AXIS_Z:	Axis = FVector( 0, 0, 1 );	break;
				}

				Axis = CustomCoordSystem.TransformNormal( Axis );

				const FLOAT RotationSpeed = (2.f*(FLOAT)PI)/65536.f;
				const FQuat DeltaQ( Axis, Val*RotationSpeed );
				
				InRotation = FRotator(DeltaQ);
			}
			break;

		case WM_Scale:
			InScale = FVector( Val, Val, Val );
			break;

		case WM_ScaleNonUniform:
		{
			FVector Axis;
			switch( CurrentAxis )
			{
				case AXIS_X:	Axis = FVector( 1, 0, 0 );	break;
				case AXIS_Y:	Axis = FVector( 0, 1, 0 );	break;
				case AXIS_Z:	Axis = FVector( 0, 0, 1 );	break;
				case AXIS_XY:	Axis = ( InDiff.X != 0 ? FVector( 1, 0, 0 ) : FVector( 0, 1, 0 ) );	break;
				case AXIS_XZ:	Axis = ( InDiff.X != 0 ? FVector( 1, 0, 0 ) : FVector( 0, 0, 1 ) );	break;
				case AXIS_YZ:	Axis = ( InDiff.X != 0 ? FVector( 0, 1, 0 ) : FVector( 0, 0, 1 ) );	break;
				case AXIS_XYZ:	Axis = FVector( 1, 1, 1 ); break;
			}

			InScale = Axis * Val;
		}
		break;

		default:
			break;
	}

}
