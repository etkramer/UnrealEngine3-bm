/**
 * Gears2-specific game resources data store.  Has logic for special-case handling of certain providers types - specifically
 * resource providers which provide collection data.  Adds support for indexing the provider by ProviderTag.
 *
 * @todo ronp - promote to engine after press demo.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUIDataStore_GameResource extends UIDataStore_GameResource
	native(UIPrivate);

/** the tags used for referencing the character classes for each team. */
var	transient	const	name	ResourceDataStoreTeamNames[2];

var	transient	const	Object	ResourceDataStoreTeamIdTypes[2];

cpptext
{
	/* === UUIDataStore interface === */
	/**
	 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
	 *
	 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
	 *
	 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
	 *			there is no list element provider associated with the specified property.
	 */
	virtual TScriptInterface<class IUIListElementProvider> ResolveListElementProvider( const FString& PropertyName );

	/* === UUIDataProvider interface === */
	/**
	 * Gets the list of data fields exposed by this data provider.
	 *
	 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
	 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
	 */
	virtual void GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields );

	/* === IUIListElementProvider interface === */
	/**
	 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
	 *
	 * @return	the list of tags supported by this element provider which correspond to list element data.
	 */
	virtual TArray<FName> GetElementProviderTags();

	/**
	 * Returns the number of list elements associated with the data tag specified.
	 *
	 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
	 *						from GetElementProviderTags.
	 *
	 * @return	the total number of elements that are required to fully represent the data specified.
	 */
	virtual INT GetElementCount( FName FieldName );

	/**
	 * Retrieves the list elements associated with the data tag specified.
	 *
	 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
	 *							from GetElementProviderTags.
	 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
	 *
	 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
	 */
	virtual UBOOL GetListElements( FName FieldName, TArray<INT>& out_Elements );

	/**
	 * Determines whether a member of a collection should be considered "enabled" by subscribed lists.  Disabled elements will still be displayed in the list
	 * but will be drawn using the disabled state.
	 *
	 * @param	FieldName			the name of the collection data field that CollectionIndex indexes into.
	 * @param	CollectionIndex		the index into the data field collection indicated by FieldName to check
	 *
	 * @return	TRUE if FieldName doesn't correspond to a valid collection data field, CollectionIndex is an invalid index for that collection,
	 *			or the item is actually enabled; FALSE only if the item was successfully resolved into a data field value, but should be considered disabled.
	 */
	virtual UBOOL IsElementEnabled( FName FieldName, INT CollectionIndex );

	/**
	 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
	 * Used by the UI editor to know which cells are available for binding to individual list cells.
	 *
	 * @param	FieldName		the tag of the list element data provider that we want the schema for.
	 *
	 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
	 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
	 */
	virtual TScriptInterface<class IUIListElementCellProvider> GetElementCellSchemaProvider( FName FieldName );

	/**
	 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
	 * of the list element indicated by CellValueProvider.DataSourceIndex
	 *
	 * @param	FieldName		the tag of the list element data field that we want the values for
	 * @param	ListIndex		the list index for the element to get values for
	 *
	 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
	 */
	virtual TScriptInterface<class IUIListElementCellProvider> GetElementCellValueProvider( FName FieldName, INT ListIndex );
}

static function name GetTeamCharacterProviderTag( int TeamIndex )
{
	return default.ResourceDataStoreTeamNames[TeamIndex % ArrayCount(default.ResourceDataStoreTeamNames)];
}

static function Object GetTeamCharacterProviderIdType( int TeamIndex )
{
	return default.ResourceDataStoreTeamIdTypes[TeamIndex % ArrayCount(default.ResourceDataStoreTeamIdTypes)];
}

/** Returns the provider for the playlist gamemode that at ProviderIndex in the list */
static function GearGamePlaylistGameTypeProvider GetPlaylistGameTypeProvider( int ProviderIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> PlaylistGameTypeProviders;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the playlist providers
		if ( GameResourceDS.GetResourceProviders('PlaylistGameTypes', PlaylistGameTypeProviders) )
		{
			// sanity check
			if ( ProviderIndex < 0 || ProviderIndex >= PlaylistGameTypeProviders.length )
			{
				ProviderIndex = 0;
			}

			return GearGamePlaylistGameTypeProvider(PlaylistGameTypeProviders[ProviderIndex]);
		}
	}

	return None;
}

/** Returns the provider index of the EGearMPTypes passed in */
static function int GetGameTypeProviderIndex( EGearMPTypes GameType )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue Value;
	local array<UIResourceDataProvider> Providers;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the chapter providers
		if ( GameResourceDS.GetResourceProviders('GameTypes', Providers) )
		{
			Value.PropertyTag = 'MPGameMode';
			Value.PropertyType = DATATYPE_Property;
			Value.StringValue = string(GameType);

			// Find the index to the provider with GameType MPGameMode
			return GameResourceDS.FindProviderIndexByFieldValue('GameTypes', 'MPGameMode', Value);
		}
	}

	return INDEX_NONE;
}

/** Returns the provider with enum EGearMPTypes */
static function GearGameInfoSummary GetGameTypeProvider( EGearMPTypes GameType )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> Providers;
	local int ProviderIndex;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the providers
		if ( GameResourceDS.GetResourceProviders('GameTypes', Providers) )
		{
			ProviderIndex = GetGameTypeProviderIndex(GameType);
			return GearGameInfoSummary(Providers[ProviderIndex]);
		}
	}

	return None;
}

/** Returns the provider at provider index */
static function GearGameInfoSummary GetGameTypeProviderUsingProviderIndex( int ProviderIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> Providers;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the providers
		if ( GameResourceDS.GetResourceProviders('GameTypes', Providers) )
		{
			return GearGameInfoSummary(Providers[ProviderIndex]);
		}
	}

	return None;
}

/** Returns the OnlinePlaylistProvider with the corresponding PlaylistId */
static function OnlinePlaylistProvider GetOnlinePlaylistProvider( int PlaylistId )
{
	local int ProviderIndex;
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> Providers;
	local UIProviderScriptFieldValue Value;
	local OnlinePlaylistProvider PlaylistProvider;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the playlist providers
		if ( GameResourceDS.GetResourceProviders('Playlists', Providers) )
		{
			Value.PropertyTag = 'PlaylistId';
			Value.PropertyType = DATATYPE_Property;
			Value.StringValue = string(PlaylistId);

			// Find the index to the provider with the playerlistid
			ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Playlists', 'PlaylistId', Value);

			if ( ProviderIndex != INDEX_NONE )
			{
				PlaylistProvider = OnlinePlaylistProvider(Providers[ProviderIndex]);
			}
		}
	}

	return PlaylistProvider;
}

/** Returns the provider index for the playlist game type of the playlist using the gametype described by the EGearMPTypes enum value as the key */
static function bool PlaylistContainsGameType( int PlaylistId, EGearMPTypes MPType )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local OnlinePlaylistProvider PlaylistProv;
	local int ProviderIndex, GameIdx;
	local GearGamePlaylistGameTypeProvider GameTypeProvider;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		PlaylistProv = GameResourceDS.GetOnlinePlaylistProvider( PlaylistId );
		if ( PlaylistProv != None )
		{
			for ( GameIdx = 0; GameIdx < PlaylistProv.PlaylistGameTypeNames.length; GameIdx++ )
			{
				ProviderIndex = GameResourceDS.ConvertPlaylistGameNameIndexToPlaylistGameTypeProviderIndex( PlaylistProv, GameIdx );
				if ( ProviderIndex != INDEX_NONE )
				{
					GameTypeProvider = GameResourceDS.GetPlaylistGameTypeProvider( ProviderIndex );
					if ( GameTypeProvider != None )
					{
						if ( GameTypeProvider.MPGameMode == MPType )
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

/** Converts the index of the GameTypeName list of the playlist into the provider index of the playlist gametype list */
static function int ConvertPlaylistGameNameIndexToPlaylistGameTypeProviderIndex( OnlinePlaylistProvider PlaylistProvider, int GameTypeNameIndex ) // <----- best name ever!
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue Value;
	local name NameId;

	if ( GameTypeNameIndex >= 0 && GameTypeNameIndex < PlaylistProvider.PlaylistGameTypeNames.length )
	{
		NameId = PlaylistProvider.PlaylistGameTypeNames[GameTypeNameIndex];
	}

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		Value.PropertyTag = 'PlaylistGameTypeName';
		Value.PropertyType = DATATYPE_Property;
		Value.StringValue = string(NameId);

		// Find the index to the provider
		return GameResourceDS.FindProviderIndexByFieldValue('PlaylistGameTypes', 'PlaylistGameTypeName', Value);
	}

	return 0;
}


/** Returns the character provider of the provider id passed in */
static function GearGameCharacterSummary GetCharacterProvider( int ProviderId, name ProviderTag )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> Providers;
	local GearGameCharacterSummary CharacterData;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the character providers
		if ( ProviderId != INDEX_NONE &&
			 GameResourceDS.GetResourceProviders(ProviderTag, Providers) )
		{
			CharacterData = GearGameCharacterSummary(Providers[ProviderId]);
			return CharacterData;
		}
	}

	return None;
}

/** Returns the character provider using the pawn class name as the key */
static function GearGameCharacterSummary GetCharacterProviderUsingClassName( string PawnClassNameString, name ProviderTag )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> Providers;
	local UIProviderScriptFieldValue Value;
	local int ProviderIndex;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		if (GameResourceDS.GetResourceProviders(ProviderTag, Providers))
		{
			Value.PropertyTag = 'ClassPathName';
			Value.PropertyType = DATATYPE_Property;
			Value.StringValue = PawnClassNameString;

			// Find the index to the provider with PawnClassNameString
			ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue(ProviderTag, 'ClassPathName', Value);
			if (ProviderIndex != INDEX_NONE)
			{
				return GearGameCharacterSummary(Providers[ProviderIndex]);
			}
		}
	}

	return None;
}

/** Returns the weapon provider of the provider id passed in */
static function GearGameWeaponSummary GetWeaponProvider( int ProviderId )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> Providers;
	local GearGameWeaponSummary WeaponData;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the character providers
		if ( ProviderId != INDEX_NONE &&
			GameResourceDS.GetResourceProviders('Weapons', Providers) )
		{
			WeaponData = GearGameWeaponSummary(Providers[ProviderId]);
			return WeaponData;
		}
	}

	return None;
}

/** Returns the chapter provider of the chapter passed in */
static function GearCampaignChapterData GetChapterDataProvider( EChapterPoint ChapType )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue Value;
	local int ProviderIndex;
	local array<UIResourceDataProvider> ChapterProviders;
	local GearCampaignChapterData ChapterData;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the chapter providers
		if ( GameResourceDS.GetResourceProviders('Chapters', ChapterProviders) )
		{
			Value.PropertyTag = 'ChapterType';
			Value.PropertyType = DATATYPE_Property;
			Value.StringValue = string(ChapType);

			// Find the index to the provider with ChapType chapterpoint
			ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Chapters', 'ChapterType', Value);

			if ( ProviderIndex != INDEX_NONE )
			{
				ChapterData = GearCampaignChapterData(ChapterProviders[ProviderIndex]);
				return ChapterData;
			}
		}
	}

	return None;
}

/** Returns the act provider of the act passed in */
static function GearCampaignActData GetActDataProvider( EGearAct ActType )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> ActProviders;
	local UIProviderScriptFieldValue Value;
	local GearCampaignActData ActData;
	local int ProviderIndex;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the act providers
		if ( GameResourceDS.GetResourceProviders('Acts', ActProviders) )
		{
			Value.PropertyTag = 'ActType';
			Value.PropertyType = DATATYPE_Property;
			Value.StringValue = string(GetEnum(enum'EGearAct', ActType));

			// Find the index to the provider with ActType
			ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Acts', 'ActType', Value);

			if ( ProviderIndex != INDEX_NONE )
			{
				ActData = GearCampaignActData(ActProviders[ProviderIndex]);
				return ActData;
			}
		}
	}

	return None;
}

/** Returns the act provider of the chapter passed in */
static function GearCampaignActData GetActDataProviderUsingChapter( EChapterPoint ChapType )
{
	local GearCampaignActData ActData;
	local GearCampaignChapterData ChapterData;

	ChapterData = GetChapterDataProvider(ChapType);
	if ( ChapterData != None )
	{
		ActData = GetActDataProvider(ChapterData.ActType);
		return ActData;
	}

	return None;
}

/** Returns the act provider of the chapter provider passed in */
static function GearCampaignActData GetActDataProviderUsingChapterProvider( GearCampaignChapterData ChapterData )
{
	local GearCampaignActData ActData;

	if ( ChapterData != None )
	{
		ActData = GetActDataProvider(ChapterData.ActType);
		return ActData;
	}

	return None;
}

static function int GetActProviderIndexFromActId( EGearAct ActId )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue Value;
	local int ProviderIndex;

	ProviderIndex = INDEX_NONE;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		Value.PropertyTag = 'ActType';
		Value.PropertyType = DATATYPE_Property;
		Value.StringValue = string(GetEnum(enum'EGearAct', ActId));

		// Find the index to the provider with ActType
		ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Acts', 'ActType', Value);
	}

	return ProviderIndex;
}

static function int GetChapterProviderIndexFromChapterId( EChapterPoint ChapterId )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue Value;
	local int ProviderIndex;

	ProviderIndex = INDEX_NONE;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		Value.PropertyTag = 'ChapterType';
		Value.PropertyType = DATATYPE_Property;
		Value.StringValue = string(GetEnum(enum'EChapterPoint', ChapterId));

		// Find the index to the provider with ActType
		ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Chapters', 'ChapterType', Value);
	}

	return ProviderIndex;
}

static function int GetActChapterProviderIndexFromChapterId( EChapterPoint ChapterId )
{
	local GearCampaignActData ActProvider;
	local GearCampaignChapterData ChapterProvider;

	ChapterProvider = GetChapterDataProvider(ChapterId);
	ActProvider = GetActDataProviderUsingChapterProvider(ChapterProvider);
	return ActProvider.ChapterProviders.Find(ChapterProvider);
}

/** Returns the mapname from the datastore of the chapter passed in */
static function string GetMapNameUsingEnum( EChapterPoint ChapType )
{
	local GearCampaignChapterData ChapterData;

	ChapterData = static.GetChapterDataProvider(ChapType);
	if ( ChapterData != None )
	{
		return ChapterData.MapName;
	}

	return "";
}

/** Returns the list of EChapterPoints this act contains */
static function bool GetChapterPointsFromAct( EGearAct ActType, out array<EChapterPoint> ChapterList )
{
	local GearCampaignActData ActData;
	local GearCampaignChapterData ChapterData;
	local int ChapIdx;

	ChapterList.length = 0;
	ActData = static.GetActDataProvider(ActType);
	if ( ActData != None )
	{
		// Loop through the act's chapter provider list and fill the array with the chapter points
		for ( ChapIdx = 0; ChapIdx < ActData.ChapterProviders.length ; ChapIdx++ )
		{
			ChapterData = ActData.ChapterProviders[ChapIdx];
			if ( ChapterData != None )
			{
				ChapterList.AddItem( ChapterData.ChapterType );
			}
		}

		return true;
	}

	return false;
}

/** Returns the chapter type from the normalized chapter number in the act that the chapter is in */
static function EChapterPoint GetChapterTypeFromNormalizedChapterInAct(EGearAct ActType, int NormalizedChapter)
{
	local GearUIDataStore_GameResource GameResourceDS;
	local GearCampaignActData ActData;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if (GameResourceDS != None)
	{
		ActData = GameResourceDS.GetActDataProvider(ActType);
		if (ActData != None)
		{
			if (ActData.ChapterProviders.length > NormalizedChapter)
			{
				return ActData.ChapterProviders[NormalizedChapter].ChapterType;
			}
		}
	}
	return INDEX_NONE;
}

/** Returns the list of EGearActs required to complete to beat the game */
static function bool GetGameCompletionActList( out array<EGearAct> ActList )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> ActProviders;
	local GearCampaignActData ActData;
	local int ActIdx;

	ActList.length = 0;
	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the act providers
		if ( GameResourceDS.GetResourceProviders('Acts', ActProviders) )
		{
			// Loop through the acts list and fill the array with the acts needed to complete the game
			for ( ActIdx = 0; ActIdx < ActProviders.length ; ActIdx++ )
			{
				ActData = GearCampaignActData(ActProviders[ActIdx]);
				if ( ActData != None && ActData.bRequiredActForGameCompletion )
				{
					ActList.AddItem( ActData.ActType );
				}
			}

			return true;
		}
	}

	return false;
}

/** Returns the ShippedMapType of an MP map using the mapname as the key */
static function EGearMapsShipped GetMPMapsShippedType( string Mapname )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> MapProviders;
	local UIProviderScriptFieldValue Value;
	local GearGameMapSummary MapData;
	local int ProviderIndex;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the act providers
		if ( GameResourceDS.GetResourceProviders('Maps', MapProviders) )
		{
			Value.PropertyTag = 'MapName';
			Value.PropertyType = DATATYPE_Property;
			Value.StringValue = Mapname;

			// Find the index to the provider with MapName
			ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Maps', 'MapName', Value);

			if ( ProviderIndex != INDEX_NONE )
			{
				MapData = GearGameMapSummary(MapProviders[ProviderIndex]);
				return MapData.ShippedMapType;
			}
		}
	}

	return eGEARMAP_None;
}

/** Returns the provider index of a particular mapname */
static function int GetProviderIndexFromMapName( string Mapname )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue Value;
	local int ProviderIndex;

	ProviderIndex = INDEX_NONE;
	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		Value.PropertyTag = 'MapName';
		Value.PropertyType = DATATYPE_Property;
		Value.StringValue = Mapname;

		// Find the index to the provider with MapName
		ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Maps', 'MapName', Value);
	}

	return ProviderIndex;
}

/** Returns the GearGameMapSummary using the mapname as the key */
static function GearGameMapSummary GetMapSummaryFromMapName( string MapName )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> MapProviders;
	local int ProviderIndex;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		if ( GameResourceDS.GetResourceProviders('Maps', MapProviders) )
		{
			ProviderIndex = GameResourceDS.GetProviderIndexFromMapName( MapName );
			if ( ProviderIndex != INDEX_NONE && ProviderIndex >= 0 && ProviderIndex < MapProviders.length )
			{
				return GearGameMapSummary(MapProviders[ProviderIndex]);
			}
		}
	}

	return None;
}

/** Returns the mapname at a specific provider index */
static function string GetMapNameUsingProviderIndex( int ProviderIndex )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> MapProviders;
	local GearGameMapSummary MapData;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		// Grab the map providers
		if ( GameResourceDS.GetResourceProviders('Maps', MapProviders) )
		{
			if ( MapProviders.length > 0 &&
				 ProviderIndex >= 0 &&
				 ProviderIndex < MapProviders.length )
			{
				MapData = GearGameMapSummary(MapProviders[ProviderIndex]);
				return MapData.MapName;
			}
		}
	}

	return "";
}

/**
 * Get the localized name and description for a specific collectable.
 *
 * @param	DiscoverableType	the enum value for the discoverable to lookup
 * @param	DiscoverableName	receives the value of the discoverable's localized name
 * @param	DiscoverableDescription		receives the value for the discoverable's localized description
 *
 * @return	true if both the name and description were successfully retrieved
 */
static function bool GetDiscoverableDisplayStrings( EGearDiscoverableType DiscoverableType, out string DiscoverableName, out string DiscoverableDescription )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue SearchValue;
	local int ProviderIdx;
	local bool bResult;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		SearchValue.PropertyTag = 'CollectableId';
		SearchValue.PropertyType = DATATYPE_Property;
		SearchValue.StringValue = string(GetEnum(enum'EGearDiscoverableType',DiscoverableType));

		ProviderIdx = GameResourceDS.FindProviderIndexByFieldValue('Collectables', 'CollectableId', SearchValue);
		if ( ProviderIdx != INDEX_NONE )
		{
			bResult = true;
			if ( !GameResourceDS.GetProviderFieldValue('Collectables', 'CollectableName', ProviderIdx, SearchValue) )
			{
				bResult = false;
			}
			else
			{
				DiscoverableName = SearchValue.StringValue;
			}

			if ( !GameResourceDS.GetProviderFieldValue('Collectables', 'CollectableDetails', ProviderIdx, SearchValue) )
			{
				bResult = false;
			}
			else
			{
				DiscoverableDescription = SearchValue.StringValue;
			}
		}
	}

	return bResult;
}

/**
 * Get the collectible specific background image path name for the collectible
 *
 * @param	DiscoverableType	the enum value for the discoverable to lookup
 * @param	ImagePath	pathname to the image for the background of the collecable
 *
 * @return	true if succedded
 */
static function bool GetDiscoverableBackgroundPath( EGearDiscoverableType DiscoverableType, out string ImagePath )
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue SearchValue;
	local int ProviderIdx;
	local bool bResult;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		SearchValue.PropertyTag = 'CollectableId';
		SearchValue.PropertyType = DATATYPE_Property;
		SearchValue.StringValue = string(GetEnum(enum'EGearDiscoverableType',DiscoverableType));

		ProviderIdx = GameResourceDS.FindProviderIndexByFieldValue('Collectables', 'CollectableId', SearchValue);
		if ( ProviderIdx != INDEX_NONE )
		{
			bResult = true;
			if ( !GameResourceDS.GetProviderFieldValue('Collectables', 'UnlockedImage_Markup', ProviderIdx, SearchValue) )
			{
				bResult = false;
			}
			else
			{
				ImagePath = SearchValue.StringValue;
			}
		}
	}

	return bResult;
}

/**
 * Get the localized name and description for a specific unlock.
 *
 * @param	UnlockType				the enum value for the unlockable to lookup
 * @param	UnlockableName			receives the value of the unlockable's localized name
 * @param	UnlockableDescription	receives the value for the unlockable's localized description
 *
 * @return	true if both the name and description were successfully retrieved
 */
static function bool GetUnlockableDisplayStrings( EGearUnlockable UnlockableType, out string UnlockableName , out string UnlockableDescription)
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue SearchValue;
	local int ProviderIdx;
	local bool bResult;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		SearchValue.PropertyTag = 'UnlockableId';
		SearchValue.PropertyType = DATATYPE_Property;
		SearchValue.StringValue = string(GetEnum(enum'EGearUnlockable',UnlockableType));

		ProviderIdx = GameResourceDS.FindProviderIndexByFieldValue('Unlockables', 'UnlockableId', SearchValue);
		if ( ProviderIdx != INDEX_NONE )
		{
			bResult = true;
			if ( !GameResourceDS.GetProviderFieldValue('Unlockables', 'UnlockableName', ProviderIdx, SearchValue) )
			{
				bResult = false;
			}
			else
			{
				UnlockableName = SearchValue.StringValue;
			}

			if ( !GameResourceDS.GetProviderFieldValue('Unlockables', 'UnlockableDescription', ProviderIdx, SearchValue) )
			{
				bResult = false;
			}
			else
			{
				UnlockableDescription = SearchValue.StringValue;
			}
		}
	}

	return bResult;
}


DefaultProperties
{
	ResourceDataStoreTeamNames(0)=COGs
	ResourceDataStoreTeamNames(1)=Locusts
	ResourceDataStoreTeamIdTypes(0)=enum'ECogMPCharacter'
	ResourceDataStoreTeamIdTypes(1)=enum'ELocustMPCharacter'
}
