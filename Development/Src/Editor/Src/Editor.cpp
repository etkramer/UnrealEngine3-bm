/*=============================================================================
	Editor.cpp: Unreal editor package.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "Factories.h"
#include "UnEdTran.h"

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

/**
 * Provides access to the FEditorModeTools singleton.
 */
class FEditorModeTools& GEditorModeTools()
{
	static FEditorModeTools* EditorModeToolsSingleton = new FEditorModeTools;
	return *EditorModeToolsSingleton;
}

FEditorLevelViewportClient* GCurrentLevelEditingViewportClient = NULL;
/** Tracks the last level editing viewport client that received a key press. */
FEditorLevelViewportClient* GLastKeyLevelEditingViewportClient = NULL;

#define STATIC_LINKING_MOJO 1

// Register things.
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name) FName EDITOR_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name) IMPLEMENT_FUNCTION(cls,idx,name)
#include "EditorClasses.h"
#undef AUTOGENERATE_FUNCTION
#undef AUTOGENERATE_NAME
#undef NAMES_ONLY

// Register natives.
#define NATIVES_ONLY
#define NAMES_ONLY
#define AUTOGENERATE_NAME(name)
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#include "EditorClasses.h"
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
void AutoInitializeRegistrantsEditor( INT& Lookup )
{
	AUTO_INITIALIZE_REGISTRANTS_EDITOR
}

/**
 * Auto generates names.
 */
void AutoGenerateNamesEditor()
{
	#define NAMES_ONLY
	#define AUTOGENERATE_FUNCTION(cls,idx,name)
	#define AUTOGENERATE_NAME(name) EDITOR_##name=FName(TEXT(#name));
	#include "EditorClasses.h"
	#undef AUTOGENERATE_FUNCTION
	#undef AUTOGENERATE_NAME
	#undef NAMES_ONLY
}

