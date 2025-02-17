/**
 * GearTutorialManager
 *
 * Contains the tutorials and manages their use.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorialManager extends Object within GearPC
	config(Game)
	native
	transient;

/** GoW global macros */


/************************************************************************/
/* Constants															*/
/************************************************************************/
const TUTORIAL_PRIORITY_AUTOINIT					= 200;
const TUTORIAL_PRIORITY_AUTOINIT_WEAPON				= 100;
const TUTORIAL_PRIORITY_LOW							= 300;
const TUTORIAL_PRIORITY_NORMAL						= 400;
const TUTORIAL_PRIORITY_HIGH						= 500;
const TUTORIAL_PRIORITY_IMMEDIATE					= 600;


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Constant list of tutorial classes, one for each class indexed by EGearTutorialType */
var array<class<GearTutorial_Base> > TutorialClasses;

/** Constant list of tutorial types that are to be automatically added to the system if not completed yet in single player */
var config array<EGearTutorialType> SPAutoInitiatedTutorialTypes;
/** Constant list of tutorial types that are to be automatically added to the system if not completed yet in multiplayer */
var config array<EGearTutorialType> MPAutoInitiatedTutorialTypes;
/** Constant list of tutorial types that are to be automatically added to the system if not completed yet in training grounds lesson 1 */
var config array<EGearTutorialType> TrainAutoInitiatedTutorialTypes_1;
/** Constant list of tutorial types that are to be automatically added to the system if not completed yet in training grounds lesson 2 */
var config array<EGearTutorialType> TrainAutoInitiatedTutorialTypes_2;
/** Constant list of tutorial types that are to be automatically added to the system if not completed yet in training grounds lesson 3 */
var config array<EGearTutorialType> TrainAutoInitiatedTutorialTypes_3;
/** Constant list of tutorial types that are to be automatically added to the system if not completed yet in training grounds lesson 4 */
var config array<EGearTutorialType> TrainAutoInitiatedTutorialTypes_4;
/** Constant list of tutorial types that are to be automatically added to the system if not completed yet in training grounds lesson 5 */
var config array<EGearTutorialType> TrainAutoInitiatedTutorialTypes_5;
/** The auto tutorials we are currently handling */
var EGearAutoInitTutorialTypes AutoInitType;
/** Whether auto tutorials are being suspended or not */
var bool bAutoTutorialsSuspended;

/** List of tutorials currently in the system */
var transient array<GearTutorial_Base> Tutorials;

/** The currently active tutorial - more than one tutorial can be active but this one would take precedence for displaying a scene */
var transient GearTutorial_Base ActiveTutorial;

/** Reference to the current tutorial's input handler if one exists */
var transient GearPlayerInput_Base CurrentTutorialInputHandler;

/** Whether the tutorial system is currently active (on/off) */
var bool bTutorialSystemIsActive;

/** Bitwise storage of the completed Sinlge Player tutorials - saved to the ProfileSettings */
var int CompletedTutorialsSP1;
var int CompletedTutorialsSP2;
var int CompletedTutorialsSP3;
/** Bitwise storage of the completed Multi-Player tutorials - saved to the ProfileSettings */
var int CompletedTutorialsMP1;
var int CompletedTutorialsMP2;
var int CompletedTutorialsMP3;

/** Bitwise storage of the completed training grounds tutorials*/
var int CompletedTutorialsTrain1;
var int CompletedTutorialsTrain2;
var int CompletedTutorialsTrain3;

/** Whether the game options for whether automatic tutorials are on or not (set by game options) */
var bool bGameIsUsingAutoTutorials;

/**
 * AudioComponent for player a sound that is associated with a tutorial.
 * This is sound is meant to get stopped if another wants to start.
 */
var transient AudioComponent TutorialSoundAC;

/************************************************************************/
/* C++ functions                                                        */
/************************************************************************/
cpptext
{
	/**
	 * Checks to see if there is an active tutorial, and if not it will
	 * look for a new tutorial to start
	 */
	void Update();
}

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Initialize the tutorial manager */
final function StartSystem( optional bool bTurnSystemOn = true, optional bool bWipeTutorials = true, optional EGearAutoInitTutorialTypes AutoTutType = eGAIT_None )
{
	`DEBUGTUTORIAL("GearTutorialManager::Initialize: bTurnSystemOn="$bTurnSystemOn);

	// Don't initialize if we are already initialized
	if ( bTutorialSystemIsActive )
	{
		`DEBUGTUTORIAL("GearTutorialManager::Initialize: Failed because system is already initialized");
		return;
	}

	// Set the auto init type
	if ( AutoTutType != eGAIT_None )
	{
		AutoInitType = AutoTutType;
	}

	// Clear the array in case this function is called more than once (shouldn't happen)
	if ( bWipeTutorials )
	{
		GearEventList.Length = 0;
		GearWeaponEquipDelegates.Length = 0;
		Tutorials.length = 0;
	}

	// Turn system on/off based on param
	bTutorialSystemIsActive = bTurnSystemOn;
}

/** Uninitialize the tutorial manager */
final function StopSystem( optional bool bWipeTutorials = true )
{
	`DEBUGTUTORIAL("GearTutorialManager::Uninitialize: bTutorialSystemIsActive="$bTutorialSystemIsActive);

	// Don't uninitialize if we aren't already initialized
	if ( !bTutorialSystemIsActive )
	{
		`DEBUGTUTORIAL("GearTutorialManager::Uninitialize: Failed because system is not initialized");
		return;
	}

	// Turn system off
	bTutorialSystemIsActive = false;

	if ( bWipeTutorials )
	{
		// Remove all tutorials
		RemoveAllTutorials();
	}
	else
	{
		// Deactivate the current tutorial
		DeactivateTutorial();
	}

	// Sanity check for the input system...  RemoveAllTutorials should have cleaned this but let's make sure
	RemoveInputHandlerFromSystem();
}

/** Deactivates the current tutorial and sets all tutorials to not be ready for activation */
final function RebootSystem()
{
	local int Idx;

	`DEBUGTUTORIAL("GearTutorialManager::RebootSystem");

	DeactivateTutorial();

	// Loop through the list of tutorials and remove all the autos
	for ( Idx = 0; Idx < Tutorials.length; Idx++ )
	{
		// delete Kismet controlled tutorials when restarting (for reloading checkpoints - they need to be retriggered)
		if (Tutorials[Idx].bIsKismetTutorial)
		{
			Tutorials.Remove(Idx, 1);
			Idx--;
		}
		else
		{
			Tutorials[Idx].bTutorialIsReadyForActivation = false;
		}
	}
}

/** Removes the current input handler from the system */
final function RemoveInputHandlerFromSystem()
{
	if ( CurrentTutorialInputHandler != None )
	{
		PopPlayerInput( CurrentTutorialInputHandler );
		CurrentTutorialInputHandler = None;
	}
}

/** Start the tutorial sound */
final function StartTutorialSound(string SoundPath)
{
	local SoundCue CueToPlay;

	// Find the sound cue
	CueToPlay = SoundCue(DynamicLoadObject(SoundPath, class'SoundCue'));
	if (CueToPlay != None)
	{
		// Stop an existing sound
		if (TutorialSoundAC != none)
		{
			StopTutorialSound();
		}

		// Create and set the new sound
		TutorialSoundAC = Outer.CreateAudioComponent(CueToPlay, false, true);
		if (TutorialSoundAC != none)
		{
			TutorialSoundAC.bAllowSpatialization = false;
			TutorialSoundAC.bAutoDestroy = true;
			TutorialSoundAC.bIsUISound = true;
			TutorialSoundAC.Play();
		}
	}
}

/** Stop the tutorial sound */
final function StopTutorialSound()
{
	if (TutorialSoundAC != none)
	{
		TutorialSoundAC.FadeOut(0.3f, 0.0f);
		TutorialSoundAC = none;
	}
}

/** Removes the auto tutorials from the system and suspends them from being added */
final function SuspendAutoTutorials()
{
	`DEBUGTUTORIAL("GearTutorialManager::SuspendAutoTutorials: AutoInitType="$AutoInitType@bAutoTutorialsSuspended);
	if ( !bAutoTutorialsSuspended )
	{
		RemoveAutoInitiatedTutorials();
		bAutoTutorialsSuspended = true;
	}
}

/** Un-suspend the auto tutorials and re-add them */
final function UnsuspendAutoTutorials()
{
	`DEBUGTUTORIAL("GearTutorialManager::UnsuspendAutoTutorials: AutoInitType="$AutoInitType@bAutoTutorialsSuspended);
	if ( bAutoTutorialsSuspended )
	{
		bAutoTutorialsSuspended = false;
		AddAutoInitiatedTutorials();
	}
}

/**
 * Returns the "completed" variable that is saved to the profile
 * and the number of bits to shift in that variable to get to the tutorial's location in that variable.
 */
final function GetTutorialCompletedData( EGearTutorialType TutorialType, out int CompletedVariable, out int BitsToShift, optional EGearAutoInitTutorialTypes AutoType = eGAIT_None )
{
	local int VarIndex;

	if ( AutoType == eGAIT_None )
	{
		AutoType = AutoInitType;
	}

	VarIndex = TutorialType / 32;

	switch (AutoType)
	{
		case eGAIT_SP:
			switch (VarIndex)
			{
				case 0: CompletedVariable = CompletedTutorialsSP1;	break;
				case 1: CompletedVariable = CompletedTutorialsSP2;	break;
				case 2: CompletedVariable = CompletedTutorialsSP3;	break;
			}
			break;

		case eGAIT_MP:
			switch (VarIndex)
			{
				case 0: CompletedVariable = CompletedTutorialsMP1;	break;
				case 1: CompletedVariable = CompletedTutorialsMP2;	break;
				case 2: CompletedVariable = CompletedTutorialsMP3;	break;
			}
			break;

		case eGAIT_Train1:
		case eGAIT_Train2:
		case eGAIT_Train3:
		case eGAIT_Train4:
		case eGAIT_Train5:
			switch (VarIndex)
			{
				case 0: CompletedVariable = CompletedTutorialsTrain1;	break;
				case 1: CompletedVariable = CompletedTutorialsTrain2;	break;
				case 2: CompletedVariable = CompletedTutorialsTrain3;	break;
			}
			break;
	}

	BitsToShift = TutorialType % 32;
}

/** Set the correct "completed" variable that will be saved to the profile */
final function SetTutorialCompletedVariable( EGearTutorialType TutorialType, EGearAutoInitTutorialTypes AutoType )
{
	local int CompleteTest, ShiftValue, CompleteValue, VarIndex, ValueToSetTo;

	// Grab the correct variable to use for this tutorial type
	GetTutorialCompletedData( TutorialType, CompleteTest, ShiftValue, AutoType );

	// Calculate the CompletedTutorials bitfield
	CompleteValue = CompleteTest | (1 << ShiftValue);
	// Construct the new value for the variable
	ValueToSetTo = CompleteTest | CompleteValue;

	// Get the variable index for marking the correct variable
	VarIndex = TutorialType / 32;

	// Set the proper variable
	switch (AutoType)
	{
		case eGAIT_SP:
			switch (VarIndex)
			{
				case 0: CompletedTutorialsSP1 = ValueToSetTo;	break;
				case 1: CompletedTutorialsSP2 = ValueToSetTo;	break;
				case 2: CompletedTutorialsSP3 = ValueToSetTo;	break;
			}
			break;

		case eGAIT_MP:
			switch (VarIndex)
			{
				case 0: CompletedTutorialsMP1 = ValueToSetTo;	break;
				case 1: CompletedTutorialsMP2 = ValueToSetTo;	break;
				case 2: CompletedTutorialsMP3 = ValueToSetTo;	break;
			}
			break;

		case eGAIT_Train1:
		case eGAIT_Train2:
		case eGAIT_Train3:
		case eGAIT_Train4:
		case eGAIT_Train5:
			switch (VarIndex)
			{
				case 0: CompletedTutorialsTrain1 = ValueToSetTo;	break;
				case 1: CompletedTutorialsTrain2 = ValueToSetTo;	break;
				case 2: CompletedTutorialsTrain3 = ValueToSetTo;	break;
			}
			break;
	}
}

/** Whether a tutorial has been completed or not */
final function bool IsTutorialCompleted( EGearTutorialType TutorialType )
{
	local int CompleteTest, ShiftValue, AutoInitIdx;
	local array<EGearTutorialType> AutoInitList;

	// If there is no online subsystem (no profile) pretend the auto init tutorials are complete
 	if ( OnlineSub == None || ProfileSettings == None )
 	{
		switch (AutoInitType)
		{
			case eGAIT_SP:		AutoInitList = SPAutoInitiatedTutorialTypes;		break;
			case eGAIT_MP:		AutoInitList = MPAutoInitiatedTutorialTypes;		break;
			case eGAIT_Train1:	AutoInitList = TrainAutoInitiatedTutorialTypes_1;	break;
			case eGAIT_Train2:	AutoInitList = TrainAutoInitiatedTutorialTypes_2;	break;
			case eGAIT_Train3:	AutoInitList = TrainAutoInitiatedTutorialTypes_3;	break;
			case eGAIT_Train4:	AutoInitList = TrainAutoInitiatedTutorialTypes_4;	break;
			case eGAIT_Train5:	AutoInitList = TrainAutoInitiatedTutorialTypes_5;	break;
		}

		for ( AutoInitIdx = 0; AutoInitIdx < AutoInitList.length; AutoInitIdx++ )
		{
			if ( AutoInitList[AutoInitIdx] == TutorialType )
			{
				return TRUE;
			}
		}
 	}

	// Grab the correct variable to use for this tutorial type
	GetTutorialCompletedData( TutorialType, CompleteTest, ShiftValue );

	return (CompleteTest & (1 << ShiftValue)) != 0;
}

/** Called by the update function when it detects that a tutorial has completed */
final event OnTutorialCompleted( GearTutorial_Base CompletedTutorial )
{
	`DEBUGTUTORIAL("GearTutorialManager::OnTutorialCompleted: Type="$CompletedTutorial.TutorialType@"CompletedTutorial="$CompletedTutorial);

	if ( CompletedTutorial != None )
	{
		// Store and save the fact that this is complete
		MarkTutorialComplete( CompletedTutorial.TutorialType, true, CompletedTutorial );

		// Remove the tutorial from the system
		RemoveTutorial( CompletedTutorial.TutorialType );
	}
}

/** Mark the tutorial as completed */
final function MarkTutorialComplete( EGearTutorialType TutorialType, optional bool bSaveProfile = true, optional GearTutorial_Base TutorialObj = none )
{
	local int AutoInitListIdx;
	local bool bMustSaveProfile;
	local array<EGearTutorialType> AutoInitList;
	local int ListIdx;

	// We will only mark this tutorial as complete if we are in SP
	if ( AutoInitType == eGAIT_SP )
	{
		// See if this tutorial is in the SP list of tutorials that are code induced and should never happen again
		for ( AutoInitListIdx = 0; AutoInitListIdx < SPAutoInitiatedTutorialTypes.length; AutoInitListIdx++ )
		{
			// Found it, now mark it completed
			if ( SPAutoInitiatedTutorialTypes[AutoInitListIdx] == TutorialType )
			{
				SetTutorialCompletedVariable( TutorialType, eGAIT_SP );
				bMustSaveProfile = TRUE;
				break;
			}
		}
	}

	// We will always mark tutorials as complete in all sections except SP
	for ( ListIdx = 0; ListIdx < eGAIT_MAX; ListIdx++ )
	{
		if ( ListIdx != eGAIT_None && ListIdx != eGAIT_SP )
		{
			switch (ListIdx)
			{
				case eGAIT_MP:		AutoInitList = MPAutoInitiatedTutorialTypes;		break;
				case eGAIT_Train1:	AutoInitList = TrainAutoInitiatedTutorialTypes_1;	break;
				case eGAIT_Train2:	AutoInitList = TrainAutoInitiatedTutorialTypes_2;	break;
				case eGAIT_Train3:	AutoInitList = TrainAutoInitiatedTutorialTypes_3;	break;
				case eGAIT_Train4:	AutoInitList = TrainAutoInitiatedTutorialTypes_4;	break;
				case eGAIT_Train5:	AutoInitList = TrainAutoInitiatedTutorialTypes_5;	break;
			}

			// See if this tutorial is in the list of tutorials that are code induced and should never happen again
			for ( AutoInitListIdx = 0; AutoInitListIdx < AutoInitList.length; AutoInitListIdx++ )
			{
				// Found it, now mark it completed
				if ( AutoInitList[AutoInitListIdx] == TutorialType )
				{
					SetTutorialCompletedVariable( TutorialType, EGearAutoInitTutorialTypes(ListIdx) );
					bMustSaveProfile = TRUE;
					break;
				}
			}
		}
	}

	// If a tutorial object was passed in, make sure it wants to be saved to the profile
	if ( TutorialObj != none && !TutorialObj.bSaveTutorialToProfile )
	{
		bMustSaveProfile = false;
	}

	if ( bMustSaveProfile )
	{
		// Set the profile values
		ProfileSettings.SetTutorialValues( CompletedTutorialsSP1, CompletedTutorialsSP2, CompletedTutorialsSP3, CompletedTutorialsMP1, CompletedTutorialsMP2, CompletedTutorialsMP3, CompletedTutorialsTrain1, CompletedTutorialsTrain2, CompletedTutorialsTrain3, Outer, bSaveProfile );
	}

	//`DEBUGTUTORIAL("GearTutorialManager::MarkTutorialComplete: Type="$TutorialType@"PrevComplete="$CompletedTutorials@"UpdatedComplete="$CompletedTutorials | CompleteValue);
}

/** Called by kismet to tell this tutorial that it has been completed */
final function OnTutorialCompletedFromKismet( EGearTutorialType TutorialType, bool bSimulatedKismetFromServer )
{
	local int TutIdx;
	local GearTutorial_Base TutorialToComplete;

	`DEBUGTUTORIAL("GearTutorialManager::OnTutorialCompletedFromKismet: Type="$TutorialType);

	// Find the tutorial to complete
	for ( TutIdx = 0; TutIdx < Tutorials.length; TutIdx++ )
	{
		TutorialToComplete = Tutorials[TutIdx];
		if ( (TutorialToComplete != None) && (TutorialToComplete.TutorialType == TutorialType) && !TutorialToComplete.bTutorialIsComplete )
		{
			`DEBUGTUTORIAL("GearTutorialManager::OnTutorialCompletedFromKismet: Type="$TutorialType@"CompletedTutorial="$TutorialToComplete);
			if (bSimulatedKismetFromServer)
			{
				TutorialToComplete.bIsKismetTutorial = false;
			}
			TutorialToComplete.OnTutorialCompleted();
			return;
		}
	}

	`DEBUGTUTORIAL("GearTutorialManager::OnTutorialCompletedFromKismet: Failed, could not find tutorial Type="$TutorialType);
}

/** Add an objective for this tutorial */
final function AddTutorialObjective( GearTutorial_Base Tutorial )
{
	if ( (Tutorial != None) && (ObjectiveMgr != None) && (Tutorial.TutorialAction != None) && (Tutorial.TutorialAction.Objective_Name != '') )
	{
		`DEBUGTUTORIAL("GearTutorialManager::AddTutorialObjective: Type="$Tutorial.TutorialType@"Tutorial="$Tutorial@"ObjName="$Tutorial.TutorialAction.Objective_Name);
		ObjectiveMgr.AddObjective( Tutorial.TutorialAction.Objective_Name, Tutorial.TutorialAction.Objective_Desc, Tutorial.TutorialAction.Objective_bNotifyPlayer );
	}
}

/** Complete an objective for this tutorial */
final function CompleteTutorialObjective( GearTutorial_Base Tutorial )
{
	local bool bNotifyPlayer;

	if ( (Tutorial != None) && (ObjectiveMgr != None) && (Tutorial.TutorialAction != None) && (Tutorial.TutorialAction.Objective_Name != '') )
	{
		`DEBUGTUTORIAL("GearTutorialManager::CompleteTutorialObjective: Type="$Tutorial.TutorialType@"Tutorial="$Tutorial@"ObjName="$Tutorial.TutorialAction.Objective_Name);

		// If the tutorial is not complete (had been stopped) silently remove the objective
		if ( !Tutorial.bTutorialIsComplete )
		{
			bNotifyPlayer = false;
		}
		// Else use the action's value for notifying the player
		else
		{
			bNotifyPlayer = Tutorial.TutorialAction.Objective_bNotifyPlayer;
		}

		ObjectiveMgr.CompleteObjective( Tutorial.TutorialAction.Objective_Name, bNotifyPlayer );
	}
}

/**
 * Adds all uncompleted auto-initiated tutorials to the system - must be called after the GearProfileSettins have been inited
 *
 * @param bAddOptionControlled - whether we are adding option controlled tutorials or non-option controlled tutorials
 */
final function AddAutoInitiatedTutorials(optional bool bAddOptionControlled = true)
{
	local int Idx;
	local array<EGearTutorialType> AutoInitList;
	local bool bAddTutorial;

	`DEBUGTUTORIAL("GearTutorialManager::AddAutoInitiatedTutorials: AutoInitType="$AutoInitType@bAutoTutorialsSuspended@bGameIsUsingAutoTutorials);

	if ( ProfileSettings != None && !bAutoTutorialsSuspended )
	{
		// Get the saved profile values
		ProfileSettings.GetTutorialValues( CompletedTutorialsSP1, CompletedTutorialsSP2, CompletedTutorialsSP3, CompletedTutorialsMP1, CompletedTutorialsMP2, CompletedTutorialsMP3, CompletedTutorialsTrain1, CompletedTutorialsTrain2, CompletedTutorialsTrain3 );
		switch (AutoInitType)
		{
			case eGAIT_SP:		AutoInitList = SPAutoInitiatedTutorialTypes;		break;
			case eGAIT_MP:		AutoInitList = MPAutoInitiatedTutorialTypes;		break;
			case eGAIT_Train1:	AutoInitList = TrainAutoInitiatedTutorialTypes_1;	break;
			case eGAIT_Train2:	AutoInitList = TrainAutoInitiatedTutorialTypes_2;	break;
			case eGAIT_Train3:	AutoInitList = TrainAutoInitiatedTutorialTypes_3;	break;
			case eGAIT_Train4:	AutoInitList = TrainAutoInitiatedTutorialTypes_4;	break;
			case eGAIT_Train5:	AutoInitList = TrainAutoInitiatedTutorialTypes_5;	break;
		}

		for ( Idx = 0; Idx < AutoInitList.length; Idx++ )
		{
			// Adding tutorials that are turned on/off through the game settings
			if (bAddOptionControlled)
			{
				if (bGameIsUsingAutoTutorials &&
					TutorialClasses[AutoInitList[Idx]].default.bOptionControlled &&
					!IsTutorialCompleted(AutoInitList[Idx]))
				{
					bAddTutorial = true;
				}
			}
			// Adding tutorials that are added by the game object every time
			else
			{
				if (!TutorialClasses[AutoInitList[Idx]].default.bOptionControlled)
				{
					bAddTutorial = true;
				}
			}

			// Add the tutorial
			if (bAddTutorial)
			{
				AddTutorial( AutoInitList[Idx] );
			}
		}
	}
}

/** Removes the auto tutorials from the system */
final function RemoveAutoInitiatedTutorials()
{
	local int Idx, ListIdx;
	local array<EGearTutorialType> AutoInitList;
	local GearTutorial_Base TutorialToRemove;

	`DEBUGTUTORIAL("GearTutorialManager::RemoveAutoInitiatedTutorials: Removing Auto Tutorials="$AutoInitType);

	switch (AutoInitType)
	{
		case eGAIT_SP:		AutoInitList = SPAutoInitiatedTutorialTypes;		break;
		case eGAIT_MP:		AutoInitList = MPAutoInitiatedTutorialTypes;		break;
		case eGAIT_Train1:	AutoInitList = TrainAutoInitiatedTutorialTypes_1;	break;
		case eGAIT_Train2:	AutoInitList = TrainAutoInitiatedTutorialTypes_2;	break;
		case eGAIT_Train3:	AutoInitList = TrainAutoInitiatedTutorialTypes_3;	break;
		case eGAIT_Train4:	AutoInitList = TrainAutoInitiatedTutorialTypes_4;	break;
		case eGAIT_Train5:	AutoInitList = TrainAutoInitiatedTutorialTypes_5;	break;
	}

	// Loop through the list of tutorials and remove all the autos
	for ( Idx = 0; Idx < Tutorials.length; Idx++ )
	{
		// Only remove option controlled tutorials
		if (TutorialClasses[Tutorials[Idx].TutorialType].default.bOptionControlled)
		{
			ListIdx = AutoInitList.Find( Tutorials[Idx].TutorialType );
			if ( ListIdx != INDEX_NONE )
			{
				TutorialToRemove = Tutorials[Idx];
				// Deactivate the active tutorial if it's an auto
				if ( TutorialToRemove == ActiveTutorial )
				{
					`DEBUGTUTORIAL("GearTutorialManager::RemoveAutoInitiatedTutorials: Deactivating the active tutorial="$ActiveTutorial);
					DeactivateTutorial();
				}

				// Remove it from the list
				`DEBUGTUTORIAL("GearTutorialManager::RemoveAutoInitiatedTutorials: Removing tutorial="$TutorialToRemove@TutorialToRemove.TutorialType);
				Tutorials.RemoveItem( TutorialToRemove );

				// Decrement the index since it was removed
				Idx--;
			}
		}
	}
}

/** Creates a tutorial instance and adds it to the list of tutorials */
final function bool AddTutorial( EGearTutorialType TutorialType, optional SeqAct_ManageTutorials Action, optional bool IsKismetDriven )
{
	local GearTutorial_Base NewTutorial;
	local int TutIdx;

	// Early out if this tutorial is already completed
	if ( IsTutorialCompleted(TutorialType) && TutorialClasses[TutorialType].default.bOptionControlled )
	{
		`DEBUGTUTORIAL("GearTutorialManager::AddTutorial: Failed, already completed: Type="$TutorialType);
		return false;
	}

	// Make sure this tutorial is not already in the system
	for ( TutIdx = 0; TutIdx < Tutorials.length; TutIdx++ )
	{
		if ( Tutorials[TutIdx].TutorialType == TutorialType )
		{
			`DEBUGTUTORIAL("GearTutorialManager::AddTutorial: Failed, already in system: Type="$TutorialType@"OldTutorial="$Tutorials[TutIdx]);
			return false;
		}
	}

	NewTutorial = new(Outer) TutorialClasses[TutorialType];
	if ( NewTutorial != None )
	{
		// Mark the tutorial not ready if the system is off
		if ( !TutorialMgr.bTutorialSystemIsActive )
		{
			NewTutorial.bTutorialIsReadyForActivation = false;
		}

		// Add the tutorial
		Tutorials.AddItem( NewTutorial );
		NewTutorial.OnTutorialAdded( self, Action, IsKismetDriven );
		`DEBUGTUTORIAL("GearTutorialManager::AddTutorial: Type="$TutorialType@"NewTutorial="$NewTutorial@"IsKismet="$IsKismetDriven);
	}
	else
	{
		`DEBUGTUTORIAL("GearTutorialManager::AddTutorial: Failed to add tutorial Type="$TutorialType);
	}

	// Add an objective if this tutorial requires one
	AddTutorialObjective( NewTutorial );

	return true;
}

/** Removes a tutorial instance from the list and deletes it */
final function bool RemoveTutorial( EGearTutorialType TutorialType )
{
	local int TutIdx;
	local GearTutorial_Base TutorialToRemove;

	// Find the tutorial to be removed
	for ( TutIdx = 0; TutIdx < Tutorials.length; TutIdx++ )
	{
		if ( Tutorials[TutIdx].TutorialType == TutorialType )
		{
			TutorialToRemove = Tutorials[TutIdx];
			break;
		}
	}

	`DEBUGTUTORIAL("GearTutorialManager::RemoveTutorial: Type="$TutorialType@"RemoveTutorial="$TutorialToRemove);

	// Found it
	if ( TutorialToRemove != None )
	{
		// Complete the objective if one exists
		CompleteTutorialObjective( TutorialToRemove );

		// Deactivate the tutorial
		if ( TutorialToRemove == ActiveTutorial )
		{
			DeactivateTutorial();
		}

		// Call for tutorial to cleanup
		TutorialToRemove.OnTutorialRemoved();

		// Remove this tutorial
		Tutorials.RemoveItem( TutorialToRemove );

		// If this is the currently active tutorial we have to setup the next tutorial
		if ( TutorialToRemove == ActiveTutorial )
		{
			SetNextActiveTutorial();
		}

		return true;
	}

	return false;
}

/** Set the tutorial to be ready for activation */
final function bool StartTutorial( EGearTutorialType TutorialType )
{
	local int TutIdx;

	for ( TutIdx = 0; TutIdx < Tutorials.length; TutIdx++ )
	{
		if ( Tutorials[TutIdx].TutorialType == TutorialType )
		{
			Tutorials[TutIdx].bTutorialIsReadyForActivation = true;
			`DEBUGTUTORIAL("GearTutorialManager::StartTutorial: Tutorial is now ready for activity: Type="$TutorialType);
			return true;
		}
	}

	`DEBUGTUTORIAL("GearTutorialManager::StartTutorial: Failed, not in the system: Type="$TutorialType);
	return false;
}

/** Stop this tutorial, but do not remove it */
final function bool StopTutorial( EGearTutorialType TutorialType )
{
	local int TutIdx;

	for ( TutIdx = 0; TutIdx < Tutorials.length; TutIdx++ )
	{
		if ( Tutorials[TutIdx].TutorialType == TutorialType )
		{
			// Deactivate the tutorial if it's the active one
			if ( Tutorials[TutIdx] == ActiveTutorial )
			{
				DeactivateTutorial();
			}

			Tutorials[TutIdx].bTutorialIsReadyForActivation = false;

			`DEBUGTUTORIAL("GearTutorialManager::StopTutorial: Tutorial is now stopped: Type="$TutorialType);
			return true;
		}
	}

	`DEBUGTUTORIAL("GearTutorialManager::StopTutorial: Failed, not in the system: Type="$TutorialType);
	return false;
}

/** Removes all tutorial from the system */
final function RemoveAllTutorials()
{
	local GearTutorial_Base TutorialToRemove;

	`DEBUGTUTORIAL("GearTutorialManager::RemoveAllTutorials");

	while ( Tutorials.length > 0 )
	{
		TutorialToRemove = Tutorials[0];
		if ( TutorialToRemove != None )
		{
			// Deactivate the active tutorial if this is it.  There is cleanup work to be done.
			if ( TutorialToRemove == ActiveTutorial )
			{
				DeactivateTutorial();
			}

			// Remove it
			Tutorials.RemoveItem( TutorialToRemove );
		}
	}
}

/**
 * Makes the tutorial passed in be the active tutorial
 *		- Deactivating the previously active tutorial in the process
 *		- @param bForceDeactivate - will deactivate the previous active tutorial even if we fail
 *				to make the new one active
 */
final function ActivateTutorial( GearTutorial_Base TutorialToActivate, bool bForceDeactivate )
{
	local bool bCanActivateTutorial;

	if ( TutorialToActivate != None )
	{
		// See if we will be able to activate the new tutorial
		if (IsPlayerReadyForTutorials() &&
			TutorialToActivate != None &&
			TutorialToActivate.bTutorialIsReadyForActivation)
		{
			bCanActivateTutorial = true;
		}
		else
		{
			bCanActivateTutorial = false;
		}

		// Make sure this isn't already the active tutorial
		if ( bCanActivateTutorial && (TutorialToActivate == ActiveTutorial) )
		{
			`DEBUGTUTORIAL("GearTutorialManager::ActivateTutorial: Failed from sanity check: Type="$TutorialToActivate.TutorialType@"TutorialToActivate="$TutorialToActivate);
			return;
		}

		// Deactivate the previously active tutorial if needed
		if ( bCanActivateTutorial || bForceDeactivate )
		{
			DeactivateTutorial();
		}

		// Activate the tutorial
		if ( bCanActivateTutorial && TutorialToActivate.TutorialActivated() )
		{
			ActiveTutorial = TutorialToActivate;

			// If the newly activated tutorial is handling input, set the new handler and push the input stack
			if ( ActiveTutorial.InputHandler != None )
			{
				CurrentTutorialInputHandler = ActiveTutorial.InputHandler;
				PushPlayerInput( CurrentTutorialInputHandler );

				`DEBUGTUTORIAL("GearTutorialManager::ActivateTutorial: Pushed PlayerInput Type="$TutorialToActivate.TutorialType@"TutorialToActivate="$TutorialToActivate@"PlayerInputObject="$TutorialToActivate.InputHandler);
			}
		}
		else
		{
			`DEBUGTUTORIAL("GearTutorialManager::ActivateTutorial: Failed tutorial's activation function Type="$TutorialToActivate.TutorialType@"TutorialToActivate="$TutorialToActivate@"CanActivateBOOL="$bCanActivateTutorial);
		}
	}
	else
	{
		`DEBUGTUTORIAL("GearTutorialManager::ActivateTutorial: Failed from sanity check: TutorialToActivate="$TutorialToActivate);
	}
}

/**
 * Deactivates the previously active tutorial
 * @Param bReadyForActivation - whether the "ready for activation" flag should remain on or not
 */
final event DeactivateTutorial( optional bool bReadyForActivation )
{
	if ( ActiveTutorial != None )
	{
		`DEBUGTUTORIAL("GearTutorialManager::DeactivateTutorial: Type="$ActiveTutorial.TutorialType@"ActiveTutorial="$ActiveTutorial);

		// If the current tutorial is handling input, pop it from the input stack
		if ( CurrentTutorialInputHandler != None )
		{
			RemoveInputHandlerFromSystem();

			`DEBUGTUTORIAL("GearTutorialManager::DeactivateTutorial: Popped PlayerInput Type="$ActiveTutorial.TutorialType@"TutorialToDeactivate="$ActiveTutorial@"PlayerInputObject="$ActiveTutorial.InputHandler);
		}

		// Set the tutorial to be deactivated
		ActiveTutorial.TutorialDeactivated( bReadyForActivation );
		ActiveTutorial = None;
	}
	else
	{
		`DEBUGTUTORIAL("GearTutorialManager::DeactivateTutorial: No active tutorial to deactivate");
	}
}

/** Whether the player controller is able to have ready tutorials or not (cinematics, dead, spectating, etc) */
final function bool IsPlayerReadyForTutorials()
{
	local GearPC PC;

	PC = Outer;
	if (PC == none ||
		PC.IsSpectating() ||
		(PC.IsDead() && (PC.MyGearPawn == none || !PC.MyGearPawn.IsDBNO())) ||
		Outer.bCinematicMode)
	{
		return false;
	}
	return true;
}

/** Find the next tutorial that should be active and make it so - assumes that the currently active tutorial should be deactivated */
final event SetNextActiveTutorial()
{
	local int TutIdx;
	local GearTutorial_Base NextTutorial;

	for ( TutIdx = 0; TutIdx < Tutorials.length; TutIdx++ )
	{
		// See if this tutorial is ready to be activated
		if ( Tutorials[TutIdx].bTutorialIsReadyForActivation && IsPlayerReadyForTutorials() )
		{
			// If we haven't found a tutorial or this one is of higher priority then choose this one
			if ( (NextTutorial == None) || (NextTutorial.TutorialPriority < Tutorials[TutIdx].TutorialPriority) )
			{
				NextTutorial = Tutorials[TutIdx];
			}
		}
	}

	if ( NextTutorial != None )
	{
		`DEBUGTUTORIAL("GearTutorialManager::SetNextActiveTutorial: Type="$NextTutorial.TutorialType@"NextTutorial="$NextTutorial);
		// Found a new tutorial to make activated - will deactivate the current one
		ActivateTutorial( NextTutorial, true );
	}
	else
	{
		`DEBUGTUTORIAL("GearTutorialManager::SetNextActiveTutorial: No Tutorial to make active");
		// Didn't find a new tutorial but we still need to deactivate the old one
		DeactivateTutorial();
	}
}

/** Called by a coop player when he has completed a tutorial so that the server can complete it and send a kismet impulse on his behalf */
final function TellServerTutorialCompletedFromClient(EGearTutorialType TutType)
{
	`DEBUGTUTORIAL("GearTutorialManager::TellServerTutorialCompletedFromClient: tell server to complete tutorial by coop player"@TutType);

	Outer.ServerClientHasCompletedTutorial(TutType);
}

/** Called by the host player when he has completed a tutorial so that the client can complete theirs on his behalf */
final function TellClientTutorialCompletedFromServer(EGearTutorialType TutType)
{
	`DEBUGTUTORIAL("GearTutorialManager::TellClientTutorialCompletedFromServer: tell client to complete tutorial by host player"@TutType);

	Outer.TellClientTutorialCompletedFromServer(TutType);
}


defaultproperties
{
	// Initialize the list of class-names which map the tutorial type to the tutorial object that will run it
	TutorialClasses={(
		class'GearTutorial_Objectives',			//GEARTUT_Objectives
		class'GearTutorial_Cover',				//GEARTUT_Cover
		class'GearTutorial_Fire',				//GEARTUT_Fire
		class'GearTutorial_Target',				//GEARTUT_Target
		class'GearTutorial_ChangeWeapon',		//GEARTUT_ChangeWeapon
		class'GearTutorial_Reload',				//GEARTUT_Reload
		class'GearTutorial_ActiveReload',		//GEARTUT_ActiveReload
		class'GearTutorial_POI',				//GEARTUT_PointOfInterest
		class'GearTutorial_Mantle1',			//GEARTUT_Mantle1
		class'GearTutorial_Mantle2',			//GEARTUT_Mantle2
		class'GearTutorial_Evade',				//GEARTUT_Evade
		class'GearTutorial_CoverSlip',			//GEARTUT_CoverSlip
		class'GearTutorial_SwatTurn',			//GEARTUT_SwatTurn
		class'GearTutorial_Use1',				//GEARTUT_Use1
		class'GearTutorial_Use2',				//GEARTUT_Use2
		class'GearTutorial_Grenades1',			//GEARTUT_Grenades1
		class'GearTutorial_Grenades2',			//GEARTUT_Grenades2
		class'GearTutorial_PlayerDamage',		//GEARTUT_PlayerDamage
		class'GearTutorial_Revive',				//GEARTUT_Revive
		class'GearTutorial_ChainsawMelee',		//GEARTUT_ChainsawMelee
		class'GearTutorial_Ladder',				//GEARTUT_Ladder
		class'GearTutorial_RoadieRun',			//GEARTUT_RoadieRun
		class'GearTutorial_BlindFire',			//GEARTUT_BlindFire
		class'GearTutorial_AimFromCover',		//GEARTUT_AimFromCover
		class'GearTutorial_Movement',			//GEARTUT_Movement
		class'GearTutorialFTE_Execution',		//GEARTUT_Executions
		class'GearTutorialFTE_MeatShield',		//GEARTUT_MeatShield
		class'GearTutorialFTE_Crawling',		//GEARTUT_Crawling
		class'GearTutorialWeap_Mortar',			//GEARTUT_WeapMortar
		class'GearTutorialWeap_Longshot',		//GEARTUT_WeapLongshot
		class'GearTutorialFTE_Shield',			//GEARTUT_Shield
		class'GearTutorialWeap_HOD',			//GEARTUT_WeapHOD
		class'GearTutorialWeap_Bow',			//GEARTUT_WeapBow
		class'GearTutorial_Centaur',			//GEARTUT_Centaur
		class'GearTutorial_CentaurCoop',		//GEARTUT_CentaurCoop
		class'GearTutorial_CentaurLights',		//GEARTUT_CentaurLights
		class'GearTutorial_Reaver',				//GEARTUT_Reaver
		class'GearTutorial_ReaverManuever',		//GEARTUT_Reaver2
		class'GearTutorial_Brumak',				//GEARTUT_Brumak
		class'GearTutorialWeap_SmokeGrenade',	//GEARTUT_WeapSmokeGrenade
		class'GearTutorialWeap_Shotgun',		//GEARTUT_WeapShotgun
		class'GearTutorialWeap_Pistol',			//GEARTUT_WeapPistol
		class'GearTutorialWeap_Minigun',		//GEARTUT_WeapMinigun
		class'GearTutorialWeap_InkGrenade',		//GEARTUT_WeapInkGrenade
		class'GearTutorialWeap_Hammerburst',	//GEARTUT_WeapHammerburst
		class'GearTutorialWeap_FragGrenade',	//GEARTUT_WeapFragGrenade
		class'GearTutorialWeap_FlameThrower',	//GEARTUT_WeapFlameThrower
		class'GearTutorialWeap_BurstPistol',	//GEARTUT_WeapBurstPistol
		class'GearTutorialWeap_Boomshot',		//GEARTUT_WeapBoomshot
		class'GearTutorialWeap_Boltok',			//GEARTUT_WeapBoltok
		class'GearTutorialWeap_AssaultRifle',	//GEARTUT_WeapAssaultRifle
		class'GearTutorial_ReloadSimple',		//GEARTUT_ReloadSimple
		class'GearTutorial_UnlimitedAmmo',		//GEARTUT_UnlimitedAmmo
		class'GearTutorial_Sneak',				//GEARTUT_Sneak
		class'GearTutorial_ObjectivesReminder',	//GEARTUT_ObjectivesReminder
		class'GearTutorialWeap_Turret',			//GEARTUT_WeapTurret
		class'GearTutorialTrain_WarWelcome',	//GEARTUT_TRAIN_WarWelcome
		class'GearTutorialTrain_SudDeath',		//GEARTUT_TRAIN_SudDeath
		class'GearTutorialTrain_Revive',		//GEARTUT_TRAIN_Revive
		class'GearTutorialTrain_Crawl',			//GEARTUT_TRAIN_Crawl
		class'GearTutorialTrain_GrenCrawl',		//GEARTUT_TRAIN_GrenCrawl
		class'GearTutorialTrain_ExeWelcome',	//GEARTUT_TRAIN_ExeWelcome
		class'GearTutorialTrain_ExeRule',		//GEARTUT_TRAIN_ExeRule
		class'GearTutorialTrain_Exe',			//GEARTUT_TRAIN_Exe
		class'GearTutorialTrain_GuarWelcome',	//GEARTUT_TRAIN_GuarWelcome
		class'GearTutorialTrain_KSpawn',		//GEARTUT_TRAIN_KSpawn
		class'GearTutorialTrain_DSpawn',		//GEARTUT_TRAIN_DSpawn
		class'GearTutorialTrain_ExeLead',		//GEARTUT_TRAIN_ExeLead
		class'GearTutorialTrain_UDed',			//GEARTUT_TRAIN_UDed
		class'GearTutorialTrain_LeDed',			//GEARTUT_TRAIN_LeDed
		class'GearTutorialTrain_Assas',			//GEARTUT_TRAIN_Assas
		class'GearTutorialTrain_HiScor',		//GEARTUT_TRAIN_HiScor
		class'GearTutorialTrain_AnxWelcome',	//GEARTUT_TRAIN_AnxWelcome
		class'GearTutorialTrain_NmyCap',		//GEARTUT_TRAIN_NmyCap
		class'GearTutorialTrain_TeamCap',		//GEARTUT_TRAIN_TeamCap
		class'GearTutorialTrain_Defend',		//GEARTUT_TRAIN_Defend
		class'GearTutorialTrain_DrainRng',		//GEARTUT_TRAIN_DrainRng
		class'GearTutorialTrain_WinRnd',		//GEARTUT_TRAIN_WinRnd
		class'GearTutorialTrain_LoseRnd',		//GEARTUT_TRAIN_LoseRnd
		class'GearTutorialTrain_MeatWelcome',	//GEARTUT_TRAIN_MeatWelcome
		class'GearTutorialTrain_MeatPick',		//GEARTUT_TRAIN_MeatPick
		class'GearTutorialTrain_MeatRng',		//GEARTUT_TRAIN_MeatRng
		class'GearTutorialTrain_MeatNmy',		//GEARTUT_TRAIN_MeatNmy
		class'GearTutorialTrain_MeatScore',		//GEARTUT_TRAIN_MeatScore
		)}
}
