/*=============================================================================
	XeD3dRenderTarget.h: XeD3D render target surface RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if USE_XeD3D_RHI

enum EXeSurfaceFlags
{
};

/**
* Info about a Xenon EDRAM surface
*/
class FXeSurfaceInfo
{
public:
	/**
	* Init constructor
	* @param Offset - offset in EDRAM memory tiles
	* @param Size - size in EDRAM memory tiles
	* @param ColorExpBias - color exponent bias to apply during resolve
	* @param Flags - combination of ESurfaceEDRAMFlags
	*/
	FXeSurfaceInfo(
		DWORD InOffset=0, 
		DWORD InSize=0, 
		INT InColorExpBias=0, 
		DWORD InFlags=0
		)
		: Offset(InOffset)
		, Size(InSize)
		, ColorExpBias(InColorExpBias)
		, Flags(InFlags)
	{
	}

	UBOOL IsOverlapping(const FXeSurfaceInfo& Other) const
	{
        return( (Other.Offset >= Offset && Other.Offset < (Offset+Size) && Other.Size > 0) ||
				(Offset >= Other.Offset && Offset < (Other.Offset+Other.Size) && Size > 0) );
	}

	FORCEINLINE INT GetColorExpBias() const
	{
		return ColorExpBias;
	}

	DWORD GetSize() const
	{
		return Size;
	}

private:
	/** offset in EDRAM memory tiles */
	DWORD Offset;
	/** size in EDRAM memory tiles */
	DWORD Size;
	/** color exponent bias to apply during resolve */
	INT ColorExpBias;
	/** combination of EXeSurfaceFlags */
	DWORD Flags;
};

class FSurfaceRHIRef : public TRefCountPtr<IDirect3DSurface9>
{
public:
	/** 2d texture to resolve surface to */
	FTexture2DRHIRef ResolveTargetTexture2D;
	/** Cube texture to resolve surface to */
	FTextureCubeRHIRef ResolveTargetTextureCube;
	/** info for this EDRAM surface */
	FXeSurfaceInfo XeSurfaceInfo;

	/** 
	* default constructor 
	*/
	FSurfaceRHIRef(IDirect3DSurface9* InSurface = NULL)
		:	TRefCountPtr<IDirect3DSurface9>(InSurface)
		,	ResolveTargetTexture2D(NULL)
		,	ResolveTargetTextureCube(NULL)
	{
	}

	/** 
	* Init constructor. Using 2d texture 
	*/
	FSurfaceRHIRef( 
		FTexture2DRHIRef InResolveTargetTexture,
		IDirect3DSurface9* InSurface = NULL,
		const FXeSurfaceInfo& InXeSurfaceInfo = FXeSurfaceInfo()
		)
		:	TRefCountPtr<IDirect3DSurface9>(InSurface)
		,	ResolveTargetTexture2D(InResolveTargetTexture)
		,	ResolveTargetTextureCube(NULL)
		,	XeSurfaceInfo(InXeSurfaceInfo)
	{
	}

	/** 
	* Init constructor. Using cube texture 
	*/
	FSurfaceRHIRef( 
		FTextureCubeRHIRef InResolveTargetTexture,
		IDirect3DSurface9* InSurface = NULL,
		const FXeSurfaceInfo& InXeSurfaceInfo = FXeSurfaceInfo()
		)
		:	TRefCountPtr<IDirect3DSurface9>(InSurface)
		,	ResolveTargetTexture2D(NULL)
		,	ResolveTargetTextureCube(InResolveTargetTexture)
		,	XeSurfaceInfo(InXeSurfaceInfo)
	{
	}

	/** 
	* Resets the reference. 
	*/
	void SafeRelease()
	{
		TRefCountPtr<IDirect3DSurface9>::SafeRelease();
		ResolveTargetTexture2D.SafeRelease();
		ResolveTargetTextureCube.SafeRelease();
	}
};

typedef const FSurfaceRHIRef& FSurfaceRHIParamRef;

#endif
