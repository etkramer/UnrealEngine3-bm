/**
 * Provides data for a UT3 map.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTUIDataProvider_MapInfo extends UTUIResourceDataProvider
	native(UI)
	PerObjectConfig;

/** Unique ID for maps. */
var config int	  MapID;

/** Actual map name to load */
var config string MapName;

/** String describing how many players the map is good for. */
var config localized string NumPlayers;

/** Friendly displayable name to the player. */
var config localized string FriendlyName;

/** Localized description of the map */
var config localized string Description;

/** Markup text used to display the preview image for the map. */
var config string PreviewImageMarkup;

/**
 * Indicates whether this data provider corresponds to an epic map.  Set natively when the data provider
 * is initialized.
 */
var	const private bool bOfficialMap;

cpptext
{
	virtual void AddChildProviders(TArray<UUTUIResourceDataProvider*>& Providers);

	/** @return 	TRUE if this data provider is not supported by the currently selected gametype */
	virtual UBOOL IsFiltered();
}

/**
 * @return	TRUE if this data provider corresponds to an epic map.
 */
native final function bool IsOfficialMap() const;

/** @return Returns whether or not this provider is supported by the current game mode */
event bool SupportedByCurrentGameMode()
{
	local int Pos, i;
	local string ThisMapPrefix, GameModePrefixes;
	local array<string> PrefixList;
	local bool bResult;

	bResult = true;

	// Get our map prefix.
	Pos = InStr(MapName,"-");
	ThisMapPrefix = left(MapName,Pos);

	if (GetDataStoreStringValue("<Registry:SelectedGameModePrefix>", GameModePrefixes) && GameModePrefixes != "")
	{
		bResult = false;
		ParseStringIntoArray(GameModePrefixes, PrefixList, "|", true);
		for (i = 0; i < PrefixList.length; i++)
		{
			bResult = (ThisMapPrefix ~= PrefixList[i]);
			if (bResult)
			{
				break;
			}
		}
	}

	return bResult;
}

defaultproperties
{
	bSearchAllInis=true
}
