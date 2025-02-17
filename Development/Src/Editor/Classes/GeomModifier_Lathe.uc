/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Lathes selected objects around the widget.
 */
class GeomModifier_Lathe
	extends GeomModifier_Edit
	native;
	
var(Settings) int	TotalSegments;
var(Settings) int	Segments;
var(Settings) EAxis	Axis;

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
	void Apply(INT InTotalSegments, INT InSegments, EAxis InAxis);
}
	
defaultproperties
{
	Description="Lathe"
	TotalSegments=16
	Segments=4
	Axis=AXIS_Z
}
