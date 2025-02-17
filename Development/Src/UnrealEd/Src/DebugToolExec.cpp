/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "DebugToolExec.h"
#include "Properties.h"

/**
 * Brings up a property window to edit the passed in object.
 *
 * @param Object	property to edit
 */
void FDebugToolExec::EditObject( UObject* Object )
{
#if FINAL_RELEASE || SHIPPING_PC_GAME
	// the effects of this cannot be easily reversed, so prevent the user from playing network games without restarting to avoid potential exploits
	GDisallowNetworkTravel = TRUE;
#endif

	// Only allow EditObject if we're using wxWidgets.
	extern UBOOL GUsewxWindows;
	if( GUsewxWindows )
	{
		//@warning @todo @fixme: EditObject isn't aware of lifetime of UObject so it might be editing a dangling pointer!
		// This means the user is currently responsible for avoiding crashes!
		WxPropertyWindowFrame* Properties = new WxPropertyWindowFrame;
		Properties->Create( NULL, -1, 1 );

		// Disallow closing so that closing the EditActor property window doesn't terminate the application.
		Properties->DisallowClose();

		Properties->SetObject( Object, 0, 1, 1, 1 );
		Properties->Show();

		if (GIsPlayInEditorWorld)
		{
			PIEFrames.AddItem(Properties);
		}
	}
}

/**
 * Exec handler, parsing the passed in command
 *
 * @param Cmd	Command to parse
 * @param Ar	output device used for logging
 */
UBOOL FDebugToolExec::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	// these commands are only allowed in standalone games
#if FINAL_RELEASE || SHIPPING_PC_GAME
	if (GWorld->GetNetMode() != NM_Standalone || (Cast<UGameEngine>(GEngine) != NULL && ((UGameEngine*)GEngine)->GPendingLevel != NULL))
	{
		return 0;
	}
	// Edits the class defaults.
	else
#endif
	if( ParseCommand(&Cmd,TEXT("EDITDEFAULT")) )
	{
		UClass* Class;
		if( ParseObject<UClass>( Cmd, TEXT("CLASS="), Class, ANY_PACKAGE ) )
		{
			EditObject( Class->GetDefaultObject() );
		}
		else 
		{
			Ar.Logf( TEXT("Missing class") );
		}
		return 1;
	}
	else if (ParseCommand(&Cmd,TEXT("EDITOBJECT")))
	{
		UClass* searchClass = NULL;
		UObject* foundObj = NULL;
		// Search by class.
		if (ParseObject<UClass>(Cmd, TEXT("CLASS="), searchClass, ANY_PACKAGE))
		{
			// pick the first valid object
			for (FObjectIterator It(searchClass); It && foundObj == NULL; ++It) 
			{
				if (!It->IsPendingKill())
				{
					foundObj = *It;
				}
			}
		}
		// Search by name.
		else
		{
			FName searchName;
			FString SearchPathName;
			if ( Parse(Cmd, TEXT("NAME="), searchName) )
			{
				// Look for actor by name.
				for( TObjectIterator<UObject> It; It && foundObj == NULL; ++It )
				{
					if (It->GetFName() == searchName) 
					{
						foundObj = *It;
					}
				}
			}
			else if ( ParseToken(Cmd,SearchPathName, TRUE) )
			{
				foundObj = FindObject<UObject>(ANY_PACKAGE,*SearchPathName);
			}
		}

		// Bring up an property editing window for the found object.
		if (foundObj != NULL)
		{
			EditObject(foundObj);
		}
		else
		{
			Ar.Logf(TEXT("Target not found"));
		}
		return 1;
	}
	// Edits an objects properties or copies them to the clipboard.
	else if( ParseCommand(&Cmd,TEXT("EDITACTOR")) )
	{
		UClass*		Class = NULL;
		AActor*		Found = NULL;

		if (ParseCommand(&Cmd, TEXT("TRACE")))
		{
			AActor* Player  = GEngine && GEngine->GamePlayers.Num() ? GEngine->GamePlayers(0)->Actor : NULL;
			APlayerController* PC = Cast<APlayerController>(Player);
			if (PC != NULL)
			{
				// Do a trace in the player's facing direction and edit anything that's hit.
				FVector PlayerLocation;
				FRotator PlayerRotation;
				PC->eventGetPlayerViewPoint(PlayerLocation, PlayerRotation);
				FCheckResult Hit(1.0f);
				// Prevent the trace intersecting with the player's pawn.
				if( PC->Pawn )
				{
					Player = PC->Pawn;
				}
				GWorld->SingleLineCheck(Hit, Player, PlayerLocation + PlayerRotation.Vector() * 10000, PlayerLocation, TRACE_SingleResult | TRACE_Actors);
				Found = Hit.Actor;
			}
		}
		// Search by class.
		else if( ParseObject<UClass>( Cmd, TEXT("CLASS="), Class, ANY_PACKAGE ) && Class->IsChildOf(AActor::StaticClass()) )
		{
			UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
			
			// Look for the closest actor of this class to the player.
			AActor* Player  = GameEngine && GameEngine->GamePlayers.Num() ? GameEngine->GamePlayers(0)->Actor : NULL;
			APlayerController* PC = Cast<APlayerController>(Player);
			FVector PlayerLocation(0.0f);
			if (PC != NULL)
			{
				FRotator DummyRotation;
				PC->eventGetPlayerViewPoint(PlayerLocation, DummyRotation);
			}

			FLOAT   MinDist = FLT_MAX;
			for( FActorIterator It; It; ++It )
			{
				FLOAT Dist = Player ? FDist(It->Location, PlayerLocation) : 0.0;
				if
				(	!It->bDeleteMe
				&&	It->IsA(Class)
				&&	Dist < MinDist
				)
				{
					MinDist = Dist;
					Found   = *It;
				}
			}
		}
		// Search by name.
		else
		{
			FName ActorName;
			if( Parse( Cmd, TEXT("NAME="), ActorName ) )
			{
				// Look for actor by name.
				for( FActorIterator It; It; ++It )
				{
					if( It->GetFName() == ActorName )
					{
						Found = *It;
						break;
					}
				}
			}
		}

		// Bring up an property editing window for the found object.
		if( Found )
		{
			EditObject( Found );
		}
		else
		{
			Ar.Logf( TEXT("Target not found") );
		}

		return 1;
	}
	else
	{
		return 0;
	}
}

/**
 * Special UnrealEd cleanup function for cleaning up after a Play In Editor session
 */
void FDebugToolExec::CleanUpAfterPIE()
{
	// destroy all PIE editobject frames
	for (INT FrameIndex = 0; FrameIndex < PIEFrames.Num(); FrameIndex++)
	{
		delete PIEFrames(FrameIndex);
	}

	// clear the array
	PIEFrames.Empty();
}
