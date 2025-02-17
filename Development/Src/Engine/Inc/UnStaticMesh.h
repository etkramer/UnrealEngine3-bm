/*=============================================================================
	UnStaticMesh.h: Static mesh class definition.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#define STATICMESH_VERSION 18

/**
 * FStaticMeshTriangle
 *
 * @warning BulkSerialize: FStaticMeshTriangle is serialized as memory dump
 * See TArray::BulkSerialize for detailed description of implied limitations.
 */
struct FStaticMeshTriangle
{
	FVector		Vertices[3];
	FVector2D	UVs[3][8];
	FColor		Colors[3];
	INT			MaterialIndex;
	INT			FragmentIndex;
	DWORD		SmoothingMask;
	INT			NumUVs;

	FVector		TangentX[3];
	FVector		TangentY[3];
	FVector		TangentZ[3];
	UBOOL		bOverrideTangentBasis;

	/** IMPORTANT: If you add a new member to this structure, remember to update the many places throughout
	       the source tree that FStaticMeshTriangles are filled with data and copied around!  Because
		   triangle arrays are often allocated with appRealloc and blindly casted, there's no point adding
		   a constructor here for initializing members. */
};

/**
 * Bulk data array of FStaticMeshTriangle
 */
struct FStaticMeshTriangleBulkData : public FUntypedBulkData
{
	/**
	 * Returns size in bytes of single element.
	 *
	 * @return Size in bytes of single element
	 */
	virtual INT GetElementSize() const;

	/**
	 * Serializes an element at a time allowing and dealing with endian conversion and backward compatiblity.
	 * 
	 * @warning BulkSerialize: FStaticMeshTriangle is serialized as memory dump
	 * See TArray::BulkSerialize for detailed description of implied limitations.
	 *
	 * @param Ar			Archive to serialize with
	 * @param Data			Base pointer to data
	 * @param ElementIndex	Element index to serialize
	 */
	virtual void SerializeElement( FArchive& Ar, void* Data, INT ElementIndex );

	/**
	 * Returns whether single element serialization is required given an archive. This e.g.
	 * can be the case if the serialization for an element changes and the single element
	 * serialization code handles backward compatibility.
	 */
	virtual UBOOL RequiresSingleElementSerialization( FArchive& Ar );
};

/** 
 * All information about a static-mesh vertex with a variable number of texture coordinates.
 * Position information is stored separately to reduce vertex fetch bandwidth in passes that only need position. (z prepass)
 */
struct FStaticMeshFullVertex
{
	FPackedNormal TangentX;
	FPackedNormal TangentZ;
	FColor Color;	

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	*/
	void Serialize(FArchive& Ar)
	{
		Ar << TangentX;
		Ar << TangentZ;
		Ar << Color;
	}
};

/** 
* 16 bit UV version of static mesh vertex
*/
template<UINT NumTexCoords>
struct TStaticMeshFullVertexFloat16UVs : public FStaticMeshFullVertex
{
	FVector2DHalf UVs[NumTexCoords];

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,TStaticMeshFullVertexFloat16UVs& Vertex)
	{
		Vertex.Serialize(Ar);
		for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
		{
			Ar << Vertex.UVs[UVIndex];
		}
		return Ar;
	}
};

/** 
* 32 bit UV version of static mesh vertex
*/
template<UINT NumTexCoords>
struct TStaticMeshFullVertexFloat32UVs : public FStaticMeshFullVertex
{
	FVector2D UVs[NumTexCoords];

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,TStaticMeshFullVertexFloat32UVs& Vertex)
	{
		Vertex.Serialize(Ar);
		for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
		{
			Ar << Vertex.UVs[UVIndex];
		}
		return Ar;
	}
};

/** The information used to build a static-mesh vertex. */
struct FStaticMeshBuildVertex
{
	FVector Position;
	FPackedNormal TangentX;
	FPackedNormal TangentY;
	FPackedNormal TangentZ;
	FVector2D UVs[MAX_TEXCOORDS];
	FColor Color;
	WORD FragmentIndex;
};

/**
* Identifies a single chunk of an index buffer
*/
struct FFragmentRange
{
	INT BaseIndex;
	INT NumPrimitives;

	friend FArchive& operator<<(FArchive& Ar,FFragmentRange& FragmentRange)
	{
		Ar << FragmentRange.BaseIndex << FragmentRange.NumPrimitives;
		return Ar;
	}
};

/**
 * A set of static mesh triangles which are rendered with the same material.
 */
class FStaticMeshElement
{
public:

	UMaterialInterface*		Material;
	/** A work area to hold the imported name during ASE importing (transient, should not be serialized) */
	FString					Name;			

	UBOOL					EnableCollision,
							OldEnableCollision,
							bEnableShadowCasting;

	UINT					FirstIndex,
							NumTriangles,
							MinVertexIndex,
							MaxVertexIndex;

	/**
	 * The index used by a StaticMeshComponent to override this element's material.  This will be the index of the element
	 * in uncooked content, but after cooking may be different from the element index due to splitting elements for platform
	 * constraints.
	 */
	INT MaterialIndex;

	TArray<FFragmentRange> Fragments;

	/** Constructor. */
	FStaticMeshElement():
		Material(NULL),
		EnableCollision(FALSE),
		OldEnableCollision(FALSE),
		bEnableShadowCasting(TRUE),
		MaterialIndex(0)
	{}

	FStaticMeshElement(UMaterialInterface* InMaterial,UINT InMaterialIndex):
		Material(InMaterial),
		EnableCollision(TRUE),
		OldEnableCollision(TRUE),
		bEnableShadowCasting(TRUE),
		MaterialIndex(InMaterialIndex)
	{
		EnableCollision = OldEnableCollision = 1;
	}

	UBOOL operator==( const FStaticMeshElement& In ) const
	{
		return Material==In.Material;
	}

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FStaticMeshElement& E)
	{
		Ar	<< E.Material
			<< E.EnableCollision
			<< E.OldEnableCollision
			<< E.bEnableShadowCasting
			<< E.FirstIndex
			<< E.NumTriangles
			<< E.MinVertexIndex
			<< E.MaxVertexIndex
			<< E.MaterialIndex;

		if (Ar.Ver() >= VER_STATICMESH_FRAGMENTINDEX)
		{
			Ar << E.Fragments;
		}

		return Ar;
	}
};

/** An interface to the static-mesh vertex data storage type. */
class FStaticMeshVertexDataInterface
{
public:

	/** Virtual destructor. */
	virtual ~FStaticMeshVertexDataInterface() {}

	/**
	* Resizes the vertex data buffer, discarding any data which no longer fits.
	* @param NumVertices - The number of vertices to allocate the buffer for.
	*/
	virtual void ResizeBuffer(UINT NumVertices) = 0;

	/** @return The stride of the vertex data in the buffer. */
	virtual UINT GetStride() const = 0;

	/** @return A pointer to the data in the buffer. */
	virtual BYTE* GetDataPointer() = 0;

	/** @return A pointer to the FResourceArrayInterface for the vertex data. */
	virtual FResourceArrayInterface* GetResourceArray() = 0;

	/** Serializer. */
	virtual void Serialize(FArchive& Ar) = 0;
};

/** A vertex that stores just position. */
struct FPositionVertex
{
	FVector	Position;

	friend FArchive& operator<<(FArchive& Ar,FPositionVertex& V)
	{
		Ar << V.Position;
		return Ar;
	}
};

/** A vertex buffer of positions. */
class FPositionVertexBuffer : public FVertexBuffer
{
public:

	/** Default constructor. */
	FPositionVertexBuffer();

	/** Destructor. */
	~FPositionVertexBuffer();

	/** Delete existing resources */
	void CleanUp();

	/**
	* Initializes the buffer with the given vertices, used to convert legacy layouts.
	* @param InVertices - The vertices to initialize the buffer with.
	*/
	void Init(const TArray<FStaticMeshBuildVertex>& InVertices);

	/**
	* Removes the cloned vertices used for extruding shadow volumes.
	* @param NumVertices - The real number of static mesh vertices which should remain in the buffer upon return.
	*/
	void RemoveShadowVolumeVertices(UINT InNumVertices);

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FPositionVertexBuffer& VertexBuffer);

	/**
	* Specialized assignment operator, only used when importing LOD's. 
	*/
	void operator=(const FPositionVertexBuffer &Other);

	// Vertex data accessors.
	FORCEINLINE FVector& VertexPosition(UINT VertexIndex)
	{
		checkSlow(VertexIndex < GetNumVertices());
		return ((FPositionVertex*)(Data + VertexIndex * Stride))->Position;
	}
	FORCEINLINE const FVector& VertexPosition(UINT VertexIndex) const
	{
		checkSlow(VertexIndex < GetNumVertices());
		return ((FPositionVertex*)(Data + VertexIndex * Stride))->Position;
	}
	// Other accessors.
	FORCEINLINE UINT GetStride() const
	{
		return Stride;
	}
	FORCEINLINE UINT GetNumVertices() const
	{
		return NumVertices;
	}

	// FRenderResource interface.
	virtual void InitRHI();
	virtual FString GetFriendlyName() const { return TEXT("PositionOnly Static-mesh vertices"); }

private:

	/** The vertex data storage type */
	class FPositionVertexData* VertexData;

	/** The cached vertex data pointer. */
	BYTE* Data;

	/** The cached vertex stride. */
	UINT Stride;

	/** The cached number of vertices. */
	UINT NumVertices;

	/** Allocates the vertex data storage type. */
	void AllocateData();
};

/** Vertex buffer for a static mesh LOD */
class FStaticMeshVertexBuffer : public FVertexBuffer
{
public:

	/** Default constructor. */
	FStaticMeshVertexBuffer();

	/** Destructor. */
	~FStaticMeshVertexBuffer();

	/** Delete existing resources */
	void CleanUp();

	/**
	 * Initializes the buffer with the given vertices.
	 * @param InVertices - The vertices to initialize the buffer with.
	 * @param InNumTexCoords - The number of texture coordinate to store in the buffer.
	 */
	void Init(const TArray<FStaticMeshBuildVertex>& InVertices,UINT InNumTexCoords);

	/**
	 * Removes the cloned vertices used for extruding shadow volumes.
	 * @param NumVertices - The real number of static mesh vertices which should remain in the buffer upon return.
	 */
	void RemoveShadowVolumeVertices(UINT NumVertices);

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FStaticMeshVertexBuffer& VertexBuffer);

	/**
	* Specialized assignment operator, only used when importing LOD's. 
	*/
	void operator=(const FStaticMeshVertexBuffer &Other);

	FORCEINLINE FPackedNormal& VertexTangentX(UINT VertexIndex)
	{
		checkSlow(VertexIndex < GetNumVertices());
		return ((FStaticMeshFullVertex*)(Data + VertexIndex * Stride))->TangentX;
	}
	FORCEINLINE const FPackedNormal& VertexTangentX(UINT VertexIndex) const
	{
		checkSlow(VertexIndex < GetNumVertices());
		return ((FStaticMeshFullVertex*)(Data + VertexIndex * Stride))->TangentX;
	}

	/**
	* Calculate the binormal (TangentY) vector using the normal,tangent vectors
	*
	* @param VertexIndex - index into the vertex buffer
	* @return binormal (TangentY) vector
	*/
	FORCEINLINE FVector VertexTangentY(UINT VertexIndex) const
	{
		const FPackedNormal& TangentX = VertexTangentX(VertexIndex);
		const FPackedNormal& TangentZ = VertexTangentZ(VertexIndex);
		return (FVector(TangentZ) ^ FVector(TangentX)) * ((FLOAT)TangentZ.Vector.W  / 127.5f - 1.0f);
	}

	FORCEINLINE FPackedNormal& VertexTangentZ(UINT VertexIndex)
	{
		checkSlow(VertexIndex < GetNumVertices());
		return ((FStaticMeshFullVertex*)(Data + VertexIndex * Stride))->TangentZ;
	}
	FORCEINLINE const FPackedNormal& VertexTangentZ(UINT VertexIndex) const
	{
		checkSlow(VertexIndex < GetNumVertices());
		return ((FStaticMeshFullVertex*)(Data + VertexIndex * Stride))->TangentZ;
	}

	FORCEINLINE FColor& VertexColor(UINT VertexIndex)
	{
		checkSlow(VertexIndex < GetNumVertices());
		return ((FStaticMeshFullVertex*)(Data + VertexIndex * Stride))->Color;
	}

	FORCEINLINE const FColor& VertexColor(UINT VertexIndex) const
	{
		checkSlow(VertexIndex < GetNumVertices());
		return ((FStaticMeshFullVertex*)(Data + VertexIndex * Stride))->Color;
	}

	/**
	* Set the vertex UV values at the given index in the vertex buffer
	*
	* @param VertexIndex - index into the vertex buffer
	* @param UVIndex - [0,MAX_TEXCOORDS] value to index into UVs array
	* @param Vec2D - UV values to set
	*/
	FORCEINLINE void SetVertexUV(UINT VertexIndex,UINT UVIndex,const FVector2D& Vec2D)
	{
		checkSlow(VertexIndex < GetNumVertices());
		if( !bUseFullPrecisionUVs )
		{
			((TStaticMeshFullVertexFloat16UVs<MAX_TEXCOORDS>*)(Data + VertexIndex * Stride))->UVs[UVIndex] = Vec2D;
		}
		else
		{
			((TStaticMeshFullVertexFloat32UVs<MAX_TEXCOORDS>*)(Data + VertexIndex * Stride))->UVs[UVIndex] = Vec2D;
		}		
	}

	/**
	* Fet the vertex UV values at the given index in the vertex buffer
	*
	* @param VertexIndex - index into the vertex buffer
	* @param UVIndex - [0,MAX_TEXCOORDS] value to index into UVs array
	* @param 2D UV values
	*/
	FORCEINLINE FVector2D GetVertexUV(UINT VertexIndex,UINT UVIndex) const
	{
		checkSlow(VertexIndex < GetNumVertices());
		if( !bUseFullPrecisionUVs )
		{
			return ((TStaticMeshFullVertexFloat16UVs<MAX_TEXCOORDS>*)(Data + VertexIndex * Stride))->UVs[UVIndex];
		}
		else
		{
			return ((TStaticMeshFullVertexFloat32UVs<MAX_TEXCOORDS>*)(Data + VertexIndex * Stride))->UVs[UVIndex];
		}		
	}

	// Other accessors.
	FORCEINLINE UINT GetStride() const
	{
		return Stride;
	}
	FORCEINLINE UINT GetNumVertices() const
	{
		return NumVertices;
	}
	FORCEINLINE UINT GetNumTexCoords() const
	{
		return NumTexCoords;
	}
	FORCEINLINE UBOOL GetUseFullPrecisionUVs() const
	{
		return bUseFullPrecisionUVs;
	}
	FORCEINLINE void SetUseFullPrecisionUVs(UBOOL UseFull)
	{
		bUseFullPrecisionUVs = UseFull;
	}
	const BYTE* GetRawVertexData() const
	{
		check( Data != NULL );
		return Data;
	}

	/**
	* Convert the existing data in this mesh from 16 bit to 32 bit UVs.
	* Without rebuilding the mesh (loss of precision)
	*/
	template<INT NumTexCoords>
	void ConvertToFullPrecisionUVs();

	// FRenderResource interface.
	virtual void InitRHI();
	virtual FString GetFriendlyName() const { return TEXT("Static-mesh vertices"); }

private:

	/** The vertex data storage type */
	FStaticMeshVertexDataInterface* VertexData;

	/** The number of texcoords/vertex in the buffer. */
	UINT NumTexCoords;

	/** The cached vertex data pointer. */
	BYTE* Data;

	/** The cached vertex stride. */
	UINT Stride;

	/** The cached number of vertices. */
	UINT NumVertices;

	/** Corresponds to UStaticMesh::UseFullPrecisionUVs. if TRUE then 32 bit UVs are used */
	UBOOL bUseFullPrecisionUVs;

	/** Allocates the vertex data storage type. */
	void AllocateData();
};

/** 
* A vertex that stores a shadow volume extrusion info. 
*/
struct FStaticMeshShadowExtrusionVertex
{
	FLOAT ShadowExtrusionPredicate;

	friend FArchive& operator<<(FArchive& Ar,FStaticMeshShadowExtrusionVertex& V)
	{
		Ar << V.ShadowExtrusionPredicate;
		return Ar;
	}
};

/** 
* A vertex buffer with shadow volume extrusion data. 
*/
class FShadowExtrusionVertexBuffer : public FVertexBuffer
{
public:

	/** 
	* Default constructor. 
	*/
	FShadowExtrusionVertexBuffer();

	/** 
	* Destructor. 
	*/
	~FShadowExtrusionVertexBuffer();

	/** 
	* Delete existing resources 
	*/
	void CleanUp();

	/**
	* Initializes the buffer with the given vertices, used to convert legacy layouts.
	*
	* @param InVertices - The vertices to initialize the buffer with.
	*/
	void Init(const class FLegacyStaticMeshVertexBuffer& InVertexBuffer);

	/**
	* Initializes the buffer with the given vertices, used to convert legacy layouts.
	*
	* @param InVertices - The vertices to initialize the buffer with.
	*/
	void Init(const TArray<FStaticMeshBuildVertex>& InVertices);

	/**
	* Removes the cloned vertices used for extruding shadow volumes.
	*
	* @param NumVertices - The real number of static mesh vertices which should remain in the buffer upon return.
	*/
	void RemoveShadowVolumeVertices(UINT InNumVertices);

	/** 
	* Serializer. 
	* 
	* @param Ar - archive to serialize with
	* @param VertexBuffer - data to serialize to/from
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,FShadowExtrusionVertexBuffer& VertexBuffer);

	/**
	* Specialized assignment operator, only used when importing LOD's. 
	*
	* @param Other - instance to copy from
	*/
	void operator=(const FShadowExtrusionVertexBuffer &Other);

	// Vertex data accessors.
	FORCEINLINE FLOAT& VertexShadowExtrusionPredicate(UINT VertexIndex)
	{
		checkSlow(VertexIndex < GetNumVertices());
		return ((FStaticMeshShadowExtrusionVertex*)(Data + VertexIndex * Stride))->ShadowExtrusionPredicate;
	}
	FORCEINLINE const FLOAT& VertexShadowExtrusionPredicate(UINT VertexIndex) const
	{
		checkSlow(VertexIndex < GetNumVertices());
		return ((FStaticMeshShadowExtrusionVertex*)(Data + VertexIndex * Stride))->ShadowExtrusionPredicate;
	}
	// Other accessors.
	FORCEINLINE UINT GetStride() const
	{
		return Stride;
	}
	FORCEINLINE UINT GetNumVertices() const
	{
		return NumVertices;
	}

	// FRenderResource interface.

	/** 
	* Initialize the vertex buffer rendering resource. Called by the render thread
	*/ 
	virtual void InitRHI();

private:

	/** The vertex data storage type */
	class FStaticMeshShadowExtrusionVertexData* VertexData;

	/** The cached vertex data pointer. */
	BYTE* Data;

	/** The cached vertex stride. */
	UINT Stride;

	/** The cached number of vertices. */
	UINT NumVertices;

	/** Allocates the vertex data storage type. */
	void AllocateData();
};

#include "UnkDOP.h"

/**
* FStaticMeshRenderData - All data to define rendering properties for a certain LOD model for a mesh.
*/
class FStaticMeshRenderData
{
public:

	/** The buffer containing vertex data. */
	FStaticMeshVertexBuffer VertexBuffer;
	/** The buffer containing the position vertex data. */
	FPositionVertexBuffer PositionVertexBuffer;
	/** The buffer containing the shadow volume extrusion factors */
	FShadowExtrusionVertexBuffer ShadowExtrusionVertexBuffer;

	/** The number of vertices in the LOD. */
	UINT NumVertices;
	/** Index buffer resource for rendering */
	FRawStaticIndexBuffer					IndexBuffer;
	/** Index buffer resource for rendering wireframe mode */
	FRawIndexBuffer							WireframeIndexBuffer;
	/** Index buffer resource for rendering wireframe mode */
	TArray<FStaticMeshElement>				Elements;
	TArray<FMeshEdge>						Edges;
	TArray<BYTE>							ShadowTriangleDoubleSided;
	/** Source data for mesh */
	FStaticMeshTriangleBulkData				RawTriangles;

	/**
	 * Special serialize function passing the owning UObject along as required by FUnytpedBulkData
	 * serialization.
	 *
	 * @param	Ar		Archive to serialize with
	 * @param	Owner	UObject this structure is serialized within
	 * @param	Idx		Index of current array entry being serialized
	 */
	void Serialize( FArchive& Ar, UObject* Owner, INT Idx );

	/** Constructor */
	FStaticMeshRenderData();

	/** @return The triangle count of this LOD. */
	INT GetTriangleCount() const;

	/**
	* Fill an array with triangles which will be used to build a KDOP tree
	* @param kDOPBuildTriangles - the array to fill
	*/
	void GetKDOPTriangles(TArray<FkDOPBuildCollisionTriangle<WORD> >& kDOPBuildTriangles);

	/**
	* Build rendering data from a raw triangle stream
	* @param kDOPBuildTriangles output collision tree. A dummy can be passed if you do not specify BuildKDop as TRUE
	* @param Whether to build and return a kdop tree from the mesh data
	* @param Parent Parent mesh
	*/
	void Build(TArray<FkDOPBuildCollisionTriangle<WORD> >& kDOPBuildTriangles, UBOOL BuildKDop, class UStaticMesh* Parent);

	/**
	 * Initialize the LOD's render resources.
	 * @param Parent Parent mesh
	 */
	void InitResources(class UStaticMesh* Parent);

	/** Releases the LOD's render resources. */
	void ReleaseResources();

	// Rendering data.
	FLocalVertexFactory VertexFactory;
	FLocalShadowVertexFactory ShadowVertexFactory;
};

/** Used to expose information in the editor for one section or a particular LOD. */
struct FStaticMeshLODElement
{
	/** Material to use for this section of this LOD. */
	UMaterialInterface*	Material;

	/** Whether to enable shadow casting for this section of this LOD. */
	UBOOL bEnableShadowCasting;

	/** Whether to enable collision for this section of this LOD */
	BITFIELD bEnableCollision:1;

	friend FArchive& operator<<(FArchive& Ar,FStaticMeshLODElement& LODElement)
	{
		// For GC - serialise pointer to material
		if( !Ar.IsLoading() && !Ar.IsSaving() )
		{
			Ar << LODElement.Material;
		}
		return Ar;
	}
};

/**
 * FStaticMeshLODInfo - Editor-exposed properties for a specific LOD
 */
struct FStaticMeshLODInfo
{
	/** Used to expose properties for each */
	TArray<FStaticMeshLODElement>			Elements;

	friend FArchive& operator<<(FArchive& Ar,FStaticMeshLODInfo& LODInfo)
	{
		// For GC - serialise pointer to materials
		if( !Ar.IsLoading() && !Ar.IsSaving() )
		{
			Ar << LODInfo.Elements;
		}
		return Ar;
	}
};

//
//	UStaticMesh
//

class UStaticMesh : public UObject
{
	DECLARE_CLASS(UStaticMesh,UObject,CLASS_SafeReplace|CLASS_CollapseCategories|CLASS_Intrinsic,Engine);
public:
	/** Array of LODs, holding their associated rendering and collision data */
	TIndirectArray<FStaticMeshRenderData>	LODModels;
	/** Per-LOD information exposed to the editor */
	TArray<FStaticMeshLODInfo>				LODInfo;
	/** LOD distance ratio for this mesh */
	FLOAT									LODDistanceRatio;
	/** Range at which only the lowest detail LOD can be displayed */
	FLOAT									LODMaxRange;
	FRotator								ThumbnailAngle;
	FLOAT									ThumbnailDistance;

	INT										LightMapResolution,
											LightMapCoordinateIndex;
	
	// Collision data.

	typedef TkDOPTree<class FStaticMeshCollisionDataProvider,WORD> kDOPTreeType;
	kDOPTreeType							kDOPTree;

	URB_BodySetup*							BodySetup;
	FBoxSphereBounds						Bounds;

	/** Array of physics-engine shapes that can be used by multiple StaticMeshComponents. */
	TArray<void*>							PhysMesh;

	/** Scale of each PhysMesh entry. Arrays should be same size. */
	TArray<FVector>							PhysMeshScale3D;

	// Artist-accessible options.

	UBOOL									UseSimpleLineCollision,
											UseSimpleBoxCollision,
											UseSimpleRigidBodyCollision,
											DoubleSidedShadowVolumes,
											UseFullPrecisionUVs;
	UBOOL									bUsedForInstancing;

	/** List of tags describing this mesh that artists can use to find assets easier. */
	TArray<FName>							ContentTags;

	INT										InternalVersion;

	/** The cached streaming texture factors.  If the array doesn't have MAX_TEXCOORDS entries in it, the cache is outdated. */
	TArray<FLOAT> CachedStreamingTextureFactors;

	/** A fence which is used to keep track of the rendering thread releasing the static mesh resources. */
	FRenderCommandFence ReleaseResourcesFence;


	/**
	 * For simplified meshes, this is the fully qualified path and name of the static mesh object we were
	 * originally duplicated from.  This is serialized to disk, but is discarded when cooking for consoles.
	 */
	FString HighResSourceMeshName;

	/** For simplified meshes, this is the CRC of the high res mesh we were originally duplicated from. */
	DWORD HighResSourceMeshCRC;


	// UObject interface.

	void StaticConstructor();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	virtual void PreEditChange(UProperty* PropertyAboutToChange);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void Serialize(FArchive& Ar);
	virtual void PostLoad();
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();
	virtual UBOOL Rename( const TCHAR* NewName=NULL, UObject* NewOuter=NULL, ERenameFlags Flags=REN_None );

	/**
	* Used by the cooker to pre-cache the convex data for a given static mesh.  
	* This data is stored with the level.
	* @param Level - the level the data is cooked for
	* @param TotalScale3D - the scale to cook the data at
	* @param Owner - owner of this mesh for debug purposes (can be NULL)
	* @param HullByteCount - running total of memory usage for hull cache
	* @param HullCount - running count of hull cache
	*/
	void CookPhysConvexDataForScale(ULevel* Level, const FVector& TotalScale3D, const AActor* Owner, INT& TriByteCount, INT& TriMeshCount, INT& HullByteCount, INT& HullCount);

	/** Set the physics triangle mesh representations owned by this StaticMesh to be destroyed. */
	void ClearPhysMeshCache();

	/**
	 * Used by various commandlets to purge Editor only data from the object.
	 * 
	 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
	 */
	virtual void StripData(UE3::EPlatformType TargetPlatform);

	/**
	 * Called after duplication & serialization and before PostLoad. Used to e.g. make sure UStaticMesh's UModel
	 * gets copied as well.
	 */
	virtual void PostDuplicate();

	/**
	 * Callback used to allow object register its direct object references that are not already covered by
	 * the token stream.
	 *
	 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
	 */
	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray );

	// UStaticMesh interface.

	void Build();

	/**
	 * Initialize the static mesh's render resources.
	 */
	virtual void InitResources();

	/**
	 * Releases the static mesh's render resources.
	 */
	virtual void ReleaseResources();

	/**
	 * Returns the scale dependent texture factor used by the texture streaming code.	
	 *
	 * @param RequestedUVIndex UVIndex to look at
	 * @return scale dependent texture factor
	 */
	FLOAT GetStreamingTextureFactor( INT RequestedUVIndex );

	/** 
	 * Returns a one line description of an object for viewing in the generic browser
	 */
	virtual FString GetDesc();

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT InIndex );

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	virtual INT GetResourceSize();

	/** 
	* Removes all vertex data needed for shadow volume rendering 
	*/
	void RemoveShadowVolumeData();

	/**
	 * Attempts to load this mesh's high res source static mesh
	 *
	 * @return The high res source mesh, or NULL if it failed to load
	 */
	UStaticMesh* LoadHighResSourceMesh() const;

	/**
	 * Computes a CRC for this mesh to be used with mesh simplification tests.
	 *
	 * @return Simplified CRC for this mesh
	 */
	DWORD ComputeSimplifiedCRCForMesh() const;


protected:

	/** 
	 * Index of an element to ignore while gathering streaming texture factors.
	 * This is useful to disregard automatically generated vertex data which breaks texture factor heuristics.
	 */
	INT	ElementToIgnoreForTexFactor;
};

struct FStaticMeshComponentLODInfo
{
	TArray<UShadowMap2D*> ShadowMaps;
	TArray<UShadowMap1D*> ShadowVertexBuffers;
	FLightMapRef LightMap;

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FStaticMeshComponentLODInfo& I)
	{
		return Ar << I.ShadowMaps << I.ShadowVertexBuffers << I.LightMap;
	}
};

//
//	UStaticMeshComponent
//

class UStaticMeshComponent : public UMeshComponent
{
	DECLARE_CLASS(UStaticMeshComponent,UMeshComponent,CLASS_NoExport,Engine);
public:
	UStaticMeshComponent();

	/** Force drawing of a specific lodmodel-1. 0 is automatic selection */
	INT									ForcedLodModel;
	/** LOD that was desired for rendering this StaticMeshComponent last frame. */
	INT									PreviousLODLevel;

	UStaticMesh* StaticMesh;
	FColor WireframeColor;

	/** 
	 *	Ignore this instance of this static mesh when calculating streaming information. 
	 *	This can be useful when doing things like applying character textures to static geometry, 
	 *	to avoid them using distance-based streaming.
	 */
	BITFIELD bIgnoreInstanceForTextureStreaming:1;

	/** Whether to override the lightmap resolution defined in the static mesh */
	BITFIELD bOverrideLightMapResolution:1;
	/** Light map resolution used if bOverrideLightMapResolution is TRUE */ 
	INT	OverriddenLightMapResolution;

	/** Subdivision step size for static vertex lighting.				*/
	INT	SubDivisionStepSize;
	/** Minimum number of subdivisions, needs to be at least 2.			*/
	INT MinSubDivisions;
	/** Maximum number of subdivisions.									*/
	INT MaxSubDivisions;
	/** Whether to use subdivisions or just the triangle's vertices.	*/
	BITFIELD bUseSubDivisions:1;
	/** if True then decals will always use the fast path and will be treated as static wrt this mesh */
	BITFIELD bForceStaticDecals:1;

	TArray<FGuid> IrrelevantLights;	// Statically irrelevant lights.

	/** Per-LOD instance information */
	TArray<FStaticMeshComponentLODInfo> LODData;
	
	// UStaticMeshComponent interface

	virtual UBOOL SetStaticMesh(UStaticMesh* NewMesh, UBOOL bForce=FALSE);

	/**
	* Changes the value of bForceStaticDecals.
	* @param bInForceStaticDecals - The value to assign to bForceStaticDecals.
	*/
	virtual void SetForceStaticDecals(UBOOL bInForceStaticDecals);

	/**
	 * Returns whether this primitive only uses unlit materials.
	 *
	 * @return TRUE if only unlit materials are used for rendering, false otherwise.
	 */
	virtual UBOOL UsesOnlyUnlitMaterials() const;

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
	 *	UStaticMeshComponent::GetMaterial
	 * @param MaterialIndex Index of material
	 * @param LOD Lod level to query from
	 * @return Material instance for this component at index
	 */
	virtual UMaterialInterface* GetMaterial(INT MaterialIndex, INT LOD) const;

	/**
	* Intersects a line with this component
	*
	* @param	LODIndex				LOD to check against
	*/
	virtual UBOOL LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags, UINT LODIndex);

	/** Initializes the resources used by the static mesh component. */
	private: void InitResources(); public:

	// UMeshComponent interface.
	virtual INT GetNumElements() const;
	virtual UMaterialInterface* GetMaterial(INT MaterialIndex) const;

	// UPrimitiveComponent interface.
	virtual class FDecalRenderData* GenerateDecalRenderData(class FDecalState* Decal) const;
	virtual void GetStaticLightingInfo(FStaticLightingPrimitiveInfo& OutPrimitiveInfo,const TArray<ULightComponent*>& InRelevantLights,const FLightingBuildOptions& Options);

	/** Allocates an implementation of FStaticLightingMesh that will handle static lighting for this component */
	virtual class FStaticMeshStaticLightingMesh* AllocateStaticLightingMesh(INT LODIndex, const TArray<ULightComponent*>& InRelevantLights);
	virtual void GetStaticTriangles(FPrimitiveTriangleDefinitionInterface* PTDI) const;
	virtual void GetStreamingTextureInfo(TArray<FStreamingTexturePrimitiveInfo>& OutStreamingTextures) const;

	virtual UBOOL PointCheck(FCheckResult& Result,const FVector& Location,const FVector& Extent,DWORD TraceFlags);
	virtual UBOOL LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags);

	virtual void UpdateBounds();

	virtual void InitComponentRBPhys(UBOOL bFixed);
	virtual void TermComponentRBPhys(FRBPhysScene* InScene);
	virtual class URB_BodySetup* GetRBBodySetup();
	virtual void CookPhysConvexDataForScale(ULevel* Level, const FVector& TotalScale3D, INT& TriByteCount, INT& TriMeshCount, INT& HullByteCount, INT& HullCount);
	virtual FKCachedConvexData* GetCachedPhysConvexData(const FVector& InScale3D);

	/** Disables physics collision between a specific pair of meshes. */
	void DisableRBCollisionWithSMC( UStaticMeshComponent* OtherSMC, UBOOL bDisabled );

	virtual FPrimitiveSceneProxy* CreateSceneProxy();
	virtual UBOOL ShouldRecreateProxyOnUpdateTransform() const;

	// UActorComponent interface.
	virtual void CheckForErrors();

	protected: virtual void Attach(); public:
	virtual UBOOL IsValidComponent() const;

	virtual void InvalidateLightingCache();

	// UObject interface.
	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray );
	virtual void Serialize(FArchive& Ar);
	virtual void PostEditUndo();

	/**
	 * Called after all objects referenced by this object have been serialized. Order of PostLoad routed to 
	 * multiple objects loaded in one set is not deterministic though ConditionalPostLoad can be forced to
	 * ensure an object has been "PostLoad"ed.
	 */
	virtual void PostLoad();

	/** 
	 * Called when any property in this object is modified in UnrealEd
	 *
	 * @param	PropertyThatChanged		changed property
	 */
	virtual void PostEditChange( UProperty* PropertyThatChanged );

	/**
	 * Returns whether native properties are identical to the one of the passed in component.
	 *
	 * @param	Other	Other component to compare against
	 *
	 * @return TRUE if native properties are identical, FALSE otherwise
	 */
	virtual UBOOL AreNativePropertiesIdenticalTo( UComponent* Other ) const;

	/**
	 * This will return detail info about this specific object. (e.g. AudioComponent will return the name of the cue,
	 * ParticleSystemComponent will return the name of the ParticleSystem)  The idea here is that in many places
	 * you have a component of interest but what you really want is some characteristic that you can use to track
	 * down where it came from.  
	 *
	 */
	virtual FString GetDetailedInfoInternal() const;

	// Script functions

	DECLARE_FUNCTION(execSetStaticMesh);
	DECLARE_FUNCTION(execSetForceStaticDecals);
	DECLARE_FUNCTION(execDisableRBCollisionWithSMC);
};

/**
 * This struct provides the interface into the static mesh collision data
 */
class FStaticMeshCollisionDataProvider
{
	/**
	 * The component this mesh is attached to
	 */
	const UStaticMeshComponent* Component;
	/**
	 * The mesh that is being collided against
	 */
	UStaticMesh* Mesh;

	/**
	 * The LOD that is being collided against
	 */
	UINT CurrentLOD;
	/**
	 * Pointer to vertex buffer containing position data.
	 */
	FPositionVertexBuffer* PositionVertexBuffer;

	/** Hide default ctor */
	FStaticMeshCollisionDataProvider(void)
	{
	}

public:
	/**
	 * Sets the component and mesh members
	 */
	FORCEINLINE FStaticMeshCollisionDataProvider(const UStaticMeshComponent* InComponent, UINT InCurrentLOD = 0) :
		Component(InComponent),
		Mesh(InComponent->StaticMesh),
		CurrentLOD(InCurrentLOD),
		PositionVertexBuffer(&InComponent->StaticMesh->LODModels(InCurrentLOD).PositionVertexBuffer)
	{
	}

	/**
	 * Given an index, returns the position of the vertex
	 *
	 * @param Index the index into the vertices array
	 */
	FORCEINLINE const FVector& GetVertex(WORD Index) const
	{
		return PositionVertexBuffer->VertexPosition(Index);
	}

	/**
	 * Returns the material for a triangle based upon material index
	 *
	 * @param MaterialIndex the index into the materials array
	 */
	FORCEINLINE UMaterialInterface* GetMaterial(WORD MaterialIndex) const
	{
		return Component->GetMaterial(MaterialIndex);
	}

	/** Returns additional information. */
	FORCEINLINE INT GetItemIndex(WORD MaterialIndex) const
	{
		return 0;
	}

	/** If we should test against this triangle. */ 
	FORCEINLINE UBOOL ShouldCheckMaterial(INT MaterialIndex) const
	{
		return TRUE;
	}

	/**
	 * Returns the kDOPTree for this mesh
	 */
	FORCEINLINE const TkDOPTree<FStaticMeshCollisionDataProvider,WORD>& GetkDOPTree(void) const
	{
		return Mesh->kDOPTree;
	}

	/**
	 * Returns the local to world for the component
	 */
	FORCEINLINE const FMatrix& GetLocalToWorld(void) const
	{
		return Component->LocalToWorld;
	}

	/**
	 * Returns the world to local for the component
	 */
	FORCEINLINE const FMatrix GetWorldToLocal(void) const
	{
		return Component->LocalToWorld.Inverse();
	}

	/**
	 * Returns the local to world transpose adjoint for the component
	 */
	FORCEINLINE FMatrix GetLocalToWorldTransposeAdjoint(void) const
	{
		return Component->LocalToWorld.TransposeAdjoint();
	}

	/**
	 * Returns the determinant for the component
	 */
	FORCEINLINE FLOAT GetDeterminant(void) const
	{
		return Component->LocalToWorldDeterminant;
	}
};

/**
 * FStaticMeshComponentReattachContext - Destroys StaticMeshComponents using a given StaticMesh and recreates them when destructed.
 * Used to ensure stale rendering data isn't kept around in the components when importing over or rebuilding an existing static mesh.
 */
class FStaticMeshComponentReattachContext
{
public:

	/** Initialization constructor. */
	FStaticMeshComponentReattachContext(UStaticMesh* InStaticMesh):
		StaticMesh(InStaticMesh)
	{
		for(TObjectIterator<UStaticMeshComponent> It;It;++It)
		{
			if(It->StaticMesh == StaticMesh)
			{
				new(ReattachContexts) FComponentReattachContext(*It);

				// Invalidate the component's static lighting.
				((UActorComponent*)*It)->InvalidateLightingCache();
			}
		}

		// Flush the rendering commands generated by the detachments.
		// The static mesh scene proxies reference the UStaticMesh, and this ensures that they are cleaned up before the UStaticMesh changes.
		FlushRenderingCommands();
	}

private:

	UStaticMesh* StaticMesh;
	TIndirectArray<FComponentReattachContext> ReattachContexts;
};

/** Represents the triangles of one LOD of a static mesh primitive to the static lighting system. */
class FStaticMeshStaticLightingMesh : public FStaticLightingMesh
{
public:

	/** The meshes representing other LODs of this primitive. */
	TArray<FStaticLightingMesh*> OtherLODs;

	/** Initialization constructor. */
	FStaticMeshStaticLightingMesh(const UStaticMeshComponent* InPrimitive,INT InLODIndex,const TArray<ULightComponent*>& InRelevantLights);

	// FStaticLightingMesh interface.

	virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2) const;

	virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const;

	virtual UBOOL ShouldCastShadow(ULightComponent* Light,const FStaticLightingMapping* Receiver) const;

	/** @return		TRUE if the specified triangle casts a shadow. */
	virtual UBOOL IsTriangleCastingShadow(UINT TriangleIndex) const;

	/** @return		TRUE if the mesh wants to control shadow casting per element rather than per mesh. */
	virtual UBOOL IsControllingShadowPerElement() const;

	virtual UBOOL IsUniformShadowCaster() const;

	virtual FLightRayIntersection IntersectLightRay(const FVector& Start,const FVector& End,UBOOL bFindNearestIntersection) const;

protected:

	/** The LOD this mesh represents. */
	const INT LODIndex;

private:

	/** The static mesh this mesh represents. */
	const UStaticMesh* StaticMesh;

	/** The primitive this mesh represents. */
	const UStaticMeshComponent* const Primitive;

	/** The inverse transpose of the primitive's local to world transform. */
	const FMatrix LocalToWorldInverseTranspose;

	/** TRUE if the primitive has a transform which reverses the winding of its triangles. */
	const BITFIELD bReverseWinding : 1;
};


/**
 * A static mesh component scene proxy.
 */
class FStaticMeshSceneProxy : public FPrimitiveSceneProxy
{
protected:
	/** Creates a light cache for the decal if it has a lit material. */
	void CreateDecalLightCache(const FDecalInteraction& DecalInteraction);

public:

	/** Initialization constructor. */
	FStaticMeshSceneProxy(const UStaticMeshComponent* Component);

	virtual ~FStaticMeshSceneProxy() {}

	/** Sets up a FMeshElement for a specific LOD and element. */
	UBOOL GetMeshElement(INT LODIndex,INT ElementIndex,INT FragmentIndex,BYTE InDepthPriorityGroup,const FMatrix& WorldToLocal,FMeshElement& OutMeshElement) const;

	// FPrimitiveSceneProxy interface.
	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI);

	/** Determines if any collision should be drawn for this mesh. */
	UBOOL ShouldDrawCollision(const FSceneView* View);

	/** Determines if the simple or complex collision should be drawn for a particular static mesh. */
	UBOOL ShouldDrawSimpleCollision(const FSceneView* View, const UStaticMesh* Mesh);

protected:
	/**
	 * @return		The index into DecalLightCaches of the specified component, or INDEX_NONE if not found.
	 */
	INT FindDecalLightCacheIndex(const UDecalComponent* DecalComponent) const;

	/**
	 * Sets IndexBuffer, FirstIndex and NumPrimitives of OutMeshElement.
	 */
	virtual void SetIndexSource(INT LODIndex, INT ElementIndex, INT FragmentIndex, FMeshElement& OutMeshElement, UBOOL bWireframe) const;

public:
	virtual void InitLitDecalFlags(UINT InDepthPriorityGroup);

	/**
	* Draws the primitive's dynamic decal elements.  This is called from the rendering thread for each frame of each view.
	* The dynamic elements will only be rendered if GetViewRelevance declares dynamic relevance.
	* Called in the rendering thread.
	*
	* @param	PDI						The interface which receives the primitive elements.
	* @param	View					The view which is being rendered.
	* @param	InDepthPriorityGroup	The DPG which is being rendered.
	* @param	bDynamicLightingPass	TRUE if drawing dynamic lights, FALSE if drawing static lights.
	* @param	bTranslucentReceiverPass	TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
	*/
	virtual void DrawDynamicDecalElements(
		FPrimitiveDrawInterface* PDI,
		const FSceneView* View,
		UINT InDepthPriorityGroup,
		UBOOL bDynamicLightingPass,
		UBOOL bTranslucentReceiverPass
		);

	/**
	* Draws the primitive's static decal elements.  This is called from the game thread whenever this primitive is attached
	* as a receiver for a decal.
	*
	* The static elements will only be rendered if GetViewRelevance declares both static and decal relevance.
	* Called in the game thread.
	*
	* @param PDI - The interface which receives the primitive elements.
	*/
	virtual void DrawStaticDecalElements(FStaticPrimitiveDrawInterface* PDI,const FDecalInteraction& DecalInteraction);

	/**
	 * Adds a decal interaction to the primitive.  This is called in the rendering thread by AddDecalInteraction_GameThread.
	 */
	virtual void AddDecalInteraction_RenderingThread(const FDecalInteraction& DecalInteraction);

	/**
	 * Removes a decal interaction from the primitive.  This is called in the rendering thread by RemoveDecalInteraction_GameThread.
	 */
	virtual void RemoveDecalInteraction_RenderingThread(UDecalComponent* DecalComponent);

	/** 
	* Draw the scene proxy as a dynamic element
	*
	* @param	PDI - draw interface to render to
	* @param	View - current view
	* @param	DPGIndex - current depth priority 
	* @param	Flags - optional set of flags from EDrawDynamicElementFlags
	*/
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags);

	/**
	 * Called by the rendering thread to notify the proxy when a light is no longer
	 * associated with the proxy, so that it can clean up any cached resources.
	 * @param Light - The light to be removed.
	 */
	virtual void OnDetachLight(const FLightSceneInfo* Light);

	virtual void OnTransformChanged();
	/**
	 * Removes potentially cached shadow volume data for the passed in light.
	 *
	 * @param Light		The light for which cached shadow volume data will be removed.
	 */
	virtual void RemoveCachedShadowVolumeData( const FLightSceneInfo* Light );

	/**
	 * Called from the rendering thread, in the FSceneRenderer::RenderLights phase.
	 */
	virtual void DrawShadowVolumes(FShadowVolumeDrawInterface* SVDI,const FSceneView* View,const FLightSceneInfo* Light,UINT DPGIndex);

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	/**
	 *	Determines the relevance of this primitive's elements to the given light.
	 *	@param	LightSceneInfo			The light to determine relevance for
	 *	@param	bDynamic (output)		The light is dynamic for this primitive
	 *	@param	bRelevant (output)		The light is relevant for this primitive
	 *	@param	bLightMapped (output)	The light is light mapped for this primitive
	 */
	virtual void GetLightRelevance(FLightSceneInfo* LightSceneInfo, UBOOL& bDynamic, UBOOL& bRelevant, UBOOL& bLightMapped);

	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocStMSP ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() + LODs.GetAllocatedSize() ); }

protected:

	/** Information used by the proxy about a single LOD of the mesh. */
	class FLODInfo : public FLightCacheInterface
	{
	public:

		/** Information about an element of a LOD. */
		struct FElementInfo
		{
			/** Number of index ranges in this element, always 1 for static meshes */
			INT NumFragments;
			UMaterialInterface* Material;

			FElementInfo() :
				NumFragments(1),
				Material(NULL)
			{}
		};
		TArray<FElementInfo> Elements;

		/** Initialization constructor. */
		FLODInfo(const UStaticMeshComponent* InComponent,INT InLODIndex);

		// Accessors.
		const FLightMap* GetLightMap() const
		{
			return LODIndex < Component->LODData.Num() ?
				Component->LODData(LODIndex).LightMap :
				NULL;
		}

		// FLightCacheInterface.
		virtual FLightInteraction GetInteraction(const FLightSceneInfo* LightSceneInfo) const;

		virtual FLightMapInteraction GetLightMapInteraction() const
		{
			const FLightMap* LightMap = 
				LODIndex < Component->LODData.Num() ?
					Component->LODData(LODIndex).LightMap :
					NULL;
			return LightMap ?
				LightMap->GetInteraction() :
				FLightMapInteraction();
		}

	private:

		/** The static mesh component. */
		const UStaticMeshComponent* const Component;

		/** The LOD index. */
		const INT LODIndex;
	};

	/** Information about lights affecting a decal. */
	class FDecalLightCache : public FLightCacheInterface
	{
	public:
		FDecalLightCache()
			: DecalComponent( NULL )
			, LightMap( NULL )
		{
			ClearFlags();
		}

		FDecalLightCache(const FDecalInteraction& DecalInteraction, const FStaticMeshSceneProxy& Proxy);

		/**
		 * Clears flags used to track whether or not this decal has already been drawn for dynamic lighting.
		 * When drawing the first set of decals for this light, the blend state needs to be "set" rather
		 * than "add."  Subsequent calls use "add" to accumulate color.
		 */
		void ClearFlags();

		/**
		 * @return		The decal component associated with the decal interaction that uses this lighting information.
		 */
		const UDecalComponent* GetDecalComponent() const
		{
			return DecalComponent;
		}

		// FLightCacheInterface.
		virtual FLightInteraction GetInteraction(const FLightSceneInfo* LightSceneInfo) const;

		virtual FLightMapInteraction GetLightMapInteraction() const
		{
			return LightMap ? LightMap->GetInteraction() : FLightMapInteraction();
		}

		/** Tracks whether or not this decal has already been rendered for dynamic lighting. */
		// @todo: make a bitfield.
		UBOOL Flags[SDPG_PostProcess];

	private:
		/** The decal component associated with the decal interaction that uses this lighting information. */
		const UDecalComponent* DecalComponent;

		/** A map from persistent light IDs to information about the light's interaction with the primitive. */
		TMap<FGuid,FLightInteraction> StaticLightInteractionMap;

		/** The light-map used by the decal. */
		const FLightMap* LightMap;
	};

	TIndirectArray<FDecalLightCache> DecalLightCaches;

	AActor* Owner;
	const UStaticMesh* StaticMesh;
	const UStaticMeshComponent* StaticMeshComponent;

	TIndirectArray<FLODInfo> LODs;

	/**
	 * Used to update the shadow volume cache
	 * LastLOD is initialized to an invalid LOD
	 * so that the cache will be updated on the first DrawVolumeShadows()
	 */
	INT LastLOD;

	/**
	 * The forcedLOD set in the static mesh editor, copied from the mesh component
	 */
	INT ForcedLodModel;

	FVector TotalScale3D;

	FLinearColor LevelColor;
	FLinearColor PropertyColor;

	const BITFIELD bCastShadow : 1;
	const BITFIELD bSelected : 1;
	const BITFIELD bShouldCollide : 1;
	const BITFIELD bBlockZeroExtent : 1;
	const BITFIELD bBlockNonZeroExtent : 1;
	const BITFIELD bBlockRigidBody : 1;
	const BITFIELD bForceStaticDecal : 1;

	/** The view relevance for all the static mesh's materials. */
	FMaterialViewRelevance MaterialViewRelevance;

	const FLinearColor WireframeColor;

	/** Cached shadow volumes. */
	FShadowVolumeCache CachedShadowVolumes;

	/**
	 * Returns the minimum distance that the given LOD should be displayed at
	 *
	 * @param CurrentLevel - the LOD to find the min distance for
	 */
	FLOAT GetMinLODDist(INT CurrentLevel) const;

	/**
	 * Returns the maximum distance that the given LOD should be displayed at
	 * If the given LOD is the lowest detail LOD, then its maxDist will be WORLD_MAX
	 *
	 * @param CurrentLevel - the LOD to find the max distance for
	 */
	FLOAT GetMaxLODDist(INT CurrentLevel) const;

	/**
	 * Returns the LOD that should be used at the given distance
	 *
	 * @param Distance - distance from the current view to the component's bound origin
	 */
	INT GetLOD(FLOAT Distance) const;

	/**
	* @return TRUE if decals can be batched as static elements on this primitive
	*/
	UBOOL UseStaticDecal() const
	{
		// decals should render dynamically on movable meshes 
		return !IsMovable() || bForceStaticDecal;
	}
};
