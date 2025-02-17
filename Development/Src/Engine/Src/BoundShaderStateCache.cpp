/*=============================================================================
	BoundShaderStateCache.cpp: Bound shader state cache implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "BoundShaderStateCache.h"

typedef TMap<FBoundShaderStateKey,FCachedBoundShaderStateLink*> FBoundShaderStateCache;

/** Lazily initialized bound shader state cache singleton. */
static FBoundShaderStateCache& GetBoundShaderStateCache()
{
	static FBoundShaderStateCache BoundShaderStateCache;
	return BoundShaderStateCache;
}

FCachedBoundShaderStateLink::FCachedBoundShaderStateLink(
	FVertexDeclarationRHIParamRef VertexDeclaration,
	DWORD* StreamStrides,
	FVertexShaderRHIParamRef VertexShader,
	FPixelShaderRHIParamRef PixelShader,
	FBoundShaderStateRHIParamRef InBoundShaderState
	):
	BoundShaderState(InBoundShaderState),
	Key(VertexDeclaration,StreamStrides,VertexShader,PixelShader)
{
	// Add this bound shader state to the cache.
	GetBoundShaderStateCache().Set(Key,this);
}

FCachedBoundShaderStateLink::~FCachedBoundShaderStateLink()
{
	GetBoundShaderStateCache().Remove(Key);
}

FCachedBoundShaderStateLink* GetCachedBoundShaderState(
	FVertexDeclarationRHIParamRef VertexDeclaration,
	DWORD* StreamStrides,
	FVertexShaderRHIParamRef VertexShader,
	FPixelShaderRHIParamRef PixelShader
	)
{
	// Find the existing bound shader state in the cache.
	return GetBoundShaderStateCache().FindRef(
		FBoundShaderStateKey(VertexDeclaration,StreamStrides,VertexShader,PixelShader)
		);
}
