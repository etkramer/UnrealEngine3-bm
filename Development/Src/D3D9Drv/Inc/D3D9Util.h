/*=============================================================================
	D3D9Util.h: D3D RHI utility definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** Returns the string equivalent of the input ErrorCode. */
extern FString GetD3D9ErrorString(HRESULT ErrorCode);

/**
 * Checks that the given result isn't a failure.  If it is, the application exits with an appropriate error message.
 * @param	Result - The result code to check
 * @param	Code - The code which yielded the result.
 * @param	Filename - The filename of the source file containing Code.
 * @param	Line - The line number of Code within Filename.
 */
extern void VerifyD3D9Result(HRESULT Result,const ANSICHAR* Code,const ANSICHAR* Filename,UINT Line);

/**
* Checks that the given result isn't a failure.  If it is, the application exits with an appropriate error message.
* @param	Result - The result code to check
* @param	Code - The code which yielded the result.
* @param	Filename - The filename of the source file containing Code.
* @param	Line - The line number of Code within Filename.	
*/
extern void VerifyD3D9CreateTextureResult(HRESULT D3DResult,const ANSICHAR* Code,const ANSICHAR* Filename,UINT Line,
										 UINT SizeX,UINT SizeY,BYTE D3DFormat,UINT NumMips,DWORD Flags);

/**
 * A macro for using VERIFYD3D9RESULT that automatically passes in the code and filename/line.
 */
#if DO_CHECK
#define VERIFYD3D9RESULT(x) VerifyD3D9Result(x,#x,__FILE__,__LINE__);
#define VERIFYD3D9CREATETEXTURERESULT(x,SizeX,SizeY,Format,NumMips,Flags) VerifyD3D9CreateTextureResult(x,#x,__FILE__,__LINE__,SizeX,SizeY,Format,NumMips,Flags);
#else
#define VERIFYD3D9RESULT(x) (x);
#define VERIFYD3D9CREATETEXTURERESULT(x,SizeX,SizeY,Format,NumMips,Flags) (x);
#endif

/**
* Convert from ECubeFace to D3DCUBEMAP_FACES type
* @param Face - ECubeFace type to convert
* @return D3D cube face enum value
*/
FORCEINLINE D3DCUBEMAP_FACES GetD3D9CubeFace(ECubeFace Face)
{
	switch(Face)
	{
	case CubeFace_PosX:
	default:
		return D3DCUBEMAP_FACE_POSITIVE_X;
	case CubeFace_NegX:
		return D3DCUBEMAP_FACE_NEGATIVE_X;
	case CubeFace_PosY:
		return D3DCUBEMAP_FACE_POSITIVE_Y;
	case CubeFace_NegY:
		return D3DCUBEMAP_FACE_NEGATIVE_Y;
	case CubeFace_PosZ:
		return D3DCUBEMAP_FACE_POSITIVE_Z;
	case CubeFace_NegZ:
		return D3DCUBEMAP_FACE_NEGATIVE_Z;
	};
}
