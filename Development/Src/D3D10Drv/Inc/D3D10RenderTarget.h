/*=============================================================================
	D3D10RenderTarget.h: D3D render target RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * A D3D surface, and an optional texture handle which can be used to read from the surface.
 */
class FD3D10Surface :
	public FRefCountedObject,
	public TDynamicRHIResource<RRT_Surface>
{
public:

	/** A view of the surface as a render target. */
	TRefCountPtr<ID3D10RenderTargetView> RenderTargetView;
	/** A view of the surface as a depth-stencil target. */
	TRefCountPtr<ID3D10DepthStencilView> DepthStencilView;
	/** 2d texture to resolve surface to */
	TRefCountPtr<ID3D10Texture2D> ResolveTargetResource;
	/** Dedicated texture when not rendering directly to resolve target texture surface */
	TRefCountPtr<ID3D10Texture2D> Resource;

	/** Initialization constructor. Using 2d resolve texture */
	FD3D10Surface(
		ID3D10RenderTargetView* InRenderTargetView,
		ID3D10DepthStencilView* InDepthStencilView = NULL,
		ID3D10Texture2D* InResolveTargetResource = NULL,
		ID3D10Texture2D* InResource = NULL
		)
		:	RenderTargetView(InRenderTargetView)
		,	DepthStencilView(InDepthStencilView)
		,	ResolveTargetResource(InResolveTargetResource)		
		,	Resource(InResource)
	{}
};

