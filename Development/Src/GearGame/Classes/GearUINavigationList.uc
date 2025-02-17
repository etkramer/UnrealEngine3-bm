/**
 * Gears-specific navigation list.  Renders additional information next to each list item to indicate various items of
 * interest, such as submenus that have new unlocked content to view.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUINavigationList extends UINavigationList
	native(UIPrivate);

DefaultProperties
{
	Begin Object Class=GearUIComp_NavListPresenter Name=NavListPresenter
	End Object
	CellDataComponent=NavListPresenter
}
