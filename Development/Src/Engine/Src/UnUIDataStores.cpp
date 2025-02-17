/*=============================================================================
	UnUIDataStores.cpp: UI data store class implementations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// engine classes
#include "EnginePrivate.h"
#include "FConfigCacheIni.h"

// widgets and supporting UI classes
#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"
#include "UnUIMarkupResolver.h"

IMPLEMENT_CLASS(UDataStoreClient);
IMPLEMENT_CLASS(UUIDataProvider);
	IMPLEMENT_CLASS(UUIConfigProvider);
		IMPLEMENT_CLASS(UUIConfigFileProvider);
		IMPLEMENT_CLASS(UUIConfigSectionProvider);
	IMPLEMENT_CLASS(UUIDataStore);
		IMPLEMENT_CLASS(UUIDataStore_GameResource);
		IMPLEMENT_CLASS(UUIDataStore_DynamicResource);
		IMPLEMENT_CLASS(UUIDataStore_Settings);
			IMPLEMENT_CLASS(UUIDataStore_SessionSettings);
			IMPLEMENT_CLASS(UUIDataStore_PlayerSettings);
		IMPLEMENT_CLASS(UUIDataStore_GameState);
			IMPLEMENT_CLASS(UCurrentGameDataStore);
			IMPLEMENT_CLASS(UPlayerOwnerDataStore);
		IMPLEMENT_CLASS(UUIDataStore_Strings);
		IMPLEMENT_CLASS(UUIDataStore_Images);
		IMPLEMENT_CLASS(UUIDataStore_Fonts);
		IMPLEMENT_CLASS(UUIDataStore_Color);
		IMPLEMENT_CLASS(UUIDataStore_Remote);
		IMPLEMENT_CLASS(UUIDataStore_StringAliasMap);
		IMPLEMENT_CLASS(USceneDataStore);
		IMPLEMENT_CLASS(UUIDataStore_StringBase);
		IMPLEMENT_CLASS(UUIDataStore_InputAlias);
	IMPLEMENT_CLASS(UUIPropertyDataProvider);
		IMPLEMENT_CLASS(UUIResourceDataProvider);
			IMPLEMENT_CLASS(UUIMapSummary);
		IMPLEMENT_CLASS(UUIDynamicDataProvider);
			IMPLEMENT_CLASS(UGameInfoDataProvider);
			IMPLEMENT_CLASS(UInventoryDataProvider);
				IMPLEMENT_CLASS(UPowerupDataProvider);
				IMPLEMENT_CLASS(UWeaponDataProvider);
					IMPLEMENT_CLASS(UCurrentWeaponDataProvider);
			IMPLEMENT_CLASS(UPickupDataProvider);
			IMPLEMENT_CLASS(UPlayerDataProvider);
				IMPLEMENT_CLASS(UPlayerOwnerDataProvider);
			IMPLEMENT_CLASS(UTeamDataProvider);
		IMPLEMENT_CLASS(UUISettingsProvider);
			IMPLEMENT_CLASS(UPlayerSettingsProvider);
			IMPLEMENT_CLASS(USessionSettingsProvider);
	IMPLEMENT_CLASS(UUIDynamicFieldProvider);
	IMPLEMENT_CLASS(UUIResourceCombinationProvider);

IMPLEMENT_CLASS(UUIDataStore_Registry);
IMPLEMENT_CLASS(UUIDataStoreSubscriber);
IMPLEMENT_CLASS(UUIDataStorePublisher);

#define ARRAY_DELIMITER TEXT(";")

typedef TMap< FName, TMap<FName,TArray<FString> > > DynamicCollectionValueMap;

static UBOOL IsUniqueNetIdStruct( UStruct* StructToCheck )
{
	static UScriptStruct* UniqueNetIdStruct = FindObject<UScriptStruct>(UOnlineSubsystem::StaticClass(), TEXT("UniqueNetId"));
	checkf(UniqueNetIdStruct!=NULL,TEXT("Unable to find UniqueNetId struct within OnlineSubsystem!"));

	return StructToCheck->IsChildOf(UniqueNetIdStruct);
}

static UBOOL IsRangeValueStruct( UStruct* StructToCheck )
{
	static UScriptStruct* UIRangeStruct = FindObject<UScriptStruct>(UUIRoot::StaticClass(), TEXT("UIRangeData"));
	checkf(UIRangeStruct!=NULL,TEXT("Unable to find UIRangeData struct within UIRoot!"));

	return StructToCheck->IsChildOf(UIRangeStruct);
}

/* ==========================================================================================================
	UUIRoot
========================================================================================================== */

UBOOL UUIRoot::ResolveDataStoreMarkup(const FString &DataFieldMarkupString, UUIScene* InOwnerScene, ULocalPlayer* InOwnerPlayer, 
						class UUIDataProvider*& out_ResolvedProvider, FString& out_DataFieldName, class UUIDataStore** out_ResolvedDataStore )
{
	UBOOL bResult = FALSE;
	UDataStoreClient* DataStoreClient = UUIInteraction::GetDataStoreClient();

	//@todo - support resolving data store values outside of the context of the UI (i.e. placing a GetDataStoreValue action in the level's sequence)
	if ( DataFieldMarkupString.Len() > 0 && DataStoreClient )
	{
		// now, resolve the data store markup
		FUIStringParser Parser;

		// scan in the markup string - we should end up with only one chunk
		Parser.ScanString(DataFieldMarkupString);

		const TIndirectArray<FTextChunk>* Chunks = Parser.GetTextChunks();
		if ( Chunks->Num() == 1 )
		{
			const FTextChunk* MarkupChunk = &((*Chunks)(0));
			if ( MarkupChunk->IsMarkup() )
			{
				FString DataStoreTag, DataStoreValue;

				// split-off the data store name from the data field path
				if ( MarkupChunk->GetDataStoreMarkup(DataStoreTag, DataStoreValue) )
				{
					// attempt to resolve the data store name
					UUIDataStore* ResolvedDataStore = NULL;
					
					if(InOwnerScene==NULL)
					{
						ResolvedDataStore = DataStoreClient->FindDataStore(*DataStoreTag, InOwnerPlayer);
					}
					else
					{
						ResolvedDataStore = InOwnerScene->ResolveDataStore(*DataStoreTag, InOwnerPlayer);
					}

					if ( ResolvedDataStore != NULL )
					{
						INT UnusedArrayIndex = INDEX_NONE;

						// now resolve the data provider that contains the tag specified
 						if ( !ResolvedDataStore->ParseDataStoreReference(DataStoreValue, out_ResolvedProvider, out_DataFieldName, UnusedArrayIndex) )
						{
							out_ResolvedProvider = ResolvedDataStore;
							out_DataFieldName = DataStoreValue;
						}

						if ( out_ResolvedDataStore != NULL )
						{
							*out_ResolvedDataStore = ResolvedDataStore;
						}

						bResult = TRUE;
					}
				}
			}
		}
	}

	return bResult;
}

/** 
 * Sets the string value of the datastore entry specified. 
 *
 * @param InDataStoreMarkup		Markup to find the field we want to set the value of.
 * @param InFieldValue			Value to set the datafield's value to.
 * @param OwnerScene			Owner scene for the datastore, used when dealing with scene specific datastores.
 * @param OwnerPlayer			Owner player for the datastore, used when dealing with player datastores.
 *
 * @return TRUE if the value was set, FALSE otherwise.
 */
UBOOL UUIRoot::SetDataStoreFieldValue(const FString &InDataStoreMarkup, const FUIProviderFieldValue &InFieldValue, UUIScene* OwnerScene, ULocalPlayer* OwnerPlayer)
{
	UBOOL bSuccess = FALSE;
	UUIDataStore* DataStore = NULL;
	UUIDataProvider* FieldProvider = NULL;
	FString FieldName;

	// resolve the markup string to get the name of the data field and the provider that contains that data
	if ( ResolveDataStoreMarkup(InDataStoreMarkup, OwnerScene, OwnerPlayer,FieldProvider, FieldName, &DataStore) )
	{
		FUIProviderScriptFieldValue ScriptFieldValue(InFieldValue);
		ScriptFieldValue.PropertyTag = *FieldName;

		bSuccess = FieldProvider->SetDataStoreValue(FieldName, ScriptFieldValue);
		if ( DataStore != NULL )
		{
			DataStore->OnCommit();
		}
	}

	return bSuccess;
}

/** 
 * Gets the field value struct of the datastore entry specified. 
 *
 * @param InDataStoreMarkup		Markup to find the field we want to retrieve the value of.
 * @param OutFieldValue			Variable to store the result field value in.
 * @param OwnerScene			Owner scene for the datastore, used when dealing with scene specific datastores.
 * @param OwnerPlayer			Owner player for the datastore, used when dealing with player datastores.
 *
 * @return TRUE if the value was retrieved, FALSE otherwise.
 */
UBOOL UUIRoot::GetDataStoreFieldValue(const FString &InDataStoreMarkup, FUIProviderFieldValue &OutFieldValue, UUIScene* OwnerScene, ULocalPlayer* OwnerPlayer)
{
	UBOOL bSuccess = FALSE;

	UUIDataProvider* FieldProvider = NULL;
	FString FieldName;

	// resolve the markup string to get the name of the data field and the provider that contains that data
	if ( ResolveDataStoreMarkup(InDataStoreMarkup, OwnerScene, OwnerPlayer, FieldProvider, FieldName ) )
	{
		if ( FieldProvider->GetDataStoreValue(FieldName, OutFieldValue) )
		{
			bSuccess = TRUE;
		}
	}

	return bSuccess;
}

/* ==========================================================================================================
	UDataStoreClient
========================================================================================================== */
/**
 * Loads and initializes all data store classes contained by the DataStoreClasses array
 */
void UDataStoreClient::InitializeDataStores()
{
	for ( INT DataStoreIndex = 0; DataStoreIndex < GlobalDataStoreClasses.Num(); DataStoreIndex++ )
	{
		UClass* DataStoreClass = LoadClass<UUIDataStore>(NULL, *GlobalDataStoreClasses(DataStoreIndex), NULL, LOAD_None, NULL);
		if (DataStoreClass != NULL )
		{
			// Allow the data store to load any dependent classes it will need later
			UUIDataStore* DefaultStore = DataStoreClass->GetDefaultObject<UUIDataStore>();
			DefaultStore->LoadDependentClasses();

			UUIDataStore* DataStore = CreateDataStore(DataStoreClass);
			if ( DataStore != NULL )
			{
				RegisterDataStore(DataStore);
			}
		}
		else
		{
			debugf(NAME_Warning, TEXT("Failed to load GlobalDataStoreClass '%s'"), *GlobalDataStoreClasses(DataStoreIndex));
		}
	}

	// PlayerDataStores are instanced when the player is created, so all we need to do is make sure the class is loaded to be compatible with streaming
	for ( INT PlayerDataStoreIndex = 0; PlayerDataStoreIndex < PlayerDataStoreClassNames.Num(); PlayerDataStoreIndex++ )
	{
		UClass* DataStoreClass = LoadClass<UUIDataStore>(NULL, *PlayerDataStoreClassNames(PlayerDataStoreIndex), NULL, LOAD_None, NULL);
		if ( DataStoreClass == NULL )
		{
			debugf(NAME_Warning, TEXT("Failed to load player data store class '%s'"), *PlayerDataStoreClassNames(PlayerDataStoreIndex));
		}
		else
		{
			// store the class in our array so that it doesn't get GC'd.
			PlayerDataStoreClasses.AddUniqueItem(DataStoreClass);
			// Allow the data store to load any dependent classes it will need later
			UUIDataStore* DefaultStore = DataStoreClass->GetDefaultObject<UUIDataStore>();
			DefaultStore->LoadDependentClasses();

			if (GIsEditor && !GIsGame)
			{
				// since we won't have any PlayerControllers in the editor, add all PlayerDataStores to the GlobalDataStores array
				UUIDataStore* PlayerDataStore = CreateDataStore(DataStoreClass);
				if ( PlayerDataStore != NULL )
				{
					RegisterDataStore(PlayerDataStore);
				}
			}
		}
	}
}

/**
 * Creates and initializes an instance of the data store class specified.
 *
 * @param	DataStoreClass	the data store class to create an instance of.  DataStoreClass should be a child class
 *							of UUIDataStore
 *
 * @return	a pointer to an instance of the data store class specified.
 */
UUIDataStore* UDataStoreClient::CreateDataStore( UClass* DataStoreClass )
{
	UUIDataStore* Result = NULL;
	if ( DataStoreClass != NULL && DataStoreClass->IsChildOf(UUIDataStore::StaticClass()) )
	{
		Result = ConstructObject<UUIDataStore>(DataStoreClass, this);
		Result->InitializeDataStore();
	}

	return Result;
}

/**
 * Retrieve the list of currently available data stores, including any temporary data stores associated with the specified scene.
 *
 * @param	CurrentScene	the scene to use as the context for determining which data stores are available
 * @param	out_DataStores	will be filled with the list of data stores which are available from the context of the specified scene
 */
void UDataStoreClient::GetAvailableDataStores(UUIScene* CurrentScene, TArray<UUIDataStore*>& out_DataStores ) const
{
	out_DataStores.Empty();

	// Add all of the global data stores to the array.
	for (INT DataStoreIndex = 0; DataStoreIndex < GlobalDataStores.Num(); DataStoreIndex++)
	{
		UUIDataStore *Data = GlobalDataStores (DataStoreIndex);
		if(Data != NULL)
		{
			out_DataStores.AddUniqueItem(Data);
		}
	}

	if(CurrentScene != NULL)
	{
		if ( CurrentScene->PlayerOwner != NULL )
		{
			INT PlayerDataStoreIndex = FindPlayerDataStoreIndex(CurrentScene->PlayerOwner);
			check(PlayerDataStoreIndex);

			const FPlayerDataStoreGroup& DataStoreGroup = PlayerDataStores(PlayerDataStoreIndex);
			for ( INT DataStoreIndex = 0; DataStoreIndex < DataStoreGroup.DataStores.Num(); DataStoreIndex++ )
			{
				UUIDataStore* PlayerDataStore = DataStoreGroup.DataStores(DataStoreIndex);
				if ( PlayerDataStore != NULL )
				{
					out_DataStores.AddItem(PlayerDataStore);
				}
			}
		}

		// Add the scene's temporary datastore to the array.
		UUIDataStore* SceneDataStore = CurrentScene->GetSceneDataStore();
		if(SceneDataStore != NULL)
		{
			out_DataStores.AddUniqueItem(SceneDataStore);
		}
	}
}

/**
 * Finds the data store indicated by DataStoreTag and returns a pointer to it.
 *
 * @param	DataStoreTag	A name corresponding to the 'Tag' property of a data store
 * @param	PlayerOwner		used for resolving the correct data stores in split-screen games.
 *
 * @return	a pointer to the data store that has a Tag corresponding to DataStoreTag, or NULL if no data
 *			were found with that tag.
 */
UUIDataStore* UDataStoreClient::FindDataStore( FName DataStoreTag, ULocalPlayer* PlayerOwner/*=NULL*/ )
{
	UUIDataStore* Result = NULL;

	if ( DataStoreTag != NAME_None )
	{
		// search the player data stores first
		if ( PlayerOwner != NULL )
		{
			INT PlayerDataIndex = FindPlayerDataStoreIndex(PlayerOwner);
			if ( PlayerDataIndex != INDEX_NONE )
			{
				FPlayerDataStoreGroup& DataStoreGroup = PlayerDataStores(PlayerDataIndex);
				for ( INT DataStoreIndex = 0; DataStoreIndex < DataStoreGroup.DataStores.Num(); DataStoreIndex++ )
				{
					if ( DataStoreGroup.DataStores(DataStoreIndex)->GetDataStoreID() == DataStoreTag )
					{
						Result = DataStoreGroup.DataStores(DataStoreIndex);
						break;
					}
				}
			}
		}

		if ( Result == NULL )
		{
			// now search the global data stores
			for ( INT DataStoreIndex = 0; DataStoreIndex < GlobalDataStores.Num(); DataStoreIndex++ )
			{
				if ( GlobalDataStores(DataStoreIndex)->GetDataStoreID() == DataStoreTag )
				{
					Result = GlobalDataStores(DataStoreIndex);
					break;
				}
			}
		}
	}

	return Result;
}

/**
 * Adds a new data store to the GlobalDataStores array.
 *
 * @param	DataStore		the data store to add
 * @param	PlayerOwner		the player that this data store should be associated with.  If specified, the data store
 *							is added to the list of data stores for that player, rather than the global data stores array.
 *
 * @return	TRUE if the data store was successfully added, or if the data store was already in the list.
 */
UBOOL UDataStoreClient::RegisterDataStore( UUIDataStore* DataStore, ULocalPlayer* PlayerOwner/*=NULL*/ )
{
	UBOOL bResult = FALSE;
	if ( DataStore != NULL )
	{
		// this is the data store tag that will be used to reference the data store
		FName DataStoreID = DataStore->GetDataStoreID();

		// this is the index into the PlayerDataStores array for PlayerOwner, if specified
		INT PlayerDataIndex = INDEX_NONE;
		UBOOL bDataStoreRegistered = FALSE;

		// if a player was specified, find the location of this player's data stores in the PlayerDataStores array.
		if ( PlayerOwner != NULL )
		{
			PlayerDataIndex = FindPlayerDataStoreIndex(PlayerOwner);
			if ( PlayerDataIndex == INDEX_NONE )
			{
				// this is the first data store we're registering for this player, so add an entry for this player now
				PlayerDataIndex = PlayerDataStores.AddZeroed();
			}

			check(PlayerDataStores.IsValidIndex(PlayerDataIndex));
			FPlayerDataStoreGroup& DataStoreGroup = PlayerDataStores(PlayerDataIndex);
			bDataStoreRegistered = DataStoreGroup.DataStores.ContainsItem(DataStore);

			// if this data store group was just added, we'll need to assign the PlayerOwner.
			DataStoreGroup.PlayerOwner = PlayerOwner;
		}

		bDataStoreRegistered = bDataStoreRegistered || GlobalDataStores.ContainsItem(DataStore);

		// if we already have this data store in the list, indicate success
		if ( bDataStoreRegistered )
		{
			bResult = TRUE;
		}
		else if ( DataStoreID != NAME_None )
		{
			// make sure there isn't already a data store with this tag in the list
			UUIDataStore* ExistingDataStore = FindDataStore(DataStoreID,PlayerOwner);
			if ( ExistingDataStore == NULL )
			{
				// if this data store is associated with a player, add it to the player's data store group
				if ( PlayerOwner != NULL && PlayerDataIndex != INDEX_NONE )
				{
					checkSlow(PlayerDataStores.IsValidIndex(PlayerDataIndex));
					FPlayerDataStoreGroup& DataStoreGroup = PlayerDataStores(PlayerDataIndex);
					DataStoreGroup.DataStores.AddItem(DataStore);

					bResult = TRUE;
				}
				else
				{
                    GlobalDataStores.AddItem(DataStore);
					bResult = TRUE;
				}

				// notify the data store that it has been registered
				DataStore->OnRegister(PlayerOwner);
			}
			else
			{
				debugf(TEXT("Failed to register data store (%s) '%s': existing data store with identical tag '%s'"), *DataStoreID.ToString(), *DataStore->GetFullName(), *ExistingDataStore->GetFullName());
			}
		}
		else
		{
			debugf(TEXT("Failed to register data store '%s': doesn't have a valid tag"), *DataStore->GetFullName());
		}
	}

	return bResult;
}

/**
 * Removes a data store from the GlobalDataStores array.
 *
 * @param	DataStore	the data store to remove
 *
 * @return	TRUE if the data store was successfully removed, or if the data store wasn't in the list.
 */
UBOOL UDataStoreClient::UnregisterDataStore( UUIDataStore* DataStore )
{
	UBOOL bResult = FALSE;

	if ( DataStore != NULL )
	{
		INT DataStoreIndex = GlobalDataStores.FindItemIndex(DataStore);

		// if this data store isn't in the list, treat as successful
		if ( DataStoreIndex == INDEX_NONE )
		{
			// search through the player data stores
			for ( INT PlayerDataIndex = 0; PlayerDataIndex < PlayerDataStores.Num(); PlayerDataIndex++ )
			{
				FPlayerDataStoreGroup& DataStoreGroup = PlayerDataStores(PlayerDataIndex);
				DataStoreIndex = DataStoreGroup.DataStores.FindItemIndex(DataStore);
				if ( DataStoreIndex != INDEX_NONE )
				{
					ULocalPlayer* PlayerOwner = DataStoreGroup.PlayerOwner;

					//@todo - perhaps add a hook to the UIDataStore class for overriding removal
					DataStoreGroup.DataStores.Remove(DataStoreIndex);

					// notify the data store that is has been unregistered
					DataStore->OnUnregister(PlayerOwner);

					// if this is the last data store associated with this player, remove the data store group as well
					if ( DataStoreGroup.DataStores.Num() == 0 )
					{
						PlayerDataStores.Remove(PlayerDataIndex);
					}

					bResult = TRUE;
					break;
				}
			}

			//@todo - hmm...should we return TRUE if the data store wasn't registered?
			bResult = TRUE;
		}
		else
		{
			//@todo - perhaps add a hook to the UIDataStore class for overriding removal
			GlobalDataStores.Remove(DataStoreIndex);

			// notify the data store that is has been unregistered
			DataStore->OnUnregister(NULL);
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Finds the index into the PlayerDataStores array for the data stores associated with the specified player.
 *
 * @param	PlayerOwner		the player to search for associated data stores for.
 */
INT UDataStoreClient::FindPlayerDataStoreIndex( ULocalPlayer* PlayerOwner ) const
{
	INT Result = INDEX_NONE;

	if ( GIsGame )
	{
		for ( INT DataStoreIndex = 0; DataStoreIndex < PlayerDataStores.Num(); DataStoreIndex++ )
		{
			const FPlayerDataStoreGroup& DataStoreGroup = PlayerDataStores(DataStoreIndex);
			if ( DataStoreGroup.PlayerOwner == PlayerOwner )
			{
				Result = DataStoreIndex;
				break;
			}
		}
	}
	else
	{
		if (PlayerDataStores.Num() > 0)
		{
			// Editor only ever has one player
			Result = 0;
		}
	}

	return Result;
}

/* ==========================================================================================================
	FUIDataProviderField
========================================================================================================== */
FUIDataProviderField::FUIDataProviderField( FName InFieldTag, EUIDataProviderFieldType InFieldType/*=DATATYPE_Property*/, UUIDataProvider* InFieldProvider/*=NULL*/ )
: FieldTag(InFieldTag), FieldType(InFieldType)
{
	if ( InFieldProvider != NULL )
	{
		check(InFieldType==DATATYPE_Provider);
		FieldProviders.AddItem(InFieldProvider);
	}
}

FUIDataProviderField::FUIDataProviderField( FName InFieldTag, const TArray<UUIDataProvider*>& InFieldProviders )
: FieldTag(InFieldTag), FieldType(DATATYPE_ProviderCollection), FieldProviders(InFieldProviders)
{
}

/**
 * Retrieves the list of providers contained by this data provider field.
 *
 * @return	FALSE if the FieldType for this provider field is not DATATYPE_Provider/ProviderCollection
 */
UBOOL FUIDataProviderField::GetProviders( TArray<UUIDataProvider*>& out_Providers ) const
{
	UBOOL bResult = FALSE;
	if ( FieldType == DATATYPE_Provider || FieldType == DATATYPE_ProviderCollection )
	{
		bResult = TRUE;
		out_Providers = FieldProviders;
	}
	return bResult;
}

/* ==========================================================================================================
	FUIProviderScriptFieldValue
========================================================================================================== */
UBOOL FUIProviderScriptFieldValue::HasValue() const
{
	UBOOL bResult = FALSE;

	switch ( PropertyType )
	{
	case DATATYPE_Property:
		bResult = StringValue.Len() > 0 || ImageValue != NULL;
		break;

	case DATATYPE_RangeProperty:
		bResult = RangeValue.HasValue();
		break;

	case DATATYPE_NetIdProperty:
		bResult = NetIdValue.HasValue();
		break;

	case DATATYPE_Collection:
	case DATATYPE_ProviderCollection:
		bResult = ArrayValue.Num() > 0;
		break;
	}

	return bResult;
}

/** Comparison operators */
UBOOL FUIProviderScriptFieldValue::operator==( const FUIProviderScriptFieldValue& Other ) const
{
	UBOOL bResult = FALSE;
	if ( PropertyTag == Other.PropertyTag )
	{
		switch ( PropertyType )
		{
		case DATATYPE_Property:
			if ( StringValue.Len() > 0 )
			{
				bResult = StringValue == Other.StringValue;
			}
			else if ( ImageValue != NULL )
			{
				bResult = (ImageValue == Other.ImageValue) && (AtlasCoordinates == Other.AtlasCoordinates);
			}
			else
			{
				bResult = Other.StringValue.Len() == 0 && Other.ImageValue == NULL;
			}
			break;

		case DATATYPE_RangeProperty:
			bResult = RangeValue == Other.RangeValue;
			break;

		case DATATYPE_NetIdProperty:
			bResult = NetIdValue == Other.NetIdValue;
			break;

		case DATATYPE_Collection:
		case DATATYPE_ProviderCollection:
			bResult = ArrayValue == Other.ArrayValue;
			break;

		default:
			// this happens if the UIProviderScriptFieldValue wasn't assigned a PropertyType
			if ( StringValue.Len() > 0 )
			{
				bResult = StringValue == Other.StringValue;
			}
			else if ( ImageValue != NULL )
			{
				bResult = ImageValue == Other.ImageValue && AtlasCoordinates == Other.AtlasCoordinates;
			}
			else if ( RangeValue.HasValue() )
			{
				bResult = RangeValue == Other.RangeValue;
			}
			else if ( NetIdValue.HasValue() )
			{
				bResult = NetIdValue == Other.NetIdValue;
			}
			else if ( ArrayValue.Num() > 0 )
			{
				bResult = ArrayValue == Other.ArrayValue;
			}
			else
			{
				bResult
					=	Other.StringValue.Len() == 0
					&&	Other.ImageValue == NULL
					&&	!Other.RangeValue.HasValue()
					&&	!Other.NetIdValue.HasValue()
					&&	Other.ArrayValue.Num() == 0;
			}

			break;
		}
	}

	return bResult;
}
UBOOL FUIProviderScriptFieldValue::operator!=( const FUIProviderScriptFieldValue& Other ) const
{
	return !(FUIProviderScriptFieldValue::operator==(Other));
}
UBOOL FUIProviderScriptFieldValue::operator==( const struct FUIProviderFieldValue& Other ) const
{
	return FUIProviderScriptFieldValue::operator==((const FUIProviderScriptFieldValue&)Other);
}
UBOOL FUIProviderScriptFieldValue::operator!=( const struct FUIProviderFieldValue& Other ) const
{
	return FUIProviderScriptFieldValue::operator!=((const FUIProviderScriptFieldValue&)Other);
}

UBOOL FUIProviderFieldValue::operator==( const struct FUIProviderScriptFieldValue& Other ) const
{
	return FUIProviderScriptFieldValue::operator==(Other);
}
UBOOL FUIProviderFieldValue::operator!=( const struct FUIProviderScriptFieldValue& Other ) const
{
	return FUIProviderScriptFieldValue::operator!=(Other);
}
UBOOL FUIProviderFieldValue::operator==( const struct FUIProviderFieldValue& Other ) const
{
	return	FUIProviderScriptFieldValue::operator==((const FUIProviderScriptFieldValue&)Other)
		&&	(CustomStringNode == NULL) == (Other.CustomStringNode == NULL);
}
UBOOL FUIProviderFieldValue::operator!=( const struct FUIProviderFieldValue& Other ) const
{
	return !(FUIProviderFieldValue::operator==(Other));
}



/* ==========================================================================================================
	UUIDataProvider
========================================================================================================== */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIDataProvider::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	eventGetSupportedScriptFields(out_Fields);
}

/**
 * Generates filler data for a given tag.  This is used by the editor to generate a preview that gives the
 * user an idea as to what a bound datastore will look like in-game.
 *
 * @param		DataTag		the tag corresponding to the data field that we want filler data for
 *
 * @return		a string of made-up data which is indicative of the typical values for the specified field.
 */
FString UUIDataProvider::GenerateFillerData( const FString& DataTag )
{
	return eventGenerateFillerData(DataTag);
}

/**
 * Determines whether publishing updated values to the specified field is allowed.
 *
 * @param	PropTag		the name of the field within this data provider to check access for (might be blank)
 * @param	ArrayIndex	optional array index for use with data collections.
 *
 * @return	TRUE if publishing updated values is allowed for the field.
 */
UBOOL UUIDataProvider::AllowsPublishingData( const FString& PropTag, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( WriteAccessType == ACCESS_WriteAll )
	{
		bResult = TRUE;
	}
	else if ( WriteAccessType == ACCESS_PerField )
	{
		bResult = eventAllowPublishingToField(PropTag, ArrayIndex);
	}

	return bResult;
}

/**
 * Parses the data store reference and resolves it into a value that can be used by the UI.
 *
 * @param	MarkupString	a markup string that can be resolved to a data field contained by this data provider, or one of its
 *							internal data providers.
 * @param	out_FieldValue	receives the value of the data field resolved from MarkupString.  If the specified property corresponds
 *							to a value that can be rendered as a string, the field value should be assigned to the StringValue member;
 *							if the specified property corresponds to a value that can only be rendered as an image, such as an object
 *							or image reference, the field value should be assigned to the ImageValue member.
 *							Data stores can optionally manually create a UIStringNode_Text or UIStringNode_Image containing the appropriate
 *							value, in order to have greater control over how the string node is initialized.  Generally, this is not necessary.
 *
 * @return	TRUE if this data store (or one of its internal data providers) successfully resolved the string specified into a data field
 *			and assigned the value to the out_FieldValue parameter; false if this data store could not resolve the markup string specified.
 */
UBOOL UUIDataProvider::GetDataStoreValue( const FString& MarkupString, FUIProviderFieldValue& out_FieldValue )
{
	UBOOL bResult = FALSE;

	UUIDataProvider* FieldOwner = NULL;
	FString FieldTag;
	INT ArrayIndex = INDEX_NONE;

	if ( ParseDataStoreReference(MarkupString, FieldOwner, FieldTag, ArrayIndex) )
	{
		bResult = FieldOwner && FieldOwner->GetFieldValue(FieldTag, out_FieldValue, ArrayIndex);
	}

	return bResult;
}

/**
 * Parses the data store reference and publishes the value specified to that location.
 *
 * @param	MarkupString	a markup string that can be resolved to a data field contained by this data provider, or one of its
 *							internal data providers.
 * @param	out_FieldValue	contains the value that should be published to the location specified by MarkupString.
 *
 * @return	TRUE if this data store (or one of its internal data providers) successfully resolved the string specified into a data field
 *			and assigned the value to the out_FieldValue parameter; false if this data store could not resolve the markup string specified.
 */
UBOOL UUIDataProvider::SetDataStoreValue( const FString& MarkupString, const FUIProviderScriptFieldValue& FieldValue )
{
	UBOOL bResult = FALSE;

	UUIDataProvider* FieldOwner = NULL;
	FString FieldTag;
	INT ArrayIndex = INDEX_NONE;

	if ( ParseDataStoreReference(MarkupString, FieldOwner, FieldTag, ArrayIndex) )
	{
		if ( FieldOwner && FieldOwner->AllowsPublishingData(FieldTag, ArrayIndex) )
		{
			bResult = FieldOwner->SetFieldValue(FieldTag, FieldValue, ArrayIndex);
		}
	}

	return bResult;
}

/**
 * Determines if the specified data tag is supported by this data provider
 *
 * @param	DataTag		the tag corresponding to the data field that we want to check for
 *
 * @return	TRUE if the data tag specified is supported by this data provider.
 */
UBOOL UUIDataProvider::IsDataTagSupported( FName DataTag )
{
	TArray<FUIDataProviderField> SupportedFields;
	return IsDataTagSupported(DataTag, SupportedFields);
}

/**
 * Determines if the specified data tag is supported by this data provider
 *
 * @param	DataTag		a tag corresponding to the data field that we want to check for; ok for the tag to contain array indexes
 * @param	SupportedFields		the collection of fields to search through; if empty, will automatically fill in the array by calling
 *								GetSupportedFields; useful optimization when calling this method repeatedly, e.g. in a loop
 *
 * @return	TRUE if the data tag specified is supported by this data provider.
 */
UBOOL UUIDataProvider::IsDataTagSupported( FName DataTag, TArray<FUIDataProviderField>& SupportedFields )
{
	UBOOL bResult = FALSE;

	if ( SupportedFields.Num() == 0 )
	{
		GetSupportedDataFields(SupportedFields);
	}

	// first, search prior to stripping off any array indices (in case this data store reports its data fields with array indices in this)
	for ( INT FieldIndex = 0; FieldIndex < SupportedFields.Num(); FieldIndex++ )
	{
		if ( SupportedFields(FieldIndex).FieldTag == DataTag )
		{
			bResult = TRUE;
			break;
		}
	}

	if ( !bResult )
	{
		FString DataTagString = DataTag.ToString();

		// strip off any array indices
		ParseArrayDelimiter(DataTagString);

		// now convert back to FName
		DataTag = *DataTagString;
		for ( INT FieldIndex = 0; FieldIndex < SupportedFields.Num(); FieldIndex++ )
		{
			if ( SupportedFields(FieldIndex).FieldTag == DataTag )
			{
				bResult = TRUE;
				break;
			}
		}
	}

	if ( !bResult )
	{
		// just in case we're also a cell value provider, and DataTag corresponds to a cell tag, search our supported cell tags as well
		IUIListElementCellProvider* CellValueProvider = InterfaceCast<IUIListElementCellProvider>(this);
		if ( CellValueProvider != NULL )
		{
			TMap<FName,FString> CellTags;
			CellValueProvider->GetElementCellTags( UCONST_UnknownCellDataFieldName, CellTags );
			if ( CellTags.Find(DataTag) != NULL )
			{
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/**
 * Returns whether the specified provider is contained by this data provider.
 *
 * @param	Provider			the provider to search for
 * @param	out_ProviderOwner	will contain the UIDataProvider that contains the field tag which corresonds to the
 *								Provider being searched for.
 *
 * @return	TRUE if Provider can be accessed through this data provider.
 */
UBOOL UUIDataProvider::ContainsProvider( UUIDataProvider* Provider, UUIDataProvider*& out_ProviderOwner )
{
	UBOOL bResult = FALSE;

	// first, check whether the specified provider is our default data provider
	UUIDataProvider* DefaultDataProvider = GetDefaultDataProvider();
	if ( DefaultDataProvider != this && Provider == DefaultDataProvider )
	{
		bResult = TRUE;
		out_ProviderOwner = this;
	}
	else
	{
		// get the list of fields supported by this data provider
		TArray<FUIDataProviderField> SupportedFields;
		GetSupportedDataFields(SupportedFields);

		for ( INT FieldIndex = 0; FieldIndex < SupportedFields.Num(); FieldIndex++ )
		{
			// for each field that contains provider references...
			FUIDataProviderField& ProviderField = SupportedFields(FieldIndex);

			// get the providers associated with this field 
			TArray<UUIDataProvider*> InternalProviders;
			if ( ProviderField.GetProviders(InternalProviders) )
			{
				// see if any of these providers contain the provider we're searching for...
				for ( INT ProviderIndex = 0; ProviderIndex < InternalProviders.Num(); ProviderIndex++ )
				{
					UUIDataProvider* InternalProvider = InternalProviders(ProviderIndex);
					if ( Provider == InternalProvider )
					{
						bResult = TRUE;
						out_ProviderOwner = this;
					}
					else if ( InternalProvider->ContainsProvider(Provider, out_ProviderOwner) )
					{
						bResult = TRUE;
						break;
					}
				}
			}
		}
	}

	return bResult;
}

/**
 * Builds the data store path name necessary to access the specified tag of this data provider.
 *
 * @param	ContainerDataStore	the data store to use as the starting point for building the data field path
 * @param	DataTag				the tag corresponding to the data field that we want a path to
 *
 * @return		a string containing the complete path name required to access the specified data field
 */
FString UUIDataProvider::BuildDataFieldPath( UUIDataStore* ContainerDataStore, const FName& DataTag )
{
	FString DataProviderPath;

	if ( IsDataTagSupported(DataTag) )
	{
		if ( ContainerDataStore != this )
		{
			UUIDataProvider* OwnerProvider = NULL;
			if ( ContainerDataStore->ContainsProvider(this, OwnerProvider) )
			{
				OwnerProvider->GetPathToProvider(ContainerDataStore, this, DataProviderPath);
			}
		}
		else
		{
			// we are the ContainerDataStore, so use a different delimiter
			DataProviderPath = FString::Printf(TEXT("%s:"), *ContainerDataStore->GetDataStoreID().ToString());
		}

		DataProviderPath += DataTag.ToString();
	}

	return DataProviderPath;
}

/**
 * Generates a data store path to the specified provider.
 *
 * @param	ContainerDataStore	the data store to use as the starting point for building the data field path
 * @param	Provider			the data store provider to generate a path name to
 * @param	out_DataStorePath	will be filled in with the path name necessary to access the specified provider, 
 *								including any trailing dots or colons
 */
void UUIDataProvider::GetPathToProvider( UUIDataStore* ContainerDataStore, UUIDataProvider* Provider, FString& out_DataStorePath )
{
	FName ProviderTag = GetProviderDataTag(Provider);
	if ( ProviderTag != NAME_None )
	{
		out_DataStorePath = BuildDataFieldPath(ContainerDataStore,ProviderTag) + TEXT(".");
	}
	else
	{
		UUIDataProvider* DefaultDataProvider = GetDefaultDataProvider();
		if ( DefaultDataProvider != this && DefaultDataProvider == Provider )
		{
			// no tag is required to access this provider, but if we are the owning data store, we'll need to append the data store tag here
			if ( ContainerDataStore == this )
			{
				// we are the ContainerDataStore, so use a different delimiter
				out_DataStorePath = ContainerDataStore->GetDataStoreID().ToString() + TEXT(":");
			}
		}
	}
}

/**
 * Returns the data tag associated with the specified provider.
 *
 * @return	the data field tag associated with the provider specified, or NAME_None if the provider specified is not
 *			contained by this data store.
 */
FName UUIDataProvider::GetProviderDataTag( UUIDataProvider* Provider )
{
	FName ProviderTag = NAME_None;

	TArray<FUIDataProviderField> SupportedFields;
	GetSupportedDataFields(SupportedFields);
	for ( INT FieldIndex = 0; FieldIndex < SupportedFields.Num(); FieldIndex++ )
	{
		// for each field that contains provider references...
		FUIDataProviderField& ProviderField = SupportedFields(FieldIndex);

		// get the providers associated with this field 
		TArray<UUIDataProvider*> InternalProviders;
		if ( ProviderField.GetProviders(InternalProviders) )
		{
			INT ProviderIndex = InternalProviders.FindItemIndex(Provider);
			if ( ProviderIndex != INDEX_NONE )
			{
				FString TagString = ProviderField.FieldTag.ToString();
				if ( ProviderField.FieldType == DATATYPE_ProviderCollection )
				{
					// if this is a collection of providers, we'll need to append an array index indicator
					TagString += *FString::Printf(ARRAY_DELIMITER TEXT("%i"), ProviderIndex);
				}

				ProviderTag = *TagString;
				break;
			}
		}
	}

	return ProviderTag;
}


/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<IUIListElementProvider> UUIDataProvider::ResolveListElementProvider( const FString& PropertyName )
{
	TScriptInterface<IUIListElementProvider> Result;

	IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(this);
	if ( ElementProvider != NULL && IsDataTagSupported(*PropertyName) )
	{
		Result.SetObject(this);
		Result.SetInterface(ElementProvider);
	}

	return Result;
}


/**
 * Retrieves the field type for the specified field
 *
 * @param	FieldName				the tag corresponding to the data field that we want the field type for
 * @param	out_ProviderFieldType	will receive the fieldtype for the data field specified; if DataTag isn't supported
 *									by this provider, this value will not be modified.
 *
 * @return	TRUE if the field specified is supported and out_ProviderFieldType was changed to the correct type.
 */
UBOOL UUIDataProvider::GetProviderFieldType( const FString& FieldName, /*EUIDataProviderFieldType*/BYTE& out_ProviderFieldType )
{
	UBOOL bResult = FALSE;

	TArray<FUIDataProviderField> SupportedFields;
	GetSupportedDataFields(SupportedFields);

	// strip off any array indexes
	FString DataTag(FieldName);
	ParseArrayDelimiter(DataTag);
	for ( INT FieldIndex = 0; FieldIndex < SupportedFields.Num(); FieldIndex++ )
	{
		if ( SupportedFields(FieldIndex).FieldTag == *DataTag )
		{
			out_ProviderFieldType = /*(EUIDataProviderFieldType)*/SupportedFields(FieldIndex).FieldType;
			bResult = TRUE;
			break;
		}
	}

	if ( bResult == FALSE )
	{
		// just in case we're also a cell value provider, and DataTag corresponds to a cell tag, 
		IUIListElementCellProvider* CellValueProvider = InterfaceCast<IUIListElementCellProvider>(this);
		if ( CellValueProvider != NULL )
		{
			BYTE FieldType;
			if ( CellValueProvider->GetCellFieldType(UCONST_UnknownCellDataFieldName, *DataTag, FieldType) )
			{
				out_ProviderFieldType = /*(EUIDataProviderFieldType)*/FieldType;
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/**
 * Generates a data store markup string which can be used to access the data field specified.
 *
 * @param	ContainerDataStore	the data store to use as the starting point for building the markup string
 * @param	DataTag				the data field tag to generate the markup string for
 *
 * @return	a datastore markup string which resolves to the datastore field associated with DataTag, in the format:
 *			<DataStoreTag:DataFieldTag>
 */
FString UUIDataProvider::GenerateDataMarkupString( UUIDataStore* ContainerDataStore, FName DataTag )
{
	FString DataFieldPath = BuildDataFieldPath(ContainerDataStore,DataTag);
	if ( DataFieldPath.Len() > 0 )
	{
		return FString::Printf(TEXT("<%s>"), *DataFieldPath);
	}

	return eventGenerateScriptMarkupString(DataTag);
}

/**
 * Builds the data store path name necessary to access the specified tag of this data provider.
 *
 * This is a bulk version of the BuildDataFieldPath function.
 *
 * @param	ContainerDataStore	the data store to use as the starting point for building the data field path
 * @param	DataTags			Array of tags to build paths for, ALL of these tags are assumed to be under the same data provider.
 * @param	Out_Paths			Array of generated paths, one for each of the datatags passed in.
 */
void UUIDataProvider::BuildDataFieldPath( class UUIDataStore* ContainerDataStore, const TArray<FName>& DataTags, TArray<FString> &Out_Paths )
{
	Out_Paths.Empty();

	if(DataTags.Num())
	{
		// Get the path for the first tag and use the same path for all of the other tags.
		const FName &FirstTag = DataTags(0);

		if ( IsDataTagSupported(FirstTag) )
		{
			FString DataProviderPath;

			if ( ContainerDataStore != this )
			{
				UUIDataProvider* OwnerProvider = NULL;
				if ( ContainerDataStore->ContainsProvider(this, OwnerProvider) )
				{
					OwnerProvider->GetPathToProvider(ContainerDataStore, this, DataProviderPath);
				}
			}
			else
			{
				// we are the ContainerDataStore, so use a different delimiter
				DataProviderPath = FString::Printf(TEXT("%s:"), *ContainerDataStore->GetDataStoreID().ToString());
			}

			// Loop through all datatags and append each of the datatags provided to the generated path.
			Out_Paths.AddZeroed(DataTags.Num());
			for(INT TagIdx=0; TagIdx<DataTags.Num();TagIdx++)
			{
				const FName &Tag = DataTags(TagIdx);
				FString &Path = Out_Paths(TagIdx);
				Path = DataProviderPath + Tag.ToString();
			}
		}
	}
}


/**
 * Generates a data store markup string which can be used to access the data field specified.
 *
 * This is a bulk version of the GenerateDataMarkupString function.
 *
 * @param	ContainerDataStore	the data store to use as the starting point for building the markup string
 * @param	DataTags			array of tags to generate the markup string for, ALL of these tags are assumed to be under the same data provider.
 * @param	Out_Markup			Array of strings of generated markup, one for each tag passed in. 
 */
void UUIDataProvider::GenerateDataMarkupString( class UUIDataStore* ContainerDataStore, const TArray<FName>& DataTags, TArray<FString> &Out_Markup )
{
	UBOOL bGeneratedPaths = FALSE;

	Out_Markup.Empty();

	// Try to build the markup using the full path for the tag first.
	BuildDataFieldPath(ContainerDataStore, DataTags, Out_Markup);

	if ( Out_Markup.Num() > 0 )
	{
		check(Out_Markup.Num()==DataTags.Num());

		// Loop through all of the returned markup strings and add markup braces <> around them.
		for(INT TagIdx=0; TagIdx<Out_Markup.Num();TagIdx++)
		{
			FString &TagPath = Out_Markup(TagIdx);

			TagPath = FString::Printf(TEXT("<%s>"), *TagPath);
		}

		bGeneratedPaths = TRUE;
	}

	// Since we were unable to build a datafield path, manually generate tags using a script event for each of the array items.
	if(bGeneratedPaths == FALSE)
	{
		Out_Markup.Add(DataTags.Num());

		for(INT TagIdx=0; TagIdx<DataTags.Num();TagIdx++)
		{
			const FName &Tag = DataTags(TagIdx);
			FString &Markup = Out_Markup(TagIdx);
			Markup = eventGenerateScriptMarkupString(Tag);
		}
	}
}

/**
 * Parses the specified markup string to get the data tag that should be evaluated by this data provider.
 *
 * @param	MarkupString	a string that contains a markup reference (either in whole, or in part), e.g. CurrentGame:Players;1.PlayerName.
 *							if successfully parsed, after parsing,
 * @param	out_NextDataTag	a string representing the data tag for the next portion of the data store path reference, including any
 *							any array delimiters.
 *
 * @return	TRUE if the a data tag was successfully parsed.
 */
UBOOL UUIDataProvider::ParseNextDataTag( FString& MarkupString, FString& out_NextDataTag ) const
{
	UBOOL bResult = FALSE;

	// first, search for a data store delimiter
	INT DelimiterPosition = MarkupString.InStr(TEXT(":"));
	if ( DelimiterPosition == INDEX_NONE )
	{
		// the next tag doesn't correspond to a data store tag....see if it corresponds to a data provider reference
		DelimiterPosition = MarkupString.InStr(TEXT("."));
	}

	if ( DelimiterPosition != INDEX_NONE )
	{
		// found a delimiter - remove the part on the left side of the delimiter and copy it to out_NextDataTag
		out_NextDataTag = MarkupString.Left(DelimiterPosition);

		// now remove that part of the string from the source MarkupString.
		MarkupString = MarkupString.Mid(DelimiterPosition+1);

		// indicate success
		bResult = TRUE;
	}
	else
	{
		// no delimiters in the MarkupString, so just move the whole thing over to out_NextDataTag
		out_NextDataTag = MarkupString;
		MarkupString.Empty();
	}

	return bResult;
}

/**
 * Parses the string specified, separating the array index portion from the data field tag.
 *
 * @param	DataTag		the data tag that possibly contains an array index
 *
 * @return	the array index that was parsed from DataTag, or INDEX_NONE if there was no array index in the string specified.
 */
INT UUIDataProvider::ParseArrayDelimiter( FString& DataTag ) const
{
	INT Result = INDEX_NONE;

	INT DelimiterLocation = DataTag.InStr(ARRAY_DELIMITER);
	if ( DelimiterLocation != INDEX_NONE )
	{
		// convert the array portion to an INT
		Result = appAtoi( *DataTag.Mid(DelimiterLocation+1) );
	
		// then remove the array portion, leaving only the data tag
		DataTag = DataTag.Left(DelimiterLocation);
	}

	return Result;
}

/**
 * Parses the data store reference and resolves the data provider and field that is referenced by the markup.
 *
 * @param	MarkupString	a markup string that can be resolved to a data field contained by this data provider, or one of its
 *							internal data providers.
 * @param	out_FieldOwner	receives the value of the data provider that owns the field referenced by the markup string.
 * @param	out_FieldTag	receives the value of the property or field referenced by the markup string.
 * @param	out_ArrayIndex	receives the optional array index for the data field referenced by the markup string.  If there is no array index embedded in the markup string,
 *							value will be INDEX_NONE.
 *
 * @return	TRUE if this data store was able to successfully resolve the string specified.
 */
UBOOL UUIDataProvider::ParseDataStoreReference( const FString& MarkupString, UUIDataProvider*& out_FieldOwner, FString& out_FieldTag, INT& out_ArrayIndex )
{
	UBOOL bResult = FALSE;

	// extract the part of the markup string which corresponds to a data field supported by this data provider
	FString ParsedDataTag = MarkupString, ProviderTag;
	if ( ParseNextDataTag(ParsedDataTag, ProviderTag) )
	{
		// ProviderTag should correspond to a provider contained within this data provider.
		// see if ProviderTag contains an array index
		INT ArrayIndex = ParseArrayDelimiter(ProviderTag);

		TArray<FUIDataProviderField> SupportedFields;
		GetSupportedDataFields(SupportedFields);

		for ( INT FieldIndex = 0; FieldIndex < SupportedFields.Num(); FieldIndex++ )
		{
			FUIDataProviderField& ProviderField = SupportedFields(FieldIndex);
			if ( ProviderField.FieldTag == *ProviderTag )
			{
				TArray<UUIDataProvider*> Providers;
				if ( ProviderField.GetProviders(Providers) )
				{
					if ( ArrayIndex == INDEX_NONE )
					{
						ArrayIndex = 0;
					}

					if ( Providers.IsValidIndex(ArrayIndex) && Providers(ArrayIndex) != NULL )
					{
						bResult = Providers(ArrayIndex)->ParseDataStoreReference(ParsedDataTag, out_FieldOwner, out_FieldTag, out_ArrayIndex);
					}
				}

				break;
			}
		}
	}
	else if ( ProviderTag.Len() > 0 )
	{
		out_FieldOwner = GetDefaultDataProvider();

		out_ArrayIndex = ParseArrayDelimiter(ProviderTag);
		out_FieldTag = ProviderTag;

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Encapsulates the construction of a UITexture wrapper for the specified USurface.
 *
 * @param	SourceImage		the texture or material instance to apply to the newly created wrapper
 *
 * @return	a pointer to a UITexture instance which wraps the SourceImage passed in, or NULL if SourceImage was invalid
 *			or the wrapper couldn't be created for some reason.
 */
UUITexture* UUIDataProvider::CreateTextureWrapper( USurface* SourceImage )
{
	UUITexture* Result = ConstructObject<UUITexture>(UUITexture::StaticClass(), INVALID_OBJECT, NAME_None, RF_Transient);
	Result->ImageTexture = SourceImage;

	return Result;
}

/**
 * Creates the appropriate type of string node for the NodeValue specified.  If NodeValue.CustomStringNode is set, returns
 * that node; If NodeValue.StringValue is set, creates and initializes a UIStringNode_Text; if NodeValue.ImageValue is
 * set, creates and initializes a UIStringNode_Image.
 *
 * @param	SourceText	the text to assign as the SourceText in the returned node
 * @param	NodeValue	the value to use for initiailizing the string node that is returned
 *
 * @return	a pointer to either a UIStringNode of the appropriate type (Text or Image) that has been initialized from the
 *			NodeValue specified.  The caller is responsible for cleaning up the memory for this return value.
 */
FUIStringNode* UUIDataProvider::CreateStringNode( const FString& SourceText, const FUIProviderFieldValue& NodeValue )
{
	FUIStringNode* Result = NULL;

	// if there is a CustomStringNode set in NodeValue, return that node instead
	if ( NodeValue.CustomStringNode != NULL )
	{
		Result = NodeValue.CustomStringNode;
	}
	else if ( SourceText.Len() > 0 )
	{
		// if a string value is set, create a text node.
		if ( NodeValue.PropertyType == DATATYPE_RangeProperty && NodeValue.RangeValue.HasValue() )
		{
			FUIStringNode_Text* TextNode = new FUIStringNode_Text(*SourceText);
			if ( NodeValue.RangeValue.bIntRange )
			{
				TextNode->SetRenderText( *FString::Printf(TEXT("%i"), appTrunc(NodeValue.RangeValue.GetCurrentValue())) );
			}
			else
			{
				TextNode->SetRenderText( *FString::Printf(TEXT("%.2f"), NodeValue.RangeValue.GetCurrentValue()) );
			}

			Result = TextNode;
		}
		else if ( NodeValue.PropertyType == DATATYPE_NetIdProperty && NodeValue.NetIdValue.HasValue() )
		{
			FUIStringNode_Text* TextNode = new FUIStringNode_Text(*SourceText);
			TextNode->SetRenderText(*UOnlineSubsystem::UniqueNetIdToString(NodeValue.NetIdValue));

			Result = TextNode;
		}
		else
		{
			if ( NodeValue.ImageValue != NULL )
			{
				UUITexture* ImageWrapper = CreateTextureWrapper(NodeValue.ImageValue);
				if ( ImageWrapper != NULL )
				{
					FUIStringNode_Image* ImageNode = new FUIStringNode_Image(*SourceText);

					ImageNode->RenderedImage = ImageWrapper;
					ImageNode->ForcedExtent.X = 0.f;
					ImageNode->ForcedExtent.Y = 0.f;

					Result = ImageNode;
				}
				else
				{
					debugf(TEXT("UUIDataProvider::CreateStringNode unable to create UITexture for surface '%s' SourceText '%s'"), *NodeValue.ImageValue->GetFullName(), *SourceText);
				}
			}
			
			else if ( NodeValue.StringValue.Len() > 0 )
			{
				FUIStringNode_Text* TextNode = new FUIStringNode_Text(*SourceText);
				TextNode->SetRenderText(*NodeValue.StringValue);

				Result = TextNode;
			}
			else
			{
				Result = new FUIStringNode_Text(*SourceText);
			}
		}
	}
	else
	{
		debugf(TEXT("UUIDataProvider::CreateStringNode called with invalid SourceText - NodeValue.PropertyTag: %s"), *NodeValue.PropertyTag.ToString());
	}

	return Result;
}

/* ==========================================================================================================
	UUIDataStore
========================================================================================================== */
/**
 * Hook for performing any initialization required for this data store
 */
void UUIDataStore::InitializeDataStore()
{
	// nothing
}

/**
 * Called when this data store is added to the data store manager's list of active data stores.
 *
 * @param	PlayerOwner		the player that will be associated with this DataStore.  Only relevant if this data store is
 *							associated with a particular player; NULL if this is a global data store.
 */
void UUIDataStore::OnRegister( ULocalPlayer* PlayerOwner )
{
	eventRegistered(PlayerOwner);
}

/**
 * Called when this data store is removed from the data store manager's list of active data stores.
 *
 * @param	PlayerOwner		the player that will be associated with this DataStore.  Only relevant if this data store is
 *							associated with a particular player; NULL if this is a global data store.
 */
void UUIDataStore::OnUnregister( ULocalPlayer* PlayerOwner )
{
	eventUnregistered(PlayerOwner);
}

/**
 * Notifies the data store that all values bound to this data store in the current scene have been saved.  Provides data stores which
 * perform buffered or batched data transactions with a way to determine when the UI system has finished writing data to the data store.
 *
 * @note: for now, this lives in UIDataStore, but it might make sense to move it up to UIDataProvider later on.
 */
void UUIDataStore::OnCommit()
{
}


/* ==========================================================================================================
	UUIDataStore_Strings
========================================================================================================== */
/**
 * Creates an UIConfigFileProvider instance for the loc file specified by FilePathName.
 *
 * @return	a pointer to a newly allocated UUIConfigFileProvider instance that contains the data for the specified
 *			loc file.
 */
UUIConfigFileProvider* UUIDataStore_Strings::CreateLocProvider( const FFilename& FilePathName )
{
	UUIConfigFileProvider* Result = NULL;

	FConfigCacheIni* ConfigCache = (FConfigCacheIni*)GConfig;
	FConfigFile* ConfigFile = ConfigCache->Find(*FilePathName, FALSE);
	if ( ConfigFile != NULL )
	{
		FString FileName = FilePathName.GetBaseFilename();
		Result = ConstructObject<UUIConfigFileProvider>(UUIConfigFileProvider::StaticClass(), this, *FileName);
		Result->ConfigFileName = FilePathName;
		Result->InitializeProvider(ConfigFile);
	}

	return Result;
}

/**
 * Loads all .int files and creates UIConfigProviders for each loc file that was loaded.
 */
void UUIDataStore_Strings::InitializeDataStore()
{
	Super::InitializeDataStore();

	// Errors during early loading can cause Localize to be called before GConfig is initialized.
	if( GSys == NULL || GIsStarted == FALSE || GConfig == NULL || GIsGame )
	{
		return;
	}

	const TCHAR* LangExt = UObject::GetLanguage();

	if( GUseSeekFreeLoading )
	{
		// with seek free loading, the GConfig already has all the files it will ever have
		// so just get the list of them
		TArray<FFilename> ConfigFilenames;
		GConfig->GetConfigFilenames(ConfigFilenames);

		// If no files were found using the language specified, then use INT as a fallback.
		UBOOL bFoundFiles = FALSE;

		for(INT TryIndex=0; TryIndex<2 && !bFoundFiles; TryIndex++)
		{
			for (INT FileIndex = 0; FileIndex < ConfigFilenames.Num(); FileIndex++)
			{
				FFilename& Filename = ConfigFilenames(FileIndex);

				// only use config files ending in the language extendion (localization files)
				if (Filename.GetExtension() == LangExt)
				{
					// create a UIConfigFileProvider for this loc file
					UUIConfigFileProvider* Provider = CreateLocProvider(Filename);
					if (Provider != NULL)
					{
						LocFileProviders.AddUniqueItem(Provider);
					}

					bFoundFiles = TRUE;
				}
			}

			if(bFoundFiles == FALSE)
			{
				LangExt = TEXT("INT");
			}
		}
	}
	else
	{
		FString Wildcard = FString::Printf(TEXT("*.%s"), LangExt);

		// We allow various .inis to contribute multiple paths for localization files.
		for( INT PathIndex=0; PathIndex < GSys->LocalizationPaths.Num(); PathIndex++ )
		{
			FString LocPath = FString::Printf(TEXT("%s") PATH_SEPARATOR TEXT("%s"), *GSys->LocalizationPaths(PathIndex), LangExt);
			FFilename SearchPath = FString::Printf(TEXT("%s") PATH_SEPARATOR TEXT("*.%s"), *LocPath, LangExt);

			TArray<FString> LocFileNames;
			GFileManager->FindFiles(LocFileNames, *SearchPath, TRUE, FALSE);

			for ( INT FileIndex = 0; FileIndex < LocFileNames.Num(); FileIndex++ )
			{
				FString& Filename = LocFileNames(FileIndex);

				// create a UIConfigFileProvider for this loc file
				UUIConfigFileProvider* Provider = CreateLocProvider(LocPath * Filename);
				if ( Provider != NULL )
				{
					LocFileProviders.AddUniqueItem(Provider);
				}
			}
		}
	}
}

/**
 * Resolves the value of the localized string specified and stores it in the output parameter.
 *
 * @param	FieldName		the path name to the localized string property value to retrieve
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_Strings::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	TArray<FString> Parts;
	FieldName.ParseIntoArray(&Parts, TEXT("."), TRUE);

	if ( Parts.Num() >= 3 )
	{
		out_FieldValue.PropertyType = DATATYPE_Property;

		out_FieldValue.StringValue = Localize(*Parts(1), *Parts(2), *Parts(0), NULL);
		bResult = out_FieldValue.StringValue.Left(2) != TEXT("<?");
	}

	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/**
 * Generates filler data for a given tag.  This is used by the editor to generate a preview that gives the
 * user an idea as to what a bound datastore will look like in-game.
 *
 * @param		DataTag		the tag corresponding to the data field that we want filler data for
 *
 * @return		a string of made-up data which is indicative of the typical [resolved] value for the specified field.
 */
FString UUIDataStore_Strings::GenerateFillerData( const FString& DataTag )
{
	return TEXT("The quick brown fox jumped over the lazy black dog.");
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIDataStore_Strings::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();

	for ( INT FileIndex = 0; FileIndex < LocFileProviders.Num(); FileIndex++ )
	{
		UUIConfigFileProvider* FileProvider = LocFileProviders(FileIndex);

		new(out_Fields) FUIDataProviderField( *FileProvider->ConfigFileName.GetBaseFilename(), DATATYPE_Provider, FileProvider );
	}

	Super::GetSupportedDataFields(out_Fields);
}

/* ==========================================================================================================
	UUIDataStore_Images
========================================================================================================== */
/**
 * Generates a UIStringNode_Image containing the image referenced by ImagePathName
 *
 * @param	ImageParameterText	A string corresponding to a path name for an image resource (e.g. EngineResources.White),
 *								along with optional formatting data.  The image path name and formatting data should be
 *								delimited with a semi-colon; each formatting field should be delimited with commas, and
 *								should use the syntax: FieldName=FieldValue
 *								the supported formatting fields are:
 *								XL:	if specified, the horizontal size of the image will be set to this value
 *								YL: if specified, the veritcal size of the image will be set to this value
 * @param	out_FieldValue		receives the resolved value for the property specified.
 *								@see GetDataStoreValue for additional notes
 * @param	ArrayIndex			optional array index for use with data collections
 */
UBOOL UUIDataStore_Images::GetFieldValue( const FString& ImageParameterText, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( ImageParameterText.Len() > 0 )
	{
		FString ImagePathName, ImageParams;

		//@fixme - this is temporary; the preferred way of doing this is to add a tag
		// to UIStringNode that can be used to uniquely identify each tag so that the
		// owning string can apply any special formatting to nodes created by data stores
		if ( !ImageParameterText.Split(TEXT(";"), &ImagePathName, &ImageParams) )
		{
			ImagePathName = ImageParameterText;
		}

		FTextureCoordinates ForcedTexCoords(EC_EventParm);
		FLOAT ImageWidth=0.f, ImageHeight=0.f;
		if ( ImageParams.Len() > 0 )
		{
			Parse(*ImageParameterText, TEXT("XL="), ImageWidth);
			Parse(*ImageParameterText, TEXT("YL="), ImageHeight);
			Parse(*ImageParameterText, TEXT("U="), ForcedTexCoords.U);
			Parse(*ImageParameterText, TEXT("UL="), ForcedTexCoords.UL);
			Parse(*ImageParameterText, TEXT("V="), ForcedTexCoords.V);
			Parse(*ImageParameterText, TEXT("VL="), ForcedTexCoords.VL);
		}


		UUITexture* ResolvedImage = NULL;
		USurface* ImageResource = LoadObject<USurface>(NULL, *ImagePathName, NULL, LOAD_None, NULL);
		if ( ImageResource == NULL )
		{
			ResolvedImage = FindObject<UUITexture>(ANY_PACKAGE, *ImagePathName);
			if ( ResolvedImage != NULL )
			{
				ImageResource = ResolvedImage->GetSurface();
			}
		}

		if ( ImageResource != NULL )
		{
			if ( ResolvedImage == NULL )
			{
				ResolvedImage = CreateTextureWrapper(ImageResource);
			}

			if ( ResolvedImage != NULL )
			{
				FUIStringNode_Image* ImageNode = new FUIStringNode_Image(*ImagePathName);

				ImageNode->RenderedImage = ResolvedImage;
				ImageNode->RenderedImage->ImageTexture = ImageResource;
				ImageNode->ForcedExtent.X = ImageWidth;
				ImageNode->ForcedExtent.Y = ImageHeight;

				ImageNode->TexCoords = ForcedTexCoords;

				out_FieldValue.PropertyTag = ImageResource->GetFName();
				out_FieldValue.PropertyType = DATATYPE_Property;
				out_FieldValue.CustomStringNode = ImageNode;
				out_FieldValue.ImageValue = ImageResource;
				out_FieldValue.AtlasCoordinates = ForcedTexCoords;

				bResult = TRUE;
			}
		}
	}

	return bResult || Super::GetFieldValue(ImageParameterText, out_FieldValue, ArrayIndex);
}

/* ==========================================================================================================
	UUIDataStore_Fonts
========================================================================================================== */
/**
 * Gets the list of font names available through this data store.
 *
 * @param	out_Fields	will be filled in with the list of font names that can be accessed through this data store.
 */
void UUIDataStore_Fonts::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();

	// TObjectIterator doesn't visit CDOs, so no need to worry about those here
	for ( TObjectIterator<UFont> It; It; ++It )
	{
		//@todo - should we store the fonts in a persistent list and use the name of the font instead?
		new(out_Fields) FUIDataProviderField(*It->GetPathName(), DATATYPE_Property);
	}

	// allow script child classes access to our fields
	Super::GetSupportedDataFields(out_Fields);
}

/**
 * Attempst to load the font specified and if successful changes the style data's DrawFont.
 *
 * @param	MarkupString	a string corresponding to the name of a font.
 * @param	StyleData		the style data to apply the changes to.
 *
 * @return	TRUE if a font was found matching the specified FontName, FALSE otherwise.
 */
UBOOL UUIDataStore_Fonts::ParseStringModifier( const FString& FontName, FUIStringNodeModifier& StyleData )
{
	UBOOL bResult = FALSE;

	if ( FontName.Len() > 0 )
	{
		if ( FontName == TEXT("/") )
		{
			// remove the current font from the string customizer; this will change the current font to which font was previously active
			if ( !StyleData.RemoveFont() )
			{
				//@todo - for some reason, the top-most font couldn't be removed (probably only one font its FontStack, indicating that we have
				// a closing font tag without the corresonding opening font tag.....should we complain about this??
			}

			bResult = TRUE;
		}
		else
		{
			UFont* NewFont = LoadObject<UFont>(NULL, *FontName, NULL, LOAD_None, NULL);
			if ( NewFont != NULL )
			{
				StyleData.AddFont(NewFont);
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/* ==========================================================================================================
	UUIDataStore_Color
========================================================================================================== */
/**
 * Attempst to change the color attributes fo markup
 *
 * @param	ColorParams	a string corresponding to the name of a font.
 * @param	StyleData		the style data to apply the changes to.
 *
 * @return	TRUE if a font was found matching the specified FontName, FALSE otherwise.
 */
UBOOL UUIDataStore_Color::ParseStringModifier( const FString& ColorParams, FUIStringNodeModifier& StyleData )
{
	if ( ColorParams == TEXT("/") )
	{
		StyleData.ClearCustomTextColor();
	}
	else
	{
		FLinearColor CurrentTextColor = StyleData.GetCustomTextColor();
		FLOAT R=0, G=0, B=0, A=0;

		if ( Parse(*ColorParams,TEXT("R="),R) )
		{
			CurrentTextColor.R = R;
		}

		if ( Parse(*ColorParams,TEXT("G="),G) )
		{
			CurrentTextColor.G = G;
		}

		if ( Parse(*ColorParams,TEXT("B="),B) )
		{
			CurrentTextColor.B = B;
		}

		if ( Parse(*ColorParams,TEXT("A="),A) )
		{
			CurrentTextColor.A = A;
		}

		StyleData.SetCustomTextColor(CurrentTextColor);
	}

	return TRUE;
}


/* ==========================================================================================================
	USceneDataStore
========================================================================================================== */
/**
 * Creates the data provider for this scene data store.
 */
void USceneDataStore::InitializeDataStore()
{
	Super::InitializeDataStore();

	if ( SceneDataProvider == NULL )
	{
		SceneDataProvider = ConstructObject<UUIDynamicFieldProvider>(UUIDynamicFieldProvider::StaticClass(), this);
	}

	check(SceneDataProvider);
	SceneDataProvider->InitializeRuntimeFields();
}


/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<IUIListElementProvider> USceneDataStore::ResolveListElementProvider( const FString& PropertyName )
{
	return TScriptInterface<IUIListElementProvider>(this);
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see ParseDataStoreReference for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL USceneDataStore::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( SceneDataProvider != NULL )
	{
		bResult = SceneDataProvider->GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
	}

	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL USceneDataStore::SetFieldValue( const FString& FieldName, const FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( SceneDataProvider != NULL)
	{
		bResult = SceneDataProvider->SetFieldValue(FieldName, FieldValue, ArrayIndex);
	}

	return bResult || Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void USceneDataStore::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();
	check(OwnerScene);

	UUIScene* ParentScene = OwnerScene->GetPreviousScene();
	if ( ParentScene != NULL )
	{
		USceneDataStore* ParentSceneDataStore = ParentScene->GetSceneDataStore();
		if ( ParentSceneDataStore != NULL )
		{
			new(out_Fields) FUIDataProviderField(TEXT("ParentScene"), DATATYPE_Provider, ParentSceneDataStore);
		}
	}

	if ( SceneDataProvider != NULL )
	{
		SceneDataProvider->GetSupportedDataFields(out_Fields);
	}
}

/**
 * Returns a pointer to the data provider which provides the tags for this data provider.  Normally, that will be this data provider,
 * but for some data stores such as the Scene data store, data is pulled from an internal provider but the data fields are presented as
 * though they are fields of the data store itself.
 */
UUIDataProvider* USceneDataStore::GetDefaultDataProvider()
{
	if ( SceneDataProvider != NULL )
	{
		return SceneDataProvider;
	}

	return this;
}


/** === IUIListElementProviderInterface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> USceneDataStore::GetElementProviderTags()
{
	TArray<FName> Result;
	Result.AddItem(TEXT("ContextMenuItems"));

	return Result;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT USceneDataStore::GetElementCount( FName FieldName )
{
	INT Result = 0;
	// is there ever used?
	if ( SceneDataProvider != NULL )
	{
// 		DynamicCollectionValueMap CollectionDataSourceMap;
// 		SceneDataProvider->GetCollectionValueArray(FieldName, CollectionDataSourceMap);
// 
// 		TMultiMap<FName,FString>* pCollectionDataValues = CollectionDataSourceMap.Find(FieldName);
// 		if ( pCollectionDataValues != NULL )
// 		{
// 			TLookupMap<FName> UniqueKeys;
// 			Result = pCollectionDataValues->Num(UniqueKeys);
// 		}
	}
	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL USceneDataStore::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	if ( SceneDataProvider != NULL )
	{
		FString ActualFieldName(FieldName.ToString()), DataFieldName;
		if ( !ParseNextDataTag(ActualFieldName, DataFieldName) )
		{
			ActualFieldName = DataFieldName;
		}

		TArray<FString> ElementValues;
		if ( SceneDataProvider->GetCollectionValueArray(FName(*ActualFieldName), ElementValues) )
		{
			for ( INT ValueIndex = 0; ValueIndex < ElementValues.Num(); ValueIndex++ )
			{
				out_Elements.AddItem(ValueIndex);
			}
		}
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data field that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> USceneDataStore::GetElementCellSchemaProvider( FName FieldName )
{
	FString ActualFieldName(FieldName.ToString()), DataFieldName;
	if ( ParseNextDataTag(ActualFieldName, DataFieldName)
	&&	(DataFieldName == TEXT("ContextMenuItems") || ActualFieldName == TEXT("ContextMenuItems")) )
	{
		return this;
	}

	return TScriptInterface<IUIListElementCellProvider>();
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> USceneDataStore::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	FString ActualFieldName(FieldName.ToString()), DataFieldName;
	ParseNextDataTag(ActualFieldName, DataFieldName);

	if ( DataFieldName == TEXT("ContextMenuItems") )
	{
		return this;
	}

	return TScriptInterface<IUIListElementCellProvider>();
}


/* === IUIListElementCellProvider === */
/**
 * Retrieves the list of tags that can be bound to individual cells in a single list element, along with the human-readable,
 * localized string that should be used in the header for each cell tag (in lists which have column headers enabled).
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
 */
void USceneDataStore::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	if ( SceneDataProvider != NULL )
	{
		// the values we set here will be passed to RefreshElementSchema
		out_CellTags.Set(TEXT("ContextMenuItems"), TEXT("ContextMenuItems"));
	}
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL USceneDataStore::GetCellFieldType( FName FieldName, const FName& CellTag, BYTE& out_CellFieldType )
{
	UBOOL bResult = FALSE;

	if ( SceneDataProvider != NULL )
	{
		if ( CellTag == TEXT("ContextMenuItems") )
		{
			out_CellFieldType = DATATYPE_Collection;
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL USceneDataStore::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( SceneDataProvider != NULL )
	{
		FString CellTagString(CellTag.ToString()), FieldNameString;
		if ( !ParseNextDataTag(CellTagString, FieldNameString) )
		{
			CellTagString = FieldNameString;
		}

		if ( CellTagString.Len() > 0 )
		{
			FString MenuItem;
			FName CellTagName(*CellTagString);
			if ( SceneDataProvider->GetCollectionValue(CellTagName, ListIndex, MenuItem) )
			{
				out_FieldValue.PropertyTag = CellTagName;
				out_FieldValue.PropertyType = DATATYPE_Property;
				out_FieldValue.StringValue = MenuItem;
				bResult = TRUE;
			}
		}
 	}

	return bResult;
}

/* ==========================================================================================================
	UUIDataStore_Registry
========================================================================================================== */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param   out_Fields   will be filled in with the list of tags which can be used to access data in this data provider.
 *                  Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIDataStore_Registry::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
   // data stores empty the array, data providers append to the array
   out_Fields.Empty();
   
   // to collapse the data provider, route the call directly to it rather than adding a DATATYPE_Provider element to the out_Fields array
   if ( RegistryDataProvider != NULL )
   {
      RegistryDataProvider->GetSupportedDataFields(out_Fields);
   }


   // allow script-only child classes to add to the list of fields
   Super::GetSupportedDataFields(out_Fields);
}

/**
 * Returns a pointer to the data provider which provides the tags for this data provider.  Normally, that will be this data provider,
 * but in this data store, the data fields are pulled from an internal provider but presented as though they are fields of the data store itself.
 */
UUIDataProvider* UUIDataStore_Registry::GetDefaultDataProvider()
{
	return this;
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see ParseDataStoreReference for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_Registry::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( RegistryDataProvider != NULL )
	{
		bResult = RegistryDataProvider->GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
	}

	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_Registry::SetFieldValue( const FString& FieldName, const FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( RegistryDataProvider != NULL)
	{
		// Make sure the field we are setting exists
		RegistryDataProvider->AddField(*FieldName,FieldValue.PropertyType,FALSE);

		// Set the value for the field
		bResult = RegistryDataProvider->SetFieldValue(FieldName, FieldValue, ArrayIndex);
	}

	return bResult || Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);
}

/**
 * Creates the data provider for this registry data store.
 */
void UUIDataStore_Registry::InitializeDataStore()
{
	Super::InitializeDataStore();

	if ( RegistryDataProvider == NULL )
	{
		// since UIDynamicFieldProvider is PerObjectConfig, we must provide a name in the call to ConstructObject to guarantee that any data previously
		// saved to the .ini by this data store is loaded correctly.
		RegistryDataProvider = ConstructObject<UUIDynamicFieldProvider>(UUIDynamicFieldProvider::StaticClass(), this, TEXT("UIRegistryDataProvider"));
	}

	check(RegistryDataProvider);

	// now tell the data provider to populate its runtime array of data fields with the fields that any previously saved fields loaded from the .ini
	RegistryDataProvider->InitializeRuntimeFields();
}

/**
 * Notifies the data store that all values bound to this data store in the current scene have been saved.  Provides data stores which
 * perform buffered or batched data transactions with a way to determine when the UI system has finished writing data to the data store.
 */
void UUIDataStore_Registry::OnCommit()
{
	Super::OnCommit();

	// the UI system (or whoever is using this data store) is finished publishing data values - time to send everything to persistent storage
	if ( RegistryDataProvider != NULL )
	{
		// since the RegistryDataProvider stores its data in the .ini, sending data to persisent storage for this provider means calling SaveConfig().
		RegistryDataProvider->SavePersistentProviderData();
	}
}

/* ==========================================================================================================
	UUIConfigFileProvider
========================================================================================================== */
/**
 * Initializes this config file provider, creating the section data providers for each of the sections contained
 * within the ConfigFile specified.
 *
 * @param	ConfigFile	the config file to associated with this data provider
 */
void UUIConfigFileProvider::InitializeProvider( FConfigFile* ConfigFile )
{
	if ( ConfigFile != NULL )
	{
		Sections.Empty();

		for ( TMap<FString,FConfigSection>::TIterator It(*ConfigFile); It ; ++It )
		{
			const FString& SectionName = It.Key();
			UUIConfigSectionProvider* SectionProvider = ConstructObject<UUIConfigSectionProvider>(UUIConfigSectionProvider::StaticClass(), this, *SectionName);
			if ( SectionProvider != NULL )
			{
				SectionProvider->SectionName = SectionName;
				Sections.AddItem(SectionProvider);
			}
		}
	}
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIConfigFileProvider::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	//@todo
	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIConfigFileProvider::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();
	for ( INT SectionIndex = 0; SectionIndex < Sections.Num(); SectionIndex++ )
	{
		UUIConfigSectionProvider* Provider = Sections(SectionIndex);

		new(out_Fields) FUIDataProviderField(*Provider->SectionName, DATATYPE_Provider, Provider);
	}

	Super::GetSupportedDataFields(out_Fields);
}

/* ==========================================================================================================
	UUIConfigSectionProvider
========================================================================================================== */
/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIConfigSectionProvider::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	//@todo: Not sure if this is the best way to get the value.
	if ( SectionName.Len() )
	{
		FFilename& ConfigFileName = GetOuterUUIConfigFileProvider()->ConfigFileName;
		if ( ConfigFileName.Len() )
		{
			FConfigFile* ConfigFile = GConfig->FindConfigFile(*ConfigFileName);
			if ( ConfigFile != NULL )
			{
				FConfigSection* ConfigSection = ConfigFile->Find(*SectionName);
				if ( ConfigSection != NULL )
				{
					FString* Value = ConfigSection->Find(FieldName);

					if(Value)
					{
						out_FieldValue.PropertyTag = *FieldName;
						out_FieldValue.StringValue = *Value;
						bResult = TRUE;
					}
				}
			}
		}
	}

	
	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIConfigSectionProvider::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();
	if ( SectionName.Len() )
	{
		FFilename& ConfigFileName = GetOuterUUIConfigFileProvider()->ConfigFileName;
		if ( ConfigFileName.Len() )
		{
			FConfigFile* ConfigFile = GConfig->FindConfigFile(*ConfigFileName);
			if ( ConfigFile != NULL )
			{
				FConfigSection* ConfigSection = ConfigFile->Find(*SectionName);
				if ( ConfigSection != NULL )
				{
					// get the keys contained by this section
					TLookupMap<FString> SectionKeys;
					ConfigSection->GetKeys(SectionKeys);

					// copy those keys into the output variable
					for ( INT KeyIndex = 0; KeyIndex < SectionKeys.Num(); KeyIndex++ )
					{
						const FString& KeyString = SectionKeys(KeyIndex);
						EUIDataProviderFieldType DataType = ConfigSection->Num(KeyString) > 1
							? DATATYPE_Collection
							: DATATYPE_Property;
						
						new(out_Fields) FUIDataProviderField(*KeyString,DataType);
					}
				}
			}
		}
	}

	Super::GetSupportedDataFields(out_Fields);
}

/* ==========================================================================================================
	UUIDataStore_InputAlias
========================================================================================================== */
IMPLEMENT_COMPARE_CONSTREF(FUIDataStoreInputAlias, UnUIDataStores, { return appStricmp(*A.AliasName.ToString(), *B.AliasName.ToString()); })

/* === UUIDataStore_InputAlias interface === */
/**
 * Populates the InputAliasLookupMap based on the elements of the InputAliases array.
 */
void UUIDataStore_InputAlias::InitializeLookupMap()
{
	if ( InputAliases.Num() > 0 )
	{
		Sort<USE_COMPARE_CONSTREF(FUIDataStoreInputAlias,UnUIDataStores)>(&InputAliases(0), InputAliases.Num());
	}

	// remove all existing elements
	InputAliasLookupMap.Empty(InputAliases.Num());

	for ( INT AliasIdx = 0; AliasIdx < InputAliases.Num(); AliasIdx++ )
	{
		FUIDataStoreInputAlias& Alias = InputAliases(AliasIdx);
		InputAliasLookupMap.Set(Alias.AliasName, AliasIdx);
	}
}
	
/**
 * @return	the platform that should be used (by default) when retrieving data associated with input aliases
 */
EInputPlatformType UUIDataStore_InputAlias::GetDefaultPlatform() const
{
	//@todo ronp - if the owning player is using an alternate input device, it may affect which platform we return
#if XBOX
	EInputPlatformType Platform = IPT_360;
#elif PS3
	EInputPlatformType Platform = IPT_PS3;
#else
	EInputPlatformType Platform = IPT_PC;
#endif

	return Platform;
}

/**
 * Retrieves the button icon font markup string for an input alias
 *
 * @param	DesiredAlias		the name of the alias (i.e. Accept) to get the markup string for
 * @param	OverridePlatform	specifies which platform's markup string is desired; if not specified, uses the current
 *								platform, taking into account whether the player is using a gamepad (PC) or a keyboard (console).
 *
 * @return	the markup string for the button icon associated with the alias.
 */
FString UUIDataStore_InputAlias::GetAliasFontMarkup( FName DesiredAlias, /*EInputPlatformType*/BYTE OverridePlatform/*=IPT_MAX*/ ) const
{
	FString Result;

	INT AliasIdx = FindInputAliasIndex(DesiredAlias);
	if ( InputAliases.IsValidIndex(AliasIdx) )
	{
		const FUIDataStoreInputAlias& Alias = InputAliases(AliasIdx);

		EInputPlatformType Platform = GetDefaultPlatform();
		if ( OverridePlatform < IPT_MAX )
		{
			Platform = static_cast<EInputPlatformType>(OverridePlatform);
		}

		check(Platform<ARRAY_COUNT(Alias.PlatformInputKeys));
		Result = Alias.PlatformInputKeys[Platform].ButtonFontMarkupString;
	}

	return Result;
}

/**
 * Retrieves the button icon font markup string for an input alias
 *
 * @param	AliasIndex			the index [into the InputAliases array] for the alias to get the markup string for.
 * @param	OverridePlatform	specifies which platform's markup string is desired; if not specified, uses the current
 *								platform, taking into account whether the player is using a gamepad (PC) or a keyboard (console).
 *
 * @return	the markup string for the button icon associated with the alias.
 */
FString UUIDataStore_InputAlias::GetAliasFontMarkupByIndex( INT AliasIndex, /*EInputPlatformType*/BYTE OverridePlatform/*=IPT_MAX*/ ) const
{
	FString Result;

	if ( InputAliases.IsValidIndex(AliasIndex) )
	{
		const FUIDataStoreInputAlias& Alias = InputAliases(AliasIndex);

		EInputPlatformType Platform = GetDefaultPlatform();
		if ( OverridePlatform < IPT_MAX )
		{
			Platform = static_cast<EInputPlatformType>(OverridePlatform);
		}

		check(Platform<ARRAY_COUNT(Alias.PlatformInputKeys));
		Result = Alias.PlatformInputKeys[Platform].ButtonFontMarkupString;
	}

	return Result;
}

/**
 * Retrieves the associated input key name for an input alias
 *
 * @param	AliasIndex			the index [into the InputAliases array] for the alias to get the input key for.
 * @param	OverridePlatform	specifies which platform's input key is desired; if not specified, uses the current
 *								platform, taking into account whether the player is using a gamepad (PC) or a keyboard (console).
 *
 * @return	the name of the input key (i.e. LeftMouseButton) which triggers the alias.
 */
FName UUIDataStore_InputAlias::GetAliasInputKeyName( FName DesiredAlias, /*EInputPlatformType*/BYTE OverridePlatform/*=IPT_MAX*/ ) const
{
	FName Result = NAME_None;

	INT AliasIdx = FindInputAliasIndex(DesiredAlias);
	if ( InputAliases.IsValidIndex(AliasIdx) )
	{
		const FUIDataStoreInputAlias& Alias = InputAliases(AliasIdx);

		EInputPlatformType Platform = GetDefaultPlatform();
		if ( OverridePlatform < IPT_MAX )
		{
			Platform = static_cast<EInputPlatformType>(OverridePlatform);
		}

		check(Platform<ARRAY_COUNT(Alias.PlatformInputKeys));
		Result = Alias.PlatformInputKeys[Platform].InputKeyData.InputKeyName;
	}

	return Result;
}

/**
 * Retrieves the associated input key name for an input alias
 *
 * @param	AliasIndex			the index [into the InputAliases array] for the alias to get the input key for.
 * @param	OverridePlatform	specifies which platform's markup string is desired; if not specified, uses the current
 *								platform, taking into account whether the player is using a gamepad (PC) or a keyboard (console).
 *
 * @return	the name of the input key (i.e. LeftMouseButton) which triggers the alias.
 */
FName UUIDataStore_InputAlias::GetAliasInputKeyNameByIndex( INT AliasIndex, /*EInputPlatformType*/BYTE OverridePlatform/*=IPT_MAX*/ ) const
{
	FName Result = NAME_None;

	if ( InputAliases.IsValidIndex(AliasIndex) )
	{
		const FUIDataStoreInputAlias& Alias = InputAliases(AliasIndex);

		EInputPlatformType Platform = GetDefaultPlatform();
		if ( OverridePlatform < IPT_MAX )
		{
			Platform = static_cast<EInputPlatformType>(OverridePlatform);
		}

		check(Platform<ARRAY_COUNT(Alias.PlatformInputKeys));
		Result = Alias.PlatformInputKeys[Platform].InputKeyData.InputKeyName;
	}

	return Result;
}

/**
 * Retrieves both the input key name and modifier keys for an input alias
 *
 * @param	DesiredAlias		the name of the alias (i.e. Accept) to get the input key data for
 * @param	OverridePlatform	specifies which platform's markup string is desired; if not specified, uses the current
 *								platform, taking into account whether the player is using a gamepad (PC) or a keyboard (console).
 *
 * @return	the struct containing the input key name and modifier keys associated with the alias.
 */
UBOOL UUIDataStore_InputAlias::GetAliasInputKeyData( FRawInputKeyEventData& out_InputKeyData, FName DesiredAlias, /*EInputPlatformType*/BYTE OverridePlatform/*=IPT_MAX*/ ) const
{
	UBOOL bResult = FALSE;

	INT AliasIdx = FindInputAliasIndex(DesiredAlias);
	if ( InputAliases.IsValidIndex(AliasIdx) )
	{
		const FUIDataStoreInputAlias& Alias = InputAliases(AliasIdx);

		EInputPlatformType Platform = GetDefaultPlatform();
		if ( OverridePlatform < IPT_MAX )
		{
			Platform = static_cast<EInputPlatformType>(OverridePlatform);
		}

		check(Platform<ARRAY_COUNT(Alias.PlatformInputKeys));
		out_InputKeyData = Alias.PlatformInputKeys[Platform].InputKeyData;
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Retrieves both the input key name and modifier keys for an input alias
 *
 * @param	AliasIndex			the index [into the InputAliases array] for the alias to get the input key data for.
 * @param	OverridePlatform	specifies which platform's markup string is desired; if not specified, uses the current
 *								platform, taking into account whether the player is using a gamepad (PC) or a keyboard (console).
 *
 * @return	the struct containing the input key name and modifier keys associated with the alias.
 */
UBOOL UUIDataStore_InputAlias::GetAliasInputKeyDataByIndex( FRawInputKeyEventData& out_InputKeyData, INT AliasIndex, /*EInputPlatformType*/BYTE OverridePlatform/*=IPT_MAX*/ ) const
{
	UBOOL bResult = FALSE;

	if ( InputAliases.IsValidIndex(AliasIndex) )
	{
		const FUIDataStoreInputAlias& Alias = InputAliases(AliasIndex);

		EInputPlatformType Platform = GetDefaultPlatform();
		if ( OverridePlatform < IPT_MAX )
		{
			Platform = static_cast<EInputPlatformType>(OverridePlatform);
		}

		check(Platform<ARRAY_COUNT(Alias.PlatformInputKeys));
		out_InputKeyData = Alias.PlatformInputKeys[Platform].InputKeyData;
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Finds the location [in the InputAliases array] for an input alias.
 *
 * @param	DesiredAlias	the name of the alias (i.e. Accept) to find
 *
 * @return	the index into the InputAliases array for the alias, or INDEX_NONE if it doesn't exist.
 */
INT UUIDataStore_InputAlias::FindInputAliasIndex( FName DesiredAlias ) const
{
	INT Result = INDEX_NONE;

	if ( DesiredAlias != NAME_None )
	{
		const INT* pIdx = InputAliasLookupMap.Find(DesiredAlias);
		if ( pIdx != NULL )
		{
			Result = *pIdx;
		}
	}

	return Result;
}

/**
 * Determines whether an input alias is supported on a particular platform.
 *
 * @param	DesiredAlias		the name of the alias (i.e. Accept) to check
 * @param	DesiredPlatform		the platform to check for an input key
 *
 * @return	TRUE if the alias has a corresponding input key for the specified platform.
 */
UBOOL UUIDataStore_InputAlias::HasAliasMappingForPlatform( FName DesiredAlias, /*EInputPlatformType*/BYTE DesiredPlatform ) const
{
	UBOOL bResult = FALSE;

	INT AliasIdx = FindInputAliasIndex(DesiredAlias);
	if ( InputAliases.IsValidIndex(AliasIdx) && DesiredPlatform < IPT_MAX )
	{
		const FUIDataStoreInputAlias& Alias = InputAliases(AliasIdx);
		bResult = Alias.PlatformInputKeys[DesiredPlatform].InputKeyData.InputKeyName != NAME_None;
	}

	return bResult;
}

/* === UUIDataStore interface === */
/**
 * Hook for performing any initialization required for this data store.
 *
 * This version builds the InputAliasLookupMap based on the elements in the InputAliases array.
 */
void UUIDataStore_InputAlias::InitializeDataStore()
{
	Super::InitializeDataStore();

	InitializeLookupMap();
}

/* === UUIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider
 *
 * @param OutFields Filled in with the list of fields supported by its aggregated providers
 */
void UUIDataStore_InputAlias::GetSupportedDataFields( TArray<FUIDataProviderField>& OutFields )
{
	OutFields.Empty(InputAliases.Num());

	for ( INT AliasIdx = 0; AliasIdx < InputAliases.Num(); AliasIdx++ )
	{
		FUIDataStoreInputAlias& Alias = InputAliases(AliasIdx);

		new(OutFields) FUIDataProviderField(Alias.AliasName, DATATYPE_Property);
	}

	Super::GetSupportedDataFields(OutFields);
}

/**
 * Gets the value for the specified field
 *
 * @param	FieldName		the field to look up the value for
 * @param	OutFieldValue	out param getting the value
 * @param	ArrayIndex		ignored
 */
UBOOL UUIDataStore_InputAlias::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = TRUE;

	BYTE OverridePlatform = IPT_MAX;
	if ( ArrayIndex >= 0 && ArrayIndex < IPT_MAX )
	{
		OverridePlatform = (BYTE)ArrayIndex;
	}
	OutFieldValue.StringValue = GetAliasFontMarkup(FName(*FieldName), OverridePlatform);
	if ( OutFieldValue.StringValue.Len() == 0 )
	{
		bResult = Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
	}

	return bResult;
}

/* ==========================================================================================================
	UUIDataStore_GameResource
========================================================================================================== */
/**
 * Finds the index for the GameResourceDataProvider with a tag matching ProviderTag.
 *
 * @return	the index into the ElementProviderTypes array for the GameResourceDataProvider element that has the
 *			tag specified, or INDEX_NONE if there are no elements of the ElementProviderTypes array that have that tag.
 */
INT UUIDataStore_GameResource::FindProviderTypeIndex( FName ProviderTag ) const
{
	INT Result = INDEX_NONE;

	for ( INT ProviderIndex = 0; ProviderIndex < ElementProviderTypes.Num(); ProviderIndex++ )
	{
		const FGameResourceDataProvider& Provider = ElementProviderTypes(ProviderIndex);
		if ( Provider.ProviderTag == ProviderTag )
		{
			Result = ProviderIndex;
			break;
		}
	}

	return Result;
}

	
/**
 * Generates a tag containing the provider type with the name of an instance of that provider as the array delimiter.
 *
 * @param	ProviderIndex	the index into the ElementProviderTypes array for the provider's type
 * @param	InstanceIndex	the index into that type's list of providers for the target instance
 *
 * @return	a name containing the provider type name (i.e. the value of GameResourceDataProvider.ProviderTag) with
 *			the instance's name as the array delimiter.
 */
FName UUIDataStore_GameResource::GenerateProviderAccessTag( INT ProviderIndex, INT InstanceIndex ) const
{
	FName Result;

	if ( ElementProviderTypes.IsValidIndex(ProviderIndex) )
	{
		const FGameResourceDataProvider& Provider = ElementProviderTypes(ProviderIndex);

		TArray<UUIResourceDataProvider*> ProviderInstances;
		ListElementProviders.MultiFind(Provider.ProviderTag, ProviderInstances);

		if ( ProviderInstances.IsValidIndex(InstanceIndex) )
		{
			UUIResourceDataProvider* ProviderInstance = ProviderInstances(InstanceIndex);
			Result = FName(*FString::Printf(TEXT("%s;%s"), *Provider.ProviderTag.ToString(), *ProviderInstance->GetName()));
		}
	}

	return Result;
}

/**
 * Get the number of UIResourceDataProvider instances associated with the specified tag.
 *
 * @param	ProviderTag		the tag to find instances for; should match the ProviderTag value of an element in the ElementProviderTypes array.
 *
 * @return	the number of instances registered for ProviderTag.
 */
INT UUIDataStore_GameResource::GetProviderCount( FName ProviderTag ) const
{
	return ListElementProviders.Num(ProviderTag);
}

/**
 * Get the UIResourceDataProvider instances associated with the tag.
 *
 * @param	ProviderTag		the tag to find instances for; should match the ProviderTag value of an element in the ElementProviderTypes array.
 * @param	out_Providers	receives the list of provider instances. this array is always emptied first.
 *
 * @return	the list of UIResourceDataProvider instances registered for ProviderTag.
 */
UBOOL UUIDataStore_GameResource::GetResourceProviders( FName ProviderTag, TArray<UUIResourceDataProvider*>& out_Providers ) const
{
	out_Providers.Empty();
	ListElementProviders.MultiFind(ProviderTag, out_Providers);
	// we may support this data provider type, but there just may not be any instances created, so check our
	// list of providers as well
	return out_Providers.Num() > 0 || FindProviderTypeIndex(ProviderTag) != INDEX_NONE;
}

/**
 * Get the list of fields supported by the provider type specified.
 *
 * @param	ProviderTag			the name of the provider type to get fields for; should match the ProviderTag value of an element in the ElementProviderTypes array.
 *								If the provider type is expanded (bExpandProviders=true), this tag should also contain the array index of the provider instance
 *								to use for retrieving the fields (this can usually be automated by calling GenerateProviderAccessTag)
 * @param	ProviderFieldTags	receives the list of tags supported by the provider specified.
 *
 * @return	TRUE if the tag was resolved successfully and the list of tags was retrieved (doesn't guarantee that the provider
 *			array will contain any elements, though).  FALSE if the data store couldn't resolve the ProviderTag.
 */
UBOOL UUIDataStore_GameResource::GetResourceProviderFields( FName ProviderTag, TArray<FName>& ProviderFieldTags ) const
{
	UBOOL bResult = FALSE;

	ProviderFieldTags.Empty();
	TScriptInterface<IUIListElementCellProvider> SchemaProvider = ResolveProviderReference(ProviderTag);
	if ( SchemaProvider )
	{
		// fill the output array with this provider's schema
		TMap<FName,FString> ProviderFieldTagColumnPairs;
		SchemaProvider->GetElementCellTags(ProviderTag, ProviderFieldTagColumnPairs);
		
		ProviderFieldTagColumnPairs.GenerateKeyArray(ProviderFieldTags);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Get the value of a single field in a specific resource provider instance. Example: GetProviderFieldValue('GameTypes', ClassName, 2, FieldValue)
 *
 * @param	ProviderTag		the name of the provider type; should match the ProviderTag value of an element in the ElementProviderTypes array.
 * @param	SearchField		the name of the field within the provider type to get the value for; should be one of the elements retrieved from a call
 *							to GetResourceProviderFields.
 * @param	ProviderIndex	the index [into the array of providers associated with the specified tag] of the instance to get the value from;
 *							should be one of the elements retrieved by calling GetResourceProviders().
 * @param	out_FieldValue	receives the value of the field
 *
 * @return	TRUE if the field value was successfully retrieved from the provider.  FALSE if the provider tag couldn't be resolved,
 *			the index was not a valid index for the list of providers, or the search field didn't exist in that provider.
 */
UBOOL UUIDataStore_GameResource::GetProviderFieldValue( FName ProviderTag, FName SearchField, INT ProviderIndex, FUIProviderScriptFieldValue& out_FieldValue ) const
{
	UBOOL bResult = FALSE;

	UUIResourceDataProvider* FieldValueProvider = NULL;

	if ( ProviderIndex == INDEX_NONE )
	{
		FieldValueProvider = ResolveProviderReference(ProviderTag, &ProviderIndex);
	}
	else
	{
		TArray<UUIResourceDataProvider*> ProviderInstances;
		ListElementProviders.MultiFind(ProviderTag, ProviderInstances);

		if ( ProviderInstances.IsValidIndex(ProviderIndex) )
		{
			FieldValueProvider = ProviderInstances(ProviderIndex);
		}
	}

	if ( FieldValueProvider != NULL )
	{
		FUIProviderFieldValue FieldValue(EC_EventParm);
		if ( FieldValueProvider->GetCellFieldValue(ProviderTag, SearchField, ProviderIndex, FieldValue, INDEX_NONE) )
		{
			out_FieldValue = FieldValue;
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Searches for resource provider instance that has a field with a value that matches the value specified.
 *
 * @param	ProviderTag			the name of the provider type; should match the ProviderTag value of an element in the ElementProviderTypes array.
 * @param	SearchField			the name of the field within the provider type to compare the value to; should be one of the elements retrieved from a call
 *								to GetResourceProviderFields.
 * @param	ValueToSearchFor	the field value to search for.
 *
 * @return	the index of the resource provider instance that has the same value for SearchField as the value specified, or INDEX_NONE if the
 *			provider tag couldn't be resolved,  none of the provider instances had a field with that name, or none of them had a field
 *			of that name with the value specified.
 */
INT UUIDataStore_GameResource::FindProviderIndexByFieldValue( FName ProviderTag, FName SearchField, const FUIProviderScriptFieldValue& ValueToSearchFor ) const
{
	INT Result = INDEX_NONE;

	TArray<UUIResourceDataProvider*> ProviderInstances;
	ListElementProviders.MultiFind(ProviderTag, ProviderInstances);

	FUIProviderFieldValue FieldValue(EC_EventParm);
	for ( INT ProviderIndex = 0; ProviderIndex < ProviderInstances.Num(); ProviderIndex++ )
	{
		UUIResourceDataProvider* Provider = ProviderInstances(ProviderIndex);
		if ( Provider->GetCellFieldValue(SearchField, SearchField, ProviderIndex, FieldValue, INDEX_NONE) )
		{
			if ( FieldValue == ValueToSearchFor )
			{
				Result = ProviderIndex;
				break;
			}
		}
	}

	return Result;
}

/* === UUIDataStore_GameResource interface === */
/**
 * Finds or creates the UIResourceDataProvider instances referenced by ElementProviderTypes, and stores the result
 * into the ListElementProvider map.
 */
void UUIDataStore_GameResource::InitializeListElementProviders()
{
	ListElementProviders.Empty();

	// for each configured provider type, retrieve the list of ini sections that contain data for that provider class
	for ( INT ProviderTypeIndex = 0; ProviderTypeIndex < ElementProviderTypes.Num(); ProviderTypeIndex++ )
	{
		FGameResourceDataProvider& ProviderType = ElementProviderTypes(ProviderTypeIndex);

		UClass* ProviderClass = ProviderType.ProviderClass;

		TArray<FString> GameTypeResourceSectionNames;
		if ( GConfig->GetPerObjectConfigSections(*ProviderClass->GetConfigName(), *ProviderClass->GetName(), GameTypeResourceSectionNames) )
		{
			for ( INT SectionIndex = 0; SectionIndex < GameTypeResourceSectionNames.Num(); SectionIndex++ )
			{
				INT POCDelimiterPosition = GameTypeResourceSectionNames(SectionIndex).InStr(TEXT(" "));
				// we shouldn't be here if the name was included in the list
				check(POCDelimiterPosition!=INDEX_NONE);

				FName ObjectName = *GameTypeResourceSectionNames(SectionIndex).Left(POCDelimiterPosition);
				if ( ObjectName != NAME_None )
				{
					UUIResourceDataProvider* Provider = Cast<UUIResourceDataProvider>( StaticFindObject(ProviderClass, ANY_PACKAGE, *ObjectName.ToString(), TRUE) );
					if ( Provider == NULL )
					{
						Provider = ConstructObject<UUIResourceDataProvider>(
							ProviderClass,
							this,
							ObjectName
						);
					}

					if ( Provider != NULL )
					{
						ListElementProviders.Add(ProviderType.ProviderTag, Provider);
					}
				}
			}
		}
	}

	for ( TMultiMap<FName,UUIResourceDataProvider*>::TIterator It(ListElementProviders); It; ++It )
	{
		UUIResourceDataProvider* Provider = It.Value();
		Provider->eventInitializeProvider(!GIsGame);
	}
}
	
/**
 * Finds the data provider associated with the tag specified.
 *
 * @param	ProviderTag		The tag of the provider to find.  Must match the ProviderTag value for one of elements
 *							in the ElementProviderTypes array, though it can contain an array index (in which case
 *							the array index will be removed from the ProviderTag value passed in).
 * @param	InstanceIndex	If ProviderTag contains an array index, this will be set to the array index value that was parsed.
 *
 * @return	a data provider instance (or CDO if no array index was included in ProviderTag) for the element provider
 *			type associated with ProviderTag.
 */
UUIResourceDataProvider* UUIDataStore_GameResource::ResolveProviderReference( FName& ProviderTag, INT* InstanceIndex/*=NULL*/ ) const
{
	UUIResourceDataProvider* Result = NULL;

	// the most accurate way to determine if this is a supported type
	INT ProviderTypeIndex = FindProviderTypeIndex(ProviderTag);
	if ( ElementProviderTypes.IsValidIndex(ProviderTypeIndex) )
	{
		const FGameResourceDataProvider& ProviderType = ElementProviderTypes(ProviderTypeIndex);
		check(ProviderType.ProviderClass);

		Result = ProviderType.ProviderClass->GetDefaultObject<UUIResourceDataProvider>();
	}
	else
	{
		// it's possible that we didn't find it because the ProviderTag contains an array index; this is usually the case if the provider type
		// has bExpandProviders = TRUE - in this, each instance of the provider may contain a different schema
		FString ProviderTagString = ProviderTag.ToString();
		INT ProviderInstanceIndex = ParseArrayDelimiter(ProviderTagString);
		if ( ProviderInstanceIndex != INDEX_NONE )
		{
			// remove the array index from the ProviderTag
			ProviderTag = *ProviderTagString;
			TArray<UUIResourceDataProvider*> ProviderInstances;
			ListElementProviders.MultiFind(ProviderTag, ProviderInstances);

			if ( ProviderInstances.IsValidIndex(ProviderInstanceIndex) )
			{
				Result = ProviderInstances(ProviderInstanceIndex);
			}

			if ( InstanceIndex != NULL )
			{
				*InstanceIndex = ProviderInstanceIndex;
			}
		}
	}

	return Result;
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUIDataStore_GameResource::GetElementProviderTags()
{
	TArray<FName> ProviderTags;

	for ( INT ProviderIndex = 0; ProviderIndex < ElementProviderTypes.Num(); ProviderIndex++ )
	{
		FGameResourceDataProvider& Provider = ElementProviderTypes(ProviderIndex);
		if ( Provider.bExpandProviders )
		{
			const INT ProviderCount = ListElementProviders.Num(Provider.ProviderTag);
			for ( INT i = 0; i < ProviderCount; i++ )
			{
				ProviderTags.AddItem(GenerateProviderAccessTag(ProviderIndex, i));
			}
		}
		else
		{
			ProviderTags.AddItem(Provider.ProviderTag);
		}
	}

	return ProviderTags;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUIDataStore_GameResource::GetElementCount( FName FieldName )
{
	return ListElementProviders.Num(FieldName);
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUIDataStore_GameResource::GetListElements( FName DataTag, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	// grab the list of instances associated with the data tag
	TArray<UUIResourceDataProvider*> Providers;
	ListElementProviders.MultiFind(DataTag, Providers);
	if ( Providers.Num() > 0 )
	{
		out_Elements.Empty();
		for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
		{
			out_Elements.AddUniqueItem(ProviderIndex);
		}
		bResult = TRUE;
	}
	else
	{
		// otherwise, we may support this data provider type, but there just may not be any instances created, so check our
		// list of providers as well
		if ( FindProviderTypeIndex(DataTag) != INDEX_NONE )
		{
			// no instances exist, so clear the array
			out_Elements.Empty();

			// but we do support this data type, so return TRUE
			bResult = TRUE;
		}
	}

	return bResult;
}

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
UBOOL UUIDataStore_GameResource::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	UBOOL bResult = FALSE;

	// grab the list of instances associated with the data tag
	TArray<UUIResourceDataProvider*> Providers;
	ListElementProviders.MultiFind(FieldName, Providers);
	if ( Providers.IsValidIndex(CollectionIndex) )
	{
		bResult = !Providers(CollectionIndex)->eventIsProviderDisabled();
	}

	return bResult;
}

/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	DataTag			the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UUIDataStore_GameResource::GetElementCellSchemaProvider( FName DataTag )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	// search for the provider that has the matching tag
	INT ProviderIndex = FindProviderTypeIndex(DataTag);
	if ( ProviderIndex != INDEX_NONE )
	{
		FGameResourceDataProvider& Provider = ElementProviderTypes(ProviderIndex);
		if ( Provider.ProviderClass != NULL )
		{
			Result = Provider.ProviderClass->GetDefaultObject<UUIResourceDataProvider>();
		}
	}

	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 * 
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UUIDataStore_GameResource::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	// grab the list of instances associated with the data tag
	TArray<UUIResourceDataProvider*> Providers;
	ListElementProviders.MultiFind(FieldName, Providers);

	if ( Providers.IsValidIndex(ListIndex) )
	{
		Result = Providers(ListIndex);
	}

	return Result;
}

/* === UIDataStore interface === */	
/**
 * Loads the classes referenced by the ElementProviderTypes array.
 */
void UUIDataStore_GameResource::LoadDependentClasses()
{
	Super::LoadDependentClasses();

	// for each configured provider type, load the UIResourceProviderClass associated with that resource type
	for ( INT ProviderTypeIndex = ElementProviderTypes.Num() - 1; ProviderTypeIndex >= 0; ProviderTypeIndex-- )
	{
		FGameResourceDataProvider& ProviderType = ElementProviderTypes(ProviderTypeIndex);
		if ( ProviderType.ProviderClassName.Len() > 0 )
		{
			ProviderType.ProviderClass = LoadClass<UUIResourceDataProvider>(NULL, *ProviderType.ProviderClassName, NULL, LOAD_None, NULL);
			if ( ProviderType.ProviderClass == NULL )
			{
				debugf(NAME_Warning, TEXT("Failed to load class for ElementProviderType %i: %s (%s)"), ProviderTypeIndex, *ProviderType.ProviderTag.ToString(), *ProviderType.ProviderClassName);

				// if we weren't able to load the specified class, remove this provider type from the list.
				ElementProviderTypes.Remove(ProviderTypeIndex);
			}
		}
		else
		{
			debugf(TEXT("No ProviderClassName specified for ElementProviderType %i: %s"), ProviderTypeIndex, *ProviderType.ProviderTag.ToString());
		}
	}
}

/**
 * Called when this data store is added to the data store manager's list of active data stores.
 *
 * Initializes the ListElementProviders map
 *
 * @param	PlayerOwner		the player that will be associated with this DataStore.  Only relevant if this data store is
 *							associated with a particular player; NULL if this is a global data store.
 */
void UUIDataStore_GameResource::OnRegister( ULocalPlayer* PlayerOwner )
{
	Super::OnRegister(PlayerOwner);

	InitializeListElementProviders();
}

/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<IUIListElementProvider> UUIDataStore_GameResource::ResolveListElementProvider( const FString& PropertyName )
{
	TScriptInterface<IUIListElementProvider> Result;

	FName DataTag = *PropertyName;
	//@todo - this might not be the correct behavior
	if ( IsDataTagSupported(DataTag) )
	{
		Result = this;
	}

	return Result;
}

/* === UIDataProvider interface === */
/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_GameResource::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	FName FieldTag(*FieldName);
	TArray<INT> ProviderIndexes;
	if ( GetListElements(FieldTag, ProviderIndexes) )
	{
		for (int ProviderIdx = 0; ProviderIdx < ProviderIndexes.Num() ; ProviderIdx++)
		{
			if ( IsElementEnabled(FieldTag, ProviderIdx) )
			{
				out_FieldValue.PropertyTag = FieldTag;
				out_FieldValue.PropertyType = DATATYPE_ProviderCollection;
				out_FieldValue.ArrayValue.AddItem(ProviderIndexes(ProviderIdx));
				bResult = TRUE;
				break;
			}
		}
	}

	return bResult || Super::GetFieldValue(FieldName,out_FieldValue,ArrayIndex);
}

/**
 * Generates filler data for a given tag.  This is used by the editor to generate a preview that gives the
 * user an idea as to what a bound datastore will look like in-game.
 *
 * @param		DataTag		the tag corresponding to the data field that we want filler data for
 *
 * @return		a string of made-up data which is indicative of the typical [resolved] value for the specified field.
 */
FString UUIDataStore_GameResource::GenerateFillerData( const FString& DataTag )
{
	//@todo
	return Super::GenerateFillerData(DataTag);
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIDataStore_GameResource::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();

	for ( INT ProviderIndex = 0; ProviderIndex < ElementProviderTypes.Num(); ProviderIndex++ )
	{
		FGameResourceDataProvider& Provider = ElementProviderTypes(ProviderIndex);

		TArray<UUIResourceDataProvider*> ResourceProviders;
		ListElementProviders.MultiFind(Provider.ProviderTag, ResourceProviders);

		if ( Provider.bExpandProviders )
		{
			for ( INT InstanceIndex = 0; InstanceIndex < ResourceProviders.Num(); InstanceIndex++ )
			{
				FName ProviderAccessTag = GenerateProviderAccessTag(ProviderIndex, InstanceIndex);

				UUIResourceDataProvider* ProviderInstance = ResourceProviders(InstanceIndex);
				new(out_Fields) FUIDataProviderField(ProviderAccessTag, DATATYPE_Provider, ProviderInstance);
			}
		}
		else
		{
			// for each of the game resource providers, add a tag to allow the UI to access the list of providers
			new(out_Fields) FUIDataProviderField( Provider.ProviderTag, (TArray<UUIDataProvider*>&)ResourceProviders );
		}
	}

	Super::GetSupportedDataFields(out_Fields);
}

/**
 * Parses the string specified, separating the array index portion from the data field tag.
 *
 * @param	DataTag		the data tag that possibly contains an array index
 *
 * @return	the array index that was parsed from DataTag, or INDEX_NONE if there was no array index in the string specified.
 */
INT UUIDataStore_GameResource::ParseArrayDelimiter( FString& DataTag ) const
{
	INT Result = INDEX_NONE;

	INT DelimiterLocation = DataTag.InStr(ARRAY_DELIMITER);
	if ( DelimiterLocation != INDEX_NONE )
	{
		FString ArrayPortion = DataTag.Mid(DelimiterLocation + 1);

		// remove the array portion, leaving only the data tag
		DataTag = DataTag.Left(DelimiterLocation);

		if ( ArrayPortion.IsNumeric() )
		{
			// convert the array portion to an INT
			Result = appAtoi( *ArrayPortion );
		}
		else
		{
			// the array index is the name of one of our internal data providers - find out which one 
			// and return its index in the appropriate array.
			FName ProviderName(*ArrayPortion);

			TArray<UUIResourceDataProvider*> Providers;
			ListElementProviders.MultiFind(*DataTag, Providers);

			for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
			{
				UUIResourceDataProvider* Provider = Providers(ProviderIndex);
				if ( ProviderName == Provider->GetFName() )
				{
					Result = ProviderIndex;
					break;
				}
			}
		}
	}

	return Result;
}

/* === UObject interface === */
/** Required since maps are not yet supported by script serialization */
void UUIDataStore_GameResource::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects(ObjectArray);

	for ( TMultiMap<FName,UUIResourceDataProvider*>::TIterator It(ListElementProviders); It; ++It )
	{
		UUIResourceDataProvider* ResourceProvider = It.Value();
		if ( ResourceProvider != NULL )
		{
			AddReferencedObject(ObjectArray, ResourceProvider);
		}
	}
}

void UUIDataStore_GameResource::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);

	if ( !Ar.IsPersistent() )
	{
		for ( TMultiMap<FName,UUIResourceDataProvider*>::TIterator It(ListElementProviders); It; ++It )
		{
			UUIResourceDataProvider* ResourceProvider = It.Value();
			Ar << ResourceProvider;
		}
	}
}


/**
 * Called from ReloadConfig after the object has reloaded its configuration data.
 */
void UUIDataStore_GameResource::PostReloadConfig( UProperty* PropertyThatWasLoaded )
{
	Super::PostReloadConfig( PropertyThatWasLoaded );

	if ( !HasAnyFlags(RF_ClassDefaultObject) )
	{
		if ( PropertyThatWasLoaded == NULL || PropertyThatWasLoaded->GetFName() == TEXT("ElementProviderTypes") )
		{
			// reload the ProviderClass for each ElementProviderType
			LoadDependentClasses();

			// the current list element providers are potentially no longer accurate, so we'll need to reload that list as well.
			InitializeListElementProviders();

			// now notify any widgets that are subscribed to this datastore that they need to re-resolve their bindings
			eventRefreshSubscribers();
		}
	}
}

/**
 * Callback for retrieving a textual representation of natively serialized properties.  Child classes should implement this method if they wish
 * to have natively serialized property values included in things like diffcommandlet output.
 *
 * @param	out_PropertyValues	receives the property names and values which should be reported for this object.  The map's key should be the name of
 *								the property and the map's value should be the textual representation of the property's value.  The property value should
 *								be formatted the same way that UProperty::ExportText formats property values (i.e. for arrays, wrap in quotes and use a comma
 *								as the delimiter between elements, etc.)
 * @param	ExportFlags			bitmask of EPropertyPortFlags used for modifying the format of the property values
 *
 * @return	return TRUE if property values were added to the map.
 */
UBOOL UUIDataStore_GameResource::GetNativePropertyValues( TMap<FString,FString>& out_PropertyValues, DWORD ExportFlags/*=0*/ ) const
{
	UBOOL bResult = Super::GetNativePropertyValues(out_PropertyValues, ExportFlags);

	INT Count=0, LongestProviderTag=0;

	TMap<FString,FString> PropertyValues;
	for ( INT ResourceIdx = 0; ResourceIdx < ElementProviderTypes.Num(); ResourceIdx++ )
	{
		const FGameResourceDataProvider& Definition = ElementProviderTypes(ResourceIdx);
		TArray<UUIResourceDataProvider*> Providers;
		ListElementProviders.MultiFind(Definition.ProviderTag, Providers);

		for ( INT ProviderIdx = 0; ProviderIdx < Providers.Num(); ProviderIdx++ )
		{
			UUIResourceDataProvider* Provider = Providers(ProviderIdx);
			FString PropertyName = *FString::Printf(TEXT("ListElementProviders[%i].%s[%i]"), ResourceIdx, *Definition.ProviderTag.ToString(), ProviderIdx);
			FString PropertyValue = Provider->GetName();

			LongestProviderTag = Max(LongestProviderTag, PropertyName.Len());
			PropertyValues.Set(*PropertyName, PropertyValue);
		}
	}

	for ( TMap<FString,FString>::TConstIterator It(PropertyValues); It; ++It )
	{
		const FString& ProviderTag = It.Key();
		const FString& ProviderName = It.Value();

		out_PropertyValues.Set(*ProviderTag, ProviderName.LeftPad(LongestProviderTag));
		bResult = TRUE;
	}

	return bResult || ListElementProviders.Num() > 0;
}

/* ==========================================================================================================
	UUIDataStore_DynamicResource
========================================================================================================== */
/**
 * Finds the index for the GameResourceDataProvider with a tag matching ProviderTag.
 *
 * @return	the index into the ElementProviderTypes array for the GameResourceDataProvider element that has the
 *			tag specified, or INDEX_NONE if there are no elements of the ResourceProviderDefinitions array that have that tag.
 */
INT UUIDataStore_DynamicResource::FindProviderTypeIndex( FName ProviderTag ) const
{
	INT Result = INDEX_NONE;

	for ( INT ProviderIndex = 0; ProviderIndex < ResourceProviderDefinitions.Num(); ProviderIndex++ )
	{
		const FDynamicResourceProviderDefinition& Provider = ResourceProviderDefinitions(ProviderIndex);
		if ( Provider.ProviderTag == ProviderTag )
		{
			Result = ProviderIndex;
			break;
		}
	}

	return Result;
}

	
/**
 * Generates a tag containing the provider type with the name of an instance of that provider as the array delimiter.
 *
 * @param	ProviderIndex	the index into the ResourceProviderDefinitions array for the provider's type
 * @param	InstanceIndex	the index into that type's list of providers for the target instance
 *
 * @return	a name containing the provider type name (i.e. the value of GameResourceDataProvider.ProviderTag) with
 *			the instance's name as the array delimiter.
 */
FName UUIDataStore_DynamicResource::GenerateProviderAccessTag( INT ProviderIndex, INT InstanceIndex ) const
{
	FName Result;

	if ( ResourceProviderDefinitions.IsValidIndex(ProviderIndex) )
	{
		const FDynamicResourceProviderDefinition& Provider = ResourceProviderDefinitions(ProviderIndex);

		TArray<UUIResourceCombinationProvider*> ProviderInstances;
		ResourceProviders.MultiFind(Provider.ProviderTag, ProviderInstances);

		if ( ProviderInstances.IsValidIndex(InstanceIndex) )
		{
			UUIResourceCombinationProvider* ProviderInstance = ProviderInstances(InstanceIndex);
			Result = FName(*FString::Printf(TEXT("%s;%s"), *Provider.ProviderTag.ToString(), *ProviderInstance->GetName()));
		}
	}

	return Result;
}

/**
 * Get the number of UIResourceCombinationProvider instances associated with the specified tag.
 *
 * @param	ProviderTag		the tag to find instances for; should match the ProviderTag value of an element in the ResourceProviderDefinitions array.
 *
 * @return	the number of instances registered for ProviderTag.
 */
INT UUIDataStore_DynamicResource::GetProviderCount( FName ProviderTag ) const
{
	return ResourceProviders.Num(ProviderTag);
}

/**
 * Get the UIResourceCombinationProvider instances associated with the tag.
 *
 * @param	ProviderTag		the tag to find instances for; should match the ProviderTag value of an element in the ResourceProviderDefinitions array.
 * @param	out_Providers	receives the list of provider instances. this array is always emptied first.
 *
 * @return	the list of UIResourceDataProvider instances registered for ProviderTag.
 */
UBOOL UUIDataStore_DynamicResource::GetResourceProviders( FName ProviderTag, TArray<UUIResourceCombinationProvider*>& out_Providers ) const
{
	out_Providers.Empty();
 	ResourceProviders.MultiFind(ProviderTag, out_Providers);

	// we may support this data provider type, but there just may not be any instances created, so check our
	// list of providers as well
	return out_Providers.Num() > 0 || FindProviderTypeIndex(ProviderTag) != INDEX_NONE;
}

/**
 * Get the list of fields supported by the provider type specified.
 *
 * @param	ProviderTag			the name of the provider type to get fields for; should match the ProviderTag value of an element in the ResourceProviderDefinitions array.
 *								If the provider type is expanded (bExpandProviders=true), this tag should also contain the array index of the provider instance
 *								to use for retrieving the fields (this can usually be automated by calling GenerateProviderAccessTag)
 * @param	ProviderFieldTags	receives the list of tags supported by the provider specified.
 *
 * @return	TRUE if the tag was resolved successfully and the list of tags was retrieved (doesn't guarantee that the provider
 *			array will contain any elements, though).  FALSE if the data store couldn't resolve the ProviderTag.
 */
UBOOL UUIDataStore_DynamicResource::GetResourceProviderFields( FName ProviderTag, TArray<FName>& ProviderFieldTags ) const
{
	UBOOL bResult = FALSE;

	ProviderFieldTags.Empty();
	TScriptInterface<IUIListElementCellProvider> SchemaProvider = ResolveProviderReference(ProviderTag);
	if ( SchemaProvider )
	{
		// fill the output array with this provider's schema
		TMap<FName,FString> ProviderFieldTagColumnPairs;
		SchemaProvider->GetElementCellTags(ProviderTag, ProviderFieldTagColumnPairs);
		
		ProviderFieldTagColumnPairs.GenerateKeyArray(ProviderFieldTags);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Get the value of a single field in a specific resource provider instance. Example: GetProviderFieldValue('GameTypes', ClassName, 2, FieldValue)
 *
 * @param	ProviderTag		the name of the provider type; should match the ProviderTag value of an element in the ResourceProviderDefinitions array.
 * @param	SearchField		the name of the field within the provider type to get the value for; should be one of the elements retrieved from a call
 *							to GetResourceProviderFields.
 * @param	ProviderIndex	the index [into the array of providers associated with the specified tag] of the instance to get the value from;
 *							should be one of the elements retrieved by calling GetResourceProviders().
 * @param	out_FieldValue	receives the value of the field
 *
 * @return	TRUE if the field value was successfully retrieved from the provider.  FALSE if the provider tag couldn't be resolved,
 *			the index was not a valid index for the list of providers, or the search field didn't exist in that provider.
 */
UBOOL UUIDataStore_DynamicResource::GetProviderFieldValue( FName ProviderTag, FName SearchField, INT ProviderIndex, FUIProviderScriptFieldValue& out_FieldValue ) const
{
	UBOOL bResult = FALSE;

	UUIResourceCombinationProvider* FieldValueProvider = NULL;

	if ( ProviderIndex == INDEX_NONE )
	{
		FieldValueProvider = ResolveProviderReference(ProviderTag, &ProviderIndex);
	}
	else
	{
		TArray<UUIResourceCombinationProvider*> ProviderInstances;
		ResourceProviders.MultiFind(ProviderTag, ProviderInstances);

		if ( ProviderInstances.IsValidIndex(ProviderIndex) )
		{
			FieldValueProvider = ProviderInstances(ProviderIndex);
		}
	}

	if ( FieldValueProvider != NULL )
	{
		FUIProviderFieldValue FieldValue(EC_EventParm);
		if ( FieldValueProvider->GetCellFieldValue(ProviderTag, SearchField, ProviderIndex, FieldValue, INDEX_NONE) )
		{
			out_FieldValue = FieldValue;
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Searches for resource provider instance that has a field with a value that matches the value specified.
 *
 * @param	ProviderTag			the name of the provider type; should match the ProviderTag value of an element in the ResourceProviderDefinitions array.
 * @param	SearchField			the name of the field within the provider type to compare the value to; should be one of the elements retrieved from a call
 *								to GetResourceProviderFields.
 * @param	ValueToSearchFor	the field value to search for.
 *
 * @return	the index of the resource provider instance that has the same value for SearchField as the value specified, or INDEX_NONE if the
 *			provider tag couldn't be resolved,  none of the provider instances had a field with that name, or none of them had a field
 *			of that name with the value specified.
 */
INT UUIDataStore_DynamicResource::FindProviderIndexByFieldValue( FName ProviderTag, FName SearchField, const FUIProviderScriptFieldValue& ValueToSearchFor ) const
{
	INT Result = INDEX_NONE;

	TArray<UUIResourceCombinationProvider*> ProviderInstances;
	ResourceProviders.MultiFind(ProviderTag, ProviderInstances);

	FUIProviderFieldValue FieldValue(EC_EventParm);
	for ( INT ProviderIndex = 0; ProviderIndex < ProviderInstances.Num(); ProviderIndex++ )
	{
		UUIResourceCombinationProvider* Provider = ProviderInstances(ProviderIndex);
		if ( Provider->GetCellFieldValue(SearchField, SearchField, ProviderIndex, FieldValue, INDEX_NONE) )
		{
			if ( FieldValue == ValueToSearchFor )
			{
				Result = ProviderIndex;
				break;
			}
		}
	}

	return Result;
}

/**
 * Re-initializes all dynamic providers.
 */
void UUIDataStore_DynamicResource::OnLoginChange()
{
	InitializeListElementProviders();
}

/* === UUIDataStore_DynamicResource interface === */
/**
 * Finds or creates the UIResourceDataProvider instances referenced by ResourceProviderDefinitions, and stores the result
 * into the ListElementProvider map.
 */
void UUIDataStore_DynamicResource::InitializeListElementProviders()
{
	ResourceProviders.Empty();

	if ( GameResourceDataStore != NULL )
	{
		TMap<UUIResourceCombinationProvider*,UUIResourceDataProvider*> StaticProviderMap;

		// for each configured provider type, retrieve the list of ini sections that contain data for that provider class
		for ( INT ProviderTypeIndex = 0; ProviderTypeIndex < ResourceProviderDefinitions.Num(); ProviderTypeIndex++ )
		{
			FDynamicResourceProviderDefinition& ProviderType = ResourceProviderDefinitions(ProviderTypeIndex);
			UClass* ProviderClass = ProviderType.ProviderClass;

			TArray<UUIResourceDataProvider*> StaticDataProviders;
			GameResourceDataStore->GetResourceProviders(ProviderType.ProviderTag, StaticDataProviders);

			for ( INT ProviderIdx = StaticDataProviders.Num() - 1; ProviderIdx >= 0; ProviderIdx-- )
			{
				UUIResourceDataProvider* StaticProvider = StaticDataProviders(ProviderIdx);
				UUIResourceCombinationProvider* Provider = Cast<UUIResourceCombinationProvider>(StaticFindObject(ProviderClass, this, *StaticProvider->GetName(), FALSE));
				if ( Provider == NULL )
				{
					Provider = ConstructObject<UUIResourceCombinationProvider>(ProviderClass, this, StaticProvider->GetFName());
				}

				if ( Provider != NULL )
				{
					StaticProviderMap.Set(Provider, StaticProvider);
					ResourceProviders.Add(ProviderType.ProviderTag, Provider);
				}
			}
		}

		for ( TMultiMap<FName,UUIResourceCombinationProvider*>::TIterator It(ResourceProviders); It; ++It )
		{
			UUIResourceCombinationProvider* Provider = It.Value();
			Provider->eventInitializeProvider(!GIsGame, StaticProviderMap.FindRef(Provider), ProfileProvider);
		}
	}
}
	
/**
 * Finds the data provider associated with the tag specified.
 *
 * @param	ProviderTag		The tag of the provider to find.  Must match the ProviderTag value for one of elements
 *							in the ResourceProviderDefinitions array, though it can contain an array index (in which case
 *							the array index will be removed from the ProviderTag value passed in).
 * @param	InstanceIndex	If ProviderTag contains an array index, this will be set to the array index value that was parsed.
 *
 * @return	a data provider instance (or CDO if no array index was included in ProviderTag) for the element provider
 *			type associated with ProviderTag.
 */
UUIResourceCombinationProvider* UUIDataStore_DynamicResource::ResolveProviderReference( FName& ProviderTag, INT* InstanceIndex/*=NULL*/ ) const
{
	UUIResourceCombinationProvider* Result = NULL;

	// the most accurate way to determine if this is a supported type
	INT ProviderTypeIndex = FindProviderTypeIndex(ProviderTag);
	if ( ResourceProviderDefinitions.IsValidIndex(ProviderTypeIndex) )
	{
		const FDynamicResourceProviderDefinition& ProviderType = ResourceProviderDefinitions(ProviderTypeIndex);
		check(ProviderType.ProviderClass);

		TArray<UUIResourceCombinationProvider*> ProviderInstances;
		ResourceProviders.MultiFind(ProviderTag, ProviderInstances);
		if ( ProviderInstances.Num() > 0 )
		{
			Result = ProviderInstances(0);
		}
		else
		{
			Result = ProviderType.ProviderClass->GetDefaultObject<UUIResourceCombinationProvider>();
		}
	}
	else
	{
		// it's possible that we didn't find it because the ProviderTag contains an array index; this is usually the case if the provider type
		// has bExpandProviders = TRUE - in this, each instance of the provider may contain a different schema
		FString ProviderTagString = ProviderTag.ToString();
		INT ProviderInstanceIndex = ParseArrayDelimiter(ProviderTagString);
		if ( ProviderInstanceIndex != INDEX_NONE )
		{
			// remove the array index from the ProviderTag
			ProviderTag = *ProviderTagString;
			TArray<UUIResourceCombinationProvider*> ProviderInstances;
			ResourceProviders.MultiFind(ProviderTag, ProviderInstances);

			if ( ProviderInstances.IsValidIndex(ProviderInstanceIndex) )
			{
				Result = ProviderInstances(ProviderInstanceIndex);
			}

			if ( InstanceIndex != NULL )
			{
				*InstanceIndex = ProviderInstanceIndex;
			}
		}
	}

	return Result;
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUIDataStore_DynamicResource::GetElementProviderTags()
{
	TArray<FName> ProviderTags;

	for ( INT ProviderIndex = 0; ProviderIndex < ResourceProviderDefinitions.Num(); ProviderIndex++ )
	{
		FDynamicResourceProviderDefinition& Provider = ResourceProviderDefinitions(ProviderIndex);
// 		if ( Provider.bExpandProviders )
// 		{
// 			const INT ProviderCount = ResourceProviders.Num(Provider.ProviderTag);
// 			for ( INT i = 0; i < ProviderCount; i++ )
// 			{
// 				ProviderTags.AddItem(GenerateProviderAccessTag(ProviderIndex, i));
// 			}
// 		}
// 		else
		{
			ProviderTags.AddItem(Provider.ProviderTag);
		}
	}

	return ProviderTags;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUIDataStore_DynamicResource::GetElementCount( FName FieldName )
{
	INT Result = INDEX_NONE;

	FString NextFieldName = FieldName.ToString(), FieldTag;

	ParseNextDataTag(NextFieldName, FieldTag);
	while ( FieldTag.Len() > 0 ) 
	{
		if (IsDataTagSupported(*FieldTag) )
		{
			INT InstanceIndex = ParseArrayDelimiter(FieldTag);
			if ( InstanceIndex != INDEX_NONE )
			{
				FName InternalFieldName = FName(*NextFieldName);
				TArray<UUIResourceCombinationProvider*> ProviderInstances;
				ResourceProviders.MultiFind(*FieldTag, ProviderInstances);

				if ( ProviderInstances.IsValidIndex(InstanceIndex) )
				{
					UUIResourceCombinationProvider* Provider = ProviderInstances(InstanceIndex);
					IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(Provider);
					if ( ElementProvider != NULL )
					{
						Result = ElementProvider->GetElementCount(InternalFieldName);
					}
				}
			}
			else
			{
				Result = ResourceProviders.Num(*FieldTag);
			}
		}

		ParseNextDataTag(NextFieldName, FieldTag);
	}

	if ( Result == INDEX_NONE )
	{
		Result = ResourceProviders.Num(FieldName);
	}

	debugf(TEXT("UUIDataStore_DynamicResource::GetElementCount - FieldName:%s  Result:%i"), *FieldName.ToString(), Result);

	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUIDataStore_DynamicResource::GetListElements( FName DataTag, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	FString NextFieldName = DataTag.ToString(), FieldTag;
	ParseNextDataTag(NextFieldName, FieldTag);
	while ( FieldTag.Len() > 0 ) 
	{
		if (IsDataTagSupported(*FieldTag) )
		{
			INT InstanceIndex = ParseArrayDelimiter(FieldTag);
			if ( InstanceIndex != INDEX_NONE )
			{
				FName InternalFieldName = FName(*NextFieldName);
				TArray<UUIResourceCombinationProvider*> ProviderInstances;
				ResourceProviders.MultiFind(*FieldTag, ProviderInstances);

				if ( ProviderInstances.IsValidIndex(InstanceIndex) )
				{
					UUIResourceCombinationProvider* Provider = ProviderInstances(InstanceIndex);
					IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(Provider);
					if ( ElementProvider != NULL )
					{
						bResult = ElementProvider->GetListElements(InternalFieldName, out_Elements);
					}
				}
			}
			else
			{
				// grab the list of instances associated with the data tag
				TArray<UUIResourceCombinationProvider*> ProviderInstances;
				ResourceProviders.MultiFind(*FieldTag, ProviderInstances);
				if ( ProviderInstances.Num() > 0 )
				{
					out_Elements.Empty();
					for ( INT ProviderIndex = 0; ProviderIndex < ProviderInstances.Num(); ProviderIndex++ )
					{
						out_Elements.AddUniqueItem(ProviderIndex);
					}
					bResult = TRUE;
				}
				else
				{
					// otherwise, we may support this data provider type, but there just may not be any instances created, so check our
					// list of providers as well
					if ( FindProviderTypeIndex(*FieldTag) != INDEX_NONE )
					{
						// no instances exist, so clear the array
						out_Elements.Empty();

						// but we do support this data type, so return TRUE
						bResult = TRUE;
					}
				}
			}
		}

		ParseNextDataTag(NextFieldName, FieldTag);
	}

	debugf(TEXT("UUIDataStore_DynamicResource::GetListElements - DataTag:%s  out_Elements:%i   bResult:%i"), *DataTag.ToString(), out_Elements.Num(), bResult);

	return bResult;
}

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
UBOOL UUIDataStore_DynamicResource::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	UBOOL bResult = FALSE;

	FString NextFieldName = FieldName.ToString(), FieldTag;
	ParseNextDataTag(NextFieldName, FieldTag);
	while ( FieldTag.Len() > 0 ) 
	{
		if ( IsDataTagSupported(*FieldTag) )
		{
			INT InstanceIndex = ParseArrayDelimiter(FieldTag);
			if ( InstanceIndex != INDEX_NONE )
			{
				FName InternalFieldName = FName(*NextFieldName);
				TArray<UUIResourceCombinationProvider*> ProviderInstances;
				ResourceProviders.MultiFind(*FieldTag, ProviderInstances);

				if ( ProviderInstances.IsValidIndex(InstanceIndex) )
				{
					UUIResourceCombinationProvider* Provider = ProviderInstances(InstanceIndex);
					IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(Provider);
					if ( ElementProvider != NULL )
					{
						bResult = ElementProvider->IsElementEnabled(InternalFieldName, CollectionIndex);
					}
				}
			}
			else
			{
				// grab the list of instances associated with the data tag
				TArray<UUIResourceCombinationProvider*> ProviderInstances;
				ResourceProviders.MultiFind(*FieldTag, ProviderInstances);
				if ( ProviderInstances.IsValidIndex(CollectionIndex) )
				{
					bResult = !ProviderInstances(CollectionIndex)->eventIsProviderDisabled();
				}
			}
		}

		ParseNextDataTag(NextFieldName, FieldTag);
	}

	if ( !bResult )
	{
		// grab the list of instances associated with the data tag
		TArray<UUIResourceCombinationProvider*> ProviderInstances;
		ResourceProviders.MultiFind(FieldName, ProviderInstances);
		if ( ProviderInstances.IsValidIndex(CollectionIndex) )
		{
			bResult = !ProviderInstances(CollectionIndex)->eventIsProviderDisabled();
		}
	}

	return bResult;
}

/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	DataTag			the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UUIDataStore_DynamicResource::GetElementCellSchemaProvider( FName DataTag )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	FString NextFieldName = DataTag.ToString(), FieldTag;
	ParseNextDataTag(NextFieldName, FieldTag);
	while ( FieldTag.Len() > 0 ) 
	{
		if ( IsDataTagSupported(*FieldTag) )
		{
			INT InstanceIndex = ParseArrayDelimiter(FieldTag);
			if ( InstanceIndex != INDEX_NONE )
			{
				FName InternalFieldName = FName(*NextFieldName);
				TArray<UUIResourceCombinationProvider*> ProviderInstances;
				ResourceProviders.MultiFind(*FieldTag, ProviderInstances);

				if ( ProviderInstances.IsValidIndex(InstanceIndex) )
				{
					UUIResourceCombinationProvider* Provider = ProviderInstances(InstanceIndex);
					IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(Provider);
					if ( ElementProvider != NULL )
					{
						Result = ElementProvider->GetElementCellSchemaProvider(InternalFieldName);
					}
				}
			}
			else
			{
				// search for the provider that has the matching tag
				INT ProviderIndex = FindProviderTypeIndex(*FieldTag);
				if ( ProviderIndex != INDEX_NONE )
				{
					FDynamicResourceProviderDefinition& Provider = ResourceProviderDefinitions(ProviderIndex);
					if ( Provider.ProviderClass != NULL )
					{
						TArray<UUIResourceCombinationProvider*> ProviderInstances;
						ResourceProviders.MultiFind(*FieldTag, ProviderInstances);
						if ( ProviderInstances.Num() > 0 )
						{
							Result = ProviderInstances(0);
						}
						else
						{
							Result = Provider.ProviderClass->GetDefaultObject<UUIResourceCombinationProvider>();
						}
					}
				}
			}
		}

		ParseNextDataTag(NextFieldName, FieldTag);
	}

	if ( !Result )
	{
		// search for the provider that has the matching tag
		INT ProviderIndex = FindProviderTypeIndex(DataTag);
		if ( ProviderIndex != INDEX_NONE )
		{
			FDynamicResourceProviderDefinition& Provider = ResourceProviderDefinitions(ProviderIndex);
			if ( Provider.ProviderClass != NULL )
			{
				TArray<UUIResourceCombinationProvider*> ProviderInstances;
				ResourceProviders.MultiFind(DataTag, ProviderInstances);
				if ( ProviderInstances.Num() > 0 )
				{
					Result = ProviderInstances(0);
				}
				else
				{
					Result = Provider.ProviderClass->GetDefaultObject<UUIResourceCombinationProvider>();
				}
			}
		}
	}
	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 * 
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UUIDataStore_DynamicResource::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	FString NextFieldName = FieldName.ToString(), FieldTag;
	ParseNextDataTag(NextFieldName, FieldTag);
	while ( FieldTag.Len() > 0 ) 
	{
		if ( IsDataTagSupported(*FieldTag) )
		{
			INT InstanceIndex = ParseArrayDelimiter(FieldTag);
			if ( InstanceIndex != INDEX_NONE )
			{
				FName InternalFieldName = FName(*NextFieldName);
				TArray<UUIResourceCombinationProvider*> ProviderInstances;
				ResourceProviders.MultiFind(*FieldTag, ProviderInstances);
				if ( ProviderInstances.IsValidIndex(InstanceIndex) )
				{
					UUIResourceCombinationProvider* Provider = ProviderInstances(InstanceIndex);
					IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(Provider);
					if ( ElementProvider != NULL )
					{
						Result = ElementProvider->GetElementCellValueProvider(InternalFieldName, ListIndex);
					}
				}
			}
			else
			{
				// grab the list of instances associated with the data tag
				TArray<UUIResourceCombinationProvider*> ProviderInstances;
				ResourceProviders.MultiFind(*FieldTag, ProviderInstances);
				if ( ProviderInstances.IsValidIndex(ListIndex) )
				{
					Result = ProviderInstances(ListIndex);
				}
			}
		}
		ParseNextDataTag(NextFieldName, FieldTag);
	}

	if ( !Result )
	{
		// grab the list of instances associated with the data tag
		TArray<UUIResourceCombinationProvider*> ProviderInstances;
		ResourceProviders.MultiFind(FieldName, ProviderInstances);
		if ( ProviderInstances.IsValidIndex(ListIndex) )
		{
			Result = ProviderInstances(ListIndex);
		}
	}

	return Result;
}

/* === UIDataStore interface === */	
/**
 * Loads the classes referenced by the ResourceProviderDefinitions array.
 */
void UUIDataStore_DynamicResource::LoadDependentClasses()
{
	Super::LoadDependentClasses();

	// for each configured provider type, load the UIResourceProviderClass associated with that resource type
	for ( INT ProviderTypeIndex = ResourceProviderDefinitions.Num() - 1; ProviderTypeIndex >= 0; ProviderTypeIndex-- )
	{
		FDynamicResourceProviderDefinition& ProviderType = ResourceProviderDefinitions(ProviderTypeIndex);
		if ( ProviderType.ProviderClassName.Len() > 0 )
		{
			ProviderType.ProviderClass = LoadClass<UUIResourceCombinationProvider>(NULL, *ProviderType.ProviderClassName, NULL, LOAD_None, NULL);
			if ( ProviderType.ProviderClass == NULL )
			{
				debugf(NAME_Warning, TEXT("Failed to load class for ElementProviderType %i: %s (%s)"), ProviderTypeIndex, *ProviderType.ProviderTag.ToString(), *ProviderType.ProviderClassName);

				// if we weren't able to load the specified class, remove this provider type from the list.
				ResourceProviderDefinitions.Remove(ProviderTypeIndex);
			}
		}
		else
		{
			debugf(TEXT("No ProviderClassName specified for ElementProviderType %i: %s"), ProviderTypeIndex, *ProviderType.ProviderTag.ToString());
		}
	}
}

/**
 * Called when this data store is added to the data store manager's list of active data stores.
 *
 * Initializes the ListElementProviders map
 *
 * @param	PlayerOwner		the player that will be associated with this DataStore.  Only relevant if this data store is
 *							associated with a particular player; NULL if this is a global data store.
 */
void UUIDataStore_DynamicResource::OnRegister( ULocalPlayer* PlayerOwner )
{
	Super::OnRegister(PlayerOwner);
	InitializeListElementProviders();
}

/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<IUIListElementProvider> UUIDataStore_DynamicResource::ResolveListElementProvider( const FString& PropertyName )
{
	TScriptInterface<IUIListElementProvider> Result;

	FString NextFieldName = PropertyName, FieldTag;
	ParseNextDataTag(NextFieldName, FieldTag);
	while ( FieldTag.Len() > 0 ) 
	{
		if ( IsDataTagSupported(*FieldTag) )
		{
			// pull off the array delimiter first, so that we can lookup this provider's type
			INT InstanceIndex = ParseArrayDelimiter(FieldTag);
			FName ProviderTypeId = FName(*FieldTag);

			// if this provider type is configured to be used as a nested element provider, use the data store as the element provider
			// so that we can handle parsing off the passed in data field names
			if ( InstanceIndex != INDEX_NONE )
			{
				TArray<UUIResourceCombinationProvider*> ProviderInstances;
				ResourceProviders.MultiFind(ProviderTypeId, ProviderInstances, FALSE);
				if ( ProviderInstances.IsValidIndex(InstanceIndex) )
				{
					UUIResourceCombinationProvider* Provider = ProviderInstances(InstanceIndex);
					Result = Provider->ResolveListElementProvider(NextFieldName);
				}
			}

			if ( !Result )
			{
				Result = this;
			}
		}
		ParseNextDataTag(NextFieldName, FieldTag);
	}

	if ( !Result )
	{
		Result = Super::ResolveListElementProvider(PropertyName);
	}

	return Result;
}

/* === UIDataProvider interface === */
/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_DynamicResource::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	FName FieldTag(*FieldName);
	TArray<INT> ProviderIndexes;
	if ( GetListElements(FieldTag, ProviderIndexes) )
	{
		for ( INT ProviderIdx = 0; ProviderIdx < ProviderIndexes.Num(); ProviderIdx++ )
		{
			if ( IsElementEnabled(FieldTag, ProviderIdx) )
			{
				out_FieldValue.PropertyTag = FieldTag;
				out_FieldValue.PropertyType = DATATYPE_ProviderCollection;
				out_FieldValue.ArrayValue.AddItem(ProviderIndexes(ProviderIdx));
				bResult = TRUE;
				break;
			}
		}
	}

	return bResult || Super::GetFieldValue(FieldName,out_FieldValue,ArrayIndex);
}

/**
 * Generates filler data for a given tag.  This is used by the editor to generate a preview that gives the
 * user an idea as to what a bound datastore will look like in-game.
 *
 * @param		DataTag		the tag corresponding to the data field that we want filler data for
 *
 * @return		a string of made-up data which is indicative of the typical [resolved] value for the specified field.
 */
FString UUIDataStore_DynamicResource::GenerateFillerData( const FString& DataTag )
{
	//@todo
	return Super::GenerateFillerData(DataTag);
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIDataStore_DynamicResource::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();

	for ( INT ProviderIndex = 0; ProviderIndex < ResourceProviderDefinitions.Num(); ProviderIndex++ )
	{
		FDynamicResourceProviderDefinition& Provider = ResourceProviderDefinitions(ProviderIndex);

		TArray<UUIResourceCombinationProvider*> ProviderInstances;
		ResourceProviders.MultiFind(Provider.ProviderTag, ProviderInstances);

// 		if ( Provider.bExpandProviders )
// 		{
// 			for ( INT InstanceIndex = 0; InstanceIndex < Providers.Num(); InstanceIndex++ )
// 			{
// 				FName ProviderAccessTag = GenerateProviderAccessTag(ProviderIndex, InstanceIndex);
// 
// 				UUIResourceCombinationProvider* ProviderInstance = ResourceProviders(InstanceIndex);
// 				new(out_Fields) FUIDataProviderField(ProviderAccessTag, DATATYPE_Provider, ProviderInstance);
// 			}
// 		}
// 		else
		{
			// for each of the game resource providers, add a tag to allow the UI to access the list of providers
			new(out_Fields) FUIDataProviderField( Provider.ProviderTag, (TArray<UUIDataProvider*>&)ProviderInstances );
		}
	}

	Super::GetSupportedDataFields(out_Fields);
}

/**
 * Parses the string specified, separating the array index portion from the data field tag.
 *
 * @param	DataTag		the data tag that possibly contains an array index
 *
 * @return	the array index that was parsed from DataTag, or INDEX_NONE if there was no array index in the string specified.
 */
INT UUIDataStore_DynamicResource::ParseArrayDelimiter( FString& DataTag ) const
{
	INT Result = INDEX_NONE;

	INT DelimiterLocation = DataTag.InStr(ARRAY_DELIMITER);
	if ( DelimiterLocation != INDEX_NONE )
	{
		FString ArrayPortion = DataTag.Mid(DelimiterLocation + 1);

		// remove the array portion, leaving only the data tag
		DataTag = DataTag.Left(DelimiterLocation);

		if ( ArrayPortion.IsNumeric() )
		{
			// convert the array portion to an INT
			Result = appAtoi( *ArrayPortion );
		}
		else
		{
			// the array index is the name of one of our internal data providers - find out which one 
			// and return its index in the appropriate array.
			FName ProviderName(*ArrayPortion);

			TArray<UUIResourceCombinationProvider*> ProviderInstances;
			ResourceProviders.MultiFind(*DataTag, ProviderInstances);

			for ( INT ProviderIndex = 0; ProviderIndex < ProviderInstances.Num(); ProviderIndex++ )
			{
				UUIResourceCombinationProvider* Provider = ProviderInstances(ProviderIndex);
				if ( ProviderName == Provider->GetFName() )
				{
					Result = ProviderIndex;
					break;
				}
			}
		}
	}

	return Result;
}

/* === UObject interface === */
/** Required since maps are not yet supported by script serialization */
void UUIDataStore_DynamicResource::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects(ObjectArray);

	for ( TMultiMap<FName,UUIResourceCombinationProvider*>::TIterator It(ResourceProviders); It; ++It )
	{
		UUIResourceCombinationProvider* ResourceProvider = It.Value();
		if ( ResourceProvider != NULL )
		{
			AddReferencedObject(ObjectArray, ResourceProvider);
		}
	}
}

void UUIDataStore_DynamicResource::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);

	if ( !Ar.IsPersistent() )
	{
		for ( TMultiMap<FName,UUIResourceCombinationProvider*>::TIterator It(ResourceProviders); It; ++It )
		{
			UUIResourceCombinationProvider*& ResourceProvider = It.Value();
			Ar << ResourceProvider;
		}
	}
}


/**
 * Called from ReloadConfig after the object has reloaded its configuration data.
 */
void UUIDataStore_DynamicResource::PostReloadConfig( UProperty* PropertyThatWasLoaded )
{
	Super::PostReloadConfig( PropertyThatWasLoaded );

	if ( !HasAnyFlags(RF_ClassDefaultObject) )
	{
		if ( PropertyThatWasLoaded == NULL || PropertyThatWasLoaded->GetFName() == TEXT("ResourceProviderDefinitions") )
		{
			// reload the ProviderClass for each ElementProviderType
			LoadDependentClasses();

			// the current list element providers are potentially no longer accurate, so we'll need to reload that list as well.
			InitializeListElementProviders();

			// now notify any widgets that are subscribed to this datastore that they need to re-resolve their bindings
			eventRefreshSubscribers();
		}
	}
}

/**
 * Callback for retrieving a textual representation of natively serialized properties.  Child classes should implement this method if they wish
 * to have natively serialized property values included in things like diffcommandlet output.
 *
 * @param	out_PropertyValues	receives the property names and values which should be reported for this object.  The map's key should be the name of
 *								the property and the map's value should be the textual representation of the property's value.  The property value should
 *								be formatted the same way that UProperty::ExportText formats property values (i.e. for arrays, wrap in quotes and use a comma
 *								as the delimiter between elements, etc.)
 * @param	ExportFlags			bitmask of EPropertyPortFlags used for modifying the format of the property values
 *
 * @return	return TRUE if property values were added to the map.
 */
UBOOL UUIDataStore_DynamicResource::GetNativePropertyValues( TMap<FString,FString>& out_PropertyValues, DWORD ExportFlags/*=0*/ ) const
{
	UBOOL bResult = Super::GetNativePropertyValues(out_PropertyValues, ExportFlags);

	INT Count=0, LongestProviderTag=0;

	TMap<FString,FString> PropertyValues;
	for ( INT ResourceIdx = 0; ResourceIdx < ResourceProviderDefinitions.Num(); ResourceIdx++ )
	{
		const FDynamicResourceProviderDefinition& Definition = ResourceProviderDefinitions(ResourceIdx);
		TArray<UUIResourceCombinationProvider*> Providers;
		ResourceProviders.MultiFind(Definition.ProviderTag, Providers);

		for ( INT ProviderIdx = 0; ProviderIdx < Providers.Num(); ProviderIdx++ )
		{
			UUIResourceCombinationProvider* Provider = Providers(ProviderIdx);
			FString PropertyName = *FString::Printf(TEXT("ResourceProviders[%i].%s[%i]"), ResourceIdx, *Definition.ProviderTag.ToString(), ProviderIdx);
			FString PropertyValue = Provider->GetName();

			LongestProviderTag = Max(LongestProviderTag, PropertyName.Len());
			PropertyValues.Set(*PropertyName, PropertyValue);
		}
	}

	for ( TMap<FString,FString>::TConstIterator It(PropertyValues); It; ++It )
	{
		const FString& ProviderTag = It.Key();
		const FString& ProviderName = It.Value();

		out_PropertyValues.Set(*ProviderTag, ProviderName.LeftPad(LongestProviderTag));
		bResult = TRUE;
	}

	return bResult || ResourceProviders.Num() > 0;
}

/* ==========================================================================================================
	UCurrentGameDataStore
========================================================================================================== */
/**
 * Parses the markup string for a reference to a specific element of the TeamData collection.
 *
 * @param	FieldName	a markup string possibly containing markup referencing a specific element of the TeamData array.
 *						If a reference is found (i.e. TeamData;0), will be set to the remainder of the string.
 *
 * @return	a pointer to the element of the TeamData array if the markup contains a reference to one; otherwise NULL.
 */
UTeamDataProvider* UCurrentGameDataStore::ParseTeamProviderCollectionReference( FString& FieldName ) const
{
	UTeamDataProvider* Result = NULL;
	
	FString MarkupString = FieldName, FieldTag;
	if ( ParseNextDataTag(MarkupString, FieldTag) )
	{
		// pull off the array delimiter first, so that we can lookup this provider's type
		INT InstanceIndex = ParseArrayDelimiter(FieldTag);
#if !CONSOLE
		if ( GIsEditor && !GIsGame )
		{
			// in the editor, we'll only have one TeamData element
			InstanceIndex = Clamp<INT>(InstanceIndex, INDEX_NONE, TeamData.Num() - 1);
		}
#endif
		if ( InstanceIndex != INDEX_NONE && FieldTag == TEXT("Teams") && TeamData.IsValidIndex(InstanceIndex) )
		{
			Result = TeamData(InstanceIndex);
			FieldName = MarkupString;
		}
	}

	return Result;
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UCurrentGameDataStore::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();

	if ( GIsEditor && !GIsGame )
	{
		// if we're in the UI editor, we won't have any data providers assigned since they're all dynamic
		// so we'll use the configured provider classes to retrieve the list of properties that could possibly
		// be accessed through the data store.
		check(ProviderTypes.GameDataProviderClass);
		UGameInfoDataProvider* GameInfoDataProvider = ProviderTypes.GameDataProviderClass->GetDefaultObject<UGameInfoDataProvider>();
		GameInfoDataProvider->GetSupportedDataFields(out_Fields);

		if ( PlayerData.Num() == 0 )
		{
			check(ProviderTypes.PlayerDataProviderClass);
			PlayerData.AddItem(ProviderTypes.PlayerDataProviderClass->GetDefaultObject<UPlayerDataProvider>());
		}

		if ( TeamData.Num() == 0 )
		{
			check(ProviderTypes.TeamDataProviderClass);

			UTeamDataProvider* TeamCDO = ProviderTypes.TeamDataProviderClass->GetDefaultObject<UTeamDataProvider>();
			TeamData.AddItem(TeamCDO);

			TeamCDO->Players.AddItem(ProviderTypes.PlayerDataProviderClass->GetDefaultObject<UPlayerDataProvider>());
		}
	}
	else if ( GameData != NULL )
	{
		GameData->GetSupportedDataFields(out_Fields);
	}

	new(out_Fields) FUIDataProviderField( TEXT("Players"), (const TArray<UUIDataProvider*>&)PlayerData );
	new(out_Fields) FUIDataProviderField( TEXT("Teams"), (const TArray<UUIDataProvider*>&)TeamData );

	Super::GetSupportedDataFields(out_Fields);
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UCurrentGameDataStore::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName == TEXT("Players") )
	{
		//@todo ronp - are there any cases where we want to dictate which player should be selected?
	}
	else if ( FieldName == TEXT("Teams") )
	{
		//@todo ronp - are there any cases where we want to dictate which team should be selected?
	}
	else
	{
		UUIDataProvider* DefaultDataProvider = GetDefaultDataProvider();
		if ( DefaultDataProvider != NULL )
		{
			// pass it through to the underlying gameinfo data provider (have to cast here as GetFieldValue is protected in UIDataProvider)
			bResult = CastChecked<UGameInfoDataProvider>(DefaultDataProvider)->GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
		}
	}

	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/**
 * Returns a pointer to the data provider which provides the tags for this data provider.  Normally, that will be this data provider,
 * but for some data stores such as the Scene data store, data is pulled from an internal provider but the data fields are presented as
 * though they are fields of the data store itself.
 */
UUIDataProvider* UCurrentGameDataStore::GetDefaultDataProvider()
{
	if(GIsEditor && !GIsGame)
	{
		return ProviderTypes.GameDataProviderClass->GetDefaultObject<UGameInfoDataProvider>();
	}
	else
	{
		return GameData;
	}	
}
/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<IUIListElementProvider> UCurrentGameDataStore::ResolveListElementProvider( const FString& PropertyName )
{
	return Super::ResolveListElementProvider(PropertyName);
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UCurrentGameDataStore::GetElementProviderTags()
{
	TArray<FName> ProviderTags;

	UUIDataProvider* DefaultDataProvider = GetDefaultDataProvider();
	if ( DefaultDataProvider != NULL )
	{
		IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(DefaultDataProvider);
		if ( ElementProvider != NULL && DefaultDataProvider != this )
		{
			ProviderTags += ElementProvider->GetElementProviderTags();
		}
	}

	ProviderTags.AddItem(TEXT("Players"));
	ProviderTags.AddItem(TEXT("Teams"));

	return ProviderTags;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UCurrentGameDataStore::GetElementCount( FName FieldName )
{
	INT Result = 0;
	if ( FieldName == TEXT("Players") )
	{
		Result = PlayerData.Num();
	}
	else if ( FieldName == TEXT("Teams") )
	{
		Result = TeamData.Num();
	}
	else
	{
		FString NextFieldName = FieldName.ToString();
		UTeamDataProvider* ReferencedTeamProvider = ParseTeamProviderCollectionReference(NextFieldName);
		if ( ReferencedTeamProvider != NULL )
		{
			Result = ReferencedTeamProvider->GetElementCount(*NextFieldName);
		}
		else
		{
			UUIDataProvider* DefaultDataProvider = GetDefaultDataProvider();
			if ( DefaultDataProvider != NULL )
			{
				IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(DefaultDataProvider);
				if ( ElementProvider != NULL && DefaultDataProvider != this )
				{
					// pass it through to the underlying gameinfo data provider
					Result = ElementProvider->GetElementCount(FieldName);
				}
			}
		}
	}

	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UCurrentGameDataStore::GetListElements( FName DataTag, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	out_Elements.Empty();
	if ( DataTag == TEXT("Players") )
	{
		for ( INT PlayerIndex = 0; PlayerIndex < PlayerData.Num(); PlayerIndex++ )
		{
			out_Elements.AddUniqueItem(PlayerIndex);
		}
		bResult = TRUE;
	}
	else if ( DataTag == TEXT("Teams") )
	{
		for ( INT TeamIndex = 0; TeamIndex < TeamData.Num(); TeamIndex++ )
		{
			out_Elements.AddUniqueItem(TeamIndex);
		}
		bResult = TRUE;
	}
	else
	{
		FString NextFieldName = DataTag.ToString();
		UTeamDataProvider* ReferencedTeamProvider = ParseTeamProviderCollectionReference(NextFieldName);
		if ( ReferencedTeamProvider != NULL )
		{
			bResult = ReferencedTeamProvider->GetListElements(*NextFieldName, out_Elements);
		}
		else
		{
			UUIDataProvider* DefaultDataProvider = GetDefaultDataProvider();
			if ( DefaultDataProvider != NULL )
			{
				IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(DefaultDataProvider);
				if ( ElementProvider != NULL && DefaultDataProvider != this )
				{
					// pass it through to the underlying gameinfo data provider
					bResult = ElementProvider->GetListElements(DataTag, out_Elements);
				}
			}
		}
	}

	return bResult;
}

/**
 * Allows list element providers the chance to perform custom sorting of a collection of list elements.  Implementors should implement this
 * method if they desire to perform complex sorting behavior, such as considering additional data when evaluting the order to sort the elements into.
 *
 * @param	CollectionDataFieldName		the name of a collection data field inside this UIListElementProvider associated with the
 *										list items provided.  Guaranteed to one of the values returned from GetElementProviderTags.
 * @param	ListItems					the array of list items that need sorting.
 * @param	SortParameters				the parameters to use for sorting
 *										PrimaryIndex:
 *											the index [into the ListItems' Cells array] for the cell which the user desires to perform primary sorting with.
 *										SecondaryIndex:
 *											the index [into the ListItems' Cells array] for the cell which the user desires to perform secondary sorting with.  Not guaranteed
 *											to be a valid value; Comparison should be performed using the value of the field indicated by PrimarySortIndex, then when these
 *											values are identical, the value of the cell field indicated by SecondarySortIndex should be used.
 *
 * @return	TRUE to indicate that custom sorting was performed by this UIListElementProvider.  Custom sorting is not required - if this method returns FALSE,
 *			the list bound to this UIListElementProvider will perform its default sorting behavior (alphabetical sorting of the desired cell values)
 */
UBOOL UCurrentGameDataStore::SortListElements( FName CollectionDataFieldName, TArray<const FUIListItem>& ListItems, const FUIListSortingParameters& SortParameters )
{
	UBOOL bResult = FALSE;

	if ( CollectionDataFieldName != TEXT("Players") && CollectionDataFieldName != TEXT("Teams") )
	{
		FString NextFieldName = CollectionDataFieldName.ToString();
		UTeamDataProvider* ReferencedTeamProvider = ParseTeamProviderCollectionReference(NextFieldName);
		if ( ReferencedTeamProvider != NULL )
		{
			bResult = ReferencedTeamProvider->SortListElements(*NextFieldName, ListItems, SortParameters);
		}
		else
		{
			UUIDataProvider* DefaultDataProvider = GetDefaultDataProvider();
			if ( DefaultDataProvider != NULL )
			{
				IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(DefaultDataProvider);
				if ( ElementProvider != NULL && DefaultDataProvider != this )
				{
					bResult = ElementProvider->SortListElements(CollectionDataFieldName, ListItems, SortParameters);
				}
			}
		}
	}

	return bResult;
}

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
UBOOL UCurrentGameDataStore::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	UBOOL bResult = FALSE;

	if ( FieldName == TEXT("Players") )
	{
		if ( PlayerData.IsValidIndex(CollectionIndex) )
		{
			UPlayerDataProvider* DataProvider = PlayerData(CollectionIndex);
			bResult = DataProvider != NULL && !DataProvider->eventIsProviderDisabled();
		}
	}
	else if ( FieldName == TEXT("Teams") )
	{
		if ( TeamData.IsValidIndex(CollectionIndex) )
		{
			UTeamDataProvider* DataProvider = TeamData(CollectionIndex);
			bResult = DataProvider != NULL && !DataProvider->eventIsProviderDisabled();
		}
	}
	else
	{
		FString NextFieldName = FieldName.ToString();
		UTeamDataProvider* ReferencedTeamProvider = ParseTeamProviderCollectionReference(NextFieldName);
		if ( ReferencedTeamProvider != NULL )
		{
			bResult = ReferencedTeamProvider->IsElementEnabled(*NextFieldName, CollectionIndex);
		}
		else
		{
			UUIDataProvider* DefaultDataProvider = GetDefaultDataProvider();
			if ( DefaultDataProvider != NULL )
			{
				IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(DefaultDataProvider);
				if ( ElementProvider != NULL && DefaultDataProvider != this )
				{
					// pass it through to the underlying gameinfo data provider
					bResult = ElementProvider->IsElementEnabled(FieldName, CollectionIndex);
				}
			}
		}
	}

	return bResult;
}

/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	DataTag			the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UCurrentGameDataStore::GetElementCellSchemaProvider( FName DataTag )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	if ( DataTag == TEXT("Players") )
	{
		if ( ProviderTypes.PlayerDataProviderClass != NULL )
		{
			Result = ProviderTypes.PlayerDataProviderClass->GetDefaultObject<UPlayerDataProvider>();
		}
	}
	else if ( DataTag == TEXT("Teams") )
	{
		if ( ProviderTypes.TeamDataProviderClass != NULL )
		{
			Result = ProviderTypes.TeamDataProviderClass->GetDefaultObject<UTeamDataProvider>();
		}
	}
	else
	{
		FString NextFieldName = DataTag.ToString();
		UTeamDataProvider* ReferencedTeamProvider = ParseTeamProviderCollectionReference(NextFieldName);
		if ( ReferencedTeamProvider != NULL )
		{
			Result = ReferencedTeamProvider->GetElementCellSchemaProvider(*NextFieldName);
		}
		else
		{
			UUIDataProvider* DefaultDataProvider = GetDefaultDataProvider();
			if ( DefaultDataProvider != NULL )
			{
				IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(DefaultDataProvider);
				if ( ElementProvider != NULL && DefaultDataProvider != this )
				{
					// pass it through to the underlying gameinfo data provider
					Result = ElementProvider->GetElementCellSchemaProvider(DataTag);
				}
			}
		}
	}

	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 * 
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UCurrentGameDataStore::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<IUIListElementCellProvider> Result;
	if ( FieldName == TEXT("Players") )
	{
		if ( PlayerData.IsValidIndex(ListIndex) )
		{
			Result = PlayerData(ListIndex);
		}
	}
	else if ( FieldName == TEXT("Teams") )
	{
		if ( TeamData.IsValidIndex(ListIndex) )
		{
			Result = TeamData(ListIndex);
		}
	}
	else
	{
		FString NextFieldName = FieldName.ToString();
		UTeamDataProvider* ReferencedTeamProvider = ParseTeamProviderCollectionReference(NextFieldName);
		if ( ReferencedTeamProvider != NULL )
		{
			Result = ReferencedTeamProvider->GetElementCellValueProvider(*NextFieldName, ListIndex);
		}
		else
		{
			UUIDataProvider* DefaultDataProvider = GetDefaultDataProvider();
			if ( DefaultDataProvider != NULL )
			{
				IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(DefaultDataProvider);
				if ( ElementProvider != NULL && DefaultDataProvider != this )
				{
					// pass it through to the underlying gameinfo data provider
					Result = ElementProvider->GetElementCellValueProvider(FieldName, ListIndex);
				}
			}
		}
	}

	return Result;
}

/* ==========================================================================================================
	UTeamDataProvider
========================================================================================================== */
/* === UUIDataProvider interface === */
/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<IUIListElementProvider> UTeamDataProvider::ResolveListElementProvider( const FString& PropertyName )
{
	TScriptInterface<IUIListElementProvider> Result;

	if ( PlayerListFieldName == *PropertyName )
	{
		Result = this;
	}

	return Result;
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UTeamDataProvider::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField(PlayerListFieldName, (const TArray<UUIDataProvider*>&)Players);

	Super::GetSupportedDataFields(out_Fields);
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UTeamDataProvider::GetElementProviderTags()
{
	TArray<FName> Result;
	Result.AddItem(PlayerListFieldName);

	return Result;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UTeamDataProvider::GetElementCount( FName FieldName )
{
	INT Result = INDEX_NONE;

	if ( FieldName == PlayerListFieldName )
	{
		Result = Players.Num();
	}

	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UTeamDataProvider::GetListElements( FName DataTag, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	if ( DataTag == PlayerListFieldName )
	{
		for ( INT PlayerIdx = 0; PlayerIdx < Players.Num(); PlayerIdx++ )
		{
			out_Elements.AddItem(PlayerIdx);
		}

		bResult = TRUE;
	}

	return bResult;
}

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
UBOOL UTeamDataProvider::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	UBOOL bResult = FALSE;

	if ( FieldName == PlayerListFieldName )
	{
		if ( Players.IsValidIndex(CollectionIndex) )
		{
			UPlayerDataProvider* DataProvider = Players(CollectionIndex);
			bResult = DataProvider != NULL && !DataProvider->eventIsProviderDisabled();
		}
	}

	return bResult;
}

/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	DataTag			the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UTeamDataProvider::GetElementCellSchemaProvider( FName DataTag )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	if ( DataTag == PlayerListFieldName )
	{
		// we need to CDO for the PlayerDataProvider class that is being used - since that class is specified in the CurrentGame
		// data store's defaultproperties, don't assume that it's UPlayerDataProvider.

		// grab the data store client
		UDataStoreClient* DSClient = UUIInteraction::GetDataStoreClient();
		if ( DSClient != NULL )
		{
			// find the CurrentGame data store
			static FName CurrentGameDSName = UCurrentGameDataStore::StaticClass()->GetDefaultObject<UCurrentGameDataStore>()->Tag;
			UCurrentGameDataStore* CurrentGameData = Cast<UCurrentGameDataStore>(DSClient->FindDataStore(CurrentGameDSName));
			if ( CurrentGameData != NULL )
			{
				// get the schema provider for the active PlayerDataProvider class
				Result = CurrentGameData->GetElementCellSchemaProvider(TEXT("Players"));
			}
		}
	}

	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 * 
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UTeamDataProvider::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	if ( FieldName == PlayerListFieldName && Players.IsValidIndex(ListIndex) )
	{
		Result = Players(ListIndex);
	}

	return Result;
}

/* ==========================================================================================================
	UPlayerOwnerDataStore
========================================================================================================== */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UPlayerOwnerDataStore::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();

	if ( GIsEditor && !GIsGame )
	{
		// if we're in the UI editor, we won't have any data providers assigned since they're all dynamic
		// so we'll use the configured provider classes to retrieve the list of properties that could possibly
		// be accessed through the data store.

		UClass* PlayerDataProviderClass = ProviderTypes.PlayerOwnerDataProviderClass;
		check(PlayerDataProviderClass);

		UPlayerOwnerDataProvider* DefaultPlayerDataProvider = PlayerDataProviderClass->GetDefaultObject<UPlayerOwnerDataProvider>();
		DefaultPlayerDataProvider->GetSupportedDataFields(out_Fields);

		check(ProviderTypes.CurrentWeaponDataProviderClass);
		new(out_Fields) FUIDataProviderField( TEXT("CurrentWeapon"), DATATYPE_Provider, ProviderTypes.CurrentWeaponDataProviderClass->GetDefaultObject<UCurrentWeaponDataProvider>() );

		check(ProviderTypes.WeaponDataProviderClass);

		TArray<UUIDataProvider*> Providers;
		Providers.AddItem(ProviderTypes.WeaponDataProviderClass->GetDefaultObject<UWeaponDataProvider>());
		new(out_Fields) FUIDataProviderField( TEXT("Weapons"), Providers );

		check(ProviderTypes.PowerupDataProviderClass);
		Providers(0) = ProviderTypes.PowerupDataProviderClass->GetDefaultObject<UPowerupDataProvider>();
		new(out_Fields) FUIDataProviderField( TEXT("Powerups"), Providers );
	}
	else if ( PlayerData != NULL )
	{
		PlayerData->GetSupportedDataFields(out_Fields);

		if ( CurrentWeapon != NULL )
		{
			new(out_Fields) FUIDataProviderField( TEXT("CurrentWeapon"), DATATYPE_Provider, CurrentWeapon );
		}

		//@todo - WeaponList
		//@todo - PowerupList
	}

	Super::GetSupportedDataFields(out_Fields);
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UPlayerOwnerDataStore::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( PlayerData != NULL )
	{
		// pass it through to the underlying player data provider
		bResult = PlayerData->GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
	}

	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/* === UIDataStore interface === */
/**
 * Hook for performing any initialization required for this data store
 */
void UUIDataStore_SessionSettings::InitializeDataStore()
{
	Super::InitializeDataStore();

	// load the classes specified in SessionSettingsProviderClassNames and save the references in SessionSettingsProviderClasses
	for ( INT ClassIndex = 0; ClassIndex < SessionSettingsProviderClassNames.Num(); ClassIndex++ )
	{
		UClass* ProviderClass = LoadClass<USessionSettingsProvider>(NULL, *SessionSettingsProviderClassNames(ClassIndex), NULL, LOAD_None, NULL);
		if ( ProviderClass != NULL )
		{
			SessionSettingsProviderClasses.AddUniqueItem(ProviderClass);
		}
		else
		{
			debugf(NAME_Error, TEXT("Invalid SessionSettingsProviderClassNames specified (%i): %s"), ClassIndex, *SessionSettingsProviderClassNames(ClassIndex));
		}	
	}

	if ( GIsGame )
	{
		for ( INT ProviderIndex = 0; ProviderIndex < SessionSettingsProviderClasses.Num(); ProviderIndex++ )
		{
			UClass* ProviderClass = SessionSettingsProviderClasses(ProviderIndex);
			check(ProviderClass);

			USessionSettingsProvider* Provider = ConstructObject<USessionSettingsProvider>(ProviderClass, this);
			if ( Provider != NULL )
			{
				SessionSettings.AddItem(Provider);
			}
		}
	}
}

/* === UIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIDataStore_SessionSettings::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	// data stores empty the array, data providers append to the array
	out_Fields.Empty();

	if ( GIsEditor && !GIsGame )
	{
		// if we're in the UI editor, we won't have any data providers assigned since they're all dynamic
		// so we'll use the configured provider classes to retrieve the list of properties that could possibly
		// be accessed through the data store.
		for ( INT ProviderIndex = 0; ProviderIndex < SessionSettingsProviderClasses.Num(); ProviderIndex++ )
		{
			UClass* ProviderClass = SessionSettingsProviderClasses(ProviderIndex);
			check(ProviderClass);

			USessionSettingsProvider* DefaultProvider = ProviderClass->GetDefaultObject<USessionSettingsProvider>();
			check(DefaultProvider);

			new(out_Fields) FUIDataProviderField( DefaultProvider->ProviderTag, DATATYPE_Provider, DefaultProvider );
		}
	}
	else
	{
		for ( INT ProviderIndex = 0; ProviderIndex < SessionSettings.Num(); ProviderIndex++ )
		{
			USessionSettingsProvider* Provider = SessionSettings(ProviderIndex);
			new(out_Fields) FUIDataProviderField( Provider->ProviderTag, DATATYPE_Provider, Provider );
		}
	}

	Super::GetSupportedDataFields(out_Fields);
}

/* ==========================================================================================================
	UUIDataStore_PlayerSettings
========================================================================================================== */
/* === UUIDataStore_PlayerSettings interface === */
/**
 * Returns a pointer to the ULocalPlayer that this PlayerSettingsProvdier provider settings data for
 */
ULocalPlayer* UUIDataStore_PlayerSettings::GetPlayerOwner() const
{
	ULocalPlayer* Result = NULL;

	if ( GEngine != NULL && GEngine->GamePlayers.IsValidIndex(PlayerIndex) )
	{
		Result = GEngine->GamePlayers(PlayerIndex);
	}

	return Result;
}

/* === UIDataStore interface === */
/**
 * Hook for performing any initialization required for this data store
 */
void UUIDataStore_PlayerSettings::InitializeDataStore()
{
	Super::InitializeDataStore();

	// load the classes specified in PlayerSettingsProviderClassNames and save the references in PlayerSettingsProviderClasses
	for ( INT ClassIndex = 0; ClassIndex < PlayerSettingsProviderClassNames.Num(); ClassIndex++ )
	{
		UClass* ProviderClass = LoadClass<UPlayerSettingsProvider>(NULL, *PlayerSettingsProviderClassNames(ClassIndex), NULL, LOAD_None, NULL);
		if ( ProviderClass != NULL )
		{
			// store the reference
			PlayerSettingsProviderClasses.AddUniqueItem(ProviderClass);
		}
		else
		{
			debugf(NAME_Error, TEXT("Invalid PlayerSettingsProviderClassName specified (%i): %s"), ClassIndex, *PlayerSettingsProviderClassNames(ClassIndex));
		}
	}
}

/**
 * Called when this data store is added to the data store manager's list of active data stores.
 *
 * @param	PlayerOwner		the player that will be associated with this DataStore.  Only relevant if this data store is
 *							associated with a particular player; NULL if this is a global data store.
 */
void UUIDataStore_PlayerSettings::OnRegister( ULocalPlayer* PlayerOwner )
{
	if ( GEngine != NULL && PlayerOwner != NULL )
	{
		// set the PlayerIndex for this player settings data store
		PlayerIndex = GEngine->GamePlayers.FindItemIndex(PlayerOwner);
		if ( GIsGame )
		{
			check(PlayerIndex != INDEX_NONE);
			for ( INT ProviderIndex = 0; ProviderIndex < PlayerSettingsProviderClasses.Num(); ProviderIndex++ )
			{
				UClass* ProviderClass = PlayerSettingsProviderClasses(ProviderIndex);
				check(ProviderClass);

				UPlayerSettingsProvider* Provider = ConstructObject<UPlayerSettingsProvider>(ProviderClass, this);
				if ( Provider != NULL )
				{
					PlayerSettings.AddItem(Provider);
					// Give the player settings provider a chance to do any per player startup
					Provider->OnRegister(PlayerOwner);
				}
			}
		}
	}

	Super::OnRegister(PlayerOwner);
}

/* === UIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIDataStore_PlayerSettings::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	// data stores empty the array, data providers append to the array
	out_Fields.Empty();

	if ( GIsEditor && !GIsGame )
	{
		// if we're in the UI editor, we won't have any data providers assigned since they're all dynamic
		// so we'll use the configured provider classes to retrieve the list of properties that could possibly
		// be accessed through the data store.
		for ( INT ProviderIndex = 0; ProviderIndex < PlayerSettingsProviderClasses.Num(); ProviderIndex++ )
		{
			UClass* ProviderClass = PlayerSettingsProviderClasses(ProviderIndex);
			check(ProviderClass);

			UPlayerSettingsProvider* DefaultProvider = ProviderClass->GetDefaultObject<UPlayerSettingsProvider>();
			check(DefaultProvider);

			new(out_Fields) FUIDataProviderField( DefaultProvider->ProviderTag, DATATYPE_Provider, DefaultProvider );
		}
	}
	else
	{
		for ( INT ProviderIndex = 0; ProviderIndex < PlayerSettings.Num(); ProviderIndex++ )
		{
			UPlayerSettingsProvider* Provider = PlayerSettings(ProviderIndex);
			new(out_Fields) FUIDataProviderField( Provider->ProviderTag, DATATYPE_Provider, Provider );
		}
	}

	Super::GetSupportedDataFields(out_Fields);
}

/* ==========================================================================================================
	UUIPropertyDataProvider
========================================================================================================== */
/**
 * Returns whether the specified property type is renderable in the UI.
 *
 * @param	Property				the property to check
 * @param	bRequireNativeSupport	TRUE to require the property to be natively supported (i.e. don't check whether it's supported in script).
 *
 * @return	TRUE if this property type is something that can be rendered in the UI.
 *
 * @note: can't be const it must call into the script VM, where we can't guarantee that the object's state won't be changed.
 */
UBOOL UUIPropertyDataProvider::IsValidProperty( UProperty* Property, UBOOL bRequireNativeSupport/*=FALSE*/ )
{
	UBOOL bResult = FALSE;
	if ( Property != NULL )
	{
		UClass* PropertyClass = Property->GetClass();
		bResult = !ComplexPropertyTypes.ContainsItem(PropertyClass);
		if ( !bResult )
		{
			UStructProperty* StructProp = Cast<UStructProperty>(Property);
			if ( StructProp != NULL )
			{
				const FName StructName = StructProp->Struct->GetFName();
				if ( StructName == TEXT("UIRangeData") || StructName == TEXT("UniqueNetId") )
				{
					// these are the only two struct types we support be default
					bResult = TRUE;
				}
			}
		}
		
		if ( !bResult && !bRequireNativeSupport && DELEGATE_IS_SET(CanSupportComplexPropertyType) )
		{
			// allow script-only child classes to indicate that a property which would not normally
			// be considered valid will be handled by the class so it should be allowed
			bResult = delegateCanSupportComplexPropertyType(Property);
		}
	}

	return bResult;
}

/**
 * Builds a list of UProperties that are flagged for exposure to data stores from the specified class.
 *
 * @param	SourceClass		a pointer to a UClass that contains properties which are marked with the "databinding" keyword.
 *							Must be a child of the class assigned as the value for DataClass.
 * @param	out_Properties	will contain pointers to the properties of SourceClass which can be exposed to the data store system.
 */
void UUIPropertyDataProvider::GetProviderDataBindings( UClass* SourceClass, TArray<UProperty*>& out_Properties )
{
	check(SourceClass);

	for ( UProperty* Property = SourceClass->PropertyLink; Property; Property = Property->PropertyLinkNext )
	{
		if ( (Property->PropertyFlags&CPF_DataBinding) != 0 )
		{
			out_Properties.AddUniqueItem(Property);
		}
	}
}

/**
 * Wrapper for copying the property value for Prop into the data field value using the appropriate method.
 *
 * @param	Prop			the property that is being copied
 * @param	BaseAddress		pointer to the beginning of the block of data containing the value for the property; for example,
 *							if the property represents a class member property, you'd pass in 'this' as the value.
 * @param	ArrayIndex		specifies the index for the value; only relevant for array properties or static arrays
 * @param	out_FieldValue	receives the property value; tag and type will also be set to the appropriate value.
 *
 * @return	TRUE if the value was successfully copied into the output var.  FALSE if the property isn't supported.
 */
UBOOL UUIPropertyDataProvider::CopyPropertyValueIntoFieldValue( UProperty* Prop, BYTE* BaseDataAddress, INT ArrayIndex, FUIProviderFieldValue& out_FieldValue )
{
	UBOOL bResult = FALSE;

	if ( Prop != NULL && BaseDataAddress != NULL )
	{
		ArrayIndex = Clamp<INT>(ArrayIndex, INDEX_NONE, Prop->ArrayDim - 1);
		if ( IsValidProperty(Prop,TRUE) )
		{
			BYTE* PropertyValueAddress = BaseDataAddress + Prop->Offset;
			UProperty* CurrentProperty = Prop;

			if ( ArrayIndex != INDEX_NONE )
			{
				UArrayProperty* ArrayProp = Cast<UArrayProperty>(CurrentProperty);
				if ( ArrayProp != NULL )
				{
					CurrentProperty = ArrayProp->Inner;

					FScriptArray* ArrayValue = (FScriptArray*)PropertyValueAddress;
					if ( ArrayValue->IsValidIndex(ArrayIndex) )
					{
						PropertyValueAddress = (BYTE*)ArrayValue->GetData() + CurrentProperty->ElementSize * ArrayIndex;
					}
				}
				else
				{
					// if it's not a dynamic array, make sure it's within range
					if ( ArrayIndex >= 0 && ArrayIndex < CurrentProperty->ArrayDim )
					{
						PropertyValueAddress += (ArrayIndex * CurrentProperty->ElementSize);
					}
				}
			}


			UObjectProperty* ObjectProp = Cast<UObjectProperty>(CurrentProperty,CLASS_IsAUObjectProperty);
			if ( ObjectProp != NULL && ObjectProp->PropertyClass->IsChildOf(USurface::StaticClass()) )
			{
				// create an image node
				USurface* ImageResource = *(USurface**)PropertyValueAddress;

				out_FieldValue.PropertyTag = Prop->GetFName();
				out_FieldValue.PropertyType = DATATYPE_Property;
				out_FieldValue.ImageValue = ImageResource;
				bResult = TRUE;
			}

			else
			{
				// might be a UIRange
				UStructProperty* StructProp = Cast<UStructProperty>(CurrentProperty,CLASS_IsAUStructProperty);
				if ( StructProp != NULL 
				&&	(IsRangeValueStruct(StructProp->Struct) || IsUniqueNetIdStruct(StructProp->Struct)) )
				{
					out_FieldValue.PropertyTag = Prop->GetFName();
					if ( IsRangeValueStruct(StructProp->Struct) )
					{
						FUIRangeData* RangeValue = (FUIRangeData*)PropertyValueAddress;

						// create a range node
						out_FieldValue.PropertyType = DATATYPE_RangeProperty;
						out_FieldValue.RangeValue = *RangeValue;
					}
					else
					{
						FUniqueNetId* NetIdValue = (FUniqueNetId*)PropertyValueAddress;
						out_FieldValue.PropertyType = DATATYPE_NetIdProperty;
						out_FieldValue.NetIdValue = *NetIdValue;
						out_FieldValue.StringValue = UOnlineSubsystem::UniqueNetIdToString(*NetIdValue);
					}

					bResult = TRUE;
				}
				else
				{
					//@fixme ronp - if the property was a dynamic array and its inner prop is a struct, we have a problem here
					FString StringValue;
					CurrentProperty->ExportTextItem(StringValue, PropertyValueAddress, NULL, this, PPF_Localized);

					// create a string node
					out_FieldValue.PropertyTag = Prop->GetFName();
					out_FieldValue.PropertyType = DATATYPE_Property;
					out_FieldValue.StringValue = StringValue;

					bResult = TRUE;
				}
			}
		}
		else
		{
			// call into script to try to get the value for this property
			FUIProviderScriptFieldValue ScriptPropertyValue(EC_EventParm);
			ScriptPropertyValue.PropertyTag = Prop->GetFName();
			ScriptPropertyValue.PropertyType = DATATYPE_MAX;

			if ( eventGetCustomPropertyValue(ScriptPropertyValue, ArrayIndex) )
			{
				if ( ScriptPropertyValue.PropertyType == DATATYPE_MAX )
				{
					debugf(NAME_ScriptWarning, TEXT("%s::GetCustomPropertyValue: PropertyType wasn't set for ProviderScriptFieldValue '%s'; defaulting to DATATYPE_Property"), *GetName(), *ScriptPropertyValue.PropertyTag.ToString());
					ScriptPropertyValue.PropertyType = DATATYPE_Property;
				}

				out_FieldValue = ScriptPropertyValue;
				bResult = ScriptPropertyValue.HasValue();
			}
		}
	}

	return bResult;
}

/**
 * Creates a UIStringNode_Text using the source and render text specified.
 *
 * @param	PropertyName		the name for the property this text node will represent.  This value is set as the
 *								source text for the text node.
 * @param	RenderString		the text that should will be rendered by this text node.
 *
 * @return	a pointer to UIStringNode_Text which will render the string specified.
 */
FUIStringNode_Text* UUIPropertyDataProvider::CreateTextNode( const FString& PropertyName, const TCHAR* RenderString ) const
{
	FUIStringNode_Text* Result = NULL;
	
	if ( PropertyName.Len() > 0 && RenderString != NULL )
	{
		Result = new FUIStringNode_Text(*PropertyName);
		Result->SetRenderText(RenderString);
	}

	return Result;
}

/**
 * Creates a UIStringNode_Text using the source and render text specified.
 *
 * @param	PropertyName		the name for the property this image node will represent.  This value is set as the
 *								source text for the image node.
 * @param	RenderImage			the image that should be rendered by this image node.
 *
 * @return	a pointer to UIStringNode_Image which will render the image specified.
 */
FUIStringNode_Image* UUIPropertyDataProvider::CreateImageNode( const FString& PropertyName, USurface* RenderImage ) const
{
	FUIStringNode_Image* Result = NULL;

	if ( PropertyName.Len() > 0 )
	{
		UUITexture* ImageWrapper = CreateTextureWrapper(RenderImage);
		if ( ImageWrapper != NULL )
		{
			Result = new FUIStringNode_Image(*PropertyName);

			Result->RenderedImage = ImageWrapper;
			Result->RenderedImage->ImageTexture = RenderImage;
			Result->ForcedExtent.X = 0.f;
			Result->ForcedExtent.Y = 0.f;
		}
	}

	return Result;
}

/* ==========================================================================================================
	UUIResourceDataProvider
========================================================================================================== */
UBOOL UUIResourceDataProvider::IsCollectionProperty( FName FieldTag, UProperty** out_CollectionProperty/*=NULL*/ )
{
	UBOOL bResult = FALSE;

	UProperty* Property = FindFieldWithFlag<UProperty,CASTCLASS_UProperty>(GetClass(), FieldTag);
	if ( Property != NULL && IsValidProperty(Property,TRUE) )
	{
		bResult = Cast<UArrayProperty>(Property) != NULL || Property->ArrayDim > 1;
		if ( bResult && out_CollectionProperty != NULL )
		{
			*out_CollectionProperty = Property;
		}
	}

	return bResult;
}

/* === UUIResourceDataProvider interface === */
UBOOL UUIResourceDataProvider::GetCollectionProperties( TArray<UProperty*>& out_CollectionProperties )
{
	UBOOL bResult = FALSE;

	// iterate over all properties which are declared in this class or a child
	const INT ParentClassSize = ThisClass::Super::StaticClass()->GetPropertiesSize();
	for ( UProperty* Prop = GetClass()->PropertyLink; Prop; Prop = Prop->PropertyLinkNext )
	{
		if ( Prop->Offset < ParentClassSize )
		{
			break;
		}

		// any array property marked with the data binding keyword will considered a collection provider
		if ((!bDataBindingPropertiesOnly || Prop->HasAnyPropertyFlags(CPF_DataBinding))
		&&	(Prop->ArrayDim > 1 || Prop->IsA(UArrayProperty::StaticClass())) )
		{
			out_CollectionProperties.AddUniqueItem(Prop);
			bResult = TRUE;
		}
	}
	
	return bResult;
}

/**
 * Attempts to find a nested data provider given the parameters
 *
 * @param	CollectionProperty	the property that potentially holds the reference to the data provider
 * @param	CollectionIndex		the index into the collection for the data provider to retrieve; if the value is invalid, the default
 *								object for the property class will be used.
 * @param	InnerProvider		receives the reference to the provider, if found.
 *
 * @return	TRUE if the property held a reference to a data provider (even if a NULL provider was found).
 */
UBOOL UUIResourceDataProvider::GetNestedProvider( UProperty* CollectionProperty, INT CollectionIndex, UUIDataProvider*& InnerProvider )
{
	check(CollectionProperty);
	InnerProvider = NULL;

	UBOOL bResult = FALSE;
	if ( CollectionProperty->ArrayDim > 1 )
	{
		UObjectProperty* ProviderProp = Cast<UObjectProperty>(CollectionProperty);
		check(ProviderProp);

		if ( ProviderProp->PropertyClass->IsChildOf(UUIDataProvider::StaticClass()) )
		{
			if ( CollectionIndex >= 0 && CollectionIndex < CollectionProperty->ArrayDim )
			{
				InnerProvider = *(UUIDataProvider**)((BYTE*)this + CollectionProperty->Offset + CollectionProperty->ElementSize * CollectionIndex);
			}
			else
			{
				InnerProvider = Cast<UUIDataProvider>(ProviderProp->PropertyClass->GetDefaultObject());
			}
		}
		bResult = TRUE;
	}
	else
	{
		FScriptArray* ArrayValue = (FScriptArray*)((BYTE*)this + CollectionProperty->Offset);

		UObjectProperty* ProviderProp = Cast<UObjectProperty>(CastChecked<UArrayProperty>(CollectionProperty)->Inner);
		if ( ProviderProp != NULL )
		{
			if ( ArrayValue->IsValidIndex(CollectionIndex) )
			{
				InnerProvider = *(UUIDataProvider**)((BYTE*)ArrayValue->GetData() + ProviderProp->ElementSize * CollectionIndex);
			}
			else
			{
				InnerProvider = Cast<UUIDataProvider>(ProviderProp->PropertyClass->GetDefaultObject());
			}
			bResult = TRUE;
		}
		else
		{
			//@fixme - is this prop a struct?
		}
	}

	return bResult;
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUIResourceDataProvider::GetElementProviderTags()
{
	TArray<FName> ProviderTags;

	TArray<UProperty*> CollectionProperties;
	if ( GetCollectionProperties(CollectionProperties) )
	{
		for ( INT PropIdx = 0; PropIdx < CollectionProperties.Num(); PropIdx++ )
		{
			ProviderTags.AddItem(CollectionProperties(PropIdx)->GetFName());
		}
	}

	return ProviderTags;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUIResourceDataProvider::GetElementCount( FName FieldName )
{
	INT Result = 0;

	if ( FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);
				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
				{
					UProperty* CollectionProperty=NULL;
					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
					{
						UUIDataProvider* InnerProvider = NULL;
						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
						&&	InnerProvider != NULL)
						{
							IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(InnerProvider);
							if ( ElementProvider != NULL )
							{
								Result = ElementProvider->GetElementCount(*NextFieldName);
								break;
							}
						}
					}
				}

				break;
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}

	//@todo - what about script only subclasses?
	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUIResourceDataProvider::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	// FieldName should be the name of a member property of this class which holds collection data
	//@todo ronp - we should still perform the parsing in order to support additional layers of nested collections
	out_Elements.Empty();
	if ( FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);
				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
				{
					UProperty* CollectionProperty=NULL;
					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
					{
						UUIDataProvider* InnerProvider = NULL;
						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
						&&	InnerProvider != NULL)
						{
							IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(InnerProvider);
							if ( ElementProvider != NULL )
							{
								bResult = ElementProvider->GetListElements(*NextFieldName, out_Elements);
								if ( bResult )
								{
									break;
								}
							}
						}
					}
				}

				UProperty* CollectionProperty=NULL;
				if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
				{
					check(CollectionProperty);
					if ( CollectionProperty->ArrayDim > 1 )
					{
						for ( INT ValueIdx = 0; ValueIdx < CollectionProperty->ArrayDim; ValueIdx )
						{
							//@todo - support for filtered items
							out_Elements.AddItem(ValueIdx);
						}
					}
					else
					{
						FScriptArray* ArrayValue = (FScriptArray*)((BYTE*)this + CollectionProperty->Offset);
						for ( INT ValueIdx = 0; ValueIdx < ArrayValue->Num(); ValueIdx++ )
						{
							//@todo - support for filtered items
							out_Elements.AddItem(ValueIdx);
						}
					}

					bResult = TRUE;
					break;
				}
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}

	return bResult;
}

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
UBOOL UUIResourceDataProvider::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	UBOOL bResult = TRUE;

	if ( FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);
				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
				{
					UProperty* CollectionProperty=NULL;
					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
					{
						UUIDataProvider* InnerProvider = NULL;
						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
						&&	InnerProvider != NULL)
						{
							IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(InnerProvider);
							if ( ElementProvider != NULL )
							{
								bResult = ElementProvider->IsElementEnabled(*NextFieldName, CollectionIndex);
							}
							else
							{
								bResult = !InnerProvider->eventIsProviderDisabled();
							}
							break;
						}
					}
				}

				break;
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}
	//@todo -

	// FieldName should be the name of a member property of this class which holds collection data
// 	if ( FieldName == GetNavItemPropertyName() )
// 	{
// 		if ( NavigationItems.IsValidIndex(CollectionIndex) )
// 		{
// 			bResult = !NavigationItems(CollectionIndex).bItemDisabled;
// 		}
// 	}

	return bResult;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data field that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UUIResourceDataProvider::GetElementCellSchemaProvider( FName FieldName )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	// FieldName will contain the entire provider path minus the data store name so we need to parse it again
	if ( FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				// So FieldTag is the left-most node of the path, and NextFieldName contains the rest
				// pull off any array indices
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);
				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
				{
					UProperty* CollectionProperty=NULL;
					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
					{
						UUIDataProvider* InnerProvider = NULL;
						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
						&&	InnerProvider != NULL)
						{
							IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(InnerProvider);
							if ( ElementProvider != NULL )
							{
								Result = ElementProvider->GetElementCellSchemaProvider(*NextFieldName);
								break;
							}
// 							else
// 							{
// 								Result = this;
// 								break;
// 							}
						}
					}
				}

				Result = this;
				break;
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}

	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UUIResourceDataProvider::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	if ( FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;
		// this is called by our owning data store, after it has parsed the first portion of the tag.
		// FieldName will contain the entire provider path minus the data store name so we need to parse it again
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				// So FieldTag is the left-most node of the path, and NextFieldName contains the rest
				// pull off any array indices
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);
				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
				{
					UProperty* CollectionProperty=NULL;
					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
					{
						UUIDataProvider* InnerProvider = NULL;
						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
						&&	InnerProvider != NULL)
						{
							IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(InnerProvider);
							if ( ElementProvider != NULL )
							{
								Result = ElementProvider->GetElementCellValueProvider(*NextFieldName, ListIndex);
								if ( Result )
								{
									break;
								}
							}
						}
					}
				}

				if ( IsCollectionProperty(*FieldTag) )
				{
					Result = this;
				}
				break;
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}

	return Result;
}

/* === IUIListElementCellProvider interface === */
/**
 * Retrieves the list of tags corresponding to the cells that can be displayed for this list element.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
 */
void UUIResourceDataProvider::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	out_CellTags.Empty();

	UBOOL bProcessedByNestedProvider = FALSE;

	if ( FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				// So FieldTag is the left-most node of the path, and NextFieldName contains the rest
				// pull off any array indices
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);
				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
				{
					UProperty* CollectionProperty=NULL;
					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
					{
						UUIDataProvider* InnerProvider = NULL;
						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
						&&	InnerProvider != NULL)
						{
							IUIListElementCellProvider* SchemaProvider = InterfaceCast<IUIListElementCellProvider>(InnerProvider);
							if ( SchemaProvider != NULL )
							{
								SchemaProvider->GetElementCellTags(*NextFieldName, out_CellTags);
								bProcessedByNestedProvider = TRUE;
								break;
							}
						}
					}
					else
					{
						//@fixme - is this prop a struct?
					}
				}

				// if the last tag in the data field markup was a property supported by this class (though not a collection property),
				// make sure that it appears first in the list of cell tags.  UUIList::RefreshElementSchema() will always bind to the
				// first cell in the list of tags, so this allows us to bind a UIList and set the first cell of its schema at the same time.
				UProperty* Property = FindField<UProperty>(GetClass(), *FieldTag);
				if ( IsValidProperty(Property) )
				{
					out_CellTags.Set(Property->GetFName(), *Property->GetFriendlyName(GetClass()));
				}

				break;
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}

	if ( !bProcessedByNestedProvider )
	{
		const INT PropertyOffsetBoundary = UUIResourceDataProvider::Super::StaticClass()->GetPropertiesSize();
		for ( UProperty* Property = GetClass()->PropertyLink; Property && Property->Offset >= PropertyOffsetBoundary; Property = Property->PropertyLinkNext )
		{
			if ( IsValidProperty(Property) )
			{
				out_CellTags.Set(Property->GetFName(), *Property->GetFriendlyName(GetClass()));
			}
		}
	}
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUIResourceDataProvider::GetCellFieldType( FName FieldName, const FName& CellTag, BYTE& out_CellFieldType )
{
	UBOOL bResult = FALSE;

	UProperty* Property = FindFieldWithFlag<UProperty,CASTCLASS_UProperty>(GetClass(), CellTag);
	if ( Property != NULL && IsValidProperty(Property,TRUE) )
	{
		bResult = TRUE;
		out_CellFieldType = DATATYPE_Property;

		UArrayProperty* ArrayProp = Cast<UArrayProperty>(Property);
		if ( ArrayProp != NULL )
		{
			UObjectProperty* ObjectPropInner = Cast<UObjectProperty>(ArrayProp->Inner);
			if ( ObjectPropInner != NULL && ObjectPropInner->PropertyClass->IsChildOf(UUIDataProvider::StaticClass()) )
			{
				out_CellFieldType = DATATYPE_ProviderCollection;
			}
			else
			{
				out_CellFieldType = DATATYPE_Collection;
			}
		}
		else
		{
			UStructProperty* StructProp = Cast<UStructProperty>(Property);
			if ( StructProp == NULL && ArrayProp != NULL )
			{
				StructProp = Cast<UStructProperty>(ArrayProp->Inner);
			}

			if ( StructProp != NULL )
			{
				if ( IsRangeValueStruct(StructProp->Struct) )
				{
					out_CellFieldType = DATATYPE_RangeProperty;
				}
				else
				{
					if ( IsUniqueNetIdStruct(StructProp->Struct) )
					{
						out_CellFieldType = DATATYPE_NetIdProperty;
					}
					else
					{
						bResult = FALSE;
					}
				}
			}
		}
	}
	else if ( IsValidProperty(Property) )
	{
		// call into script to try to get the value for this property
		FUIProviderScriptFieldValue ScriptPropertyValue(EC_EventParm);
		ScriptPropertyValue.PropertyTag = Property->GetFName();
		ScriptPropertyValue.PropertyType = DATATYPE_MAX;

		if ( eventGetCustomPropertyValue(ScriptPropertyValue, INDEX_NONE) )
		{
			if ( ScriptPropertyValue.PropertyType != DATATYPE_MAX )
			{
				out_CellFieldType = ScriptPropertyValue.PropertyType;
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUIResourceDataProvider::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	FString FieldTag;
	FString NextFieldName = FieldName != NAME_None 
								? FieldName.ToString()
								: CellTag != NAME_None
									? CellTag.ToString()
									: TEXT("");

	TArray<FUIDataProviderField> SupportedFields;
	ParseNextDataTag(NextFieldName, FieldTag);
	while ( FieldTag.Len() > 0 ) 
	{
		if ( IsDataTagSupported(*FieldTag, SupportedFields) )
		{
			const INT NonCollectionArrayIndex = ParseArrayDelimiter(FieldTag);

			UProperty* Property = NULL;
			if ( NextFieldName.Len() > 0 && IsCollectionProperty(*FieldTag, &Property) )
			{
				UUIDataProvider* InnerProvider = NULL;
				if ( GetNestedProvider(Property, ListIndex, InnerProvider)
				&&	InnerProvider != NULL)
				{
					IUIListElementCellProvider* ElementValueProvider = InterfaceCast<IUIListElementCellProvider>(InnerProvider);
					if ( ElementValueProvider != NULL )
					{
						bResult = ElementValueProvider->GetCellFieldValue(*NextFieldName, CellTag, ListIndex, out_FieldValue, ArrayIndex);
					}
				}

				// otherwise, we're dealing with a normal array
				else if ( FieldName == NAME_None )
				{
					INT CollectionArrayIndex = ArrayIndex;
					if ( CollectionArrayIndex == INDEX_NONE && NonCollectionArrayIndex != INDEX_NONE )
					{
						CollectionArrayIndex = NonCollectionArrayIndex;
					}

					BYTE* PropertyAddressBase = (BYTE*)this + Property->Offset;
					UStructProperty* StructProp = NULL;
					if ( Property->ArrayDim > 1 )
					{
						StructProp = Cast<UStructProperty>(Property);
						PropertyAddressBase += Clamp(CollectionArrayIndex, 0, Property->ArrayDim) * Property->ElementSize;
					}

					if ( StructProp == NULL )
					{
						UArrayProperty* ArrayProp = CastChecked<UArrayProperty>(Property);
						StructProp = Cast<UStructProperty>(ArrayProp->Inner);

						FScriptArray* ArrayValue = (FScriptArray*)PropertyAddressBase;
						if ( ArrayValue->IsValidIndex(CollectionArrayIndex) )
						{
							PropertyAddressBase = (BYTE*)ArrayValue->GetData() + CollectionArrayIndex * StructProp->ElementSize;
						}
					}

					if ( StructProp != NULL )
					{
						//@fixme - we should really be parsing NextFieldName for additional levels of nesting like we do for the FieldName
						UProperty* NextProperty = FindField<UProperty>(StructProp->Struct, *NextFieldName);
						if ( NextProperty != NULL )
						{
							//@fixme - need to pass a valid value for ArrayIndex if NextFieldName contains more than one tag (i.e. SomeArray;ArrayIndex.AnotherNestedProperty)
							bResult = CopyPropertyValueIntoFieldValue(NextProperty, PropertyAddressBase, INDEX_NONE, out_FieldValue);
						}
					}

					// if FieldName is NAME_None, it means we were parsing the CellTag...so now we're done - if we haven't found the value by now it doesn't exist.
					break;
				}
			}

			if ( !bResult && CellTag == *FieldTag )
			{
				Property = FindField<UProperty>(GetClass(), CellTag);
				if ( Property != NULL )
				{
					if ( ArrayIndex == INDEX_NONE && NonCollectionArrayIndex != INDEX_NONE )
					{
						ArrayIndex = NonCollectionArrayIndex;
					}

					bResult = CopyPropertyValueIntoFieldValue(Property, (BYTE*)this, ArrayIndex, out_FieldValue);
					if ( bResult && ListIndex != INDEX_NONE && out_FieldValue.ArrayValue.Num() > 0 && out_FieldValue.StringValue.Len() > 0 )
					{
						out_FieldValue.ArrayValue.Empty();
					}
					break;
				}
			}

			break;
		}

		if ( NextFieldName.Len() == 0 && FieldName != NAME_None && CellTag != NAME_None )
		{
			// don't let this happen again - clear FieldName
			FieldName = NAME_None;
			NextFieldName = *CellTag.ToString();
		}
		ParseNextDataTag(NextFieldName, FieldTag);
	}

	return bResult;
}

/* === UUIPropertyDataProvider interface === */
/**
 * Returns whether the specified property type is renderable in the UI.
 *
 * @param	Property				the property to check
 * @param	bRequireNativeSupport	TRUE to require the property to be natively supported (i.e. don't check whether it's supported in script).
 *
 * @return	TRUE if this property type is something that can be rendered in the UI.
 *
 * @note: can't be const it must call into the script VM, where we can't guarantee that the object's state won't be changed.
 */
UBOOL UUIResourceDataProvider::IsValidProperty( UProperty* Property, UBOOL bRequireNativeSupport/*=FALSE*/ )
{
	UBOOL bResult = Super::IsValidProperty(Property,bRequireNativeSupport);
	if ( bResult )
	{
		bResult = !bDataBindingPropertiesOnly || Property->HasAnyPropertyFlags(CPF_DataBinding);
	}

	return bResult;
}

/* === UIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIResourceDataProvider::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();
	const INT PropertyOffsetBoundary = UUIResourceDataProvider::Super::StaticClass()->GetPropertiesSize();
	for ( UProperty* Property = GetClass()->PropertyLink; Property && Property->Offset >= PropertyOffsetBoundary; Property = Property->PropertyLinkNext )
	{
		if ( IsValidProperty(Property) )
		{
			// for each property contained by this ResourceDataProvider, add a provider field to expose them to the UI
			UArrayProperty* ArrayProp = Cast<UArrayProperty>(Property);
			if ( ArrayProp != NULL )
			{
				// see if this is an array of providers
				UObjectProperty* ObjectPropInner = Cast<UObjectProperty>(ArrayProp->Inner);
				if ( ObjectPropInner != NULL && ObjectPropInner->PropertyClass->IsChildOf(UUIDataProvider::StaticClass()) )
				{
					TArray<UUIDataProvider*> Providers;
					FScriptArray* ArrayValue = (FScriptArray*)((BYTE*)this + Property->Offset);
					for ( INT Idx = 0; Idx < ArrayValue->Num(); Idx++ )
					{
						UUIDataProvider* Provider = *(UUIDataProvider**)((BYTE*)ArrayValue->GetData() + ObjectPropInner->ElementSize * Idx);
						Providers.AddItem(Provider);
					}
			
					new(out_Fields) FUIDataProviderField( Property->GetFName(), Providers );
				}
				else
				{
					new(out_Fields) FUIDataProviderField( Property->GetFName(), DATATYPE_Collection );
				}
			}
			else
			{
				EUIDataProviderFieldType FieldType = DATATYPE_Property;
				UStructProperty* StructProp = Cast<UStructProperty>(Property);
				if ( StructProp != NULL )
				{
					if ( IsRangeValueStruct(StructProp->Struct) )
					{
						FieldType = DATATYPE_RangeProperty;
					}
					else if ( IsUniqueNetIdStruct(StructProp->Struct) )
					{
						FieldType = DATATYPE_NetIdProperty;
					}
				}

				new(out_Fields) FUIDataProviderField( Property->GetFName(), FieldType );
			}

		}
	}

	Super::GetSupportedDataFields(out_Fields);
}

/**
 * Resolves the value of the data cell specified by FieldName and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIResourceDataProvider::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	//@todo ronp - hmmmmmmm
	FName PropertyFName(*FieldName, FNAME_Find);
	if ( PropertyFName != NAME_None )
	{
		bResult = GetCellFieldValue(UCONST_UnknownCellDataFieldName,PropertyFName,ArrayIndex,out_FieldValue,INDEX_NONE);
	}

	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

#define DEBUG_RESOURCECOMBO_PROVIDER	0

/* ==========================================================================================================
	UUIResourceCombinationProvider
========================================================================================================== */
/* === UIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIResourceCombinationProvider::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	if ( StaticDataProvider != NULL )
	{
		StaticDataProvider->GetSupportedDataFields(out_Fields);
	}

	Super::GetSupportedDataFields(out_Fields);
}

/**
 * Resolves the value of the data cell specified by FieldName and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIResourceCombinationProvider::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( StaticDataProvider != NULL )
	{
		bResult = StaticDataProvider->GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
	}
#if DEBUG_RESOURCECOMBO_PROVIDER
	debugf(TEXT("(%s) UUIResourceCombinationProvider::GetFieldValue  FieldName:%s   bResult(static):%i"), *GetName(), *FieldName, bResult);
#endif

	// allow the dynamic provider to alter the result
	return Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex) || bResult;
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUIResourceCombinationProvider::GetElementProviderTags()
{
	TArray<FName> ProviderTags;

	if ( StaticDataProvider != NULL )
	{
		ProviderTags = StaticDataProvider->GetElementProviderTags();
	}
#if DEBUG_RESOURCECOMBO_PROVIDER
	debugf(TEXT("(%s) UUIResourceCombinationProvider::GetElementProviderTags  (static had %i tags)"), *GetName(), ProviderTags.Num());
#endif

	ProviderTags += eventGetElementProviderTags();
	return ProviderTags;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUIResourceCombinationProvider::GetElementCount( FName FieldName )
{
	INT Result = 0;

	if ( StaticDataProvider != NULL )
	{
		Result = StaticDataProvider->GetElementCount(FieldName);
	}
#if DEBUG_RESOURCECOMBO_PROVIDER
	debugf(TEXT("(%s) UUIResourceCombinationProvider::GetElementCount  FieldName:%s     Result:%i"), *GetName(), *FieldName.ToString(), Result);
#endif

	if ( FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);
				Result += eventGetElementCount(FieldName);

//@todo ronp - eventually make this generic enough that script implementations aren't needed.
// 				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
// 				{
// 					UProperty* CollectionProperty=NULL;
// 					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
// 					{
// 						UUIDataProvider* InnerProvider = NULL;
// 						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
// 						&&	InnerProvider != NULL)
// 						{
// 							IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(InnerProvider);
// 							if ( ElementProvider != NULL )
// 							{
// 								Result = ElementProvider->GetElementCount(*NextFieldName);
// 								break;
// 							}
// 						}
// 					}
// 				}

				break;
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}

	return Result;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUIResourceCombinationProvider::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	// FieldName should be the name of a member property of this class which holds collection data
	out_Elements.Empty();

#if DEBUG_RESOURCECOMBO_PROVIDER
//	debugf(TEXT("(%s) UUIResourceCombinationProvider::GetListElements  FieldName:%s"), *GetName(), *FieldName.ToString());
#endif

	if ( StaticDataProvider != NULL )
	{
		bResult = StaticDataProvider->GetListElements(FieldName, out_Elements);
	}

	if ( FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);
				if ( eventGetListElements(*FieldTag, out_Elements) )
				{
					bResult = TRUE;
					break;
				}

//@todo ronp - eventually make this generic enough that script implementations aren't needed.
// 				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
// 				{
// 					UProperty* CollectionProperty=NULL;
// 					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
// 					{
// 						UUIDataProvider* InnerProvider = NULL;
// 						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
// 						&&	InnerProvider != NULL)
// 						{
// 							IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(InnerProvider);
// 							if ( ElementProvider != NULL )
// 							{
// 								bResult = ElementProvider->GetListElements(*NextFieldName, out_Elements);
// 								if ( bResult )
// 								{
// 									break;
// 								}
// 							}
// 						}
// 					}
// 				}
// 
// 				UProperty* CollectionProperty=NULL;
// 				if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
// 				{
// 					check(CollectionProperty);
// 					if ( CollectionProperty->ArrayDim > 1 )
// 					{
// 						for ( INT ValueIdx = 0; ValueIdx < CollectionProperty->ArrayDim; ValueIdx )
// 						{
// 							//@todo - support for filtered items
// 							out_Elements.AddItem(ValueIdx);
// 						}
// 					}
// 					else
// 					{
// 						FScriptArray* ArrayValue = (FScriptArray*)((BYTE*)this + CollectionProperty->Offset);
// 						for ( INT ValueIdx = 0; ValueIdx < ArrayValue->Num(); ValueIdx++ )
// 						{
// 							//@todo - support for filtered items
// 							out_Elements.AddItem(ValueIdx);
// 						}
// 					}
// 
// 					bResult = TRUE;
// 					break;
// 				}
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}

	return bResult;
}

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
UBOOL UUIResourceCombinationProvider::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	UBOOL bResult = TRUE;

	if ( StaticDataProvider != NULL )
	{
		bResult = StaticDataProvider->IsElementEnabled(FieldName, CollectionIndex);
	}

#if DEBUG_RESOURCECOMBO_PROVIDER
	debugf(TEXT("(%s) UUIResourceCombinationProvider::IsElementEnabled  FieldName:%s    CollectionIndex:%i    bResult:%i"), *GetName(), *FieldName.ToString(), CollectionIndex, bResult);
#endif

	if ( FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);
				if ( eventIsElementEnabled(FieldName, CollectionIndex) )
				{
					bResult = TRUE;
					break;
				}
// 				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
// 				{
// 					UProperty* CollectionProperty=NULL;
// 					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
// 					{
// 						UUIDataProvider* InnerProvider = NULL;
// 						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
// 						&&	InnerProvider != NULL)
// 						{
// 							IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(InnerProvider);
// 							if ( ElementProvider != NULL )
// 							{
// 								bResult = ElementProvider->IsElementEnabled(*NextFieldName, CollectionIndex);
// 							}
// 							else
// 							{
// 								bResult = !InnerProvider->eventIsProviderDisabled();
// 							}
// 							break;
// 						}
// 					}
// 				}

				break;
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}

	return bResult;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data field that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UUIResourceCombinationProvider::GetElementCellSchemaProvider( FName FieldName )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	if ( StaticDataProvider != NULL )
	{
		Result = StaticDataProvider->GetElementCellSchemaProvider(FieldName);
	}

#if DEBUG_RESOURCECOMBO_PROVIDER
	debugf(TEXT("(%s) UUIResourceCombinationProvider::GetElementCellSchemaProvider  FieldName:%s    Result:%s"), *GetName(), *FieldName.ToString(), *(Result.GetObject()->GetName()));
#endif

	if ( Result )
	{
		// can't let the static provider become the schema provider or we'll not be able to access dynamic data
		Result = this;
	}
	else
	{
		TScriptInterface<IUIListElementCellProvider> ScriptResult;
		if ( eventGetElementCellSchemaProvider(FieldName, ScriptResult) )
		{
			Result = ScriptResult;
		}
	}

	// FieldName will contain the entire provider path minus the data store name so we need to parse it again
	if ( Result == this && FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				// So FieldTag is the left-most node of the path, and NextFieldName contains the rest
				// pull off any array indices
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);
				TScriptInterface<IUIListElementCellProvider> ScriptResult;
				if ( eventGetElementCellSchemaProvider(FieldName, ScriptResult) )
				{
					Result = ScriptResult;
				}
// 				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
// 				{
// 					UProperty* CollectionProperty=NULL;
// 					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
// 					{
// 						UUIDataProvider* InnerProvider = NULL;
// 						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
// 						&&	InnerProvider != NULL)
// 						{
// 							IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(InnerProvider);
// 							if ( ElementProvider != NULL )
// 							{
// 								Result = ElementProvider->GetElementCellSchemaProvider(*NextFieldName);
// 								break;
// 							}
// // 							else
// // 							{
// // 								Result = this;
// // 								break;
// // 							}
// 						}
// 					}
// 				}
// 
				Result = this;
				break;
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}

	if ( !Result )
	{
		Result = this;
	}

	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UUIResourceCombinationProvider::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	if ( StaticDataProvider != NULL )
	{
		Result = StaticDataProvider->GetElementCellValueProvider(FieldName, ListIndex);
	}

#if DEBUG_RESOURCECOMBO_PROVIDER
	debugf(TEXT("(%s) UUIResourceCombinationProvider::GetElementCellValueProvider  FieldName:%s   ListIndex:%i  Result:%s"), *GetName(), *FieldName.ToString(), ListIndex, *(Result.GetObject()->GetName()));
#endif

	if ( Result )
	{
		// intercept calls to anything that would have come from the static provider
		Result = this;
	}
	else
	{
		TScriptInterface<IUIListElementCellProvider> ScriptResult;
		if ( eventGetElementCellValueProvider(FieldName, ListIndex, ScriptResult) )
		{
			Result = ScriptResult;
		}
	}

	if ( Result == this && FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;

		// this is called by our owning data store, after it has parsed the first portion of the tag.
		// FieldName will contain the entire provider path minus the data store name so we need to parse it again
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);

				TScriptInterface<IUIListElementCellProvider> ScriptResult;
				if ( eventGetElementCellValueProvider(*FieldTag, ListIndex, ScriptResult) )
				{
					Result = ScriptResult;
					break;
				}
// 				// So FieldTag is the left-most node of the path, and NextFieldName contains the rest
// 				// pull off any array indices
// 				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
// 				{
// 					UProperty* CollectionProperty=NULL;
// 					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
// 					{
// 						UUIDataProvider* InnerProvider = NULL;
// 						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
// 						&&	InnerProvider != NULL)
// 						{
// 							IUIListElementProvider* ElementProvider = InterfaceCast<IUIListElementProvider>(InnerProvider);
// 							if ( ElementProvider != NULL )
// 							{
// 								Result = ElementProvider->GetElementCellValueProvider(*NextFieldName, ListIndex);
// 								if ( Result )
// 								{
// 									break;
// 								}
// 							}
// 						}
// 					}
// 				}
// 
// 				if ( IsCollectionProperty(*FieldTag) )
// 				{
// 					Result = this;
// 				}
				break;
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}

	return Result;
}

/* === IUIListElementCellProvider interface === */
/**
 * Retrieves the list of tags corresponding to the cells that can be displayed for this list element.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
 */
void UUIResourceCombinationProvider::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	if ( StaticDataProvider != NULL )
	{
		StaticDataProvider->GetElementCellTags(FieldName, out_CellTags);
	}

#if DEBUG_RESOURCECOMBO_PROVIDER
	debugf(TEXT("(%s) UUIResourceCombinationProvider::GetElementCellTags  FieldName:%s  tags from static:%i"), *GetName(), *FieldName.ToString(), out_CellTags.Num());
#endif

// 	UBOOL bProcessedByNestedProvider = FALSE;
	if ( FieldName != NAME_None )
	{
		TArray<FUIDataProviderField> SupportedFields;
		FString NextFieldName = FieldName.ToString(), FieldTag;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				// So FieldTag is the left-most node of the path, and NextFieldName contains the rest
				// pull off any array indices
				INT InstanceIndex = ParseArrayDelimiter(FieldTag);

				TArray<FName> ScriptDefinedTags;
				TArray<FString> ScriptDefinedColumnHeaderText;
				eventGetElementCellTags(FieldName, ScriptDefinedTags, &ScriptDefinedColumnHeaderText);
				for ( INT TagIndex = 0; TagIndex < ScriptDefinedTags.Num(); TagIndex++ )
				{
					if ( ScriptDefinedColumnHeaderText.IsValidIndex(TagIndex) )
					{
						out_CellTags.Set(ScriptDefinedTags(TagIndex), ScriptDefinedColumnHeaderText(TagIndex));
					}
					else
					{
						out_CellTags.Set(ScriptDefinedTags(TagIndex), *ScriptDefinedTags(TagIndex).ToString());
					}
				}

// 				if ( NextFieldName.Len() > 0 || InstanceIndex != INDEX_NONE )
// 				{
// 					UProperty* CollectionProperty=NULL;
// 					if ( IsCollectionProperty(*FieldTag, &CollectionProperty) )
// 					{
// 						UUIDataProvider* InnerProvider = NULL;
// 						if ( GetNestedProvider(CollectionProperty, InstanceIndex, InnerProvider)
// 						&&	InnerProvider != NULL)
// 						{
// 							IUIListElementCellProvider* SchemaProvider = InterfaceCast<IUIListElementCellProvider>(InnerProvider);
// 							if ( SchemaProvider != NULL )
// 							{
// 								SchemaProvider->GetElementCellTags(*NextFieldName, out_CellTags);
// 								bProcessedByNestedProvider = TRUE;
// 								break;
// 							}
// 						}
// 					}
// 					else
// 					{
// 						//@fixme - is this prop a struct?
// 					}
// 				}
// 
// 				// if the last tag in the data field markup was a property supported by this class (though not a collection property),
// 				// make sure that it appears first in the list of cell tags.  UUIList::RefreshElementSchema() will always bind to the
// 				// first cell in the list of tags, so this allows us to bind a UIList and set the first cell of its schema at the same time.
// 				UProperty* Property = FindField<UProperty>(GetClass(), *FieldTag);
// 				if ( IsValidProperty(Property) )
// 				{
// 					out_CellTags.Set(Property->GetFName(), *Property->GetFriendlyName(GetClass()));
// 				}

				break;
			}

			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}
// 
// 	if ( !bProcessedByNestedProvider )
// 	{
// 		const INT PropertyOffsetBoundary = UUIResourceCombinationProvider::Super::StaticClass()->GetPropertiesSize();
// 		for ( UProperty* Property = GetClass()->PropertyLink; Property && Property->Offset >= PropertyOffsetBoundary; Property = Property->PropertyLinkNext )
// 		{
// 			if ( IsValidProperty(Property) )
// 			{
// 				out_CellTags.Set(Property->GetFName(), *Property->GetFriendlyName(GetClass()));
// 			}
// 		}
// 	}
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUIResourceCombinationProvider::GetCellFieldType( FName FieldName, const FName& CellTag, BYTE& out_CellFieldType )
{
	UBOOL bResult = FALSE;

	if ( StaticDataProvider != NULL )
	{
		bResult = StaticDataProvider->GetCellFieldType(FieldName, CellTag, out_CellFieldType);
	}

#if DEBUG_RESOURCECOMBO_PROVIDER
	debugf(TEXT("(%s) UUIResourceCombinationProvider::GetCellFieldType  FieldName:%s   CellTag:%s  bResult:%i"), *GetName(), *FieldName.ToString(), *CellTag.ToString(), bResult);
#endif

// 	UProperty* Property = FindFieldWithFlag<UProperty,CASTCLASS_UProperty>(GetClass(), CellTag);
// 	if ( Property != NULL && IsValidProperty(Property,TRUE) )
// 	{
// 		bResult = TRUE;
// 		out_CellFieldType = DATATYPE_Property;
// 
// 		UArrayProperty* ArrayProp = Cast<UArrayProperty>(Property);
// 		if ( ArrayProp != NULL )
// 		{
// 			UObjectProperty* ObjectPropInner = Cast<UObjectProperty>(ArrayProp->Inner);
// 			if ( ObjectPropInner != NULL && ObjectPropInner->PropertyClass->IsChildOf(UUIDataProvider::StaticClass()) )
// 			{
// 				out_CellFieldType = DATATYPE_ProviderCollection;
// 			}
// 			else
// 			{
// 				out_CellFieldType = DATATYPE_Collection;
// 			}
// 		}
// 		else
// 		{
// 			UStructProperty* StructProp = Cast<UStructProperty>(Property);
// 			if ( StructProp == NULL && ArrayProp != NULL )
// 			{
// 				StructProp = Cast<UStructProperty>(ArrayProp->Inner);
// 			}
// 
// 			if ( StructProp != NULL )
// 			{
// 				if ( IsRangeValueStruct(StructProp->Struct) )
// 				{
// 					out_CellFieldType = DATATYPE_RangeProperty;
// 				}
// 				else
// 				{
// 					if ( IsUniqueNetIdStruct(StructProp->Struct) )
// 					{
// 						out_CellFieldType = DATATYPE_NetIdProperty;
// 					}
// 					else
// 					{
// 						bResult = FALSE;
// 					}
// 				}
// 			}
// 		}
// 	}
// 	else if ( IsValidProperty(Property) )
// 	{
// 		// call into script to try to get the value for this property
// 		FUIProviderScriptFieldValue ScriptPropertyValue(EC_EventParm);
// 		ScriptPropertyValue.PropertyTag = Property->GetFName();
// 		ScriptPropertyValue.PropertyType = DATATYPE_MAX;
// 
// 		if ( eventGetCustomPropertyValue(ScriptPropertyValue, INDEX_NONE) )
// 		{
// 			if ( ScriptPropertyValue.PropertyType != DATATYPE_MAX )
// 			{
// 				out_CellFieldType = ScriptPropertyValue.PropertyType;
// 				bResult = TRUE;
// 			}
// 		}
// 	}

	return eventGetCellFieldType(FieldName, CellTag, out_CellFieldType) || bResult;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUIResourceCombinationProvider::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( StaticDataProvider != NULL )
	{
		bResult = StaticDataProvider->GetCellFieldValue(FieldName, CellTag, ListIndex, out_FieldValue, ArrayIndex);
	}

#if DEBUG_RESOURCECOMBO_PROVIDER
	debugf(TEXT("(%s) UUIResourceCombinationProvider::GetCellFieldValue %i   FieldName:%s     CellTag:%s   bResult:%i"), *GetName(), ListIndex, *FieldName.ToString(), *CellTag.ToString(), bResult);
#endif

	if ( !bResult )
	{
		FString FieldTag;
		FString NextFieldName = FieldName != NAME_None 
			? FieldName.ToString()
			: CellTag != NAME_None
			? CellTag.ToString()
			: TEXT("");

		TArray<FUIDataProviderField> SupportedFields;
		ParseNextDataTag(NextFieldName, FieldTag);
		while ( FieldTag.Len() > 0 ) 
		{
			if ( IsDataTagSupported(*FieldTag, SupportedFields) )
			{
				const INT NonCollectionArrayIndex = ParseArrayDelimiter(FieldTag);
				if ( eventGetCellFieldValue(*FieldTag, CellTag, ListIndex, out_FieldValue, ArrayIndex) )
				{
					bResult = TRUE;
					break;
				}

// 				UProperty* Property = NULL;
// 				if ( NextFieldName.Len() > 0 && IsCollectionProperty(*FieldTag, &Property) )
// 				{
// 					UUIDataProvider* InnerProvider = NULL;
// 					if ( GetNestedProvider(Property, ListIndex, InnerProvider)
// 					&&	InnerProvider != NULL)
// 					{
// 						IUIListElementCellProvider* ElementValueProvider = InterfaceCast<IUIListElementCellProvider>(InnerProvider);
// 						if ( ElementValueProvider != NULL )
// 						{
// 							bResult = ElementValueProvider->GetCellFieldValue(*NextFieldName, CellTag, ListIndex, out_FieldValue, ArrayIndex);
// 						}
// 					}
// 
// 					// otherwise, we're dealing with a normal array
// 					else if ( FieldName == NAME_None )
// 					{
// 						INT CollectionArrayIndex = ArrayIndex;
// 						if ( CollectionArrayIndex == INDEX_NONE && NonCollectionArrayIndex != INDEX_NONE )
// 						{
// 							CollectionArrayIndex = NonCollectionArrayIndex;
// 						}
// 
// 						BYTE* PropertyAddressBase = (BYTE*)this + Property->Offset;
// 						UStructProperty* StructProp = NULL;
// 						if ( Property->ArrayDim > 1 )
// 						{
// 							StructProp = Cast<UStructProperty>(Property);
// 							PropertyAddressBase += Clamp(CollectionArrayIndex, 0, Property->ArrayDim) * Property->ElementSize;
// 						}
// 
// 						if ( StructProp == NULL )
// 						{
// 							UArrayProperty* ArrayProp = CastChecked<UArrayProperty>(Property);
// 							StructProp = Cast<UStructProperty>(ArrayProp->Inner);
// 
// 							FScriptArray* ArrayValue = (FScriptArray*)PropertyAddressBase;
// 							if ( ArrayValue->IsValidIndex(CollectionArrayIndex) )
// 							{
// 								PropertyAddressBase = (BYTE*)ArrayValue->GetData() + CollectionArrayIndex * StructProp->ElementSize;
// 							}
// 						}
// 
// 						if ( StructProp != NULL )
// 						{
// 							//@fixme - we should really be parsing NextFieldName for additional levels of nesting like we do for the FieldName
// 							UProperty* NextProperty = FindField<UProperty>(StructProp->Struct, *NextFieldName);
// 							if ( NextProperty != NULL )
// 							{
// 								//@fixme - need to pass a valid value for ArrayIndex if NextFieldName contains more than one tag (i.e. SomeArray;ArrayIndex.AnotherNestedProperty)
// 								bResult = CopyPropertyValueIntoFieldValue(NextProperty, PropertyAddressBase, INDEX_NONE, out_FieldValue);
// 							}
// 						}
// 
// 						// if FieldName is NAME_None, it means we were parsing the CellTag...so now we're done - if we haven't found the value by now it doesn't exist.
// 						break;
// 					}
// 				}
// 
// 				if ( !bResult && CellTag == *FieldTag )
// 				{
// 					Property = FindField<UProperty>(GetClass(), CellTag);
// 					if ( Property != NULL )
// 					{
// 						if ( ArrayIndex == INDEX_NONE && NonCollectionArrayIndex != INDEX_NONE )
// 						{
// 							ArrayIndex = NonCollectionArrayIndex;
// 						}
// 
// 						bResult = CopyPropertyValueIntoFieldValue(Property, (BYTE*)this, ArrayIndex, out_FieldValue);
// 						if ( bResult && ListIndex != INDEX_NONE && out_FieldValue.ArrayValue.Num() > 0 && out_FieldValue.StringValue.Len() > 0 )
// 						{
// 							out_FieldValue.ArrayValue.Empty();
// 						}
// 						break;
// 					}
// 				}

				break;
			}

			if ( NextFieldName.Len() == 0 && FieldName != NAME_None && CellTag != NAME_None )
			{
				// don't let this happen again - clear FieldName
				FieldName = NAME_None;
				NextFieldName = *CellTag.ToString();
			}
			ParseNextDataTag(NextFieldName, FieldTag);
		}
	}

	return GetFieldValue(CellTag.ToString(), out_FieldValue, ArrayIndex) || bResult;
}

/* ==========================================================================================================
	UUIDynamicDataProvider
========================================================================================================== */
/**
 * Associates this data provider with the specified instance.  
 *
 * @param	DataSourceInstance	a pointer to the object instance that this data provider should present data for.  DataSourceInstance
 *								must be of type DataClass.
 *
 * @return	TRUE if the instance specified was successfully associated with this data provider.  FALSE if the object specified
 *			wasn't of the correct type or was otherwise invalid.
 */
UBOOL UUIDynamicDataProvider::BindProviderInstance( UObject* DataSourceInstance )
{
	UBOOL bResult = FALSE;

	check(DataClass != NULL);
	if ( DataSourceInstance != NULL && DataSourceInstance->IsA(DataClass) )
	{
		DataSource = DataSourceInstance;

		UClass* DataProviderClass = DataSourceInstance->GetClass();
		for ( UProperty* Property = DataProviderClass->PropertyLink; Property; Property = Property->PropertyLinkNext )
		{
			if ( (Property->PropertyFlags&CPF_DataBinding) != 0 )
			{
				// do something
			}
		}

		eventProviderInstanceBound(DataSource);

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Clears the instance associated with this data provider.
 *
 * @return	TRUE if the instance reference was successfully cleared.
 */
UBOOL UUIDynamicDataProvider::UnbindProviderInstance()
{
	UObject* OldDataSource = DataSource;
	if ( OldDataSource != NULL )
	{
		DataSource = NULL;
		eventProviderInstanceUnbound(OldDataSource);
	}

	return TRUE;
}

/**
 * Determines whether the specified class should be represented by this dynamic data provider.
 *
 * @param	PotentialDataSourceClass	a pointer to a UClass that is being considered for binding by this provider.
 *
 * @return	TRUE to allow the databinding properties of PotentialDataSourceClass to be displayed in the UI editor's data store browser
 *			under this data provider.
 */
UBOOL UUIDynamicDataProvider::IsValidDataSourceClass( UClass* PotentialDataSourceClass )
{
	checkSlow(DataClass);
	UBOOL bResult = PotentialDataSourceClass != NULL && PotentialDataSourceClass->IsChildOf(DataClass);

	if ( bResult == TRUE )
	{
		// allow script to override this evaluation
		bResult = eventIsValidDataSourceClass(PotentialDataSourceClass);
	}

	return bResult;
}

/**
 * Builds an array of classes that are supported by this data provider.  Used in the editor to generate the list of 
 * supported data fields.  Since dynamic data providers are only created during the game, the editor needs a way to
 * retrieve the list of data field tags that can be bound without requiring instances of this data provider's DataClass to exist.
 */
void UUIDynamicDataProvider::GetSupportedClasses( TArray<UClass*>& out_Classes )
{
	if ( GIsEditor )
	{
		check(DataClass);

		out_Classes.Empty();
		for ( TObjectIterator<UClass> It; It; ++It )
		{
			if ( IsValidDataSourceClass(*It) )
			{
				out_Classes.AddItem(*It);
			}
		}
	}
	else
	{
		warnf(NAME_Warning, TEXT("UUIDynamicDataProvider::GetSupportedClasses should only be called in the editor!"));
	}
}

IMPLEMENT_COMPARE_POINTER(UProperty,UnUIDataStores_DynamicPropertyBinding,{ return appStricmp(*A->GetName(),*B->GetName()); });

/* === UUIDataProvider interface === */
/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDynamicDataProvider::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( DataSource != NULL )
	{
		UProperty* Property = FindFieldWithFlag<UProperty,CASTCLASS_UProperty>(DataSource->GetClass(), *FieldName);
		if ( Property != NULL )
		{
			if ( IsValidProperty(Property,TRUE) )
			{
//				if ( bMetaValue == FALSE )
				{
					BYTE* PropertyValueAddress = (BYTE*)DataSource + Property->Offset;

					if ( ArrayIndex != INDEX_NONE )
					{
						// for now, we only support static arrays in dynamic data providers
						//@todo ronp - fix this to correctly support dynamic arrays; once that has been implemented, will need to
						// ensure that IsValidProperty returns TRUE for UArrayProperty as well
						check(Property->ArrayDim>1);
						PropertyValueAddress += (ArrayIndex * Property->ElementSize);
					}

					UObjectProperty* ObjectProp = Cast<UObjectProperty>(Property);
					if ( ObjectProp != NULL && ObjectProp->PropertyClass->IsChildOf(USurface::StaticClass()) )
					{
						// create an image node
						USurface* ImageResource = *(USurface**)PropertyValueAddress;

						out_FieldValue.PropertyTag = *FieldName;
						out_FieldValue.PropertyType = DATATYPE_Property;
						out_FieldValue.ImageValue = ImageResource;
						bResult = TRUE;
					}

					else
					{
						// might be a UIRange
						UStructProperty* StructProp = Cast<UStructProperty>(Property);
						if ( StructProp != NULL )
						{
							if ( IsRangeValueStruct(StructProp->Struct) )
							{
								FUIRangeData* RangeValue = (FUIRangeData*)PropertyValueAddress;

								// create a range node
								out_FieldValue.PropertyTag = *FieldName;
								out_FieldValue.PropertyType = DATATYPE_RangeProperty;
								out_FieldValue.RangeValue = *RangeValue;
								bResult = TRUE;
							}
							else if ( IsUniqueNetIdStruct(StructProp->Struct) )
							{
								FUniqueNetId* NetIdValue = (FUniqueNetId*)PropertyValueAddress;

								out_FieldValue.PropertyTag = *FieldName;
								out_FieldValue.PropertyType = DATATYPE_NetIdProperty;
								out_FieldValue.StringValue = UOnlineSubsystem::UniqueNetIdToString(*NetIdValue);
								out_FieldValue.NetIdValue = *NetIdValue;
								bResult = TRUE;
							}
						}


						if ( !bResult )
						{
							FString StringValue;
							Property->ExportTextItem(StringValue, PropertyValueAddress, NULL, this, Property->PropertyFlags & PPF_Localized);

							// create a string node
							out_FieldValue.PropertyTag = *FieldName;
							out_FieldValue.PropertyType = DATATYPE_Property;
							out_FieldValue.StringValue = StringValue;
							bResult = TRUE;
						}
					}
				}
// 				else
// 				{
// 					Result = CreateTextNode(*CellTag, *CellTag);
// 				}
			}
			else if ( IsValidProperty(Property) )
			{
				// call into script to try to get the value for this property
				FUIProviderScriptFieldValue ScriptPropertyValue(EC_EventParm);
				ScriptPropertyValue.PropertyTag = Property->GetFName();
				ScriptPropertyValue.PropertyType = DATATYPE_MAX;

				if ( eventGetCustomPropertyValue(ScriptPropertyValue, ArrayIndex/*, bMetaValue*/) )
				{
					if ( ScriptPropertyValue.PropertyType == DATATYPE_MAX )
					{
						debugf(NAME_ScriptWarning, TEXT("%s::GetCustomPropertyValue: PropertyType wasn't set for ProviderScriptFieldValue '%s'; defaulting to DATATYPE_Property"), *GetName(), *ScriptPropertyValue.PropertyTag.ToString());
						ScriptPropertyValue.PropertyType = DATATYPE_Property;
					}
					out_FieldValue = ScriptPropertyValue;
					bResult = ScriptPropertyValue.HasValue();
				}
			}
		}
	}

	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/**
 * Generates filler data for a given tag.  This is used by the editor to generate a preview that gives the
 * user an idea as to what a bound datastore will look like in-game.
 *
 * @param		DataTag		the tag corresponding to the data field that we want filler data for
 *
 * @return		a string of made-up data which is indicative of the typical [resolved] value for the specified field.
 */
FString UUIDynamicDataProvider::GenerateFillerData( const FString& DataTag )
{
	FString Result;

	check(DataClass);
	UProperty* Property = FindFieldWithFlag<UProperty,CASTCLASS_UProperty>(DataSource->GetClass(), *DataTag);
	if ( Property != NULL )
	{
		if ( IsValidProperty(Property,TRUE) )
		{
			UObjectProperty* ObjectProp = Cast<UObjectProperty>(Property);
			if ( ObjectProp != NULL && ObjectProp->PropertyClass->IsChildOf(USurface::StaticClass()) )
			{
				Result = TEXT("{IMAGE}");
			}
			else
			{
				Result = FString::Printf(TEXT("An example %s value"), *Property->GetName());
			}
		}
		else if ( IsValidProperty(Property) )
		{
			// call into script to try to get the value for this property
			Result = eventGenerateFillerData(DataTag);
		}
	}

	return Result;
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIDynamicDataProvider::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	check(DataClass);

	TArray<UProperty*> BindableProperties;
	if ( DataSource != NULL )	// in game
	{
		// retrieve the list of properties that are available in the class that is currently bound to this data provider
		GetProviderDataBindings(DataSource->GetClass(), BindableProperties);
	}
	else if ( GIsEditor )		// in editor
	{
		// if we are in the editor, we won't have any instances of the data class associated with this data provider,
		// so expose all properties which could possibly be accessed through this data provider

		// get the list of classes that can be hooked up to this data provider
		TArray<UClass*> SupportedClasses;
		GetSupportedClasses(SupportedClasses);

		// retrieve the list of properties that might be accessed through this data provider
		for ( INT ClassIndex = 0; ClassIndex < SupportedClasses.Num(); ClassIndex++ )
		{
			UClass* SupportedClass = SupportedClasses(ClassIndex);

			GetProviderDataBindings(SupportedClass, BindableProperties);
		}
	}

	Sort<USE_COMPARE_POINTER(UProperty,UnUIDataStores_DynamicPropertyBinding)>( &BindableProperties(0), BindableProperties.Num()  );

	// now generate the list of data fields using the properties we collected
	for ( INT PropertyIndex = 0; PropertyIndex < BindableProperties.Num(); PropertyIndex++ )
	{
		UProperty* DataProperty = BindableProperties(PropertyIndex);
		if ( IsValidProperty(DataProperty) )
		{
			EUIDataProviderFieldType FieldType = DATATYPE_Property;

			// for each property contained by this ResourceDataProvider, add a provider field to expose them to the UI
			if ( Cast<UArrayProperty>(DataProperty) != NULL )
			{
				FieldType = DATATYPE_Collection;
			}
			else
			{
				UStructProperty* StructProp = Cast<UStructProperty>(DataProperty);
				if ( StructProp != NULL )
				{
					if ( IsRangeValueStruct(StructProp->Struct) )
					{
						FieldType = DATATYPE_RangeProperty;
					}
					else if ( IsUniqueNetIdStruct(StructProp->Struct) )
					{
						FieldType = DATATYPE_NetIdProperty;
					}
				}
			}

			// for each property contained by this ResourceDataProvider, add a provider field to expose them to the UI
			new(out_Fields) FUIDataProviderField( DataProperty->GetFName(), FieldType );
		}
	}
}

/**
 * Gets the list of data fields (and their localized friendly name) for the fields exposed this provider.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	receives the name/friendly name pairs for all data fields in this provider.
 */
void UUIDynamicDataProvider::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	if ( DataClass != NULL )
	{
		TArray<UProperty*> BindableProperties;
		if ( DataSource != NULL )	// in game
		{
			// retrieve the list of properties that are available in the class that is currently bound to this data provider
			GetProviderDataBindings(DataSource->GetClass(), BindableProperties);
		}
		else if ( GIsEditor )		// in editor
		{
			// if we are in the editor, we won't have any instances of the data class associated with this data provider,
			// so expose all properties which could possibly be accessed through this data provider

			// get the list of classes that can be hooked up to this data provider
			TArray<UClass*> SupportedClasses;
			GetSupportedClasses(SupportedClasses);

			// retrieve the list of properties that might be accessed through this data provider
			for ( INT ClassIndex = 0; ClassIndex < SupportedClasses.Num(); ClassIndex++ )
			{
				UClass* SupportedClass = SupportedClasses(ClassIndex);

				GetProviderDataBindings(SupportedClass, BindableProperties);
			}
		}

		Sort<USE_COMPARE_POINTER(UProperty,UnUIDataStores_DynamicPropertyBinding)>( &BindableProperties(0), BindableProperties.Num()  );

		// now generate the list of data fields using the properties we collected
		for ( INT PropertyIndex = 0; PropertyIndex < BindableProperties.Num(); PropertyIndex++ )
		{
			UProperty* DataProperty = BindableProperties(PropertyIndex);
			if ( IsValidProperty(DataProperty) )
			{
				// for each property contained by this ResourceDataProvider, add the name and friendly name of the property
				out_CellTags.Set(DataProperty->GetFName(), *DataProperty->GetFriendlyName());
			}
		}
	}
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName			the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *								instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUIDynamicDataProvider::GetCellFieldType( FName FieldName, const FName& CellTag, /*EUIDataProviderFieldType*/BYTE& out_CellFieldType )
{
	UBOOL bResult = FALSE;

	if ( DataClass != NULL )
	{
		TArray<UProperty*> BindableProperties;
		if ( GIsEditor )		// in editor
		{
			// if we are in the editor, we won't have any instances of the data class associated with this data provider,
			// so expose all properties which could possibly be accessed through this data provider

			// get the list of classes that can be hooked up to this data provider
			TArray<UClass*> SupportedClasses;
			GetSupportedClasses(SupportedClasses);

			// retrieve the list of properties that might be accessed through this data provider
			for ( INT ClassIndex = 0; ClassIndex < SupportedClasses.Num(); ClassIndex++ )
			{
				UClass* SupportedClass = SupportedClasses(ClassIndex);
				GetProviderDataBindings(SupportedClass, BindableProperties);
			}
		}
		else					// in game
		{
			UClass* BoundClass = DataClass;
			if ( DataSource != NULL )
			{
				BoundClass = DataSource->GetClass();
			}

			// retrieve the list of properties that are available in the class that is currently bound to this data provider
			GetProviderDataBindings(BoundClass, BindableProperties);
		}

		for ( INT PropIndex = 0; PropIndex < BindableProperties.Num(); PropIndex++ )
		{
			UProperty* Property = BindableProperties(PropIndex);
			if ( Property->GetFName() == CellTag )
			{
				checkf(IsValidProperty(Property),TEXT("%s::GetCellFieldType - IsValidProperty returned FALSE for %s"), *GetFullName(), *Property->GetPathName());

				bResult = TRUE;
				out_CellFieldType = DATATYPE_Property;

				UBOOL bIsCollection = Property->ArrayDim > 1;
				
				if ( !bIsCollection )
				{
					UArrayProperty* ArrayProp = Cast<UArrayProperty>(Property);
					if ( ArrayProp != NULL )
					{
						bIsCollection = TRUE;
						Property = ArrayProp->Inner;
					}
				}

				UObjectProperty* ObjectProp = Cast<UObjectProperty>(Property);
				if ( ObjectProp != NULL )
				{
					if ( ObjectProp->PropertyClass->IsChildOf(UUIDataProvider::StaticClass()) )
					{
						out_CellFieldType = bIsCollection ? DATATYPE_ProviderCollection : DATATYPE_Provider;
					}
				}
				else if ( bIsCollection )
				{
					out_CellFieldType = DATATYPE_Collection;
				}
				else
				{
					UStructProperty* StructProp = Cast<UStructProperty>(Property);
					if ( StructProp != NULL )
					{
						if ( IsRangeValueStruct(StructProp->Struct) )
						{
							out_CellFieldType = DATATYPE_RangeProperty;
						}
						else if ( IsUniqueNetIdStruct(StructProp->Struct) )
						{
							out_CellFieldType = DATATYPE_NetIdProperty;
						}
					}
				}
			}
		}
	}

	return bResult;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUIDynamicDataProvider::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( DataClass != NULL )
	{
		UObject* SourceObject = NULL;

		TArray<UProperty*> BindableProperties;
		if ( GIsEditor )		// in editor
		{
			// if we are in the editor, we won't have any instances of the data class associated with this data provider,
			// so expose all properties which could possibly be accessed through this data provider

			// get the list of classes that can be hooked up to this data provider
			TArray<UClass*> SupportedClasses;
			GetSupportedClasses(SupportedClasses);

			// retrieve the list of properties that might be accessed through this data provider
			for ( INT ClassIndex = 0; ClassIndex < SupportedClasses.Num(); ClassIndex++ )
			{
				UClass* SupportedClass = SupportedClasses(ClassIndex);
				GetProviderDataBindings(SupportedClass, BindableProperties);
			}
		}
		else					// in game
		{
			UClass* BoundClass = DataClass;
			if ( DataSource != NULL )
			{
				SourceObject = DataSource;
				BoundClass = DataSource->GetClass();
			}

			// retrieve the list of properties that are available in the class that is currently bound to this data provider
			GetProviderDataBindings(BoundClass, BindableProperties);
		}

		for ( INT PropIndex = 0; PropIndex < BindableProperties.Num(); PropIndex++ )
		{
			UProperty* Property = BindableProperties(PropIndex);
			if ( Property->GetFName() == CellTag )
			{
				// the default behavior is that each UIDynamicDataProvider corresponds to a single element in a list; therefore,
				// we pass ArrayIndex rather than ListIndex.
				bResult = CopyPropertyValueIntoFieldValue(Property, SourceObject ? (BYTE*)SourceObject : (BYTE*)Property->GetOwnerClass()->GetDefaultObject(), ArrayIndex, out_FieldValue);
				break;
			}
		}
	}

	return bResult;
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDynamicDataProvider::SetFieldValue( const FString& FieldName, const FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex )
{
	UBOOL bResult = FALSE;

	if ( DataSource != NULL && FieldValue.HasValue() )
	{
		UProperty* Property = FindFieldWithFlag<UProperty,CASTCLASS_UProperty>(DataSource->GetClass(), *FieldName);
		if ( Property != NULL )
		{
			if ( IsValidProperty(Property,TRUE) )
			{
//				if ( bMetaValue == FALSE )
				{
					BYTE* PropertyValueAddress = (BYTE*)DataSource + Property->Offset;

					if ( ArrayIndex != INDEX_NONE )
					{
						// for now, we only support static arrays in dynamic data providers
						//@todo ronp - fix this to correctly support dynamic arrays; once that has been implemented, will need to
						// ensure that IsValidProperty returns TRUE for UArrayProperty as well
						check(Property->ArrayDim>1);
						PropertyValueAddress += (ArrayIndex * Property->ElementSize);
					}

					UObjectProperty* ObjectProp = Cast<UObjectProperty>(Property);
					if ( ObjectProp != NULL && ObjectProp->PropertyClass->IsChildOf(USurface::StaticClass()) )
					{
						// create an image node
						USurface** ImageResource = (USurface**)PropertyValueAddress;
						
						(*ImageResource) = FieldValue.ImageValue;
						bResult = TRUE;
					}

					else
					{
						// might be a UIRange
						UStructProperty* StructProp = Cast<UStructProperty>(Property);
						if ( StructProp != NULL )
						{
							if ( IsRangeValueStruct(StructProp->Struct) )
							{
								FUIRangeData* RangeValue = (FUIRangeData*)PropertyValueAddress;

								// create a range node
								(*RangeValue) = FieldValue.RangeValue;
								bResult = TRUE;
							}
							else if ( IsUniqueNetIdStruct(StructProp->Struct) )
							{
								FUniqueNetId* NetIdValue = (FUniqueNetId*)PropertyValueAddress;
								if ( FieldValue.NetIdValue.HasValue() )
								{
									*NetIdValue = FieldValue.NetIdValue;
								}
								else
								{
									UOnlineSubsystem::StringToUniqueNetId(FieldValue.StringValue, *NetIdValue);
								}
								bResult = TRUE;
							}
						}


						if ( !bResult )
						{
							Property->ImportText(*FieldValue.StringValue, PropertyValueAddress, PPF_Localized, this);
							bResult = TRUE;
						}
					}
				}
// 				else
// 				{
// 					Result = CreateTextNode(*CellTag, *CellTag);
// 				}
			}
			else
			{
				// @todo: Not sure how to handle this case - ASM 6/15/2006
				//// call into script to try to get the value for this property
				//FUIProviderScriptFieldValue ScriptPropertyValue(EC_EventParm);
				//ScriptPropertyValue.PropertyTag = Property->GetFName();
				//ScriptPropertyValue.PropertyType = DATATYPE_MAX;

				//if ( eventGetCustomPropertyValue(ScriptPropertyValue, ArrayIndex/*, bMetaValue*/) )
				//{
				//	if ( ScriptPropertyValue.PropertyType == DATATYPE_MAX )
				//	{
				//		debugf(NAME_ScriptWarning, TEXT("%s::GetCustomPropertyValue: PropertyType wasn't set for ProviderScriptFieldValue '%s'; defaulting to DATATYPE_Property"), GetName(), *ScriptPropertyValue.PropertyTag);
				//		ScriptPropertyValue.PropertyType = DATATYPE_Property;
				//	}
				//	out_FieldValue = ScriptPropertyValue;
				//	bResult = ScriptPropertyValue.HasValue();
				//}
			}
		}
	}

	return bResult || Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);	
}

/* ==========================================================================================================
	UIDynamicFieldProvider
========================================================================================================== */
/**
 * Copies the elements from the PersistentDataFields array into the RuntimeDataFields array.  Should only be called once when the provider
 * is initialized.
 */
void UUIDynamicFieldProvider::InitializeRuntimeFields()
{
	if ( GIsGame && RuntimeDataFields.Num() == 0 )
	{
		for ( INT PersistentFieldIndex = 0; PersistentFieldIndex < PersistentDataFields.Num(); PersistentFieldIndex++ )
		{
			FUIProviderScriptFieldValue& PersistentField = PersistentDataFields(PersistentFieldIndex);
			new(RuntimeDataFields) FUIProviderScriptFieldValue(PersistentField);
		}
	}
}

/**
 * Adds a new data field to the list of supported fields.
 *
 * @param	FieldName			the name to give the new field
 * @param	FieldType			the type of data field being added
 * @param	bPersistent			specify TRUE to add the field to the PersistentDataFields array as well.
 * @param	out_InsertPosition	allows the caller to find out where the element was inserted
 *
 * @return	TRUE if the field was successfully added to the list; FALSE if the a field with that name already existed
 *			or the specified name was invalid.
 */
UBOOL UUIDynamicFieldProvider::AddField( FName FieldName, /*EUIDataProviderFieldType*/BYTE FieldType/*=DATATYPE_Property*/, UBOOL bPersistent/*=FALSE*/, INT* out_InsertPosition/*=NULL*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		// use a loop here to apply the change to both the RuntimeDataFields and PersistentDataFields arrays
		while ( true )
		{
			INT ExistingFieldIndex = FindFieldIndex(FieldName,bPersistent);
			if ( ExistingFieldIndex == INDEX_NONE )
			{
				TArrayNoInit<FUIProviderScriptFieldValue>& DataFields = (GIsGame && !bPersistent) ? RuntimeDataFields : PersistentDataFields;
				FUIProviderScriptFieldValue* NewField = new(DataFields) FUIProviderScriptFieldValue(EC_EventParm);
				NewField->PropertyTag = FieldName;
				NewField->PropertyType = FieldType;
				if ( out_InsertPosition != NULL )
				{
					*out_InsertPosition = DataFields.Num();
				}

				bResult = TRUE;
			}

			if ( bPersistent )
			{
				bPersistent = FALSE;
			}
			else
			{
				break;
			}
		}
	}

	return bResult;
}

/**
 * Removes the data field that has the specified tag.
 *
 * @param	FieldName	the name of the data field to remove from this data provider.
 *
 * @return	TRUE if the field was successfully removed from the list of supported fields or the field name wasn't in the list
 *			to begin with; FALSE if the name specified was invalid or the field couldn't be removed for some reason
 */
UBOOL UUIDynamicFieldProvider::RemoveField( FName FieldName )
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		INT ExistingFieldIndex = FindFieldIndex(FieldName);
		if ( ExistingFieldIndex != INDEX_NONE )
		{
			if ( GIsGame )
			{
				RuntimeDataFields.Remove(ExistingFieldIndex);
			}
			else
			{
				PersistentDataFields.Remove(ExistingFieldIndex);
			}
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Finds the index into the DataFields array for the data field specified.
 *
 * @param	FieldName	the name of the data field to search for
 *
 * @param	the index into the DataFields array for the data field specified, or INDEX_NONE if it isn't in the array.
 */
INT UUIDynamicFieldProvider::FindFieldIndex(FName FieldName, UBOOL bSearchPersistentFields/*=FALSE*/) const
{
	INT Result = INDEX_NONE;

	const TArrayNoInit<FUIProviderScriptFieldValue>& DataFields = (GIsGame && !bSearchPersistentFields) ? RuntimeDataFields : PersistentDataFields;
	for ( INT FieldIndex = 0; FieldIndex < DataFields.Num(); FieldIndex++ )
	{
		const FUIProviderScriptFieldValue& Field = DataFields(FieldIndex);
		if ( Field.PropertyTag == FieldName )
		{
			Result = FieldIndex;
			break;
		}
	}

	return Result;
}

/**
 * Removes all data fields from this data provider.
 *
 * @param	bReinitializeRuntimeFields	specify TRUE to reset the elements of the RuntimeDataFields array to match the elements
 *										in the PersistentDataFields array.  Ignored in the editor.
 *
 * @return	TRUE indicates that all fields were removed successfully; FALSE otherwise.
 */
UBOOL UUIDynamicFieldProvider::ClearFields( UBOOL bReinitializeRuntimeFields/*=TRUE*/ )
{
	if ( GIsGame )
	{
		RuntimeDataFields.Empty();

		if ( bReinitializeRuntimeFields == TRUE )
		{
			InitializeRuntimeFields();
		}
	}
	else
	{
		PersistentDataFields.Empty();
	}

	return TRUE;
}

/**
 * Copies the values of all fields which exist in the PersistentDataFields array from the RuntimeDataFields array into the PersistentDataFields array and
 * saves everything to the .ini.
 */
void UUIDynamicFieldProvider::SavePersistentProviderData()
{
	if ( GIsGame )
	{
		// if we're in the game, any modifications to values are performed on the RuntimeDataFields array.  However, only the entries which
		// exist in the PersistentDataFields array should be saved though, so for each element in the PersistentDataFields array, see
		// if there is a corresponding entry in the RuntimeDataFields array and copy it over to the PeristentDataFields array prior to saving
		for ( INT PersistentFieldIndex = 0; PersistentFieldIndex < PersistentDataFields.Num(); PersistentFieldIndex++ )
		{
			FUIProviderScriptFieldValue& DataField = PersistentDataFields(PersistentFieldIndex);
			INT RuntimeFieldIndex = FindFieldIndex(DataField.PropertyTag);
			if ( RuntimeFieldIndex != INDEX_NONE )
			{
				DataField = RuntimeDataFields(RuntimeFieldIndex);
			}
		}
	}

// Can't save config on consoles, so only do it on PC.
#if !CONSOLE
	//@todo ronp - implement ClearConfig() and call that if PersistentDataFields.Num() == 0
	SaveConfig();
#endif

}

/** ============================================== */
/** === source data for collection data fields === */
/** ============================================== */
/**
 * Gets the list of schema tags set for the data value source array stored for FieldName
 *
 * @param	FieldName			the name of the data field the source data should be associated with.
 * @param	out_CellTagArray	the list of unique tags stored in the source array data for the specified data field name.
 * @param	bPersistent			specify TRUE to ensure that the PersistentCollectionData is used, even if it otherwise
 *								wouldn't be.
 *
 * @return	TRUE if the array containing possible values for the FieldName data field was successfully located and copied
 *			into the out_CellTagArray variable.
 */
UBOOL UUIDynamicFieldProvider::GetCollectionValueSchema( FName FieldName, TArray<FName>& out_CellTagArray, UBOOL bPersistent/*=FALSE*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		// determine which data source should be used
		DynamicCollectionValueMap& CollectionDataSourceMap = (GIsGame && !bPersistent) ? RuntimeCollectionData : PersistentCollectionData;
		TMap<FName,TArray<FString> >* pCollectionDataValues = CollectionDataSourceMap.Find(FieldName);
		if ( pCollectionDataValues != NULL )
		{
			TLookupMap<FName> CellTagArray;
			bResult = pCollectionDataValues->GetKeys(CellTagArray) > 0;
			for ( INT TagIndex = 0; TagIndex < CellTagArray.Num(); TagIndex++ )
			{
				out_CellTagArray.AddItem(CellTagArray(TagIndex));
			}
		}
	}

	return bResult;
}

/**
 * Gets the data value source array for the specified data field.
 *
 * @param	FieldName			the name of the data field the source data should be associated with.
 * @param	out_DataValueArray	receives the array of data values available for FieldName.
 * @param	bPersistent			specify TRUE to ensure that the PersistentCollectionData is used, even if it otherwise
 *								wouldn't be.
 * @param	CellTag				optional name of a subfield within the list of values for FieldName. if not specified, FieldName is used.
 *
 * @return	TRUE if the array containing possible values for the FieldName data field was successfully located and copied
 *			into the out_DataValueArray variable.
 */
UBOOL UUIDynamicFieldProvider::GetCollectionValueArray( FName FieldName, TArray<FString>& out_DataValueArray, UBOOL bPersistent/*=FALSE*/, FName CellTag/*=NAME_None*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		// determine which data source should be used
		DynamicCollectionValueMap& CollectionDataSourceMap = (GIsGame && !bPersistent) ? RuntimeCollectionData : PersistentCollectionData;
		TMap<FName,TArray<FString> >* pCollectionDataValues = CollectionDataSourceMap.Find(FieldName);
		if ( pCollectionDataValues != NULL )
		{
			if ( CellTag == NAME_None )
			{
				CellTag = FieldName;
			}

			TArray<FString>* CellDataValues = pCollectionDataValues->Find(CellTag);
			if(CellDataValues)
			{
				Move(out_DataValueArray,*CellDataValues);
			}
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Sets the source data for a collection data field to the values specified.  It is not necessary to add the field first
 * (via AddField) in order to set the collection values.
 *
 * @param	FieldName			the name of the data field the source data should be associated with.
 * @param	CollectionValues	the actual values that will be associated with FieldName.
 * @param	bClearExisting		specify TRUE to clear the existing collection data before adding the new values
 * @param	InsertIndex			the position to insert the new values (only relevant if bClearExisting is FALSE)
 * @param	bPersistent			specify TRUE to ensure that the values will be added to PersistentCollectionData, even
 *								if they otherwise wouldn't be.
 * @param	CellTag				optional name of a subfield within the list of values for FieldName. if not specified, FieldName is used.
 *
 * @return	TRUE if the collection data was applied successfully; FALSE if there was also already data for this collection
 *			data field [and bOverwriteExisting was FALSE] or the data couldn't otherwise
 */
UBOOL UUIDynamicFieldProvider::SetCollectionValueArray( FName FieldName, const TArray<FString>& CollectionValues, UBOOL bClearExisting/*=TRUE*/,
													   INT InsertIndex/*=INDEX_NONE*/, UBOOL bPersistent/*=FALSE*/, FName CellTag/*=NAME_None*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		if ( CellTag == NAME_None )
		{
			CellTag = FieldName;
		}

		// determine which data source should be used
		DynamicCollectionValueMap& CollectionDataSourceMap = (GIsGame && !bPersistent) ? RuntimeCollectionData : PersistentCollectionData;
		TMap<FName,TArray<FString> >* pCollectionDataValues = CollectionDataSourceMap.Find(FieldName);

		// if it didn't already exist
		if ( pCollectionDataValues == NULL

		// or we're allowed to overwrite the existing values
		||	(bClearExisting && ClearCollectionValueArray(FieldName, bPersistent, CellTag)) )
		{
			// copy the new values into the storage map
			pCollectionDataValues = &CollectionDataSourceMap.Set(FieldName, TMap<FName,TArray<FString> >());
			pCollectionDataValues->Set(CellTag,CollectionValues);
			bResult = TRUE;
		}
	}

	return bResult;
}


/**
 * Removes all data values for a single collection data field.
 *
 * @param	FieldName		the name of the data field associated with the data source collection being manipulated.
 * @param	bPersistent		specify TRUE to ensure that the PersistentCollectionData is used, even if it otherwise
 *							wouldn't be.
 * @param	CellTag			optional name of a subfield within the list of values for FieldName. if not specified, FieldName is used.
 *
 * @return	TRUE if the data values were successfully cleared or didn't exist in the first place; FALSE if they couldn't be removed.
 */
UBOOL UUIDynamicFieldProvider::ClearCollectionValueArray( FName FieldName, UBOOL bPersistent/*=FALSE*/, FName CellTag/*=NAME_None*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		DynamicCollectionValueMap& CollectionDataValues = (GIsGame && !bPersistent) ? RuntimeCollectionData : PersistentCollectionData;
		if ( CollectionDataValues.HasKey(FieldName) )
		{
			if ( CellTag == NAME_None )
			{
				if ( CollectionDataValues.Remove(FieldName) > 0 )
				{
					bResult = TRUE;
				}
				else
				{
					//@todo ronp - hmmm, under what circumstances might this occur?
				}
			}
			else
			{
				TMap<FName,TArray<FString> >* pCollectionDataValues = CollectionDataValues.Find(FieldName);
				check(pCollectionDataValues);

				pCollectionDataValues->RemoveKey(CellTag);
				bResult = TRUE;
			}
		}
		else
		{
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Inserts a new string into the list of values for the specified collection data field.
 *
 * @param	FieldName		the name of the data field associated with the data source collection being manipulated.
 * @param	NewValue		the value to insert
 * @param	InsertIndex		the index [into the array of values for FieldName] to insert the new value, or INDEX_NONE to
 *							append the value to the end of the list.
 * @param	bPersistent		specify TRUE to ensure that the PersistentCollectionData is used, even if it otherwise
 *							wouldn't be.
 * @param	bAllowDuplicateValues
 *							controls whether multiple elements containing the same value should be allowed in the data source
 *							collection.  If FALSE is specified, and NewValue already exists in the collection source array, method
 *							return TRUE but it does not modify the array.  If TRUE is specified, NewValue will be added anyway,
 *							resulting in multiple copies of NewValue existing in the array.
 * @param	CellTag			optional name of a subfield within the list of values for FieldName. if not specified, FieldName is used.
 *
 * @return	TRUE if the new value was successfully inserted into the collection data source for the specified field.
 */
UBOOL UUIDynamicFieldProvider::InsertCollectionValue( FName FieldName, const FString& NewValue, INT InsertIndex/*=INDEX_NONE*/,
													 UBOOL bPersistent/*=FALSE*/, UBOOL bAllowDuplicateValues/*=FALSE*/, FName CellTag/*=NAME_None*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		if ( CellTag == NAME_None )
		{
			CellTag = FieldName;
		}

		// determine which data source should be used
		DynamicCollectionValueMap& CollectionDataSourceMap = (GIsGame && !bPersistent) ? RuntimeCollectionData : PersistentCollectionData;
		
		// Find or create a cell-value map for this field.
		TMap<FName,TArray<FString> >* pCollectionDataValues = CollectionDataSourceMap.Find(FieldName);
		if ( !pCollectionDataValues )
		{
			pCollectionDataValues = &CollectionDataSourceMap.Set(FieldName, TMap<FName,TArray<FString> >());
		}

		// Find or create a list of values for this cell.
		TArray<FString>* pCellValues = pCollectionDataValues->Find(CellTag);
		if(!pCellValues)
		{
			pCellValues = &pCollectionDataValues->Set(CellTag,TArray<FString>());
		}

		// Add the new value to the cell's value list.
		if(bAllowDuplicateValues || pCellValues->FindItemIndex(NewValue) == INDEX_NONE)
		{
			if(InsertIndex != INDEX_NONE)
			{
				pCellValues->InsertItem(NewValue,InsertIndex);
			}
			else
			{
				pCellValues->AddItem(NewValue);
			}
		}
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Removes a value from the collection data source specified by FieldName.
 *
 * @param	FieldName		the name of the data field associated with the data source collection being manipulated.
 * @param	ValueToRemove	the value that should be removed
 * @param	bPersistent		specify TRUE to ensure that the PersistentCollectionData is used, even if it otherwise
 *							wouldn't be.
 * @param	CellTag			optional name of a subfield within the list of values for FieldName. if not specified, FieldName is used.
 *
 * @return	TRUE if the value was successfully removed or didn't exist in the first place.
 */
UBOOL UUIDynamicFieldProvider::RemoveCollectionValue( FName FieldName, const FString& ValueToRemove, UBOOL bPersistent/*=FALSE*/, FName CellTag/*=NAME_None*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		// determine which data source should be used
		DynamicCollectionValueMap& CollectionDataSourceMap = (GIsGame && !bPersistent) ? RuntimeCollectionData : PersistentCollectionData;
		TMap<FName,TArray<FString> >* pCollectionDataValues = CollectionDataSourceMap.Find(FieldName);

		if ( pCollectionDataValues != NULL )
		{
			if ( CellTag == NAME_None )
			{
				CellTag = FieldName;
			}

			TArray<FString>* CellValues = pCollectionDataValues->Find(CellTag);
			if(CellValues && CellValues->RemoveItem(ValueToRemove))
			{
				bResult = TRUE;
			}
		}
		else
		{
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Removes the value from the collection data source specified by FieldName located at ValueIndex.
 *
 * @param	FieldName		the name of the data field associated with the data source collection being manipulated.
 * @param	ValueIndex		the index [into the array of values for FieldName] of the value that should be removed.
 * @param	bPersistent		specify TRUE to ensure that the PersistentCollectionData is used, even if it otherwise
 *							wouldn't be.
 * @param	CellTag			optional name of a subfield within the list of values for FieldName. if not specified, FieldName is used.
 *
 * @return	TRUE if the value was successfully removed; FALSE if ValueIndex wasn't valid or the value couldn't be removed.
 */
UBOOL UUIDynamicFieldProvider::RemoveCollectionValueByIndex( FName FieldName, INT ValueIndex, UBOOL bPersistent/*=FALSE*/, FName CellTag/*=NAME_None*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		// determine which data source should be used
		DynamicCollectionValueMap& CollectionDataSourceMap = (GIsGame && !bPersistent) ? RuntimeCollectionData : PersistentCollectionData;
		TMap<FName,TArray<FString> >* pCollectionDataValues = CollectionDataSourceMap.Find(FieldName);

		if ( pCollectionDataValues != NULL )
		{
			if ( CellTag == NAME_None )
			{
				CellTag = FieldName;
			}

			TArray<FString>* Values = pCollectionDataValues->Find(CellTag);
			if(Values->IsValidIndex(ValueIndex))
			{
				Values->Remove(ValueIndex);
				bResult = TRUE;
			}
		}
		else
		{
			// in this case, if the value array doesn't exist, then the result of this function is FALSE
			// (because technically ValueIndex wasn't a valid value)
		}
	}

	return bResult;
}

/**
 * Replaces the value in a collection data source with a different value.
 *
 * @param	FieldName		the name of the data field associated with the data source collection being manipulated.
 * @param	CurrentValue	the value that will be replaced.
 * @param	NewValue		the value that will replace CurrentValue
 * @param	bPersistent		specify TRUE to ensure that the PersistentCollectionData is used, even if it otherwise
 *							wouldn't be.
 * @param	CellTag			optional name of a subfield within the list of values for FieldName. if not specified, FieldName is used.
 *
 * @return	TRUE if the old value was successfully replaced with the new value.
 */
UBOOL UUIDynamicFieldProvider::ReplaceCollectionValue( FName FieldName, const FString& CurrentValue, const FString& NewValue, UBOOL bPersistent/*=FALSE*/, FName CellTag/*=NAME_None*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		// determine which data source should be used
		DynamicCollectionValueMap& CollectionDataSourceMap = (GIsGame && !bPersistent) ? RuntimeCollectionData : PersistentCollectionData;
		TMap<FName,TArray<FString> >* pCollectionDataValues = CollectionDataSourceMap.Find(FieldName);
		if ( pCollectionDataValues != NULL )
		{
			if ( CellTag == NAME_None )
			{
				CellTag = FieldName;
			}

			TArray<FString>* CurrentValues = pCollectionDataValues->Find(CellTag);
			if(CurrentValues)
			{
				const INT ValueIndex = CurrentValues->FindItemIndex(CurrentValue);
				if(ValueIndex != INDEX_NONE)
				{
					(*CurrentValues)(ValueIndex) = NewValue;
					bResult = TRUE;
				}
			}
		}
	}

	return bResult;
}

/**
 * Replaces the value located at ValueIndex in a collection data source with a different value
 *
 * @param	FieldName		the name of the data field associated with the data source collection being manipulated.
 * @param	ValueIndex		the index [into the array of values for FieldName] of the value that should be replaced.
 * @param	NewValue		the value that should replace the old value.
 * @param	bPersistent		specify TRUE to ensure that the PersistentCollectionData is used, even if it otherwise
 *							wouldn't be.
 * @param	CellTag			optional name of a subfield within the list of values for FieldName. if not specified, FieldName is used.
 *
 * @return	TRUE if the value was successfully replaced; FALSE if ValueIndex wasn't valid or the value couldn't be removed.
 */
UBOOL UUIDynamicFieldProvider::ReplaceCollectionValueByIndex( FName FieldName, INT ValueIndex, const FString& NewValue, UBOOL bPersistent/*=FALSE*/, FName CellTag/*=NAME_None*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		// determine which data source should be used
		DynamicCollectionValueMap& CollectionDataSourceMap = (GIsGame && !bPersistent) ? RuntimeCollectionData : PersistentCollectionData;
		TMap<FName,TArray<FString> >* pCollectionDataValues = CollectionDataSourceMap.Find(FieldName);
		if ( pCollectionDataValues != NULL )
		{
			if ( CellTag == NAME_None )
			{
				CellTag = FieldName;
			}

			TArray<FString>* Values = pCollectionDataValues->Find(CellTag);

			if ( Values && Values->IsValidIndex(ValueIndex) )
			{
				(*Values)(ValueIndex) = NewValue;
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/**
 * Retrieves the value of an element in a collection data source array.
 *
 * @param	FieldName		the name of the data field associated with the data source collection being manipulated.
 * @param	ValueIndex		the index [into the array of values for FieldName] of the value that should be retrieved.
 * @param	out_Value		receives the value of the collection data source element
 * @param	bPersistent		specify TRUE to ensure that the PersistentCollectionData is used, even if it otherwise
 *							wouldn't be.
 * @param	CellTag			optional name of a subfield within the list of values for FieldName. if not specified, FieldName is used.
 *
 * @return	TRUE if the value was successfully retrieved and copied to out_Value.
 */
UBOOL UUIDynamicFieldProvider::GetCollectionValue( FName FieldName, INT ValueIndex, FString& out_Value, UBOOL bPersistent/*=FALSE*/, FName CellTag/*=NAME_None*/ ) const
{
	UBOOL bResult = FALSE;

	if ( FieldName != NAME_None )
	{
		// determine which data source should be used
		const DynamicCollectionValueMap& CollectionDataSourceMap = (GIsGame && !bPersistent) ? RuntimeCollectionData : PersistentCollectionData;
		const TMap<FName,TArray<FString> >* pCollectionDataValues = CollectionDataSourceMap.Find(FieldName);
		if ( pCollectionDataValues != NULL )
		{
			if ( CellTag == NAME_None )
			{
				CellTag = FieldName;
			}

			const TArray<FString>* Values = pCollectionDataValues->Find(CellTag);

			if ( Values && Values->IsValidIndex(ValueIndex) )
			{
				out_Value = (*Values)(ValueIndex);
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/**
 * Finds the index [into the array of values for FieldName] for a specific value.
 *
 * @param	FieldName		the name of the data field associated with the data source collection being manipulated.
 * @param	ValueToFind		the value that should be found.
 * @param	bPersistent		specify TRUE to ensure that the PersistentCollectionData is used, even if it otherwise
 *							wouldn't be.
 * @param	CellTag			optional name of a subfield within the list of values for FieldName. if not specified, FieldName is used.
 *
 * @return	the index for the specified value, or INDEX_NONE if it couldn't be found.
 */
INT UUIDynamicFieldProvider::FindCollectionValueIndex( FName FieldName, const FString& ValueToFind, UBOOL bPersistent/*=FALSE*/, FName CellTag/*=NAME_None*/ ) const
{
	INT Result = INDEX_NONE;

	if ( FieldName != NAME_None )
	{
		// determine which data source should be used
		const DynamicCollectionValueMap& CollectionDataSourceMap = (GIsGame && !bPersistent) ? RuntimeCollectionData : PersistentCollectionData;
		const TMap<FName,TArray<FString> >* pCollectionDataValues = CollectionDataSourceMap.Find(FieldName);
		if ( pCollectionDataValues != NULL )
		{
			if ( CellTag == NAME_None )
			{
				CellTag = FieldName;
			}

			const TArray<FString>* Values = pCollectionDataValues->Find(CellTag);
			if(Values)
			{
				Result = Values->FindItemIndex(ValueToFind);
			}
		}
	}

	return Result;
}


/* === Unrealscript stubs === */
void UUIDynamicFieldProvider::execAddField( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(FieldName);
	P_GET_BYTE_OPTX(FieldType,DATATYPE_Property);
	P_GET_UBOOL_OPTX(bPersistent,FALSE);
	P_GET_INT_OPTX_REF(out_InsertPosition,INDEX_NONE);
	P_FINISH;
	*(UBOOL*)Result=AddField(FieldName,FieldType,bPersistent,pout_InsertPosition);
}
void UUIDynamicFieldProvider::execGetField( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(FieldName);
	P_GET_STRUCT_INIT_REF(FUIProviderScriptFieldValue,out_Field);
	P_FINISH;
	UBOOL& bResult = *(UBOOL*)Result;

	FUIProviderFieldValue FieldValue(EC_EventParm);

	bResult = GetFieldValue(FieldName.ToString(), FieldValue);
	if ( bResult == TRUE )
	{
		out_Field.PropertyTag = FieldName;
		out_Field.PropertyType = FieldValue.PropertyType;

		out_Field.StringValue = FieldValue.StringValue;
		out_Field.ImageValue = FieldValue.ImageValue;
		out_Field.ArrayValue = FieldValue.ArrayValue;
		out_Field.RangeValue = FieldValue.RangeValue;
		out_Field.NetIdValue = FieldValue.NetIdValue;
	}
}
void UUIDynamicFieldProvider::execSetField( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(FieldName);
	P_GET_STRUCT_INIT_REF(FUIProviderScriptFieldValue,FieldValue);
	P_GET_UBOOL_OPTX(bChangeExistingOnly,TRUE);
	P_FINISH;
	UBOOL& bResult = *(UBOOL*)Result;

	FUIProviderFieldValue FieldValueCopy(FieldValue);
	if ( bChangeExistingOnly == FALSE )
	{
		// add a field of this name
		AddField(FieldName);
	}
	bResult = SetFieldValue(FieldName.ToString(), FieldValueCopy);
}

/* === UIDataProvider interface === */
/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see ParseDataStoreReference for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDynamicFieldProvider::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName.Len() > 0 )
	{
		INT ExistingFieldIndex = FindFieldIndex(*FieldName);
		if ( ExistingFieldIndex != INDEX_NONE )
		{
			out_FieldValue = GIsGame ? RuntimeDataFields(ExistingFieldIndex) : PersistentDataFields(ExistingFieldIndex);
			bResult = TRUE;
		}
	}

	return bResult || Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDynamicFieldProvider::SetFieldValue( const FString& FieldName, const FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	if ( FieldName.Len() > 0 )
	{
		INT ExistingFieldIndex = FindFieldIndex(*FieldName);
		if ( ExistingFieldIndex != INDEX_NONE )
		{
			FUIProviderScriptFieldValue& DataField = GIsGame ? RuntimeDataFields(ExistingFieldIndex) : PersistentDataFields(ExistingFieldIndex);
			DataField = FieldValue;
			DataField.PropertyTag = *FieldName;

			eventNotifyPropertyChanged(DataField.PropertyTag);
			bResult = TRUE;
		}
	}

	return bResult || Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIDynamicFieldProvider::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	TArrayNoInit<FUIProviderScriptFieldValue>& DataFields = GIsGame ? RuntimeDataFields : PersistentDataFields;
	for ( INT FieldIndex = 0; FieldIndex < DataFields.Num(); FieldIndex++ )
	{
		FUIProviderScriptFieldValue& DataField = DataFields(FieldIndex);

		new(out_Fields) FUIDataProviderField(DataField.PropertyTag, (EUIDataProviderFieldType)DataField.PropertyType);
	}
}

/* === UObject interface interface === */
/**
 * Serializes the value of the PersistentCollectionData and RuntimeCollectionData members, since they are not supported
 * by script serialization.
 */
void UUIDynamicFieldProvider::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);

	if ( Ar.IsLoading() && Ar.Ver() < VER_ADDED_MULTICOLUMN_SUPPORT )
	{
		TMap< FName, TLookupMap<FString> > OldVersion;
		Ar << OldVersion;

		for ( TMap< FName, TLookupMap<FString> >::TIterator It(OldVersion); It; ++It )
		{
			const FName& FieldName = It.Key();
			TLookupMap<FString>& Values = It.Value();

			TMap<FName,TArray<FString> >& NewVersion = PersistentCollectionData.Set(FieldName, TMap<FName,TArray<FString> >());
			NewVersion.Set(FieldName,Values.GetUniqueElements());
		}
	}
	else
	{
		Ar << PersistentCollectionData;
	}

	// serialize the RuntimeCollectionData only if the archive isn't persistent or we're performing binary duplication
	if ( !Ar.IsPersistent() || (Ar.GetPortFlags()&PPF_Duplicate) != 0 )
	{
		Ar << RuntimeCollectionData;
	}
}

/* ==========================================================================================================
	USessionSettingsDataProvider
========================================================================================================== */
/**
 * Associates this data provider with the specified class.
 *
 * @param	DataSourceClass	a pointer to the specific child of Dataclass that this data provider should present data for.  
 *
 * @return	TRUE if the class specified was successfully associated with this data provider.  FALSE if the object specified
 *			wasn't of the correct type or was otherwise invalid.
 */
UBOOL USessionSettingsProvider::BindProviderClient( UClass* DataSourceClass )
{
	UBOOL bResult = FALSE;

	check(ProviderClientMetaClass != NULL);
	if ( DataSourceClass->IsChildOf(ProviderClientMetaClass) && DataSourceClass->ImplementsInterface(ProviderClientClass) )
	{
		ProviderClient = DataSourceClass;		
		for ( UProperty* Property = DataSourceClass->PropertyLink; Property; Property = Property->PropertyLinkNext )
		{
			if ( (Property->PropertyFlags&CPF_DataBinding) != 0 )
			{
				// do something
			}
		}

		eventProviderClientBound(DataSourceClass);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Clears the reference to the class associated with this data provider.
 *
 * @return	TRUE if the class reference was successfully cleared.
 */
UBOOL USessionSettingsProvider::UnbindProviderClient()
{
	UClass* OldProviderClient = ProviderClient;
	if ( OldProviderClient != NULL )
	{
		ProviderClient = NULL;
		eventProviderClientUnbound(OldProviderClient);
	}

	return TRUE;
}

/**
 * Determines whether the specified class should be represented by this settings data provider.
 *
 * @param	PotentialDataSourceClass	a pointer to a UClass that is being considered for binding by this provider.
 *
 * @return	TRUE to allow the databinding properties of PotentialDataSourceClass to be displayed in the UI editor's data store browser
 *			under this data provider.
 */
UBOOL USessionSettingsProvider::IsValidDataSourceClass( UClass* PotentialDataSourceClass )
{
	checkSlow(ProviderClientMetaClass);

	UBOOL bResult = PotentialDataSourceClass != NULL && PotentialDataSourceClass->IsChildOf(ProviderClientMetaClass) && PotentialDataSourceClass->ImplementsInterface(ProviderClientClass);
	if ( bResult == TRUE )
	{
		// allow script to override this evaluation
		bResult = eventIsValidDataSourceClass(PotentialDataSourceClass);
	}

	return bResult;
}

/**
 * Builds an array of classes that are supported by this data provider.  Used in the editor to generate the list of 
 * supported data fields.  Since dynamic data providers are only created during the game, the editor needs a way to
 * retrieve the list of data field tags that can be bound without requiring instances of this data provider's DataClass to exist.
 */
void USessionSettingsProvider::GetSupportedClasses( TArray<UClass*>& out_Classes )
{
	if ( GIsEditor && !GIsGame )
	{
		check(ProviderClientMetaClass);

		out_Classes.Empty();
		for ( TObjectIterator<UClass> It; It; ++It )
		{
			if ( IsValidDataSourceClass(*It) )
			{
				out_Classes.AddItem(*It);
			}
		}
	}
	else
	{
		warnf(NAME_Warning, TEXT("USessionSettingsProvider::GetSupportedClasses should only be called in the editor!"));
	}
}

/* === UIDataProvider interface === */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void USessionSettingsProvider::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	check(ProviderClientMetaClass);

	TArray<UProperty*> BindableProperties;
	if ( GIsGame )	// in game
	{
		if ( ProviderClient != NULL )
		{
            // retrieve the list of properties that are available in the class that is currently bound to this data provider
			GetProviderDataBindings(ProviderClient, BindableProperties);
		}
		else
		{
			GetProviderDataBindings(ProviderClientMetaClass, BindableProperties);
		}
	}
	else if ( GIsEditor )		// in editor
	{
		// if we are in the editor, we won't have any instances of the data class associated with this data provider,
		// so expose all properties which could possibly be accessed through this data provider

		// get the list of classes that can be hooked up to this data provider
		TArray<UClass*> SupportedClasses;
		GetSupportedClasses(SupportedClasses);

		// retrieve the list of properties that might be accessed through this data provider
		for ( INT ClassIndex = 0; ClassIndex < SupportedClasses.Num(); ClassIndex++ )
		{
			UClass* SupportedClass = SupportedClasses(ClassIndex);

			GetProviderDataBindings(SupportedClass, BindableProperties);
		}
	}

	Sort<USE_COMPARE_POINTER(UProperty,UnUIDataStores_DynamicPropertyBinding)>( &BindableProperties(0), BindableProperties.Num()  );

	// now generate the list of data fields using the properties we collected

	//@todo - present these properties under providers for each child of DataClass
	for ( INT PropertyIndex = 0; PropertyIndex < BindableProperties.Num(); PropertyIndex++ )
	{
		UProperty* DataProperty = BindableProperties(PropertyIndex);

		EUIDataProviderFieldType FieldType = DATATYPE_Property;
		// for each property contained by this ResourceDataProvider, add a provider field to expose them to the UI
		if ( Cast<UArrayProperty>(DataProperty) != NULL )
		{
			FieldType = DATATYPE_Collection;
		}
		else
		{
			UStructProperty* StructProp = Cast<UStructProperty>(DataProperty);
			if ( StructProp != NULL )
			{
				if ( IsRangeValueStruct(StructProp->Struct) )
				{
					FieldType = DATATYPE_RangeProperty;
				}
				else if ( IsUniqueNetIdStruct(StructProp->Struct) )
				{
					FieldType = DATATYPE_NetIdProperty;
				}
			}
		}

		// for each property contained by this ResourceDataProvider, add a provider field to expose them to the UI
		new(out_Fields) FUIDataProviderField( DataProperty->GetFName(), FieldType );
	}
}

/* ==========================================================================================================
	UPlayerOwnerDataProvider
========================================================================================================== */
/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UPlayerOwnerDataProvider::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	if ( PlayerData != NULL )
	{
		PlayerData->GetSupportedDataFields(out_Fields);
	}
	else
	{
		Super::GetSupportedDataFields(out_Fields);
	}
}

/* ==========================================================================================================
	UUIDataStore_StringAliasMap
========================================================================================================== */

/**
 * Return the string of the of the field being queried
 */
FString UUIDataStore_StringAliasMap::GetStringFromIndex( INT MapArrayIndex )
{
	if ( MapArrayIndex < 0 || MapArrayIndex >= MenuInputMapArray.Num() )
	{
		return TEXT("");
	}

	return MenuInputMapArray(MapArrayIndex).MappedText;
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	out_FieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UUIDataStore_StringAliasMap::GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	out_FieldValue.PropertyTag = *FieldName;
	
	INT Idx = GetStringWithFieldName( FieldName, out_FieldValue.StringValue );

	// Always return true, in the case that we couldn't find a field, return a empty string.
	if ( Idx == INDEX_NONE )
	{
		out_FieldValue.StringValue = TEXT("");

		// give script a chance to fill the value
		Super::GetFieldValue(FieldName, out_FieldValue, ArrayIndex);
	}

	return TRUE;
}

/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
* 						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UUIDataStore_StringAliasMap::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	Super::GetSupportedDataFields(out_Fields);

	// add a field for each mapping in the array
	for ( INT MapIdx = 0; MapIdx < MenuInputMapArray.Num(); MapIdx++ )
	{
		new(out_Fields) FUIDataProviderField( MenuInputMapArray(MapIdx).FieldName, DATATYPE_Property );
	}
}

/**
 * Returns a pointer to the ULocalPlayer that this PlayerSettingsProvdier provider settings data for
 */
ULocalPlayer* UUIDataStore_StringAliasMap::GetPlayerOwner() const
{
	ULocalPlayer* Result = NULL;

	if ( GEngine != NULL && GEngine->GamePlayers.IsValidIndex(PlayerIndex) )
	{
		Result = GEngine->GamePlayers(PlayerIndex);
	}

	return Result;
}

/**
 * Called when this data store is added to the data store manager's list of active data stores.
 *
 * @param	PlayerOwner		the player that will be associated with this DataStore.  Only relevant if this data store is
 *							associated with a particular player; NULL if this is a global data store.
 */
void UUIDataStore_StringAliasMap::OnRegister( ULocalPlayer* PlayerOwner )
{
	if ( GEngine != NULL && PlayerOwner != NULL )
	{
		// set the PlayerIndex for this player
		PlayerIndex = GEngine->GamePlayers.FindItemIndex(PlayerOwner);
	}

	// add a field for each mapping in the array
	for ( INT MapIdx = 0; MapIdx < MenuInputMapArray.Num(); MapIdx++ )
	{
		FUIMenuInputMap &InputMap = MenuInputMapArray(MapIdx);
		TMap<FName, INT> *SetMap = MenuInputSets.Find(InputMap.Set);

		if(SetMap==NULL)
		{
			SetMap = &MenuInputSets.Set(InputMap.Set, TMap<FName, INT>());
		}

		if(SetMap)
		{
			SetMap->Set(InputMap.FieldName, MapIdx);
		}
		
	}

	Super::OnRegister(PlayerOwner);
}

/**
 * Attempts to find a mapping index given a field name.
 *
 * @param FieldName		Fieldname to search for.
 *
 * @return Returns the index of the mapping in the mapping array, otherwise INDEX_NONE if the mapping wasn't found.
 */
INT UUIDataStore_StringAliasMap::FindMappingWithFieldName( const FString& FieldName, const FString &SetName )
{
	INT Result = INDEX_NONE;

	// add a field for each mapping in the array
	TMap<FName, INT> *SetMap = MenuInputSets.Find(*SetName);

	if(SetMap)
	{
		INT *FieldIdx = SetMap->Find(*FieldName);

		if(FieldIdx)
		{
			Result = *FieldIdx;
		}
	}

	return Result;
}


/**
 * Set MappedString to be the localized string using the FieldName as a key
 * Returns the index into the mapped string array of where it was found.
 */
INT UUIDataStore_StringAliasMap::GetStringWithFieldName( const FString& FieldName, FString& MappedString )
{
	INT FieldIdx = FindMappingWithFieldName(FieldName);

	if(FieldIdx != INDEX_NONE)
	{
		MappedString = MenuInputMapArray(FieldIdx).MappedText;
	}

	return FieldIdx;
}

/* ==========================================================================================================
	UPlayerSettingsProvider
========================================================================================================== */
/* === UPlayerSettingsProvider interface === */

/* ==========================================================================================================
	UUIDataStore_OnlineGameSettings
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataStore_OnlineGameSettings);

/**
 * Loads and creates an instance of the registered settings class(es)
 */
void UUIDataStore_OnlineGameSettings::InitializeDataStore(void)
{
	UClass* SettingsClass = SettingsProviderClass;
	if ( SettingsClass == NULL || !SettingsClass->IsChildOf(UUIDataProvider_Settings::StaticClass()) )
	{
		debugf(NAME_Warning, TEXT("%s::InitializeDataStore: Invalid SettingsProviderClass specified.  Falling back to UIDataProvider_Settings."), *GetClass()->GetName());
		SettingsClass = UUIDataProvider_Settings::StaticClass();
	}

	// Create the objects for each registered setting and provider
	for (INT Index = 0; Index < GameSettingsCfgList.Num(); Index++)
	{
		FGameSettingsCfg& Cfg = GameSettingsCfgList(Index);
		// Create an instance of a the registered OnlineGameSettings class
		// and create a provider to handle the data for it
		Cfg.GameSettings = ConstructObject<UOnlineGameSettings>(Cfg.GameSettingsClass);
		if (Cfg.GameSettings != NULL)
		{
			// Create an instance of the data provider that will handle it
			Cfg.Provider = ConstructObject<UUIDataProvider_Settings>(SettingsClass);
			if (Cfg.Provider != NULL)
			{
				// Tell the provider the object it is responsible for handling
				Cfg.Provider->BindSettings(Cfg.GameSettings);
			}
			else
			{
				debugf(NAME_Error,TEXT("Failed to create UUIDataProvider_Settings instance for %s"),
					*Cfg.GameSettingsClass->GetName());
			}
		}
		else
		{
			debugf(NAME_Error,TEXT("Failed to create instance of class %s"),
				*Cfg.GameSettingsClass->GetName());
		}
	}
}

/**
 * Builds a list of available fields from the array of properties in the
 * game settings object
 *
 * @param OutFields	out value that receives the list of exposed properties
 */
void UUIDataStore_OnlineGameSettings::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	// Data stores always empty and providers always append
	OutFields.Empty();


	//@todo ronp - not ready for this yet...to fully support tokenized array indices, we'll need to update all methods in this class
	// to check for the array index as part of the field tag (see UGearUIDataStore_GameResource for an example).
// 	TArray<UUIDataProvider*> GameSettingsProviders;
// 	for ( INT CfgIndex = 0; CfgIndex < GameSettingsCfgList.Num(); CfgIndex++ )
// 	{
// 		FGameSettingsCfg& Cfg = GameSettingsCfgList(CfgIndex);
// 		if ( Cfg.Provider != NULL )
// 		{
// 			GameSettingsProviders.AddItem(Cfg.Provider);
// 		}
// 	}
// 	new(OutFields) FUIDataProviderField(TEXT("GameSettings"), GameSettingsProviders);
	new(OutFields) FUIDataProviderField(TEXT("CurrentGameSettingsTag"),	DATATYPE_Property);
	new(OutFields) FUIDataProviderField(TEXT("SelectedIndex"),			DATATYPE_Property);


	FGameSettingsCfg& Cfg = GameSettingsCfgList(SelectedIndex);
	if (Cfg.Provider)
	{
		// Just forward the call to the provider
		Cfg.Provider->GetSupportedDataFields(OutFields);
	}
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_OnlineGameSettings::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	UBOOL bResult = FALSE;
	if ( FieldName == TEXT("SelectedIndex") )
	{
		OutFieldValue.PropertyTag = *FieldName;
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = appItoa(SelectedIndex);
		bResult = TRUE;
	}
	else if ( FieldName == TEXT("CurrentGameSettingsTag") )
	{
		OutFieldValue.PropertyTag = *FieldName;
		OutFieldValue.PropertyType = DATATYPE_Property;
		if ( GameSettingsCfgList.IsValidIndex(SelectedIndex) )
		{
			OutFieldValue.StringValue = GameSettingsCfgList(SelectedIndex).SettingsName.ToString();
		}
		bResult = TRUE;
	}
	else if ( GameSettingsCfgList.IsValidIndex(SelectedIndex) )
	{
		FGameSettingsCfg& Cfg = GameSettingsCfgList(SelectedIndex);
		if (Cfg.Provider)
		{
			bResult = Cfg.Provider->GetFieldValue(FieldName,OutFieldValue,ArrayIndex);
		}
	}

	return bResult || Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_OnlineGameSettings::SetFieldValue(const FString& FieldName,const FUIProviderScriptFieldValue& FieldValue,INT ArrayIndex)
{
	UBOOL bResult = FALSE;

	if ( FieldName == TEXT("SelectedIndex") )
	{
		const INT NewIndex = appAtoi(*FieldValue.StringValue);
		eventSetCurrentByIndex(NewIndex);
		bResult = TRUE;
	}
	else if ( FieldName == TEXT("CurrentGameSettingsTag") )
	{
		eventSetCurrentByName(*FieldValue.StringValue);
		bResult = TRUE;
	}
	else if ( GameSettingsCfgList.IsValidIndex(SelectedIndex) )
	{
		FGameSettingsCfg& Cfg = GameSettingsCfgList(SelectedIndex);
		if (Cfg.Provider)
		{
// 			debugf(TEXT("GameSettings::SetFieldValue: %s = %s"), *FieldName, *FieldValue.StringValue);
			bResult = Cfg.Provider->SetFieldValue(FieldName,FieldValue,ArrayIndex);
		}
	}

	return bResult || Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);
}

/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<class IUIListElementProvider> UUIDataStore_OnlineGameSettings::ResolveListElementProvider(const FString& PropertyName)
{
	FGameSettingsCfg& Cfg = GameSettingsCfgList(SelectedIndex);
	if (Cfg.Provider)
	{
		return Cfg.Provider->ResolveListElementProvider(PropertyName);
	}
	return TScriptInterface<class IUIListElementProvider>();
}

/**
 * Handler for this data store internal settings data providers OnDataProviderPropertyChange delegate.  When the setting associated with that
 * provider is updated, issues a refresh notification which causes e.g. widgets to refresh their values.
 *
 * @param	SourceProvider	the data provider that generated the notification
 * @param	SettingsName	the name of the setting that changed
 */
void UUIDataStore_OnlineGameSettings::OnSettingProviderChanged( UUIDataProvider* SourceProvider, FName SettingsName/*=NAME_None*/ )
{
	INT ValueContextId=INDEX_NONE;

	UUIDataProvider_SettingsArray* SettingsArrayProvider = Cast<UUIDataProvider_SettingsArray>(SourceProvider);
	if ( SettingsArrayProvider != NULL && SettingsArrayProvider->Settings != NULL )
	{
		if ( !SettingsArrayProvider->Settings->GetStringSettingValue(SettingsArrayProvider->SettingsId, ValueContextId) )
		{
			ValueContextId = INDEX_NONE;
		}
	}

	eventRefreshSubscribers(SettingsName, TRUE, SourceProvider, ValueContextId);

	if ( ProviderChangedNotifies.Num() > 0 )
	{
		UIDataProvider_eventOnDataProviderPropertyChange_Parms Parms(EC_EventParm);
		Parms.SourceProvider = SourceProvider;
		Parms.PropTag = SettingsName;

		for ( INT Index = 0; Index < ProviderChangedNotifies.Num(); Index++ )
		{
			FScriptDelegate& Delegate = ProviderChangedNotifies(Index);
			if ( Delegate.IsCallable(this) )
			{
				ProcessDelegate(NAME_None, &Delegate, &Parms);
			}
		}
	}
}

/**
 * Parses the string specified, separating the array index portion from the data field tag.
 *
 * @param	DataTag		the data tag that possibly contains an array index
 *
 * @return	the array index that was parsed from DataTag, or INDEX_NONE if there was no array index in the string specified.
 */

//@todo ronp - not ready for this yet...to fully support tokenized array indices, we'll need to update all methods in this class
// to check for the array index as part of the field tag (see UGearUIDataStore_GameResource for an example).
// INT UUIDataStore_OnlineGameSettings::ParseArrayDelimiter( FString& DataTag ) const
// {
// 	INT Result = INDEX_NONE;
// 
// 	INT DelimiterLocation = DataTag.InStr(ARRAY_DELIMITER);
// 	if ( DelimiterLocation != INDEX_NONE )
// 	{
// 		FString ArrayPortion = DataTag.Mid(DelimiterLocation + 1);
// 
// 		// remove the array portion, leaving only the data tag
// 		DataTag = DataTag.Left(DelimiterLocation);
// 
// 		if ( ArrayPortion.IsNumeric() )
// 		{
// 			// convert the array portion to an INT
// 			Result = appAtoi( *ArrayPortion );
// 		}
// 		else
// 		{
// 			// the array index is the name of one of our internal data providers - find out which one 
// 			// and return its index in the appropriate array.
// 			FName ProviderName(*ArrayPortion);
// 			for ( INT GameIndex = 0; GameIndex < GameSettingsCfgList.Num(); GameIndex++ )
// 			{
// 				FGameSettingsCfg& Cfg = GameSettingsCfgList(GameIndex);
// 				if ( ProviderName == Cfg.SettingsName )
// 				{
// 					Result = GameIndex;
// 					break;
// 				}
// 			}
// 		}
// 	}
// 
// 	return Result;
// }

/* ==========================================================================================================
	UUIDataProvider_Settings
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_Settings);

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataProvider_Settings::GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	check(Settings);
	// Let the base class handle dynamically bound properties first
	UBOOL bResult = Super::GetFieldValue(FieldName,out_FieldValue,ArrayIndex);
	// If it wasn't a UProperty, then it must be either from the Properties,
	// Contexts, or GameMode fields
	if (bResult == FALSE)
	{
		// Set the default type to property
		out_FieldValue.PropertyType = DATATYPE_Property;
		INT ArrayVal=INDEX_NONE;
		FString StringVal;
		// Try a context first
		FName ValueName = Settings->GetStringSettingValueNameByName(FName(*FieldName));
		if (ValueName != NAME_None)
		{
			StringVal = ValueName.ToString();
			Settings->GetStringSettingValueByName(FName(*FieldName), ArrayVal);
		}
		if (StringVal.Len() == 0)
		{
			INT PropertyId;
			// Find the ID so we can look up meta data info
			if (Settings->GetPropertyId(FName(*FieldName),PropertyId))
			{
				BYTE MappingType;
				// Determine the type of property mapping for correct UI exposure
				Settings->GetPropertyMappingType(PropertyId,MappingType);
				if (MappingType == PVMT_Ranged)
				{
					out_FieldValue.PropertyType = DATATYPE_RangeProperty;
					FLOAT MinV, MaxV, IncrementV;
					BYTE bFormatAsInt;
					// Read and set the range for this value
					Settings->GetPropertyRange(PropertyId,MinV,MaxV,IncrementV,bFormatAsInt);
					out_FieldValue.RangeValue.MaxValue = MaxV;
					out_FieldValue.RangeValue.MinValue = MinV;
					out_FieldValue.RangeValue.bIntRange = bFormatAsInt;
					out_FieldValue.RangeValue.SetNudgeValue(IncrementV);
					FLOAT Value;
					// Read the value it currently has
					Settings->GetRangedPropertyValue(PropertyId,Value);
					out_FieldValue.RangeValue.SetCurrentValue(Value);
					bResult = TRUE;
				}
				else
				{
					// Check the properties for this DataTag if still unknown
					StringVal = Settings->GetPropertyAsStringByName(FName(*FieldName));
					if ( StringVal.Len() > 0 )
					{
						if ( !bIsAListRow )
						{
							FName FieldFName(*FieldName);
							FName NameVal = FName(*StringVal);
							// Add each settings array provider
							for (INT Index = 0; Index < SettingsArrayProviders.Num(); Index++)
							{
								FSettingsArrayProvider& Mapping = SettingsArrayProviders(Index);
								if ( Mapping.SettingsName == FieldFName )
								{
									if ( Mapping.Provider != NULL )
									{
										for ( INT ValueIndex = 0; ValueIndex < Mapping.Provider->Values.Num(); ValueIndex++ )
										{
											if ( Mapping.Provider->Values(ValueIndex).Name == NameVal )
											{
												ArrayVal = Mapping.Provider->Values(ValueIndex).Id;
												break;
											}
										}
										
									}
									break;
								}
							}
						}
					}
				}
			}
		}
		// Only set the string as the value if it's a property type
		if ((StringVal.Len() > 0 || ArrayVal != INDEX_NONE)
		&&	out_FieldValue.PropertyType == DATATYPE_Property)
		{
			out_FieldValue.PropertyTag = *FieldName;
			out_FieldValue.StringValue = StringVal;
			if ( ArrayVal != INDEX_NONE )
			{
				out_FieldValue.ArrayValue.AddItem(ArrayVal);
			}
			bResult = TRUE;
		}
	}
	return bResult;
}

/**
 * Generates filler data for a given tag. Uses the OnlineDataType to determine
 * what the hardcoded filler data will look like
 *
 * @param DataTag the tag to generate filler data for
 *
 * @return a string containing example data
 */
FString UUIDataProvider_Settings::GenerateFillerData(const FString& DataTag)
{
	check(Settings);
	// Ask the dynamically bound properties first
	FString Filler = Super::GenerateFillerData(DataTag);
	// If it didn't know about the property, search the contexts, properties, etc.
	if (Filler.Len() == 0)
	{
		// Try contexts first
		FName ValueName = Settings->GetStringSettingValueNameByName(FName(*DataTag));
		if (ValueName != NAME_None)
		{
			Filler = ValueName.ToString();
		}
		// Check the properties for this DataTag if still unknown
		if (Filler.Len() == 0)
		{
			Filler = Settings->GetPropertyAsStringByName(FName(*DataTag));
		}
	}
	return Filler;
}

/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<class IUIListElementProvider> UUIDataProvider_Settings::ResolveListElementProvider(const FString& PropertyName)
{
	// If we are in a list, then we aren't providing array support
	if (bIsAListRow == FALSE)
	{
		FString CompareName(PropertyName);
		FString ProviderName;
		// If there is an intervening provider name, snip it off
		if (ParseNextDataTag(CompareName,ProviderName) == FALSE)
		{
			CompareName = ProviderName;
		}
		FName PropName(*CompareName);
		// Search for the property in the cached list
		for (INT Index = 0; Index < SettingsArrayProviders.Num(); Index++)
		{
			FSettingsArrayProvider& Mapping = SettingsArrayProviders(Index);
			// If the names match, return the corresponding provider
			if (PropName == Mapping.SettingsName)
			{
				return Mapping.Provider;
			}
		}
	}
	return TScriptInterface<class IUIListElementProvider>();
}

/**
 * Builds a list of available fields from the array of properties in the
 * game settings object
 *
 * @param OutFields	out value that receives the list of exposed properties
 */
void UUIDataProvider_Settings::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	check(Settings);
	// Have the dynamically bound values added first
	Super::GetSupportedDataFields(OutFields);
	if (bIsAListRow)
	{
		// For each localized string setting in the settings object, find its friendly name and add it
		for (INT Index = 0; Index < Settings->LocalizedSettingsMappings.Num(); Index++)
		{
			// Get the human readable name from the mapping object
			const FLocalizedStringSettingMetaData& Mapping = Settings->LocalizedSettingsMappings(Index);
			// Now add it to the list of properties
			new(OutFields) FUIDataProviderField(Mapping.Name);
		}
	}
	else
	{
		// Add each settings array provider
		for (INT Index = 0; Index < SettingsArrayProviders.Num(); Index++)
		{
			FSettingsArrayProvider& Mapping = SettingsArrayProviders(Index);
			// Find the friendly name for this
			FName StringSettingName = Mapping.SettingsName;
			if (StringSettingName != NAME_None)
			{
				// Add it as a collection with a custom provider
				new(OutFields) FUIDataProviderField(StringSettingName,DATATYPE_Collection);
			}
		}
	}
	// For each property in the settings object, find its friendly name and add it
	for (INT Index = 0; Index < Settings->PropertyMappings.Num(); Index++)
	{
		// Get the human readable name from the settings object
		const FSettingsPropertyPropertyMetaData& Mapping = Settings->PropertyMappings(Index);
		if (bIsAListRow || Mapping.MappingType != PVMT_PredefinedValues)
		{
			// Expose as a range if applicable
			EUIDataProviderFieldType FieldType = Mapping.MappingType == PVMT_Ranged ? DATATYPE_RangeProperty : DATATYPE_Property;
			// Now add it to the list of properties
			new(OutFields) FUIDataProviderField(Mapping.Name,FieldType);
		}
	}
}

/**
 * Gets the list of data fields (and their localized friendly name) for the fields exposed this provider.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	receives the name/friendly name pairs for all data fields in this provider.
 */
void UUIDataProvider_Settings::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	check(Settings);
	// Have the dynamically bound values added first
	Super::GetElementCellTags(FieldName, out_CellTags);

	if ( FieldName == UCONST_UnknownCellDataFieldName )
	{
		if (bIsAListRow)
		{
			// For each localized string setting in the settings object, find its friendly name and add it
			for (INT Index = 0; Index < Settings->LocalizedSettingsMappings.Num(); Index++)
			{
				// Get the human readable name from the mapping object
				const FLocalizedStringSettingMetaData& Mapping = Settings->LocalizedSettingsMappings(Index);
				// Now add it to the list of properties
				out_CellTags.Set(Mapping.Name, *Mapping.ColumnHeaderText);
			}
		}
		else
		{
			// Add each settings array provider
			for (INT Index = 0; Index < SettingsArrayProviders.Num(); Index++)
			{
				FSettingsArrayProvider& Mapping = SettingsArrayProviders(Index);
				// Find the friendly name for this
				FName StringSettingName = Mapping.SettingsName;
				if (StringSettingName != NAME_None && Mapping.Provider != NULL)
				{
					// Add it as a collection with a custom provider
					out_CellTags.Set(StringSettingName, *Mapping.Provider->ColumnHeaderText);
				}
			}
		}
		// For each property in the settings object, find its friendly name and add it
		for (INT Index = 0; Index < Settings->PropertyMappings.Num(); Index++)
		{
			// Get the human readable name from the settings object
			const FSettingsPropertyPropertyMetaData& Mapping = Settings->PropertyMappings(Index);
			if (bIsAListRow || Mapping.MappingType != PVMT_PredefinedValues)
			{
				// Now add it to the list of properties
				out_CellTags.Set(Mapping.Name, *Mapping.ColumnHeaderText);
			}
		}
	}
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataProvider_Settings::SetFieldValue(const FString& FieldName,
	const FUIProviderScriptFieldValue& FieldValue,INT ArrayIndex)
{
	// Ask the dynamically bound properties first
	UBOOL bRet = Super::SetFieldValue(FieldName,FieldValue,ArrayIndex);
	if (bRet == FALSE)
	{
		// Try string settings
		if ( FieldValue.StringValue.Len() > 0 || FieldValue.ArrayValue.Num() == 0 )
		{
			bRet = Settings->SetStringSettingValueFromStringByName(FName(*FieldName),
				FieldValue.StringValue);
		}
		else if ( FieldValue.ArrayValue.Num() > 0 )
		{
			INT StringSettingId;
			if ( Settings->GetStringSettingId(FName(*FieldName), StringSettingId) )
			{
				Settings->SetStringSettingValue(StringSettingId,
					FieldValue.ArrayValue(0), FALSE);
				bRet = TRUE;
			}
		}
		// Check the properties if still unknown
		if (bRet == FALSE)
		{
			INT PropertyId;
			// Find the ID so we can look up meta data info
			if (Settings->GetPropertyId(FName(*FieldName),PropertyId))
			{
				BYTE Type;
				// Now find the mapping type by id
				if (Settings->GetPropertyMappingType(PropertyId,Type))
				{
					if (Type == PVMT_Ranged)
					{
						// Set the value and clamp to its range
						bRet = Settings->SetRangedPropertyValue(PropertyId,
							FieldValue.RangeValue.GetCurrentValue());
					}
					else
					{
						if ( FieldValue.StringValue.Len() > 0 || FieldValue.ArrayValue.Num() == 0 )
						{
							bRet = Settings->SetPropertyFromStringByName(FName(*FieldName),
								FieldValue.StringValue);
						}
						else if ( FieldValue.ArrayValue.Num() > 0 )
						{
							//@todo ronp - add support for setting property value by array index
						}
					}
				}
			}
		}
	}

	return bRet;
}

/**
 * Binds the new settings object to this provider. Sets the type to instance.
 * Forwards the call to the dynamic data provider so it can walk the property
 * list and expose those
 *
 * @param NewSettings the new object to bind
 * @param bIsInList whether to use list handling or not
 *
 * @return TRUE if bound ok, FALSE otherwise
 */
UBOOL UUIDataProvider_Settings::BindSettings(USettings* NewSettings,UBOOL bIsInList)
{
	bIsAListRow = bIsInList;
	Settings = NewSettings;
	DataClass = NewSettings->GetClass();
	// Let the dynamic property binding happen
	if (BindProviderInstance(Settings))
	{
		// Lists don't do array processing
		if (bIsAListRow == FALSE)
		{
			SettingsArrayProviders.Empty(Settings->LocalizedSettingsMappings.Num());
			// Create all of the providers needed for each string settings
			for (INT Index = 0; Index < Settings->LocalizedSettingsMappings.Num(); Index++)
			{
				FSettingsArrayProvider Mapping;
				Mapping.SettingsId = Settings->LocalizedSettingsMappings(Index).Id;
				// Create the object that will manage the exposing of the data
				Mapping.Provider = ConstructObject<UUIDataProvider_SettingsArray>(UUIDataProvider_SettingsArray::StaticClass());
				// Let it cache whatever data it needs
				if (Mapping.Provider->BindStringSetting(Settings,Mapping.SettingsId))
				{
					// Cache the name for later compares
					Mapping.SettingsName = Settings->GetStringSettingName(Mapping.SettingsId);
					if (Mapping.SettingsName != NAME_None)
					{
						// Everything worked, so add to our cache
						SettingsArrayProviders.AddItem(Mapping);
					}
				}
			}
			// Iterate through the property mappings and add any PVMT_PredefinedValues
			for (INT Index = 0; Index < Settings->PropertyMappings.Num(); Index++)
			{
				const FSettingsPropertyPropertyMetaData& PropMapping = Settings->PropertyMappings(Index);
				// This needs to be exposed as an array
				if (PropMapping.MappingType == PVMT_PredefinedValues)
				{
					FSettingsArrayProvider Mapping;
					Mapping.SettingsId = PropMapping.Id;
					// Create the object that will manage the exposing of the data
					Mapping.Provider = ConstructObject<UUIDataProvider_SettingsArray>(UUIDataProvider_SettingsArray::StaticClass());
					// Let it cache whatever data it needs
					if (Mapping.Provider->BindPropertySetting(Settings,Mapping.SettingsId))
					{
						// Cache the name for later compares
						Mapping.SettingsName = PropMapping.Name;
						if (Mapping.SettingsName != NAME_None)
						{
							// Everything worked, so add to our cache
							SettingsArrayProviders.AddItem(Mapping);
						}
					}
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

/* ==========================================================================================================
	UUIDataProvider_SettingsArray
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_SettingsArray);

/**
 * Binds the new settings object and id to this provider.
 *
 * @param NewSettings the new object to bind
 * @param NewSettingsId the id of the settings array to expose
 *
 * @return TRUE if the call worked, FALSE otherwise
 */
UBOOL UUIDataProvider_SettingsArray::BindStringSetting(USettings* NewSettings,INT NewSettingsId)
{
	// Copy the values
	Settings = NewSettings;
	SettingsId = NewSettingsId;
	// And figure out the name of this setting for perf reasons
	SettingsName = Settings->GetStringSettingName(SettingsId);
	ColumnHeaderText = Settings->GetStringSettingColumnHeader(SettingsId);
	// Ditto for the various values
	Settings->GetStringSettingValueNames(SettingsId,Values);
	return SettingsName != NAME_None;
}

/**
 * Binds the property id as an array item. Requires that the property
 * has a mapping type of PVMT_PredefinedValues
 *
 * @param NewSettings the new object to bind
 * @param PropertyId the id of the property to expose as an array
 *
 * @return TRUE if the call worked, FALSE otherwise
 */
UBOOL UUIDataProvider_SettingsArray::BindPropertySetting(USettings* NewSettings,INT PropertyId)
{
	// Copy the values
	Settings = NewSettings;
	SettingsId = PropertyId;
	FSettingsPropertyPropertyMetaData* PropMeta = NewSettings->FindPropertyMetaData(PropertyId);
	if (PropMeta)
	{
		SettingsName = PropMeta->Name;
		ColumnHeaderText = PropMeta->ColumnHeaderText;
		// Iterate through the possible values adding them to our array of choices
		Values.Empty(PropMeta->PredefinedValues.Num());
		Values.AddZeroed(PropMeta->PredefinedValues.Num());
		for (INT Index = 0; Index < PropMeta->PredefinedValues.Num(); Index++)
		{
			const FString& StrVal = PropMeta->PredefinedValues(Index).ToString();
			Values(Index).Id = Index;
			Values(Index).Name = FName(*StrVal);
		}
	}
	return SettingsName != NAME_None;
}

/**
 * Determines if the specified name matches ours
 *
 * @param Property the name to compare with our own
 *
 * @return TRUE if the name matches, FALSE otherwise
 */
UBOOL UUIDataProvider_SettingsArray::IsMatch(const TCHAR* Property)
{
	// Make a copy because we potentially modify the string
	FString CompareName(Property);
	FString ProviderName;
	// If there is an intervening provider name, snip it off
	if (ParseNextDataTag(CompareName,ProviderName) == FALSE)
	{
		CompareName = ProviderName;
	}
	// Now compare the final results
	return FName(*CompareName) == SettingsName;
}

/**
 * Gets the current value for the field specified
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataProvider_SettingsArray::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	check(Settings && SettingsName != NAME_None);
	UBOOL bResult = FALSE;
	// If this is what we are bound to
	if (IsMatch(*FieldName))
	{
		INT ValueIndex;
		// Then get the value
		if (Settings->GetStringSettingValue(SettingsId,ValueIndex))
		{
			// And now look up the value name
			FName ValueName = Settings->GetStringSettingValueName(SettingsId,ValueIndex);
			// If the value has proper metadata, expose it
			if (ValueName != NAME_None)
			{
				OutFieldValue.PropertyTag = SettingsName;
				OutFieldValue.PropertyType = DATATYPE_Collection;
				OutFieldValue.StringValue = ValueName.ToString();
				OutFieldValue.ArrayValue.AddItem(ValueIndex);
				bResult = TRUE;
			}
			else
			{
				//@todo ronp - implement support for property mappings
			}
		}
	}
	return bResult || Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
}

/**
 * Sets the value for the specified field
 *
 * @param	FieldName		the data field to set the value of
 * @param	FieldValue		the value to store for the property specified
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataProvider_SettingsArray::SetFieldValue(const FString& FieldName,
	const FUIProviderScriptFieldValue& FieldValue,INT ArrayIndex)
{
	check(Settings && SettingsName != NAME_None);
	UBOOL bResult = FALSE;
	// If this is what we are bound to
	if ( IsMatch(*FieldName) )
	{
		// Set the value index by string
		if ( FieldValue.StringValue.Len() == 0 && FieldValue.ArrayValue.Num() > 0 )
		{
			INT StringSettingId;
			if ( Settings->GetStringSettingId(FName(*FieldName), StringSettingId) )
			{
				Settings->SetStringSettingValue(StringSettingId,
					FieldValue.ArrayValue(0), FALSE);
				bResult = TRUE;
			}
		}
		else
		{
			bResult = Settings->SetStringSettingValueFromStringByName(SettingsName,
				FieldValue.StringValue);
		}

		if ( !bResult )
		{
			//@todo ronp - implement support for property mappings
		}
	}
	return bResult || Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);
}

/**
 * Builds a list of available fields from the array of properties in the
 * game settings object
 *
 * @param OutFields	out value that receives the list of exposed properties
 */
void UUIDataProvider_SettingsArray::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	check(Settings && SettingsName != NAME_None);
	// Add ourselves as a collection
	new(OutFields) FUIDataProviderField(SettingsName,DATATYPE_Collection);
}

/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<IUIListElementProvider> UUIDataProvider_SettingsArray::ResolveListElementProvider(const FString& PropertyName)
{
	check(Settings && SettingsName != NAME_None);
	if (IsMatch(*PropertyName))
	{
		return TScriptInterface<IUIListElementProvider>(this);
	}
	return TScriptInterface<IUIListElementProvider>();
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUIDataProvider_SettingsArray::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	check(Settings && SettingsName != NAME_None);
	OutCellTags.Set(SettingsName,*ColumnHeaderText);
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	OutCellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUIDataProvider_SettingsArray::GetCellFieldType(FName FieldName, const FName& CellTag,BYTE& OutCellFieldType)
{
	check(Settings && SettingsName != NAME_None);
	if (IsMatch(*CellTag.ToString()))
	{
		OutCellFieldType = DATATYPE_Property;
		return TRUE;
	}
	return FALSE;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUIDataProvider_SettingsArray::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT)
{
	check(Settings && SettingsName != NAME_None);
	if (IsMatch(*CellTag.ToString()))
	{
		for ( INT ValueIndex = 0; ValueIndex < Values.Num(); ValueIndex++ )
		{
			FIdToStringMapping& Value = Values(ValueIndex);
			if ( Value.Id == ListIndex )
			{
				// Set the value to the value cached in the values array
				OutFieldValue.StringValue = Value.Name.ToString();
				OutFieldValue.PropertyTag = SettingsName;
				OutFieldValue.PropertyType = DATATYPE_Property;
				return TRUE;
			}
		}
	}
	return FALSE;
}

// IUIListElementProvider interface

/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUIDataProvider_SettingsArray::GetElementProviderTags(void)
{
	check(Settings && SettingsName != NAME_None);
	TArray<FName> Tags;
	Tags.AddItem(SettingsName);
	return Tags;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	DataTag		a tag corresponding to tag of a data provider in this list element provider
 *						that can be represented by a list
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUIDataProvider_SettingsArray::GetElementCount(FName DataTag)
{
	check(Settings && SettingsName != NAME_None);
	if (IsMatch(*DataTag.ToString()))
	{
		return Values.Num();
	}
	return 0;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	OutElements		will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUIDataProvider_SettingsArray::GetListElements(FName FieldName,TArray<INT>& OutElements)
{
	check(Settings && SettingsName != NAME_None);
	if (IsMatch(*FieldName.ToString()))
	{
		// Loop through the values adding the index
		for (INT Index = 0; Index < Values.Num(); Index++)
		{
			OutElements.AddItem(Values(Index).Id);
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	DataTag			the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UUIDataProvider_SettingsArray::GetElementCellSchemaProvider(FName FieldName)
{
	check(Settings && SettingsName != NAME_None);
	// If this is our field, return this object
	if (IsMatch(*FieldName.ToString()))
	{
		return TScriptInterface<IUIListElementCellProvider>(this);
	}
	return TScriptInterface<IUIListElementCellProvider>();
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UUIDataProvider_SettingsArray::GetElementCellValueProvider(FName FieldName,INT ListIndex)
{
	check(Settings && SettingsName != NAME_None);
	// If this is our field, return this object
	if (IsMatch(*FieldName.ToString()))
	{
		return TScriptInterface<IUIListElementCellProvider>(this);
	}
	return TScriptInterface<IUIListElementCellProvider>();
}

/* ==========================================================================================================
	UUIDataProvider_OnlineFriends
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_OnlinePlayerDataBase);
IMPLEMENT_CLASS(UUIDataProvider_OnlineFriends);

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUIDataProvider_OnlineFriends::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bFound = FALSE;
	OutFieldValue.PropertyTag = CellTag;
	OutFieldValue.PropertyType = DATATYPE_Property;
	UBOOL bUseFriendStatus = FALSE;

	if (FriendsList.IsValidIndex(ListIndex))
	{
		FOnlineFriend& Friend = FriendsList(ListIndex);
		if (CellTag == FName(TEXT("NickName")))
		{
			OutFieldValue.StringValue = Friend.NickName;
		}
		else if (CellTag == FName(TEXT("PresenceInfo")))
		{
			// If the friend doesn't have presence then just use the friend state as presence
			if (Friend.PresenceInfo.Len())
			{
				OutFieldValue.StringValue = Friend.PresenceInfo;
			}
			else
			{
				bUseFriendStatus = TRUE;
			}
		}
		else if (CellTag == FName(TEXT("bIsOnline")))
		{
			OutFieldValue.StringValue = Friend.bIsOnline ? GTrue : GFalse;
		}
		else if (CellTag == FName(TEXT("bIsPlaying")))
		{
			OutFieldValue.StringValue = Friend.bIsPlaying ? GTrue : GFalse;
		}
		else if (CellTag == FName(TEXT("bIsPlayingThisGame")))
		{
			OutFieldValue.StringValue = Friend.bIsPlayingThisGame ? GTrue : GFalse;
		}
		else if (CellTag == FName(TEXT("bIsJoinable")))
		{
			OutFieldValue.StringValue = Friend.bIsJoinable ? GTrue : GFalse;
		}
		else if (CellTag == FName(TEXT("bHasVoiceSupport")))
		{
			OutFieldValue.StringValue = Friend.bHasVoiceSupport ? GTrue : GFalse;
		}
		else if (CellTag == FName(TEXT("bHaveInvited")))
		{
			OutFieldValue.StringValue = Friend.bHaveInvited ? GTrue : GFalse;
		}
		else if (CellTag == FName(TEXT("bHasInvitedYou")))
		{
			OutFieldValue.StringValue = Friend.bHasInvitedYou ? GTrue : GFalse;
		}
		if (bUseFriendStatus ||
			CellTag == FName(TEXT("FriendState")))
		{
			// Use the friend state as the presence string
			switch (Friend.FriendState)
			{
				case OFS_Offline:
				{
					OutFieldValue.StringValue = OfflineText;
					break;
				}
				case OFS_Online:
				{
					OutFieldValue.StringValue = OnlineText;
					break;
				}
				case OFS_Away:
				{
					OutFieldValue.StringValue = AwayText;
					break;
				}
				case OFS_Busy:
				{
					OutFieldValue.StringValue = BusyText;
					break;
				}
			}
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT(" ");
	}
	return TRUE;
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataProvider_OnlineFriends::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(UCONST_UnknownCellDataFieldName,FName(*FieldName),ArrayIndex,OutFieldValue,INDEX_NONE)
		|| Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
}

/* ==========================================================================================================
	UUIDataProvider_OnlineFriendMessages
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_OnlineFriendMessages);

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUIDataProvider_OnlineFriendMessages::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bFound = FALSE;
	OutFieldValue.PropertyTag = CellTag;
	OutFieldValue.PropertyType = DATATYPE_Property;

	if (Messages.IsValidIndex(ListIndex))
	{
		if (CellTag == FName(TEXT("SendingPlayerNick")))
		{
			OutFieldValue.StringValue = Messages(ListIndex).SendingPlayerNick;
		}
		else if (CellTag == FName(TEXT("bIsFriendInvite")))
		{
			OutFieldValue.StringValue = Messages(ListIndex).bIsFriendInvite ? GTrue : GFalse;
		}
		else if (CellTag == FName(TEXT("bWasAccepted")))
		{
			OutFieldValue.StringValue = Messages(ListIndex).bWasAccepted ? GTrue : GFalse;
		}
		else if (CellTag == FName(TEXT("bWasDenied")))
		{
			OutFieldValue.StringValue = Messages(ListIndex).bWasDenied ? GTrue : GFalse;
		}
		else if (CellTag == FName(TEXT("Message")))
		{
			OutFieldValue.StringValue = Messages(ListIndex).Message;
		}
	}

	// Make sure we provide something (or we'll crash)
	if (OutFieldValue.StringValue.Len() == 0)
	{
		OutFieldValue.StringValue = TEXT(" ");
	}
	return TRUE;
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataProvider_OnlineFriendMessages::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	if (FieldName == TEXT("LastInviteFrom"))
	{
		OutFieldValue.PropertyTag = FName(*FieldName);
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = LastInviteFrom;
		return TRUE;
	}
	return GetCellFieldValue(UCONST_UnknownCellDataFieldName,FName(*FieldName),ArrayIndex,OutFieldValue,INDEX_NONE)
		 || Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
}

/* ==========================================================================================================
	UUIDataProvider_PlayerAchievements
========================================================================================================== */
IMPLEMENT_CLASS(UUIDataProvider_PlayerAchievements);

static UScriptStruct* GetAchievementDetailsStruct()
{
	static UScriptStruct* AchievementsDetailsStruct = FindField<UScriptStruct>(UOnlineSubsystem::StaticClass(),TEXT("AchievementDetails"));
	checkSlow(AchievementsDetailsStruct);

	return AchievementsDetailsStruct;
}

/**
 * Returns the number of gamer points this profile has accumulated across all achievements
 *
 * @return	a number between 0 and the maximum number of gamer points allocated for each game (currently 1000), representing the total
 * gamer points earned from all achievements for this profile.
 */
INT UUIDataProvider_PlayerAchievements::GetTotalGamerScore() const
{
	INT Result = 0;

	for ( INT Idx = 0; Idx < Achievements.Num(); Idx++ )
	{
		const FAchievementDetails& Achievement = Achievements(Idx);
		if ( Achievement.bWasAchievedOffline || Achievement.bWasAchievedOnline )
		{
			Result += Achievement.GamerPoints;
		}
	}

	return Min(1000, Result);
}

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	out_CellTags	the columns supported by this row
 */
void UUIDataProvider_PlayerAchievements::GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
{
	UScriptStruct* AchievementDetailsStruct = GetAchievementDetailsStruct();
	
	for ( TFieldIterator<UProperty> It(AchievementDetailsStruct); It; ++It )
	{
		UProperty* Prop = *It;
		out_CellTags.Set(Prop->GetFName(), *Prop->GetFriendlyName());
	}

	out_CellTags.Set(TEXT("ConditionalDescription"), TEXT("ConditionalDescription"));
	out_CellTags.Set(TEXT("IsCompleted"), TEXT("IsCompleted"));
	out_CellTags.Set(TEXT("ProgressRatio"), TEXT("ProgressRatio"));
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	OutCellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUIDataProvider_PlayerAchievements::GetCellFieldType(FName FieldName, const FName& CellTag,BYTE& OutCellFieldType)
{
	UBOOL bResult = FALSE;

	UScriptStruct* AchievementDetailsStruct = GetAchievementDetailsStruct();
	if ( FindField<UProperty>(AchievementDetailsStruct,CellTag) != NULL )
	{
		OutCellFieldType = DATATYPE_Property;
		bResult = TRUE;
	}
	else if ( CellTag == TEXT("IsCompleted") || CellTag == TEXT("ConditionalDescription") )
	{
		OutCellFieldType = DATATYPE_Property;
		bResult = TRUE;
	}
	else if ( CellTag == TEXT("ProgressRatio") )
	{
		OutCellFieldType = DATATYPE_RangeProperty;
		bResult = TRUE;
	}

	return bResult;
}
/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUIDataProvider_PlayerAchievements::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& OutFieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	FString FieldTag, NextFieldName = FieldName != NAME_None 
		? FieldName.ToString()
		: CellTag != NAME_None
			? CellTag.ToString()
			: TEXT("");

	TArray<FUIDataProviderField> SupportedFields;
	ParseNextDataTag(NextFieldName, FieldTag);
// 	debugf(TEXT("GetCellFieldValue - FieldName:%s CellTag:%s    NextFieldName:%s   FieldTag:%s"), *FieldName.ToString(), *CellTag.ToString(), *NextFieldName, *FieldTag);
	while ( FieldTag.Len() > 0 ) 
	{
		if ( IsDataTagSupported(*FieldTag, SupportedFields) )
		{
			INT NonCollectionArrayIndex = ParseArrayDelimiter(FieldTag);
			if ( FieldTag == TEXT("Achievements") )
			{
				if ( Achievements.IsValidIndex(ListIndex) )
				{
					OutFieldValue.PropertyTag = CellTag;
					OutFieldValue.PropertyType = DATATYPE_Property;
					FAchievementDetails& Achievement = Achievements(ListIndex);
					
					UScriptStruct* AchievementDetailsStruct = GetAchievementDetailsStruct();
					UProperty* Property = FindField<UProperty>(AchievementDetailsStruct,CellTag);
					if ( Property != NULL )
					{
						if ( CellTag == TEXT("Id") )
						{
							OutFieldValue.StringValue = appItoa(Achievement.Id);
							bResult = TRUE;
						}
						else if ( CellTag == TEXT("AchievementName") )
						{
							OutFieldValue.StringValue = Achievement.AchievementName;
							bResult = TRUE;
						}
						else if ( CellTag == TEXT("Description") )
						{
							OutFieldValue.StringValue=  Achievement.Description;
							bResult = TRUE;
						}
						else if ( CellTag == TEXT("HowTo") )
						{
							OutFieldValue.StringValue = Achievement.HowTo;
							bResult = TRUE;
						}
						else if ( CellTag == TEXT("GamerPoints") )
						{
							OutFieldValue.StringValue = appItoa(Achievement.GamerPoints);
							bResult = TRUE;
						}
						else if ( CellTag == TEXT("bIsSecret") )
						{
							OutFieldValue.StringValue = Achievement.bIsSecret ? TEXT("1") : TEXT("0");
							OutFieldValue.ArrayValue.AddItem(Achievement.bIsSecret ? 1 : 0);
							bResult = TRUE;
						}
						else if ( CellTag == TEXT("bWasAchievedOnline") )
						{
							OutFieldValue.StringValue = Achievement.bWasAchievedOnline ? TEXT("1") : TEXT("0");
							OutFieldValue.ArrayValue.AddItem(Achievement.bWasAchievedOnline ? 1 : 0);
							bResult = TRUE;
						}
						else if ( CellTag == TEXT("bWasAchievedOffline") )
						{
							OutFieldValue.StringValue = Achievement.bWasAchievedOffline ? TEXT("1") : TEXT("0");
							OutFieldValue.ArrayValue.AddItem(Achievement.bWasAchievedOffline ? 1 : 0);
							bResult = TRUE;
						}
						else if ( CellTag == TEXT("Image") )
						{
							OutFieldValue.ImageValue = Achievement.Image;
							bResult = TRUE;
						}
					}

					// the remaining tags are virtual - that is, their values are determined programmatically
					else if ( CellTag == TEXT("IsCompleted") )
					{
						INT ValueInt = (Achievement.bWasAchievedOffline || Achievement.bWasAchievedOnline) ? 1 : 0;
						OutFieldValue.StringValue = appItoa(ValueInt);
						OutFieldValue.ArrayValue.AddItem(ValueInt);
						bResult = TRUE;
					}
					else if ( CellTag == TEXT("ConditionalDescription") )
					{
						OutFieldValue.StringValue = (Achievement.bWasAchievedOnline || Achievement.bWasAchievedOnline)
							? Achievement.Description
							: Achievement.HowTo;
						bResult = TRUE;
					}
					else if ( CellTag == TEXT("ProgressRatio") )
					{
						OutFieldValue.PropertyType = DATATYPE_RangeProperty;
						if ( Player != NULL && Player->Actor != NULL )
						{
							FLOAT CurrentValue, MaxValue;
							CurrentValue = MaxValue = OutFieldValue.RangeValue.MinValue = 0.f;
							if ( Player->Actor->eventGetAchievementProgression(Achievement.Id, CurrentValue, MaxValue) )
							{
								OutFieldValue.RangeValue.MaxValue = MaxValue;
								OutFieldValue.RangeValue.SetCurrentValue(CurrentValue);
							}
							else
							{
								OutFieldValue.RangeValue.MaxValue = 1.f;
								CurrentValue = (Achievement.bWasAchievedOffline || Achievement.bWasAchievedOnline) ? 1.f : 0.f;
								OutFieldValue.RangeValue.SetCurrentValue(CurrentValue);
							}
							OutFieldValue.StringValue = appItoa(appTrunc(CurrentValue));
							bResult = TRUE;
						}
					}
				}
			}

			break;
		}

		ParseNextDataTag(NextFieldName, FieldTag);
	}

	return bResult;
}

/**
 * Gets the list of data fields exposed by this data provider
 *
 * @param OutFields Filled in with the list of fields supported by its aggregated providers
 */
void UUIDataProvider_PlayerAchievements::GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField(TEXT("Achievements"), DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField(TEXT("TotalGamerPoints"), DATATYPE_RangeProperty);

	Super::GetSupportedDataFields(out_Fields);
}

/**
 * Gets the value for the specified field
 *
 * @param	FieldName		the field to look up the value for
 * @param	OutFieldValue	out param getting the value
 * @param	ArrayIndex		ignored
 */
UBOOL UUIDataProvider_PlayerAchievements::GetFieldValue(const FString& FieldName, FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	/*
	Values should come in like so:
	FieldName: <one of the properties in the AchievementsData struct>
	ArrayIndex: <the index for the element of the Achievements array to get data for>
	*/

	UBOOL bResult = FALSE;

	FString NextFieldName = FieldName, FieldTag;
	if ( ParseNextDataTag(NextFieldName, FieldTag) )
	{
		const INT InstanceIndex = ParseArrayDelimiter(FieldTag);
		if ( NextFieldName.Len() > 0 && Achievements.IsValidIndex(InstanceIndex) )
		{
			bResult = GetCellFieldValue(*FieldTag, *NextFieldName, InstanceIndex, OutFieldValue, ArrayIndex);
		}
	}
	else if ( FieldTag == TEXT("TotalGamerPoints") )
	{
		OutFieldValue.PropertyTag = TEXT("TotalGamerPoints");
		OutFieldValue.PropertyType = DATATYPE_RangeProperty;
		OutFieldValue.RangeValue.MinValue = 0;
		OutFieldValue.RangeValue.MaxValue = 1000;
		OutFieldValue.RangeValue.bIntRange = TRUE;

		const INT TotalScore = GetTotalGamerScore();
		OutFieldValue.RangeValue.SetCurrentValue(TotalScore);
		OutFieldValue.StringValue = appItoa(TotalScore);
		bResult = TRUE;
	}

// 	debugf(TEXT("UUIDataProvider_PlayerAchievements::GetFieldValue  FieldName:%s    ArrayIndex:%i  bResult:%i"), *FieldName, ArrayIndex, bResult);

	return bResult || Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
}

/* ==========================================================================================================
	UUIDataStore_OnlinePlayerData
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataStore_OnlinePlayerData);

/**
 * Loads the game specific OnlineProfileSettings class
 */
void UUIDataStore_OnlinePlayerData::LoadDependentClasses(void)
{
	if (ProfileSettingsClassName.Len() > 0)
	{
		// Try to load the specified class
		ProfileSettingsClass = LoadClass<UOnlineProfileSettings>(NULL,*ProfileSettingsClassName,NULL,LOAD_None,NULL);
		if (ProfileSettingsClass == NULL)
		{
			debugf(NAME_Error,TEXT("Failed to load OnlineProfileSettings class %s"),*ProfileSettingsClassName);
		}
	}
	if (FriendsProviderClassName.Len() > 0)
	{
		// Try to load the specified class
		FriendsProviderClass = LoadClass<UUIDataProvider_OnlineFriends>(NULL,*FriendsProviderClassName,NULL,LOAD_None,NULL);
	}
	if (FriendsProviderClass == NULL)
	{
		FriendsProviderClass = UUIDataProvider_OnlineFriends::StaticClass();
	}
	if (FriendMessagesProviderClassName.Len() > 0)
	{
		// Try to load the specified class
		FriendMessagesProviderClass = LoadClass<UUIDataProvider_OnlineFriendMessages>(NULL,*FriendMessagesProviderClassName,NULL,LOAD_None,NULL);
	}
	if (FriendMessagesProviderClass == NULL)
	{
		FriendMessagesProviderClass = UUIDataProvider_OnlineFriendMessages::StaticClass();
	}
	if (PlayersProviderClassName.Len() > 0)
	{
		// Try to load the specified class
		PlayersProviderClass = LoadClass<UUIDataProvider_OnlinePlayers>(NULL,*PlayersProviderClassName,NULL,LOAD_None,NULL);
	}
	if (PlayersProviderClass == NULL)
	{
		PlayersProviderClass = UUIDataProvider_OnlinePlayers::StaticClass();
	}
	if (ClanMatesProviderClassName.Len() > 0)
	{
		// Try to load the specified class
		ClanMatesProviderClass = LoadClass<UUIDataProvider_OnlineClanMates>(NULL,*ClanMatesProviderClassName,NULL,LOAD_None,NULL);
	}
	if (ClanMatesProviderClass == NULL)
	{
		ClanMatesProviderClass = UUIDataProvider_OnlineClanMates::StaticClass();
	}
	if (AchievementsProviderClassName.Len() > 0)
	{
		// Try to load the specified class
		AchievementsProviderClass = LoadClass<UUIDataProvider_PlayerAchievements>(NULL,*AchievementsProviderClassName,NULL,LOAD_None,NULL);
	}
	if (AchievementsProviderClass == NULL)
	{
		AchievementsProviderClass = UUIDataProvider_PlayerAchievements::StaticClass();
	}
}

/**
 * Creates the data providers exposed by this data store
 */
void UUIDataStore_OnlinePlayerData::InitializeDataStore(void)
{
	if (FriendsProvider == NULL)
	{
		FriendsProvider = ConstructObject<UUIDataProvider_OnlineFriends>(FriendsProviderClass);
#if !CONSOLE
		// Create one element in the editor
		if (GIsEditor && !GIsGame && FriendsProvider != NULL)
		{
			// Add three fake entires in the editor for UI authors
			FOnlineFriend FakeFriend;
			FriendsProvider->FriendsList.AddItem(FakeFriend);
			FriendsProvider->FriendsList.AddItem(FakeFriend);
			FriendsProvider->FriendsList.AddItem(FakeFriend);
		}
#endif
	}
	if (PlayersProvider == NULL)
	{
		PlayersProvider = ConstructObject<UUIDataProvider_OnlinePlayers>(PlayersProviderClass);
	}
	if (ClanMatesProvider == NULL)
	{
		ClanMatesProvider = ConstructObject<UUIDataProvider_OnlineClanMates>(ClanMatesProviderClass);
	}
	if (ProfileProvider == NULL)
	{
		ProfileProvider = ConstructObject<UUIDataProvider_OnlineProfileSettings>(UUIDataProvider_OnlineProfileSettings::StaticClass());
	}
	if (FriendMessagesProvider == NULL)
	{
		FriendMessagesProvider = ConstructObject<UUIDataProvider_OnlineFriendMessages>(FriendMessagesProviderClass);
	}
	if (AchievementsProvider == NULL)
	{
		AchievementsProvider = ConstructObject<UUIDataProvider_PlayerAchievements>(AchievementsProviderClass);
	}
	check(FriendsProvider && PlayersProvider && ClanMatesProvider && FriendMessagesProvider && AchievementsProvider);
}

/**
 * Forwards the calls to the data providers so they can do their start up
 *
 * @param Player the player that will be associated with this DataStore
 */
void UUIDataStore_OnlinePlayerData::OnRegister(ULocalPlayer* Player)
{
	if (FriendsProvider)
	{
		FriendsProvider->eventOnRegister(Player);
	}
	if (FriendMessagesProvider)
	{
		FriendMessagesProvider->eventOnRegister(Player);
	}
	if (PlayersProvider)
	{
		PlayersProvider->eventOnRegister(Player);
	}
	if (ClanMatesProvider)
	{
		ClanMatesProvider->eventOnRegister(Player);
	}
	if ( ProfileProvider && ProfileSettingsClass )
	{
		// if a cached profile exists, bind to that profile instead
		UOnlineProfileSettings* Profile = eventGetCachedPlayerProfile(Player != NULL ? Player->ControllerId : 0);
		if ( Profile == NULL )
		{
			Profile = ConstructObject<UOnlineProfileSettings>(ProfileSettingsClass);
		}

		// Construct the game specific profile settings
		ProfileProvider->BindProfileSettings(Profile);
		// Now kick off the read for it
		ProfileProvider->eventOnRegister(Player);
	}

	if ( AchievementsProvider != NULL )
	{
		AchievementsProvider->eventOnRegister(Player);
	}

	// Our local events
	eventOnRegister(Player);
}

/**
 * Tells all of the child providers to clear their player data
 *
 * @param Player ignored
 */
void UUIDataStore_OnlinePlayerData::OnUnregister(ULocalPlayer*)
{
	if (FriendsProvider)
	{
		FriendsProvider->eventOnUnregister();
	}
	if (FriendMessagesProvider)
	{
		FriendMessagesProvider->eventOnUnregister();
	}
	if (PlayersProvider)
	{
		PlayersProvider->eventOnUnregister();
	}
	if (ClanMatesProvider)
	{
		ClanMatesProvider->eventOnUnregister();
	}
	if (ProfileProvider)
	{
		ProfileProvider->eventOnUnregister();
	}

	if ( AchievementsProvider != NULL )
	{
		AchievementsProvider->eventOnUnregister();
	}
	// Our local events
	eventOnUnregister();
}

/**
 * Handler for this data store internal settings data providers OnDataProviderPropertyChange delegate.  When the setting associated with that
 * provider is updated, issues a refresh notification which causes e.g. widgets to refresh their values.
 *
 * @param	SourceProvider	the data provider that generated the notification
 * @param	SettingsName	the name of the setting that changed
 */
void UUIDataStore_OnlinePlayerData::OnSettingProviderChanged( UUIDataProvider* SourceProvider, FName SettingsName/*=NAME_None*/ )
{
	INT ValueContextId=INDEX_NONE;

	FName UpdatedPropertyName = SettingsName;
	UUIDataProvider_OnlineProfileSettingsArray* SettingsArrayProvider = Cast<UUIDataProvider_OnlineProfileSettingsArray>(SourceProvider);
	if ( SettingsArrayProvider != NULL && SettingsArrayProvider->ProfileSettings != NULL )
	{
		if ( !SettingsArrayProvider->ProfileSettings->GetProfileSettingValueId(SettingsArrayProvider->ProfileSettingId, ValueContextId) )
		{
			ValueContextId = INDEX_NONE;
		}

		UpdatedPropertyName = *(ProfileProvider->ProviderName.ToString() + TEXT(".") + *SettingsName.ToString());
	}

	eventRefreshSubscribers(UpdatedPropertyName, TRUE, SourceProvider, ValueContextId);

	if ( ProviderChangedNotifies.Num() > 0 )
	{
		UIDataProvider_eventOnDataProviderPropertyChange_Parms Parms(EC_EventParm);
		Parms.SourceProvider = SourceProvider;
		Parms.PropTag = UpdatedPropertyName;

		for ( INT Index = 0; Index < ProviderChangedNotifies.Num(); Index++ )
		{
			FScriptDelegate& Delegate = ProviderChangedNotifies(Index);
			if ( Delegate.IsCallable(this) )
			{
				ProcessDelegate(NAME_None, &Delegate, &Parms);
			}
		}
	}
}

/**
 * Gets the list of data fields exposed by this data provider
 *
 * @param OutFields Filled in with the list of fields supported by its aggregated providers
 */
void UUIDataStore_OnlinePlayerData::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	// Data stores empty the array, data providers append to the array
	OutFields.Empty();

	// Add the player nick name
	new(OutFields)FUIDataProviderField(FName(TEXT("PlayerNickName")),DATATYPE_Property);
	// Add the downloadable content counts
	new(OutFields)FUIDataProviderField(FName(TEXT("NumNewDownloadsAvailable")),DATATYPE_Property);
	new(OutFields)FUIDataProviderField(FName(TEXT("TotalDownloadsAvailable")),DATATYPE_Property);

	if ( GIsEditor && !GIsGame )
	{
		// Use the default objects in the editor
		UUIDataProvider_OnlineFriends* DefaultFriendsProvider = UUIDataProvider_OnlineFriends::StaticClass()->GetDefaultObject<UUIDataProvider_OnlineFriends>();
		check(DefaultFriendsProvider);
		DefaultFriendsProvider->GetSupportedDataFields(OutFields);
		
		UUIDataProvider_OnlineFriendMessages* DefaultFriendMessagesProvider = UUIDataProvider_OnlineFriendMessages::StaticClass()->GetDefaultObject<UUIDataProvider_OnlineFriendMessages>();
		check(DefaultFriendMessagesProvider);
		DefaultFriendMessagesProvider->GetSupportedDataFields(OutFields);
		
		UUIDataProvider_OnlinePlayers* DefaultPlayersProvider = UUIDataProvider_OnlinePlayers::StaticClass()->GetDefaultObject<UUIDataProvider_OnlinePlayers>();
		check(DefaultPlayersProvider);
		DefaultPlayersProvider->GetSupportedDataFields(OutFields);
		
		UUIDataProvider_OnlineClanMates* DefaultClanMatesProvider = UUIDataProvider_OnlineClanMates::StaticClass()->GetDefaultObject<UUIDataProvider_OnlineClanMates>();
		check(DefaultClanMatesProvider);
		DefaultClanMatesProvider->GetSupportedDataFields(OutFields);

		UUIDataProvider_PlayerAchievements* DefaultAchievementsProvider = AchievementsProviderClass->GetDefaultObject<UUIDataProvider_PlayerAchievements>();
		check(DefaultAchievementsProvider);
		DefaultAchievementsProvider->GetSupportedDataFields(OutFields);

		if (ProfileSettingsClass != NULL)
		{
			UUIDataProvider_OnlineProfileSettings* DefaultProfileProvider = UUIDataProvider_OnlineProfileSettings::StaticClass()->GetDefaultObject<UUIDataProvider_OnlineProfileSettings>();
			check(DefaultProfileProvider);
			// The profile provider needs to construct a default settings object in editor
			if (DefaultProfileProvider->Profile == NULL)
			{
				UOnlineProfileSettings* Profile = ProfileSettingsClass->GetDefaultObject<UOnlineProfileSettings>();
				Profile->ProfileSettings.Empty();
				// Copy the defaults into the settings so there is data to view in editor
				Profile->ProfileSettings.AddZeroed(Profile->DefaultSettings.Num());
				for (INT Index = 0; Index < Profile->ProfileSettings.Num(); Index++)
				{
					Profile->ProfileSettings(Index) = Profile->DefaultSettings(Index);
				}
				DefaultProfileProvider->BindProfileSettings(Profile);
			}
			// Add the profile data as its own provider
			new(OutFields)FUIDataProviderField(DefaultProfileProvider->ProviderName,
				DATATYPE_Provider,DefaultProfileProvider);
		}
	}
	else
	{
		check(FriendsProvider && PlayersProvider && ClanMatesProvider && FriendMessagesProvider);
		// Ask the providers for their fields
		FriendsProvider->GetSupportedDataFields(OutFields);
		FriendMessagesProvider->GetSupportedDataFields(OutFields);
		PlayersProvider->GetSupportedDataFields(OutFields);
		ClanMatesProvider->GetSupportedDataFields(OutFields);
		if (ProfileProvider)
		{
			// Add the profile data as its own provider
			new(OutFields)FUIDataProviderField(ProfileProvider->ProviderName,
				DATATYPE_Provider,ProfileProvider);
		}

		if ( AchievementsProvider )
		{
			AchievementsProvider->GetSupportedDataFields(OutFields);
		}
	}
}

/**
 * Parses the data store reference and resolves the data provider and field that is referenced by the markup.
 *
 * @param	MarkupString	a markup string that can be resolved to a data field contained by this data provider, or one of its
 *							internal data providers.
 * @param	out_FieldOwner	receives the value of the data provider that owns the field referenced by the markup string.
 * @param	out_FieldTag	receives the value of the property or field referenced by the markup string.
 * @param	out_ArrayIndex	receives the optional array index for the data field referenced by the markup string.  If there is no array index embedded in the markup string,
 *							value will be INDEX_NONE.
 *
 * @return	TRUE if this data store was able to successfully resolve the string specified.
 */
UBOOL UUIDataStore_OnlinePlayerData::ParseDataStoreReference( const FString& MarkupString, UUIDataProvider*& out_FieldOwner, FString& out_FieldTag, INT& out_ArrayIndex )
{
	UBOOL bResult = FALSE;

	// extract the part of the markup string which corresponds to a data field supported by this data provider
	FString NextFieldName = MarkupString, FieldTag;
	if ( ParseNextDataTag(NextFieldName, FieldTag) )
	{
		INT InstanceIndex = ParseArrayDelimiter(FieldTag);
		if ( (FieldTag == TEXT("Achievements") && InstanceIndex != INDEX_NONE) || FieldTag == TEXT("TotalGamerPoints") )
		{
			// accessing a single of the AchievementsProvider's Achievements array
			out_FieldOwner = this;
			out_FieldTag = MarkupString;
			bResult = TRUE;
		}
	}

	return bResult || Super::ParseDataStoreReference(MarkupString, out_FieldOwner, out_FieldTag, out_ArrayIndex);
}

/**
 * Gets the value for the specified field
 *
 * @param	FieldName		the field to look up the value for
 * @param	OutFieldValue	out param getting the value
 * @param	ArrayIndex		ignored
 */
UBOOL UUIDataStore_OnlinePlayerData::GetFieldValue(const FString& FieldName, FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	FString NextFieldName = FieldName, FieldTag;
	ParseNextDataTag(NextFieldName, FieldTag);

	const INT InstanceIndex = ParseArrayDelimiter(FieldTag);

	// See if this is one of our per player properties
	UBOOL bResult = FALSE;
	if (FieldTag == TEXT("PlayerNickName"))
	{
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = PlayerNick;
		bResult = TRUE;
	}
	else if (FieldTag == TEXT("NumNewDownloadsAvailable"))
	{
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"),NumNewDownloads);
		bResult = TRUE;
	}
	else if (FieldTag == TEXT("TotalDownloadsAvailable"))
	{
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = FString::Printf(TEXT("%d"),NumTotalDownloads);
		bResult = TRUE;
	}
	else if ( (FieldTag == TEXT("Achievements") || FieldTag == TEXT("TotalGamerPoints")) && AchievementsProvider != NULL )
	{
		bResult = AchievementsProvider->GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
	}

	return bResult || Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
}

/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUIDataStore_OnlinePlayerData::GetElementProviderTags(void)
{
	TArray<FName> Tags;
	Tags.AddItem(FName(TEXT("Friends")));
	Tags.AddItem(FName(TEXT("FriendMessages")));
	Tags.AddItem(FName(TEXT("Achievements")));
	return Tags;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUIDataStore_OnlinePlayerData::GetElementCount( FName FieldName )
{
	if ( FieldName == TEXT("Achievements") && AchievementsProvider != NULL )
	{
		return AchievementsProvider->Achievements.Num();
	}

	return 1;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	OutElements		will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUIDataStore_OnlinePlayerData::GetListElements(FName FieldName,TArray<INT>& OutElements)
{
	if (FriendsProvider && FieldName == FName(TEXT("Friends")))
	{
		// For each friend add the provider as an entry
		for (INT Index = 0; Index < FriendsProvider->FriendsList.Num(); Index++)
		{
			OutElements.AddItem(Index);
		}
	}
	if (FriendMessagesProvider && FieldName == FName(TEXT("FriendMessages")))
	{
		// For each message add the provider as an entry
		for (INT Index = 0; Index < FriendMessagesProvider->Messages.Num(); Index++)
		{
			OutElements.AddItem(Index);
		}
	}
	if (PlayersProvider && FieldName == FName(TEXT("Players")))
	{
		// These aren't wired up yet
	}
	if (ClanMatesProvider && FieldName == FName(TEXT("ClanMates")))
	{
		// These aren't wired up yet
	}
	if ( AchievementsProvider && FieldName == TEXT("Achievements") )
	{
		//@todo ronp - here we need to create a syntax for specifying the subrange of achievements to pull
		for ( INT AchievementIdx = 0; AchievementIdx < AchievementsProvider->Achievements.Num(); AchievementIdx++ )
		{
			OutElements.AddItem(AchievementIdx);
		}
	}
	return 
		FieldName == FName(TEXT("ClanMates")) ||
		FieldName == FName(TEXT("Players")) ||
		FieldName == FName(TEXT("Friends")) ||
		FieldName == FName(TEXT("FriendMessages")) ||
		FieldName == TEXT("Achievements");
}


/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UUIDataStore_OnlinePlayerData::GetElementCellSchemaProvider(FName FieldName)
{
	if (FieldName == FName(TEXT("Friends")))
	{
		return FriendsProvider;
	}
	else if (FieldName == FName(TEXT("FriendMessages")))
	{
		return FriendMessagesProvider;
	}
	else if (FieldName == FName(TEXT("Players")))
	{
		return PlayersProvider;
	}
	else if (FieldName == FName(TEXT("ClanMates")))
	{
		return ClanMatesProvider;
	}
	else if ( FieldName == TEXT("Achievements") )
	{
		return AchievementsProvider;
	}
	return TScriptInterface<IUIListElementCellProvider>();
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 * 
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UUIDataStore_OnlinePlayerData::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	return GetElementCellSchemaProvider(FieldName);
}

/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<class IUIListElementProvider> UUIDataStore_OnlinePlayerData::ResolveListElementProvider(const FString& PropertyName)
{
	FString CompareName(PropertyName);
	FString ProviderName;
	// If there is an intervening provider name, snip it off
	ParseNextDataTag(CompareName,ProviderName);
	// If this provider matches the profile provider..
	if (FName(*ProviderName) == ProfileProvider->ProviderName)
	{
		return ProfileProvider->ResolveListElementProvider(CompareName);
	}
	return this;
}

/* ==========================================================================================================
	UUIDataProvider_OnlineProfileSettings
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_OnlineProfileSettings);

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataProvider_OnlineProfileSettings::GetFieldValue(const FString& FieldName,
	FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	UBOOL bResult = FALSE;
	FName TagName(*FieldName);
	INT ProfileSettingId;
	// Find the ID so we can look up meta data info
	if (Profile->GetProfileSettingId(TagName,ProfileSettingId))
	{
		BYTE MappingType;
		// Determine the type of property mapping for correct UI exposure
		Profile->GetProfileSettingMappingType(ProfileSettingId,MappingType);
		if (MappingType == PVMT_Ranged)
		{
			OutFieldValue.PropertyType = DATATYPE_RangeProperty;
			FLOAT MinV, MaxV, IncrementV;
			BYTE bIntRange;
			// Read and set the range for this value
			Profile->GetProfileSettingRange(ProfileSettingId,MinV,MaxV,IncrementV,bIntRange);
			OutFieldValue.RangeValue.MaxValue = MaxV;
			OutFieldValue.RangeValue.MinValue = MinV;
			OutFieldValue.RangeValue.SetNudgeValue(IncrementV);
			OutFieldValue.RangeValue.bIntRange = bIntRange != 0 ? TRUE : FALSE;
			FLOAT Value;
			// Read the value it currently has
			Profile->GetRangedProfileSettingValue(ProfileSettingId,Value);
			OutFieldValue.RangeValue.SetCurrentValue(Value);
			bResult = TRUE;
		}
		else
		{
			// Read the value by name and store the results if found
			if (Profile->GetProfileSettingValueByName(TagName,OutFieldValue.StringValue))
			{
				OutFieldValue.PropertyType = DATATYPE_Property;
				OutFieldValue.PropertyTag = TagName;
				bResult = TRUE;
			}
			for (INT ProviderIndex = 0; ProviderIndex < ProfileSettingsArrayProviders.Num(); ProviderIndex++)
			{
				FProfileSettingsArrayProvider& ArrayProvider = ProfileSettingsArrayProviders(ProviderIndex);
				// If the requested field actually corresponds to a collection, we should also fill in the ArrayValue.
				if (ArrayProvider.ProfileSettingsId == ProfileSettingId)
				{
					ArrayProvider.Provider->GetFieldValue(TagName.ToString(), OutFieldValue, ArrayIndex);
					bResult = TRUE;
					break;
				}
			}
		}
	}
	return bResult || Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataProvider_OnlineProfileSettings::SetFieldValue(const FString& FieldName,
	const FUIProviderScriptFieldValue& FieldValue,INT ArrayIndex)
{
	UBOOL bResult = FALSE;
	UBOOL bValueChanged = FALSE;

	FName TagName(*FieldName);
	INT ProfileSettingId;
	// Find the ID so we can look up meta data info
	if (Profile->GetProfileSettingId(FName(*FieldName),ProfileSettingId))
	{
		// If the requested field actually corresponds to a collection, we should use the ArrayValue to determine which value to use for saving.
		if ( FieldValue.PropertyType == DATATYPE_Collection )
		{
			for ( INT ProviderIndex = 0; ProviderIndex < ProfileSettingsArrayProviders.Num(); ProviderIndex++ )
			{
				FProfileSettingsArrayProvider& ArrayProvider = ProfileSettingsArrayProviders(ProviderIndex);
				if ( ArrayProvider.ProfileSettingsId == ProfileSettingId )
				{
					FUIProviderFieldValue CurrentValue(EC_EventParm);
					ArrayProvider.Provider->GetFieldValue(FieldName, CurrentValue, ArrayIndex);

					if ( ArrayProvider.Provider->SetFieldValue(FieldName, FieldValue, ArrayIndex) )
					{
						bValueChanged = CurrentValue.ArrayValue != FieldValue.ArrayValue
								|| CurrentValue.StringValue != FieldValue.StringValue
								|| CurrentValue.RangeValue.GetCurrentValue() != FieldValue.RangeValue.GetCurrentValue();
					}
					bResult = TRUE;
					break;
				}
			}
		}
		BYTE Type;
		// Now find the mapping type by id
		if (Profile->GetProfileSettingMappingType(ProfileSettingId,Type))
		{
			if (Type == PVMT_Ranged)
			{
				FLOAT CurrentValue=0.f;
				Profile->GetRangedProfileSettingValue(ProfileSettingId, CurrentValue);

				// Set the value and clamp to its range
				if ( Profile->SetRangedProfileSettingValue(ProfileSettingId,
					FieldValue.RangeValue.GetCurrentValue()) )
				{
					bValueChanged = CurrentValue != FieldValue.RangeValue.GetCurrentValue();
					bResult = TRUE;
				}
			}
			else
			{
				FString CurrentValue;
				Profile->GetProfileSettingValueByName(TagName, CurrentValue);

				// Save the value by name
				if ( Profile->SetProfileSettingValueByName(TagName,FieldValue.StringValue) )
				{
					bValueChanged = CurrentValue != FieldValue.StringValue;
					bResult = TRUE;
				}
			}
		}
	}

// 	if ( bValueChanged )
// 	{
// 		eventNotifyPropertyChanged(*(ProviderName.ToString() + TEXT(".") + FieldName));
// 	}
	return bResult || Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);
}

/**
 * Builds a list of available fields from the array of properties in the
 * game settings object
 *
 * @param OutFields	out value that receives the list of exposed properties
 */
void UUIDataProvider_OnlineProfileSettings::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	// Add all the array mappings as collections
	for (INT Index = 0; Index < ProfileSettingsArrayProviders.Num(); Index++)
	{
		FProfileSettingsArrayProvider& Provider = ProfileSettingsArrayProviders(Index);
		new(OutFields)FUIDataProviderField(Provider.ProfileSettingsName,DATATYPE_Collection);
	}
	// For each profile setting in the profile object, add it to the list
	for (INT Index = 0; Index < Profile->DefaultSettings.Num(); Index++)
	{
		const FOnlineProfileSetting& Setting = Profile->DefaultSettings(Index);
		BYTE Type;
		// Determine the mapping type for this setting
		if (Profile->GetProfileSettingMappingType(Setting.ProfileSetting.PropertyId,Type))
		{
			// Don't add the ones that are provider mapped
			if (Type != PVMT_IdMapped && Type != PVMT_PredefinedValues)
			{
				// Get the human readable name from the profile object
				FName ProfileName = Profile->GetProfileSettingName(Setting.ProfileSetting.PropertyId);
				// Expose as a range if applicable
				EUIDataProviderFieldType FieldType = Type == PVMT_Ranged ? DATATYPE_RangeProperty : DATATYPE_Property;
				// Now add it to the list of properties
				new(OutFields) FUIDataProviderField(ProfileName,FieldType);
			}
		}
	}
}

/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<class IUIListElementProvider> UUIDataProvider_OnlineProfileSettings::ResolveListElementProvider(const FString& PropertyName)
{
	FName PropName(*PropertyName);
	// Search our cached providers for the matching property
	for (INT Index = 0; Index < ProfileSettingsArrayProviders.Num(); Index++)
	{
		FProfileSettingsArrayProvider& Provider = ProfileSettingsArrayProviders(Index);
		if (Provider.ProfileSettingsName == PropName)
		{
			return Provider.Provider;
		}
	}
	return TScriptInterface<class IUIListElementProvider>();
}

/**
 * Tells the provider the settings object it is resposible for exposing to
 * the UI
 *
 * @param InSettings the settings object to expose
 */
void UUIDataProvider_OnlineProfileSettings::BindProfileSettings(UOnlineProfileSettings* InSettings)
{
	Profile = InSettings;
	check(Profile);

	if ( bInitializeFromDefaults )
	{
		Profile->eventSetToDefaults();
	}

	Profile->eventModifyAvailableProfileSettings();

	// Create all of the providers needed for each string settings
	for (INT Index = 0; Index < Profile->ProfileMappings.Num(); Index++)
	{
		const FSettingsPropertyPropertyMetaData& ProfMapping = Profile->ProfileMappings(Index);
		FProfileSettingsArrayProvider Mapping;
		Mapping.ProfileSettingsId = ProfMapping.Id;
		// Ignore raw properties as they are handled differently
		if (ProfMapping.MappingType == PVMT_IdMapped || ProfMapping.MappingType == PVMT_PredefinedValues)
		{
			// Create the object that will manage the exposing of the data
			Mapping.Provider = ConstructObject<UUIDataProvider_OnlineProfileSettingsArray>(UUIDataProvider_OnlineProfileSettingsArray::StaticClass());
			// Let it cache whatever data it needs
			if ((ProfMapping.MappingType == PVMT_IdMapped && Mapping.Provider->BindStringSetting(Profile,ProfMapping.Id)) ||
				(ProfMapping.MappingType == PVMT_PredefinedValues && Mapping.Provider->BindPropertySetting(Profile,ProfMapping.Id)))
			{
				// Cache the name for later compares
				Mapping.ProfileSettingsName = ProfMapping.Name;
				if (Mapping.ProfileSettingsName != NAME_None)
				{
					// Everything worked, so add to our cache
					ProfileSettingsArrayProviders.AddItem(Mapping);
				}
			}
		}
	}
}

/* ==========================================================================================================
	UUIDataProvider_OnlineProfileSettingsArray
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_OnlineProfileSettingsArray);

/**
 * Binds the new profile object and id to this provider.
 *
 * @param NewProfile the new object to bind
 * @param NewProfileSettingsId the id of the settings array to expose
 *
 * @return TRUE if the call worked, FALSE otherwise
 */
UBOOL UUIDataProvider_OnlineProfileSettingsArray::BindStringSetting(UOnlineProfileSettings* NewProfile,INT NewProfileSettingsId)
{
	// Copy the values
	ProfileSettings = NewProfile;
	ProfileSettingId = NewProfileSettingsId;
	// And figure out the name of this setting for perf reasons
	ProfileSettingsName = ProfileSettings->GetProfileSettingName(ProfileSettingId);
	ColumnHeaderText = ProfileSettings->GetProfileSettingColumnHeader(ProfileSettingId);
	// Ditto for the various values
	ProfileSettings->GetProfileSettingValues(ProfileSettingId,Values);
	return ProfileSettingsName != NAME_None;
}

/**
 * Binds the property id as an array item. Requires that the property
 * has a mapping type of PVMT_PredefinedValues
 *
 * @param NewSettings the new object to bind
 * @param PropertyId the id of the property to expose as an array
 *
 * @return TRUE if the call worked, FALSE otherwise
 */
UBOOL UUIDataProvider_OnlineProfileSettingsArray::BindPropertySetting(UOnlineProfileSettings* NewProfile,INT PropertyId)
{
	// Copy the values
	ProfileSettings = NewProfile;
	ProfileSettingId = PropertyId;
	FSettingsPropertyPropertyMetaData* PropMeta = NewProfile->FindProfileSettingMetaData(PropertyId);
	if (PropMeta)
	{
		ProfileSettingsName = PropMeta->Name;
		ColumnHeaderText = PropMeta->ColumnHeaderText;
		// Iterate through the possible values adding them to our array of choices
		for (INT Index = 0; Index < PropMeta->PredefinedValues.Num(); Index++)
		{
			const FString& StrVal = PropMeta->PredefinedValues(Index).ToString();
			Values.AddItem(FName(*StrVal));
		}
	}
	return ProfileSettingsName != NAME_None;
}

/**
 * Determines if the specified name matches ours
 *
 * @param Property the name to compare with our own
 *
 * @return TRUE if the name matches, FALSE otherwise
 */
UBOOL UUIDataProvider_OnlineProfileSettingsArray::IsMatch(const TCHAR* Property)
{
	// Make a copy because we potentially modify the string
	FString CompareName(Property);
	FString ProviderName;
	// If there is an intervening provider name, snip it off
	if (ParseNextDataTag(CompareName,ProviderName) == FALSE)
	{
		CompareName = ProviderName;
	}
	// Now compare the final results
	return FName(*CompareName) == ProfileSettingsName;
}

/**
 * Gets the current value for the field specified
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataProvider_OnlineProfileSettingsArray::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	UBOOL bResult = FALSE;
	// If this is what we are bound to
	if (IsMatch(*FieldName))
	{
		FName ValueName = ProfileSettings->GetProfileSettingValueName(ProfileSettingId);
		// If the value has proper metadata, expose it
		if (ValueName != NAME_None)
		{
			OutFieldValue.PropertyTag = ProfileSettingsName;
			OutFieldValue.PropertyType = DATATYPE_Collection;
			OutFieldValue.StringValue = ValueName.ToString();

			// also the ArrayValue for Collection fields, as this is what UILists need to know
			INT ValueIndex = Values.FindItemIndex(ValueName);
			if ( ValueIndex != INDEX_NONE )
			{
				OutFieldValue.ArrayValue.AddUniqueItem(ValueIndex);
			}
			bResult = TRUE;
		}
	}
	return bResult || Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
}

/**
 * Sets the value for the specified field
 *
 * @param	FieldName		the data field to set the value of
 * @param	FieldValue		the value to store for the property specified
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataProvider_OnlineProfileSettingsArray::SetFieldValue(const FString& FieldName,
	const FUIProviderScriptFieldValue& FieldValue,INT ArrayIndex)
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	UBOOL bResult = FALSE;
	// If this is what we are bound to
	if (IsMatch(*FieldName))
	{
		FString StringValue = FieldValue.StringValue;

		if ( (FieldValue.PropertyType == DATATYPE_Collection
		||	StringValue.Len() == 0) && FieldValue.ArrayValue.Num() > 0 )
		{
			// UILists always save the selected item in the ArrayValue member, so if this field is a collection
			// and it has elements in the ArrayValue, use that one instead since StringValue might not be correct.
			StringValue = *Values(FieldValue.ArrayValue(0)).ToString();
		}

		// Set the value index by string
		bResult = ProfileSettings->SetProfileSettingValueByName(ProfileSettingsName,
			StringValue);
	}
	return bResult || Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);
}

/**
 * Builds a list of available fields from the array of properties in the
 * game settings object
 *
 * @param OutFields	out value that receives the list of exposed properties
 */
void UUIDataProvider_OnlineProfileSettingsArray::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	// Add ourselves as a collection
	new(OutFields) FUIDataProviderField(ProfileSettingsName,DATATYPE_Collection);
}

/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<IUIListElementProvider> UUIDataProvider_OnlineProfileSettingsArray::ResolveListElementProvider(const FString& PropertyName)
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	if (IsMatch(*PropertyName))
	{
		return TScriptInterface<IUIListElementProvider>(this);
	}
	return TScriptInterface<IUIListElementProvider>();
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUIDataProvider_OnlineProfileSettingsArray::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	OutCellTags.Set(ProfileSettingsName,*ColumnHeaderText);
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	OutCellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUIDataProvider_OnlineProfileSettingsArray::GetCellFieldType(FName FieldName, const FName& CellTag,BYTE& OutCellFieldType)
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	if (IsMatch(*CellTag.ToString()))
	{
		OutCellFieldType = DATATYPE_Property;
		return TRUE;
	}
	return FALSE;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the index of the value to fetch
 * @param	OutFieldValue	receives the resolved value for the property specified.
 * @param	ArrayIndex		ignored
 */
UBOOL UUIDataProvider_OnlineProfileSettingsArray::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT)
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	if (IsMatch(*CellTag.ToString()))
	{
		if (Values.IsValidIndex(ListIndex))
		{
			// Set the value to the value cached in the values array
			OutFieldValue.StringValue = Values(ListIndex).ToString();
			OutFieldValue.PropertyTag = ProfileSettingsName;
			OutFieldValue.PropertyType = DATATYPE_Property;
			return TRUE;
		}
	}
	return FALSE;
}

// IUIListElementProvider interface

/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUIDataProvider_OnlineProfileSettingsArray::GetElementProviderTags(void)
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	TArray<FName> Tags;
	Tags.AddItem(ProfileSettingsName);
	return Tags;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	DataTag		a tag corresponding to tag of a data provider in this list element provider
 *						that can be represented by a list
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUIDataProvider_OnlineProfileSettingsArray::GetElementCount(FName DataTag)
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	if (IsMatch(*DataTag.ToString()))
	{
		return Values.Num();
	}
	return 0;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	OutElements		will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUIDataProvider_OnlineProfileSettingsArray::GetListElements(FName FieldName,TArray<INT>& OutElements)
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	if (IsMatch(*FieldName.ToString()))
	{
		// Loop through the values adding the index
		for (INT Index = 0; Index < Values.Num(); Index++)
		{
			OutElements.AddItem(Index);
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	DataTag			the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UUIDataProvider_OnlineProfileSettingsArray::GetElementCellSchemaProvider(FName FieldName)
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	// If this is our field, return this object
	if (IsMatch(*FieldName.ToString()))
	{
		return TScriptInterface<IUIListElementCellProvider>(this);
	}
	return TScriptInterface<IUIListElementCellProvider>();
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UUIDataProvider_OnlineProfileSettingsArray::GetElementCellValueProvider(FName FieldName,INT ListIndex)
{
	check(ProfileSettings && ProfileSettingsName != NAME_None);
	// If this is our field, return this object
	if (IsMatch(*FieldName.ToString()))
	{
		return TScriptInterface<IUIListElementCellProvider>(this);
	}
	return TScriptInterface<IUIListElementCellProvider>();
}

/* ==========================================================================================================
	UUIDataProvider_OnlinePlayers
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_OnlinePlayers);

//@todo joeg -- Fill in the real natives once supported by the online subsystem

/* ==========================================================================================================
	UUIDataProvider_OnlineClanMates
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataProvider_OnlineClanMates);

//@todo joeg -- Fill in the real natives once supported by the online subsystem

/* ==========================================================================================================
	UUIDataStore_OnlineGameSearch
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataStore_OnlineGameSearch);

/**
 * Loads and creates an instance of the registered provider object for the
 * registered OnlineGameSearch class
 */
void UUIDataStore_OnlineGameSearch::InitializeDataStore(void)
{
	// Iterate through the registered searches creating their data
	for (INT Index = 0; Index < GameSearchCfgList.Num(); Index++)
	{
		FGameSearchCfg& Cfg = GameSearchCfgList(Index);
		// Create an instance of a the registered OnlineGameSearch class
		Cfg.Search = ConstructObject<UOnlineGameSearch>(Cfg.GameSearchClass);
		if (Cfg.Search != NULL)
		{
			// Create an instance of the data provider that will handle it
			Cfg.DesiredSettingsProvider = ConstructObject<UUIDataProvider_Settings>(UUIDataProvider_Settings::StaticClass());
			if (Cfg.DesiredSettingsProvider != NULL)
			{
				// Wire the search object to the provider
				if (Cfg.DesiredSettingsProvider->BindSettings(Cfg.Search) == TRUE)
				{
#if !CONSOLE
					// Create one search result in the editor
					if (GIsEditor && !GIsGame && Cfg.Search->Results.Num() == 0 && Cfg.DefaultGameSettingsClass)
					{
						ActiveSearchIndex = 0;
						Cfg.Search->Results.AddZeroed();
						// This will allow the UI editor to display the list of columns in the results
						Cfg.Search->Results(0).GameSettings = ConstructObject<UOnlineGameSettings>(Cfg.DefaultGameSettingsClass);
					}
#endif
					// Bind any search results that may have been present
					BuildSearchResults();
				}
				else
				{
					debugf(NAME_Error,TEXT("Failed to bind online game search %s to UI provider"),
						*Cfg.GameSearchClass->GetName());
				}
			}
			else
			{
				debugf(NAME_Error,TEXT("Failed to create provider context instance for %s"),
					*Cfg.GameSearchClass->GetName());
			}
		}
		else
		{
			debugf(NAME_Error,TEXT("Failed to create instance of class %s"),
				*Cfg.GameSearchClass->GetName());
		}
	}
	// Do script initialization
	eventInit();
}

/**
 * Builds a list of available fields from the array of properties in the
 * game settings object
 *
 * @param OutFields	out value that receives the list of exposed properties
 */
void UUIDataStore_OnlineGameSearch::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	// Data stores always empty and providers always append
	OutFields.Empty();

// 	new(OutFields) FUIDataProviderField(TEXT("SelectedIndex"),			DATATYPE_Property);
// 	new(OutFields) FUIDataProviderField(TEXT("CurrentGameSettingsTag"),	DATATYPE_Property);

	FGameSearchCfg& Cfg = GameSearchCfgList(SelectedIndex);
	// Add the two types of providers
	new(OutFields) FUIDataProviderField(FName(TEXT("DesiredSettings")),DATATYPE_Provider,
		Cfg.DesiredSettingsProvider);

	new(OutFields) FUIDataProviderField(SearchResultsName,DATATYPE_Collection);
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_OnlineGameSearch::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	UBOOL bResult = FALSE;

	if ( GameSearchCfgList.IsValidIndex(SelectedIndex) )
	{
		FGameSearchCfg& Cfg = GameSearchCfgList(SelectedIndex);
		if (Cfg.DesiredSettingsProvider)
		{
			bResult = Cfg.DesiredSettingsProvider->GetFieldValue(FieldName,OutFieldValue,ArrayIndex);
		}
	}

	return bResult || Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_OnlineGameSearch::SetFieldValue(const FString& FieldName,const FUIProviderScriptFieldValue& FieldValue,INT ArrayIndex)
{
	UBOOL bResult = FALSE;
	if ( GameSearchCfgList.IsValidIndex(SelectedIndex) )
	{
		FGameSearchCfg& Cfg = GameSearchCfgList(SelectedIndex);
		if (Cfg.DesiredSettingsProvider)
		{
			bResult = Cfg.DesiredSettingsProvider->SetFieldValue(FieldName,FieldValue,ArrayIndex);
		}
	}

	return bResult || Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);
}

/**
 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
 *
 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
 *
 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
 *			there is no list element provider associated with the specified property.
 */
TScriptInterface<IUIListElementProvider> UUIDataStore_OnlineGameSearch::ResolveListElementProvider(const FString& PropertyName)
{
	if (FName(*PropertyName) == SearchResultsName)
	{
		return TScriptInterface<IUIListElementProvider>(this);
	}
	FGameSearchCfg& Cfg = GameSearchCfgList(SelectedIndex);
	// See if the desired settings exposed something as a list
	return Cfg.DesiredSettingsProvider->ResolveListElementProvider(PropertyName);
}

// IUIListElement interface

/**
 * Returns the names of the exposed members in the first entry in the array
 * of search results
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUIDataStore_OnlineGameSearch::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	FGameSearchCfg& Cfg = GameSearchCfgList(SelectedIndex);
	if (Cfg.SearchResults.Num() > 0)
	{
		Cfg.SearchResults(0)->GetElementCellTags(UCONST_UnknownCellDataFieldName, OutCellTags);
	}
	else if ( Cfg.SearchResultsProviderClass != NULL )
	{
		// we don't have any search results, but we still need to provide a schema for the list, use the CDO for the SearchResultsProviderClass
		// since the we're looking for isn't specific to an instance anyway.
		UUIDataProvider_Settings* ProviderCDO = Cfg.SearchResultsProviderClass->GetDefaultObject<UUIDataProvider_Settings>();
		UOnlineGameSettings* SettingsCDO = Cfg.DefaultGameSettingsClass->GetDefaultObject<UOnlineGameSettings>();

		// in order to provide cell tags, the provider must have a valid DataSource...since this is the CDO that's not going to be
		// the case.  So we temporarily provide the right stuff...
		ProviderCDO->DataClass = Cfg.DefaultGameSettingsClass;
		ProviderCDO->DataSource = SettingsCDO;
		ProviderCDO->Settings = SettingsCDO;
		ProviderCDO->bIsAListRow = TRUE;
		ProviderCDO->GetElementCellTags(UCONST_UnknownCellDataFieldName, OutCellTags);
		ProviderCDO->bIsAListRow = FALSE;
		ProviderCDO->Settings = NULL;
		ProviderCDO->DataSource = NULL;
		ProviderCDO->DataClass = NULL;
	}
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag				the tag for the element cell to get the field type for
 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
 *
 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
 */
UBOOL UUIDataStore_OnlineGameSearch::GetCellFieldType(FName FieldName, const FName& CellTag,BYTE& out_CellFieldType)
{
	//@fixme joeg - implement this
	out_CellFieldType = DATATYPE_Property;
	return TRUE;
}

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UUIDataStore_OnlineGameSearch::GetCellFieldValue(FName FieldName, const FName& CellTag,INT ListIndex,
	FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	FGameSearchCfg& Cfg = GameSearchCfgList(SelectedIndex);
	if (Cfg.SearchResults.IsValidIndex(ListIndex))
	{
		// Ask the provider to fill in the data here
		return Cfg.SearchResults(ListIndex)->GetFieldValue(*CellTag.ToString(),OutFieldValue,ArrayIndex);
	}
	return FALSE;
}

// IUIListElementProvider interface

/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUIDataStore_OnlineGameSearch::GetElementProviderTags(void)
{
	TArray<FName> Tags;
	Tags.AddItem(SearchResultsName);
	return Tags;
}

/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	DataTag		a tag corresponding to tag of a data provider in this list element provider
 *						that can be represented by a list
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUIDataStore_OnlineGameSearch::GetElementCount(FName DataTag)
{
	if (DataTag == SearchResultsName)
	{
		FGameSearchCfg& Cfg = GameSearchCfgList(SelectedIndex);
		return Cfg.SearchResults.Num();
	}
	return 0;
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	OutElements		will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUIDataStore_OnlineGameSearch::GetListElements(FName FieldName,TArray<INT>& OutElements)
{
	if (FieldName == SearchResultsName)
	{
		FGameSearchCfg& Cfg = GameSearchCfgList(SelectedIndex);
		// Add an entry for each search result
		for (INT Index = 0; Index < Cfg.SearchResults.Num(); Index++)
		{
			OutElements.AddItem(Index);
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	DataTag			the tag of the list element data provider that we want the schema for.
 *
 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<IUIListElementCellProvider> UUIDataStore_OnlineGameSearch::GetElementCellSchemaProvider(FName FieldName)
{
	if (FieldName == SearchResultsName)
	{
		return TScriptInterface<IUIListElementCellProvider>(this);
	}
	return TScriptInterface<IUIListElementCellProvider>();
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<IUIListElementCellProvider> UUIDataStore_OnlineGameSearch::GetElementCellValueProvider(FName FieldName,INT ListIndex)
{
	if (FieldName == SearchResultsName)
	{
		return TScriptInterface<IUIListElementCellProvider>(this);
	}
	return TScriptInterface<IUIListElementCellProvider>();
}

/**
 * Creates all of the providers to display the search results
 */
void UUIDataStore_OnlineGameSearch::BuildSearchResults(void)
{
	if ( ActiveSearchIndex != INDEX_NONE )
	{
		FGameSearchCfg& Cfg = GameSearchCfgList(ActiveSearchIndex);
		check(Cfg.SearchResultsProviderClass != NULL);
		Cfg.SearchResults.Empty(Cfg.Search->Results.Num());
		// Create a provider for every item in the results array
		for (INT Index = 0; Index < Cfg.Search->Results.Num(); Index++)
		{
			UUIDataProvider_Settings* Provider = ConstructObject<UUIDataProvider_Settings>(Cfg.SearchResultsProviderClass);
			// Try to bind the dynamic data fields to the search result
			if (Provider->BindSettings(Cfg.Search->Results(Index).GameSettings,TRUE))
			{
				Cfg.SearchResults.AddItem(Provider);
			}
		}
	}
}

/* ==========================================================================================================
	UUIDataStore_OnlineStats
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataStore_OnlineStats);

// UIDataStore interface

/**
 * Loads and creates an instance of the registered stats read object
 */
void UUIDataStore_OnlineStats::InitializeDataStore(void)
{
	// Create each of the stats classes that are registered
	for (INT Index = 0; Index < StatsReadClasses.Num(); Index++)
	{
		UClass* Class = StatsReadClasses(Index);
		if (Class)
		{
			StatsRead = ConstructObject<UOnlineStatsRead>(Class);
			if (StatsRead != NULL)
			{
				StatsReadObjects.AddItem(StatsRead);
#if !CONSOLE
				// Create one search result in the editor
				if (GIsEditor && !GIsGame && StatsRead->Rows.Num() == 0)
				{
					StatsRead->Rows.AddZeroed();
					// Set up the list of columns that are expected back
					for (INT Index2 = 0; Index2 < StatsRead->ColumnIds.Num(); Index2++)
					{
						StatsRead->Rows(0).Columns.AddZeroed();
						StatsRead->Rows(0).Columns(Index2).ColumnNo = StatsRead->ColumnIds(Index2);
						StatsRead->Rows(0).Columns(Index2).StatValue.SetData((INT)100);
					}
				}
#endif
			}
			else
			{
				debugf(NAME_Error,TEXT("Failed to create instance of class %s"),*Class->GetName());
			}
		}
		else
		{
			//Placeholder for None entry
			StatsReadObjects.AddItem(NULL);
		}

	}
	// Now kick off the script initialization
	eventInit();
}

// IUIListElement interface

/**
 * Returns the names of the columns that can be bound to
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param OutCellTags the columns supported by this row
 */
void UUIDataStore_OnlineStats::GetElementCellTags( FName FieldName, TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Empty();
	// Add the constant ones (per row)
	OutCellTags.Set(RankNameMetaData.RankName, RankNameMetaData.RankColumnName.Len() > 0 ? *RankNameMetaData.RankColumnName : *RankNameMetaData.RankName.ToString());
	OutCellTags.Set(PlayerNickData.PlayerNickName, PlayerNickData.PlayerNickColumnName.Len() > 0 ? *PlayerNickData.PlayerNickColumnName : *PlayerNickData.PlayerNickName.ToString());

	// Now add the dynamically bound columns
	for (INT Index = 0; Index < StatsRead->ColumnMappings.Num(); Index++)
	{
		OutCellTags.Set(StatsRead->ColumnMappings(Index).Name,
			StatsRead->ColumnMappings(Index).ColumnName.Len() ? *StatsRead->ColumnMappings(Index).ColumnName : *StatsRead->ColumnMappings(Index).Name.ToString());
	}
}

/**
 * Retrieves the field type for the specified cell.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param CellTag the tag for the element cell to get the field type for
 * @param OutCellFieldType receives the field type for the specified cell (property)
 *
 * @return TRUE if the cell tag is valid, FALSE otherwise
 */
UBOOL UUIDataStore_OnlineStats::GetCellFieldType(FName FieldName, const FName& CellTag,BYTE& OutCellFieldType)
{
	OutCellFieldType = DATATYPE_Property;
	return TRUE;
}

/**
 * Finds the value for the specified column in a row (if valid)
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param CellTag the tag for the element cell to resolve the value for
 * @param ListIndex the index into the stats read results array
 * @param OutFieldValue the out value that holds the cell's value
 * @param ArrayIndex ignored
 *
 * @return TRUE if the cell value was found, FALSE otherwise
 */
UBOOL UUIDataStore_OnlineStats::GetCellFieldValue(FName FieldName, const FName& CellTag,
	INT ListIndex,FUIProviderFieldValue& OutFieldValue,INT)
{
	if (StatsRead->Rows.IsValidIndex(ListIndex))
	{
		// Get the row using the list index
		const FOnlineStatsRow& Row = StatsRead->Rows(ListIndex);
		// Check the non-dynamic properties first
		if (CellTag == RankNameMetaData.RankName)
		{
			OutFieldValue.StringValue = Row.Rank.ToString();
			OutFieldValue.PropertyTag = RankNameMetaData.RankName;
			return TRUE;
		}
		else if (CellTag == PlayerNickData.PlayerNickName)
		{
			OutFieldValue.StringValue = Row.NickName;
			OutFieldValue.PropertyTag = PlayerNickData.PlayerNickName;
			return TRUE;
		}
		else
		{
			// Search the column mappings and then find that column in the row
			for (INT Index = 0; Index < StatsRead->ColumnMappings.Num(); Index++)
			{
				// If this is our column
				if (StatsRead->ColumnMappings(Index).Name == CellTag)
				{
					INT ColId = StatsRead->ColumnMappings(Index).Id;
					// Find the column matching this
					for (INT Index2 = 0; Index2 < Row.Columns.Num(); Index2++)
					{
						if (Row.Columns(Index2).ColumnNo == ColId)
						{
							// Copy out the value
							OutFieldValue.StringValue = Row.Columns(Index2).StatValue.ToString();
							OutFieldValue.PropertyTag = CellTag;
							return TRUE;
						}
					}
				}
			}
		}
	}
	return FALSE;
}

/**
 * Returns the list of indices for the list items
 *
 * @param	FieldName		the name of the property to get the indices for
 * @param	OutElements		will be filled with the indices into the list
 *
 * @return	TRUE if the field name is valid, FALSE otherwise
 */
UBOOL UUIDataStore_OnlineStats::GetListElements(FName FieldName,TArray<INT>& OutElements)
{
	if (FieldName == StatsReadName)
	{
		// Just add an index for each row we have
		for (INT Index = 0; Index < StatsRead->Rows.Num(); Index++)
		{
			OutElements.AddItem(Index);
		}
		return TRUE;
	}
	return FALSE;
}

// IUIListElementProvider interface

/**
 * Fetches the column names that can be bound
 *
 * @return the list of tags supported
 */
TArray<FName> UUIDataStore_OnlineStats::GetElementProviderTags(void)
{
	TArray<FName> Tags;
	Tags.AddItem(StatsReadName);
	return Tags;
}

/** Sort class that compares stats rows by their rank */
struct FStatRowSorter
{
	static inline INT Compare(const FOnlineStatsRow& A,const FOnlineStatsRow& B)
	{
		INT AVal = 0;
		// If the rank is not an integer, it's not set
		if (A.Rank.Type == SDT_Int32)
		{
			A.Rank.GetData(AVal);
		}
		else
		{
			// Not set, so assume max
			AVal = MAXINT;
		}
		INT BVal = 0;
		// If the rank is not an integer, it's not set
		if (B.Rank.Type == SDT_Int32)
		{
			B.Rank.GetData(BVal);
		}
		else
		{
			// Not set, so assume max
			BVal = MAXINT;
		}
		return AVal - BVal;
	}
};

/**
 * Sorts the returned results by their rank (lowest to highest)
 */
void UUIDataStore_OnlineStats::SortResultsByRank(void)
{
	if (StatsRead != NULL && StatsRead->Rows.Num() > 0)
	{
		// Use the stats row comparator to sort the results by rank
		::Sort<FOnlineStatsRow,FStatRowSorter>(&StatsRead->Rows(0),StatsRead->Rows.Num());
	}
}

/* ==========================================================================================================
	UUIDataStore_Gamma
========================================================================================================== */

IMPLEMENT_CLASS(UUIDataStore_Gamma);

/**
 * Adds the gamma property
 *
 * @param OutFields	out value that receives the list of exposed properties
 */
void UUIDataStore_Gamma::GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields)
{
	// Data stores always empty and providers always append
	OutFields.Empty();
	new(OutFields) FUIDataProviderField(FName(TEXT("Gamma")));
}

/**
 * Returns the current gamma value as a string
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	OutFieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_Gamma::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	if (FName(*FieldName) == FName(TEXT("Gamma")))
	{
#if !PS3
		extern FLOAT GetDisplayGamma(void);
		//@todo joeg - change this to DATATYPE_Range once you've changed this to set the RangeValue, rather than the StringValue
		OutFieldValue.PropertyType = DATATYPE_Property;
		OutFieldValue.StringValue = FString::Printf(TEXT("%f"),GetDisplayGamma());
		return TRUE;
#endif
	}
	return Super::GetFieldValue(FieldName, OutFieldValue, ArrayIndex);
}

/**
 * Sets the gamma to the value specified
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_Gamma::SetFieldValue(const FString& FieldName,const FUIProviderScriptFieldValue& FieldValue,INT ArrayIndex)
{
	if (FName(*FieldName) == FName(TEXT("Gamma")))
	{
#if !PS3
		extern void SetDisplayGamma(FLOAT Gamma);
		FLOAT NewGamma = appAtof(*FieldValue.StringValue);
		SetDisplayGamma(NewGamma);
		return TRUE;
#endif
	}
	return Super::SetFieldValue(FieldName, FieldValue, ArrayIndex);
}

/**
 * Tells the D3D device to flush it's INI settings to disk to save the gamma value
 */
void UUIDataStore_Gamma::OnCommit(void)
{
#if !PS3
	extern void SaveDisplayGamma(void);
	SaveDisplayGamma();
#endif
}

/* ==========================================================================================================
	UUIDataStore_MenuItems
========================================================================================================== */
IMPLEMENT_CLASS(UUIDataStore_MenuItems);
/* === UUIDataStore_MenuItems interface === */
/**
 * Converts the specified name into the tag required to access the options for the current game settings object,
 * if the specified field is CurrentGameSettingsTag.  Otherwise, does nothing.
 *
 * @param	FieldName	the name of the field that was passed to this data store.
 *
 * @return	a tag which can be used to access the settings collection for the current game settings object, or the same
 *			value as the input value.
 */
FName UUIDataStore_MenuItems::ResolveFieldName( FName FieldName ) const
{
	if ( FieldName == CurrentGameSettingsTag )
	{
		UDataStoreClient* DSClient = UUIInteraction::GetDataStoreClient();
		if ( DSClient != NULL )
		{
			UUIDataStore_OnlineGameSettings* GameSettingsDataStore = Cast<UUIDataStore_OnlineGameSettings>(DSClient->FindDataStore(TEXT("OnlineGameSettings")));
			if ( GameSettingsDataStore != NULL && GameSettingsDataStore->GameSettingsCfgList.IsValidIndex(GameSettingsDataStore->SelectedIndex) )
			{
				FGameSettingsCfg& GameConfiguration = GameSettingsDataStore->GameSettingsCfgList(GameSettingsDataStore->SelectedIndex);
				FieldName = GameConfiguration.SettingsName;
			}
		}
	}

	return FieldName;
}
FString UUIDataStore_MenuItems::ResolveFieldString( const FString& FieldString ) const
{
	if ( CurrentGameSettingsTag == *FieldString )
	{
		UDataStoreClient* DSClient = UUIInteraction::GetDataStoreClient();
		if ( DSClient != NULL )
		{
			UUIDataStore_OnlineGameSettings* GameSettingsDataStore = Cast<UUIDataStore_OnlineGameSettings>(DSClient->FindDataStore(TEXT("OnlineGameSettings")));
			if ( GameSettingsDataStore != NULL && GameSettingsDataStore->GameSettingsCfgList.IsValidIndex(GameSettingsDataStore->SelectedIndex) )
			{
				FGameSettingsCfg& GameConfiguration = GameSettingsDataStore->GameSettingsCfgList(GameSettingsDataStore->SelectedIndex);
				return GameConfiguration.SettingsName.ToString();
			}
		}
	}

	return FieldString;
}


/**
* Called when this data store is added to the data store manager's list of active data stores.
*
* Initializes the ListElementProviders map
*
* @param	PlayerOwner		the player that will be associated with this DataStore.  Only relevant if this data store is
*							associated with a particular player; NULL if this is a global data store.
*/
void UUIDataStore_MenuItems::OnRegister( ULocalPlayer* PlayerOwner )
{
	Super::OnRegister(PlayerOwner);

	// Initialize all of the option providers, go backwards because of the way MultiMap appends items.
	TArray<UUIResourceDataProvider*> Providers;
	ListElementProviders.MultiFind(TEXT("OptionCategory"), Providers);

	for ( INT ProviderIndex = Providers.Num()-1; ProviderIndex >= 0; ProviderIndex-- )
	{
		UUIDataProvider_MenuItem* DataProvider = Cast<UUIDataProvider_MenuItem>(Providers(ProviderIndex));
		if(DataProvider)
		{
			for (INT OptionIndex=0;OptionIndex<DataProvider->OptionSet.Num();OptionIndex++)
			{
				OptionProviders.Add(DataProvider->OptionSet(OptionIndex), DataProvider);
			}
		}
	}
}


/** 
 * Gets the list of element providers for a fieldname with filtered elements removed.
 *
 * @param FieldName				Fieldname to use to search for providers.
 * @param OutElementProviders	Array to store providers in.
 */
void UUIDataStore_MenuItems::GetFilteredElementProviders(FName FieldName, TArray<UUIDataProvider_MenuItem*> &OutElementProviders)
{
	OutElementProviders.Empty();

	FieldName = ResolveFieldName(FieldName);

	TArray<UUIDataProvider_MenuItem*> Providers;
	OptionProviders.MultiFind(FieldName, Providers);

	for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
	{
		UUIDataProvider_MenuItem* DataProvider = Providers(ProviderIndex);
		if(DataProvider && DataProvider->IsFiltered() == FALSE)
		{
			OutElementProviders.AddUniqueItem(DataProvider);
		}
	}
}

/* === IUIListElementProvider interface === */
/**
 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
 *
 * @return	the list of tags supported by this element provider which correspond to list element data.
 */
TArray<FName> UUIDataStore_MenuItems::GetElementProviderTags()
{
	TLookupMap<FName> Keys;
	TArray<FName> OptionSets;
	OptionProviders.GetKeys(Keys);

	OptionSets.AddItem(CurrentGameSettingsTag);
	for(INT OptionIdx=0; OptionIdx<Keys.Num(); OptionIdx++)
	{
		OptionSets.AddItem(Keys(OptionIdx));
	}

	return OptionSets;
}


/**
 * Returns the number of list elements associated with the data tag specified.
 *
 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
 *						from GetElementProviderTags.
 *
 * @return	the total number of elements that are required to fully represent the data specified.
 */
INT UUIDataStore_MenuItems::GetElementCount( FName FieldName )
{
	TArray<UUIDataProvider_MenuItem*> Providers;
	GetFilteredElementProviders(FieldName, Providers);

	return Providers.Num();
}

/**
 * Retrieves the list elements associated with the data tag specified.
 *
 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
 *							from GetElementProviderTags.
 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
 *
 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
 */
UBOOL UUIDataStore_MenuItems::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	FieldName = ResolveFieldName(FieldName);

	TArray<UUIDataProvider_MenuItem*> Providers;
	OptionProviders.MultiFind(FieldName, Providers);

	if ( Providers.Num() > 0 )
	{
		out_Elements.Empty();
		for ( INT ProviderIndex = 0; ProviderIndex < Providers.Num(); ProviderIndex++ )
		{
			UUIDataProvider_MenuItem* DataProvider = Providers(ProviderIndex);
			if(DataProvider && DataProvider->IsFiltered() == FALSE)
			{
				out_Elements.AddUniqueItem(ProviderIndex);
			}
		}
		bResult = TRUE;
	}

	return bResult;
}

/* === UIDataProvider interface === */
/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 *
 * @todo - not yet implemented
 */
UBOOL UUIDataStore_MenuItems::GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	return Super::GetFieldValue(ResolveFieldString(FieldName), out_FieldValue, ArrayIndex);
}

/**
 * Gets the list of data fields exposed by this data provider.
 *
 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
 */
void UUIDataStore_MenuItems::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	out_Fields.Empty();

	new(out_Fields) FUIDataProviderField(CurrentGameSettingsTag, DATATYPE_Collection);

	TLookupMap<FName> OptionSets;
	OptionProviders.GetKeys(OptionSets);
	for ( INT ProviderIndex = 0; ProviderIndex < OptionSets.Num(); ProviderIndex++ )
	{
		FName ProviderTag = *OptionSets(ProviderIndex);

		TArray<UUIDataProvider_MenuItem*> ResourceProviders;
		OptionProviders.MultiFind(ProviderTag, ResourceProviders);

		// for each of the game resource providers, add a tag to allow the UI to access the list of providers
		new(out_Fields) FUIDataProviderField( ProviderTag, (TArray<UUIDataProvider*>&)ResourceProviders );
	}
}

/**
 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
 * Used by the UI editor to know which cells are available for binding to individual list cells.
 *
 * @param	FieldName		the tag of the list element data provider that we want the schema for.
 *

 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
 */
TScriptInterface<class IUIListElementCellProvider> UUIDataStore_MenuItems::GetElementCellSchemaProvider( FName FieldName )
{
	TScriptInterface<IUIListElementCellProvider> Result;

	//@todo ronp - this won't work if we add more types to the ElementProviderTypes array [besides UIDataProvider_MenuItem]
	// search for the provider that has the matching tag
	INT ProviderIndex = FindProviderTypeIndex(TEXT("OptionCategory"));
	if ( ProviderIndex != INDEX_NONE )
	{
		FGameResourceDataProvider& Provider = ElementProviderTypes(ProviderIndex);
		if ( Provider.ProviderClass != NULL )
		{
			Result = Provider.ProviderClass->GetDefaultObject<UUIResourceDataProvider>();
		}
	}

	return Result;
}

/**
 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
 * of the list element indicated by CellValueProvider.DataSourceIndex
 *
 * @param	FieldName		the tag of the list element data field that we want the values for
 * @param	ListIndex		the list index for the element to get values for
 *
 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
 */
TScriptInterface<class IUIListElementCellProvider> UUIDataStore_MenuItems::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	TScriptInterface<class IUIListElementCellProvider> Result;
	
	FieldName = ResolveFieldName(FieldName);

	// search for the provider that has the matching tag
	TArray<UUIDataProvider_MenuItem*> Providers;
	OptionProviders.MultiFind(FieldName, Providers);

	if(Providers.IsValidIndex(ListIndex))
	{
		Result = Providers(ListIndex);
	}

	return Result;
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 */
UBOOL UUIDataStore_MenuItems::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex )
{
	return Super::SetFieldValue(ResolveFieldString(FieldName), FieldValue, ArrayIndex);
}

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
UBOOL UUIDataStore_MenuItems::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	return TRUE;
}

/** 
 * Clears all options in the specified set.
 *
 * @param SetName		Set to clear
 */
void UUIDataStore_MenuItems::ClearSet(FName SetName)
{
	SetName = ResolveFieldName(SetName);

	TArray<UUIDataProvider_MenuItem*> Providers;
	OptionProviders.MultiFind(SetName, Providers);

	for(INT ProviderIdx=0; ProviderIdx<Providers.Num(); ProviderIdx++)
	{
		DynamicProviders.RemoveItem(Providers(ProviderIdx));
	}

	// Remove the key
	OptionProviders.RemoveKey(SetName);
}

/** 
 * Appends N amount of providers to the specified set.
 *
 * @param SetName		Set to append to
 * @param NumOptions	Number of options to append
 */
void UUIDataStore_MenuItems::AppendToSet(FName SetName, INT NumOptions)
{
	SetName = ResolveFieldName(SetName);

	for(INT AddIdx=0; AddIdx<NumOptions; AddIdx++)
	{
		UUIDataProvider_MenuItem* NewProvider = ConstructObject<UUIDataProvider_MenuItem>(UUIDataProvider_MenuItem::StaticClass(), this);
		OptionProviders.Add(SetName, NewProvider);
		DynamicProviders.AddUniqueItem(NewProvider);
	}
}

/**
 * Retrieves a set of option providers.
 *
 * @param SetName	Set to retrieve
 * 
 * @return Array of dataproviders for all the options in the set.
 */
void UUIDataStore_MenuItems::GetSet(FName SetName, TArray<UUIDataProvider_MenuItem*> &Providers)
{
	SetName = ResolveFieldName(SetName);

	Providers.Empty();
	OptionProviders.MultiFind(SetName, Providers);
}


/* ==========================================================================================================
	UUIDataProvider_MenuItem
========================================================================================================== */
IMPLEMENT_CLASS(UUIDataProvider_MenuItem);

/** @return 	TRUE if this menu option's configuration isn't compatible with the desired game settings  */
UBOOL UUIDataProvider_MenuItem::IsFiltered()
{
#if !CONSOLE
	UBOOL bFiltered = bRemoveOnPC;
#elif XBOX
	UBOOL bFiltered = bRemoveOn360;
#elif PS3
	UBOOL bFiltered = bRemoveOnPS3;
#else
#error Please define your platform.
#endif

	if ( !bFiltered )
	{
		UDataStoreClient* DataStoreClient = UUIInteraction::GetDataStoreClient();
		if (DataStoreClient != NULL)
		{
			//@todo ronp - we don't use this in Gears
			UUIDataStore_Registry* Registry = Cast<UUIDataStore_Registry>(DataStoreClient->FindDataStore(TEXT("Registry")));
			if(Registry)
			{
				FUIProviderFieldValue OutFieldValue(EC_EventParm);

				if ( Registry->GetDataStoreValue(TEXT("SelectedGameMode"), OutFieldValue ))
				{
					bFiltered = (RequiredGameMode != NAME_None && RequiredGameMode != *OutFieldValue.StringValue);
				}

				if ( !bFiltered && Registry->GetDataStoreValue(TEXT("StandaloneGame"), OutFieldValue) )
				{
								// bOnlineOnly option
					bFiltered = (bOnlineOnly && OutFieldValue.StringValue == TEXT("1"))
								// bOfflineOnly option
							||	(bOfflineOnly && OutFieldValue.StringValue == TEXT("0"));
				}
			}
		}

#if PS3
		// If this menu option corresponds to something that is only relevant for keyboard/mouse (i.e. mouse sensitivity, etc.),
		// filter it out if we don't have a keyboard or mouse
		//@todo ronp - this needs to be per-player
		if ( bKeyboardOrMouseOption )
		{
			if(GEngine && GEngine->GamePlayers.Num() && GEngine->GamePlayers(0))
			{
				APlayerController* PC = GEngine->GamePlayers(0)->Actor;
				if(PC != NULL)
				{
					if ( !PC->IsKeyboardAvailable() && !PC->IsMouseAvailable() )
					{
						bFiltered=TRUE;
					}
				}
			}
		}
#endif
	}

	return bFiltered;
}






