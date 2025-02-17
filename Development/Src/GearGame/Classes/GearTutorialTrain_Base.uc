/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
/**
 * GearTutorialTrain_Base
 *
 * Base class for tutorials that will be triggered in the trainging grounds feature
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorialTrain_Base extends GearTutorial_GenericPress_Base
	config(Game);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

var config string AnyaCuePath;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Overloaded to play the anya sound */
function OnTutorialSceneOpened()
{
	Super.OnTutorialSceneOpened();

	// Play the anya sound
	TutorialMgr.StartTutorialSound(AnyaCuePath);
}

/** Called when the tutorial's scene is finished closing */
function OnTutorialSceneClosed( UIScene DeactivatedScene )
{
	Super.OnTutorialSceneClosed( DeactivatedScene );

	// Tutorial is done after the scene closes
	OnTutorialCompleted();
}

defaultproperties
{
	TutorialPriority=TUTORIAL_PRIORITY_HIGH
	bSaveTutorialToProfile=false
	GameButtonType=GB_A
}
