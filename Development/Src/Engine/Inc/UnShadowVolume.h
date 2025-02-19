/*=============================================================================
	UnShadowVolume.h: Shadow volume definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

//
//	FShadowVertex
//

struct FShadowVertex
{
	FVector	Position;
	FLOAT	Extrusion;

	FShadowVertex() {}
	FShadowVertex(FVector InPosition,FLOAT InExtrusion): Position(InPosition), Extrusion(InExtrusion) {}

	friend FArchive& operator<<(FArchive& Ar,FShadowVertex& V)
	{
		return Ar << V.Position << V.Extrusion;
	}
};


//
//	FMeshEdge
//

struct FMeshEdge
{
	SHORT	Vertices[2];
	SHORT	Faces[2];
	
	// Constructor.

	FMeshEdge() {}

	friend FArchive& operator<<(FArchive& Ar,FMeshEdge& E)
	{
		// BM: Compat with old 32-bit FMeshEdge.
		if (Ar.LicenseeVer() < VER_BATMAN1)
		{
			INT Vertices[2], Faces[2];
			Ar << Vertices[0] << Vertices[1] << Faces[0] << Faces[1];
			E.Vertices[0] = Vertices[0];
			E.Vertices[1] = Vertices[1];
			E.Faces[0] = Faces[0];
			E.Faces[1] = Faces[1];
			return Ar;
		}

		// @warning BulkSerialize: FMeshEdge is serialized as memory dump
		// See TArray::BulkSerialize for detailed description of implied limitations.
		return Ar << E.Vertices[0] << E.Vertices[1] << E.Faces[0] << E.Faces[1];
	}
};

/**
 * Vertex factory for all shadow volumes.
 */
class FLocalShadowVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FLocalShadowVertexFactory);
public:

	struct DataType
	{
		// Constructor.
		DataType(): BaseExtrusion(0.0f)	{}

		/** The stream to read the vertex position from. */
		FVertexStreamComponent	PositionComponent;

		/**
		 * The stream to read the extrusion flag from
		 * (greater than 0.5f means the vertex should be extruded).
		 */
		FVertexStreamComponent	ExtrusionComponent;

		/** A small distance to extrude front-facing vertices. */
		FLOAT BaseExtrusion;
	};

	/**
	 * Should we cache the material's shadertype on this platform with this vertex factory? 
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
	{
		// Only compile the shadow volume vertex shader for this vertex factory type.
		return !appStricmp(ShaderType->GetName(),TEXT("FShadowVolumeVertexShader"));
	}

	/**
	 * An implementation of the interface used by TSynchronizedResource to update
	 * the resource with new data from the game thread.
	 */
	void SetData(const DataType& InData)
	{
		Data = InData;
		UpdateRHI();
	}

	/** Returns the small distance to extrude front-facing vertices. */
	FLOAT GetBaseExtrusion() const
	{
		return Data.BaseExtrusion;
	}

	// FRenderResource interface.
	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		Elements.AddItem(AccessStreamComponent(Data.PositionComponent,VEU_Position));
		Elements.AddItem(AccessStreamComponent(Data.ExtrusionComponent,VEU_TextureCoordinate,0));
		InitDeclaration(Elements,FVertexFactory::DataType());
	}

private:
	DataType Data;
};



/**
 * FShadowVertexBuffer
 * Used by the shadow volumes, primarily for skinned skeletal meshes who
 * re-calculate them on the CPU (no matter if the visual geometry is
 * skinned on CPU or GPU).
 */
class FShadowVertexBuffer : public FVertexBuffer
{
public:

	/**
	 * Constructor
	 * @param InNumVertices Number of vertices in the original geometry
	 * @param InBaseVertices Optional small amount to extrude front faces
	 */
	FShadowVertexBuffer( UINT InNumVertices, FLOAT InBaseExtrusion=0.0f)
	:	MaxVertices( 2*InNumVertices )
	,	BaseExtrusion( InBaseExtrusion )
	{
	}

	/**
	 * Separate Setup function, if the parameters weren't known at construction time.
	 * @param InNumVertices Number of vertices in the original geometry
	 * @param InBaseVertices Optional small amount to extrude front faces
	 */
	void Setup( UINT InNumVertices, FLOAT InBaseExtrusion=0.0f )
	{
		MaxVertices = 2*InNumVertices;
		BaseExtrusion = InBaseExtrusion;
	}

	/**
	 * Initializes the resource.
	 * This is only called by the rendering thread.
	 */
	virtual void InitResource()
	{
		FVertexBuffer::InitResource();

		FLocalShadowVertexFactory::DataType VertexFactoryData;
		VertexFactoryData.PositionComponent = FVertexStreamComponent(this,STRUCT_OFFSET(FShadowVertex,Position),sizeof(FShadowVertex),VET_Float3);
		VertexFactoryData.ExtrusionComponent = FVertexStreamComponent(this,STRUCT_OFFSET(FShadowVertex,Extrusion),sizeof(FShadowVertex),VET_Float1);
		VertexFactoryData.BaseExtrusion = BaseExtrusion;
		ShadowVertexFactory.SetData( VertexFactoryData );

		ShadowVertexFactory.InitResource();
	}

	/**
	 * Prepares the resource for deletion.
	 * This is only called by the rendering thread.
	 */
	virtual void ReleaseResource()
	{
		ShadowVertexFactory.ReleaseResource();

		FVertexBuffer::ReleaseResource();
	}

	/** Initialize the RHI for this rendering resource */
	virtual void InitRHI()
	{
		VertexBufferRHI = RHICreateVertexBuffer( MaxVertices*sizeof(FShadowVertex), NULL, RUF_Static /*RUF_Dynamic*/ );
	}

	/**
	 * Copy the input vertices into the vertex buffer, duplicated.
	 * @param InVertices Array of input vertices (assuming the first three floats in each vertex is the position)
	 * @param NumVertices Number of vertices in the input array
	 * @param Stride Vertex size in bytes
	 */
	void UpdateVertices( const void* InVertices, UINT NumVertices, UINT Stride )
	{
		check( NumVertices <= 2*MaxVertices );
		void *Buffer = RHILockVertexBuffer( VertexBufferRHI, 0, NumVertices*2*sizeof(FShadowVertex), FALSE );
		FShadowVertex* RESTRICT DestVertex = (FShadowVertex*)Buffer;
		FShadowVertex* RESTRICT DestVertex2 = (FShadowVertex*)Buffer + NumVertices;
		const BYTE* RESTRICT SrcVertices = (const BYTE*)InVertices;

		for (UINT VertexIndex = 0; VertexIndex < NumVertices; VertexIndex++)
		{
			DestVertex->Position = *((FVector*)SrcVertices);
			DestVertex->Extrusion = 0.f;
			DestVertex++;

			DestVertex2->Position = *((FVector*)SrcVertices);
			DestVertex2->Extrusion = 1.f;
			DestVertex2++;

			SrcVertices += Stride;
		}
		RHIUnlockVertexBuffer( VertexBufferRHI );
	}

	/** Get the vertex factory for the shadow volume */
	const FLocalShadowVertexFactory &GetVertexFactory() const	{ return ShadowVertexFactory; }

    /** Cpu skinned vertex name */
	virtual FString GetFriendlyName() const { return TEXT("Shadow volume vertices"); }

private:
	/** Maximum number of vertices in the shadow volume */
	UINT	MaxVertices;

	/** Optional small amount to extrude front faces */
	FLOAT	BaseExtrusion;

	FLocalShadowVertexFactory ShadowVertexFactory;
};


//
//	FShadowIndexBuffer
//

struct FShadowIndexBuffer
{
	TArray<WORD> Indices;

	FORCEINLINE void AddFace(WORD V1, WORD V2, WORD V3)
	{
		INT	BaseIndex = Indices.Add(3);
		Indices(BaseIndex + 0) = V1;
		Indices(BaseIndex + 1) = V2;
		Indices(BaseIndex + 2) = V3;
	}

	FORCEINLINE void AddEdge(WORD V1, WORD V2, WORD ExtrudeOffset)
	{
		INT	BaseIndex = Indices.Add(6);

		Indices(BaseIndex + 0) = V1;
		Indices(BaseIndex + 1) = V1 + ExtrudeOffset;
		Indices(BaseIndex + 2) = V2 + ExtrudeOffset;

		Indices(BaseIndex + 3) = V2 + ExtrudeOffset;
		Indices(BaseIndex + 4) = V2;
		Indices(BaseIndex + 5) = V1;
	}
};

/** A set of shadow volumes cached for the lights affecting a specific primitive. */
class FShadowVolumeCache : public FRenderResource
{
public:

	/** Information about the shadow volume cached for a specific light. */
	struct FCachedShadowVolume
	{
		FIndexBufferRHIRef IndexBufferRHI;
		UINT NumTriangles;
	};

	/** Destructor. */
	~FShadowVolumeCache();

	/**
	 * Adds a shadow volume to the cache.
	 * @param LightSceneInfo - The light the shadow volume is for.
	 * @param IndexBuffer - The shadow volume indices.
	 * @return The shadow volume.
	 */
	FCachedShadowVolume* AddShadowVolume(const class FLightSceneInfo* LightSceneInfo,FShadowIndexBuffer& IndexBuffer);

	/**
	 * Removes the shadow volume for a specific light from the cache.
	 * @param LightSceneInfo - The light to remove the shadow volume for.
	 */
	void RemoveShadowVolume(const class FLightSceneInfo* LightSceneInfo);

	/**
	 * Finds a shadow volume for a given light in the cache.
	 * @param LightSceneInfo - The light whose shadow volume is desired.
	 * @return If a shadow volume for the light is in the cache, the shadow volume is returned; otherwise, NULL is returned.
	 */
	const FCachedShadowVolume* GetShadowVolume(const class FLightSceneInfo* LightSceneInfo) const;

	// FRenderResource interface.
	virtual void ReleaseRHI();

private:

	TMap<const class FLightSceneInfo*,FCachedShadowVolume> CachedShadowVolumes;
};
