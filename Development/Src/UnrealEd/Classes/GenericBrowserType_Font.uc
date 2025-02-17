/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_Font: Fonts
//=============================================================================

class GenericBrowserType_Font
	extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();
	/**
	 * Displays the font properties window for editing & importing/exporting of
	 * font pages
	 *
	 * @param InObject the object being edited
	 */
	virtual UBOOL ShowObjectEditor( UObject* InObject );
}
	
defaultproperties
{
	Description="Font"
}
