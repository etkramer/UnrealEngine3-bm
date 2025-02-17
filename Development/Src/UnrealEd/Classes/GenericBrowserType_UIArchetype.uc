/**
 * Generic browser type for editing UIObject archetypes which are not contained by UI prefabs.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GenericBrowserType_UIArchetype extends GenericBrowserType_Archetype
	native;

cpptext
{
	/**
	 * Initialize the supported classes for this browser type.
	 */
	virtual void Init();

	/**
	 * Display the editor for the object specified.
	 *
	 * @param	InObject	the object to edit.  this should always be a UIPrefab which has the RF_ArchetypeObject flag set.
	 */
	virtual UBOOL ShowObjectEditor( UObject* InObject );

	/**
	 * Called when the user chooses to delete objects from the generic browser.  Gives the resource type the opportunity
	 * to perform any special logic prior to the delete.
	 *
	 * @param	ObjectToDelete	the object about to be deleted.
	 *
	 * @return	TRUE to allow the object to be deleted, FALSE to prevent the object from being deleted.
	 */
	virtual UBOOL NotifyPreDeleteObject( UObject* ObjectToDelete );

protected:
	/**
	 * Determines whether the specified package is allowed to be saved.
	 *
	 * @param	PackageToSave		the package that is about to be saved
	 * @param	StandaloneObjects	a list of objects from PackageToSave which were marked RF_Standalone
	 */
	virtual UBOOL IsSavePackageAllowed( UPackage* PackageToSave, TArray<UObject*>& StandaloneObjects );

public:
}

/**
 * Points to the UISceneManager singleton stored in the BrowserManager.
 */
var	const transient	UISceneManager				SceneManager;

DefaultProperties
{
	Description="UI Prefabs"
}
