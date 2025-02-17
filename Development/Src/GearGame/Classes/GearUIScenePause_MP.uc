/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUIScenePause_MP extends GearUIScenePause_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Label for the game/mapname */
var UILabel GameAndMapLabel;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Initializes the widget references for the UI scene */
function InitializeWidgetReferences()
{
	Super.InitializeWidgetReferences();

	GameAndMapLabel = UILabel(FindChild('lblTitle', true));
}

/** Called from the overloaded Tick() function so we can update the labels */
event UpdateSceneLabels(float DeltaTime)
{
	UpdateDescription();
}

/** Sets the labels for displaying the game description widgets */
function UpdateDescription()
{
	local string LabelString;
	local GearGRI MyGRI;
	local WorldInfo WI;

	WI = GetWorldInfo();
	if ( WI != None )
	{
		if ( WI.Pauser != None )
		{
			LabelString = "<Strings:GearGame.Generic.Paused>";
		}
		else
		{
			MyGRI = GearGRI(WI.GRI);

			// Update the gamemode/mapname label
			if ( MyGRI != None )
			{
				LabelString = GetGameModeString(GetMPGameType()) @ GameToMapConjunction @ GetMapNameString(MyGRI.GetURLMap());
			}
		}

		GameAndMapLabel.SetDataStoreBinding( LabelString );
	}
}

defaultproperties
{
	bIsCampaignScene=false
	OnGearUISceneTick=UpdateSceneLabels
	LoopingAudioSoundResource=SoundCue'Interface_Audio.Menu.MenuPauseLoopCue'
	bMenuLevelRestoresScene=true
	bAllowPlayerJoin=false
}
