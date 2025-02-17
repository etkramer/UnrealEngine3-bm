/**
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUIScene_HordeMod extends GearUISceneMP_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Label for the damage multiplier */
var transient UILabel DamageValueLabel;

/** Label for the Health multiplier */
var transient UILabel HealthValueLabel;

/** Label for the accuracy multiplier */
var transient UILabel AccuracyValueLabel;

/** defines the animation for fading in the labels */
var	const UIAnimationSeq FadeInSequence;

/** The prefix that displays before the multiplier */
var localized string ModPrefix;

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
	local float Multiplier;

	DamageValueLabel = UILabel(FindChild('lblDamageValue', true));
	Multiplier = GetHordeMulitplier("GearGame.GearRules_COGDmgTakenMult");
	DamageValueLabel.SetDataStoreBinding(GetFormattedMuliplierString(Multiplier));

	HealthValueLabel = UILabel(FindChild('lblHealthValue', true));
	Multiplier = GetHordeMulitplier("GearGame.GearMut_ExtraLocustHealth");
	HealthValueLabel.SetDataStoreBinding(GetFormattedMuliplierString(Multiplier));

	AccuracyValueLabel = UILabel(FindChild('lblEnemiesValue', true));
	Multiplier = GetHordeMulitplier("GearGame.GearMut_LocustAccuracyMultiplier");
	AccuracyValueLabel.SetDataStoreBinding(GetFormattedMuliplierString(Multiplier));

	PlayUIAnimation('', FadeInSequence);
}

/** Returns the multiplier for a specific modifier to horde */
final function float GetHordeMulitplier(string MutatorPath)
{
	local GearGRI GRI;
	local array<ExtendedWaveInfo> WaveInfo;
	local int Idx;
	local int MutIdx;

	GRI = GetGRI();
	if (GRI != none)
	{
		WaveInfo = class'GearGameHorde_Base'.default.ExtendedWaveList;
		// Iterate in reverse order through the extensions for the maximum value
		for (Idx = GRI.ExtendedRestartCount-1; Idx >= 0; Idx--)
		{
			// Iterate through the mutator list for this extension
			for (MutIdx = 0; MutIdx < WaveInfo[Idx].ExtendedMutators.length; MutIdx++)
			{
				// First try the Mutator
				if (WaveInfo[Idx].ExtendedMutators[MutIdx].MutClassPath ~= MutatorPath)
				{
					return WaveInfo[Idx].ExtendedMutators[MutIdx].Multiplier;
				}
				// Then try the Rule
				else if (WaveInfo[Idx].ExtendedMutators[MutIdx].RulesClassPath ~= MutatorPath)
				{
					return WaveInfo[Idx].ExtendedMutators[MutIdx].Multiplier;
				}
			}
		}
	}
	// Found nothing so return 1
	return 1.0f;
}

/** Returns the formatted version of the multiplier */
final function string GetFormattedMuliplierString(float Multiplier)
{
	local string StringToDraw;
	local float Remainder;

	StringToDraw = ModPrefix;
	StringToDraw $= int(Multiplier);
	StringToDraw $= ".";
	Remainder = Multiplier - int(Multiplier);
	Remainder *= 10;
	StringToDraw $= int(Remainder);

	return StringToDraw;
}


defaultproperties
{
	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object

	/** For the first time the scene is open. */
	Begin Object Class=UIAnimationSeq Name=FadeTextIn
		SeqName=FadeInText
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.4f,Data=(DestAsFloat=1.0))))
	End Object
	FadeInSequence=FadeTextIn

	SceneStackPriority=GEAR_SCENE_PRIORITY_NORMAL
}