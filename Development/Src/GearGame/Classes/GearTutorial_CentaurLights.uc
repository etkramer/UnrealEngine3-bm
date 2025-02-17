/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearTutorial_CentaurLights extends GearTutorial_Base
	config(Game);


/** Called when the tutorial's scene is finished closing */
function OnTutorialSceneClosed( UIScene DeactivatedScene )
{
	Super.OnTutorialSceneClosed( DeactivatedScene );

	// Tutorial is done after the scene closes
	OnTutorialCompleted();
}

defaultproperties
{
	TutorialType=GEARTUT_CentaurLights
}
