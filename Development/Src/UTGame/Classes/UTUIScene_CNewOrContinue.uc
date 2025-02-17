/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUIScene_CNewOrContinue extends UTUIFrontEnd;

var transient UTSimpleMenu Menu;
var string OptionsSceneTemplate;
var transient bool bIsNewMission;
var transient bool bSelectingChapter;
var UTSimpleList ChapterList;
var UIPanel ChapterPanel;

/** Reference to the description label for the menu. */
var transient UILabel DescriptionLabel;

var transient int ChapterLocked[5];
var array<string> OptionDescriptions;
var transient int ChapterToLoad;


event PostInitialize( )
{
	Super.PostInitialize();

	Menu = UTSimpleMenu( FindChild('SimpleMenu',true) );
	Menu.OnSelectionChange = OnMenuOptionChanged;
	Menu.bHotTracking=true;	// Allow hottracking

	DescriptionLabel = UILabel(FindChild('Description', true));
	ChapterPanel = UIPanel( FindCHild('ChapterPanel',true));
	ChapterList = UTSimpleList( FindChild('ChapterList',true));
    ChapterList.OnDrawItem = None;//DrawChapterItem;
    ChapterList.OnDrawSelectionBar = none;//DrawChapterSelection;

	Menu.OnItemChosen = ItemChosen;
	ChapterList.OnItemChosen = ItemChosen;
	Setup();
}

/** Callback for when the menu option changes. */
function OnMenuOptionChanged(UTSimpleList SourceList, int NewSelectedIndex)
{
	if(bSelectingChapter==false)
	{
		DescriptionLabel.SetDataStoreBinding(OptionDescriptions[Menu.List[Menu.Selection].Tag]);
	}
	else
	{
		DescriptionLabel.SetDataStoreBinding("");
	}
}

function Setup()
{
	local string markup;
	local UTProfileSettings Profile;
	local int i;

	Profile = GetPlayerProfile(0);

	ChapterList.Empty();
	for (i=0;i<5;i++)
	{
		Markup = "<Strings:UTGameUI.Campaign.Chapter"$string(i)$">";
		ChapterList.AddItem(Markup,i);
		ChapterLocked[i] = int( Profile.IsChapterUnlocked(i));
	}


	Menu.Empty();
	Menu.AddItem("<Strings:UTGameUI.Campaign.CampaignStartNew>",0);

	if ( Profile.bGameInProgress() )
	{
		Menu.AddItem("<Strings:UTGameUI.Campaign.CampaignContinue>",1);
	}

	if ( Profile.AreAnyChaptersUnlocked() )
	{
     	Menu.AddItem("<Strings:UTGameUI.Campaign.CampaignChapter>",3);
    }


	Menu.AddItem("<Strings:UTGameUI.Campaign.CampaignOnline>",2);

	Menu.SelectItem(0);
}


/** Sets up the button bar for the scene. */
function SetupButtonBar()
{
	if(ButtonBar != None)
	{
		ButtonBar.Clear();
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Accept>", OnButtonBar_Accept);
	}
}


function ItemChosen(UTSimpleList SourceList, int SelectedIndex, int PlayerIndex)
{
	local UTUIScene_MessageBox MB;
	local UTProfileSettings Profile;

	Profile = GetPlayerProfile(0);

	if (bSelectingChapter)
	{
		if ( ChapterLocked[ChapterList.Selection] == 0 )
		{
			DisplayMessageBox("<strings:UTGameUI.Campaign.ChapterLockedMsg>","<strings:UTGameUI.Campaign.ChapterLockedTitle>");
		}
		else
		{
			ChapterToLoad = ChapterList.Selection;
			if ( Profile.bGameInProgress() )
			{
				MB = GetMessageBoxScene();
				if (MB!=none)
				{
					MB.DisplayAcceptCancelBox("<Strings:UTGameUI.Campaign.ChapterWarning>","<Strings:UTGameUI.Campaign.Confirmation", MB_Selection);
				}
			}
			else
			{
				GotoOptions(true);
			}
		}
		return;
	}

	switch ( SourceList.GetTag() )
	{
		case INDEX_None:
			break;

		case 0:
			if ( Profile.bGameInProgress() )
			{
				MB = GetMessageBoxScene();
				if (MB!=none)
				{
					MB.DisplayAcceptCancelBox("<Strings:UTGameUI.Campaign.NewWarning>","<Strings:UTGameUI.Campaign.Confirmation", MB_Selection);
				}
			}
			else
			{
				GotoOptions(true);
			}
			break;

		case 1:
			GotoOptions(false);
			break;

		case 2: //	Server Browser
			if ( CheckLinkConnectionAndError() && CheckOnlinePrivilegeAndError() )
			{
				OpenSceneByName(class'UTUIFrontEnd_Multiplayer'.default.JoinScene,,JoinOpen);
			}
			break;

		case 3: // Chapter
			Menu.PlayUIAnimation('FadeOut',,,3.0);
			ChapterPanel.SetVisibility(true);
			ChapterPanel.PlayUIAnimation('FadeIn',,,3.0);
			ChapterList.SetFocus(none);
			bSelectingChapter = true;
			DescriptionLabel.SetDataStoreBinding("");
	}
}

function GotoOptions(bool bNewMission)
{
	bIsNewMission = bNewMission;
    UTUIScene_COptions( OpenSceneByName(OptionsSceneTemplate,, OptionsOpen) );
}

function OptionsOpen(UIScene OpenedScene, bool bInitialActivation)
{
	local UTUIScene_COptions OptionsScene;
	OptionsScene = UTUIScene_COptions(OpenedScene);
	if ( OptionsScene != none && bInitialActivation )
	{
		OptionsScene.Configure(bIsNewMission,bSelectingChapter ? ChapterToLoad : INDEX_None);
	}
}

function JoinOpen(UIScene OpenedScene, bool bInitialActivation)
{
	local UTUIFrontEnd_JoinGame JoinScene;
	JoinScene = UTUIFrontEnd_JoinGame(OpenedScene);
	if ( JoinScene != none && bInitialActivation )
	{
		JoinScene.UseCampaignMode();
	}
}

function MB_Selection(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	if (SelectedOption == 0)
	{
		GotoOptions(true);
	}
}

/**
 * Call when it's time to go back to the previous scene
 */
function bool Scene_Back()
{

	if (bSelectingChapter)
	{
		bSelectingChapter = false;
		Menu.PlayUIAnimation('FadeIn',,,3.0);
		ChapterPanel.PlayUIAnimation('FadeOut',,,3.0);
		Menu.SetFocus(none);
		return true;
	}

	CloseScene(self);
	return true;
}

/**
 * Button bar callbacks - Back Button
 */
function bool OnButtonBar_Back(UIScreenObject InButton, int InPlayerIndex)
{
	return Scene_Back();
}

function bool OnButtonBar_Accept(UIScreenObject InButton, int InPlayerIndex)
{
	ItemChosen(Menu,Menu.Selection,InPlayerIndex);
	return true;
}

function bool HandleInputKey( const out InputEventParameters EventParms )
{
	local bool bResult;

	bResult=false;


	if(EventParms.EventType==IE_Released)
	{
		if(EventParms.InputKeyName=='XboxTypeS_B' || EventParms.InputKeyName=='Escape')
		{
			Scene_Back();
			bResult=true;
		}

		if(EventParms.InputKeyName=='XboxTypeS_A')
		{
			ItemChosen(Menu,Menu.Selection,EventParms.PlayerIndex);
		}

	}

	return bResult;
}

defaultproperties
{
	OptionsSceneTemplate="UI_Scenes_Campaign.Scenes.CampOptions"
	OptionDescriptions.Add("<Strings:UTGameUI.Campaign.CampaignStartNew_Desc>")
	OptionDescriptions.Add("<Strings:UTGameUI.Campaign.CampaignContinue_Desc>")
	OptionDescriptions.Add("<Strings:UTGameUI.Campaign.CampaignOnline_Desc>")
	OptionDescriptions.Add("<Strings:UTGameUI.Campaign.CampaignChapter_Desc>")
}
