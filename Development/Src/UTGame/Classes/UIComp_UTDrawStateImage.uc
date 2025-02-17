/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UIComp_UTDrawStateImage extends UIComp_DrawImage
	native(UI);

cpptext
{
 	/**
	 * Applies the current style data (including any style data customization which might be enabled) to the component's image.
	 */
	void RefreshAppliedStyleData();
}

/** State for the image component, used when resolving styles. */
var			transient		class<UIState>							ImageState;

