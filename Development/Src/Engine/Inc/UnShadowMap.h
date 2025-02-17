/*=============================================================================
	UnShadowMap.h: Shadow-map definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * A sample of the visibility factor between a light and a single point.
 */
struct FShadowSample
{
	/** The fraction of light that reaches this point from the light, between 0 and 1. */
	FLOAT Visibility;

	/** True if this sample maps to a valid point on a surface. */
	UBOOL IsMapped;
};

/**
 * The raw data which is used to construct a 2D light-map.
 */
class FShadowMapData2D
{
public:

	/**
	 * Minimal initialization constructor.
	 */
	FShadowMapData2D(UINT InSizeX,UINT InSizeY):
		SizeX(InSizeX),
		SizeY(InSizeY)
	{
		Data.Empty(SizeX * SizeY);
		Data.AddZeroed(SizeX * SizeY);
	}

	// Accessors.
	const FShadowSample& operator()(UINT X,UINT Y) const { return Data(SizeX * Y + X); }
	FShadowSample& operator()(UINT X,UINT Y) { return Data(SizeX * Y + X); }
	UINT GetSizeX() const { return SizeX; }
	UINT GetSizeY() const { return SizeY; }

	// USurface interface
	virtual FLOAT GetSurfaceWidth() const { return SizeX; }
	virtual FLOAT GetSurfaceHeight() const { return SizeY; }

private:

	/** The incident light samples for a 2D array of points on the surface. */
	TArray<FShadowSample> Data;

	/** The width of the shadow-map. */
	UINT SizeX;

	/** The height of the shadow-map. */
	UINT SizeY;
};

/**
 * A 2D texture used to store shadow-map data.
 */
class UShadowMapTexture2D : public UTexture2D
{
	DECLARE_CLASS(UShadowMapTexture2D,UTexture2D,CLASS_NoExport,Engine)

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
 * A 2D array of shadow-map data.
 */
class UShadowMap2D : public UObject
{
	DECLARE_CLASS(UShadowMap2D,UObject,CLASS_NoExport,Engine);
public:

	// Accessors.
	UShadowMapTexture2D* GetTexture() const { check(IsValid()); return Texture; }
	const FVector2D& GetCoordinateScale() const { check(IsValid()); return CoordinateScale; }
	const FVector2D& GetCoordinateBias() const { check(IsValid()); return CoordinateBias; }
	const FGuid& GetLightGuid() const { return LightGuid; }
	UBOOL IsValid() const { return Texture != NULL; }

	/**
	 * Samples the shadow-map.
	 * @param	Coordinate - The texture coordinates used to sample the shadow-map.
	 * @return	The filtered shadow factor at the given coordinate in the shadow-map.
	 */
	FLOAT SampleTexture(const FVector2D& Coordinate) const;

	/**
	 * Allocates texture space for the shadow-map and stores the shadow-map's raw data for deferred encoding.
	 * @param	RawData - The raw shadow-map data to fill the texture with.
	 * @param	InLightGuid - The GUID of the light the shadow-map is for.
	 * @param	Material - The material which the shadow-map will be rendered with.  Used as a hint to pack shadow-maps for the same material in the same texture.  Can be NULL.
	 * @param	Bounds - The bounds of the primitive the shadow-map will be rendered on.  Used as a hint to pack shadow-maps on nearby primitives in the same texture.
	 */
	UShadowMap2D(const FShadowMapData2D& RawData,const FGuid& InLightGuid,UMaterialInterface* Material,const FBoxSphereBounds& Bounds);

	/**
	 * Minimal initialization constructor.
	 */
	UShadowMap2D() {}

	/**
	 * Executes all pending shadow-map encoding requests.
	 */
	static void FinishEncoding();

private:

	/** The texture which contains the shadow-map data. */
	UShadowMapTexture2D* Texture;

	/** The scale which is applied to the shadow-map coordinates before sampling the shadow-map textures. */
	FVector2D CoordinateScale;

	/** The bias which is applied to the shadow-map coordinates before sampling the shadow-map textures. */
	FVector2D CoordinateBias;

	/** The GUID of the light which this shadow-map is for. */
	FGuid LightGuid;
};

/**
 * The raw data which is used to construct a 1D shadow-map.
 */
class FShadowMapData1D
{
public:

	/**
	 * Minimal initialization constructor.
	 */
	FShadowMapData1D(INT Size)
	{
		Data.Empty(Size);
		Data.AddZeroed(Size);
	}

	// Accessors.
	FLOAT operator()(UINT Index) const { return Data(Index); }
	FLOAT& operator()(UINT Index) { return Data(Index); }
	INT GetSize() const { return Data.Num(); }

private:

	/** The occlusion samples for a 1D array of points. */
	TArray<FLOAT> Data;
};

/**
 * A 1D array of shadow occlusion data.
 */
class UShadowMap1D : public UObject, public FVertexBuffer
{
	DECLARE_CLASS(UShadowMap1D,UObject,CLASS_Intrinsic,Engine)
public:

	UShadowMap1D()
		:	Samples(FALSE) // samples are not CPU accessible
	{}

	/**
	 * Uses the raw light-map data to construct a vertex buffer.
	 * @param	Data - The raw light-map data.
	 */
	UShadowMap1D(const FGuid& InLightGuid,const FShadowMapData1D& Data);

	// UObject interface.
	virtual void PostLoad();
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();

	// Accessors.
	INT NumSamples() const { return Samples.Num(); }
	const FGuid& GetLightGuid() const { return LightGuid; }

	// UObject interface.
	virtual void Serialize(FArchive& Ar);

private:
	/** The incident light samples for a 1D array of points. */
	TResourceArray<FLOAT,VERTEXBUFFER_ALIGNMENT> Samples;

	/** The GUID of the light which this shadow-map is for. */
	FGuid LightGuid;

	/** A fence used to track when the light-map resource has been released. */
	FRenderCommandFence ReleaseFence;

	// FRenderResource interface.
	virtual void InitRHI();
};
