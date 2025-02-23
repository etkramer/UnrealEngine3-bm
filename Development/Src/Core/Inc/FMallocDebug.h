/*=============================================================================
	FMallocDebug.h: Debug memory allocator.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __FMALLOCDEBUG_H__
#define __FMALLOCDEBUG_H__

extern FRunnableThread* GRenderingThread;

// Debug memory allocator.
class FMallocDebug : public FMalloc
{
	// Tags.
	enum {MEM_PreTag =0xf0ed1cee};
	enum {MEM_PostTag=0xdeadf00f};
	enum {MEM_Tag    =0xfe      };
	enum {MEM_WipeTag=0xcd      };

	// Alignment.
	enum {ALLOCATION_ALIGNMENT=16};

	// Number of block sizes to collate (in steps of 4 bytes)
	enum {MEM_SizeMax = 128};
	enum {MEM_Recent = 5000};
	enum {MEM_AgeMax = 80};
	enum {MEM_AgeSlice = 100};

private:
	// Structure for memory debugging.
	struct FMemDebug
	{
		SIZE_T		Size;
		INT			RefCount;
		INT*		PreTag;
		FMemDebug*	Next;
		FMemDebug**	PrevLink;
	};

	// Variables.
	FMemDebug*	GFirstDebug;
	/** Total size of allocations */
	SIZE_T		TotalAllocationSize;
	DWORD		ThreadBeingTracked;

public:
	// FMalloc interface.
	FMallocDebug()
	:	GFirstDebug( NULL )
	,	TotalAllocationSize( 0 )
	,	ThreadBeingTracked( 0 )
	{}

	virtual void* Malloc( DWORD Size, DWORD Alignment )
	{
		check(Alignment == DEFAULT_ALIGNMENT && "Alignment currently unsupported in this allocator");
		FMemDebug* Ptr = NULL;
		Ptr = (FMemDebug*)malloc( sizeof(FMemDebug) + sizeof(FMemDebug*) + sizeof(INT) + ALLOCATION_ALIGNMENT + Size + sizeof(INT) );
		check(Ptr);
		BYTE* AlignedPtr = Align( (BYTE*) Ptr + sizeof(FMemDebug) + sizeof(FMemDebug*) + sizeof(INT), ALLOCATION_ALIGNMENT );
		Ptr->RefCount = 1;
		if( ThreadBeingTracked && (appGetCurrentThreadId() == ThreadBeingTracked) )
		{
			INC_DWORD_STAT_BY( STAT_GameToRendererMalloc, Size );
			INC_DWORD_STAT_BY( STAT_GameToRendererNet, Size );
		}

		Ptr->Size = Size;
		Ptr->Next = GFirstDebug;
		Ptr->PrevLink = &GFirstDebug;
		Ptr->PreTag = (INT*) (AlignedPtr - sizeof(INT));
		*Ptr->PreTag = MEM_PreTag;
		*((FMemDebug**)(AlignedPtr - sizeof(INT) - sizeof(FMemDebug*)))	= Ptr;
		*(INT*)(AlignedPtr+Size) = MEM_PostTag;
		appMemset( AlignedPtr, MEM_Tag, Size );
		if( GFirstDebug )
		{
			check(GIsCriticalError||GFirstDebug->PrevLink==&GFirstDebug);
			GFirstDebug->PrevLink = &Ptr->Next;
		}
		GFirstDebug = Ptr;
		TotalAllocationSize += Size;
		check(!(PTRINT(AlignedPtr) & ((PTRINT)0xf)));
		return AlignedPtr;
	}

	virtual void* Realloc( void* InPtr, DWORD NewSize, DWORD Alignment )
	{
		check(Alignment == DEFAULT_ALIGNMENT && "Alignment currently unsupported in this allocator");
		if( InPtr && NewSize )
		{
			FMemDebug* Ptr = *((FMemDebug**)((BYTE*)InPtr - sizeof(INT) - sizeof(FMemDebug*)));
			check(GIsCriticalError||(Ptr->RefCount==1));
			void* Result = Malloc( NewSize, Alignment );
			appMemcpy( Result, InPtr, Min<SIZE_T>(Ptr->Size,NewSize) );
			Free( InPtr );
			return Result;
		}
		else if( InPtr == NULL )
		{
			return Malloc( NewSize, Alignment );
		}
		else
		{
			Free( InPtr );
			return NULL;
		}
	}

	virtual void Free( void* InPtr )
	{
		if( !InPtr )
		{
			return;
		}

		FMemDebug* Ptr = *((FMemDebug**)((BYTE*)InPtr - sizeof(INT) - sizeof(FMemDebug*)));

#if BATMAN
		// Not clear why this happens sometimes on shutdown. But we probably don't care if we're shutting down anyway.
		if ( !GIsCriticalError && (UINT)Ptr == 0xdddddddd )
		{
			return;
		}
#endif

		check(GIsCriticalError||Ptr->RefCount==1);
		check(GIsCriticalError||*Ptr->PreTag==MEM_PreTag);
		check(GIsCriticalError||*(INT*)((BYTE*)InPtr+Ptr->Size)==MEM_PostTag);
	
		if( ThreadBeingTracked && (appGetCurrentThreadId() == ThreadBeingTracked) )
		{
			INC_DWORD_STAT_BY( STAT_GameToRendererFree, Ptr->Size );
			DEC_DWORD_STAT_BY( STAT_GameToRendererNet, Ptr->Size );
		}

		TotalAllocationSize -= Ptr->Size;
		appMemset( InPtr, MEM_WipeTag, Ptr->Size );	
		Ptr->Size = 0;
		Ptr->RefCount = 0;

		check(GIsCriticalError||Ptr->PrevLink);
		check(GIsCriticalError||*Ptr->PrevLink==Ptr);
		*Ptr->PrevLink = Ptr->Next;
		if( Ptr->Next )
		{
			Ptr->Next->PrevLink = Ptr->PrevLink;
		}

		free( Ptr );
	}
	/**
	 * Gathers memory allocations for both virtual and physical allocations.
	 *
	 * @param Virtual	[out] size of virtual allocations
	 * @param Physical	[out] size of physical allocations	
	 */
	virtual void GetAllocationInfo( SIZE_T& Virtual, SIZE_T& Physical )
	{
		Virtual		= TotalAllocationSize;
		Physical	= 0;
	}

	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar ) 
	{
#if !SHIPPING_PC_GAME
		if( ParseCommand(&Cmd,TEXT("HEAPCHECK")) )
		{
			HeapCheck();
			return TRUE;
		}
		else
#endif
		if( ParseCommand(&Cmd,TEXT("DUMPALLOCS")) )
		{
			DumpAllocs( Ar );
			return TRUE;
		}
		else if( ParseCommand(&Cmd,TEXT("BEGINTRACKINGTHREAD")) )
		{
			ThreadBeingTracked = appGetCurrentThreadId();
			return TRUE;
		}
		else if( ParseCommand(&Cmd,TEXT("ENDTRACKINGTHREAD")) )
		{
			ThreadBeingTracked = 0;
			return TRUE;
		}
		return FALSE;
	}

	// Internal helpers.

	void DumpAllocs( FOutputDevice& Ar )
	{
		INT Count = 0;
		INT Chunks = 0;

		Ar.Logf( TEXT( "" ) );
		Ar.Logf( TEXT( "Unfreed memory:" ) );
		for( FMemDebug* Ptr = GFirstDebug; Ptr; Ptr = Ptr->Next )
		{
			Count += Ptr->Size;
			Chunks++;
		}

		Ar.Logf( TEXT( "End of list: %i Bytes still allocated" ), Count );
		Ar.Logf( TEXT( "             %i Chunks allocated" ), Chunks );
	}

	void HeapCheck()
	{
		for( FMemDebug** Link = &GFirstDebug; *Link; Link=&(*Link)->Next )
		{
			check(GIsCriticalError||*(*Link)->PrevLink==*Link);
		}
#if (defined _MSC_VER)
		INT Result = _heapchk();
		check(Result!=_HEAPBADBEGIN);
		check(Result!=_HEAPBADNODE);
		check(Result!=_HEAPBADPTR);
		check(Result!=_HEAPEMPTY);
		check(Result==_HEAPOK);
#endif
	}
};

#endif
