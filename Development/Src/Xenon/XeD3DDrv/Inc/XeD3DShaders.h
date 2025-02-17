/*=============================================================================
	XeD3DShaders.h: XeD3D shader RHI definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if USE_XeD3D_RHI

/*-----------------------------------------------------------------------------
	RHI shader types
-----------------------------------------------------------------------------*/

/**
 * Our version of vertex shader resource. Required as we need to manage the lifetime of the object
 * and delete it ourselves.
 */
struct FXeVertexShader : public FXeGPUResource 
{
	/**
	 * Constructor, creating D3D resource from passed in code.
	 *
	 * @param	Code	Shader code, as created by D3DXCompileShader
	 */
	FXeVertexShader( const void* Code );

#if TRACK_GPU_RESOURCES
	/** Destructor, used when tracking memory. */
	virtual ~FXeVertexShader();
#endif

	/**
	 * @return Resource type name.
	 */
	virtual TCHAR* GetTypeName() const
	{
		return TEXT("VertexShader");
	}
};

/**
 * Our version of pixel shader resource. Required as we need to manage the lifetime of the object
 * and delete it ourselves.
 */
struct FXePixelShader : public FXeGPUResource
{
	/**
	 * Constructor, creating D3D resource from passed in code.
	 *
	 * @param	Code	Shader code, as created by D3DXCompileShader
	 */
	FXePixelShader( const void* Code );

#if TRACK_GPU_RESOURCES
	/** Destructor, used when tracking memory. */
	virtual ~FXePixelShader();
#endif

	/**
	 * @return Resource type name.
	 */
	virtual TCHAR* GetTypeName() const
	{
		return TEXT("PixelShader");
	}
};

typedef TXeGPUResourceRef<IDirect3DVertexShader9,FXeVertexShader> FVertexShaderRHIRef;
typedef FXeVertexShader* FVertexShaderRHIParamRef;

typedef TXeGPUResourceRef<IDirect3DPixelShader9,FXePixelShader> FPixelShaderRHIRef;
typedef FXePixelShader* FPixelShaderRHIParamRef;

/** vertex shader and its current refcount */
struct FCachedVertexShader
{
	FVertexShaderRHIRef VertexShader;
	UINT CacheRefCount;
};

void MarkShaderForDeletion();

class FBoundShaderStateRHIRef;
typedef const FBoundShaderStateRHIRef& FBoundShaderStateRHIParamRef;

/**
 * Combined shader state and vertex definition for rendering geometry. 
 * Each unique instance consists of a vertex decl, vertex shader, and pixel shader.
 */
class FBoundShaderStateRHIRef
{
public:
	/**
	 * Constructor
	 */
	FBoundShaderStateRHIRef() :
	  BoundVertexShader(NULL)
	  {}

	/**
	 * Constructor (Copy)
	 * @param Copy - instance to copy from
	 */
	FBoundShaderStateRHIRef(const FBoundShaderStateRHIRef& Copy)
	{
		BoundVertexShader = Copy.BoundVertexShader;
		if(BoundVertexShader)
		{
			// addref on copy
			++ BoundVertexShader->CacheRefCount;
		}
		PixelShader = Copy.PixelShader;
	}

	/**
	 * Destructor
	 */
	~FBoundShaderStateRHIRef()
	{
		SafeRelease();
	}

	/**
	 * Assignment op
	 * @param InPtr - instance to assign from
	 * @return bound shader state instance
	 */
	FBoundShaderStateRHIRef& operator=(const FBoundShaderStateRHIRef& InPtr)
	{
		++ InPtr.BoundVertexShader->CacheRefCount;

		if (BoundVertexShader)
		{
			// decrement existing vertex shader ref
			check(BoundVertexShader->CacheRefCount > 0);
			-- BoundVertexShader->CacheRefCount;
			if (0 == BoundVertexShader->CacheRefCount)
			{
				// defer deletion
				MarkShaderForDeletion();
			}
		}
		BoundVertexShader = InPtr.BoundVertexShader;
		PixelShader = InPtr.PixelShader;

		return *this;
	}

	/**
	 * Equality is based on vertex shader and pixel shader only. Decl is not important on Xe
	 * @param Other - instance to compare against
	 * @return TRUE if equal
	 */
	UBOOL operator==(const FBoundShaderStateRHIRef& Other) const
	{
		return (BoundVertexShader == Other.BoundVertexShader && PixelShader == Other.PixelShader);
	}

	/**
	 * Get the hash for this type. 
	 * @param Key - struct to hash
	 * @return dword hash based on type
	 */
	friend DWORD GetTypeHash(const FBoundShaderStateRHIRef &Key)
	{
		return PointerHash(Key.BoundVertexShader, PointerHash((IDirect3DPixelShader9 *)Key.PixelShader));
	}

	// RHI resource reference interface.
	friend UBOOL IsValidRef(const FBoundShaderStateRHIRef& Ref)
	{
		return Ref.BoundVertexShader != NULL;
	}
	void SafeRelease()
	{
		if(BoundVertexShader)
		{
			check(BoundVertexShader->CacheRefCount > 0);
			-- BoundVertexShader->CacheRefCount;
			if (0 == BoundVertexShader->CacheRefCount)
			{
				// defer deletion
				MarkShaderForDeletion();
			}

			BoundVertexShader = NULL;
		}

		PixelShader = NULL;
#ifdef _DEBUG
		VertexDeclaration = NULL;
#endif
	}

	friend FBoundShaderStateRHIRef RHICreateBoundShaderState(FVertexDeclarationRHIParamRef VertexDeclaration, DWORD *StreamStrides, FVertexShaderRHIParamRef VertexShader, FPixelShaderRHIParamRef PixelShader);
	friend void RHISetBoundShaderState(FBoundShaderStateRHIParamRef BoundShaderState);

private:
#ifdef _DEBUG
	//@TODO: Workaround for bug in D3D that fails to accept prepatched vertex shaders
	FVertexDeclarationRHIRef VertexDeclaration;
#endif
	FCachedVertexShader *BoundVertexShader;
	FPixelShaderRHIRef PixelShader;
};

#endif
