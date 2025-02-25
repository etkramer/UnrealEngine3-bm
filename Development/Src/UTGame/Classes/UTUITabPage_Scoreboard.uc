/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUITabPage_Scoreboard extends UTTabPage_MidGame;


var transient array<UTScoreboardPanel> Scoreboards;
var transient UTScoreInfoPanel InfoPanel;
var int SelectedPI;

function PostInitialize()
{
	local Name PanelTag;
	local class<UTGame> GameClass;

	Super.PostInitialize();

	InfoPanel = UTScoreInfoPanel( FindChild('InfoPanel',true));

	if ( GetScene().GetWorldInfo().GRI.GameClass.Default.bTeamGame )
	{
		InfoPanel.SetPosition( 0.247424, UIFACE_Left, EVALPOS_PercentageOwner);
		InfoPanel.SetPosition( 0.002756, UIFACE_Top, EVALPOS_PercentageOwner);
		InfoPanel.SetPosition( 0.505429, UIFACE_Right, EVALPOS_PercentageOwner);
		InfoPanel.SetPosition( 0.167105, UIFACE_Bottom, EVALPOS_PercentageOwner);
	}

	GameClass = Class<UTGame>( GetScene().GetWorldInfo().GRI.GameClass);
	PanelTag = GameClass.Default.MidgameScorePanelTag;
	FindScoreboards(PanelTag);

	OnRawInputKey=None;

	OnInitialSceneUpdate = RenderCallBack;

}

function RenderCallBack()
{
	local rotator R;
	if ( InfoPanel != none && GetScene().GetWorldInfo().GRI.GameClass.Default.bTeamGame )
	{
		R.Roll = 2736.6666; // 15 degrees
		InfoPanel.RotateWidget(r);
	}
}

function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	local WorldInfo WI;
	local UTGameReplicationInfo GRI;

	Super.SetupButtonBar(ButtonBar);

	WI = GetScene().GetWorldInfo();
	GRI = UTGameReplicationInfo(WI.GRI);
	if ( GRI != None && GRI.CanChangeTeam() )
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ChangeTeam>", OnChangeTeam);
	}

	if (WI.NetMode != NM_StandAlone)
	{
		if ( UTUIScene(GetScene()).GetPRIOwner().bAdmin )
		{
			ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.PlayerAdmin>", OnPlayerDetails);
		}
		else
		{
			ButtonBar.AppendButton( "<Strings:UTGameUI.ButtonCallouts.Details>", OnPlayerDetails);
		}
	}

}

function FindScoreboards(name PanelTagName)
{
	local int i;
	local array<UIObject> Kids;
	local UTScoreboardPanel SB;
	local UIPanel Panel;

	Panel = UIPanel( FindChild(PanelTagName,true));
	if ( Panel == none )
	{
		`log("ERROR: Could not find Scoreboard panel ["$PanelTagName$"] so MidGame Scoreboard is broken.");
		return;
	}

	Panel.SetVisibility(true);
	Kids = Panel.GetChildren(true);

	for (i=0;i<Kids.Length;i++)
	{
		SB = UTScoreboardPanel(Kids[i]);
		if ( SB != none )
		{
			Scoreboards[Scoreboards.Length] = SB;
			SB.OnSelectionChange = OnScoreboardSelectionChange;
		}
	}
}


function OnScoreboardSelectionChange(UTScoreboardPanel TargetScoreboard, UTPlayerReplicationInfo PRI)
{
    SelectedPI = PRI.PlayerId;
}

function UTPlayerReplicationInfo GetSelectedPRI()
{
	local int i;
	local WorldInfo WI;
	WI = UTUIScene(GetScene()).GetWorldInfo();
	for (i=0;i<WI.GRI.PRIArray.Length;i++)
	{
		if (WI.GRI.PRIArray[i].PlayerID == SelectedPI)
		{
			return UTPlayerReplicationInfo(WI.GRI.PRIArray[i]);
		}
	}
	return none;
}

function bool OnPlayerDetails(UIScreenObject InButton, int InPlayerIndex)
{
	local UTUIScene_PlayerCard PCScene;
	local UTUIScene Scene;
	local bool bIsAdmin, bIsLocal, bkick, bban;
	local UTPlayerController PC;
	local WorldInfo WI;
	local string PName, PAlias;
	local UTPlayerReplicationInfo SelectedPRI;

	Scene = UTUIScene(GetScene());
	WI = Scene.GetWorldInfo();
	PC = Scene.GetUTPlayerOwner();
	bIsAdmin = PC.PlayerReplicationInfo.bAdmin;
	bIsLocal = WI.NetMode == NM_Standalone || WI.NetMode == NM_ListenServer;

	SelectedPRI = GetSelectedPRI();
	if ( SelectedPRI != None && SelectedPRI != PC.PlayerReplicationInfo  )
	{
		if ( SelectedPRI.bBot )
		{
			Scene.DisplayMessageBox ("<Strings:UTGameUI.MidGameMenu.DetailsWithBots>");
			return true;
		}

		bKick = bIsAdmin || bIsLocal;
		bBan  = bKick && !SelectedPRI.bBot;

		PName = SelectedPRI.PlayerName;
		PAlias = SelectedPRI.GetPlayerAlias();

		PCScene = Scene.ShowPlayerCard(SelectedPRI.UniqueId, PAlias, (PName == PAlias) ? "" : PName,bKick,bBan);	// FIXME - Add account name for PC
		if ( PCScene != none )
		{
			PCScene.OnSceneDeactivated = DetailsIsDone;
		    Scene.FindChild('MidGameSafeRegion',true).PlayUIAnimation('FadeOut',,,3.0);
		}
	}
	else
	{
		Scene.DisplayMessageBox ("<Strings:UTGameUI.MidGameMenu.CantViewDetails>");
	}

	return true;
}

function DetailsIsDone( UIScene DeactivatedScene )
{
	GetScene().FindChild('MidGameSafeRegion',true).PlayUIAnimation('FadeIn',,,3.0);
}


function bool OnChangeTeam(UIScreenObject InButton, int InPlayerIndex)
{
	local LocalPlayer LP;

   	LP = GetPlayerOwner(InPlayerIndex);
	if ( LP != none && LP.Actor != none )
	{
		LP.Actor.ChangeTeam();
		CloseParentScene();
	}
	return true;
}


/**
 * Setup Input subscriptions
 */
event GetSupportedUIActionKeyNames(out array<Name> out_KeyNames )
{
	out_KeyNames[out_KeyNames.Length] = 'CloseScoreboard';
}

function bool HandleInputKey( const out InputEventParameters EventParms )
{
	local WorldInfo WI;
	local UTGameReplicationInfo GRI;

	if ( EventParms.InputKeyName == 'F1' )
	{
		CloseParentScene();
		return true;
	}


	if (EventParms.EventType == IE_Released)
	{
		if (EventParms.InputKeyName == 'XboxtypeS_X')
		{
			WI = GetScene().GetWorldInfo();
			GRI = UTGameReplicationInfo(WI.GRI);
			if ( GRI != None && GRI.CanChangeTeam() )
			{
				OnChangeTeam(none, EventParms.PlayerIndex);
				return true;
			}
		}
		else if (EventParms.InputKeyName =='XboxtypeS_A')
		{
			OnPlayerDetails(none, EventParms.PlayerIndex);
			return true;
		}
	}
	return true;
}

function NotifyGameSessionEnded()
{
	SelectedPI = INDEX_None;
}

defaultproperties
{
	SelectedPI = -1;
}
