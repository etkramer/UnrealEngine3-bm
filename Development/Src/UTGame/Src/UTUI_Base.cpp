//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//
// This holds all of the base UI Objects
//=============================================================================

#include "UTGame.h"
#include "UTGameUIClasses.h"
#include "UTGameUIFrontEndClasses.h"

#include "EngineAudioDeviceClasses.h"

IMPLEMENT_CLASS(UUTGameUISceneClient);
IMPLEMENT_CLASS(UUTGameInteraction);
IMPLEMENT_CLASS(UUTUIScene);
IMPLEMENT_CLASS(UUTGameSettingsCommon);

/*=========================================================================================
UUTGameSettingsCommon - Stores common game settings for online games.
========================================================================================= */

/**
 * Converts a string to a hexified blob.
 *
 * @param InString	String to convert.
 * @param OutBlob	Resulting blob.
 *
 * @return	Returns the hexified string.
 */
UBOOL UUTGameSettingsCommon::StringToBlob(const FString& InString, FString& OutBlob)
{
	BYTE BlobbedString[1024];
	UBOOL bResult = appStringToBlob(InString,BlobbedString,1024);

	INT ExpectedLength = InString.Len() / 3;
	if(bResult)
	{
		// Zero the buffer and treat as if its unicode
		BlobbedString[ExpectedLength] = 0;
		BlobbedString[ExpectedLength + 1] = 0;
		OutBlob = (const TCHAR*)BlobbedString;
	}

	return bResult;
}

/**
 * Converts a hexified blob to a normal string.
 *
 * @param InBlob	String to convert back.
 * @return	Returns the normal string.
 */
FString UUTGameSettingsCommon::BlobToString(const FString& InBlob)
{
	return appBlobToString((const BYTE*)(*InBlob),sizeof(TCHAR)*InBlob.Len());
}

/*=========================================================================================
  UTGameUIScenenClient - We have our own scene client so that we can provide Tick/Prerender
  passes.  We also use this object as a repository for all UTFontPool objects
  ========================================================================================= */

/**
 * Override Tick so that we can measure how fast it's working
 * @See UGameUISceneClient for more details
 */
void UUTGameUISceneClient::Tick(FLOAT DeltaTime)
{
	// Clock the tick pass
	TickTime=0.0f;


	CLOCK_CYCLES(TickTime);
	Super::Tick(DeltaTime);
	UNCLOCK_CYCLES(TickTime);
}

UBOOL UUTGameUISceneClient::InputKey(INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed,UBOOL bGamepad)
{
	for (INT i=0;i<ActiveScenes.Num();i++)
	{
		UUTUIScene* Scene = Cast<UUTUIScene>(ActiveScenes(i));
		if (Scene)
		{
			if ( Scene->PreChildrenInputKey(ControllerId, Key, Event, AmountDepressed, bGamepad) )
			{
				return true;
			}
		}
	}

	return Super::InputKey(ControllerId, Key, Event, AmountDepressed, bGamepad);
}

/**
 * Determines whether the any active scenes process axis input.
 *
 * @param	bProcessAxisInput	receives the flags for whether axis input is needed for each player.
 */
void UUTGameUISceneClient::CheckAxisInputSupport( UBOOL* bProcessAxisInput[UCONST_MAX_SUPPORTED_GAMEPADS] ) const
{
	UBOOL bResult = IsUIActive(SCENEFILTER_InputProcessorOnly);

	for ( INT SceneIndex = ActiveScenes.Num() - 1; SceneIndex >= 0; SceneIndex-- )
	{
		UUTUIScene* UTScene = Cast<UUTUIScene>(ActiveScenes(SceneIndex));
		if ( UTScene != NULL )
		{
			bResult = !UTScene->bIgnoreAxisInput;
			break;
		}
	}

	for ( INT Idx = 0; Idx < UCONST_MAX_SUPPORTED_GAMEPADS; Idx++ )
	{
		*bProcessAxisInput[Idx] = bResult;
	}
}

/**
 * @Return the WorldInfo for the current game
 */
AWorldInfo* UUTGameUISceneClient::GetWorldInfo()
{
	return GWorld ? GWorld->GetWorldInfo() : NULL;
}

/**
 * Renders the UI toast.
 *
 * @param Canvas	Canvas to draw to.
 */
void UUTGameUISceneClient::DrawUIToast(FCanvas* Canvas)
{
	FLOAT Alpha = 0.0f;
	
	if(bHidingToast)
	{
		FLOAT DeltaTime = GWorld->GetRealTimeSeconds()-HideStartTime;
		
		if(DeltaTime > ToastTransitionTime)
		{
			bToastVisible = FALSE;
		}
		else
		{
			Alpha = 1.0f - Clamp<FLOAT>(DeltaTime, 0.0f, ToastTransitionTime) / ToastTransitionTime;
		}
	}
	else
	{
		FLOAT DeltaTime = 0.0f;

		// If the first frame of the toast message hasn't been rendered then we reset the show start time in case the toast message
		// was created during a bink movie or anything else that would prevent it from being rendered thus skewing its render time.
		if(!bFirstFrame)
		{
			bFirstFrame = TRUE;
			ShowStartTime = GWorld->GetRealTimeSeconds();
		}
		else
		{
			DeltaTime = GWorld->GetRealTimeSeconds() - ShowStartTime;
		}
		
		// start the hide
		if(DeltaTime > ToastTransitionTime+ToastDuration && ToastDuration > 0.0f)
		{
			eventFinishToast();
		}
		
		Alpha = Clamp<FLOAT>(DeltaTime, 0.0f, ToastTransitionTime) / ToastTransitionTime;
	}

	if(ToastFont!=NULL && ToastImage!=NULL)
	{
		// Grab the viewport size
		FVector2D ViewportSize;
		GetViewportSize( ( ActiveScenes.Num() > 0 ) ? ActiveScenes(0) : NULL, ViewportSize);

		const FLOAT ScaleFactor = ToastScale * ViewportSize.Y / 768.0f;	// We use 1024x768 for our base sizes.
		const FLOAT Padding = 8.0f*ScaleFactor;	// In pixels;
		FVector2D ToastPosition;
		FVector2D ToastSize;
		FLinearColor ToastColorTemp = ToastColor;
		ToastColorTemp.A *= Alpha;
		
		// Calculate toast message size using the text size plus some padding.
		FIntPoint ToastMessageSize;
		StringSize(ToastFont, ToastMessageSize.X, ToastMessageSize.Y, TEXT("%s"), *ToastMessage);

		ToastSize.X = ToastMessageSize.X*ScaleFactor + Padding*6.0f;
		ToastSize.Y = ToastMessageSize.Y*ScaleFactor + Padding*2.0f;

		// Center the toast message near the bottom of the screen
		ToastPosition.X = (ViewportSize.X - ToastSize.X) / 2.0f;
		ToastPosition.Y = ViewportSize.Y - (ToastSize.Y*2+Padding)*Alpha;

		FLOAT TexW = ToastImage->GetSurfaceWidth();
		FLOAT TexH = ToastImage->GetSurfaceHeight();

		DrawTile(Canvas, ToastPosition.X, ToastPosition.Y, ToastSize.X, ToastSize.Y, ToastImageU/TexW, ToastImageV/TexH, ToastImageUL/TexW, ToastImageVL/TexH, ToastColorTemp, ToastImage->Resource);
		DrawString(Canvas, ToastPosition.X+Padding*3, ToastPosition.Y+Padding, *ToastMessage, ToastFont, ToastTextColor, ScaleFactor, ScaleFactor, 0.f, &ViewportSize.Y);
	}
}



/**
 * Renders the screen warning message
 *
 * @param Canvas	Canvas to draw to.
 */
void UUTGameUISceneClient::DrawScreenWarningMessage( FCanvas* Canvas )
{
	// @todo: Support a separate font other than the toast font for screen warnings?
	if( ToastFont != NULL && ScreenWarningMessage.Len() > 0 )
	{
		// @todo: Allow these to be configurable in script?
		const float ScreenWarningScale = 1.75f;
		const FLinearColor MessageColor( 0.9f, 0.9f, 0.9f, 1.0f );
		const FLinearColor MessageBackgroundColor( 0.0f, 0.0f, 0.0f, 0.9f );

		// Grab the viewport size
		FVector2D ViewportSize;
		GetViewportSize( ( ActiveScenes.Num() > 0 ) ? ActiveScenes( 0 ) : NULL, ViewportSize );

		// Handle aspect ratio
		const FLOAT ScaleFactor = ScreenWarningScale * ViewportSize.Y / 768.0f;	// We use 1024x768 for our base sizes.
		const FLOAT Padding = 8.0f * ScaleFactor;	// In pixels;

		// Calculate message size using the text size plus some padding.
		FIntPoint MessageIntSize;
		StringSize( ToastFont, MessageIntSize.X, MessageIntSize.Y, TEXT("%s"), *ScreenWarningMessage );
		FVector2D MessageSize;
		MessageSize.X = MessageIntSize.X * ScaleFactor + Padding * 6.0f;
		MessageSize.Y = MessageIntSize.Y * ScaleFactor + Padding * 2.0f;

		// Center the message on the screen
		FVector2D MessagePosition;
		MessagePosition.X = ( ViewportSize.X - MessageSize.X ) / 2.0f;
		MessagePosition.Y = ViewportSize.Y / 2.0f - MessageSize.Y + Padding;

		// Draw background tile
		DrawTile( Canvas, MessagePosition.X, MessagePosition.Y, MessageSize.X, MessageSize.Y, 0.0f, 0.0f, 1.0f, 1.0f, MessageBackgroundColor );

		// Draw message text
		DrawString( Canvas, MessagePosition.X + Padding * 3, MessagePosition.Y + Padding, *ScreenWarningMessage, ToastFont, MessageColor, ScaleFactor, ScaleFactor, 0.f, &ViewportSize.Y );
	}
}



/**
 * Render all the active scenes
 */
void UUTGameUISceneClient::RenderScenes( FCanvas* Canvas )
{
	Super::RenderScenes(Canvas);

	// Draw toast UI if we have one
	if( bToastVisible )
	{
		DrawUIToast(Canvas);
	}

	// Draw the screen warning message (if we have one)
	DrawScreenWarningMessage( Canvas );

	// Dim the screen if enabled
	if(bDimScreen)
	{
		DrawTile(Canvas, 0,0, Canvas->GetRenderTarget()->GetSizeX(), Canvas->GetRenderTarget()->GetSizeY(), 0,0,0,0,FLinearColor(0,0,0,1.0f));
	}
}

/**
 * We override the Render_Scene so that we can provide a PreRender pass
 * @See UGameUISceneClient for more details
 */
void UUTGameUISceneClient::Render_Scene(FCanvas* Canvas, UUIScene *Scene, EUIPostProcessGroup UIPostProcessGroup)
{
	if( UIPostProcessGroup != UIPostProcess_None )
	{
		return;
	}

	PreRenderTime=0.0f;
	RenderTime=0.0f;

	// UTUIScene's support a pre-render pass.  If this is a UTUIScene, start that pass and clock it

	UUTUIScene* UTScene = Cast<UUTUIScene>(Scene);
	if ( UTScene )
	{
		CLOCK_CYCLES(PreRenderTime);
		UTScene->PreRender(Canvas);
		UNCLOCK_CYCLES(PreRenderTime);
	}

	// Render the scene

	CLOCK_CYCLES(RenderTime);
	Super::Render_Scene(Canvas,Scene,UIPostProcessGroup);
	UNCLOCK_CYCLES(RenderTime);

	// Debug

	if (bShowRenderTimes)
	{

		FLOAT Mod =  GSecondsPerCycle * 1000.f;

		FLOAT TotalRenderTime = PreRenderTime + RenderTime;
		AvgRenderTime = AvgRenderTime + TotalRenderTime;

		FLOAT TotalTime = TickTime + PreRenderTime + RenderTime;
		AvgTime = AvgTime + TotalTime;

		FrameCount = FrameCount + 1.0;

		FString Test;
		Test = FString::Printf(TEXT("FrameTime : Total[%3.5f]   Tick[%3.5f]   PRend[%3.5f]   Rend[%3.5f]   Animation[%3.5f]"),
					TotalTime * Mod, (TickTime * Mod),(PreRenderTime * Mod),(RenderTime * Mod), (AnimTime * Mod));
		DrawString(Canvas, 0, 200, *Test, GEngine->SmallFont, FLinearColor(1.0f,1.0f,1.0f,1.0f));

		Test = FString::Printf(TEXT("Strings  : %3.5f"), StringRenderTime * Mod);
		DrawString(Canvas, 400, 220, *Test, GEngine->SmallFont, FLinearColor(1.0f,1.0f,1.0f,1.0f));

		Test = FString::Printf(TEXT("Averages  : Frames %f   Total %f    Render %f"), FrameCount,(AvgTime/FrameCount)*Mod, (AvgRenderTime/FrameCount) * Mod);
		DrawString(Canvas, 0, 230, *Test, GEngine->SmallFont, FLinearColor(1.0f,1.0f,1.0f,1.0f));
	}
	else if (FrameCount>0.0f)
	{
		FrameCount = 0.0f;
		AvgTime = 0.0f;
		AvgRenderTime = 0.0f;
	}
}

/** @return TRUE if there are any scenes currently accepting input, FALSE otherwise. */
UBOOL UUTGameUISceneClient::IsUIAcceptingInput()
{
	return IsUIActive(SCENEFILTER_InputProcessorOnly);
}

UBOOL UUTGameUISceneClient::IsInSeamlessTravel()
{
	AWorldInfo* WI = GWorld->GetWorldInfo();
	return ( WI && WI->IsInSeamlessTravel() );
}


/*=========================================================================================
	UUTGameInteraction - Supports globally blocking UI input.
========================================================================================= */

/**
 * @return Whether or not we should process input.
 */
UBOOL UUTGameInteraction::ShouldProcessUIInput() const
{
	return BlockUIInputSemaphore == 0;
}

/**
 * Check a key event received by the viewport.
 *
 * @param	Viewport - The viewport which the key event is from.
 * @param	ControllerId - The controller which the key event is from.
 * @param	Key - The name of the key which an event occured for.
 * @param	Event - The type of event which occured.
 * @param	AmountDepressed - For analog keys, the depression percent.
 * @param	bGamepad - input came from gamepad (ie xbox controller)
 *
 * @return	True to consume the key event, FALSE to pass it on.
 */
UBOOL UUTGameInteraction::InputKey(INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed/*=1.f*/,UBOOL bGamepad/*=FALSE*/)
{
	UBOOL bResult = FALSE;

	if ( ShouldProcessUIInput() )
	{
#if PS3
		// if we are supposed to swap the accept and cancel keys, do that now
		static UBOOL bUseCircleToAccept = appPS3UseCircleToAccept();
		if ( bUseCircleToAccept )
		{
			if ( Key == KEY_XboxTypeS_A )
			{
				Key = KEY_XboxTypeS_B;
			}
			else if ( Key == KEY_XboxTypeS_B )
			{
				Key = KEY_XboxTypeS_A;
			}
		}
#endif
		bResult = Super::InputKey(ControllerId, Key, Event, AmountDepressed,bGamepad);
	}
	else
	{
		// Always swallow input.
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Check an axis movement received by the viewport.
 *
 * @param	Viewport - The viewport which the axis movement is from.
 * @param	ControllerId - The controller which the axis movement is from.
 * @param	Key - The name of the axis which moved.
 * @param	Delta - The axis movement delta.
 * @param	DeltaTime - The time since the last axis update.
 *
 * @return	True to consume the axis movement, FALSE to pass it on.
 */
UBOOL UUTGameInteraction::InputAxis(INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad)
{
	UBOOL bResult = FALSE;

	if ( ShouldProcessUIInput() )
	{
		bResult = Super::InputAxis(ControllerId, Key, Delta, DeltaTime, bGamepad);
	}
	else
	{
		// Always swallow input.
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Check a character input received by the viewport.
 *
 * @param	Viewport - The viewport which the axis movement is from.
 * @param	ControllerId - The controller which the axis movement is from.
 * @param	Character - The character.
 *
 * @return	True to consume the character, FALSE to pass it on.
 */
UBOOL UUTGameInteraction::InputChar(INT ControllerId,TCHAR Character)
{
	UBOOL bResult = FALSE;

	if ( ShouldProcessUIInput() )
	{
		bResult = Super::InputChar(ControllerId, Character);
	}
	else
	{
		// Always swallow input.
		bResult = TRUE;
	}

	return bResult;
}


/*=========================================================================================
  UTUIScene - Our scenes support a PreRender and Tick pass
  ========================================================================================= */

/** 
 * Automatically add any Instanced children and auto-link transient children
 */

void UUTUIScene::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);
	AutoPlaceChildren( this );
 
}

/** Activates a level remote event in kismet. */
void UUTUIScene::ActivateLevelEvent(FName EventName)
{
	ULocalPlayer* LP = GetPlayerOwner();

	if(LP)
	{
		APlayerController* PlayerOwner = LP->Actor;
		UBOOL bFoundRemoteEvents = FALSE;

		// now find the level's sequence
		AWorldInfo* WorldInfo = GetWorldInfo();
		if ( WorldInfo != NULL )
		{
			USequence* GameSequence = WorldInfo->GetGameSequence();
			if ( GameSequence != NULL )
			{
				// now find the root sequence
				USequence* RootSequence = GameSequence->GetRootSequence();
				if ( RootSequence != NULL )
				{
					TArray<USeqEvent_RemoteEvent*> RemoteEvents;
					RootSequence->FindSeqObjectsByClass(USeqEvent_RemoteEvent::StaticClass(), (TArray<USequenceObject*>&)RemoteEvents);

					// now iterate through the list of events, activating those events that have a matching event tag
					for ( INT EventIndex = 0; EventIndex < RemoteEvents.Num(); EventIndex++ )
					{
						USeqEvent_RemoteEvent* RemoteEvt = RemoteEvents(EventIndex);
						if ( RemoteEvt != NULL && RemoteEvt->EventName == EventName && RemoteEvt->bEnabled == TRUE )
						{
							// attempt to activate the remote event
							RemoteEvt->CheckActivate(WorldInfo, PlayerOwner);
							bFoundRemoteEvents = TRUE;
						}
					}
				}
			}
		}
	}
}

#define INSTANCED_PROPERTY_FLAG (CPF_EditInline|CPF_ExportObject|CPF_NeedCtorLink)

/**
 * Search the object for all property that either require auto-linkup (to widgets already in the scene) or
 * instanced
 *
 * @Param	BaseObj - The object to search for properties in
 */
void UUTUIScene::AutoPlaceChildren( UUIScreenObject *const BaseObj)
{
	// Look at all properties of the UTUIScene and perform any needed initialization on them

	for( TFieldIterator<UObjectProperty,CASTCLASS_UObjectProperty> It( BaseObj->GetClass() ); It; ++It)
	{
		UObjectProperty* P = *It;

		//@todo xenon: fix up with May XDK
		if ( (P->PropertyFlags&(CPF_EditInline|CPF_ExportObject|CPF_Transient)) != 0)
		{

			/* We handle 2 cases here. 
			
			First we look to see if we can fill-out any tranisent UIObject member references.  This allows us to have
			a script base scene class that automatically pick up Widgets provided we maintain good nomenclature.  

			Second is auto-adding instanced widgets in a script-based class.
			*/
			if ( (P->PropertyFlags & CPF_Transient) != 0 )
			{
				// We are a transient property, try to auto-assign us. FIXME: Finish support for Arrays by appending _<index> to
				// the property name for element lookup.

				for (INT Index=0; Index < It->ArrayDim; Index++)
				{
					// Find the location of the data
					BYTE* ObjLoc = (BYTE*)BaseObj + It->Offset + (Index * It->ElementSize);
					UObject* ObjPtr = *((UObject**)( ObjLoc ));

					// Skip the assignment if already assigned via default properties.

					if ( !ObjPtr )
					{
						// TypeCheck!!
						UUIObject* SourceObj = BaseObj->FindChild( P->GetFName(), FALSE);
						if (SourceObj && SourceObj->GetClass()->IsChildOf(P->PropertyClass ) )
						{
							// Copy the value in to place
							P->CopyCompleteValue( ObjLoc, &SourceObj);
						}
					}
				}
			}
			else if ( (P->PropertyFlags&INSTANCED_PROPERTY_FLAG) == INSTANCED_PROPERTY_FLAG )
			{
				// Look to see if there is a UIObject assocaited with this property and if so
				// Insert it in to the children array
				for (INT Index=0; Index < It->ArrayDim; Index++)
				{
					UObject* ObjPtr = *((UObject**)((BYTE*)BaseObj + It->Offset + (Index * It->ElementSize) ));
					if ( ObjPtr )
					{
						UUIObject* UIObject = Cast<UUIObject>(ObjPtr);
						if ( UIObject )
						{
							// Should not be trying to attach a default object!
							checkf(!UIObject->IsTemplate(RF_ClassDefaultObject), TEXT("UUTUIObject::Initialize : Trying to insert Default Object %s (from property %s) into %s."), *UIObject->GetName(), *P->GetName(), *BaseObj->GetName());

							//@HACK - Big hack time.  The current UI doesn't support any type of render ordering
							// so rendering is determined solely by the order in which the components are added to
							// the children array.  To kind of account for this, we insert each child in to the 
							// array at the head. 
							//
							// This means you need to be careful with how you layout your member variables.
							// Members in a child will be found before members in the parent.  Otherwise they are in the 
							// order as defined by the code (top down).
							
							// Once a render ordering scheme is in place, remove the forced index and just let nature do the work.

							BaseObj->InsertChild(UIObject, 0);
						}
					}
				}
			}
		}
	}
}

/**
 * Perform a tick pass on all of the children requesting it from this UTUIScene
 *
 * @Param	DeltaTime		How much time since the last tick
 */
void UUTUIScene::Tick( FLOAT DeltaTime )
{
	if (GIsGame || bEditorRealTimePreview)
	{
		TickChildren(this, DeltaTime * GWorld->GetWorldInfo()->TimeDilation);
	}

	Super::Tick(DeltaTime);
}

/**
 * Tick the children before the scene.  This way children and change their dims and still
 *  be insured they are updated correctly.
 *
 * @Param	ParentObject		The Parent object who's children will be ticked
 * @Param	DeltaTime			How much time since the last time
 */

void UUTUIScene::TickChildren(UUIScreenObject* ParentObject, FLOAT DeltaTime)
{
	for ( INT ChildIndex = 0; ChildIndex < ParentObject->Children.Num() ; ChildIndex++ )
	{
		UUTUI_Widget* UTWidget = Cast<UUTUI_Widget>( ParentObject->Children(ChildIndex) );
		if (UTWidget && UTWidget->bRequiresTick)
		{
			// Perform the Tick on this widget
			UTWidget->Tick_Widget(DeltaTime);
		}
		else
		{
			UUTTabPage* UTTabPage = Cast<UUTTabPage>( ParentObject->Children(ChildIndex) );
			if (UTTabPage && UTTabPage->bRequiresTick)
			{
				// Perform the Tick on this widget
				UTTabPage->Tick_Widget(DeltaTime);
			}
		}

		TickChildren( ParentObject->Children(ChildIndex) ,DeltaTime );
	}
}

/**
 * Perform a pre-render pass on all children that are UTUI_Widgets
 *
 * @Param	Canvas		The drawing surface
 */
void UUTUIScene::PreRender(FCanvas* Canvas)
{
	// Prerender the children

	for ( INT ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++ )
	{
		UUTUI_Widget* UTWidget = Cast<UUTUI_Widget>( Children(ChildIndex) );
		if ( UTWidget )
		{
			// Perform the Tick on this widget
			UTWidget->PreRender_Widget(Canvas);
		}
	}
}

/**
 * Sets the screen resolution.
 *
 * @param ResX			Width of the screen
 * @param ResY			Height of the screen
 * @param bFullscreen	Whether or not we are fullscreen.
 */
void UUTUIScene::SetScreenResolution(INT ResX, INT ResY, UBOOL bFullscreen)
{
	FSystemSettingsFriendly FriendlySettings = GSystemSettings.ConvertToFriendlySettings();
	FriendlySettings.ResX = ResX;
	FriendlySettings.ResY = ResY;
	FriendlySettings.bFullScreen = bFullscreen;
	GSystemSettings.ApplyFriendlySettings(FriendlySettings, TRUE);
}

/**
 * This function will attempt to find the UTPlayerController associated with an Index.  If that Index is < 0 then it will find the first 
 * available UTPlayerController.  
 */
AUTPlayerController* UUTUIScene::GetUTPlayerOwner(INT PlayerIndex)
{
	if ( PlayerIndex < 0 )
	{
		if ( PlayerOwner )
		{
			// Attempt to find the first available
			AUTPlayerController* UTPC = Cast<AUTPlayerController>( PlayerOwner->Actor );

			if (UTPC)
			{
				return UTPC;
			}
		}
		PlayerIndex = 0;
	}

	ULocalPlayer* LP = GetPlayerOwner(PlayerIndex);
	if ( LP )
	{
		AUTPlayerController* UTPC = Cast<AUTPlayerController>( LP->Actor );
		return UTPC;
	}
		
	return NULL;
}

/**
 * This function will attempt to resolve the pawn that is associated with this scene
 */
APawn* UUTUIScene::GetPawnOwner()
{
	AUTPlayerController* UTPOwner = GetUTPlayerOwner();
	if ( UTPOwner )
	{
		APawn* ViewedPawn = UTPOwner->ViewTarget ? UTPOwner->ViewTarget->GetAPawn() : NULL;
		if ( !ViewedPawn )
		{
			ViewedPawn = UTPOwner->Pawn;
		}

		return ViewedPawn;
	}
	return NULL;
}

/** 
 * This function returns the UTPlayerReplicationInfo of the Player that owns the hud.  
 */
AUTPlayerReplicationInfo* UUTUIScene::GetPRIOwner()
{
	AUTPlayerController* UTPOwner = GetUTPlayerOwner();
	return UTPOwner 
		?  Cast<AUTPlayerReplicationInfo>(UTPOwner->PlayerReplicationInfo)
		: NULL;
}



/**
 * @Returns GIsGame
 */
UBOOL UUTUIScene::IsGame()
{
	return GIsGame;
}


/** 
 * Tries to unlock a character using a code.
 *
 * @param UnlockCode	Code to use to unlock the character.
 *
 * @return TRUE if the unlock succeeded, FALSE otherwise.
 */
UBOOL UUTUIScene::TryCharacterUnlock(const FString &UnlockCode)
{
	UBOOL bResult = FALSE;
	ANSICHAR Code1[8];
	ANSICHAR Code2[6];
	FString LowerCode = UnlockCode.ToLower();

	Code1[0]='p';
	Code1[1]='h';
	Code1[2]='a';
	Code1[3]='y';
	Code1[4]='d';
	Code1[5]='e';
	Code1[6]='r';
	Code1[7]=NULL;

	Code2[0]='j';
	Code2[1]='i';
	Code2[2]='h';
	Code2[3]='a';
	Code2[4]='n';
	Code2[5]=NULL;

	AUTPlayerController* PC = GetUTPlayerOwner(0);

	if(PC != NULL)
	{
		if(LowerCode==ANSI_TO_TCHAR(Code1))
		{
			PC->UnlockChar(TEXT("Alanna"));
			bResult=TRUE;
		}
		else if(LowerCode==ANSI_TO_TCHAR(Code2))
		{
			PC->UnlockChar(TEXT("Ariel"));
			bResult=TRUE;
		}
	}

	return bResult;
}
	
/**
 * Appends any command-line switches that should be carried over to the new process when starting a dedicated server instance.
 */
void UUTUIScene::AppendPersistentSwitches( FString& ExtraSwitches )
{
	FString OriginalCommandLine = appCmdLine();
	if ( ParseParam(*OriginalCommandLine, TEXT("SeekFreeLoading")) )
	{
		ExtraSwitches += TEXT(" -SeekFreeLoading");
	}
	if ( ParseParam(*OriginalCommandLine, TEXT("VADebug")) )
	{
		ExtraSwitches += TEXT(" -VADebug");
	}
	if ( ParseParam(*OriginalCommandLine, TEXT("CookPackages")) )
	{
		ExtraSwitches += TEXT(" -CookPackages");
	}
	if ( ParseParam(*OriginalCommandLine, TEXT("Installed")) )
	{
		ExtraSwitches += TEXT(" -Installed");
	}
	if ( ParseParam(*OriginalCommandLine, TEXT("User")) )
	{
		ExtraSwitches += TEXT(" -User");
	}
	if ( ParseParam(*OriginalCommandLine, TEXT("USEUNPUBLISHED")) )
	{
		ExtraSwitches += TEXT(" -UseUnpublished");
	}
	if ( ParseParam(*OriginalCommandLine, TEXT("PRIMARYNET")) )
	{
		ExtraSwitches += TEXT(" -PRIMARYNET");
	}
	if ( ParseParam(*OriginalCommandLine, TEXT("ONETHREAD")) )
	{
		ExtraSwitches += TEXT(" -ONETHREAD");
	}
	if ( ParseParam(*OriginalCommandLine, TEXT("NOMOVIE")) )
	{
		ExtraSwitches += TEXT(" -NOMOVIE");
	}
	if ( ParseParam(*OriginalCommandLine, TEXT("UseUnpublished")) )
	{
		ExtraSwitches += TEXT(" -UseUnpublished");
	}
}

/** Starts a dedicated server. */
void UUTUIScene::StartDedicatedServer(const FString &TravelURL)
{
	FString ExtraSwitches = TEXT(" -log=DedicatedServer.log");
	AppendPersistentSwitches(ExtraSwitches);

	// Append login name and password
	UUTUIFrontEnd_LoginScreen* LoginScreen = UUTUIFrontEnd_LoginScreen::StaticClass()->GetDefaultObject<UUTUIFrontEnd_LoginScreen>();
	if(LoginScreen->PlayerNames.Num()>0)
	{
		FString Password=LoginScreen->GetPassword();

		if(Password.Len()>0)
		{
			ExtraSwitches += TEXT(" -Login=");
			ExtraSwitches += LoginScreen->PlayerNames(0);
			ExtraSwitches += TEXT(" -Password=");
			ExtraSwitches += Password;
		}
	}

#if SHIPPING_PC_GAME
	appCreateProc(TEXT("UT3.exe"), *(US + TEXT("Server ") + TravelURL + ExtraSwitches));
#else
	appCreateProc(TEXT("UTGame.exe"), *(US + TEXT("Server ") + TravelURL + ExtraSwitches));
#endif

	// Kill our own process.
	AUTPlayerController* PC = GetUTPlayerOwner(0);
	PC->ConsoleCommand(TEXT("Exit"));
}

/**
 * Creates/Removes local players for splitscreen. 
 *
 * @param bCreatePlayers	Whether we are creating or removing players.
 */
void UUTUIScene::UpdateSplitscreenPlayers(UBOOL bCreatePlayers)
{
	FString Error;

	// If a player is logged in and doesn't have a LP yet, then create one.
	for(INT ControllerId=0; ControllerId<UCONST_MAX_SUPPORTED_GAMEPADS; ControllerId++)
	{
		const INT PlayerIndex = UUIInteraction::GetPlayerIndex(ControllerId);

		if(eventIsLoggedIn(ControllerId))
		{
			if(PlayerIndex==INDEX_NONE)
			{
				debugf(TEXT("UUTUIFrontEnd_TitleScreen::UpdateGamePlayersArray() - Creating LocalPlayer with Controller Id: %i"), ControllerId);
				GEngine->GameViewport->eventCreatePlayer(ControllerId, Error, TRUE);
			}
		}
	}

	// If a player is not logged in, and has a LP, then remove it, but make sure there is always 1 LP in existence.
	for(INT ControllerId=0; ControllerId<UCONST_MAX_SUPPORTED_GAMEPADS && GEngine->GamePlayers.Num() > 1; ControllerId++)
	{
		if(eventIsLoggedIn(ControllerId)==FALSE)
		{
			const INT PlayerIndex = UUIInteraction::GetPlayerIndex(ControllerId);

			if(PlayerIndex!=INDEX_NONE)
			{
				debugf(TEXT("UUTUIFrontEnd_TitleScreen::UpdateGamePlayersArray() - Removing LocalPlayer(Index: %i) with Controller Id: %i"), PlayerIndex, ControllerId);
				GEngine->GameViewport->eventRemovePlayer(GEngine->GamePlayers(PlayerIndex));
			}
		}
	}

	if(bCreatePlayers && GEngine->GamePlayers.Num()<2)
	{
		// Find the first free controller and create a local player for it
		// @todo: Should we have more intelligent controller picking?
		for(INT ControllerId=0; ControllerId<UCONST_MAX_SUPPORTED_GAMEPADS; ControllerId++)
		{
			// look to see if the controller id is already assigned to a player
			UBOOL bExists = FALSE;
			for (INT PlayerIndex = 0; PlayerIndex < GEngine->GamePlayers.Num(); PlayerIndex++)
			{
				if (GEngine->GamePlayers(PlayerIndex)->ControllerId == ControllerId)
				{
					bExists = TRUE;
					break;
				}
			}

			if (bExists == FALSE)
			{
				GEngine->GameViewport->eventCreatePlayer(ControllerId, Error, TRUE);
				break;
			}
		}
	}
}

/** Retrieves all of the possible screen resolutions from the display driver. */
void UUTUIScene::GetPossibleScreenResolutions(TArray<FString> &OutResults)
{
	OutResults.Empty();

	FScreenResolutionArray Resolutions;
	if (RHIGetAvailableResolutions(Resolutions, TRUE))
	{
		// Sort the resolutions based on width then height - purely to look more visually pleasing and to make it easier to find a specific resolution.
		for (INT SortedResolutions=1; SortedResolutions < Resolutions.Num(); ++SortedResolutions )
		{
			const FScreenResolutionRHI UnsortedResolution = Resolutions(SortedResolutions);
			INT ResolutionIndex = SortedResolutions - 1;
			while ( ResolutionIndex >= 0 )
			{
				FScreenResolutionRHI& SortedResolution = Resolutions(ResolutionIndex);
				if ( UnsortedResolution.Width > SortedResolution.Width || (UnsortedResolution.Width == SortedResolution.Width && UnsortedResolution.Height > SortedResolution.Height) )
				{
					break;
				}
				Resolutions(ResolutionIndex+1) = Resolutions(ResolutionIndex);
				ResolutionIndex--;
			}
			Resolutions(ResolutionIndex+1) = UnsortedResolution;
		}

		for (INT ResolutionIndex=0; ResolutionIndex < Resolutions.Num(); ++ResolutionIndex )
		{
			FScreenResolutionRHI& Resolution = Resolutions(ResolutionIndex);
			FString ResString = FString::Printf(TEXT("%dx%d"), Resolution.Width, Resolution.Height);
			OutResults.AddItem(ResString);
		}
	}
}

/** Retrieves all of the possible audio devices from the audio driver. */
void UUTUIScene::GetPossibleAudioDevices(TArray<FString> &OutResults)
{
	OutResults.Empty();
	new(OutResults)FString(TEXT("Generic Software"));
}

/** @return Returns the currently selected audio device. */
FString UUTUIScene::GetCurrentAudioDevice()
{
	FString DeviceName(TEXT(""));
#if !CONSOLE
	GConfig->GetString(TEXT("ALAudio.ALAudioDevice"), TEXT("DeviceName"), DeviceName, GEngineIni);
#endif
	
	return DeviceName;
}

/** 
 * Sets the audio device to use for playback.
 *
 * @param InAudioDevice		Audio device to use.
 */
void UUTUIScene::SetAudioDeviceToUse(const FString &InAudioDevice)
{
#if !CONSOLE
	GConfig->SetString(TEXT("ALAudio.ALAudioDevice"), TEXT("DeviceName"), *InAudioDevice, GEngineIni);
#endif

}

/**
 * @return	TRUE if the user's machine is below the minimum required specs to play the game.
 */
UBOOL UUTUIScene::IsBelowMinSpecs() const
{
	UBOOL bResult = FALSE;

	INT CurrentCompatLevel=0;

	const TCHAR* AppCompatStr = TEXT("AppCompat");
	const TCHAR* AppCompatCompositeEntryStr = TEXT("CompatLevelComposite");
	if ( GConfig->GetInt(AppCompatStr, AppCompatCompositeEntryStr, CurrentCompatLevel, GEngineIni) )
	{
		bResult = (CurrentCompatLevel == 0);
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////
// UUTUIScene_OnlineToast
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIScene_OnlineToast);

void UUTUIScene_OnlineToast::Tick(FLOAT DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bFullyVisible)
	{	
		AWorldInfo* WorldInfo = GetWorldInfo();

		if(WorldInfo != NULL && WorldInfo->RealTimeSeconds-ShowStartTime > ToastDuration)
		{
			eventFinishToast();
			bFullyVisible = FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// UTUIScene_InputBox
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIScene_InputBox);

/**
 * Callback that happens the first time the scene is rendered, any widget positioning initialization should be done here.
 *
 * By default this function recursively calls itself on all of its children.
 */
void UUTUIScene_InputBox::PreInitialSceneUpdate()
{
	Super::Super::PreInitialSceneUpdate();

	if(GIsGame)
	{
		LayoutScene();
	}
}

void UUTUIScene_InputBox::LayoutScene()
{
	FVector2D ViewportSize;
	GetViewportSize(ViewportSize);

	// Position messagebox elements
	const FLOAT ResScale = ViewportSize.Y / 768.0f; // The option height and padding amounts are for 1024x768 so use that as our scaling factor.
	const FLOAT ImagePadding=40.0f*ResScale;
	const FLOAT ButtonPadding=12.0f*ResScale;
	const FLOAT TextPadding=16.0f*ResScale;
	const FLOAT ButtonHeight = 24.0f*ResScale;

	const FLOAT TitleHeight = TitleLabel ? TitleLabel->GetPosition(UIFACE_Bottom, EVALPOS_PixelOwner) : 0.f;
	const FLOAT MessageHeight = MessageLabel ? MessageLabel->GetPosition(UIFACE_Bottom, EVALPOS_PixelOwner) : 0.f;
	const FLOAT MessageLeft = MessageLabel ? MessageLabel->GetPosition(UIFACE_Left, EVALPOS_PixelViewport) : 0.f;
	const FLOAT MessageRight = MessageLabel ? MessageLabel->GetPosition(UIFACE_Right, EVALPOS_PixelOwner) : 0.f;

	const FLOAT EditBoxHeight = InputEditbox ? InputEditbox->GetPosition(UIFACE_Bottom, EVALPOS_PixelOwner) : 0.f;

	FLOAT TotalHeight = TitleHeight+TextPadding+MessageHeight+EditBoxHeight+TextPadding+(ButtonHeight+ButtonPadding)*PotentialOptions.Num();
	FLOAT TotalWidth = MessageLabel ? MessageLabel->GetPosition(UIFACE_Right, EVALPOS_PixelOwner) : 0.f;

	// Position all elements except for BG
	FLOAT StartingY = (ViewportSize.Y - TotalHeight) / 2.0f;
	FLOAT CurrentY = StartingY;
	FLOAT CurrentX = (ViewportSize.X - TotalWidth) / 2.0f;
	
	// Title
	if ( TitleLabel != NULL )
	{
		TitleLabel->UUIScreenObject::SetPosition(CurrentX, UIFACE_Left, EVALPOS_PixelViewport);
		TitleLabel->UUIScreenObject::SetPosition(CurrentY, UIFACE_Top, EVALPOS_PixelOwner);
		CurrentY += TitleHeight+TextPadding;
	}

	// Message
	if ( MessageLabel != NULL )
	{
		MessageLabel->UUIScreenObject::SetPosition(CurrentX, UIFACE_Left, EVALPOS_PixelViewport);
		MessageLabel->UUIScreenObject::SetPosition(CurrentY, UIFACE_Top, EVALPOS_PixelOwner);
		CurrentY += MessageHeight+TextPadding;
	}

	// Input box
	if ( InputEditbox != NULL )
	{
		InputEditbox->UUIScreenObject::SetPosition(CurrentX, UIFACE_Left, EVALPOS_PixelViewport);
		InputEditbox->UUIScreenObject::SetPosition(CurrentY, UIFACE_Top, EVALPOS_PixelOwner);
		CurrentY = InputEditbox->GetPosition(UIFACE_Top, EVALPOS_PixelViewport) + EditBoxHeight;
	}

	// Find the max width of the buttons and make them all the same size
	FLOAT MaxWidth=0;
	if ( ButtonBar != NULL )
	{
		for(INT ButtonIdx=0; ButtonIdx < ARRAY_COUNT(ButtonBar->Buttons) && ButtonIdx < PotentialOptions.Num(); ButtonIdx++)
		{
			if ( ButtonBar->Buttons[ButtonIdx] != NULL )
			{
				FLOAT ButtonWidth = ButtonBar->Buttons[ButtonIdx]->GetPosition(UIFACE_Right, EVALPOS_PixelOwner);
				if(MaxWidth<ButtonWidth)
				{
					MaxWidth=ButtonWidth;
				}
			}
		}

		// Position all of the buttons
		FLOAT ButtonX=(ViewportSize.X/2.0f);
		for(INT ButtonIdx=PotentialOptions.Num()-1; ButtonIdx>=0; ButtonIdx--)
		{
			UUIButton* CurrentButton = ButtonBar->Buttons[ButtonIdx];
			if ( CurrentButton != NULL )
			{
				CurrentButton->SetDockTarget(UIFACE_Left, NULL, UIFACE_MAX);
				CurrentButton->SetDockTarget(UIFACE_Right, NULL, UIFACE_MAX);
				CurrentButton->SetDockTarget(UIFACE_Top, NULL, UIFACE_MAX);
				CurrentButton->SetDockTarget(UIFACE_Bottom, NULL, UIFACE_MAX);

				CurrentButton->SetPosition(ButtonX, UIFACE_Left, EVALPOS_PixelViewport);
				CurrentButton->SetPosition(MaxWidth, UIFACE_Right, EVALPOS_PixelOwner);
				CurrentButton->SetPosition(CurrentY+ButtonPadding, UIFACE_Top, EVALPOS_PixelViewport);
				CurrentButton->SetPosition(ButtonHeight, UIFACE_Bottom, EVALPOS_PixelOwner);

				CurrentY += ButtonHeight+ButtonPadding;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// UTUIScene_MessageBox
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIScene_MessageBox);

/**
 * Callback that happens the first time the scene is rendered, any widget positioning initialization should be done here.
 *
 * By default this function recursively calls itself on all of its children.
 */
void UUTUIScene_MessageBox::PreInitialSceneUpdate()
{
	Super::PreInitialSceneUpdate();

	if(GIsGame)
	{
		FVector2D ViewportSize;
		GetViewportSize(ViewportSize);

		// Position messagebox elements
		const FLOAT ResScale = ViewportSize.Y / 768.0f; // The option height and padding amounts are for 1024x768 so use that as our scaling factor.
		const FLOAT ImagePadding=40.0f*ResScale;
		const FLOAT ButtonPadding=12.0f*ResScale;
		const FLOAT TextPadding=16.0f*ResScale;
		const FLOAT ButtonHeight = 30.0f*ResScale;

		const FLOAT TitleHeight = TitleLabel ? TitleLabel->GetPosition(UIFACE_Bottom, EVALPOS_PixelOwner) : 0.f;
		const FLOAT MessageHeight = MessageLabel ? MessageLabel->GetPosition(UIFACE_Bottom, EVALPOS_PixelOwner) : 0.f;
		const FLOAT MessageLeft = MessageLabel ? MessageLabel->GetPosition(UIFACE_Left, EVALPOS_PixelViewport) : 0.f;
		const FLOAT MessageRight = MessageLabel ? MessageLabel->GetPosition(UIFACE_Right, EVALPOS_PixelOwner) : 0.f;
		FLOAT TotalHeight = TitleHeight+TextPadding+MessageHeight+(ButtonHeight+ButtonPadding)*PotentialOptions.Num();
		FLOAT TotalWidth = MessageLabel ? MessageLabel->GetPosition(UIFACE_Right, EVALPOS_PixelOwner) : 0.f;

		// Position all elements except for BG
		FLOAT StartingY = (ViewportSize.Y - TotalHeight) / 2.0f;
		FLOAT CurrentY = StartingY;
		FLOAT CurrentX = (ViewportSize.X - TotalWidth) / 2.0f;
		
		if ( bRepositionMessageToCenter || !MessageLabel )
		{
			// Title
			if ( TitleLabel != NULL )
			{
				TitleLabel->UUIScreenObject::SetPosition(CurrentX, UIFACE_Left, EVALPOS_PixelViewport);
				TitleLabel->UUIScreenObject::SetPosition(CurrentY, UIFACE_Top, EVALPOS_PixelOwner);
				CurrentY += TitleHeight+TextPadding;
			}

			// Message
			if ( ScrollWindow != NULL )
			{
				ScrollWindow->UUIScreenObject::SetPosition(CurrentX, UIFACE_Left, EVALPOS_PixelViewport);
				ScrollWindow->UUIScreenObject::SetPosition(CurrentY, UIFACE_Top, EVALPOS_PixelOwner);
				CurrentY += MessageHeight;
			}
		}
		else
		{
			CurrentY = MessageLabel->GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport);
			CurrentY -= ButtonPadding;
		}

		// Find the max width of the buttons and make them all the same size
		FLOAT MaxWidth=0;
		if ( ButtonBar != NULL )
		{
			for(INT ButtonIdx=0; ButtonIdx < ARRAY_COUNT(ButtonBar->Buttons) && ButtonIdx < PotentialOptions.Num(); ButtonIdx++)
			{
				if ( ButtonBar->Buttons[ButtonIdx] != NULL )
				{
					FLOAT ButtonWidth = ButtonBar->Buttons[ButtonIdx]->GetPosition(UIFACE_Right, EVALPOS_PixelOwner);
					if(MaxWidth<ButtonWidth)
					{
						MaxWidth=ButtonWidth;
					}
				}
			}

			// Position all of the buttons
			FLOAT ButtonX=(ViewportSize.X/2.0f);
			for(INT ButtonIdx=PotentialOptions.Num()-1; ButtonIdx>=0; ButtonIdx--)
			{
				UUIButton* CurrentButton = ButtonBar->Buttons[ButtonIdx];
				if ( CurrentButton != NULL )
				{
					CurrentButton->SetDockTarget(UIFACE_Left, NULL, UIFACE_MAX);
					CurrentButton->SetDockTarget(UIFACE_Right, NULL, UIFACE_MAX);
					CurrentButton->SetDockTarget(UIFACE_Top, NULL, UIFACE_MAX);
					CurrentButton->SetDockTarget(UIFACE_Bottom, NULL, UIFACE_MAX);

					CurrentButton->SetPosition(ButtonX, UIFACE_Left, EVALPOS_PixelViewport);
					CurrentButton->SetPosition(MaxWidth, UIFACE_Right, EVALPOS_PixelOwner);
					CurrentButton->SetPosition(CurrentY+ButtonPadding, UIFACE_Top, EVALPOS_PixelViewport);
					CurrentButton->SetPosition(ButtonHeight, UIFACE_Bottom, EVALPOS_PixelOwner);

					CurrentY += ButtonHeight+ButtonPadding;
				}
			}
		}
	}
}

/**
 * Provides scenes with a way to alter the amount of transparency to use when rendering parent scenes.
 *
 * @param	AlphaModulationPercent	the value that will be used for modulating the alpha when rendering the scene below this one.
 *
 * @return	TRUE if alpha modulation should be applied when rendering the scene below this one.
 */
UBOOL UUTUIScene_MessageBox::ShouldModulateBackgroundAlpha( FLOAT& AlphaModulationPercent )
{
	if ( IsFading() )
	{
		if ( FadeDuration > 0 )
		{
			// this value represents how far along the interpolation we are
			FLOAT CurrentPosition = IsFadingIn()
				? (GWorld->GetRealTimeSeconds() - FadeStartTime) / FadeDuration
				: 1.f - (GWorld->GetRealTimeSeconds() - FadeStartTime) / FadeDuration;

			if ( CurrentPosition < 0 || CurrentPosition > 1.f )
			{
				if ( IsFadingIn() )
				{
					eventOnShowComplete();
					CurrentPosition = 1.f;
				}
				else
				{
					eventOnHideComplete();
					CurrentPosition = 0.f;
				}
			}
			if ( BackgroundImage != NULL )
			{
				BackgroundImage->Opacity = CurrentPosition;
			}

			const FLOAT InverseModulation = (1.f - AlphaModulationPercent) * CurrentPosition;
			AlphaModulationPercent = 1.f - InverseModulation;
		}
	}
	else if(bHideOnNextTick && bFullyVisible && ((GWorld->GetRealTimeSeconds() - DisplayTime) > MinimumDisplayTime))	// Delay the hide if we need to wait until a minimum display time has passed.
	{
		BeginHide();
	}

	return bRenderParentScenes;
}

void UUTUIScene_MessageBox::BeginShow()
{
	//debugf(TEXT("UUTUIScene_MessageBox::BeginShow() - Showing MessageBox"));
	bFullyVisible = FALSE;
	FadeStartTime = GWorld->GetRealTimeSeconds();
	DisplayTime = FadeStartTime+FadeDuration;
	FadeDirection = 1;

	if ( BackgroundImage == NULL )
	{
		debugf(TEXT("NULL BackgroundImage in %s::BeginShow()"), *GetFullName());
	}
	else
	{
		BackgroundImage->Opacity = 0;
	}

	// Show this scene.
	eventSetVisibility(TRUE);
}

void UUTUIScene_MessageBox::BeginHide()
{
	//debugf(TEXT("UUTUIScene_MessageBox::BeginHide() - Hiding MessageBox"));
	bHideOnNextTick=FALSE;
	bFullyVisible = FALSE;
	FadeStartTime = GWorld->GetRealTimeSeconds();
	FadeDirection = -1;
}

/** 
 * Converts a 2D Screen coordiate in to 3D space
 *
 * @Param	LocalPlayerOwner		The LocalPlayer that owns the viewport where the projection occurs
 * @Param	OutScreenLocation		Returns the location in 2D space
 */
void UUTUIScene::ViewportProject(ULocalPlayer* LocalPlayerOwner, FVector WorldLocation, FVector& OutScreenLocation)
{
	FPlane V(0,0,0,0);

	FViewport* Viewport = SceneClient->RenderViewport;
	if (Viewport)
	{
		// create the view family for rendering the world scene to the viewport's render target
		FSceneViewFamilyContext ViewFamily(
			Viewport,
			GWorld->Scene,
			LocalPlayerOwner->ViewportClient->ShowFlags,
			GWorld->GetTimeSeconds(),
			GWorld->GetDeltaSeconds(),
			GWorld->GetRealTimeSeconds(), 
			GWorld->bGatherDynamicShadowStatsSnapshot ? &GWorld->DynamicShadowStats : NULL, 
			FALSE,
			FALSE,
			FALSE,
			FALSE,					// disable the initial scene color clear in game
			TRUE
			);

		if (LocalPlayerOwner->Actor)
		{
			// Calculate the player's view information.
			FVector		ViewLocation;
			FRotator	ViewRotation;

			FSceneView* View = LocalPlayerOwner->CalcSceneView( &ViewFamily, ViewLocation, ViewRotation, Viewport );
			if (View!=NULL)
			{
				V = View->Project(WorldLocation);

				FLOAT ClipX = Viewport->GetSizeX();
				FLOAT ClipY = Viewport->GetSizeY();

				FVector resultVec(V);
				resultVec.X = (ClipX/2.f) + (resultVec.X*(ClipX/2.f));
				resultVec.Y *= -1.f;
				resultVec.Y = (ClipY/2.f) + (resultVec.Y*(ClipY/2.f));
				OutScreenLocation = resultVec;
			}
		}
	}
	
}

/** 
 * Converts a 2D Screen coordiate in to 3D space
 *
 * @Param	LocalPlayerOwner		The LocalPlayer that owns the viewport where the projection occurs
 * @Param	OutLocation				Returns the Location in world space
 * @Param	OutDirection			Returns the view direction
 */
void UUTUIScene::ViewportDeProject(ULocalPlayer* LocalPlayerOwner, FVector ScreenLocation, FVector& OutLocation, FVector& OutDirection)
{
	FViewport* Viewport = SceneClient->RenderViewport;
	if (Viewport)
	{
		// create the view family for rendering the world scene to the viewport's render target
		FSceneViewFamilyContext ViewFamily(
			Viewport,
			GWorld->Scene,
			LocalPlayerOwner->ViewportClient->ShowFlags,
			GWorld->GetTimeSeconds(),
			GWorld->GetDeltaSeconds(),
			GWorld->GetRealTimeSeconds(), 
			GWorld->bGatherDynamicShadowStatsSnapshot ? &GWorld->DynamicShadowStats : NULL, 
			FALSE,
			FALSE,
			FALSE,
			FALSE,					// disable the initial scene color clear in game
			TRUE
			);

		if (LocalPlayerOwner->Actor)
		{
			// Calculate the player's view information.
			FVector		ViewLocation;
			FRotator	ViewRotation;

			FSceneView* View = LocalPlayerOwner->CalcSceneView( &ViewFamily, ViewLocation, ViewRotation, Viewport );

			INT		X = appTrunc(ScreenLocation.X),
					Y = appTrunc(ScreenLocation.Y);

			// Get the eye position and direction of the mouse cursor in two stages (inverse transform projection, then inverse transform view).
			// This avoids the numerical instability that occurs when a view matrix with large translation is composed with a projection matrix
			FMatrix InverseProjection = View->ProjectionMatrix.Inverse();
			FMatrix InverseView = View->ViewMatrix.Inverse();

			// The start of the raytrace is defined to be at mousex,mousey,0 in projection space
			// The end of the raytrace is at mousex, mousey, 0.5 in projection space
			FLOAT ScreenSpaceX = (X-Viewport->GetSizeX()/2.0f)/(Viewport->GetSizeX()/2.0f);
			FLOAT ScreenSpaceY = (Y-Viewport->GetSizeY()/2.0f)/-(Viewport->GetSizeY()/2.0f);
			FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY,    0, 1.0f);
			FVector4 RayEndProjectionSpace   = FVector4(ScreenSpaceX, ScreenSpaceY, 0.5f, 1.0f);

			// Projection (changing the W coordinate) is not handled by the FMatrix transforms that work with vectors, so multiplications
			// by the projection matrix should use homogenous coordinates (i.e. FPlane).
			FVector4 HGRayStartViewSpace = InverseProjection.TransformFVector4(RayStartProjectionSpace);
			FVector4 HGRayEndViewSpace   = InverseProjection.TransformFVector4(RayEndProjectionSpace);
			FVector RayStartViewSpace(HGRayStartViewSpace.X, HGRayStartViewSpace.Y, HGRayStartViewSpace.Z);
			FVector   RayEndViewSpace(HGRayEndViewSpace.X,   HGRayEndViewSpace.Y,   HGRayEndViewSpace.Z);
			// divide vectors by W to undo any projection and get the 3-space coordinate 
			if (HGRayStartViewSpace.W != 0.0f)
			{
				RayStartViewSpace /= HGRayStartViewSpace.W;
			}
			if (HGRayEndViewSpace.W != 0.0f)
			{
				RayEndViewSpace /= HGRayEndViewSpace.W;
			}
			FVector RayDirViewSpace = RayEndViewSpace - RayStartViewSpace;
			RayDirViewSpace = RayDirViewSpace.SafeNormal();

			// The view transform does not have projection, so we can use the standard functions that deal with vectors and normals (normals
			// are vectors that do not use the translational part of a rotation/translation)
			FVector RayStartWorldSpace = InverseView.TransformFVector(RayStartViewSpace);
			FVector RayDirWorldSpace   = InverseView.TransformNormal(RayDirViewSpace);

			// Finally, store the results in the hitcheck inputs.  The start position is the eye, and the end position
			// is the eye plus a long distance in the direction the mouse is pointing.
			OutLocation = RayStartWorldSpace;
			OutDirection = RayDirWorldSpace.SafeNormal();
		}
	}
}
void UUTUIScene::DeleteDemo(const FString &DemoFilename)
{
	if ( DemoFilename.Right(5) == TEXT(".demo") )
	{
		FString FullPath = appGameDir() * TEXT("Demos") * DemoFilename;
		if ( GFileManager->Delete(*FullPath, true, false) )
		{
			// Remove it from the list
			UDataStoreClient* DSClient = UUIInteraction::GetDataStoreClient();
			if ( GEngine->GamePlayers.Num() > 0 && GEngine->GamePlayers(0) != NULL && GEngine->GameViewport != NULL && DSClient != NULL )
			{

				UUTUIDataStore_MenuItems* MenuItemStore = Cast<UUTUIDataStore_MenuItems>(DSClient->FindDataStore(FName(TEXT("UTMenuItems")), GEngine->GamePlayers(0)));
				if (MenuItemStore != NULL)
				{
					TArray<UUIResourceDataProvider*> CurrentProviders;
					MenuItemStore->ListElementProviders.MultiFind(TEXT("DemoFiles"), CurrentProviders);

					for (INT i = 0; i < CurrentProviders.Num(); i++)
					{
						UUTUIDataProvider_DemoFile* Provider = Cast<UUTUIDataProvider_DemoFile>(CurrentProviders(i));
	#if PLATFORM_UNIX
						if (Provider != NULL && appStrcmp(*Provider->Filename, *DemoFilename) == 0)
	#else
						if (Provider != NULL && appStricmp(*Provider->Filename, *DemoFilename) == 0)
	#endif
						{
							MenuItemStore->ListElementProviders.RemovePair(TEXT("DemoFiles"), Provider);
							MenuItemStore->eventRefreshSubscribers();

							break;
						}
					}
				}
			}
		}
	}
}
