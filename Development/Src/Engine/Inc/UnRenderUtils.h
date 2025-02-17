/*=============================================================================
	UnRenderUtils.h: Rendering utility classes.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

//
//	FPackedNormal
//

struct FPackedNormal
{
	union
	{
		struct
		{
#if __INTEL_BYTE_ORDER__ || PS3
			BYTE	X,
					Y,
					Z,
					W;
#else
			BYTE	W,
					Z,
					Y,
					X;
#endif
		};
		DWORD		Packed;
	}				Vector;

	// Constructors.

	FPackedNormal( ) { Vector.Packed = 0; }
	FPackedNormal( DWORD InPacked ) { Vector.Packed = InPacked; }
	FPackedNormal( const FVector& InVector ) { *this = InVector; }

	// Conversion operators.

	void operator=(const FVector& InVector);
	operator FVector() const;
	operator VectorRegister() const;

	// Set functions.
	void Set( const FVector& InVector ) { *this = InVector; }

	// Equality operator.

	UBOOL operator==(const FPackedNormal& B) const;

	// Serializer.

	friend FArchive& operator<<(FArchive& Ar,FPackedNormal& N);
};

extern const VectorRegister GVectorHalf255_W0;

/**
* FPackedNormal operator =
*/
FORCEINLINE void FPackedNormal::operator=(const FVector& InVector)
{
#if PS3
	// Rescale [-1..1] range to [0..255]
	VectorRegister VectorToPack		= VectorLoadFloat3( &InVector );
	VectorToPack					= VectorMultiplyAdd( VectorToPack, GVectorHalf255_W0, GVectorHalf255_W0 );
	// Write out as bytes, clamped to [0..255]
	VectorStoreByte4( VectorToPack, this );
	VectorResetFloatRegisters();
#elif XBOX
	// Rescale [-1..1] range to [0..255]
	VectorRegister VectorToPack		= VectorLoadFloat3( &InVector );
	VectorToPack					= VectorMultiplyAdd( VectorToPack, GVectorHalf255_W0, GVectorHalf255_W0 );
	VectorToPack					= VectorSwizzle( VectorToPack, 3, 2, 1, 0 );
	// Write out as bytes, clamped to [0..255]
	VectorStoreByte4( VectorToPack, this );
	VectorResetFloatRegisters();
#else
	Vector.X = Clamp(appTrunc(InVector.X * 127.5f + 127.5f),0,255);
	Vector.Y = Clamp(appTrunc(InVector.Y * 127.5f + 127.5f),0,255);
	Vector.Z = Clamp(appTrunc(InVector.Z * 127.5f + 127.5f),0,255);
	Vector.W = 127.5f;
#endif
}

/**
 * FPackedNormal operator ==
 */
FORCEINLINE UBOOL FPackedNormal::operator==(const FPackedNormal& B) const
{
	if(Vector.Packed != B.Vector.Packed)
		return 0;

	FVector	V1 = *this,
			V2 = B;

	if(Abs(V1.X - V2.X) > THRESH_NORMALS_ARE_SAME * 4.0f)
		return 0;

	if(Abs(V1.Y - V2.Y) > THRESH_NORMALS_ARE_SAME * 4.0f)
		return 0;

	if(Abs(V1.Z - V2.Z) > THRESH_NORMALS_ARE_SAME * 4.0f)
		return 0;

	return 1;
}

/**
* Constructs a basis matrix for the axis vectors and returns the sign of the determinant
*
* @param XAxis - x axis (tangent)
* @param YAxis - y axis (binormal)
* @param ZAxis - z axis (normal)
* @return sign of determinant either -1 or +1 
*/
FORCEINLINE FLOAT GetBasisDeterminantSign( const FVector& XAxis, const FVector& YAxis, const FVector& ZAxis )
{
	FMatrix Basis(
		FPlane(XAxis,0),
		FPlane(YAxis,0),
		FPlane(ZAxis,0),
		FPlane(0,0,0,1)
		);
	return (Basis.Determinant() < 0) ? -1.0f : +1.0f;
}

/**
* Constructs a basis matrix for the axis vectors and returns the sign of the determinant
*
* @param XAxis - x axis (tangent)
* @param YAxis - y axis (binormal)
* @param ZAxis - z axis (normal)
* @return sign of determinant either 0 (-1) or +1 (255)
*/
FORCEINLINE BYTE GetBasisDeterminantSignByte( const FPackedNormal& XAxis, const FPackedNormal& YAxis, const FPackedNormal& ZAxis )
{
	return appTrunc(GetBasisDeterminantSign(XAxis,YAxis,ZAxis) * 127.5f + 127.5f);
}

//
//	FWindPointSource
//

struct FWindPointSource
{
	FVector	SourceLocation;
	FLOAT	Strength,
			Phase,
			Frequency,
			InvRadius,
			InvDuration;

	// GetWind

	FVector GetWind(const FVector& Location) const;
};

//
//	EMouseCursor
//

enum EMouseCursor
{
	MC_None,
	MC_NoChange,		// Keeps the platform client from calling setcursor so a cursor can be set elsewhere (ie using wxSetCursor).
	MC_Arrow,
	MC_Cross,
	MC_SizeAll,
	MC_SizeUpRightDownLeft,
	MC_SizeUpLeftDownRight,
	MC_SizeLeftRight,
	MC_SizeUpDown,
	MC_Hand
};

/** Enumeration of pixel format flags used for GPixelFormats.Flags */
enum EPixelFormatFlags
{
	/** Whether this format supports SRGB read, aka gamma correction on texture sampling or whether we need to do it manually (if this flag is set) */
	PF_REQUIRES_GAMMA_CORRECTION = 0x01,

	/** Whether this is a linear format */
	PF_LINEAR = 0x02,
};

//
//	FPixelFormatInfo
//

struct FPixelFormatInfo
{
	const TCHAR*	Name;
	INT				BlockSizeX,
					BlockSizeY,
					BlockSizeZ,
					BlockBytes,
					NumComponents;
	/** Platform specific token, e.g. D3DFORMAT with D3DDrv										*/
	DWORD			PlatformFormat;
	/** Format specific internal flags, e.g. whether SRGB is supported with this format		*/
	DWORD			Flags;
	/** Whether the texture format is supported on the current platform/ rendering combination	*/
	UBOOL			Supported;
};

extern FPixelFormatInfo GPixelFormats[];		// Maps members of EPixelFormat to a FPixelFormatInfo describing the format.


#define NUM_DEBUG_UTIL_COLORS (32)
static const FColor DebugUtilColor[NUM_DEBUG_UTIL_COLORS] = 
{
	FColor(20,226,64),
	FColor(210,21,0),
	FColor(72,100,224),
	FColor(14,153,0),
	FColor(186,0,186),
	FColor(54,0,175),
	FColor(25,204,0),
	FColor(15,189,147),
	FColor(23,165,0),
	FColor(26,206,120),
	FColor(28,163,176),
	FColor(29,0,188),
	FColor(130,0,50),
	FColor(31,0,163),
	FColor(147,0,190),
	FColor(1,0,109),
	FColor(2,126,203),
	FColor(3,0,58),
	FColor(4,92,218),
	FColor(5,151,0),
	FColor(18,221,0),
	FColor(6,0,131),
	FColor(7,163,176),
	FColor(8,0,151),
	FColor(102,0,216),
	FColor(10,0,171),
	FColor(11,112,0),
	FColor(12,167,172),
	FColor(13,189,0),
	FColor(16,155,0),
	FColor(178,161,0),
	FColor(19,25,126)
};

//
//	CalculateImageBytes
//

extern SIZE_T CalculateImageBytes(DWORD SizeX,DWORD SizeY,DWORD SizeZ,BYTE Format);

/**
 * Handles initialization/release for a global resource.
 */
template<class ResourceType>
class TGlobalResource : public ResourceType
{
public:
	TGlobalResource()
	{
		if(IsInRenderingThread())
		{
			// If the resource is constructed in the rendering thread, directly initialize it.
			((ResourceType*)this)->InitResource();
		}
		else
		{
			// If the resource is constructed outside of the rendering thread, enqueue a command to initialize it.
			BeginInitResource((ResourceType*)this);
		}
	}

	virtual ~TGlobalResource()
	{
		// This should be called in the rendering thread, or at shutdown when the rendering thread has exited.
		// However, it may also be called at shutdown after an error, when the rendering thread is still running.
		// To avoid a second error in that case we don't assert.
		#if 0
			check(IsInRenderingThread());
		#endif

		// Cleanup the resource.
		((ResourceType*)this)->ReleaseResource();
	}
};

/** A global white texture. */
extern class FTexture* GWhiteTexture;

/** A global white cube texture. */
extern class FTexture* GWhiteTextureCube;

//
// Primitive drawing utility functions.
//

// Solid shape drawing utility functions. Not really designed for speed - more for debugging.
// These utilities functions are implemented in UnScene.cpp using GetTRI.

extern void DrawBox(class FPrimitiveDrawInterface* PDI,const FMatrix& BoxToWorld,const FVector& Radii,const FMaterialRenderProxy* MaterialRenderProxy,BYTE DepthPriority);
extern void DrawSphere(class FPrimitiveDrawInterface* PDI,const FVector& Center,const FVector& Radii,INT NumSides,INT NumRings,const FMaterialRenderProxy* MaterialRenderProxy,BYTE DepthPriority);
extern void DrawCone(class FPrimitiveDrawInterface* PDI,const FMatrix& ConeToWorld, FLOAT Angle1, FLOAT Angle2, INT NumSides, UBOOL bDrawSideLines, const FColor& SideLineColor, const FMaterialRenderProxy* MaterialRenderProxy, BYTE DepthPriority);


extern void DrawCylinder(class FPrimitiveDrawInterface* PDI,const FVector& Base, const FVector& XAxis, const FVector& YAxis, const FVector& ZAxis,
	FLOAT Radius, FLOAT HalfHeight, INT Sides, const FMaterialRenderProxy* MaterialInstance, BYTE DepthPriority);
extern void DrawChoppedCone(class FPrimitiveDrawInterface* PDI,const FVector& Base, const FVector& XAxis, const FVector& YAxis, const FVector& ZAxis,
	FLOAT Radius, FLOAT TopRadius, FLOAT HalfHeight, INT Sides, const FMaterialRenderProxy* MaterialInstance, BYTE DepthPriority);


// Line drawing utility functions.

extern void DrawWireBox(class FPrimitiveDrawInterface* PDI,const FBox& Box,FColor Color,BYTE DepthPriority);
extern void DrawCircle(class FPrimitiveDrawInterface* PDI,const FVector& Base,const FVector& X,const FVector& Y,FColor Color,FLOAT Radius,INT NumSides,BYTE DepthPriority);
extern void DrawWireSphere(class FPrimitiveDrawInterface* PDI, const FVector& Base, FColor Color, FLOAT Radius, INT NumSides, BYTE DepthPriority);
extern void DrawWireCylinder(class FPrimitiveDrawInterface* PDI,const FVector& Base,const FVector& X,const FVector& Y,const FVector& Z,FColor Color,FLOAT Radius,FLOAT HalfHeight,INT NumSides,BYTE DepthPriority);
extern void DrawWireChoppedCone(class FPrimitiveDrawInterface* PDI,const FVector& Base,const FVector& X,const FVector& Y,const FVector& Z,FColor Color,FLOAT Radius,FLOAT TopRadius,FLOAT HalfHeight,INT NumSides,BYTE DepthPriority);
extern void DrawOrientedWireBox(FPrimitiveDrawInterface* PDI,const FVector& Base,const FVector& X,const FVector& Y,const FVector& Z, FVector Extent, FColor Color,BYTE DepthPriority);
extern void DrawDirectionalArrow(class FPrimitiveDrawInterface* PDI,const FMatrix& ArrowToWorld,FColor InColor,FLOAT Length,FLOAT ArrowSize,BYTE DepthPriority);
extern void DrawWireStar(class FPrimitiveDrawInterface* PDI,const FVector& Position, FLOAT Size, FColor Color,BYTE DepthPriority);
extern void DrawDashedLine(class FPrimitiveDrawInterface* PDI,const FVector& Start, const FVector& End, FColor Color, FLOAT DashSize,BYTE DepthPriority);
extern void DrawWireDiamond(class FPrimitiveDrawInterface* PDI,const FMatrix& DiamondMatrix, FLOAT Size, const FColor& InColor,BYTE DepthPriority);

/**
 * Draws a wireframe of the bounds of a frustum as defined by a transform from clip-space into world-space.
 * @param PDI - The interface to draw the wireframe.
 * @param FrustumToWorld - A transform from clip-space to world-space that defines the frustum.
 * @param Color - The color to draw the wireframe with.
 * @param DepthPriority - The depth priority group to draw the wireframe with.
 */
extern void DrawFrustumWireframe(
	FPrimitiveDrawInterface* PDI,
	const FMatrix& WorldToFrustum,
	FColor Color,
	BYTE DepthPriority
	);

/** The indices for drawing a cube. */
extern const WORD GCubeIndices[12*3];

/**
 * Maps from an X,Y,Z cube vertex coordinate to the corresponding vertex index.
 */
inline const WORD GetCubeVertexIndex(UBOOL X,UBOOL Y,UBOOL Z) { return X * 4 + Y * 2 + Z; }

/**
 * Given a base color and a selection state, returns a color which accounts for the selection state.
 * @param BaseColor - The base color of the object.
 * @param bSelected - The selection state of the object.
 * @return The color to draw the object with, accounting for the selection state
 */
extern FLinearColor GetSelectionColor(const FLinearColor& BaseColor,UBOOL bSelected);

/**
 * Returns true if the given view is "rich".  Rich means that calling DrawRichMesh for the view will result in a modified draw call
 * being made.
 * A view is rich if is missing the SHOW_Materials showflag, or has any of the render mode affecting showflags.
 */
extern UBOOL IsRichView(const FSceneView* View);

/** 
 *	Returns true if the given view is showing a collision mode. 
 *	This means one of the flags SHOW_CollisionNonZeroExtent, SHOW_CollisionZeroExtent or SHOW_CollisionRigidBody is set.
 */
extern UBOOL IsCollisionView(const FSceneView* View);

/**
 * Draws a mesh, modifying the material which is used depending on the view's show flags.
 * Meshes with materials irrelevant to the pass which the mesh is being drawn for may be entirely ignored.
 *
 * @param PDI - The primitive draw interface to draw the mesh on.
 * @param Mesh - The mesh to draw.
 * @param WireframeColor - The color which is used when rendering the mesh with SHOW_Wireframe.
 * @param LevelColor - The color which is used when rendering the mesh with SHOW_LevelColoration.
 * @param PropertyColor - The color to use when rendering the mesh with SHOW_PropertyColoration.
 * @param PrimitiveInfo - The FScene information about the UPrimitiveComponent.
 * @param bSelected - True if the primitive is selected.
 * @param ExtraDrawFlags - optional flags to override the view family show flags when rendering
 * @return Number of passes rendered for the mesh
 */
extern INT DrawRichMesh(
	FPrimitiveDrawInterface* PDI,
	const struct FMeshElement& Mesh,
	const FLinearColor& WireframeColor,
	const FLinearColor& LevelColor,
	const FLinearColor& PropertyColor,
	class FPrimitiveSceneInfo *PrimitiveInfo,
	UBOOL bSelected,
	DWORD ExtraDrawFlags=0
	);

/**
 *	Timer helper class.
 **/
class FTimer
{
public:
	/**
	 *	Constructor
	 **/
	FTimer()
	:	CurrentDeltaTime(0.0f)
	,	CurrentTime(0.0f)
	{
	}

	/**
	 *	Returns the current time, in seconds.
	 *	@return Current time, in seconds
	 */
	FLOAT	GetCurrentTime() const
	{
		return CurrentTime;
	}

	/**
	 *	Returns the current delta time.
	 *	@return Current delta time (number of seconds that passed between the last two tick)
	 */
	FLOAT	GetCurrentDeltaTime() const
	{
		return CurrentDeltaTime;
	}

	/**
	 *	Updates the timer.
	 *	@param DeltaTime	Number of seconds that have passed since the last tick
	 **/
	void	Tick( FLOAT DeltaTime )
	{
		CurrentDeltaTime = DeltaTime;
		CurrentTime += DeltaTime;
	}

protected:
	/** Current delta time (number of seconds that passed between the last two tick). */
	FLOAT	CurrentDeltaTime;
	/** Current time, in seconds. */
	FLOAT	CurrentTime;
};

/** Global realtime clock for the rendering thread. */
extern FTimer GRenderingRealtimeClock;

/** Whether to pause the global realtime clock for the rendering thread. */
extern UBOOL GPauseRenderingRealtimeClock;

/** Whether to enable mip-level fading or not: +1.0f if enabled, -1.0f if disabled. */
extern FLOAT GEnableMipLevelFading;

/**
 * Functionality for fading in/out texture mip-levels.
 */
struct FMipBiasFade
{
	/** Default constructor that sets all values to default (no mips). */
	FMipBiasFade()
	:	TotalMipCount(0.0f)
	,	MipCountDelta(0.0f)
	,	StartTime(0.0f)
	,	MipCountFadingRate(0.0f)
	,	BiasOffset(0.0f)
	{
	}

	/** Number of mip-levels in the texture. */
	FLOAT	TotalMipCount;

	/** Number of mip-levels to fade (negative if fading out / decreasing the mipcount). */
	FLOAT	MipCountDelta;

	/** Timestamp when the fade was started. */
	FLOAT	StartTime;

	/** Number of seconds to interpolate through all MipCountDelta (inverted). */
	FLOAT	MipCountFadingRate;

	/** Difference between total texture mipcount and the starting mipcount for the fade. */
	FLOAT	BiasOffset;

	/**
	 *	Sets up a new interpolation target for the mip-bias.
	 *	@param ActualMipCount	Number of mip-levels currently in memory
	 *	@param TargetMipCount	Number of mip-levels we're changing to
	 *	@param LastRenderTime	Timestamp when it was last rendered (GCurrentTime time space)
	 */
	void	SetNewMipCount( FLOAT ActualMipCount, FLOAT TargetMipCount, DOUBLE LastRenderTime );

	/**
	 *	Calculates the interpolated mip-bias based on the current time.
	 *	@return				Interpolated mip-bias value
	 */
	FLOAT	CalcMipBias() const
	{
 		FLOAT DeltaTime		= GRenderingRealtimeClock.GetCurrentTime() - StartTime;
 		FLOAT TimeFactor	= Min<FLOAT>(DeltaTime * MipCountFadingRate, 1.0f);
		FLOAT MipBias		= BiasOffset - MipCountDelta*TimeFactor;
		return appFloatSelect(GEnableMipLevelFading, MipBias, 0.0f);
	}

	/**
	 *	Checks whether the mip-bias is still interpolating.
	 *	@param CurrentTime	Current time
	 *	@return				TRUE if the mip-bias is still interpolating
	 */
	UBOOL	IsFading( FLOAT CurrentTime ) const
	{
		FLOAT DeltaTime = GRenderingRealtimeClock.GetCurrentTime() - MipCountFadingRate;
		FLOAT TimeFactor = DeltaTime * MipCountFadingRate;
		return (Abs<FLOAT>(MipCountDelta) > SMALL_NUMBER && TimeFactor < 1.0f);
	}
};

/** Emits draw events for a given FMeshElement and the FPrimitiveSceneInfo corresponding to that mesh element. */
extern void EmitMeshDrawEvents(const class FPrimitiveSceneInfo* PrimitiveSceneInfo, const struct FMeshElement& Mesh);
