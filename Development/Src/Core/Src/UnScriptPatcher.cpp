/*=============================================================================
	UnScriptPatcher.cpp: Implementation for script bytecode patcher and helper classes
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"
#include "UnScriptPatcher.h"

#if !__INTEL_BYTE_ORDER__
#include "SwappedScriptPatches.h"
#else
#include "ScriptPatches.h"
#endif


/*
	TODO:

	- support for patching changes to property flags
	- support for patching changes to class flags
*/

#if SCRIPT_PATCHER_SECOND_PASS
/*----------------------------------------------------------------------------
	FPatchData / FScriptPatchData
----------------------------------------------------------------------------*/
/** Serializer */
FArchive& operator<<( FArchive& Ar, FPatchData& Patch )
{
	return Ar << Patch.DataName << Patch.Data;
}

FArchive& operator<<( FArchive& Ar, FScriptPatchData& Patch )
{
	//@todo - how should the name be serialized?  unless it's serialized as a string, it won't be serialized at all
	// because the standard file reader archive doesn't implement operator<<(FName&)
#if 0
	return Ar << Patch.StructName << (FPatchData&)(Patch);
#else
	FString StructNameString;
	if ( Ar.IsLoading() )
	{
		Ar << StructNameString;
		Patch.StructName = FName(*StructNameString);
	}
	else
	{
		StructNameString = Patch.StructName.ToString();
		Ar << StructNameString;
	}
	return Ar << (FPatchData&)(Patch);
#endif
}
#endif

/*----------------------------------------------------------------------------
	FScriptPatcher.
----------------------------------------------------------------------------*/
FScriptPatcher::FScriptPatcher()
{
	BuildPatchList(PackageUpdates);
}

FScriptPatcher::~FScriptPatcher()
{
	for ( INT i = PackageUpdates.Num() - 1; i >= 0; i-- )
	{
		delete PackageUpdates(i);
		PackageUpdates(i) = NULL;
	}
}


/**
 * Retrieves the patch data for the specified package.
 *
 * @param	PackageName			the package name to retrieve a patch for
 * @param	out_LinkerPatch		receives the patch associated with the specified package
 *
 * @return	TRUE if the script patcher contains a patch for the specified package.
 */
UBOOL FScriptPatcher::GetLinkerPatch( const FName& PackageName, FLinkerPatchData*& out_LinkerPatch ) const
{
	UBOOL bResult = FALSE;

	for ( INT UpdateIndex = 0; UpdateIndex < PackageUpdates.Num(); UpdateIndex++ )
	{
		FLinkerPatchData* Update = PackageUpdates(UpdateIndex);
		if ( PackageName == Update->PackageName )
		{
			out_LinkerPatch = Update;
			bResult = TRUE;
			break;
		}
	}

	return bResult;
}
