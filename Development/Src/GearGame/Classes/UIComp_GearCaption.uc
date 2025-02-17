/**
 * Specialized component for widgets that have more than one draw string component.  All logic in this class
 * will eventually be moved into the base class.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class UIComp_GearCaption extends UIComp_DrawCaption
	native(UIPrivate);

cpptext
{
	/* === UUIComp_DrawString interface === */
	/**
	 * Calculates the position and size of the bounding region available for rendering this component's string, taking
	 * into account any configured bounding region clamping.
	 *
	 * @param	[out] BoundingRegionStart	receives the location of the upper left corner of the bounding region, in
	 *										pixels relative to the upper left corner of the screen.
	 * @param	[out] BoundingRegionSize	receives the size of the bounding region, in absolute pixels.
	 */
	virtual void CalculateBoundingRegion( FLOAT* BoundingRegionStart[UIORIENT_MAX], FLOAT* BoundingRegionSize[UIORIENT_MAX] ) const;

protected:
	/**
	 * Initializes the render parameters that will be used for formatting the string.
	 *
	 * @param	Face			the face that was being resolved
	 * @param	out_Parameters	[out] the formatting parameters to use for formatting the string.
	 *
	 * @return	TRUE if the formatting data is ready to be applied to the string, taking into account the autosize settings.
	 */
	virtual UBOOL GetStringFormatParameters( EUIWidgetFace Face, struct FRenderParameters& out_Parameters ) const;

	/**
	 * Wrapper for getting the docking-state of the owning widget's four faces.  No special logic here, but child classes
	 * can use this method to make the formatting code ignore the fact that the widget may be docked (in cases where it is
	 * irrelevant)
	 *
	 * @param	bFaceDocked		[out] an array of bools representing whether the widget is docked on the respective face.
	 */
	virtual void GetOwnerDockingState( UBOOL* bFaceDocked[UIFACE_MAX] ) const;

	/**
	 * Adjusts the owning widget's bounds according to the wrapping mode and autosize behaviors.
	 */
	virtual void UpdateOwnerBounds( struct FRenderParameters& Parameters );
}

DefaultProperties
{

}
