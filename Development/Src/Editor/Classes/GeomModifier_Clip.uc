/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Allows clipping of BSP brushes against a plane.
 */
class GeomModifier_Clip
	extends GeomModifier_Edit
	native;

var(Settings)	bool	bFlipNormal;
var(Settings)	bool	bSplit;

cpptext
{
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

	/**
	 * @return		TRUE if the key was handled by this editor mode tool.
	 */
	virtual UBOOL InputKey(struct FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event);

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();

private:
 	void ApplyClip();
}
	
defaultproperties
{
	Description="Clip"
}
