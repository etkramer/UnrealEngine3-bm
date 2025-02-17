/*=============================================================================
	BoundShaderStateCache.h: Bound shader state cache definition.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __BOUNDSHADERCACHE_H__
#define __BOUNDSHADERCACHE_H__

/**
* Key used to map a set of unique decl/vs/ps combinations to
* a vertex shader resource
*/
class FBoundShaderStateKey
{
public:
	/** Initialization constructor. */
	FBoundShaderStateKey(
		FVertexDeclarationRHIParamRef InVertexDeclaration, 
		DWORD* InStreamStrides, 
		FVertexShaderRHIParamRef InVertexShader, 
		FPixelShaderRHIParamRef InPixelShader
		)
		: VertexDeclaration(InVertexDeclaration)
		, VertexShader(InVertexShader)
		, PixelShader(InPixelShader)
	{
		for( UINT Idx=0; Idx < MaxVertexElementCount; ++Idx )
		{
			StreamStrides[Idx] = (BYTE)InStreamStrides[Idx];
		}
	}

	/**
	* Equality is based on decl, vertex shader and pixel shader 
	* @param Other - instance to compare against
	* @return TRUE if equal
	*/
	friend UBOOL operator ==(const FBoundShaderStateKey& A,const FBoundShaderStateKey& B)
	{
		return	A.VertexDeclaration == B.VertexDeclaration && 
			A.VertexShader == B.VertexShader && 
			A.PixelShader == B.PixelShader && 
			!appMemcmp(A.StreamStrides, B.StreamStrides, sizeof(A.StreamStrides));
	}

	/**
	* Get the hash for this type. 
	* @param Key - struct to hash
	* @return dword hash based on type
	*/
	friend DWORD GetTypeHash(const FBoundShaderStateKey &Key)
	{
		return GetTypeHash(Key.VertexDeclaration) ^
			GetTypeHash(Key.VertexShader) ^
			GetTypeHash(Key.PixelShader) ^
			appMemCrc(Key.StreamStrides,sizeof(Key.StreamStrides));
	}

private:
	/** Note: we intentionally do not AddRef() these shaders
	*  because we only need their pointer values for hashing purposes.
	*  The object which owns the corresponding RHIBoundShaderState 
	*  should have references to these objects in its material
	*  and vertex factory.
	*/

	/** vertex decl for this combination */
	FVertexDeclarationRHIParamRef VertexDeclaration;
	/** vs for this combination */
	FVertexShaderRHIParamRef VertexShader;
	/** ps for this combination */
	FPixelShaderRHIParamRef PixelShader;
	/** assuming stides are always smaller than 8-bits */
	BYTE StreamStrides[MaxVertexElementCount];
};

/**
 * Encapsulates a bound shader state's entry in the cache.
 * Handles removal from the bound shader state cache on destruction.
 * RHIs that use cached bound shader states should create one for each bound shader state.
 */
class FCachedBoundShaderStateLink
{
public:

	/**
	 * The cached bound shader state.  This is not a reference counted pointer because we rely on the RHI to destruct this object
	 * when the bound shader state this references is destructed.
	 */
	FBoundShaderStateRHIParamRef BoundShaderState;

	/** Adds the bound shader state to the cache. */
	FCachedBoundShaderStateLink(
		FVertexDeclarationRHIParamRef VertexDeclaration,
		DWORD* StreamStrides,
		FVertexShaderRHIParamRef VertexShader,
		FPixelShaderRHIParamRef PixelShader,
		FBoundShaderStateRHIParamRef InBoundShaderState
		);

	/** Destructor.  Removes the bound shader state from the cache. */
	~FCachedBoundShaderStateLink();

private:

	FBoundShaderStateKey Key;
};

/**
 * Searches for a cached bound shader state.
 * @return If a bound shader state matching the parameters is cached, it is returned; otherwise NULL is returned.
 */
extern FCachedBoundShaderStateLink* GetCachedBoundShaderState(
	FVertexDeclarationRHIParamRef VertexDeclaration,
	DWORD* StreamStrides,
	FVertexShaderRHIParamRef VertexShader,
	FPixelShaderRHIParamRef PixelShader
	);

/**
 * A list of the most recently used bound shader states.
 * This is used to keep bound shader states that have been used recently from being freed, as they're likely to be used again soon.
 */
template<UINT Size>
class TBoundShaderStateHistory : public FRenderResource
{
public:

	/** Initialization constructor. */
	TBoundShaderStateHistory():
		NextBoundShaderStateIndex(0)
	{}

	/** Adds a bound shader state to the history. */
	void Add(FBoundShaderStateRHIParamRef BoundShaderState)
	{
		BoundShaderStates[NextBoundShaderStateIndex] = BoundShaderState;
		NextBoundShaderStateIndex = (NextBoundShaderStateIndex + 1) % Size;
	}

	// FRenderResource interface.
	virtual void ReleaseRHI()
	{
		for(UINT Index = 0;Index < Size;Index++)
		{
			BoundShaderStates[Index].SafeRelease();
		}
	}

private:

	FBoundShaderStateRHIRef BoundShaderStates[Size];
	UINT NextBoundShaderStateIndex;
};

#endif
