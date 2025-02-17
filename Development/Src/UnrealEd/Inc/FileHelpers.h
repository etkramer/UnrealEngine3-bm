/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __FILEHELPERS_H__
#define __FILEHELPERS_H__

// Forward declarations.
class FFileName;
class FString;

/**
 * For saving map files through the main editor frame.
 */
class FEditorFileUtils
{
public:
	////////////////////////////////////////////////////////////////////////////
	// New

	/**
	 * Prompts the user to save the current map if necessary, then creates a new (blank) map.
	 */
	static void NewMap();

	////////////////////////////////////////////////////////////////////////////
	// Loading

	/**
	 * Prompts the user to save the current map if necessary, the presents a load dialog and
	 * loads a new map if selected by the user.
	 */
	static void LoadMap();

	/**
	 * Loads the specified map.  Does not prompt the user to save the current map.
	 *
	 * @param	Filename		Map package filename, including path.
	 */
	static void LoadMap(const FFilename& Filename);

	////////////////////////////////////////////////////////////////////////////
	// Saving

	/**
	 * Saves the specified map package, returning TRUE on success.
	 *
	 * @param	World			The world to save.
	 * @param	Filename		Map package filename, including path.
	 * @return					TRUE if the map was saved successfully.
	 */
	static UBOOL SaveMap(UWorld* World, const FFilename& Filename);

	/**
	 * Saves the specified level.  SaveAs is performed as necessary.
	 *
	 * @param	Level		The level to be saved.
	 * @return				TRUE if the level was saved.
	 */
	static UBOOL SaveLevel(ULevel* Level);

	/**
	 * Does a saveAs for the specified level.
	 *
	 * @param	Level		The level to be SaveAs'd.
	 * @return				TRUE if the level was saved.
	 */
	static UBOOL SaveAs(UObject* LevelObject);

	/**
	 * Checks to see if GWorld's package is dirty and if so, asks the user if they want to save it.
	 * If the user selects yes, does a save or SaveAs, as necessary.
	 */
	static UBOOL AskSaveChanges();

	/**
	 * Saves all levels associated with GWorld.
	 *
	 * @param	bCheckDirty		If TRUE, don't save level packages that aren't dirty.
	 */
	static void SaveAllLevels(UBOOL bCheckDirty);

	/**
	 * Saves all levels to the specified directory.
	 *
	 * @param	AbsoluteAutosaveDir		Autosave directory.
	 * @param	AutosaveIndex			Integer prepended to autosave filenames..
	 */
	static UBOOL AutosaveMap(const FString& AbsoluteAutosaveDir, INT AutosaveIndex);

	////////////////////////////////////////////////////////////////////////////
	// Import/Export

	/**
	 * Presents the user with a file dialog for importing.
	 * If the import is not a merge (bMerging is FALSE), AskSaveChanges() is called first.
	 *
	 * @param	bMerge	If TRUE, merge the file into this map.  If FALSE, merge the map into a blank map.
	 */
	static void Import(UBOOL bMerge);
	static void Export(UBOOL bExportSelectedActorsOnly);
};

#endif
