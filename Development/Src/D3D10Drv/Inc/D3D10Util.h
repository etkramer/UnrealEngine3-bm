/*=============================================================================
	D3D10Util.h: D3D RHI utility definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * Checks that the given result isn't a failure.  If it is, the application exits with an appropriate error message.
 * @param	Result - The result code to check
 * @param	Code - The code which yielded the result.
 * @param	Filename - The filename of the source file containing Code.
 * @param	Line - The line number of Code within Filename.
 */
extern void VerifyD3D10Result(HRESULT Result,const ANSICHAR* Code,const ANSICHAR* Filename,UINT Line);

/**
* Checks that the given result isn't a failure.  If it is, the application exits with an appropriate error message.
* @param	Result - The result code to check
* @param	Code - The code which yielded the result.
* @param	Filename - The filename of the source file containing Code.
* @param	Line - The line number of Code within Filename.	
*/
extern void VerifyD3D10CreateTextureResult(HRESULT D3DResult,const ANSICHAR* Code,const ANSICHAR* Filename,UINT Line,
										 UINT SizeX,UINT SizeY,BYTE D3DFormat,UINT NumMips,DWORD Flags);

/**
 * A macro for using VERIFYD3D10RESULT that automatically passes in the code and filename/line.
 */
#if DO_CHECK
#define VERIFYD3D10RESULT(x) VerifyD3D10Result(x,#x,__FILE__,__LINE__);
#define VERIFYD3D10CREATETEXTURERESULT(x,SizeX,SizeY,Format,NumMips,Flags) VerifyD3D10CreateTextureResult(x,#x,__FILE__,__LINE__,SizeX,SizeY,Format,NumMips,Flags);
#else
#define VERIFYD3D10RESULT(x) (x);
#define VERIFYD3D10CREATETEXTURERESULT(x,SizeX,SizeY,Format,NumMips,Flags) (x);
#endif

/**
* Convert from ECubeFace to D3DCUBEMAP_FACES type
* @param Face - ECubeFace type to convert
* @return D3D cube face enum value
*/
FORCEINLINE UINT GetD3D10CubeFace(ECubeFace Face)
{
	switch(Face)
	{
	case CubeFace_PosX:
	default:
		return 0;//D3DCUBEMAP_FACE_POSITIVE_X;
	case CubeFace_NegX:
		return 1;//D3DCUBEMAP_FACE_NEGATIVE_X;
	case CubeFace_PosY:
		return 2;//D3DCUBEMAP_FACE_POSITIVE_Y;
	case CubeFace_NegY:
		return 3;//D3DCUBEMAP_FACE_NEGATIVE_Y;
	case CubeFace_PosZ:
		return 4;//D3DCUBEMAP_FACE_POSITIVE_Z;
	case CubeFace_NegZ:
		return 5;//D3DCUBEMAP_FACE_NEGATIVE_Z;
	};
}

/**
 * Keeps track of Locks for D3D10 objects
 */
class FD3D10LockedKey
{
public:
	void* SourceObject;
	UINT Subresource;

public:
	FD3D10LockedKey() : SourceObject(NULL)
		, Subresource(0)
	{}
	FD3D10LockedKey(ID3D10Texture2D* source, UINT subres=0) : SourceObject((void*)source)
		, Subresource(subres)
	{}
	FD3D10LockedKey(ID3D10Buffer* source, UINT subres=0) : SourceObject((void*)source)
		, Subresource(subres)
	{}
	UBOOL operator==( const FD3D10LockedKey& Other ) const
	{
		return SourceObject == Other.SourceObject && Subresource == Other.Subresource;
	}
	UBOOL operator!=( const FD3D10LockedKey& Other ) const
	{
		return SourceObject != Other.SourceObject || Subresource != Other.Subresource;
	}
	FD3D10LockedKey& operator=( const FD3D10LockedKey& Other )
	{
		SourceObject = Other.SourceObject;
		Subresource = Other.Subresource;
		return *this;
	}
	DWORD GetHash() const
	{
		return (DWORD)SourceObject;
	}

	/** Hashing function. */
	friend DWORD GetTypeHash( const FD3D10LockedKey& K )
	{
		return K.GetHash();
	}
};

/** Information about a D3D resource that is currently locked. */
struct FD3D10LockedData
{
	TRefCountPtr<ID3D10Resource> StagingResource;
	BYTE* Data;
	UINT Pitch;
	UBOOL bWrite;
};
