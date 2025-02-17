/**
 * Activated when a tab page and tab button are enabled or disabled.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIEvent_TabPageEnabled extends UIEvent_TabControl;

DefaultProperties
{
	ObjName="Page Enabled/Disabled"
	bAutoActivateOutputLinks=false

	OutputLinks.Empty
	OutputLinks(0)=(LinkDesc="Enabled")
	OutputLinks(1)=(LinkDesc="Disabled")
}
