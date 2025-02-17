/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFE_MPWeaponSwap extends GearUISceneFEOptions_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Reference to the selected widget image */
var transient UIImage SelectedWeapImage;

/** Reference to the widget image that show what the current setting is set to */
var transient UIImage SwappedWeapImage;

/** Reference to the widget image that contains the "NO" symbol for "disabled" */
var transient UIImage DisabledImage;

/** Names of all the weapon styles */
var array<name> WeaponStyleNames;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

// Initialize references to widgets
function InitializeWidgetReferences()
{
	Super.InitializeWidgetReferences();

	SelectedWeapImage = UIImage(FindChild('imgSelectedWeaponIcon', true));
	SwappedWeapImage = UIImage(FindChild('imgSwappedWeaponIcon', true));
	DisabledImage = UIImage(FindChild('imgDisabled', true));
}

/** Assigns delegates in important child widgets to functions in this scene class */
function SetupCallbacks()
{
	Super.SetupCallbacks();

	if ( SettingsButtonBar != None )
	{
		SettingsButtonBar.SetButtonCallback( 'DisableAll', OnSetAllDisabledClicked );
	}
}

/** Sets the settings to disabled */
function bool OnSetAllDisabledClicked(UIScreenObject EventObject, int PlayerIndex)
{
	//TODO: ARE YOU SURE? MESSAGE GOES HERE
	SetAllWeaponsToDisable();

	return true;
}

/** Disables all of the weapons by setting all of the settings to the disable value */
final function SetAllWeaponsToDisable()
{
	local int Idx;

	for ( Idx = 0; Idx < SettingsData.length; Idx ++ )
	{
		UIOptionList(SettingsData[Idx].SettingWidget).SetCurrentIndex( eGEARWEAP_Disabled );
	}
}

/** Update the widget styles, strings, etc. */
function UpdateWidgetData(float DeltaTime)
{
	local int FocusIdx, Value, DefaultId, DefaultIndex, PlayerIndex;

	// Find which weapon is currently selected and set the images accordingly
	FocusIdx = GetSettingIndexOfFocusedWidget();
	if ( (FocusIdx != -1) && (MyProfile != None) && MyProfile.GetProfileSettingDefaultId(SettingsData[FocusIdx].ProfileSettingId, DefaultId, DefaultIndex) )
	{
		PlayerIndex = GetPlayerOwnerIndex();
		Value = UIOptionList(SettingsData[FocusIdx].SettingWidget).GetCurrentIndex();

		// Weapon has been disabled or unchanged
		if ( (Value == eGEARWEAP_Disabled) || (Value == DefaultIndex) )
		{
			// Set the selected widget (picture on the left side)
			SelectedWeapImage.SetWidgetStyleByName( 'Image Style', WeaponStyleNames[DefaultIndex] );
			SelectedWeapImage.EnableWidget(PlayerIndex);

			// Set the swapped widget (picture on right side and possibly the disabled image)
			SwappedWeapImage.SetWidgetStyleByName( 'Image Style', WeaponStyleNames[DefaultIndex] );
			SwappedWeapImage.DisableWidget(PlayerIndex);
			DisabledImage.SetVisibility( Value == eGEARWEAP_Disabled );
		}
		// Weapon has been changed
		else
		{
			// Set the selected widget (picture on the left side)
			SelectedWeapImage.SetWidgetStyleByName( 'Image Style', WeaponStyleNames[DefaultIndex] );
			SelectedWeapImage.DisableWidget(PlayerIndex);

			// Set the swapped widget (picture on right side and possibly the disabled image)
			SwappedWeapImage.SetWidgetStyleByName( 'Image Style', WeaponStyleNames[Value] );
			SwappedWeapImage.EnableWidget(PlayerIndex);
			DisabledImage.SetVisibility( false );
		}
	}
	else
	{
		SelectedWeapImage.SetWidgetStyleByName( 'Image Style', WeaponStyleNames[eGEARWEAP_Lancer] );
		SwappedWeapImage.SetWidgetStyleByName( 'Image Style', WeaponStyleNames[eGEARWEAP_Lancer] );
		DisabledImage.SetVisibility( false );
	}
}

/** Called when the scene is closed so we can stop the music */
function OnSceneDeactivatedCallback( UIScene DeactivatedScene )
{
}

defaultproperties
{
	OnGearUISceneTick=UpdateWidgetData

	SettingsData(0)=(ProfileSettingId=WeapSwap_FragGrenade,WidgetName="olistFragGrenade")
	SettingsData(1)=(ProfileSettingId=WeapSwap_InkGrenade,WidgetName="olistInkGrenade")
	SettingsData(2)=(ProfileSettingId=WeapSwap_Boomshot,WidgetName="olistBoomshot")
	SettingsData(3)=(ProfileSettingId=WeapSwap_Flame,WidgetName="olistScorcher")
	SettingsData(4)=(ProfileSettingId=WeapSwap_Sniper,WidgetName="olistSniper")
	SettingsData(5)=(ProfileSettingId=WeapSwap_Bow,WidgetName="olistTorqueBow")
	SettingsData(6)=(ProfileSettingId=WeapSwap_Mulcher,WidgetName="olistMulcher")
	SettingsData(7)=(ProfileSettingId=WeapSwap_Mortar,WidgetName="olistMortar")
	SettingsData(8)=(ProfileSettingId=WeapSwap_HOD,WidgetName="olistHammerofDawn")
	SettingsData(9)=(ProfileSettingId=WeapSwap_Gorgon,WidgetName="olistGorgon")
	SettingsData(10)=(ProfileSettingId=WeapSwap_Boltok,WidgetName="olistBoltok")
	SettingsData(11)=(ProfileSettingId=WeapSwap_Shield,WidgetName="olistShield")

	WeaponStyleNames(eGEARWEAP_FragGrenade)="FragGrenade"
	WeaponStyleNames(eGEARWEAP_InkGrenade)="InkGrenade"
	WeaponStyleNames(eGEARWEAP_SmokeGrenade)="SmokeGrenades"
	WeaponStyleNames(eGEARWEAP_Boomshot)="Boomshot"
	WeaponStyleNames(eGEARWEAP_Flame)="Scorcher"
	WeaponStyleNames(eGEARWEAP_Sniper)="SniperRifle"
	WeaponStyleNames(eGEARWEAP_Bow)="TorqueBow"
	WeaponStyleNames(eGEARWEAP_Mulcher)="Mulcher"
	WeaponStyleNames(eGEARWEAP_Mortar)="Mortar"
	WeaponStyleNames(eGEARWEAP_HOD)="HammerofDawn"
	WeaponStyleNames(eGEARWEAP_Gorgon)="Gorgon"
	WeaponStyleNames(eGEARWEAP_Boltok)="Boltok"
	WeaponStyleNames(eGEARWEAP_Pistol)="COGPistol"
	WeaponStyleNames(eGEARWEAP_Lancer)="Lancer"
	WeaponStyleNames(eGEARWEAP_Hammerburst)="Hammerburst"
	WeaponStyleNames(eGEARWEAP_Shotgun)="Shotgun"
	WeaponStyleNames(eGEARWEAP_Shield)="BoomShield"
}
