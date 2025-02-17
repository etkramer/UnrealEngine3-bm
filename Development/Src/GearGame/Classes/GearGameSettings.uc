/** 
 * 
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameSettings extends Object
	config(Game)
	native;

struct native SettingInfo
{
	/** the name of the widget that this setting should be associated with. */
	var name	WidgetName;

	/** Description used in the UI for this setting */
	var string Desc;
};

struct native SettingInfo_Bool extends SettingInfo
{
	/** String descriptions associated with the false(0)/true(1) values */
	var string ValuesDesc[2];
	/** Value of this setting */
	var bool bValue;

	structdefaultproperties
	{
		ValuesDesc[0]="False"
		ValuesDesc[1]="True"
	}
};

struct native SettingInfo_Float extends SettingInfo
{
	/** Min/max value for the slider */
	var float MinValue, MaxValue;
	/** Actual value of this setting */
	var float Value;

	structdefaultproperties
	{
		MinValue=0.f
		MaxValue=1.f
	}
};

struct native SettingInfo_Int extends SettingInfo
{
	/** List of potential values */
	var array<int> PotentialValues;
	/** String descriptions associated with PotentialValues */
	var array<string> ValuesDesc;
	/** Current value of this setting */
	var int Value;
};

/** Game configuration */
var config SettingInfo_Int Gore;
var config SettingInfo_Bool LanguageFilter;

/** Video configuration */
var config SettingInfo_Int DisplayDevice;
var config SettingInfo_Bool ClosedCaptioning;
var config SettingInfo_Float Brightness;
var config SettingInfo_Int Splitscreen;

/** Audio configuration */
var config SettingInfo_Float MusicVolume;
var config SettingInfo_Float EffectsVolume;

/** Look Inversion - Temporary */
var config SettingInfo_Bool InvertLook;

cpptext
{
	UBOOL GetWidgetIntSetting( class UUIObject* Widget, FSettingInfo_Int** IntSetting );
	UBOOL GetWidgetBoolSetting( class UUIObject* Widget, FSettingInfo_Bool** BoolSetting );
	UBOOL GetWidgetFloatSetting( class UUIObject* Widget, FSettingInfo_Float** FloatSetting );

	void LoadSceneValues( class UUIScene* Scene );
	void SaveSceneValues( class UUIScene* Scene );

	void LoadIntValue( FSettingInfo_Int& Setting, class UUILabelButton* LabelButtonWidget );
	void SaveIntValue( FSettingInfo_Int& Setting, class UUILabelButton* LabelButtonWidget );

	void LoadFloatValue( FSettingInfo_Float& Setting, class UUISlider* SliderWidget );
	void SaveFloatValue( FSettingInfo_Float& Setting, class UUISlider* SliderWidget );

	void LoadBoolValue( FSettingInfo_Bool& Setting, class UUILabelButton* LabelButtonWidget );
	void SaveBoolValue( FSettingInfo_Bool& Setting, class UUILabelButton* LabelButtonWidget );

	void LoadWidgetValue( class UUIObject* Widget );
	UBOOL SaveWidgetValue( class UUIObject* Widget );

	void GetNextValue( class UUIObject* Widget );
	void GetPrevValue( class UUIObject* Widget );

	/** @return Whether or not the specified widget can navigate right. */
	UBOOL HasNextValue( class UUIObject* Widget );

	/** @return Whether or not the specified widget can navigate left. */
	UBOOL HasPrevValue( class UUIObject* Widget );
}

/** Function to save the invert look to the game settings */
native static function SaveInvertLook( bool bLook );
