/*=============================================================================
	StaticBoundShaderState.cpp: Static bound shader state implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "StaticBoundShaderState.h"

TLinkedList<FGlobalBoundShaderStateResource*>*& FGlobalBoundShaderStateResource::GetGlobalBoundShaderStateList()
{
	static TLinkedList<FGlobalBoundShaderStateResource*>* List = NULL;
	return List;
}

FGlobalBoundShaderStateResource::FGlobalBoundShaderStateResource():
	GlobalListLink(this)
{
	// Add this resource to the global list in the rendering thread.
	if(IsInRenderingThread())
	{
		GlobalListLink.Link(GetGlobalBoundShaderStateList());
	}
	else
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			LinkGlobalBoundShaderStateResource,FGlobalBoundShaderStateResource*,Resource,this,
			{
				Resource->GlobalListLink.Link(GetGlobalBoundShaderStateList());
			});
	}
}

FGlobalBoundShaderStateResource::~FGlobalBoundShaderStateResource()
{
	// Remove this resource from the global list.
	GlobalListLink.Unlink();
}

/**
 * Initializes a global bound shader state with a vanilla bound shader state and required information.
 */
FBoundShaderStateRHIParamRef FGlobalBoundShaderStateResource::GetInitializedRHI(
	FVertexDeclarationRHIParamRef VertexDeclaration, 
	FVertexShaderRHIParamRef VertexShader, 
	FPixelShaderRHIParamRef PixelShader,
	UINT VertexStreamStride
	)
{
	check(IsInitialized());

	// This should only be called in the rendering thread after the RHI has been initialized.
	check(GIsRHIInitialized);
	check(IsInRenderingThread());

	// Create the bound shader state if it hasn't been cached yet.
	if(!IsValidRef(BoundShaderState))
	{
		DWORD VertexStreamStrides[MaxVertexElementCount];
		appMemzero(VertexStreamStrides, sizeof(VertexStreamStrides));
		VertexStreamStrides[0] = VertexStreamStride;
		BoundShaderState = RHICreateBoundShaderState(VertexDeclaration,VertexStreamStrides,VertexShader,PixelShader);
	}
#if _DEBUG
	else
	{
		DWORD VertexStreamStrides[MaxVertexElementCount];
		appMemzero(VertexStreamStrides, sizeof(VertexStreamStrides));
		VertexStreamStrides[0] = VertexStreamStride;
		FBoundShaderStateRHIRef TempBoundShaderState = RHICreateBoundShaderState(VertexDeclaration,VertexStreamStrides,VertexShader,PixelShader);
		// Verify that bound shader state caching is working and that the passed in shaders will actually be used
		// This will catch cases where one bound shader state is being used with more than one combination of shaders
		// Otherwise setting the shader will just silently fail once the bound shader state has been initialized with a different shader 
		// The catch is that this uses the caching mechanism to test for equality, instead of comparing the actual platform dependent shader references.
		checkSlow(TempBoundShaderState == BoundShaderState);
	}
#endif

	return BoundShaderState;
}

void FGlobalBoundShaderStateResource::ReleaseRHI()
{
	// Release the cached bound shader state.
	BoundShaderState.SafeRelease();
}

/**
 * SetGlobalBoundShaderState - sets the global bound shader state, also creates and caches it if necessary
 *
 * @param BoundShaderState - current bound shader state, will be updated if it wasn't a valid ref
 * @param VertexDeclaration - the vertex declaration to use in creating the new bound shader state
 * @param VertexShader - the vertex shader to use in creating the new bound shader state
 * @param PixelShader - the pixel shader to use in creating the new bound shader state
 * @param Stride
 */
void SetGlobalBoundShaderState(
	FGlobalBoundShaderState& GlobalBoundShaderState,
	FVertexDeclarationRHIParamRef VertexDeclaration,
	FShader* VertexShader,
	FShader* PixelShader,
	UINT VertexStreamStride
	)
{
	RHISetBoundShaderState(
		GlobalBoundShaderState.GetInitializedRHI(
			VertexDeclaration,
			VertexShader ? (FVertexShaderRHIParamRef)VertexShader->GetVertexShader() : FVertexShaderRHIParamRef(),
			PixelShader ? (FPixelShaderRHIParamRef)PixelShader->GetPixelShader() : FPixelShaderRHIParamRef(),
			VertexStreamStride
			)
		);
}
