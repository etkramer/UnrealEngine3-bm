/*=============================================================================
	UnLightMap.h: Light-map definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * The abstract base class of 1D and 2D light-maps.
 */
class FLightMap : private FDeferredCleanupInterface
{
public:
	enum
	{
		LMT_None = 0,
		LMT_1D = 1,
		LMT_2D = 2,
	};

	/** The GUIDs of lights which this light-map stores. */
	TArray<FGuid> LightGuids;

	/** Default constructor. */
	FLightMap(UBOOL InbAllowDirectionalLightMaps=GSystemSettings.bAllowDirectionalLightMaps) : 
		bAllowDirectionalLightMaps(InbAllowDirectionalLightMaps),
		NumRefs(0) 
	{}

	/** Destructor. */
	virtual ~FLightMap() { check(!NumRefs); }

	/**
	 * Checks if a light is stored in this light-map.
	 * @param	LightGuid - The GUID of the light to check for.
	 * @return	True if the light is stored in the light-map.
	 */
	UBOOL ContainsLight(const FGuid& LightGuid) const
	{
		return LightGuids.FindItemIndex(LightGuid) != INDEX_NONE;
	}

	// FLightMap interface.
	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray ) {}
	virtual void Serialize(FArchive& Ar);
	virtual void InitResources() {}
	virtual FLightMapInteraction GetInteraction() const = 0;

	// Runtime type casting.
	virtual const class FLightMap1D* GetLightMap1D() const { return NULL; }
	virtual const class FLightMap2D* GetLightMap2D() const { return NULL; }

	// Reference counting.
	void AddRef()
	{
		NumRefs++;
	}
	void Release()
	{
		if(--NumRefs == 0)
		{
			Cleanup();
		}
	}

	/**
	* @return TRUE if non-simple lightmaps are allowed
	*/
	FORCEINLINE UBOOL AllowsDirectionalLightmaps() const
	{
		return bAllowDirectionalLightMaps;
	}

protected:

	/** Indicates whether the lightmap is being used for directional or simple lighting. */
	const UBOOL bAllowDirectionalLightMaps;

	/**
	 * Called when the light-map is no longer referenced.  Should release the lightmap's resources.
	 */
	virtual void Cleanup()
	{
		BeginCleanup(this);
	}

private:
	INT NumRefs;
	
	// FDeferredCleanupInterface
	virtual void FinishCleanup();
};

/** A reference to a light-map. */
typedef TRefCountPtr<FLightMap> FLightMapRef;

/** Lightmap reference serializer */
extern FArchive& operator<<(FArchive& Ar,FLightMap*& R);

/** Helper class to allow for forcing simple lighting during serialization */
class FLightMapSerializeHelper
{
public:
	FLightMapSerializeHelper(UBOOL InbAllowDirectionalLightMaps,FLightMapRef& InLightMapRef) 
		:	bAllowDirectionalLightMaps(InbAllowDirectionalLightMaps)
		,	LightMapRef(InLightMapRef)
	{}
	const UBOOL bAllowDirectionalLightMaps;
	FLightMapRef& LightMapRef;
};
extern FArchive& operator<<(FArchive& Ar,FLightMapSerializeHelper& R);

/** The light incident for a point on a surface. */
struct FLightSample
{
	/** The lighting coefficients, colored. */
	FLOAT Coefficients[NUM_GATHERED_LIGHTMAP_COEF][3];

	/** True if this sample maps to a valid point on a surface. */
	UBOOL bIsMapped;

	/** Initialization constructor. */
	FLightSample():
		bIsMapped(FALSE)
	{
		appMemzero(Coefficients,sizeof(Coefficients));
	}

	/**
	 * Constructs a light sample representing a point light.
	 * @param Color - The color/intensity of the light at the sample point.
	 * @param Direction - The direction toward the light at the sample point.
	 */
	static FLightSample PointLight(const FLinearColor& Color,const FVector& Direction);

	/**
	 * Constructs a light sample representing a sky light.
	 * @param UpperColor - The color/intensity of the sky light's upper hemisphere.
	 * @param LowerColor - The color/intensity of the sky light's lower hemisphere.
	 * @param WorldZ - The world's +Z axis in tangent space.
	 */
	static FLightSample SkyLight(const FLinearColor& UpperColor,const FLinearColor& LowerColor,const FVector& WorldZ);

	/**
	 * Adds a weighted light sample to this light sample.
	 * @param OtherSample - The sample to add.
	 * @param Weight - The weight to multiply the other sample by before addition.
	 */
	void AddWeighted(const FLightSample& OtherSample,FLOAT Weight);
};

/**
 * The raw data which is used to construct a 2D light-map.
 */
class FLightMapData2D
{
public:

	/** The lights which this light-map stores. */
	TArray<ULightComponent*> Lights;

	/**
	 * Minimal initialization constructor.
	 */
	FLightMapData2D(UINT InSizeX,UINT InSizeY):
		Data(InSizeX * InSizeY),
		SizeX(InSizeX),
		SizeY(InSizeY)
	{}

	// Accessors.
	const FLightSample& operator()(UINT X,UINT Y) const { return Data(SizeX * Y + X); }
	FLightSample& operator()(UINT X,UINT Y) { return Data(SizeX * Y + X); }
	UINT GetSizeX() const { return SizeX; }
	UINT GetSizeY() const { return SizeY; }

private:

	/** The incident light samples for a 2D array of points on the surface. */
	TChunkedArray<FLightSample> Data;

	/** The width of the light-map. */
	UINT SizeX;

	/** The height of the light-map. */
	UINT SizeY;
};

/**
 * A 2D texture containing lightmap coefficients.
 */
class ULightMapTexture2D: public UTexture2D
{
	DECLARE_CLASS(ULightMapTexture2D,UTexture2D,CLASS_Intrinsic,Engine)

	// UObject interface.
	virtual void Serialize( FArchive& Ar );

	/** 
	 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
	 */
	virtual FString GetDesc();

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT InIndex );
};

/**
 * A 2D array of incident lighting data.
 */
class FLightMap2D : public FLightMap
{
public:

	FLightMap2D(UBOOL InbAllowDirectionalLightMaps=GSystemSettings.bAllowDirectionalLightMaps);

	// FLightMap2D interface.

	/**
	 * Returns the texture containing the RGB coefficients for a specific basis.
	 * @param	BasisIndex - The basis index.
	 * @param	OutScale - The scale to apply to the coefficients of the texture.
	 * @return	The RGB coefficient texture.
	 */
	const UTexture2D* GetTexture(UINT BasisIndex) const;

	// FLightMap interface.
	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray );

	virtual void Serialize(FArchive& Ar);
	virtual FLightMapInteraction GetInteraction() const;

	// Runtime type casting.
	virtual const FLightMap2D* GetLightMap2D() const { return this; }

	/**
	 * Allocates texture space for the light-map and stores the light-map's raw data for deferred encoding.
	 * If the light-map has no lights in it, it will return NULL.
	 * @param	LightMapOuter - The package to create the light-map and textures in.
	 * @param	InRawData - The raw light-map data to fill the texture with.
	 * @param	Material - The material which the light-map will be rendered with.  Used as a hint to pack light-maps for the same material in the same texture.  Can be NULL.
	 * @param	Bounds - The bounds of the primitive the light-map will be rendered on.  Used as a hint to pack light-maps on nearby primitives in the same texture.
	 */
	static class FLightMap2D* AllocateLightMap(UObject* LightMapOuter,FLightMapData2D* RawData,UMaterialInterface* Material, const FBoxSphereBounds& Bounds);

	/**
	 * Executes all pending light-map encoding requests.
	 */
	static void FinishEncoding();

protected:

	friend struct FLightMapPendingTexture;

	FLightMap2D(const TArray<FGuid>& InLightGuids);
	
	/** The textures containing the light-map data. */
	ULightMapTexture2D* Textures[NUM_GATHERED_LIGHTMAP_COEF];

	/** A scale to apply to the coefficients. */
	FVector4 ScaleVectors[NUM_GATHERED_LIGHTMAP_COEF];

	/** The scale which is applied to the light-map coordinates before sampling the light-map textures. */
	FVector2D CoordinateScale;

	/** The bias which is applied to the light-map coordinates before sampling the light-map textures. */
	FVector2D CoordinateBias;
};

/**
 * The raw data which is used to construct a 1D light-map.
 */
class FLightMapData1D
{
public:

	/** The lights which this light-map stores. */
	TArray<ULightComponent*> Lights;

	/**
	 * Minimal initialization constructor.
	 */
	FLightMapData1D(INT Size)
	{
		Data.Empty(Size);
		Data.AddZeroed(Size);
	}

	// Accessors.
	const FLightSample& operator()(UINT Index) const { return Data(Index); }
	FLightSample& operator()(UINT Index) { return Data(Index); }
	INT GetSize() const { return Data.Num(); }

private:

	/** The incident light samples for a 1D array of points. */
	TArray<FLightSample> Data;
};

/**
 * The light incident for a point on a surface in three directions, stored as bytes representing values from 0-1.
 *
 * @warning BulkSerialize: FQuantizedDirectionalLightSample is serialized as memory dump
 * See TArray::BulkSerialize for detailed description of implied limitations.
 */
struct FQuantizedDirectionalLightSample
{
	/** The lighting coefficients, colored. */
	FColor	Coefficients[NUM_DIRECTIONAL_LIGHTMAP_COEF];
};

/**
* The light incident for a point on a surface along the surface normal, stored as bytes representing values from 0-1.
*
* @warning BulkSerialize: FQuantizedSimpleLightSample is serialized as memory dump
* See TArray::BulkSerialize for detailed description of implied limitations.
*/
struct FQuantizedSimpleLightSample
{
	/** The lighting coefficients, colored. */
	FColor	Coefficients[NUM_SIMPLE_LIGHTMAP_COEF];
};

/**
 * Bulk data array of FQuantizedLightSamples
 */
template<class QuantizedLightSampleType>
struct TQuantizedLightSampleBulkData : public FUntypedBulkData
{
	typedef QuantizedLightSampleType SampleType;
	/**
	 * Returns whether single element serialization is required given an archive. This e.g.
	 * can be the case if the serialization for an element changes and the single element
	 * serialization code handles backward compatibility.
	 */
	virtual UBOOL RequiresSingleElementSerialization( FArchive& Ar );

	/**
	 * Returns size in bytes of single element.
	 *
	 * @return Size in bytes of single element
	 */
	virtual INT GetElementSize() const;

	/**
	 * Serializes an element at a time allowing and dealing with endian conversion and backward compatiblity.
	 * 
	 * @param Ar			Archive to serialize with
	 * @param Data			Base pointer to data
	 * @param ElementIndex	Element index to serialize
	 */
	virtual void SerializeElement( FArchive& Ar, void* Data, INT ElementIndex );
};

/** A 1D array of incident lighting data. */
class FLightMap1D : public FLightMap, public FVertexBuffer
{
public:

	FLightMap1D(UBOOL InbAllowDirectionalLightMaps=GSystemSettings.bAllowDirectionalLightMaps)
		:	FLightMap(InbAllowDirectionalLightMaps) 
		,	Owner(NULL)
		,	CachedSampleDataSize(0)
		,	CachedSampleData(NULL) 
	{}

	/**
	 * Uses the raw light-map data to construct a vertex buffer.
	 * @param	Owner - The object which owns the light-map.
	 * @param	Data - The raw light-map data.
	 */
	FLightMap1D(UObject* InOwner,FLightMapData1D& Data);

	/** Destructor. */
	virtual ~FLightMap1D();

	// FLightMap interface.
	virtual void Serialize(FArchive& Ar);
	virtual void InitResources();
	virtual void Cleanup();
	virtual FLightMapInteraction GetInteraction() const
	{
		return FLightMapInteraction::Vertex(this,ScaleVectors,bAllowDirectionalLightMaps);
	}

	// Runtime type casting.
	virtual const FLightMap1D* GetLightMap1D() const { return this; }

	// Accessors.
	INT NumSamples() const 
	{ 
		return bAllowDirectionalLightMaps ? DirectionalSamples.GetElementCount() : SimpleSamples.GetElementCount(); 
	}

	/**
	 * Creates a new lightmap that is a copy of this light map, but where the sample data
	 * has been remapped according to the specified sample index remapping.
	 *
	 * @param		SampleRemapping		Sample remapping: Dst[i] = Src[RemappedIndices[i]].
	 * @return							The new lightmap.
	 */
	FLightMap1D* DuplicateWithRemappedVerts(const TArray<INT>& SampleRemapping);

private:

	/**
	 * Scales, gamma corrects and quantizes the passed in samples and then stores them in BulkData.
	 *
	 * @param BulkData - The bulk data where the quantized samples are stored.
	 * @param Data - The samples to process.
	 * @param InvScale - 1 / max sample value.
	 * @param NumCoefficients - Number of coefficients in the BulkData samples.
	 * @param RelativeCoefficientOffset - Coefficient offset in the Data samples.
	 */
	template<class BulkDataType>
	void QuantizeBulkSamples(
		BulkDataType& BulkData, 
		FLightMapData1D& Data, 
		const FLOAT InvScale[][3], 
		UINT NumCoefficients, 
		UINT RelativeCoefficientOffset);

	/**
	 * Copies samples using the given remapping.
	 *
	 * @param SourceBulkData - The source samples
	 * @param DestBulkData - The destination samples
	 * @param SampleRemapping - Dst[i] = Src[SampleRemapping[i]].
	 * @param NumCoefficients - Number of coefficients in the sample type.
	 */
	template<class BulkDataType>
	void CopyBulkSamples(
		BulkDataType& SourceBulkData, 
		BulkDataType& DestBulkData, 
		const TArray<INT>& SampleRemapping,
		UINT NumCoefficients);

	/** The object which owns the light-map. */
	UObject* Owner;

	/** The incident light samples for a 1D array of points from three directions. */
	TQuantizedLightSampleBulkData<FQuantizedDirectionalLightSample> DirectionalSamples;

	/** The incident light samples for a 1D array of points along the surface normal. */
	TQuantizedLightSampleBulkData<FQuantizedSimpleLightSample> SimpleSamples;

	/** Size of CachedSampleData */
	UINT CachedSampleDataSize;

	/** 
	 * Cached copy of bulk data that is freed by rendering thread and valid between BeginInitResource
	 * and InitRHI.
	 */
	void* CachedSampleData;

	/** A scale to apply to the coefficients. */
	FVector4 ScaleVectors[NUM_GATHERED_LIGHTMAP_COEF];

	// FRenderResource interface.
	virtual void InitRHI();
};
