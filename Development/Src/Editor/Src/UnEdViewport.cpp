/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "EngineDecalClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineMaterialClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "EnginePrefabClasses.h"
#include "CameraController.h"
#include "MouseDeltaTracker.h"
#include "ScopedTransaction.h"
#include "HModel.h"
#include "SpeedTree.h"
#include "BSPOps.h"

static const FLOAT MOUSE_CLICK_DRAG_DELTA	= 1.0f;
static const FLOAT CAMERA_MAYAPAN_SPEED		= 0.6f;

/**
 * A hit proxy class for sockets in the main editor viewports.
 */
struct HLevelSocketProxy : public HHitProxy
{
	DECLARE_HIT_PROXY( HLevelSocketProxy, HHitProxy );

	AActor* Actor;
	USkeletalMeshComponent* SkelMeshComponent;
	INT SocketIndex;

	HLevelSocketProxy(AActor* InActor, USkeletalMeshComponent* InSkelMeshComponent, INT InSocketIndex)
		:	HHitProxy(HPP_UI)
		,	Actor( InActor )
		,	SkelMeshComponent( InSkelMeshComponent )
		,	SocketIndex( InSocketIndex )
	{}
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Actor;
		Ar << SkelMeshComponent;
	}
};

// Loop through all viewports and disable any realtime viewports viewing the edited level before running the game.
// This won't disable things like preview viewports in Cascade etc.
void UEditorEngine::DisableRealtimeViewports()
{
	for( INT x = 0 ; x < ViewportClients.Num() ; ++x)
	{
		FEditorLevelViewportClient* VC = ViewportClients(x);
		if( VC )
		{
			VC->SetRealtime( FALSE, TRUE );
		}
	}

	RedrawAllViewports();

	GCallbackEvent->Send( CALLBACK_UpdateUI );
}

/**
 * Restores any realtime viewports that have been disabled by DisableRealtimeViewports. This won't
 * disable viewporst that were realtime when DisableRealtimeViewports has been called and got
 * latter toggled to be realtime.
 */
void UEditorEngine::RestoreRealtimeViewports()
{
	for( INT x = 0 ; x < ViewportClients.Num() ; ++x)
	{
		FEditorLevelViewportClient* VC = ViewportClients(x);
		if( VC )
		{
			VC->RestoreRealtime();
		}
	}

	RedrawAllViewports();

	GCallbackEvent->Send( CALLBACK_UpdateUI );
}


/**
 * Adds an actor to the world at the specified location.
 *
 * @param	Class		A valid non-abstract, non-transient, placeable class.
 * @param	Location	The world-space location to spawn the actor.
 * @param	bSilent		If TRUE, suppress logging (optional, defaults to FALSE).
 * @result				A pointer to the newly added actor, or NULL if add failed.
 */
AActor* UEditorEngine::AddActor(UClass* Class, const FVector& Location, UBOOL bSilent)
{
	check( Class );

	if( !bSilent )
	{
		debugf( NAME_Log,
				TEXT("Attempting to add actor of class '%s' to level at %0.2f,%0.2f,%0.2f"),
				*Class->GetName(), Location.X, Location.Y, Location.Z );
	}

	///////////////////////////////
	// Validate class flags.

	if( Class->ClassFlags & CLASS_Abstract )
	{
		warnf( NAME_Error, TEXT("Class %s is abstract.  You can't add actors of this class to the world."), *Class->GetName() );
		return NULL;
	}
	if( !(Class->ClassFlags & CLASS_Placeable) )
	{
		warnf( NAME_Error, TEXT("Class %s isn't placeable.  You can't add actors of this class to the world."), *Class->GetName() );
		return NULL;
	}
	if( Class->ClassFlags & CLASS_Transient )
	{
		warnf( NAME_Error, TEXT("Class %s is transient.  You can't add actors of this class in UnrealEd."), *Class->GetName() );
		return NULL;
	}

	///////////////////////////////
	// Don't spawn the actor if the current level is locked.

	// Find the streaming level associated with the current level.
	AWorldInfo*	WorldInfo = GWorld->GetWorldInfo();
	ULevelStreaming* StreamingLevel = NULL;

	for( INT LevelIndex = 0 ; LevelIndex < WorldInfo->StreamingLevels.Num() ; ++LevelIndex )
	{
		ULevelStreaming* CurStreamingLevel = WorldInfo->StreamingLevels( LevelIndex );
		if( CurStreamingLevel && CurStreamingLevel->LoadedLevel == GWorld->CurrentLevel )
		{
			StreamingLevel = CurStreamingLevel;
			break;
		}
	}

	if ( StreamingLevel && StreamingLevel->bLocked )
	{
		warnf( NAME_Error, TEXT("Can't add actor: the current level is locked.") );
		return NULL;
	}

	///////////////////////////////
	// Transactionally add the actor.
	AActor* Actor = NULL;
	{
		const FScopedTransaction Transaction( TEXT("Add Actor") );

		SelectNone( FALSE, TRUE );

		AActor* Default = Class->GetDefaultActor();
		Actor = GWorld->SpawnActor( Class, NAME_None, Location, Default->Rotation, NULL, TRUE );
		if( Actor )
		{
			SelectActor( Actor, 1, NULL, 0 );
			if( Actor->IsA(ABrush::StaticClass()) )
			{
				FBSPOps::csgCopyBrush( (ABrush*)Actor, (ABrush*)Class->GetDefaultActor(), 0, 0, 1, TRUE );
			}
			Actor->InvalidateLightingCache();
			Actor->PostEditMove( TRUE );
		}
		else
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd("Error_Couldn'tSpawnActor") );
		}
	}

	///////////////////////////////
	// If this actor is part of any groups (set in its default properties), add them into the visible groups list.

	if( Actor )
	{
		TArray<FString> NewGroups;
		Actor->Group.ToString().ParseIntoArray( &NewGroups, TEXT(","), 0 );

		TArray<FString> VisibleGroups;
		GWorld->GetWorldInfo()->VisibleGroups.ParseIntoArray( &VisibleGroups, TEXT(","), 0 );

		for( INT x = 0 ; x < NewGroups.Num() ; ++x )
		{
			VisibleGroups.AddUniqueItem( NewGroups(x) );
		}

		GWorld->GetWorldInfo()->VisibleGroups = TEXT("");
		for( INT x = 0 ; x < VisibleGroups.Num() ; ++x )
		{
			if( GWorld->GetWorldInfo()->VisibleGroups.Len() > 0 )
			{
				GWorld->GetWorldInfo()->VisibleGroups += TEXT(",");
			}
			GWorld->GetWorldInfo()->VisibleGroups += VisibleGroups(x);
		}

		GCallbackEvent->Send( CALLBACK_GroupChange );
	}

	///////////////////////////////
	// Clean up.

	if ( Actor )
	{
		Actor->MarkPackageDirty();
		GCallbackEvent->Send( CALLBACK_LevelDirtied );
		GCallbackEvent->Send( CALLBACK_RefreshEditor_LevelBrowser );
	}

	NoteSelectionChange();

	return Actor;
}

/**
 * Looks for an appropriate actor factory for the specified UClass.
 *
 * @param	InClass		The class to find the factory for.
 * @return				A pointer to the factory to use.  NULL if no factories support this class.
 */
UActorFactory* UEditorEngine::FindActorFactory( const UClass* InClass )
{
	for( INT i = 0 ; i < ActorFactories.Num() ; ++i )
	{
		UActorFactory* Factory = ActorFactories(i);
		if( Factory->NewActorClass == InClass )
		{
			return Factory;
		}
	}

	return NULL;
}

/**
* Uses the supplied factory to create an actor at the clicked location ands add to level.
*
* @param	Factory					The factor to create the actor from.  Must be non-NULL.
* @param	bIgnoreCanCreate		[opt] If TRUE, don't call UActorFactory::CanCreateActor.  Default is FALSE.
* @param	bUseSurfaceOrientation	[opt] If TRUE, align new actor's orientation to the underlying surface normal.  Default is FALSE.
* @return							A pointer to the new actor, or NULL on fail.
*/
AActor* UEditorEngine::UseActorFactory(UActorFactory* Factory, UBOOL bIgnoreCanCreate, UBOOL bUseSurfaceOrientation )
{
	check( Factory );
	Factory->AutoFillFields( GetSelectedObjects() );

	if ( !bIgnoreCanCreate )
	{
		FString ActorErrorMsg;
		if( !Factory->CanCreateActor( ActorErrorMsg ) )
		{
			appMsgf( AMT_OK, *LocalizeUnrealEd( *ActorErrorMsg ) );
			return NULL;
		}
	}

	Constraints.Snap( ClickLocation, FVector(0, 0, 0) );

	AActor* NewActorTemplate = Factory->GetDefaultActor();
	FVector Collision = NewActorTemplate->GetCylinderExtent();

	// if no collision extent found, try alternate collision for editor placement
	if ( Collision.IsZero() && NewActorTemplate->bCollideWhenPlacing )
	{
		// can't use AActor::GetComponentsBoundingBox() since default actor's components aren't attached
		if ( NewActorTemplate->CollisionComponent )
		{
			FBox Box(0.f);

			// Only use collidable components to find collision bounding box.
			if( NewActorTemplate->CollisionComponent->CollideActors )
			{
				FBoxSphereBounds DefaultBounds = NewActorTemplate->CollisionComponent->Bounds;
				NewActorTemplate->CollisionComponent->UpdateBounds();
				Box += NewActorTemplate->CollisionComponent->Bounds.GetBox();
				NewActorTemplate->CollisionComponent->Bounds = DefaultBounds;
			}
			FVector BoxExtent = Box.GetExtent();
			FLOAT CollisionRadius = appSqrt( (BoxExtent.X * BoxExtent.X) + (BoxExtent.Y * BoxExtent.Y) );
			Collision = FVector(CollisionRadius, CollisionRadius, BoxExtent.Z);
		}
	}
	FVector Location = ClickLocation + ClickPlane * (FBoxPushOut(ClickPlane, Collision) + 0.1);

	Constraints.Snap( Location, FVector(0, 0, 0) );

	// Orient the new actor with the surface normal?
	const FRotator SurfaceOrientation( ClickPlane.Rotation() );
	const FRotator* const Rotation = bUseSurfaceOrientation ? &SurfaceOrientation : NULL;

	AActor* Actor = NULL;
	{
		const FScopedTransaction Transaction( TEXT("Create Actor") );
		// Create the actor.
		Actor = Factory->CreateActor( &Location, Rotation, NULL ); 
		if(Actor)
		{
			SelectNone( FALSE, TRUE );
			SelectActor( Actor, TRUE, NULL, TRUE );
			Actor->InvalidateLightingCache();
			Actor->PostEditMove( TRUE );
		}
	}

	RedrawLevelEditingViewports();

	if ( Actor )
	{
		Actor->MarkPackageDirty();
		GCallbackEvent->Send( CALLBACK_LevelDirtied );
		GCallbackEvent->Send( CALLBACK_RefreshEditor_LevelBrowser );
	}

	return Actor;
}

/**
 * Replaces the selected Actors with the same number of a different kind of Actor
 * if a Factory is specified, it is used to spawn the requested Actors, otherwise NewActorClass is used (one or the other must be specified)
 * note that only Location, Rotation, Drawscale, Drawscale3D, Tag, and Group are copied from the old Actors
 * 
 * @param Factory - the Factory to use to create Actors
 */
void UEditorEngine::ReplaceSelectedActors(UActorFactory* Factory, UClass* NewActorClass)
{
	// Provide the option to abort the delete if e.g. Kismet-referenced actors are selected.
	UBOOL bIgnoreKismetReferenced = FALSE;
	if (ShouldAbortActorDeletion(bIgnoreKismetReferenced))
	{
		return;
	}
	else if (Factory != NULL)
	{
		Factory->AutoFillFields(GetSelectedObjects());

		FString ActorErrorMsg;
		if (!Factory->CanCreateActor(ActorErrorMsg))
		{
			appMsgf(AMT_OK, *LocalizeUnrealEd(*ActorErrorMsg));
			return;
		}
	}
	else if (NewActorClass == NULL)
	{
		debugf(NAME_Error, TEXT("UEditorEngine::ReplaceSelectedActors() called with NULL parameters!"));
		return;
	}

	{
		const FScopedTransaction Transaction(TEXT("Replace Actor(s)"));

		ULevel* OldCurrentLevel = GWorld->CurrentLevel;

		// construct a list of Actors to replace in a separate pass so we can modify the selection set as we perform the replacement
		TArray<AActor*> ActorsToReplace;
		for (FSelectionIterator It = GetSelectedActorIterator(); It; ++It)
		{
			if (!It->IsInPrefabInstance() || It->IsA(APrefabInstance::StaticClass()))
			{
				ActorsToReplace.AddItem((AActor*)*It);
			}
		}

		while (ActorsToReplace.Num() > 0)
		{
			AActor* OldActor = ActorsToReplace.Pop();
			GWorld->CurrentLevel = OldActor->GetLevel();
			AActor* NewActor = NULL;
			// create the actor
			if (Factory != NULL)
			{
				NewActor = Factory->CreateActor(&OldActor->Location, &OldActor->Rotation, NULL);
			}
			else
			{
				NewActor = GWorld->SpawnActor(NewActorClass, NAME_None, OldActor->Location, OldActor->Rotation);
			}
			if (NewActor != NULL)
			{
				NewActor->DrawScale = OldActor->DrawScale;
				NewActor->DrawScale3D = OldActor->DrawScale3D;
				NewActor->Group = OldActor->Group;
				NewActor->Tag = OldActor->Tag;
				SelectActor(OldActor, FALSE, NULL, TRUE);
				SelectActor(NewActor, TRUE, NULL, TRUE);
				NewActor->InvalidateLightingCache();
				NewActor->PostEditMove(TRUE);
				NewActor->MarkPackageDirty();
				if (OldActor->IsA(APrefabInstance::StaticClass()))
				{
					((APrefabInstance*)OldActor)->DestroyPrefab(GetSelectedActors());
				}
				GWorld->EditorDestroyActor(OldActor, TRUE);
			}
		}

		GWorld->CurrentLevel = OldCurrentLevel;
	}

	RedrawLevelEditingViewports();

	GCallbackEvent->Send( CALLBACK_LevelDirtied );
	GCallbackEvent->Send( CALLBACK_RefreshEditor_LevelBrowser );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FViewportClick::FViewportClick - Calculates useful information about a click for the below ClickXXX functions to use.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

FViewportClick::FViewportClick(FSceneView* View,FEditorLevelViewportClient* ViewportClient,FName InKey,EInputEvent InEvent,INT X,INT Y):
	Key(InKey),
	Event(InEvent),
	ClickPos(X,Y)
{
	const FMatrix InvViewMatrix = View->ViewMatrix.Inverse();
	const FMatrix InvProjMatrix = View->ProjectionMatrix.Inverse();

	const FLOAT ScreenX = (FLOAT)X / (FLOAT)ViewportClient->Viewport->GetSizeX() * 2.0f - 1.0f;
	const FLOAT ScreenY = 1.0f - (FLOAT)Y / (FLOAT)ViewportClient->Viewport->GetSizeY() * 2.0f;

	if(ViewportClient->ViewportType == LVT_Perspective)
	{
		Origin = InvViewMatrix.GetOrigin();
		Direction = InvViewMatrix.TransformNormal(FVector(InvProjMatrix.TransformFVector4(FVector4(ScreenX * NEAR_CLIPPING_PLANE,ScreenY * NEAR_CLIPPING_PLANE,0.0f,NEAR_CLIPPING_PLANE)))).SafeNormal();
	}
	else
	{
		Origin = InvViewMatrix.TransformFVector4(InvProjMatrix.TransformFVector4(FVector4(ScreenX,ScreenY,0.0f,1.0f)));
		Direction = InvViewMatrix.TransformNormal(FVector(0,0,1)).SafeNormal();
	}

	ControlDown = ViewportClient->Viewport->KeyState(KEY_LeftControl) || ViewportClient->Viewport->KeyState(KEY_RightControl);
	ShiftDown = ViewportClient->Viewport->KeyState(KEY_LeftShift) || ViewportClient->Viewport->KeyState(KEY_RightShift);
	AltDown = ViewportClient->Viewport->KeyState(KEY_LeftAlt) || ViewportClient->Viewport->KeyState(KEY_RightAlt);
}

namespace
{
	/**
	 * This function picks a color from under the mouse in the viewport and adds a light with that color.
	 * This is to make it easy for LDs to add lights that fake radiosity.
	 * @param Viewport	Viewport to pick color from.
	 * @param Click		A class that has information about where and how the user clicked on the viewport.
	 */
	static void PickColorAndAddLight(FViewport* Viewport, const FViewportClick &Click)
	{
		// Read pixels from viewport.
		TArray<FColor> OutputBuffer;

		// We need to redraw the viewport before reading pixels otherwise we may be reading back from an old buffer.
		Viewport->Draw();
		Viewport->ReadPixels(OutputBuffer);

		// Sample the color we want.
		const INT ClickX = Click.GetClickPos().X;
		const INT ClickY = Click.GetClickPos().Y;
		const INT PixelIdx = ClickX + ClickY * (INT)Viewport->GetSizeX();

		if(PixelIdx < OutputBuffer.Num())
		{
			const FColor PixelColor = OutputBuffer(PixelIdx);
			
			UActorFactory* ActorFactory = GEditor->FindActorFactory( APointLight::StaticClass() );
			
			AActor* NewActor = GEditor->UseActorFactory( ActorFactory, TRUE );

			APointLight* Light = CastChecked<APointLight>(NewActor);
			UPointLightComponent* PointLightComponent = Cast<UPointLightComponent>( Light->LightComponent );

			// Set properties to defaults that will make it easy for LDs to use this light for faking radiosity.
			PointLightComponent->LightColor = PixelColor;
			PointLightComponent->Brightness = 0.2f;
			PointLightComponent->Radius = 512.0f;

			if ( PointLightComponent->PreviewLightRadius )
			{
				PointLightComponent->PreviewLightRadius->SphereRadius = PointLightComponent->Radius;
				PointLightComponent->PreviewLightRadius->Translation = PointLightComponent->Translation;
			}
		}
	}

	/**
 	 * Creates an actor of the specified type, trying first to find an actor factory,
	 * falling back to "ACTOR ADD" exec and SpawnActor if no factory is found.
	 * Does nothing if ActorClass is NULL.
	 */
	static AActor* PrivateAddActor(UClass* ActorClass, UBOOL bUseSurfaceOrientation=FALSE)
	{
		if ( ActorClass )
		{
			// use an actor factory if possible
			UActorFactory* ActorFactory = GEditor->FindActorFactory( ActorClass );
			if( ActorFactory )
			{
				return GEditor->UseActorFactory( ActorFactory, FALSE, bUseSurfaceOrientation );
			}
			// otherwise use AddActor so that we can return the newly created actor
			else
			{
				FVector Collision = ActorClass->GetDefaultActor()->GetCylinderExtent();
				return GEditor->AddActor(ActorClass,GEditor->ClickLocation + GEditor->ClickPlane * (FBoxPushOut(GEditor->ClickPlane,Collision) + 0.1));
			}
		}
		return NULL;
	}

	static void ClickActor(FEditorLevelViewportClient* ViewportClient,AActor* Actor,const FViewportClick& Click)
	{
		// Find the point on the actor component which was clicked on.
		FCheckResult Hit(1);
		if(!Actor->ActorLineCheck(Hit,Click.GetOrigin() + Click.GetDirection() * HALF_WORLD_MAX,Click.GetOrigin(),FVector(0,0,0),TRACE_ComplexCollision))
		{	
			GEditor->ClickLocation = Hit.Location;
			GEditor->ClickPlane = FPlane(Hit.Location,Hit.Normal);
		}

		// Pivot snapping
		if( Click.GetKey() == KEY_MiddleMouseButton && Click.IsAltDown() )
		{
			GEditor->ClickLocation = Click.GetOrigin() + Click.GetDirection() * HALF_WORLD_MAX;
			GEditor->SetPivot( GEditor->ClickLocation, TRUE, Click.IsShiftDown(), FALSE );
		}
		// Handle selection.
		else if( Click.GetKey() == KEY_RightMouseButton && !Click.IsControlDown() && !ViewportClient->Viewport->KeyState(KEY_LeftMouseButton) )
		{
			UBOOL bNeedViewportRefresh = FALSE;
			{
				const FScopedTransaction Transaction( TEXT("Clicking on Actors (context menu)") );

				GEditor->GetSelectedActors()->Modify();

				// If the actor the user clicked on was already selected, then we won't bother clearing the selection
				if( !Actor->IsSelected() )
				{
					GEditor->SelectNone( FALSE, TRUE );
					bNeedViewportRefresh = TRUE;
				}

				// Select the actor the user clicked on
				GEditor->SelectActor( Actor, TRUE, NULL, TRUE );
			}

			if( bNeedViewportRefresh )
			{
				// Redraw the viewport so the user can see which object was right clicked on
				ViewportClient->Viewport->Draw();
				FlushRenderingCommands();
			}

			// Display the context menu for the selected actor(s)
			GEditor->ShowUnrealEdContextMenu();
		}
		else if( Click.GetEvent() == IE_DoubleClick && Click.GetKey() == KEY_LeftMouseButton && !Click.IsControlDown() )
		{
			{
				const FScopedTransaction Transaction( TEXT("Clicking on Actors (context menu)") );

				GEditor->GetSelectedActors()->Modify();

				// Clear the selection
				GEditor->SelectNone( FALSE, TRUE );

				// Select the actor the user clicked on
				GEditor->SelectActor( Actor, TRUE, NULL, TRUE );
			}

			// Display the actor properties window
			GEditor->Exec( TEXT("EDCALLBACK ACTORPROPS") );
		}
		else if( Click.GetKey() != KEY_RightMouseButton )
		{
			if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_L) )
			{
				// If shift is down, we pick a color from under the mouse in the viewport and create a light with that color.
				if(Click.IsControlDown())
				{
					PickColorAndAddLight(ViewportClient->Viewport, Click);
				}
				else
				{
					// Create a point light.
					PrivateAddActor( APointLight::StaticClass(), FALSE );
				}
			}
			else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_D) )
			{
				// Create a decal.
				PrivateAddActor( ADecalActor::StaticClass(), TRUE );
			}
			else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_S) )
			{
				// Create a static mesh.
				PrivateAddActor( AStaticMeshActor::StaticClass(), FALSE );
			}
			else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_A) )
			{
				// Create an actor of the selected class.
				UClass* SelectedClass = GEditor->GetSelectedObjects()->GetTop<UClass>();
				if( SelectedClass )
				{
					PrivateAddActor( SelectedClass, FALSE );
				}
			}
			else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_Period) )
			{
				// Create a pathnode.
				PrivateAddActor( APathNode::StaticClass(), FALSE );
			}
			else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_Comma) )
			{
				// Create a coverlink.
				ACoverLink *Link = Cast<ACoverLink>(PrivateAddActor( ACoverLink::StaticClass(), FALSE ));
				if (Click.IsControlDown() && Link != NULL)
				{
					// have the link automatically setup
					Link->EditorAutoSetup(Click.GetDirection());
				}
			}
			else
			{
				const FScopedTransaction Transaction( TEXT("Clicking on Actors") );
				GEditor->GetSelectedActors()->Modify();

				if( Click.IsControlDown() )
				{
					GEditor->SelectActor( Actor, !Actor->IsSelected(), NULL, TRUE );
				}
				else
				{
					GEditor->SelectNone( FALSE, TRUE );
					GEditor->SelectActor( Actor, TRUE, NULL, TRUE );
				}
			}
		}
	}

	static void ClickBrushVertex(FEditorLevelViewportClient* ViewportClient,ABrush* InBrush,FVector* InVertex,const FViewportClick& Click)
	{
		// Pivot snapping
		if( Click.GetKey() == KEY_MiddleMouseButton && Click.IsAltDown() )
		{
			GEditor->ClickLocation = Click.GetOrigin() + Click.GetDirection() * HALF_WORLD_MAX;
			GEditor->SetPivot( GEditor->ClickLocation, TRUE, Click.IsShiftDown(), FALSE );
		}
		else if( Click.GetKey() == KEY_RightMouseButton )
		{
			const FScopedTransaction Transaction( TEXT("Clicking on Brush Vertex") );
			GEditor->SetPivot( InBrush->LocalToWorld().TransformFVector(*InVertex), FALSE, FALSE, FALSE );

			FEdMode* mode = GEditorModeTools().GetCurrentMode();

			const FVector World = InBrush->LocalToWorld().TransformFVector(*InVertex);
			FVector Snapped = World;
			GEditor->Constraints.Snap( Snapped, FVector(GEditor->Constraints.GetGridSize()) );
			const FVector Delta = Snapped - World;
			GEditor->SetPivot( Snapped, FALSE, FALSE, FALSE );

			switch( mode->GetID() )
			{
				case EM_Default:
				{
					// All selected actors need to move by the delta.
					for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
					{
						AActor* Actor = static_cast<AActor*>( *It );
						checkSlow( Actor->IsA(AActor::StaticClass()) );

						Actor->Modify();
						Actor->Location += Delta;
						Actor->ForceUpdateComponents();
					}
				}
				break;
			}
			ViewportClient->Invalidate( TRUE, TRUE );
		}
	}

	static void ClickStaticMeshVertex(FEditorLevelViewportClient* ViewportClient,AActor* InActor,FVector& InVertex,const FViewportClick& Click)
	{
		// Pivot snapping
		if( Click.GetKey() == KEY_MiddleMouseButton && Click.IsAltDown() )
		{
			GEditor->ClickLocation = Click.GetOrigin() + Click.GetDirection() * HALF_WORLD_MAX;
			GEditor->SetPivot( GEditor->ClickLocation, TRUE, Click.IsShiftDown(), FALSE );
		}
		else if( Click.GetKey() == KEY_RightMouseButton )
		{
			const FScopedTransaction Transaction( TEXT("Clicking on Static Mesh Vertex") );

			FVector Snapped = InVertex;
			GEditor->Constraints.Snap( Snapped, FVector(GEditor->Constraints.GetGridSize()) );
			const FVector Delta = Snapped - InVertex;
			GEditor->SetPivot( Snapped, FALSE, FALSE, TRUE );

			// All selected actors need to move by the delta.
			for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
			{
				AActor* Actor = static_cast<AActor*>( *It );
				checkSlow( Actor->IsA(AActor::StaticClass()) );

				Actor->Modify();
				Actor->Location += Delta;
				Actor->ForceUpdateComponents();
			}

			ViewportClient->Invalidate( TRUE, TRUE );
		}
	}

	static void ClickGeomPoly(FEditorLevelViewportClient* ViewportClient, HGeomPolyProxy* InHitProxy, const FViewportClick& Click)
	{
		// Pivot snapping
		if( Click.GetKey() == KEY_MiddleMouseButton && Click.IsAltDown() )
		{
			GEditor->ClickLocation = Click.GetOrigin() + Click.GetDirection() * HALF_WORLD_MAX;
			GEditor->SetPivot( GEditor->ClickLocation, TRUE, Click.IsShiftDown(), FALSE );
		}
		else if( Click.GetKey() == KEY_LeftMouseButton )
		{
			FEdMode* mode = GEditorModeTools().GetCurrentMode();
			mode->GetCurrentTool()->StartTrans();

			if( !Click.IsControlDown() )
			{
				mode->GetCurrentTool()->SelectNone();
			}

			FGeomPoly& gp = InHitProxy->GeomObject->PolyPool( InHitProxy->PolyIndex );
			gp.Select( Click.IsControlDown() ? !gp.IsSelected() : TRUE );

			mode->GetCurrentTool()->EndTrans();
			ViewportClient->Invalidate( TRUE, TRUE );
		}
	}

	/**
	 * Utility method used by ClickGeomEdge and ClickGeomVertex.  Returns TRUE if the projections of
	 * the vectors onto the specified viewport plane are equal within the given tolerance.
	 */
	static UBOOL OrthoEqual(ELevelViewportType ViewportType, const FVector& Vec0, const FVector& Vec1, FLOAT Tolerance=0.1f)
	{
		UBOOL bResult = FALSE;
		switch( ViewportType )
		{
			case LVT_OrthoXY:	bResult = Abs(Vec0.X - Vec1.X) < Tolerance && Abs(Vec0.Y - Vec1.Y) < Tolerance;	break;
			case LVT_OrthoXZ:	bResult = Abs(Vec0.X - Vec1.X) < Tolerance && Abs(Vec0.Z - Vec1.Z) < Tolerance;	break;
			case LVT_OrthoYZ:	bResult = Abs(Vec0.Y - Vec1.Y) < Tolerance && Abs(Vec0.Z - Vec1.Z) < Tolerance;	break;
			default:			check( 0 );		break;
		}
		return bResult;
	}

	static void ClickGeomEdge(FEditorLevelViewportClient* ViewportClient, HGeomEdgeProxy* InHitProxy, const FViewportClick& Click)
	{
		// Pivot snapping
		if( Click.GetKey() == KEY_MiddleMouseButton && Click.IsAltDown() )
		{
			GEditor->ClickLocation = Click.GetOrigin() + Click.GetDirection() * HALF_WORLD_MAX;
			GEditor->SetPivot( GEditor->ClickLocation, TRUE, Click.IsShiftDown(), FALSE );
		}
		else if( Click.GetKey() == KEY_LeftMouseButton )
		{
			FEdMode* mode = GEditorModeTools().GetCurrentMode();
			mode->GetCurrentTool()->StartTrans();

			const UBOOL bControlDown = Click.IsControlDown();
			if( !bControlDown )
			{
				mode->GetCurrentTool()->SelectNone();
			}

			FGeomEdge& HitEdge = InHitProxy->GeomObject->EdgePool( InHitProxy->EdgeIndex );
			HitEdge.Select( bControlDown ? !HitEdge.IsSelected() : TRUE );

			if( ViewportClient->IsOrtho() )
			{
				// Select all edges in the brush that match the projected mid point of the original edge.
				for( INT EdgeIndex = 0 ; EdgeIndex < InHitProxy->GeomObject->EdgePool.Num() ; ++EdgeIndex )
				{
					if ( EdgeIndex != InHitProxy->EdgeIndex )
					{
						FGeomEdge& GeomEdge = InHitProxy->GeomObject->EdgePool( EdgeIndex );
						if ( OrthoEqual( ViewportClient->ViewportType, GeomEdge.GetMid(), HitEdge.GetMid() ) )
						{
							GeomEdge.Select( bControlDown ? !GeomEdge.IsSelected() : TRUE );
						}
					}
				}
			}

			mode->GetCurrentTool()->EndTrans();
			ViewportClient->Invalidate( TRUE, TRUE );
		}
	}

	static void ClickGeomVertex(FEditorLevelViewportClient* ViewportClient,HGeomVertexProxy* InHitProxy,const FViewportClick& Click)
	{
		check( GEditorModeTools().GetCurrentModeID() == EM_Geometry );

		FEdModeGeometry* mode = static_cast<FEdModeGeometry*>( GEditorModeTools().GetCurrentMode() );
		UBOOL bInvalidateViewport = FALSE;

		// Note: The expected behavior is that right clicking on a vertex will snap the vertex that was
		// right-clicked on to the nearest grid point, then move all SELECTED verts by the appropriate
		// delta.  So, we need to handle the right mouse button click BEFORE we change the selection set below.
		if( Click.GetKey() == KEY_RightMouseButton )
		{
			FModeTool_GeometryModify* Tool = static_cast<FModeTool_GeometryModify*>( mode->GetCurrentTool() );
			Tool->GetCurrentModifier()->StartTrans();

			// Compute out far to move to get back on the grid.
			const FVector WorldLoc = InHitProxy->GeomObject->GetActualBrush()->LocalToWorld().TransformFVector( InHitProxy->GeomObject->VertexPool( InHitProxy->VertexIndex ) );

			FVector SnappedLoc = WorldLoc;
			GEditor->Constraints.Snap( SnappedLoc, FVector(GEditor->Constraints.GetGridSize()) );

			const FVector Delta = SnappedLoc - WorldLoc;
			GEditor->SetPivot( SnappedLoc, FALSE, FALSE, FALSE );

			for( INT VertexIndex = 0 ; VertexIndex < InHitProxy->GeomObject->VertexPool.Num() ; ++VertexIndex )
			{
				FGeomVertex& GeomVertex = InHitProxy->GeomObject->VertexPool(VertexIndex);
				if( GeomVertex.IsSelected() )
				{
					GeomVertex += Delta;
				}
			}

			Tool->GetCurrentModifier()->EndTrans();
			InHitProxy->GeomObject->SendToSource();
			bInvalidateViewport = TRUE;
		}

		// Pivot snapping
		if( Click.GetKey() == KEY_MiddleMouseButton && Click.IsAltDown() )
		{
			GEditor->ClickLocation = Click.GetOrigin() + Click.GetDirection() * HALF_WORLD_MAX;
			GEditor->SetPivot( GEditor->ClickLocation, TRUE, Click.IsShiftDown(), FALSE );
		}
		else if( Click.GetKey() == KEY_LeftMouseButton || Click.GetKey() == KEY_RightMouseButton )
		{
			mode->GetCurrentTool()->StartTrans();

			// Disable Ctrl+clicking for selection if selecting with RMB.
			const UBOOL bControlDown = Click.IsControlDown();
			if( !bControlDown )
			{
				mode->GetCurrentTool()->SelectNone();
			}

			FGeomVertex& HitVertex = InHitProxy->GeomObject->VertexPool( InHitProxy->VertexIndex );
			if( Click.GetKey() == KEY_RightMouseButton )
			{
				HitVertex.Select( TRUE );
			}
			else
			{
				HitVertex.Select( bControlDown ? !HitVertex.IsFullySelected() : TRUE );
			}

			if( ViewportClient->IsOrtho() )
			{
				// Select all vertices that project to the same location.
				for( INT VertexIndex = 0 ; VertexIndex < InHitProxy->GeomObject->VertexPool.Num() ; ++VertexIndex )
				{
					if ( VertexIndex != InHitProxy->VertexIndex )
					{
						FGeomVertex& GeomVertex = InHitProxy->GeomObject->VertexPool(VertexIndex);
						if ( OrthoEqual( ViewportClient->ViewportType, GeomVertex, HitVertex ) )
						{
							if( Click.GetKey() == KEY_RightMouseButton )
							{
								GeomVertex.Select( TRUE );
							}
							else
							{
								GeomVertex.Select( bControlDown ? !GeomVertex.IsSelected() : TRUE );
							}
						}
					}
				}
			}

			mode->GetCurrentTool()->EndTrans();

			bInvalidateViewport = TRUE;
		}


		if ( bInvalidateViewport )
		{
			ViewportClient->Invalidate( TRUE, TRUE );
		}
	}


	static FBspSurf GSaveSurf;

	static void ClickSurface(FEditorLevelViewportClient* ViewportClient,UModel* Model,INT iSurf,const FViewportClick& Click)
	{
		// Gizmos can cause BSP surfs to become selected without this check
		if(Click.GetKey() == KEY_RightMouseButton && Click.IsControlDown())
		{
			return;
		}

		// Remember hit location for actor-adding.
		FBspSurf& Surf			= Model->Surfs(iSurf);
		const FPlane Plane		= Surf.Plane;
		GEditor->ClickLocation	= FLinePlaneIntersection( Click.GetOrigin(), Click.GetOrigin() + Click.GetDirection(), Plane );
		GEditor->ClickPlane		= Plane;

		// Pivot snapping
		if( Click.GetKey() == KEY_MiddleMouseButton && Click.IsAltDown() )
		{
			GEditor->ClickLocation = Click.GetOrigin() + Click.GetDirection() * HALF_WORLD_MAX;
			GEditor->SetPivot( GEditor->ClickLocation, TRUE, Click.IsShiftDown(), FALSE );
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && Click.IsShiftDown() && Click.IsControlDown() )
		{
			// Add to the actor selection set the brush actor that belongs to this BSP surface.
			// Check Surf.Actor, as it can be NULL after deleting brushes and before rebuilding BSP.
			if( Surf.Actor )
			{
				const FScopedTransaction Transaction( TEXT("Select Brush from Surface") );

				// If the builder brush is selected, first deselect it.
				USelection* SelectedActors = GEditor->GetSelectedActors();
				for ( USelection::TObjectIterator It( SelectedActors->ObjectItor() ) ; It ; ++It )
				{
					ABrush* Brush = Cast<ABrush>( *It );
					if ( Brush && Brush->IsCurrentBuilderBrush() )
					{
						GEditor->SelectActor( Brush, FALSE, NULL, FALSE );
						break;
					}
				}

				GEditor->SelectActor( Surf.Actor, TRUE, NULL, TRUE );
			}
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && Click.IsShiftDown() )
		{
			// Apply texture to all selected.
			const FScopedTransaction Transaction( TEXT("Apply Material to Selected Surfaces") );
			UMaterialInterface* SelectedMaterialInstance = GEditor->GetSelectedObjects()->GetTop<UMaterialInterface>();
			for( INT i=0; i<Model->Surfs.Num(); i++ )
			{
				if( Model->Surfs(i).PolyFlags & PF_Selected )
				{
					Model->ModifySurf( i, 1 );
					Model->Surfs(i).Material = SelectedMaterialInstance;
					GEditor->polyUpdateMaster( Model, i, 0 );
				}
			}
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_A) )
		{
			// Create an actor of the selected class.
			UClass* SelectedClass = GEditor->GetSelectedObjects()->GetTop<UClass>();
			if( SelectedClass )
			{
				PrivateAddActor( SelectedClass, FALSE );
			}
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_L) )
		{
			// If shift is down, we pick a color from under the mouse in the viewport and create a light with that color.
			if(Click.IsControlDown())
			{
				PickColorAndAddLight(ViewportClient->Viewport, Click);
			}
			else
			{
				// Create a point light.
				PrivateAddActor( APointLight::StaticClass(), FALSE );
			}
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_D) )
		{
			// Create a decal.
			PrivateAddActor( ADecalActor::StaticClass(), TRUE );
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_S) )
		{
			// Create a static mesh.
			PrivateAddActor( AStaticMeshActor::StaticClass(), FALSE );
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_Period) )
		{
			// Create a pathnode.
			PrivateAddActor( APathNode::StaticClass(), FALSE );
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_Comma) )
		{
			// Create a coverlink.
			ACoverLink *Link = Cast<ACoverLink>(PrivateAddActor( ACoverLink::StaticClass(), FALSE ));
			if (Click.IsControlDown() && Link != NULL)
			{
				// have the link automatically setup
				FVector HitL(GEditor->ClickLocation), HitN(Plane);
				Link->EditorAutoSetup(Click.GetDirection(),&HitL,&HitN);
			}
		}
		else if( Click.IsAltDown() && Click.GetKey() == KEY_RightMouseButton )
		{
			// Grab the texture.
			GEditor->GetSelectedObjects()->SelectNone(UMaterialInterface::StaticClass());
			if(Surf.Material)
			{
				GEditor->GetSelectedObjects()->Select(Surf.Material);
			}
			GSaveSurf = Surf;
		}
		else if( Click.IsAltDown() && Click.GetKey() == KEY_LeftMouseButton)
		{
			// Apply texture to the one polygon clicked on.
			const FScopedTransaction Transaction( TEXT("Apply Material to Surface") );
			Model->ModifySurf( iSurf, TRUE );
			Surf.Material = GEditor->GetSelectedObjects()->GetTop<UMaterialInterface>();
			if( Click.IsControlDown() )
			{
				Surf.vTextureU	= GSaveSurf.vTextureU;
				Surf.vTextureV	= GSaveSurf.vTextureV;
				if( Surf.vNormal == GSaveSurf.vNormal )
				{
					GLog->Logf( TEXT("WARNING: the texture coordinates were not parallel to the surface.") );
				}
				Surf.PolyFlags	= GSaveSurf.PolyFlags;
				GEditor->polyUpdateMaster( Model, iSurf, 1 );
			}
			else
			{
				GEditor->polyUpdateMaster( Model, iSurf, 0 );
			}
		}
		else if( Click.GetKey() == KEY_RightMouseButton && !Click.IsControlDown() )
		{
			// Select surface and display context menu
			check( Model );

			UBOOL bNeedViewportRefresh = FALSE;
			{
				const FScopedTransaction Transaction( TEXT("Select Surfaces") );

				// We only need to unselect surfaces if the surface the user clicked on was not already selected
				if( !( Surf.PolyFlags & PF_Selected ) )
				{
					GEditor->SelectNone( FALSE, TRUE );
					bNeedViewportRefresh = TRUE;
				}

				// Select the surface the user clicked on
				Model->ModifySurf( iSurf, FALSE );
				Surf.PolyFlags |= PF_Selected;

				GEditor->NoteSelectionChange();
			}

			if( bNeedViewportRefresh )
			{
				// Redraw the viewport so the user can see which object was right clicked on
				ViewportClient->Viewport->Draw();
				FlushRenderingCommands();
			}

			// Display the context menu for the selected surface(s)
			GEditor->ShowUnrealEdContextSurfaceMenu();	
		}
		else
		{
			// Select or deselect surfaces.
			{
				const FScopedTransaction Transaction( TEXT("Select Surfaces") );
				const DWORD SelectMask = Surf.PolyFlags & PF_Selected;
				if( !Click.IsControlDown() )
				{
					GEditor->SelectNone( FALSE, TRUE );
				}
				Model->ModifySurf( iSurf, FALSE );
				Surf.PolyFlags = (Surf.PolyFlags & ~PF_Selected) | (SelectMask ^ PF_Selected);
			}
			GEditor->NoteSelectionChange();
		}

	}

	static void ClickBackdrop(FEditorLevelViewportClient* ViewportClient,const FViewportClick& Click)
	{
		GEditor->ClickLocation = Click.GetOrigin() + Click.GetDirection() * HALF_WORLD_MAX;
		GEditor->ClickPlane    = FPlane(0,0,0,0);

		// Pivot snapping
		if( Click.GetKey() == KEY_MiddleMouseButton && Click.IsAltDown() )
		{
			GEditor->ClickLocation = Click.GetOrigin() + Click.GetDirection() * HALF_WORLD_MAX;
			GEditor->SetPivot( GEditor->ClickLocation, TRUE, Click.IsShiftDown(), FALSE );
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_A) )
		{
			// Create an actor of the selected class.
			UClass* SelectedClass = GEditor->GetSelectedObjects()->GetTop<UClass>();
			if( SelectedClass )
			{
				PrivateAddActor( SelectedClass, FALSE );
			}
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_L) )
		{
			// If shift is down, we pick a color from under the mouse in the viewport and create a light with that color.
			if(Click.IsControlDown())
			{
				PickColorAndAddLight(ViewportClient->Viewport, Click);
			}
			else
			{
				// Create a point light.
				PrivateAddActor( APointLight::StaticClass(), FALSE );
			}
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_D) )
		{
			// Create a decal.
			PrivateAddActor( ADecalActor::StaticClass(), FALSE );
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_S) )
		{
			// Create a static mesh.
			PrivateAddActor( AStaticMeshActor::StaticClass(), FALSE );
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_Period) )
		{
			// Create a pathnode.
			PrivateAddActor( APathNode::StaticClass(), FALSE );
		}
		else if( Click.GetKey() == KEY_LeftMouseButton && ViewportClient->Viewport->KeyState(KEY_Comma) )
		{
			// Create a coverlink.
			ACoverLink *Link = Cast<ACoverLink>(PrivateAddActor( ACoverLink::StaticClass(), FALSE ));
			if (Click.IsControlDown() && Link != NULL)
			{
				// have the link automatically setup
				Link->EditorAutoSetup(Click.GetDirection());
			}
		}
		else if( Click.GetKey() == KEY_RightMouseButton && !Click.IsControlDown() && !ViewportClient->Viewport->KeyState(KEY_LeftMouseButton) )
		{
			// Redraw the viewport so the user can see which object was right clicked on
			ViewportClient->Viewport->Draw();
			FlushRenderingCommands();

			GEditor->ShowUnrealEdContextMenu();
		}
		else if( Click.GetKey() == KEY_LeftMouseButton )
		{
			if( !Click.IsControlDown() )
			{
				const FScopedTransaction Transaction( TEXT("Select None") );
				GEditor->SelectNone( TRUE, TRUE );
			}
		}
	}
}

//
//	FEditorLevelViewportClient::FEditorLevelViewportClient
//
/**
 * Constructor
 *
 * @param	ViewportInputClass	the input class to use for creating this viewport's input object
 */

FEditorLevelViewportClient::FEditorLevelViewportClient( UClass* ViewportInputClass/*=NULL*/ ):
	Viewport(NULL),	
	ViewLocation( 0.0f, 0.0f, 0.0f ),
	ViewRotation( 0.0f, 0.0f, 0.0f ),
	ViewFOV(GEditor->FOVAngle),
	ViewportType(LVT_Perspective),

	OrthoZoom(DEFAULT_ORTHOZOOM),
	
	ViewState(AllocateViewState()),

	ShowFlags((SHOW_DefaultEditor&~SHOW_ViewMode_Mask)|SHOW_ViewMode_Lit),
	CachedShowFlags(0),
	LastShowFlags(SHOW_DefaultEditor),
	LastSpecialMode(0),
	bMoveUnlit(FALSE),
	bLevelStreamingVolumePrevis(FALSE),
	bPostProcessVolumePrevis(TRUE),
	bLockSelectedToCamera(FALSE),
	bViewportLocked(FALSE),
	bVariableFarPlane(FALSE),

	bAllowMayaCam(FALSE),
	bUsingMayaCam(FALSE),
	MayaRotation(0,0,0),
	MayaLookAt(0,0,0),
	MayaZoom(0,0,0),
	MayaActor(NULL),

	bDrawAxes(TRUE),
	bDrawVertices(FALSE),
	bDuplicateActorsOnNextDrag( FALSE ),
	bDuplicateActorsInProgress( FALSE ),
	bIsTracking( FALSE ),
	bDraggingByHandle( FALSE ),
    
	bConstrainAspectRatio(FALSE),
	AspectRatio(1.777777f),

	NearPlane(NEAR_CLIPPING_PLANE),

	bAllowAmbientOcclusion(FALSE),
	bOverridePostProcessSettings(FALSE),
	OverrideProcessSettings(0),
	bAdditivePostProcessSettings(FALSE),

	bEnableFading(FALSE),
	FadeAmount(0.f),
	FadeColor( FColor(0,0,0) ),

	bEnableColorScaling(FALSE),
	ColorScale( FVector(1,1,1) ),

	bAudioRealTime(FALSE),
	bSetListenerPosition(FALSE),

	NumPendingRedraws(0),

	CameraSpeed(MOVEMENTSPEED_SLOW),

	bIsRealtime(FALSE),
	bStoredRealtime(FALSE),
	bUseSquintMode(FALSE),

	bIsFloatingViewport(FALSE),
	bAllowMatineePreview(FALSE),

	CameraController( NULL ),
	CameraUserImpulseData( NULL ),
	FlightCameraSpeedScale( 1.0f ),
	RealTimeFlightCameraSpeedScaleChanged( -10000.0 )
{
	if ( ViewportInputClass == NULL )
	{
		ViewportInputClass = UEditorViewportInput::StaticClass();
	}
	else
	{
		checkSlow(ViewportInputClass->IsChildOf(UEditorViewportInput::StaticClass()));

		// ensure that this class is fully loaded
		LoadClass<UEditorViewportInput>(ViewportInputClass->GetOuter(), *ViewportInputClass->GetName(), NULL, LOAD_None, NULL);
	}

	CameraController = new FEditorCameraController();
	CameraUserImpulseData = new FCameraControllerUserImpulseData();

	Input = ConstructObject<UEditorViewportInput>(ViewportInputClass);
	Input->Editor = GEditor;
	Widget = new FWidget;
	MouseDeltaTracker = new FMouseDeltaTracker;
	GEditor->ViewportClients.AddItem(this);

}

//
//	FEditorLevelViewportClient::~FEditorLevelViewportClient
//

FEditorLevelViewportClient::~FEditorLevelViewportClient()
{
	ViewState->Destroy();
	ViewState = NULL;

	delete Widget;
	delete MouseDeltaTracker;

	delete CameraController;
	CameraController = NULL;

	delete CameraUserImpulseData;
	CameraUserImpulseData = NULL;


	if(Viewport)
	{
		appErrorf(*LocalizeUnrealEd("Error_ViewportNotNULL"));
	}

	GEditor->ViewportClients.RemoveItem(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FEditorLevelViewportClient::SetViewLocationForOrbiting(const FVector& Loc)
{
	FMatrix EpicMat = FTranslationMatrix(-ViewLocation);
	EpicMat = EpicMat * FInverseRotationMatrix(ViewRotation);
	FMatrix CamRotMat = EpicMat.Inverse();
	FVector CamDir = FVector(CamRotMat.M[0][0],CamRotMat.M[0][1],CamRotMat.M[0][2]);
	ViewLocation = Loc - 256 * CamDir;
}

void FEditorLevelViewportClient::SwitchStandardToMaya()
{
	FMatrix EpicMat = FTranslationMatrix(-ViewLocation);
	EpicMat = EpicMat * FInverseRotationMatrix(ViewRotation);

	FMatrix MayaMat = EpicMat * 
		FRotationMatrix( FRotator(0,16384,0) ) * FTranslationMatrix( FVector(0,-256,0) );

	MayaZoom = FVector(0,0,0);

	FMatrix CamRotMat = EpicMat.Inverse();
	FVector CamDir = FVector(CamRotMat.M[0][0],CamRotMat.M[0][1],CamRotMat.M[0][2]);
	MayaLookAt = ViewLocation + 256 * CamDir;

	FMatrix RotMat = FTranslationMatrix( -MayaLookAt ) * MayaMat;
	FMatrix RotMatInv = RotMat.Inverse();
	FRotator RollVec = RotMatInv.Rotator();
	FMatrix YawMat = RotMatInv * FInverseRotationMatrix( FRotator(0, 0, -RollVec.Roll));
	FMatrix YawMatInv = YawMat.Inverse();
	FRotator YawVec = YawMat.Rotator();
	FRotator rotYawInv = YawMatInv.Rotator();
	MayaRotation = FRotator(-RollVec.Roll,-YawVec.Yaw,0);

	bUsingMayaCam = TRUE;
}

void FEditorLevelViewportClient::SwitchMayaToStandard()
{
	FMatrix MayaMat =
		FTranslationMatrix( -MayaLookAt ) *
		FRotationMatrix( FRotator(0,MayaRotation.Yaw,0) ) * 
		FRotationMatrix( FRotator(0, 0, MayaRotation.Pitch)) *
		FTranslationMatrix( MayaZoom ) *
		FTranslationMatrix( FVector(0,256,0) ) *
		FInverseRotationMatrix( FRotator(0,16384,0) );
	MayaMat =  MayaMat.Inverse();

	ViewRotation = MayaMat.Rotator();
	ViewLocation = MayaMat.GetOrigin();

	bUsingMayaCam = FALSE;
}

void FEditorLevelViewportClient::SwitchMayaCam()
{
	if ( bAllowMayaCam )
	{
		if ( GEditor->bUseMayaCameraControls && !bUsingMayaCam )
		{
			SwitchStandardToMaya();
		}
		else if ( !GEditor->bUseMayaCameraControls && bUsingMayaCam )
		{
			SwitchMayaToStandard();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
// Configures the specified FSceneView object with the view and projection matrices for this viewport. 

FSceneView* FEditorLevelViewportClient::CalcSceneView(FSceneViewFamily* ViewFamily)
{
	UPostProcessChain* LevelViewPostProcess = NULL;

	FMatrix ViewMatrix = FTranslationMatrix(-ViewLocation);	

	if(ViewportType == LVT_OrthoXY)
	{
		ViewMatrix = ViewMatrix * FMatrix(
			FPlane(0,	1,	0,					0),
			FPlane(1,	0,	0,					0),
			FPlane(0,	0,	-1,					0),
			FPlane(0,	0,	-ViewLocation.Z,	1));
	}
	else if(ViewportType == LVT_OrthoXZ)
	{
		ViewMatrix = ViewMatrix * FMatrix(
			FPlane(1,	0,	0,					0),
			FPlane(0,	0,	-1,					0),
			FPlane(0,	1,	0,					0),
			FPlane(0,	0,	-ViewLocation.Y,	1));
	}
	else if(ViewportType == LVT_OrthoYZ)
	{
		ViewMatrix = ViewMatrix * FMatrix(
			FPlane(0,	0,	1,					0),
			FPlane(1,	0,	0,					0),
			FPlane(0,	1,	0,					0),
			FPlane(0,	0,	ViewLocation.X,		1));
	}

	FMatrix ProjectionMatrix;

	INT X = 0;
	INT Y = 0;
	UINT SizeX = Viewport->GetSizeX();
	UINT SizeY = Viewport->GetSizeY();

	if(ViewportType == LVT_Perspective)
	{
		// only use post process chain for the perspective view
		LevelViewPostProcess = GEngine->DefaultPostProcess;

		if (GEditor->bIsPushingView)
		{
			GEngine->Exec(*FString::Printf(TEXT("REMOTE PUSHVIEW %f %f %f %d %d %d"), ViewLocation.X, ViewLocation.Y, ViewLocation.Z, ViewRotation.Pitch, ViewRotation.Yaw, ViewRotation.Roll), *GNull);
		}

		SwitchMayaCam();

		if ( bAllowMayaCam && GEditor->bUseMayaCameraControls )
		{
			ViewMatrix =
				FTranslationMatrix( -MayaLookAt ) *
				FRotationMatrix( FRotator(0,MayaRotation.Yaw,0) ) * 
				FRotationMatrix( FRotator(0, 0, MayaRotation.Pitch)) *
				FTranslationMatrix( MayaZoom ) *
				FTranslationMatrix( FVector(0,256,0) ) *
				FInverseRotationMatrix( FRotator(0,16384,0) );

			FMatrix MayaMat =  ViewMatrix.Inverse();
			ViewRotation = MayaMat.Rotator();
			ViewLocation = MayaMat.GetOrigin();
		}
		else
		{
			ViewMatrix = ViewMatrix * FInverseRotationMatrix(ViewRotation);
		}

		ViewMatrix = ViewMatrix * FMatrix(
			FPlane(0,	0,	1,	0),
			FPlane(1,	0,	0,	0),
			FPlane(0,	1,	0,	0),
			FPlane(0,	0,	0,	1));

		if ( bVariableFarPlane && GEditor->FarClippingPlane > NEAR_CLIPPING_PLANE )
		{
			if( bConstrainAspectRatio )
			{
				ProjectionMatrix = FPerspectiveMatrix(
					ViewFOV * (FLOAT)PI / 360.0f,
					AspectRatio,
					1.0f,
					NearPlane,
					GEditor->FarClippingPlane
					);

				Viewport->CalculateViewExtents( AspectRatio, X, Y, SizeX, SizeY );
			}
			else
			{
				ProjectionMatrix = FPerspectiveMatrix(
					ViewFOV * (FLOAT)PI / 360.0f,
					SizeX,
					SizeY,
					NearPlane,
					GEditor->FarClippingPlane
					);
			}
		}
		else
		{
			if( bConstrainAspectRatio )
			{
				ProjectionMatrix = FPerspectiveMatrix(
					ViewFOV * (FLOAT)PI / 360.0f,
					AspectRatio,
					1.0f,
					NearPlane
					);

				Viewport->CalculateViewExtents( AspectRatio, X, Y, SizeX, SizeY );
			}
			else
			{
				ProjectionMatrix = FPerspectiveMatrix(
					ViewFOV * (FLOAT)PI / 360.0f,
					SizeX,
					SizeY,
					NearPlane
					);
			}
		}
	}
	else
	{
		const FLOAT	Zoom = OrthoZoom / (SizeX * 15.0f);
		ProjectionMatrix = FOrthoMatrix(
			Zoom * SizeX / 2.0f,
			Zoom * SizeY / 2.0f,
			0.5f / HALF_WORLD_MAX,
			HALF_WORLD_MAX
			);
	}

	// See if we want to specify the PP settings, or use those from the level.
	if(bOverridePostProcessSettings)
	{
		PostProcessSettings = OverrideProcessSettings;
	}
	else if(GetScene() == GWorld->Scene)
	{
		// Find the post-process settings for the view.
		GWorld->GetWorldInfo()->GetPostProcessSettings(
			ViewLocation,
			bPostProcessVolumePrevis,
			PostProcessSettings
			);
	}
	else
	{
		// If this isn't a level viewport, use the default post process settings.
		PostProcessSettings = GetDefault<AWorldInfo>()->DefaultPostProcessSettings;
	}
	if (bAdditivePostProcessSettings)
	{
		PostProcessSettings.AddInterpolatable(AdditivePostProcessSettings);
	}

	// If in squint mode, override post process settings and enable DOF.
	if ( bUseSquintMode )
	{
		PostProcessSettings.bEnableDOF = TRUE;
		PostProcessSettings.DOF_BlurKernelSize = GWorld->GetWorldInfo()->SquintModeKernelSize;
		PostProcessSettings.DOF_FalloffExponent = 1.0f;
		PostProcessSettings.DOF_FocusDistance = 0.0f;
		PostProcessSettings.DOF_FocusInnerRadius = 0.0f;
		PostProcessSettings.DOF_FocusType = FOCUS_Distance;
		PostProcessSettings.DOF_InterpolationDuration = 0.0f;
		PostProcessSettings.DOF_MaxFarBlurAmount = 1.0f;
		PostProcessSettings.DOF_MaxNearBlurAmount = 1.0f;
		PostProcessSettings.DOF_ModulateBlurColor = FColor(255,255,255,255);
	}

	FSceneView* View = new FSceneView(
		ViewFamily,
		ViewState,
		-1,
		NULL,
		NULL,
		NULL,
		LevelViewPostProcess,
		&PostProcessSettings,
		NULL,
		this,
		X,
		Y,
		SizeX,
		SizeY,
		ViewMatrix,
		ProjectionMatrix,
		GetBackgroundColor(),
		FLinearColor(0,0,0,0),
		FLinearColor::White,
		TSet<UPrimitiveComponent*>()
		);
	ViewFamily->Views.AddItem(View);
	return View;
}

//
//	FEditorLevelViewportClient::ProcessClick
//

void FEditorLevelViewportClient::ProcessClick(FSceneView* View,HHitProxy* HitProxy,FName Key,EInputEvent Event,UINT HitX,UINT HitY)
{
	const FViewportClick Click(View,this,Key,Event,HitX,HitY);
	if (!GEditorModeTools().GetCurrentMode()->HandleClick(HitProxy,Click))
	{
		if (HitProxy == NULL)
		{
			ClickBackdrop(this,Click);
		}
		else if (HitProxy->IsA(HActor::StaticGetType()))
		{
			ClickActor(this,((HActor*)HitProxy)->Actor,Click);
		}
		else if (HitProxy->IsA(HBSPBrushVert::StaticGetType()))
		{
			ClickBrushVertex(this,((HBSPBrushVert*)HitProxy)->Brush,((HBSPBrushVert*)HitProxy)->Vertex,Click);
		}
		else if (HitProxy->IsA(HStaticMeshVert::StaticGetType()))
		{
			ClickStaticMeshVertex(this,((HStaticMeshVert*)HitProxy)->Actor,((HStaticMeshVert*)HitProxy)->Vertex,Click);
		}
		else if (HitProxy->IsA(HGeomPolyProxy::StaticGetType()))
		{
			ClickGeomPoly(this,(HGeomPolyProxy*)HitProxy,Click);
		}
		else if (HitProxy->IsA(HGeomEdgeProxy::StaticGetType()))
		{
			ClickGeomEdge(this,(HGeomEdgeProxy*)HitProxy,Click);
		}
		else if (HitProxy->IsA(HGeomVertexProxy::StaticGetType()))
		{
			ClickGeomVertex(this,(HGeomVertexProxy*)HitProxy,Click);
		}
		else if (HitProxy->IsA(HModel::StaticGetType()))
		{
			HModel* ModelHit = (HModel*)HitProxy;

			// Compute the viewport's current view family.
			FSceneViewFamilyContext ViewFamily(Viewport,GetScene(),ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL);
			FSceneView* View = CalcSceneView( &ViewFamily );

			UINT SurfaceIndex = INDEX_NONE;
			if(ModelHit->ResolveSurface(View,HitX,HitY,SurfaceIndex))
			{
				ClickSurface(this,ModelHit->GetModel(),SurfaceIndex,Click);
			}
		}
		else if (HitProxy->IsA(HLevelSocketProxy::StaticGetType()))
		{
			// Assign to selected actors the socket that was clicked.
			HLevelSocketProxy* SocketProxy = static_cast<HLevelSocketProxy*>( HitProxy );
			for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
			{
				check( SocketProxy->SkelMeshComponent );
				USkeletalMeshSocket* Socket = SocketProxy->SkelMeshComponent->SkeletalMesh->Sockets(SocketProxy->SocketIndex);
				FMatrix SocketTM;
				if( Socket->GetSocketMatrix( SocketTM, SocketProxy->SkelMeshComponent ) )
				{
					AActor* Actor = static_cast<AActor*>( *It );
					checkSlow( Actor->IsA(AActor::StaticClass()) );

					Actor->Location = SocketTM.GetOrigin();
					Actor->Rotation = SocketTM.Rotator();

					Actor->Base = SocketProxy->Actor;
					Actor->BaseSkelComponent = SocketProxy->SkelMeshComponent;
					Actor->BaseBoneName = Socket->BoneName;

					Actor->PreEditChange( NULL );
					Actor->PostEditChange( NULL );

					GEditor->RedrawLevelEditingViewports();
				}
			}
		}
		else if( HitProxy->IsA( HWidgetAxis::StaticGetType() ) )
		{
			// The user clicked on an axis translation/rotation hit proxy.  However, we want
			// to find out what's underneath the axis widget.  To do this, we'll need to render
			// the viewport's hit proxies again, this time *without* the axis widgets!

			// OK, we need to be a bit evil right here.  Basically we want to hijack the ShowFlags
			// for the scene so we can re-render the hit proxies without any axis widgets.  We'll
			// store the original ShowFlags and modify them appropriately
			const EShowFlags OldShowFlags = ShowFlags;
			const EShowFlags OldSCFShowFlags = View->Family->ShowFlags;
			ShowFlags &= ~SHOW_ModeWidgets;
			FSceneViewFamily* SceneViewFamily = const_cast< FSceneViewFamily* >( View->Family );
			SceneViewFamily->ShowFlags &= ~SHOW_ModeWidgets;

			// Invalidate the hit proxy map so it will be rendered out again when GetHitProxy
			// is called
			Viewport->InvalidateHitProxy();

			// This will actually re-render the viewport's hit proxies!
			HHitProxy* HitProxyWithoutAxisWidgets = Viewport->GetHitProxy( HitX, HitY );
			if( HitProxyWithoutAxisWidgets != NULL )
			{
				// We should never encounter a widget axis.  If we do, then something's wrong
				// with our ShowFlags (or the widget drawing code)
				check( !HitProxyWithoutAxisWidgets->IsA( HWidgetAxis::StaticGetType() ) );

				// Try this again, but without the widgets this time!
				ProcessClick( View, HitProxyWithoutAxisWidgets, Key, Event, HitX, HitY );
			}

			// Undo the evil
			ShowFlags = OldShowFlags;
			SceneViewFamily->ShowFlags = OldSCFShowFlags;

			// Invalidate the hit proxy map again so that it'll be refreshed with the original
			// scene contents if we need it again later.
			Viewport->InvalidateHitProxy();
		}
	}
}

// Frustum parameters for the active view parent.
static FLOAT GParentFrustumAngle=90.f;
static FLOAT GParentFrustumAspectRatio=1.77777f;
static FLOAT GParentFrustumStartDist=NEAR_CLIPPING_PLANE;
static FLOAT GParentFrustumEndDist=HALF_WORLD_MAX;
static FMatrix GParentViewMatrix;

//
//	FEditorLevelViewportClient::Tick
//

void FEditorLevelViewportClient::Tick(FLOAT DeltaTime)
{
	// Update any real-time camera movement
	UpdateCameraMovement( DeltaTime );

	UpdateMouseDelta();

	GEditorModeTools().GetCurrentMode()->Tick(this,DeltaTime);

	// Copy perspective views to the global
	if ( ViewState->IsViewParent() )
	{
		GParentFrustumAngle=ViewFOV;
		GParentFrustumAspectRatio=AspectRatio;
		GParentFrustumStartDist=NearPlane;

		GParentFrustumEndDist=( bVariableFarPlane && GEditor->FarClippingPlane > NEAR_CLIPPING_PLANE )
			? GEditor->FarClippingPlane
			: HALF_WORLD_MAX;

		FSceneViewFamilyContext ViewFamily(Viewport,GetScene(),ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL,IsRealtime());
		FSceneView* View = CalcSceneView(&ViewFamily);
		GParentViewMatrix = View->ViewMatrix;
	}
}



/**
 * Updates real-time camera movement.  Should be called every viewport tick!
 *
 * @param	DeltaTime	Time interval in seconds since last update
 */
void FEditorLevelViewportClient::UpdateCameraMovement( FLOAT DeltaTime )
{
	// We only want to move perspective cameras around like this
	if( Viewport != NULL && ViewportType == LVT_Perspective )
	{
		// Certain keys are only available while the flight camera input mode is active
		const UBOOL bUsingFlightInput = IsFlightCameraInputModeActive();

		// Do we want to remap the various WASD keys for flight input?
		const UBOOL bRemapWASDKeys =
			bUsingFlightInput &&
			GEditor->GetUserSettings().AllowFlightCameraToRemapKeys;

		// Do we want to use the regular arrow keys for flight input?
		// Because the arrow keys are used for nudging actors, we'll only do this while the ALT key is up
		const UBOOL bAltDown = Viewport->KeyState( KEY_LeftAlt ) || Viewport->KeyState( KEY_RightAlt );
		const UBOOL bRemapArrowKeys = !bAltDown;

		// Forward/back
		CameraUserImpulseData->MoveForwardBackwardImpulse = 0.0f;
		if( ( bRemapWASDKeys && Viewport->KeyState( KEY_W ) ) ||
			( bRemapArrowKeys && Viewport->KeyState( KEY_Up ) ) ||
			Viewport->KeyState( KEY_NumPadEight ) )
		{
			CameraUserImpulseData->MoveForwardBackwardImpulse += 1.0f;
		}
		if( ( bRemapWASDKeys && Viewport->KeyState( KEY_S ) ) ||
			( bRemapArrowKeys && Viewport->KeyState( KEY_Down ) ) ||
			Viewport->KeyState( KEY_NumPadTwo ) )
		{
			CameraUserImpulseData->MoveForwardBackwardImpulse -= 1.0f;
		}

		// Right/left
		CameraUserImpulseData->MoveRightLeftImpulse = 0.0f;
		if( ( bRemapWASDKeys && Viewport->KeyState( KEY_D ) ) ||
			( bRemapArrowKeys && Viewport->KeyState( KEY_Right ) ) ||
			Viewport->KeyState( KEY_NumPadSix ) )
		{
			CameraUserImpulseData->MoveRightLeftImpulse += 1.0f;
		}
		if( ( bRemapWASDKeys && Viewport->KeyState( KEY_A ) ) ||
			( bRemapArrowKeys && Viewport->KeyState( KEY_Left ) ) ||
			Viewport->KeyState( KEY_NumPadFour ) )
		{
			CameraUserImpulseData->MoveRightLeftImpulse -= 1.0f;
		}

		// Up/down
		CameraUserImpulseData->MoveUpDownImpulse = 0.0f;
		if( ( bRemapWASDKeys && Viewport->KeyState( KEY_E ) ) ||
			Viewport->KeyState( KEY_PageUp ) || Viewport->KeyState( KEY_NumPadNine ) )
		{
			CameraUserImpulseData->MoveUpDownImpulse += 1.0f;
		}
		if( ( bRemapWASDKeys && Viewport->KeyState( KEY_Q ) ) ||
			Viewport->KeyState( KEY_PageDown ) || Viewport->KeyState( KEY_NumPadSeven ) )
		{
			CameraUserImpulseData->MoveUpDownImpulse -= 1.0f;
		}

		// Zoom FOV out/in
		CameraUserImpulseData->ZoomOutInImpulse = 0.0f;
		if( ( bRemapWASDKeys && Viewport->KeyState( KEY_Z ) ) ||
			Viewport->KeyState( KEY_NumPadOne ) )
		{
			CameraUserImpulseData->ZoomOutInImpulse += 1.0f;
		}
		if( ( bRemapWASDKeys && Viewport->KeyState( KEY_C ) ) ||
			Viewport->KeyState( KEY_NumPadThree ) )
		{
			CameraUserImpulseData->ZoomOutInImpulse -= 1.0f;
		}


		FVector ViewEuler = ViewRotation.Euler();
		FVector NewViewLocation = ViewLocation;
		FRotator NewViewRotation = ViewRotation;
		FVector NewViewEuler = ViewEuler;
		FLOAT NewViewFOV = ViewFOV;

		// We'll combine the regular camera speed scale (controlled by viewport toolbar setting) with
		// the flight camera speed scale (controlled by mouse wheel).
		const FLOAT FinalCameraSpeedScale = FlightCameraSpeedScale * ( CameraSpeed / MOVEMENTSPEED_SLOW );

		// Only allow FOV recoil if flight camera mode is currently inactive.
		const UBOOL bAllowRecoilIfNoImpulse = !bUsingFlightInput;

		// Update the camera's position, rotation and FOV
		CameraController->UpdateSimulation(
			*CameraUserImpulseData,
			DeltaTime,
			bAllowRecoilIfNoImpulse,
			FinalCameraSpeedScale,
			NewViewLocation,
			NewViewEuler,
			NewViewFOV );


		// We'll zero out rotation velocity modifier after updating the simulation since these actions
		// are always momentary -- that is, when the user mouse looks some number of pixels,
		// we increment the impulse value right there
		{
			CameraUserImpulseData->RotateYawVelocityModifier = 0.0f;
			CameraUserImpulseData->RotatePitchVelocityModifier = 0.0f;
			CameraUserImpulseData->RotateRollVelocityModifier = 0.0f;
		}


		if( !ViewEuler.Equals( NewViewEuler, SMALL_NUMBER ) )
		{
			NewViewRotation = FRotator::MakeFromEuler( NewViewEuler );
		}

		if( !NewViewLocation.Equals( ViewLocation, SMALL_NUMBER ) ||
			NewViewRotation != ViewRotation ||
			!appIsNearlyEqual( NewViewFOV, ViewFOV, FLOAT(SMALL_NUMBER) ) )
		{
			// Something has changed!
			Invalidate();

			// Update the FOV
			ViewFOV = NewViewFOV;
			
			// Actually move/rotate the camera
			MoveViewportPerspectiveCamera(
				NewViewLocation - ViewLocation,
				NewViewRotation - ViewRotation,
				TRUE );		// Unlit movement?
		}
	}
}



void FEditorLevelViewportClient::UpdateMouseDelta()
{
	// Do nothing if a drag tool is being used.
	if( MouseDeltaTracker->UsingDragTool() )
	{
		return;
	}

	FVector DragDelta;

	// If any actor in the selection requires snapping, they all need to be snapped.
	UBOOL bNeedMovementSnapping = FALSE;

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( Actor->bEdShouldSnap )
		{
			bNeedMovementSnapping = TRUE;
			break;
		}
	}

	if( Widget->GetCurrentAxis() != AXIS_None && bNeedMovementSnapping )
	{
		DragDelta = MouseDeltaTracker->GetDeltaSnapped();
	}
	else
	{
		DragDelta = MouseDeltaTracker->GetDelta();
	}

	GEditor->MouseMovement += FVector( Abs(DragDelta.X), Abs(DragDelta.Y), Abs(DragDelta.Z) );

	if( Viewport )
	{
		// Update the status bar text using the absolute change.
		UpdateMousePositionTextUsingDelta( bNeedMovementSnapping );

		if( !DragDelta.IsNearlyZero() )
		{
			const UBOOL AltDown = Input->IsAltPressed();
			const UBOOL ShiftDown = Input->IsShiftPressed();
			const UBOOL ControlDown = Input->IsCtrlPressed();
			const UBOOL LeftMouseButtonDown = Viewport->KeyState(KEY_LeftMouseButton);
			const UBOOL RightMouseButtonDown = Viewport->KeyState(KEY_RightMouseButton);

			if ( !Input->IsPressed(KEY_L) )
			{
				MayaActor = NULL;
			}

			for( INT x = 0 ; x < 3 ; ++x )
			{
				FVector WkDelta( 0.f, 0.f, 0.f );
				WkDelta[x] = DragDelta[x];
				if( !WkDelta.IsZero() )
				{
					// Convert the movement delta into drag/rotation deltas
					FVector Drag;
					FRotator Rot;
					FVector Scale;
					EAxis CurrentAxis = Widget->GetCurrentAxis();
					if ( IsOrtho() && LeftMouseButtonDown && RightMouseButtonDown )
					{
						bWidgetAxisControlledByDrag = FALSE;
						Widget->SetCurrentAxis( AXIS_None );
						MouseDeltaTracker->ConvertMovementDeltaToDragRot( this, WkDelta, Drag, Rot, Scale );
						Widget->SetCurrentAxis( CurrentAxis );
						CurrentAxis = AXIS_None;
					}
					else
					{
						MouseDeltaTracker->ConvertMovementDeltaToDragRot( this, WkDelta, Drag, Rot, Scale );
					}

					// Give the current editor mode a chance to use the input first.  If it does, don't apply it to anything else.
					if( GEditorModeTools().GetCurrentMode()->InputDelta( this, Viewport, Drag, Rot, Scale ) )
					{
						if( GEditorModeTools().GetCurrentMode()->AllowWidgetMove() )
						{
							GEditorModeTools().PivotLocation += Drag;
							GEditorModeTools().SnappedLocation += Drag;
						}
					}
					else
					{
						if( CurrentAxis != AXIS_None )
						{
							// Is the user not dragging by the widget handles? If so, Ctrl key toggles between dragging
							// the viewport or dragging the actor.
							if( !ControlDown && !AltDown && CurrentAxis == AXIS_None )
							{
								// Only apply camera speed modifiers to the drag if we aren't zooming in an ortho viewport.
								if( !IsOrtho() || !(LeftMouseButtonDown && RightMouseButtonDown) )
								{
									Drag *= GCurrentLevelEditingViewportClient->CameraSpeed / 4.f;
								}
								FVector CameraDelta( Drag );
								if (ViewportType==LVT_OrthoXY)
								{
									CameraDelta.X = -Drag.Y;
									CameraDelta.Y = Drag.X;
								}
								MoveViewportCamera( CameraDelta, Rot );
							}
							else
							{
								// If duplicate dragging . . .
								if ( AltDown && (LeftMouseButtonDown || RightMouseButtonDown) )
								{
									// The widget has been offset, so check if we should duplicate actors.
									if ( bDuplicateActorsOnNextDrag )
									{
										// Only duplicate if we're translating or rotating.
										if ( !Drag.IsNearlyZero() || !Rot.IsZero() )
										{
											// Actors haven't been dragged since ALT+LMB went down.
											bDuplicateActorsOnNextDrag = FALSE;

											GEditor->edactDuplicateSelected( FALSE );
										}
									}
								}

								// Apply deltas to selected actors or viewport cameras
								ApplyDeltaToActors( Drag, Rot, Scale );
								GEditorModeTools().PivotLocation += Drag;
								GEditorModeTools().SnappedLocation += Drag;

								if( ShiftDown )
								{
									FVector CameraDelta( Drag );
									if (ViewportType==LVT_OrthoXY)
									{
										CameraDelta.X = -Drag.Y;
										CameraDelta.Y = Drag.X;
									}
									MoveViewportCamera( CameraDelta, FRotator(0,0,0) );
								}

								GEditorModeTools().GetCurrentMode()->UpdateInternalData();
							} 
						}
						else
						{
							// Only apply camera speed modifiers to the drag if we aren't zooming in an ortho viewport.

							AActor* SelectedActor = GEditor->GetSelectedActors()->GetTop<AActor>();

							if ( !IsOrtho() && GEditor->bUseMayaCameraControls && 
									(Input->IsPressed(KEY_U) || 
									(Input->IsPressed(KEY_L) && SelectedActor)) )
							{
								if (!bUsingMayaCam)
								{
									// Enable Maya cam if U or L keys are down.
									bAllowMayaCam = TRUE;
									SwitchStandardToMaya();
									if (Input->IsPressed(KEY_L) && SelectedActor)
									{
										MayaZoom.Y = (ViewLocation - SelectedActor->Location).Size();
										MayaLookAt = SelectedActor->Location;
										MayaActor = SelectedActor;
									}
								}
								else
								{
									// Switch focus if selected actor changes
									if (Input->IsPressed(KEY_L) && SelectedActor && 
										(SelectedActor != MayaActor))
									{
										MayaLookAt = SelectedActor->Location;
										MayaActor = SelectedActor;
									}
								}

								FVector TempDrag;
								FRotator TempRot;
								InputAxisMayaCam( Viewport, DragDelta, TempDrag, TempRot );
							}
							else
							{
								// Disable Maya cam
								if ( bUsingMayaCam )
								{
									bAllowMayaCam = FALSE;
									SwitchMayaToStandard();
								}

								if( !IsOrtho() || !(LeftMouseButtonDown && RightMouseButtonDown) )
								{
									Drag *= GCurrentLevelEditingViewportClient->CameraSpeed / 4.f;
								}
								MoveViewportCamera( Drag, Rot );
							}
						}
					}

					// Clean up
					MouseDeltaTracker->ReduceBy( WkDelta );
				}
			}

			Invalidate( FALSE, TRUE );
		}
	}
}

/**
* Calculates absolute transform for mouse position status text using absolute mouse delta.
*
* @param bUseSnappedDelta Whether or not to use absolute snapped delta for transform calculations.
*/
void FEditorLevelViewportClient::UpdateMousePositionTextUsingDelta( UBOOL bUseSnappedDelta )
{
	// Only update using the info from the active viewport.
	if(GCurrentLevelEditingViewportClient != this || !bIsTracking)
	{
		return;
	}

	// We need to calculate the absolute change in Drag, Rotation, and Scale since we started,
	// so we use GEditor->MouseMovement as our delta.
	FVector Delta;

	if( bUseSnappedDelta )
	{
		Delta = MouseDeltaTracker->GetAbsoluteDeltaSnapped();
	}
	else
	{
		Delta = MouseDeltaTracker->GetAbsoluteDelta();
	}

	// Delta.Z isn't necessarily set by the MouseDeltaTracker
	Delta.Z = 0.0f;

	//Make sure our delta is a tangible amount to avoid divide by zero problems.
	if( Delta.IsNearlyZero() )
	{
		if( CanUpdateStatusBarText() )
		{
			GEditor->UpdateMousePositionText(UEditorEngine::MP_NoChange, FVector(0,0,0));
		}

		return;
	}

	// We need to separate the deltas for each of the axis and compute the sum of the change vectors
	// because the ConvertMovement function only uses the 'max' axis as its source for converting delta to movement.
	FVector Drag;
	FRotator Rot;
	FVector Scale;

	FVector TempDrag;
	FRotator TempRot;
	FVector TempScale;

	MouseDeltaTracker->ConvertMovementDeltaToDragRot( this, FVector(Delta.X,0,0), TempDrag, TempRot, TempScale );
	MouseDeltaTracker->ConvertMovementDeltaToDragRot( this, FVector(0,Delta.Y,0), Drag, Rot, Scale );

	Drag += TempDrag;
	Rot += TempRot;
	Scale += TempScale;

	// Now we see which movement mode we are working on and use that to determine which operation the
	// user is doing.
	FVector ReadableRotation( Rot.Pitch, Rot.Yaw, Rot.Roll );
	const UBOOL bTranslating = !Drag.IsNearlyZero();
	const UBOOL bRotating = !ReadableRotation.IsNearlyZero();
	const UBOOL bScaling = !Scale.IsNearlyZero();

	
	if( CanUpdateStatusBarText() )
	{
		if( bTranslating )
		{
			GEditor->UpdateMousePositionText(UEditorEngine::MP_Translate, Drag);
		}
		else if( bRotating )
		{
			GEditor->UpdateMousePositionText(UEditorEngine::MP_Rotate, ReadableRotation);
		}
		else if( bScaling )
		{
			GEditor->UpdateMousePositionText(UEditorEngine::MP_Scale, Scale);
		}
		else
		{
			GEditor->UpdateMousePositionText(UEditorEngine::MP_NoChange, FVector(0,0,0));
		}
	}
}

//
//	FEditorLevelViewportClient::InputKey
//

UBOOL FEditorLevelViewportClient::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT/*AmountDepressed*/,UBOOL/*Gamepad*/)
{
	const INT	HitX = Viewport->GetMouseX();
	const INT	HitY = Viewport->GetMouseY();

	// Remember which keys and buttons are pressed down.
	const UBOOL bCtrlButtonEvent = (Key == KEY_LeftControl || Key == KEY_RightControl);
	const UBOOL bShiftButtonEvent = (Key == KEY_LeftShift || Key == KEY_RightShift);
	const UBOOL bAltButtonEvent = (Key == KEY_LeftAlt || Key == KEY_RightAlt);
	const UBOOL AltDown = !( bAltButtonEvent && Event == IE_Released ) && Input->IsAltPressed();
	const UBOOL	ShiftDown = !( bShiftButtonEvent && Event == IE_Released ) && Input->IsShiftPressed();
	const UBOOL ControlDown = !( bCtrlButtonEvent && Event == IE_Released ) && Input->IsCtrlPressed();
	const UBOOL TabDown	= Viewport->KeyState(KEY_Tab); 
	const UBOOL LeftMouseButtonDown = Viewport->KeyState(KEY_LeftMouseButton);
	const UBOOL MiddleMouseButtonDown = Viewport->KeyState(KEY_MiddleMouseButton);
	const UBOOL RightMouseButtonDown = Viewport->KeyState(KEY_RightMouseButton);
	const UBOOL bMouseButtonDown = (LeftMouseButtonDown || MiddleMouseButtonDown || RightMouseButtonDown );
	const UBOOL bMouseButtonEvent = (Key == KEY_LeftMouseButton || Key == KEY_MiddleMouseButton || Key == KEY_RightMouseButton);

	// Check to see if the current event is a modifier key and that key was already in the
	// same state.
	const UBOOL bIsRedundantModifierEvent =
		( bAltButtonEvent && ( ( Event != IE_Released ) == Input->IsAltPressed() ) ) ||
		( bCtrlButtonEvent && ( ( Event != IE_Released ) == Input->IsCtrlPressed() ) ) ||
		( bShiftButtonEvent && ( ( Event != IE_Released ) == Input->IsShiftPressed() ) );


	// If we are in Color Picking Mode, then let the editor handle the special case mouse input,
	// we capture all mouse button events until we get a mouse up, we use that as our disabling input.
	if ( GEditor->GetColorPickModeEnabled() )
	{
		if( bMouseButtonEvent )
		{
			if( Event == IE_Pressed )
			{
				// We need to invalidate the viewport in order to generate the correct pixel buffer for picking.
				Invalidate( FALSE, TRUE );
			}
			else if( Event == IE_Released )
			{
				//Only pick a color if they use the left mouse button, otherwise cancel out of pick mode.
				if(Key == KEY_LeftMouseButton)
				{
					GEditor->PickColorFromViewport( Viewport, HitX, HitY );
					GEditor->SetColorPickModeEnabled( FALSE );
				}
				else
				{
					GEditor->CancelColorPickMode();
				}
			}
		}

		return TRUE;
	}

	// Store a reference to the last viewport that received a keypress.
	GLastKeyLevelEditingViewportClient = this;

	if( GCurrentLevelEditingViewportClient != this )
	{
		GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
		GCurrentLevelEditingViewportClient = this;
	}

	// Compute a view.
	FSceneViewFamilyContext ViewFamily(Viewport,GetScene(),ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL,IsRealtime());
	FSceneView* View = CalcSceneView( &ViewFamily );

	// Compute the click location.

	GEditor->ClickLocation = FVector((View->ViewMatrix * View->ProjectionMatrix).Inverse().TransformFVector4(FVector4((HitX - Viewport->GetSizeX() / 2.0f) / (Viewport->GetSizeX() / 2.0f),(HitY - Viewport->GetSizeY() / 2.0f) / -(Viewport->GetSizeY() / 2.0f),0.5f,1.0f)));

	FEdMode *CurrentMode = GEditorModeTools().GetCurrentMode();
	check(CurrentMode != NULL);

	if( Key == KEY_V )
	{
		int x = 5;
	}

	// Let the current mode have a look at the input before reacting to it.
	if( CurrentMode->InputKey(this, Viewport, Key, Event) )
	{
		return TRUE;
	}

	// Tell the viewports input handler about the keypress.
	if( Input->InputKey(ControllerId, Key, Event) )
	{
		return TRUE;
	}

	// Tell Sentinel stats viewer about key
	GEditor->SentinelInputKey(this, Key, Event);

	// Tell the other editor viewports about the keypress so they will be aware if the user tries
	// to do something like box select inside of them without first clicking them for focus.
	for( INT v = 0 ; v < GEditor->ViewportClients.Num() ; ++v )
	{
		FEditorLevelViewportClient* vc = GEditor->ViewportClients(v);

		if( vc != this )
		{
			vc->Input->InputKey( ControllerId, Key, Event );
		}
	}

	if ( !AltDown && ControlDown && !LeftMouseButtonDown && !MiddleMouseButtonDown && RightMouseButtonDown && IsOrtho() )
	{
		GEditorModeTools().SetWidgetModeOverride( FWidget::WM_Rotate );
	}
	else
	{
		GEditorModeTools().SetWidgetModeOverride( FWidget::WM_None );
	}

	// We'll make sure the mouse is confined and the cursor is hidden while the mouse button
	// is pressed down, unless we've just double-clicked
	if ( bMouseButtonEvent && Event != IE_DoubleClick )
	{
		Viewport->ShowCursor( !bMouseButtonDown );
		Viewport->LockMouseToWindow( bMouseButtonDown );
	}

	// Handle viewport screenshot.
	InputTakeScreenshot( Viewport, Key, Event );

	// Start tracking if any mouse button is down and it was a tracking event (MouseButton/Ctrl/Shift/Alt):
	if ( bMouseButtonDown && (Event == IE_Pressed || Event == IE_Released) && (bMouseButtonEvent || bCtrlButtonEvent || bAltButtonEvent || bShiftButtonEvent) )
	{
		// Only start (or retart) tracking mode if the current event wasn't a modifier key that
		// we was already pressed or released.
		if( !bIsRedundantModifierEvent )
		{
			bDraggingByHandle = (Widget->GetCurrentAxis() != AXIS_None);

			// Stop current tracking
			if ( bIsTracking )
			{
				MouseDeltaTracker->EndTracking( this );
				bIsTracking = FALSE;
			}

			// Re-initialize new tracking only if a new button was pressed, otherwise we continue the previous one.
			if ( Event == IE_Pressed )
			{
				// Tracking initialization:
				GEditor->MouseMovement = FVector(0,0,0);
			
				if ( AltDown )
				{
					if(Event == IE_Pressed && (Key == KEY_LeftMouseButton || Key == KEY_RightMouseButton) && !bDuplicateActorsInProgress)
					{
						// Set the flag so that the actors actors will be duplicated as soon as the widget is displaced.
						bDuplicateActorsOnNextDrag = TRUE;
						bDuplicateActorsInProgress = TRUE;
					}
				}
				else
				{
					bDuplicateActorsOnNextDrag = FALSE;
				}
			}

			// Start new tracking. Potentially reset the widget so that StartTracking can pick a new axis.
			if ( !bDraggingByHandle || (ControlDown && !TabDown) )
			{
				bWidgetAxisControlledByDrag = FALSE;
				Widget->SetCurrentAxis( AXIS_None );
			}
			MouseDeltaTracker->StartTracking( this, HitX, HitY );
			bIsTracking = TRUE;
		}
		return TRUE;
	}

	// Stop tracking if no mouse button is down
	if ( bIsTracking && !bMouseButtonDown )
	{
		if ( CachedShowFlags  )
		{
			ShowFlags = CachedShowFlags;
			CachedShowFlags = 0;
		}

		if ( MouseDeltaTracker->EndTracking( this ) )
		{
			// If the mouse haven't moved too far, treat the button release as a click.
			if( GEditor->MouseMovement.Size() < MOUSE_CLICK_DRAG_DELTA && !MouseDeltaTracker->UsingDragTool() )
			{
				HHitProxy* HitProxy = Viewport->GetHitProxy(HitX,HitY);

				// If this was a mouse click that clicked on something (thereby selecting it) and the builder brush
				// had been auto-selected deselect it so that only the clicked object becomes selected.
				if(HitProxy != NULL && MouseDeltaTracker->bSelectedBuilderBrush)
				{
					GEditor->SelectNone(FALSE, FALSE);
				}

				ProcessClick(View,HitProxy,Key,Event,HitX,HitY);
			}
			else
			{
				// Only disable the duplicate on next drag flag if we actually dragged the mouse.
				bDuplicateActorsOnNextDrag = FALSE;
			}

			bWidgetAxisControlledByDrag = FALSE;
			Widget->SetCurrentAxis( AXIS_None );
			Invalidate( TRUE, TRUE );
		}
		
		MouseDeltaTracker->bSelectedBuilderBrush = FALSE;
		bIsTracking = FALSE;
	}

	// Clear Duplicate Actors mode when ALT and all mouse buttons are released
	if ( !AltDown && !bMouseButtonDown )
	{
		bDuplicateActorsInProgress = FALSE;
	}

	if ( Event == IE_DoubleClick )
	{
		// Stop current tracking
		if ( bIsTracking )
		{
			MouseDeltaTracker->EndTracking( this );
			bIsTracking = FALSE;
		}

		MouseDeltaTracker->StartTracking( this, HitX, HitY );
		bIsTracking = TRUE;
		GEditor->MouseMovement = FVector(0,0,0);
		HHitProxy*	HitProxy = Viewport->GetHitProxy(HitX,HitY);
		ProcessClick(View,HitProxy,Key,Event,HitX,HitY);
		MouseDeltaTracker->EndTracking( this );
		bIsTracking = FALSE;
		return TRUE;
	}

	if( ( Key == KEY_MouseScrollUp || Key == KEY_MouseScrollDown ) && Event == IE_Pressed )
	{
		if ( IsOrtho() )
		{
			// Scrolling the mousewheel up/down zooms the orthogonal viewport in/out.
			INT Delta = 25;
			if( Key == KEY_MouseScrollUp )
			{
				Delta *= -1;
			}

			OrthoZoom += (OrthoZoom / CAMERA_ZOOM_DAMPEN) * Delta;
			OrthoZoom = Clamp<FLOAT>( OrthoZoom, MIN_ORTHOZOOM, MAX_ORTHOZOOM );

			Invalidate( TRUE, TRUE );
		}
		else
		{
			// If flight camera input is active, then the mouse wheel will control the speed of camera
			// movement
			if( IsFlightCameraInputModeActive() )
			{
				const FLOAT MinCameraSpeedScale = 0.1f;
				const FLOAT MaxCameraSpeedScale = 10.0f;

				// Adjust and clamp the camera speed scale
				if( Key == KEY_MouseScrollUp )
				{
					if( FlightCameraSpeedScale >= 2.0f )
					{
						FlightCameraSpeedScale += 0.5f;
					}
					else if( FlightCameraSpeedScale >= 1.0f )
					{
						FlightCameraSpeedScale += 0.2f;
					}
					else
					{
						FlightCameraSpeedScale += 0.1f;
					}
				}
				else
				{
					if( FlightCameraSpeedScale > 2.49f )
					{
						FlightCameraSpeedScale -= 0.5f;
					}
					else if( FlightCameraSpeedScale >= 1.19f )
					{
						FlightCameraSpeedScale -= 0.2f;
					}
					else
					{
						FlightCameraSpeedScale -= 0.1f;
					}
				}

				FlightCameraSpeedScale = Clamp( FlightCameraSpeedScale, MinCameraSpeedScale, MaxCameraSpeedScale );

				if( appIsNearlyEqual( FlightCameraSpeedScale, 1.0f, 0.01f ) )
				{
					// Snap to 1.0 if we're really close to that
					FlightCameraSpeedScale = 1.0f;
				}

				// Keep track of when the camera speed was changed list so that the status bar is
				// updated appropriately
				RealTimeFlightCameraSpeedScaleChanged = appSeconds();

				GEditor->UpdateMousePositionText(
					UEditorEngine::MP_CameraSpeed,
					FVector( FlightCameraSpeedScale, 0.0f, 0.0f ) );	// X = Camera speed value
			}
			else
			{
				// Scrolling the mousewheel up/down moves the perspective viewport forwards/backwards.
				FVector Drag(0,0,0);

				Drag.X = GMath.CosTab( ViewRotation.Yaw ) * GMath.CosTab( ViewRotation.Pitch );
				Drag.Y = GMath.SinTab( ViewRotation.Yaw ) * GMath.CosTab( ViewRotation.Pitch );
				Drag.Z = GMath.SinTab( ViewRotation.Pitch );

				if( Key == KEY_MouseScrollDown )
				{
					Drag = -Drag;
				}

				Drag *= GCurrentLevelEditingViewportClient->CameraSpeed * 8.f;

				MoveViewportCamera( Drag, FRotator(0,0,0), FALSE );
				Invalidate( TRUE, TRUE );
			}
		}
	}
	else if( ControlDown && MiddleMouseButtonDown )
	{
		FVector vec;

		if( IsOrtho() )
		{
			vec = GEditor->ClickLocation;
		}
		else
		{
			vec = ViewLocation;
		}

		INT idx = -1;

		switch( ViewportType )
		{
			case LVT_OrthoXY:
				idx = 2;
				break;
			case LVT_OrthoXZ:
				idx = 1;
				break;
			case LVT_OrthoYZ:
				idx = 0;
				break;
		}

		for( INT v = 0 ; v < GEditor->ViewportClients.Num() ; ++v )
		{
			FEditorLevelViewportClient* vc = GEditor->ViewportClients(v);

			if( vc != this )
			{
				if( IsOrtho() )
				{
					if( idx != 0 )		vc->ViewLocation.X = vec.X;
					if( idx != 1 )		vc->ViewLocation.Y = vec.Y;
					if( idx != 2 )		vc->ViewLocation.Z = vec.Z;
				}
				else
				{
					vc->ViewLocation = vec;
				}
				vc->Invalidate( FALSE, TRUE );
			}
		}
	}
	else if( (IsOrtho() || AltDown) && (Key == KEY_Left || Key == KEY_Right || Key == KEY_Up || Key == KEY_Down) )
	{
		if( Event == IE_Pressed || Event == IE_Repeat )
		{
			// If this is a pressed event, start tracking.
			if(Event == IE_Pressed)
			{
				// Stop current tracking
				if ( bIsTracking )
				{
					MouseDeltaTracker->EndTracking( this );
					bIsTracking = FALSE;
				}

				GEditorModeTools().GetCurrentMode()->StartTracking();
				MouseDeltaTracker->StartTracking( this, HitX, HitY, TRUE );
				bIsTracking = TRUE;
			}

			// Keyboard nudging of the widget
			if( Key == KEY_Left || Key == KEY_Right )
			{
				bWidgetAxisControlledByDrag = FALSE;
				Widget->SetCurrentAxis( GetHorizAxis() );
				MouseDeltaTracker->AddDelta( this, KEY_MouseX, GEditor->Constraints.GetGridSize() * (Key == KEY_Left?-1:1), 1 );
				Widget->SetCurrentAxis( GetHorizAxis() );
			} 
			else if( Key == KEY_Up || Key == KEY_Down )
			{
				bWidgetAxisControlledByDrag = FALSE;
				Widget->SetCurrentAxis( GetVertAxis() );
				MouseDeltaTracker->AddDelta( this, KEY_MouseY, GEditor->Constraints.GetGridSize() * (Key == KEY_Up?1:-1), 1 );
				Widget->SetCurrentAxis( GetVertAxis() );
			}

			UpdateMouseDelta();
		}
		else if( Event == IE_Released )
		{
			bWidgetAxisControlledByDrag = FALSE;
			GEditorModeTools().GetCurrentMode()->EndTracking();
			MouseDeltaTracker->EndTracking( this );
			bIsTracking = FALSE;
			Widget->SetCurrentAxis( AXIS_None );
		}
		
		GEditor->RedrawLevelEditingViewports();
	}
	else if( IsFlightCameraInputModeActive() &&
			 GEditor->GetUserSettings().AllowFlightCameraToRemapKeys &&
			 ( Key == KEY_W || Key == KEY_S || Key == KEY_A || Key == KEY_D ||
			   Key == KEY_E || Key == KEY_Q || Key == KEY_Z || Key == KEY_C ) )
	{
		// Flight camera control is active, so simply absorb the key.  The camera will update based
		// on currently pressed keys (Viewport->KeyState) in the Tick function.

		// (Don't remove this condition!)
	}
	else if( Event == IE_Pressed || Event == IE_Repeat )
	{
		UBOOL bRedrawAllViewports = FALSE;

		// BOOKMARKS

		TCHAR ch = 0;
		if( Key == KEY_Zero )		ch = '0';
		else if( Key == KEY_One )	ch = '1';
		else if( Key == KEY_Two )	ch = '2';
		else if( Key == KEY_Three )	ch = '3';
		else if( Key == KEY_Four )	ch = '4';
		else if( Key == KEY_Five )	ch = '5';
		else if( Key == KEY_Six )	ch = '6';
		else if( Key == KEY_Seven )	ch = '7';
		else if( Key == KEY_Eight )	ch = '8';
		else if( Key == KEY_Nine )	ch = '9';

		if( (ch >= '0' && ch <= '9') && !AltDown && !ShiftDown )
		{
			// Bookmarks.
			const INT BookMarkIndex = ch - '0';

			// CTRL+# will set a bookmark, # will jump to it.
			if( ControlDown )
			{
				GEditorModeTools().SetBookMark( BookMarkIndex, this );
			}
			else
			{
				GEditorModeTools().JumpToBookMark( BookMarkIndex );
			}

			bRedrawAllViewports = TRUE;
		}

		// Change grid size

		if( Key == KEY_LeftBracket )
		{
			GEditor->Constraints.SetGridSz( GEditor->Constraints.CurrentGridSz - 1 );
			bRedrawAllViewports = TRUE;
		}
		if( Key == KEY_RightBracket )
		{
			GEditor->Constraints.SetGridSz( GEditor->Constraints.CurrentGridSz + 1 );
			bRedrawAllViewports = TRUE;
		}

		// Change editor modes

		if( ShiftDown )
		{
			if( Key == KEY_One )
			{
				GEditor->Exec(TEXT("MODE CAMERAMOVE"));
				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_Two )
			{
				GEditor->Exec(TEXT("MODE GEOMETRY"));
				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_Three )
			{
				GEditor->Exec(TEXT("MODE TERRAINEDIT"));
				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_Four )
			{
				GEditor->Exec(TEXT("MODE TEXTURE"));
				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_Five )
			{
				GEditor->Exec(TEXT("MODE COVEREDIT"));
				bRedrawAllViewports = TRUE;
			}
		}

		// Change viewport mode

		if( AltDown )
		{
			if( Key == KEY_One )
			{
				ShowFlags &= ~SHOW_ViewMode_Mask;
				ShowFlags |= SHOW_ViewMode_BrushWireframe;
				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_Two )
			{
				ShowFlags &= ~SHOW_ViewMode_Mask;
				ShowFlags |= SHOW_ViewMode_Wireframe;
				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_Three )
			{
				ShowFlags &= ~SHOW_ViewMode_Mask;
				ShowFlags |= SHOW_ViewMode_Unlit;
				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_Four )
			{
				ShowFlags &= ~SHOW_ViewMode_Mask;
				ShowFlags |= SHOW_ViewMode_Lit;
				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_Five )
			{
				ShowFlags &= ~SHOW_ViewMode_Mask;
				ShowFlags |= SHOW_ViewMode_LightingOnly;
				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_Six )
			{
				ShowFlags &= ~SHOW_ViewMode_Mask;
				ShowFlags |= SHOW_ViewMode_LightComplexity;
				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_Seven )
			{
				ShowFlags &= ~SHOW_ViewMode_Mask;
				ShowFlags |= SHOW_ViewMode_TextureDensity;
				bRedrawAllViewports = TRUE;
			}

			if( bRedrawAllViewports )
			{
				GCallbackEvent->Send( CALLBACK_UpdateUI );
			}
		}

		// Try to see if this input can be handled by showflags key bindings, if not,
		// handle other special input cases.
		const UBOOL bInputHandled = CheckForShowFlagInput(Key, ControlDown, AltDown, ShiftDown);

		if(bInputHandled == TRUE)
		{
			bRedrawAllViewports = TRUE;
		}
		else
		{
			if( Key == KEY_G )
			{
				// CTRL+G selects all actors belonging to the groups of the currently selected actors.
				if ( ControlDown && !AltDown && !ShiftDown )
				{
					// Iterate over selected actors and make a list of all groups the selected actors belong to.
					TArray<FString> SelectedGroups;

					for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
					{
						AActor* Actor = static_cast<AActor*>( *It );
						checkSlow( Actor->IsA(AActor::StaticClass()) );

						// Parse the actor's groups into an array.
						TArray<FString> NewGroups;
						Actor->Group.ToString().ParseIntoArray( &NewGroups, TEXT(","), 0 );

						// Add them to the list of selected groups.
						for( INT NewGroupIndex = 0 ; NewGroupIndex < NewGroups.Num() ; ++NewGroupIndex )
						{
							SelectedGroups.AddUniqueItem( NewGroups(NewGroupIndex) );
						}
					}

					if ( SelectedGroups.Num() > 0 )
					{
						const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("SelectActorsByGroup")) );
						GEditor->SelectNone( FALSE, FALSE );
						for ( FActorIterator It ; It ; ++It )
						{
							AActor* Actor = *It;

							// Take the actor's group string and break it up into an array.
							const FString GroupNames = Actor->Group.ToString();

							TArray<FString> GroupList;
							GroupNames.ParseIntoArray( &GroupList, TEXT(","), FALSE );

							// Iterate over the array of group names searching for the input group.
							UBOOL bShouldSelect = FALSE;
							for( INT GroupIndex = 0 ; GroupIndex < GroupList.Num() && !bShouldSelect ; ++GroupIndex )
							{
								const FString& ActorGroup = GroupList(GroupIndex);
								for ( INT SelectedGroupIndex = 0 ; SelectedGroupIndex < SelectedGroups.Num() ; ++SelectedGroupIndex )
								{
									const FString& SelectedGroup = SelectedGroups(SelectedGroupIndex);
									if ( SelectedGroup == ActorGroup )
									{
										bShouldSelect = TRUE;
										break;
									}
								}
							}

							if ( bShouldSelect )
							{
								GEditor->SelectActor( Actor, TRUE, NULL, FALSE );
							}
						}
						GEditor->NoteSelectionChange();
					}
				}

				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_D )
			{
				SetRealtime( !IsRealtime() );
				GCallbackEvent->Send( CALLBACK_UpdateUI );
				bRedrawAllViewports = TRUE;
			}
			else if( Key == KEY_V )
			{
				if( ShiftDown )
				{
					EShowFlags SaveShowFlags = ShowFlags;

					// Turn off all collision flags
					ShowFlags &= ~SHOW_Collision_Any;

					if( (SaveShowFlags&SHOW_Collision_Any) == 0 )
					{
						ShowFlags |= SHOW_CollisionZeroExtent;
					}
					else if( SaveShowFlags&SHOW_CollisionZeroExtent )
					{
						ShowFlags |= SHOW_CollisionNonZeroExtent;
					}
					else if( SaveShowFlags&SHOW_CollisionNonZeroExtent )
					{
						ShowFlags |= SHOW_CollisionRigidBody;
					}

					bRedrawAllViewports = TRUE;
				}
			}
			else if( Key == KEY_R )
			{
				if ( ControlDown )
				{
					// Toggle whether or not this viewport updates in realtime.
					ToggleRealtime();
					GCallbackEvent->Send( CALLBACK_UpdateUI );
				}
				bRedrawAllViewports = TRUE;
			}
		}

		if( bRedrawAllViewports )
		{
			GEditor->RedrawLevelEditingViewports();
		}
	}

	return TRUE;
}

/**
 * Checks to see if the current input event modified any show flags.
 * @param Key				Key that was pressed.
 * @param bControlDown		Flag for whether or not the control key is held down.
 * @param bAltDown			Flag for whether or not the alt key is held down.
 * @param bShiftDown		Flag for whether or not the shift key is held down.
 * @return					Flag for whether or not we handled the input.
 */
UBOOL FEditorLevelViewportClient::CheckForShowFlagInput(FName Key, UBOOL bControlDown, UBOOL bAltDown, UBOOL bShiftDown)
{
	// We handle input only in the cases where alt&shift are held down, or no modifiers are held down.
	// If the user holds down ALT-SHIFT, and presses a button, then instead of toggling that showflag,
	// we toggle everything BUT that showflag.

	// @todo: For some reason we aren't getting the invert key modifiers right now, need to fix this later to a combination that works.
	UBOOL bToggleInverse = FALSE;
	UBOOL bSpecialMode = FALSE;

	if(IsShiftDown(Viewport) == TRUE)
	{
		bToggleInverse = TRUE;
	}
	else if(bAltDown == TRUE || bControlDown == TRUE || bShiftDown == TRUE)
	{
		return FALSE;
	}	
	
	// Toggle mode is the way that the flags will be set.  
	enum EToggleMode 
	{
		TM_Toggle,			// Just toggles the flag.
		TM_ToggleView,		// Toggles the view flag.  This is necessary to toggle flags that are part of SHOW_ViewMode_Mask.
		TM_SetOffOnly,		// Turns the flag off only.
		TM_Set				// Sets only the flags specified ON, all others off.
	};

	// Show flags mapping table.  This table maps key inputs to various show flag toggles.
	struct FShowFlagKeyPair
	{
		EShowFlags Flags;
		FName Key;
		EToggleMode ToggleMode;
		EShowFlags InvertFlags;
	};

	// Remember the current viewmode
	EShowFlags CurrentViewFlags = ShowFlags & SHOW_ViewMode_Mask;
	const EShowFlags DefaultGameNoView = SHOW_DefaultGame & ~SHOW_ViewMode_Mask;

	// Operate in a viewmode independent fashion
	ShowFlags &= ~SHOW_ViewMode_Mask;
	EShowFlags ToggleFlags = ShowFlags;
	EShowFlags ToggleViewFlags = CurrentViewFlags;

	const FShowFlagKeyPair KeyMappings [] = 
	{
		{SHOW_Volumes,			KEY_O, 		TM_Toggle},
		{SHOW_BSP,				KEY_Q, 		TM_Toggle},
		{SHOW_StaticMeshes,		KEY_W,		TM_Toggle},
		{SHOW_Paths,			KEY_P, 		TM_Toggle},
		{SHOW_NavigationNodes,	KEY_N, 		TM_Toggle},
		{SHOW_Terrain,			KEY_T, 		TM_Toggle},
		{SHOW_KismetRefs,		KEY_K, 		TM_Toggle},
		{SHOW_Collision,		KEY_C, 		TM_Toggle},
		{SHOW_Decals,			KEY_E, 		TM_Toggle},
		{SHOW_Fog,				KEY_F, 		TM_ToggleView},
		{SHOW_LightRadius,		KEY_R, 		TM_Toggle},
		// Toggling 'G' mode does not touch view flags.
		{DefaultGameNoView,		KEY_G, 		TM_Set, SHOW_DefaultEditor},
		{SHOW_StaticMeshes | SHOW_NavigationNodes | SHOW_Terrain | SHOW_Decals | SHOW_Volumes | SHOW_Constraints | SHOW_Sprites | SHOW_Particles, KEY_H,  TM_SetOffOnly},	//BSP Only Mode

		{0} // Must be the last element in the array.
	};

	// Loop through the KeyMappings array and see if the key passed in matches.
	UBOOL bFoundKey = FALSE;
	const FShowFlagKeyPair* Pair = KeyMappings;

	while(Pair->Flags != 0)
	{
		if(Pair->Key == Key)
		{
			switch(Pair->ToggleMode)
			{
			case TM_Toggle:
				ToggleFlags ^= Pair->Flags;
				break;
			case TM_ToggleView:
				ToggleViewFlags ^= Pair->Flags;
				break;
			case TM_Set:
				bSpecialMode = TRUE;
				ToggleFlags = Pair->Flags;
			break;
			case TM_SetOffOnly:
				if((ToggleFlags & Pair->Flags) == 0)
				{
					ToggleFlags |= Pair->Flags;
				}
				else
				{
					ToggleFlags &= ~(Pair->Flags);
				}
				
				break;
			default:
				break;
			}

			bFoundKey = TRUE;
			break;
		}

		Pair++;
	}

	// The builder brush case needs to be handled a bit differently because it sets its value to all viewports.
	if( Key == KEY_B )
	{
		// Use the builder brush show status in this viewport to determine what the new state should be for all viewports.
		const UBOOL bNewBuilderBrushState = !(ShowFlags & SHOW_BuilderBrush ? TRUE : FALSE);
		for( INT ViewportIndex = 0 ; ViewportIndex < GEditor->ViewportClients.Num() ; ++ViewportIndex )
		{
			FEditorLevelViewportClient* ViewportClient = GEditor->ViewportClients( ViewportIndex );
			const UBOOL bCurBuilderBrushState = ViewportClient->ShowFlags & SHOW_BuilderBrush ? TRUE : FALSE;
			if ( bCurBuilderBrushState != bNewBuilderBrushState )
			{
				ViewportClient->ShowFlags ^= SHOW_BuilderBrush;
			}
		}
		
		bFoundKey = TRUE;
	}
	else if(bFoundKey == TRUE)
	{
		// If this is a special mode and is the name special mode as the last special mode that was set, it means that the user is toggling the special mode
		// so reset the showflags to whatever they were before the user entered the special mode.
		if(bSpecialMode && LastSpecialMode == ToggleFlags)
		{
			LastSpecialMode = 0;
			ShowFlags = LastShowFlags;
		}
		else
		{
			// If the user is changing the current showflags, then check to see if this is a special mode,
			// if it is and we are not already in a existing special mode, then store the current show flag state before changing it.
			if(ToggleFlags != ShowFlags
				|| (ToggleViewFlags != CurrentViewFlags))
			{
				if(bSpecialMode == TRUE)
				{
					if(LastSpecialMode == 0)
					{
						LastSpecialMode = ToggleFlags;
						LastShowFlags = ShowFlags;
					}
				}

				ShowFlags = ToggleFlags;
			}
			else
			{
				ShowFlags = LastShowFlags;
			}
		}
	}

	// Reset the current viewmode
	ShowFlags |= ToggleViewFlags;
	return bFoundKey;
}

/**
 * Returns the horizontal axis for this viewport.
 */

EAxis FEditorLevelViewportClient::GetHorizAxis() const
{
	switch( ViewportType )
	{
		case LVT_OrthoXY:
			return AXIS_Y;
		case LVT_OrthoXZ:
			return AXIS_X;
		case LVT_OrthoYZ:
			return AXIS_Y;
	}

	return AXIS_X;
}

/**
 * Returns the vertical axis for this viewport.
 */

EAxis FEditorLevelViewportClient::GetVertAxis() const
{
	switch( ViewportType )
	{
		case LVT_OrthoXY:
			return AXIS_X;
		case LVT_OrthoXZ:
			return AXIS_Z;
		case LVT_OrthoYZ:
			return AXIS_Z;
	}

	return AXIS_Y;
}

//
//	FEditorLevelViewportClient::InputAxis
//

UBOOL FEditorLevelViewportClient::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad)
{
	if( GCurrentLevelEditingViewportClient != this )
	{
		//GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
		GCurrentLevelEditingViewportClient = this;
	}

	FEdMode* CurrentMode = GEditorModeTools().GetCurrentMode();
	// Let the current mode have a look at the input before reacting to it.
	if (CurrentMode && CurrentMode->InputAxis(this, Viewport, ControllerId, Key, Delta, DeltaTime))
	{
		return TRUE;
	}

	// Let the engine try to handle the input first.
	if( Input->InputAxis(ControllerId,Key,Delta,DeltaTime,bGamepad) )
	{
		return TRUE;
	}

	if( bIsTracking	)
	{
		// Accumulate and snap the mouse movement since the last mouse button click.
		MouseDeltaTracker->AddDelta( this, Key, Delta, 0 );
	}

	// If we are using a drag tool, paint the viewport so we can see it update.
	if( MouseDeltaTracker->UsingDragTool() )
	{
		Invalidate( FALSE, FALSE );
	}
	
	return TRUE;
}

void FEditorLevelViewportClient::InputAxisMayaCam(FViewport* Viewport, const FVector& DragDelta, FVector& Drag, FRotator& Rot)
{
	FRotator tempRot = ViewRotation;
	ViewRotation = FRotator(0,16384,0);	
	ConvertMovementToDragRotMayaCam(DragDelta, Drag, Rot);
	ViewRotation = tempRot;
	Drag.X = DragDelta.X;

	UBOOL	LeftMouseButton = Viewport->KeyState(KEY_LeftMouseButton),
		MiddleMouseButton = Viewport->KeyState(KEY_MiddleMouseButton),
		RightMouseButton = Viewport->KeyState(KEY_RightMouseButton);

	if ( LeftMouseButton )
	{
		MayaRotation += FRotator( Rot.Pitch, -Rot.Yaw, Rot.Roll );
	}
	else if ( MiddleMouseButton )
	{
		FVector DeltaLocation = FVector(Drag.X, 0, -Drag.Z) * CAMERA_MAYAPAN_SPEED;

		FMatrix rotMat =
			FTranslationMatrix( -MayaLookAt ) *
			FRotationMatrix( FRotator(0,MayaRotation.Yaw,0) ) * 
			FRotationMatrix( FRotator(0, 0, MayaRotation.Pitch));

		MayaLookAt = MayaLookAt + rotMat.Inverse().TransformNormal(DeltaLocation);
	}
	else if ( RightMouseButton )
	{
		MayaZoom.Y += -Drag.Y;
	}
}

/**
 * Implements screenshot capture for editor viewports.  Should be called by derived class' InputKey.
 */
void FEditorLevelViewportClient::InputTakeScreenshot(FViewport* Viewport, FName Key, EInputEvent Event)
{
	const UBOOL F9Down = Viewport->KeyState(KEY_F9);
	if ( F9Down )
	{
		if ( Key == KEY_LeftMouseButton )
		{
			if( Event == IE_Pressed )
			{
				// We need to invalidate the viewport in order to generate the correct pixel buffer for picking.
				Invalidate( FALSE, TRUE );
			}
			else if( Event == IE_Released )
			{
				// Read the contents of the viewport into an array.
				TArray<FColor> Bitmap;
				if( Viewport->ReadPixels(Bitmap) )
				{
					check(Bitmap.Num() == Viewport->GetSizeX() * Viewport->GetSizeY());

					// Create screenshot folder if not already present.
					GFileManager->MakeDirectory( *GSys->ScreenShotPath, TRUE );

					// Save the contents of the array to a bitmap file.
					appCreateBitmap(*(GSys->ScreenShotPath * TEXT("ScreenShot")),Viewport->GetSizeX(),Viewport->GetSizeY(),&Bitmap(0),GFileManager);
				}
			}
		}
	}
}


/**
 * Invalidates this viewport and optionally child views.
 *
 * @param	bInvalidateChildViews		[opt] If TRUE (the default), invalidate views that see this viewport as their parent.
 * @param	bInvalidateHitProxies		[opt] If TRUE (the default), invalidates cached hit proxies too.
 */
void FEditorLevelViewportClient::Invalidate(UBOOL bInvalidateChildViews, UBOOL bInvalidateHitProxies)
{
	if ( Viewport )
	{
		if ( bInvalidateHitProxies )
		{
			// Invalidate hit proxies and display pixels.
			Viewport->Invalidate();
		}
		else
		{
			// Invalidate only display pixels.
			Viewport->InvalidateDisplay();
		}

		// If this viewport is a view parent . . .
		if ( bInvalidateChildViews &&
			ViewState->IsViewParent() )
		{
			GEditor->InvalidateChildViewports( ViewState, bInvalidateHitProxies );	
		}
	}
}

/**
 * Determines if InComponent is inside of InSelBBox.  This check differs depending on the type of component.
 * If InComponent is NULL, FALSE is returned.
 *
 * @param	InActor				Used only when testing sprite components.
 * @param	InComponent			The component to query.  If NULL
 * @param	InSelBox			The selection box.
 * @param	bConsiderOnlyBSP	If TRUE, consider only BSP.
 */
UBOOL FEditorLevelViewportClient::ComponentIsTouchingSelectionBox(AActor* InActor, UPrimitiveComponent* InComponent, const FBox& InSelBBox, UBOOL bConsiderOnlyBSP)
{
	if( (ShowFlags & (SHOW_Volumes|SHOW_BSP)) && InComponent->IsA(UBrushComponent::StaticClass()) )
	{
		if ( InActor->IsAVolume() )
		{
			// Don't select if the brush is a volume and the volume show flag is unset.
			if ( !(ShowFlags & SHOW_Volumes) )
			{
				return FALSE;
			}
		}
		else
		{
			// Don't select if the brush is regular bsp and the bsp show flag is unset.
			if ( !(ShowFlags & SHOW_BSP) )
			{
				return FALSE;
			}
		}

		const UBrushComponent* BrushComponent = static_cast<UBrushComponent*>( InComponent );
		if( BrushComponent->Brush && BrushComponent->Brush->Polys )
		{
			for( INT PolyIndex = 0 ; PolyIndex < BrushComponent->Brush->Polys->Element.Num() ; ++PolyIndex )
			{
				const FPoly& Poly = BrushComponent->Brush->Polys->Element( PolyIndex );

				for( INT VertexIndex = 0 ; VertexIndex < Poly.Vertices.Num() ; ++VertexIndex )
				{
					const FVector Location = InComponent->LocalToWorld.TransformFVector( Poly.Vertices(VertexIndex) );
					if( FPointBoxIntersection(Location, InSelBBox) )
					{
						return TRUE;
					}
				}
			}
		}
	}
	else if( !bConsiderOnlyBSP && (ShowFlags & SHOW_StaticMeshes) && InComponent->IsA( UStaticMeshComponent::StaticClass() ) )
	{
		const UStaticMeshComponent* StaticMeshComponent = static_cast<UStaticMeshComponent*>( InComponent );

		if( StaticMeshComponent->StaticMesh )
		{
			check( StaticMeshComponent->StaticMesh->LODModels.Num() > 0 );
			const FStaticMeshRenderData& LODModel = StaticMeshComponent->StaticMesh->LODModels(0);
			for( UINT VertexIndex = 0 ; VertexIndex < LODModel.NumVertices ; ++VertexIndex )
			{
				const FVector& SrcPosition = LODModel.PositionVertexBuffer.VertexPosition(VertexIndex);
				const FVector Location = StaticMeshComponent->LocalToWorld.TransformFVector( SrcPosition );

				if( FPointBoxIntersection(Location, InSelBBox) )
				{
					return TRUE;
				}
			}
		}
	}
	else if( !bConsiderOnlyBSP && (ShowFlags & SHOW_Sprites) && InComponent->IsA(USpriteComponent::StaticClass()))
	{
		const USpriteComponent*	SpriteComponent = static_cast<USpriteComponent*>( InComponent );
		if(InSelBBox.Intersect(
			FBox(
				InActor->Location - InActor->DrawScale * SpriteComponent->Scale * Max(SpriteComponent->Sprite->SizeX,SpriteComponent->Sprite->SizeY) * FVector(1,1,1),
				InActor->Location + InActor->DrawScale * SpriteComponent->Scale * Max(SpriteComponent->Sprite->SizeX,SpriteComponent->Sprite->SizeY) * FVector(1,1,1)
				)
			))
		{
			// Disallow selection based on sprites of nav nodes that aren't being drawn because of SHOW_NavigationNodes.
			const UBOOL bIsHiddenNavNode = ( !(ShowFlags&SHOW_NavigationNodes) && InActor->IsA(ANavigationPoint::StaticClass()) );
			if ( !bIsHiddenNavNode )
			{
				return TRUE;
			}
		}
	}
	else if( !bConsiderOnlyBSP && (ShowFlags & SHOW_SkeletalMeshes) && InComponent->IsA( USkeletalMeshComponent::StaticClass() ) )
	{
		const USkeletalMeshComponent* SkeletalMeshComponent = static_cast<USkeletalMeshComponent*>( InComponent );
		if( SkeletalMeshComponent->SkeletalMesh )
		{
			check(SkeletalMeshComponent->SkeletalMesh->LODModels.Num() > 0);

			// Transform hard and soft verts into world space. Note that this assumes skeletal mesh is in reference pose...
			const FStaticLODModel& LODModel = SkeletalMeshComponent->SkeletalMesh->LODModels(0);
			for( INT ChunkIndex = 0 ; ChunkIndex < LODModel.Chunks.Num() ; ++ChunkIndex )
			{
				const FSkelMeshChunk& Chunk = LODModel.Chunks(ChunkIndex);
				for( INT i = 0; i < Chunk.RigidVertices.Num(); ++i )
				{
					const FVector Location = SkeletalMeshComponent->LocalToWorld.TransformFVector( Chunk.RigidVertices(i).Position );
					if( FPointBoxIntersection(Location, InSelBBox) )
					{
						return TRUE;
					}
				}

				for( INT i = 0 ; i < Chunk.SoftVertices.Num() ; ++i )
				{
					const FVector Location = SkeletalMeshComponent->LocalToWorld.TransformFVector( Chunk.SoftVertices(i).Position );
					if( FPointBoxIntersection(Location, InSelBBox) )
					{
						return TRUE;
					}
				}
			}
		}
	}
	else if ( !bConsiderOnlyBSP )
	{
		UBOOL bSelectByBoundingBox = FALSE;

		// Determine whether the component may be selected by bounding box.
		if( (ShowFlags & SHOW_SpeedTrees) && InComponent->IsA(USpeedTreeComponent::StaticClass()) )
		{
			bSelectByBoundingBox = TRUE;
		}

		// If it's selectable by bounding box, check if the bounding box intersections the selection box.
		if(bSelectByBoundingBox && InComponent->Bounds.GetBox().Intersect(InSelBBox))
		{
			return TRUE;
		}
	}

	return FALSE;
}

// Determines which axis InKey and InDelta most refer to and returns
// a corresponding FVector.  This vector represents the mouse movement
// translated into the viewports/widgets axis space.
//
// @param InNudge		If 1, this delta is coming from a keyboard nudge and not the mouse

FVector FEditorLevelViewportClient::TranslateDelta( FName InKey, FLOAT InDelta, UBOOL InNudge )
{
	UBOOL AltDown = Input->IsAltPressed(),
		ShiftDown = Input->IsShiftPressed(),
		ControlDown = Input->IsCtrlPressed(),
		LeftMouseButtonDown = Viewport->KeyState(KEY_LeftMouseButton),
		RightMouseButtonDown = Viewport->KeyState(KEY_RightMouseButton);

	FVector vec(0.0f, 0.0f, 0.0f);

	const FLOAT X = InKey == KEY_MouseX ? InDelta : 0.f;
	const FLOAT Y = InKey == KEY_MouseY ? InDelta : 0.f;

	switch( ViewportType )
	{
		case LVT_OrthoXY:
		case LVT_OrthoXZ:
		case LVT_OrthoYZ:

			if( InNudge )
			{
				vec = FVector( X, Y, 0.f );
			}
			else
			{
				vec = FVector( X * (OrthoZoom/CAMERA_ZOOM_DIV), Y * (OrthoZoom/CAMERA_ZOOM_DIV), 0.f );

				if( Widget->GetCurrentAxis() == AXIS_None )
				{
					switch( ViewportType )
					{
						case LVT_OrthoXY:
							vec.X *= -1;
							break;
						case LVT_OrthoXZ:
							vec = FVector( X * (OrthoZoom/CAMERA_ZOOM_DIV), 0.f, Y * (OrthoZoom/CAMERA_ZOOM_DIV) );
							break;
						case LVT_OrthoYZ:
							vec = FVector( 0.f, X * (OrthoZoom/CAMERA_ZOOM_DIV), Y * (OrthoZoom/CAMERA_ZOOM_DIV) );
							break;
					}
				}
			}
			break;

		case LVT_Perspective:
			vec = FVector( X, Y, 0.f );
			break;

		default:
			check(0);		// Unknown viewport type
			break;
	}

	if( IsOrtho() && (LeftMouseButtonDown && RightMouseButtonDown) && Y != 0.f )
	{
		vec = FVector(0,0,Y);
	}

	return vec;
}

// Converts a generic movement delta into drag/rotation deltas based on the viewport and keys held down

void FEditorLevelViewportClient::ConvertMovementToDragRot(const FVector& InDelta,
														  FVector& InDragDelta,
														  FRotator& InRotDelta)
{
	const UBOOL AltDown = Input->IsAltPressed();
	const UBOOL ShiftDown = Input->IsShiftPressed();
	const UBOOL ControlDown = Input->IsCtrlPressed();
	const UBOOL LeftMouseButtonDown = Viewport->KeyState(KEY_LeftMouseButton);
	const UBOOL RightMouseButtonDown = Viewport->KeyState(KEY_RightMouseButton);

	InDragDelta = FVector(0,0,0);
	InRotDelta = FRotator(0,0,0);

	switch( ViewportType )
	{
		case LVT_OrthoXY:
		case LVT_OrthoXZ:
		case LVT_OrthoYZ:
		{
			if( LeftMouseButtonDown && RightMouseButtonDown )
			{
				// Both mouse buttons change the ortho viewport zoom.
				InDragDelta = FVector(0,0,InDelta.Z);
			}
			else if( RightMouseButtonDown )
			{
				// @todo: set RMB to move opposite to the direction of drag, in other words "grab and pull".
				InDragDelta = InDelta;
			}
			else if( LeftMouseButtonDown )
			{
				// LMB moves in the direction of the drag.
				InDragDelta = InDelta;
			}
		}
		break;

		case LVT_Perspective:
		{
			if( LeftMouseButtonDown && !RightMouseButtonDown )
			{
				// Move forward and yaw

				InDragDelta.X = InDelta.Y * GMath.CosTab( ViewRotation.Yaw );
				InDragDelta.Y = InDelta.Y * GMath.SinTab( ViewRotation.Yaw );

				InRotDelta.Yaw = InDelta.X * CAMERA_ROTATION_SPEED;
			}
			else if( RightMouseButtonDown && LeftMouseButtonDown )
			{
				// Pan left/right/up/down

				InDragDelta.X = InDelta.X * -GMath.SinTab( ViewRotation.Yaw );
				InDragDelta.Y = InDelta.X *  GMath.CosTab( ViewRotation.Yaw );
				InDragDelta.Z = InDelta.Y;
			}
			else if( RightMouseButtonDown && !LeftMouseButtonDown )
			{
				// Change viewing angle

				InRotDelta.Yaw = InDelta.X * CAMERA_ROTATION_SPEED;
				InRotDelta.Pitch = InDelta.Y * CAMERA_ROTATION_SPEED;
			}
		}
		break;

		default:
			check(0);	// unknown viewport type
			break;
	}
}

void FEditorLevelViewportClient::ConvertMovementToDragRotMayaCam(const FVector& InDelta,
																 FVector& InDragDelta,
																 FRotator& InRotDelta)
{
	const UBOOL AltDown = Input->IsAltPressed();
	const UBOOL ShiftDown = Input->IsShiftPressed();
	const UBOOL ControlDown = Input->IsCtrlPressed();
	const UBOOL LeftMouseButtonDown = Viewport->KeyState(KEY_LeftMouseButton);
	const UBOOL RightMouseButtonDown = Viewport->KeyState(KEY_RightMouseButton);
	const UBOOL MiddleMouseButtonDown = Viewport->KeyState(KEY_MiddleMouseButton);

	InDragDelta = FVector(0,0,0);
	InRotDelta = FRotator(0,0,0);

	switch( ViewportType )
	{
	case LVT_OrthoXY:
	case LVT_OrthoXZ:
	case LVT_OrthoYZ:
		{
			if( LeftMouseButtonDown && RightMouseButtonDown )
			{
				// Change ortho zoom.
				InDragDelta = FVector(0,0,InDelta.Z);
			}
			else if( RightMouseButtonDown )
			{
				// Move camera.
				InDragDelta = InDelta;
			}
			else if( LeftMouseButtonDown )
			{
				// Move actors.
				InDragDelta = InDelta;
			}
		}
		break;

	case LVT_Perspective:
		{
			if( LeftMouseButtonDown )
			{
				// Change the viewing angle
				InRotDelta.Yaw = InDelta.X * CAMERA_ROTATION_SPEED;
				InRotDelta.Pitch = InDelta.Y * CAMERA_ROTATION_SPEED;
			}
			else if( MiddleMouseButtonDown )
			{
				// Pan left/right/up/down
				InDragDelta.X = InDelta.X * -GMath.SinTab( ViewRotation.Yaw );
				InDragDelta.Y = InDelta.X *  GMath.CosTab( ViewRotation.Yaw );
				InDragDelta.Z = InDelta.Y;
			}
			else if( RightMouseButtonDown )
			{
				// Zoom in and out.
				InDragDelta.X = InDelta.Y * GMath.CosTab( ViewRotation.Yaw );
				InDragDelta.Y = InDelta.Y * GMath.SinTab( ViewRotation.Yaw );
			}
		}
		break;

	default:
		check(0);	// unknown viewport type
		break;
	}
}



/** Returns true if perspective flight camera input mode is currently active in this viewport */
UBOOL FEditorLevelViewportClient::IsFlightCameraInputModeActive() const
{
	if( Viewport != NULL && ViewportType == LVT_Perspective )
	{
		if( CameraController != NULL )
		{
			const UBOOL bControlDown = Viewport->KeyState( KEY_LeftControl ) || Viewport->KeyState( KEY_RightControl );
			const UBOOL bShiftDown = Viewport->KeyState( KEY_LeftShift ) || Viewport->KeyState( KEY_RightShift );
			const UBOOL bAltDown = Viewport->KeyState( KEY_LeftAlt ) || Viewport->KeyState( KEY_RightAlt );
			const UBOOL bLeftMouseButtonDown = Viewport->KeyState( KEY_LeftMouseButton );
			const UBOOL bRightMouseButtonDown = Viewport->KeyState( KEY_RightMouseButton );

			const UBOOL bIsMouseLooking =
				Widget->GetCurrentAxis() == AXIS_None &&
				bRightMouseButtonDown &&
				!bLeftMouseButtonDown &&
				!bControlDown && !bShiftDown && !bAltDown;

			return bIsMouseLooking;
		}
	}

	return FALSE;
}



/** Moves a perspective camera */
void FEditorLevelViewportClient::MoveViewportPerspectiveCamera( const FVector& InDrag, const FRotator& InRot, UBOOL bUnlitMovement )
{
	check( ViewportType == LVT_Perspective );

	// Update camera Rotation
	ViewRotation += FRotator( InRot.Pitch, InRot.Yaw, InRot.Roll );
	
	// If rotation is pitching down by using a big positive number, change it so its using a smaller negative number
	if((ViewRotation.Pitch > 49151) && (ViewRotation.Pitch <= 65535))
	{
		ViewRotation.Pitch = ViewRotation.Pitch - 65535;
	}

	// Make sure its withing  +/- 90 degrees.
	ViewRotation.Pitch = Clamp( ViewRotation.Pitch, -16384, 16384 );

	// Update camera Location
	ViewLocation.AddBounded( InDrag, HALF_WORLD_MAX1 );

	// Tell the editing mode that the camera moved, in case its interested.
	GEditorModeTools().GetCurrentMode()->CamMoveNotify(this);

	// If turned on, move any selected actors to the cameras location/rotation
	if( bLockSelectedToCamera )
	{
		for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );

			if ( !Actor->bLockLocation )
			{
				Actor->SetLocation( GCurrentLevelEditingViewportClient->ViewLocation );
			}
			if( Actor->IsBrush() )
			{
				FBSPOps::RotateBrushVerts( (ABrush*)Actor, GCurrentLevelEditingViewportClient->ViewRotation, TRUE );
			}
			else
			{
				Actor->SetRotation( GCurrentLevelEditingViewportClient->ViewRotation );
			}
		}
	}

	if ( bUnlitMovement && bMoveUnlit && !CachedShowFlags )
	{
		CachedShowFlags = ShowFlags;
		ShowFlags &= ~SHOW_Lighting;
	}
}

void FEditorLevelViewportClient::MoveViewportCamera(const FVector& InDrag,
													const FRotator& InRot,
													UBOOL bUnlitMovement)
{
	const UBOOL LeftMouseButtonDown = Viewport->KeyState(KEY_LeftMouseButton);
	const UBOOL RightMouseButtonDown = Viewport->KeyState(KEY_RightMouseButton);

	switch( ViewportType )
	{
		case LVT_OrthoXY:
		case LVT_OrthoXZ:
		case LVT_OrthoYZ:
		{
			if( LeftMouseButtonDown && RightMouseButtonDown )
			{
				OrthoZoom += (OrthoZoom / CAMERA_ZOOM_DAMPEN) * InDrag.Z;
				OrthoZoom = Clamp<FLOAT>( OrthoZoom, MIN_ORTHOZOOM, MAX_ORTHOZOOM );
			}
			else
			{
				if ( ViewportType == LVT_OrthoXY )
					ViewLocation.AddBounded( FVector(InDrag.Y, -InDrag.X, InDrag.Z), HALF_WORLD_MAX1 );
				else
					ViewLocation.AddBounded( InDrag, HALF_WORLD_MAX1 );
			}

			if ( bUnlitMovement && bMoveUnlit && !CachedShowFlags )
			{
				CachedShowFlags = ShowFlags;
				ShowFlags &= ~SHOW_Lighting;
			}
		}
		break;

		case LVT_Perspective:
		{
			// If the flight camera is active, we'll update the rotation impulse data for that instead
			// of rotating the camera ourselves here
			if( IsFlightCameraInputModeActive() && CameraController->GetConfig().bUsePhysicsBasedRotation )
			{
				// NOTE: We damp the rotation for impulse input since the camera controller will
				//	apply its own rotation speed
				const FLOAT VelModRotSpeed = 900.0f;
				const FVector RotEuler = InRot.Euler();
				CameraUserImpulseData->RotateYawVelocityModifier += VelModRotSpeed * RotEuler.X / CAMERA_ROTATION_SPEED;
				CameraUserImpulseData->RotatePitchVelocityModifier += VelModRotSpeed * RotEuler.Y / CAMERA_ROTATION_SPEED;
				CameraUserImpulseData->RotateRollVelocityModifier += VelModRotSpeed * RotEuler.Z / CAMERA_ROTATION_SPEED;
			}
			else
			{
				MoveViewportPerspectiveCamera( InDrag, InRot, bUnlitMovement );
			}
		}
		break;
	}
}

void FEditorLevelViewportClient::ApplyDeltaToActors(const FVector& InDrag,
													const FRotator& InRot,
													const FVector& InScale)
{
	if( GEditorModeTools().GetMouseLock() || (InDrag.IsZero() && InRot.IsZero() && InScale.IsZero()) )
	{
		return;
	}

	// If we are scaling, we need to change the scaling factor a bit to properly align to grid.
	FVector ModifiedScale = InScale;
	USelection* SelectedActors = GEditor->GetSelectedActors();
	const UBOOL bScalingActors = !InScale.IsNearlyZero();

	if( bScalingActors )
	{
		/* @todo: May reenable this form of calculating scaling factors later on.
		// Calculate a bounding box for the actors.
		FBox ActorsBoundingBox( 0 );

		for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );

			const FBox ActorsBox = Actor->GetComponentsBoundingBox( TRUE );
			ActorsBoundingBox += ActorsBox;
		}

		const FVector BoxExtent = ActorsBoundingBox.GetExtent();

		for (INT Idx=0; Idx < 3; Idx++)
		{
			ModifiedScale[Idx] = InScale[Idx] / BoxExtent[Idx];
		}
		*/

		// If we are uniformly scaling, make sure to change scale x, y, z by the same amount.
		INT ScalePercent;

		// If we are snapping to grid, use the selected scaling value to determine how fast to scale.
		// otherwise, just default to 10%, since that seems to be a good default.
		if(GEditor->Constraints.SnapScaleEnabled)
		{
			ScalePercent = GEditor->Constraints.ScaleGridSize;
		}
		else
		{
			ScalePercent = 10;
		}

		if(GEditorModeTools().GetWidgetMode() == FWidget::WM_Scale)
		{
			FLOAT ScaleFactor = (GEditor->Constraints.ScaleGridSize / 100.0f) * InScale[0] / GEditor->Constraints.GetGridSize();
			
			ModifiedScale = FVector(ScaleFactor, ScaleFactor, ScaleFactor);
		}
		else
		{
			ModifiedScale = InScale * ((GEditor->Constraints.ScaleGridSize / 100.0f) / GEditor->Constraints.GetGridSize());
		}
	}

	// Transact the actors.
	GEditor->NoteActorMovement();

	// Apply the deltas to any selected actors.
	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( !Actor->bLockLocation )
		{
			ApplyDeltaToActor( Actor, InDrag, InRot, ModifiedScale );
		}
	}
}

//
//	FEditorLevelViewportClient::ApplyDeltaToActor
//

void FEditorLevelViewportClient::ApplyDeltaToActor( AActor* InActor, const FVector& InDeltaDrag, const FRotator& InDeltaRot, const FVector& InDeltaScale )
{
	const UBOOL bAltDown = Input->IsAltPressed();
	const UBOOL	bShiftDown = Input->IsShiftPressed();
	const UBOOL bControlDown = Input->IsCtrlPressed();

	GEditor->ApplyDeltaToActor(
		InActor,
		TRUE,
		&InDeltaDrag,
		&InDeltaRot,
		&InDeltaScale,
		bAltDown,
		bShiftDown,
		bControlDown );
}

//
//	FEditorLevelViewportClient::MouseMove
//

void FEditorLevelViewportClient::MouseMove(FViewport* Viewport,INT x, INT y)
{
	// Let the current editor mode know about the mouse movement.
	if( GEditorModeTools().GetCurrentMode()->MouseMove(this, Viewport, x, y) )
	{
		return;
	}

	// Inform Sentinel about the mouse movement
	GEditor->SentinelMouseMove(this, x, y);

	//Only update the position if we are not tracking mouse movement because it will be updated elsewhere if we are.
	if( !bIsTracking )
	{
		if( Viewport->GetSizeX() > 0 && Viewport->GetSizeY() > 0 )
		{
			// Since there is no editor tool that cares about the mouse worldspace position, generate our own status string using the worldspace position
			// of the mouse.  If we are mousing over the perspective viewport, blank out the string because we cannot accurate display the mouse position in that viewport.
			FSceneViewFamilyContext ViewFamily(Viewport,GetScene(),ShowFlags,GWorld->GetTimeSeconds(),GWorld->GetDeltaSeconds(),GWorld->GetRealTimeSeconds(),NULL,IsRealtime());
			FSceneView* View = CalcSceneView(&ViewFamily);

			if( CanUpdateStatusBarText() )
			{
				if( ViewportType == LVT_Perspective )
				{
					GEditor->UpdateMousePositionText( UEditorEngine::MP_None, FVector() );
				}
				else
				{
					GEditor->UpdateMousePositionText(
						UEditorEngine::MP_WorldspacePosition,
						View->ScreenToWorld(View->PixelToScreen(x,y,0.5f))
						);
				}
			}
		}
	}
}

//
//	FEditorLevelVIewportClient::GetCursor
//

EMouseCursor FEditorLevelViewportClient::GetCursor(FViewport* Viewport,INT X,INT Y)
{
	EMouseCursor MouseCursor = MC_Arrow;

	// If color pick mode is enabled, always return the cross cursor.
	if( GEditor->GetColorPickModeEnabled() )
	{
		MouseCursor = MC_Hand;
	}
	else
	{
		// Don't select widget axes by mouse over while they're being controlled by a mouse drag.
		if(!bWidgetAxisControlledByDrag)
		{
			const EAxis SaveAxis = Widget->GetCurrentAxis();
			HHitProxy* HitProxy = Viewport->GetHitProxy(X,Y);
			EAxis NewAxis = AXIS_None;

			// Change the mouse cursor if the user is hovering over something they can interact with.
			if( HitProxy )
			{
				MouseCursor = HitProxy->GetMouseCursor();

				if( HitProxy->IsA(HWidgetAxis::StaticGetType() ) )
				{
					NewAxis = ((HWidgetAxis*)HitProxy)->Axis;
				}
			}

			// If the current axis on the widget changed, repaint the viewport.
			if( NewAxis != SaveAxis )
			{
				Widget->SetCurrentAxis( NewAxis );
				GEditorModeTools().GetCurrentMode()->SetCurrentWidgetAxis( NewAxis );
				Invalidate( FALSE, FALSE );
			}
		}
	}

	return MouseCursor;
}

void FEditorLevelViewportClient::RedrawRequested(FViewport* InViewport)
{
	// Whenever a redraw is needed, redraw at least twice to account for occlusion queries being a frame behind.
	NumPendingRedraws = 2;
}

/** Renders the specified view frustum of the parent view. */
static void RenderViewParentFrustum(const FSceneView* View,
									FPrimitiveDrawInterface* PDI,
									const FLinearColor& FrustumColor,
									FLOAT FrustumAngle,
									FLOAT FrustumAspectRatio,
									FLOAT FrustumStartDist,
									FLOAT FrustumEndDist,
									const FMatrix& InViewMatrix)
{
	FVector Direction(0,0,1);
	FVector LeftVector(1,0,0);
	FVector UpVector(0,1,0);

	FVector Verts[8];

	// FOVAngle controls the horizontal angle.
	FLOAT HozHalfAngle = (FrustumAngle) * ((FLOAT)PI/360.f);
	FLOAT HozLength = FrustumStartDist * appTan(HozHalfAngle);
	FLOAT VertLength = HozLength/FrustumAspectRatio;

	// near plane verts
	Verts[0] = (Direction * FrustumStartDist) + (UpVector * VertLength) + (LeftVector * HozLength);
	Verts[1] = (Direction * FrustumStartDist) + (UpVector * VertLength) - (LeftVector * HozLength);
	Verts[2] = (Direction * FrustumStartDist) - (UpVector * VertLength) - (LeftVector * HozLength);
	Verts[3] = (Direction * FrustumStartDist) - (UpVector * VertLength) + (LeftVector * HozLength);

	HozLength = FrustumEndDist * appTan(HozHalfAngle);
	VertLength = HozLength/FrustumAspectRatio;

	// far plane verts
	Verts[4] = (Direction * FrustumEndDist) + (UpVector * VertLength) + (LeftVector * HozLength);
	Verts[5] = (Direction * FrustumEndDist) + (UpVector * VertLength) - (LeftVector * HozLength);
	Verts[6] = (Direction * FrustumEndDist) - (UpVector * VertLength) - (LeftVector * HozLength);
	Verts[7] = (Direction * FrustumEndDist) - (UpVector * VertLength) + (LeftVector * HozLength);

	for( INT x = 0 ; x < 8 ; ++x )
	{
		Verts[x] = InViewMatrix.Inverse().TransformFVector( Verts[x] );
	}

	const BYTE PrimitiveDPG = SDPG_Foreground;
	PDI->DrawLine( Verts[0], Verts[1], FrustumColor, PrimitiveDPG );
	PDI->DrawLine( Verts[1], Verts[2], FrustumColor, PrimitiveDPG );
	PDI->DrawLine( Verts[2], Verts[3], FrustumColor, PrimitiveDPG );
	PDI->DrawLine( Verts[3], Verts[0], FrustumColor, PrimitiveDPG );

	PDI->DrawLine( Verts[4], Verts[5], FrustumColor, PrimitiveDPG );
	PDI->DrawLine( Verts[5], Verts[6], FrustumColor, PrimitiveDPG );
	PDI->DrawLine( Verts[6], Verts[7], FrustumColor, PrimitiveDPG );
	PDI->DrawLine( Verts[7], Verts[4], FrustumColor, PrimitiveDPG );

	PDI->DrawLine( Verts[0], Verts[4], FrustumColor, PrimitiveDPG );
	PDI->DrawLine( Verts[1], Verts[5], FrustumColor, PrimitiveDPG );
	PDI->DrawLine( Verts[2], Verts[6], FrustumColor, PrimitiveDPG );
	PDI->DrawLine( Verts[3], Verts[7], FrustumColor, PrimitiveDPG );
}

void FEditorLevelViewportClient::Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI)
{
	if (ShowFlags & SHOW_StreamingBounds)
	{
		DrawTextureStreamingBounds(View, PDI);
	}

	GEditorModeTools().GetCurrentMode()->GetComponent()->Draw(View,PDI);

	// Draw the current editor mode.
	GEditorModeTools().GetCurrentMode()->Render( View, Viewport, PDI );

	// Draw parent view's frustum.
	if ( ViewState->HasViewParent() )
	{
		RenderViewParentFrustum( View, PDI,
			FLinearColor(1.0,0.0,1.0,1.0),
			GParentFrustumAngle,
			GParentFrustumAspectRatio,
			GParentFrustumStartDist,
			GParentFrustumEndDist,
			GParentViewMatrix);
	}

	// Draw the drag tool.
	MouseDeltaTracker->Render3DDragTool( View, PDI );

	// Draw the widget.
	Widget->Render( View, PDI );

	if ( GEditor->bEnableSocketSnapping )
	{
		for( FActorIterator It; It; ++It )
		{
			for ( INT ComponentIndex = 0 ; ComponentIndex < It->Components.Num() ; ++ComponentIndex )
			{
				USkeletalMeshComponent* SkelMeshComponent = Cast<USkeletalMeshComponent>(It->Components(ComponentIndex));
				if ( SkelMeshComponent && SkelMeshComponent->SkeletalMesh )
				{
					for ( INT SocketIndex = 0 ; SocketIndex < SkelMeshComponent->SkeletalMesh->Sockets.Num() ; ++SocketIndex )
					{
						USkeletalMeshSocket* Socket = SkelMeshComponent->SkeletalMesh->Sockets(SocketIndex);
						FMatrix SocketTM;
						if( Socket->GetSocketMatrix(SocketTM, SkelMeshComponent) )
						{
							PDI->SetHitProxy( new HLevelSocketProxy( *It, SkelMeshComponent, SocketIndex ) );
							DrawWireDiamond(PDI, SocketTM, 2.f, FColor(255,128,128), SDPG_Foreground );
							PDI->SetHitProxy( NULL );
						}
					}
				}
			}
		}
	}

	// Let Sentinel draw things if desired
	if((ViewportType == LVT_Perspective) && (View->Family->ShowFlags & SHOW_SentinelStats))
	{
		GEditor->SentinelStatRender3D(View, PDI);
	}
}

void FEditorLevelViewportClient::Draw(FViewport* Viewport,FCanvas* Canvas)
{
	// Determine whether we should use world time or real time based on the scene.
	FLOAT TimeSeconds;
	FLOAT RealTimeSeconds;

	if (( GetScene() != GWorld->Scene) || (IsRealtime() == TRUE))
	{
		// Use time relative to start time to avoid issues with FLOAT vs DOUBLE
		TimeSeconds = appSeconds() - GStartTime;
		RealTimeSeconds = appSeconds() - GStartTime;
	}
	else
	{
		TimeSeconds = GWorld->GetTimeSeconds();
		RealTimeSeconds = GWorld->GetRealTimeSeconds();
	}

	// Setup a FSceneViewFamily/FSceneView for the viewport.
	FSceneViewFamilyContext ViewFamily(Canvas->GetRenderTarget(),GetScene(),ShowFlags,TimeSeconds,GWorld->GetDeltaSeconds(),RealTimeSeconds,NULL,IsRealtime(),bAllowAmbientOcclusion);
	FSceneView* View = CalcSceneView( &ViewFamily );

	// Update the listener.
	UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();	
	if( AudioDevice && ( ViewportType == LVT_Perspective ) && IsRealtime() && bSetListenerPosition )
	{
		FMatrix CameraToWorld = View->ViewMatrix.Inverse();

		FVector ProjUp = CameraToWorld.TransformNormal( FVector( 0, 1000, 0 ) );
		FVector ProjRight = CameraToWorld.TransformNormal( FVector( 1000, 0, 0 ) );
		FVector ProjFront = ProjRight ^ ProjUp;

		ProjUp.Z = Abs( ProjUp.Z ); // Don't allow flipping "up".

		ProjUp.Normalize();
		ProjRight.Normalize();
		ProjFront.Normalize();

		AudioDevice->SetListener( 0, 1, ViewLocation, ProjUp, ProjRight, ProjFront );

		// Update reverb settings based on the view of the first player we encounter.
		FReverbSettings ReverbSettings;
		AReverbVolume* ReverbVolume = GWorld->GetWorldInfo()->GetReverbSettings( ViewLocation, TRUE/*bReverbVolumePrevis*/, ReverbSettings );
		AudioDevice->SetReverbSettings( ReverbSettings );
	}

	if((ViewportType == LVT_Perspective) && bConstrainAspectRatio)
	{
		// Clear the background to black if the aspect ratio is constrained, as the scene view won't write to all pixels.
		Clear(Canvas,FLinearColor::Black);
	}

	// Don't use fading or color scaling while we're in light complexity mode, since it may change the colors!
	if( ( ShowFlags & SHOW_LightComplexity ) == 0 )
	{
		if(bEnableFading)
		{
			View->OverlayColor = FadeColor;
			View->OverlayColor.A = Clamp(FadeAmount, 0.f, 1.f);
		}

		if(bEnableColorScaling)
		{
				View->ColorScale = FLinearColor(ColorScale.X,ColorScale.Y,ColorScale.Z);
		}
	}

	BeginRenderingViewFamily(Canvas,&ViewFamily);

	// Remove temporary debug lines.
	// Possibly a hack. Lines may get added without the scene being rendered etc.
	if (GWorld->LineBatcher != NULL && GWorld->LineBatcher->BatchedLines.Num())
	{
		GWorld->LineBatcher->BatchedLines.Empty();
		GWorld->LineBatcher->BeginDeferredReattach();
	}

	GEditorModeTools().GetCurrentMode()->DrawHUD(this,Viewport,View,Canvas);

	// Axes indicators

	if(bDrawAxes && ViewportType == LVT_Perspective)
	{
		DrawAxes(Viewport, Canvas);
	}

	// Information string

	DrawShadowedString(Canvas, 4,4, *GEditorModeTools().InfoString, GEngine->SmallFont, FColor(255,255,255) );

	// If we are the current viewport, draw a yellow-highlighted border around the outer edge.

	FColor BorderColor(0,0,0);
	if( GCurrentLevelEditingViewportClient == this )
	{
		BorderColor = FColor(255,255,0);
	}

	for( INT c = 0 ; c < 3 ; ++c )
	{
		const INT X = 1+c;
		const INT Y = 1+c;
		const INT W = Viewport->GetSizeX()-c;
		const INT H = Viewport->GetSizeY()-c;

		DrawLine2D(Canvas, FVector2D(X,Y), FVector2D(W,Y), BorderColor );
		DrawLine2D(Canvas, FVector2D(W,Y), FVector2D(W,H), BorderColor );
		DrawLine2D(Canvas, FVector2D(W,H), FVector2D(X,H), BorderColor );
		DrawLine2D(Canvas, FVector2D(X,H), FVector2D(X,Y-1), BorderColor );
	}

	// Kismet references
	if((View->Family->ShowFlags & SHOW_KismetRefs) && GWorld->GetGameSequence())
	{
		for ( INT LevelIndex = 0 ; LevelIndex < GWorld->Levels.Num() ; ++LevelIndex )
		{
			ULevel *Level = GWorld->Levels( LevelIndex );
			DrawKismetRefs( Level, Viewport, View, Canvas );
		}
	}

	// Tag drawing
	if(View->Family->ShowFlags & SHOW_ActorTags)
	{
		DrawActorTags(Viewport, View, Canvas);
	}
	
	if((ViewportType != LVT_Perspective) && (View->Family->ShowFlags & SHOW_SentinelStats))
	{
		GEditor->SentinelStatRender(this, View, Canvas);
	}

	if(!IsRealtime())
	{
		// Wait for the rendering thread to finish drawing the view before returning.
		// This reduces the apparent latency of dragging the viewport around.
		FlushRenderingCommands();
	}
}

/** Draw screen-space box around each Actor in the level referenced by the Kismet sequence. */
void FEditorLevelViewportClient::DrawKismetRefs(ULevel* Level, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas )
{
	USequence* Seq = GWorld->GetGameSequence( Level );
	if ( !Seq )
	{
		return;
	}

	// Get all SeqVar_Objects and SequenceEvents from the Sequence (and all subsequences).
	TArray<USequenceObject*> SeqObjVars;
	Seq->FindSeqObjectsByClass( USequenceObject::StaticClass(), SeqObjVars );
	
	const FColor KismetRefColor(175, 255, 162);
	const UBOOL bIsCurrentLevel = ( Level == GWorld->CurrentLevel );

	TArray<AActor*>	DrawnActors;

	for(INT i=0; i<SeqObjVars.Num(); i++)
	{
		if( !SeqObjVars(i) )
		{
			continue;
		}

		// Give object a chance to draw in viewport also
		SeqObjVars(i)->DrawKismetRefs( Viewport, View, Canvas );

		AActor* RefActor = NULL;
		USeqVar_Object* VarObj = Cast<USeqVar_Object>( SeqObjVars(i) );
		if(VarObj)
		{
			RefActor = Cast<AActor>(VarObj->ObjValue);
		}

		USequenceEvent* EventObj = Cast<USequenceEvent>( SeqObjVars(i) );
		if(EventObj)
		{
			RefActor = EventObj->Originator;
		}

		// If this is a refence to an un-Hidden Actor that we haven't drawn yet, draw it now.
		if(RefActor && !RefActor->bHiddenEd && !DrawnActors.ContainsItem(RefActor))
		{
			// If this actor 
			USpriteComponent* Sprite = NULL;
			for(INT j=0; j<RefActor->Components.Num() && !Sprite; j++)
			{
				Sprite = Cast<USpriteComponent>( RefActor->Components(j) );
			}

			FBox ActorBox;
			if(Sprite)
			{
				ActorBox = Sprite->Bounds.GetBox();
			}
			else
			{
				ActorBox = RefActor->GetComponentsBoundingBox(true);
			}

			// If we didn't get a valid bounding box, just make a little one around the actor location
			if(!ActorBox.IsValid || ActorBox.GetExtent().GetMin() < KINDA_SMALL_NUMBER )
			{
				ActorBox = FBox(RefActor->Location - FVector(-20), RefActor->Location + FVector(20));
			}

			FVector BoxCenter, BoxExtents;
			ActorBox.GetCenterAndExtents(BoxCenter, BoxExtents);

			// Project center of bounding box onto screen.
			const FVector4 ProjBoxCenter = View->WorldToScreen(BoxCenter);

			// Do nothing if behind camera
			if(ProjBoxCenter.W > 0.f)
			{
				// Project verts of world-space bounding box onto screen and take their bounding box
				const FVector Verts[8] = {	FVector( 1, 1, 1),
											FVector( 1, 1,-1),
											FVector( 1,-1, 1),
											FVector( 1,-1,-1),
											FVector(-1, 1, 1),
											FVector(-1, 1,-1),
											FVector(-1,-1, 1),
											FVector(-1,-1,-1) };

				const INT HalfX = 0.5f * Viewport->GetSizeX();
				const INT HalfY = 0.5f * Viewport->GetSizeY();

				FVector2D ScreenBoxMin(1000000000, 1000000000);
				FVector2D ScreenBoxMax(-1000000000, -1000000000);

				for(INT j=0; j<8; j++)
				{
					// Project vert into screen space.
					const FVector WorldVert = BoxCenter + (Verts[j]*BoxExtents);
					FVector2D PixelVert;
					if(View->ScreenToPixel(View->WorldToScreen(WorldVert),PixelVert))
					{
						// Update screen-space bounding box with with transformed vert.
						ScreenBoxMin.X = ::Min<INT>(ScreenBoxMin.X, PixelVert.X);
						ScreenBoxMin.Y = ::Min<INT>(ScreenBoxMin.Y, PixelVert.Y);

						ScreenBoxMax.X = ::Max<INT>(ScreenBoxMax.X, PixelVert.X);
						ScreenBoxMax.Y = ::Max<INT>(ScreenBoxMax.Y, PixelVert.Y);
					}
				}

				if ( !bIsCurrentLevel )
				{
					// Draw a bracket when considering the non-current level.
					const FLOAT DeltaX = ScreenBoxMax.X - ScreenBoxMin.X;
					const FLOAT DeltaY = ScreenBoxMax.X - ScreenBoxMin.X;
					const FIntPoint Offset( DeltaX * 0.2f, DeltaY * 0.2f );

					DrawLine2D(Canvas, FVector2D(ScreenBoxMin.X, ScreenBoxMin.Y), FVector2D(ScreenBoxMin.X + Offset.X, ScreenBoxMin.Y), KismetRefColor );
					DrawLine2D(Canvas, FVector2D(ScreenBoxMin.X, ScreenBoxMax.Y), FVector2D(ScreenBoxMin.X + Offset.X, ScreenBoxMax.Y), KismetRefColor );

					DrawLine2D(Canvas, FVector2D(ScreenBoxMax.X, ScreenBoxMin.Y), FVector2D(ScreenBoxMax.X - Offset.X, ScreenBoxMin.Y), KismetRefColor );
					DrawLine2D(Canvas, FVector2D(ScreenBoxMax.X, ScreenBoxMax.Y), FVector2D(ScreenBoxMax.X - Offset.X, ScreenBoxMax.Y), KismetRefColor );

					DrawLine2D(Canvas, FVector2D(ScreenBoxMin.X, ScreenBoxMin.Y), FVector2D(ScreenBoxMin.X, ScreenBoxMin.Y + Offset.Y), KismetRefColor );
					DrawLine2D(Canvas, FVector2D(ScreenBoxMax.X, ScreenBoxMin.Y), FVector2D(ScreenBoxMax.X, ScreenBoxMin.Y + Offset.Y), KismetRefColor );

					DrawLine2D(Canvas, FVector2D(ScreenBoxMin.X, ScreenBoxMax.Y), FVector2D(ScreenBoxMin.X, ScreenBoxMax.Y - Offset.Y), KismetRefColor );
					DrawLine2D(Canvas, FVector2D(ScreenBoxMax.X, ScreenBoxMax.Y), FVector2D(ScreenBoxMax.X, ScreenBoxMax.Y - Offset.Y), KismetRefColor );
				}
				else
				{
					// Draw a box when considering the current level.
					DrawLine2D(Canvas, FVector2D(ScreenBoxMin.X, ScreenBoxMin.Y), FVector2D(ScreenBoxMin.X, ScreenBoxMax.Y), KismetRefColor );
					DrawLine2D(Canvas, FVector2D(ScreenBoxMin.X, ScreenBoxMax.Y), FVector2D(ScreenBoxMax.X, ScreenBoxMax.Y), KismetRefColor );
					DrawLine2D(Canvas, FVector2D(ScreenBoxMax.X, ScreenBoxMax.Y), FVector2D(ScreenBoxMax.X, ScreenBoxMin.Y), KismetRefColor );
					DrawLine2D(Canvas, FVector2D(ScreenBoxMax.X, ScreenBoxMin.Y), FVector2D(ScreenBoxMin.X, ScreenBoxMin.Y), KismetRefColor );
				}

				FString KismetDesc = SeqObjVars(i)->ObjComment;
				//debugf(TEXT("%s desc %s (%s)"),*RefActor->GetName(),*KismetDesc,*SeqObjVars(i)->ObjComment);
				if (KismetDesc.Len() > 0)
				{
					DrawStringCentered(Canvas,ScreenBoxMin.X + ((ScreenBoxMax.X - ScreenBoxMin.X) * 0.5f),ScreenBoxMin.Y,*KismetDesc,GEngine->GetMediumFont(),KismetRefColor);
				}
			}

			// Not not to draw this actor again.
			DrawnActors.AddItem(RefActor);
		}
	}
}

void FEditorLevelViewportClient::DrawActorTags(FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		// Actors default to having a tag set to their class name. Ignore those - we only want tags we have explicitly changed.
		if( Actor->Tag != NAME_None && Actor->Tag != Actor->GetClass()->GetFName() )
		{
			FVector2D PixelLocation;
			if(View->ScreenToPixel(View->WorldToScreen(Actor->Location),PixelLocation))
			{
				const FColor TagColor(250,160,50);
				DrawShadowedString(Canvas,PixelLocation.X,PixelLocation.Y, *Actor->Tag.ToString(), GEngine->SmallFont, TagColor);
			}
		}
	}
}

void FEditorLevelViewportClient::DrawAxes(FViewport* Viewport, FCanvas* Canvas, const FRotator* InRotation)
{
	FRotationMatrix ViewTM( this->ViewRotation );

	if( InRotation )
	{
		ViewTM = FRotationMatrix( *InRotation );
	}

	const INT SizeX = Viewport->GetSizeX();
	const INT SizeY = Viewport->GetSizeY();

	const FIntPoint AxisOrigin( 30, SizeY - 30 );
	const FLOAT AxisSize = 25.f;

	INT XL, YL;
	StringSize(GEngine->SmallFont, XL, YL, TEXT("Z"));

	FVector AxisVec = AxisSize * ViewTM.InverseTransformNormal( FVector(1,0,0) );
	FIntPoint AxisEnd = AxisOrigin + FIntPoint( AxisVec.Y, -AxisVec.Z );
	DrawLine2D(Canvas, AxisOrigin, AxisEnd, FColor(255,0,0) );
	DrawString(Canvas, AxisEnd.X + 2, AxisEnd.Y - 0.5*YL, TEXT("X"), GEngine->SmallFont, FColor(255,0,0) );

	AxisVec = AxisSize * ViewTM.InverseTransformNormal( FVector(0,1,0) );
	AxisEnd = AxisOrigin + FIntPoint( AxisVec.Y, -AxisVec.Z );
	DrawLine2D(Canvas, AxisOrigin, AxisEnd, FColor(0,255,0) );
	DrawString(Canvas, AxisEnd.X + 2, AxisEnd.Y - 0.5*YL, TEXT("Y"), GEngine->SmallFont, FColor(0,255,0) );

	AxisVec = AxisSize * ViewTM.InverseTransformNormal( FVector(0,0,1) );
	AxisEnd = AxisOrigin + FIntPoint( AxisVec.Y, -AxisVec.Z );
	DrawLine2D(Canvas, AxisOrigin, AxisEnd, FColor(0,0,255) );
	DrawString(Canvas, AxisEnd.X + 2, AxisEnd.Y - 0.5*YL, TEXT("Z"), GEngine->SmallFont, FColor(0,0,255) );
}

/**
 *	Draw the texture streaming bounds.
 */
void FEditorLevelViewportClient::DrawTextureStreamingBounds(const FSceneView* View,FPrimitiveDrawInterface* PDI)
{
	// Iterate each level
	for (TObjectIterator<ULevel> It; It; ++It)
	{
		ULevel* Level = *It;
		// Grab the streaming bounds entries for the level
		UTexture2D* TargetTexture = NULL;
		TArray<FStreamableTextureInstance>* STIA = Level->GetStreamableTextureInstances(TargetTexture);
		if (STIA)
		{
			for (INT Index = 0; Index < STIA->Num(); Index++)
			{
				FStreamableTextureInstance& STI = (*STIA)(Index);
#if defined(_STREAMING_BOUNDS_DRAW_BOX_)
				FVector InMin = FVector(STI.BoundingSphere.X, STI.BoundingSphere.Y, STI.BoundingSphere.Z);
				FVector InMax = FVector(STI.BoundingSphere.X, STI.BoundingSphere.Y, STI.BoundingSphere.Z);
				FLOAT Max = STI.BoundingSphere.W;
				InMin -= FVector(Max);
				InMax += FVector(Max);
				FBox Box = FBox(InMin, InMax);
				DrawWireBox(PDI, Box, FColor(255,255,0), SDPG_World);
#else	//#if defined(_STREAMING_BOUNDS_DRAW_BOX_)
				// Draw bounding spheres
				FVector Origin = FVector(STI.BoundingSphere.X, STI.BoundingSphere.Y, STI.BoundingSphere.Z);
				FLOAT Radius = STI.BoundingSphere.W;
				DrawCircle(PDI, Origin, FVector(1,0,0), FVector(0,1,0), FColor(255,255,0), Radius, 32, SDPG_World);
				DrawCircle(PDI, Origin, FVector(1,0,0), FVector(0,0,1), FColor(255,255,0), Radius, 32, SDPG_World);
				DrawCircle(PDI, Origin, FVector(0,1,0), FVector(0,0,1), FColor(255,255,0), Radius, 32, SDPG_World);
#endif	//#if defined(_STREAMING_BOUNDS_DRAW_BOX_)
			}
		}
	}
}



/**
 * Copies layout and camera settings from the specified viewport
 *
 * @param InViewport The viewport to copy settings from
 */
void FEditorLevelViewportClient::CopyLayoutFromViewport( const FEditorLevelViewportClient& InViewport )
{
	ViewLocation = InViewport.ViewLocation;
	ViewRotation = InViewport.ViewRotation;
	ViewFOV = InViewport.ViewFOV;
	ViewportType = InViewport.ViewportType;
	OrthoZoom = InViewport.OrthoZoom;
	ShowFlags = InViewport.ShowFlags;
	bMoveUnlit = InViewport.bMoveUnlit;
	bLevelStreamingVolumePrevis = InViewport.bLevelStreamingVolumePrevis;
	bPostProcessVolumePrevis = InViewport.bPostProcessVolumePrevis;
	bViewportLocked = InViewport.bViewportLocked;
	bLockSelectedToCamera = InViewport.bLockSelectedToCamera;
	CameraSpeed = InViewport.CameraSpeed;
	bUseSquintMode = InViewport.bUseSquintMode;
	bAllowMatineePreview = InViewport.bAllowMatineePreview;
}



//
//	FEditorLevelViewportClient::GetScene
//

FSceneInterface* FEditorLevelViewportClient::GetScene()
{
	return GWorld->Scene;
}

//
//	FEditorLevelViewportClient::GetBackgroundColor
//

FLinearColor FEditorLevelViewportClient::GetBackgroundColor()
{
	return (ViewportType == LVT_Perspective) ? GEditor->C_WireBackground : GEditor->C_OrthoBackground;
}

//
//	UEditorViewportInput::Exec
//

UBOOL UEditorViewportInput::Exec(const TCHAR* Cmd,FOutputDevice& Ar)
{
	if(CurrentEvent == IE_Pressed || (CurrentEvent == IE_Released && ParseCommand(&Cmd,TEXT("OnRelease"))))
	{
		if(Editor->Exec(Cmd,Ar))
			return 1;
	}

	return Super::Exec(Cmd,Ar);

}

IMPLEMENT_CLASS(UEditorViewportInput);
