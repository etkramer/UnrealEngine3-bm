/*=============================================================================
	UnName.cpp: Unreal global name code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

/*-----------------------------------------------------------------------------
	FName helpers.
-----------------------------------------------------------------------------*/

/**
* Helper function that can be used inside the debuggers watch window. E.g. "DebugFName(Class->Name.Index)". 
*
* @param	Index	Name index to look up string for
* @return			Associated name
*/
const TCHAR* DebugFName(INT Index)
{
	// Hardcoded static array. This function is only used inside the debugger so it should be fine to return it.
	static TCHAR TempName[256];
	appStrcpy(TempName, *FName::SafeString((EName)Index));
	return TempName;
}

/**
* Helper function that can be used inside the debuggers watch window. E.g. "DebugFName(Class->Name.Index, Class->Name.Number)". 
*
* @param	Index	Name index to look up string for
* @param	Number	Internal instance number of the FName to print (which is 1 more than the printed number)
* @return			Associated name
*/
const TCHAR* DebugFName(INT Index, INT Number)
{
	// Hardcoded static array. This function is only used inside the debugger so it should be fine to return it.
	static TCHAR TempName[256];
	appStrcpy(TempName, *FName::SafeString((EName)Index, Number));
	return TempName;
}

/**
 * Helper function that can be used inside the debuggers watch window. E.g. "DebugFName(Class->Name)". 
 *
 * @param	Name	Name to look up string for
 * @return			Associated name
 */
const TCHAR* DebugFName(FName& Name)
{
	// Hardcoded static array. This function is only used inside the debugger so it should be fine to return it.
	static TCHAR TempName[256];
	appStrcpy(TempName, *FName::SafeString((EName)Name.Index, Name.Number));
	return TempName;
}

/**
 * Helper function that can be used inside the debuggers watch window. E.g. "DebugFName(Class)". 
 *
 * @param	Object	Object to look up the name for 
 * @return			Associated name
 */
const TCHAR* DebugFName(UObject* Object)
{
	// Hardcoded static array. This function is only used inside the debugger so it should be fine to return it.
	static TCHAR TempName[256];
	appStrcpy(TempName,Object ? *FName::SafeString((EName)Object->Name.Index, Object->Name.Number) : TEXT("NULL"));
	return TempName;
}

/**
 * Helper function that can be used inside the debuggers watch window. E.g. "DebugFName(Object)". 
 *
 * @param	Object	Object to look up the name for 
 * @return			Fully qualified path name
 */
const TCHAR* DebugPathName(UObject* Object)
{
	if( Object )
	{
		// Hardcoded static array. This function is only used inside the debugger so it should be fine to return it.
		static TCHAR PathName[1024];
		PathName[0] = 0;

		// Keep track of how many outers we have as we need to print them in inverse order.
		UObject*	TempObject = Object;
		INT			OuterCount = 0;
		while( TempObject )
		{
			TempObject = TempObject->GetOuter();
			OuterCount++;
		}

		// Iterate over each outer + self in reverse oder and append name.
		for( INT OuterIndex=OuterCount-1; OuterIndex>=0; OuterIndex-- )
		{
			// Move to outer name.
			TempObject = Object;
			for( INT i=0; i<OuterIndex; i++ )
			{
				TempObject = TempObject->GetOuter();
			}

			// Dot separate entries.
			if( OuterIndex != OuterCount -1 )
			{
				appStrcat( PathName, TEXT(".") );
			}
			// And app end the name.
			appStrcat( PathName, DebugFName( TempObject ) );
		}

		return PathName;
	}
	else
	{
		return TEXT("None");
	}
}

/**
 * Helper function that can be used inside the debuggers watch window. E.g. "DebugFName(Object)". 
 *
 * @param	Object	Object to look up the name for 
 * @return			Fully qualified path name prepended by class name
 */
const TCHAR* DebugFullName(UObject* Object)
{
	if( Object )
	{
		// Hardcoded static array. This function is only used inside the debugger so it should be fine to return it.
    	static TCHAR FullName[1024];
		FullName[0]=0;

		// Class Full.Path.Name
		appStrcat( FullName, DebugFName(Object->GetClass()) );
		appStrcat( FullName, TEXT(" "));
		appStrcat( FullName, DebugPathName(Object) );

		return FullName;
	}
	else
	{
		return TEXT("None");
	}
}


/*-----------------------------------------------------------------------------
	FNameEntry
-----------------------------------------------------------------------------*/


/**
 * @return FString of name portion minus number.
 */
FString FNameEntry::GetNameString() const
{
	if( IsUnicode() )
	{
		return FString(UniName);
	}
	else
	{
		return FString(AnsiName);
	}
}

/**
 * Appends this name entry to the passed in string.
 *
 * @param	String	String to append this name to
 */
void FNameEntry::AppendNameToString( FString& String ) const
{
	if( IsUnicode() )
	{
		String += UniName;
	}
	else
	{
		String += AnsiName;
	}
}

/**
 * @return case insensitive hash of name
 */
DWORD FNameEntry::GetNameHash() const
{
	if( IsUnicode() )
	{
		return appStrihash(UniName);
	}
	else
	{
		return appStrihash(AnsiName);
	}
}

/**
 * @return length of name
 */
INT FNameEntry::GetNameLength() const
{
	if( IsUnicode() )
	{
		return appStrlen( UniName );
	}
	else
	{
		return appStrlen( AnsiName );
	}
}

/**
 * Compares name without looking at case.
 *
 * @param	InName	Name to compare to
 * @return	TRUE if equal, FALSE otherwise
 */
UBOOL FNameEntry::IsEqual( const ANSICHAR* InName ) const
{
	if( IsUnicode() )
	{
		// Matching unicodeness means strings are not equal.
		return FALSE;
	}
	else
	{
		return appStricmp( AnsiName, InName ) == 0;
	}
}

/**
 * Compares name without looking at case.
 *
 * @param	InName	Name to compare to
 * @return	TRUE if equal, FALSE otherwise
 */
UBOOL FNameEntry::IsEqual( const UNICHAR* InName ) const
{
	if( !IsUnicode() )
	{
		// Matching unicodeness means strings are not equal.
		return FALSE;
	}
	else
	{
		return appStricmp( UniName, InName ) == 0;
	}
}

/**
 * Returns the size in bytes for FNameEntry structure. This is != sizeof(FNameEntry) as we only allocated as needed.
 *
 * @param	Name	Name to determine size for
 * @return	required size of FNameEntry structure to hold this string (might be unicode or ansi)
 */
INT FNameEntry::GetSize( const TCHAR* Name )
{
	return FNameEntry::GetSize( appStrlen( Name ), appIsPureAnsi( Name ) );
}

/**
 * Returns the size in bytes for FNameEntry structure. This is != sizeof(FNameEntry) as we only allocated as needed.
 *
 * @param	Length			Length of name
 * @param	bIsPureAnsi		Whether name is pure ANSI or not
 * @return	required size of FNameEntry structure to hold this string (might be unicode or ansi)
 */
INT FNameEntry::GetSize( INT Length, UBOOL bIsPureAnsi )
{
	// Calculate base size without union array.
	INT Size = sizeof(FNameEntry) - NAME_SIZE * sizeof(UNICHAR);
	// Add size required for string.
	Size += (Length+1) * (bIsPureAnsi ? sizeof(ANSICHAR) : sizeof(UNICHAR));
	return Size;
}

/**
 * Used to safely check whether any of the passed in flags are set. This is required
 * as EObjectFlags currently is a 64 bit data type and UBOOL is a 32 bit data type so
 * simply using GetFlags() & RF_MyFlagBiggerThanMaxInt won't work correctly when 
 * assigned directly to an UBOOL.
 *
 * @param	FlagsToCheck	Object flags to check for.
 * @return					TRUE if any of the passed in flags are set, FALSE otherwise  (including no flags passed in).
 */
UBOOL FNameEntry::HasAnyFlags( EObjectFlags FlagsToCheck ) const
{
#if SUPPORT_NAME_FLAGS
	return (Flags & FlagsToCheck) != 0;
#else
	appErrorf(TEXT("Set SUPPORT_NAME_FLAGS to 1 in UnBuild.h if name flags are needed."));
	return 0;
#endif
}
/**
 * Sets the passed in flags on the name entry.
 *
 * @param	FlagsToSet		Flags to set.
 */
void FNameEntry::SetFlags( EObjectFlags Set )
{
#if SUPPORT_NAME_FLAGS
	// make sure we only set the expected tags on here
	Flags |= Set;
#else
	appErrorf(TEXT("Set SUPPORT_NAME_FLAGS to 1 in UnBuild.h if name flags are needed."));
#endif
}
/**
 * Clears the passed in flags on the name entry.
 *
 * @param	FlagsToClear	Flags to clear.
 */
void FNameEntry::ClearFlags( EObjectFlags Clear )
{
#if SUPPORT_NAME_FLAGS
	Flags &= ~Clear;
#else
	appErrorf(TEXT("Set SUPPORT_NAME_FLAGS to 1 in UnBuild.h if name flags are needed."));
#endif
}


/*-----------------------------------------------------------------------------
	FName statics.
-----------------------------------------------------------------------------*/

// Static variables.
FNameEntry*					FName::NameHash[4096];
TArrayNoInit<FNameEntry*>	FName::Names;
INT							FName::NameEntryMemorySize;
/** Number of ANSI names in name table.						*/
INT							FName::NumAnsiNames;			
/** Number of Unicode names in name table.					*/
INT							FName::NumUnicodeNames;


/*-----------------------------------------------------------------------------
	FName implementation.
-----------------------------------------------------------------------------*/

//
// Hardcode a name.
//
void FName::Hardcode(FNameEntry* AutoName)
{
	// Add name to name hash.
	INT iHash          = AutoName->GetNameHash() & (ARRAY_COUNT(NameHash)-1);
	AutoName->HashNext = NameHash[iHash];
	NameHash[iHash]    = AutoName;

	// Expand the table if needed.
	for( INT i=Names.Num(); i<=AutoName->GetIndex(); i++ )
	{
		Names.AddItem( NULL );
	}

	// Add name to table.
	if( Names(AutoName->GetIndex()) )
	{
		appErrorf( TEXT("Hardcoded name '%s' at index %i was duplicated. Existing entry is '%s'."), *AutoName->GetNameString(), AutoName->GetIndex(), *Names(AutoName->GetIndex())->GetNameString() );
	}
	Names(AutoName->GetIndex()) = AutoName;
}

/**
 * Create an FName. If FindType is FNAME_Find, and the string part of the name 
 * doesn't already exist, then the name will be NAME_None
 *
 * @param Name Value for the string portion of the name
 * @param FindType Action to take (see EFindName)
 */
FName::FName( const TCHAR* Name, EFindName FindType, UBOOL )
{
	Init(Name, NAME_NO_NUMBER_INTERNAL, FindType);
}

/**
 * Create an FName. If FindType is FNAME_Find, and the string part of the name 
 * doesn't already exist, then the name will be NAME_None
 *
 * @param Name Value for the string portion of the name
 * @param Number Value for the number portion of the name
 * @param FindType Action to take (see EFindName)
 */
FName::FName( const TCHAR* Name, INT Number, EFindName FindType )
{
	Init(Name, Number, FindType);
}


/**
 * Constructor used by ULinkerLoad when loading its name table; Creates an FName with an instance
 * number of 0 that does not attempt to split the FName into string and number portions.
 */
FName::FName( ELinkerNameTableConstructor, const TCHAR* Name )
{
	Init(Name, NAME_NO_NUMBER_INTERNAL, FNAME_Add, FALSE);
}


/**
 * Shared initialization code (between two constructors)
 * 
 * @param InName		String name of the name/number pair
 * @param InNumber		Number part of the name/number pair
 * @param FindType		Operation to perform on names
 * @param bSplitName	If TRUE, the constructor will attempt to split a number off of the string portion (turning Rocket_17 to Rocket and number 17)
 */
void FName::Init(const TCHAR* InName, INT InNumber, EFindName FindType, UBOOL bSplitName/*=TRUE*/ )
{
	checkSlow(appStrlen(InName)<=NAME_SIZE);

	// initialiuze the name subsystem if necessary
	if (!GetIsInitialized())
	{
		StaticInit();
	}

	TCHAR TempBuffer[NAME_SIZE];
	INT TempNumber;
	// if we were passed in a number, we can't split again, other wise, a_1_2_3_4 would change everytime
	// it was loaded in
	if (InNumber == NAME_NO_NUMBER_INTERNAL && bSplitName == TRUE )
	{
		if (SplitNameWithCheck(InName, TempBuffer, ARRAY_COUNT(TempBuffer), TempNumber))
		{
			InName = TempBuffer;
			InNumber = NAME_EXTERNAL_TO_INTERNAL(TempNumber);
		}
	}

	check(InName);

	// If empty or invalid name was specified, return NAME_None.
	if( !InName[0] )
	{
		Index = NAME_None;
		Number = NAME_NO_NUMBER_INTERNAL;
		return;
	}

	// set the number
	Number = InNumber;

	// Hash value of string. Depends on whether the name is going to be ansi or unicode.
	INT iHash;

	// Figure out whether we have a pure ansi name or not.
	ANSICHAR AnsiName[NAME_SIZE];
	UBOOL bIsPureAnsi = appIsPureAnsi( InName );
	if( bIsPureAnsi )
	{
		appStrncpyANSI( AnsiName, TCHAR_TO_ANSI(InName), ARRAY_COUNT(AnsiName) );
		iHash = appStrihash( AnsiName ) & (ARRAY_COUNT(NameHash)-1);
	}
	else
	{
		iHash = appStrihash( InName ) & (ARRAY_COUNT(NameHash)-1);
	}

	// Try to find the name in the hash.
	for( FNameEntry* Hash=NameHash[iHash]; Hash; Hash=Hash->HashNext )
	{
		// Compare the passed in string, either ANSI or UNICHAR.
		if( ( bIsPureAnsi && Hash->IsEqual( AnsiName )) 
		||  (!bIsPureAnsi && Hash->IsEqual( InName )) )
		{
			// Found it in the hash.
			Index = Hash->GetIndex();

			// Check to see if the caller wants to replace the contents of the
			// FName with the specified value. This is useful for compiling
			// script classes where the file name is lower case but the class
			// was intended to be uppercase.
			if (FindType == FNAME_Replace)
			{
				// This should be impossible due to the compare above
				// This *must* be true, or we'll overwrite memory when the
				// copy happens if it is longer
				checkSlow(appStrlen(InName) == Hash->GetNameLength());
				// Can't rely on the template override for static arrays since the safe crt version of strcpy will fill in
				// the remainder of the array of NAME_SIZE with 0xfd.  So, we have to pass in the length of the dynamically allocated array instead.
				if( bIsPureAnsi )
				{
					appStrcpy(Hash->GetAnsiName(),Hash->GetNameLength()+1,AnsiName);
				}
				else
				{
					appStrcpy(Hash->GetUniName(),Hash->GetNameLength()+1,InName);
				}
			}

			return;
		}
	}

	// Didn't find name.
	if( FindType==FNAME_Find )
	{
		// Not found.
		Index = NAME_None;
		Number = NAME_NO_NUMBER_INTERNAL;
		return;
	}

	// Add new entry.
	Index = Names.Add();

	// Allocate and set the name.
	const void* NewName = NULL;
	if( bIsPureAnsi )
	{
		NewName = AnsiName;
	}
	else
	{
		NewName = InName;
	}
	Names(Index) = NameHash[iHash] = AllocateNameEntry( NewName, Index, NameHash[iHash], bIsPureAnsi );
}


/**
 * Converts an FName to a readable format
 *
 * @return String representation of the name
 */
FString FName::ToString() const
{
	FString Out;	
	ToString(Out);
	return Out;
}

/**
 * Converts an FName to a readable format, in place
 * 
 * @param Out String to fill ot with the string representation of the name
 */
void FName::ToString(FString& Out) const
{
	// a version of ToString that saves at least one string copy
	checkName(Index < Names.Num());
	checkName(Names(Index));
	FNameEntry* NameEntry = Names(Index);
	Out.Empty( NameEntry->GetNameLength() + 6);
	AppendString(Out);
}

/**
 * Converts an FName to a readable format, in place
 * 
 * @param Out String to fill ot with the string representation of the name
 */
void FName::AppendString(FString& Out) const
{
	checkName(Index < Names.Num());
	checkName(Names(Index));
	FNameEntry* NameEntry = Names(Index);
	NameEntry->AppendNameToString( Out );
	if (Number != NAME_NO_NUMBER_INTERNAL)
	{
		Out += TEXT("_");
		appItoaAppend(NAME_INTERNAL_TO_EXTERNAL(Number),Out);
	}
}

INT FName::GetNumber() const
{
	return Number;
}


/*-----------------------------------------------------------------------------
	CRC functions. (MOVED FROM UNMISC.CPP FOR MASSIVE WORKAROUND @todo put back!
-----------------------------------------------------------------------------*/
// CRC 32 polynomial.
#define CRC32_POLY 0x04c11db7

/** 
* Helper class for initializing the global GCRCTable
*/
class FCRCTableInit
{
public:
	/**
	* Constructor
	*/
	FCRCTableInit()
	{
		// Init CRC table.
		for( DWORD iCRC=0; iCRC<256; iCRC++ )
		{
			for( DWORD c=iCRC<<24, j=8; j!=0; j-- )
			{
				GCRCTable[iCRC] = c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
			}
		}
	}	
};
DWORD GCRCTable[256];


/*-----------------------------------------------------------------------------
	FName subsystem.
-----------------------------------------------------------------------------*/

//
// Initialize the name subsystem.
//
void FName::StaticInit()
{
	/** Global instance used to initialize the GCRCTable. It used to be initialized in appInit() */
	//@todo: Massive workaround for static init order without needing to use a function call for every use of GCRCTable
	// This ASSUMES that fname::StaticINit is going to be called BEFORE ANY use of GCRCTable
	static FCRCTableInit GCRCTableInit;

	check(GetIsInitialized() == FALSE);
	check((ARRAY_COUNT(NameHash)&(ARRAY_COUNT(NameHash)-1)) == 0);
	GetIsInitialized() = 1;

	// initialize the TArrayNoInit() on first use, NOT when the constructor is called (as it could happen
	// AFTER this function)
	appMemzero(&FName::Names, sizeof(FName::Names));

	// Init the name hash.
	for (INT HashIndex = 0; HashIndex < ARRAY_COUNT(FName::NameHash); HashIndex++)
	{
		NameHash[HashIndex] = NULL;
	}

	// Register all hardcoded names.
	#define REGISTER_NAME(num,namestr) Hardcode(AllocateNameEntry(#namestr,num,NULL,TRUE));
	#include "UnNames.h"

#if DO_CHECK
	// Verify no duplicate names.
	for (INT HashIndex = 0; HashIndex < ARRAY_COUNT(NameHash); HashIndex++)
	{
		for (FNameEntry* Hash = NameHash[HashIndex]; Hash; Hash = Hash->HashNext)
		{
			for (FNameEntry* Other = Hash->HashNext; Other; Other = Other->HashNext)
			{
				if (appStricmp(*Hash->GetNameString(), *Other->GetNameString()) == 0)
				{
					// we can't print out here because there may be no log yet if this shappens before main starts
					appDebugBreak();
				}
			}
		}
	}
#endif
}

//
// Display the contents of the global name hash.
//
void FName::DisplayHash( FOutputDevice& Ar )
{
	INT UsedBins=0, NameCount=0, MemUsed = 0;
	for( INT i=0; i<ARRAY_COUNT(NameHash); i++ )
	{
		if( NameHash[i] != NULL ) UsedBins++;
		for( FNameEntry *Hash = NameHash[i]; Hash; Hash=Hash->HashNext )
		{
			NameCount++;
			// Count how much memory this entry is using
			MemUsed += FNameEntry::GetSize( Hash->GetNameLength(), Hash->IsUnicode() );
		}
	}
	Ar.Logf( TEXT("Hash: %i names, %i/%i hash bins, Mem in bytes %i"), NameCount, UsedBins, ARRAY_COUNT(NameHash), MemUsed);
}

/**
 * Helper function to split an old-style name (Class_Number, ie Rocket_17) into
 * the component parts usable by new-style FNames
 *
 * @param OldName		Old-style name
 * @param NewName		Ouput string portion of the name/number pair
 * @param NewNumber		Number portion of the name/number pair
 */
void FName::SplitOldName(const TCHAR* OldName, FString& NewName, INT& NewNumber)
{
	TCHAR Temp[NAME_SIZE] = TEXT("");

	// try to split the name
	if (SplitNameWithCheck(OldName, Temp, NAME_SIZE, NewNumber))
	{
		// if the split succeeded, copy the temp buffer to the string
		NewName = Temp;
	}
	else
	{
		// otherwise, just copy the old name to new name, with no number
		NewName = OldName;
		NewNumber = NAME_NO_NUMBER;
	}
}

/**
 * Helper function to split an old-style name (Class_Number, ie Rocket_17) into
 * the component parts usable by new-style FNames. Only use results if this function
 * returns TRUE.
 *
 * @param OldName		Old-style name
 * @param NewName		Ouput string portion of the name/number pair
 * @param NewNameLen	Size of NewName buffer (in TCHAR units)
 * @param NewNumber		Number portion of the name/number pair
 *
 * @return TRUE if the name was split, only then will NewName/NewNumber have valid values
 */
UBOOL FName::SplitNameWithCheck(const TCHAR* OldName, TCHAR* NewName, INT NewNameLen, INT& NewNumber)
{
	UBOOL bSucceeded = FALSE;
	const INT OldNameLength = appStrlen(OldName);

	if(OldNameLength > 0)
	{
		// get string length
		const TCHAR* LastChar = OldName + (OldNameLength - 1);
		
		// if the last char is a number, then we will try to split
		const TCHAR* Ch = LastChar;
		if (*Ch >= '0' && *Ch <= '9')
		{
			// go backwards, looking an underscore or the start of the string
			// (we don't look at first char because '_9' won't split well)
			while (*Ch >= '0' && *Ch <= '9' && Ch > OldName)
			{
				Ch--;
			}

			// if the first non-number was an underscore (as opposed to a letter),
			// we can split
			if (*Ch == '_')
			{
				// check for the case where there are multiple digits after the _ and the first one
				// is a 0 ("Rocket_04"). Can't split this case. (So, we check if the first char
				// is not 0 or the length of the number is 1 (since ROcket_0 is valid)
				if (Ch[1] != '0' || LastChar - Ch == 1)
				{
					// attempt to convert what's following it to a number
					NewNumber = appAtoi(Ch + 1);

					// copy the name portion into the buffer
					appStrncpy(NewName, OldName, Min<INT>(Ch - OldName + 1, NewNameLen));

					// mark successful
					bSucceeded = TRUE;
				}
			}
		}
	}

	// return success code
	return bSucceeded;
}


/*-----------------------------------------------------------------------------
	FNameEntry implementation.
-----------------------------------------------------------------------------*/

FArchive& operator<<( FArchive& Ar, FNameEntry& E )
{
	if( Ar.IsLoading() )
	{
		FString Str;
		Ar << Str;
		// Can't rely on the template override for static arrays since the safe crt version of strcpy will fill in
		// the remainder of the array of NAME_SIZE with 0xfd.  So, we have to pass in the length of the dynamically allocated array instead.
		//
		// Loaded name entries are always unicode as they are serialized out this way. They are only interim though and
		// will have AllocateNameEntry called on them which does the proper ANSI/UNICHAR handling.
		appStrcpy( E.GetUniName(), Str.Len()+1, *Str.Left(NAME_SIZE-1) );
	}
	else
	{
		FString Str = E.GetNameString();
		Ar << Str;
	}

#if SUPPORT_NAME_FLAGS
	Ar << E.Flags;
#else
	EObjectFlags DummyFlags = 0;
	Ar << DummyFlags;
#endif
	return Ar;
}

/**
 * Pooled allocator for FNameEntry structures. Doesn't have to worry about freeing memory as those 
 * never go away. It simply uses 64K chunks and allocates new ones as space runs out. This reduces
 * allocation overhead significantly (only minor waste on 64k boundaries) and also greatly helps
 * with fragmentation as 50-100k allocations turn into tens of allocations.
 */
class FNameEntryPoolAllocator
{
public:
	/** Initializes all member variables. */
	FNameEntryPoolAllocator()
	{
		CurrentPoolStart = NULL;
		CurrentPoolEnd	 = NULL;
	}

	/** 
	 * Allocates the requested amount of bytes and casts them to a FNameEntry pointer. 
	 *
	 * @param	Size	Size in bytes to allocate
	 * @return	Allocation of passed in size cast to a FNameEntry pointer.
	 */
	FNameEntry* Allocate( INT Size )
	{
		// Allocate a new pool if current one is exhausted. We don't worry about a little bit
		// of waste at the end given the relative size of pool to average and max allocation.
		if( CurrentPoolEnd - CurrentPoolStart < Size )
		{
			AllocateNewPool();
		}
		check( CurrentPoolEnd - CurrentPoolStart >= Size );
		// Return current pool start as allocation and increment by size.
		FNameEntry* NameEntry = (FNameEntry*) CurrentPoolStart;
		CurrentPoolStart += Size;
		return NameEntry;
	}

private:
	/** Allocates a new pool. */
	void AllocateNewPool()
	{
		// Allocate in 64k chunks as it's ideal for page size.
		INT PoolSize		= 64 * 1024;
		CurrentPoolStart	= (BYTE*) appMalloc(PoolSize);
		CurrentPoolEnd		= CurrentPoolStart + PoolSize;
	}
	
	/** Beginning of pool. Allocated by AllocateNewPool, incremented by Allocate.	*/
	BYTE* CurrentPoolStart;
	/** End of current pool. Set by AllocateNewPool and checked by Allocate.		*/
	BYTE* CurrentPoolEnd;
};

/** Global allocator for name entries. */
FNameEntryPoolAllocator GNameEntryPoolAllocator;

FNameEntry* AllocateNameEntry( const void* Name, NAME_INDEX Index, FNameEntry* HashNext, UBOOL bIsPureAnsi )
{
	const SIZE_T NameLen  = bIsPureAnsi ? appStrlen((ANSICHAR*)Name) : appStrlen((UNICHAR*)Name);
	INT NameEntrySize	  = FNameEntry::GetSize( NameLen, bIsPureAnsi );
	FNameEntry* NameEntry = GNameEntryPoolAllocator.Allocate( NameEntrySize );
	FName::NameEntryMemorySize += NameEntrySize;
#if SUPPORT_NAME_FLAGS
	NameEntry->Flags      = 0;
#endif
	NameEntry->Index      = (Index << NAME_INDEX_SHIFT) | (bIsPureAnsi ? 0 : 1);
	NameEntry->HashNext   = HashNext;
	// Can't rely on the template override for static arrays since the safe crt version of strcpy will fill in
	// the remainder of the array of NAME_SIZE with 0xfd.  So, we have to pass in the length of the dynamically allocated array instead.
	if( bIsPureAnsi )
	{
		appStrcpy( NameEntry->GetAnsiName(), NameLen + 1, (ANSICHAR*) Name );
		FName::NumAnsiNames++;
	}
	else
	{
		appStrcpy( NameEntry->GetUniName(), NameLen + 1, (UNICHAR*) Name );
		FName::NumUnicodeNames++;
	}
	return NameEntry;
}


