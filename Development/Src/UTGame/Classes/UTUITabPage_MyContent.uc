/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for content a user has installed.
 */
class UTUITabPage_MyContent extends UTTabPage
	placeable
	dependson(UTUIScene_MessageBox)
	native(UI);

/** Reference to the list that displays user content. */
var transient UIList	ContentList;

/** Reference to a message box scene. */
var transient UTUIScene_MessageBox MessageBoxReference;

/**
 * Actually deletes the content from the storage device.
 *
 * @param ContentName		Filename of content to delete.
 */
native function PerformDeleteContent(string ContentName);

/**
 * Reloads all downloaded content.
 */
native function ReloadContent();

/**
 * Returns the filename of some content given its content idx.
 *
 * @param ContentIdx	Index of the content file to retrieve.
 */
function bool GetContentName(int ContentIdx, out string ContentName)
{
	return class'UTUIMenuList'.static.GetCellFieldString(ContentList, 'FileName', ContentIdx, ContentName);
}

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	Super.PostInitialize();

	// Get widget references
	ContentList = UIList(FindChild('lstContent', true));
	ContentList.OnValueChanged = OnContentList_ValueChanged;

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Community.MyContent>");
}

/** Callback allowing the tabpage to setup the button bar for the current scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	if(ContentList.GetCurrentItem()!=INDEX_NONE)
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.DeleteContent>", OnButtonBar_DeleteContent);
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.DeleteAllContent>", OnButtonBar_DeleteAllContent);
	}
}

/** Deletes the currently selected content file. */
function OnDeleteContent()
{
	local string ContentName;
	local int ContentIdx;
	local string FinalString;
	local array<string> MessageBoxOptions;

		ContentIdx = ContentList.GetCurrentItem();

		// Display messagebox to confirm deletion.
		MessageBoxReference = UTUIScene(GetScene()).GetMessageBoxScene();

		if(MessageBoxReference != none && GetContentName(ContentIdx, ContentName))
		{

			FinalString = Localize("MessageBox", "DeleteContent_Message", "UTGameUI");
			FinalString = Repl(FinalString, "\`ContentName\`", ContentName);

			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.DeleteContentAccept>");
			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Cancel>");

			MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
			MessageBoxReference.Display(FinalString, "<Strings:UTGameUI.MessageBox.DeleteContent_Title>", OnDeleteContent_Confirm, 1);
		}
	}

/** Confirmation for the delete content dialog. */
function OnDeleteContent_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	local string ContentName;
	MessageBoxReference = None;

	if(SelectedItem==0 && GetContentName(ContentList.GetCurrentItem(), ContentName))
	{
		PerformDeleteContent(ContentName);
		ReloadContent();
		OnContentListChanged();
	}
}


/** Deletes all content. */
function OnDeleteAllContent()
{
	local string FinalString;
	local array<string> MessageBoxOptions;

		// Display messagebox to confirm deletion.
		MessageBoxReference = UTUIScene(GetScene()).GetMessageBoxScene();

		if(MessageBoxReference != none)
		{

			FinalString = Localize("MessageBox", "DeleteAllContent_Message", "UTGameUI");
			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.DeleteContentAccept>");
			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Cancel>");

			MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
			MessageBoxReference.Display(FinalString, "<Strings:UTGameUI.MessageBox.DeleteAllContent_Title>", OnDeleteAllContent_Confirm, 1);
		}
	}

/** Confirmation for the delete all content dialog. */
function OnDeleteAllContent_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	local int ElementIdx;
	local string ContentName;

	MessageBoxReference = None;

	if(SelectedItem==0)
	{
		// Delete all items.
		for(ElementIdx=0; ElementIdx<ContentList.Items.length; ElementIdx++)
		{
			if(GetContentName(ContentList.Items[ElementIdx], ContentName))
			{
				PerformDeleteContent(ContentName);
			}
		}

		ReloadContent();
		OnContentListChanged();
	}
}

/** Callback for when the content list has changed. */
function OnContentListChanged()
{
	local int OldIdx;

	OldIdx = ContentList.Index;
	ContentList.RefreshSubscriberValue();
	ContentList.SetIndex(OldIdx);
}


/** Buttonbar Callbacks */
function bool OnButtonBar_DeleteContent(UIScreenObject InButton, int PlayerIndex)
{
	OnDeleteContent();

	return true;
}

function bool OnButtonBar_DeleteAllContent(UIScreenObject InButton, int PlayerIndex)
{
	OnDeleteAllContent();

	return true;
}

/** Callback for when the user changes the currently selected list item. */
function OnContentList_ValueChanged( UIObject Sender, optional int PlayerIndex=0 )
{
	UTUIFrontEnd(GetScene()).SetupButtonBar();
}



/**
 * Provides a hook for unrealscript to respond to input using actual input key names (i.e. Left, Tab, etc.)
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called BEFORE kismet is given a chance to process the input.
 *
 * @param	EventParms	information about the input event.
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool HandleInputKey( const out InputEventParameters EventParms )
{
	local bool bResult;

	bResult=false;

	if(EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_Y')
		{
			OnDeleteContent();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_Back')
		{
			OnDeleteAllContent();
			bResult=true;
		}
	}

	return bResult;
}
