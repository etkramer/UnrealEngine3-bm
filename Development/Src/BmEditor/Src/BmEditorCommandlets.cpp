/*=============================================================================
	BmEditorCommandlets.cpp: Bm editor comandlet definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "BmEditorClasses.h"

#include "PackageHelperFunctions.h"

IMPLEMENT_CLASS(UExtractPackagesCommandlet);

INT UExtractPackagesCommandlet::Main(const FString& Params)
{
	// For now, list these out manually.
	LoadPackage(NULL, TEXT("BmGame"), LOAD_None);
	LoadPackage(NULL, TEXT("Admin_A"), LOAD_None);
	LoadPackage(NULL, TEXT("Admin_B2"), LOAD_None);

	// Set GIsCooking. Without this, SavePackage() will strip the PKG_Cooked flag.
	GIsCooking = TRUE;
	GCookingTarget = UE3::EPlatformType::PLATFORM_PC;

	// Enumerate all objects, searching for force-exported packages
	for ( TObjectIterator<UPackage> It ; It ; ++It )
	{
		UPackage* Package = *It;

		// Skip "group" (not top-level) packages
		if (Package->GetOuter())
		{
			continue;
		}

		// Skip non-BM, non-cooked packages
		if (!Package->IsBmCooked())
		{
			continue;
		}

		// Skip map packages
		if (Package->ContainsMap())
		{
			continue;
		}

		warnf(TEXT("  BM1: Found package '%s'"), *Package->GetPathName());

		// Determine output path
		FString FilePath = TEXT("..\\BmGame\\Packages\\");
		FilePath += Package->GetName();
		FilePath += (Package->PackageFlags & PKG_ContainsScript) ? TEXT(".u") : TEXT(".upk");

		// Save as cooked, uncompressed
		Package->PackageFlags |= PKG_Cooked;
		Package->PackageFlags &= ~PKG_StoreCompressed;

		SavePackage(Package, NULL, RF_Standalone, *FilePath, GWarn);
	}

	return 0;
}
