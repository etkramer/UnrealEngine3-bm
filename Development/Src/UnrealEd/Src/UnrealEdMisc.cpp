/*=============================================================================
	UnrealEdMisc.cpp: Misc UnrealEd helper functions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"

/*-----------------------------------------------------------------------------
	FPolyBreaker.
-----------------------------------------------------------------------------*/

FPolyBreaker::FPolyBreaker()
{
	PolyVerts = NULL;
}
FPolyBreaker::~FPolyBreaker()
{
	FinalPolys.Empty();
}

void FPolyBreaker::Process( TArray<FVector>* InPolyVerts, FVector InPolyNormal )
{
	PolyVerts = InPolyVerts;
	PolyNormal = InPolyNormal;

	MakeConvexPoly( PolyVerts );
	Optimize();
	
	FPlane testplane( FinalPolys(0).Vertices(0), FinalPolys(0).Vertices(1), FinalPolys(0).Vertices(2) );
	if( InPolyNormal != testplane )
	{
		for( INT x = 0 ; x < FinalPolys.Num() ; ++x )
			FinalPolys(x).Reverse();

		TArray<FVector> NewVerts;
		for( INT x = PolyVerts->Num()-1 ; x > -1 ; --x )
			new(NewVerts)FVector((*PolyVerts)(x));
		*PolyVerts = NewVerts;
	}

}
UBOOL FPolyBreaker::IsPolyConvex( FPoly* InPoly )
{
	TArray<FVector> Verts;
	for( INT x = 0 ; x < InPoly->Vertices.Num() ; ++x )
		new(Verts)FVector( (*InPoly).Vertices(x) );
	UBOOL Ret = IsPolyConvex( &Verts );
	Verts.Empty();
	return Ret;
}
UBOOL FPolyBreaker::IsPolyConvex( TArray<FVector>* InVerts )
{
	for( INT x = 0 ; x < InVerts->Num() ; ++x )
	{
		FVector Edge = (*InVerts)(x) - (*InVerts)( x < InVerts->Num()-1 ? x+1 : 0 );
		Edge.Normalize();

		FPlane CuttingPlane( (*InVerts)(x), (*InVerts)(x) + (PolyNormal * 16), (*InVerts)(x) + (Edge * 16 ) );
		TArray<FVector> FrontPoly, BackPoly;

		INT result = SplitWithPlane( InVerts, InVerts->Num(), (*InVerts)(x), CuttingPlane, &FrontPoly, &BackPoly );

		if( result == SP_Split )
		{
			return 0;
		}
	}

	return 1;

}
void FPolyBreaker::MakeConvexPoly( TArray<FVector>* InVerts )
{
	for( INT x = 0 ; x < InVerts->Num() ; ++x )
	{
		FVector Edge = (*InVerts)(x) - (*InVerts)( x < InVerts->Num()-1 ? x+1 : 0 );
		Edge.Normalize();

		FPlane CuttingPlane( (*InVerts)(x), (*InVerts)(x) + (PolyNormal * 16), (*InVerts)(x) + (Edge * 16 ) );
		TArray<FVector> FrontPoly, BackPoly;

		INT result = SplitWithPlane( InVerts, InVerts->Num(), (*InVerts)(x), CuttingPlane, &FrontPoly, &BackPoly );

		if( result == SP_Split )
		{
			MakeConvexPoly( &FrontPoly );
			MakeConvexPoly( &BackPoly );
			return;
		}
	}

	FPoly NewPoly;
	NewPoly.Init();
	for( INT x = 0 ; x < InVerts->Num() ; ++x )
	{
		new(NewPoly.Vertices) FVector((*InVerts)(x));
	}

	if( NewPoly.Vertices.Num() > 2 )
	{
		NewPoly.CalcNormal();
		new(FinalPolys)FPoly( NewPoly );
	}
}
INT FPolyBreaker::TryToMerge( FPoly *Poly1, FPoly *Poly2 )
{
	// Find one overlapping point.
	INT Start1=0, Start2=0;
	for( Start1=0; Start1<Poly1->Vertices.Num(); ++Start1 )
		for( Start2=0; Start2<Poly2->Vertices.Num(); ++Start2 )
			if( FPointsAreSame(Poly1->Vertices(Start1), Poly2->Vertices(Start2)) )
				goto FoundOverlap;
	return 0;
	FoundOverlap:

	// Wrap around trying to merge.
	INT End1  = Start1;
	INT End2  = Start2;
	INT Test1 = Start1+1; if (Test1>=Poly1->Vertices.Num()) Test1 = 0;
	INT Test2 = Start2-1; if (Test2<0)                   Test2 = Poly2->Vertices.Num()-1;
	if( FPointsAreSame(Poly1->Vertices(Test1),Poly2->Vertices(Test2)) )
	{
		End1   = Test1;
		Start2 = Test2;
	}
	else
	{
		Test1 = Start1-1; if (Test1<0)                   Test1=Poly1->Vertices.Num()-1;
		Test2 = Start2+1; if (Test2>=Poly2->Vertices.Num()) Test2=0;
		if( FPointsAreSame(Poly1->Vertices(Test1),Poly2->Vertices(Test2)) )
		{
			Start1 = Test1;
			End2   = Test2;
		}
		else return 0;
	}

	// Build a new edpoly containing both polygons merged.
	FPoly NewPoly = *Poly1;
	NewPoly.Vertices.Empty();
	INT Vertex = End1;
	for( INT i=0; i<Poly1->Vertices.Num(); ++i )
	{
		new(NewPoly.Vertices) FVector(Poly1->Vertices(Vertex));
		if( ++Vertex >= Poly1->Vertices.Num() )
			Vertex=0;
	}
	Vertex = End2;
	for( INT i=0; i<(Poly2->Vertices.Num()-2); ++i )
	{
		if( ++Vertex >= Poly2->Vertices.Num() )
			Vertex=0;
		new(NewPoly.Vertices) FVector(Poly2->Vertices(Vertex));
	}

	// Remove colinear vertices and check convexity.
	if( NewPoly.RemoveColinears() )
	{
		*Poly1 = NewPoly;
		Poly2->Vertices.Empty();
		return TRUE;
	}
	else return 0;
}
// Looks at the resulting polygons and tries to put polys with matching edges
// together.  This reduces the total number of polys in the final shape.
void FPolyBreaker::Optimize()
{
	while( OptimizeList( &FinalPolys ) );
}
// Returns 1 if any polys were merged
UBOOL FPolyBreaker::OptimizeList( TArray<FPoly>* PolyList )
{
	TArray<FPoly> OptimizedPolys, TempPolys;
	UBOOL bDidMergePolys = 0;

	TempPolys = FinalPolys;

	for( INT x = 0 ; x < TempPolys.Num() && !bDidMergePolys ; ++x )
	{
		for( INT y = 0 ; y < PolyList->Num()  && !bDidMergePolys ; ++y )
		{
			if( TempPolys(x) != (*PolyList)(y) )
			{
				FPoly Poly1 = TempPolys(x);
				bDidMergePolys = TryToMerge( &Poly1, &(*PolyList)(y) );
				new(OptimizedPolys)FPoly(Poly1);
			}
		}
	}

	if( bDidMergePolys )
		FinalPolys = OptimizedPolys;

	OptimizedPolys.Empty();
	return bDidMergePolys;
}
// This is basically the same function as FPoly::SplitWithPlane, but modified
// to work with this classes data structures.
INT FPolyBreaker::SplitWithPlane
(
	TArray<FVector>			*Vertex,
	int						NumVertices,
	const FVector			&PlaneBase,
	const FVector			&PlaneNormal,
	TArray<FVector>			*FrontPoly,
	TArray<FVector>			*BackPoly
) const
{
	FVector 	Intersection;
	FLOAT   	Dist=0,MaxDist=0,MinDist=0;
	FLOAT		PrevDist,Thresh = THRESH_SPLIT_POLY_PRECISELY;
	enum 	  	{V_FRONT,V_BACK,V_EITHER} Status,PrevStatus=V_EITHER;
	INT     	i,j;

	// Find number of vertices.
	check(NumVertices>=3);

	*FrontPoly = *Vertex;
	*BackPoly = *Vertex;

	// See if the polygon is split by SplitPoly, or it's on either side, or the
	// polys are coplanar.  Go through all of the polygon points and
	// calculate the minimum and maximum signed distance (in the direction
	// of the normal) from each point to the plane of SplitPoly.
	for( i=0; i<NumVertices; ++i )
	{
		Dist = FPointPlaneDist( (*Vertex)(i), PlaneBase, PlaneNormal );

		if( i==0 || Dist>MaxDist ) MaxDist=Dist;
		if( i==0 || Dist<MinDist ) MinDist=Dist;

		if      (Dist > +Thresh) PrevStatus = V_FRONT;
		else if (Dist < -Thresh) PrevStatus = V_BACK;
	}
	if( MaxDist<Thresh && MinDist>-Thresh )
	{
		return SP_Coplanar;
	}
	else if( MaxDist < Thresh )
	{
		return SP_Back;
	}
	else if( MinDist > -Thresh )
	{
		return SP_Front;
	}
	else
	{
		// Split.
		if( FrontPoly==NULL )
			return SP_Split; // Caller only wanted status.

		FrontPoly->Empty();
		BackPoly->Empty();

		j = NumVertices-1; // Previous vertex; have PrevStatus already.

		for( i=0; i<NumVertices; ++i )
		{
			PrevDist	= Dist;
      		Dist		= FPointPlaneDist( (*Vertex)(i), PlaneBase, PlaneNormal );

			if      (Dist > +Thresh)  	Status = V_FRONT;
			else if (Dist < -Thresh)  	Status = V_BACK;
			else						Status = PrevStatus;

			if( Status != PrevStatus )
			{
				// Crossing.  Either Front-to-Back or Back-To-Front.
				// Intersection point is naturally on both front and back polys.
				if( (Dist >= -Thresh) && (Dist < +Thresh) )
				{
					// This point lies on plane.
					if( PrevStatus == V_FRONT )
					{
						new(*FrontPoly)FVector( (*Vertex)(i) );
						new(*BackPoly)FVector( (*Vertex)(i) );
					}
					else
					{
						new(*BackPoly)FVector( (*Vertex)(i) );
						new(*FrontPoly)FVector( (*Vertex)(i) );
					}
				}
				else if( (PrevDist >= -Thresh) && (PrevDist < +Thresh) )
				{
					// Previous point lies on plane.
					if (Status == V_FRONT)
					{
						new(*FrontPoly)FVector( (*Vertex)(j) );
						new(*FrontPoly)FVector( (*Vertex)(i) );
					}
					else
					{
						new(*BackPoly)FVector( (*Vertex)(j) );
						new(*BackPoly)FVector( (*Vertex)(i) );
					}
				}
				else
				{
					// Intersection point is in between.
					Intersection = FLinePlaneIntersection((*Vertex)(j),(*Vertex)(i),PlaneBase,PlaneNormal);

					if( PrevStatus == V_FRONT )
					{
						new(*FrontPoly)FVector( Intersection );
						new(*BackPoly)FVector( Intersection );
						new(*BackPoly)FVector( (*Vertex)(i) );
					}
					else
					{
						new(*BackPoly)FVector( Intersection );
						new(*FrontPoly)FVector( Intersection );
						new(*FrontPoly)FVector( (*Vertex)(i) );
					}
				}
			}
			else
			{
        		if (Status==V_FRONT) new(*FrontPoly)FVector( (*Vertex)(i) );
        		else                 new(*BackPoly)FVector( (*Vertex)(i) );
			}
			j          = i;
			PrevStatus = Status;
		}

		// Handle possibility of sliver polys due to precision errors.
		if( FrontPoly->Num()<3 )
			return SP_Back;
		else if( BackPoly->Num()<3 )
			return SP_Front;
		else return SP_Split;
	}
}

