/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Extrudes selected objects.
 */
class GeomModifier_Extrude
	extends GeomModifier_Edit
	native;
	
var(Settings)	int		Length;
var(Settings)	int		Segments;

cpptext
{
	/**
	 * @return		TRUE if the specified selection type is supported by this modifier, FALSE otherwise.
	 */
	virtual UBOOL Supports(INT InSelType);

	/**
	 * Gives the individual modifiers a chance to do something the first time they are activated.
	 */
	virtual void Initialize();

protected:
	/**
	 * Implements the modifier application.
	 */
 	virtual UBOOL OnApply();
 	
private:
 	void Apply(INT InLength, INT InSegments);
}
	
defaultproperties
{
	Description="Extrude"
	Length=16
	Segments=1
}
