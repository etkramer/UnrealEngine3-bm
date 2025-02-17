/*=============================================================================
	ExampleGame.cpp
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes.
#include "ExampleGame.h"

/*-----------------------------------------------------------------------------
	The following must be done once per package.
-----------------------------------------------------------------------------*/

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName EXAMPLEGAME_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "ExampleGameClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "ExampleGameClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NATIVES_ONLY
#undef NAMES_ONLY

/**
 * Initialize registrants, basically calling StaticClass() to create the class and also 
 * populating the lookup table.
 *
 * @param	Lookup	current index into lookup table
 */
void AutoInitializeRegistrantsExampleGame( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_EXAMPLEGAME;
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesExampleGame()
{
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
    #define AUTOGENERATE_NAME(name) EXAMPLEGAME_##name = FName(TEXT(#name));
	#include "ExampleGameClasses.h"
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
}

/**
 * Game-specific code to handle DLC being added or removed
 * 
 * @param bContentWasInstalled TRUE if DLC was installed, FALSE if it was removed
 */
void appOnDownloadableContentChanged(UBOOL bContentWasInstalled)
{
	// ExampleGame doesn't care
}

FSystemSettings& GetSystemSettings()
{
	static FSystemSettings SystemSettingsSingleton;
	return SystemSettingsSingleton;
}
