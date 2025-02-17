/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFEOptions_Versus extends GearUISceneFEOptions_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Scene to open for weapon swap */
var	transient GearUISceneFE_MPWeaponSwap WeaponSwapScene;

/** Reference to the selected COG image */
var transient UIImage SelectedCOGImage;

/** Reference to the selected COG image */
var transient UIImage SelectedLocustImage;

/** Reference to the selected COG image */
var transient UIImage SelectedWeaponImage;

/** List of all the COG characters to choose from */
var transient array<UIResourceDataProvider> COGList;

/** List of all the Locust characters to choose from */
var transient array<UIResourceDataProvider> LocustList;

/** List of all the default weapons to choose from */
var transient array<UIResourceDataProvider> WeaponList;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

// Initialize references to widgets
function InitializeWidgetReferences()
{
	local GearUIDataStore_GameResource GameResourceDS;

	Super.InitializeWidgetReferences();

	SelectedCOGImage = UIImage(FindChild('imgPrefCOG', true));
	SelectedLocustImage = UIImage(FindChild('imgPrefLocust', true));
	SelectedWeaponImage = UIImage(FindChild('imgPrefWeapon', true));

	// Retrieve the character lists
	GameResourceDS = GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		GameResourceDS.GetResourceProviders('COGs', COGList);
		GameResourceDS.GetResourceProviders('Locusts', LocustList);
		GameResourceDS.GetResourceProviders('Weapons', WeaponList);
	}
}

/** Assigns delegates in important child widgets to functions in this scene class */
function SetupCallbacks()
{
	Super.SetupCallbacks();

	if ( SettingsButtonBar != None )
	{
		SettingsButtonBar.SetButtonCallback( 'WeaponSwap', OnWeaponSwapClicked );
	}
}

/** Open the Weapon Swap screen */
function bool OnWeaponSwapClicked(UIScreenObject EventObject, int PlayerIndex)
{
	OpenScene(WeaponSwapScene, GetPlayerOwner());
	return true;
}

/** Return the COGCharacterSummary object via ProfileId */
final function COGCharacterSummary GetCOGCharacter( int ProfileId )
{
	local int Idx;
	local COGCharacterSummary COGCharacter;

	for ( Idx = 0; Idx < COGList.length; Idx++ )
	{
		COGCharacter = COGCharacterSummary(COGList[Idx]);
		if ( COGCharacter.ProfileId == ProfileId )
		{
			return COGCharacter;
		}
	}

	return None;
}

/** Return the LocustCharacterSummary object via ProfileId */
final function LocustCharacterSummary GetLocustCharacter( int ProfileId )
{
	local int Idx;
	local LocustCharacterSummary LocustCharacter;

	for ( Idx = 0; Idx < LocustList.length; Idx++ )
	{
		LocustCharacter = LocustCharacterSummary(LocustList[Idx]);
		if ( LocustCharacter.ProfileId == ProfileId )
		{
			return LocustCharacter;
		}
	}

	return None;
}

/** Update the widget styles, strings, etc. */
function UpdateWidgetData(float DeltaTime)
{
	local int SettingsIdx, ListIdx;
	local COGCharacterSummary COGCharacter;
	local LocustCharacterSummary LocustCharacter;
	local GearGameWeaponSummary WeaponInfo;
	local UIOptionList OptionList;
	local Surface Portrait;

	// Set the COG image
	if ( SelectedCOGImage != None && SelectedCOGImage.ImageComponent != None )
	{
		SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.CogMPCharacter );
		if ( SettingsIdx != -1 )
		{
			ListIdx = UIOptionList(SettingsData[SettingsIdx].SettingWidget).GetCurrentIndex();
			COGCharacter = GetCOGCharacter( ListIdx );
			Portrait = COGCharacter.GetPortraitImage();
			if ( COGCharacter != None && Portrait != None )
			{
				SelectedCOGImage.ImageComponent.SetImage( Portrait );
				SelectedCOGImage.ImageComponent.SetCoordinates( COGCharacter.PortraitIcon.Coordinates );
				SelectedCOGImage.SetWidgetStyleByName( 'Image Style', 'Default Image Style' );
			}
			else
			{
				SelectedCOGImage.SetWidgetStyleByName( 'Image Style', 'imgBlank' );
			}
		}
	}

	// Set the Locust image
	if ( SelectedLocustImage != None && SelectedLocustImage.ImageComponent != None )
	{
		SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.LocustMPCharacter );
		if ( SettingsIdx != -1 )
		{
			ListIdx = UIOptionList(SettingsData[SettingsIdx].SettingWidget).GetCurrentIndex();
			LocustCharacter = GetLocustCharacter( ListIdx );
			Portrait = LocustCharacter.GetPortraitImage();
			if ( LocustCharacter != None && Portrait != None )
			{
				SelectedLocustImage.ImageComponent.SetImage( Portrait );
				SelectedLocustImage.ImageComponent.SetCoordinates( LocustCharacter.PortraitIcon.Coordinates );
				SelectedLocustImage.SetWidgetStyleByName( 'Image Style', 'Default Image Style' );
			}
			else
			{
				SelectedLocustImage.SetWidgetStyleByName( 'Image Style', 'imgBlank' );
			}
		}
	}

	// Set the weapon style
	if ( SelectedWeaponImage != None )
	{
		SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.DefaultMPWeapon );
		if ( SettingsIdx != -1 )
		{
			OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
			ListIdx = OptionList.GetCurrentIndex();
			WeaponInfo = GearGameWeaponSummary(WeaponList[ListIdx]);
			Portrait = WeaponInfo.GetWeaponIcon();
			if ( WeaponInfo != None && Portrait != None )
			{
				SelectedWeaponImage.ImageComponent.SetImage( Portrait );
				//SelectedWeaponImage.ImageComponent.SetCoordinates( WeaponInfo.WeaponIcon.Coordinates );
				SelectedWeaponImage.SetWidgetStyleByName( 'Image Style', 'Default Image Style' );
			}
			else
			{
				SelectedWeaponImage.SetWidgetStyleByName( 'Image Style', 'imgBlank' );
			}
		}
	}
}

/** Initializes the optionlists that use raw values in the profile */
function InitializeOptionLists()
{
	local int Value;

	MyProfile.GetProfileSettingValueInt( MyProfile.const.DefaultMPWeapon, Value );
	InitializeWeaponListFromWeaponId( Value );

	MyProfile.GetProfileSettingValueInt( MyProfile.const.CogMPCharacter, Value );
	InitializeCOGListFromCharacterId( Value );

	MyProfile.GetProfileSettingValueInt( MyProfile.const.LocustMPCharacter, Value );
	InitializeLocustListFromCharacterId( Value );
}

/** Initializes the COG list based off a character id */
function InitializeCOGListFromCharacterId( int CharacterId )
{
	local int SettingsIdx, ListIdx;
	local COGCharacterSummary CharInfo;
	local UIOptionList OptionList;

	ListIdx = ConvertCharacterIdToProviderId( CharacterId, 'COGs' );
	SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.CogMPCharacter );

	if ( SettingsIdx != -1 && ListIdx != -1 )
	{
		// Get the current character info
		CharInfo = COGCharacterSummary(COGList[ListIdx]);
		// Get the character option list
		OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
		// Set the Character option list to the current character
		if ( CharInfo != None )
		{
			OptionList.SetCurrentIndex( ListIdx );
		}

		// Set the IsValid callback so we can skip non-default weapons
		OptionList.OnIsCurrValueValid = OnIsCurrValueValid_COG;
	}
}

/** Initializes the Locust list based off a character id */
function InitializeLocustListFromCharacterId( int CharacterId )
{
	local int SettingsIdx, ListIdx;
	local LocustCharacterSummary CharInfo;
	local UIOptionList OptionList;

	ListIdx = ConvertCharacterIdToProviderId( CharacterId, 'Locusts' );
	SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.LocustMPCharacter );

	if ( SettingsIdx != -1 && ListIdx != -1 )
	{
		// Get the current character info
		CharInfo = LocustCharacterSummary(LocustList[ListIdx]);
		// Get the character option list
		OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
		// Set the Character option list to the current character
		if ( CharInfo != None )
		{
			OptionList.SetCurrentIndex( ListIdx );
		}

		// Set the IsValid callback so we can skip non-default weapons
		OptionList.OnIsCurrValueValid = OnIsCurrValueValid_Locust;
	}
}

/** Initializes the weapon list based off a weapon id */
function InitializeWeaponListFromWeaponId( int WeaponId )
{
	local int SettingsIdx, ListIdx;
	local GearGameWeaponSummary WeaponInfo;
	local UIOptionList OptionList;

	ListIdx = ConvertWeaponIdToProviderId( WeaponId );
	SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.DefaultMPWeapon );

	if ( SettingsIdx != -1 && ListIdx != -1 )
	{
		// Get the current weapon info
		WeaponInfo = GearGameWeaponSummary(WeaponList[ListIdx]);
		// Get the weapon option list
		OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
		// Set the weapon option list to the current weapon
		if ( WeaponInfo != None )
		{
			OptionList.SetCurrentIndex( ListIdx );
		}

		// Set the IsValid callback so we can skip non-default weapons
		OptionList.OnIsCurrValueValid = OnIsCurrValueValid_Weapon;
	}
}

/** Perform any actions that need to take place before the determining if the profile should be saved or not */
function PreProfileSaveCheck( int PlayerIndex )
{
	local int SettingsIdx, ListIdx;
	local GearGameWeaponSummary WeaponInfo;
	local COGCharacterSummary COGInfo;
	local LocustCharacterSummary LocustInfo;
	local UIOptionList OptionList;

	// Check weapon
	SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.DefaultMPWeapon );
	if ( SettingsIdx != -1 )
	{
		OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
		ListIdx = OptionList.GetCurrentIndex();
		WeaponInfo = GearGameWeaponSummary(WeaponList[ListIdx]);
		if ( WeaponInfo != None )
		{
			MyProfile.SetProfileSettingValueInt( MyProfile.const.DefaultMPWeapon, WeaponInfo.WeaponId );
		}
	}

	// Check COG
	SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.CogMPCharacter );
	if ( SettingsIdx != -1 )
	{
		OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
		ListIdx = OptionList.GetCurrentIndex();
		COGInfo = COGCharacterSummary(COGList[ListIdx]);
		if ( COGInfo != None )
		{
			MyProfile.SetProfileSettingValueInt( MyProfile.const.COGMPCharacter, COGInfo.ProfileId );
		}
	}

	// Check Locust
	SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.LocustMPCharacter );
	if ( SettingsIdx != -1 )
	{
		OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
		ListIdx = OptionList.GetCurrentIndex();
		LocustInfo = LocustCharacterSummary(LocustList[ListIdx]);
		if ( LocustInfo != None )
		{
			MyProfile.SetProfileSettingValueInt( MyProfile.const.LocustMPCharacter, LocustInfo.ProfileId );
		}
	}
}

/** Delegate that can be used to determine if IsCurrValueValid() should succeed or not */
function bool OnIsCurrValueValid_COG()
{
	local int SettingsIdx, ListIdx;
	local COGCharacterSummary CharacterInfo;
	local UIOptionList OptionList;

	SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.CogMPCharacter );
	if ( SettingsIdx != -1 )
	{
		OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
		ListIdx = OptionList.GetCurrentIndex();
		CharacterInfo = COGCharacterSummary(COGList[ListIdx]);
		if ( CharacterInfo == None ||
			 CharacterInfo.UnlockableValue != eUNLOCK_Character_None && !MyProfile.HasUnlockableBeenUnlocked(CharacterInfo.UnlockableValue) )
		{
			return false;
		}
	}

	return true;
}

/** Delegate that can be used to determine if IsCurrValueValid() should succeed or not */
function bool OnIsCurrValueValid_Locust()
{
	local int SettingsIdx, ListIdx;
	local LocustCharacterSummary CharacterInfo;
	local UIOptionList OptionList;

	SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.LocustMPCharacter );
	if ( SettingsIdx != -1 )
	{
		OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
		ListIdx = OptionList.GetCurrentIndex();
		CharacterInfo = LocustCharacterSummary(LocustList[ListIdx]);
		if ( CharacterInfo == None ||
			 CharacterInfo.UnlockableValue != eUNLOCK_Character_None && !MyProfile.HasUnlockableBeenUnlocked(CharacterInfo.UnlockableValue) )
		{
			return false;
		}
	}

	return true;
}

/** Delegate that can be used to determine if IsCurrValueValid() should succeed or not */
function bool OnIsCurrValueValid_Weapon()
{
	local int SettingsIdx, ListIdx;
	local GearGameWeaponSummary WeaponInfo;
	local UIOptionList OptionList;

	SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.DefaultMPWeapon );
	if ( SettingsIdx != -1 )
	{
		OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
		ListIdx = OptionList.GetCurrentIndex();
		WeaponInfo = GearGameWeaponSummary(WeaponList[ListIdx]);
		if ( WeaponInfo != None )
		{
			return WeaponInfo.bIsDefaultWeapon;
		}
	}

	return true;
}

/** Overloaded to handle weapons */
function SetAllSettingsToDefault( int PlayerIndex )
{
	local int Value;

	Super.SetAllSettingsToDefault(PlayerIndex);

	MyProfile.GetProfileSettingDefaultInt( MyProfile.const.DefaultMPWeapon, Value );
	InitializeWeaponListFromWeaponId( Value );

	MyProfile.GetProfileSettingDefaultInt( MyProfile.const.CogMPCharacter, Value );
	InitializeCOGListFromCharacterId( Value );

	MyProfile.GetProfileSettingDefaultInt( MyProfile.const.LocustMPCharacter, Value );
	InitializeLocustListFromCharacterId( Value );
}

/** Overloaded to handle weapons */
function bool SettingChangesHaveBeenMade()
{
	local bool bResult;
	local int SettingsIdx, ListIdx, ProfileValue;
	local GearGameWeaponSummary WeaponInfo;
	local COGCharacterSummary COGInfo;
	local LocustCharacterSummary LocustInfo;
	local UIOptionList OptionList;

	bResult = Super.SettingChangesHaveBeenMade();
	// If nothing has changed, do a check on the weapon since it needs done manually
	if ( !bResult )
	{
		SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.DefaultMPWeapon );
		if ( SettingsIdx != -1 )
		{
			OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
			ListIdx = OptionList.GetCurrentIndex();
			WeaponInfo = GearGameWeaponSummary(WeaponList[ListIdx]);
			if ( MyProfile.GetProfileSettingValueInt( MyProfile.const.DefaultMPWeapon, ProfileValue ) )
			{
				bResult = (ProfileValue != WeaponInfo.WeaponId);
			}
		}
	}

	// If nothing has changed, do a check on the COG character since it needs done manually
	if ( !bResult )
	{
		SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.CogMPCharacter );
		if ( SettingsIdx != -1 )
		{
			OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
			ListIdx = OptionList.GetCurrentIndex();
			COGInfo = COGCharacterSummary(COGList[ListIdx]);
			if ( MyProfile.GetProfileSettingValueInt( MyProfile.const.CogMPCharacter, ProfileValue ) )
			{
				bResult = (ProfileValue != COGInfo.ProfileId);
			}
		}
	}

	// If nothing has changed, do a check on the Locust character since it needs done manually
	if ( !bResult )
	{
		SettingsIdx = GetSettingIndexUsingProfileId( MyProfile.const.LocustMPCharacter );
		if ( SettingsIdx != -1 )
		{
			OptionList = UIOptionList(SettingsData[SettingsIdx].SettingWidget);
			ListIdx = OptionList.GetCurrentIndex();
			LocustInfo = LocustCharacterSummary(LocustList[ListIdx]);
			if ( MyProfile.GetProfileSettingValueInt( MyProfile.const.LocustMPCharacter, ProfileValue ) )
			{
				bResult = (ProfileValue != LocustInfo.ProfileId);
			}
		}
	}

	return bResult;
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
	if ( bInitialActivation )
	{
		InitializeOptionLists();
	}
}

defaultproperties
{
	OnGearUISceneTick=UpdateWidgetData
	OnSceneActivated=SceneActivationComplete

	SettingsData(0)=(ProfileSettingId=CogMPCharacter,WidgetName="olistCog",bUsesRawValue=true)
	SettingsData(1)=(ProfileSettingId=LocustMPCharacter,WidgetName="olistLocust",bUsesRawValue=true)
	SettingsData(2)=(ProfileSettingId=DefaultMPWeapon,WidgetName="olistWeapon",bUsesRawValue=true)
	SettingsData(3)=(ProfileSettingId=PREFERRED_SPLIT_TYPE,WidgetName="olistSSType",bIsGlobalValue=true)

	WeaponSwapScene=GearUISceneFE_MPWeaponSwap'UI_Scenes_Common.UI_FE_WeaponSwap'
}

