/*=============================================================================
FMallocProfiler.cpp: Memory profiling support.
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

#include "FMallocProfiler.h"
#include "ProfilingHelpers.h"

#if USE_MALLOC_PROFILER

/**
 * Maximum depth of stack backtrace.
 * Reducing this will sometimes truncate the amount of callstack info you get but will also reduce
 * the number of over all unique call stacks as some of the script call stacks are REALLY REALLY
 * deep and end up eating a lot of memory which will OOM you on consoles.  A good value for consoles when doing long
 * runs is 50.
 */
#define	MEMORY_PROFILER_MAX_BACKTRACE_DEPTH		75
/** Number of backtrace entries to skip											*/
#define MEMORY_PROFILER_SKIP_NUM_BACKTRACE_ENTRIES	5
/** Presizes callstack array to avoid large re-allocations failing.				*/
#define MEMORY_PROFILER_CALLSTACK_PRESIZE_COUNT		250000

/** Magic value, determining that file is a memory profiler file.				*/
#define MEMORY_PROFILER_MAGIC						0xDA15F7D8
/** Version of memory profiler.													*/
#define MEMORY_PROFILER_VERSION						2

enum EProfilingPayloadType
{
	TYPE_Malloc  = 0,
	TYPE_Free	 = 1,
	TYPE_Realloc = 2,
	TYPE_Other   = 3,
	// Don't add more than 4 values - we only have 2 bits to store this.
};

enum EProfilingPayloadSubType
{
	SUBTYPE_EndOfStreamMarker	= 0,
	SUBTYPE_EndOfFileMarker		= 1,
	SUBTYPE_SnapshotMarker		= 2,
};

/*=============================================================================
	Helpers
=============================================================================*/

/**
 * Serializes a string as ANSI char array.
 *
 * @param	String			String to serialize
 * @param	Ar				Archive to serialize with
 * @param	MinCharacters	Minimum number of characters to serialize.
 */
static void SerializeStringAsANSICharArray( const FString& String, FArchive& Ar, INT MinCharacters=0 )
{
	INT	Length = Max( String.Len(), MinCharacters );
	Ar << Length;
	for( INT CharIndex=0; CharIndex<String.Len(); CharIndex++ )
	{
		ANSICHAR AnsiChar = ToAnsi( String[CharIndex] );
		Ar << AnsiChar;
	}
	// Zero pad till minimum number of characters are written.
	for( INT i=String.Len(); i<Length; i++ )
	{
		ANSICHAR NullChar = 0;
		Ar << NullChar;
	}
}

/*=============================================================================
	Profiler header.
=============================================================================*/

struct FProfilerHeader
{
	/** Magic to ensure we're opening the right file.	*/
	DWORD	Magic;
	/** Version number to detect version mismatches.	*/
	DWORD	Version;
	/** Platform that was captured.						*/
	DWORD	Platform;
	/** Whether symbol information is being serialized. */
	DWORD	bShouldSerializeSymbolInfo;
	/** Name of executable, used for finding symbols.	*/
	FString ExecutableName;

	/** Offset in file for name table.					*/
	DWORD	NameTableOffset;
	/** Number of name table entries.					*/
	DWORD	NameTableEntries;

	/** Offset in file for callstack address table.		*/
	DWORD	CallStackAddressTableOffset;
	/** Number of callstack address entries.			*/
	DWORD	CallStackAddressTableEntries;

	/** Offset in file for callstack table.				*/
	DWORD	CallStackTableOffset;
	/** Number of callstack entries.					*/
	DWORD	CallStackTableEntries;

	/** Number of data files total.						*/
	DWORD	NumDataFiles;

	/**
	 * Serialization operator.
	 *
	 * @param	Ar			Archive to serialize to
	 * @param	Header		Header to serialize
	 * @return	Passed in archive
	 */
	friend FArchive& operator << ( FArchive& Ar, FProfilerHeader Header )
	{
		Ar	<< Header.Magic
			<< Header.Version
			<< Header.Platform
			<< Header.bShouldSerializeSymbolInfo
			<< Header.NameTableOffset
			<< Header.NameTableEntries
			<< Header.CallStackAddressTableOffset
			<< Header.CallStackAddressTableEntries
			<< Header.CallStackTableOffset
			<< Header.CallStackTableEntries
			<< Header.NumDataFiles;
		check( Ar.IsSaving() );
		SerializeStringAsANSICharArray( Header.ExecutableName, Ar, 255 );
		return Ar;
	}
};

/*=============================================================================
CallStack address information.
=============================================================================*/

/**
 * Helper structure encapsulating a single address/ point in a callstack
 */
struct FCallStackAddressInfo
{
	/** Program counter address of entry.			*/
	QWORD	ProgramCounter;
#if !CONSOLE
	/** Index into name table for filename.			*/
	INT		FilenameNameTableIndex;
	/** Index into name table for function name.	*/
	INT		FunctionNameTableIndex;
	/** Line number in file.						*/
	INT		LineNumber;
#endif

	/**
	 * Serialization operator.
	 *
	 * @param	Ar			Archive to serialize to
	 * @param	AddressInfo	AddressInfo to serialize
	 * @return	Passed in archive
	 */
	friend FArchive& operator << ( FArchive& Ar, FCallStackAddressInfo AddressInfo )
	{
		Ar	<< AddressInfo.ProgramCounter
#if !CONSOLE
			<< AddressInfo.FilenameNameTableIndex
			<< AddressInfo.FunctionNameTableIndex
			<< AddressInfo.LineNumber
#endif
			;
		return Ar;
	}
};

/**
 * Helper structure encapsulating a callstack.
 */
struct FCallStackInfo
{
	/** CRC of program counters for this callstack.				*/
	DWORD	CRC;
	/** Array of indices into callstack address info array.		*/
	INT		AddressIndices[MEMORY_PROFILER_MAX_BACKTRACE_DEPTH];

	/**
	 * Serialization operator.
	 *
	 * @param	Ar			Archive to serialize to
	 * @param	AllocInfo	Callstack info to serialize
	 * @return	Passed in archive
	 */
	friend FArchive& operator << ( FArchive& Ar, FCallStackInfo CallStackInfo )
	{
		Ar << CallStackInfo.CRC;
		// Serialize valid callstack indices.
		INT i=0;
		for( ; i<ARRAY_COUNT(CallStackInfo.AddressIndices) && CallStackInfo.AddressIndices[i]!=-1; i++ )
		{
			Ar << CallStackInfo.AddressIndices[i];
		}
		// Terminate list of address indices with -1 if we have a normal callstack.
		INT Stopper = -1;
		// And terminate with -2 if the callstack was truncated.
		if( i== ARRAY_COUNT(CallStackInfo.AddressIndices) )
		{
			Stopper = -2;
		}

		Ar << Stopper;
		return Ar;
	}
};

/*=============================================================================
	Allocation infos.
=============================================================================*/

/**
 * Relevant information for a single malloc operation.
 *
 * 20 bytes (assuming 32 bit pointers)
 */
struct FProfilerAllocInfo
{
	/** Pointer of allocation, lower two bits are used for payload type.	*/
	PTRINT Pointer;
	/** Index of callstack.													*/
	INT CallStackIndex;
	/** Size of allocation.													*/
	DWORD Size;

	/**
	 * Serialization operator.
	 *
	 * @param	Ar			Archive to serialize to
	 * @param	AllocInfo	Allocation info to serialize
	 * @return	Passed in archive
	 */
	friend FArchive& operator << ( FArchive& Ar, FProfilerAllocInfo AllocInfo )
	{
		Ar	<< AllocInfo.Pointer
			<< AllocInfo.CallStackIndex
			<< AllocInfo.Size;
		return Ar;
	}
};

/**
 * Relevant information for a single free operation.
 *
 * 8 bytes (assuming 32 bit pointers)
 */
struct FProfilerFreeInfo
{
	/** Free'd pointer, lower two bits are used for payload type.			*/
	PTRINT Pointer;
	
	/**
	 * Serialization operator.
	 *
	 * @param	Ar			Archive to serialize to
	 * @param	FreeInfo	Free info to serialize
	 * @return	Passed in archive
	 */
	friend FArchive& operator << ( FArchive& Ar, FProfilerFreeInfo FreeInfo )
	{
		Ar	<< FreeInfo.Pointer;
		return Ar;
	}
};

/**
 * Relevant information for a single realloc operation.
 *
 * 24 bytes (assuming 32 bit pointers)
 */
struct FProfilerReallocInfo
{
	/** Old pointer, lower two bits are used for payload type.				*/
	PTRINT OldPointer;
	/** New pointer, might be indentical to old.							*/
	PTRINT NewPointer;
	/** Index of callstack.													*/
	INT CallStackIndex;
	/** Size of allocation.													*/
	DWORD Size;

	/**
	 * Serialization operator.
	 *
	 * @param	Ar			Archive to serialize to
	 * @param	ReallocInfo	Realloc info to serialize
	 * @return	Passed in archive
	 */
	friend FArchive& operator << ( FArchive& Ar, FProfilerReallocInfo ReallocInfo )
	{
		Ar	<< ReallocInfo.OldPointer
			<< ReallocInfo.NewPointer
			<< ReallocInfo.CallStackIndex
			<< ReallocInfo.Size;
		return Ar;
	}
};

/**
 * Helper structure for misc data like e.g. end of stream marker.
 *
 * 12 bytes (assuming 32 bit pointers)
 */
struct FProfilerOtherInfo
{
	/** Dummy pointer as all tokens start with a pointer (TYPE_Other)		*/
	PTRINT	DummyPointer;
	/** Subtype.															*/
	INT		SubType;
	/** Subtype specific payload.											*/
	DWORD	Payload;

	/**
	 * Serialization operator.
	 *
	 * @param	Ar			Archive to serialize to
	 * @param	OtherInfo	Info to serialize
 	 * @return	Passed in archive
 	 */
	friend FArchive& operator << ( FArchive& Ar, FProfilerOtherInfo OtherInfo )
	{
		Ar	<< OtherInfo.DummyPointer
			<< OtherInfo.SubType
			<< OtherInfo.Payload;
		return Ar;
	}
};

/*=============================================================================
	FMallocProfiler implementation.
=============================================================================*/


/**
 * Constructor, initializing all member variables and potentially loading symbols.
 *
 * @param	InMalloc	The allocator wrapped by FMallocProfiler that will actually do the allocs/deallocs.
 */
FMallocProfiler::FMallocProfiler(FMalloc* InMalloc)
:	UsedMalloc( InMalloc )
// Default to FALSE as we might need to do a few allocations here.
,	bShouldTrackOperations( FALSE )
,   bEndProfilingHasBeenCalled( FALSE )
{
#if LOAD_SYMBOLS_FOR_STACK_WALKING
	// Initialize symbols.
	appInitStackWalking();
#endif
	StartTime = appSeconds();

	// Serialize dummy header, overwritten in EndProfiling.
	FProfilerHeader DummyHeader;
	appMemzero( &DummyHeader, sizeof(DummyHeader) );
	BufferedFileWriter << DummyHeader;

	// Start tracking now that we're initialized.
	bShouldTrackOperations = TRUE;
}

/**
 * Tracks malloc operation.
 *
 * @param	Ptr		Allocated pointer 
 * @param	Size	Size of allocated pointer
 */
void FMallocProfiler::TrackMalloc( void* Ptr, DWORD Size )
{	
	// Avoid tracking operations caused by tracking!
	if( bShouldTrackOperations )
	{
		// Disable allocation tracking as we're using malloc & co internally.
		bShouldTrackOperations = FALSE;

		// Gather information about operation.
		FProfilerAllocInfo AllocInfo;
		AllocInfo.Pointer			= (PTRINT) Ptr | TYPE_Malloc;
		AllocInfo.CallStackIndex	= GetCallStackIndex();
		AllocInfo.Size				= Size;

		// Serialize to HDD.
		BufferedFileWriter << AllocInfo;

		// Re-enable allocation tracking again.
		PossiblySplitMprof();
		bShouldTrackOperations = TRUE;
	}
}


/**
 * Tracks free operation
 *
 * @param	Ptr		Freed pointer
 */
void FMallocProfiler::TrackFree( void* Ptr )
{
	// Avoid tracking operations caused by tracking!
	if( bShouldTrackOperations )
	{
		// Disable allocation tracking as we're using malloc & co internally.
		bShouldTrackOperations = FALSE;

		// Gather information about operation.
		FProfilerFreeInfo FreeInfo;
		FreeInfo.Pointer = (PTRINT) Ptr | TYPE_Free;

		// Serialize to HDD.
		BufferedFileWriter << FreeInfo;

		// Re-enable allocation tracking again.
		PossiblySplitMprof();
		bShouldTrackOperations = TRUE;
	}
}

/**
 * Tracks realloc operation
 *
 * @param	OldPtr	Previous pointer
 * @param	NewPtr	New pointer
 * @param	NewSize	New size of allocation
 */
void FMallocProfiler::TrackRealloc( void* OldPtr, void* NewPtr, DWORD NewSize )
{
	// Avoid tracking operations caused by tracking!
	if( bShouldTrackOperations )
	{
		// Disable allocation tracking as we're using malloc & co internally.
		bShouldTrackOperations = FALSE;

		// Gather information about operation.
		FProfilerReallocInfo ReallocInfo;
		ReallocInfo.OldPointer		= (PTRINT) OldPtr | TYPE_Realloc;
		ReallocInfo.NewPointer		= (PTRINT) NewPtr;
		ReallocInfo.CallStackIndex	= GetCallStackIndex();
		ReallocInfo.Size			= NewSize;

		// Serialize to HDD.
		BufferedFileWriter << ReallocInfo;

		// Re-enable allocation tracking again.
		PossiblySplitMprof();
		bShouldTrackOperations = TRUE;
	}
}

/**
 * Ends profiling operation and closes file.
 */
void FMallocProfiler::EndProfiling()
{
	bEndProfilingHasBeenCalled = TRUE;

	// Stop all further tracking operations. This cannot be re-enabled as we close the file writer at the end.
	bShouldTrackOperations = FALSE;

	// Write end of stream marker.
	FProfilerOtherInfo EndOfStream;
	EndOfStream.DummyPointer	= TYPE_Other;
	EndOfStream.SubType			= SUBTYPE_EndOfStreamMarker;
	EndOfStream.Payload			= 0;
	BufferedFileWriter << EndOfStream;

	debugf(TEXT("FMallocProfiler: dumping file [%s].mprof"),*BufferedFileWriter.FullFilepath.GetBaseFilename());

#if !CONSOLE
	// Look up symbols on platforms supporting it at runtime.
	for( INT AddressIndex=0; AddressIndex<CallStackAddressInfoArray.Num(); AddressIndex++ )
	{
		FCallStackAddressInfo&	AddressInfo = CallStackAddressInfoArray(AddressIndex);
		// Look up symbols.
		const FProgramCounterSymbolInfo SymbolInfo	= appProgramCounterToSymbolInfo( AddressInfo.ProgramCounter );

		// Convert to strings, and clean up in the process.
		const FString ModulName		= FFilename(SymbolInfo.ModuleName).GetCleanFilename();
		const FString FileName		= FString(SymbolInfo.Filename);
		const FString FunctionName	= FString(SymbolInfo.FunctionName);

		// Propagate to our own struct, also populating name table.
		AddressInfo.FilenameNameTableIndex	= GetNameTableIndex( FileName );
		AddressInfo.FunctionNameTableIndex	= GetNameTableIndex( FunctionName );
		AddressInfo.LineNumber				= SymbolInfo.LineNumber;
	}
#endif // !CONSOLE

	// Archive used to write out symbol information. This will always be written to the first file, which
	// in the case of multiple files won't be pointed to by BufferedFileWriter.
	FArchive* SymbolFileWriter = NULL;
	// Use the BufferedFileWriter if we're still on the first file.
	if( BufferedFileWriter.FileNumber == 0 )
	{
		SymbolFileWriter = &BufferedFileWriter;
	}
	// Create a new file writer for first data file.
	else
	{
		// Create a file writer appending to the first file.
		FString Filename = FFilename(BufferedFileWriter.FullFilepath).GetPath() + FFilename(BufferedFileWriter.FullFilepath).GetBaseFilename() + TEXT(".mprof");
		SymbolFileWriter = GFileManager->CreateFileWriter( *Filename, FILEWRITE_Append );
		// Seek to the end of the first file.
		SymbolFileWriter->Seek( SymbolFileWriter->Tell() );
	}

	// Real header, written at start of the file but written out right before we close the file.
	FProfilerHeader Header;
	Header.Magic				= MEMORY_PROFILER_MAGIC;
	Header.Version				= MEMORY_PROFILER_VERSION;
	Header.Platform				= appGetPlatformType();
	Header.bShouldSerializeSymbolInfo = CONSOLE ? 0 : 1;
#if XBOX
	Header.ExecutableName		= appExecutableName();
#endif
	Header.NumDataFiles			= BufferedFileWriter.FileNumber + 1;

	// Write out name table and update header with offset and count.
	Header.NameTableOffset	= SymbolFileWriter->Tell();
	Header.NameTableEntries	= NameArray.Num();
	for( INT NameIndex=0; NameIndex<NameArray.Num(); NameIndex++ )
	{
		SerializeStringAsANSICharArray( NameArray(NameIndex), *SymbolFileWriter );
	}

	// Write out callstack address infos and update header with offset and count.
	Header.CallStackAddressTableOffset	= SymbolFileWriter->Tell();
	Header.CallStackAddressTableEntries	= CallStackAddressInfoArray.Num();
	for( INT CallStackAddressIndex=0; CallStackAddressIndex<CallStackAddressInfoArray.Num(); CallStackAddressIndex++ )
	{
		(*SymbolFileWriter) << CallStackAddressInfoArray(CallStackAddressIndex);
	}

	// Write out callstack infos and update header with offset and count.
	Header.CallStackTableOffset			= SymbolFileWriter->Tell();
	Header.CallStackTableEntries		= CallStackInfoArray.Num();
	for( INT CallStackIndex=0; CallStackIndex<CallStackInfoArray.Num(); CallStackIndex++ )
	{
		(*SymbolFileWriter) << CallStackInfoArray(CallStackIndex);
	}

	// Seek to the beginning of the file and write out proper header.
	SymbolFileWriter->Seek( 0 );
	(*SymbolFileWriter) << Header;

	// Close file writers.
	SymbolFileWriter->Close();
	if( SymbolFileWriter != &BufferedFileWriter )
	{
		BufferedFileWriter.Close();
	}

	

	warnf(TEXT("FMallocProfiler: done writing file [%s]"), *(BufferedFileWriter.FullFilepath) );

	SendDataToPCViaUnrealConsole( TEXT("UE_PROFILER!MEMORY:"), *(BufferedFileWriter.FullFilepath) );

}

/**
 * Returns index of passed in program counter into program counter array. If not found, adds it.
 *
 * @param	ProgramCounter	Program counter to find index for
 * @return	Index of passed in program counter if it's not NULL, INDEX_NONE otherwise
 */
INT FMallocProfiler::GetProgramCounterIndex( QWORD ProgramCounter )
{
	INT	Index = INDEX_NONE;

	// Look up index in unique array of program counter infos, if we have a valid program counter.
	if( ProgramCounter )
	{
		// Use index if found.
		INT* IndexPtr = ProgramCounterToIndexMap.Find( ProgramCounter );
		if( IndexPtr )
		{
			Index = *IndexPtr;
		}
		// Encountered new program counter, add to array and set index mapping.
		else
		{
			// Add to aray and set mapping for future use.
			Index = CallStackAddressInfoArray.AddZeroed();
			ProgramCounterToIndexMap.Set( ProgramCounter, Index );

			// Only initialize program counter, rest will be filled in at the end, once symbols are loaded.
			FCallStackAddressInfo& CallStackAddressInfo = CallStackAddressInfoArray(Index);
			CallStackAddressInfo.ProgramCounter = ProgramCounter;
		}
		check(Index!=INDEX_NONE);
	}

	return Index;
}

/** 
 * Returns index of callstack, captured by this function into array of callstack infos. If not found, adds it.
 *
 * @return index into callstack array
 */
INT FMallocProfiler::GetCallStackIndex()
{
	// Index of callstack in callstack info array.
	INT Index = INDEX_NONE;

	// Capture callstack and create CRC.
	QWORD FullBackTrace[MEMORY_PROFILER_MAX_BACKTRACE_DEPTH + MEMORY_PROFILER_SKIP_NUM_BACKTRACE_ENTRIES];
	appCaptureStackBackTrace( FullBackTrace, MEMORY_PROFILER_MAX_BACKTRACE_DEPTH + MEMORY_PROFILER_SKIP_NUM_BACKTRACE_ENTRIES );
	// Skip first 5 entries as they are inside the allocator.
	QWORD* BackTrace = &FullBackTrace[MEMORY_PROFILER_SKIP_NUM_BACKTRACE_ENTRIES];
	DWORD CRC = appMemCrc( BackTrace, MEMORY_PROFILER_MAX_BACKTRACE_DEPTH * sizeof(QWORD), 0 );

	// Presize callstack info array to avoid large re-allocations failing due to fragmentation.
	if( CallStackInfoArray.Num() == 0 )
	{
		CallStackInfoArray.Empty( MEMORY_PROFILER_CALLSTACK_PRESIZE_COUNT );
	}

	// Use index if found
	INT* IndexPtr = CRCToCallStackIndexMap.Find( CRC );
	if( IndexPtr )
	{
		Index = *IndexPtr;
	}
	// Encountered new call stack, add to array and set index mapping.
	else
	{
		// Add to array and set mapping for future use.
		Index = CallStackInfoArray.Add();
		CRCToCallStackIndexMap.Set( CRC, Index );

		// Set up callstack info with captured call stack, translating program counters
		// to indices in program counter array (unique entries).
		FCallStackInfo& CallStackInfo = CallStackInfoArray(Index);
		CallStackInfo.CRC = CRC;
		for( INT i=0; i<MEMORY_PROFILER_MAX_BACKTRACE_DEPTH; i++ )
		{
			CallStackInfo.AddressIndices[i] = GetProgramCounterIndex( BackTrace[i] );
		}
	}

	check(Index!=INDEX_NONE);
	return Index;
}	

/**
 * Returns index of passed in name into name array. If not found, adds it.
 *
 * @param	Name	Name to find index for
 * @return	Index of passed in name
 */
INT FMallocProfiler::GetNameTableIndex( const FString& Name )
{
	// Index of name in name table.
	INT Index = INDEX_NONE;

	// Use index if found.
	INT* IndexPtr = NameToNameTableIndexMap.Find( Name );
	if( IndexPtr )
	{
		Index = *IndexPtr;
	}
	// Encountered new name, add to array and set index mapping.
	else
	{
		Index = NameArray.Num();
		new(NameArray)FString(Name);
		NameToNameTableIndexMap.Set(*Name,Index);
	}

	check(Index!=INDEX_NONE);
	return Index;
}

/**
 * Exec handler. Parses command and returns TRUE if handled.
 *
 * @param	Cmd		Command to parse
 * @param	Ar		Output device to use for logging
 * @return	TRUE if handled, FALSE otherwise
 */
UBOOL FMallocProfiler::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	// End profiling.
    if( ParseCommand(&Cmd,TEXT("DUMPALLOCSTOFILE")) )
	{
		// once EndProfiling() been called then most operations are no longer valid
		if( bEndProfilingHasBeenCalled == TRUE )
		{
			warnf(TEXT("FMallocProfiler: EndProfiling() has been called further actions will not be recorded please restart memory tracking process"));
			return TRUE;
		}

		warnf(TEXT("FMallocProfiler: DUMPALLOCSTOFILE"));
		EndProfiling();
		return TRUE;
	}
	else if( ParseCommand(&Cmd,TEXT("SNAPSHOTMEMORY")) )
	{
		// once EndProfiling() been called then most operations are no longer valid
		if( bEndProfilingHasBeenCalled == TRUE )
		{
			warnf(TEXT("FMallocProfiler: EndProfiling() has been called further actions will not be recorded please restart memory tracking process"));
			return TRUE;
		}

		Ar.Logf(TEXT("FMallocProfiler: SNAPSHOTMEMORY"));
		SnapshotMemory();
		return TRUE;
	}
	return UsedMalloc->Exec(Cmd, Ar);
}

/**
 * Embeds token into stream to snapshot memory at this point.
 */
void FMallocProfiler::SnapshotMemory()
{
		// Write snapshot marker to stream.
		FProfilerOtherInfo SnapshotMarker;
		SnapshotMarker.DummyPointer	= TYPE_Other;
		SnapshotMarker.SubType		= SUBTYPE_SnapshotMarker;
		SnapshotMarker.Payload		= 0;
		BufferedFileWriter << SnapshotMarker;
	}

/** 
 * Checks whether file is too big and will create new files with different extension but same base name.
 */
void FMallocProfiler::PossiblySplitMprof()
{
	// Nothing to do if we haven't created a file write yet. This happens at startup as GFileManager is initialized after
	// quite a few allocations.
	if( !BufferedFileWriter.IsBufferingToMemory() )
	{
		const INT CurrentSize = BufferedFileWriter.Tell();

		// Create a new file if current one exceeds 1 GByte.
		#define SIZE_OF_DATA_FILE 1.9 * 1024 * 1024 * 1024
		if( CurrentSize > SIZE_OF_DATA_FILE ) 
		{
			warnf( TEXT("Ran out of Space in the mprof dumping now!") );
			EndProfiling();

			// this doesn't currently work correctly so we are just going to EndProfiling()
			//FProfilerOtherInfo EndOfFile;
			//EndOfFile.DummyPointer	= TYPE_Other;
			//EndOfFile.SubType		= SUBTYPE_EndOfFileMarker;
			//EndOfFile.Payload		= 0;
			//BufferedFileWriter << EndOfFile;
            //
			//// Split the file and create a new one.
			//BufferedFileWriter.Split();
		}
	}
}




/*=============================================================================
	FMallocProfilerBufferedFileWriter implementation
=============================================================================*/

/**
 * Constructor. Called before GMalloc is initialized!!!
 */
FMallocProfilerBufferedFileWriter::FMallocProfilerBufferedFileWriter()
:	FileWriter( NULL )
,	FileNumber( 0 )
{
	ArIsSaving		= TRUE;
	ArIsPersistent	= TRUE;
}

/**
 * Destructor, cleaning up FileWriter.
 */
FMallocProfilerBufferedFileWriter::~FMallocProfilerBufferedFileWriter()
{
	if( FileWriter )
	{
		delete FileWriter;
		FileWriter = NULL;
	}
}

/**
 * Whether we are currently buffering to memory or directly writing to file.
 *
 * @return	TRUE if buffering to memory, FALSE otherwise
 */
UBOOL FMallocProfilerBufferedFileWriter::IsBufferingToMemory()
{
	return FileWriter == NULL;
}

/**
 * Splits the file and creates a new one with different extension.
 */
void FMallocProfilerBufferedFileWriter::Split()
{
	// Increase file number.
	FileNumber++;
	// Delete existing file writer. A new one will be automatically generated
	// the first time further data is being serialized.
	if( FileWriter )
	{
		delete FileWriter;
		FileWriter = NULL;
	}
}

// FArchive interface.

void FMallocProfilerBufferedFileWriter::Serialize( void* V, INT Length )
{
#if ALLOW_DEBUG_FILES
	// Copy to buffered memory array if file manager hasn't been set up yet.
	if( GFileManager == NULL )
	{
		const INT Index = BufferedData.Add( Length );
		appMemcpy( &BufferedData(Index), V, Length );
	} 
	// File manager is set up but we haven't created file writer yet, do it now.
	else if( FileWriter == NULL )
	{
		const FString SysTime = appSystemTimeString();
		//const FString PathName = *(appProfilingDir() + TEXT("Memory") PATH_SEPARATOR );
		const FString PathName = appProfilingDir() + GGameName + TEXT("-") + SysTime + PATH_SEPARATOR + GGameName + TEXT("-") + SysTime;
		// Create file writer to serialize data to HDD.
		if( FullFilepath.GetBaseFilename() == TEXT("") )
		{
			// Use .mprof extension for first file.
			if( FileNumber == 0 )
			{
				FullFilepath = PathName + TEXT(".mprof");//CreateProfileFilename( TEXT(".mprof") ); 
			}
			// Use .mX extension for subsequent files.
			else
			{
				FullFilepath = PathName + FString::Printf(TEXT(".m%i"),FileNumber); // CreateProfileFilename( FString::Printf(TEXT(".m%i"),FileNumber) ); 
			}
		}

		GFileManager->MakeDirectory( *PathName );
		FileWriter = GFileManager->CreateFileWriter( *FullFilepath, FILEWRITE_NoFail );
		checkf( FileWriter );

		// Serialize existing buffered data and empty array.
		FileWriter->Serialize( BufferedData.GetData(), BufferedData.Num() );
		BufferedData.Empty();
	}

	// Serialize data to HDD via FileWriter if it already has been created.
	if( FileWriter )
	{
		FileWriter->Serialize( V, Length );
	}
#endif
}

void FMallocProfilerBufferedFileWriter::Seek( INT InPos )
{
	check( FileWriter );
	FileWriter->Seek( InPos );
}

UBOOL FMallocProfilerBufferedFileWriter::Close()
{
	check( FileWriter );
	return FileWriter->Close();
}

INT FMallocProfilerBufferedFileWriter::Tell()
{
	check( FileWriter );
	return FileWriter->Tell();
}

#else

// Suppress linker warning "warning LNK4221: no public symbols found; archive member will be inaccessible"
INT FMallocProfilerLinkerHelper;

#endif //USE_MALLOC_PROFILER
