/*=============================================================================
	RHIMETHOD_SPECIFIERSs.h: The RHI method definitions.  The same methods are defined multiple places, so they're simply included from this file where necessary.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// DEFINE_RHIMETHOD is used by the includer to modify the RHI method definitions.
// It's defined by the dynamic RHI to pass parameters from the statically bound RHI method to the dynamically bound RHI method.
// To enable that, the parameter list must be given a second time, as they will be passed to the dynamically bound RHI method.
// The last parameter should be return if the method returns a value, and nothing otherwise.
#ifndef DEFINE_RHIMETHOD
#define DEFINE_RHIMETHOD(Type,Name,ParameterTypesAndNames,ParameterNames,ReturnStatement,NullImplementation) Type Name Parameters
#endif

//
// RHI resource management functions.
//

DEFINE_RHIMETHOD(
	FSamplerStateRHIRef,
	CreateSamplerState,
	(const FSamplerStateInitializerRHI& Initializer),(Initializer),
	return,
	return new TNullRHIResource<RRT_SamplerState>();
	);
DEFINE_RHIMETHOD(
	FRasterizerStateRHIRef,
	CreateRasterizerState,
	(const FRasterizerStateInitializerRHI& Initializer),
	(Initializer),
	return,
	return new TNullRHIResource<RRT_RasterizerState>();
	);
DEFINE_RHIMETHOD(
	FDepthStateRHIRef,
	CreateDepthState,
	(const FDepthStateInitializerRHI& Initializer),
	(Initializer),
	return,
	return new TNullRHIResource<RRT_DepthState>();
	);
DEFINE_RHIMETHOD(
	FStencilStateRHIRef,
	CreateStencilState,
	(const FStencilStateInitializerRHI& Initializer),
	(Initializer),
	return,
	return new TNullRHIResource<RRT_StencilState>();
	);
DEFINE_RHIMETHOD(
	FBlendStateRHIRef,
	CreateBlendState,
	(const FBlendStateInitializerRHI& Initializer),
	(Initializer),
	return,
	return new TNullRHIResource<RRT_BlendState>();
	);

DEFINE_RHIMETHOD(
	FVertexDeclarationRHIRef,
	CreateVertexDeclaration,
	(const FVertexDeclarationElementList& Elements),
	(Elements),
	return,
	return new TNullRHIResource<RRT_VertexDeclaration>();
	);
DEFINE_RHIMETHOD(FPixelShaderRHIRef,CreatePixelShader,(const TArray<BYTE>& Code),(Code),return,return new TNullRHIResource<RRT_PixelShader>(););
DEFINE_RHIMETHOD(FVertexShaderRHIRef,CreateVertexShader,(const TArray<BYTE>& Code),(Code),return,return new TNullRHIResource<RRT_VertexShader>(););

/**
 * Creates a bound shader state instance which encapsulates a decl, vertex shader, and pixel shader
 * @param VertexDeclaration - existing vertex decl
 * @param StreamStrides - optional stream strides
 * @param VertexShader - existing vertex shader
 * @param PixelShader - existing pixel shader
 */
DEFINE_RHIMETHOD(
	FBoundShaderStateRHIRef,
	CreateBoundShaderState,
	(FVertexDeclarationRHIParamRef VertexDeclaration, DWORD *StreamStrides, FVertexShaderRHIParamRef VertexShader, FPixelShaderRHIParamRef PixelShader),
	(VertexDeclaration,StreamStrides,VertexShader,PixelShader),
	return,
	return new TNullRHIResource<RRT_BoundShaderState>();
	);

DEFINE_RHIMETHOD(
	FIndexBufferRHIRef,
	CreateIndexBuffer,
	(UINT Stride,UINT Size,FResourceArrayInterface* ResourceArray,EResourceUsageFlag InUsage),
	(Stride,Size,ResourceArray,InUsage),
	return,
	return new TNullRHIResource<RRT_IndexBuffer>();
	);

DEFINE_RHIMETHOD(
	FIndexBufferRHIRef,
	CreateInstancedIndexBuffer,
	(UINT Stride,UINT Size,EResourceUsageFlag InUsage,UINT& OutNumInstances),
	(Stride,Size,InUsage,OutNumInstances),
	return,
	OutNumInstances = 1; return new TNullRHIResource<RRT_IndexBuffer>();
	);

DEFINE_RHIMETHOD(
	void*,
	LockIndexBuffer,
	(FIndexBufferRHIParamRef IndexBuffer,UINT Offset,UINT Size),
	(IndexBuffer,Offset,Size),
	return,
	return GetStaticBuffer();
	);
DEFINE_RHIMETHOD(void,UnlockIndexBuffer,(FIndexBufferRHIParamRef IndexBuffer),(IndexBuffer),,);

/**
 * @param ResourceArray - An optional pointer to a resource array containing the resource's data.
 */
DEFINE_RHIMETHOD(
	FVertexBufferRHIRef,
	CreateVertexBuffer,
	(UINT Size,FResourceArrayInterface* ResourceArray,EResourceUsageFlag InUsage),
	(Size,ResourceArray,InUsage),
	return,
	return new TNullRHIResource<RRT_VertexBuffer>();
	);

DEFINE_RHIMETHOD(
	void*,
	LockVertexBuffer,
	(FVertexBufferRHIParamRef VertexBuffer,UINT Offset,UINT SizeRHI,UBOOL bReadOnlyInsteadOfWriteOnly),
	(VertexBuffer,Offset,SizeRHI,bReadOnlyInsteadOfWriteOnly),
	return,
	return GetStaticBuffer();
	);
DEFINE_RHIMETHOD(void,UnlockVertexBuffer,(FVertexBufferRHIParamRef VertexBuffer),(VertexBuffer),,);

/**
 * Retrieves texture memory stats.
 *
 * @param	OutAllocatedMemorySize	[out]	Size of allocated memory
 * @param	OutAvailableMemorySize	[out]	Size of available memory
 *
 * @return TRUE if supported, FALSE otherwise
 */
DEFINE_RHIMETHOD(UBOOL,GetTextureMemoryStats,( INT& AllocatedMemorySize, INT& AvailableMemorySize ),(AllocatedMemorySize,AvailableMemorySize),return,return FALSE);

/**
* Creates a 2D RHI texture resource
* @param SizeX - width of the texture to create
* @param SizeY - height of the texture to create
* @param Format - EPixelFormat texture format
* @param NumMips - number of mips to generate or 0 for full mip pyramid
* @param Flags - ETextureCreateFlags creation flags
*/
DEFINE_RHIMETHOD(
	FTexture2DRHIRef,
	CreateTexture2D,
	(UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,DWORD Flags,FResourceBulkDataInterface* BulkData),
	(SizeX,SizeY,Format,NumMips,Flags,BulkData),
	return,
	return new TNullRHIResource<RRT_Texture2D>();
	);

/**
 * Tries to reallocate the texture without relocation. Returns a new valid reference to the resource if successful.
 * Both the old and new reference refer to the same texture (at least the shared mip-levels) and both can be used or released independently.
 *
 * @param	Texture2D		- Texture to reallocate
 * @param	NewMipCount		- New number of mip-levels
 * @return					- New reference to the updated texture, or invalid if the reallocation failed
 */
DEFINE_RHIMETHOD(
	FTexture2DRHIRef,
	ReallocateTexture2D,
	(FTexture2DRHIParamRef Texture2D, INT NewMipCount),
	(Texture2D, NewMipCount),
	return,
	return new TNullRHIResource<RRT_Texture2D>();
	);

/**
* Locks an RHI texture's mip surface for read/write operations on the CPU
* @param Texture - the RHI texture resource to lock
* @param MipIndex - mip level index for the surface to retrieve
* @param bIsDataBeingWrittenTo - used to affect the lock flags 
* @param DestStride - output to retrieve the textures row stride (pitch)
* @param bLockWithinMiptail - for platforms that support packed miptails allow locking of individual mip levels within the miptail
* @return pointer to the CPU accessible resource data
*/
DEFINE_RHIMETHOD(
	void*,
	LockTexture2D,
	(FTexture2DRHIParamRef Texture,UINT MipIndex,UBOOL bIsDataBeingWrittenTo,UINT& DestStride,UBOOL bLockWithinMiptail),
	(Texture,MipIndex,bIsDataBeingWrittenTo,DestStride,bLockWithinMiptail),
	return,
	DestStride = 0; return GetStaticBuffer()
	);

/**
* Unlocks a previously locked RHI texture resource
* @param Texture - the RHI texture resource to unlock
* @param MipIndex - mip level index for the surface to unlock
* @param bLockWithinMiptail - for platforms that support packed miptails allow locking of individual mip levels within the miptail
*/
DEFINE_RHIMETHOD(
	void,
	UnlockTexture2D,
	(FTexture2DRHIParamRef Texture,UINT MipIndex,UBOOL bLockWithinMiptail),
	(Texture,MipIndex,bLockWithinMiptail),
	,
	);

/**
* For platforms that support packed miptails return the first mip level which is packed in the mip tail
* @return mip level for mip tail or -1 if mip tails are not used
*/
DEFINE_RHIMETHOD(INT,GetMipTailIdx,(FTexture2DRHIParamRef Texture),(Texture),return,return INDEX_NONE);

/**
 * Copies a region within the same mip levels of one texture to another.  An optional region can be speci
 * Note that the textures must be the same size and of the same format.
 * @param DstTexture - destination texture resource to copy to
 * @param MipIdx - mip level for the surface to copy from/to. This mip level should be valid for both source/destination textures
 * @param BaseSizeX - width of the texture (base level). Same for both source/destination textures
 * @param BaseSizeY - height of the texture (base level). Same for both source/destination textures 
 * @param Format - format of the texture. Same for both source/destination textures
 * @param Region - list of regions to specify rects and source textures for the copy
 */
DEFINE_RHIMETHOD(
	void,
	CopyTexture2D,
	(FTexture2DRHIParamRef DstTexture, UINT MipIdx, INT BaseSizeX, INT BaseSizeY, INT Format, const TArray<struct FCopyTextureRegion2D>& Regions),
	(DstTexture,MipIdx,BaseSizeX,BaseSizeY,Format,Regions),
	,
	);

/**
 * Copies texture data freom one mip to another
 * Note that the mips must be the same size and of the same format.
 * @param SrcTexture Source texture to copy from
 * @param SrcMipIndex Mip index into the source texture to copy data from
 * @param DestTexture Destination texture to copy to
 * @param DestMipIndex Mip index in the destination texture to copy to - note this is probably different from source mip index if the base widths/heights are different
 * @param Size Size of mip data
 * @param Counter Thread safe counter used to flag end of transfer
 */
DEFINE_RHIMETHOD(
	void,
	CopyMipToMipAsync,
	(FTexture2DRHIParamRef SrcTexture, INT SrcMipIndex, FTexture2DRHIParamRef DestTexture, INT DestMipIndex, INT Size, FThreadSafeCounter& Counter),
	(SrcTexture,SrcMipIndex,DestTexture,DestMipIndex,Size,Counter),
	,
	);

/**
 * Copies mip data from one location to another, selectively copying only used memory based on
 * the texture tiling memory layout of the given mip level
 * Note that the mips must be the same size and of the same format.
 * @param Texture - texture to base memory layout on
 * @param Src - source memory base address to copy from
 * @param Dst - destination memory base address to copy to
 * @param MemSize - total size of mip memory
 * @param MipIdx - mip index to base memory layout on
 */
DEFINE_RHIMETHOD(
	void,
	SelectiveCopyMipData,
	(FTexture2DRHIParamRef Texture, BYTE *Src, BYTE *Dst, UINT MemSize, UINT MipIdx),
	(Texture,Src,Dst,MemSize,MipIdx),
	return,
	appMemcpy(Dst, Src, MemSize);
	);

/**
 * Finalizes an asynchronous mip-to-mip copy.
 * This must be called once asynchronous copy has signaled completion by decrementing the counter passed to CopyMipToMipAsync.
 * @param SrcText Source texture to copy from
 * @param SrcMipIndex Mip index into the source texture to copy data from
 * @param DestText Destination texture to copy to
 * @param DestMipIndex Mip index in the destination texture to copy to - note this is probably different from source mip index if the base widths/heights are different
 */
DEFINE_RHIMETHOD(
	void,
	FinalizeAsyncMipCopy,
	(FTexture2DRHIParamRef SrcTexture, INT SrcMipIndex, FTexture2DRHIParamRef DestTexture, INT DestMipIndex),
	(SrcTexture,SrcMipIndex,DestTexture,DestMipIndex),
	,
	);

/**
* Create resource memory to be shared by multiple RHI resources
* @param Size - aligned size of allocation
* @return shared memory resource RHI ref
*/
DEFINE_RHIMETHOD(FSharedMemoryResourceRHIRef,CreateSharedMemory,(SIZE_T Size),(Size),return,return new TNullRHIResource<RRT_SharedMemoryResource>(););

/**
 * Creates a RHI texture and if the platform supports it overlaps it in memory with another texture
 * Note that modifying this texture will modify the memory of the overlapped texture as well
 * @param SizeX - The width of the surface to create.
 * @param SizeY - The height of the surface to create.
 * @param Format - The surface format to create.
 * @param ResolveTargetTexture - The 2d texture to use the memory from if the platform allows
 * @param Flags - Surface creation flags
 * @return The surface that was created.
 */
DEFINE_RHIMETHOD(
	FSharedTexture2DRHIRef,
	CreateSharedTexture2D,
	(UINT SizeX,UINT SizeY,BYTE Format,UINT NumMips,FSharedMemoryResourceRHIParamRef SharedMemory,DWORD Flags),
	(SizeX,SizeY,Format,NumMips,SharedMemory,Flags),
	return,
	return new TNullRHIResource<RRT_SharedTexture2D>();
	);

/**
* Creates a Cube RHI texture resource
* @param Size - width/height of the texture to create
* @param Format - EPixelFormat texture format
* @param NumMips - number of mips to generate or 0 for full mip pyramid
* @param Flags - ETextureCreateFlags creation flags
*/
DEFINE_RHIMETHOD(
	FTextureCubeRHIRef,
	CreateTextureCube,
	(UINT Size,BYTE Format,UINT NumMips,DWORD Flags,FResourceBulkDataInterface* BulkData),
	(Size,Format,NumMips,Flags,BulkData),
	return,
	return new TNullRHIResource<RRT_TextureCube>();
	);

/**
* Locks an RHI texture's mip surface for read/write operations on the CPU
* @param Texture - the RHI texture resource to lock
* @param MipIndex - mip level index for the surface to retrieve
* @param bIsDataBeingWrittenTo - used to affect the lock flags 
* @param DestStride - output to retrieve the textures row stride (pitch)
* @param bLockWithinMiptail - for platforms that support packed miptails allow locking of individual mip levels within the miptail
* @return pointer to the CPU accessible resource data
*/
DEFINE_RHIMETHOD(
	void*,
	LockTextureCubeFace,
	(FTextureCubeRHIParamRef Texture,UINT FaceIndex,UINT MipIndex,UBOOL bIsDataBeingWrittenTo,UINT& DestStride,UBOOL bLockWithinMiptail),
	(Texture,FaceIndex,MipIndex,bIsDataBeingWrittenTo,DestStride,bLockWithinMiptail),
	return,
	DestStride = 0; return GetStaticBuffer();
	);

/**
* Unlocks a previously locked RHI texture resource
* @param Texture - the RHI texture resource to unlock
* @param MipIndex - mip level index for the surface to unlock
* @param bLockWithinMiptail - for platforms that support packed miptails allow locking of individual mip levels within the miptail
*/
DEFINE_RHIMETHOD(
	void,
	UnlockTextureCubeFace,
	(FTextureCubeRHIParamRef Texture,UINT FaceIndex,UINT MipIndex,UBOOL bLockWithinMiptail),
	(Texture,FaceIndex,MipIndex,bLockWithinMiptail),
	,
	);

/**
 * Creates a RHI surface that can be bound as a render target.
 * Note that a surface cannot be created which is both resolvable AND readable.
 * @param SizeX - The width of the surface to create.
 * @param SizeY - The height of the surface to create.
 * @param Format - The surface format to create.
 * @param ResolveTargetTexture - The 2d texture which the surface will be resolved to.  It must have been allocated with bResolveTargetable=TRUE
 * @param Flags - Surface creation flags
 * @param UsageStr - Text describing usage for this surface
 * @return The surface that was created.
 */
DEFINE_RHIMETHOD(
	FSurfaceRHIRef,
	CreateTargetableSurface,
	(UINT SizeX,UINT SizeY,BYTE Format,FTexture2DRHIParamRef ResolveTargetTexture,DWORD Flags,const TCHAR* UsageStr),
	(SizeX,SizeY,Format,ResolveTargetTexture,Flags,UsageStr),
	return,
	return new TNullRHIResource<RRT_Surface>();
	);

/**
* Creates a RHI surface that can be bound as a render target and can resolve w/ a cube texture
* Note that a surface cannot be created which is both resolvable AND readable.
* @param SizeX - The width of the surface to create.
* @param Format - The surface format to create.
* @param ResolveTargetTexture - The cube texture which the surface will be resolved to.  It must have been allocated with bResolveTargetable=TRUE
* @param CubeFace - face from resolve texture to use as surface
* @param Flags - Surface creation flags
* @param UsageStr - Text describing usage for this surface
* @return The surface that was created.
*/
DEFINE_RHIMETHOD(
	FSurfaceRHIRef,
	CreateTargetableCubeSurface,
	(UINT SizeX,BYTE Format,FTextureCubeRHIParamRef ResolveTargetTexture,ECubeFace CubeFace,DWORD Flags,const TCHAR* UsageStr),
	(SizeX,Format,ResolveTargetTexture,CubeFace,Flags,UsageStr),
	return,
	return new TNullRHIResource<RRT_Surface>();
	);

/**
* Copies the contents of the given surface to its resolve target texture.
* @param SourceSurface - surface with a resolve texture to copy to
* @param bKeepOriginalSurface - TRUE if the original surface will still be used after this function so must remain valid
* @param ResolveParams - optional resolve params
*/
DEFINE_RHIMETHOD(
	void,
	CopyToResolveTarget,
	(FSurfaceRHIParamRef SourceSurface, UBOOL bKeepOriginalSurface, const FResolveParams& ResolveParams),
	(SourceSurface,bKeepOriginalSurface,ResolveParams),
	,
	);

/**
 * Copies the contents of the given surface's resolve target texture back to the surface.
 * If the surface isn't currently allocated, the copy may be deferred until the next time it is allocated.
 * @param DestSurface - surface with a resolve texture to copy from
 */
DEFINE_RHIMETHOD(void,CopyFromResolveTarget,(FSurfaceRHIParamRef DestSurface),(DestSurface),,);

/**
* Copies the contents of the given surface's resolve target texture back to the surface without doing
* anything to the pixels (no exponent correction, no gamma correction).
* If the surface isn't currently allocated, the copy may be deferred until the next time it is allocated.
*
* @param DestSurface - surface with a resolve texture to copy from
*/
DEFINE_RHIMETHOD(void,CopyFromResolveTargetFast,(FSurfaceRHIParamRef DestSurface),(DestSurface),,);

/**
* Copies a subset of the contents of the given surface's resolve target texture back to the surface without doing
* anything to the pixels (no exponent correction, no gamma correction).
* If the surface isn't currently allocated, the copy may be deferred until the next time it is allocated.
*
* @param DestSurface - surface with a resolve texture to copy from
*/
DEFINE_RHIMETHOD(void,CopyFromResolveTargetRectFast,(FSurfaceRHIParamRef DestSurface, FLOAT X1,FLOAT Y1,FLOAT X2,FLOAT Y2),(DestSurface,X1,Y1,X2,Y2),,);

/**
 * Reads the contents of a surface to an output buffer.
 */
DEFINE_RHIMETHOD(
	void,
	ReadSurfaceData,
	(FSurfaceRHIParamRef Surface,UINT MinX,UINT MinY,UINT MaxX,UINT MaxY,TArray<BYTE>& OutData,ECubeFace CubeFace),
	(Surface,MinX,MinY,MaxX,MaxY,OutData,CubeFace),
	,
	);

DEFINE_RHIMETHOD(FOcclusionQueryRHIRef,CreateOcclusionQuery,(),(),return,return new TNullRHIResource<RRT_OcclusionQuery>(););
DEFINE_RHIMETHOD(void,ResetOcclusionQuery,(FOcclusionQueryRHIParamRef OcclusionQuery),(OcclusionQuery),,);
DEFINE_RHIMETHOD(
	UBOOL,
	GetOcclusionQueryResult,
	(FOcclusionQueryRHIParamRef OcclusionQuery,DWORD& OutNumPixels,UBOOL bWait),
	(OcclusionQuery,OutNumPixels,bWait),
	return,
	return TRUE
	);

DEFINE_RHIMETHOD(void,BeginDrawingViewport,(FViewportRHIParamRef Viewport),(Viewport),,);
DEFINE_RHIMETHOD(void,EndDrawingViewport,(FViewportRHIParamRef Viewport,UBOOL bPresent,UBOOL bLockToVsync),(Viewport,bPresent,bLockToVsync),,);
DEFINE_RHIMETHOD(FSurfaceRHIRef,GetViewportBackBuffer,(FViewportRHIParamRef Viewport),(Viewport),return,return new TNullRHIResource<RRT_Surface>(););

DEFINE_RHIMETHOD(void,BeginScene,(),(),return,return;);
DEFINE_RHIMETHOD(void,EndScene,(),(),return,return;);

/*
 * Returns the total GPU time taken to render the last frame. Same metric as appCycles().
 */
DEFINE_RHIMETHOD(DWORD,GetGPUFrameCycles,(),(),return,return 0;);

/*
 * Returns an approximation of the available video memory that textures can use, rounded to the nearest MB, in MB.
 */
DEFINE_RHIMETHOD(DWORD,GetAvailableTextureMemory,(),(),return,return 0;);

/**
 * The following RHI functions must be called from the main thread.
 */
DEFINE_RHIMETHOD(
	FViewportRHIRef,
	CreateViewport,
	(void* WindowHandle,UINT SizeX,UINT SizeY,UBOOL bIsFullscreen),
	(WindowHandle,SizeX,SizeY,bIsFullscreen),
	return,
	return new TNullRHIResource<RRT_Viewport>();
	);
DEFINE_RHIMETHOD(void,ResizeViewport,(FViewportRHIParamRef Viewport,UINT SizeX,UINT SizeY,UBOOL bIsFullscreen),(Viewport,SizeX,SizeY,bIsFullscreen),,);
DEFINE_RHIMETHOD(void,Tick,( FLOAT DeltaTime ),(DeltaTime),,);

//
// RHI commands.
//

// Vertex state.
DEFINE_RHIMETHOD(
	void,
	SetStreamSource,
	(UINT StreamIndex,FVertexBufferRHIParamRef VertexBuffer,UINT Stride,UBOOL bUseInstanceIndex,UINT NumVerticesPerInstance,UINT NumInstances),
	(StreamIndex,VertexBuffer,Stride,bUseInstanceIndex,NumVerticesPerInstance,NumInstances),
	,
	);

// Rasterizer state.
DEFINE_RHIMETHOD(void,SetRasterizerState,(FRasterizerStateRHIParamRef NewState),(NewState),,);
DEFINE_RHIMETHOD(void,SetRasterizerStateImmediate,(const FRasterizerStateInitializerRHI& ImmediateState),(ImmediateState),,);
DEFINE_RHIMETHOD(void,SetViewport,(UINT MinX,UINT MinY,FLOAT MinZ,UINT MaxX,UINT MaxY,FLOAT MaxZ),(MinX,MinY,MinZ,MaxX,MaxY,MaxZ),,);
DEFINE_RHIMETHOD(void,SetScissorRect,(UBOOL bEnable,UINT MinX,UINT MinY,UINT MaxX,UINT MaxY),(bEnable,MinX,MinY,MaxX,MaxY),,);
DEFINE_RHIMETHOD(void,SetDepthBoundsTest,(UBOOL bEnable,const FVector4& ClipSpaceNearPos,const FVector4& ClipSpaceFarPos),(bEnable,ClipSpaceNearPos,ClipSpaceFarPos),,);

// Shader state.
/**
 * Set bound shader state. This will set the vertex decl/shader, and pixel shader
 * @param BoundShaderState - state resource
 */
DEFINE_RHIMETHOD(void,SetBoundShaderState,(FBoundShaderStateRHIParamRef BoundShaderState),(BoundShaderState),,);

DEFINE_RHIMETHOD(
	void,
	SetSamplerState,
	(FPixelShaderRHIParamRef PixelShader,UINT TextureIndex,UINT SamplerIndex,FSamplerStateRHIParamRef NewState,FTextureRHIParamRef NewTexture,FLOAT MipBias),
	(PixelShader,TextureIndex,SamplerIndex,NewState,NewTexture,MipBias),
	,
	);
DEFINE_RHIMETHOD(
	void,
	SetVertexShaderParameter,
	(FVertexShaderRHIParamRef VertexShader,UINT BufferIndex,UINT BaseIndex,UINT NumBytes,const void* NewValue),
	(VertexShader,BufferIndex,BaseIndex,NumBytes,NewValue),
	,
	);
DEFINE_RHIMETHOD(
	void,
	SetPixelShaderParameter,
	(FPixelShaderRHIParamRef PixelShader,UINT BufferIndex,UINT BaseIndex,UINT NumBytes,const void* NewValue),
	(PixelShader,BufferIndex,BaseIndex,NumBytes,NewValue),
	,
	);
DEFINE_RHIMETHOD(
	void,
	SetPixelShaderBoolParameter,
	(FPixelShaderRHIParamRef PixelShader,UINT BaseIndex,UBOOL NewValue),
	(PixelShader,BaseIndex,NewValue),
	,
	);
DEFINE_RHIMETHOD(void,SetRenderTargetBias,(FLOAT ColorBias),(ColorBias),,);

/**
 * Set engine vertex shader parameters for the view.
 * @param View					The current view
 * @param ViewProjectionMatrix	Matrix that transforms from world space to projection space for the view
 * @param ViewOrigin			World space position of the view's origin
 */
DEFINE_RHIMETHOD(
	void,
	SetViewParameters,
	(const FSceneView* View,const FMatrix& ViewProjectionMatrix,const FVector4& ViewOrigin),
	(View,ViewProjectionMatrix,ViewOrigin),
	,
	);

/**
 * Set engine pixel shader parameters for the view.
 * Some platforms needs to set this for each pixelshader, whereas others can set it once, globally.
 * @param View								The current view
 * @param PixelShader						The pixel shader to set the parameters for
 * @param SceneDepthCalcParameter			Handle for the scene depth calc parameter (PSR_MinZ_MaxZ_Ratio). May be NULL.
 * @param SceneDepthCalcValue				Handle for the scene depth calc parameter (PSR_MinZ_MaxZ_Ratio). Ignored if SceneDepthCalcParameter is NULL.
 * @param ScreenPositionScaleBiasParameter	Handle for the screen position scale and bias parameter (PSR_ScreenPositionScaleBias). May be NULL.
 * @param ScreenPositionScaleValue			Value for the screen position scale and bias parameter (PSR_ScreenPositionScaleBias). Ignored if ScreenPositionScaleBiasParameter is NULL.
 */
DEFINE_RHIMETHOD(
	void,
	SetViewPixelParameters,
	(const FSceneView* View,FPixelShaderRHIParamRef PixelShader,const class FShaderParameter* SceneDepthCalcParameter,const class FShaderParameter* ScreenPositionScaleBiasParameter),
	(View,PixelShader,SceneDepthCalcParameter,ScreenPositionScaleBiasParameter),
	,
	);

/**
 * Control the GPR (General Purpose Register) allocation 
 * @param NumVertexShaderRegisters - num of GPRs to allocate for the vertex shader (default is 64)
 * @param NumPixelShaderRegisters - num of GPRs to allocate for the pixel shader (default is 64)
 */
DEFINE_RHIMETHOD(
	void,
	SetShaderRegisterAllocation,
	(UINT NumVertexShaderRegisters,UINT NumPixelShaderRegisters),
	(NumVertexShaderRegisters,NumPixelShaderRegisters),
	,
	);

/**
 * Optimizes pixel shaders that are heavily texture fetch bound due to many L2 cache misses.
 * @param PixelShader	The pixel shader to optimize texture fetching for
 */
DEFINE_RHIMETHOD(void,ReduceTextureCachePenalty,(FPixelShaderRHIParamRef PixelShader),(PixelShader),,);

// Output state.
DEFINE_RHIMETHOD(void,SetDepthState,(FDepthStateRHIParamRef NewState),(NewState),,);
DEFINE_RHIMETHOD(void,SetStencilState,(FStencilStateRHIParamRef NewState),(NewState),,);
DEFINE_RHIMETHOD(void,SetBlendState,(FBlendStateRHIParamRef NewState),(NewState),,);
DEFINE_RHIMETHOD(void,SetRenderTarget,(FSurfaceRHIParamRef NewRenderTarget,FSurfaceRHIParamRef NewDepthStencilTarget),(NewRenderTarget,NewDepthStencilTarget),,);
DEFINE_RHIMETHOD(void,SetMRTRenderTarget,(FSurfaceRHIParamRef NewRenderTarget,UINT TargetIndex),(NewRenderTarget,TargetIndex),,);
DEFINE_RHIMETHOD(void,SetColorWriteEnable,(UBOOL bEnable),(bEnable),,);
DEFINE_RHIMETHOD(void,SetColorWriteMask,(UINT ColorWriteMask),(ColorWriteMask),,);

// Hi stencil optimization
DEFINE_RHIMETHOD(void,BeginHiStencilRecord,(UBOOL bCompareFunctionEqual, UINT RefValue),(bCompareFunctionEqual, RefValue),,);
DEFINE_RHIMETHOD(void,BeginHiStencilPlayback,(UBOOL bFlush),(bFlush),,);
DEFINE_RHIMETHOD(void,EndHiStencil,(),(),,);

// Occlusion queries.
DEFINE_RHIMETHOD(void,BeginOcclusionQuery,(FOcclusionQueryRHIParamRef OcclusionQuery),(OcclusionQuery),,);
DEFINE_RHIMETHOD(void,EndOcclusionQuery,(FOcclusionQueryRHIParamRef OcclusionQuery),(OcclusionQuery),,);

// Primitive drawing.
DEFINE_RHIMETHOD(
	void,
	DrawPrimitive,
	(UINT PrimitiveType,UINT BaseVertexIndex,UINT NumPrimitives),
	(PrimitiveType,BaseVertexIndex,NumPrimitives),
	,
	);
DEFINE_RHIMETHOD(
	void,
	DrawIndexedPrimitive,
	(FIndexBufferRHIParamRef IndexBuffer,UINT PrimitiveType,INT BaseVertexIndex,UINT MinIndex,UINT NumVertices,UINT StartIndex,UINT NumPrimitives),
	(IndexBuffer,PrimitiveType,BaseVertexIndex,MinIndex,NumVertices,StartIndex,NumPrimitives),
	,
	);

/**
 * Draws a primitive with pre-vertex-shader culling.
 * The parameters are the same as RHIDrawIndexedPrimitive, plus the primitive's LocalToWorld transform to use for culling.
 */
DEFINE_RHIMETHOD(
	void,
	DrawIndexedPrimitive_PreVertexShaderCulling,
	(FIndexBufferRHIParamRef IndexBuffer,UINT PrimitiveType,INT BaseVertexIndex,UINT MinIndex,UINT NumVertices,UINT StartIndex,UINT NumPrimitives,const FMatrix& LocalToWorld),
	(IndexBuffer,PrimitiveType,BaseVertexIndex,MinIndex,NumVertices,StartIndex,NumPrimitives,LocalToWorld),
	,
	);

// Immediate Primitive drawing
/**
 * Preallocate memory or get a direct command stream pointer to fill up for immediate rendering . This avoids memcpys below in DrawPrimitiveUP
 * @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
 * @param NumPrimitives The number of primitives in the VertexData buffer
 * @param NumVertices The number of vertices to be written
 * @param VertexDataStride Size of each vertex 
 * @param OutVertexData Reference to the allocated vertex memory
 */
DEFINE_RHIMETHOD(
	void,
	BeginDrawPrimitiveUP,
	(UINT PrimitiveType,UINT NumPrimitives,UINT NumVertices,UINT VertexDataStride,void*& OutVertexData),
	(PrimitiveType,NumPrimitives,NumVertices,VertexDataStride,OutVertexData),
	,
	OutVertexData = GetStaticBuffer();
	);

/**
 * Draw a primitive using the vertex data populated since RHIBeginDrawPrimitiveUP and clean up any memory as needed
 */
DEFINE_RHIMETHOD(void,EndDrawPrimitiveUP,(),(),,);

/**
 * Draw a primitive using the vertices passed in
 * VertexData is NOT created by BeginDrawPrimitveUP
 * @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
 * @param NumPrimitives The number of primitives in the VertexData buffer
 * @param VertexData A reference to memory preallocate in RHIBeginDrawPrimitiveUP
 * @param VertexDataStride Size of each vertex
 */
DEFINE_RHIMETHOD(
	void,
	DrawPrimitiveUP,
	(UINT PrimitiveType, UINT NumPrimitives, const void* VertexData,UINT VertexDataStride),
	(PrimitiveType,NumPrimitives,VertexData,VertexDataStride),
	,
	);

/**
 * Preallocate memory or get a direct command stream pointer to fill up for immediate rendering . This avoids memcpys below in DrawIndexedPrimitiveUP
 * @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
 * @param NumPrimitives The number of primitives in the VertexData buffer
 * @param NumVertices The number of vertices to be written
 * @param VertexDataStride Size of each vertex
 * @param OutVertexData Reference to the allocated vertex memory
 * @param MinVertexIndex The lowest vertex index used by the index buffer
 * @param NumIndices Number of indices to be written
 * @param IndexDataStride Size of each index (either 2 or 4 bytes)
 * @param OutIndexData Reference to the allocated index memory
 */
DEFINE_RHIMETHOD(
	void,
	BeginDrawIndexedPrimitiveUP,
	(UINT PrimitiveType,UINT NumPrimitives,UINT NumVertices,UINT VertexDataStride,void*& OutVertexData,UINT MinVertexIndex,UINT NumIndices,UINT IndexDataStride,void*& OutIndexData),
	(PrimitiveType,NumPrimitives,NumVertices,VertexDataStride,OutVertexData,MinVertexIndex,NumIndices,IndexDataStride,OutIndexData),
	,
	OutVertexData = GetStaticBuffer();
	OutIndexData = GetStaticBuffer();
	);

/**
 * Draw a primitive using the vertex and index data populated since RHIBeginDrawIndexedPrimitiveUP and clean up any memory as needed
 */
DEFINE_RHIMETHOD(void,EndDrawIndexedPrimitiveUP,(),(),,);

/**
 * Draw a primitive using the vertices passed in as described the passed in indices. 
 * IndexData and VertexData are NOT created by BeginDrawIndexedPrimitveUP
 * @param PrimitiveType The type (triangles, lineloop, etc) of primitive to draw
 * @param MinVertexIndex The lowest vertex index used by the index buffer
 * @param NumVertices The number of vertices in the vertex buffer
 * @param NumPrimitives THe number of primitives described by the index buffer
 * @param IndexData The memory preallocated in RHIBeginDrawIndexedPrimitiveUP
 * @param IndexDataStride The size of one index
 * @param VertexData The memory preallocate in RHIBeginDrawIndexedPrimitiveUP
 * @param VertexDataStride The size of one vertex
 */
DEFINE_RHIMETHOD(
	void,
	DrawIndexedPrimitiveUP,
	(UINT PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT NumPrimitives,const void* IndexData,UINT IndexDataStride,const void* VertexData,UINT VertexDataStride),
	(PrimitiveType,MinVertexIndex,NumVertices,NumPrimitives,IndexData,IndexDataStride,VertexData,VertexDataStride),
	,
	);

/**
 * Draw a sprite particle emitter.
 *
 * @param Mesh The mesh element containing the data for rendering the sprite particles
 */
DEFINE_RHIMETHOD(void,DrawSpriteParticles,(const FMeshElement& Mesh),(Mesh),,);

/**
 * Draw a sprite subuv particle emitter.
 *
 * @param Mesh The mesh element containing the data for rendering the sprite subuv particles
 */
DEFINE_RHIMETHOD(void,DrawSubUVParticles,(const FMeshElement& Mesh),(Mesh),,);

// Raster operations.
DEFINE_RHIMETHOD(
	void,
	Clear,
	(UBOOL bClearColor,const FLinearColor& Color,UBOOL bClearDepth,FLOAT Depth,UBOOL bClearStencil,DWORD Stencil),
	(bClearColor,Color,bClearDepth,Depth,bClearStencil,Stencil),
	,
	);

// Kick the rendering commands that are currently queued up in the GPU command buffer.
DEFINE_RHIMETHOD(void,KickCommandBuffer,(),(),,);

// Blocks the CPU until the GPU catches up and goes idle.
DEFINE_RHIMETHOD(void,BlockUntilGPUIdle,(),(),,);

// Operations to suspend title rendering and yield control to the system
DEFINE_RHIMETHOD(void,SuspendRendering,(),(),,);
DEFINE_RHIMETHOD(void,ResumeRendering,(),(),,);
DEFINE_RHIMETHOD(UBOOL,IsRenderingSuspended,(),(),return,return FALSE;);

// MSAA-specific functions
DEFINE_RHIMETHOD(void,MSAAInitPrepass,(),(),,);
DEFINE_RHIMETHOD(void,MSAAFixViewport,(),(),,);
DEFINE_RHIMETHOD(
	void,
	MSAABeginRendering,
	(UBOOL bRequiresClear),
	(bRequiresClear),
	,
	);
DEFINE_RHIMETHOD(
	void,
	MSAAEndRendering,
	(FTexture2DRHIParamRef DepthTexture, FTexture2DRHIParamRef ColorTexture, UINT ViewIndex),
	(DepthTexture,ColorTexture, ViewIndex),
	,
	);
DEFINE_RHIMETHOD(void,RestoreColorDepth,(FTexture2DRHIParamRef ColorTexture, FTexture2DRHIParamRef DepthTexture),(ColorTexture,DepthTexture),,);
DEFINE_RHIMETHOD(void,SetTessellationMode,(ETessellationMode TessellationMode, FLOAT MinTessellation, FLOAT MaxTessellation),(TessellationMode,MinTessellation,MaxTessellation),,);

/**
 *	Retrieve available screen resolutions.
 *
 *	@param	Resolutions			TArray<FScreenResolutionRHI> parameter that will be filled in.
 *	@param	bIgnoreRefreshRate	If TRUE, ignore refresh rates.
 *
 *	@return	UBOOL				TRUE if successfully filled the array
 */
DEFINE_RHIMETHOD(
	UBOOL,
	GetAvailableResolutions,
	(FScreenResolutionArray& Resolutions, UBOOL bIgnoreRefreshRate),
	(Resolutions,bIgnoreRefreshRate),
	return,
	return FALSE;
	);

/**
 * Returns a supported screen resolution that most closely matches input.
 * @param Width - Input: Desired resolution width in pixels. Output: A width that the platform supports.
 * @param Height - Input: Desired resolution height in pixels. Output: A height that the platform supports.
 */
DEFINE_RHIMETHOD(void,GetSupportedResolution,(UINT& Width,UINT& Height),(Width,Height),,);

/**
* Used by non-tiling MSAA split screen; returns Screen Height / 2, and a valid offset to keep Resolve happy
* @param HalfScreenY - returns a point halfway down the screen, e.g. 360 when in 1280x720 mode
* @param ResolveOffset - returns the correct to render to in order to be able to resolve to the bottom half of the screen
*/
DEFINE_RHIMETHOD(
	void,
	GetMSAAOffsets,
	(UINT *HalfScreenY, UINT *ResolveOffset),
	(HalfScreenY,ResolveOffset),
	,
	);


/**
 *	Sets the maximum viewport size the application is expecting to need for the time being, or zero for
 *	no preference.  This is used as a hint to the RHI to reduce redundant device resets when viewports
 *	are created or destroyed (typically in the editor.)
 *
 *	@param NewLargestExpectedViewportWidth Maximum width of all viewports (or zero if not known)
 *	@param NewLargestExpectedViewportHeight Maximum height of all viewports (or zero if not known)
 */
DEFINE_RHIMETHOD(
	void,
	SetLargestExpectedViewportSize,
	( UINT NewLargestExpectedViewportWidth, UINT NewLargestExpectedViewportHeight ),
	( NewLargestExpectedViewportWidth, NewLargestExpectedViewportHeight ),
	,
	);

/**
 * Checks if a texture is still in use by the GPU.
 * @param Texture - the RHI texture resource to check
 * @param MipIndex - Which mipmap we're interested in
 * @return TRUE if the texture is still in use by the GPU, otherwise FALSE
 */
DEFINE_RHIMETHOD(
	UBOOL,
	IsBusyTexture2D,
	(FTexture2DRHIParamRef Texture, UINT MipIndex),
	( Texture, MipIndex ),
	return,
	return FALSE;
	);

/**
 * Checks if a vertex buffer is still in use by the GPU.
 * @param VertexBuffer - the RHI texture resource to check
 * @return TRUE if the texture is still in use by the GPU, otherwise FALSE
 */
DEFINE_RHIMETHOD(
	UBOOL,
	IsBusyVertexBuffer,
	(FVertexBufferRHIParamRef VertexBuffer),
	( VertexBuffer ),
	return,
	return FALSE;
	);
