/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearUISceneFE_Training extends GearUISceneFrontEnd_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** References to the buttons you can select in the scene */
var transient array<UILabelButton> SelectionButtons;

/** Reference to the lesson image */
var transient UIImage LessonImage;

/** String names of the paths to the lesson images */
var array<string> LessonImagePaths;

/** Reference to the button bar */
var transient UICalloutButtonPanel ButtonBar;

/** The index of the button that was last selected */
var transient int LastSelectedIndex;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
* Called after this screen object's children have been initialized
* Overloaded to initialized the references to this scene's widgets
*/
event PostInitialize()
{
	InitializeWidgetReferences();

	Super.PostInitialize();
}

/** Initialize the widget references */
final function InitializeWidgetReferences()
{
	local int Idx;
	local string ButtonName, DataBindString;
	local GearPC CurrGearPC;

	LessonImage = UIImage(FindChild('imgDescriptionImage', true));

	CurrGearPC = GetGearPlayerOwner(GetBestPlayerIndex());
	SelectionButtons.length = eGEARTRAIN_MAX + 1;
	for (Idx = 0; Idx < SelectionButtons.length; Idx++ )
	{
		ButtonName = "btnLesson" $ Idx;
		SelectionButtons[Idx] = UILabelButton(FindChild(Name(ButtonName), true));
		SelectionButtons[Idx].OnClicked = SelectionButtonClicked;
		if (Idx == 0)
		{
			SelectionButtons[Idx].SetFocus(none);
			LessonImage.SetDataStoreBinding( LessonImagePaths[Idx] );
		}

		if ( CurrGearPC.ProfileSettings != None &&
			 Idx < eGEARTRAIN_MAX )
		{
			if ( CurrGearPC.ProfileSettings.HasCompletedTraining(EGearTrainingType(Idx)) )
			{
				DataBindString = SelectionButtons[Idx].GetDataStoreBinding();
				AddNavigationIconMarkup( eGEARNAVICON_Complete, DataBindString );
				SelectionButtons[Idx].SetDataStoreBinding( DataBindString );
			}
			else if ( CurrGearPC.ProfileSettings.HasTrainingBeenViewed(EGearTrainingType(Idx)) )
			{
				DataBindString = SelectionButtons[Idx].GetDataStoreBinding();
				AddNavigationIconMarkup( eGEARNAVICON_NewStuff, DataBindString );
				SelectionButtons[Idx].SetDataStoreBinding( DataBindString );
			}
		}
	}

	ButtonBar = UICalloutButtonPanel(FindChild('btnbarCommon', true));
	ButtonBar.SetButtonCallback('GenericBack', OnBackClicked);
}

/**
 * Called just after the scene is added to the ActiveScenes array, or when this scene has become the active scene as a result
 * of closing another scene.
 *
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
event SceneActivated( bool bInitialActivation )
{
	Super.SceneActivated( bInitialActivation );
	RefreshTooltip( SelectionButtons[0] );
}

/** Accepts the changes and closes the scene */
function bool OnBackClicked(UIScreenObject EventObject, int PlayerIndex)
{
	CloseScene(self);
	return true;
}

/**
 * Handler for the OnClick delegate for all of the selection buttons in this scene.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool SelectionButtonClicked(UIScreenObject EventObject, int PlayerIndex)
{
	local int Idx;
	local GearPC CurrGearPC;

	CurrGearPC = GetGearPlayerOwner(PlayerIndex);

	// Find the matching widget so we know which value to set the profile to
	for (Idx = 0; Idx < SelectionButtons.length; Idx++)
	{
		if ( EventObject == SelectionButtons[Idx] )
		{
			LastSelectedIndex = Idx;
			CurrGearPC.SaveProfile(OnProfileWriteCompleteForTrainingTransition);
			break;
		}
	}

	return true;
}

/** Called when we return from saving the profile - start the training grounds */
function OnProfileWriteCompleteForTrainingTransition(byte LocalUserNum, bool bWasSuccessful)
{
	local EGearTrainingType TrainType;
	local int PlayerIndex;
	local OnlinePlayerInterface PlayerInt;
	local OnlineSubsystem OnlineSub;
	local GearPC CurrGearPC;

	// Clear the delegate
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != none)
	{
		PlayerInt = OnlineSub.PlayerInterface;
		if (PlayerInt != None)
		{
			PlayerInt.ClearWriteProfileSettingsCompleteDelegate(LocalUserNum, OnProfileWriteCompleteForTrainingTransition);
		}
	}

	// Get the training type
	if (LastSelectedIndex < eGEARTRAIN_MAX)
	{
		TrainType = EgearTrainingType(LastSelectedIndex);
	}

	// Mark the training ground as viewed
	PlayerIndex = class'UIInteraction'.static.GetPlayerIndex(LocalUserNum);
	CurrGearPC = GetGearPlayerOwner(PlayerIndex);
	if (CurrGearPC != none && LastSelectedIndex < eGEARTRAIN_MAX)
	{
		if ( CurrGearPC.ProfileSettings.HasTrainingBeenViewed(TrainType) )
		{
			CurrGearPC.ProfileSettings.MarkTrainingAsViewed(TrainType, CurrGearPC);
		}
	}

	// Launch the training ground
	LaunchTrainingGame(TrainType, LastSelectedIndex >= eGEARTRAIN_MAX);
}

/** Launch the game we selected */
function bool LaunchTrainingGame( EGearTrainingType TrainType, bool bBuildYourOwn )
{
	local string URL;
	local GearPC CurrGearPC;
	local GearMenuGame MenuGame;

	if ( bBuildYourOwn )
	{
		CurrGearPC = GetGearPlayerOwner(GetBestPlayerIndex());
		if (CurrGearPC != None && CurrGearPC.ProfileSettings != None)
		{
			MenuGame = GearMenuGame(GetWorldInfo().Game);
			if (MenuGame != None)
			{
				MenuGame.CreateLocalParty(CurrGearPC.ProfileSettings, true);
			}
		}
	}
	else
	{
		URL = "MP_Training?game=geargamecontent.";
		switch ( TrainType )
		{
			case eGEARTRAIN_Basic:
				URL $= "GearGameTDM?bots=5?goalscore=3";
				break;
			case eGEARTRAIN_Execution:
				URL $= "GearGameTDM?bIsExecution?bots=9?goalscore=3";
				break;
			case eGEARTRAIN_Respawn:
				URL $= "GearGameKTL?bots=9?goalscore=1";
				break;
			case eGEARTRAIN_Objective:
				URL $= "GearGameAnnex?bots=9?goalscore=1";
				break;
			case eGEARTRAIN_Meatflag:
				URL $= "GearGameCTM?bots=9?goalscore=1";
				break;
		}
		URL $= "?casual?Training=" $ int(TrainType);
		`Log("Launching training with URL '"$URL$"'");

		ShowLoadingMovie(true);
		//@todo should this use ClientTravel instead? otherwise the UI doesn't get the notification...
		GetWorldInfo().ConsoleCommand("open" @ URL);
	}

	return true;
}

/**
 * Called when a new UIState becomes the widget's currently active state, after all activation logic has occurred.
 *
 * @param	Sender					the widget that changed states.
 * @param	PlayerIndex				the index [into the GamePlayers array] for the player that activated this state.
 * @param	NewlyActiveState		the state that is now active
 * @param	PreviouslyActiveState	the state that used the be the widget's currently active state.
 */
function OnStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, UIState PreviouslyActiveState )
{
	local int Idx;

	if ( LessonImage != None )
	{
		for ( Idx = 0; Idx < SelectionButtons.length; Idx++ )
		{
			if ( SelectionButtons[Idx] != None && SelectionButtons[Idx] == Sender && NewlyActiveState.IsA('UIState_Focused') )
			{
				LessonImage.SetDataStoreBinding( LessonImagePaths[Idx] );
				break;
			}
		}
	}

	Super.OnStateChanged( Sender, PlayerIndex, NewlyActiveState, PreviouslyActiveState );
}

defaultproperties
{
	bAllowPlayerJoin=false
	bAllowSigninChanges=false

	LessonImagePaths(eGEARTRAIN_Basic)="<Images:UI_Portraits.Training.TR_Lesson01>"
	LessonImagePaths(eGEARTRAIN_Execution)="<Images:UI_Portraits.Training.TR_Lesson02>"
	LessonImagePaths(eGEARTRAIN_Respawn)="<Images:UI_Portraits.Training.TR_Lesson03>"
	LessonImagePaths(eGEARTRAIN_Objective)="<Images:UI_Portraits.Training.TR_Lesson04>"
	LessonImagePaths(eGEARTRAIN_Meatflag)="<Images:UI_Portraits.Training.TR_Lesson05>"
	LessonImagePaths(eGEARTRAIN_MAX)="<Images:UI_Portraits.Training.TR_Build>"
}
