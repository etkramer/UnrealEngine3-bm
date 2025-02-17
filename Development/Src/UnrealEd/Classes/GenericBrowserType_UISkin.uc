/**
 * Generic browser type for editing UISkins
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GenericBrowserType_UISkin extends GenericBrowserType
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
	 * @param	InObject	the object to edit.  this should always be a UISkin object.
	 */
	virtual UBOOL ShowObjectEditor( UObject* InObject );
}

DefaultProperties
{
	Description="UI Skins"
}
