/**
 * This scene allows the player to view and manage any screenshots they have taken during matches.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_ScreenshotManagement extends GearUISceneWJ_PopupBase
	Config(UI);

/** displays the scene's title */
var	transient	UILabel	lblTitle;

/** displays the "score" text label */
var	transient	UILabel	lblScore;

/** displays the rating value */
var	transient	UILabel	lblScoreRating;

/** the list of screenshots */
var	transient	UIList	lstScreenshots;

/** this holds the screenshot preview image, along with the panel that is displayed while uploading */
var	transient	UIImage	imgScreenshotBorder;

/** the screenshot preview image */
var	transient	UIImage	imgScreenshot;

/** the panel that is displayed while a screenshot is being uploaded (or loaded) */
var	transient	UIPanel	pnlUpload;

/** the text label on the panel while uploading (or loading) */
var	transient	UILabel	lblUploading;

/** the texture that stores a loaded screenshots */
var	transient	Texture2DDynamic		ScreenshotTexture;

/** the unique id of the currently selected screenshot */
var	transient	Guid					CurrentScreenshotID;

/** the device on which the currently selected screenshot exists */
var	transient	int						CurrentScreenshotDeviceID;

/** the raw JPG data for the currently loaded screenshot */
var	transient	array<byte>				CurrentScreenshotImage;

/** the info for the currently loaded screenshot */
var	transient	ScreenshotInfo			CurrentScreenshotInfo;

/** whether or not a screenshot load succeeded */
var transient	bool					bScreenshotLoadSucceeded;

/** reference count for the updating panel */
var transient	int						UpdatingPanelCount;

/** set to true after screenshot enumeration completes */
var transient	bool					bEnumeratedScreenshots;

var	transient	GearUISceneWJ_ScreenshotViewer	ViewScreenshotSceneResource;

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */
function bool IsWidescreen()
{
	local float AspectRatio;

	AspectRatio = GetAspectRatio();

	//`log(`location @ `showvar(AspectRatio) @ `showvar(AspectRatio==ASPECTRATIO_Widescreen,IsWidescreen));
	return AspectRatio == ASPECTRATIO_Widescreen;
}

function bool IsScreenshotWidescreen(int Width, int Height)
{
	local float AspectRatio;
	local float Delta;
	local bool Result;

	Delta = 0.00001;
	Result = false;

	// this is based on UUIScreenObject::GetAspectRatio()
	if(Abs(Height) > Delta)
	{
		AspectRatio = float(Width) / float(Height);
		if(Abs(AspectRatio - ASPECTRATIO_Widescreen) < Delta)
		{
			Result = true;
		}
		//`log(`location @ `showvar(AspectRatio) @ `showvar(Result,IsWidescreen));
	}

	return Result;
}

function ShowThumbnail(Surface Thumbnail, bool bUseWidescreen, EMaterialAdjustmentType AdjustType)
{
	local UIImageAdjustmentData ImageAdjustmentData;

	//`log(`location @ `showvar(bUseWidescreen) @ `showvar(AdjustType));

	SelectObjectsByAspectRatio(bUseWidescreen);

	if(Thumbnail != None)
	{
		imgScreenshot.SetValue(Thumbnail);
		lblScore.SetVisibility(true);
		lblScoreRating.SetVisibility(true);
	}
	else
	{
		// this is the "no screenshot loaded" thumbnail
		imgScreenshot.SetValue(Surface'UI_Art_WarJournal.WJ_SSDefault');
		lblScore.SetVisibility(false);
		lblScoreRating.SetVisibility(false);
	}

	ImageAdjustmentData = imgScreenshot.ImageComponent.StyleCustomization.Formatting[UIORIENT_Horizontal];
	ImageAdjustmentData.AdjustmentType = AdjustType;
	imgScreenshot.ImageComponent.SetFormatting(UIORIENT_Horizontal, ImageAdjustmentData);

	ImageAdjustmentData = imgScreenshot.ImageComponent.StyleCustomization.Formatting[UIORIENT_Vertical];
	ImageAdjustmentData.AdjustmentType = AdjustType;
	imgScreenshot.ImageComponent.SetFormatting(UIORIENT_Vertical, ImageAdjustmentData);
}

function GearScreenshotManager GetScreenshotManager()
{
	local GearPC PC;
	local GearScreenshotManager Result;

	PC = GetGearPlayerOwner(GetPlayerOwnerIndex());
	if ( PC != None && PC.LoadScreenshotManager() )
	{
		Result = PC.ScreenshotManager;
	}

	return Result;
}

function bool GetScreenshot(int Index, out SavedScreenshotInfo out_SSInfo)
{
	local GearUIDataProvider_Screenshots SSProvider;
	local bool bResult;

	SSProvider = GetScreenshotsDataProvider(GetPlayerOwner());
	if ( SSProvider != None && Index >= 0 && Index < SSProvider.Screenshots.Length )
	{
		out_SSInfo = SSProvider.Screenshots[Index];
		bResult = true;
	}

	return bResult;
}

/**
 * Called when the value of this UIObject is changed.  Only called for widgets that contain data values.
 *
 * @param	Sender			the UIObject whose value changed
 * @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 */
function OnScreenshotsIndexChanged( UIObject Sender, int PlayerIndex )
{
`log(`location @ `showvar(lstScreenshots.GetCurrentItem(),SSIndex));
	LoadScreenshotData(lstScreenshots.GetCurrentItem());
}

/**
 * Called when the user presses Enter (or any other action bound to UIKey_SubmitListSelection) while this list has focus.
 *
 * @param	Sender	the list that is submitting the selection
 */
function OnScreenshotsClicked( UIList Sender, optional int PlayerIndex=GetBestPlayerIndex() )
{
	local int SSIndex;
	local GearUIDataProvider_Screenshots SSProvider;
	local GearUISceneWJ_ScreenshotViewer ViewScene;

	if(bScreenshotLoadSucceeded)
	{
		SSProvider = GetScreenshotsDataProvider(GetPlayerOwner(PlayerIndex));
		SSIndex = lstScreenshots.GetCurrentItem();
		if ( SSProvider != None && SSIndex >= 0 && SSIndex < SSProvider.Screenshots.Length )
		{
			ViewScene = GearUISceneWJ_ScreenshotViewer(OpenScene(ViewScreenshotSceneResource, GetPlayerOwner(PlayerIndex)));
			if ( ViewScene != None )
			{
				ViewScene.SetScreenshotInfo(ScreenshotTexture, CurrentScreenshotInfo);
			}
		}
	}
}

function bool LoadScreenshotData( int SSIndex )
{
	local GearUIDataProvider_Screenshots SSProvider;
	local GearScreenshotManager SSManager;
	local bool bResult;

	`log(`location @ `showvar(SSIndex));

	SSProvider = GetScreenshotsDataProvider(GetPlayerOwner());
	SSManager = GetScreenshotManager();

	if ( SSProvider != None && SSManager != None )
	{
		if ( SSIndex >= 0 && SSIndex < SSProvider.Screenshots.Length )
		{
			// load the screenshot
			SSManager.AddLoadScreenshotCompleteDelegate(LoadScreenshotComplete);
			CurrentScreenshotID = SSProvider.Screenshots[SSIndex].ID;
			CurrentScreenshotDeviceID = SSProvider.Screenshots[SSIndex].DeviceID;
			if ( SSManager.LoadScreenshot(CurrentScreenshotID, CurrentScreenshotDeviceID, CurrentScreenshotImage, CurrentScreenshotInfo) )
			{
				ShowUpdatingPanel("Loading");
				bResult = true;
			}
			else
			{
				SSManager.ClearLoadScreenshotCompleteDelegate(LoadScreenshotComplete);
				`log(`location @ "failed to load screenshot" @ SSIndex);
			}
		}
		else
		{
			`log(`location @ "invalid index specified -" @ `showvar(SSIndex) @ `showvar(SSProvider.Screenshots.Length,TotalScreenshots));
		}
	}
	else
	{
		`log(`location @ "no screenshot subsystem present:" @ `showobj(SSProvider) @ `showobj(SSManager));
	}

	UpdateScene();
	return bResult;
}

function LoadScreenshotComplete(bool bWasSuccessful)
{
	local GearScreenshotManager SSManager;
	local bool bIsWidescreen;
	local bool bIsScreenshotCorrupt;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	`log(`location @ `showvar(bWasSuccessful));

	HideUpdatingPanel();

	bScreenshotLoadSucceeded = false;
	bIsScreenshotCorrupt = false;

	SSManager = GetScreenshotManager();
	if ( SSManager != None )
	{
		SSManager.ClearLoadScreenshotCompleteDelegate(LoadScreenshotComplete);
		if ( bWasSuccessful )
		{
			bScreenshotLoadSucceeded = true;

			if(ScreenshotTexture == none)
			{
				//`log("Creating texture");
				ScreenshotTexture = class'Texture2DDynamic'.static.Create(CurrentScreenshotInfo.Width,CurrentScreenshotInfo.Height,PF_A8R8G8B8,TRUE);
			}
			else
			{
				ScreenshotTexture.Init(CurrentScreenshotInfo.Width, CurrentScreenshotInfo.Height, PF_A8R8G8B8, true);
			}

			SSManager.ScreenshotToTexture(CurrentScreenshotImage, ScreenshotTexture);
			//`log("Converted screenshot to texture");

			bIsWidescreen = IsScreenshotWidescreen(CurrentScreenshotInfo.Width,CurrentScreenshotInfo.Height);
			ShowThumbnail(ScreenshotTexture, bIsWidescreen, ADJUST_Justified);

			// update the rating label
			lblScoreRating.SetDataStoreBinding(string(CurrentScreenshotInfo.Rating));
		}
		else
		{
			bIsScreenshotCorrupt = true;
			ShowThumbnail(None, IsWidescreen(), ADJUST_Normal);
		}
	}
	else
	{
		`log(`location @ "no screenshot manager present!");
	}

	UpdateScene();

	if(bIsScreenshotCorrupt)
	{
		ButtonAliases.AddItem('GenericCancel');
		ButtonAliases.AddItem('ConfirmDelete');
		
		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage(
			'ScreenshotCorrupt',
			"<Strings:GearGameUI.MessageBoxStrings.CorruptSS_Title>",
			"<Strings:GearGameUI.MessageBoxStrings.CorruptSS_Question>",
			"",
			ButtonAliases,
			DeleteScreenshotOptionSelected,
			GetPlayerOwner(GetBestPlayerIndex())
		);
	}
}

function bool UploadSelectedScreenshot( optional int PlayerIndex=GetPlayerOwnerIndex() )
{
	return UploadScreenshot(CurrentScreenshotImage, CurrentScreenshotInfo, PlayerIndex);
}

function bool UploadScreenshot( const out array<byte> SSImage, ScreenshotInfo SSInfo, optional int PlayerIndex=GetPlayerOwnerIndex()  )
{
	local GearScreenshotManager SSManager;
	local bool bResult;
	local int ControllerId;

	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
	if ( IsLoggedIn(ControllerId, true) )
	{
		SSManager = GetScreenshotManager();

		if ( SSManager != None )
		{
			// upload the screenshot
			SSManager.AddUploadScreenshotCompleteDelegate(UploadScreenshotComplete);
			if ( SSManager.UploadScreenshot(SSImage, SSInfo) )
			{
				ShowUpdatingPanel("Uploading");
				bResult = true;
			}
			else
			{
				SSManager.ClearUploadScreenshotCompleteDelegate(UploadScreenshotComplete);
				`log(`location @ "failed to upload screenshot");
			}
		}
		else
		{
			`log(`location @ "no screenshot manager present:" @ `showobj(SSManager));
		}

		UpdateScene();
	}
	else
	{
		if ( HasLinkConnection() )
		{
			DisplayErrorMessage("NeedLiveForUpload_Message", "NeedLiveForUpload_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
		}
		else
		{
			DisplayErrorMessage("NoLinkConnection_Message", "NoLinkConnection_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
		}
	}

	return bResult;
}

function ShowScreenshotUploadSuccess()
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('GenericAccept');

	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage('ScreenshotUploadSuccess',
		"<Strings:GearGameUI.MessageBoxStrings.ScreenshotUploaded_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.ScreenshotUploaded_Message>",
		"", ButtonAliases, None, GetPlayerOwner(GetBestPlayerIndex()));
}

/** This is called when an attempt to upload a screenshot completes. */
function UploadScreenshotComplete(bool bWasSuccessful)
{
	local GearScreenshotManager SSManager;

	`log(`location @ `showvar(bWasSuccessful));

	HideUpdatingPanel();

	SSManager = GetScreenshotManager();
	if ( SSManager != None )
	{
		SSManager.ClearUploadScreenshotCompleteDelegate(UploadScreenshotComplete);
	}

	UpdateScene();

	if(bWasSuccessful)
	{
		ShowScreenshotUploadSuccess();
	}
	else
	{
		DisplayErrorMessage("UploadFailed_Message", "UploadFailed_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
	}
}

/**
 * Deletes the screenshot at the specified index
 */
function DeleteSelectedScreenshot( optional int PlayerIndex=GetPlayerOwnerIndex() )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('GenericCancel');
	ButtonAliases.AddItem('ConfirmDelete');

	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage(
		'ConfirmPhotoDelete',
		"<Strings:GearGameUI.MessageBoxStrings.DeleteSS_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.DeleteSS_Question>",
		"",
		ButtonAliases,
		DeleteScreenshotOptionSelected,
		GetPlayerOwner(PlayerIndex)
	);
}

/**
 * Handler for the selection delegate of the screenshot delete confirmation dialog.  Triggers the delete if the user confirmed.
 */
function bool DeleteScreenshotOptionSelected( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local int Selection;

	Sender.CloseScene(Sender, false);

	if ( SelectedInputAlias == 'ConfirmDelete' )
	{
		Selection = lstScreenshots.GetCurrentItem();
		DeleteScreenshot(Selection, PlayerIndex);
	}

	return false;
}

/**
 * Deletes the screenshot at the specified index
 */
function bool DeleteScreenshot( int SSIndex, optional int PlayerIndex=GetPlayerOwnerIndex()  )
{
	local GearUIDataProvider_Screenshots SSProvider;
	local GearScreenshotManager SSManager;
	local bool bResult;
	local int NumScreenshots;

	`log(`location @ `showvar(SSIndex));

	SSProvider = GetScreenshotsDataProvider(GetPlayerOwner(PlayerIndex));
	SSManager = GetScreenshotManager();
	if ( SSProvider != None && SSManager != None )
	{
		NumScreenshots = SSProvider.Screenshots.Length;
		if ( SSIndex >= 0 && SSIndex < NumScreenshots )
		{
			if(SSProvider.DeleteScreenshot(SSIndex))
			{
				bResult = true;
				if(SSIndex < (NumScreenshots - 1))
				{
					LoadScreenshotData(SSIndex);
				}
			}
		}
		else
		{
			`log(`location @ "invalid index specified -" @ `showvar(SSIndex) @ `showvar(SSProvider.Screenshots.Length,TotalScreenshots));
		}
	}
	else
	{
		`log(`location @ "no screenshot subsystem present:" @ `showobj(SSProvider) @ `showobj(SSManager));
	}

	UpdateScene();
	return bResult;
}

/**
 * Updates the state of widgets in the scene based on various conditions.
 *
 * - disables buttonbar buttons when we're executing a task
 * - disables the list of screenshots when executing a task
 */
function UpdateScene()
{
	local int PlayerIndex;
	local bool bValidIndex;

	PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(PlayerOwner.ControllerId);
	bValidIndex = lstScreenshots.Index >= 0 && lstScreenshots.Index < lstScreenshots.GetItemCount();

	//`log(`location @ `showvar(lstScreenshots.Index) @ `showvar(bValidIndex));

	if ( btnbarMain != None )
	{
		btnbarMain.EnableButton('WarJournalNextPage', PlayerIndex, bValidIndex, false);
		btnbarMain.EnableButton('WarJournalPrevPage', PlayerIndex, bValidIndex, false);

		btnbarMain.EnableButton('ScreenshotUpload', PlayerIndex, bValidIndex, false);
		btnbarMain.EnableButton('ScreenshotDelete', PlayerIndex, bValidIndex, false);
		btnbarMain.EnableButton('WarJournalTOC', PlayerIndex, bEnumeratedScreenshots, false);
	}

	if(!bValidIndex)
	{
		ShowThumbnail(None, IsWidescreen(), ADJUST_Normal);
	}

//`log(`location @ `showvar(HasOutstandingAsyncTasks()) @ `showvar(IsFocused(PlayerIndex)) @ `showvar(lstScreenshots.IsEnabled(PlayerIndex)) @ `showvar(lstScreenshots.IsFocused(PlayerIndex)));

	SelectionHintLabel.SetVisibility(!IsBusy());
	lstScreenshots.SetEnabled(!IsBusy(), PlayerIndex);
	if ( lstScreenshots.IsEnabled(PlayerIndex) && IsFocused(PlayerIndex) && !lstScreenshots.IsFocused(PlayerIndex) )
	{
		lstScreenshots.SetFocus(None, PlayerIndex);
	}
}

/**
 * Show the uploading/loading panel over the thumbnail area.
 *
 * @param LocalizeKeyName This is the keyname for the localized string that should be shown on the panel.
 */
function ShowUpdatingPanel(string LocalizeKeyName)
{
	UpdatingPanelCount++;
	lblUploading.SetDataStoreBinding(Localize("GearUI_ScreenshotScene",LocalizeKeyName,"GearGame"));
	pnlUpload.SetVisibility(true);
}

/**
 * Hide the uploading/loading panel over the thumbnail area.
 */
function HideUpdatingPanel()
{
	UpdatingPanelCount--;
	if(UpdatingPanelCount == 0)
	{
		pnlUpload.SetVisibility(false);
	}
}

/**
 * Used to determine if the updating panel is visible or hidden.
 */
function bool IsBusy()
{
	return pnlUpload.IsVisible();
}

/* === GearUISceneWJ_Base interface === */
function SelectObjectsByAspectRatio(bool bUseWidescreen)
{
	local UIImage OtherBorder;

	//`log(`location @ `showvar(bUseWidescreen));

	if ( bUseWidescreen )
	{
		imgScreenshotBorder = UIImage(FindChild('imgScreenshotBorderWide',true));
		OtherBorder = UIImage(FindChild('imgScreenshotBorderSD',true));
	}
	else
	{
		imgScreenshotBorder = UIImage(FindChild('imgScreenshotBorderSD',true));
		OtherBorder = UIImage(FindChild('imgScreenshotBorderWide',true));
	}

	imgScreenshotBorder.SetVisibility(true);
	OtherBorder.SetVisibility(false);

	imgScreenshot = UIImage(imgScreenshotBorder.FindChild('imgScreenshot',true));
	pnlUpload = UIPanel(imgScreenshotBorder.FindChild('pnlUpload',true));
	lblUploading = UILabel(pnlUpload.FindChild('lblUploading',true));
}

/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	Super.InitializeWidgetReferences();

	lstScreenshots = UIList(FindChild('listScreenshots',true));

	lblScore = UILabel(FindChild('lblScore',true));
	lblScoreRating = UILabel(FindChild('lblScoreRating',true));
	lblTitle = UILabel(FindChild('lblTitle',true));

	ShowThumbnail(None, IsWidescreen(), ADJUST_Normal);
}

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	Super.SetupCallbacks();

	lstScreenshots.OnValueChanged = OnScreenshotsIndexChanged;
	lstScreenshots.OnSubmitSelection = OnScreenshotsClicked;

	btnbarMain.SetButtonCallback('WarJournalTOC', CalloutButtonClicked);
	btnbarMain.SetButtonCallback('ScreenshotUpload', CalloutButtonClicked);
	btnbarMain.SetButtonCallback('ScreenshotDelete', CalloutButtonClicked);
	btnbarMain.SetButtonCallback('WarJournalNextPage', CalloutButtonClicked);
	btnbarMain.SetButtonCallback('WarJournalPrevPage', CalloutButtonClicked);
}

/**
 * Worker for CalloutButtonClicked - only called once all conditions for handling the input have been met.  Child classes should
 * override this function rather than CalloutButtonClicked, unless additional constraints are necessary.
 *
 * @param	InputAliasTag	the callout input alias associated with the input that was received
 * @param	PlayerIndex		index for the player that generated the event
 *
 * @return	TRUE if this click was processed.
 */
function bool HandleCalloutButtonClick( name InputAliasTag, int PlayerIndex )
{
	local bool bResult;
	local int PreviousIndex;

	// don't allow button presses during a busy time
	if(IsBusy())
	{
		return true;
	}

	if ( InputAliasTag == 'ScreenshotUpload' )
	{
		if(bScreenshotLoadSucceeded)
		{
			UploadSelectedScreenshot();
		}
		bResult = true;
	}
	else if ( InputAliasTag == 'ScreenshotDelete' )
	{
		DeleteSelectedScreenshot();
		bResult = true;
	}
	else if ( InputAliasTag == 'WarJournalPrevPage' )
	{
		PreviousIndex = lstScreenshots.Index;
		if ( lstScreenshots.NavigateIndex(false, true, false) && PreviousIndex != lstScreenshots.Index )
		{
			lstScreenshots.PlayUISound(lstScreenshots.DecrementIndexCue, PlayerIndex);
		}
		bResult = true;
	}
	else if ( InputAliasTag == 'WarJournalNextPage' )
	{
		PreviousIndex = lstScreenshots.Index;
		if ( lstScreenshots.NavigateIndex(true, true, false) && PreviousIndex != lstScreenshots.Index )
		{
			lstScreenshots.PlayUISound(lstScreenshots.IncrementIndexCue, PlayerIndex);
		}
		bResult = true;
	}

	return bResult || Super.HandleCalloutButtonClick(InputAliasTag, PlayerIndex);
}


/* === UIScene interface === */
event SceneDeactivated()
{
	local GearUIDataProvider_Screenshots SSProvider;
	local GearScreenshotManager SSManager;

	Super.SceneDeactivated();

	SSProvider = GetScreenshotsDataProvider(GetPlayerOwner());
	if ( SSProvider != none )
	{
		SSProvider.Screenshots.Length = 0;
	}

	// free up memory
	CurrentScreenshotImage.Length = 0;
	ScreenshotTexture = None;

	// make sure the content server connection is closed
	SSManager = GetScreenshotManager();
	if ( SSManager != None )
	{
		SSManager.CloseCommunityContentConnection();
	}
}

/**
 * Notification that the login status a player has changed.
 *
 * @param	ControllerId	the id of the gamepad for the player that changed login status
 * @param	NewStatus		the value for the player's current login status
 *
 * @return	TRUE if this scene wishes to handle the event and prevent further processing.
 */
function bool NotifyLoginStatusChanged( int ControllerId, ELoginStatus NewStatus )
{
	local bool bResult;

	`log(`location @ `showvar(NewStatus));

	bResult = Super.NotifyLoginStatusChanged(ControllerId, NewStatus);

	UpdateScene();

	return bResult;
}

/**
 * Notification that the player's connection to the platform's online service is changed.
 */
function NotifyOnlineServiceStatusChanged( EOnlineServerConnectionStatus NewConnectionStatus )
{
	`log(`location @ `showvar(NewConnectionStatus));

	Super.NotifyOnlineServiceStatusChanged(NewConnectionStatus);

	UpdateScene();
}

/**
 * Called when the status of the platform's network connection changes.
 */
function NotifyLinkStatusChanged( bool bConnected )
{
	`log(`location @ `showvar(bConnected));

	Super.NotifyLinkStatusChanged(bConnected);

	UpdateScene();
}

/**
 * Called when a storage device is inserted or removed.
 */
function NotifyStorageDeviceChanged()
{
	CloseScene();
}

/** Updates the Screenshots member with the list of screenshots on disk. */
function EnumerateScreenshots()
{
	local LocalPlayer LP;
	local GearUIDataProvider_Screenshots SSProvider;

	`log(`location);

	LP = GetPlayerOwner();
	if ( LP != None && IsLoggedIn(LP.ControllerId) )
	{
		SSProvider = GetScreenshotsDataProvider(LP);
		if ( SSProvider != None )
		{
			ShowUpdatingPanel("Loading");
			SSProvider.EnumerateScreenshots(EnumScreenshotsComplete);
		}
		else
		{
			`log(`location @ "No screenshot provider found!");
		}
	}
}

/** Delegate called when a screenshot enumeration completes. */
function EnumScreenshotsComplete(bool bWasSuccessful)
{
	local GearScreenshotManager SSManager;

	`log(`location @ `showvar(bWasSuccessful));

	bEnumeratedScreenshots = true;

	HideUpdatingPanel();

	SSManager = GetScreenshotManager();
	if ( SSManager != None )
	{
		SSManager.ClearEnumerateScreenshotsCompleteDelegate(EnumScreenshotsComplete);
	}

	UpdateScene();
}

/**
 * Handler for the completion of this scene's opening animation...
 *
 * @warning - if you override this in a child class, keep in mind that this function will not be called if the scene has no opening animation.
 */
function OnOpenAnimationComplete( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	Super.OnOpenAnimationComplete(Sender, AnimName, TrackTypeMask);

	if ( TrackTypeMask == 0 )
	{
		EnumerateScreenshots();
	}

	UpdateScene();
}

/* === UIScreenObject interface === */
/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	local int PlayerIndex;

	Super.PostInitialize();

	// hide these labels initially.
	lblScore.SetVisibility(false);
	lblScoreRating.SetVisibility(false);

	// disable buttons until loading completes
	if ( btnbarMain != None )
	{
		PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(PlayerOwner.ControllerId);
		btnbarMain.EnableButton('WarJournalNextPage', PlayerIndex, false, false);
		btnbarMain.EnableButton('WarJournalPrevPage', PlayerIndex, false, false);
		btnbarMain.EnableButton('ScreenshotUpload', PlayerIndex, false, false);
		btnbarMain.EnableButton('ScreenshotDelete', PlayerIndex, false, false);
		btnbarMain.EnableButton('WarJournalTOC', PlayerIndex, false, false);
	}
}

/**
 * Generates a array of UI input aliases that this widget supports.
 *
 * @param	out_KeyNames	receives the list of input alias names supported by this widget
 */
event GetSupportedUIActionKeyNames( out array<Name> out_KeyNames )
{
	Super.GetSupportedUIActionKeyNames(out_KeyNames);
}

DefaultProperties
{
	bRequiresProfile=true
	ViewScreenshotSceneResource=GearUISceneWJ_ScreenshotViewer'UI_Scenes_WarJournal.ScreenshotViewer'

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object
}
