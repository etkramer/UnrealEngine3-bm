/*=============================================================================
	D3DShaders.cpp: D3D shader RHI implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeD3DDrvPrivate.h"

#if USE_XeD3D_RHI

FVertexShaderRHIRef RHICreateVertexShader(const TArray<BYTE>& Code)
{
	FVertexShaderRHIRef VertexShaderRHIRef( new FXeVertexShader( Code.GetData() ) );
	return VertexShaderRHIRef;
}

FPixelShaderRHIRef RHICreatePixelShader(const TArray<BYTE>& Code)
{
	FPixelShaderRHIRef PixelShaderRHIRef( new FXePixelShader( Code.GetData() ) );
	return PixelShaderRHIRef;
}

/**
 * Vertex shader GPU resource constructor.
 * 
 * @param	Code	Compiled code created by D3DXCompileShader
 */
FXeVertexShader::FXeVertexShader( const void* Code ) 
:	FXeGPUResource(RUF_Static)
{
	XGMICROCODESHADERPARTS MicroShaderParts;
	XGGetMicrocodeShaderParts( Code, &MicroShaderParts );

	IDirect3DVertexShader9* VertexShader = (IDirect3DVertexShader9*) new char[MicroShaderParts.cbCachedPartSize];
	XGSetVertexShaderHeader( VertexShader, MicroShaderParts.cbCachedPartSize, &MicroShaderParts );

	INT AlignedSize = Align( MicroShaderParts.cbPhysicalPartSize, VERTEXSHADER_ALIGNMENT );
	BaseAddress		= appPhysicalAlloc( AlignedSize, CACHE_WriteCombine );

	appMemcpy( BaseAddress, MicroShaderParts.pPhysicalPart, MicroShaderParts.cbPhysicalPartSize );
	XGRegisterVertexShader( VertexShader, BaseAddress );

	Resource = VertexShader;

#if TRACK_GPU_RESOURCES
	PhysicalSize = AlignedSize;
	VirtualSize  = MicroShaderParts.cbCachedPartSize;
	INC_DWORD_STAT_BY(STAT_VertexShaderMemory,PhysicalSize+VirtualSize);
#endif
}

#if TRACK_GPU_RESOURCES
/** Destructor, used when tracking memory. */
FXeVertexShader::~FXeVertexShader()
{
	DEC_DWORD_STAT_BY(STAT_VertexShaderMemory,PhysicalSize+VirtualSize);
}
#endif

/**
 * Pixel shader GPU resource constructor.
 * 
 * @param	Code	Compiled code created by D3DXCompileShader
 */
FXePixelShader::FXePixelShader( const void* Code )
:	FXeGPUResource(RUF_Static)
{
	XGMICROCODESHADERPARTS MicroShaderParts;
	XGGetMicrocodeShaderParts( Code, &MicroShaderParts );

	IDirect3DPixelShader9* PixelShader = (IDirect3DPixelShader9*) new char[MicroShaderParts.cbCachedPartSize];
	XGSetPixelShaderHeader( PixelShader, MicroShaderParts.cbCachedPartSize, &MicroShaderParts );

	INT AlignedSize	= Align( MicroShaderParts.cbPhysicalPartSize, PIXELSHADER_ALIGNMENT );
	BaseAddress		= appPhysicalAlloc( AlignedSize, CACHE_WriteCombine );

	appMemcpy( BaseAddress, MicroShaderParts.pPhysicalPart, MicroShaderParts.cbPhysicalPartSize );
	XGRegisterPixelShader( PixelShader, BaseAddress );

	Resource = PixelShader;

#if TRACK_GPU_RESOURCES
	PhysicalSize = AlignedSize;
	VirtualSize  = MicroShaderParts.cbCachedPartSize;
	INC_DWORD_STAT_BY(STAT_PixelShaderMemory,PhysicalSize+VirtualSize);
#endif
}

#if TRACK_GPU_RESOURCES
/** Destructor, used when tracking memory. */
FXePixelShader::~FXePixelShader()
{
	DEC_DWORD_STAT_BY(STAT_PixelShaderMemory,PhysicalSize+VirtualSize);
}
#endif

/**
 * Key used to map a set of unique decl/vs/ps combinations to
 * a vertex shader resource
 */
class FShaderCombinationKey
{
public:
	/**
	 * Constructor (Init)
	 * @param InVertexDeclaration - existing decl
	 * @param InStreamStrides - array of vertex stream strides
	 * @param InVertexShader - existing vs
	 * @param InPixelShader - existing ps
	 */
	FShaderCombinationKey(
		void *InVertexDeclaration, 
		DWORD *InStreamStrides, 
		void *InVertexShader, 
		void *InPixelShader
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
	UBOOL operator == (const FShaderCombinationKey &Other) const
	{
		return( 
			VertexDeclaration == Other.VertexDeclaration && 
			VertexShader == Other.VertexShader && 
			PixelShader == Other.PixelShader && 
			(0 == appMemcmp(StreamStrides, Other.StreamStrides, sizeof(StreamStrides)))
			);
	}

	/**
	 * Get the hash for this type. 
	 * @param Key - struct to hash
	 * @return dword hash based on type
	 */
	friend DWORD GetTypeHash(const FShaderCombinationKey &Key)
	{
		return PointerHash(
			Key.VertexDeclaration, 
			PointerHash(
				Key.VertexShader, 
				PointerHash(
					Key.PixelShader, 
					appMemCrc(Key.StreamStrides, sizeof(Key.StreamStrides))
					)
				)
			);
	}

private:
	/** Note: we intentionally do not AddRef() these shaders
	  *  because we only need their pointer values for hashing purposes.
	  *  The object which owns the corresponding RHIBoundShaderState 
	  *  should have references to these objects in its material
	  *  and vertex factory.
	  */

	/** vertex decl for this combination */
	void* VertexDeclaration;
	/** vs for this combination */
	void* VertexShader;
	/** ps for this combination */
	void* PixelShader;
	/** assuming stides are always smaller than 8-bits */
	BYTE StreamStrides[MaxVertexElementCount];
};

/**
 * Set of bound shaders
 */
class FBoundShaderCache
{
public:
	/**
	 * Constructor
	 */
	FBoundShaderCache()
	{
		// We are assuming that strides can fit into 8-bits
        check((BYTE)GPU_MAX_VERTEX_STRIDE == GPU_MAX_VERTEX_STRIDE);
		PendingDeletes = 0;
	}

	/**
	 * Destructor
	 */
	~FBoundShaderCache()
	{
		PurgeAndRelax(TRUE);

		check(0 == BoundVertexShaderMap.Num());
	}

	/**
	 * Create a new vertex shader using the given vs decl,vs,ps combination. Tries to find an existing entry in the set first.
	 * 
	 * @param InVertexDeclaration - existing vertex decl
	 * @param InStreamStrides - optional stream strides
	 * @param InVertexShader - existing vertex shader
	 * @param InPixelShader - existing pixel shader
	 * @return bound vertex shader entry
	 */
	FCachedVertexShader * GetBoundShader(
		FVertexDeclarationRHIParamRef InVertexDeclaration, 
		DWORD *InStreamStrides, 
		FVertexShaderRHIParamRef InVertexShader, 
		FPixelShaderRHIParamRef InPixelShader
		)
	{
		PurgeAndRelax(FALSE);

		FShaderCombinationKey Key(InVertexDeclaration, InStreamStrides, InVertexShader, InPixelShader);
		FCachedVertexShader **Value = BoundVertexShaderMap.Find(Key);

		if (!Value)
		{
			FCachedVertexShader *NewCachedShader = new FCachedVertexShader;
			NewCachedShader->VertexShader = CloneVertexShader(InVertexShader);
			NewCachedShader->CacheRefCount = 1;
			
			// Workaround for bind
			if (!InPixelShader)
			{
				TShaderMapRef<FNULLPixelShader> NullPixelShader(GetGlobalShaderMap());
				InPixelShader = NullPixelShader->GetPixelShader();
			}
            
			IDirect3DVertexShader9* XeD3DVertexShader = NewCachedShader->VertexShader;
			VERIFYD3DRESULT(XeD3DVertexShader->Bind(0, InVertexDeclaration, InStreamStrides, (IDirect3DPixelShader9*) InPixelShader->Resource ));
			BoundVertexShaderMap.Set(Key, NewCachedShader);

			return NewCachedShader;
		}
		else if ((*Value)->CacheRefCount == 0)
		{
			// This one was formerly marked for deletion, but we can re-use it.  Drop the pending delete count by 1
			check(PendingDeletes > 0);
			-- PendingDeletes;
		}
		
		++ (*Value)->CacheRefCount;
		return *Value;
	}

	/**
	 * Deferred cleanup only when we hit the threshold of shaders to delete. 
	 * Iterates over the map of bound shaders and deletes unreferenced entries.
	 * @param bForce - ignore threshold and just delete unreferenced shaders
	 */
	void PurgeAndRelax(UBOOL bForce)
	{
		INT DeletedSoFar = 0;
		if (PendingDeletes > PendingDeleteThreshold || bForce)
		{
			TMap<FShaderCombinationKey, FCachedVertexShader *>::TIterator ShaderMapIt(BoundVertexShaderMap);
			while (ShaderMapIt)
			{
				const FCachedVertexShader* Shader = ShaderMapIt.Value();

				if (0 == Shader->CacheRefCount)
				{
					delete Shader;
					ShaderMapIt.RemoveCurrent();
					++ DeletedSoFar;
				}
				++ ShaderMapIt;
			}
			check(PendingDeletes == DeletedSoFar);
			PendingDeletes = 0;
		}
	}

	/**
	 * Increment a new shader delete request
	 */
	void IncrementDeletedShaderCount()
	{
		++ PendingDeletes;
	}

private:

	/**
	 * Create a unique copy of the given vs
	 * @param InVertexShader - instance to copy from
	 * @return newly created vertex shader 
	 */
	FVertexShaderRHIParamRef CloneVertexShader(FVertexShaderRHIParamRef InVertexShader)
	{
		UINT					ShaderCodeSize;
		IDirect3DVertexShader9* XeD3DVertexShader = (IDirect3DVertexShader9*) InVertexShader->Resource;

		VERIFYD3DRESULT(XeD3DVertexShader->GetFunction(NULL, &ShaderCodeSize));
		TempVertexShaderCode.Reserve(ShaderCodeSize);
		VERIFYD3DRESULT(XeD3DVertexShader->GetFunction(TempVertexShaderCode.GetTypedData(), &ShaderCodeSize));

		return new FXeVertexShader( TempVertexShaderCode.GetData() );
	}

	/** map decl/vs/ps combination to bound vertex shader resource */
	TMap<FShaderCombinationKey, FCachedVertexShader *> BoundVertexShaderMap;
	/** stores vertex shader code in CloneVertexShader */
	TArray<BYTE> TempVertexShaderCode;

	/** Cleaning the map gets expensive; best not to do it on every delete */
	UINT PendingDeletes;
	static const UINT PendingDeleteThreshold = 200;
};

FBoundShaderCache GBoundShaderCache;

/**
 * Keep track of the number of deleted shader requests
 */
void MarkShaderForDeletion()
{
	GBoundShaderCache.IncrementDeletedShaderCount();
}

/**
 * Creates a bound shader state instance which encapsulates a decl, vertex shader, and pixel shader
 * @param VertexDeclaration - existing vertex decl
 * @param StreamStrides - optional stream strides
 * @param VertexShader - existing vertex shader
 * @param PixelShader - existing pixel shader
 */
FBoundShaderStateRHIRef RHICreateBoundShaderState(
	FVertexDeclarationRHIParamRef VertexDeclaration, 
	DWORD *StreamStrides, 
	FVertexShaderRHIParamRef VertexShader, 
	FPixelShaderRHIParamRef PixelShader
	)
{
	FBoundShaderStateRHIRef NewBoundShaderState;

#ifdef _DEBUG
	//@TODO: Workaround for bug in D3D that fails to accept prepatched vertex shaders
	NewBoundShaderState.VertexDeclaration = VertexDeclaration;
#endif

	NewBoundShaderState.BoundVertexShader = GBoundShaderCache.GetBoundShader(VertexDeclaration, StreamStrides, VertexShader, PixelShader);
	NewBoundShaderState.PixelShader = PixelShader;

	return NewBoundShaderState;
}

#endif
