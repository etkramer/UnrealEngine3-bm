
/*=============================================================================
UnPenLev.cpp: Unreal pending level class.
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"
#include "DemoRecording.h"

/*-----------------------------------------------------------------------------
UPendingLevel implementation.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
UPendingLevel::UPendingLevel( const FURL& InURL )
: ULevelBase( InURL ), NetDriver(NULL), DemoRecDriver(NULL)
, bSuccessfullyConnected(FALSE), bSentJoinRequest(FALSE), FilesNeeded(0)
{}
IMPLEMENT_CLASS(UPendingLevel);


void UPendingLevel::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);
	if( !Ar.IsLoading() && !Ar.IsSaving() )
	{
		Ar << NetDriver << DemoRecDriver;
	}
}

/*-----------------------------------------------------------------------------
UNetPendingLevel implementation.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
UNetPendingLevel::UNetPendingLevel( const FURL& InURL )
: UPendingLevel( InURL )
{
	if (!GDisallowNetworkTravel)
	{
		// Try to create network driver.
		UClass* NetDriverClass = StaticLoadClass( UNetDriver::StaticClass(), NULL, TEXT("engine-ini:Engine.Engine.NetworkDevice"), NULL, LOAD_None, NULL );
		NetDriver = ConstructObject<UNetDriver>( NetDriverClass );
		check(NetDriver);
		if( NetDriver->InitConnect( this, URL, ConnectionError ) )
		{
			// Send initial message.
			NetDriver->ServerConnection->Logf( TEXT("HELLO P=%i REVISION=0 MINVER=%i VER=%i"),
				(INT)appGetPlatformType(), GEngineMinNetVersion, GEngineVersion );
			NetDriver->ServerConnection->FlushNet();

			// cache the guid caches
			for (TObjectIterator<UGuidCache> It; It; ++It)
			{
				CachedGuidCaches.AddItem(*It);
			}
		}
		else
		{
			// error initializing the network stack...
			NetDriver = NULL;

			// ConnectionError should be set by calling InitConnect...however, if we set NetDriver to NULL without setting a
			// value for ConnectionError, we'll trigger the assertion at the top of UNetPendingLevel::Tick() so make sure it's set
			if ( ConnectionError.Len() == 0 )
			{
				ConnectionError = LocalizeError(TEXT("NetworkInit"), TEXT("Engine"));
			}
		}
	}
	else
	{
		ConnectionError = LocalizeError(TEXT("UsedCheatCommands"), TEXT("Engine"));
	}
}

//
// FNetworkNotify interface.
//
EAcceptConnection UNetPendingLevel::NotifyAcceptingConnection()
{
	return ACCEPTC_Reject;
}
void UNetPendingLevel::NotifyAcceptedConnection( class UNetConnection* Connection )
{
}
UBOOL UNetPendingLevel::NotifyAcceptingChannel( class UChannel* Channel )
{
	return 0;
}
UWorld* UNetPendingLevel::NotifyGetWorld()
{
	return NULL;
}
void UNetPendingLevel::NotifyReceivedText( UNetConnection* Connection, const TCHAR* Text )
{
	check(Connection==NetDriver->ServerConnection);
#if !SHIPPING_PC_GAME
	debugf( NAME_DevNet, TEXT("PendingLevel received: %s"), Text );
#endif

	// This client got a response from the server.
	if( ParseCommand(&Text,TEXT("UPGRADE")) )
	{
		// Report mismatch.
		INT RemoteMinVer=0, RemoteVer=0;
		Parse( Text, TEXT("MINVER="), RemoteMinVer );
		Parse( Text, TEXT("VER="),    RemoteVer    );
		if( GEngineVersion < RemoteMinVer )
		{
			ConnectionError = LocalizeError(TEXT("ClientOutdated"),TEXT("Engine"));
			// Upgrade message.
			GEngine->SetProgress( PMT_ConnectionFailure, LocalizeError(TEXT("ConnectionFailed_Title"), TEXT("Engine")), ConnectionError );
		}
		else
		{
			ConnectionError = LocalizeError(TEXT("ServerOutdated"),TEXT("Engine"));
			// Downgrade message.
			GEngine->SetProgress( PMT_ConnectionFailure, LocalizeError(TEXT("ConnectionFailed_Title"), TEXT("Engine")), ConnectionError );
		}
	}
	else if (ParseCommand(&Text, TEXT("USES")))
	{
		// Dependency information.
		FPackageInfo Info(NULL);
		Connection->ParsePackageInfo(Text, Info);

		// verify that we have this package (and Guid matches) 
		FString Filename;
		UBOOL bWasPackageFound = FALSE;

		// use guid caches for seekfree loading, since we may not ahve the original package on disk
		if (GUseSeekFreeLoading)
		{
			UBOOL bFoundCache = FALSE;
			// go over all guid caches
			for (INT CacheIndex = 0; CacheIndex < CachedGuidCaches.Num() && !bWasPackageFound; CacheIndex++)
			{
				UGuidCache* Cache = CachedGuidCaches(CacheIndex);

				// note that we found one
				bFoundCache = TRUE;

				FGuid Guid;
				// look for this package in it
				if (Cache->GetPackageGuid(Info.PackageName, Guid))
				{
					// does the Guid match?
					if (Guid == Info.Guid)
					{
						bWasPackageFound = TRUE;
					}
				}
			}

			if (!bFoundCache)
			{
				debugf(TEXT("  Failed to find Guid because there was no package cache"));
			}
		}

		// if the packge guid wasn't found using GuidCache above, then look for it on disk (for mods, etc, or the PC)
		if (!bWasPackageFound)
		{
			if (GPackageFileCache->FindPackageFile(*Info.PackageName.ToString(), &Info.Guid, Filename, NULL))
			{
				// if we are not doing seekfree loading, then open the package and tell the server we have it
				// (seekfree loading case will open the packages in UGameEngine::LoadMap)
				if (!GUseSeekFreeLoading)
				{
					Info.Parent = CreatePackage(NULL, *Info.PackageName.ToString());

					BeginLoad();
					ULinkerLoad* Linker = GetPackageLinker(Info.Parent, NULL, LOAD_NoWarn | LOAD_NoVerify | LOAD_Quiet, NULL, &Info.Guid);
					EndLoad();
					if (Linker)
					{
						Info.LocalGeneration = Linker->Summary.Generations.Num();
						// tell the server what we have
						NetDriver->ServerConnection->Logf(TEXT("HAVE GUID=%s GEN=%i"), *Linker->Summary.Guid.String(), Info.LocalGeneration);

						bWasPackageFound = TRUE;
					}
					else
					{
						// there must be some problem with the cache if this happens, as the package was found with the right Guid,
						// but then it failed to load, so log a message and mark that we didn't find it
						debugf(NAME_Warning, TEXT("Linker for package %s failed to load, even though it should have [from %s]"), *Info.PackageName.ToString(), *Filename);
					}
				}
				else
				{
					bWasPackageFound = TRUE;
				}
			}
		}

		if (!bWasPackageFound)
		{
			// we need to download this package
			FilesNeeded++;
			Info.PackageFlags |= PKG_Need;
			if (!NetDriver->AllowDownloads || !(Info.PackageFlags & PKG_AllowDownload))
			{
#if OLD_CONNECTION_FAILURE_CODE
 				ConnectionError = FString::Printf( TEXT("Downloading '%s' not allowed"), *Info.PackageName.ToString());
 				FailedConnectionType = FCT_VersionMismatch;
#else
				ConnectionError = FString::Printf(LocalizeSecure(LocalizeError(TEXT("NoDownload"), TEXT("Engine")), *Info.PackageName.ToString()));
#endif
				NetDriver->ServerConnection->Close();
				return;
			}
		}
		Connection->PackageMap->AddPackageInfo(Info);
	}
	else if (ParseCommand(&Text, TEXT("UNLOAD")))
	{
		// remove a package from the package map
		FGuid Guid;
		Parse(Text, TEXT("GUID="), Guid);
		Connection->PackageMap->RemovePackageByGuid(Guid);
	}
	else if( ParseCommand(&Text,TEXT("FAILURE")) )
	{
		// Report problem to user.
		// 1. We had a package with an incorrect GUID (UWorld::NotifyReceivedText, parsing HAVE)
		// 2. The challenge response we sent to the server didn't match the server's version (same, parsing LOGIN)
		// 3. eventPreLogin returned FALSE (same, same, just before WelcomePlayer is called)
		// 4. SpawnPlayActor failed (UWorld::NotifyReceivedText (same), parsing JOIN)
		// 5. SpawnPlayActor failed (same, parsing JOINSPLIT)

#if OLD_CONNECTION_FAILURE_CODE
#if !CONSOLE //@todo  matto: replace with new system
		GEngine->SetProgress( TEXT("Rejected By Server"), Text, 10.f );
#endif
#else
		while (*Text != 0 && appIsWhitespace(*Text))
		{
			Text++;
		}
		FString Error = Text;
		if (Error != TEXT(""))
		{
			// try to get localized error message
			TArray<FString> Pieces;
			Error.ParseIntoArray(&Pieces, TEXT("."), TRUE);
			if (Pieces.Num() >= 3)
			{
				Error = Localize(*Pieces(1), *Pieces(2), *Pieces(0), NULL, TRUE);
			}
		}
		ConnectionError = Error;
#endif

		// Force close the session
		Connection->Close();
	}
	else if( ParseCommand(&Text,TEXT("USERFLAG")) )
	{
		Connection->UserFlags = appAtoi(Text);
	}
	else if( ParseCommand( &Text, TEXT("CHALLENGE") ) )
	{
		// Challenged by server.
		Parse( Text, TEXT("VER="), Connection->NegotiatedVer );
		Parse( Text, TEXT("CHALLENGE="), Connection->Challenge );

		FURL PartialURL(URL);
		PartialURL.Host = TEXT("");
		for( INT i=URL.Op.Num()-1; i>=0; i-- )
		{
			if( URL.Op(i).Left(5)==TEXT("game=") )
			{
				URL.Op.Remove( i );
			}
		}
#if WITH_GAMESPY
		extern void appGetGameSpyChallengeResponse(UNetConnection*,UBOOL);
		// Calculate the response string based upon the user's CD Key
		appGetGameSpyChallengeResponse(Connection,FALSE);
#else
		Connection->ClientResponse = TEXT("0");
#endif
		NetDriver->ServerConnection->Logf( TEXT("NETSPEED %i"), Connection->CurrentNetSpeed );
		NetDriver->ServerConnection->Logf( TEXT("LOGIN RESPONSE=%s URL=%s"), *Connection->ClientResponse, *PartialURL.String() );
		NetDriver->ServerConnection->FlushNet();
	}
	else if( ParseCommand( &Text, TEXT("DLMGR") ) )
	{
		INT i = Connection->DownloadInfo.AddZeroed();
		Parse( Text, TEXT("CLASS="), Connection->DownloadInfo(i).ClassName );
		Parse( Text, TEXT("PARAMS="), Connection->DownloadInfo(i).Params );
		ParseUBOOL( Text, TEXT("COMPRESSION="), Connection->DownloadInfo(i).Compression );
		Connection->DownloadInfo(i).Class = StaticLoadClass( UDownload::StaticClass(), NULL, *Connection->DownloadInfo(i).ClassName, NULL, LOAD_NoWarn | LOAD_Quiet, NULL );
		if( !Connection->DownloadInfo(i).Class )
			Connection->DownloadInfo.Remove(i);
	}
	else if( ParseCommand( &Text, TEXT("WELCOME") ) )
	{
		// Server accepted connection.
		debugf( NAME_DevNet, TEXT("Welcomed by server: %s"), Text );

		// Parse welcome message.
		Parse( Text, TEXT("LEVEL="), URL.Map );
		Parse( Text, TEXT("CHALLENGE="), Connection->Challenge );

		// Send first download request.
		ReceiveNextFile( Connection );

		// We have successfully connected.
		bSuccessfullyConnected = TRUE;
	}
	else
	{
		// Other command.
		warnf( NAME_DevNet, TEXT("NotifyReceivedText: Received text that was not parsed correctly %s"), Text );
	}
}

UBOOL UNetPendingLevel::TrySkipFile()
{
	return NetDriver->ServerConnection->Download && NetDriver->ServerConnection->Download->TrySkipFile();
}
void UNetPendingLevel::NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const TCHAR* InError, UBOOL Skipped )
{
	check(Connection->PackageMap->List.IsValidIndex(PackageIndex));

	// Map pack to package.
	FPackageInfo& Info = Connection->PackageMap->List(PackageIndex);
	if( *InError )
	{
		if( Connection->DownloadInfo.Num() > 1 )
		{
			// Try with the next download method.
			Connection->DownloadInfo.Remove(0);
			ReceiveNextFile( Connection );
		}
		else
		{
			// If transfer failed, so propagate error.
			if( ConnectionError==TEXT("") )
			{
				ConnectionError = FString::Printf( LocalizeSecure(LocalizeError(TEXT("DownloadFailed"),TEXT("Engine")), *Info.PackageName.ToString(), InError) );
// 				FailedConnectionType = FCT_VersionMismatch;
			}
		}
	}
	else
	{
		// Now that a file has been successfully received, mark its package as downloaded.
		check(Connection==NetDriver->ServerConnection);
		check(Info.PackageFlags&PKG_Need);
		Info.PackageFlags &= ~PKG_Need;
		FilesNeeded--;
		if( Skipped )
		{
			Connection->PackageMap->List.Remove( PackageIndex );
		}
		else
		{
			if (!GUseSeekFreeLoading)
			{
				// load it, verify, then tell the server
				Info.Parent = CreatePackage(NULL, *Info.PackageName.ToString());
				BeginLoad();
				ULinkerLoad* Linker = GetPackageLinker(Info.Parent, NULL, LOAD_NoWarn | LOAD_NoVerify | LOAD_Quiet, NULL, &Info.Guid);
				EndLoad();
				if (Linker == NULL || Linker->Summary.Guid != Info.Guid)
				{
					debugf(NAME_DevNet, TEXT("Downloaded package '%s' mismatched - Server GUID: %s Client GUID: %s"), *Info.Parent->GetName(), *Info.Guid.String(), *Linker->Summary.Guid.String());
#if OLD_CONNECTION_FAILURE_CODE
 					ConnectionError = FString::Printf(TEXT("Package '%s' version mismatch"), *Info.Parent->GetName());
 					FailedConnectionType = FCT_VersionMismatch;
#else
					ConnectionError = FString::Printf(LocalizeSecure(LocalizeError(TEXT("PackageVersion"),TEXT("Core")), *Info.Parent->GetName()));
#endif
					NetDriver->ServerConnection->Close();
				}
				else
				{
					Info.LocalGeneration = Linker->Summary.Generations.Num();
					// tell the server what we have
					NetDriver->ServerConnection->Logf(TEXT("HAVE GUID=%s GEN=%i"), *Linker->Summary.Guid.String(), Info.LocalGeneration);
				}
			}
		}

		// Send next download request.
		ReceiveNextFile( Connection );
	}
}
void UNetPendingLevel::ReceiveNextFile( UNetConnection* Connection )
{
	// look for a file that needs downloading
	for (INT PackageIndex = 0; PackageIndex < Connection->PackageMap->List.Num(); PackageIndex++)
	{
		if (Connection->PackageMap->List(PackageIndex).PackageFlags & PKG_Need)
		{
			Connection->ReceiveFile(PackageIndex); 
			return;
		}
	}

	// if no more are needed, cleanup the downloader
	if (Connection->Download != NULL)
	{
		Connection->Download->CleanUp();
	}
}

UBOOL UNetPendingLevel::NotifySendingFile( UNetConnection* Connection, FGuid Guid )
{
	// Server has requested a file from this client.
	debugf( NAME_DevNet, *LocalizeError(TEXT("RequestDenied"),TEXT("Engine")) );
	return 0;

}

void UNetPendingLevel::NotifyProgress( EProgressMessageType MessageType, const FString& Title, const FString& Message )
{
	GEngine->SetProgress( MessageType, Title, Message );
}

//
// Update the pending level's status.
//
void UNetPendingLevel::Tick( FLOAT DeltaTime )
{
	check(NetDriver);
	check(NetDriver->ServerConnection);

	// Handle timed out or failed connection.
	if( NetDriver->ServerConnection->State==USOCK_Closed && ConnectionError==TEXT("") )
	{
		ConnectionError = LocalizeError(TEXT("ConnectionFailed"),TEXT("Engine"));
		return;
	}

	// Update network driver.
	NetDriver->TickDispatch( DeltaTime );
	NetDriver->TickFlush();

}
//
// Send JOIN to other end
//
void UNetPendingLevel::SendJoin()
{
	bSentJoinRequest = TRUE;
	NetDriver->ServerConnection->Logf( TEXT("JOIN") );
	NetDriver->ServerConnection->FlushNet();
}
IMPLEMENT_CLASS(UNetPendingLevel);

