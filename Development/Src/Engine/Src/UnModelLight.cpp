/*=============================================================================
	UnModelLight.cpp: Unreal model lighting.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnTextureLayout.h"

//
//	Definitions.
//

#define SHADOWMAP_MAX_WIDTH			1024
#define SHADOWMAP_MAX_HEIGHT		1024

#define SHADOWMAP_TEXTURE_WIDTH		512
#define SHADOWMAP_TEXTURE_HEIGHT	512

// Forward declarations.
class FBSPSurfaceStaticLighting;

/** Used to aggregate the static lighting for the BSP surfaces of a ModelComponent. */
class FModelComponentStaticLighting : public FRefCountedObject
{
public:

	/** The model component this represents. */
	UModelComponent* const Component;

	/** The component's surfaces. */
	TArray<FBSPSurfaceStaticLighting*> Surfaces;

	/** The number of surfaces remaining with incomplete static lighting. */
	INT NumIncompleteSurfaces;

	/** The lights potentially relevant to the primitive. */
	TArray<ULightComponent*> RelevantLights;

	/** Initialization constructor. */
	FModelComponentStaticLighting(UModelComponent* InComponent):
		Component(InComponent),
		NumIncompleteSurfaces(0)
	{}
};

/** The static lighting mapping for a BSP surface. */
struct FBSPSurfaceStaticLightingMapping
{
	INT SurfaceIndex;
	TArray<INT> Nodes;

	FVector TangentX;
	FVector TangentY;
	FVector TangentZ;

	FMatrix MapToWorld;
	FMatrix WorldToMap;

	TArray<ULightComponent*> RelevantLights;

	FBox BoundingBox;

	INT SizeX;
	INT SizeY;

	/** The surface's vertices. */
	TArray<FStaticLightingVertex> Vertices;

	/** The vertex indices of the surface's triangles. */
	TArray<INT> TriangleVertexIndices;
};

/** Represents a BSP surface to the static lighting system. */
class FBSPSurfaceStaticLighting : public FStaticLightingTextureMapping, public FStaticLightingMesh
{
public:

	/** The surface's static lighting mapping. */
	const FBSPSurfaceStaticLightingMapping Mapping;

	/** TRUE if the surface has complete static lighting. */
	UBOOL bComplete;

	FLightMapData2D* LightMapData;
	TMap<ULightComponent*,FShadowMapData2D*> ShadowMapData;

	/** Initialization constructor. */
	FBSPSurfaceStaticLighting(
		const FBSPSurfaceStaticLightingMapping& InMapping,
		FModelComponentStaticLighting* InComponentStaticLighting
		);

	/** Destructor. */
	~FBSPSurfaceStaticLighting();

	/** Resets the surface's static lighting data. */
	void ResetStaticLightingData();

	// FStaticLightingMesh interface.
	virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2) const;
	virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const;
	virtual FLightRayIntersection IntersectLightRay(const FVector& Start,const FVector& End,UBOOL bFindNearestIntersection) const;

	//FStaticLightingTextureMapping interface.
	virtual void Apply(FLightMapData2D* InLightMapData,const TMap<ULightComponent*,FShadowMapData2D*>& InShadowMapData);

private:

	/** The static lighting for the component which this mapping is a part of. */
	TRefCountPtr<FModelComponentStaticLighting> ComponentStaticLighting;
};

/** Sorts the BSP surfaces by descending static lighting texture size. */
struct FBSPSurfaceDescendingTextureSizeSort
{
	static inline INT Compare( const FBSPSurfaceStaticLighting* A, const FBSPSurfaceStaticLighting* B	)
	{
		return B->SizeX * B->SizeY - A->SizeX * A->SizeY;
	}
};

/**
 * Checks whether a sphere intersects a BSP node.
 * @param	Model - The BSP tree containing the node.
 * @param	NodeIndex - The index of the node in Model.
 * @param	Point - The origin of the sphere.
 * @param	Radius - The radius of the sphere.
 * @return	True if the sphere intersects the BSP node.
 */
static UBOOL SphereOnNode(UModel* Model,UINT NodeIndex,FVector Point,FLOAT Radius)
{
	FBspNode&	Node = Model->Nodes(NodeIndex);
	FBspSurf&	Surf = Model->Surfs(Node.iSurf);

	for(UINT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
	{
		// Create plane perpendicular to both this side and the polygon's normal.
		FVector	Edge = Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex) - Model->Points(Model->Verts(Node.iVertPool + ((VertexIndex + Node.NumVertices - 1) % Node.NumVertices)).pVertex),
				EdgeNormal = Edge ^ (FVector)Surf.Plane;
		FLOAT	VertexDot = Node.Plane.PlaneDot(Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex));

		// Ignore degenerate edges.
		if(Edge.SizeSquared() < 2.0f*2.0f)
			continue;

		// If point is not behind all the planes created by this polys edges, it's outside the poly.
		if(FPointPlaneDist(Point,Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex),EdgeNormal.SafeNormal()) > Radius)
			return 0;
	}

	return 1;
}

FBSPSurfaceStaticLighting::FBSPSurfaceStaticLighting(
	const FBSPSurfaceStaticLightingMapping& InMapping,
	FModelComponentStaticLighting* InComponentStaticLighting
	):
	FStaticLightingTextureMapping(this,InComponentStaticLighting->Component,InMapping.SizeX,InMapping.SizeY,1,InComponentStaticLighting->Component->bForceDirectLightMap),
	FStaticLightingMesh(InMapping.TriangleVertexIndices.Num() / 3,InMapping.Vertices.Num(),TRUE,FALSE,FALSE,InMapping.RelevantLights,InMapping.BoundingBox),
	Mapping(InMapping),
	bComplete(FALSE),
	LightMapData(NULL),
	ComponentStaticLighting(InComponentStaticLighting)
{
	// Add this surface to the component's list of surface static lighting.
	ComponentStaticLighting->Surfaces.AddItem(this);

	// Update the number of surfaces with incomplete static lighting.
	ComponentStaticLighting->NumIncompleteSurfaces++;
}

FBSPSurfaceStaticLighting::~FBSPSurfaceStaticLighting()
{
	// Free the surface's static lighting data.
	ResetStaticLightingData();

	// Remove the surface from the component's list of surface static lighting.
	ComponentStaticLighting->Surfaces.RemoveItem(this);
}

void FBSPSurfaceStaticLighting::GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2) const
{
	OutV0 = Mapping.Vertices(Mapping.TriangleVertexIndices(TriangleIndex * 3 + 0));
	OutV1 = Mapping.Vertices(Mapping.TriangleVertexIndices(TriangleIndex * 3 + 1));
	OutV2 = Mapping.Vertices(Mapping.TriangleVertexIndices(TriangleIndex * 3 + 2));
}

void FBSPSurfaceStaticLighting::GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
{
	OutI0 = Mapping.TriangleVertexIndices(TriangleIndex * 3 + 0);
	OutI1 = Mapping.TriangleVertexIndices(TriangleIndex * 3 + 1);
	OutI2 = Mapping.TriangleVertexIndices(TriangleIndex * 3 + 2);
}

FLightRayIntersection FBSPSurfaceStaticLighting::IntersectLightRay(const FVector& Start,const FVector& End,UBOOL bFindNearestIntersection) const
{
	FCheckResult Result(1.0f);

	for(INT TriangleIndex = 0;TriangleIndex < Mapping.TriangleVertexIndices.Num();TriangleIndex += 3)
	{
		const INT I0 = Mapping.TriangleVertexIndices(TriangleIndex + 0);
		const INT I1 = Mapping.TriangleVertexIndices(TriangleIndex + 1);
		const INT I2 = Mapping.TriangleVertexIndices(TriangleIndex + 2);

		const FVector& V0 = Mapping.Vertices(I0).WorldPosition;
		const FVector& V1 = Mapping.Vertices(I1).WorldPosition;
		const FVector& V2 = Mapping.Vertices(I2).WorldPosition;

		if(LineCheckWithTriangle(Result,V2,V1,V0,Start,End,End - Start))
		{
			// Setup a vertex to represent the intersection.
			FStaticLightingVertex IntersectionVertex;
			IntersectionVertex.WorldPosition = Start + (End - Start) * Result.Time;
			IntersectionVertex.WorldTangentZ = Result.Normal;
			return FLightRayIntersection(TRUE,IntersectionVertex);
		}
	}

	return FLightRayIntersection(FALSE,FStaticLightingVertex());
}

void FBSPSurfaceStaticLighting::Apply(FLightMapData2D* InLightMapData,const TMap<ULightComponent*,FShadowMapData2D*>& InShadowMapData)
{
	if(bComplete)
	{
		// Free the surface's old static lighting data.
		ResetStaticLightingData();
	}
	else
	{
		// Update the number of surfaces with incomplete static lighting.
		ComponentStaticLighting->NumIncompleteSurfaces--;
	}

	// Save the static lighting until all of the component's static lighting has been built.
	LightMapData = InLightMapData;
	ShadowMapData = InShadowMapData;
	bComplete = TRUE;

	// If all the surfaces have complete static lighting, apply the component's static lighting.
	if(ComponentStaticLighting->NumIncompleteSurfaces == 0)
	{
		ComponentStaticLighting->Component->ApplyStaticLighting(ComponentStaticLighting);
	}
}

void FBSPSurfaceStaticLighting::ResetStaticLightingData()
{
	// Free the light-map data.
	if(LightMapData)
	{
		delete LightMapData;
		LightMapData = NULL;
	}

	// Free the shadow-map data.
	for(TMap<ULightComponent*,FShadowMapData2D*>::TConstIterator ShadowMapDataIt(ShadowMapData);ShadowMapDataIt;++ShadowMapDataIt)
	{
		delete ShadowMapDataIt.Value();
	}
	ShadowMapData.Empty();
}

void UModelComponent::GetStaticLightingInfo(FStaticLightingPrimitiveInfo& OutPrimitiveInfo,const TArray<ULightComponent*>& InRelevantLights,const FLightingBuildOptions& Options)
{
	if(!HasStaticShadowing())
	{
		return;
	}

	// Create an object to represent the model's static lighting.
	FModelComponentStaticLighting* ComponentStaticLighting = new FModelComponentStaticLighting(this);

	// Make a copy of the primitive's relevant light list.
	ComponentStaticLighting->RelevantLights = InRelevantLights;

	// Generate shadow map coordinates for the nodes in the component's zone.
	for(INT SurfaceIndex = 0;SurfaceIndex < Model->Surfs.Num();SurfaceIndex++)
	{
		const FBspSurf& Surf = Model->Surfs(SurfaceIndex);
		FBSPSurfaceStaticLightingMapping SurfaceMapping;

		SurfaceMapping.SurfaceIndex = SurfaceIndex;

		// Find the surface's nodes.
		for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = Model->Nodes(Nodes(NodeIndex));
			if(Node.iSurf == SurfaceIndex)
			{
				// Add the node to the list of nodes in this surface.
				SurfaceMapping.Nodes.AddItem((WORD)Nodes(NodeIndex));
			}
		}

		if(SurfaceMapping.Nodes.Num())
		{
			// Find a plane parallel to the surface.
			FVector MapX;
			FVector MapY;
			Surf.Plane.FindBestAxisVectors(MapX,MapY);

			// Calculate the part of the plane the surface's nodes map to.
			FVector2D MinUV(WORLD_MAX,WORLD_MAX);
			FVector2D MaxUV(-WORLD_MAX,-WORLD_MAX);
			for(INT NodeIndex = 0;NodeIndex < SurfaceMapping.Nodes.Num();NodeIndex++)
			{
				const FBspNode& Node = Model->Nodes(SurfaceMapping.Nodes(NodeIndex));
				for(UINT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					FVector	Position = Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);
					FLOAT	X = MapX | Position,
							Y = MapY | Position;
					MinUV.X = Min(X,MinUV.X);
					MinUV.Y = Min(Y,MinUV.Y);
					MaxUV.X = Max(X,MaxUV.X);
					MaxUV.Y = Max(Y,MaxUV.Y);
				}
			}

			// Create a surface mapping.
			const FLOAT ShadowMapScale = Options.bPerformFullQualityBuild ? Surf.ShadowMapScale : (Surf.ShadowMapScale * 2.0f);
			SurfaceMapping.SizeX = Clamp(appCeil((MaxUV.X - MinUV.X) / ShadowMapScale),2,SHADOWMAP_MAX_WIDTH);
			SurfaceMapping.SizeY = Clamp(appCeil((MaxUV.Y - MinUV.Y) / ShadowMapScale),2,SHADOWMAP_MAX_HEIGHT);
			SurfaceMapping.WorldToMap = FMatrix(
				FPlane(MapX.X / (MaxUV.X - MinUV.X),	MapY.X / (MaxUV.Y - MinUV.Y),	Surf.Plane.X,	0),
				FPlane(MapX.Y / (MaxUV.X - MinUV.X),	MapY.Y / (MaxUV.Y - MinUV.Y),	Surf.Plane.Y,	0),
				FPlane(MapX.Z / (MaxUV.X - MinUV.X),	MapY.Z / (MaxUV.Y - MinUV.Y),	Surf.Plane.Z,	0),
				FPlane(-MinUV.X / (MaxUV.X - MinUV.X),	-MinUV.Y / (MaxUV.Y - MinUV.Y),	-Surf.Plane.W,	1)
				) *
				FScaleMatrix(FVector(
					(FLOAT)(SurfaceMapping.SizeX - 1) / (FLOAT)SurfaceMapping.SizeX,
					(FLOAT)(SurfaceMapping.SizeY - 1) / (FLOAT)SurfaceMapping.SizeY,
					1.0f
					));
			SurfaceMapping.MapToWorld = SurfaceMapping.WorldToMap.Inverse();

			// Compute the surface's tangent basis.
			SurfaceMapping.TangentX = Model->Vectors(Surf.vTextureU).SafeNormal();
			SurfaceMapping.TangentY = Model->Vectors(Surf.vTextureV).SafeNormal();
			SurfaceMapping.TangentZ = Model->Vectors(Surf.vNormal).SafeNormal();

			// Cache the surface's vertices and triangles.
			SurfaceMapping.BoundingBox.Init();
			for(INT NodeIndex = 0;NodeIndex < SurfaceMapping.Nodes.Num();NodeIndex++)
			{
				const FBspNode& Node = Model->Nodes(SurfaceMapping.Nodes(NodeIndex));
				const FVector& TextureBase = Model->Points(Surf.pBase);
				const FVector& TextureX = Model->Vectors(Surf.vTextureU);
				const FVector& TextureY = Model->Vectors(Surf.vTextureV);
				const INT BaseVertexIndex = SurfaceMapping.Vertices.Num();

				// Generate the node's vertices.
				for(UINT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					const FVert& Vert = Model->Verts(Node.iVertPool + VertexIndex);
					const FVector& VertexWorldPosition = Model->Points(Vert.pVertex);

					FStaticLightingVertex* DestVertex = new(SurfaceMapping.Vertices) FStaticLightingVertex;
					DestVertex->WorldPosition = VertexWorldPosition;
					DestVertex->TextureCoordinates[0].X = ((VertexWorldPosition - TextureBase) | TextureX) / 128.0f;
					DestVertex->TextureCoordinates[0].Y = ((VertexWorldPosition - TextureBase) | TextureY) / 128.0f;
					DestVertex->TextureCoordinates[1].X = SurfaceMapping.WorldToMap.TransformFVector(VertexWorldPosition).X + 0.5f / (FLOAT)SurfaceMapping.SizeX;
					DestVertex->TextureCoordinates[1].Y = SurfaceMapping.WorldToMap.TransformFVector(VertexWorldPosition).Y + 0.5f / (FLOAT)SurfaceMapping.SizeY;
					DestVertex->WorldTangentX = SurfaceMapping.TangentX;
					DestVertex->WorldTangentY = SurfaceMapping.TangentY;
					DestVertex->WorldTangentZ = SurfaceMapping.TangentZ;

					// Include the vertex in the surface's bounding box.
					SurfaceMapping.BoundingBox += VertexWorldPosition;
				}

				// Generate the node's vertex indices.
				for(UINT VertexIndex = 2;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					SurfaceMapping.TriangleVertexIndices.AddItem(BaseVertexIndex + 0);
					SurfaceMapping.TriangleVertexIndices.AddItem(BaseVertexIndex + VertexIndex);
					SurfaceMapping.TriangleVertexIndices.AddItem(BaseVertexIndex + VertexIndex - 1);
				}
			}

			// Find the lights relevant to this surface's nodes.
			for(INT NodeIndex = 0;NodeIndex < SurfaceMapping.Nodes.Num();NodeIndex++)
			{
				const FBspNode& Node = Model->Nodes(SurfaceMapping.Nodes(NodeIndex));
				const UBOOL IsFrontVisible = !(Surf.PolyFlags & PF_TwoSided) || Node.iZone[1] == ZoneIndex || ZoneIndex == INDEX_NONE;
				const UBOOL IsBackVisible = (Surf.PolyFlags & PF_TwoSided) && (Node.iZone[0] == ZoneIndex || ZoneIndex == INDEX_NONE);

				for(INT LightIndex = 0;LightIndex < ComponentStaticLighting->RelevantLights.Num();LightIndex++)
				{
					ULightComponent* Light = ComponentStaticLighting->RelevantLights(LightIndex);
					if ( !Light->LightingChannels.OverlapsWith(Surf.LightingChannels) )
					{
						continue;
					}
					UPointLightComponent* PointLight = Cast<UPointLightComponent>(Light);
					UDirectionalLightComponent* DirectionalLight = Cast<UDirectionalLightComponent>(Light);
					FVector4 LightPosition = Light->GetPosition();

					// Ignore lights which don't affect the node.
					if(PointLight)
					{
						UBOOL IsWithinRadius = SphereOnNode(Model,SurfaceMapping.Nodes(NodeIndex),PointLight->GetOrigin(),PointLight->Radius);

						if( !IsWithinRadius )
							continue;

						if( Surf.PolyFlags & PF_TwoSided )
						{
							if( !IsFrontVisible && !IsBackVisible )
							{
								continue;
							}
						}
						else
						{
							UBOOL BackFacing = Node.Plane.PlaneDot(PointLight->GetOrigin()) < 0.0f;

							if( !IsFrontVisible || BackFacing )
							{
								continue;
							}
						}
					}
					else if(DirectionalLight)
					{
						if( Surf.PolyFlags & PF_TwoSided )
						{
							if( !IsFrontVisible && !IsBackVisible )
							{
								continue;
							}
						}
						else
						{
							UBOOL BackFacing = (FVector(Node.Plane) | DirectionalLight->GetDirection()) > 0.0f;

							if( !IsFrontVisible || BackFacing )
							{
								continue;
							}
						}
					}

					// Add the light to the surface's relevant light list.
					SurfaceMapping.RelevantLights.AddUniqueItem(Light);
				}
			}

			// Create the object to represent the surface's mapping/mesh to the static lighting system.
			FBSPSurfaceStaticLighting* SurfaceStaticLighting = new FBSPSurfaceStaticLighting(SurfaceMapping,ComponentStaticLighting);
			OutPrimitiveInfo.Mappings.AddItem(SurfaceStaticLighting);
			OutPrimitiveInfo.Meshes.AddItem(SurfaceStaticLighting);
		}
	}

	// Sort the surfaces descending by texture size to improve texture layout.
	Sort<FBSPSurfaceStaticLighting*,FBSPSurfaceDescendingTextureSizeSort>(&ComponentStaticLighting->Surfaces(0),ComponentStaticLighting->Surfaces.Num());
}

/** A group of BSP surfaces which have the same static lighting relevance. */
class FSurfaceStaticLightingGroup
{
public:

	/** Information about a grouped surface. */
	struct FSurfaceInfo
	{
		FBSPSurfaceStaticLighting* SurfaceStaticLighting;
		UINT BaseX;
		UINT BaseY;
	};

	/** The surfaces in the group. */
	TArray<FSurfaceInfo> Surfaces;

	/** The shadow-mapped lights affecting the group. */
	TArray<ULightComponent*> ShadowMappedLights;

	/** The material used by surfaces in the group. */
	UMaterialInterface* Material;

	/** The approximate brightness of the group's light-map. */
	const INT BrightnessGroup;

	/** The layout of the group's static lighting texture. */
	FTextureLayout TextureLayout;

	/**
	 * Minimal initialization constructor.
	 */
	FSurfaceStaticLightingGroup(UINT InSizeX,UINT InSizeY,UMaterialInterface* InMaterial,INT InBrightnessGroup):
		Material(InMaterial),
		BrightnessGroup(InBrightnessGroup),
		TextureLayout(1,1,InSizeX,InSizeY)
	{}

	/**
	 * Attempts to add a surface to the group.  It may fail if the surface doesn't match the group or won't fit in the group's texture.
	 * @param SurfaceStaticLighting - The static lighting for the surface to add.
	 * @return TRUE if the surface was successfully added.
	 */
	UBOOL AddSurface(FBSPSurfaceStaticLighting* SurfaceStaticLighting)
	{
		// Check that the surface's relevant shadow-mapped lights match the group.
		if(ShadowMappedLights.Num() != SurfaceStaticLighting->ShadowMapData.Num())
		{
			return FALSE;
		}
		UBOOL bShadowMappedLightsMatch = TRUE;
		for(INT LightIndex = 0;LightIndex < ShadowMappedLights.Num();LightIndex++)
		{
			if(SurfaceStaticLighting->ShadowMapData.Find(ShadowMappedLights(LightIndex)) == NULL)
			{
				bShadowMappedLightsMatch = FALSE;
				break;
			}
		}
		if(!bShadowMappedLightsMatch)
		{
			return FALSE;
		}

		// Attempt to add the surface to the group's texture.
		UINT PaddedSurfaceBaseX = 0;
		UINT PaddedSurfaceBaseY = 0;
		if(TextureLayout.AddElement(&PaddedSurfaceBaseX,&PaddedSurfaceBaseY,SurfaceStaticLighting->SizeX,SurfaceStaticLighting->SizeY))
		{
			// The surface fits in the group's texture, add it to the group's surface list.
			FSurfaceInfo* SurfaceInfo = new(Surfaces) FSurfaceInfo;
			SurfaceInfo->SurfaceStaticLighting = SurfaceStaticLighting;

			SurfaceInfo->BaseX = PaddedSurfaceBaseX;
			SurfaceInfo->BaseY = PaddedSurfaceBaseY;

			return TRUE;
		}
		else
		{
			// The surface didn't fit in the group's texture, return failure.
			return FALSE;
		}
	}

	/**
	 * Converts the passed in max light brightness to a light classification group index.
	 * @param	MaxLightMapBrightness	Brightness to convert
	 * @return	Brightness classification
	 */
	static INT ConvertMaxLightBrightnessToClassification( FLOAT MaxLightMapBrightness )
	{
		if(MaxLightMapBrightness > DELTA)
		{
			return appRoundUpToPowerOfTwo(appCeil(MaxLightMapBrightness));
		}
		else
		{
			return 0;
		}
	}
};

void UModelComponent::ApplyStaticLighting(FModelComponentStaticLighting* ComponentStaticLighting)
{
	// Group surfaces based on their static lighting relevance.
	TArray<FSurfaceStaticLightingGroup> SurfaceGroups;
	for(INT SurfaceIndex = 0;SurfaceIndex < ComponentStaticLighting->Surfaces.Num();SurfaceIndex++)
	{
		FBSPSurfaceStaticLighting* SurfaceStaticLighting = ComponentStaticLighting->Surfaces(SurfaceIndex);
		const FBspSurf& Surf = Model->Surfs(SurfaceStaticLighting->Mapping.SurfaceIndex);

		// Calculate the surface light-map's maximum brightness.
		FLOAT MaxLightMapBrightness = 0.0f;
		for(INT Y = 0;Y < SurfaceStaticLighting->SizeY;Y++)
		{
			for(INT X = 0;X < SurfaceStaticLighting->SizeX;X++)
			{
				const FLightSample& LightMapSample = (*SurfaceStaticLighting->LightMapData)(X,Y);
				if(LightMapSample.bIsMapped)
				{
					for(INT CoefficientIndex = 0;CoefficientIndex < NUM_GATHERED_LIGHTMAP_COEF;CoefficientIndex++)
					{
						for(INT ColorIndex = 0;ColorIndex < 3;ColorIndex++)
						{
							MaxLightMapBrightness = Max(MaxLightMapBrightness,LightMapSample.Coefficients[CoefficientIndex][ColorIndex]);
						}
					}
				}
			}
		}

		// Calculate the light-map's brightness group.
		INT BrightnessGroup = FSurfaceStaticLightingGroup::ConvertMaxLightBrightnessToClassification(MaxLightMapBrightness);

		// Find an existing surface group with the same static lighting relevance.
		FSurfaceStaticLightingGroup* Group = NULL;
		for(INT GroupIndex = 0;GroupIndex < SurfaceGroups.Num();GroupIndex++)
		{
			FSurfaceStaticLightingGroup& ExistingGroup = SurfaceGroups(GroupIndex);

			// Check that the surface's light-map brightness approximately matches the group.
			// This improves light-map precision by not merging dim light-maps with bright light-maps.
			if(BrightnessGroup == ExistingGroup.BrightnessGroup)
			{
				// Attempt to add the surface to the group.
				if(ExistingGroup.AddSurface(SurfaceStaticLighting))
				{
					Group = &ExistingGroup;
					break;
				}
			}
		}

		// If the surface didn't fit in any existing group, create a new group.
		if(!Group)
		{
			// If the surface is larger than the standard group texture size, create a special group with the texture the same size as the surface.
			UINT TextureSizeX = SHADOWMAP_TEXTURE_WIDTH;
			UINT TextureSizeY = SHADOWMAP_TEXTURE_HEIGHT;
			if(SurfaceStaticLighting->SizeX > SHADOWMAP_TEXTURE_WIDTH || SurfaceStaticLighting->SizeY > SHADOWMAP_TEXTURE_HEIGHT)
			{
				TextureSizeX = (SurfaceStaticLighting->SizeX + 3) & ~3;
				TextureSizeY = (SurfaceStaticLighting->SizeY + 3) & ~3;
			}

			// Create the new group.
			Group = ::new(SurfaceGroups) FSurfaceStaticLightingGroup(TextureSizeX,TextureSizeY,Surf.Material,BrightnessGroup);

			// Initialize the group's light lists from the surface.
			for(TMap<ULightComponent*,FShadowMapData2D*>::TConstIterator ShadowMapDataIt(SurfaceStaticLighting->ShadowMapData);ShadowMapDataIt;++ShadowMapDataIt)
			{
				Group->ShadowMappedLights.AddItem(ShadowMapDataIt.Key());
			}

			// Add the surface to the new group.
			verify(Group->AddSurface(SurfaceStaticLighting));
		}
	}

	// Create an element for each surface group.
	Elements.Empty();
	for(INT GroupIndex = 0;GroupIndex < SurfaceGroups.Num();GroupIndex++)
	{
		const FSurfaceStaticLightingGroup& SurfaceGroup = SurfaceGroups(GroupIndex);
		const UINT GroupSizeX = SurfaceGroup.TextureLayout.GetSizeX();
		const UINT GroupSizeY = SurfaceGroup.TextureLayout.GetSizeY();

		// Create an element for the surface group.
		FModelElement* Element = new(Elements) FModelElement(this,SurfaceGroup.Material);

		// Create merged light-map data.
		FLightMapData2D* GroupLightMapData = new FLightMapData2D(GroupSizeX,GroupSizeY);
		for(INT SurfaceIndex = 0;SurfaceIndex < SurfaceGroup.Surfaces.Num();SurfaceIndex++)
		{
			const FSurfaceStaticLightingGroup::FSurfaceInfo& SurfaceInfo = SurfaceGroup.Surfaces(SurfaceIndex);
			const FBSPSurfaceStaticLighting* SurfaceStaticLighting = SurfaceInfo.SurfaceStaticLighting;

			// Merge the surface's light-mapped light list into the group's light-mapped light list.
			for(INT LightIndex = 0;LightIndex < SurfaceStaticLighting->LightMapData->Lights.Num();LightIndex++)
			{
				GroupLightMapData->Lights.AddUniqueItem(SurfaceStaticLighting->LightMapData->Lights(LightIndex));
			}

			// Copy the surface's light-map into the merged group light-map.
			for(INT Y = 0;Y < SurfaceStaticLighting->SizeY;Y++)
			{
				for(INT X = 0;X < SurfaceStaticLighting->SizeX;X++)
				{
					(*GroupLightMapData)(SurfaceInfo.BaseX + X,SurfaceInfo.BaseY + Y) = (*SurfaceStaticLighting->LightMapData)(X,Y);
				}
			}
		}

		// Create the element's light-map.
		Element->LightMap = FLightMap2D::AllocateLightMap(this,GroupLightMapData,SurfaceGroup.Material,Bounds);

		// Free the merged raw light-map data.
		delete GroupLightMapData;
		GroupLightMapData = NULL;

		// Allocate merged shadow-map data.
		TMap<ULightComponent*,FShadowMapData2D*> GroupShadowMapData;
		for(INT LightIndex = 0;LightIndex < SurfaceGroup.ShadowMappedLights.Num();LightIndex++)
		{
			GroupShadowMapData.Set(
				SurfaceGroup.ShadowMappedLights(LightIndex),
				new FShadowMapData2D(GroupSizeX,GroupSizeY)
				);
		}

		// Merge surface shadow-maps into the group shadow-maps.
		for(INT SurfaceIndex = 0;SurfaceIndex < SurfaceGroup.Surfaces.Num();SurfaceIndex++)
		{
			const FSurfaceStaticLightingGroup::FSurfaceInfo& SurfaceInfo = SurfaceGroup.Surfaces(SurfaceIndex);
			const FBSPSurfaceStaticLighting* SurfaceStaticLighting = SurfaceInfo.SurfaceStaticLighting;

			for(TMap<ULightComponent*,FShadowMapData2D*>::TConstIterator ShadowMapDataIt(SurfaceStaticLighting->ShadowMapData);ShadowMapDataIt;++ShadowMapDataIt)
			{
				const FShadowMapData2D* SurfaceShadowMap = ShadowMapDataIt.Value();
				FShadowMapData2D* GroupShadowMap = GroupShadowMapData.FindRef(ShadowMapDataIt.Key());
				check(GroupShadowMap);

				// Copy the surface's shadow-map into the merged group shadow-map.
				for(INT Y = 0;Y < SurfaceStaticLighting->SizeY;Y++)
				{
					for(INT X = 0;X < SurfaceStaticLighting->SizeX;X++)
					{
						(*GroupShadowMap)(SurfaceInfo.BaseX + X,SurfaceInfo.BaseY + Y) = (*SurfaceShadowMap)(X,Y);
					}
				}
			}
		}

		// Create the element's shadow-maps.
		for(TMap<ULightComponent*,FShadowMapData2D*>::TConstIterator ShadowMapDataIt(GroupShadowMapData);ShadowMapDataIt;++ShadowMapDataIt)
		{
			Element->ShadowMaps.AddItem(new(this) UShadowMap2D(
				*ShadowMapDataIt.Value(),
				ShadowMapDataIt.Key()->LightGuid,
				SurfaceGroup.Material,
				Bounds
				));

			// Free the merged raw shadow-map data.
			delete ShadowMapDataIt.Value();
		}

		// Build the list of the element's statically irrelevant lights.
		for(INT LightIndex = 0;LightIndex < ComponentStaticLighting->RelevantLights.Num();LightIndex++)
		{
			ULightComponent* Light = ComponentStaticLighting->RelevantLights(LightIndex);

			// Check if the light is stored in the light-map.
			const UBOOL bIsInLightMap = Element->LightMap && Element->LightMap->LightGuids.ContainsItem(Light->LightmapGuid);

			// Check if the light is stored in the shadow-map.
			const UBOOL bIsInShadowMap = GroupShadowMapData.Find(Light) != NULL;

			// Add the light to the statically irrelevant light list if it is in the potentially relevant light list, but didn't contribute to the light-map or a shadow-map.
			if(!bIsInLightMap && !bIsInShadowMap)
			{	
				Element->IrrelevantLights.AddUniqueItem(Light->LightGuid);
			}
		}

		// Apply the surface's static lighting mapping to its vertices.
		for(INT SurfaceIndex = 0;SurfaceIndex < SurfaceGroup.Surfaces.Num();SurfaceIndex++)
		{
			const FSurfaceStaticLightingGroup::FSurfaceInfo& SurfaceInfo = SurfaceGroup.Surfaces(SurfaceIndex);
			const FBSPSurfaceStaticLighting* SurfaceStaticLighting = SurfaceInfo.SurfaceStaticLighting;

			for(INT NodeIndex = 0;NodeIndex < SurfaceStaticLighting->Mapping.Nodes.Num();NodeIndex++)
			{
				const FBspNode& Node = Model->Nodes(SurfaceStaticLighting->Mapping.Nodes(NodeIndex));
				for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					FVert& Vert = Model->Verts(Node.iVertPool + VertexIndex);
					const FVector& WorldPosition = Model->Points(Vert.pVertex);
					const FVector4 StaticLightingTextureCoordinate = SurfaceStaticLighting->Mapping.WorldToMap.TransformFVector(WorldPosition);

					Vert.ShadowTexCoord.X = (SurfaceInfo.BaseX + StaticLightingTextureCoordinate.X * SurfaceStaticLighting->SizeX) / (FLOAT)GroupSizeX;
					Vert.ShadowTexCoord.Y = (SurfaceInfo.BaseY + StaticLightingTextureCoordinate.Y * SurfaceStaticLighting->SizeY) / (FLOAT)GroupSizeY;
				}
			}
		}

		// Add the surfaces' nodes to the element.
		for(INT SurfaceIndex = 0;SurfaceIndex < SurfaceGroup.Surfaces.Num();SurfaceIndex++)
		{
			const FSurfaceStaticLightingGroup::FSurfaceInfo& SurfaceInfo = SurfaceGroup.Surfaces(SurfaceIndex);
			const FBSPSurfaceStaticLighting* SurfaceStaticLighting = SurfaceInfo.SurfaceStaticLighting;
			for(INT NodeIndex = 0;NodeIndex < SurfaceStaticLighting->Mapping.Nodes.Num();NodeIndex++)
			{
				Element->Nodes.AddItem(SurfaceStaticLighting->Mapping.Nodes(NodeIndex));
			}
		}
	}

	// Free the surfaces' static lighting data.
	for(INT SurfaceIndex = 0;SurfaceIndex < ComponentStaticLighting->Surfaces.Num();SurfaceIndex++)
	{
		ComponentStaticLighting->Surfaces(SurfaceIndex)->ResetStaticLightingData();
	}

	// Invalidate the model's vertex buffer.
	Model->InvalidSurfaces = TRUE;

	{
		FComponentReattachContext ReattachContext(this);

		// Release the model's index buffers since they will be modified by BuildRenderData()
		for(TMap<UMaterialInterface*,TScopedPointer<FRawIndexBuffer32> >::TIterator IndexBufferIt(Model->MaterialIndexBuffers);IndexBufferIt;++IndexBufferIt)
		{
			BeginReleaseResource(IndexBufferIt.Value());
		}

		// Block until the index buffers have been released
		FlushRenderingCommands();

		// Build the render data for the new elements.
		BuildRenderData();

		// Initialize the model's index buffers.
		for(TMap<UMaterialInterface*,TScopedPointer<FRawIndexBuffer32> >::TIterator IndexBufferIt(Model->MaterialIndexBuffers);
			IndexBufferIt;
			++IndexBufferIt)
		{
			BeginInitResource(IndexBufferIt.Value());
		}
	}
	
	// Mark the model's package as dirty.
	MarkPackageDirty();
}

void UModelComponent::InvalidateLightingCache()
{
	// Save the model state for transactions.
	Modify();

	// Mark lighting as requiring a rebuilt.
	MarkLightingRequiringRebuild();

	FComponentReattachContext ReattachContext(this);

	// Clear all statically cached shadowing data.
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		FModelElement& Element = Elements(ElementIndex);
		Element.ShadowMaps.Empty();
		Element.IrrelevantLights.Empty();
		Element.LightMap = NULL;
	}
}


/**
 * Returns the lightmap resolution used for this primivite instnace in the case of it supporting texture light/ shadow maps.
 * 0 if not supported or no static shadowing.
 *
 * @param Width		[out]	Width of light/shadow map
 * @param Height	[out]	Height of light/shadow map
 */
void UModelComponent::GetLightMapResolution( INT& Width, INT& Height ) const
{
	INT LightMapArea = 0;
	for(INT SurfaceIndex = 0;SurfaceIndex < Model->Surfs.Num();SurfaceIndex++)
	{
		FBspSurf& Surf = Model->Surfs(SurfaceIndex);

		// Find a plane parallel to the surface.
		FVector MapX;
		FVector MapY;
		Surf.Plane.FindBestAxisVectors(MapX,MapY);

		// Find the surface's nodes and the part of the plane they map to.
		TArray<WORD> SurfaceNodes;
		FVector2D MinUV(WORLD_MAX,WORLD_MAX);
		FVector2D MaxUV(-WORLD_MAX,-WORLD_MAX);
		for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = Model->Nodes(Nodes(NodeIndex));
			if(Node.iSurf == SurfaceIndex)
			{
				// Add the node to the list of nodes in this surface.
				SurfaceNodes.AddItem((WORD)Nodes(NodeIndex));

				// Compute the bounds of the node's vertices on the surface plane.
				for(UINT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
				{
					FVector	Position = Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);
					FLOAT	X = MapX | Position,
							Y = MapY | Position;
					MinUV.X = Min(X,MinUV.X);
					MinUV.Y = Min(Y,MinUV.Y);
					MaxUV.X = Max(X,MaxUV.X);
					MaxUV.Y = Max(Y,MaxUV.Y);
				}
			}
		}

		// Ignore the surface if it has no nodes in this component's zone.
		if(!SurfaceNodes.Num())
		{
			continue;
		}

		// Calculate surface area of lightmap.
		INT SizeX = Clamp(appCeil((MaxUV.X - MinUV.X) / Surf.ShadowMapScale),2,SHADOWMAP_MAX_WIDTH);
		INT SizeY = Clamp(appCeil((MaxUV.Y - MinUV.Y) / Surf.ShadowMapScale),2,SHADOWMAP_MAX_HEIGHT);
		LightMapArea += SizeX * SizeY;
	}

	Width	= appTrunc( appSqrt( LightMapArea ) );
	Height	= Width;
}

/**
 * Returns the light and shadow map memory for this primite in its out variables.
 *
 * Shadow map memory usage is per light whereof lightmap data is independent of number of lights, assuming at least one.
 *
 * @param [out] LightMapMemoryUsage		Memory usage in bytes for light map (either texel or vertex) data
 * @param [out]	ShadowMapMemoryUsage	Memory usage in bytes for shadow map (either texel or vertex) data
 */
void UModelComponent::GetLightAndShadowMapMemoryUsage( INT& LightMapMemoryUsage, INT& ShadowMapMemoryUsage ) const
{
	INT LightMapWidth	= 0;
	INT LightMapHeight	= 0;
	GetLightMapResolution( LightMapWidth, LightMapHeight );
	
	// Stored in texture.
	const FLOAT MIP_FACTOR = 1.33f;
	ShadowMapMemoryUsage	= appTrunc( MIP_FACTOR * LightMapWidth * LightMapHeight ); // G8
	const UINT NumLightMapCoefficients = GSystemSettings.bAllowDirectionalLightMaps ? NUM_DIRECTIONAL_LIGHTMAP_COEF : NUM_SIMPLE_LIGHTMAP_COEF;
	LightMapMemoryUsage		= appTrunc( NumLightMapCoefficients * MIP_FACTOR * LightMapWidth * LightMapHeight / 2 ); // DXT1
	return;
}

