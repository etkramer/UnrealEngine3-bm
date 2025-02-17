/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFE_LobbyPlayerOptions extends GearUISceneFrontEnd_Base
	Config(UI);


/************************************************************************/
/* Constants, structures, enums, etc.									*/
/************************************************************************/

/** Enum of possible options on the player options screen */
enum EGearPlayerOptions
{
	eGPOPTION_Gamercard,
	eGPOPTION_Feedback,
	eGPOPTION_Kick,
	eGPOPTION_Cancel,
};


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** List of options this screen is to show */
var transient array<EGearPlayerOptions> OptionTypes;

/** The PRI of the player these options are for */
var transient GearPRI PlayerPRI;

/** The PRI of the person who brought the screen up */
var transient GearPRI OwnerPRI;

/** References to the labe buttons in the scene */
var transient array<UILabelButton> OptionButtons;

/** Reference to the label that displays the player's name */
var transient UILabel PlayerNameLabel;

/** Reference to the label that displays the player's rank */
var transient UILabel PlayerRankLabel;

/** Reference to the image of the player's playercard portrait */
var transient UIImage PlayerImage;

/** Whether the lobby is currently in a LIVE enabled state */
var transient bool bIsLiveEnabled;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/


/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	Super.PostInitialize();

	if ( !IsEditor() )
	{
		InitializeWidgetReferences();
	}
}

/** Assigns all member variables to appropriate child widget from this scene */
function InitializeWidgetReferences()
{
	// Grab references to the option buttons
	OptionButtons.length = eGPOPTION_MAX;
	OptionButtons[eGPOPTION_Gamercard] = UILabelButton(FindChild('btnOption1',true));
	OptionButtons[eGPOPTION_Feedback] = UILabelButton(FindChild('btnOption2',true));
	OptionButtons[eGPOPTION_Kick] = UILabelButton(FindChild('btnOption3',true));
	OptionButtons[eGPOPTION_Cancel] = UILabelButton(FindChild('btnCancel',true));

	// Get references to the other widgets for the scene
	PlayerNameLabel = UILabel(FindChild('lblName',true));
	PlayerRankLabel = UILabel(FindChild('lblRank',true));
	PlayerImage = UIImage(FindChild('imgGamerPic',true));
}

/** Initialize the widgets using the data from the setup function */
function InitializePlayerOptionsScene()
{
	local int UseIdx;
	local UniqueNetId ZeroId;
	local int PlayerIndex;
	local int OptionToFocus;

	if ( !IsEditor() )
	{
		PlayerIndex = GetBestPlayerIndex();
		OptionToFocus = -1;

		UseIdx = OptionTypes.Find( eGPOPTION_Gamercard );
		if ( UseIdx != INDEX_NONE )
		{
			if (PlayerPRI.UniqueId == ZeroId || !bIsLiveEnabled)
			{
				if (OptionButtons[eGPOPTION_Gamercard].IsFocused())
				{
					OptionButtons[eGPOPTION_Gamercard].KillFocus(none, PlayerIndex);
				}
				OptionButtons[eGPOPTION_Gamercard].DisableWidget(PlayerIndex);
			}
			else
			{
				OptionButtons[eGPOPTION_Gamercard].OnClicked = OnGamercardOptionClicked;
				OptionToFocus = eGPOPTION_Gamercard;
			}
		}

		UseIdx = OptionTypes.Find( eGPOPTION_Feedback );
		if ( UseIdx != INDEX_NONE )
		{
			if (PlayerPRI == OwnerPRI || PlayerPRI.UniqueId == ZeroId || !bIsLiveEnabled)
			{
				if (OptionButtons[eGPOPTION_Feedback].IsFocused())
				{
					OptionButtons[eGPOPTION_Feedback].KillFocus(none, PlayerIndex);
				}
				OptionButtons[eGPOPTION_Feedback].DisableWidget(PlayerIndex);
			}
			else
			{
				OptionButtons[eGPOPTION_Feedback].OnClicked = OnFeedbackOptionClicked;
				OptionToFocus = (OptionToFocus == -1) ? eGPOPTION_Feedback : OptionToFocus;
			}
		}

		UseIdx = OptionTypes.Find( eGPOPTION_Kick );
		if ( UseIdx != INDEX_NONE )
		{
			if ( PlayerController(PlayerPRI.Owner).IsLocalPlayerController() )
			{
				if (OptionButtons[eGPOPTION_Kick].IsFocused())
				{
					OptionButtons[eGPOPTION_Kick].KillFocus(none, PlayerIndex);
				}
				OptionButtons[eGPOPTION_Kick].DisableWidget(PlayerIndex);
			}
			else
			{
				OptionButtons[eGPOPTION_Kick].OnClicked = OnKickPlayerOptionClicked;
				OptionToFocus = (OptionToFocus == -1) ? eGPOPTION_Kick : OptionToFocus;
			}
		}
		else
		{
			OptionButtons[eGPOPTION_Kick].SetVisibility( false );
			OptionButtons[eGPOPTION_Cancel].SetDockTarget( UIFACE_Top, OptionButtons[eGPOPTION_Feedback], UIFACE_Bottom );
		}

		OptionButtons[eGPOPTION_Cancel].OnClicked = OnCancelOptionClicked;
		OptionToFocus = (OptionToFocus == -1) ? eGPOPTION_Cancel : OptionToFocus;

		PlayerNameLabel.SetDataStoreBinding( PlayerPRI.PlayerName );
		PlayerRankLabel.SetDataStoreBinding( GetPlayerSkillString(PlayerPRI.PlayerSkill) );

		OptionButtons[OptionToFocus].SetFocus(none);
	}
}

/** Sets up the scene using the information from the screen that called it */
final function SetupOptionsVariables( GearPRI SceneOwnerPRI, GearPRI SelectedPRI, array<EGearPlayerOptions> Options, bool bLiveEnabled )
{
	local int Idx;

	OwnerPRI = SceneOwnerPRI;
	PlayerPRI = SelectedPRI;
	bIsLiveEnabled = bLiveEnabled;

	OptionTypes.length = 0;
	for ( Idx = 0; Idx < Options.length; Idx++ )
	{
		OptionTypes.AddItem( Options[Idx] );
	}

	InitializePlayerOptionsScene();
}

/** Called when someone selects the gamercard option */
function bool OnGamercardOptionClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local int ControllerId;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None )
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if ( PlayerIntEx != None )
		{
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
			PlayerIntEx.ShowGamerCardUI( ControllerId, PlayerPRI.UniqueId );
		}
	}

	return true;
}

/** Called when someone selects the feedback option */
function bool OnFeedbackOptionClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local int ControllerId;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None )
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if ( PlayerIntEx != None )
		{
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
			PlayerIntEx.ShowFeedbackUI( ControllerId, PlayerPRI.UniqueId );
		}
	}

	return true;
}

/** Called when someone selects the kicks player option */
function bool OnKickPlayerOptionClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('GenericCancel');
	ButtonAliases.AddItem('AcceptKick');
	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage('KickPlayer_Title',
		"<Strings:GearGameUI.MessageBoxStrings.KickPlayer_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.KickPlayer_Msg>",
		"",
		ButtonAliases, OnKick_Confirmed, GetPlayerOwner(PlayerIndex));

	return true;
}

/** Called when someone confirms kicking another player */
function bool OnKick_Confirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local GearPC PC;

	if ( SelectedInputAlias == 'AcceptKick' )
	{
		// Tell the player to return to their main menu rather than just close their connection
		PC = GearPC(PlayerPRI.Owner);
		if (PC != None)
		{
			// Tell the client we kicked them
			PC.ClientSetProgressMessage(PMT_ConnectionFailure,
				"<Strings:GearGameUI.MessageBoxStrings.PlayerWasKicked_Desc>",
				"<Strings:GearGameUI.MessageBoxStrings.KickedForPartyKill_Title>",
				true);
			PC.ClientReturnToMainMenu();
		}
		CloseScene(self);
	}

	return true;
}

/** Called when someone selects the cancel option */
function bool OnCancelOptionClicked(UIScreenObject EventObject, int PlayerIndex)
{
	CloseScene(self);
	return true;
}


defaultproperties
{
}
