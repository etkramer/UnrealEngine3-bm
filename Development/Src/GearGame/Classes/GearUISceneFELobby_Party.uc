/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFELobby_Party extends GearUISceneFELobby_Base
	ClassRedirect(GearUISceneFEParty_Versus)
	Config(inherit);

`include(GearOnlineConstants.uci)

const NumMatchmakeSteps = 3;

/************************************************************************/
/* Constants, structures, enums, etc.									*/
/************************************************************************/

/** Structure containing all data required for each player in the player list */
struct GearPartyPlayerData
{
	/** Label button of the player */
	var transient UILabelButton ParentButton;

	/** Label on the left edge which determines focus and profile */
	var transient UILabel ProfileLabel;

	/** Label which displays the rank of the player */
	var transient UILabel RankLabel;

	/** Chat bubble image of the player */
	var transient UIImage ChatIcon;

	/** The PRI of the player */
	var transient GearPartyPRI PlayerPRI;
};

/** The state of a line in the matchmaking progress panel */
enum StepState
{
	SS_Disabled,
	SS_Active,
	SS_Complete,
	SS_Hidden
};

/** The currently displayed matchmaking step */
var transient EMatchmakingState CurDisplayedMatchmakingStep;

/**
 * Widget references for visualizing a single step in the matchmaking process.
 * i.e. Current step, and checkbox/cog progress indicator for that step.
 */
struct MatchmakingStepWidgets
{
	var transient UILabel LabelStepName;
	var transient UICheckbox Checkbox;
	var transient UIImage Cog;
};

/**
 * Widget references for visualizing the current matchmaking progress.
 */
struct PanelMatchmakingProgress
{
	var transient UIPanel Panel;
	var transient UILabel LabelPlaylist;
	var transient MatchmakingStepWidgets Steps[NumMatchmakeSteps];
	var transient UIImage Background;
};

/** Enum determining the whether a party update is occurring or not */
enum EPartyUpdateType
{
	ePUT_Started,
	ePUT_NoUpdate,
	ePUT_Pending,
};


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Localized strings */
var localized string PlaylistTitlePrefix;
var localized string PlaylistTitleSuffix;
var localized string OpenPlayerSlot;
var localized string InvitablePlayerSlot;
var localized string FoundPlayerSlot;
var localized string SearchPlayerSlot;
var localized string ClosedPlayerSlot;
var localized string ConnectingPlayerSlot;
var localized string MainButtonFindTeam;
var localized string MainButtonFindMatch;
var localized string MainButtonIsMatchmaking;
var localized string MainButtonIsCancelling;
var localized string MainButtonCreateMatch;
var localized string MatchmakeCancelMsg;
var localized string MatchmakeReadSkillsMsg;
var localized string MatchmakeFindBestMsg;
var localized string MatchmakeFindAnyMsg;
var localized string MatchmakeFindOpponent;
var localized string MatchmakeConnectOpponent;
var localized string MatchErrorTooManyPlayers;
var localized string GameErrorTooManyPlayers;
var localized string MatchErrorMissingDLC;
var localized string MatchErrorNoGuestInPublic;
var localized string MatchErrorMissingFlashback;
var localized string MainButtonDoMatchmake_Desc;
var localized string MainButtonIsMatchmaking_Desc;
var localized string MainButtonIsCancelling_Desc;
var localized string MainButtonCreateMatch_Desc;
var localized string PlayerClickFull_Desc;
var localized string PlayerClickEmpty_Desc;
var localized string MainButtonIsConnecting;
var localized string MainButtonIsConnecting_Desc;
var localized string MatchmakeHostCancelled;
var localized string BotSlotString;
var localized string SearchingForTeammates;

var localized string MatchModeDesc_Public;
var localized string MatchModeDesc_Private;
var localized string MatchModeDesc_LAN;
var localized string MatchModeDesc_Local;
var localized string MapSelectDesc_Vote;
var localized string MapSelectDesc_Host;
var localized string WeapSelectDesc_Cycle;
var localized string WeapSelectDesc_Normal;
var localized string WeapSelectDesc_Custom;
var localized string GameType_Desc;
var localized string MapSelectType_Desc;
var localized string WeapSwap_Desc;

/** the item that allows the user to choose the desired gametype */
var	transient GearUIObjectList lstPartyOptions;

var transient UIImage PlaylistDescriptionBackground;

/** the list of gametype settings */
var	transient GearUIObjectList lstGameOptions;
/** the image behind the game options */
var	transient UIImage imgGameOptionsBG;

var	transient UICalloutButtonPanel btnbarPlayers;
var	transient UICalloutButtonPanel btnbarPlayers2;
var transient UICalloutButtonPanel btnbarLAN;

var	transient UIPanel pnlOptions;

/** Resource reference to the weapon swap scene */
var	transient GearUISceneFE_MPWeaponSwap WeaponSwapScene;

/** Parent panel of the player list */
var	transient UIPanel pnlPlayers;
/** List of player data for the party lobby */
var transient array<GearPartyPlayerData> PlayerData;
/** Background image of the player list */
var transient UIImage PlayerBackgroundImage;

/** Parent image widget for playlist descriptions */
var transient UIImage ParentPlaylistDescribeImage;
/** Playlist title label */
var transient UILabel PlaylistTitleLabel;
/** Playlist game mode description label */
var transient UILabel PlaylistDescribeLabel;

/** The main button label that begins/cancels matchmaking */
var transient UILabelButton MatchmakeButton;
/** The image that appears in front of the matchmake button */
var transient UILabel MatchmakeButtonIcon;
/** Normal COG symbol when nothing is going on */
var transient UIImage COGSymbolNormal;
/** COG symbol when matchmaking is taking place */
var transient UIImage COGSymbolMatchmaking;
/** COG symbol when something is wrong and the button is not usable */
var transient UIImage COGSymbolBroken;
/** The blood splatter image in the background */
var transient UIImage BloodSplat;

/** Visualises the current step in the matchmaking process */
var transient PanelMatchmakingProgress PnlMatchmakeProgress;

/** How many teammates are there in the scene */
var transient int TeammateCount;


/** Parent panel of the party message */
var transient UIPanel PartyMsgPanel;
/** Text label for the party message */
var transient UILabel PartyMsgLabel;
/** Image background of the party message */
var transient UIImage PartyMsgImage;

/** Images used for special backgrounds */
var Surface MatchRedBG;
var Surface MatchOrangeBG;
var Surface MatchGrayBG;

/** Scene resource reference to the What's Up screen */
var UIScene WhatsUpSceneReference;

/** Whether we're waiting for the matchmaking process to begin or not (this is a hack since there is an async wait from the
    time we click the button till the time matchmaking begins )*/
var transient bool bWaitingForMatchmakeToBegin;

/** The previous value of the match mode */
var transient int PreviousMatchMode;

/** Whether the player chose to start a LAN party when on the LAN screen */
var transient bool bWantsToStartLANParty;
/** Whether the player chose to cancel from the LAN scene or not */
var transient bool bCanceledLANScene;

/** the maximum number of items to display at once in the drop-down lists */
var(Controls)	int		MaxVisibleListItems;

/** The matchmode widget */
var transient GearUICollapsingSelectionList MatchModeList;

/** The horde wave widget */
var transient GearUICollapsingSelectionList HordeWaveList;

/** The map selections widget */
var transient GearUICollapsingSelectionList MapSelectionList;

/** The Weapon swapping widget */
var transient GearUICollapsingSelectionList WeaponSwapList;

/** Time till next DLC check */
var transient float DLCCheckTime;

/** this sound is played when a widget is clicked; we use it for fake widget clicking when we cancel matchmaking */
var(Sound) name ClickedCue;

/** The waves completed by the host */
var transient byte HordeWaveCompleted[50];

/** Whether the scene has been initialized yet */
var transient bool bPartySceneHasInitialized;

/** Whether we're starting a game (only used for unofficial matches) */
var transient bool bGameStarting;

/** Whether we want to publish settings to profile */
var transient bool bGameSettingHasChanged;

/** Config value that determines whether the Flashback Map Pack is able to be purchased or not */
var config bool bFlashbackMapPackCanBePurchased;

/** Hack flag to turn on the DLC button */
var transient bool bIsDlcButtonEnabled;

/************************************************************************/
/* Animations                                                           */
/************************************************************************/
var transient UIAnimationSeq FadeIn;
var transient UIAnimationSeq FadeOut;
var transient UIAnimationSeq SlideIn;
var transient UIAnimationSeq SlideOut;


/** Update the widget styles, strings, etc. */
function UpdatePartyLobbyScene(float DeltaTime)
{
	UpdateSceneState(,true);

	UpdatePlayerButtons();
	UpdateChatIcons();
	RefreshMainPartyButton();
	RefreshDescriptionWidget();

	// Check for DLC every few seconds
	DLCCheckTime += DeltaTime;
	if (DLCCheckTime > 5.0f)
	{
		UpdateLocalPlayersDLCValues();
		DLCCheckTime = 0.0f;
	}
}

/** Update the chat icon opacities */
final function UpdateChatIcons()
{
	local int Idx;
	local GearPRI CurrPRI;

	for ( Idx = 0; Idx < PlayerData.length; Idx++ )
	{
		if ( PlayerData[Idx].ChatIcon != None )
		{
			PlayerData[Idx].ChatIcon.Opacity = 0.0f;

			CurrPRI = PlayerData[Idx].PlayerPRI;
			if ( CurrPRI != None )
			{
				if( CurrPRI.ChatFadeValue > 0 )
				{
					CurrPRI.ChatFadeValue = 1.0f - (GetWorldInfo().TimeSeconds - CurrPRI.TaccomChatFadeStart) / CurrPRI.ChatFadeTime;
					PlayerData[Idx].ChatIcon.Opacity = CurrPRI.ChatFadeValue;
				}
			}
		}
	}
}

/* === UIScreenObject interface === */
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
		SetGRICallbacks();

		InitializeWidgetReferences();

		SetupCallbacks();

		InitializeGameTypeFromProfile();
	}
}

/**
 * Handler for the completion of this scene's opening animation...
 *
 * @warning - if you override this in a child class, keep in mind that this function will not be called if the scene has no opening animation.
 */
function OnOpenAnimationComplete( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	Super.OnOpenAnimationComplete(Sender, AnimName, TrackTypeMask);

	if (Sender == Self)
	{
		if (GetWorldInfo().NetMode == NM_Client)
		{
			RefreshTooltip(PlayerData[0].ParentButton);
		}
		else
		{
			RefreshTooltip(MatchmakeButton);
		}
	}
}

/**
 * Set up the default state for the matchmaking progress panel.
 */
function InitMatchamkingProgPanel()
{
	SetMatchmakingStepState(0, SS_Disabled);
	PnlMatchmakeProgress.Steps[0].LabelStepName.SetDataStoreBinding(MatchmakeReadSkillsMsg);

	SetMatchmakingStepState(1, SS_Disabled);
	PnlMatchmakeProgress.Steps[1].LabelStepName.SetDataStoreBinding(SearchingForTeammates);

	SetMatchmakingStepState(2, SS_Disabled);
	PnlMatchmakeProgress.Steps[2].LabelStepName.SetDataStoreBinding(MatchmakeConnectOpponent);
}

/**
 * Set the state of a matchmaking step to disabled, active or completed.
 *
 * @param StepSlot The slot to set, 0-2
 * @param InState What the new state of the slot should be
 */
function SetMatchmakingStepState(int StepSlot, StepState InState)
{
	switch(InState)
	{
		case SS_Disabled:
				PnlMatchmakeProgress.Steps[StepSlot].LabelStepName.SetEnabled(FALSE);
				PnlMatchmakeProgress.Steps[StepSlot].LabelStepName.SetVisibility(TRUE);
				PnlMatchmakeProgress.Steps[StepSlot].Checkbox.SetVisibility(TRUE);
				PnlMatchmakeProgress.Steps[StepSlot].Checkbox.SetValue(FALSE);
				PnlMatchmakeProgress.Steps[StepSlot].Cog.SetVisibility(FALSE);
			break;
		case SS_Active:
				PnlMatchmakeProgress.Steps[StepSlot].LabelStepName.SetEnabled(TRUE);
				PnlMatchmakeProgress.Steps[StepSlot].LabelStepName.SetVisibility(TRUE);
				PnlMatchmakeProgress.Steps[StepSlot].Checkbox.SetValue(FALSE);
				PnlMatchmakeProgress.Steps[StepSlot].Checkbox.SetVisibility(FALSE);
				PnlMatchmakeProgress.Steps[StepSlot].Cog.SetVisibility(TRUE);
			break;
		case SS_Complete:
				PnlMatchmakeProgress.Steps[StepSlot].LabelStepName.SetEnabled(TRUE);
				PnlMatchmakeProgress.Steps[StepSlot].LabelStepName.SetVisibility(TRUE);
				PnlMatchmakeProgress.Steps[StepSlot].Checkbox.SetVisibility(TRUE);
				PnlMatchmakeProgress.Steps[StepSlot].Checkbox.SetValue(TRUE);
				PnlMatchmakeProgress.Steps[StepSlot].Cog.SetVisibility(FALSE);
			break;
		case SS_Hidden:
				PnlMatchmakeProgress.Steps[StepSlot].LabelStepName.SetVisibility(FALSE);
				PnlMatchmakeProgress.Steps[StepSlot].Checkbox.SetVisibility(FALSE);
				PnlMatchmakeProgress.Steps[StepSlot].Cog.SetVisibility(FALSE);
			break;

		default:
				`warn("Invalid matchmaking step" $ InState);
			break;
	}
}

/** Updates the playerlist */
function UpdatePlayerButtons()
{
	local GearPartyGRI GRI;

	GRI = GearPartyGRI(GetGRI());
	// If there is no GRI, hide the player panel and exit
	if ( GRI == None )
	{
		pnlPlayers.SetVisibility( false );
		PlayerBackgroundImage.SetVisibility( false );
		return;
	}
	else
	{
		pnlPlayers.SetVisibility( true );
		PlayerBackgroundImage.SetVisibility( true );
	}

	SetPlayerListPRIs();
	RefreshPlayerWidgets();
}

/** Forces all PRIs to be in the top portion of the list of buttons */
function SetPlayerListPRIs()
{
	local int PRIIdx, CurrDataIdx, DataIdx;
	local GearPartyGRI MyGRI;
	local GearPartyPRI CurrPRI;

	MyGRI = GearPartyGRI(GetGRI());
	if ( MyGRI != None )
	{
		CurrDataIdx = 0;
		// Set PRIs
		for ( PRIIdx = 0; PRIIdx < MyGRI.PRIArray.length; PRIIdx++ )
		{
			CurrPRI = GearPartyPRI(MyGRI.PRIArray[PRIIdx]);
			if ( CurrPRI != None && !CurrPRI.bIsInactive )
			{
				PlayerData[CurrDataIdx].PlayerPRI = CurrPRI;
				CurrDataIdx++;
			}
		}

		// Null out empties
		for ( DataIdx = CurrDataIdx; DataIdx < PlayerData.length; DataIdx++ )
		{
			PlayerData[DataIdx].PlayerPRI = None;
		}
	}
}

/** Returns whether Horde is the currently selected gametype or not */
function bool IsHordeGametype()
{
	local int ValueId;
	local OnlineGameSettings GameSettings;

	if ( !IsOfficialMatch() )
	{
		GameSettings = GetCurrentGameSettings();
		if ( GameSettings != None &&
			 GameSettings.GetStringSettingValue(class'GearVersusGameSettings'.const.CONTEXT_VERSUSMODES, ValueId) &&
			 ValueId == eGEARMP_CombatTrials )
		{
			return true;
		}
	}

	return false;
}

/** Returns the team size for the party mode and gametype that is currently active */
function int GetPartyTeamSize()
{
	local GearPartyGRI PartyGRI;
	local int Result;

	Result = 5;
	if ( IsOfficialMatch() )
	{
		PartyGRI = GearPartyGRI(GetGRI());
		if ( PartyGRI != None )
		{
			Result = PartyGRI.TeamSize;
		}
	}
	else
	{
		if ( IsLocalMatch() )
		{
			Result = IsHordeGametype() ? 2 : 10;
		}
		else
		{
			Result = IsHordeGametype() ? 5 : 10;
		}
	}

	return Result;
}

/** Refreshes the player data widgets */
function RefreshPlayerWidgets()
{
	local int DataIdx, NumPRIs, PartyTeamSize;
	local GearPartyGRI PartyGRI;
	local UILabelButton LastVisibleButton;
	local UILabelButton FirstEnabledButton;
	local UILabelButton LastEnabledButton;
	local bool bPlayerPanelHasFocus;
	local bool bAPlayerButtonHasFocus;
	local bool bIsInvitableMatch;
	local bool bButtonIsDisabled;
	local int PlayerIndex;
	local int NumBots;
	local int NumOpenSpots;

	PartyGRI = GearPartyGRI(GetGRI());
	NumPRIs = GetNumValidPRIs();
	PartyTeamSize = GetPartyTeamSize();
	PlayerIndex = GetBestPlayerIndex();
	LastEnabledButton = none;

	// If the host, check for teamsize error
	if ( PartyGRI != None && PartyGRI.Role == ROLE_Authority )
	{
		if ( NumPRIs > PartyTeamSize ||
			(IsOfficialMatch() && (PartyMemberLacksRequiredDLC() || PartyMemberIsAGuest())) )
		{
			PartyGRI.bPartyLobbyErrorExists = true;
		}
		else
		{
			PartyGRI.bPartyLobbyErrorExists = false;
		}
	}

	// Determine if the player button panel has focus
	bPlayerPanelHasFocus = (pnlPlayers.GetCurrentState(GetBestPlayerIndex()).class == class'UIState_Focused');
	bAPlayerButtonHasFocus = false;

	for ( DataIdx = 0; DataIdx < PlayerData.length; DataIdx++ )
	{
		if ( DataIdx < PartyTeamSize || DataIdx < NumPRIs )
		{
			// See if this match is invitable
			bIsInvitableMatch = !IsLocalMatch() && !IsSystemLinkMatch();
			// Determine if the button should be disabled or not
			NumBots = GetNumBots();
			NumOpenSpots = Max(0, GetPartyTeamSize() - (NumPRIs + PartyGRI.ConnectingPlayerCount + NumBots));
			// See if this button should be disabled or not
			if ((PlayerData[DataIdx].PlayerPRI == None) &&
				(!bIsInvitableMatch || (DataIdx >= NumPRIs + PartyGRI.ConnectingPlayerCount + NumOpenSpots)))
			{
				bButtonIsDisabled = true;
			}

			// Wipe the forced navigation
			PlayerData[DataIdx].ParentButton.SetForcedNavigationTarget(UIFACE_Top, none);
			PlayerData[DataIdx].ParentButton.SetForcedNavigationTarget(UIFACE_Bottom, none);

			// Set visibility
			if (bButtonIsDisabled)
			{
				// Disable the button
				if (PlayerData[DataIdx].ParentButton.IsFocused())
				{
					PlayerData[DataIdx].ParentButton.KillFocus(none, PlayerIndex);
					LastEnabledButton.SetFocus(none);
				}
				PlayerData[DataIdx].ParentButton.SetEnabled(false, PlayerIndex);
			}
			else
			{
				// Enabled the button
				PlayerData[DataIdx].ParentButton.SetEnabled(true, PlayerIndex);
				// Keep track of the last enabled button for navigating
				LastEnabledButton = PlayerData[DataIdx].ParentButton;
				if (FirstEnabledButton == none)
				{
					FirstEnabledButton = LastEnabledButton;
				}
			}
			RefreshPlayerName( DataIdx, bIsInvitableMatch );
			RefreshPlayerProfileIcon( DataIdx, bIsInvitableMatch );
			RefreshPlayerRankIcon( DataIdx );

			// Need to see if any of the player buttons have focus when the parent has focus
			if ( bPlayerPanelHasFocus && !bAPlayerButtonHasFocus )
			{
				if ( PlayerData[DataIdx].ParentButton.GetCurrentState(GetBestPlayerIndex()).class == class'UIState_Focused' ||
					 PlayerData[DataIdx].ParentButton.GetCurrentState(GetBestPlayerIndex()).class == class'UIState_Pressed' )
				bAPlayerButtonHasFocus = true;
			}
			// Keep track of the last visible button
			LastVisibleButton = PlayerData[DataIdx].ParentButton;
			LastVisibleButton.SetVisibility(true);
		}
		else
		{
			PlayerData[DataIdx].ParentButton.SetVisibility( false );
		}
	}

	// Set the navigation
	if (FirstEnabledButton != none && LastEnabledButton != none && FirstEnabledButton != LastEnabledButton)
	{
		FirstEnabledButton.SetForcedNavigationTarget(UIFACE_Top, LastEnabledButton);
		LastEnabledButton.SetForcedNavigationTarget(UIFACE_Bottom, FirstEnabledButton);
	}

	if ( LastVisibleButton != None )
	{
		PlayerBackgroundImage.SetDockTarget( UIFACE_Bottom, LastVisibleButton, UIFACE_Bottom );

		// If the parent panel is focused, but no player buttons are focused, then we have the player button
		// that WOULD have been focused invisible, so set the last visible button to focused
		if ( bPlayerPanelHasFocus && !bAPlayerButtonHasFocus )
		{
			LastVisibleButton.SetFocus(None);
		}
	}
}

/** Iterates the PRI array and returns true if any players are guests of another player */
function bool PartyMemberIsAGuest()
{
	local int Index;
	local GearPartyGRI GRI;

	GRI = GearPartyGRI(GetGRI());
	if (GRI != None)
	{
		// Loop through all players checking for guest status
		for (Index = 0; Index < GRI.PRIArray.Length; Index++)
		{
			if (GearPartyPRI(GRI.PRIArray[Index]).bIsGuest)
			{
				return true;
			}
		}
	}
	return false;
}

/** Whether a player should be showing the error background or not */
function bool PlayerShouldDisplayError( GearPartyPRI PlayerPRI, int PlayerDataIdx )
{
	local GearPartyGRI PartyGRI;

	PartyGRI = GearPartyGRI(GetGRI());

	if ( PartyGRI != None &&
		 (PlayerDataIdx >= GetPartyTeamSize() || (IsOfficialMatch() && (!PlayerHasRequiredDLC(PlayerPRI) || PlayerPRI.bIsGuest))) )
	{
		return true;
	}

	return false;
}

/**
 * Returns whether anyone in the party does not have the DLC required for the currently selected playlist
 * @param bUseStrictCheck - used when proper replication has not taken place yet
 *		(this is useful for when you MUST know if the player has DLC, for normal user feedback we won't care about
 *		 this but for a button click that will require a proper check, we must use it)
 */
function bool PartyMemberLacksRequiredDLC( optional bool bUseStrictCheck )
{
	local int Idx;
	local GearPartyGRI GRI;

	GRI = GearPartyGRI(GetGRI());
	if (GRI != None)
	{
		// Loop through all players
		for (Idx = 0; Idx < GRI.PRIArray.length; Idx++)
		{
			// If one of the player doesn't have the required DLC return true
			if (!PlayerHasRequiredDLC(GearPRI(GRI.PRIArray[Idx]), bUseStrictCheck))
			{
				return true;
			}
		}
	}
	return false;
}

/**
 * Whether the player has the required DLCs needed for this playlist or not
 * @param bUseStrictCheck - used when proper replication has not taken place yet
 *		(this is useful for when you MUST know if the player has DLC, for normal user feedback we won't care about
 *		 this but for a button click that will require a proper check, we must use it)
 */
function bool PlayerHasRequiredDLC(GearPRI PlayerPRI, optional bool bUseStrictCheck)
{
	local int DLCIdx;
	local GearGRI GRI;
	local OnlinePlaylistManager PlaylistMan;
	local OnlineSubsystem OnlineSub;
	local array<int> ContentIds;
	local bool bResult;

	bResult = true;

	GRI = GetGRI();
	if (GRI != None)
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			PlaylistMan = OnlinePlaylistManager(OnlineSub.GetNamedInterface('PlaylistManager'));
			if (PlaylistMan != None)
			{
				// Get the content ids required for this playlist
				PlaylistMan.GetContentIdsFromPlaylist(GRI.PlaylistId, ContentIds);
				// If the content id list is empty then this playlist only uses shipped maps,
				// so we only need to do the content check if this list is > 0 in length
				if (ContentIds.length > 0)
				{
					// Make sure this is a valid PRI
					if (PlayerPRI != None && !PlayerPRI.bIsInactive)
					{
						// If they haven't replicated their DLC flag yet, return based on the strict check
						if (PlayerPRI.DLCFlag == -1)
						{
							bResult = !bUseStrictCheck;
						}
						// They've replicated so do the check
						else
						{
							// Loop through all the content ids and break out and return false if one fails
							for (DLCIdx = 0; DLCIdx < ContentIds.length; DLCIdx++)
							{
								if (ContentIds[DLCIdx] != 0 &&
									((PlayerPRI.DLCFlag & (1<<ContentIds[DLCIdx])) == 0))
								{
									bResult = false;
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		bResult = !bUseStrictCheck;
	}

	return bResult;
}

/** Refresh the string drawn in the player button */
function RefreshPlayerName( int DataIdx, bool bIsInvitableMatch )
{
	local GearPartyPRI CurrPRI;
	local GearPartyGRI PartyGRI;
	local int NumPRIs;
	local String DotsString;
	local int DotIdx, TotalDots;
	local UniqueNetId ZeroId;
	local UILabelButton lblButton;
	local int NumBots;
	local int NumOpenSpots;
	local bool bIsPublic;

	PartyGRI = GearPartyGRI(GetGRI());
	CurrPRI = PlayerData[DataIdx].PlayerPRI;
	NumPRIs = GetNumValidPRIs();
	lblButton = PlayerData[DataIdx].ParentButton;

	// We have a PRI so display the name
	if ( CurrPRI != None )
	{
		if ((CurrPRI.UniqueId != ZeroId && CurrPRI.DLCFlag != -1) || IsLocalMatch())
		{
			lblButton.SetDataStoreBinding( CurrPRI.PlayerName );
		}
		else
		{
			lblButton.SetDataStoreBinding( ConnectingPlayerSlot );
		}
		if ( PlayerShouldDisplayError(CurrPRI, DataIdx) )
		{
			SetLabelButtonBGStyle(lblButton, 'PartyPlayerError');
		}
		else
		{
			SetLabelButtonBGStyle(lblButton, 'PartyPlayersNormal');
		}
	}
	// No PRI so the GRI show determine what should be drawn
	else if ( PartyGRI != None )
	{
		// Matchmaking is happening
		if ( PartyGRI.MatchmakingState >= MMS_CancelingMatchmaking &&
			 PartyGRI.MatchmakingState <= MMS_ConnectingToOpposingParty )
		{
			if ( DataIdx < NumPRIs + PartyGRI.PartySlotsFilled )
			{
				lblButton.SetDataStoreBinding( FoundPlayerSlot );
			}
			else if ( DataIdx < GetPartyTeamSize() && IsOfficialMatch() )
			{
				DotsString = "";
				TotalDots = PartyGRI.WorldInfo.TimeSeconds % 4;
				for ( DotIdx = 0; DotIdx < TotalDots; DotIdx++ )
				{
					DotsString $= ".";
				}
				lblButton.SetDataStoreBinding( SearchPlayerSlot $ DotsString );
			}
			else
			{
				lblButton.SetDataStoreBinding( " " );
			}
		}
		// Is not doing any matchmaking
		else
		{
			bIsPublic = IsOfficialMatch();
			NumBots = bIsPublic ? 0 : GetNumBots();
			NumOpenSpots = Max(0, GetPartyTeamSize() - (NumPRIs + PartyGRI.ConnectingPlayerCount + NumBots));
			if ( DataIdx < NumPRIs + PartyGRI.ConnectingPlayerCount )
			{
				lblButton.SetDataStoreBinding( ConnectingPlayerSlot );
			}
			else
			{
				// If this is a Live match first try invitable spots
				if (bIsPublic || IsCustomMatch())
				{
					if (DataIdx < NumPRIs + PartyGRI.ConnectingPlayerCount + NumOpenSpots)
					{
						PlayerData[DataIdx].ProfileLabel.SetDataStoreBinding("<Fonts:Warfare_HUD.Xbox360_18pt>a<Fonts:/>");
						lblButton.SetDataStoreBinding( InvitablePlayerSlot );
					}
					// now try bots for custom matches
					else if (DataIdx < NumPRIs + PartyGRI.ConnectingPlayerCount + NumOpenSpots + NumBots)
					{
						PlayerData[DataIdx].ProfileLabel.SetDataStoreBinding(" ");
						lblButton.SetDataStoreBinding(BotSlotString);
					}
				}
				// Handle system link and local
				else
				{
					// If the match has bots in it
					if (DataIdx < NumPRIs + PartyGRI.ConnectingPlayerCount + NumBots)
					{
						PlayerData[DataIdx].ProfileLabel.SetDataStoreBinding(" ");
						lblButton.SetDataStoreBinding(BotSlotString);
					}
					// Otherwise show it as open
					else
					{
						PlayerData[DataIdx].ProfileLabel.SetDataStoreBinding(" ");
						lblButton.SetDataStoreBinding( OpenPlayerSlot );
					}
				}
			}
		}

		SetLabelButtonBGStyle(lblButton, 'PartyEmptyNormal');
	}
}

/** Refresh the lable on the far left that shows either the A button or the profile icon */
function RefreshPlayerProfileIcon( int DataIdx, bool bIsInvitableMatch )
{
	local PlayerController PC;
	local string ControllerString;
	local LocalPlayer LP;
	local GearPartyGRI PartyGRI;
	local int NumPRIs;

	ControllerString = " ";

	if ( PlayerData[DataIdx].PlayerPRI != None )
	{
		PC = PlayerController(PlayerData[DataIdx].PlayerPRI.Owner);
		if ( PC != None && PC.IsLocalPlayerController() )
		{
			LP = LocalPlayer(PC.Player);
			if ( LP != None )
			{
				ControllerString = GetControllerIconString( LP.ControllerId );
			}
		}
	}
	else
	{
		PartyGRI = GearPartyGRI(GetGRI());
		NumPRIs = GetNumValidPRIs();
		// See if we are matchmaking
		if ( PartyGRI.MatchmakingState >= MMS_CancelingMatchmaking &&
			 PartyGRI.MatchmakingState <= MMS_ConnectingToOpposingParty )
		{
			if ( DataIdx < NumPRIs + PartyGRI.PartySlotsFilled )
			{
				// Show that we found a remote player
				ControllerString = "<Fonts:Warfare_HUD.Xbox360_18pt>f<Fonts:/>";
			}
		}
		else
		{
			if ( DataIdx >= NumPRIs + PartyGRI.ConnectingPlayerCount )
			{
				// Show the invite icon if a live match
				if (bIsInvitableMatch)
				{
					ControllerString = "<Fonts:Warfare_HUD.Xbox360_18pt>a<Fonts:/>";
				}
			}
		}
	}

	PlayerData[DataIdx].ProfileLabel.SetDataStoreBinding( ControllerString );
}

/** Refresh the player rank icon */
function RefreshPlayerRankIcon( int DataIdx )
{
	local string RankString;

	RankString = " ";

	if ( PlayerData[DataIdx].PlayerPRI != None )
	{
		RankString = GetPlayerSkillString( PlayerData[DataIdx].PlayerPRI.PlayerSkill );
	}

	PlayerData[DataIdx].RankLabel.SetDataStoreBinding( RankString );
}

/** Returns the number of valid PRIs in the GRI's PRIArray */
function int GetNumValidPRIs()
{
	local GearGRI GRI;
	local int Idx, Count;
	local UniqueNetId ZeroId;
	local GearPRI PRI;

	Count = 0;
	GRI = GetGRI();

	if ( GRI != None )
	{
		for ( Idx = 0; Idx < GRI.PRIArray.length; Idx++ )
		{
			PRI = GearPRI(GRI.PRIArray[Idx]);
			if ( PRI != None &&
				!PRI.bIsInactive &&
				// This means that have't fully replicated
				(PRI.UniqueId != ZeroId || IsLocalMatch()) &&
				PRI.DLCFlag != -1)
			{
				Count++;
			}
		}
	}

	return Count;
}

/** Returns the number of bots that the server has selected for this game */
final function int GetNumBots()
{
	local OnlineGameSettings CurrentSettings;
	local int IntValue;

	if (!IsOfficialMatch())
	{
		CurrentSettings = GetCurrentGameSettings();
		CurrentSettings.GetStringSettingValue(class'GearVersusGameSettings'.const.CONTEXT_NUMBOTS, IntValue);
	}
	else
	{
		IntValue = 0;
	}
	return IntValue;
}

/** Set the party list to the proper menu items */
function SetMenuItemsInPartyList()
{
	local GearProfileSettings Profile;
	local int SelectedContext;
	local OnlineGameSettings CurrentSettings;

	if ( IsPartyLeader(0) )
	{
		Profile = GetPlayerProfile(0);
		if ( Profile != None && Profile.GetProfileSettingValueId(Profile.const.VERSUS_MATCH_MODE, SelectedContext) )
		{
			if ( SelectedContext == eGVMT_Official )
			{
				ParentPlaylistDescribeImage.SetVisibility( true );
				lstGameOptions.SetVisibility( false );
				imgGameOptionsBG.SetVisibility( false );
				lstPartyOptions.SetDataStoreBinding("<MenuItems:PartyLobbyPublic>");
			}
			else if ( SelectedContext == eGVMT_Custom )
			{
				ParentPlaylistDescribeImage.SetVisibility( false );
				lstGameOptions.SetVisibility( true );
				imgGameOptionsBG.SetVisibility( true );
				lstPartyOptions.SetDataStoreBinding("<MenuItems:PartyLobbyPrivate>");
				RefreshGameOptions(GetBestPlayerIndex());
			}
			else if ( SelectedContext == eGVMT_SystemLink )
			{
				ParentPlaylistDescribeImage.SetVisibility( false );
				lstGameOptions.SetVisibility( true );
				imgGameOptionsBG.SetVisibility( true );
				lstPartyOptions.SetDataStoreBinding("<MenuItems:PartyLobbyLAN>");
				RefreshGameOptions(GetBestPlayerIndex());
			}
			else
			{
				ParentPlaylistDescribeImage.SetVisibility( false );
				lstGameOptions.SetVisibility( true );
				imgGameOptionsBG.SetVisibility( true );
				lstPartyOptions.SetDataStoreBinding("<MenuItems:PartyLobbyLocal>");
				RefreshGameOptions(GetBestPlayerIndex());
			}
		}
	}
	else
	{
		CurrentSettings = GetCurrentGameSettings();
		if ( CurrentSettings != None && CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, SelectedContext) )
		{
			if ( SelectedContext == eGVMT_Official )
			{
				ParentPlaylistDescribeImage.SetVisibility( true );
				lstGameOptions.SetVisibility( false );
				imgGameOptionsBG.SetVisibility( false );
				lstPartyOptions.SetDataStoreBinding("<MenuItems:PartyLobbyClientPublic>");
			}
			else if ( SelectedContext == eGVMT_Custom )
			{
				ParentPlaylistDescribeImage.SetVisibility( false );
				lstGameOptions.SetVisibility( true );
				imgGameOptionsBG.SetVisibility( true );
				lstPartyOptions.SetDataStoreBinding("<MenuItems:PartyLobbyClientPrivate>");
			}
			else if ( SelectedContext == eGVMT_SystemLink )
			{
				ParentPlaylistDescribeImage.SetVisibility( false );
				lstGameOptions.SetVisibility( true );
				imgGameOptionsBG.SetVisibility( true );
				lstPartyOptions.SetDataStoreBinding("<MenuItems:PartyLobbyClientLAN>");
			}
			else
			{
				ParentPlaylistDescribeImage.SetVisibility( false );
				lstGameOptions.SetVisibility( true );
				imgGameOptionsBG.SetVisibility( true );
				lstPartyOptions.SetDataStoreBinding("<MenuItems:PartyLobbyClientLocal>");
			}
		}
	}
}

/** Refresh the playlist descriptions */
function RefreshPlaylistDescription( int ProviderIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> PlaylistProviders;
	local OnlinePlaylistProvider PlaylistProvider;
	local string GameTypesString;
	local int Idx, GameProviderIndex;
	local GearGamePlaylistGameTypeProvider GameTypeProvider;
	local GearGameInfoSummary GameInfoProvider;

	GameTypesString = "";
	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the playlist providers
		if ( GameResourceDS.GetResourceProviders('Playlists', PlaylistProviders) && ProviderIndex < PlaylistProviders.length )
		{
			PlaylistProvider = OnlinePlaylistProvider(PlaylistProviders[ProviderIndex]);
			if ( PlaylistProvider != none )
			{
				PlaylistTitleLabel.SetDataStoreBinding( PlaylistTitlePrefix@PlaylistProvider.DisplayName@PlaylistTitleSuffix );

				// Set the playlist name for matchmaking progress panel
				PnlMatchmakeProgress.LabelPlaylist.SetDataStoreBinding(PlaylistProvider.DisplayName);

				for ( Idx = 0; Idx < PlaylistProvider.PlaylistGameTypeNames.length; Idx++ )
				{
					GameProviderIndex = GameResourceDS.ConvertPlaylistGameNameIndexToPlaylistGameTypeProviderIndex( PlaylistProvider, Idx );
					GameTypeProvider = GameResourceDS.GetPlaylistGameTypeProvider( GameProviderIndex );
					if ( GameTypeProvider != None )
					{
						if ( GameTypesString != "" )
						{
							GameTypesString $= "\n";
						}

						if ( GameTypeProvider.DisplayName != "" )
						{
							GameTypesString $= GameTypeProvider.DisplayName;
						}
						else
						{
							GameInfoProvider = GameResourceDS.GetGameTypeProvider( GameTypeProvider.MPGameMode );
							if ( GameInfoProvider != None )
							{
								GameTypesString $= GameInfoProvider.GameName;
							}
						}
					}
				}
				PlaylistDescribeLabel.SetDataStoreBinding( GameTypesString );
			}
		}
	}
}

/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	local int Idx, PlayerIndex;
	local String WidgetName;
	local GearPC OwnerPC;
	local bool bIsHost;
	local UILabel TmpLabel;

	lstPartyOptions = GearUIObjectList(FindChild('lstOptions', true));
	PlaylistDescriptionBackground = UIImage(FindChild('imgPlaylistDescribe', true));
	lstGameOptions = GearUIObjectList(FindChild('lstGameOptions', true));
	btnbarPlayers = UICalloutButtonPanel(FindChild('btnbarPlayers', true));
	btnbarPlayers2 = UICalloutButtonPanel(FindChild('btnbarPlayers2', true));
	btnbarLAN = UICalloutButtonPanel(FindChild('btnbarLAN', true));
	pnlOptions = UIPanel(FindChild('pnlOptions',true));
	imgGameOptionsBG = UIImage(FindChild('imgGameOptionsBG',true));

	pnlPlayers = UIPanel(FindChild('pnlPartyMembers',true));
	PlayerBackgroundImage = UIImage(FindChild('imgPartyMemberBG',true));
	PlayerData.length = 10;
	for ( Idx = 0; Idx < PlayerData.length; Idx++ )
	{
		WidgetName = "btnPlayer" $ Idx;
		PlayerData[Idx].ParentButton = UILabelButton(pnlPlayers.FindChild(Name(WidgetName),true));
		PlayerData[Idx].ParentButton.OnClicked = OnPlayerButtonClicked;

		PlayerData[Idx].ProfileLabel = UILabel(PlayerData[Idx].ParentButton.FindChild('lblProfile',true));
		PlayerData[Idx].RankLabel = UILabel(PlayerData[Idx].ParentButton.FindChild('lblRank',true));
		WidgetName = "imgChat" $ Idx;
		PlayerData[Idx].ChatIcon = UIImage(FindChild(Name(WidgetName),true));
		if ( PlayerData[Idx].ChatIcon != None )
		{
			PlayerData[Idx].ChatIcon.SetVisibility(true);
			PlayerData[Idx].ChatIcon.Opacity = 0.0f;
		}
	}

	// Playlist description widgets
	ParentPlaylistDescribeImage = UIImage(FindChild('imgPlaylistHeader',true));
	PlaylistTitleLabel = UILabel(FindChild('lblPlaylistDescribe',true));
	PlaylistDescribeLabel = UILabel(FindChild('lblGametypes',true));

	// Uber-button widgets
	MatchmakeButton = UILabelButton(FindChild('btnFindGame',true));
	MatchmakeButtonIcon = UILabel(FindChild('lblStart',true));
	COGSymbolNormal = UIImage(FindChild('imgCOG',true));
	COGSymbolMatchmaking = UIImage(FindChild('imgCOGLoading',true));
	COGSymbolBroken = UIImage(FindChild('imgCOGBroken',true));
	BloodSplat = UIImage(FindChild('imgSplat',true));

	// Party message widgets
	PartyMsgPanel = UIPanel(FindChild('pnlPartyMessage',true));
	PartyMsgLabel = UILabel(FindChild('lblPartyMessage',true));
	PartyMsgImage = UIImage(FindChild('imgPartyMessages',true));

	SetMenuItemsInPartyList();

	PlayerIndex = GetBestPlayerIndex();
	btnbarPlayers.SetVisibility( true );
	btnbarPlayers.EnableButton('PostGame', PlayerIndex, ShouldEnableGameStats(), false);
	bIsDlcButtonEnabled = false;
	btnbarPlayers.EnableButton('DLC', PlayerIndex, false, false);
	btnbarPlayers2.SetVisibility( true );
	btnbarPlayers2.EnableButton('WeapOption', PlayerIndex, IsAllowedToOpenWeaponSwap(PlayerIndex), IsOfficialMatch() || (GetWorldInfo().NetMode == NM_Client));
	btnbarLAN.SetVisibility( true );
	btnbarLAN.EnableButton('LANBrowser', PlayerIndex, CanOpenLANScene(PlayerIndex), true);

	lstGameOptions.SetDataStoreBinding("<MenuItems:CurrentGameSettings>");

	// Get references to Widgets that describe the current matchmaking step.
	PnlMatchmakeProgress.Panel = UIPanel(FindChild('pnlMatchmaking',true));
	PnlMatchmakeProgress.LabelPlaylist = UILabel(FindChild('lblPlaylist',true));
	PnlMatchmakeProgress.Background = UIImage(FindChild('imgMatchmakingBG',true));

	for (Idx=0; Idx<NumMatchmakeSteps; Idx++)
	{
		TmpLabel = UILabel( PnlMatchmakeProgress.Panel.FindChild( Name( "lblStep" $ (Idx+1) ) ) );
		PnlMatchmakeProgress.Steps[Idx].LabelStepName = TmpLabel;
		PnlMatchmakeProgress.Steps[Idx].Checkbox = UICheckbox( TmpLabel.FindChild( Name( "ckbox0" $ (Idx+1) ) ) );
		PnlMatchmakeProgress.Steps[Idx].Cog = UIImage( TmpLabel.FindChild( 'imgSpinningCog' ) );
	}
	PnlMatchmakeProgress.Panel.SetVisibility(FALSE);

	//@hack: make the background of the panel render correctly
	PnlMatchmakeProgress.Background.SetDockTarget(UIFACE_Left, PnlMatchmakeProgress.Panel, UIFACE_Left);
	PnlMatchmakeProgress.Background.SetDockTarget(UIFACE_Right, PnlMatchmakeProgress.Panel, UIFACE_Right);

	InitMatchamkingProgPanel();

	// Check DLC offerings
	ReadAvailableContent();

	// Cache the number of consecutive waves completed
	OwnerPC = GetGearPlayerOwner(PlayerIndex);
	bIsHost = (GetWorldInfo().NetMode < NM_Client);
	if (bIsHost && OwnerPC != none && OwnerPC.ProfileSettings != none)
	{
		for (Idx = 0; Idx < 50; Idx++)
		{
			HordeWaveCompleted[Idx] = OwnerPC.ProfileSettings.HasCompletedHordeWave(Idx) ? 1 : 0;
		}
	}

}

/** Starts an async task to read map only DLC packages */
function ReadAvailableContent()
{
	local OnlineSubsystem OnlineSub;
	local int ControllerId;

	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(GetBestPlayerIndex());

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.ContentInterface != None)
	{
		OnlineSub.ContentInterface.AddQueryAvailableDownloadsComplete(ControllerId,OnReadAvailableContentComplete);
		// NOTE: 2 is map DLC mask
		OnlineSub.ContentInterface.QueryAvailableDownloads(ControllerId,2);
	}
}

/**
 * Called when the content read has completed so we can enable/disable the DLC button
 * based upon whether there is new content to download
 *
 * @param bWasSuccessful whether the read worked or not
 */
function OnReadAvailableContentComplete(bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local int ControllerId;
	local int NewDownloads;
	local int TotalDownloads;

	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(GetBestPlayerIndex());

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.ContentInterface != None)
	{
		OnlineSub.ContentInterface.ClearQueryAvailableDownloadsComplete(ControllerId,OnReadAvailableContentComplete);
		if (bWasSuccessful)
		{
			// Get the number of new items and enable the button based upon that
			OnlineSub.ContentInterface.GetAvailableDownloadCounts(ControllerId,NewDownloads,TotalDownloads);
		}
	}
	bIsDlcButtonEnabled = NewDownloads > 0;
	btnbarPlayers.EnableButton('DLC', GetBestPlayerIndex(), NewDownloads > 0, false);
}

/** Whether the player can open the LAN browser or not */
function bool CanOpenLANScene(int PlayerIndex)
{
	return IsSystemLinkMatch() && GetWorldInfo().NetMode != NM_Client;
}

/**
 * LAN Browser button pressed
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnLANBrowserClicked( UIScreenObject EventObject, int PlayerIndex )
{
	if ( IsSystemLinkMatch() &&
		 PlayerIndex == 0 )
	{
		AttemptSystemLinkTransition();
	}

	return true;
}

/** Whether there are stats from a previous game we can view or not */
function bool ShouldEnableGameStats()
{
	local GearRecentPlayersList PlayersList;
	local OnlineSubsystem OnlineSub;
	local GameUISceneClient GameSceneClient;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		// Get the recent player data for the last match stats
		PlayersList = GearRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
		if (PlayersList != None)
		{
			if (PlayersList.LastMatchResults != None)
			{
				GameSceneClient = class'UIRoot'.static.GetSceneClient();
				if (GameSceneClient != None)
				{
					return GameSceneClient.FindSceneByTag('PreviousGameReview') == None;
				}
			}
		}
	}
	return false;
}

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	lstPartyOptions.OnOptionChanged = OptionValueChanged;
	lstPartyOptions.OnListOptionSubmitted = ListValueSubmitted;
	lstPartyOptions.OnRegeneratedOptions = OnRegeneratedOptionsCallback;
	lstGameOptions.OnOptionChanged = OptionValueChanged;
	lstGameOptions.OnListOptionSubmitted = ListValueSubmitted;
	lstGameOptions.OnRegeneratedOptions = OnRegeneratedOptionsCallback;
	MatchmakeButton.OnClicked = StartMatchButtonClicked;

	btnbarPlayers.SetButtonCallback('PostGame', OnPostGameStatsClicked);
	btnbarPlayers.SetButtonCallback('DLC', OnDLCClicked);
	btnbarPlayers2.SetButtonCallback('WeapOption', OnWeaponSwapClicked);
	btnbarLAN.SetButtonCallback('LANBrowser', OnLANBrowserClicked);
}

/**
 * Responsible for hooking up all delegates in the GearGRI class.  If the GRI hasn't been replicated yet, sets a timer in
 * the owning player's PlayerController to check again next tick.
 */
function SetGRICallbacks()
{
	local GearPC OwnerPC;
	local GearGRI MyGRI;

	OwnerPC = GetGearPlayerOwner(TEMP_SPLITSCREEN_INDEX);
	if ( OwnerPC.PlayerReplicationInfo != None )
	{
		MyGRI = GetGRI();
		if ( MyGRI != None )
		{
			MyGRI.OnPlaylistIdChanged = OnPlaylistChanged;
			return;
		}
	}

	OwnerPC.SetTimer( 0.1, false, nameof(self.SetGRICallbacks), self );
}

/**
 * Clears all delegates which have been assigned to methods in this class so that those objects
 * don't hold a reference to this scene.
 */
function ClearDelegates()
{
	local GearGRI MyGRI;

	MyGRI = GetGRI();
	if ( MyGRI != None )
	{
		MyGRI.OnPlaylistIdChanged = None;
	}
}

/**
 * Retrieves the gametype that should be initially selected (per the user's profile) and sets that gametype as the
 * game settings datastore's current gametype.
 */
function InitializeGameTypeFromProfile()
{
	local GearProfileSettings Profile;
	local int SelectedContext;
	local GearUIDataStore_GameSettings SettingsDS;

	// initialize the game settings data store
	if ( IsPartyLeader(0) )
	{
		SettingsDS = GetGameSettingsDataStore();
		Profile = GetPlayerProfile(0);
		if ( SettingsDS != None && Profile != None )
		{
			if ( Profile.GetProfileSettingValueId(Profile.const.VERSUS_GAMETYPE, SelectedContext) )
			{
				SettingsDS.SetCurrentByContextId(SelectedContext);

				// now update other settings for this gametype
				LoadGameSettingsFromProfile(true);
			}
			else
			{
				`log("!!!!!!!!!!!!!!>>>><<<< Couldn't retrieve VERSUS_GAMETYPE from profile!");
			}

			UpdateRemoteSettings();
		}

		SettingsDS.AddPropertyNotificationChangeRequest(SettingsProviderUpdated);
	}
}

/**
 * Manages the notification area; responsible for choosing between showing
 *
 * @param The current matchmaking step.
 */
function UpdateOptionsAndStatusArea( EMatchmakingState InMatchmakingStep )
{
	local int NewTeammateCount;

	NewTeammateCount = GetNumValidPRIs() + GearPartyGRI(GetGRI()).PartySlotsFilled;

	if (InMatchmakingStep != CurDisplayedMatchmakingStep)
	{
		// We were in a resting state and now we're matchmaking; show matchmaking progress panel
		if (CurDisplayedMatchmakingStep == MMS_NotMatchmaking)
		{
			// Reset everything in the matchmaking panel to "OFF"
			InitMatchamkingProgPanel();

			// Set the name of the playlist on the display panel
			PnlMatchmakeProgress.LabelPlaylist.SetVisibility(TRUE);

			// Show the matchmaking progress panel
			PnlMatchmakeProgress.Panel.SetVisibility(TRUE);
			PnlMatchmakeProgress.Panel.PlayUIAnimation( '', SlideIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
			PnlMatchmakeProgress.Panel.Opacity = 1.0;

			lstPartyOptions.PlayUIAnimation( '', FadeOut, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
			PlaylistDescriptionBackground.PlayUIAnimation( '', FadeOut, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
		}

		// We were actively matchmaking and now we stopped; hide matchmaking progress panel
		if (InMatchmakingStep == MMS_NotMatchmaking)
		{
			// Hide the matchmaking progress panel
			PnlMatchmakeProgress.Panel.PlayUIAnimation( '', SlideOut, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );

			lstPartyOptions.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
			PlaylistDescriptionBackground.PlayUIAnimation( '', FadeIn, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, false );
		}
	}


	// If the state (or number of teammates) has recently changed we should show/hide/update the relevant widgets
	if (InMatchmakingStep != CurDisplayedMatchmakingStep || TeammateCount != NewTeammateCount)
	{
		TeammateCount = NewTeammateCount;


		// Set the panel to reflect the current matchmaking state
		switch ( InMatchmakingStep )
		{
			case MMS_HostCancelingMatchmaking:
					// We have restarted matchmaking
					InitMatchamkingProgPanel();
					PnlMatchmakeProgress.Steps[0].LabelStepName.SetDataStoreBinding(MatchmakeHostCancelled);
					SetMatchmakingStepState(0, SS_Active);
				break;

			case MMS_CancelingMatchmaking:
					SetMatchmakingStepState(0, SS_Hidden);
					PnlMatchmakeProgress.Steps[0].LabelStepName.SetDataStoreBinding("");
					SetMatchmakingStepState(1, SS_Active);
					PnlMatchmakeProgress.Steps[1].LabelStepName.SetDataStoreBinding(MatchmakeCancelMsg);
					SetMatchmakingStepState(2, SS_Hidden);
					PnlMatchmakeProgress.Steps[2].LabelStepName.SetDataStoreBinding("");
				break;

			case MMS_ReadingSkills:
					PnlMatchmakeProgress.Steps[0].LabelStepName.SetDataStoreBinding(MatchmakeReadSkillsMsg);
					SetMatchmakingStepState(0, SS_Active);
					SetMatchmakingStepState(1, SS_Disabled);
					SetMatchmakingStepState(2, SS_Disabled);
				break;


			//MMS_FindingOpposingParty
			//MMS_WaitingForOpposingParty -> Searching for a team

			case MMS_FindingBestParty:
			case MMS_FindingAnyParty:
				PnlMatchmakeProgress.Steps[1].LabelStepName.SetDataStoreBinding(SearchingForTeammates);
				SetMatchmakingStepState(0, SS_Complete);
				SetMatchmakingStepState(1, SS_Active);
				SetMatchmakingStepState(2, SS_Disabled);
				break;

			case MMS_WaitingForOpposingParty:
				if ( GetPartyTeamSize() != TeammateCount )
				{
					SetMatchmakingStepState(0, SS_Complete);
					SetMatchmakingStepState(1, SS_Active);
					PnlMatchmakeProgress.Steps[1].LabelStepName.SetDataStoreBinding(SearchingForTeammates);
					SetMatchmakingStepState(2, SS_Disabled);
				}
				else
				{
					SetMatchmakingStepState(0, SS_Complete);
					SetMatchmakingStepState(1, SS_Complete);
					SetMatchmakingStepState(2, SS_Active);
					PnlMatchmakeProgress.Steps[2].LabelStepName.SetDataStoreBinding(MatchmakeFindOpponent);
				}
				break;

			case MMS_FindingOpposingParty:
				SetMatchmakingStepState(0, SS_Complete);
				SetMatchmakingStepState(1, SS_Complete);
				SetMatchmakingStepState(2, SS_Active);
				PnlMatchmakeProgress.Steps[2].LabelStepName.SetDataStoreBinding(MatchmakeFindOpponent);
				break;

			case MMS_ConnectingToOpposingParty:
				SetMatchmakingStepState(0, SS_Complete);
				SetMatchmakingStepState(1, SS_Complete);
				SetMatchmakingStepState(2, SS_Active);
				PnlMatchmakeProgress.Steps[2].LabelStepName.SetDataStoreBinding(MatchmakeConnectOpponent);
				break;

			default:
				break;
		}
	}


	CurDisplayedMatchmakingStep = InMatchmakingStep;
}

/** Refresh the main button */
function RefreshMainPartyButton()
{
	local GearPartyGRI PartyGRI;

	PartyGRI = GearPartyGRI(GetGRI());
//`log("----> "@`showvar(GetWorldInfo().GRI) @ `showvar(PartyGRI) @ `showvar(GetGRI()));
	if ( IsOfficialMatch() )
	{
		if ( PartyGRI != None )
		{
			UpdateCursorIcon();

			PartyMsgPanel.SetVisibility( PartyGRI.bPartyLobbyErrorExists );

			if ( PartyGRI.MatchmakingState == MMS_NotMatchmaking )
			{
				if ( IsHordeGametype() )
				{
					MatchmakeButton.SetDataStoreBinding( ( GetPartyTeamSize() > GetNumValidPRIs() ) ? MainButtonFindTeam : MainButtonCreateMatch );
				}
				else
				{
					MatchmakeButton.SetDataStoreBinding( ( GetPartyTeamSize() > GetNumValidPRIs() ) ? MainButtonFindTeam : MainButtonFindMatch );
				}

				COGSymbolNormal.SetVisibility( !PartyGRI.bPartyLobbyErrorExists );
				COGSymbolMatchmaking.SetVisibility( false );
				COGSymbolBroken.SetVisibility( PartyGRI.bPartyLobbyErrorExists );
			}
			else if ( PartyGRI.MatchmakingState == MMS_CancelingMatchmaking )
			{
				MatchmakeButton.SetDataStoreBinding( MainButtonIsCancelling );

				COGSymbolNormal.SetVisibility( false );
				COGSymbolMatchmaking.SetVisibility( true );
				COGSymbolBroken.SetVisibility( false );
			}
			else if ( PartyGRI.MatchmakingState == MMS_ConnectingToOpposingParty )
			{
				MatchmakeButton.SetDataStoreBinding( MainButtonIsConnecting );

				COGSymbolNormal.SetVisibility( false );
				COGSymbolMatchmaking.SetVisibility( true );
				COGSymbolBroken.SetVisibility( false );
			}
			else
			{
				MatchmakeButton.SetDataStoreBinding( MainButtonIsMatchmaking );

				COGSymbolNormal.SetVisibility( false );
				COGSymbolMatchmaking.SetVisibility( true );
				COGSymbolBroken.SetVisibility( false );
			}

			RefreshPartyOptionsState();
		}

		UpdateOptionsAndStatusArea(PartyGRI.MatchmakingState);
	}
	else
	{
		PartyMsgPanel.SetVisibility( PartyGRI.bPartyLobbyErrorExists );
		COGSymbolNormal.SetVisibility( !PartyGRI.bPartyLobbyErrorExists );
		COGSymbolMatchmaking.SetVisibility( false );
		COGSymbolBroken.SetVisibility( PartyGRI.bPartyLobbyErrorExists );

		MatchmakeButton.SetDataStoreBinding( MainButtonCreateMatch );
		SetLabelButtonBGStyle(MatchmakeButton, 'FindGameButtonBG');

		if ( GetWorldInfo().NetMode == NM_Client || bGameStarting )
		{
			MatchmakeButtonIcon.SetVisibility( FALSE );
		}
		else
		{
			MatchmakeButtonIcon.SetVisibility( TRUE );
		}
	}




	RefreshPartyMessage();
	RefreshPartyBackgrounds();
}

/** Refreshes the enable/disable state of the partylist when in a public game */
function RefreshPartyOptionsState()
{
	local GearPartyGRI PartyGRI;
	local int PlayerIndex;

	PartyGRI = GearPartyGRI(GetGRI());

	// If this is the host we must disable/enable the party options depending on the matchmaking state
	if ( PartyGRI != None && PartyGRI.Role == ROLE_Authority )
	{
		PlayerIndex = GetBestPlayerIndex();
		if ( PartyGRI.MatchmakingState == MMS_NotMatchmaking && !bWaitingForMatchmakeToBegin )
		{
			lstPartyOptions.EnableWidget(PlayerIndex);
		}
		else
		{
			if ( bWaitingForMatchmakeToBegin )
			{
				bWaitingForMatchmakeToBegin = (PartyGRI.MatchmakingState == MMS_NotMatchmaking);
			}
			if (lstPartyOptions.IsFocused())
			{
				lstPartyOptions.KillFocus(none, PlayerIndex);
			}
			lstPartyOptions.DisableWidget(PlayerIndex);
		}
	}
}

/** Sets the image background for the two special backgrounds which will show error coloring */
function RefreshPartyBackgrounds()
{
	local GearPartyGRI PartyGRI;

	PartyGRI = GearPartyGRI(GetGRI());
	if (IsOfficialMatch())
	{
		if (PartyGRI != None)
		{
			PartyMsgImage.ImageComponent.SetImage(PartyGRI.bPartyLobbyErrorExists ? MatchOrangeBG : MatchRedBG);
		}
	}
	else
	{
		if (PartyGRI.bPartyLobbyErrorExists)
		{
			PartyMsgImage.ImageComponent.SetImage(MatchOrangeBG);
		}
		else if (IsSystemLinkMatch())
		{
			PartyMsgImage.ImageComponent.SetImage(MatchRedBG);
		}
	}
}

/** Whether the currently selected playlist contains maps from the "Flashback Map Pack" */
function bool IsAFlashbackMapPackPlaylist()
{
	local GearPartyGRI PartyGRI;
	local OnlinePlaylistManager PlaylistMan;
	local OnlineSubsystem OnlineSub;
	local array<int> ContentIds;
	local int DLCIdx;

	// Only public matches use playlists
	if (IsOfficialMatch())
	{
		PartyGRI = GearPartyGRI(GetGRI());
		if (PartyGRI != none)
		{
			OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
			if ( OnlineSub != None )
			{
				PlaylistMan = OnlinePlaylistManager(OnlineSub.GetNamedInterface('PlaylistManager'));
				if ( PlaylistMan != None )
				{
					PlaylistMan.GetContentIdsFromPlaylist(PartyGRI.PlaylistId, ContentIds);
					for (DLCIdx = 0; DLCIdx < ContentIds.length; DLCIdx++)
					{
						// 1 = Flashback Map Pack, so return true if one of the content ids is 1
						if (ContentIds[DLCIdx] == 1)
						{
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

/** Refresh the party message label */
function RefreshPartyMessage()
{
	local GearPartyGRI PartyGRI;
	local string PartyMessage;
	local int NumPRIs, PartyTeamSize;

	PartyMessage = "";

	if ( IsOfficialMatch() )
	{
		PartyGRI = GearPartyGRI(GetGRI());

		if ( PartyGRI != None )
		{
			NumPRIs = GetNumValidPRIs();
			PartyTeamSize = GetPartyTeamSize();
			// First handle any errors
			if ( PartyGRI.bPartyLobbyErrorExists )
			{
				if ( NumPRIs > PartyTeamSize )
				{
					PartyMessage = MatchErrorTooManyPlayers;
					PartyMessage = Repl( PartyMessage, "\`NUMPLAYERS\`", string(NumPRIs-PartyTeamSize) );
				}
				else if (PartyMemberIsAGuest())
				{
					PartyMessage = MatchErrorNoGuestInPublic;
				}
				else
				{
					if (IsAFlashbackMapPackPlaylist() && !bFlashbackMapPackCanBePurchased)
					{
						PartyMessage = MatchErrorMissingFlashback;
					}
					else
					{
						PartyMessage = MatchErrorMissingDLC;
					}
				}
			}
			else
			{
				switch ( PartyGRI.MatchmakingState )
				{
					case MMS_HostCancelingMatchmaking:
						PartyMessage = MatchmakeHostCancelled;
						break;
					case MMS_CancelingMatchmaking:
						PartyMessage = MatchmakeCancelMsg;
						break;
					case MMS_ReadingSkills:
						PartyMessage = MatchmakeReadSkillsMsg;
						break;
					case MMS_FindingBestParty:
						PartyMessage = MatchmakeFindBestMsg;
						PartyMessage = Repl( PartyMessage, "\`NUMPLAYERS\`", string(NumPRIs) );
						break;
					case MMS_WaitingForOpposingParty:
					case MMS_FindingAnyParty:
						PartyMessage = MatchmakeFindAnyMsg;
						PartyMessage = Repl( PartyMessage, "\`NUMPLAYERS\`", string(NumPRIs + PartyGRI.PartySlotsFilled) );
						break;
					case MMS_FindingOpposingParty:
						if (PartyGRI.TeamCount > 1)
						{
							PartyMessage = MatchmakeFindOpponent;
						}
						else
						{
							PartyMessage = MatchmakeConnectOpponent;
						}
						break;
					case MMS_ConnectingToOpposingParty:
						PartyMessage = MatchmakeConnectOpponent;
						break;
				}
			}

			PartyMsgLabel.SetDataStoreBinding( PartyMessage );
		}
	}
	else
	{
		NumPRIs = GetNumValidPRIs();
		PartyTeamSize = GetPartyTeamSize();
		if (NumPRIs > PartyTeamSize)
		{
			PartyMessage = GameErrorTooManyPlayers;
			PartyMessage = Repl( PartyMessage, "\`NUMPLAYERS\`", string(NumPRIs-PartyTeamSize) );
			PartyMsgLabel.SetDataStoreBinding( PartyMessage );
		}
	}
}

/**
 * Updates all widgets in the scene according to the current state of the game and status of the player, enabling &
 * disabling wigets as required.
 *
 * @param	PlayerIndex		the index of the player that generated this event
 */
function UpdateSceneState( optional int PlayerIndex=GetPlayerOwnerIndex(), optional bool bFocusUpdateOnly = false )
{
	local int Idx;
	local bool bPlayerButtonHasFocus;
	local GearPartyGRI GRI;

	if ( !IsEditor() && bPartySceneHasInitialized && IsSceneActive(true) )
	{
		if (!bIsDlcButtonEnabled)
		{
			bIsDlcButtonEnabled = PartyMemberLacksRequiredDLC();
			btnbarPlayers.EnableButton('DLC', GetBestPlayerIndex(), bIsDlcButtonEnabled, false);
		}

		GRI = GearPartyGRI(GetGRI());

		if (GRI != none)
		{
			if ( IsAllowedToCreateMatch(PlayerIndex) )
			{
				// these options can only be modified if we're the party leader
				if ( lstPartyOptions != None )
				{
					lstPartyOptions.EnableWidget(PlayerIndex);
				}
				if ( lstGameOptions != None )
				{
					lstGameOptions.EnableWidget(PlayerIndex);
				}
			}
			else if ( GRI != None && GRI.Role == ROLE_Authority )
			{
				// these options can only be modified if we're the party leader
				RefreshPartyOptionsState();

				if (lstGameOptions != None)
				{
					if (lstGameOptions.IsFocused())
					{
						lstGameOptions.KillFocus(none, PlayerIndex);
					}
					lstGameOptions.DisableWidget(PlayerIndex);
				}
			}
			else
			{
				// not allowed to start matches
				if (MatchmakeButton.IsFocused())
				{
					MatchmakeButton.KillFocus(none, PlayerIndex);
				}
				MatchmakeButton.DisableWidget(PlayerIndex);

				// none of these options can be modified if we're not the party leader
				if ( lstPartyOptions != None )
				{
					if (lstPartyOptions.IsFocused())
					{
						lstPartyOptions.KillFocus(none, PlayerIndex);
					}
					lstPartyOptions.DisableWidget(PlayerIndex);
				}
				if ( lstGameOptions != None )
				{
					if (lstGameOptions.IsFocused())
					{
						lstGameOptions.KillFocus(none, PlayerIndex);
					}
					lstGameOptions.DisableWidget(PlayerIndex);
				}
				if (pnlOptions.IsFocused())
				{
					pnlOptions.KillFocus(none, PlayerIndex);
				}
				pnlOptions.DisableWidget(PlayerIndex);

				// make sure the player's list is focused
				bPlayerButtonHasFocus = false;
				for ( Idx = 0; Idx < PlayerData.length; Idx++ )
				{
					if ( PlayerData[Idx].ParentButton.IsFocused() )
					{
						bPlayerButtonHasFocus = true;
						break;
					}
				}
				if ( !bPlayerButtonHasFocus &&
					PlayerData.length > 0 &&
					PlayerData[0].ParentButton != None )
				{
					PlayerData[0].ParentButton.SetFocus(None, PlayerIndex);
				}
			}

			if (!bFocusUpdateOnly)
			{
				btnbarPlayers.EnableButton('PostGame', PlayerIndex, ShouldEnableGameStats(), false);
				btnbarPlayers2.EnableButton('WeapOption', PlayerIndex, IsAllowedToOpenWeaponSwap(PlayerIndex), IsOfficialMatch() || (GetWorldInfo().NetMode == NM_Client));
				btnbarLAN.EnableButton('LANBrowser', PlayerIndex, CanOpenLANScene(PlayerIndex), true);
			}
		}
	}
}

/**
 * Copies option values from the player's profile for the currently selected gametype to the settings object for that gametype.
 */
function LoadGameSettingsFromProfile( optional bool bInitializingScene )
{
	local GearProfileSettings Profile;
	local OnlineGameSettings GameSettings;
	local GearPartyGRI PartyGRI;
	local int ValueId;
	local int SettingId;

	Profile = GetPlayerProfile(TEMP_SPLITSCREEN_INDEX);
	GameSettings = GetCurrentGameSettings();

	// set gamesettings
	if (Profile.GetProfileSettingValueId(Profile.const.VERSUS_GAMETYPE, ValueId))
	{
		// Map selection mode
		if (Profile.GetProfileSettingValueId(Profile.const.MAP_SELECTION_MODE, SettingId))
		{
			GameSettings.SetStringSettingValue(Profile.const.MAP_SELECTION_MODE, SettingId, false);
		}
		// Set settings common to every gametype except Horde
		if (ValueId != eGEARMP_CombatTrials)
		{
			// Num bots
			if (Profile.GetProfileSettingValueInt(Profile.const.MPNumBots, SettingId))
			{
				GameSettings.SetStringSettingValue(CONTEXT_NUMBOTS, SettingId, false);
			}
			// Bot difficulty
			if (Profile.GetProfileSettingValueInt(Profile.const.MPBotDiff, SettingId))
			{
				GameSettings.SetStringSettingValue(CONTEXT_BOTDIFFICULTY, SettingId, false);
			}
			// Bleedout time
			if (Profile.GetProfileSettingValueInt(Profile.const.MPBleedout, SettingId))
			{
				GameSettings.SetStringSettingValue(CONTEXT_BLEEDOUTTIME, SettingId, false);
			}
			// Friendly fire
			if (Profile.GetProfileSettingValueInt(Profile.const.MPFriendlyFire, SettingId))
			{
				GameSettings.SetStringSettingValue(CONTEXT_FRIENDLYFIRE, SettingId, false);
			}
			// Weapon swap
			if (Profile.GetProfileSettingValueInt(Profile.const.MPWeaponSwap, SettingId))
			{
				GameSettings.SetStringSettingValue(CONTEXT_WEAPONSWAP, SettingId, false);
			}
			// Round time
			switch (ValueId)
			{
				case eGEARMP_Warzone:
				case eGEARMP_Execution:
				case eGEARMP_Wingman:
					//
					if (Profile.GetProfileSettingValueInt(Profile.const.MPRoundTime, SettingId))
					{
						GameSettings.SetStringSettingValue(CONTEXT_ROUNDTIME, SettingId, false);
					}
					break;
			}
		}
		// Set game specific settings
		switch (ValueId)
		{
			case eGEARMP_CombatTrials:
				// Horde difficulty
				if (Profile.GetProfileSettingValueInt(Profile.const.HordeEnemyDiff, SettingId))
				{
					GameSettings.SetStringSettingValue(CONTEXT_ENEMYDIFFICULTY, SettingId, false);
				}
				// Horde wave
				if (Profile.GetProfileSettingValueInt(Profile.const.HordeWave, SettingId))
				{
					GameSettings.SetStringSettingValue(CONTEXT_HORDE_WAVE, SettingId, false);
				}
				break;
			case eGEARMP_Annex:
				// Annex roundscore
				if (Profile.GetProfileSettingValueInt(Profile.const.AnnexRoundScore, SettingId))
				{
					GameSettings.SetStringSettingValue(CONTEXT_ROUNDSCORE, SettingId, false);
				}
				break;
			case eGEARMP_Wingman:
				// Wingman Score
				if (Profile.GetProfileSettingValueInt(Profile.const.WingmanScore, SettingId))
				{
					GameSettings.SetStringSettingValue(CONTEXT_WINGMAN_SCOREGOAL, SettingId, false);
				}
				break;
			case eGEARMP_KOTH:
				// KOTH roundscore
				if (Profile.GetProfileSettingValueInt(Profile.const.KOTHRoundScore, SettingId))
				{
					GameSettings.SetStringSettingValue(CONTEXT_ROUNDSCORE, SettingId, false);
				}
				break;
		}

		// Set team size
		if ( !bInitializingScene )
		{
			PartyGRI = GearPartyGRI(GetGRI());
			if ( PartyGRI != None )
			{
				PartyGRI.TeamSize = (SettingId == eGEARMP_CombatTrials) ? 5 : 10;
			}
		}
	}
}

/**
 * Copes option values from the current game's settings to the player's profile.
 *
 * @param	WriteProfileSettingsCompleteDelegate	the function to call when the profile save is complete.
 */
function bool SaveGameSettingsToProfile( int PlayerIndex, optional delegate<OnlinePlayerInterface.OnWriteProfileSettingsComplete> WriteProfileSettingsCompleteDelegate, optional bool bSaveProfile=true )
{
	local OnlineGameSettings GameSettings;
	local GearProfileSettings Profile;
	local bool bResult;
	local int ValueId;
	local int SettingId;

	if ( IsPartyLeader(PlayerIndex) )
	{
		GameSettings = GetCurrentGameSettings();
		Profile = GetPlayerProfile(PlayerIndex);

		// set gamesettings
		if (GameSettings.GetStringSettingValue(class'GearVersusGameSettings'.const.CONTEXT_VERSUSMODES, ValueId))
		{
			// Map selection mode
			if (GameSettings.GetStringSettingValue(Profile.const.MAP_SELECTION_MODE, SettingId))
			{
				Profile.SetProfileSettingValueId(Profile.const.MAP_SELECTION_MODE, SettingId);
			}
			// Set settings common to every gametype except Horde
			if (ValueId != eGEARMP_CombatTrials)
			{
				// Num bots
				if (GameSettings.GetStringSettingValue(CONTEXT_NUMBOTS, SettingId))
				{
					Profile.SetProfileSettingValueInt(Profile.const.MPNumBots, SettingId);
				}
				// Bot difficulty
				if (GameSettings.GetStringSettingValue(CONTEXT_BOTDIFFICULTY, SettingId))
				{
					Profile.SetProfileSettingValueInt(Profile.const.MPBotDiff, SettingId);
				}
				// Bleedout time
				if (GameSettings.GetStringSettingValue(CONTEXT_BLEEDOUTTIME, SettingId))
				{
					Profile.SetProfileSettingValueInt(Profile.const.MPBleedout, SettingId);
				}
				// Friendly fire
				if (GameSettings.GetStringSettingValue(CONTEXT_FRIENDLYFIRE, SettingId))
				{
					Profile.SetProfileSettingValueInt(Profile.const.MPFriendlyFire, SettingId);
				}
				// Weapon swap
				if (GameSettings.GetStringSettingValue(CONTEXT_WEAPONSWAP, SettingId))
				{
					Profile.SetProfileSettingValueInt(Profile.const.MPWeaponSwap, SettingId);
				}
				// Non-respawning modes
				switch (ValueId)
				{
					case eGEARMP_Warzone:
					case eGEARMP_Execution:
					case eGEARMP_Wingman:
						// Round time
						if (GameSettings.GetStringSettingValue(CONTEXT_ROUNDTIME, SettingId))
						{
							Profile.SetProfileSettingValueInt(Profile.const.MPRoundTime, SettingId);
						}
						break;
				}
			}
			// Set game specific settings
			switch (ValueId)
			{
				case eGEARMP_CombatTrials:
					// Horde difficulty
					if (GameSettings.GetStringSettingValue(CONTEXT_ENEMYDIFFICULTY, SettingId))
					{
						Profile.SetProfileSettingValueInt(Profile.const.HordeEnemyDiff, SettingId);
					}
					// Horde wave
					if (GameSettings.GetStringSettingValue(CONTEXT_HORDE_WAVE, SettingId))
					{
						Profile.SetProfileSettingValueInt(Profile.const.HordeWave, SettingId);
					}
					break;
				case eGEARMP_Annex:
					// Annex roundscore
					if (GameSettings.GetStringSettingValue(CONTEXT_ROUNDSCORE, SettingId))
					{
						Profile.SetProfileSettingValueInt(Profile.const.AnnexRoundScore, SettingId);
					}
					break;
				case eGEARMP_Wingman:
					// Wingman Score
					if (GameSettings.GetStringSettingValue(CONTEXT_WINGMAN_SCOREGOAL, SettingId))
					{
						Profile.SetProfileSettingValueInt(Profile.const.WingmanScore, SettingId);
					}
					break;
				case eGEARMP_KOTH:
					// KOTH roundscore
					if (GameSettings.GetStringSettingValue(CONTEXT_ROUNDSCORE, SettingId))
					{
						Profile.SetProfileSettingValueInt(Profile.const.KOTHRoundScore, SettingId);
					}
					break;
			}

			if (bSaveProfile)
			{
				bResult = SaveProfile(PlayerIndex, WriteProfileSettingsCompleteDelegate);
			}
			else
			{
				bResult = true;
			}
		}
	}

	return bResult;
}

/**
 * Copies the host's choices for any settings which come from the profile into a different game settings value element so that clients can
 * easily retrieve that information without affecting their own profile values.
 */
function UpdateRemoteSettings()
{
	local GearProfileSettings Profile;
	local int SettingsIdx, MatchModeValue, PartyTypeValue;
	local GearUIDataStore_GameSettings GameSettingsDS;

	if ( IsPartyLeader(TEMP_SPLITSCREEN_INDEX) )
	{
		Profile = GetPlayerProfile(TEMP_SPLITSCREEN_INDEX);
		GameSettingsDS = GetGameSettingsDataStore();
		if ( Profile != None && GameSettingsDS != None )
		{
			if ( Profile.GetProfileSettingValueId(Profile.const.VERSUS_MATCH_MODE, MatchModeValue) &&
				Profile.GetProfileSettingValueId(Profile.const.VERSUS_PARTY_TYPE, PartyTypeValue) )
			{
				for ( SettingsIdx = 0; SettingsIdx < GameSettingsDS.GameSettingsCfgList.length; SettingsIdx++ )
				{
					GameSettingsDS.GameSettingsCfgList[SettingsIdx].GameSettings.SetStringSettingValue(Profile.const.VERSUS_MATCH_MODE, MatchModeValue, false);
					GameSettingsDS.GameSettingsCfgList[SettingsIdx].GameSettings.SetStringSettingValue(Profile.const.VERSUS_PARTY_TYPE, PartyTypeValue, false);
				}
			}
		}
	}
}

/**
 * @return	the GameSettings object for the currently selected gametype.
 */
final function GearVersusGameSettings GetCurrentVersusGameSettings()
{
	return GearVersusGameSettings(GetCurrentGameSettings());
}

/** ReBuild the DesciptionList for displaying the descriptions for all of the host options */
function RebuildDescriptionList()
{
	// No point in doing anything if there's no description label to deal with
	if ( DescriptionLabel != None )
	{
		// Empty the description list
		DescriptionList.length = 0;

		// Add the Party options to the description list
		AddObjectListElementsToDescriptionList( lstPartyOptions, class'UIDataProvider_MenuItem' );

		// Add the Game options to the description list
		AddObjectListElementsToDescriptionList( lstGameOptions, class'UIDataProvider_MenuItem' );

		// Add the other selections to the description list
		AddMiscWidgetsToDescriptionList();
	}
}

/** Append the other selections to the description list */
function AddMiscWidgetsToDescriptionList()
{
	local int NewIdx, PlayerIdx;
	local string WidgetName;

	if ( MatchmakeButton != None )
	{
		NewIdx = DescriptionList.length;
		DescriptionList.length = NewIdx + 1;
		DescriptionList[NewIdx].LocalizationPath = IsOfficialMatch() ? MainButtonDoMatchmake_Desc : MainButtonCreateMatch_Desc;
		DescriptionList[NewIdx].WidgetToDescribeName = 'btnFindGame';
		DescriptionList[NewIdx].WidgetToDescribe = MatchmakeButton;
	}

	for ( PlayerIdx = 0; PlayerIdx < PlayerData.length; PlayerIdx++ )
	{
		if ( PlayerData[PlayerIdx].ParentButton != None )
		{
			NewIdx = DescriptionList.length;
			DescriptionList.length = NewIdx + 1;
			WidgetName = "btnPlayer" $ PlayerIdx;
			DescriptionList[NewIdx].LocalizationPath = (PlayerData[PlayerIdx].PlayerPRI != None) ? PlayerClickFull_Desc : PlayerClickEmpty_Desc;
			DescriptionList[NewIdx].WidgetToDescribeName = Name(WidgetName);
			DescriptionList[NewIdx].WidgetToDescribe = PlayerData[PlayerIdx].ParentButton;
		}
	}
}

/** Get dynamically changing description text at a particular index */
function string GetDynamicDescriptionText( int DescriptionIndex )
{
	local string DescribeString, WidgetName;
	local UIObject WidgetToDescribe;
	local int PlayerIdx;
	local GearPartyGRI PartyGRI;
	local int CurrentIndex;

	WidgetToDescribe = None;
	DescribeString = "";

	if ( DescriptionList.length > DescriptionIndex )
	{
		WidgetToDescribe = DescriptionList[DescriptionIndex].WidgetToDescribe;
	}

	// Matchmode tooltips
	if (MatchModeList != none && MatchModeList.IsFocused())
	{
		if (MatchModeList.IsExpanded() || MatchModeList.IsExpanding())
		{
			CurrentIndex = MatchModeList.GetCurrentItem();
			switch (CurrentIndex)
			{
				case eGVMT_Official:
					DescribeString = MatchModeDesc_Public;
					break;
				case eGVMT_Custom:
					DescribeString = MatchModeDesc_Private;
					break;
				case eGVMT_SystemLink:
					DescribeString = MatchModeDesc_LAN;
					break;
				case eGVMT_Local:
					DescribeString = MatchModeDesc_Local;
					break;
			}
		}
		else
		{
			DescribeString = GameType_Desc;
		}
	}
	// Map selection tooltips
	else if (MapSelectionList != none && MapSelectionList.IsFocused())
	{
		if (MapSelectionList.IsExpanded() || MapSelectionList.IsExpanding())
		{
			CurrentIndex = MapSelectionList.GetCurrentItem();
			switch (CurrentIndex)
			{
			case eGEARMAPSELECT_VOTE:
				DescribeString = MapSelectDesc_Vote;
				break;
			case eGEARMAPSELECT_HOSTSELECT:
				DescribeString = MapSelectDesc_Host;
				break;
			}
		}
		else
		{
			DescribeString = MapSelectType_Desc;
		}
	}
	// Weapon swap tooltips
	else if (WeaponSwapList != none && WeaponSwapList.IsFocused())
	{
		if (WeaponSwapList.IsExpanded() || WeaponSwapList.IsExpanding())
		{
			CurrentIndex = WeaponSwapList.GetCurrentItem();
			switch (CurrentIndex)
			{
			case CONTEXT_WEAPONSWAP_CYCLE:
				DescribeString = WeapSelectDesc_Cycle;
				break;
			case CONTEXT_WEAPONSWAP_NORMAL:
				DescribeString = WeapSelectDesc_Normal;
				break;
			case CONTEXT_WEAPONSWAP_CUSTOM:
				DescribeString = WeapSelectDesc_Custom;
				break;
			}
		}
		else
		{
			DescribeString = WeapSwap_Desc;
		}
	}
	// Check for Uber button tooltips
	else if ( WidgetToDescribe != None )
	{
		if ( WidgetToDescribe.WidgetTag == 'btnFindGame' )
		{
			if ( IsOfficialMatch() )
			{
				PartyGRI = GearPartyGRI(GetGRI());
				if ( PartyGRI != None )
				{
					switch ( PartyGRI.MatchmakingState )
					{
					case MMS_NotMatchmaking:
						DescribeString = MainButtonDoMatchmake_Desc;
						break;
					case MMS_CancelingMatchmaking:
						DescribeString = MainButtonIsCancelling_Desc;
						break;
					case MMS_ConnectingToOpposingParty:
						DescribeString = MainButtonIsConnecting_Desc;
						break;
					case MMS_ReadingSkills:
					case MMS_FindingBestParty:
					case MMS_FindingAnyParty:
					case MMS_FindingOpposingParty:
						DescribeString = MainButtonIsMatchmaking_Desc;
						break;
					}
				}
			}
			else
			{
				DescribeString = MainButtonCreateMatch_Desc;
			}
		}
		// Player tooltips
		else
		{
			for ( PlayerIdx = 0; PlayerIdx < PlayerData.length; PlayerIdx++ )
			{
				WidgetName = "btnPlayer" $ PlayerIdx;
				if ( WidgetToDescribe.WidgetTag == Name(WidgetName) )
				{
					DescribeString = (PlayerData[PlayerIdx].PlayerPRI != None) ? PlayerClickFull_Desc : PlayerClickEmpty_Desc;
					break;
				}
			}
		}
	}

	return DescribeString;
}

/** Called when the description goes active for the widget */
function OnDescriptionActive( int DescriptionIndex )
{
	local string DescribeText;

	DescribeText = GetDynamicDescriptionText( DescriptionIndex );
	if ( DescribeText != "" )
	{
		DescriptionList[DescriptionIndex].LocalizationPath = DescribeText;
	}
}

/** Refresh the description widget */
function RefreshDescriptionWidget()
{
	local string DescribeText;

	DescribeText = GetDynamicDescriptionText( CurrentDescriptionListIndex );
	if ( DescribeText != "" &&
		 DescribeText != DescriptionList[CurrentDescriptionListIndex].LocalizationPath )
	{
		DescriptionList[CurrentDescriptionListIndex].LocalizationPath = DescribeText;
		UpdateDescriptionLabel( DescriptionList[CurrentDescriptionListIndex].LocalizationPath, DescriptionList[CurrentDescriptionListIndex].WidgetToDockTo, DescriptionList[CurrentDescriptionListIndex].FaceToDockTo, DescriptionList[CurrentDescriptionListIndex].DockPadding );
	}
}

/**
 * Determines whether the specified player is allowed to create a game session.
 *
 * @param	PlayerIndex		the index of the player that generated this event
 *
 * @return	TRUE if the player is allowed to create matches.
 */
function bool IsAllowedToCreateMatch( int PlayerIndex )
{
	if (IsPartyLeader(PlayerIndex))
	{
		// if we have all required players in our one party, then we can create a match
		// or the configured match type is not public
		return (!IsOfficialMatch() || (GearPartyGRI(GetGRI()).TeamCount == 1 && GetPartyTeamSize() == GetNumValidPRIs()));
	}
	return false;
}

/**
 * Determines whether all conditions required to begin a match have been met.
 *
 * @param	PlayerIndex		the index of the player that generated this event
 */
function bool IsReadyToCreateMatch( int PlayerIndex )
{
	local GearPartyGame_Base PartyGame;
	local bool bResult;

	if ( IsAllowedToCreateMatch(PlayerIndex) )
	{
		// if we have the correct number of players for the selected gametype and match type, we're ready to start
		PartyGame = GetPartyGameInfo();
		if ( PartyGame != None )
		{
			bResult = PartyGame.CanStartMatch();
		}
		bResult = true;
	}

	return bResult;
}

/**
 * Determines whether the specified player is allowed to initiate an online search for a match.
 *
 * @param	PlayerIndex		the index of the player that generated this event
 */
function bool IsAllowedToSearchForMatches( int PlayerIndex )
{
	if (IsPartyLeader(PlayerIndex))
	{
		return IsOfficialMatch();
	}
	return false;
}

/**
 * Determines whether all conditions required to begin searching for a match have been met.
 *
 * @param	PlayerIndex		the index of the player that generated this event
 */
function bool IsReadyToSearchForMatches( int PlayerIndex )
{
	local GearPartyGame_Base PartyGame;
	local bool bResult;

	if ( IsAllowedToSearchForMatches(PlayerIndex) )
	{
		// if we have the correct number of players for the selected gametype and match type, we're ready to start
		PartyGame = GetPartyGameInfo();
		if ( PartyGame != None )
		{
			bResult = !PartyGame.ArePlayersNeeded();
		}
	}

	return bResult;
}

/**
 * Determines whether the specified player is allowed to initiate an online search for a match.
 *
 * @param PlayerIndex the index of the player that generated this event
 * @param bShouldSkipPartyLeaderCheck whether to check is party leader
 */
function bool IsMatchmaking(int PlayerIndex,optional bool bShouldSkipPartyLeaderCheck)
{
	local GearPartyGRI PartyGRI;

	PartyGRI = GearPartyGRI(GetGRI());

	// Explicitly check our matchmaking state
	if ( PartyGRI != None )
	{
		switch (PartyGRI.MatchmakingState)
		{
			case MMS_ReadingSkills:
			case MMS_FindingBestParty:
			case MMS_FindingAnyParty:
			case MMS_FindingOpposingParty:
			case MMS_WaitingForOpposingParty:
			case MMS_ConnectingToOpposingParty:
				return bShouldSkipPartyLeaderCheck || IsPartyLeader(PlayerIndex);
		}
	}

	return false;
}

/**
 * Wrapper for determining whether the match is configured as an official public match
 */
function bool IsOfficialMatch()
{
	local GearProfileSettings Profile;
	local GearVersusGameSettings CurrentSettings;
	local int SelectedMatchType;
	local bool bResult;

	if ( IsPartyLeader(TEMP_SPLITSCREEN_INDEX) )
	{
		Profile = GetPlayerProfile(TEMP_SPLITSCREEN_INDEX);
		if ( Profile != None && Profile.GetProfileSettingValueId(Profile.const.VERSUS_MATCH_MODE, SelectedMatchType) )
		{
			bResult = SelectedMatchType == eGVMT_Official;
		}
	}
	else
	{
		// pull the remote value
		CurrentSettings = GetCurrentVersusGameSettings();
		if ( CurrentSettings != None && CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, SelectedMatchType) )
		{
			bResult = SelectedMatchType == eGVMT_Official;
		}
	}

	return bResult;
}

/**
 * Wrapper for determining whether the match is configured as an custom match (i.e. modification of game settings are allowed)
 */
function bool IsCustomMatch()
{
	local GearProfileSettings Profile;
	local int SelectedMatchType;
	local GearVersusGameSettings CurrentSettings;
	local bool bResult;

	if ( IsPartyLeader(TEMP_SPLITSCREEN_INDEX) )
	{
		Profile = GetPlayerProfile(TEMP_SPLITSCREEN_INDEX);
		if ( Profile != None && Profile.GetProfileSettingValueId(Profile.const.VERSUS_MATCH_MODE, SelectedMatchType) )
		{
			bResult = SelectedMatchType == eGVMT_Custom;
		}
	}
	else
	{
		// pull the remote value
		CurrentSettings = GetCurrentVersusGameSettings();
		if ( CurrentSettings != None && CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, SelectedMatchType) )
		{
			bResult = SelectedMatchType == eGVMT_Custom;
		}
	}

	return bResult;
}

/**
 * Wrapper for determining whether the match is configured as an custom match (i.e. modification of game settings are allowed)
 */
function bool IsSystemLinkMatch()
{
	local GearProfileSettings Profile;
	local int SelectedMatchType;
	local GearVersusGameSettings CurrentSettings;
	local bool bResult;

	if ( IsPartyLeader(TEMP_SPLITSCREEN_INDEX) )
	{
		Profile = GetPlayerProfile(TEMP_SPLITSCREEN_INDEX);
		if ( Profile != None && Profile.GetProfileSettingValueId(Profile.const.VERSUS_MATCH_MODE, SelectedMatchType) )
		{
			bResult = SelectedMatchType == eGVMT_SystemLink;
		}
	}
	else
	{
		// pull the remote value
		CurrentSettings = GetCurrentVersusGameSettings();
		if ( CurrentSettings != None && CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, SelectedMatchType) )
		{
			bResult = SelectedMatchType == eGVMT_SystemLink;
		}
	}

	return bResult;
}

/**
 * Wrapper for determining whether the match is configured as local match
 */
function bool IsLocalMatch()
{
	local GearProfileSettings Profile;
	local GearVersusGameSettings CurrentSettings;
	local int SelectedMatchType;
	local bool bResult;

	if ( IsPartyLeader(TEMP_SPLITSCREEN_INDEX) )
	{
		Profile = GetPlayerProfile(TEMP_SPLITSCREEN_INDEX);
		if ( Profile != None && Profile.GetProfileSettingValueId(Profile.const.VERSUS_MATCH_MODE, SelectedMatchType) )
		{
			bResult = SelectedMatchType == eGVMT_Local;
		}
	}
	else
	{
		// pull the remote value
		CurrentSettings = GetCurrentVersusGameSettings();
		if ( CurrentSettings != None && CurrentSettings.GetStringSettingValue(class'GearProfileSettings'.const.VERSUS_MATCH_MODE, SelectedMatchType) )
		{
			bResult = SelectedMatchType == eGVMT_Local;
		}
	}

	return bResult;
}

/**
 * Determine whether the player is allowed to open the weapon swap configuration scene
 *
 * @param	PlayerIndex		the index for the player to check
 * @param	bRenderingOnly	specify TRUE if checking conditions for displaying the callout button; FALSE if checking whether to actually
 *							allow the scene to be opened.
 *
 * @return	TRUE if all conditions have been met for configuring weapon swap.
 */
final function bool IsAllowedToOpenWeaponSwap( int PlayerIndex, optional bool bRenderingOnly )
{
	if ( IsPartyLeader(PlayerIndex) &&
		 IsWeaponSwapEnabled() &&
		 !IsOfficialMatch() )
	{
		return true;
	}

	return false;
}

/**
 * Simple wrapper method for determining whether the user turned on weapon swap.
 */
final function bool IsWeaponSwapEnabled()
{
	local OnlineGameSettings GameSettings;
	local int SelectedContext;
	local bool bResult;

	GameSettings = GetCurrentGameSettings();
	if ( GameSettings != None )
	{
		bResult = GameSettings.GetStringSettingValue(CONTEXT_WEAPONSWAP, SelectedContext) && SelectedContext == CONTEXT_WEAPONSWAP_CUSTOM;
	}

	return bResult;
}

/**
 * Notifies the online subsystem to begin creating a new match.
 *
 * @fixme ronp - remove this function...it will be handled by the gameinfo it seems
 *
 * @param	PlayerIndex		the index of the player that generated this event
 */
function bool CreateMatch( int PlayerIndex )
{
	local GearPartyGame_Base PartyGame;
	local int ControllerId;
	local bool bResult;

	PartyGame = GetPartyGameInfo();
	if ( PartyGame != None )
	{
		if ( IsReadyToCreateMatch(PlayerIndex) )
		{
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
			if ( ControllerId != 255 )
			{
				SaveGameSettingsToProfile(PlayerIndex, OnCreateMatchProfileWriteComplete);
				bResult = true;
			}
			else
			{
				//@todo error message
			}
		}
		else
		{
			//@todo error message
		}
	}
	else
	{
		`log("Couldn't create match because current game isn't GearPartyGame:" @ `showobj(GetCurrentGameInfo(),CurrentGame));
	}

	return bResult;
}

/**
 * Notifies the online subsystem to begin searching for a match.
 *
 * @param	PlayerIndex		the index of the player that generated this event
 */
function bool FindMatch( int PlayerIndex )
{
	local GearPartyGame_Base PartyGame;
	local int ControllerId;
	local bool bResult;

	PartyGame = GetPartyGameInfo();
	if ( PartyGame != None )
	{
		if ( IsReadyToSearchForMatches(PlayerIndex) )
		{
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
			if ( ControllerId != 255 )
			{
				bWaitingForMatchmakeToBegin = true;
				SaveGameSettingsToProfile(PlayerIndex, OnFindMatchProfileWriteComplete);
				bResult = true;
			}
			else
			{
				`warn(`location $ ": Invalid PlayerIndex specified:" @ PlayerIndex);
			}
		}
		else
		{
			//@todo error message
		}
	}
	else
	{
		`log("Couldn't search for match because current game isn't GearPartyGame:" @ `showobj(GetCurrentGameInfo(),CurrentGame));
	}

	return bResult;
}

/** Notifies the online subsystem to begin canceling the matchmaking process */
function bool CancelMatchmaking()
{
	local GearPartyGame_Base PartyGame;
	local GearPartyGRI GRI;
	local LocalPlayer LP;
	local GearPartyPC PC;
	local int NumPRIs;

	GRI = GearPartyGRI(GetGRI());
	NumPRIs = GetNumValidPRIs();

	if ( GRI != None && GRI.MatchmakingState > MMS_CancelingMatchmaking && GRI.MatchmakingState != MMS_ConnectingToOpposingParty )
	{
		if (GRI.TeamCount == 1 && GetPartyTeamSize() == NumPRIs + GRI.PartySlotsFilled)
		{
			// Horde match that is full, so no canceling
			return false;
		}
		else
		{
			PartyGame = GetPartyGameInfo();
			if (PartyGame != None)
			{
				PartyGame.CancelPartyMatchmaking();
			}
			else
			{
				LP = GetPlayerOwner(GetBestPlayerIndex());

				PC = GearPartyPC(LP.Actor);
				if (PC != None)
				{
					PC.ServerCancelMatchmaking();
				}
				else
				{
					return false;
				}
			}
			return true;
		}
	}

	return false;
}

/** Called when the profile is done writing */
function OnCreateMatchProfileWriteComplete(byte ControllerId,bool bWasSuccessful)
{
	local LocalPlayer LP;
	local GearPC GearPO;
	local GearPartyGame_Base PartyGame;

	LP = GetPlayerOwner(Class'UIInteraction'.static.GetPlayerIndex(ControllerId));

	GearPO = GearPC(LP.Actor);
	GearPO.ClearSaveProfileDelegate(OnCreateMatchProfileWriteComplete);

	PartyGame = GetPartyGameInfo();
	if ( PartyGame.StartPartyMatchmaking(LP.ControllerId) )
	{
		//ShowLoadingMovie(true);
	}
}

/** Called when the profile is done writing */
function OnFindMatchProfileWriteComplete(byte LocalUserNum,bool bWasSuccessful)
{
	local LocalPlayer LP;
	local GearPC GearPO;
//	local GameUISceneClient GameSceneClient;
	local GearPartyGame_Base PartyGame;

	LP = GetPlayerOwner(Class'UIInteraction'.static.GetPlayerIndex(LocalUserNum));

	GearPO = GearPC(LP.Actor);
	GearPO.ClearSaveProfileDelegate(OnFindMatchProfileWriteComplete);

 	PartyGame = GetPartyGameInfo();
	if ( !PartyGame.StartPartyMatchmaking(LP.ControllerId) )
	{
		bWaitingForMatchmakeToBegin = false;
	}

// 	if ( PartyGame.StartPartyMatchmaking(LP.ControllerId) )
// 	{
// 		GameSceneClient = GetSceneClient();
// 		GameSceneClient.CloseScene(Self, false);
// 	}
}

/**
 * Called when the EndOnlineGame task for the 'Party' session is completed, resetting
 * the party session to be joinable.
 *
 * @param	SessionName		the name of the session that the task operated on
 * @param	bWasSuccessful	TRUE if the task was completed successfully.
 */
function PartyEndComplete(name SessionName,bool bWasSuccessful)
{
	Super.PartyEndComplete(SessionName, bWasSuccessful);

	UpdateSceneState();
}

/**
 * Called when the EndOnlineGame task for the 'Party' session is completed, in cases
 * where we want to completely tear down the party so that it no longer exists.
 *
 * @param	SessionName		the name of the session that the task operated on
 * @param	bWasSuccessful	TRUE if the task was completed successfully.
 */
function PartyEndCompleteDestroy(name SessionName,bool bWasSuccessful)
{
	Super.PartyEndCompleteDestroy(SessionName, bWasSuccessful);
}

/** Publishes the current match mode to the party session object */
function ApplyMatchModeUpdate( int PlayerIndex )
{
	local GearProfileSettings Profile;
	local int SelectedContext;

	Profile = GetPlayerProfile(PlayerIndex);
	if ( Profile != None && Profile.GetProfileSettingValueId(Profile.const.VERSUS_MATCH_MODE, SelectedContext) )
	{
		switch ( SelectedContext )
		{
			case eGVMT_Official:
			case eGVMT_Custom:
				AttemptLIVEMatchTransition( SelectedContext, Profile );
				break;
			break;

			case eGVMT_SystemLink:
				AttemptSystemLinkTransition();
			break;

			case eGVMT_Local:
				AttemptLocalTransition();
			break;
		}
	}
}

/**
 * First checks to see if there are any local players who do not have LIVE permissions and rejects the change if so
 * Next it checks to see if there are any non-local players who need kicked and prompts for the kick
 * Next it transitions to the LIVE match mode
 */
function AttemptLIVEMatchTransition( int LIVEMatchMode, GearProfileSettings Profile )
{
	local GearPartyGame_Base PartyGI;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local int PlayerIndex;
	local GameViewportClient VPClient;

	PartyGI = GetPartyGameInfo();
	PlayerIndex = GetBestPlayerIndex();
	VPClient = PartyGI.GetPlayerOwnerViewportClient(PlayerIndex);

	// Return if we can't get a GameViewportClient or UI Controller
	if ( VPClient == None || VPClient.UIController == None )
	{
		return;
	}

	// Check for a network connection first
	if ( !VPClient.UIController.HasLinkConnection() )
	{
		DisplayErrorMessage("NoLinkConnection_Message", "NoLinkConnection_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
		CancelLIVEMatchSelection();
		return;
	}

	// Check for LIVE logins for all local players
	if ( VPClient.UIController.GetLowestLoginStatusOfControllers() != LS_LoggedIn )
	{
		DisplayErrorMessage("NeedGoldTier_Message", "NeedGoldTier_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
		CancelLIVEMatchSelection();
		return;
	}

	// Check for guest accounts if they're attempting a public match
	if (LIVEMatchMode == eGVMT_Official &&
		VPClient.UIController.GetNumGuestsLoggedIn() > 0)
	{
		DisplayErrorMessage("IllegalGuestAction_Message", "IllegalGuestAction_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
		CancelLIVEMatchSelection();
		return;
	}

	// Check for whether all local players can play online
	if ( !VPClient.UIController.CanAllPlayOnline() )
	{
		DisplayErrorMessage("NeedOnlinePermission_Message", "NeedOnlinePermission_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
		CancelLIVEMatchSelection();
		return;
	}

	// Check for whether the NAT is open.  If so warn them but don't fail
	if ( !PartyGI.IsNatOkForParty() )
	{
		DisplayErrorMessage("StrictNAT_Message", "StrictNAT_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
	}

	// If the new match mode is public or private and the previous match mode was public or private, update the session
	if ( PreviousMatchMode == eGVMT_Official || PreviousMatchMode == eGVMT_Custom )
	{
		// Set the match mode for the new setting
		PartyGI.ChangeMatchModeSettings( PartyGI.PartySettings, LIVEMatchMode, Profile );

		// Tell live to update the advertised settings
		PartyGI.UpdatePartySettings();

		// Keep track of the last match mode so we can back out of LAN and Player matches
		PreviousMatchMode = LIVEMatchMode;

		// Update the lobby and the clients
		SetMenuItemsInPartyList();
		UpdateRemoteSettings();
	}
	// Else if the match mode was system link, we need to check for kicking non-local players and then rebuilding the party
	else if ( PreviousMatchMode == eGVMT_SystemLink )
	{
		// Prompt a warning if there are non-local players as they will be kicked
		if ( PartyHasNonLocalPlayers() )
		{
			ButtonAliases.AddItem('GenericCancel');
			ButtonAliases.AddItem('GenericAccept');
			GameSceneClient = GetSceneClient();
			GameSceneClient.ShowUIMessage(
				'ConfirmNetworkedPlayersKicked',
				"<Strings:GearGameUI.MessageBoxStrings.NetworkPlayersKicked_Title>",
				"<Strings:GearGameUI.MessageBoxStrings.NetworkPlayersKicked_Desc>",
				"",
				ButtonAliases,
				OnKickNetworkedPlayersForLIVEParty_Confirm,
				GetPlayerOwner(GetBestPlayerIndex()) );
		}
		// Else go ahead and make the transition
		else
		{
			TransitionToLIVEParty();
		}
	}
	// The match was Local so we need to rebuild the party
	else
	{
		TransitionToLIVEParty();
	}
}

/** Make the setting go back to what it was before the host selected a LIVE seleciton */
function CancelLIVEMatchSelection()
{
	local GearProfileSettings Profile;

	Profile = GetPlayerProfile(GetBestPlayerIndex());
	if ( Profile != None )
	{
		// Reset the profile to the previous match mode
		Profile.SetProfileSettingValueId( Profile.const.VERSUS_MATCH_MODE, PreviousMatchMode );
		MatchModeList.RefreshSubscriberValue();
	}
}

/** Callback from asking the host if they really want to kick non-local players */
function bool OnKickNetworkedPlayersForLIVEParty_Confirm( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'GenericAccept' )
	{
		TransitionToLIVEParty();
	}
	else
	{
		CancelLIVEMatchSelection();
	}

	return true;
}

/** Transitions to the LIVE party set in the profile by recreating the party */
function TransitionToLIVEParty()
{
	// Send prompts and kick non-local players
	PromptAndKickNonLocalPlayers();

	// If the previous match mode was system link we have to destroy the party
	if ( PreviousMatchMode == eGVMT_SystemLink )
	{
		DestroySystemLinkPartyForLIVEParty();
	}
	else
	{
		CreateLIVEParty( GetBestPlayerIndex() );
	}
}

/** Destroys a system link party to build a LIVE party */
function DestroySystemLinkPartyForLIVEParty()
{
	local GearPartyGame_Base PartyGI;

	PartyGI = GetPartyGameInfo();
	PartyGI.GameInterface.AddDestroyOnlineGameCompleteDelegate(OnDestroySystemLinkForLIVEPartyComplete);
	if (PartyGI.GameInterface.DestroyOnlineGame('Party'))
	{
		PartyGI.OpenUpdatingPartyScene();
	}
	else
	{
		PartyGI.GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroySystemLinkForLIVEPartyComplete);
	}
}

/**
 * Delegate fired when a destroying an online game has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnDestroySystemLinkForLIVEPartyComplete(name SessionName,bool bWasSuccessful)
{
	local GearPartyGame_Base PartyGI;

	PartyGI = GetPartyGameInfo();
	PartyGI.GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroySystemLinkForLIVEPartyComplete);
	PartyGI.PartySettings = None;
	PartyGI.CloseUpdatingPartyScene();
	CreateLIVEParty(GetBestPlayerIndex());
}

/** Creates the LIVE party and update the UI */
function CreateLIVEParty( int PlayerIndex )
{
	local GearPartyGame_Base PartyGI;
	local GearProfileSettings Profile;

	PartyGI = GetPartyGameInfo();

	// Create the LIVE party
	PartyGI.RecreateParty( class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex), false );

	// Get the new match mode from the profile since a failure to create the LIVE party result in a local party
	Profile = GetPlayerProfile(PlayerIndex);
	if ( Profile != None )
	{
		Profile.GetProfileSettingValueId( Profile.const.VERSUS_MATCH_MODE, PreviousMatchMode );
	}

	// Update the lobby
	SetMenuItemsInPartyList();
	UpdateRemoteSettings();
}

/** Check for networked players and warn the host that they will kick them */
function AttemptLocalTransition()
{
	local GearPartyGame_Base PartyGI;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local int PlayerIndex;
	local GameViewportClient VPClient;

	PartyGI = GetPartyGameInfo();
	PlayerIndex = GetBestPlayerIndex();
	VPClient = PartyGI.GetPlayerOwnerViewportClient(PlayerIndex);

	// Return if we can't get a GameViewportClient or UI Controller
	if ( VPClient == None || VPClient.UIController == None )
	{
		return;
	}

	// Prompt a warning if there are non-local players as they will be kicked
	if ( PartyHasNonLocalPlayers() )
	{
		ButtonAliases.AddItem('GenericCancel');
		ButtonAliases.AddItem('GenericAccept');
		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage(
			'ConfirmNetworkedPlayersKicked',
			"<Strings:GearGameUI.MessageBoxStrings.NetworkPlayersKicked_Title>",
			"<Strings:GearGameUI.MessageBoxStrings.NetworkPlayersKicked_Desc>",
			"",
			ButtonAliases,
			OnKickNetworkedPlayersForLocal_Confirm,
			GetPlayerOwner(GetBestPlayerIndex()));
	}
	// Else go ahead and make the transition
	else
	{
		TransitionToLocal();
	}

}

/** Callback from asking the host if they really want to kick non-local players */
function bool OnKickNetworkedPlayersForLocal_Confirm( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'GenericAccept' )
	{
		TransitionToLocal();
	}
	else
	{
		CancelLocalSelection();
	}

	return true;
}

/** Make the setting go back to what it was before the host selected local */
function CancelLocalSelection()
{
	local GearProfileSettings Profile;

	Profile = GetPlayerProfile(GetBestPlayerIndex());
	if ( Profile != None )
	{
		// Reset the profile to the previous match mode
		Profile.SetProfileSettingValueId( Profile.const.VERSUS_MATCH_MODE, PreviousMatchMode );
		MatchModeList.RefreshSubscriberValue();
	}
}

/** Transitions to a local party */
function TransitionToLocal()
{
	local GearPartyGame_Base PartyGI;

	// Send prompts and kick non-local players
	PromptAndKickNonLocalPlayers();

	// Destroy the party
	PartyGI = GetPartyGameInfo();
	PartyGI.DestroyParty();

	// Keep track of the last matchmode
	PreviousMatchMode = eGVMT_Local;

	// Update the lobby
	SetMenuItemsInPartyList();
	UpdateRemoteSettings();
}

/** Whether there are any non-local players in the party */
function bool PartyHasNonLocalPlayers()
{
	local int PRIIdx;
	local GearPartyGRI PartyGRI;
	local PlayerReplicationInfo CurrPRI;
	local GearPC CurrController;

	PartyGRI = GearPartyGRI(GetGRI());
	if ( PartyGRI != None )
	{
		for ( PRIIdx = 0; PRIIdx < PartyGRI.PRIArray.length; PRIIdx++ )
		{
			CurrPRI = PartyGRI.PRIArray[PRIIdx];
			if ( CurrPRI != None && !CurrPRI.bIsInactive )
			{
				CurrController = GearPC(CurrPRI.Owner);
				if ( CurrController != None && !CurrController.IsLocalPlayerController() )
				{
					return true;
				}
			}
		}
	}

	return false;
}

/** Checks to see if there are any non-local players and sends a warning, else makes the transition */
function AttemptSystemLinkTransition()
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local int PlayerCount;

	// Check for signed in players
	PlayerCount = class'UIInteraction'.static.GetPlayerCount();
	if ( class'UIInteraction'.static.GetLoggedInPlayerCount(false) != PlayerCount && IsConsole() )
	{
		DisplayErrorMessage("AControllerNotLoggedIn_Message", "NotLoggedIn_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
		CancelSystemLinkSelection( false );
		return;
	}

	// Check for a network connection first
	if ( !class'UIInteraction'.static.HasLinkConnection() )
	{
		DisplayErrorMessage("NoLinkConnection_Message", "NoLinkConnection_Title", "GenericContinue", GetPlayerOwner(GetBestPlayerIndex()));
		CancelSystemLinkSelection( false );
		return;
	}

	// Prompt a warning if there are non-local players as they will be kicked
	if ( PartyHasNonLocalPlayers() )
	{
		ButtonAliases.AddItem('GenericCancel');
		ButtonAliases.AddItem('GenericAccept');
		GameSceneClient = GetSceneClient();
		GameSceneClient.ShowUIMessage(
			'ConfirmNetworkedPlayersKicked',
			"<Strings:GearGameUI.MessageBoxStrings.NetworkPlayersKicked_Title>",
			"<Strings:GearGameUI.MessageBoxStrings.NetworkPlayersKicked_Desc>",
			"",
			ButtonAliases,
			OnKickNetworkedPlayersForSystemLink_Confirm,
			GetPlayerOwner(GetBestPlayerIndex()));
	}
	// Else go ahead and make the transition
	else
	{
		TransitionToSystemLink();
	}
}

/**
 * Make the setting go back to what it was before the host selected systemlink
 *
 * @param bRecreateParty - whether we need to recreate the party or not because of it being destroyed
 */
function CancelSystemLinkSelection( bool bRecreateParty )
{
	local GearProfileSettings Profile;
	local GearPartyGame_Base PartyGame;

	Profile = GetPlayerProfile(GetBestPlayerIndex());
	if ( Profile != None )
	{
		// Reset the profile to the previous match mode
		Profile.SetProfileSettingValueId( Profile.const.VERSUS_MATCH_MODE, PreviousMatchMode );
		MatchModeList.RefreshSubscriberValue();

		// If the previous match mode was a Public or Private LIVE match
		// and we need to recreate the LIVE party because of it being destroyed before going to the LAN scene
		if ( PreviousMatchMode != eGVMT_Local && bRecreateParty )
		{
			PartyGame = GetPartyGameInfo();
			PartyGame.RecreateParty( class'UIInteraction'.static.GetPlayerControllerId(GetBestPlayerIndex()), PreviousMatchMode == eGVMT_SystemLink );

			// Get the new match mode from the profile since a failure to create the LIVE party result in a local party
			Profile.GetProfileSettingValueId( Profile.const.VERSUS_MATCH_MODE, PreviousMatchMode );
		}
	}
}

/** Callback from asking the host if they really want to kick non-local players */
function bool OnKickNetworkedPlayersForSystemLink_Confirm( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	if ( SelectedInputAlias == 'GenericAccept' )
	{
		TransitionToSystemLink();
	}
	else
	{
		CancelSystemLinkSelection( false );
	}

	return true;
}

/** Finds all non-local players, prompts them that they are being booted, and then boots them */
function PromptAndKickNonLocalPlayers( optional string TitleMarkup="<Strings:GearGameUI.MessageBoxStrings.KickedForPartyKill_Title>", optional string MessageMarkup="<Strings:GearGameUI.MessageBoxStrings.KickedForPartyKill_Desc>" )
{
	local int PRIIdx;
	local GearPartyGRI PartyGRI;
	local PlayerReplicationInfo CurrPRI;
	local GearPC CurrController;

	PartyGRI = GearPartyGRI(GetGRI());

	// Kick non-local players
	if ( PartyGRI != None )
	{
		for ( PRIIdx = 0; PRIIdx < PartyGRI.PRIArray.length; PRIIdx++ )
		{
			CurrPRI = PartyGRI.PRIArray[PRIIdx];
			if ( CurrPRI != None && !CurrPRI.bIsInactive )
			{
				CurrController = GearPC(CurrPRI.Owner);
				if ( CurrController != None && !CurrController.IsLocalPlayerController() )
				{
					// Send a warning to the client
					CurrController.ClientSetProgressMessage( PMT_ConnectionFailure, MessageMarkup, TitleMarkup, true );

					// Kick the player
					CurrController.ClientReturnToMainMenu();
				}
			}
		}
	}
}

/** Kicks non-local players, tells the gameinfo to shut the party session down, switches the invite policy, opens the LAN scene */
function TransitionToSystemLink()
{
	PromptAndKickNonLocalPlayers();

	// Kill the party session
	DestroyPartyForSystemLink();
}

/** Destroys a party to transition to system link */
function DestroyPartyForSystemLink()
{
	local GearPartyGame_Base PartyGI;

	PartyGI = GetPartyGameInfo();
	PartyGI.GameInterface.AddDestroyOnlineGameCompleteDelegate(OnDestroyPartyForSystemLinkComplete);
	if (PartyGI.GameInterface.DestroyOnlineGame('Party'))
	{
		PartyGI.OpenUpdatingPartyScene();
	}
}

/**
 * Delegate fired when a destroying an online game has completed
 *
 * @param SessionName the name of the session this callback is for
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnDestroyPartyForSystemLinkComplete(name SessionName,bool bWasSuccessful)
{
	local GearPartyGame_Base PartyGI;

	PartyGI = GetPartyGameInfo();
	PartyGI.GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroyPartyForSystemLinkComplete);
	PartyGI.PartySettings = None;
	PartyGI.CloseUpdatingPartyScene();
	OpenLANScene();
}

/** Opens the LAN scene */
function OpenLANScene()
{
	local UIScene FoundLANScene;
	local GameUISceneClient GameSceneClient;
	local UIScene SceneInstance;

	// Open the LAN screen
	GameSceneClient = GetSceneClient();
	if ( GameSceneClient != None )
	{
		FoundLANScene = UIScene(FindObject("UI_Scenes_Lobby.LANGame", class'UIScene'));
		if ( FoundLANScene != None )
		{
			SceneInstance = OpenScene(FoundLANScene, GetPlayerOwner(GetBestPlayerIndex()));
			if ( SceneInstance != None )
			{
				bWantsToStartLANParty = false;
				bCanceledLANScene = false;
				SceneInstance.OnSceneDeactivated = OnLANSceneClosed;
				GearUISceneFE_LAN(SceneInstance).OnStartLANParty = OnLANSceneCreateParty;
				GearUISceneFE_LAN(SceneInstance).OnCancelFromLANScene = OnCancelLANScene;
			}
		}
	}
}

/** Called if the player chose to start their own party from the LAN scene */
function OnLANSceneCreateParty()
{
	bWantsToStartLANParty = true;
}

/** Called if the player canceled from the LAN scene */
function OnCancelLANScene()
{
	bCanceledLANScene = true;
}

/** Called when the LAN screen is closed */
function OnLANSceneClosed( UIScene ClosedScene )
{
	local GearPartyGame_Base PartyGame;
	local GearProfileSettings Profile;

	// If the player chose to create their own party then do so
	if ( bWantsToStartLANParty )
	{
		PartyGame = GetPartyGameInfo();
		if ( PartyGame != None )
		{
			PartyGame.RecreateParty( GetBestControllerId(), true );

			// Get the new match mode from the profile since a failure to create the LAN party results in a local party
			Profile = GetPlayerProfile(GetBestPlayerIndex());
			if ( Profile != None )
			{
				Profile.GetProfileSettingValueId( Profile.const.VERSUS_MATCH_MODE, PreviousMatchMode );
			}
		}
	}
	else if ( bCanceledLANScene )
	{
		CancelSystemLinkSelection( true );
		return;
	}

	// Keep track of the last matchmode
	PreviousMatchMode = eGVMT_SystemLink;

	SetMenuItemsInPartyList();
	UpdateRemoteSettings();
}

/** Publishes the current party mode to the party session object */
function ApplyPartyModeUpdate( int PlayerIndex )
{
	local GearPartyGame_Base PartyGI;
	local GearProfileSettings Profile;
	local int SelectedContext;

	Profile = GetPlayerProfile(PlayerIndex);
	if ( Profile != None && Profile.GetProfileSettingValueId(Profile.const.VERSUS_PARTY_TYPE, SelectedContext) )
	{
		PartyGI = GetPartyGameInfo();
		if ( PartyGI != None )
		{
			// Set the party setting for the new invite policy
			PartyGI.ChangePartySettings( PartyGI.PartySettings, SelectedContext );

			// Tell live to update the advertised settings
			PartyGI.UpdatePartySettings();
		}
	}
}

/* === Delegate Handlers === */
/**
 * Handler for the OnSubmitSelection delegate of list children in this scene.
 *
 * @param	Sender			the list that submitted a valuewhose value changed
 * @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 */
function ListValueSubmitted( UIList Sender, name OptionName, int PlayerIndex )
{
	local array<UIDataStore> Unused;
	local UIDataStorePublisher Publisher;
	local GearProfileSettings Profile;
	local int SelectedContext;

	// always publish the value immediately
	Publisher = UIDataStorePublisher(Sender);
	if ( Publisher != None )
	{
		Publisher.SaveSubscriberValue(Unused);
	}

	switch ( OptionName )
	{
	case 'MatchModeOption':
		Profile = GetPlayerProfile(PlayerIndex);
		if ( Profile != None && Profile.GetProfileSettingValueId(Profile.const.VERSUS_MATCH_MODE, SelectedContext) )
		{
			// Make sure to not do anything if the same mode was selected
			if ( SelectedContext != PreviousMatchMode )
			{
				ApplyMatchModeUpdate( PlayerIndex );
			}
		}
		break;

	case 'PartyTypeOption':
		ApplyPartyModeUpdate( PlayerIndex );
		UpdateRemoteSettings();
		break;

	case 'GameTypeOption':
		RefreshGameOptions( PlayerIndex );
		break;

	case 'PlaylistOption':
		SetPlaylistDataInProfile( Sender, PlayerIndex );
		UpdatePartySizeFromPlaylistChange( PlayerIndex );
		break;

	default:
		MarkGameSettingsDirty(true);
		break;
	}

	UpdateSceneState();
}

/**
 * Called when a playlist gets changed. If the GRI hasn't been replicated yet, sets a timer in
 * the owning player's PlayerController to check again next tick.
 */
function OnPlaylistChanged()
{
	local GearPC OwnerPC;
	local int ObjectIndex;
	local GearUICollapsingSelectionList PlaylistList;
	local GearGRI MyGRI;

	OwnerPC = GetGearPlayerOwner(0);
	if ( OwnerPC != None )
	{
		MyGRI = GetGRI();
		if ( MyGRI != None )
		{
			if ( lstPartyOptions != None )
			{
				ObjectIndex = lstPartyOptions.GetObjectInfoIndexFromName( (MyGRI.Role == ROLE_Authority) ? 'PlaylistOption' : 'RemotePlaylistOption' );
				if ( ObjectIndex != INDEX_NONE )
				{
					PlaylistList = GearUICollapsingSelectionList(lstPartyOptions.GeneratedObjects[ObjectIndex].OptionObj);
					SetPlaylistDataInMenuOption( PlaylistList );
				}
			}
			return;
		}
		else
		{
			OwnerPC.SetTimer( 0.1f, false, nameof(self.OnPlaylistChanged), self );
		}
	}
}

/** If the team size has changed we must update UI team size */
function UpdatePartySizeFromPlaylistChange( int PlayerIndex )
{
	local LocalPlayer LP;
	local GearPC PC;
	local int PlaylistValue, TeamSize, TeamCount;
	local OnlinePlaylistManager PlaylistMan;
	local OnlineSubsystem OnlineSub;
	local GearPartyGRI PartyGRI;

	LP = GetPlayerOwner( PlayerIndex );
	PC = GearPC(LP.Actor);

	if ( PC.ProfileSettings != None && IsOfficialMatch() )
	{
		PlaylistValue = PC.ProfileSettings.GetPlaylistID();
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if ( OnlineSub != None )
		{
			PartyGRI = GearPartyGRI(GetGRI());
			PlaylistMan = OnlinePlaylistManager(OnlineSub.GetNamedInterface('PlaylistManager'));
			if ( PartyGRI != None && PlaylistMan != None )
			{
				PlaylistMan.GetTeamInfoFromPlaylist( PlaylistValue, TeamSize, TeamCount );
				PartyGRI.TeamSize = TeamSize;
			}
		}
	}
}

/** Sets the profile value for the playlist selected and informs the clients to update their UI */
function SetPlaylistDataInProfile( UIList Sender, int PlayerIndex )
{
	local int CurrListIndex, ProviderIndex;
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> PlaylistProviders;
	local OnlinePlaylistProvider PlaylistProvider;
	local LocalPlayer LP;
	local GearPC MyGearPC;
	local GearPartyGRI PartyGRI;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	CurrListIndex = Sender.GetCurrentItem();
	LP = GetPlayerOwner(PlayerIndex);
	MyGearPC = GearPC(LP.Actor);

	if ( GameResourceDS != None &&
		 CurrListIndex != INDEX_NONE &&
		 MyGearPC.ProfileSettings != None )
	{
		// Grab the playlist providers
		if ( GameResourceDS.GetResourceProviders('Playlists', PlaylistProviders) )
		{
			ProviderIndex = Sender.Items[CurrListIndex];
			PlaylistProvider = OnlinePlaylistProvider(PlaylistProviders[ProviderIndex]);
			MyGearPC.ProfileSettings.SetProfileSettingValueInt(MyGearPC.ProfileSettings.PlaylistId, PlaylistProvider.PlaylistId);

			PartyGRI = GearPartyGRI(GetGRI());
			if ( PartyGRI != None )
			{
				// Set the playlist value on the GRI for clients
				PartyGRI.SetPlaylist(PlaylistProvider.PlaylistId);

				RefreshPlaylistDescription( ProviderIndex );
			}
		}
	}
}

/**
 * Sets the current list index of the playlist options to the value stored in the profile for the host
 * or has the client use the value in the GRI
 */
function SetPlaylistDataInMenuOption( GearUICollapsingSelectionList Sender )
{
	local int PlaylistId, ProviderIndex, ListIndex;
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue Value;
	local array<UIResourceDataProvider> PlaylistProviders;
	local LocalPlayer LP;
	local GearPC MyGearPC;
	local GearPartyGRI MyGRI;
	local OnlinePlaylistProvider PlaylistProv;

	MyGRI = GearPartyGRI(GetGRI());

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));

	// Grab the playlist providers
	if ( MyGRI != None && GameResourceDS != None && GameResourceDS.GetResourceProviders('Playlists', PlaylistProviders) )
	{
		LP = GetPlayerOwner(0);
		MyGearPC = GearPC(LP.Actor);

		// This is the host so get the value from the profile
		if ( MyGRI.Role == ROLE_Authority )
		{
			PlaylistId = MyGearPC.ProfileSettings.GetPlaylistID();
		}
		// else get it from the GRI
		else
		{
			PlaylistId = MyGRI.PlaylistId;
		}

		// Find the selected provider
		Value.PropertyTag = 'PlaylistId';
		Value.PropertyType = DATATYPE_Property;
		Value.StringValue = string(PlaylistId);
		ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Playlists', 'PlaylistId', Value);

		// If there is not a valid playlist index and this is the host we must grab the first playlist and set it in the profile
		if ( ProviderIndex == INDEX_NONE && MyGRI.Role == ROLE_Authority )
		{
			ProviderIndex = Sender.Items[0];
			PlaylistProv = OnlinePlaylistProvider(PlaylistProviders[ProviderIndex]);
			MyGearPC.ProfileSettings.SetProfileSettingValueInt(MyGearPC.ProfileSettings.PlaylistId, PlaylistProv.PlaylistId);
			PlaylistId = PlaylistProv.PlaylistId;
		}

		ListIndex = Sender.Items.Find(ProviderIndex);
		ListIndex = (ListIndex == INDEX_NONE) ? 0 : ListIndex;
		Sender.SetIndexValue( ListIndex, 0 );

		// If this is the host, we must set the GRI value
		if ( MyGRI.Role == ROLE_Authority )
		{
			MyGRI.SetPlaylist(PlaylistId);
		}

		RefreshPlaylistDescription( ProviderIndex );
	}
}

/** Sets the dirty flag for publishing the settings to the profile */
function MarkGameSettingsDirty(bool bValue)
{
	bGameSettingHasChanged = bValue;
}

/** Called to update the ui with the new game options */
function RefreshGameOptions(int PlayerIndex)
{
	local GearProfileSettings Profile;
	local GearUIDataStore_GameSettings SettingsDS;
	local int SelectedContext;

	Profile = GetPlayerProfile(PlayerIndex);
	if ( Profile != None && Profile.GetProfileSettingValueId(Profile.const.VERSUS_GAMETYPE, SelectedContext) )
	{
		if (bGameSettingHasChanged)
		{
			SaveGameSettingsToProfile(PlayerIndex, , false);
			MarkGameSettingsDirty(false);
		}

		SettingsDS = GetGameSettingsDataStore();
		if ( SettingsDS != None )
		{
			// now change the GameSettings data store's current game settings object - this is what actually triggers the repopulation
			// of the game option widgets
			SettingsDS.SetCurrentByContextId(SelectedContext);
		}
	}
}

/**
 * Displays a modal dialog while the async task for updating the party settings on live is active.
 */
function ShowSettingsUpdateUI()
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.Length = 0; // gotta trick the compiler

	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage(
		'UpdatingPartyMessage',
		"<Strings:GearGameUI.MessageBoxStrings.UpdatingTitle>",
		"<Strings:GearGameUI.MessageBoxStrings.UpdatingPartySettingsMessage>", "",
		ButtonAliases, None, GetPlayerOwner()
		);
}

/**
 * Called when the update of the session state has completed
 *
 * @param SessionName the name of the session this event is for
 * @param bWasSuccessful whether the update completed ok or not
 */
function OnPartySettingsUpdateComplete( name SessionName, bool bWasSuccessful )
{
	local GameUISceneClient GameSceneClient;
	local int SceneIdx;
	local OnlineGameInterface GameInterface;

	if ( SessionName == 'Party' )
	{
		GameInterface = GetOnlineGameInterface();
		GameInterface.ClearUpdateOnlineGameCompleteDelegate(OnPartySettingsUpdateComplete);

		GameSceneClient = GetSceneClient();
		SceneIdx = GameSceneClient.FindSceneIndexByTag('UpdatingPartyMessage', GetPlayerOwner());
		GameSceneClient.CloseSceneAtIndex(SceneIdx, false);

		if ( !bWasSuccessful )
		{
			// show that the update was not successful
			//DisplayErrorMessage(
		}
	}
}

/**
 * Handler for the OnValueChanged delegate of various children in this scene.
 *
 * @param	Sender			the UIObject whose value changed
 * @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 */
function OptionValueChanged( UIScreenObject Sender, name OptionName, int PlayerIndex )
{
	UpdateSceneState();
}

/** Called by GearMenuPC to notify the client when a game setting value has changed */
function OnGameSettingValueChange( int SettingId, int ValueId )
{
	local GearUIDataStore_GameSettings GameSettingsDS;
	local OnlineGameSettings CurrentSettings;
	local int SettingsIdx;

	if ( SettingId == class'GearProfileSettings'.const.VERSUS_MATCH_MODE ||
		 SettingId == class'GearProfileSettings'.const.VERSUS_PARTY_TYPE )
	{
		CurrentSettings = GetCurrentGameSettings();
		GameSettingsDS = GetGameSettingsDataStore();
		if ( CurrentSettings != None && GameSettingsDS != None )
		{
			// Set the value on all the other settings objects so we don't see the blip that happens on a gametype change
			for ( SettingsIdx = 0; SettingsIdx < GameSettingsDS.GameSettingsCfgList.length; SettingsIdx++ )
			{
				if ( CurrentSettings != GameSettingsDS.GameSettingsCfgList[SettingsIdx].GameSettings )
				{
					GameSettingsDS.GameSettingsCfgList[SettingsIdx].GameSettings.SetStringSettingValue(SettingId, ValueId, false);
				}
			}
		}

		if ( SettingId == class'GearProfileSettings'.const.VERSUS_MATCH_MODE )
		{
			SetMenuItemsInPartyList();
		}
	}
}

/** Called by GearMenuPC to notify the client when a game property value has changed */
function OnGamePropertyValueChange( int SettingId, int ValueId )
{

}

/**
 * Called when the widget is no longer being pressed.  Not implemented by all widget types.
 *
 * The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
 * widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
 * when the key was released, which might not necessarily be the widget that received the key press.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnPlayerButtonClicked( UIScreenObject EventObject, int PlayerIndex )
{
	local GearPRI SelectedPRI;
	local int DataIndex;
	local array<EGearPlayerOptions> Options;

	DataIndex = GetPlayerDataIndexFromWidget( EventObject );
	if ( DataIndex != -1 )
	{
		SelectedPRI = PlayerData[DataIndex].PlayerPRI;
		// If there is a player in this slot, go to the player options
		if ( SelectedPRI != None )
		{
			Options.AddItem(eGPOPTION_Gamercard);
			Options.AddItem(eGPOPTION_Feedback);
			if ( GetGRI().Role == ROLE_Authority )
			{
				Options.AddItem(eGPOPTION_Kick);
			}
			Options.AddItem(eGPOPTION_Cancel);

			OpenPlayerOptionsScene( PlayerIndex, SelectedPRI, Options, IsOfficialMatch() || IsCustomMatch() );
		}
		// If there is not a player in this slot, go to the what's up screen
		else
		{
			OpenScene(WhatsUpSceneReference, GetPlayerOwner(PlayerIndex));
		}
	}

	return true;
}

/** Returns the index in the PlayerData array of which playerdata contains this widget */
function int GetPlayerDataIndexFromWidget( UIScreenObject Widget )
{
	local int Idx;

	for ( Idx = 0; Idx < PlayerData.length; Idx++ )
	{
		if ( PlayerData[Idx].ParentButton == Widget )
		{
			return Idx;
		}
	}

	return -1;
}

/** @return number of local players */
final function int GetNumLocalPlayers()
{
	local GearPC PC;
	local int Count;

	foreach GetPlayerOwner().Actor.LocalPlayerControllers(class'GearPC', PC)
	{
		Count++;
	}

	return Count;
}

/**
 * Called when the start match button is clicked.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool StartMatchButtonClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local bool bResult;
	local GearPartyGRI PartyGRI;
	local int NumPRIs, PartyTeamSize;
	local array<name> ButtonAliases;
	local GameUISceneClient GameSceneClient;
	local string AlteredMsgString;
	local GearPartyGame_Base PartyGame;

	PartyGRI = GearPartyGRI(GetGRI());
	NumPRIs = GetNumValidPRIs();
	PartyTeamSize = GetPartyTeamSize();

`Log("");
`Log("StartMatchButtonClicked");
`Log("");
`Log("IsPartyLeader(PlayerIndex) "$IsPartyLeader(PlayerIndex));
`Log("IsMatchmaking(PlayerIndex) "$IsMatchmaking(PlayerIndex));
`Log("PartyGRI.ConnectingPlayerCount "$PartyGRI.ConnectingPlayerCount);
`Log("NumPRIs "$NumPRIs);
`Log("IsAllowedToCreateMatch(PlayerIndex) "$IsAllowedToCreateMatch(PlayerIndex));
`Log("LobbyHasEnoughPlayers() "$LobbyHasEnoughPlayers());
`Log("IsAllowedToSearchForMatches(PlayerIndex) "$IsAllowedToSearchForMatches(PlayerIndex));
`Log("IsOfficialMatch() "$IsOfficialMatch());
`Log("");

	if ( IsPartyLeader(PlayerIndex) )
	{
		if (IsMatchmaking(PlayerIndex) == false)
		{
			PartyGame = GetPartyGameInfo();
`log("PartyGRI.TeamCount = "$PartyGRI.TeamCount);
`log("IsOfficialMatch() = "$IsOfficialMatch());
`log("NumPRIs == PartyTeamSize = "$NumPRIs == PartyTeamSize);
`log("PartyTeamSize > 2 = "$PartyTeamSize > 2);
`log("!PartyGame.CanHostVersusMatch() = "$!PartyGame.CanHostVersusMatch());
			// Display a message if a full horde party is hosted by a splitscreen host and they try to start a public match
			if (PartyGRI.TeamCount == 1 &&
				IsOfficialMatch() &&
				NumPRIs == PartyTeamSize &&
				PartyTeamSize > 2 &&
				GetNumLocalPlayers() > 1)
			{
				ButtonAliases.AddItem('GenericContinue');
				GameSceneClient = GetSceneClient();
				GameSceneClient.ShowUIMessage('NotEnoughPlayers',
					"<Strings:GearGameUI.MessageBoxErrorStrings.NoSplitscreenFullPublicHorde_Title>",
					"<Strings:GearGameUI.MessageBoxErrorStrings.NoSplitscreenFullPublicHorde_Message>",
					"",
					ButtonAliases,
					OnMatchmakingError_Confirmed);
				bResult = false;
			}
			// Don't allow clicking create/start/find while a player is joining
			else if (PartyGRI.ConnectingPlayerCount <= 0 &&
				NumPRIs == PartyGRI.PRIArray.Length)
			{
				if ( IsAllowedToCreateMatch(PlayerIndex) )
				{
					// Send error message if there are too many players
					if ( NumPRIs > PartyTeamSize )
					{
						AlteredMsgString = Localize("MessageBoxErrorStrings", "GameErrorTooManyPlayers_Msg", "GearGameUI");
						AlteredMsgString = Repl( AlteredMsgString, "\`NUMPLAYERS\`", string(NumPRIs-PartyTeamSize));
						ButtonAliases.AddItem('GenericContinue');
						GameSceneClient = GetSceneClient();
						GameSceneClient.ShowUIMessage('NumPlayersError',
							"<Strings:GearGameUI.MessageBoxErrorStrings.GameErrorTooManyPlayers_Title>",
							AlteredMsgString,
							"",
							ButtonAliases,
							OnMatchmakingError_Confirmed);
						bResult = false;
					}
					else if (IsOfficialMatch() && PartyMemberIsAGuest())
					{
						if (Len(MatchErrorNoGuestInPublic) > 0)
						{
							ButtonAliases.AddItem('GenericContinue');
							GameSceneClient = GetSceneClient();
							GameSceneClient.ShowUIMessage('MatchmakeError',
								"<Strings:GearGameUI.MessageBoxErrorStrings.StartMPGameFailed_Title>",
								"<Strings:GearGameUI.MessageBoxErrorStrings.NoGuestInPublic_Msg>",
								"",
								ButtonAliases,
								OnMatchmakingError_Confirmed);
						}
						return false;
					}
					else if (LobbyHasEnoughPlayers())
					{
						`log("Creating match for player" @ PlayerIndex);
						if ( CreateMatch(PlayerIndex) )
						{
							if (MatchmakeButton.IsFocused())
							{
								MatchmakeButton.KillFocus(none, PlayerIndex);
							}
							MatchmakeButton.DisableWidget(PlayerIndex);
							bResult = true;
						}
						else
						{
							`Log("Failed to create match");
						}
					}
					else
					{
						ButtonAliases.AddItem('GenericContinue');
						GameSceneClient = GetSceneClient();
						GameSceneClient.ShowUIMessage('NotEnoughPlayers',
							"<Strings:GearGameUI.MessageBoxErrorStrings.NotEnoughPlayers_Title>",
							"<Strings:GearGameUI.MessageBoxErrorStrings.NotEnoughPlayers_Message>",
							"",
							ButtonAliases,
							OnNotEnoughPlayers_Confirmed);
						return false;
					}
				}
				else if ( IsAllowedToSearchForMatches(PlayerIndex) )
				{
					if ( PartyGRI != None && PartyGRI.bPartyLobbyErrorExists || PartyMemberLacksRequiredDLC(true) )
					{
						if ( NumPRIs > PartyTeamSize )
						{
							AlteredMsgString = Localize("MessageBoxErrorStrings", "MatchErrorTooManyPlayers_Msg", "GearGameUI");
							AlteredMsgString = Repl( AlteredMsgString, "\`NUMPLAYERS\`", string(NumPRIs-PartyTeamSize));
							ButtonAliases.AddItem('GenericContinue');
							GameSceneClient = GetSceneClient();
							GameSceneClient.ShowUIMessage('MatchmakeError',
								"<Strings:GearGameUI.MessageBoxErrorStrings.MatchErrorTooManyPlayers_Title>",
								AlteredMsgString,
								"",
								ButtonAliases,
								OnMatchmakingError_Confirmed);
						}
						else if (PartyMemberIsAGuest())
						{
							if (Len(MatchErrorNoGuestInPublic) > 0)
							{
								ButtonAliases.AddItem('GenericContinue');
								GameSceneClient = GetSceneClient();
								GameSceneClient.ShowUIMessage('MatchmakeError',
									"<Strings:GearGameUI.MessageBoxErrorStrings.StartMPGameFailed_Title>",
									"<Strings:GearGameUI.MessageBoxErrorStrings.NoGuestInPublic_Msg>",
									"",
									ButtonAliases,
									OnMatchmakingError_Confirmed);
							}
							return false;
						}
						else
						{
							ButtonAliases.AddItem('GenericContinue');
							GameSceneClient = GetSceneClient();
							GameSceneClient.ShowUIMessage('MatchmakeError',
								"<Strings:GearGameUI.MessageBoxErrorStrings.MatchErrorMissingDLC_Title>",
								"<Strings:GearGameUI.MessageBoxErrorStrings.MatchErrorMissingDLC_Msg>",
								"",
								ButtonAliases,
								OnMatchmakingError_Confirmed);
						}
					}
					else
					{
						if ( PartyGRI.MatchmakingState == MMS_NotMatchmaking )
						{
							`log("Starting match search for player" @ PlayerIndex);
							if ( FindMatch(PlayerIndex) )
							{
								bResult = true;
							}
							else
							{
								`Log("Find match failed");
							}
						}
					}
				}
			}
			else
			{
				`Log("Failed the block while connecting check ConnectingPlayerCount("$PartyGRI.ConnectingPlayerCount$") NumPRIs("$NumPRIs$") PRIArray.Length("$PartyGRI.PRIArray.Length$")");
				// Play an error sound
				PlayUISound( 'Error' );
			}

			// We are matchmaking, so disable the click handler
			// since we want B-presses to cancel and those are handled
			// by the scene.
			if (bResult)
			{
				//MatchmakeButton.OnClicked = None;
				if (!IsOfficialMatch())
				{
					bGameStarting = TRUE;
				}
			}
			else
			{
				`Log("Matchmaking failed to start");
			}

		}
		else
		{
			`log("Canceling match search for player" @ PlayerIndex);
			if ( CancelMatchmaking() )
			{
				bResult = true;

				// Play the sound as if we had clicked on the button
				PlayUISound( ClickedCue );

				// Reenable the click handler because we are not matchmaking, which means
				// that we are no longer overriding the matchmaking button's click with B button.
				//MatchmakeButton.OnClicked = StartMatchButtonClicked;

			}
			else
			{
				`Log("Cancel matchmaking failed");
				// Play an error sound
				PlayUISound( 'Error' );
			}
		}
	}
	else
	{
		`Log("Failed the IsPartyLeader() check");
	}

	UpdateSceneState(PlayerIndex);
	return bResult;
}

/** Called when the error message closes for not enough players */
function bool OnNotEnoughPlayers_Confirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	MatchmakeButton.SetFocus(None);
	return true;
}

/** Whether there are enough players to begin a match */
final function bool LobbyHasEnoughPlayers()
{
	local int NumHumans;
	local int NumBots;

	if (IsHordeGametype())
	{
		return true;
	}

	NumHumans = GetNumValidPRIs();
	NumBots = GetNumBots();
	if (NumHumans + NumBots > 1)
	{
		return true;
	}

	return false;
}

/** Called when the error message closes for matchmaking */
function bool OnMatchmakingError_Confirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	return true;
}

/**
 * Called when the local player is about to travel to a new URL.  This callback should be used to perform any preparation
 * tasks, such as updating status text and such.  All cleanup should be done from NotifyGameSessionEnded, as that function
 * will be called in some cases where NotifyClientTravel is not.
 *
 * @param	TravelURL		a string containing the mapname (or IP address) to travel to, along with option key/value pairs
 * @param	TravelType		indicates whether the player will clear previously added URL options or not.
 * @param	bIsSeamless		indicates whether seamless travelling will be used.
 */
function NotifyPreClientTravel( string TravelURL, ETravelType TravelType, bool bIsSeamless )
{
	Super.NotifyPreClientTravel(TravelURL, TravelType, bIsSeamless);

	//ShowLoadingMovie(true);
	//@todo ronp animation
	CloseScene(Self, false, true);
}

/**
 * Handler for the scene's OnSceneActivated delegate.  Called after scene focus has been initialized, or when the scene becomes active as
 * the result of another scene closing.
 *
 * @param	ActivatedScene			the scene that was activated
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
function SceneActivationComplete( UIScene ActivatedScene, bool bInitialActivation )
{
	local GearGRI GRI;
	local GearProfileSettings Profile;

	if ( !IsEditor() )
	{
		GRI = GetGRI();

		// Turn chat delegates on
		if ( bInitialActivation )
		{
			bPartySceneHasInitialized = true;

			if ( GRI != None && GRI.Role == ROLE_Authority && MatchmakeButton != None )
			{
				MatchmakeButton.SetFocus(None);
			}

			// Have all players send their DLC values
			UpdateLocalPlayersDLCValues();

			TrackChat( true );

			// Need to grab the initial value of match mode
			Profile = GetGearPlayerOwner(GetBestPlayerIndex()).ProfileSettings;
			if ( Profile != None )
			{
				Profile.GetProfileSettingValueId( Profile.VERSUS_MATCH_MODE, PreviousMatchMode );
			}
		}

		if ( ActivatedScene == Self )
		{
			UpdateSceneState();
		}
	}
}

/** Called just after this scene is removed from the active scenes array */
event SceneDeactivated()
{
	local GearUIDataStore_GameSettings SettingsDS;

	Super.SceneDeactivated();

	ClearDelegates();

	SettingsDS = GetGameSettingsDataStore();
	if ( SettingsDS != None )
	{
		SettingsDS.RemovePropertyNotificationChangeRequest(SettingsProviderUpdated);
	}

	// Turn chat delegates off
	TrackChat( false );
}

/**
 * Handler for the scene's OnProcessInputKey delegate - overrides the default handling of the CloseScene alias to issue
 * a disconnect command.
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called AFTER kismet is given a chance to process the input, but BEFORE any native code processes the input.
 *
 * @param	EventParms	information about the input event, including the name of the input alias associated with the
 *						current key name (Tab, Space, etc.), event type (Pressed, Released, etc.) and modifier keys (Ctrl, Alt)
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool ProcessInputKey( const out SubscribedInputEventParameters EventParms )
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local GearUICollapsingSelectionList ExpandedList;
	local bool bResult;
	local WorldInfo WI;

	if ( EventParms.PlayerIndex == GetPlayerOwnerIndex() )
	{
		if ( IsMatchmaking(EventParms.PlayerIndex,true) && EventParms.EventType == IE_Pressed && EventParms.InputKeyName == 'XboxTypeS_B')
		{
			if (GetWorldInfo().NetMode == NM_Client)
			{
				if (CancelMatchmaking())
				{
					// Play the sound as if we had clicked on the button
					PlayUISound(ClickedCue);
				}
				else
				{
					`Log("Cancel matchmaking failed");
					// Play an error sound
					PlayUISound('Error');
				}
			}
			else
			{
				StartMatchButtonClicked( None, EventParms.PlayerIndex );
			}
			bResult = true;
		}
		else if ( EventParms.InputAliasName == 'CloseScene'/* && !bCancelIsMainAction */)
		{
			if ( EventParms.EventType == IE_Released )
			{
				if ((IsEditingSelectionList(lstPartyOptions, ExpandedList) || IsEditingSelectionList(lstGameOptions,ExpandedList))
				&&	ExpandedList != None )
				{
					ExpandedList.Collapse();
				}
				else
				{
					// Don't let anyone close the scene if we are in the process of matchmaking
					if ( GearPartyGRI(GetGRI()).MatchmakingState == MMS_NotMatchmaking )
					{
						ButtonAliases.AddItem('CancelStay');
						ButtonAliases.AddItem('AcceptLeave');
						GameSceneClient = GetSceneClient();
						WI = GetWorldInfo();
						if ( WI != None && WI.NetMode != NM_Client )
						{
							GameSceneClient.ShowUIMessage('ConfirmQuit',
								"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_Title>",
								"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_LeaderMessage>",
								"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_Question>",
								ButtonAliases, OnReturnToMainMenuConfirmed, GetPlayerOwner());
						}
						else
						{
							GameSceneClient.ShowUIMessage('ConfirmQuit',
								"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_Title>",
								"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_MemberMessage>",
								"<Strings:GearGameUI.MessageBoxStrings.ReturnToMainMenu_Question>",
								ButtonAliases, OnReturnToMainMenuConfirmed, GetPlayerOwner());
						}
					}
				}
			}

			bResult = true;
		}
	}
	else
	{
		bResult = true;
	}

	return bResult;
}

/** Callback when the user confirms leaving */
function bool OnReturnToMainMenuConfirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local WorldInfo WI;
	local GearPC PC;

	if ( SelectedInputAlias == 'AcceptLeave' )
	{
		PC = GetGearPlayerOwner(0);
		WI = GetWorldInfo();
		if ( PC != None && WI != None )
		{
			if ( PC.Role == ROLE_Authority )
			{
				PromptAndKickNonLocalPlayers(
					"<Strings:GearGameUI.MessageBoxStrings.PartyDisbanded_Title>",
					"<Strings:GearGameUI.MessageBoxStrings.PartyDisbanded_Message>"
				);
			}

			SaveGameSettingsToProfile(PlayerIndex, , false);
			PC.ReturnToMainMenu();
			// Save the profiles
			foreach WI.LocalPlayerControllers(class'GearPC', PC)
			{
				PC.SaveProfile();
			}
		}
	}
	return true;
}

/**
 * Weapon Swap was clicked
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnWeaponSwapClicked( UIScreenObject EventObject, int PlayerIndex )
{
	if (WeaponSwapScene != None && IsAllowedToOpenWeaponSwap(PlayerIndex))
	{
		OpenScene(WeaponSwapScene, GetPlayerOwner(PlayerIndex));
	}
	return true;
}

/**
 * DLC was clicked
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnDLCClicked( UIScreenObject EventObject, int PlayerIndex )
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local int ControllerId;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if (PlayerIntEx != None)
		{
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
			PlayerIntEx.ShowContentMarketplaceUI(ControllerId);
		}
	}
	return true;
}

/**
 * Post game stats was clicked
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnPostGameStatsClicked( UIScreenObject EventObject, int PlayerIndex )
{
	local UIScene FoundStatScene;
	local GameUISceneClient GameSceneClient;

	// Open the player stats screen
	GameSceneClient = GetSceneClient();
	if (GameSceneClient != None)
	{
		FoundStatScene = UIScene(FindObject("UI_Scenes_PostGame.PreviousGameReview", class'UIScene'));
		if (FoundStatScene != None)
		{
			OpenScene(FoundStatScene, GetPlayerOwner(PlayerIndex));
		}
	}
	UpdateSceneState(PlayerIndex);

	return true;
}


/**
 * Figure out if we are in a state where the main shortcut is to cancel
 */
function bool IsMainActionCancel()
{
	local GearPartyGRI PartyGRI;
	PartyGRI = GearPartyGRI(GetGRI());

	switch(PartyGRI.MatchmakingState)
	{
		case MMS_ReadingSkills:
		case MMS_FindingBestParty:
		case MMS_FindingAnyParty:
		case MMS_FindingOpposingParty:
		case MMS_WaitingForOpposingParty:
		case MMS_ConnectingToOpposingParty:
		case MMS_CancelingMatchmaking:
			return true;
			break;
	}

	return false;
}


/**
 * Update the cursor icon for the Official Game Match
 *
 * Updates the cursor icon to be a B for cancel matchmaking or A for everything else.
 * Updates the matchmake button's shortcut icon ("lblStart") to B or start label.
 *
 * When the main button's action is to cancel we use B. At other times we use A and symbol.
 */
function UpdateCursorIcon()
{
	//local UILabel CursorIcon;
	local bool bMainActionIsCancel;

	//CursorIcon = UILabel(GetFocusHint());
	bMainActionIsCancel = IsMainActionCancel();

	//if (CursorIcon != None)
	//{
	//	if ( bMainActionIsCancel && MatchmakeButton.IsFocused() )
	//	{
	//		CursorIcon.SetDataStoreBinding("<Strings:GearGameUI.ControllerFont.Controller_B>");
	//	}
	//	else
	//	{
	//		CursorIcon.SetDataStoreBinding("<Strings:GearGameUI.ControllerFont.Controller_A>");
	//	}
	//}

	if (bMainActionIsCancel)
	{
		MatchmakeButtonIcon.SetDataStoreBinding("<Strings:GearGameUI.ControllerFont.Controller_B>");
		MatchmakeButtonIcon.SetVisibility(TRUE);
	}
	else
	{
		MatchmakeButtonIcon.SetDataStoreBinding("<Strings:GearGameUI.ControllerFont.Controller_Start>");
		MatchmakeButtonIcon.SetVisibility(GetWorldInfo().NetMode != NM_Client);
	}

}

/**
 * Called when a new UIState becomes the widget's currently active state, after all activation logic has occurred.
 *
 * @param	Sender					the widget that changed states.
 * @param	PlayerIndex				the index [into the GamePlayers array] for the player that activated this state.
 * @param	NewlyActiveState		the state that is now active
 * @param	PreviouslyActiveState	the state that used the be the widget's currently active state.
 */
function OnStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, optional UIState PreviouslyActiveState )
{
	local bool bTriggerSceneUpdate;

	if (UIState_Focused(NewlyActiveState) != None )
	{
		if ( Sender == pnlOptions || Sender == pnlPlayers || (UIObject(Sender) != None && UIObject(Sender).WidgetTag == 'WeaponSwap') )
		{
			bTriggerSceneUpdate = true;
		}
	}

	if ( Sender == lstGameOptions &&
		 (UIState_Disabled(NewlyActiveState) != None || UIState_Disabled(PreviouslyActiveState) != None) )
	{
		bTriggerSceneUpdate = true;
	}

	if ( bTriggerSceneUpdate )
	{
		UpdateSceneState();
	}

	if ( Sender == MatchmakeButton )
	{
		UpdateCursorIcon();
		if ( UIState_Focused(NewlyActiveState) != None && !IsMatchmaking(PlayerIndex) )
		{
			COGSymbolNormal.EnableWidget(PlayerIndex);
			COGSymbolMatchmaking.EnableWidget(PlayerIndex);
			COGSymbolBroken.EnableWidget(PlayerIndex);
			BloodSplat.EnableWidget(PlayerIndex);
		}
		else
		{
			COGSymbolNormal.DisableWidget(PlayerIndex);
			COGSymbolMatchmaking.DisableWidget(PlayerIndex);
			COGSymbolBroken.DisableWidget(PlayerIndex);
			BloodSplat.DisableWidget(PlayerIndex);
		}
	}

	Super.OnStateChanged( Sender, PlayerIndex, NewlyActiveState, PreviouslyActiveState );
}

/** Called after the options are regenerated but before the scene is updated */
function OnRegeneratedOptionsCallback(GearUIObjectList ObjectList)
{
	local int ChildIndex;
	local GearUICollapsingSelectionList ListChild;
	local UIComp_CollapsingListPresenter ListComp;
	local int Idx;
	local bool bIsHost;
	local GearUICollapsingSelectionList FirstWidgetToNavLoop;
	local GearUICollapsingSelectionList LastWidgetToNavLoop;
	local int ChildIdx;
	local bool bIsPublic;
	local int ListViewSize;


	ListViewSize = (ObjectList == lstPartyOptions) ? 10 : MaxVisibleListItems;

	OnPlaylistChanged();
	RebuildDescriptionList();

	bIsHost = (GetWorldInfo().NetMode < NM_Client);
	Idx = ObjectList.GetObjectInfoIndexFromName(bIsHost ? 'MatchModeOption' : 'RemoteMatchModeOption');
	if (Idx != INDEX_NONE)
	{
		MatchModeList = GearUICollapsingSelectionList(ObjectList.GeneratedObjects[Idx].OptionObj);
	}

	if (bIsHost)
	{
		Idx = ObjectList.GetObjectInfoIndexFromName('HordeWaveOption');
		if (Idx != INDEX_NONE)
		{
			HordeWaveList = GearUICollapsingSelectionList(ObjectList.GeneratedObjects[Idx].OptionObj);
			if (HordeWaveList != none)
			{
				HordeWaveList.ShouldDisableElement = ShouldDisableElementWave;
			}
		}

		Idx = ObjectList.GetObjectInfoIndexFromName('MapSelectOption');
		if (Idx != INDEX_NONE)
		{
			MapSelectionList = GearUICollapsingSelectionList(ObjectList.GeneratedObjects[Idx].OptionObj);
		}

		Idx = ObjectList.GetObjectInfoIndexFromName('WeaponSwapOption');
		if (Idx != INDEX_NONE)
		{
			WeaponSwapList = GearUICollapsingSelectionList(ObjectList.GeneratedObjects[Idx].OptionObj);
		}
	}

	for ( ChildIndex = 0; ChildIndex < ObjectList.Children.Length; ChildIndex++ )
	{
		ListChild = GearUICollapsingSelectionList(ObjectList.Children[ChildIndex]);
		if ( ListChild != None )
		{
			ListChild.OnListCollapsed = SelectionListCollapsed;
			ListChild.IsExpandAllowed = ShouldAllowSelectionListExpansion;

			ListComp = UIComp_CollapsingListPresenter(ListChild.CellDataComponent);
			if ( ListComp != None )
			{
				ListComp.SetMaxElementsPerPage(ListViewSize);
			}
		}
	}

	// See if we generated a widget to loop the uber button to
	if (ObjectList.Children.length > 0)
	{
		ObjectList.SetForcedNavigationTarget(UIFACE_Top, none);
		ObjectList.SetForcedNavigationTarget(UIFACE_Bottom, none);

		bIsPublic = IsOfficialMatch();
		if (bIsPublic)
		{
			if (ObjectList == lstPartyOptions)
			{
				for (ChildIdx = 0; ChildIdx < ObjectList.Children.length; ChildIdx++)
				{
					FirstWidgetToNavLoop = GearUICollapsingSelectionList(ObjectList.Children[ChildIdx]);
					if (FirstWidgetToNavLoop != none)
					{
						break;
					}
				}

				for (ChildIdx = ObjectList.Children.length-1; ChildIdx >= 0; ChildIdx--)
				{
					LastWidgetToNavLoop = GearUICollapsingSelectionList(ObjectList.Children[ChildIdx]);
					if (LastWidgetToNavLoop != none)
					{
						break;
					}
				}

				if (FirstWidgetToNavLoop != none && LastWidgetToNavLoop != none)
				{
					MatchmakeButton.SetForcedNavigationTarget(UIFACE_Top, LastWidgetToNavLoop);
					MatchmakeButton.SetForcedNavigationTarget(UIFACE_Bottom, FirstWidgetToNavLoop);
					FirstWidgetToNavLoop.SetForcedNavigationTarget(UIFACE_Top, MatchmakeButton);
					LastWidgetToNavLoop.SetForcedNavigationTarget(UIFACE_Bottom, MatchmakeButton);
				}
			}
		}
		else
		{
			if (ObjectList == lstPartyOptions)
			{
				for (ChildIdx = 0; ChildIdx < ObjectList.Children.length; ChildIdx++)
				{
					FirstWidgetToNavLoop = GearUICollapsingSelectionList(ObjectList.Children[ChildIdx]);
					if (FirstWidgetToNavLoop != none)
					{
						break;
					}
				}
				if (FirstWidgetToNavLoop != none)
				{
					MatchmakeButton.SetForcedNavigationTarget(UIFACE_Bottom, FirstWidgetToNavLoop);
					FirstWidgetToNavLoop.SetForcedNavigationTarget(UIFACE_Top, MatchmakeButton);
					FirstWidgetToNavLoop = none;
				}

				// Link the party list with the game list
				if (lstGameOptions != none)
				{
					for (ChildIdx = ObjectList.Children.length-1; ChildIdx >= 0; ChildIdx--)
					{
						LastWidgetToNavLoop = GearUICollapsingSelectionList(ObjectList.Children[ChildIdx]);
						if (LastWidgetToNavLoop != none)
						{
							break;
						}
					}
					for (ChildIdx = 0; ChildIdx < lstGameOptions.Children.length; ChildIdx++)
					{
						FirstWidgetToNavLoop = GearUICollapsingSelectionList(lstGameOptions.Children[ChildIdx]);
						if (FirstWidgetToNavLoop != none)
						{
							break;
						}
					}
					// Hook the navigation
					if (FirstWidgetToNavLoop != none && LastWidgetToNavLoop != none)
					{
						LastWidgetToNavLoop.SetForcedNavigationTarget(UIFACE_Bottom, FirstWidgetToNavLoop);
						FirstWidgetToNavLoop.SetForcedNavigationTarget(UIFACE_Top, LastWidgetToNavLoop);
					}
				}
			}
			else if (ObjectList == lstGameOptions)
			{
				for (ChildIdx = ObjectList.Children.length-1; ChildIdx >= 0; ChildIdx--)
				{
					LastWidgetToNavLoop = GearUICollapsingSelectionList(ObjectList.Children[ChildIdx]);
					if (LastWidgetToNavLoop != none)
					{
						break;
					}
				}
				if (LastWidgetToNavLoop != none)
				{
					MatchmakeButton.SetForcedNavigationTarget(UIFACE_Top, LastWidgetToNavLoop);
					LastWidgetToNavLoop.SetForcedNavigationTarget(UIFACE_Bottom, MatchmakeButton);
					LastWidgetToNavLoop = none;
				}

				// Link the party list with the game list
				if (lstPartyOptions != none)
				{
					for (ChildIdx = 0; ChildIdx < ObjectList.Children.length; ChildIdx++)
					{
						FirstWidgetToNavLoop = GearUICollapsingSelectionList(ObjectList.Children[ChildIdx]);
						if (FirstWidgetToNavLoop != none)
						{
							break;
						}
					}
					for (ChildIdx = lstPartyOptions.Children.length-1; ChildIdx >= 0; ChildIdx--)
					{
						LastWidgetToNavLoop = GearUICollapsingSelectionList(lstPartyOptions.Children[ChildIdx]);
						if (LastWidgetToNavLoop != none)
						{
							break;
						}
					}
					// Hook the navigation
					if (FirstWidgetToNavLoop != none && LastWidgetToNavLoop != none)
					{
						LastWidgetToNavLoop.SetForcedNavigationTarget(UIFACE_Bottom, FirstWidgetToNavLoop);
						FirstWidgetToNavLoop.SetForcedNavigationTarget(UIFACE_Top, LastWidgetToNavLoop);
					}
				}
			}
		}
	}
}

/**
 * Allows the widget to force specific elements to be disabled.  If not implemented, or if the return value
 * is false, the list's data provider will then be given an opportunity to disable the item.
 *
 * @param	Sender			the list calling the delegate
 * @param	ElementIndex	the index [into the data store's list of items] for the item to query
 *
 * @return	TRUE if the specified element should be disabled.
 */
function bool ShouldDisableElementWave( UIList Sender, int ElementIndex )
{
	return (ElementIndex > 0 && HordeWaveCompleted[ElementIndex-1] == 0);
}

/**
 * Handler for the OnListCollapsed delegate of all GearUISelectionLists in the scene.  Clears the null override on the list's horizontal
 * navigation links to re-enable the user to navigate left and right.
 *
 * @param	Sender	the list that is about the begin collapsing
 */
function SelectionListCollapsed( GearUICollapsingSelectionList Sender )
{
	// list is now collapsed - clear the forced navigation override
	Sender.SetForcedNavigationTarget(UIFACE_Left, None, false);
	Sender.SetForcedNavigationTarget(UIFACE_Right, None, false);
}

/**
 * Handler for the IsExpandAllowed delegate of all GearUISelectionLists in the scene.  Sets a null override on the list's horizontal
 * navigation links to prevent the user from navigating left or right while the list is open.
 *
 * @param	Sender	the list that is about the begin expanding
 *
 * @return	return FALSE to prevent the list from expanding.
 */
function bool ShouldAllowSelectionListExpansion( GearUICollapsingSelectionList Sender )
{
	// list is expanded - set the null override on horizontal navigation
	Sender.SetForcedNavigationTarget(UIFACE_Left, None, true);
	Sender.SetForcedNavigationTarget(UIFACE_Right, None, true);
	return true;
}

/**
 * Handler for options' list element data providers OnDataProviderPropertyChange delegate.  When the value of an option is changed, this
 * function is called.
 *
 * Responsible for sending updated changes to clients via GearMenuPC.ClientUpdateGameSettingValue()
 *
 * @param	SourceProvider	the data provider that generated the notification
 * @param	SettingsName	the name of the setting that changed
 */
singular function SettingsProviderUpdated( UIDataProvider SourceProvider, optional name SettingsName )
{
	local UIDataStore_OnlineGameSettings SettingsDS;
	local GearProfileSettings Profile;
	local UIDataProvider_SettingsArray ArraySettingsProvider;
	local int SettingsId, ValueContextId;
	local GearMenuPC OwnerPC, PartyPC;
	local bool bSendUpdate;
	local GearPartyGameSettings PartySettings;

	if ( IsPartyLeader(GetBestPlayerIndex()) )
	{
		ArraySettingsProvider = UIDataProvider_SettingsArray(SourceProvider);
		if ( ArraySettingsProvider != None )
		{
			if ( ArraySettingsProvider.Settings != None )
			{
				SettingsId = ArraySettingsProvider.SettingsId;
				SettingsName = ArraySettingsProvider.SettingsName;
				if ( ArraySettingsProvider.Settings.GetStringSettingValue(SettingsId, ValueContextId) )
				{
					bSendUpdate = true;
				}
			}
		}
		else
		{
			// this is the case when the gametype itself has been changed.  We need to send this value change to all clients, then we'll
			// send the server's values for the new gametype's options once they're loaded from the game settings data store (this will happen
			// automatically as each option's value is initialized).
			SettingsDS = UIDataStore_OnlineGameSettings(SourceProvider);
			if ( SettingsDS != None && SettingsName == 'SelectedIndex' )
			{
				Profile = GetPlayerProfile(0);
				if ( Profile.GetProfileSettingValueId(Profile.const.VERSUS_GAMETYPE, ValueContextId) )
				{
					SettingsId = class'GearVersusGameSettings'.const.CONTEXT_VERSUSMODES;
					SettingsName = 'GameType';
					bSendUpdate = true;

					LoadGameSettingsFromProfile();
				}
			}
		}
	}

	if ( bSendUpdate )
	{
		PartySettings = GetPartySettings();
		if ( PartySettings != None )
		{
			if ( SettingsId == class'GearProfileSettings'.const.VERSUS_MATCH_MODE ||
				 SettingsId == class'GearProfileSettings'.const.VERSUS_PARTY_TYPE ||
				 SettingsId == PartySettings.const.CONTEXT_VERSUSMODES )
			{
				PartySettings.SetStringSettingValue(SettingsId, ValueContextId, false);
			}

			OwnerPC = GearMenuPC(GetGearPlayerOwner(0));
			foreach OwnerPC.WorldInfo.AllControllers(class'GearMenuPC', PartyPC)
			{
				if ( !PartyPC.IsLocalPlayerController() && PartyPC.IsPrimaryPlayer() )
				{
					PartyPC.ClientUpdateGameSettingValue(SettingsId, ValueContextId);
				}
			}
		}
	}
}

/**
 * Callback function when the scene gets input
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool ProcessRawInput( out InputEventParameters EventParms )
{
	if (EventParms.InputKeyName == 'XboxTypeS_Start' &&
		     EventParms.EventType == IE_Pressed &&
			 EventParms.PlayerIndex == 0 &&
			 GetWorldInfo().NetMode != NM_Client)
	{
		if ( !IsMainActionCancel() )
		{
			if ( !pnlPlayers.IsFocused() )
			{
				MatchmakeButton.SetFocus(none);
			}

			StartMatchButtonClicked(MatchmakeButton, 0);

			return true;
		}

	}

	return false;
}


/**
 * Allows others to be notified when this scene is closed.  Called after the SceneDeactivated event, after the scene has published
 * its data to the data stores bound by the widgets of this scene.
 *
 * @param	DeactivatedScene	the scene that was deactivated
 */
delegate SceneDeactivatedHandler( UIScene DeactivatedScene )
{
	local OnlineSubsystem OnlineSub;
	local int ControllerId;

	if (DeactivatedScene == Self)
	{
		ControllerId = class'UIInteraction'.static.GetPlayerControllerId(GetBestPlayerIndex());
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();

		if (OnlineSub != None && OnlineSub.ContentInterface != None)
		{
			OnlineSub.ContentInterface.ClearQueryAvailableDownloadsComplete(ControllerId, OnReadAvailableContentComplete);
		}
	}
}

defaultproperties
{
	OnGearUISceneTick=UpdatePartyLobbyScene
	OnSceneActivated=SceneActivationComplete
	OnSceneDeactivated=SceneDeactivatedHandler
	OnProcessInputKey=ProcessInputKey
	OnRawInputKey=ProcessRawInput
	MaxVisibleListItems=4

	WeaponSwapScene=GearUISceneFE_MPWeaponSwap'UI_Scenes_Common.UI_FE_WeaponSwap'

	MatchRedBG=Surface'UI_Art.FrontEnd.Party_FindGame_Message_Red'
	MatchOrangeBG=Surface'UI_Art.FrontEnd.Party_FindGame_Message_PlaylistOrange'
	MatchGrayBG=Surface'UI_Art.FrontEnd.Party_FindGame_Message_Playlist'

	WhatsUpSceneReference=UIScene'UI_Scenes_WU.UI_FE_WhatsUp'

	ClickedCue = Clicked

	Begin Object Class=UIAnimationSeq Name=FadeIn_Template
		SeqName=FadeIn
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.3,Data=(DestAsFloat=1.0))))
	End Object
	FadeIn = FadeIn_Template

	Begin Object Class=UIAnimationSeq Name=FadeOut_Template
		SeqName=FadeOut
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.3,Data=(DestAsFloat=0.0))))
	End Object
	FadeOut = FadeOut_Template

	Begin Object Class=UIAnimationSeq Name=SlideIn_Template
		SeqName=SlideIn
		Tracks(0)=(TrackType=EAT_Left,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=-800.0)),(RemainingTime=0.50,Data=(DestAsFloat=0.0),InterpMode=UIANIMMODE_EaseOut,InterpExponent=1.5)))
	End Object
	SlideIn = SlideIn_Template

	Begin Object Class=UIAnimationSeq Name=SlideOut_Template
		SeqName=SlideOut
		Tracks(0)=(TrackType=EAT_Left,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.50,Data=(DestAsFloat=-800.0),InterpMode=UIANIMMODE_EaseIn,InterpExponent=1.5)))
	End Object
	SlideOut = SlideOut_Template
}
