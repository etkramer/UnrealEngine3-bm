/*=============================================================================
	UnFPoly.cpp: FPoly implementation (Editor polygons).
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#define PolyCheck( x )			check( x )

/**
 * Constructor, initializing all member variables.
 */
FPoly::FPoly()
{
	Init();
}

/**
 * Initialize everything in an  editor polygon structure to defaults.
 * Changes to these default values should also be mirrored to UPolysExporterT3D::ExportText(...).
 */
void FPoly::Init()
{
	Base			= FVector(0,0,0);
	Normal			= FVector(0,0,0);
	TextureU		= FVector(0,0,0);
	TextureV		= FVector(0,0,0);
	Vertices.Empty();
	PolyFlags       = PF_DefaultFlags;
	Actor			= NULL;
	Material        = NULL;
	ItemName        = NAME_None;
	iLink           = INDEX_NONE;
	iBrushPoly		= INDEX_NONE;
	SmoothingMask	= 0;
	ShadowMapScale	= 32.0f;

	FLightingChannelContainer		ChannelInitializer;
	ChannelInitializer.Bitfield		= 0;
	ChannelInitializer.BSP			= TRUE;
	ChannelInitializer.bInitialized	= TRUE;
	LightingChannels				= ChannelInitializer.Bitfield;
}

/**
 * Reverse an FPoly by reversing the normal and reversing the order of its vertices.
 */
void FPoly::Reverse()
{
	FVector Temp;
	int i,c;

	Normal *= -1;

	c=Vertices.Num()/2;
	for( i=0; i<c; i++ )
	{
		// Flip all points except middle if odd number of points.
		Temp      = Vertices(i);

		Vertices(i) = Vertices((Vertices.Num()-1)-i);
		Vertices((Vertices.Num()-1)-i) = Temp;
	}
}

/**
 * Fix up an editor poly by deleting vertices that are identical.  Sets
 * vertex count to zero if it collapses.  Returns number of vertices, 0 or >=3.
 */
int FPoly::Fix()
{
	int i,j,prev;

	j=0; prev=Vertices.Num()-1;
	for( i=0; i<Vertices.Num(); i++ )
	{
		if( !FPointsAreSame( Vertices(i), Vertices(prev) ) )
		{
			if( j != i )
				Vertices(j) = Vertices(i);
			prev = j;
			j    ++;
		}
		else debugf( NAME_Warning, TEXT("FPoly::Fix: Collapsed a point") );
	}
	if(j < 3)
	{
		Vertices.Empty();
	}
	else if(j < Vertices.Num())
	{
		Vertices.Remove(j,Vertices.Num() - j);
	}
	return Vertices.Num();
}

/**
 * Computes the 2D area of the polygon.  Returns zero if the polygon has less than three verices.
 */
FLOAT FPoly::Area()
{
	// If there are less than 3 verts
	if(Vertices.Num() < 3)
	{
		return 0.f;
	}

	FVector Side1,Side2;
	FLOAT Area;
	int i;

	Area  = 0.f;
	Side1 = Vertices(1) - Vertices(0);
	for( i=2; i<Vertices.Num(); i++ )
	{
		Side2 = Vertices(i) - Vertices(0);
		Area += (Side1 ^ Side2).Size() * 0.5f;
		Side1 = Side2;
	}
	return Area;
}

/**
 * Split with plane. Meant to be numerically stable.
 */
int FPoly::SplitWithPlane
(
	const FVector	&PlaneBase,
	const FVector	&PlaneNormal,
	FPoly			*FrontPoly,
	FPoly			*BackPoly,
	int				VeryPrecise
) const
{
	FVector 	Intersection;
	FLOAT   	Dist=0,MaxDist=0,MinDist=0;
	FLOAT		PrevDist,Thresh;
	enum 	  	{V_FRONT,V_BACK,V_EITHER} Status,PrevStatus=V_EITHER;
	int     	i,j;

	if (VeryPrecise)	Thresh = THRESH_SPLIT_POLY_PRECISELY;	
	else				Thresh = THRESH_SPLIT_POLY_WITH_PLANE;

	// Find number of vertices.
	check(Vertices.Num()>=3);

	// See if the polygon is split by SplitPoly, or it's on either side, or the
	// polys are coplanar.  Go through all of the polygon points and
	// calculate the minimum and maximum signed distance (in the direction
	// of the normal) from each point to the plane of SplitPoly.
	for( i=0; i<Vertices.Num(); i++ )
	{
		Dist = FPointPlaneDist( Vertices(i), PlaneBase, PlaneNormal );

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

		*FrontPoly = *this; // Copy all info.
		FrontPoly->PolyFlags |= PF_EdCut; // Mark as cut.
		FrontPoly->Vertices.Empty();

		*BackPoly = *this; // Copy all info.
		BackPoly->PolyFlags |= PF_EdCut; // Mark as cut.
		BackPoly->Vertices.Empty();

		j = Vertices.Num()-1; // Previous vertex; have PrevStatus already.

		for( i=0; i<Vertices.Num(); i++ )
		{
			PrevDist	= Dist;
      		Dist		= FPointPlaneDist( Vertices(i), PlaneBase, PlaneNormal );

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
						new(FrontPoly->Vertices) FVector(Vertices(i));
						new(BackPoly->Vertices) FVector(Vertices(i));
					}
					else
					{
						new(BackPoly->Vertices) FVector(Vertices(i));
						new(FrontPoly->Vertices) FVector(Vertices(i));
					}
				}
				else if( (PrevDist >= -Thresh) && (PrevDist < +Thresh) )
				{
					// Previous point lies on plane.
					if (Status == V_FRONT)
					{
						new(FrontPoly->Vertices) FVector(Vertices(j));
						new(FrontPoly->Vertices) FVector(Vertices(i));
					}
					else
					{
						new(BackPoly->Vertices) FVector(Vertices(j));
						new(BackPoly->Vertices) FVector(Vertices(i));
					}
				}
				else
				{
					// Intersection point is in between.
					Intersection = FLinePlaneIntersection(Vertices(j),Vertices(i),PlaneBase,PlaneNormal);

					if( PrevStatus == V_FRONT )
					{
						new(FrontPoly->Vertices) FVector(Intersection);
						new(BackPoly->Vertices) FVector(Intersection);
						new(BackPoly->Vertices) FVector(Vertices(i));
					}
					else
					{
						new(BackPoly->Vertices) FVector(Intersection);
						new(FrontPoly->Vertices) FVector(Intersection);
						new(FrontPoly->Vertices) FVector(Vertices(i));
					}
				}
			}
			else
			{
        		if (Status==V_FRONT) new(FrontPoly->Vertices)FVector(Vertices(i));
        		else                 new(BackPoly->Vertices)FVector(Vertices(i));
			}
			j          = i;
			PrevStatus = Status;
		}

		// Handle possibility of sliver polys due to precision errors.
		if( FrontPoly->Fix()<3 )
		{
			debugf( NAME_Warning, TEXT("FPoly::SplitWithPlane: Ignored front sliver") );
			return SP_Back;
		}
		else if( BackPoly->Fix()<3 )
	    {
			debugf( NAME_Warning, TEXT("FPoly::SplitWithPlane: Ignored back sliver") );
			return SP_Front;
		}
		else return SP_Split;
	}
}

/**
 * Split with a Bsp node.
 */
int FPoly::SplitWithNode
(
	const UModel	*Model,
	INT				iNode,
	FPoly			*FrontPoly,
	FPoly			*BackPoly,
	INT				VeryPrecise
) const
{
	const FBspNode &Node = Model->Nodes(iNode       );
	const FBspSurf &Surf = Model->Surfs(Node.iSurf  );

	return SplitWithPlane
	(
		Model->Points (Model->Verts(Node.iVertPool).pVertex),
		Model->Vectors(Surf.vNormal),
		FrontPoly, 
		BackPoly, 
		VeryPrecise
	);
}

/**
 * Split with plane quickly for in-game geometry operations.
 * Results are always valid. May return sliver polys.
 */
int FPoly::SplitWithPlaneFast
(
	const FPlane&	Plane,
	FPoly*			FrontPoly,
	FPoly*			BackPoly
) const
{
	FMemMark MemMark(GMainThreadMemStack);
	enum EPlaneClassification
	{
		V_FRONT=0,
		V_BACK=1
	};
	EPlaneClassification Status,PrevStatus;
	EPlaneClassification* VertStatus = new(GMainThreadMemStack) EPlaneClassification[Vertices.Num()];
	int Front=0,Back=0;

	EPlaneClassification* StatusPtr = &VertStatus[0];
	for( int i=0; i<Vertices.Num(); i++ )
	{
		FLOAT Dist = Plane.PlaneDot(Vertices(i));
		if( Dist >= 0.f )
		{
			*StatusPtr++ = V_FRONT;
			if( Dist > +THRESH_SPLIT_POLY_WITH_PLANE )
				Front=1;
		}
		else
		{
			*StatusPtr++ = V_BACK;
			if( Dist < -THRESH_SPLIT_POLY_WITH_PLANE )
				Back=1;
		}
	}
	ESplitType Result;
	if( !Front )
	{
		if( Back ) Result = SP_Back;
		else       Result = SP_Coplanar;
	}
	else if( !Back )
	{
		Result = SP_Front;
	}
	else
	{
		// Split.
		if( FrontPoly )
		{
			const FVector *V  = &Vertices(0);
			const FVector *W  = &Vertices(Vertices.Num()-1);
			FVector *V1       = &FrontPoly->Vertices(0);
			FVector *V2       = &BackPoly ->Vertices(0);
			PrevStatus        = VertStatus         [Vertices.Num()-1];
			StatusPtr         = &VertStatus        [0];

			for( int i=0; i<Vertices.Num(); i++ )
			{
				Status = *StatusPtr++;
				if( Status != PrevStatus )
				{
					// Crossing.
					const FVector& Intersection = FLinePlaneIntersection( *W, *V, Plane );
					new(FrontPoly->Vertices) FVector(Intersection);
					new(BackPoly->Vertices) FVector(Intersection);
					if( PrevStatus == V_FRONT )
					{
						new(BackPoly->Vertices) FVector(*V);
					}
					else
					{
						new(FrontPoly->Vertices) FVector(*V);
					}
				}
				else if( Status==V_FRONT )
				{
					new(FrontPoly->Vertices) FVector(*V);
				}
				else
				{
					new(BackPoly->Vertices) FVector(*V);
				}

				PrevStatus = Status;
				W          = V++;
			}
			FrontPoly->Base			= Base;
			FrontPoly->Normal		= Normal;
			FrontPoly->PolyFlags	= PolyFlags;

			BackPoly->Base			= Base;
			BackPoly->Normal		= Normal;
			BackPoly->PolyFlags		= PolyFlags;
		}
		Result = SP_Split;
	}

	MemMark.Pop();

	return Result;
}

/**
 * Compute normal of an FPoly.  Works even if FPoly has 180-degree-angled sides (which
 * are often created during T-joint elimination).  Returns nonzero result (plus sets
 * normal vector to zero) if a problem occurs.
 */
int FPoly::CalcNormal( UBOOL bSilent )
{
	Normal = FVector(0,0,0);
	for( int i=2; i<Vertices.Num(); i++ )
		Normal += (Vertices(i-1) - Vertices(0)) ^ (Vertices(i) - Vertices(0));

	if( Normal.SizeSquared() < (FLOAT)THRESH_ZERO_NORM_SQUARED )
	{
		if( !bSilent )
			debugf( NAME_Warning, TEXT("FPoly::CalcNormal: Zero-area polygon") );
		return 1;
	}
	Normal.Normalize();
	return 0;
}

/**
 * Transform an editor polygon with a coordinate system, a pre-transformation
 * addition, and a post-transformation addition.
 */
void FPoly::Transform
(
	const FVector&		PreSubtract,
	const FVector&		PostAdd
)
{
	FVector 	Temp;
	int 		i;

	Base = (Base - PreSubtract) + PostAdd;
	for( i=0; i<Vertices.Num(); i++ )
		Vertices(i)  = (Vertices(i) - PreSubtract) + PostAdd;

	// Transform normal.  Since the transformation coordinate system is
	// orthogonal but not orthonormal, it has to be renormalized here.
	Normal = Normal.SafeNormal();

}

/**
 * Remove colinear vertices and check convexity.  Returns 1 if convex, 0 if nonconvex or collapsed.
 */
INT FPoly::RemoveColinears()
{
	FMemMark MemMark(GMainThreadMemStack);
	FVector* SidePlaneNormal = new(GMainThreadMemStack) FVector[Vertices.Num()];
	FVector  Side;
	INT      i,j;
	UBOOL Result = TRUE;

	for( i=0; i<Vertices.Num(); i++ )
	{
		j=(i+Vertices.Num()-1)%Vertices.Num();

		// Create cutting plane perpendicular to both this side and the polygon's normal.
		Side = Vertices(i) - Vertices(j);
		SidePlaneNormal[i] = Side ^ Normal;

		if( !SidePlaneNormal[i].Normalize() )
		{
			// Eliminate these nearly identical points.
			Vertices.Remove(i,1);
			if(Vertices.Num() < 3)
			{
				// Collapsed.
				Vertices.Empty();
				Result = FALSE;
				break;
			}
			i--;
		}
	}
	if(Result)
	{
		for( i=0; i<Vertices.Num(); i++ )
		{
			j=(i+1)%Vertices.Num();

			if( FPointsAreNear(SidePlaneNormal[i],SidePlaneNormal[j],FLOAT_NORMAL_THRESH) )
			{
				// Eliminate colinear points.
				appMemcpy (&SidePlaneNormal[i],&SidePlaneNormal[i+1],(Vertices.Num()-(i+1)) * sizeof (FVector));
				Vertices.Remove(i,1);
				if(Vertices.Num() < 3)
				{
					// Collapsed.
					Vertices.Empty();
					Result = FALSE;
					break;
				}
				i--;
			}
			else
			{
				switch( SplitWithPlane (Vertices(i),SidePlaneNormal[i],NULL,NULL,0) )
				{
					case SP_Front:
						Result = FALSE;
						break;
					case SP_Split:
						Result = FALSE;
						break;
					// SP_BACK: Means it's convex
					// SP_COPLANAR: Means it's probably convex (numerical precision)
				}
				if(!Result)
				{
					break;
				}
			}
		}
	}

	MemMark.Pop();

	return Result; // Ok.
}

/**
 * Checks to see if the specified line intersects this poly or not.  If "Intersect" is
 * a valid pointer, it is filled in with the intersection point.
 */
UBOOL FPoly::DoesLineIntersect( FVector Start, FVector End, FVector* Intersect )
{
	// If the ray doesn't cross the plane, don't bother going any further.
	const float DistStart = FPointPlaneDist( Start, Vertices(0), Normal );
	const float DistEnd = FPointPlaneDist( End, Vertices(0), Normal );

	if( (DistStart < 0 && DistEnd < 0) || (DistStart > 0 && DistEnd > 0 ) )
	{
		return 0;
	}

	// Get the intersection of the line and the plane.
	FVector Intersection = FLinePlaneIntersection(Start,End,Vertices(0),Normal);
	if( Intersect )	*Intersect = Intersection;
	if( Intersection == Start || Intersection == End )
	{
		return 0;
	}

	// Check if the intersection point is actually on the poly.
	return OnPoly( Intersection );
}

/**
 * Checks to see if the specified vertex is on this poly.  Assumes the vertex is on the same
 * plane as the poly and that the poly is convex.
 *
 * This can be combined with FLinePlaneIntersection to perform a line-fpoly intersection test.
 */
UBOOL FPoly::OnPoly( FVector InVtx )
{
	FVector  SidePlaneNormal;
	FVector  Side;

	for( INT x = 0 ; x < Vertices.Num() ; x++ )
	{
		// Create plane perpendicular to both this side and the polygon's normal.
		Side = Vertices(x) - Vertices((x-1 < 0 ) ? Vertices.Num()-1 : x-1 );
		SidePlaneNormal = Side ^ Normal;
		SidePlaneNormal.Normalize();

		// If point is not behind all the planes created by this polys edges, it's outside the poly.
		if( FPointPlaneDist( InVtx, Vertices(x), SidePlaneNormal ) > THRESH_POINT_ON_PLANE )
		{
			return 0;
		}
	}

	return 1;
}

// Inserts a vertex into the poly at a specific position.
//
void FPoly::InsertVertex( INT InPos, FVector InVtx )
{
	check( InPos <= Vertices.Num() );

	Vertices.InsertItem(InVtx,InPos);
}

// Removes a vertex from the polygons list of vertices
//
void FPoly::RemoveVertex( FVector InVtx )
{
	Vertices.RemoveItem(InVtx);
}

/**
 * Checks to see if all the vertices on a polygon are coplanar.
 */

UBOOL FPoly::IsCoplanar()
{
	// 3 or fewer vertices is automatically coplanar

	if( Vertices.Num() < 3 )
	{
		return 1;
	}

	CalcNormal(1);

	for( INT x = 0 ; x < Vertices.Num() ; ++x )
	{
		if( !OnPlane( Vertices(x) ) )
		{
			return 0;
		}
	}

	// If we got this far, the poly has to be coplanar.

	return 1;
}

/**
 * Breaks up this polygon into separate triangles.
 *
 * NOTE: Assumes that the polygon is convex and breaks it up
 * into a triangle strip configuration (in layout only - not polygon winding).
 *
 * NOTE: It is up to the caller to make sure this original
 * polygon is removed from the brush afterwards!
 *
 * @param	InOwner		The ABrush we want to add the new triangles into
 *
 * @return	The number of triangles created
 */

INT FPoly::Triangulate( ABrush* InOwner )
{
	if( Vertices.Num() < 3 )
	{
		return 0;
	}

	FPoly* NewTriangle;
	INT Count = 0;
	UBOOL bFlip = 0;
	FVector v0, v1, v2;
	INT Front,Back;

	v0 = Vertices(0);
	v1 = Vertices(1);
	Front = 2;
	Back = Vertices.Num()-1;
	for( INT x = 2 ; x < Vertices.Num() ; ++x )
	{
		if( bFlip )
		{
			v2 = Vertices( Back );
		}
		else
		{
			v2 = Vertices( Front );
		}

		NewTriangle = ::new( InOwner->Brush->Polys->Element )FPoly();

		NewTriangle->Init();
		new(NewTriangle->Vertices) FVector(v0);
		new(NewTriangle->Vertices) FVector(v1);
		new(NewTriangle->Vertices) FVector(v2);

		NewTriangle->CalcNormal();
		NewTriangle->TextureU = TextureU;
		NewTriangle->TextureV = TextureV;
		NewTriangle->Base = Base;
		NewTriangle->PolyFlags = PolyFlags;
		NewTriangle->Material = Material;
		NewTriangle->PolyFlags &= ~PF_GeomMarked;

		Count++;

		if( bFlip )
		{
			v0 = v2;
			Back--;
		}
		else
		{
			v1 = v2;
			Front++;
		}

		bFlip = !bFlip;
	}

	debugf( TEXT("FPoly::Triangulate : Created %d triangles"), Count );
	return Count;
}

/**
 * Finds the index of the specific vertex.
 *
 * @param	InVtx	The vertex to find the index of
 *
 * @return	The index of the vertex, if found.  Otherwise INDEX_NONE.
 */

INT FPoly::GetVertexIndex( FVector& InVtx )
{
	INT idx = INDEX_NONE;

	for( INT v = 0 ; v < Vertices.Num() ; ++v )
	{
		if( Vertices(v) == InVtx )
		{
			idx = v;
			break;
		}
	}

	return idx;
}

/**
 * Computes the mid point of the polygon (in local space).
 */

FVector FPoly::GetMidPoint()
{
	FVector mid(0,0,0);

	for( INT v = 0 ; v < Vertices.Num() ; ++v )
	{
		mid += Vertices(v);
	}

	return mid / Vertices.Num();
}

/**
 * Checks to see if the specified vertex lies on this polygons plane.
 */
UBOOL FPoly::OnPlane( FVector InVtx )
{
	return ( ( FPointPlaneDist( InVtx, Vertices(0), Normal ) > -THRESH_POINT_ON_PLANE )
		&& ( FPointPlaneDist( InVtx, Vertices(0), Normal ) < THRESH_POINT_ON_PLANE ) );
}

/**
 * Split a poly and keep only the front half. Returns number of vertices, 0 if clipped away.
 */
int FPoly::Split( const FVector &Normal, const FVector &Base )
{
	// Split it.
	FPoly Front, Back;
	Front.Init();
	Back.Init();
	switch( SplitWithPlaneFast( FPlane(Base,Normal), &Front, &Back ))
	{
		case SP_Back:
			return 0;
		case SP_Split:
			*this = Front;
			return Vertices.Num();
		default:
			return Vertices.Num();
	}
}

/**
 * Compute all remaining polygon parameters (normal, etc) that are blank.
 * Returns 0 if ok, nonzero if problem.
 */
int FPoly::Finalize( ABrush* InOwner, int NoError )
{
	// Check for problems.
	Fix();
	if( Vertices.Num()<3 )
	{
		// Since we don't have enough vertices, remove this polygon from the brush
		check( InOwner );
		for( INT p = 0 ; p < InOwner->Brush->Polys->Element.Num() ; ++p )
		{
			if( InOwner->Brush->Polys->Element(p) == *this )
			{
				InOwner->Brush->Polys->Element.Remove(p);
				break;
			}
		}
	
		debugf( NAME_Warning, TEXT("FPoly::Finalize: Not enough vertices (%i)"), Vertices.Num() );
		if( NoError )
			return -1;
		else
		{
			debugf( TEXT("FPoly::Finalize: Not enough vertices (%i) : polygon removed from brush"), Vertices.Num() );
			return -2;
		}
	}

	// If no normal, compute from cross-product and normalize it.
	if( Normal.IsZero() && Vertices.Num()>=3 )
	{
		if( CalcNormal() )
		{
			debugf( NAME_Warning, TEXT("FPoly::Finalize: Normalization failed, verts=%i, size=%f"), Vertices.Num(), Normal.Size() );
			if( NoError )
				return -1;
			else
				appErrorf( LocalizeSecure(LocalizeUnrealEd("Error_FinalizeNormalizationFailed"), Vertices.Num(), Normal.Size()) );
		}
	}

	// If texture U and V coordinates weren't specified, generate them.
	if( TextureU.IsZero() && TextureV.IsZero() )
	{
		for( int i=1; i<Vertices.Num(); i++ )
		{
			TextureU = ((Vertices(0) - Vertices(i)) ^ Normal).SafeNormal();
			TextureV = (Normal ^ TextureU).SafeNormal();
			if( TextureU.SizeSquared()!=0 && TextureV.SizeSquared()!=0 )
				break;
		}
	}
	return 0;
}

/**
 * Return whether this poly and Test are facing each other.
 * The polys are facing if they are noncoplanar, one or more of Test's points is in 
 * front of this poly, and one or more of this poly's points are behind Test.
 */
int FPoly::Faces( const FPoly &Test ) const
{
	// Coplanar implies not facing.
	if( IsCoplanar( Test ) )
		return 0;

	// If this poly is frontfaced relative to all of Test's points, they're not facing.
	for( int i=0; i<Test.Vertices.Num(); i++ )
	{
		if( !IsBackfaced( Test.Vertices(i) ) )
		{
			// If Test is frontfaced relative to on or more of this poly's points, they're facing.
			for( i=0; i<Vertices.Num(); i++ )
				if( Test.IsBackfaced( Vertices(i) ) )
					return 1;
			return 0;
		}
	}
	return 0;
}

/**
* Static constructor called once per class during static initialization via IMPLEMENT_CLASS
* macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
* properties for native- only classes.
*/
void UPolys::StaticConstructor()
{
	UClass* TheClass = GetClass();
	const DWORD SkipIndexIndex = TheClass->EmitStructArrayBegin( STRUCT_OFFSET( UPolys, Element ), sizeof(FPoly) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( FPoly, Actor ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( FPoly, Material ) );
	TheClass->EmitStructArrayEnd( SkipIndexIndex );
}

/**
* Note that the object has been modified.  If we are currently recording into the 
* transaction buffer (undo/redo), save a copy of this object into the buffer and 
* marks the package as needing to be saved.
*
* @param	bAlwaysMarkDirty	if TRUE, marks the package dirty even if we aren't
*								currently recording an active undo/redo transaction
*/
void UPolys::Modify(UBOOL bAlwaysMarkDirty)
{
	Super::Modify(bAlwaysMarkDirty);

	Element.ModifyAllItems();
}

	// UObject interface.
void UPolys::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	if( Ar.IsTransacting() )
	{
		Ar << Element;
	}
	else
	{
		Element.CountBytes( Ar );
		INT DbNum=Element.Num(), DbMax=DbNum;
		Ar << DbNum << DbMax;

		UObject* ElementOwner = Element.GetOwner();
		Ar << ElementOwner;

		Element.SetOwner(ElementOwner);

		if( Ar.IsLoading() )
		{
			Element.Empty( DbNum );
			Element.AddZeroed( DbNum );
		}
		for( INT i=0; i<Element.Num(); i++ )
		{		
			Ar << Element(i);
		}
	}
}

/**
 * Fixup improper load flags on save so that the load flags are always up to date
 */
void UPolys::PreSave()
{
	Super::PreSave();

	UObject* FlagsSource = NULL;
	// Figure out what outer to copy load flags from (usually model, or the brush holding a model)
	if (GetOuter()->IsA(UModel::StaticClass()) && GetOuter()->GetOuter() && GetOuter()->GetOuter()->IsA(ABrush::StaticClass()))
	{
		FlagsSource = GetOuter()->GetOuter();
	}
	else
	{
		FlagsSource = GetOuter();
	}

	// reset our not load flags
	ClearFlags(RF_NotForEdit | RF_NotForClient | RF_NotForServer);

	// propagate the not for flags from the source
	if (FlagsSource->HasAnyFlags(RF_NotForEdit))
	{
		ClearFlags(RF_LoadForEdit);
		SetFlags(RF_NotForEdit);
	}
	if (FlagsSource->HasAnyFlags(RF_NotForClient))
	{
		ClearFlags(RF_LoadForClient);
		SetFlags(RF_NotForClient);
	}
	if (FlagsSource->HasAnyFlags(RF_NotForServer))
	{
		ClearFlags(RF_LoadForServer);
		SetFlags(RF_NotForServer);
	}
}

IMPLEMENT_CLASS(UPolys);
