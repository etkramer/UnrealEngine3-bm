/*=============================================================================
	XeD3DViewport.h: XeD3D viewport RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if USE_XeD3D_RHI

/** viewport resource */
class FD3DViewport : public FXeResource
{
public:
	
	FD3DViewport(UINT InSizeX,UINT InSizeY,UBOOL bInIsFullscreen);
	~FD3DViewport();

	void Resize(UINT InSizeX,UINT InSizeY,UBOOL bInIsFullscreen);

	// Accessors.
	UINT GetSizeX() const { return SizeX; }
	UINT GetSizeY() const { return SizeY; }
	UBOOL IsFullscreen() const { return bIsFullscreen; }

private:
	UINT SizeX;
	UINT SizeY;
	UBOOL bIsFullscreen;
};

/*-----------------------------------------------------------------------------
RHI viewport type
-----------------------------------------------------------------------------*/
typedef TRefCountPtr<class FD3DViewport> FViewportRHIRef;
typedef FD3DViewport* FViewportRHIParamRef;

#endif
