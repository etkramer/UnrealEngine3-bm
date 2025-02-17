/*=============================================================================
	UnEdCnst.cpp: Editor movement contraints.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"

/*------------------------------------------------------------------------------
	Functions.
------------------------------------------------------------------------------*/

FLOAT FEditorConstraints::GetGridSize()
{
	return GridSizes[ CurrentGridSz ];
}

void FEditorConstraints::SetGridSz( INT InIndex )
{
	GEditor->FinishAllSnaps();

	CurrentGridSz = InIndex;
	CurrentGridSz = Clamp<INT>( CurrentGridSz, 0, MAX_GRID_SIZES-1 );

	GEditor->RedrawLevelEditingViewports();
	GCallbackEvent->Send( CALLBACK_UpdateUI );
}

void FEditorConstraints::Snap( FVector& Point, const FVector& GridBase )
{
	if( GridEnabled )
	{
		Point = (Point - GridBase).GridSnap( GetGridSize() ) + GridBase;
	}
}

void FEditorConstraints::SnapScale( FVector& Point, const FVector& GridBase )
{
	if( SnapScaleEnabled )
	{
		Point = (Point - GridBase).GridSnap( GetGridSize() ) + GridBase;
	}
}

void FEditorConstraints::Snap( FRotator& Rotation )
{
	if( RotGridEnabled )
	{
		Rotation = Rotation.GridSnap( RotGridSize );
	}
}
UBOOL FEditorConstraints::Snap( FVector& Location, FVector GridBase, FRotator& Rotation )
{
	UBOOL Snapped = 0;
	Snap( Rotation );
	if( SnapVertices )
	{
		FVector	DestPoint;
		INT Temp;
		if( GWorld->GetModel()->FindNearestVertex( Location, DestPoint, SnapDistance, Temp ) >= 0.0)
		{
			Location = DestPoint;
			Snapped = 1;
		}
	}
	if( !Snapped )
		Snap( Location, GridBase );
	return Snapped;
}


