/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EnginePrivate.h"

#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"

IMPLEMENT_CLASS(UOnlinePlaylistManager);

/**
 * Uses the configuration data to create the requested objects and then applies any
 * specific game settings changes to them
 */
void UOnlinePlaylistManager::FinalizePlaylistObjects(void)
{
	// Process the config entries creating and updating as specified
	for (INT PlaylistIndex = 0; PlaylistIndex < Playlists.Num(); PlaylistIndex++)
	{
		FPlaylist& Playlist = Playlists(PlaylistIndex);
		// Create each game setting object and update its data
		for (INT GameIndex = 0; GameIndex < Playlist.ConfiguredGames.Num(); GameIndex++)
		{
			FConfiguredGameSetting& ConfiguredGame = Playlist.ConfiguredGames(GameIndex);
			// If there is a valid class name specified try to load it and instance it
			if (ConfiguredGame.GameSettingsClassName.Len())
			{
				UClass* GameSettingsClass = LoadClass<UOnlineGameSettings>(NULL,
					*ConfiguredGame.GameSettingsClassName,
					NULL,
					LOAD_None,
					NULL);
				if (GameSettingsClass)
				{
					// Now create an instance with that class
					ConfiguredGame.GameSettings = ConstructObject<UOnlineGameSettings>(GameSettingsClass);
					if (ConfiguredGame.GameSettings)
					{
						// Update the game object with these settings, if not using the defaults
						if (ConfiguredGame.URL.Len())
						{
							ConfiguredGame.GameSettings->UpdateFromURL(ConfiguredGame.URL,NULL);
						}
					}
					else
					{
						debugf(NAME_DevOnline,
							TEXT("Failed to create class (%s) for playlist (%s)"),
							*ConfiguredGame.GameSettingsClassName,
							*Playlist.Name);
					}
				}
				else
				{
					debugf(NAME_DevOnline,
						TEXT("Failed to load class (%s) for playlist (%s)"),
						*ConfiguredGame.GameSettingsClassName,
						*Playlist.Name);
				}
			}
		}
	}
	if (DatastoresToRefresh.Num())
	{
		INT DatastoreIndex = INDEX_NONE;
		// Iterate through the registered set of datastores and refresh them
		for (TObjectIterator<UUIDataStore_GameResource> ObjIt; ObjIt; ++ObjIt)
		{
			DatastoresToRefresh.FindItem(ObjIt->Tag,DatastoreIndex);
			// Don't refresh it if it isn't in our list
			if (DatastoreIndex != INDEX_NONE)
			{
				(*ObjIt)->InitializeListElementProviders();
			}
		}
	}
}

/** Uses the current loc setting and game ini name to build the download list */
void UOnlinePlaylistManager::DetermineFilesToDownload(void)
{
	PlaylistFileNames.Empty(3);
	// Build the game specific playlist ini
	PlaylistFileNames.AddItem(FString::Printf(TEXT("%sPlaylist.ini"),appGetGameName()));
	FFilename GameIni(GGameIni);
	// Add the game ini for downloading per object config
	PlaylistFileNames.AddItem(GameIni.GetCleanFilename());
	// Now build the loc file name from the ini filename
	PlaylistFileNames.AddItem(FString::Printf(TEXT("Engine.%s"),*appGetLanguageExt()));
}
