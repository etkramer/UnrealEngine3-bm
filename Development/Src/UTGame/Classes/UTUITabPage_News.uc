/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for downloaded news.
 */
class UTUITabPage_News extends UTTabPage
	placeable;

/**  Reference to the news label. */
var transient UILabel	NewsLabel;

/** Reference to the news interface. */
var transient OnlineNewsInterface NewsInterface;

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	local OnlineSubsystem OnlineSub;

	Super.PostInitialize();

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Community.News>");

	// Find a news label
	NewsLabel = UILabel(FindChild('lblNews', true));

	// Figure out if we have an online subsystem registered
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		// Grab the news interface to verify the subsystem supports it
		NewsInterface = OnlineSub.NewsInterface;
		if (NewsInterface == None)
		{
			`Log("UTUITabPage_News::PostInitialize() - Couldn't find news interface!");
		}
	}

	if ( IsLoggedIn(GetBestControllerId(), true) )
	{
		// Start a news read
		ReadNews();
	}
}

/** Starts reading the latest news. */
function ReadNews()
{
	`Log("UTUITabPage_News::ReadNews() - Reading news from the web...");

	// Set placeholder label
	NewsLabel.SetDataStoreBinding("<Strings:UTGameUI.Generic.Loading>");

	// Start news read
	NewsInterface.AddReadNewsCompletedDelegate(OnReadGameNewsCompleted);
	if(NewsInterface.ReadNews(GetPlayerOwner().ControllerId,ONT_GameNews)==false)
	{
		OnReadGameNewsCompleted(false,ONT_GameNews);
	}
}

/**
 * Delegate used in notifying the UI/game that the news read operation completed
 *
 * @param bWasSuccessful true if the read completed ok, false otherwise
 * @param NewsType the type of news this callback is for
 */
function OnReadGameNewsCompleted(bool bWasSuccessful,EOnlineNewsType NewsType)
{
	`Log("UTUITabPage_News::OnReadGameNewsCompleted() - bWasSuccessful: "$bWasSuccessful);

	if ( NewsInterface != None )
	{
		NewsInterface.ClearReadNewsCompletedDelegate(OnReadGameNewsCompleted);
	}

	if(bWasSuccessful)
	{
		NewsLabel.SetDataStoreBinding(NewsInterface.GetNews(GetPlayerOwner().ControllerId,ONT_GameNews));
	}
	else
	{
		NewsLabel.SetDataStoreBinding("<Strings:UTGameUI.Errors.FailedToReadNews>");
	}
}


/**
 * Notification that this widget's parent is about to remove this widget from its children array.  Allows the widget
 * to clean up any references to the old parent.
 *
 * @param	WidgetOwner		the screen object that this widget was removed from.
 */
event RemovedFromParent( UIScreenObject WidgetOwner )
{
	Super.RemovedFromParent(WidgetOwner);

	if ( NewsInterface != None )
	{
		NewsInterface.ClearReadNewsCompletedDelegate(OnReadGameNewsCompleted);
	}
}

defaultproperties
{

}
