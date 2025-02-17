/**
 * This action allows designers to scroll widgets (which support scrolling) up or down.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_ApplyScrolling extends UIAction;

/** Specifies whether the widget should scroll horizontally or vertically */
var()	EUIOrientation	ScrollOrientation;

/** TRUE to scroll further towards the end of the "scrolling region"; FALSE to scroll towards the beginning */
var()	bool			bIncreaseScroll;

/** TRUE to indicate that the widget should scroll a full page, if possible. */
var()	bool			bFullPage;

DefaultProperties
{
	ScrollOrientation=UIORIENT_Vertical

	ObjName="Apply Scrolling"
	bAutoTargetOwner=true

	OutputLinks(0)=(LinkDesc="Succeeded")
	OutputLinks(1)=(LinkDesc="Failed")
}
