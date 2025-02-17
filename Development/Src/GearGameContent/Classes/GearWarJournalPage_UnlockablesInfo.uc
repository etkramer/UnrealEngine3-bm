/**
 * This panel displays information about the item currently selected in the list of unlockables.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearWarJournalPage_UnlockablesInfo extends GearWarJournalPage_Base;

var	transient	UIImage		imgPreview;
var	transient	UILabel		lblDescription;

/* == GearWarJournalPage_Base interface == */
/**
 * Assigns all member variables to the appropriate child widgets in this panel.
 */
function InitializePageReferences()
{
	imgPreview = UIImage(FindChild('imgPreview',true));
	lblDescription = UILabel(FindChild('lblDescription',true));

	Super.InitializePageReferences();
}

DefaultProperties
{

}
