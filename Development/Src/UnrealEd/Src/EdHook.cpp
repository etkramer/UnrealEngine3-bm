/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes.
#include "UnrealEd.h"
#include "..\..\Core\Inc\UnMsg.h"
#include "Properties.h"

// Thread exchange.
HANDLE			hEngineThreadStarted;
HANDLE			hEngineThread;
HWND			hWndEngine;
DWORD			EngineThreadId;
const TCHAR*	GItem;
const TCHAR*	GValue;
TCHAR*			GCommand;

extern int GLastScroll;

// Misc.
UEngine* Engine;

/*-----------------------------------------------------------------------------
	Editor hook exec.
-----------------------------------------------------------------------------*/

void UUnrealEdEngine::NotifyDestroy( void* Src )
{
	const INT idx = ActorProperties.FindItemIndex( (WxPropertyWindowFrame*)Src );
	if( idx != INDEX_NONE )
	{
		ActorProperties.Remove( idx );
	}
	if( Src==LevelProperties )
		LevelProperties = NULL;
	if( Src==GApp->ObjectPropertyWindow )
		GApp->ObjectPropertyWindow = NULL;
}
void UUnrealEdEngine::NotifyPreChange( void* Src, UProperty* PropertyAboutToChange )
{
	BeginTransaction( *LocalizeUnrealEd("EditProperties") );
}
void UUnrealEdEngine::NotifyPostChange( void* Src, UProperty* PropertyThatChanged )
{
	EndTransaction();

	if( ActorProperties.FindItemIndex( (WxPropertyWindowFrame*)Src ) != INDEX_NONE )
	{
		GCallbackEvent->Send( CALLBACK_ActorPropertiesChange );
	}
	GEditorModeTools().GetCurrentMode()->ActorPropChangeNotify();
}
void UUnrealEdEngine::NotifyExec( void* Src, const TCHAR* Cmd )
{
	if( ParseCommand(&Cmd,TEXT("BROWSECLASS")) )
	{
		appErrorf(LocalizeSecure(LocalizeUnrealEd("Error_TriedToExecDeprecatedCmd"),Cmd));
	}
	else if( ParseCommand(&Cmd,TEXT("USECURRENT")) )
	{
		appErrorf(LocalizeSecure(LocalizeUnrealEd("Error_TriedToExecDeprecatedCmd"),Cmd));
	}
}

void UUnrealEdEngine::UpdatePropertyWindows()
{
	TArray<UObject*> SelectedActors;

	// Assemble a set of valid selected actors.
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( !Actor->bDeleteMe )
		{
			SelectedActors.AddItem( Actor );
		}
	}

	for( INT x = 0 ; x < ActorProperties.Num() ; ++x )
	{
		if( !ActorProperties(x)->IsLocked() )
		{
			// Update the unlocked window.
			ActorProperties(x)->SetObjectArray( SelectedActors, 0,1,1 );
		}
		else
		{
			ActorProperties(x)->Freeze();
			// Validate the contents of any locked windows.  Any object that is being viewed inside
			// of a locked property window but is no longer valid needs to be removed.
			for ( WxPropertyWindowFrame::TObjectIterator Itor( ActorProperties(x)->ObjectIterator() ) ; Itor ; ++Itor )
			{
				AActor* Actor = Cast<AActor>( *Itor );

				// Iterate over all actors, searching for the selected actor.
				// @todo DB: Optimize -- much object iteration happening here...
				UBOOL Found = FALSE;
				for( FActorIterator It; It; ++It )
				{
					if( Actor == *It )
					{
						Found = TRUE;
						break;
					}
				}

				// If the selected actor no longer exists, remove it from the property window.
				if( !Found )
				{
					ActorProperties(x)->RemoveActor( Actor );
					Itor.Reset();
				}
			}
			ActorProperties(x)->Thaw();
		}
	}
}
