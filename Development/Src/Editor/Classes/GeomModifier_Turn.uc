/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Turns selected objects.
 */
class GeomModifier_Turn
	extends GeomModifier_Edit
	native;
	
cpptext
{
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
}
	
defaultproperties
{
	Description="Turn"
	bPushButton=True
}
