/*=============================================================================
	D3D10Viewport.h: D3D viewport RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

class FD3D10Viewport : public FRefCountedObject, public TDynamicRHIResource<RRT_Viewport>
{
public:

	FD3D10Viewport(class FD3D10DynamicRHI* InD3DRHI,HWND InWindowHandle,UINT InSizeX,UINT InSizeY,UBOOL bInIsFullscreen);
	~FD3D10Viewport();

	void Resize(UINT InSizeX,UINT InSizeY,UBOOL bInIsFullscreen);

	/**
	 * If the swap chain has been invalidated by DXGI, resets the swap chain to the expected state; otherwise, does nothing.
	 * Called once/frame by the game thread on all viewports.
	 * @param bIgnoreFocus - Whether the reset should happen regardless of whether the window is focused.
     */
	void ConditionalResetSwapChain(UBOOL bIgnoreFocus);

	/** Presents the swap chain. */
	void Present(UBOOL bLockToVsync);

	// Accessors.
	UINT GetSizeX() const { return SizeX; }
	UINT GetSizeY() const { return SizeY; }
	FD3D10Surface* GetBackBuffer() const { return BackBuffer; }

private:
	FD3D10DynamicRHI* D3DRHI;
	HWND WindowHandle;
	UINT SizeX;
	UINT SizeY;
	UBOOL bIsFullscreen;
	UBOOL bIsValid;
	TRefCountPtr<IDXGISwapChain> SwapChain;
	TRefCountPtr<FD3D10Surface> BackBuffer;
};

