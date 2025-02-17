/*=============================================================================
	UnStaticMesh.cpp: Static mesh class implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineDecalClasses.h"
#include "EnginePhysicsClasses.h"

IMPLEMENT_CLASS(AStaticMeshActorBase);
IMPLEMENT_CLASS(AStaticMeshActor);
IMPLEMENT_CLASS(AStaticMeshCollectionActor);
IMPLEMENT_CLASS(ADynamicSMActor);
IMPLEMENT_CLASS(UStaticMesh);
IMPLEMENT_CLASS(AStaticMeshActorBasedOnExtremeContent);


/** Package name, that if set will cause only static meshes in that package to be rebuilt based on SM version. */
FName GStaticMeshPackageNameToRebuild = NAME_None;

/*-----------------------------------------------------------------------------
	FStaticMeshTriangleBulkData (FStaticMeshTriangle version of bulk data)
-----------------------------------------------------------------------------*/

/**
 * Returns size in bytes of single element.
 *
 * @return Size in bytes of single element
 */
INT FStaticMeshTriangleBulkData::GetElementSize() const
{
	return sizeof(FStaticMeshTriangle);
} 

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
void FStaticMeshTriangleBulkData::SerializeElement( FArchive& Ar, void* Data, INT ElementIndex )
{
	FStaticMeshTriangle& StaticMeshTriangle = *((FStaticMeshTriangle*)Data + ElementIndex);
	Ar << StaticMeshTriangle.Vertices[0];
	Ar << StaticMeshTriangle.Vertices[1];
	Ar << StaticMeshTriangle.Vertices[2];

	for( INT VertexIndex=0; VertexIndex<3; VertexIndex++ )
	{
		for( INT UVIndex=0; UVIndex<8; UVIndex++ )
		{
			Ar << StaticMeshTriangle.UVs[VertexIndex][UVIndex];
		}
	}

	Ar << StaticMeshTriangle.Colors[0];
	Ar << StaticMeshTriangle.Colors[1];
	Ar << StaticMeshTriangle.Colors[2];
	Ar << StaticMeshTriangle.MaterialIndex;
	if (Ar.Ver() >= VER_STATICMESH_FRAGMENTINDEX)
	{
		Ar << StaticMeshTriangle.FragmentIndex;
	}
	else
	{
		StaticMeshTriangle.FragmentIndex = 0;
	}
	Ar << StaticMeshTriangle.SmoothingMask;
	Ar << StaticMeshTriangle.NumUVs;

	if(Ar.Ver() >= VER_OVERRIDETANGENTBASIS)
	{
		Ar << StaticMeshTriangle.TangentX[0];
		Ar << StaticMeshTriangle.TangentX[1];
		Ar << StaticMeshTriangle.TangentX[2];
		Ar << StaticMeshTriangle.TangentY[0];
		Ar << StaticMeshTriangle.TangentY[1];
		Ar << StaticMeshTriangle.TangentY[2];
		Ar << StaticMeshTriangle.TangentZ[0];
		Ar << StaticMeshTriangle.TangentZ[1];
		Ar << StaticMeshTriangle.TangentZ[2];

		Ar << StaticMeshTriangle.bOverrideTangentBasis;
	}
	else if(Ar.IsLoading())
	{
		for(INT VertexIndex = 0;VertexIndex < 3;VertexIndex++)
		{
			StaticMeshTriangle.TangentX[VertexIndex] = FVector(0,0,0);
			StaticMeshTriangle.TangentY[VertexIndex] = FVector(0,0,0);
			StaticMeshTriangle.TangentZ[VertexIndex] = FVector(0,0,0);
		}

		if(Ar.Ver() >= VER_STATICMESH_EXTERNAL_VERTEX_NORMALS)
		{
			Ar << StaticMeshTriangle.TangentZ[0];
			Ar << StaticMeshTriangle.TangentZ[1];
			Ar << StaticMeshTriangle.TangentZ[2];

			Ar << StaticMeshTriangle.bOverrideTangentBasis;
		}
		else
		{
			StaticMeshTriangle.bOverrideTangentBasis = FALSE;
		}
	}
}

/**
 * Returns whether single element serialization is required given an archive. This e.g.
 * can be the case if the serialization for an element changes and the single element
 * serialization code handles backward compatibility.
 */
UBOOL FStaticMeshTriangleBulkData::RequiresSingleElementSerialization( FArchive& Ar )
{
	if( Ar.Ver() < VER_OVERRIDETANGENTBASIS )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/** The implementation of the static mesh position-only vertex data storage type. */
class FPositionVertexData :
	public FStaticMeshVertexDataInterface,
	public TResourceArray<FPositionVertex,VERTEXBUFFER_ALIGNMENT>
{
public:

	typedef TResourceArray<FPositionVertex,VERTEXBUFFER_ALIGNMENT> ArrayType;

	/**
	* Constructor
	* @param InNeedsCPUAccess - TRUE if resource array data should be CPU accessible
	*/
	FPositionVertexData(UBOOL InNeedsCPUAccess=FALSE)
		:	TResourceArray<FPositionVertex,VERTEXBUFFER_ALIGNMENT>(InNeedsCPUAccess)
	{
	}

	virtual void ResizeBuffer(UINT NumVertices)
	{
		if((UINT)ArrayType::Num() < NumVertices)
		{
			// Enlarge the array.
			ArrayType::Add(NumVertices - ArrayType::Num());
		}
		else if((UINT)ArrayType::Num() > NumVertices)
		{
			// Shrink the array.
			ArrayType::Remove(NumVertices,ArrayType::Num() - NumVertices);
		}
	}
	virtual UINT GetStride() const
	{
		return sizeof(FPositionVertex);
	}
	virtual BYTE* GetDataPointer()
	{
		return (BYTE*)&(*this)(0);
	}
	virtual FResourceArrayInterface* GetResourceArray()
	{
		return this;
	}
	virtual void Serialize(FArchive& Ar)
	{
		TResourceArray<FPositionVertex,VERTEXBUFFER_ALIGNMENT>::BulkSerialize(Ar);
	}
};

/*-----------------------------------------------------------------------------
	FPositionVertexBuffer
-----------------------------------------------------------------------------*/

FPositionVertexBuffer::FPositionVertexBuffer():
	VertexData(NULL),
	Data(NULL)
{}

FPositionVertexBuffer::~FPositionVertexBuffer()
{
	CleanUp();
}

/** Delete existing resources */
void FPositionVertexBuffer::CleanUp()
{
	if (VertexData)
	{
		delete VertexData;
		VertexData = NULL;
	}
}

/**
 * Initializes the buffer with the given vertices, used to convert legacy layouts.
 * @param InVertices - The vertices to initialize the buffer with.
 */
void FPositionVertexBuffer::Init(const TArray<FStaticMeshBuildVertex>& InVertices)
{
	NumVertices = InVertices.Num() * 2;

	// Allocate the vertex data storage type.
	AllocateData();

	// Allocate the vertex data buffer.
	VertexData->ResizeBuffer(NumVertices);
	Data = VertexData->GetDataPointer();

	// Copy the vertices into the buffer.
	for(INT VertexIndex = 0;VertexIndex < InVertices.Num();VertexIndex++)
	{
		const FStaticMeshBuildVertex& SourceVertex = InVertices(VertexIndex);
		for(INT Extruded = 0;Extruded < 2;Extruded++)
		{
			const UINT DestVertexIndex = (Extruded ? InVertices.Num() : 0) + VertexIndex ;
			VertexPosition(DestVertexIndex) = SourceVertex.Position;
		}
	}
}

/**
* Removes the cloned vertices used for extruding shadow volumes.
* @param NumVertices - The real number of static mesh vertices which should remain in the buffer upon return.
*/
void FPositionVertexBuffer::RemoveShadowVolumeVertices(UINT InNumVertices)
{
	check(VertexData);
	VertexData->ResizeBuffer(InNumVertices);
	NumVertices = InNumVertices;

	// Make a copy of the vertex data pointer.
	Data = VertexData->GetDataPointer();
}

/** Serializer. */
FArchive& operator<<(FArchive& Ar,FPositionVertexBuffer& VertexBuffer)
{
	Ar << VertexBuffer.Stride << VertexBuffer.NumVertices;

	if(Ar.IsLoading())
	{
		// Allocate the vertex data storage type.
		VertexBuffer.AllocateData();
	}

	// Serialize the vertex data.
	VertexBuffer.VertexData->Serialize(Ar);

	// Make a copy of the vertex data pointer.
	VertexBuffer.Data = VertexBuffer.VertexData->GetDataPointer();

	return Ar;
}

/**
* Specialized assignment operator, only used when importing LOD's.  
*/
void FPositionVertexBuffer::operator=(const FPositionVertexBuffer &Other)
{
	//VertexData doesn't need to be allocated here because Build will be called next,
	VertexData = NULL;
}

void FPositionVertexBuffer::InitRHI()
{
	check(VertexData);
	FResourceArrayInterface* ResourceArray = VertexData->GetResourceArray();
	if(ResourceArray->GetResourceDataSize())
	{
		// Create the vertex buffer.
		VertexBufferRHI = RHICreateVertexBuffer(ResourceArray->GetResourceDataSize(),ResourceArray,RUF_Static);
	}
}

void FPositionVertexBuffer::AllocateData()
{
	// Clear any old VertexData before allocating.
	CleanUp();

	const UBOOL bNeedsCPUAccess=TRUE;
	VertexData = new FPositionVertexData(bNeedsCPUAccess);
	// Calculate the vertex stride.
	Stride = VertexData->GetStride();
}


/*-----------------------------------------------------------------------------
	FStaticMeshVertexBuffer
-----------------------------------------------------------------------------*/

/** The implementation of the static mesh vertex data storage type. */
template<typename VertexDataType>
class TStaticMeshVertexData :
	public FStaticMeshVertexDataInterface,
	public TResourceArray<VertexDataType,VERTEXBUFFER_ALIGNMENT>
{
public:

	typedef TResourceArray<VertexDataType,VERTEXBUFFER_ALIGNMENT> ArrayType;

	/**
	* Constructor
	* @param InNeedsCPUAccess - TRUE if resource array data should be CPU accessible
	*/
	TStaticMeshVertexData(UBOOL InNeedsCPUAccess=FALSE)
		:	TResourceArray<VertexDataType,VERTEXBUFFER_ALIGNMENT>(InNeedsCPUAccess)
	{
	}

	/**
	* Resizes the vertex data buffer, discarding any data which no longer fits.
	*
	* @param NumVertices - The number of vertices to allocate the buffer for.
	*/
	virtual void ResizeBuffer(UINT NumVertices)
	{
		if((UINT)ArrayType::Num() < NumVertices)
		{
			// Enlarge the array.
			ArrayType::Add(NumVertices - ArrayType::Num());
		}
		else if((UINT)ArrayType::Num() > NumVertices)
		{
			// Shrink the array.
			ArrayType::Remove(NumVertices,ArrayType::Num() - NumVertices);
		}
	}
	/**
	* @return stride of the vertex type stored in the resource data array
	*/
	virtual UINT GetStride() const
	{
		return sizeof(VertexDataType);
	}
	/**
	* @return BYTE pointer to the resource data array
	*/
	virtual BYTE* GetDataPointer()
	{
		return (BYTE*)&(*this)(0);
	}
	/**
	* @return resource array interface access
	*/
	virtual FResourceArrayInterface* GetResourceArray()
	{
		return this;
	}
	/**
	* Serializer for this class
	*
	* @param Ar - archive to serialize to
	* @param B - data to serialize
	*/
	virtual void Serialize(FArchive& Ar)
	{
		TResourceArray<VertexDataType,VERTEXBUFFER_ALIGNMENT>::BulkSerialize(Ar);
	}
	/**
	* Assignment operator. This is currently the only method which allows for 
	* modifying an existing resource array
	*/
	TStaticMeshVertexData<VertexDataType>& operator=(const TArray<VertexDataType>& Other)
	{
		TResourceArray<VertexDataType,VERTEXBUFFER_ALIGNMENT>::operator=(Other);
		return *this;
	}
};

FStaticMeshVertexBuffer::FStaticMeshVertexBuffer():
	VertexData(NULL),
	Data(NULL),
	bUseFullPrecisionUVs(FALSE)
{}

FStaticMeshVertexBuffer::~FStaticMeshVertexBuffer()
{
	CleanUp();
}

/** Delete existing resources */
void FStaticMeshVertexBuffer::CleanUp()
{
	delete VertexData;
	VertexData = NULL;
}

/**
* Initializes the buffer with the given vertices.
* @param InVertices - The vertices to initialize the buffer with.
* @param InNumTexCoords - The number of texture coordinate to store in the buffer.
*/
void FStaticMeshVertexBuffer::Init(const TArray<FStaticMeshBuildVertex>& InVertices,UINT InNumTexCoords)
{
	NumTexCoords = InNumTexCoords;
	NumVertices = InVertices.Num() * 2;

	// Allocate the vertex data storage type.
	AllocateData();

	// Allocate the vertex data buffer.
	VertexData->ResizeBuffer(NumVertices);
	Data = VertexData->GetDataPointer();

	// Copy the vertices into the buffer.
	for(INT VertexIndex = 0;VertexIndex < InVertices.Num();VertexIndex++)
	{
		const FStaticMeshBuildVertex& SourceVertex = InVertices(VertexIndex);
		for(INT Extruded = 0;Extruded < 2;Extruded++)
		{
			const UINT DestVertexIndex = (Extruded ? InVertices.Num() : 0) + VertexIndex ;
			VertexTangentX(DestVertexIndex) = SourceVertex.TangentX;
			VertexTangentZ(DestVertexIndex) = SourceVertex.TangentZ;

			// store the sign of the determinant in TangentZ.W
			VertexTangentZ(DestVertexIndex).Vector.W = GetBasisDeterminantSignByte( 
				SourceVertex.TangentX, SourceVertex.TangentY, SourceVertex.TangentZ );

			VertexColor(DestVertexIndex) = SourceVertex.Color;
			for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
			{
				SetVertexUV(DestVertexIndex,UVIndex,SourceVertex.UVs[UVIndex]);
			}
		}
	}
}

/**
* Removes the cloned vertices used for extruding shadow volumes.
* @param NumVertices - The real number of static mesh vertices which should remain in the buffer upon return.
*/
void FStaticMeshVertexBuffer::RemoveShadowVolumeVertices(UINT InNumVertices)
{
	check(VertexData);
	VertexData->ResizeBuffer(InNumVertices);
	NumVertices = InNumVertices;

	// Make a copy of the vertex data pointer.
	Data = VertexData->GetDataPointer();
}

/**
* Convert the existing data in this mesh from 16 bit to 32 bit UVs.
* Without rebuilding the mesh (loss of precision)
*/
template<INT NumTexCoordsT>
void FStaticMeshVertexBuffer::ConvertToFullPrecisionUVs()
{
	if( !bUseFullPrecisionUVs )
	{
		check(NumTexCoords == NumTexCoordsT);
		// create temp array to store 32 bit values
		TArray< TStaticMeshFullVertexFloat32UVs<NumTexCoordsT> > DestVertexData;
		// source vertices
		TStaticMeshVertexData< TStaticMeshFullVertexFloat16UVs<NumTexCoordsT> >& SrcVertexData = 
			*(TStaticMeshVertexData< TStaticMeshFullVertexFloat16UVs<NumTexCoordsT> >*)VertexData;
		// copy elements from source vertices to temp data
		DestVertexData.Add(SrcVertexData.Num());
		for( INT VertIdx=0; VertIdx < SrcVertexData.Num(); VertIdx++ )
		{
			TStaticMeshFullVertexFloat32UVs<NumTexCoordsT>& DestVert = DestVertexData(VertIdx);
			TStaticMeshFullVertexFloat16UVs<NumTexCoordsT>& SrcVert = SrcVertexData(VertIdx);		
			appMemcpy(&DestVert,&SrcVert,sizeof(FStaticMeshFullVertex));
			for( INT UVIdx=0; UVIdx < NumTexCoordsT; UVIdx++ )
			{
				DestVert.UVs[UVIdx] = FVector2D(SrcVert.UVs[UVIdx]);
			}
		}
		// force 32 bit UVs
		bUseFullPrecisionUVs = TRUE;
		AllocateData();
		*(TStaticMeshVertexData< TStaticMeshFullVertexFloat32UVs<NumTexCoordsT> >*)VertexData = DestVertexData;
		Data = VertexData->GetDataPointer();
		Stride = VertexData->GetStride();
	}
}

/** Serializer. */
FArchive& operator<<(FArchive& Ar,FStaticMeshVertexBuffer& VertexBuffer)
{
	Ar << VertexBuffer.NumTexCoords << VertexBuffer.Stride << VertexBuffer.NumVertices;
	Ar << VertexBuffer.bUseFullPrecisionUVs;								

	if( Ar.IsLoading() )
	{
		// Allocate the vertex data storage type.
		VertexBuffer.AllocateData();
	}

	// Serialize the vertex data.
	VertexBuffer.VertexData->Serialize(Ar);

	// Make a copy of the vertex data pointer.
	VertexBuffer.Data = VertexBuffer.VertexData->GetDataPointer();

	return Ar;
}

/**
* Specialized assignment operator, only used when importing LOD's.  
*/
void FStaticMeshVertexBuffer::operator=(const FStaticMeshVertexBuffer &Other)
{
	//VertexData doesn't need to be allocated here because Build will be called next,
	VertexData = NULL;
	bUseFullPrecisionUVs = Other.bUseFullPrecisionUVs;
}

void FStaticMeshVertexBuffer::InitRHI()
{
	check(VertexData);
	FResourceArrayInterface* ResourceArray = VertexData->GetResourceArray();
	if(ResourceArray->GetResourceDataSize())
	{
		// Create the vertex buffer.
		VertexBufferRHI = RHICreateVertexBuffer(ResourceArray->GetResourceDataSize(),ResourceArray,RUF_Static);
	}
}

void FStaticMeshVertexBuffer::AllocateData()
{
	// Clear any old VertexData before allocating.
	CleanUp();

	const UBOOL bNeedsCPUAccess=TRUE;
	if( !bUseFullPrecisionUVs )
	{
		switch(NumTexCoords)
		{
		case 1: VertexData = new TStaticMeshVertexData< TStaticMeshFullVertexFloat16UVs<1> >(bNeedsCPUAccess); break;
		case 2: VertexData = new TStaticMeshVertexData< TStaticMeshFullVertexFloat16UVs<2> >(bNeedsCPUAccess); break;
		case 3: VertexData = new TStaticMeshVertexData< TStaticMeshFullVertexFloat16UVs<3> >(bNeedsCPUAccess); break;
		case 4: VertexData = new TStaticMeshVertexData< TStaticMeshFullVertexFloat16UVs<4> >(bNeedsCPUAccess); break;
		default: appErrorf(TEXT("Invalid number of texture coordinates"));
		};		
	}
	else
	{
		switch(NumTexCoords)
		{
		case 1: VertexData = new TStaticMeshVertexData< TStaticMeshFullVertexFloat32UVs<1> >(bNeedsCPUAccess); break;
		case 2: VertexData = new TStaticMeshVertexData< TStaticMeshFullVertexFloat32UVs<2> >(bNeedsCPUAccess); break;
		case 3: VertexData = new TStaticMeshVertexData< TStaticMeshFullVertexFloat32UVs<3> >(bNeedsCPUAccess); break;
		case 4: VertexData = new TStaticMeshVertexData< TStaticMeshFullVertexFloat32UVs<4> >(bNeedsCPUAccess); break;
		default: appErrorf(TEXT("Invalid number of texture coordinates"));
		};		
	}	

	// Calculate the vertex stride.
	Stride = VertexData->GetStride();
}

/*-----------------------------------------------------------------------------
	FShadowExtrusionVertexBuffer
-----------------------------------------------------------------------------*/

/** The implementation of the static mesh position-only vertex data storage type. */
class FStaticMeshShadowExtrusionVertexData :
	public FStaticMeshVertexDataInterface,
	public TResourceArray<FStaticMeshShadowExtrusionVertex,VERTEXBUFFER_ALIGNMENT>
{
public:

	typedef TResourceArray<FStaticMeshShadowExtrusionVertex,VERTEXBUFFER_ALIGNMENT> ArrayType;

	/**
	* Constructor
	* @param InNeedsCPUAccess - TRUE if resource array data should be CPU accessible
	*/
	FStaticMeshShadowExtrusionVertexData(UBOOL InNeedsCPUAccess=FALSE)
		:	TResourceArray<FStaticMeshShadowExtrusionVertex,VERTEXBUFFER_ALIGNMENT>(InNeedsCPUAccess)
	{
	}

	virtual void ResizeBuffer(UINT NumVertices)
	{
		if((UINT)ArrayType::Num() < NumVertices)
		{
			// Enlarge the array.
			ArrayType::Add(NumVertices - ArrayType::Num());
		}
		else if((UINT)ArrayType::Num() > NumVertices)
		{
			// Shrink the array.
			ArrayType::Remove(NumVertices,ArrayType::Num() - NumVertices);
		}
	}
	virtual UINT GetStride() const
	{
		return sizeof(FStaticMeshShadowExtrusionVertex);
	}
	virtual BYTE* GetDataPointer()
	{
		return (BYTE*)&(*this)(0);
	}
	virtual FResourceArrayInterface* GetResourceArray()
	{
		return this;
	}
	virtual void Serialize(FArchive& Ar)
	{
		TResourceArray<FStaticMeshShadowExtrusionVertex,VERTEXBUFFER_ALIGNMENT>::BulkSerialize(Ar);
	}
};

/*-----------------------------------------------------------------------------
	FShadowExtrusionVertexBuffer
-----------------------------------------------------------------------------*/

/** 
* Default constructor. 
*/
FShadowExtrusionVertexBuffer::FShadowExtrusionVertexBuffer()
:	VertexData(NULL)
,	Data(NULL)
{
}

/** 
* Destructor. 
*/
FShadowExtrusionVertexBuffer::~FShadowExtrusionVertexBuffer()
{
	CleanUp();
}

/** 
* Delete existing resources 
*/
void FShadowExtrusionVertexBuffer::CleanUp()
{
	delete VertexData;
	VertexData = NULL;
}

/**
* Initializes the buffer with the given vertices, used to convert legacy layouts.
*
* @param InVertices - The vertices to initialize the buffer with.
*/
void FShadowExtrusionVertexBuffer::Init(const TArray<FStaticMeshBuildVertex>& InVertices)
{
	NumVertices = InVertices.Num() * 2;

	// Allocate the vertex data storage type.
	AllocateData();

	// Allocate the vertex data buffer.
	VertexData->ResizeBuffer(NumVertices);
	Data = VertexData->GetDataPointer();

	// Copy the vertices into the buffer.
	for(INT VertexIndex = 0;VertexIndex < InVertices.Num();VertexIndex++)
	{
		const FStaticMeshBuildVertex& SourceVertex = InVertices(VertexIndex);
		for(INT Extruded = 0;Extruded < 2;Extruded++)
		{
			const UINT DestVertexIndex = (Extruded ? InVertices.Num() : 0) + VertexIndex ;
			VertexShadowExtrusionPredicate(DestVertexIndex) = Extruded ? 1.0f : 0.0f;
		}
	}
}

/**
* Removes the cloned vertices used for extruding shadow volumes.
*
* @param NumVertices - The real number of static mesh vertices which should remain in the buffer upon return.
*/
void FShadowExtrusionVertexBuffer::RemoveShadowVolumeVertices(UINT InNumVertices)
{
	check(VertexData);
	VertexData->ResizeBuffer(0);
	NumVertices = 0;
	Data = NULL;
}

/** 
* Serializer. 
* 
* @param Ar - archive to serialize with
* @param VertexBuffer - data to serialize to/from
* @return archive that was used
*/
FArchive& operator<<(FArchive& Ar,FShadowExtrusionVertexBuffer& VertexBuffer)
{
	Ar << VertexBuffer.Stride << VertexBuffer.NumVertices;

	if(Ar.IsLoading())
	{
		// Allocate the vertex data storage type.
		VertexBuffer.AllocateData();
	}

	// Serialize the vertex data.
	VertexBuffer.VertexData->Serialize(Ar);

	// Make a copy of the vertex data pointer.
	VertexBuffer.Data = VertexBuffer.VertexData->GetDataPointer();

	return Ar;
}

/**
* Specialized assignment operator, only used when importing LOD's. 
*
* @param Other - instance to copy from
*/
void FShadowExtrusionVertexBuffer::operator=(const FShadowExtrusionVertexBuffer &Other)
{
	VertexData = NULL;
}

/** 
* Initialize the vertex buffer rendering resource. Called by the render thread
*/ 
void FShadowExtrusionVertexBuffer::InitRHI()
{
	check(VertexData);
	FResourceArrayInterface* ResourceArray = VertexData->GetResourceArray();
	if(ResourceArray->GetResourceDataSize())
	{
		// Create the vertex buffer.
		VertexBufferRHI = RHICreateVertexBuffer(ResourceArray->GetResourceDataSize(),ResourceArray,RUF_Static);
	}
}

/** 
* Allocates the internal vertex data storage type. 
*/
void FShadowExtrusionVertexBuffer::AllocateData()
{
	// Clear any old VertexData before allocating.
	CleanUp();

	const UBOOL bNeedsCPUAccess=TRUE;
	VertexData = new FStaticMeshShadowExtrusionVertexData(bNeedsCPUAccess);
	// Calculate the vertex stride.
	Stride = VertexData->GetStride();
}

/*-----------------------------------------------------------------------------
	FStaticMeshRenderData
-----------------------------------------------------------------------------*/

FStaticMeshRenderData::FStaticMeshRenderData()
:	IndexBuffer(TRUE)	// Needs to be CPU accessible for CPU-skinned decals.
{
}

/**
 * Special serialize function passing the owning UObject along as required by FUnytpedBulkData
 * serialization.
 *
 * @param	Ar		Archive to serialize with
 * @param	Owner	UObject this structure is serialized within
 * @param	Idx		Index of current array entry being serialized
 */
void FStaticMeshRenderData::Serialize( FArchive& Ar, UObject* Owner, INT Idx )
{
	RawTriangles.Serialize( Ar, Owner );
	
	Ar	<< Elements;
	Ar	<< PositionVertexBuffer 
		<< VertexBuffer 
		<< ShadowExtrusionVertexBuffer 
		<< NumVertices;

	// Revert to using 32 bit Float UVs on hardware that doesn't support rendering with 16 bit Float UVs 
	if( !GIsCooking && Ar.IsLoading() && !GVertexElementTypeSupport.IsSupported(VET_Half2) )
	{
		switch(VertexBuffer.GetNumTexCoords())
		{
		case 1:	VertexBuffer.ConvertToFullPrecisionUVs<1>(); break;
		case 2:	VertexBuffer.ConvertToFullPrecisionUVs<2>(); break;
		case 3:	VertexBuffer.ConvertToFullPrecisionUVs<3>(); break;
		case 4:	VertexBuffer.ConvertToFullPrecisionUVs<4>(); break;
		default: appErrorf(TEXT("Invalid number of texture coordinates"));
		}
	}

	Ar << IndexBuffer;
	Ar << WireframeIndexBuffer;
	Edges.BulkSerialize( Ar );
	Ar << ShadowTriangleDoubleSided;
}

INT FStaticMeshRenderData::GetTriangleCount() const
{
	INT NumTriangles = 0;
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		NumTriangles += Elements(ElementIndex).NumTriangles;
	}
	return NumTriangles;
}

/**
 * Initializes the LOD's render resources.
 * @param Parent Parent mesh
 */
void FStaticMeshRenderData::InitResources(UStaticMesh* Parent)
{
	if (Parent->bUsedForInstancing && 
		IndexBuffer.Indices.Num() && 
		VertexBuffer.GetNumVertices() &&
		Elements.Num() == 1 // You really can't use hardware instancing on the consoles with multiple elements because they share the same index buffer. 
		// The mesh emitter and foliage don't bother to enforce this; they will just render wrong, even on the PC.
		)
	{
		IndexBuffer.SetupForInstancing(VertexBuffer.GetNumVertices());
	}

	// Initialize the vertex and index buffers.
	BeginInitResource(&IndexBuffer);
	if( WireframeIndexBuffer.Indices.Num() )
	{
		BeginInitResource(&WireframeIndexBuffer);
	}	
	BeginInitResource(&VertexBuffer);
	BeginInitResource(&PositionVertexBuffer);
	if( UEngine::ShadowVolumesAllowed() )
	{
		BeginInitResource(&ShadowExtrusionVertexBuffer);
	}	

	// Initialize the static mesh's vertex factory.
	ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
		InitStaticMeshVertexFactory,
		FLocalVertexFactory*,VertexFactory,&VertexFactory,
		FStaticMeshRenderData*,RenderData,this,
		UStaticMesh*,Parent,Parent,
		{
			FLocalVertexFactory::DataType Data;
			Data.PositionComponent = FVertexStreamComponent(
				&RenderData->PositionVertexBuffer,
				STRUCT_OFFSET(FPositionVertex,Position),
				RenderData->PositionVertexBuffer.GetStride(),
				VET_Float3
				);
			Data.TangentBasisComponents[0] = FVertexStreamComponent(
				&RenderData->VertexBuffer,
				STRUCT_OFFSET(FStaticMeshFullVertex,TangentX),
				RenderData->VertexBuffer.GetStride(),
				VET_PackedNormal
				);
			Data.TangentBasisComponents[1] = FVertexStreamComponent(
				&RenderData->VertexBuffer,
				STRUCT_OFFSET(FStaticMeshFullVertex,TangentZ),
				RenderData->VertexBuffer.GetStride(),
				VET_PackedNormal
				);
			Data.ColorComponent = FVertexStreamComponent(
				&RenderData->VertexBuffer,
				STRUCT_OFFSET(FStaticMeshFullVertex,Color),
				RenderData->VertexBuffer.GetStride(),
				VET_Color
				);

			Data.TextureCoordinates.Empty();

			if( !RenderData->VertexBuffer.GetUseFullPrecisionUVs() )
			{
				for(UINT UVIndex = 0;UVIndex < RenderData->VertexBuffer.GetNumTexCoords();UVIndex++)
				{
					Data.TextureCoordinates.AddItem(FVertexStreamComponent(
						&RenderData->VertexBuffer,
						STRUCT_OFFSET(TStaticMeshFullVertexFloat16UVs<MAX_TEXCOORDS>,UVs) + sizeof(FVector2DHalf) * UVIndex,
						RenderData->VertexBuffer.GetStride(),
						VET_Half2
						));
				}
				if(	Parent->LightMapCoordinateIndex >= 0 && (UINT)Parent->LightMapCoordinateIndex < RenderData->VertexBuffer.GetNumTexCoords())
				{
					Data.ShadowMapCoordinateComponent = FVertexStreamComponent(
						&RenderData->VertexBuffer,
						STRUCT_OFFSET(TStaticMeshFullVertexFloat16UVs<MAX_TEXCOORDS>,UVs) + sizeof(FVector2DHalf) * Parent->LightMapCoordinateIndex,
						RenderData->VertexBuffer.GetStride(),
						VET_Half2
						);
				}
			}
			else
			{
				for(UINT UVIndex = 0;UVIndex < RenderData->VertexBuffer.GetNumTexCoords();UVIndex++)
				{
					Data.TextureCoordinates.AddItem(FVertexStreamComponent(
						&RenderData->VertexBuffer,
						STRUCT_OFFSET(TStaticMeshFullVertexFloat32UVs<MAX_TEXCOORDS>,UVs) + sizeof(FVector2D) * UVIndex,
						RenderData->VertexBuffer.GetStride(),
						VET_Float2
						));
				}

				if(	Parent->LightMapCoordinateIndex >= 0 && (UINT)Parent->LightMapCoordinateIndex < RenderData->VertexBuffer.GetNumTexCoords())
				{
					Data.ShadowMapCoordinateComponent = FVertexStreamComponent(
						&RenderData->VertexBuffer,
						STRUCT_OFFSET(TStaticMeshFullVertexFloat32UVs<MAX_TEXCOORDS>,UVs) + sizeof(FVector2D) * Parent->LightMapCoordinateIndex,
						RenderData->VertexBuffer.GetStride(),
						VET_Float2
						);
				}
			}	
			VertexFactory->SetData(Data);
		});
	BeginInitResource(&VertexFactory);

	// Initialize the static mesh's shadow vertex factory.
	if( UEngine::ShadowVolumesAllowed() )
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
			InitStaticMeshShadowVertexFactory,
			FLocalShadowVertexFactory*,VertexFactory,&ShadowVertexFactory,
			FPositionVertexBuffer*,PositionVertexBuffer,&PositionVertexBuffer,
			FShadowExtrusionVertexBuffer*,ShadowExtrusionVertexBuffer,&ShadowExtrusionVertexBuffer,
			{
				FLocalShadowVertexFactory::DataType Data;
				Data.PositionComponent = FVertexStreamComponent(
					PositionVertexBuffer,
					STRUCT_OFFSET(FPositionVertex,Position),
					PositionVertexBuffer->GetStride(),
					VET_Float3
					);
				Data.ExtrusionComponent = FVertexStreamComponent(
					ShadowExtrusionVertexBuffer,
					STRUCT_OFFSET(FStaticMeshShadowExtrusionVertex,ShadowExtrusionPredicate),
					ShadowExtrusionVertexBuffer->GetStride(),
					VET_Float1
					);
				VertexFactory->SetData(Data);
			});
		BeginInitResource(&ShadowVertexFactory);
	}

	INC_DWORD_STAT_BY( STAT_StaticMeshVertexMemory, VertexBuffer.GetStride() * VertexBuffer.GetNumVertices() + PositionVertexBuffer.GetStride() * PositionVertexBuffer.GetNumVertices() + ShadowExtrusionVertexBuffer.GetStride() * ShadowExtrusionVertexBuffer.GetNumVertices() );
	INC_DWORD_STAT_BY( STAT_StaticMeshIndexMemory, (IndexBuffer.Indices.Num() + WireframeIndexBuffer.Indices.Num()) * 2 );
}

/**
 * Releases the LOD's render resources.
 */
void FStaticMeshRenderData::ReleaseResources()
{
	DEC_DWORD_STAT_BY( STAT_StaticMeshVertexMemory, VertexBuffer.GetStride() * VertexBuffer.GetNumVertices() + PositionVertexBuffer.GetStride() * PositionVertexBuffer.GetNumVertices() + ShadowExtrusionVertexBuffer.GetStride() * ShadowExtrusionVertexBuffer.GetNumVertices() );
	DEC_DWORD_STAT_BY( STAT_StaticMeshIndexMemory, (IndexBuffer.Indices.Num() + WireframeIndexBuffer.Indices.Num()) * 2 );

	// Release the vertex and index buffers.
	BeginReleaseResource(&IndexBuffer);
	BeginReleaseResource(&WireframeIndexBuffer);
	BeginReleaseResource(&VertexBuffer);
	BeginReleaseResource(&PositionVertexBuffer);
	BeginReleaseResource(&ShadowExtrusionVertexBuffer);

	// Release the vertex factories.
	BeginReleaseResource(&VertexFactory);
	BeginReleaseResource(&ShadowVertexFactory);
}

/*-----------------------------------------------------------------------------
UStaticMesh
-----------------------------------------------------------------------------*/

/**
 * Initializes the static mesh's render resources.
 */
void UStaticMesh::InitResources()
{
	for(INT LODIndex = 0;LODIndex < LODModels.Num();LODIndex++)
	{
		LODModels(LODIndex).InitResources(this);
	}
}

/**
 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
 *
 * @return size of resource as to be displayed to artists/ LDs in the Editor.
 */
INT UStaticMesh::GetResourceSize()
{
	FArchiveCountMem CountBytesSize( this );
	INT ResourceSize = CountBytesSize.GetNum();
	return ResourceSize;
}


/**
 * Releases the static mesh's render resources.
 */
void UStaticMesh::ReleaseResources()
{
	for(INT LODIndex = 0;LODIndex < LODModels.Num();LODIndex++)
	{
		LODModels(LODIndex).ReleaseResources();
	}

	// insert a fence to signal when these commands completed
	ReleaseResourcesFence.BeginFence();
}

//
//	UStaticMesh::StaticConstructor
//
void UStaticMesh::StaticConstructor()
{
#if !CONSOLE
	new(GetClass()->HideCategories) FName(NAME_Object);
#endif

	new(GetClass(),TEXT("UseSimpleLineCollision"),RF_Public)		UBoolProperty(CPP_PROPERTY(UseSimpleLineCollision),TEXT(""),CPF_Edit);
	new(GetClass(),TEXT("UseSimpleBoxCollision"),RF_Public)			UBoolProperty(CPP_PROPERTY(UseSimpleBoxCollision),TEXT(""),CPF_Edit);
	new(GetClass(),TEXT("UseSimpleRigidBodyCollision"),RF_Public)	UBoolProperty(CPP_PROPERTY(UseSimpleRigidBodyCollision),TEXT(""),CPF_Edit);
	new(GetClass(),TEXT("ForceDoubleSidedShadowVolumes"),RF_Public)	UBoolProperty(CPP_PROPERTY(DoubleSidedShadowVolumes),TEXT(""),CPF_Edit);
	new(GetClass(),TEXT("UseFullPrecisionUVs"),RF_Public)			UBoolProperty(CPP_PROPERTY(UseFullPrecisionUVs),TEXT(""),CPF_Edit);
	new(GetClass(),TEXT("bUsedForInstancing"),RF_Public)			UBoolProperty(CPP_PROPERTY(bUsedForInstancing),TEXT(""),CPF_Edit);

	new(GetClass(),TEXT("LightMapResolution"),RF_Public)			UIntProperty(CPP_PROPERTY(LightMapResolution),TEXT(""),CPF_Edit);
	new(GetClass(),TEXT("LightMapCoordinateIndex"),RF_Public)		UIntProperty(CPP_PROPERTY(LightMapCoordinateIndex),TEXT(""),CPF_Edit);
	new(GetClass(),TEXT("LODDistanceRatio"),RF_Public)				UFloatProperty(CPP_PROPERTY(LODDistanceRatio),TEXT(""),CPF_Edit);

	UArrayProperty *TagsProp = new(GetClass(),TEXT("ContentTags"),RF_Public) UArrayProperty(CPP_PROPERTY(ContentTags),TEXT(""),CPF_Edit);
	TagsProp->Inner = new(TagsProp,TEXT("NameProp1"),RF_Public) UNameProperty(EC_CppProperty,0,TEXT(""),CPF_Edit);

	/**
	 * The following code creates a dynamic array of structs, where the struct contains a dynamic array of MaterialInstances...In unrealscript, this declaration
	 * would look something like this:
	 *
	 *	struct StaticMeshLODInfo
	 *	{
	 *		var() editfixedsize native array<MaterialInterface> EditorMaterials;
	 *	};
	 * 
	 *	var() editfixedsize native array<StaticMeshLODInfo> LODInfo;
	 *
	 */

	//////////////////////////////////////////////////////////////////////////
	// First create StaticMeshLODElement struct
	UScriptStruct* LODElementStruct = new(GetClass(),TEXT("StaticMeshLODElement"),RF_Public|RF_Transient|RF_Native) UScriptStruct(NULL);
	INT StructPropertyOffset = 0;
	new(LODElementStruct,TEXT("Material"),RF_Public)				UObjectProperty(EC_CppProperty,StructPropertyOffset,TEXT(""),CPF_Edit,UMaterialInterface::StaticClass());
	StructPropertyOffset += sizeof(UMaterialInterface*);
	new(LODElementStruct,TEXT("bEnableShadowCasting"),RF_Public)	UBoolProperty(EC_CppProperty,StructPropertyOffset,TEXT(""),CPF_Edit | CPF_Native);
	StructPropertyOffset += sizeof(UBOOL);
	new(LODElementStruct,TEXT("bEnableCollision"),RF_Public)		UBoolProperty(EC_CppProperty,StructPropertyOffset,TEXT(""),CPF_Edit | CPF_Native);

	// We're finished adding properties to the FStaticMeshLODElement struct - now we link the struct's properties (which sets the PropertiesSize for the struct) and initialize its defaults
	LODElementStruct->SetPropertiesSize(sizeof(FStaticMeshLODElement));
	LODElementStruct->AllocateStructDefaults();
	FArchive ArDummy0;
	LODElementStruct->Link(ArDummy0,0);

	//////////////////////////////////////////////////////////////////////////
	// Then create the StaticMeshLODInfo struct
	UScriptStruct* LODStruct = new(GetClass(),TEXT("StaticMeshLODInfo"),RF_Public|RF_Transient|RF_Native) UScriptStruct(NULL);

	// Next, create the dynamic array of LODElements - use the struct as the outer for the new array property so that the array is contained by the struct
	UArrayProperty*	ElementsProp = new(LODStruct,TEXT("Elements"),RF_Public) UArrayProperty(EC_CppProperty,0,TEXT(""),CPF_Edit | CPF_EditFixedSize | CPF_Native);

	// Dynamic arrays have an Inner property which corresponds to the array type.
	ElementsProp->Inner	= new(ElementsProp,TEXT("StructProperty1"),RF_Public) UStructProperty(EC_CppProperty,0,TEXT(""),CPF_Edit,LODElementStruct);

	// Link defaults
	LODStruct->SetPropertiesSize(sizeof(FStaticMeshLODInfo));
	LODStruct->AllocateStructDefaults();
	FArchive ArDummy1;
	LODStruct->Link(ArDummy1,0);

	//////////////////////////////////////////////////////////////////////////
	// Finally add array property to the StaticMesh class itself.

	// Next, create the dynamic array of StaticMeshLODInfo structs...same procedure as creating the Materials array above, except that this time, we use the class as the Outer for
	// the array, so that the property becomes a member of the class
	UArrayProperty*	InfoProp = new(GetClass(),TEXT("LODInfo"),RF_Public) UArrayProperty(CPP_PROPERTY(LODInfo),TEXT(""),CPF_Edit | CPF_EditFixedSize | CPF_Native);
	InfoProp->Inner = new(InfoProp,TEXT("StructProperty0"),RF_Public) UStructProperty(EC_CppProperty,0,TEXT(""),CPF_Edit,LODStruct);

	// Add physics body setup
	new(GetClass(),TEXT("BodySetup"),RF_Public)							UObjectProperty(CPP_PROPERTY(BodySetup),TEXT(""),CPF_Edit | CPF_EditInline, URB_BodySetup::StaticClass());

	UClass* TheClass = GetClass();
	TheClass->EmitObjectReference( STRUCT_OFFSET( UStaticMesh, BodySetup ) ); //@todo rtgc: is this needed seeing that BodySetup is exposed above?
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UStaticMesh::InitializeIntrinsicPropertyValues()
{
	UseSimpleLineCollision		= TRUE;
	UseSimpleBoxCollision		= TRUE;
	UseSimpleRigidBodyCollision = TRUE;
	UseFullPrecisionUVs			= FALSE;
	bUsedForInstancing			= FALSE;
	LODDistanceRatio			= 1.0;
	LODMaxRange					= 2000;
	ElementToIgnoreForTexFactor	= -1;
	HighResSourceMeshCRC = 0;
}

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void UStaticMesh::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );
	for( INT LODIndex=0; LODIndex<LODModels.Num(); LODIndex++ )
	{
		const FStaticMeshRenderData& LODRenderData = LODModels(LODIndex);
		for( INT ElementIndex=0; ElementIndex<LODRenderData.Elements.Num(); ElementIndex++ ) 
		{
			AddReferencedObject( ObjectArray, LODRenderData.Elements(ElementIndex).Material );
		}
	}
}

void UStaticMesh::PreEditChange(UProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	// Release the static mesh's resources.
	ReleaseResources();

	// Flush the resource release commands to the rendering thread to ensure that the edit change doesn't occur while a resource is still
	// allocated, and potentially accessing the UStaticMesh.
	ReleaseResourcesFence.Wait();
}

void UStaticMesh::PostEditChange(UProperty* PropertyThatChanged)
{
	// Ensure that LightMapResolution is either 0 or a power of two.
	if( LightMapResolution > 0 )
	{
		LightMapResolution = appRoundUpToPowerOfTwo( LightMapResolution );
	}
	else
	{
		LightMapResolution = 0;
	}

	UBOOL	bNeedsRebuild = FALSE;

	const UPackage* ExistingPackage = GetOutermost();
	if( ExistingPackage->PackageFlags & PKG_Cooked )
	{
		// We can't rebuild the mesh because it has been cooked and
		// therefore the raw mesh data has been stripped.
	}
	else
	{
		// If any of the elements have had collision added or removed, rebuild the static mesh.
		// If any of the elements have had materials altered, update here
		for(INT i=0;i<LODModels.Num();i++)
		{
			FStaticMeshRenderData&	RenderData				= LODModels(i);
			FStaticMeshLODInfo& ThisLODInfo = LODInfo(i);

			for(INT ElementIndex = 0;ElementIndex < RenderData.Elements.Num();ElementIndex++)
			{
				if(ElementIndex < ThisLODInfo.Elements.Num())
				{
					FStaticMeshLODElement& ElementInfo = ThisLODInfo.Elements(ElementIndex);
					UMaterialInterface*& EditorMat = ElementInfo.Material;

					// This only NULLs out direct references to materials.  We purposefully do not NULL
					// references to material instances that refer to decal materials, and instead just
					// warn about them during map checking.  The reason is that the side effects of NULLing
					// refs to instances pointing to decal materials would be strange (e.g. changing an
					// instance to refer to a decal material instead would then cause references to the
					// instance to break.
					if ( EditorMat && EditorMat->IsA( UDecalMaterial::StaticClass() ) )
					{
						EditorMat = NULL;
					}

					const UBOOL bEditorEnableShadowCasting = ElementInfo.bEnableShadowCasting;
					const UBOOL bEditorEnableCollision = ElementInfo.bEnableCollision;

					// Copy from UI expose array to 'real' array.
					FStaticMeshElement&	MeshElement				= RenderData.Elements(ElementIndex);

					MeshElement.Material				= EditorMat;
					MeshElement.bEnableShadowCasting	= bEditorEnableShadowCasting;
					MeshElement.EnableCollision			= bEditorEnableCollision;
					if(MeshElement.OldEnableCollision != MeshElement.EnableCollision)
					{
						bNeedsRebuild = TRUE;
						MeshElement.OldEnableCollision = MeshElement.EnableCollision;
					}
				}
			}
		}

		// if UV storage precision has changed then rebuild from source vertex data
		if( PropertyThatChanged != NULL && 
			PropertyThatChanged->GetFName() == FName(TEXT("UseFullPrecisionUVs")) )
		{
			if( !UseFullPrecisionUVs && !GVertexElementTypeSupport.IsSupported(VET_Half2) )
			{
				UseFullPrecisionUVs = TRUE;
				warnf(TEXT("16 bit UVs not supported. Reverting to 32 bit UVs"));			
			}
			if( LODModels.Num() > 0 )
			{
				FStaticMeshRenderData& RenderData = LODModels(0);
				if( UseFullPrecisionUVs != RenderData.VertexBuffer.GetUseFullPrecisionUVs() )
				{
					bNeedsRebuild = TRUE;
				}
			}
		}
	}

	if(bNeedsRebuild)
	{
		Build();
	}
	else
	{
        // Reinitialize the static mesh's resources.		
		InitResources();

		FStaticMeshComponentReattachContext(this);		
	}
	
	Super::PostEditChange(PropertyThatChanged);
}

void UStaticMesh::BeginDestroy()
{
	Super::BeginDestroy();
	ReleaseResources();

	// Free any physics-engine per-poly meshes.
	ClearPhysMeshCache();
}

UBOOL UStaticMesh::IsReadyForFinishDestroy()
{
	return ReleaseResourcesFence.GetNumPendingFences() == 0;
}

//
//	UStaticMesh::Rename
//

UBOOL UStaticMesh::Rename( const TCHAR* InName, UObject* NewOuter, ERenameFlags Flags )
{
	// Rename the static mesh
    return Super::Rename( InName, NewOuter, Flags );
}


/**
 * Called after duplication & serialization and before PostLoad. Used to e.g. make sure UStaticMesh's UModel
 * gets copied as well.
 */
void UStaticMesh::PostDuplicate()
{
	Super::PostDuplicate();
}

/**
 *	UStaticMesh::Serialize
 */
void UStaticMesh::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar << Bounds;
	Ar << BodySetup;
	Ar << kDOPTree;

	if( Ar.IsLoading() )
	{
		Ar << InternalVersion;
	}
	else if( Ar.IsSaving() )
	{
		InternalVersion = STATICMESH_VERSION;
		Ar << InternalVersion;
	}

	// serialize the content tags if correct version is serialized
	const INT STATICMESH_VERSION_CONTENT_TAGS = 17;
	if (InternalVersion >= STATICMESH_VERSION_CONTENT_TAGS)
	{
		Ar << ContentTags;
	}

	LODModels.Serialize( Ar, this );
	
	Ar << LODInfo;
	
	Ar << ThumbnailAngle;

	Ar << ThumbnailDistance;
	
	if( Ar.IsCountingMemory() )
	{
		Ar << LODMaxRange;
		Ar << PhysMeshScale3D;

		//TODO: Count these members when calculating memory used
		//Ar << kDOPTreeType;
		//Ar << PhysMesh;
		//Ar << ReleaseResourcesFence;
	}

	if( !Ar.IsLoading() || Ar.Ver() >= VER_STATICMESH_VERSION_18 )
	{
		Ar << HighResSourceMeshName;
		Ar << HighResSourceMeshCRC;
	}

#if !CONSOLE
	// Strip away loaded Editor-only data if we're a client and never care about saving.
	if( Ar.IsLoading() && GIsClient && !GIsEditor && !GIsUCC )
	{
		// Console platform is not a mistake, this ensures that as much as possible will be tossed.
		StripData( UE3::PLATFORM_Console );
	}
#endif
}

//
//	UStaticMesh::PostLoad
//

void UStaticMesh::PostLoad()
{
	Super::PostLoad();

	// Rebuild static mesh if internal version has been bumped and we allow meshes in this package to be rebuilt. E.g. during package resave we only want to
	// rebuild static meshes in the package we are going to save to cut down on the time it takes to resave all.
	if(InternalVersion < STATICMESH_VERSION && (GStaticMeshPackageNameToRebuild == NAME_None || GStaticMeshPackageNameToRebuild == GetOutermost()->GetFName()) )
	{	
		// Don't bother rebuilding if there are only minor format differences
		UBOOL bOnlyMinorDifferences = FALSE;

		// NOTE: Version 18 only introduced a named reference to a 'high res source mesh name', no structural differences
		const INT STATICMESH_VERSION_SIMPLIFICATION = 18;
		if( STATICMESH_VERSION == STATICMESH_VERSION_SIMPLIFICATION &&
			( InternalVersion == STATICMESH_VERSION - 1 ) )
		{
			bOnlyMinorDifferences = TRUE;
		}

		if( !bOnlyMinorDifferences )
		{
			Build();
		}
	}

	if( !UEngine::ShadowVolumesAllowed() )
	{
		RemoveShadowVolumeData();
	}

	if( !GIsUCC && !HasAnyFlags(RF_ClassDefaultObject) )
	{
		InitResources();
	}
	check(LODModels.Num() == LODInfo.Num());
}

/**
 * Used by various commandlets to purge Editor only data from the object.
 * 
 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
 */
void UStaticMesh::StripData(UE3::EPlatformType TargetPlatform)
{
	Super::StripData(TargetPlatform);

	// RawTriangles is only used in the Editor and for rebuilding static meshes.
	for( INT i=0; i<LODModels.Num(); i++ )
	{
		FStaticMeshRenderData& LODModel = LODModels(i);
		LODModel.RawTriangles.RemoveBulkData();
		LODModel.WireframeIndexBuffer.Indices.Empty();
	}

	// remove shadow volume data based on engine config setting
	if( !UEngine::ShadowVolumesAllowed() )
	{
		RemoveShadowVolumeData();
	}

	// HighResSourceMeshName is used for mesh simplification features in the editor
	HighResSourceMeshName.Empty();
}

/**
* Used by the cooker to pre cache the convex data for a given static mesh.  
* This data is stored with the level.
* @param Level - the level the data is cooked for
* @param TotalScale3D - the scale to cook the data at
* @param Owner - owner of this mesh for debug purposes (can be NULL)
* @param TriByteCount - running total of memory usage for per-tri collision
* @param TriMeshCount - running count of per-tri collision cache
* @param HullByteCount - running total of memory usage for hull cache
* @param HullCount - running count of hull cache
*/
void UStaticMesh::CookPhysConvexDataForScale(ULevel* Level, const FVector& TotalScale3D, const AActor* Owner, INT& TriByteCount, INT& TriMeshCount, INT& HullByteCount, INT& HullCount)
{
	// If we are doing per-tri collision..
	if(!UseSimpleRigidBodyCollision)
	{
        check(Level);

		// See if we already have cached data for this mesh at this scale.
		FKCachedPerTriData* TestData = Level->FindPhysPerTriStaticMeshCachedData(this, TotalScale3D);
		if(!TestData)
		{
			// If not, cook it now and add to store and map
			INT NewPerTriDataIndex = Level->CachedPhysPerTriSMDataStore.AddZeroed();
			FKCachedPerTriData* NewPerTriData = &Level->CachedPhysPerTriSMDataStore(NewPerTriDataIndex);

			FCachedPerTriPhysSMData NewCachedData;
			NewCachedData.Scale3D = TotalScale3D;
			NewCachedData.CachedDataIndex = NewPerTriDataIndex;

			FString DebugName = FString::Printf(TEXT("%s %s"), *Level->GetName(), *GetName() );
			MakeCachedPerTriMeshDataForStaticMesh( NewPerTriData, this, TotalScale3D, *DebugName );

			// Log to memory used total.
			TriByteCount += NewPerTriData->CachedPerTriData.Num();
			TriMeshCount++;

			Level->CachedPhysPerTriSMDataMap.Add( this, NewCachedData );

			//debugf( TEXT("Added PER-TRI: %s @ [%f %f %f]"), *SMComp->StaticMesh->GetName(), TotalScale3D.X, TotalScale3D.Y, TotalScale3D.Z );
		}
	}
	// If we have simplified collision..
	// And it has some convex bits
	else if(BodySetup && BodySetup->AggGeom.ConvexElems.Num() > 0)
	{
		check(Level);

		// First see if its already in the cache
		FKCachedConvexData* TestData = Level->FindPhysStaticMeshCachedData(this, TotalScale3D);

		// If not, cook it and add it.
		if(!TestData)
		{
			// Create new struct for the cache
			INT NewConvexDataIndex = Level->CachedPhysSMDataStore.AddZeroed();
			FKCachedConvexData* NewConvexData = &Level->CachedPhysSMDataStore(NewConvexDataIndex);

			FCachedPhysSMData NewCachedData;
			NewCachedData.Scale3D = TotalScale3D;
			NewCachedData.CachedDataIndex = NewConvexDataIndex;

			// Cook the collision geometry at the scale its used at in-level.
			FString DebugName = FString::Printf(TEXT("%s %s"), Owner ? *Owner->GetName() : TEXT("NoOwner"), *GetName() );
			MakeCachedConvexDataForAggGeom( NewConvexData, BodySetup->AggGeom.ConvexElems, TotalScale3D, *DebugName );

			// Add to memory used total.
			for(INT HullIdx = 0; HullIdx < NewConvexData->CachedConvexElements.Num(); HullIdx++)
			{
				FKCachedConvexDataElement& Hull = NewConvexData->CachedConvexElements(HullIdx);
				HullByteCount += Hull.ConvexElementData.Num();
				HullCount++;
			}

			// And add to the cache.
			Level->CachedPhysSMDataMap.Add( this, NewCachedData );

			//debugf( TEXT("Added SIMPLE: %d - %s @ [%f %f %f]"), NewConvexDataIndex, *GetName(), TotalScale3D.X, TotalScale3D.Y, TotalScale3D.Z );
		}
	}
}
//
//	UStaticMesh::GetDesc
//

/** 
 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
 */
FString UStaticMesh::GetDesc()
{
	//@todo: Handle LOD descs here?
	return FString::Printf( TEXT("%d Tris, %d Verts"), LODModels(0).IndexBuffer.Indices.Num() / 3, LODModels(0).NumVertices );
}

/** 
 * Returns detailed info to populate listview columns
 */
FString UStaticMesh::GetDetailedDescription( INT InIndex )
{
	FString Description = TEXT( "" );
	switch( InIndex )
	{
	case 0:
		Description = FString::Printf( TEXT( "%d Triangles" ), LODModels(0).IndexBuffer.Indices.Num() / 3 );
		break;
	case 1: 
		Description = FString::Printf( TEXT( "%d Vertices" ), LODModels(0).NumVertices );
		break;
	}
	return( Description );
}


/**
 * This will return detail info about this specific object. (e.g. AudioComponent will return the name of the cue,
 * ParticleSystemComponent will return the name of the ParticleSystem)  The idea here is that in many places
 * you have a component of interest but what you really want is some characteristic that you can use to track
 * down where it came from.  
 *
 */
FString UStaticMeshComponent::GetDetailedInfoInternal() const
{
	FString Result;  

	if( StaticMesh != NULL )
	{
		Result = StaticMesh->GetPathName( NULL );
	}
	else
	{
		Result = TEXT("No_StaticMesh");
	}

	return Result;  
}

FString ADynamicSMActor::GetDetailedInfoInternal() const
{
	FString Result;  

	if( StaticMeshComponent != NULL )
	{
		Result = StaticMeshComponent->GetDetailedInfoInternal();
	}
	else
	{
		Result = TEXT("No_StaticMeshComponent");
	}

	return Result;  
}


FString AStaticMeshActor::GetDetailedInfoInternal() const
{
	FString Result;  

	if( StaticMeshComponent != NULL )
	{
		Result = StaticMeshComponent->GetDetailedInfoInternal();
	}
	else
	{
		Result = TEXT("No_StaticMeshComponent");
	}

	return Result;  
}

/** 
* Removes all vertex data needed for shadow volume rendering 
*/
void UStaticMesh::RemoveShadowVolumeData()
{
#if !CONSOLE
	for( INT LODIdx=0; LODIdx < LODModels.Num(); LODIdx++ )
	{
		FStaticMeshRenderData& LODModel = LODModels(LODIdx);
		LODModel.Edges.Empty();
		LODModel.ShadowTriangleDoubleSided.Empty();
		LODModel.PositionVertexBuffer.RemoveShadowVolumeVertices(LODModel.NumVertices);
		LODModel.ShadowExtrusionVertexBuffer.RemoveShadowVolumeVertices(LODModel.NumVertices);
		LODModel.VertexBuffer.RemoveShadowVolumeVertices(LODModel.NumVertices);
	}
#endif
}



/**
 * Attempts to load this mesh's high res source static mesh
 *
 * @return The high res source mesh, or NULL if it failed to load
 */
UStaticMesh* UStaticMesh::LoadHighResSourceMesh() const
{
	UStaticMesh* HighResSourceStaticMesh = NULL;

	const UBOOL bIsAlreadySimplified = ( HighResSourceMeshName.Len() > 0 );
	if( bIsAlreadySimplified )
	{
		// Mesh in editor is already simplified.  We'll need to track down the original source mesh.

		// HighResSourceMesh name is a *path name*, so it has the package and the name in a single string
		INT DotPos = HighResSourceMeshName.InStr( TEXT( "." ) );
		check( DotPos > 0 );

		// Grab everything before the dot; this is the package name
		FString PackageName = HighResSourceMeshName.Left( DotPos );

		// Grab everything after the dot; this is the object name
		FString ObjectName = HighResSourceMeshName.Right( HighResSourceMeshName.Len() - ( DotPos + 1 ) );

		check( PackageName.Len() > 0 );
		check( ObjectName.Len() > 0 );


		// OK, now load the source mesh from the package
		// NOTE: Unless the package is already loaded, this will fail!
		HighResSourceStaticMesh =
			LoadObject<UStaticMesh>( NULL, *HighResSourceMeshName, NULL, LOAD_None, NULL );
		if( HighResSourceStaticMesh == NULL )
		{
			// @todo: We probably shouldn't need to have two separate loading paths here

			// Load the package the source mesh is in
			// NOTE: Until the package is SAVED, this will fail!
			UPackage* SourceMeshPackage = UObject::LoadPackage( NULL, *PackageName, 0 );
			if( SourceMeshPackage != NULL )
			{
				// OK, now load the source mesh from the package
				HighResSourceStaticMesh = LoadObject<UStaticMesh>( SourceMeshPackage, *ObjectName, NULL, LOAD_None, NULL );
			}
		}
	}

	return HighResSourceStaticMesh;
}


/**
 * Computes a CRC for this mesh to be used with mesh simplification tests.
 *
 * @return Simplified CRC for this mesh
 */
DWORD UStaticMesh::ComputeSimplifiedCRCForMesh() const
{
	TArray< BYTE > MeshBytes;

	// @todo: Ideally we'd check more aspects of the mesh, like material bindings

	if( LODModels.Num() > 0 )
	{
		const FStaticMeshRenderData& LOD = LODModels( 0 );

		// Position vertex data
		{
			const INT PosVertexBufferDataSize =
				LOD.PositionVertexBuffer.GetStride() * LOD.PositionVertexBuffer.GetNumVertices();
			const void* PosVertexBufferDataPtr =
				&LOD.PositionVertexBuffer.VertexPosition( 0 );

			const INT CurMeshBytes = MeshBytes.Num();
			MeshBytes.Add( PosVertexBufferDataSize );
			void* DestPtr = &MeshBytes( CurMeshBytes );
			
			appMemcpy( DestPtr, PosVertexBufferDataPtr, PosVertexBufferDataSize );
		}

		// Other vertex data
		{
			const INT VertexBufferDataSize =
				LOD.VertexBuffer.GetStride() * LOD.PositionVertexBuffer.GetNumVertices();
			const void* VertexBufferDataPtr =
				LOD.VertexBuffer.GetRawVertexData();

			const INT CurMeshBytes = MeshBytes.Num();
			MeshBytes.Add( VertexBufferDataSize );
			void* DestPtr = &MeshBytes( CurMeshBytes );
			
			appMemcpy( DestPtr, VertexBufferDataPtr, VertexBufferDataSize );
		}

		// Indices
		{
			const INT IndexBufferDataSize =
				LOD.IndexBuffer.Indices.GetResourceDataSize();
			const void* IndexBufferDataPtr =
				LOD.IndexBuffer.Indices.GetResourceData();

			const INT CurMeshBytes = MeshBytes.Num();
			MeshBytes.Add( IndexBufferDataSize );
			void* DestPtr = &MeshBytes( CurMeshBytes );
			
			appMemcpy( DestPtr, IndexBufferDataPtr, IndexBufferDataSize );
		}
	}


	// Compute CRC
	DWORD CRCValue = 0;
	if( MeshBytes.Num() > 0 )
	{
		CRCValue = appMemCrc( MeshBytes.GetData(), MeshBytes.Num(), 0 );
	}

	return CRCValue;
}


/*-----------------------------------------------------------------------------
UStaticMeshComponent
-----------------------------------------------------------------------------*/

void UStaticMeshComponent::InitResources()
{
	// Create the light-map resources.
	for(INT LODIndex = 0;LODIndex < LODData.Num();LODIndex++)
	{
		if(LODData(LODIndex).LightMap != NULL)
		{
			LODData(LODIndex).LightMap->InitResources();
		}
	}
}

void UStaticMeshComponent::PostEditUndo()
{
	// The component's light-maps are loaded from the transaction, so their resources need to be reinitialized.
	InitResources();

	Super::PostEditUndo();
}

/**
 * Called after all objects referenced by this object have been serialized. Order of PostLoad routed to 
 * multiple objects loaded in one set is not deterministic though ConditionalPostLoad can be forced to
 * ensure an object has been "PostLoad"ed.
 */
void UStaticMeshComponent::PostLoad()
{
	Super::PostLoad();

	// Initialize the resources for the freshly loaded component.
	InitResources();
}

/** Change the StaticMesh used by this instance. */
UBOOL UStaticMeshComponent::SetStaticMesh(UStaticMesh* NewMesh, UBOOL bForce)
{
	// Do nothing if we are already using the supplied static mesh
	if(NewMesh == StaticMesh && !bForce)
	{
		return FALSE;
	}

	// Don't allow changing static meshes if the owner is "static".
	if( !Owner || !Owner->bStatic )
	{
		// Terminate rigid-body data for this StaticMeshComponent
		TermComponentRBPhys(NULL);

		// Force the recreate context to be destroyed by going out of scope after setting the new static-mesh.
		{
			FComponentReattachContext ReattachContext(this);
			StaticMesh = NewMesh;
		}

		// Re-init the rigid body info.
		UBOOL bFixed = true;
		if(!Owner || Owner->Physics == PHYS_RigidBody)
		{
			bFixed = false;
		}

		if(IsAttached())
		{
			InitComponentRBPhys(bFixed);
		}
		return TRUE;
	}
	return FALSE;
}

/** Script version of SetStaticMesh. */
void UStaticMeshComponent::execSetStaticMesh(FFrame& Stack, RESULT_DECL)
{
	P_GET_OBJECT(UStaticMesh, NewMesh);
	P_GET_UBOOL_OPTX(bForceSet,FALSE);
	P_FINISH;

	*(UBOOL*)Result = SetStaticMesh(NewMesh,bForceSet);
}

/**
* Changes the value of bForceStaticDecals.
* @param bInForceStaticDecals - The value to assign to bForceStaticDecals.
*/
void UStaticMeshComponent::SetForceStaticDecals(UBOOL bInForceStaticDecals)
{
	if( bForceStaticDecals != bInForceStaticDecals )
	{
		bForceStaticDecals = bInForceStaticDecals;
		FComponentReattachContext ReattachContext(this);
	}
}

/** Script version of SetForceStaticDecals. */
void UStaticMeshComponent::execSetForceStaticDecals(FFrame& Stack, RESULT_DECL)
{
	P_GET_UBOOL(bInForceStaticDecals);
	P_FINISH;

	SetForceStaticDecals(bInForceStaticDecals);
}

/**
 * Returns whether this primitive only uses unlit materials.
 *
 * @return TRUE if only unlit materials are used for rendering, false otherwise.
 */
UBOOL UStaticMeshComponent::UsesOnlyUnlitMaterials() const
{
	if( StaticMesh )
	{
		// Figure out whether any of the sections has a lit material assigned.
		UBOOL bUsesOnlyUnlitMaterials = TRUE;
		for( INT ElementIndex=0; ElementIndex<StaticMesh->LODModels(0).Elements.Num(); ElementIndex++ )
		{
			UMaterialInterface*	MaterialInterface	= GetMaterial(ElementIndex);
			UMaterial*			Material			= MaterialInterface ? MaterialInterface->GetMaterial() : NULL;

			if( !Material || Material->LightingModel != MLM_Unlit )
			{
				bUsesOnlyUnlitMaterials = FALSE;
				break;
			}
		}
		return bUsesOnlyUnlitMaterials;
	}
	else
	{
		return FALSE;
	}
}

/**
 * Returns the lightmap resolution used for this primivite instnace in the case of it supporting texture light/ shadow maps.
 * 0 if not supported or no static shadowing.
 *
 * @param Width		[out]	Width of light/shadow map
 * @param Height	[out]	Height of light/shadow map
 */
void UStaticMeshComponent::GetLightMapResolution( INT& Width, INT& Height ) const
{
	if( StaticMesh )
	{
		// Use overriden per component lightmap resolution.
		if( bOverrideLightMapResolution )
		{
			Width	= OverriddenLightMapResolution;
			Height	= OverriddenLightMapResolution;
		}
		// Use the lightmap resolution defined in the static mesh.
		else
		{
			Width	= StaticMesh->LightMapResolution;
			Height	= StaticMesh->LightMapResolution;
		}
	}
	// No associated static mesh!
	else
	{
		Width	= 0;
		Height	= 0;
	}
}

/**
 * Returns the light and shadow map memory for this primite in its out variables.
 *
 * Shadow map memory usage is per light whereof lightmap data is independent of number of lights, assuming at least one.
 *
 * @param [out] LightMapMemoryUsage		Memory usage in bytes for light map (either texel or vertex) data
 * @param [out]	ShadowMapMemoryUsage	Memory usage in bytes for shadow map (either texel or vertex) data
 */
void UStaticMeshComponent::GetLightAndShadowMapMemoryUsage( INT& LightMapMemoryUsage, INT& ShadowMapMemoryUsage ) const
{
	// Zero initialize.
	ShadowMapMemoryUsage	= 0;
	LightMapMemoryUsage		= 0;

	// Cache light/ shadow map resolution.
	INT LightMapWidth		= 0;
	INT	LightMapHeight		= 0;
	GetLightMapResolution( LightMapWidth, LightMapHeight );

	// Determine whether static mesh/ static mesh component has static shadowing.
	if( HasStaticShadowing() && StaticMesh )
	{
		// Determine whether we are using a texture or vertex buffer to store precomputed data.
		if( LightMapWidth > 0 && LightMapHeight > 0
		&&	StaticMesh->LightMapCoordinateIndex >= 0 
		&&	(UINT)StaticMesh->LightMapCoordinateIndex < StaticMesh->LODModels(0).VertexBuffer.GetNumTexCoords() )
		{
			// Stored in texture.
			const FLOAT MIP_FACTOR = 1.33f;
			ShadowMapMemoryUsage	= appTrunc( MIP_FACTOR * LightMapWidth * LightMapHeight ); // G8
			const UINT NumLightMapCoefficients = GSystemSettings.bAllowDirectionalLightMaps ? NUM_DIRECTIONAL_LIGHTMAP_COEF : NUM_SIMPLE_LIGHTMAP_COEF;
			LightMapMemoryUsage		= appTrunc( NumLightMapCoefficients * MIP_FACTOR * LightMapWidth * LightMapHeight / 2 ); // DXT1
		}
		else
		{
			// Stored in vertex buffer.
			ShadowMapMemoryUsage	= sizeof(FLOAT) * StaticMesh->LODModels(0).NumVertices;
			const UINT LightMapSampleSize = GSystemSettings.bAllowDirectionalLightMaps ? sizeof(FQuantizedDirectionalLightSample) : sizeof(FQuantizedSimpleLightSample);
			LightMapMemoryUsage	= LightMapSampleSize * StaticMesh->LODModels(0).NumVertices;
		}
	}
}

INT UStaticMeshComponent::GetNumElements() const
{
	if(StaticMesh)
	{
		check(StaticMesh->LODModels.Num() >= 1);
		return StaticMesh->LODModels(0).Elements.Num();
	}
	else
	{
		return 0;
	}
}

/**
 *	UStaticMeshComponent::GetMaterial
 * @param MaterialIndex Index of material
 * @return Material instance for this component at index
 */
UMaterialInterface* UStaticMeshComponent::GetMaterial(INT MaterialIndex) const
{
	// Call GetMateiral using the base (zeroth) LOD.
	return GetMaterial( MaterialIndex, 0 );
}

/**
*	UStaticMeshComponent::GetMaterial
* @param MaterialIndex Index of material
* @param LOD Lod level to query from
* @return Material instance for this component at index
*/
UMaterialInterface* UStaticMeshComponent::GetMaterial(INT MaterialIndex, INT LOD) const
{
	// If we have a base materials array, use that
	if(MaterialIndex < Materials.Num() && Materials(MaterialIndex))
	{
		return Materials(MaterialIndex);
	}
	// Otherwise get from static mesh lod
	else
	{
		if(StaticMesh && LOD < StaticMesh->LODModels.Num())
		{
			// Find the first element that has the corresponding material index, and use its material.
			for(INT ElementIndex = 0;ElementIndex < StaticMesh->LODModels(LOD).Elements.Num();ElementIndex++)
			{
				const FStaticMeshElement& Element = StaticMesh->LODModels(LOD).Elements(ElementIndex);
				if(Element.MaterialIndex == MaterialIndex)
				{
					return Element.Material;
				}
			}
		}

		// If there wasn't a static mesh element with the given material index, return NULL.
		return NULL;
	}
}

UStaticMeshComponent::UStaticMeshComponent()
{
}

void UStaticMeshComponent::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects(ObjectArray);
	for(INT LODIndex = 0;LODIndex < LODData.Num();LODIndex++)
	{
		if(LODData(LODIndex).LightMap != NULL)
		{
			LODData(LODIndex).LightMap->AddReferencedObjects(ObjectArray);
		}
	}
}


//
//	UStaticMeshComponent::Serialize
//
void UStaticMeshComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	Ar << LODData;
}

/**
 * Returns whether native properties are identical to the one of the passed in component.
 *
 * @param	Other	Other component to compare against
 *
 * @return TRUE if native properties are identical, FALSE otherwise
 */
UBOOL UStaticMeshComponent::AreNativePropertiesIdenticalTo( UComponent* Other ) const
{
	UBOOL bNativePropertiesAreIdentical = Super::AreNativePropertiesIdenticalTo( Other );
	UStaticMeshComponent* OtherSMC = CastChecked<UStaticMeshComponent>(Other);

	if( bNativePropertiesAreIdentical )
	{
		// Components are not identical if they have lighting information.
		if( LODData.Num() || OtherSMC->LODData.Num() )
		{
			bNativePropertiesAreIdentical = FALSE;
		}
	}
	
	return bNativePropertiesAreIdentical;
}

/** 
 * Called when any property in this object is modified in UnrealEd
 *
 * @param	PropertyThatChanged		changed property
 */
void UStaticMeshComponent::PostEditChange( UProperty* PropertyThatChanged )
{
	// Ensure that OverriddenLightMapResolution is either 0 or a power of two.
	if( OverriddenLightMapResolution > 0 )
	{
		OverriddenLightMapResolution = appRoundUpToPowerOfTwo( OverriddenLightMapResolution );
	}
	else
	{
		OverriddenLightMapResolution = 0;
	}

	// This only NULLs out direct references to materials.  We purposefully do not NULL
	// references to material instances that refer to decal materials, and instead just
	// warn about them during map checking.  The reason is that the side effects of NULLing
	// refs to instances pointing to decal materials would be strange (e.g. changing an
	// instance to refer to a decal material instead would then cause references to the
	// instance to break.
	for( INT MaterialIndex = 0 ; MaterialIndex < Materials.Num() ; ++MaterialIndex )
	{
		UMaterialInterface*&	MaterialInterface = Materials(MaterialIndex);
		if( MaterialInterface && MaterialInterface->IsA( UDecalMaterial::StaticClass() ) )
		{
			MaterialInterface = NULL;
		}
	}

	// Ensure properties are in sane range.
	SubDivisionStepSize = Clamp( SubDivisionStepSize, 1, 128 );
	MaxSubDivisions		= Clamp( MaxSubDivisions, 2, 50 );
	MinSubDivisions		= Clamp( MinSubDivisions, 2, MaxSubDivisions );

	Super::PostEditChange( PropertyThatChanged );
}

void UStaticMeshComponent::CheckForErrors()
{
	Super::CheckForErrors();

	// Get the mesh owner's name.
	FString OwnerName(GNone);
	if ( Owner )
	{
		OwnerName = Owner->GetName();
	}

	// Warn about direct or indirect references to decal materials.
	for( INT MaterialIndex = 0 ; MaterialIndex < Materials.Num() ; ++MaterialIndex )
	{
		UMaterialInterface* MaterialInterface = Materials(MaterialIndex);
		if ( MaterialInterface )
		{
			if( MaterialInterface->IsA( UDecalMaterial::StaticClass() ) )
			{
				GWarn->MapCheck_Add(MCTYPE_WARNING, Owner, *FString::Printf(TEXT("%s::%s : Decal material %s is applied to a static mesh"), *GetName(), *OwnerName, *MaterialInterface->GetName() ), MCACTION_NONE, TEXT("DecalMaterialStaticMesh"));
			}

			const UMaterial* ReferencedMaterial = MaterialInterface->GetMaterial();
			if( ReferencedMaterial && ReferencedMaterial->IsA( UDecalMaterial::StaticClass() ) )
			{
				GWarn->MapCheck_Add(MCTYPE_WARNING, Owner, *FString::Printf(TEXT("%s::%s : Material instance %s refers to a decal material (%s) but is applied to a static mesh"), *GetName(), *OwnerName, *MaterialInterface->GetName(), *ReferencedMaterial->GetName() ), MCACTION_NONE, TEXT("DecalMaterialInstanceStaticMesh"));
			}
		}
	}


	// Make sure any simplified meshes can still find their high res source mesh
	if( StaticMesh != NULL )
	{
		const UBOOL bIsAlreadySimplified = ( StaticMesh->HighResSourceMeshName.Len() > 0 );
		if( bIsAlreadySimplified )
		{
			// Mesh in editor is already simplified.  We'll need to track down the original source mesh.
			UStaticMesh* HighResSourceStaticMesh = StaticMesh->LoadHighResSourceMesh();

			if( HighResSourceStaticMesh != NULL )
			{
				// Compute CRC of high resolution mesh
				const DWORD HighResMeshCRC = HighResSourceStaticMesh->ComputeSimplifiedCRCForMesh();

				if( HighResMeshCRC != StaticMesh->HighResSourceMeshCRC )
				{
					// It looks like the high res mesh has changed since we were generated.  We'll need
					// to let the user know about this.
					GWarn->MapCheck_Add(
						MCTYPE_WARNING, Owner,
						*FString::Printf( TEXT( "High res source mesh '%s' for simplified mesh '%s' appears to have changed.  You should use the Mesh Simplification tool to update the simplified mesh." ),
							*HighResSourceStaticMesh->GetName(), *StaticMesh->GetName() ), MCACTION_NONE );
				}
			}
			else
			{
				// Can't find the high res mesh?
				GWarn->MapCheck_Add(
					MCTYPE_WARNING, Owner,
					*FString::Printf( TEXT( "Unable to load high res source mesh '%s' for simplified mesh '%s'." ),
						*StaticMesh->HighResSourceMeshName, *StaticMesh->GetName() ), MCACTION_NONE );
			}
		}
	}

	// Make sure any non uniform scaled meshes have appropriate collision
	if ( StaticMesh != NULL && StaticMesh->BodySetup != NULL)
	{
		// Overall scale factor for this mesh.
		const FVector& TotalScale3D = Scale * Owner->DrawScale * Scale3D * Owner->DrawScale3D;
		if ( !TotalScale3D.IsUniform() &&
			 (StaticMesh->BodySetup->AggGeom.BoxElems.Num() > 0   ||
			  StaticMesh->BodySetup->AggGeom.SphylElems.Num() > 0 ||
			  StaticMesh->BodySetup->AggGeom.SphereElems.Num() > 0) )

		{
			GWarn->MapCheck_Add(
				MCTYPE_WARNING, Owner,
				*FString::Printf( TEXT( "Mesh '%s' has simple collision, but is being scaled non-uniformly, collision creation will fail." ),
				*StaticMesh->GetName() ), MCACTION_NONE );
		}
	}
}


void UStaticMeshComponent::UpdateBounds()
{
	if(StaticMesh)
	{
		// Graphics bounds.
		Bounds = StaticMesh->Bounds.TransformBy(LocalToWorld);
		
		// Add bounds of collision geometry (if present).
		if(StaticMesh->BodySetup)
		{
			FMatrix Transform;
			FVector Scale3D;
			GetTransformAndScale(Transform, Scale3D);

			FBox AggGeomBox = StaticMesh->BodySetup->AggGeom.CalcAABB(Transform, Scale3D);
			if (AggGeomBox.IsValid)
			{
				Bounds = LegacyUnion(Bounds,FBoxSphereBounds(AggGeomBox));
			}
		}

		// Takes into account that the static mesh collision code nudges collisions out by up to 1 unit.
		Bounds.BoxExtent += FVector(1,1,1);
		Bounds.SphereRadius += 1.0f;
	}
	else
	{
		Super::UpdateBounds();
	}
}

UBOOL UStaticMeshComponent::IsValidComponent() const
{
	return StaticMesh != NULL && StaticMesh->LODModels.Num() && StaticMesh->LODModels(0).NumVertices > 0 && Super::IsValidComponent();
}

void UStaticMeshComponent::Attach()
{
	// Check that the static-mesh hasn't been changed to be incompatible with the cached light-map.
	for(INT i=0;i<LODData.Num();i++)
	{
		FStaticMeshComponentLODInfo& LODInfo = LODData(i);
		if (!bUsePrecomputedShadows)
		{
			LODInfo.LightMap = NULL;
			LODInfo.ShadowMaps.Empty();
			LODInfo.ShadowVertexBuffers.Empty();
		}
		else if (LODInfo.LightMap)
		{
			const FLightMap1D* LightMap1D = LODData(i).LightMap->GetLightMap1D();
			if(	StaticMesh->LODModels.Num() != LODData.Num() || 
				(LightMap1D && LightMap1D->NumSamples() != StaticMesh->LODModels(i).NumVertices))
			{
				// If the vertex light-map doesn't have the same number of elements as the static mesh has vertices, discard the light-map.
				LODData(i).LightMap = NULL;
			}
		}
	}

	// Change the tick group based upon blocking or not
	TickGroup = TickGroup < TG_PostAsyncWork ? (BlockActors ? TG_PreAsyncWork : TG_DuringAsyncWork) : TG_PostAsyncWork;

	Super::Attach();
}

void UStaticMeshComponent::GetStaticTriangles(FPrimitiveTriangleDefinitionInterface* PTDI) const
{
	if(StaticMesh && StaticMesh->LODModels.Num() > 0)
	{
		const INT LODIndex = 0;
		const FStaticMeshRenderData& LODRenderData = StaticMesh->LODModels(LODIndex);

		// Compute the primitive transform's inverse transpose, for transforming normals.
		const FMatrix LocalToWorldInverseTranspose = LocalToWorld.Inverse().Transpose();

		// Reverse triangle vertex windings if the primitive is negatively scaled.
		const UBOOL bReverseWinding = LocalToWorldDeterminant < 0.0f;

		// Iterate over the static mesh's triangles.
		const INT NumTriangles = LODRenderData.GetTriangleCount();
		for(INT TriangleIndex = 0;TriangleIndex < NumTriangles;TriangleIndex++)
		{
			// Lookup the triangle's vertices;
			FPrimitiveTriangleVertex Vertices[3];
			for(INT TriangleVertexIndex = 0;TriangleVertexIndex < 3;TriangleVertexIndex++)
			{
				const WORD VertexIndex = LODRenderData.IndexBuffer.Indices(TriangleIndex * 3 + TriangleVertexIndex);
				FPrimitiveTriangleVertex& Vertex = Vertices[bReverseWinding ? 2 - TriangleVertexIndex : TriangleVertexIndex];

				Vertex.WorldPosition = LocalToWorld.TransformFVector(LODRenderData.PositionVertexBuffer.VertexPosition(VertexIndex));
				Vertex.WorldTangentX = LocalToWorld.TransformNormal(LODRenderData.VertexBuffer.VertexTangentX(VertexIndex)).SafeNormal();
				Vertex.WorldTangentY = LocalToWorld.TransformNormal(LODRenderData.VertexBuffer.VertexTangentY(VertexIndex)).SafeNormal();
				Vertex.WorldTangentZ = LocalToWorldInverseTranspose.TransformNormal(LODRenderData.VertexBuffer.VertexTangentZ(VertexIndex)).SafeNormal();
			}

			// Pass the triangle to the caller's interface.
			PTDI->DefineTriangle(
				Vertices[0],
				Vertices[1],
				Vertices[2]
				);
		}
	}
}

void UStaticMeshComponent::GetStreamingTextureInfo(TArray<FStreamingTexturePrimitiveInfo>& OutStreamingTextures) const
{
	check(StaticMesh);

	if(!bIgnoreInstanceForTextureStreaming)
	{
		const FSphere BoundingSphere = Bounds.GetSphere();
		const FLOAT LocalTexelFactor = StaticMesh->GetStreamingTextureFactor(0);
		const FLOAT WorldTexelFactor = LocalTexelFactor * GetMaximumAxisScale(LocalToWorld);

		// Process each material applied to the top LOD of the mesh. TODO: Handle lower LODs?
		for( INT ElementIndex = 0; ElementIndex < StaticMesh->LODModels(0).Elements.Num(); ElementIndex++ )
		{
			const FStaticMeshElement& Element = StaticMesh->LODModels(0).Elements(ElementIndex);
			UMaterialInterface* Material = GetMaterial(Element.MaterialIndex);
			if(!Material)
			{
				Material = GEngine->DefaultMaterial;
			}

			// Enumerate the textures used by the material.
			TArray<UTexture*> Textures;
			Material->GetTextures(Textures);

			// Add each texture to the output with the appropriate parameters.
			// TODO: Take into account which UVIndex is being used.
			for(INT TextureIndex = 0;TextureIndex < Textures.Num();TextureIndex++)
			{
				FStreamingTexturePrimitiveInfo& StreamingTexture = *new(OutStreamingTextures) FStreamingTexturePrimitiveInfo;
				StreamingTexture.Bounds = BoundingSphere;
				StreamingTexture.TexelFactor = WorldTexelFactor;
				StreamingTexture.Texture = Textures(TextureIndex);
			}
		}
	}
}

/* ==========================================================================================================
	AStaticMeshCollectionActor
========================================================================================================== */
/* === AActor interface === */
/**
 * Updates the CachedLocalToWorld transform for all attached components.
 */
void AStaticMeshCollectionActor::UpdateComponentsInternal( UBOOL bCollisionUpdate/*=FALSE*/ )
{
	checkf(!HasAnyFlags(RF_Unreachable), TEXT("%s"), *GetFullName());
	checkf(!HasAnyFlags(RF_ArchetypeObject|RF_ClassDefaultObject), TEXT("%s"), *GetFullName());
	checkf(!ActorIsPendingKill(), TEXT("%s"), *GetFullName());

	// Local to world used for attached non static mesh components.
	const FMatrix& ActorToWorld = LocalToWorld();

	for(INT ComponentIndex = 0;ComponentIndex < Components.Num();ComponentIndex++)
	{	
		UActorComponent* ActorComponent = Components(ComponentIndex);
		if( ActorComponent != NULL )
		{
			UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(ActorComponent);
			if( MeshComponent )
			{
				// never reapply the CachedParentToWorld transform for our StaticMeshComponents, since it will never change.
				MeshComponent->UpdateComponent(GWorld->Scene, this, MeshComponent->CachedParentToWorld);
			}
			else
			{
				ActorComponent->UpdateComponent(GWorld->Scene, this, ActorToWorld);
			}
		}
	}
}


/* === UObject interface === */
/**
 * Serializes the LocalToWorld transforms for the StaticMeshComponents contained in this actor.
 */
void AStaticMeshCollectionActor::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);

	if (!HasAnyFlags(RF_ClassDefaultObject) && Ar.GetLinker() != NULL )
	{
		if ( Ar.IsLoading() )
		{
			FMatrix IdentityMatrix;
			for ( INT CompIndex = 0; CompIndex < StaticMeshComponents.Num(); CompIndex++ )
			{
				if ( StaticMeshComponents(CompIndex) != NULL )
				{
					Ar << StaticMeshComponents(CompIndex)->CachedParentToWorld;
				}
				else
				{
					// even if we had a NULL component for whatever reason, we still need to read the matrix data
					// from the stream so that we de-serialize the same amount of data that was serialized.
					Ar << IdentityMatrix;
				}
			}

			Components = (TArrayNoInit<UActorComponent*>&)StaticMeshComponents;
			StaticMeshComponents.Empty();
		}
		else if ( Ar.IsSaving() )
		{
			// serialize the default matrix for any components which are NULL so that we are always guaranteed to
			// de-serialize the correct amount of data
			FMatrix IdentityMatrix(FMatrix::Identity);
			for ( INT CompIndex = 0; CompIndex < StaticMeshComponents.Num(); CompIndex++ )
			{
				if ( StaticMeshComponents(CompIndex) != NULL )
				{
					Ar << StaticMeshComponents(CompIndex)->CachedParentToWorld;
				}
				else
				{
					Ar << IdentityMatrix;
				}
			}
		}
	}
}


// EOF




