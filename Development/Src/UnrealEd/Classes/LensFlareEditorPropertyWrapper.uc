/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class LensFlareEditorPropertyWrapper extends Object
	native
	dependson(LensFlare)
	dontcollapsecategories
	hidecategories(Object);

/** The element being displayed the lens flare */
var()	LensFlareElement		Element;

/** The lens flare being edited. */
var	private const LensFlare		SourceLensFlare;
/** The index of the element being edited. */
var private const int			ElementIndex;

//
cpptext
{
	// UObject interface.
	virtual void PreEditChange(UProperty* PropertyAboutToChange);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
}

//
defaultproperties
{
}
