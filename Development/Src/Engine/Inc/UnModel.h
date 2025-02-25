/*=============================================================================
	UnModel.h: Unreal UModel definition.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

//
// One vertex associated with a Bsp node's polygon.  Contains a vertex index
// into the level's FPoints table, and a unique number which is common to all
// other sides in the level which are cospatial with this side.
//
class FVert
{
public:
	// Variables.
	INT 	pVertex;	// Index of vertex.
	INT		iSide;		// If shared, index of unique side. Otherwise INDEX_NONE.

	/** The vertex's shadow map coordinate. */
	FVector2D ShadowTexCoord;

	/** The vertex's shadow map coordinate for the backface of the node. */
	FVector2D BackfaceShadowTexCoord;

	// Functions.
	friend FArchive& operator<< (FArchive &Ar, FVert &Vert)
	{
		// @warning BulkSerialize: FVert is serialized as memory dump
		// See TArray::BulkSerialize for detailed description of implied limitations.
		Ar << Vert.pVertex << Vert.iSide;
		Ar << Vert.ShadowTexCoord;
		Ar << Vert.BackfaceShadowTexCoord;
		return Ar;
	}
};

//
//	FBspNode
//

// Flags associated with a Bsp node.
enum EBspNodeFlags
{
	// Flags.
	NF_NotCsg			= 0x01, // Node is not a Csg splitter, i.e. is a transparent poly.
	NF_NotVisBlocking   = 0x04, // Node does not block visibility, i.e. is an invisible collision hull.
	NF_BrightCorners	= 0x10, // Temporary.
	NF_IsNew 		 	= 0x20, // Editor: Node was newly-added.
	NF_IsFront     		= 0x40, // Filter operation bounding-sphere precomputed and guaranteed to be front.
	NF_IsBack      		= 0x80, // Guaranteed back.
};

//
// FBspNode defines one node in the Bsp, including the front and back
// pointers and the polygon data itself.  A node may have 0 or 3 to (MAX_NODE_VERTICES-1)
// vertices. If the node has zero vertices, it's only used for splitting and
// doesn't contain a polygon (this happens in the editor).
//
// vNormal, vTextureU, vTextureV, and others are indices into the level's
// vector table.  iFront,iBack should be INDEX_NONE to indicate no children.
//
// If iPlane==INDEX_NONE, a node has no coplanars.  Otherwise iPlane
// is an index to a coplanar polygon in the Bsp.  All polygons that are iPlane
// children can only have iPlane children themselves, not fronts or backs.
//
struct FBspNode // 62 bytes
{
	enum {MAX_NODE_VERTICES=255};	// Max vertices in a Bsp node.
	enum {MAX_ZONES=64};			// Max zones per level.

	// Persistent information.
	FPlane		Plane;			// 16 Plane the node falls into (X, Y, Z, W).
	INT			iVertPool;		// 4  Index of first vertex in vertex pool, =iTerrain if NumVertices==0 and NF_TerrainFront.
	INT			iSurf;			// 4  Index to surface information.

	/** The index of the node's first vertex in the UModel's vertex buffer. */
	INT			iVertexIndex;

	/** The index in ULevel::ModelComponents of the UModelComponent containing this node. */
	WORD		ComponentIndex;

	/** The index of the node in the UModelComponent's Nodes array. */
	WORD		ComponentNodeIndex;

	/** The index of the element in the UModelComponent's Element array. */
	INT			ComponentElementIndex;

	// iBack:  4  Index to node in front (in direction of Normal).
	// iFront: 4  Index to node in back  (opposite direction as Normal).
	// iPlane: 4  Index to next coplanar poly in coplanar list.
	union { INT iBack; INT iChild[1]; };
	        INT iFront;
			INT iPlane;

	INT		iCollisionBound;// 4  Collision bound.

	BYTE	iZone[2];		// 2  Visibility zone in 1=front, 0=back.
	BYTE	NumVertices;	// 1  Number of vertices in node.
	BYTE	NodeFlags;		// 1  Node flags.
	INT		iLeaf[2];		// 8  Leaf in back and front, INDEX_NONE=not a leaf.

	// Functions.
	UBOOL IsCsg( DWORD ExtraFlags=0 ) const
	{
		return (NumVertices>0) && !(NodeFlags & (NF_IsNew | NF_NotCsg | ExtraFlags));
	}
	UBOOL ChildOutside( INT IniChild, UBOOL Outside, DWORD ExtraFlags=0 ) const
	{
		return IniChild ? (Outside || IsCsg(ExtraFlags)) : (Outside && !IsCsg(ExtraFlags));
	}
	friend FArchive& operator<<( FArchive& Ar, FBspNode& N );
};

//
//	FZoneProperties
//

struct FZoneProperties
{
public:
	// Variables.
	class AZoneInfo*	ZoneActor;		// Optional actor defining the zone's property.
	FLOAT				LastRenderTime;	// Most recent level TimeSeconds when rendered.
	FZoneSet			Connectivity;	// (Connect[i]&(1<<j))==1 if zone i is adjacent to zone j.
	FZoneSet			Visibility;		// (Connect[i]&(1<<j))==1 if zone i can see zone j.
};

//
//	FLeaf
//

struct FLeaf
{
	INT		iZone;          // The zone this convex volume is in.

	// Functions.
	FLeaf()
	{}
	FLeaf(INT iInZone):
		iZone(iInZone)
	{}
	friend FArchive& operator<<( FArchive& Ar, FLeaf& L )
	{
		// @warning BulkSerialize: FLeaf is serialized as memory dump
		// See TArray::BulkSerialize for detailed description of implied limitations.
		Ar << L.iZone;
		return Ar;
	}
};

//
//	FBspSurf
//

//
// One Bsp polygon.  Lists all of the properties associated with the
// polygon's plane.  Does not include a point list; the actual points
// are stored along with Bsp nodes, since several nodes which lie in the
// same plane may reference the same poly.
//
struct FBspSurf
{
public:

	UMaterialInterface*	Material;		// 4 Material.
	DWORD				PolyFlags;		// 4 Polygon flags.
	INT					pBase;			// 4 Polygon & texture base point index (where U,V==0,0).
	INT					vNormal;		// 4 Index to polygon normal.
	INT					vTextureU;		// 4 Texture U-vector index.
	INT					vTextureV;		// 4 Texture V-vector index.
	INT					iBrushPoly;		// 4 Editor brush polygon index.
	ABrush*				Actor;			// 4 Brush actor owning this Bsp surface.
	FPlane				Plane;			// 16 The plane this surface lies on.
	FLOAT				ShadowMapScale;	// 4 The number of units/lightmap texel on this surface.

	FLightingChannelContainer LightingChannels;	// 4 Lighting channels of affecting lights.

	// Functions.
	friend FArchive& operator<<( FArchive& Ar, FBspSurf& Surf );
};

// Flags describing effects and properties of a Bsp polygon.
enum EPolyFlags
{
	// Regular in-game flags.
	PF_Invisible			= 0x00000001,	// Poly is invisible.
	PF_NotSolid				= 0x00000008,	// Poly is not solid, doesn't block.
	PF_Semisolid			= 0x00000020,	// Poly is semi-solid = collision solid, Csg nonsolid.
	PF_GeomMarked			= 0x00000040,	// Geometry mode sometimes needs to mark polys for processing later.
	PF_TwoSided				= 0x00000100,	// Poly is visible from both sides.
	PF_ForceLightMap		= 0x00000200,	// Surface is going to use lightmap.
	PF_AcceptsLights		= 0x00000400,	// Whether the surface accepts lights.
	PF_AcceptsDynamicLights	= 0x00000800,	// Whether the surface accepts dynamic lights.
	PF_Portal				= 0x04000000,	// Portal between iZones.

	// Editor flags.
	PF_Memorized     		= 0x01000000,	// Editor: Poly is remembered.
	PF_Selected      		= 0x02000000,	// Editor: Poly is selected.

	// Internal.
	PF_EdProcessed 			= 0x40000000,	// FPoly was already processed in editorBuildFPolys.
	PF_EdCut       			= 0x80000000,	// FPoly has been split by SplitPolyWithPlane.

	// Combinations of flags.
	PF_NoEdit				= PF_Memorized | PF_Selected | PF_EdProcessed | PF_EdCut,
	PF_NoImport				= PF_NoEdit | PF_Memorized | PF_Selected | PF_EdProcessed | PF_EdCut,
	PF_AddLast				= PF_Semisolid | PF_NotSolid,
	PF_NoAddToBSP			= PF_EdCut | PF_EdProcessed | PF_Selected | PF_Memorized,
	PF_ModelComponentMask	= PF_ForceLightMap | PF_AcceptsLights | PF_AcceptsDynamicLights,

	PF_DefaultFlags			= PF_AcceptsLights | PF_AcceptsDynamicLights | PF_ForceLightMap,
};

struct FModelVertex
{
	FVector Position;
	FPackedNormal TangentX;
	FPackedNormal TangentZ;
	FVector2D TexCoord;
	FVector2D ShadowTexCoord;

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,FModelVertex& V);
};

/**
 * A vertex buffer for a set of BSP nodes.
 */
class FModelVertexBuffer : public FVertexBuffer
{
public:
	/** model vertex data */
	TResourceArray<FModelVertex,VERTEXBUFFER_ALIGNMENT> Vertices;

	/** Minimal initialization constructor. */
	FModelVertexBuffer(UModel* InModel);

	// FRenderResource interface.
	virtual void InitRHI();
	virtual FString GetFriendlyName() const { return TEXT("BSP vertices"); }

	/**
	* Serializer for this class
	* @param Ar - archive to serialize to
	* @param B - data to serialize
	*/
	friend FArchive& operator<<(FArchive& Ar,FModelVertexBuffer& B);

private:
	UModel* Model;
};

class FModelShadowVertexBuffer : public FShadowVertexBuffer
{
public:
	/** Minimal initialization constructor. */
	FModelShadowVertexBuffer(UModel* InModel);

	// FRenderResource interface.
	virtual void InitRHI();
	virtual FString GetFriendlyName() const { return TEXT("BSP shadow vertices"); }

private:
	UModel* Model;
};

//
//	UModel
//

enum {MAX_NODES  = 65536};
enum {MAX_POINTS = 128000};
class UModel : public UObject
{
	DECLARE_CLASS(UModel,UObject,CLASS_Intrinsic,Engine)

	// Arrays and subobjects.
	UPolys*						Polys;
	TTransArray<FBspNode>		Nodes;
	TTransArray<FVert>			Verts;
	TTransArray<FVector>		Vectors;
	TTransArray<FVector>		Points;
	TTransArray<FBspSurf>		Surfs;
	TArray<INT>					LeafHulls;
	TArray<FLeaf>				Leaves;
	TArray<INT>					PortalNodes;

	/** An index buffer for each material used by the model, containing all the nodes with that material applied. */
	TMap<UMaterialInterface*,TScopedPointer<FRawIndexBuffer32> > MaterialIndexBuffers;

	/** A vertex buffer containing the vertices for all nodes in the UModel. */
	FModelVertexBuffer VertexBuffer;

	/** The vertex factory which is used to access VertexBuffer. */
	FLocalVertexFactory VertexFactory;

    /** The node edges, used for building shadow volumes. */
	TArray<FMeshEdge> Edges;

	/** The vertex buffer used to render the shadow volumes. It also contains the vertex factory. */
	FModelShadowVertexBuffer ShadowVertexBuffer;

	/** A fence which is used to keep track of the rendering thread releasing the model resources. */
	FRenderCommandFence ReleaseResourcesFence;

	/** True if surfaces in the model have been changed without calling ULevel::CommitModelSurfaces. */
	UBOOL InvalidSurfaces;

	/** The number of vertices in the model's vertex buffer. */
	UINT NumVertices;

	// Other variables.
	UBOOL						RootOutside;
	UBOOL						Linked;
	INT							MoverLink;
	INT							NumSharedSides;
	INT							NumZones;
	FZoneProperties				Zones[FBspNode::MAX_ZONES];
	FBoxSphereBounds			Bounds;

	// Constructors.
	UModel()
	: Nodes( this )
	, Verts( this )
	, Vectors( this )
	, Points( this )
	, Surfs( this )
	, VertexBuffer( this )
	, ShadowVertexBuffer( this )
	, RootOutside( 1 )
	{
		if ( !HasAnyFlags(RF_ClassDefaultObject) )
		{
			EmptyModel( 1, 0 );
			if( GIsEditor && !GIsGame )
			{
				UpdateVertices();
			}
		}
	}
	UModel( ABrush* Owner, UBOOL InRootOutside=1 );

	/**
	 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
	 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
	 * properties for native- only classes.
	 */
	void StaticConstructor();

	// UObject interface.
	virtual void PreSave();
	virtual void Serialize( FArchive& Ar );
	virtual void PostLoad();
	virtual void PreEditUndo();
	virtual void PostEditUndo();
	virtual UBOOL Rename( const TCHAR* InName=NULL, UObject* NewOuter=NULL, ERenameFlags Flags=REN_None );
	/**
	 * Called after duplication & serialization and before PostLoad. Used to make sure UModel's FPolys
	 * get duplicated as well.
	 */
	virtual void PostDuplicate();
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();
	
	/**
	 * Used by various commandlets to purge Editor only data from the object.
	 * 
	 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
	 */
	virtual void StripData(UE3::EPlatformType TargetPlatform);

	/** 
	 * Removes all vertex data needed for shadow volume rendering 
	 */
	void RemoveShadowVolumeData();

	// UModel interface.
	virtual void Modify( UBOOL bAlwaysMarkDirty=FALSE );
	void BuildBound();
	void Transform( ABrush* Owner );
	void EmptyModel( INT EmptySurfInfo, INT EmptyPolys );
	void ShrinkModel();

	/** Begins releasing the model's resources. */
	void BeginReleaseResources();

	/** Begins initializing the model's VertexBuffer. */
	void UpdateVertices();

	/** Compute the "center" location of all the verts */
	FVector GetCenter();

	/**
	* Initialize vertex buffer data from UModel data
	*/
	void BuildVertexBuffers();

	/** Builds the data used to render shadow volumes: ShadowVertexBuffer, ShadowVertexFactory and Edges. */
	void BuildShadowData();

	// UModel transactions.
	void ModifySelectedSurfs( UBOOL UpdateMaster );
	void ModifyAllSurfs( UBOOL UpdateMaster );
	void ModifySurf( INT InIndex, UBOOL UpdateMaster );

	// UModel collision functions.
	typedef void (*PLANE_FILTER_CALLBACK )(UModel *Model, INT iNode, int Param);
	typedef void (*SPHERE_FILTER_CALLBACK)(UModel *Model, INT iNode, int IsBack, int Outside, int Param);
	FLOAT FindNearestVertex
	(
		const FVector	&SourcePoint,
		FVector			&DestPoint,
		FLOAT			MinRadius,
		INT				&pVertex
	) const;
	void PrecomputeSphereFilter
	(
		const FPlane	&Sphere
	);

	/**
	 * Update passed in array with list of nodes intersecting box.
	 *
	 * @warning: this is an approximation and may result in false positives
	 *
	 * @param			Box				Box to filter down the BSP
	 * @param	[out]	OutComponentIndices		Component indices associated with nodes
	 */
	void GetBoxIntersectingNodesAndComponents( const FBox& Box, TArray<INT>& OutNodeIndices, TArray<INT>& OutComponentIndices  ) const;

	/**
	 * Returns whether bounding box of polygon associated with passed in node intersects passed in box.
	 *
	 * @param	Node	Node to check
	 * @param	Box		Box to check node against
	 * @return	TRUE if node polygon intersects box or is entirely contained, FALSE otherwise
	 */
	UBOOL IsNodeBBIntersectingBox( const FBspNode& Node, const FBox& Box ) const;

	friend class UWorld;
	friend class UBrushComponent;
	friend class UStaticMeshComponent;
	friend class AActor;
	friend class AVolume;

private:
	UBOOL PointCheck
	(
		FCheckResult	&Result,
		AActor			*Owner,
		const FMatrix	*OwnerLocalToWorld,
		FVector			Location,
		FVector			Extent
	);
	UBOOL LineCheck
	(
		FCheckResult	&Result,
		AActor			*Owner,
		const FMatrix	*OwnerLocalToWorld,
		FVector			End,
		FVector			Start,
		FVector			Extent,
		DWORD			TraceFlags	
		);
	BYTE FastLineCheck(FVector End,FVector Start);	
};

/**
 * A set of BSP nodes which have the same material and relevant lights.
 */
class FModelElement
{
public:

	/** The model component containing this element. */
	class UModelComponent* Component;

	/** The material used by the nodes in this element. */
	class UMaterialInterface* Material;

	/** The nodes in the element. */
	TArray<WORD> Nodes;

	/** The shadow maps for this element. */
	TArray<UShadowMap2D*> ShadowMaps;

	/** The light-map for this element. */
	FLightMapRef LightMap;

	/** The statically irrelevant lights for this element. */
	TArray<FGuid> IrrelevantLights;

	/** A pointer to the index buffer holding this element's indices. */
	FIndexBuffer* IndexBuffer;

	/** The first index in the component index buffer used by this element. */
	UINT FirstIndex;

	/** The number of triangles contained by the component index buffer for this element. */
	UINT NumTriangles;

	/** The lowest vertex index used by this element. */
	UINT MinVertexIndex;

	/** The highest vertex index used by this element. */
	UINT MaxVertexIndex;

	/** The bounding box of the vertices in the element. */
	FBox BoundingBox;

	/**
	 * Minimal initialization constructor.
	 */
	FModelElement(UModelComponent* InComponent,UMaterialInterface* InMaterial);
	FModelElement() {}

	/**
	 * Serializer.
	 */
	friend FArchive& operator<<(FArchive& Ar,FModelElement& Element);
};

/**
 * Represents all the nodes in a single BSP zone.
 */
class UModelComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UModelComponent,UPrimitiveComponent,CLASS_NoExport,Engine);

	/**
	 * Minimal initialization constructor.
	 */
	UModelComponent( UModel* InModel, INT InZoneIndex, WORD InComponentIndex, DWORD MaskedSurfaceFlags, DWORD InLightingChannels, const TArray<WORD>& InNodes );
	UModelComponent() {}

	/**
	 * Commits the editor's changes to BSP surfaces.  Reconstructs rendering info based on the new surface data.
	 */
	void CommitSurfaces();

	/**
	 * Rebuilds the model's rendering info.
	 */
	void BuildRenderData();

	/**
	 * Called from the rendering thread to generate shadow volume indices for a particular light.
	 * @param IndexBuffer	Index buffer to store the generated shadow volume indices
	 * @param Light			The light to generate the shadow volume from
	 */
	void BuildShadowVolume( struct FShadowIndexBuffer &IndexBuffer, const class FLightSceneInfo* Light ) const;

	/**
	 * Frees empty elements.
	 */
	void ShrinkElements();

	/**
	 * Selects all surfaces that are part of this model component.
	 */
	void SelectAllSurfaces();

	/**
	 * Returns the lightmap resolution used for this primivite instnace in the case of it supporting texture light/ shadow maps.
	 * 0 if not supported or no static shadowing.
	 *
	 * @param Width		[out]	Width of light/shadow map
	 * @param Height	[out]	Height of light/shadow map
	 */
	virtual void GetLightMapResolution( INT& Width, INT& Height ) const;

	/**
	 * Returns the light and shadow map memory for this primite in its out variables.
	 *
	 * Shadow map memory usage is per light whereof lightmap data is independent of number of lights, assuming at least one.
	 *
	 * @param [out] LightMapMemoryUsage		Memory usage in bytes for light map (either texel or vertex) data
	 * @param [out]	ShadowMapMemoryUsage	Memory usage in bytes for shadow map (either texel or vertex) data
	 */
	virtual void GetLightAndShadowMapMemoryUsage( INT& LightMapMemoryUsage, INT& ShadowMapMemoryUsage ) const;

	/**
	 * Applies the given static lighting to the component.
	 * @param StaticLighting - The pending static lighting for the component.
	 */
	void ApplyStaticLighting(class FModelComponentStaticLighting* ComponentStaticLighting);

	// UPrimitiveComponent interface.
	virtual void UpdateBounds();
	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual UBOOL ShouldRecreateProxyOnUpdateTransform() const;
	virtual class FDecalRenderData* GenerateDecalRenderData(class FDecalState* Decal) const;
	virtual void GetStaticLightingInfo(FStaticLightingPrimitiveInfo& OutPrimitiveInfo,const TArray<ULightComponent*>& InRelevantLights,const FLightingBuildOptions& Options);
	virtual void GetStaticTriangles(FPrimitiveTriangleDefinitionInterface* PTDI) const;
	virtual void GetStreamingTextureInfo(TArray<FStreamingTexturePrimitiveInfo>& OutStreamingTextures) const;

	// UActorComponent interface.
	virtual void InvalidateLightingCache();

	// UObject interface.
	virtual void Serialize(FArchive& Ar);
	virtual void PostLoad();
	virtual void PostEditUndo();

	/**
	 * Callback used to allow object register its direct object references that are not already covered by
	 * the token stream.
	 *
	 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
	 */
	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray );

	// Accessors.

	UModel* GetModel() const { return Model; }
	INT GetZoneIndex() const { return ZoneIndex; }
	const TIndirectArray<FModelElement>& GetElements() const { return Elements; }

private:

	/** The BSP tree. */
	UModel* Model;

	/** The index of the zone in Model which this component represents. */
	INT ZoneIndex;

	/** The index of this component in the ULevel's ModelComponents array. */
	WORD ComponentIndex;

	/** The nodes which this component renders. */
	TArray<WORD> Nodes;

	/** Indices into Model->Edges of edges with an adjacent node contained by this component. */
	TArray<INT> Edges;

	/** The elements used to render the nodes. */
	TIndirectArray<FModelElement> Elements;
};
