/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================================================================
// GenericBrowserType_Texture: Textures
//=============================================================================

class GenericBrowserType_TextureWithAlpha
	extends GenericBrowserType_Texture
	native;

cpptext
{
	virtual void Init();

	/**
	 * Returns TRUE if passed in UObject is a texture using an alpha channel.
	 *
	 * @param	Object	Object to check whether it's a texture utilizing an alpha channel
	 * @return TRUE if passed in UObject is a texture using an alpha channel, FALSE otherwise
	 */
	static UBOOL IsTextureWithAlpha( UObject* Object );
}
	
defaultproperties
{
	Description="Texture (with alpha)"
}
