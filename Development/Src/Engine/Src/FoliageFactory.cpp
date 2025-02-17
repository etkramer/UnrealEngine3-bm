/*=============================================================================
	FoliageFactory.cpp: Foliage factory implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineFoliageClasses.h"
#include "UnTerrain.h"

IMPLEMENT_CLASS(AFoliageFactory);

/**
 * Generates the foliage instances on a triangle for a factory.
 * @param Factory - The factory to generate foliage for.
 * @param Primitive - The primitive to generate foliage on.
 * @param Vertex0 - The triangle's first vertex.
 * @param Vertex1 - The triangle's second vertex.
 * @param Vertex2 - The triangle's third vertex.
 * @param StaticLighting - The triangle's static lighting information.
 */
static void GenerateFoliageInstancesForTriangle(
	AFoliageFactory* Factory,
	const UPrimitiveComponent* Primitive,
	const FPrimitiveTriangleVertex& Vertex0,
	const FPrimitiveTriangleVertex& Vertex1,
	const FPrimitiveTriangleVertex& Vertex2
	)
{
	// Compute the triangle's normal.
	const FVector TriangleNormal = (Vertex2.WorldPosition - Vertex0.WorldPosition) ^ (Vertex1.WorldPosition - Vertex0.WorldPosition);

	// Compute the triangle's area.
	const FLOAT TriangleArea = 0.5f * TriangleNormal.Size();

	// Don't create foliage on large triangles; these are probably the sky dome, a water sheet, etc.
	static const FLOAT MaxTriangleArea = 16384.0f * 16384.0f * 0.5f;
	if(TriangleArea > MaxTriangleArea)
	{
		return;
	}

	// Determine the primitive's type.
	const UBOOL bPrimitiveIsBSP = Primitive->IsA(UModelComponent::StaticClass());
	const UBOOL bPrimitiveIsStaticMesh = Primitive->IsA(UStaticMeshComponent::StaticClass());
	const UBOOL bPrimitiveIsTerrain = Primitive->IsA(UTerrainComponent::StaticClass());

	// Generate foliage instances for each of the factory's foliage meshes.
	for(INT MeshIndex = 0;MeshIndex < Factory->Meshes.Num();MeshIndex++)
	{
		FFoliageMesh& FoliageMesh = Factory->Meshes(MeshIndex);

		// Check if the mesh should be created on this primitive type.
		const UBOOL bCreateInstancesOnThisPrimitiveType =
			(bPrimitiveIsBSP && FoliageMesh.bCreateInstancesOnBSP) ||
			(bPrimitiveIsStaticMesh && FoliageMesh.bCreateInstancesOnStaticMeshes) ||
			(bPrimitiveIsTerrain && FoliageMesh.bCreateInstancesOnTerrain);

		// Check that the foliage mesh parameters are valid before creating instances of it.
		const UBOOL bValidFoliageMesh = FoliageMesh.InstanceStaticMesh != NULL;

		if(bValidFoliageMesh && bCreateInstancesOnThisPrimitiveType)
		{
			// Compute the maximum number of foliage instances on the triangle, proportional to the triangle area.
			const FLOAT InstancesPerTriangle = TriangleArea / FoliageMesh.SurfaceAreaPerInstance;
			const INT NumSamples = Max(0,appTrunc(InstancesPerTriangle));

			// Calculate the fraction of the draw radius that meshes start to disappear at.
			const FLOAT MinThinningRadiusFraction = Clamp(FoliageMesh.MinThinningRadius / FoliageMesh.MaxDrawRadius,0.0f,1.0f);

			// Sample the foliage instance locations on the triangle.
			for(INT SampleIndex = 0;SampleIndex <= NumSamples;SampleIndex++)
			{
				if(SampleIndex < NumSamples || appFrand() < (InstancesPerTriangle - NumSamples))
				{
					// Choose a uniformly distributed random point on the triangle.
					const FLOAT S = 1.0f - appSqrt(appFrand());
					const FLOAT T = appFrand() * (1.0f - S);
					const FLOAT U = 1 - S - T;

					// Interpolate the position and normal of the sample point.
					const FVector SamplePosition =
						S * Vertex0.WorldPosition +
						T * Vertex1.WorldPosition +
						U * Vertex2.WorldPosition;
					const FVector SampleNormal =
						(S * Vertex0.WorldTangentZ +
						T * Vertex1.WorldTangentZ +
						U * Vertex2.WorldTangentZ).SafeNormal();

					// Check whether an instance should be created at this point on the surface.
					if(Factory->ShouldCreateInstance(SamplePosition,SampleNormal))
					{
						// Limit the number of instances for the component.
						if(FoliageMesh.Component->Instances.Num() >= Factory->MaxInstanceCount)
						{
							return;
						}

						// Create a foliage instance at this surface point.
						FGatheredFoliageInstance* Instance = new(FoliageMesh.Component->Instances) FGatheredFoliageInstance;
						Instance->Location = SamplePosition;

						// Build an orthonormal basis with +Z being the surface normal.
						FVector XAxis;
						FVector YAxis;
						SampleNormal.FindBestAxisVectors(XAxis,YAxis);

						// Rotate the basis around Z by a random amount.
						const FMatrix RotationMatrix =
							FRotationMatrix(FRotator(0,appRand(),0)) *
							FMatrix(-XAxis,YAxis,SampleNormal,FVector(0,0,0));

						// Calculate the instance scale.
						const FLOAT UniformScale = Lerp(FoliageMesh.MinUniformScale,FoliageMesh.MaxUniformScale,appFrand());
						const FLOAT ScaleX = UniformScale * Lerp(FoliageMesh.MinScale.X,FoliageMesh.MaxScale.X,appFrand());
						const FLOAT ScaleY = UniformScale * Lerp(FoliageMesh.MinScale.Y,FoliageMesh.MaxScale.Y,appFrand());
						const FLOAT ScaleZ = UniformScale * Lerp(FoliageMesh.MinScale.Z,FoliageMesh.MaxScale.Z,appFrand());

						// Build the instance's axes from its scale and rotation.
						Instance->XAxis = RotationMatrix.GetAxis(0) * ScaleX;
						Instance->YAxis = RotationMatrix.GetAxis(1) * ScaleY;
						Instance->ZAxis = RotationMatrix.GetAxis(2) * ScaleZ;

						// Choose a random factor proportional to the mesh scale that is applied to the distance-based rendering of the instance.
						const FLOAT ScaleFraction = UniformScale / FoliageMesh.MaxUniformScale;
						Instance->DistanceFactorSquared = 1.0f / Square(Lerp(MinThinningRadiusFraction,1.0f,appFrand() * ScaleFraction));

						// The static lighting will be built by the foliage component.
						for(UINT CoefficientIndex = 0; CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF; CoefficientIndex++)
						{
							Instance->StaticLighting[CoefficientIndex] = FColor(0,0,0,0);
						}
					}
				}
			}
		}
	}
}

/**
 * Clips a polygon against a plane.
 * @param Plane - The polygon's plane.
 * @param InVertices - The input polygon's vertices.
 * @param OutVertices - Upon return, contains the clipped polygon's vertices.
 */
static void ClipPolygonWithPlane(
	const FPlane& Plane,
	const TArray<FPrimitiveTriangleVertex>& InVertices,
	TArray<FPrimitiveTriangleVertex>& OutBackVertices
	)
{
	FMemMark MemMark(GMainThreadMemStack);

	enum EPlaneClassification
	{
		V_FRONT = 0,
		V_BACK = 1,
	};
	EPlaneClassification Status;
	EPlaneClassification PrevStatus;
	EPlaneClassification* VertStatus = new(GMainThreadMemStack) EPlaneClassification[InVertices.Num()];
	INT Front=0;
	INT Back=0;

	EPlaneClassification* StatusPtr = &VertStatus[0];
	for( int i=0; i<InVertices.Num(); i++ )
	{
		FLOAT Dist = Plane.PlaneDot(InVertices(i).WorldPosition);
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
	if( !Front )
	{
		OutBackVertices = InVertices;
	}
	else if(Front && Back)
	{
		// Split.

		const FPrimitiveTriangleVertex *V  = &InVertices(0);
		const FPrimitiveTriangleVertex *W  = &InVertices(InVertices.Num()-1);
		PrevStatus        = VertStatus         [InVertices.Num()-1];
		StatusPtr         = &VertStatus        [0];

		for( int i=0; i<InVertices.Num(); i++ )
		{
			Status = *StatusPtr++;
			if( Status != PrevStatus )
			{
				// Crossing.
				const FVector Edge = V->WorldPosition - W->WorldPosition;
				const FLOAT IntersectionTime = ((Plane.W - (W->WorldPosition|Plane)) / (Edge|Plane));

				FPrimitiveTriangleVertex* IntersectionVertex = new(OutBackVertices) FPrimitiveTriangleVertex;
				IntersectionVertex->WorldPosition = W->WorldPosition + Edge * IntersectionTime;
				IntersectionVertex->WorldTangentX = Lerp(W->WorldTangentX,V->WorldTangentX,IntersectionTime);
				IntersectionVertex->WorldTangentY = Lerp(W->WorldTangentY,V->WorldTangentY,IntersectionTime);
				IntersectionVertex->WorldTangentZ = Lerp(W->WorldTangentZ,V->WorldTangentZ,IntersectionTime);
			}

			if( Status==V_BACK)
			{
				new(OutBackVertices) FPrimitiveTriangleVertex(*V);
			}

			PrevStatus = Status;
			W          = V++;
		}
	}
	
	MemMark.Pop();

	if(OutBackVertices.Num() < 3)
	{
		OutBackVertices.Empty();
	}
}

/**
 * Clips a polygon with a foliage factory's volume.
 * @param Factory - The foliage factory.
 * @param InVertices - The vertices of the input polygon.
 * @param OutPolygons - Upon return, contains the clipped polygons.
 */
static void ClipPolygonWithVolume(
	const AFoliageFactory* Factory,
	const TArray<FPrimitiveTriangleVertex>& InVertices,
	TArray<TArray<FPrimitiveTriangleVertex> >& OutPolygons
	)
{
	// Clip the triangle against each of the brush's convex volumes.
	for( INT ConvexElementIndex=0; ConvexElementIndex<Factory->BrushComponent->BrushAggGeom.ConvexElems.Num(); ConvexElementIndex++ )
	{
		const FKConvexElem& ConvexElement = Factory->BrushComponent->BrushAggGeom.ConvexElems(ConvexElementIndex);

		// Start out with the unclipped polygon.
		TArray<FPrimitiveTriangleVertex> ClippedVertices = InVertices;

		// Clip the triangle against each of the element's bounding planes.
		UBOOL bBoundsIntersect = TRUE;
		for( INT PlaneIndex=0; PlaneIndex<ConvexElement.FacePlaneData.Num(); PlaneIndex++ )
		{
			// Transform the bounding plane into world-space.
			const FPlane WorldPlane = ConvexElement.FacePlaneData(PlaneIndex).TransformBy(Factory->BrushComponent->LocalToWorld);

			// Expand the plane by the factory's falloff radius.
			const FPlane ExpandedWorldPlane((FVector)WorldPlane,WorldPlane.W + Factory->VolumeFalloffRadius);

			// Clip the triangle against the expanded plane.
			TArray<FPrimitiveTriangleVertex> NewClippedVertices;
			ClipPolygonWithPlane(ExpandedWorldPlane,ClippedVertices,NewClippedVertices);
			
			// Proceed with the part of the triangle behind the plane.
			ClippedVertices = NewClippedVertices;
			if(ClippedVertices.Num() < 3)
			{
				ClippedVertices.Empty();
				break;
			}
		}

		// If any part of the polygon was inside the volume, add it to the output polygons list.
		OutPolygons.AddItem(ClippedVertices);
	}
}

/** An implementation of the triangle definition interface used to generate foliage instances upon the triangles. */
class FFoliageGeneratorTriangleDefinitionInterface : public FPrimitiveTriangleDefinitionInterface
{
public:

	/** Initialization constructor. */
	FFoliageGeneratorTriangleDefinitionInterface(AFoliageFactory* InFactory,const UPrimitiveComponent* InPrimitive):
		Factory(InFactory),
		Primitive(InPrimitive)
	{}

	// FPrimitiveTriangleDefinitionInterface
	virtual void DefineTriangle(
		const FPrimitiveTriangleVertex& Vertex0,
		const FPrimitiveTriangleVertex& Vertex1,
		const FPrimitiveTriangleVertex& Vertex2
		)
	{
		// Clip the triangle against the factory's volume.
		TArray< TArray<FPrimitiveTriangleVertex> > ClippedPolygons;
		TArray<FPrimitiveTriangleVertex> Vertices;
		Vertices.AddItem(Vertex0);
		Vertices.AddItem(Vertex1);
		Vertices.AddItem(Vertex2);
		ClipPolygonWithVolume(Factory,Vertices,ClippedPolygons);

		// Generate the foliage instances on the clipped polygons.
		for(INT PolygonIndex = 0;PolygonIndex < ClippedPolygons.Num();PolygonIndex++)
		{
			const TArray<FPrimitiveTriangleVertex>& PolygonVertices = ClippedPolygons(PolygonIndex);
			for(INT TriangleIndex = 0;TriangleIndex < PolygonVertices.Num() - 2;TriangleIndex++)
			{
				GenerateFoliageInstancesForTriangle(
					Factory,
					Primitive,
					PolygonVertices(0),
					PolygonVertices(TriangleIndex + 1),
					PolygonVertices(TriangleIndex + 2)
					);
			}
		}
	}

private:

	AFoliageFactory* const Factory;
	const UPrimitiveComponent* const Primitive;
};

/**
 * Generates the foliage instances on a primitive for a factory.
 * @param Factory - The factory to generate foliage for.
 * @param Primitive - The primitive to generate foliage on.
 */
static void GenerateFoliageInstancesForPrimitive(AFoliageFactory* Factory,const UPrimitiveComponent* Primitive)
{
	if(Primitive->bAcceptsFoliage && Factory->IsPrimitiveRelevant(Primitive))
	{
		// Use a consistent random seed for each primitive.
		appRandInit(0);

		// Enumerate the primitive's triangles.
		FFoliageGeneratorTriangleDefinitionInterface PTDI(Factory,Primitive);
		Primitive->GetStaticTriangles(&PTDI);
	}
}

UBOOL AFoliageFactory::ShouldCreateInstance(const FVector& Point,const FVector& Normal) const
{
	// Randomly offset the point with a PDF corresponding to the falloff of the volume.
	const FVector OffsetDirection = VRand();
	const FLOAT OffsetDistance = appPow(1.0f - appFrand(),VolumeFalloffExponent) * VolumeFalloffRadius;
	const FVector OffsetPoint = Point + OffsetDirection * OffsetDistance;

	// Transform the point to local space.
	const FVector LocalPoint = BrushComponent->LocalToWorld.Inverse().TransformFVector(OffsetPoint);

	// Check the point against each of the brush's convex volumes.
	UBOOL bPointIsInsideAny = FALSE;
	for( INT ConvexElementIndex=0; ConvexElementIndex<BrushComponent->BrushAggGeom.ConvexElems.Num(); ConvexElementIndex++ )
	{
		const FKConvexElem& ConvexElement = BrushComponent->BrushAggGeom.ConvexElems(ConvexElementIndex);

		// Check the point against each of the element's bounding planes.
		UBOOL bPointIsInside = TRUE;
		for( INT PlaneIndex=0; PlaneIndex<ConvexElement.FacePlaneData.Num(); PlaneIndex++ )
		{
			const FPlane& ElementPlane = ConvexElement.FacePlaneData(PlaneIndex);
			const FLOAT Distance = ElementPlane.PlaneDot(LocalPoint);

			if(Distance > 0.0f)
			{
				bPointIsInside = FALSE;
				break;
			}
		}

		// If the offset point is inside any of the volumes, spawn an instance.
		if(bPointIsInside)
		{
			bPointIsInsideAny = TRUE;
			break;
		}
	}

	FLOAT Density = 0.0f;
	if(bPointIsInsideAny)
	{
		// If the point is inside the volume, calculate the density based on its normal.
		const FLOAT UpFraction =   appPow(Max(0.0f,+Normal.Z),FacingFalloffExponent);
		const FLOAT DownFraction = appPow(Max(0.0f,-Normal.Z),FacingFalloffExponent);
		const FLOAT SideFraction = 1.0f - UpFraction - DownFraction;

		Density =
			UpFraction * SurfaceDensityUpFacing +
			DownFraction * SurfaceDensityDownFacing +
			SideFraction * SurfaceDensitySideFacing;
	}

	return appFrand() < Density;
}

UBOOL AFoliageFactory::IsPrimitiveRelevant(const UPrimitiveComponent* Primitive) const
{
	// Expand the primitive bounds by the falloff radius of the foliage volume.
	FBoxSphereBounds ExpandedBounds = FBoxSphereBounds(
		Primitive->Bounds.Origin,
		Primitive->Bounds.BoxExtent + FVector(VolumeFalloffRadius,VolumeFalloffRadius,VolumeFalloffRadius),
		Primitive->Bounds.SphereRadius + VolumeFalloffRadius
		);

	// Check the expanded primitive bounds against each of the brush's convex volumes.
	for( INT ConvexElementIndex=0; ConvexElementIndex<BrushComponent->BrushAggGeom.ConvexElems.Num(); ConvexElementIndex++ )
	{
		const FKConvexElem& ConvexElement = BrushComponent->BrushAggGeom.ConvexElems(ConvexElementIndex);

		// Check the expanded primitive bounds against each of the element's bounding planes.
		UBOOL bBoundsIntersect = TRUE;
		for( INT PlaneIndex=0; PlaneIndex<ConvexElement.FacePlaneData.Num(); PlaneIndex++ )
		{
			const FPlane WorldPlane = ConvexElement.FacePlaneData(PlaneIndex).TransformBy(BrushComponent->LocalToWorld);

			// Check both the bounding box and sphere against the plane.
			const FLOAT Distance = WorldPlane.PlaneDot(ExpandedBounds.Origin);
			const FLOAT BoxPushOut = FBoxPushOut(WorldPlane,ExpandedBounds.BoxExtent);
			if(Distance > ExpandedBounds.SphereRadius && Distance > BoxPushOut)
			{
				bBoundsIntersect = FALSE;
				break;
			}
		}

		// If the expanded primitive bounds intersect the volume, it may be affected by the factory.
		if(bBoundsIntersect)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void AFoliageFactory::RegenerateFoliageInstances()
{
	// Discard the existing foliage components.
	for(INT ComponentIndex = 0;ComponentIndex < Components.Num();ComponentIndex++)
	{
		if(Components(ComponentIndex) && Components(ComponentIndex)->IsA(UFoliageComponent::StaticClass()))
		{
			Components(ComponentIndex)->ConditionalDetach();
			Components.Remove(ComponentIndex--);
		}
	}

	// Create new foliage components.
	for(INT MeshIndex = 0;MeshIndex < Meshes.Num();MeshIndex++)
	{
		FFoliageMesh& FoliageMesh = Meshes(MeshIndex);

		FoliageMesh.Component = ConstructObject<UFoliageComponent>(UFoliageComponent::StaticClass(),this);
		FoliageMesh.Component->InstanceStaticMesh = FoliageMesh.InstanceStaticMesh;
		FoliageMesh.Component->Material = FoliageMesh.Material;
		FoliageMesh.Component->MaxDrawRadius = FoliageMesh.MaxDrawRadius;
		FoliageMesh.Component->MinTransitionRadius = FoliageMesh.MinTransitionRadius;
		FoliageMesh.Component->MinScale = FoliageMesh.MinScale * FoliageMesh.MinUniformScale;
		FoliageMesh.Component->MaxScale = FoliageMesh.MaxScale * FoliageMesh.MaxUniformScale;
		FoliageMesh.Component->SwayScale = FoliageMesh.SwayScale;

		Components.AddItem(FoliageMesh.Component);
	}

	// Find level BSP which the foliage factory may spawn instances on.
	for(INT LevelIndex = 0;LevelIndex < GWorld->Levels.Num();LevelIndex++)
	{
		ULevel* const Level = GWorld->Levels(LevelIndex);
		for(INT ComponentIndex = 0;ComponentIndex < Level->ModelComponents.Num();ComponentIndex++)
		{
			const UPrimitiveComponent* Primitive = Level->ModelComponents(ComponentIndex);
			if(Primitive)
			{
				GenerateFoliageInstancesForPrimitive(this,Primitive);
			}
		}
	}

	// Find actor primitives which the foliage factory may spawn instances on.
	for(FActorIterator ActorIt;ActorIt;++ActorIt)
	{
		for(INT ComponentIndex = 0;ComponentIndex < ActorIt->AllComponents.Num();ComponentIndex++)
		{
			const UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(ActorIt->AllComponents(ComponentIndex));
			if(Primitive)
			{
				GenerateFoliageInstancesForPrimitive(this,Primitive);
			}
		}
	}

	// Attach the new foliage components.
	ConditionalUpdateComponents(FALSE);
}

void AFoliageFactory::PostEditMove(UBOOL bFinished)
{
	Super::PostEditMove(bFinished);

	// Regenerate foliage instances after a move.
	if(bFinished)
	{
		RegenerateFoliageInstances();
	}
}

void AFoliageFactory::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	// Regenerate foliage instances after a property has changed.
	RegenerateFoliageInstances();
}

void AFoliageFactory::PostLoad()
{
	Super::PostLoad();

	// Fix foliage meshes that were created before the MinUniformScale, MaxUniformScale, and MinThinningRadius properties were added.
	if(GetLinker()->Ver() < VER_FIX_DEFAULT_FOLIAGE_PARAMETERS)
	{
		for(INT MeshIndex = 0;MeshIndex < Meshes.Num();MeshIndex++)
		{
			FFoliageMesh& Mesh = Meshes(MeshIndex);
			if(Mesh.MinUniformScale == 0.0f && Mesh.MaxUniformScale == 0.0f && Mesh.MinThinningRadius == 0.0f)
			{
				Mesh.MinUniformScale = Mesh.MaxUniformScale = 1.0f;
				Mesh.MinThinningRadius = 1024.0f;
			}
		}
	}

	// Reattach the foliage components.
	for(INT MeshIndex = 0;MeshIndex < Meshes.Num();MeshIndex++)
	{
		UFoliageComponent* FoliageComponent = Meshes(MeshIndex).Component;
		if(FoliageComponent)
		{
			Components.AddItem(FoliageComponent);
		}
	}
}
