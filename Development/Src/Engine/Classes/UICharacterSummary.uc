/**
 * Provides information about the static resources associated with a single character class.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class UICharacterSummary extends UIResourceDataProvider
	PerObjectConfig
	Config(Game);

var	config				string				ClassPathName;
var	config	localized	string				CharacterName;
var	config	localized	string				CharacterBio;

var	config				bool				bIsDisabled;

/**
 * Allows a resource data provider instance to indicate that it should be unselectable in subscribed lists
 *
 * @return	FALSE to indicate that list elements which represent this data provider should be considered unselectable
 *			or otherwise disabled (though it will still appear in the list).
 */
event bool IsProviderDisabled()
{
	return bIsDisabled;
}

DefaultProperties
{

}
