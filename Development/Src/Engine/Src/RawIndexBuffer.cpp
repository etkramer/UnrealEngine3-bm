/*=============================================================================
	RawIndexBuffer.cpp: Raw index buffer implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#if !CONSOLE && !PLATFORM_UNIX
#include "NVTriStrip.h"

/**
* Converts a triangle list into a triangle strip.
*/
template<typename Allocator>
INT StripifyIndexBuffer(TArray<WORD,Allocator>& Indices)
{
	PrimitiveGroup*	PrimitiveGroups = NULL;
	WORD			NumPrimitiveGroups = 0;

	SetListsOnly(false);
	GenerateStrips(&Indices(0),Indices.Num(),&PrimitiveGroups,&NumPrimitiveGroups);

	Indices.Empty();
	Indices.Add(PrimitiveGroups->numIndices);
	appMemcpy(&Indices(0),PrimitiveGroups->indices,Indices.Num() * sizeof(WORD));

	delete [] PrimitiveGroups;

	return Indices.Num() - 2;
}

/**
* Orders a triangle list for better vertex cache coherency.
*/
template<typename Allocator>
static void CacheOptimizeIndexBuffer(TArray<WORD,Allocator>& Indices)
{
	PrimitiveGroup*	PrimitiveGroups = NULL;
	WORD			NumPrimitiveGroups = 0;

	SetListsOnly(true);
	GenerateStrips(&Indices(0),Indices.Num(),&PrimitiveGroups,&NumPrimitiveGroups);

	Indices.Empty();
	Indices.Add(PrimitiveGroups->numIndices);
	appMemcpy(&Indices(0),PrimitiveGroups->indices,Indices.Num() * sizeof(WORD));

	delete [] PrimitiveGroups;
}
#endif //#if !CONSOLE && !PLATFORM_UNIX

/*-----------------------------------------------------------------------------
FRawIndexBuffer
-----------------------------------------------------------------------------*/

/**
* Converts a triangle list into a triangle strip.
*/
INT FRawIndexBuffer::Stripify()
{
#if !CONSOLE && !PLATFORM_UNIX
	return StripifyIndexBuffer(Indices);
#else
	return 0;
#endif
}

/**
* Orders a triangle list for better vertex cache coherency.
*/
void FRawIndexBuffer::CacheOptimize()
{
#if !CONSOLE && !PLATFORM_UNIX
	CacheOptimizeIndexBuffer(Indices);
#endif
}

void FRawIndexBuffer::InitRHI()
{
	DWORD Size = Indices.Num() * sizeof(WORD);
	if( Size > 0 )
	{
		// Create the index buffer.
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(WORD),Size,NULL,RUF_Static);

		// Initialize the buffer.
		void* Buffer = RHILockIndexBuffer(IndexBufferRHI,0,Size);
		appMemcpy(Buffer,&Indices(0),Size);
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
}

FArchive& operator<<(FArchive& Ar,FRawIndexBuffer& I)
{
	I.Indices.BulkSerialize( Ar );
	return Ar;
}

/*-----------------------------------------------------------------------------
FRawIndexBuffer32
-----------------------------------------------------------------------------*/

void FRawIndexBuffer32::InitRHI()
{
	DWORD Size = Indices.Num() * sizeof(DWORD);
	if( Size > 0 )
	{
		// Create the index buffer.
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(DWORD),Size,NULL,RUF_Static);

		// Initialize the buffer.
		void* Buffer = RHILockIndexBuffer(IndexBufferRHI,0,Size);
		appMemcpy(Buffer,&Indices(0),Size);
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
}

FArchive& operator<<(FArchive& Ar,FRawIndexBuffer32& I)
{
	I.Indices.BulkSerialize( Ar );
	return Ar;
}

/*-----------------------------------------------------------------------------
FRawStaticIndexBuffer
-----------------------------------------------------------------------------*/

/**
* Create the index buffer RHI resource and initialize its data
*/
void FRawStaticIndexBuffer::InitRHI()
{
	DWORD Size = Indices.Num() * sizeof(WORD);
	if(Indices.Num())
	{
		if (bSetupForInstancing)
		{
			// Create an instanced index buffer.
			check(NumVertsPerInstance > 0);
			// Create the index buffer.
			UINT NumInstances = 0;
			IndexBufferRHI = RHICreateInstancedIndexBuffer(sizeof(WORD),Size,RUF_Static,NumInstances);
			check(NumInstances);
			// Initialize the buffer.
			WORD* Buffer = (WORD *)RHILockIndexBuffer(IndexBufferRHI,0,Size * NumInstances);
			WORD Offset = 0;
			check((NumInstances + 1) * NumVertsPerInstance < 65536);
			for (UINT Instance = 0; Instance < NumInstances; Instance++)
			{
				for (INT Index = 0; Index < Indices.Num(); Index++)
				{
					*Buffer++ = Indices(Index) + Offset;
				}
				Offset += (WORD)NumVertsPerInstance;
			}
		}
		else
		{
			// Create the index buffer.
			IndexBufferRHI = RHICreateIndexBuffer(sizeof(WORD),Size,&Indices,RUF_Static);
		}
	}    
}

/**
* Serializer for this class
* @param Ar - archive to serialize to
* @param I - data to serialize
*/
FArchive& operator<<(FArchive& Ar,FRawStaticIndexBuffer& I)
{
	I.Indices.BulkSerialize( Ar );
	if (Ar.IsLoading())
	{
		// Make sure these are set to no-instancing values
		I.NumVertsPerInstance = 0;
		I.bSetupForInstancing = FALSE;
	}
	return Ar;
}

/**
* Converts a triangle list into a triangle strip.
*/
INT FRawStaticIndexBuffer::Stripify()
{
#if !CONSOLE && !PLATFORM_UNIX
	return StripifyIndexBuffer(Indices);
#else
	return 0;
#endif
}

/**
* Orders a triangle list for better vertex cache coherency.
*/
void FRawStaticIndexBuffer::CacheOptimize()
{
#if !CONSOLE && !PLATFORM_UNIX
	CacheOptimizeIndexBuffer(Indices);
#endif
}


/*-----------------------------------------------------------------------------
	FRawGPUIndexBuffer
-----------------------------------------------------------------------------*/

/**
 *	Default constructor
 */
FRawGPUIndexBuffer::FRawGPUIndexBuffer()
:	NumIndices(0)
,	Stride(sizeof(WORD))
,	bIsDynamic(FALSE)
,	bIsEmpty(TRUE)
{
}

/**
 *	Setup constructor
 *	@param InNumIndices		- Number of indices to allocate space for
 *	@param InIsDynamic		- TRUE if the index buffer should be dynamic
 *	@param InStride			- Number of bytes per index
 */
FRawGPUIndexBuffer::FRawGPUIndexBuffer(UINT InNumIndices, UBOOL InIsDynamic/*=FALSE*/, UINT InStride/*=sizeof(WORD)*/)
:	NumIndices(InNumIndices)
,	Stride(InStride)
,	bIsDynamic(InIsDynamic)
,	bIsEmpty(TRUE)
{
}

/**
 *	Sets up the index buffer, if the default constructor was used.
 *	@param InNumIndices		- Number of indices to allocate space for
 *	@param InIsDynamic		- TRUE if the index buffer should be dynamic
 *	@param InStride			- Number of bytes per index
 */
void FRawGPUIndexBuffer::Setup(UINT InNumIndices, UBOOL InIsDynamic/*=FALSE*/, UINT InStride/*=sizeof(WORD)*/)
{
	check( bIsEmpty );
	NumIndices	= InNumIndices;
	Stride		= InStride;
	bIsDynamic	= InIsDynamic;
	bIsEmpty	= TRUE;
}

/**
 *	Renderthread API.
 *	Create the index buffer RHI resource and initialize its data.
 */
void FRawGPUIndexBuffer::InitRHI()
{
	if ( !bIsDynamic )
	{
		IndexBufferRHI = RHICreateIndexBuffer(Stride, NumIndices * sizeof(Stride), NULL, bIsDynamic ? RUF_Dynamic : RUF_Static );
		bIsEmpty = TRUE;
	}
}

/**
 *	Renderthread API.
 *	Releases a dynamic index buffer RHI resource.
 *	Called when the resource is released, or when reseting all RHI resources.
 */
void FRawGPUIndexBuffer::ReleaseRHI()
{
	if ( !bIsDynamic )
	{
		IndexBufferRHI.SafeRelease();
		bIsEmpty = TRUE;
	}
}

/**
 *	Renderthread API.
 *	Create an empty dynamic index buffer RHI resource.
 */
void FRawGPUIndexBuffer::InitDynamicRHI()
{
	if ( bIsDynamic )
	{
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(Stride), NumIndices * sizeof(Stride), NULL, bIsDynamic ? RUF_Dynamic : RUF_Static);
		bIsEmpty = TRUE;
	}
}

/**
 *	Renderthread API.
 *	Releases a dynamic index buffer RHI resource.
 *	Called when the resource is released, or when reseting all RHI resources.
 */
void FRawGPUIndexBuffer::ReleaseDynamicRHI()
{
	if ( bIsDynamic )
	{
		IndexBufferRHI.SafeRelease();
		bIsEmpty = TRUE;
	}
}

/**
 *	Renderthread API.
 *	Locks the index buffer and returns a pointer to the first index in the locked region.
 *
 *	@param FirstIndex		- First index in the locked region. Defaults to the first index in the buffer.
 *	@param NumIndices		- Number of indices to lock. Defaults to the remainder of the buffer.
 */
void* FRawGPUIndexBuffer::Lock( UINT FirstIndex, UINT InNumIndices )
{
	if ( InNumIndices == 0 )
	{
		InNumIndices = NumIndices - FirstIndex;
	}
	return RHILockIndexBuffer( IndexBufferRHI, FirstIndex * sizeof(Stride), InNumIndices * sizeof(Stride) );
}

/**
 *	Renderthread API.
 *	Unlocks the index buffer.
 */
void FRawGPUIndexBuffer::Unlock( )
{
	RHIUnlockIndexBuffer( IndexBufferRHI );
	bIsEmpty = FALSE;
}
