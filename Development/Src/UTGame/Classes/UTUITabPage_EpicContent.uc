/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for epic content available for download.
 */
class UTUITabPage_EpicContent extends UTTabPage
	placeable
	native(UIFrontEnd);

/** Reference to the content list. */
var transient UIList	ContentList;

/**  Reference to the status label. */
var transient UILabel	StatusLabel;

/** Reference to the content datastore. */
var transient UTUIDataStore_Content ContentDataStore;

/** Reference to the news interface. */
var transient OnlineNewsInterface NewsInterface;

/** Reference to a message box scene. */
var transient UTUIScene_MessageBox MessageBoxReference;

/**
 * Launches a webbrowser to the specify URL
 *
 * @param string	URL		Page to browse to
 */
native function LaunchWebBrowser(string URL);

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	local OnlineSubsystem OnlineSub;

	Super.PostInitialize();

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Community.EpicContent>");

	ContentDataStore = UTUIDataStore_Content(UTUIScene(GetScene()).FindDataStore('UTContent'));

	// Store widget references
	ContentList = UIList(FindChild('lstContent', true));
	if(ContentList != None)
	{
		ContentList.OnSubmitSelection = OnContentList_SubmitSelection;
	}

	StatusLabel = UILabel(FindChild('lblStatus', true));

	// Figure out if we have an online subsystem registered
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		// Grab the news interface to verify the subsystem supports it
		NewsInterface = OnlineSub.NewsInterface;
		if (NewsInterface == None)
		{
			`Log("UTUITabPage_EpicContent::PostInitialize() - Couldn't find news interface!");
		}
	}
}

/** Callback allowing the tabpage to setup the button bar for the current scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	if(ContentList.GetCurrentItem()!=INDEX_NONE)
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.DownloadContent>", OnButtonBar_DownloadContent);
	}
}


/** Starts downloading content from the server. */
function ReadContent()
{
	`Log("UTUITabPage_EpicContent::ReadContent() - Reading content from the web...");

	// Clear content list
	ContentDataStore.AvailableContentProvider.ParseContentString("");
	ContentList.RefreshSubscriberValue();

	// Set placeholder label
	StatusLabel.SetDataStoreBinding("<Strings:UTGameUI.Generic.Loading>");

	// Start news read
	NewsInterface.AddReadNewsCompletedDelegate(OnReadNewsCompleted);
	if(NewsInterface.ReadNews(GetPlayerOwner().ControllerId,ONT_ContentAnnouncements)==false)
	{
		OnReadNewsCompleted(false,ONT_ContentAnnouncements);
	}
}

/**
 * Delegate used in notifying the UI/game that the content announcements read operation completed
 *
 * @param bWasSuccessful true if the read completed ok, false otherwise
 * @param NewsType the type of news this callback is for
 */
function OnReadNewsCompleted(bool bWasSuccessful,EOnlineNewsType NewsType)
{
	`Log("UTUITabPage_EpicContent::OnReadContentAnnouncementsCompleted() - bWasSuccessful: "$bWasSuccessful);

	NewsInterface.ClearReadNewsCompletedDelegate(OnReadNewsCompleted);

	if(bWasSuccessful)
	{
		// Update content datastore
		StatusLabel.SetDataStoreBinding("");
		ContentDataStore.AvailableContentProvider.ParseContentString(NewsInterface.GetNews(GetPlayerOwner().ControllerId,ONT_ContentAnnouncements));
		ContentList.RefreshSubscriberValue();
	}
	else
	{
		StatusLabel.SetDataStoreBinding("<Strings:UTGameUI.Errors.FailedToReadContent>");
	}
}


/** Downloads the selected content file. */
function OnDownloadContent()
{
	local string FinalString;
	local array<string> MessageBoxOptions;

	// Display messagebox to confirm deletion.
	MessageBoxReference = UTUIScene(GetScene()).GetMessageBoxScene();

	if(MessageBoxReference != none)
	{

		FinalString = Localize("MessageBox", "DownloadContent_Message", "UTGameUI");
		MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.DownloadContentAccept>");
		MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Cancel>");

		MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
		MessageBoxReference.Display(FinalString, "<Strings:UTGameUI.MessageBox.DownloadContent_Title>", OnDownloadContent_Confirm, 1);
	}
}


/** Confirmation for the download content dialog. */
function OnDownloadContent_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	local string ContentFileName;

	MessageBoxReference = None;

	if(SelectedItem==0)
	{
		ContentFileName = ContentDataStore.AvailableContentProvider.Packages[ContentList.GetCurrentItem()].ContentName;
		LaunchWebBrowser(ContentFileName);
	}
}

/** Content List - Submit Selection. */
function OnContentList_SubmitSelection( UIList Sender, int PlayerIndex )
{
	OnDownloadContent();
}


/** Buttonbar Callbacks */
function bool OnButtonBar_DownloadContent(UIScreenObject InButton, int PlayerIndex)
{
	OnDownloadContent();

	return true;
}


defaultproperties
{

}
