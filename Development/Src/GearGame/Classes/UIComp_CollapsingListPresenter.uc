/**
 * This list component handles formatting and rendering rows for a GearUICollapsingSelectionList.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class UIComp_CollapsingListPresenter extends UIComp_ListPresenter
	within GearUICollapsingSelectionList
	native(UIPrivate);

/** Opacity to use for rendering the list's text; does not affect the opacity of the widget's labels or background image */
var	transient	const	float		AnimationOpacity;

/**
 * The offset used to translate the list during animation.  The value should be interpreted as follows:
 * - X is the percentage of the widget's width to offset the list text horizontally
 * - Y is the percentage of the widget's height to offset the list text vertically.
 * - Z is currently unused
 */
var	transient	const	vector		RenderOffsetPercentage;

cpptext
{
	/* === UUIComp_ListPresenter interface === */
	/**
	 * Renders the elements in this list.
	 *
	 * @param	RI					the render interface to use for rendering
	 */
	virtual void Render_List( FCanvas* Canvas );

	/**
	 * Renders the overlay image for a single list element.  Moved into a separate function to allow child classes to easily override
	 * and modify the way that the overlay is rendered.
	 *
	 * @param	same as Render_ListElement, except that no values are passed back to the caller.
	 */
	virtual void Render_ElementOverlay( FCanvas* Canvas, INT ElementIndex, const FRenderParameters& Parameters, const FVector2D& DefaultCellSize );

	/* === UUIComp_ListPresenterBase interface === */
	/**
	 * Determines the appropriate position for the selection hint object based on the size of the list's rows and any padding that must be taken
	 * into account.
	 *
	 * @param	SelectionHintObject		the widget that will display the selection hint (usually a label).
	 * @param	ElementIndex			the index of the element to display the selection hint next to.
	 */
	virtual UBOOL SetSelectionHintPosition( UUIObject* SelectionHintObject, INT ElementIndex );

	/**
	 * Initializes the render parameters that will be used for formatting the list elements.
	 *
	 * @param	Face			the face that was being resolved
	 * @param	out_Parameters	[out] the formatting parameters to use when calling ApplyFormatting.
	 *
	 * @return	TRUE if the formatting data is ready to be applied to the list elements, taking into account the autosize settings.
	 */
	virtual UBOOL GetListRenderParameters( EUIWidgetFace Face, FRenderParameters& out_Parameters );

	/**
	 * Wrapper for getting the docking-state of the owning widget's four faces.  No special logic here, but child classes
	 * can use this method to make the formatting code ignore the fact that the widget may be docked (in cases where it is
	 * irrelevant)
	 *
	 * @param	bFaceDocked		[out] an array of bools representing whether the widget is docked on the respective face.
	 */
	virtual void GetOwnerDockingState( UBOOL* bFaceDocked[UIFACE_MAX] ) const;

	/**
	 * Adjusts the owning widget's bounds according to the autosize settings.
	 */
	virtual void UpdateOwnerBounds( FRenderParameters& Parameters );

	/**
	 * Setup the left, top, width, and height values that will be used to render the list.  This will typically be the list's
	 * RenderBounds, unless the elements should be rendered in a subportion of the list.
	 *
	 * @fixme ronp - mmmmm, this is a bit hacky..  we're already doing something similar on the formatting side...seems like
	 * we should be able to leverage that work so that we don't get out of sync.  :\
	 */
	virtual void InitializeRenderingParms( FRenderParameters& Parameters, FCanvas* Canvas=NULL );
}

DefaultProperties
{
	bDisplayColumnHeaders=false
	RenderOffsetPercentage=(Y=-1.0)
	AnimationOpacity=0
}
