/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "UnPath.h"
#include "FileHelpers.h"
#include "ScopedTransaction.h"
#include "UnFaceFXSupport.h"
#include "Kismet.h"
#include "EngineSequenceClasses.h"
#include "SpeedTree.h"
#include "Properties.h"
#include "GenericBrowser.h"
#include "LevelUtils.h"
#include "EngineMeshClasses.h"

#if WITH_FACEFX
using namespace OC3Ent;
using namespace Face;
#endif

//@hack: this needs to be cleaned up!
static TCHAR TempStr[MAX_EDCMD], TempName[MAX_EDCMD], Temp[MAX_EDCMD];
static WORD Word1, Word2, Word4;

/**
 * Dumps a set of selected objects to debugf.
 */
static void PrivateDumpSelection(USelection* Selection)
{
	for ( USelection::TObjectConstIterator Itor = Selection->ObjectConstItor() ; Itor ; ++Itor )
	{
		UObject *CurObject = *Itor;
		if ( CurObject )
		{
			debugf(TEXT("    %s"), *CurObject->GetClass()->GetName() );
		}
		else
		{
			debugf(TEXT("    NULL object"));
		}
	}
}

UBOOL UUnrealEdEngine::Exec( const TCHAR* Stream, FOutputDevice& Ar )
{
	if( UEditorEngine::Exec( Stream, Ar ) )
	{
		return TRUE;
	}

	const TCHAR* Str = Stream;

	if( ParseCommand(&Str, TEXT("DUMPSELECTION")) )
	{
		debugf(TEXT("Selected Actors:"));
		PrivateDumpSelection( GetSelectedActors() );
		debugf(TEXT("Selected Non-Actors:"));
		PrivateDumpSelection( GetSelectedObjects() );
	}
	//----------------------------------------------------------------------------------
	// EDIT
	//
	if( ParseCommand(&Str,TEXT("EDIT")) )
	{
		if( Exec_Edit( Str, Ar ) )
		{
			return TRUE;
		}
	}
	//------------------------------------------------------------------------------------
	// ACTOR: Actor-related functions
	//
	else if (ParseCommand(&Str,TEXT("ACTOR")))
	{
		if( Exec_Actor( Str, Ar ) )
		{
			return TRUE;
		}
	}
	//------------------------------------------------------------------------------------
	// CTRLTAB: Brings up the ctrl + tab window
	//
	else if(ParseCommand(&Str, TEXT("CTRLTAB")))
	{
		UBOOL bIsShiftDown = FALSE;

		ParseUBOOL(Str, TEXT("SHIFTDOWN="), bIsShiftDown);

		WxTrackableWindowBase::HandleCtrlTab(GApp->EditorFrame, bIsShiftDown);
	}
	//------------------------------------------------------------------------------------
	// SKELETALMESH: SkeletalMesh-related functions
	//
	else if(ParseCommand(&Str, TEXT("SKELETALMESH")))
	{
		if(Exec_SkeletalMesh(Str, Ar))
		{
			return TRUE;
		}
	}
	//------------------------------------------------------------------------------------
	// MODE management (Global EDITOR mode):
	//
	else if( ParseCommand(&Str,TEXT("MODE")) )
	{
		if( Exec_Mode( Str, Ar ) )
		{
			return TRUE;
		}
	}
	//----------------------------------------------------------------------------------
	// PIVOT
	//
	else if( ParseCommand(&Str,TEXT("PIVOT")) )
	{
		if(		Exec_Pivot( Str, Ar ) )
		{
			return TRUE;
		}
	}
	//----------------------------------------------------------------------------------
	// QUERY VALUE
	//
	else if (ParseCommand(&Str, TEXT("QUERYVALUE")))
	{
		FString Key;
		// get required key value
		if (!ParseToken(Str, Key, FALSE))
		{
			return FALSE;
		}

		FString Label;
		// get required prompt
		if (!ParseToken(Str, Label, FALSE))
		{
			return FALSE;
		}

		FString Default;
		// default is optional
		ParseToken(Str, Default, FALSE);

		wxTextEntryDialog Dlg(NULL, *Label, *Key, *Default);

		if(Dlg.ShowModal() == wxID_OK)
		{
			// if the user hit OK, pass back the result in the OutputDevice
			Ar.Log(Dlg.GetValue());
		}

		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("UNMOUNTALLFACEFX")) )
	{
#if WITH_FACEFX
		for( TObjectIterator<UFaceFXAsset> It; It; ++It )
		{
			UFaceFXAsset* Asset = *It;
			FxActor* fActor = Asset->GetFxActor();
			if(fActor)
			{
				// If its open in studio - do not modify it (warn instead).
				if(fActor->IsOpenInStudio())
				{
					appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("CannotUnmountFaceFXOpenInStudio"), *Asset->GetPathName()));
				}
				else
				{
					// Copy array, as we will be unmounting things and changing the one on the asset!
					TArray<UFaceFXAnimSet*> MountedSets = Asset->MountedFaceFXAnimSets;
					for(INT i=0; i<MountedSets.Num(); i++)
					{
						UFaceFXAnimSet* Set = MountedSets(i);
						Asset->UnmountFaceFXAnimSet(Set);
						debugf( TEXT("Unmounting: %s From %s"), *Set->GetName(), *Asset->GetName() );
					}
				}
			}
		}
#endif // WITH_FACEFX

		return TRUE;
	}
	else
	{
		UUISceneManager* SceneManager = GetBrowserManager()->UISceneManager;
		if ( SceneManager != NULL && SceneManager->Exec(Stream, Ar) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** @return Returns whether or not the user is able to autosave. **/
UBOOL UUnrealEdEngine::CanAutosave() const
{
	return ( AutoSave && !GIsSlowTask && GEditorModeTools().GetCurrentModeID() != EM_InterpEdit && !PlayWorld );
}

/** @return Returns whether or not autosave is going to occur within the next minute. */
UBOOL UUnrealEdEngine::AutoSaveSoon() const
{
	UBOOL bResult = FALSE;
	if(CanAutosave())
	{
		FLOAT TimeTillAutoSave = (FLOAT)AutosaveTimeMinutes - AutosaveCount/60.0f;
		bResult = TimeTillAutoSave < 1.0f && TimeTillAutoSave > 0.0f;
	}

	return bResult;
}

/** @return Returns the amount of time until the next autosave in seconds. */
INT UUnrealEdEngine::GetTimeTillAutosave() const
{
	INT Result = -1;

	if(CanAutosave())
	{
		 Result = appTrunc(60*(AutosaveTimeMinutes - AutosaveCount/60.0f));
	}

	return Result;
}



/**
 * Checks to see if any worlds are dirty (that is, they need to be saved.)
 *
 * @return TRUE if any worlds are dirty
 */
UBOOL UUnrealEdEngine::AnyWorldsAreDirty() const
{
	// Get the set of all reference worlds.
	TArray<UWorld*> WorldsArray;
	FLevelUtils::GetWorlds( WorldsArray, TRUE );

	if ( WorldsArray.Num() > 0 )
	{
		FString FinalFilename;
		for ( INT WorldIndex = 0 ; WorldIndex < WorldsArray.Num() ; ++WorldIndex )
		{
			UWorld* World = WorldsArray( WorldIndex );
			UPackage* Package = Cast<UPackage>( World->GetOuter() );
			check( Package );

			// If this world needs saving . . .
			if ( Package->IsDirty() )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}



void UUnrealEdEngine::AttemptLevelAutosave()
{
	// Don't autosave if disabled or if it is not yet time to autosave.
	const UBOOL bTimeToAutosave = ( AutoSave && (AutosaveCount/60.0f >= (FLOAT)AutosaveTimeMinutes) );
	if ( bTimeToAutosave )
	{
		// Don't autosave during interpolation editing, if there's another slow task
		// already in progress, or while a PIE world is playing.
		const UBOOL bCanAutosave = CanAutosave();

		if( bCanAutosave )
		{
			// Check to see if the user is in the middle of a drag operation.
			UBOOL bUserIsInteracting = FALSE;
			for( INT ClientIndex = 0 ; ClientIndex < ViewportClients.Num() ; ++ClientIndex )
			{
				if ( ViewportClients(ClientIndex)->bIsTracking )
				{
					bUserIsInteracting = TRUE;
					break;
				}
			}

			// Don't interrupt the user with an autosave.
			if ( !bUserIsInteracting )
			{
				SaveConfig();

				// Make sure the autosave directory exists before attempting to write the file.
				const FString AbsoluteAutoSaveDir( FString(appBaseDir()) * AutoSaveDir );
				GFileManager->MakeDirectory( *AbsoluteAutoSaveDir, 1 );

				// Autosave all levels.
				const INT NewAutoSaveIndex = (AutoSaveIndex+1)%10;
				const UBOOL bLevelSaved = FEditorFileUtils::AutosaveMap( AbsoluteAutoSaveDir, NewAutoSaveIndex );

				if ( bLevelSaved )
				{
					// If a level was actually saved, update the autosave index.
					AutoSaveIndex = NewAutoSaveIndex;
				}

				ResetAutosaveTimer();
			}
		}
	}
}

UBOOL UUnrealEdEngine::Exec_Edit( const TCHAR* Str, FOutputDevice& Ar )
{
	if( ParseCommand(&Str,TEXT("CUT")) )
	{
		CopySelectedActorsToClipboard( TRUE, TRUE );
	}
	else if( ParseCommand(&Str,TEXT("COPY")) )
	{
		CopySelectedActorsToClipboard( FALSE, TRUE );
	}
	else if( ParseCommand(&Str,TEXT("PASTE")) )
	{
		const FVector SaveClickLocation = GEditor->ClickLocation;

		enum EPasteTo
		{
			PT_OriginalLocation	= 0,
			PT_Here				= 1,
			PT_WorldOrigin		= 2
		} PasteTo = PT_OriginalLocation;

		FString TransName = *LocalizeUnrealEd("Paste");
		if( Parse( Str, TEXT("TO="), TempStr, 15 ) )
		{
			if( !appStrcmp( TempStr, TEXT("HERE") ) )
			{
				PasteTo = PT_Here;
				TransName = *LocalizeUnrealEd("PasteHere");
			}
			else
			{
				if( !appStrcmp( TempStr, TEXT("ORIGIN") ) )
				{
					PasteTo = PT_WorldOrigin;
					TransName = *LocalizeUnrealEd("PasteToWorldOrigin");
				}
			}
		}

		const FScopedTransaction Transaction( *TransName );

		GEditor->SelectNone( TRUE, FALSE );
		edactPasteSelected( FALSE, FALSE, TRUE );

		if( PasteTo != PT_OriginalLocation )
		{
			// Get a bounding box for all the selected actors locations.
			FBox bbox(0);
			INT NumActorsToMove = 0;

			for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
			{
				AActor* Actor = static_cast<AActor*>( *It );
				checkSlow( Actor->IsA(AActor::StaticClass()) );

				bbox += Actor->Location;
				++NumActorsToMove;
			}

			if ( NumActorsToMove > 0 )
			{
				// Figure out which location to center the actors around.
				const FVector Origin( PasteTo == PT_Here ? SaveClickLocation : FVector(0,0,0) );

				// Compute how far the actors have to move.
				const FVector Location = bbox.GetCenter();
				const FVector Adjust = Origin - Location;

				// Move the actors.
				AActor* SingleActor = NULL;
				for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
				{
					AActor* Actor = static_cast<AActor*>( *It );
					checkSlow( Actor->IsA(AActor::StaticClass()) );

					SingleActor = Actor;
					Actor->Location += Adjust;
					Actor->ForceUpdateComponents();
				}

				// Update the pivot location.
				check(SingleActor);
				SetPivot( SingleActor->Location, FALSE, FALSE, TRUE );
			}
		}

		RedrawLevelEditingViewports();
	}

	return FALSE;
}

UBOOL UUnrealEdEngine::Exec_Pivot( const TCHAR* Str, FOutputDevice& Ar )
{
	if( ParseCommand(&Str,TEXT("HERE")) )
	{
		const UBOOL bAssignToPrePivot = ParseCommand(&Str,TEXT("ASSIGN"));
		NoteActorMovement();
		SetPivot( ClickLocation, FALSE, bAssignToPrePivot, FALSE );
		FinishAllSnaps();
		RedrawLevelEditingViewports();
	}
	else if( ParseCommand(&Str,TEXT("SNAPPED")) )
	{
		const UBOOL bAssignToPrePivot = ParseCommand(&Str,TEXT("ASSIGN"));
		NoteActorMovement();
		SetPivot( ClickLocation, TRUE, bAssignToPrePivot, FALSE );
		FinishAllSnaps();
		RedrawLevelEditingViewports();
	}
	else if( ParseCommand(&Str,TEXT("CENTERSELECTION")) )
	{
		NoteActorMovement();

		// Figure out the center location of all selections

		INT Count = 0;
		FVector Center(0,0,0);

		for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );

			Center += Actor->Location;
			Count++;
		}

		if( Count > 0 )
		{
			ClickLocation = Center / Count;

			SetPivot( ClickLocation, FALSE, FALSE, FALSE );
			FinishAllSnaps();
		}

		RedrawLevelEditingViewports();
	}

	return FALSE;
}

static void MirrorActors(const FVector& MirrorScale)
{
	const FScopedTransaction Transaction( *LocalizeUnrealEd("MirroringActors") );

	// Fires CALLBACK_LevelDirtied when falling out of scope.
	FScopedLevelDirtied		LevelDirtyCallback;

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if( Actor->IsABrush() )
		{
			const FVector LocalToWorldOffset = Actor->Location - GEditorModeTools().PivotLocation;

			ABrush* Brush = (ABrush*)Actor;
			Brush->Brush->Modify();

			Brush->PrePivot *= MirrorScale;

			for( INT poly = 0 ; poly < Brush->Brush->Polys->Element.Num() ; poly++ )
			{
				FPoly* Poly = &(Brush->Brush->Polys->Element(poly));

				Poly->TextureU *= MirrorScale;
				Poly->TextureV *= MirrorScale;

				Poly->Base += LocalToWorldOffset;
				Poly->Base *= MirrorScale;
				Poly->Base -= LocalToWorldOffset;

				for( INT vtx = 0 ; vtx < Poly->Vertices.Num(); vtx++ )
				{
					Poly->Vertices(vtx) += LocalToWorldOffset;
					Poly->Vertices(vtx) *= MirrorScale;
					Poly->Vertices(vtx) -= LocalToWorldOffset;
				}

				Poly->Reverse();
				Poly->CalcNormal();
			}

			Brush->ClearComponents();
		}
		else
		{
			const FRotationMatrix TempRot( Actor->Rotation );
			const FVector New0( TempRot.GetAxis(0) * MirrorScale );
			const FVector New1( TempRot.GetAxis(1) * MirrorScale );
			const FVector New2( TempRot.GetAxis(2) * MirrorScale );
			// Revert the handedness of the rotation, but make up for it in the scaling.
			// Arbitrarily choose the X axis to remain fixed.
			const FMatrix NewRot( -New0, New1, New2, FVector(0,0,0) );

			Actor->Modify();
			Actor->DrawScale3D.X = -Actor->DrawScale3D.X;
			Actor->Rotation = NewRot.Rotator();
			Actor->Location -= GEditorModeTools().PivotLocation - Actor->PrePivot;
			Actor->Location *= MirrorScale;
			Actor->Location += GEditorModeTools().PivotLocation - Actor->PrePivot;
		}

		Actor->InvalidateLightingCache();
		Actor->PostEditMove( TRUE );

		Actor->MarkPackageDirty();
		LevelDirtyCallback.Request();
	}

	GEditor->RedrawLevelEditingViewports();
}

/**
* Gathers up a list of selection FPolys from selected static meshes.
*
* @return	A TArray containing FPolys representing the triangles in the selected static meshes (note that these
*           triangles are transformed into world space before being added to the array.
*/

TArray<FPoly*> GetSelectedPolygons()
{
	// Build a list of polygons from all selected static meshes

	TArray<FPoly*> SelectedPolys;

	for( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		for(INT j=0; j<Actor->AllComponents.Num(); j++)
		{
			// If its a static mesh component, with a static mesh
			UActorComponent* Comp = Actor->AllComponents(j);
			UStaticMeshComponent* SMComp = Cast<UStaticMeshComponent>(Comp);
			if(SMComp && SMComp->StaticMesh)
			{
				UStaticMesh* StaticMesh = SMComp->StaticMesh;
				const FStaticMeshTriangle* RawTriangleData = (FStaticMeshTriangle*) StaticMesh->LODModels(0).RawTriangles.Lock(LOCK_READ_ONLY);

				if( StaticMesh->LODModels(0).RawTriangles.GetElementCount() )
				{
					for( INT TriangleIndex = 0 ; TriangleIndex < StaticMesh->LODModels(0).RawTriangles.GetElementCount() ; TriangleIndex++ )
					{
						const FStaticMeshTriangle& Triangle = RawTriangleData[TriangleIndex];
						FPoly* Polygon = new FPoly;

						// Add the poly

						Polygon->Init();
						Polygon->PolyFlags = PF_DefaultFlags;

						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[2] ) );
						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[1] ) );
						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[0] ) );

						Polygon->CalcNormal(1);
						Polygon->Fix();
						if( Polygon->Vertices.Num() > 2 )
						{
							if( !Polygon->Finalize( NULL, 1 ) )
							{
								SelectedPolys.AddItem( Polygon );
							}
						}

						// And add a flipped version of it to account for negative scaling

						Polygon = new FPoly;

						Polygon->Init();
						Polygon->PolyFlags = PF_DefaultFlags;

						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[2] ) );
						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[0] ) );
						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[1] ) );

						Polygon->CalcNormal(1);
						Polygon->Fix();
						if( Polygon->Vertices.Num() > 2 )
						{
							if( !Polygon->Finalize( NULL, 1 ) )
							{
								SelectedPolys.AddItem( Polygon );
							}
						}
					}
				}
				StaticMesh->LODModels(0).RawTriangles.Unlock();
			}
		}
	}

	return SelectedPolys;
}

/**
* Creates an axis aligned bounding box based on the bounds of SelectedPolys.  This bounding box
* is then copied into the builder brush.  This function is a set up function that the blocking volume
* creation execs will call before doing anything fancy.
*
* @param	SelectedPolys	The list of selected FPolys to create the bounding box from.
*/

void CreateBoundingBoxBuilderBrush( const TArray<FPoly*> SelectedPolys )
{
	int x;
	FPoly* poly;
	FBox bbox(0);

	for( x = 0 ; x < SelectedPolys.Num() ; ++x )
	{
		poly = SelectedPolys(x);

		for( int v = 0 ; v < poly->Vertices.Num() ; ++v )
		{
			bbox += poly->Vertices(v);
		}
	}

	// Change the builder brush to match the bounding box so that it exactly envelops the selected meshes

	FVector extent = bbox.GetExtent();
	UCubeBuilder* CubeBuilder = ConstructObject<UCubeBuilder>( UCubeBuilder::StaticClass() );
	CubeBuilder->X = extent.X * 2;
	CubeBuilder->Y = extent.Y * 2;
	CubeBuilder->Z = extent.Z * 2;
	CubeBuilder->eventBuild();

	GWorld->GetBrush()->Location = bbox.GetCenter();

	GWorld->GetBrush()->ClearComponents();
	GWorld->GetBrush()->ConditionalUpdateComponents();
}

/**
* Take a plane and creates a gigantic triangle polygon that lies along it.  The blocking
* volume creation routines call this when they are cutting geometry and need to create
* capping polygons.
*
* This polygon is so huge that it doesn't matter where the vertices actually land.
*
* @param	InPlane		The plane to lay the polygon on
* @return	An FPoly representing the giant triangle we created (NULL if there was a problem)
*/

FPoly* CreateHugeTrianglePolygonOnPlane( const FPlane* InPlane )
{
	// Using the plane normal, get 2 good axis vectors

	FVector A, B;
	InPlane->SafeNormal().FindBestAxisVectors( A, B );

	// Create 4 vertices from the plane origin and the 2 axis generated above

	FPoly* Triangle = new FPoly();

	FVector Center = FVector( InPlane->X, InPlane->Y, InPlane->Z ) * InPlane->W;
	FVector V0 = Center + (A * WORLD_MAX);
	FVector V1 = Center + (B * WORLD_MAX);
	FVector V2 = Center - (((A + B) / 2.0f) * WORLD_MAX);

	// Create a triangle that lays on InPlane

	Triangle->Init();
	Triangle->PolyFlags = PF_DefaultFlags;

	new(Triangle->Vertices) FVector( V0 );
	new(Triangle->Vertices) FVector( V2 );
	new(Triangle->Vertices) FVector( V1 );

	Triangle->CalcNormal(1);
	Triangle->Fix();
	if( Triangle->Finalize( NULL, 1 ) )
	{
		delete Triangle;
		Triangle = NULL;
	}

	return Triangle;
}

/**
* Utility function to quickly set the collision type on selected static meshes.
*
* @param	InCollisionType		The collision type to use (COLLIDE_??)
*/

void SetCollisionTypeOnSelectedActors( BYTE InCollisionType )
{
	for( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );
		AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);

		if( StaticMeshActor )
		{
			StaticMeshActor->Modify();
			StaticMeshActor->CollisionType = InCollisionType;
			StaticMeshActor->SetCollisionFromCollisionType();
		}
	}
}

UBOOL UUnrealEdEngine::Exec_SkeletalMesh( const TCHAR* Str, FOutputDevice& Ar )
{
	//This command sets the offset and orientation for all skeletal meshes within the set of currently selected packages
	if(ParseCommand(&Str, TEXT("CHARBITS"))) //SKELETALMESH CHARBITS
	{
		FVector Offset = FVector(0.0f, 0.0f, 0.0f);
		FRotator Orientation = FRotator(0, 0, 0);
		UBOOL bHasOffset = GetFVECTOR(Str, TEXT("OFFSET="), Offset);
		
		TCHAR Temp[80];
		UBOOL bHasOrientation = GetSUBSTRING(Str, TEXT("ORIENTATION="), Temp, 80);
		
		//If orientation is present do custom parsing to allow for a proper conversion from a floating point representation of degress
		//to its integer representation in FRotator. GetFROTATOR() does not allow us to do this.
		if(bHasOrientation)
		{
			FLOAT Value = 0.0f;
			
			if(Parse(Temp, TEXT("YAW="), Value))
			{
				Value = appFmod(Value, 360.0f); //Make sure it's in the range 0-360
				Orientation.Yaw = (INT)(Value / 360.0f * 65536); //Convert the angle to int craziness
			}

			if(Parse(Temp, TEXT("PITCH="), Value))
			{
				Value = appFmod(Value, 360.0f); //Make sure it's in the range 0-360
				Orientation.Pitch = (INT)(Value / 360.0f * 65536); //Convert the angle to int craziness
			}

			if(Parse(Temp, TEXT("ROLL="), Value))
			{
				Value = appFmod(Value, 360.0f); //Make sure it's in the range 0-360
				Orientation.Roll = (INT)(Value / 360.0f * 65536); //Convert the angle to int craziness
			}
		}

		WxGenericBrowser* GenericBrowser = GetBrowser<WxGenericBrowser>(TEXT("GenericBrowser"));
		if(GenericBrowser && (bHasOffset || bHasOrientation))
		{
			TArray<UPackage*> Packages;
			GenericBrowser->LeftContainer->GetSelectedPackages(&Packages);

			for(TObjectIterator<USkeletalMesh> Iter; Iter; ++Iter)
			{
				for(INT PackageIndex = 0; PackageIndex < Packages.Num(); ++PackageIndex)
				{
					UPackage* CurrentPackage = Packages(PackageIndex);
					if(Iter->IsIn(CurrentPackage))
					{
						if(bHasOffset)
						{
							Iter->Origin = Offset;
						}

						if(bHasOrientation)
						{
							Iter->RotOrigin = Orientation;
						}

						//To get to this point offset or orientation must be present so mark package as dirty
						Iter->MarkPackageDirty();
						break;
					}
				}
			}

			GCallbackEvent->Send(CALLBACK_RefreshEditor_GenericBrowser);
		}

		return TRUE;
	}

	return FALSE;
}

UBOOL UUnrealEdEngine::Exec_Actor( const TCHAR* Str, FOutputDevice& Ar )
{
	if( ParseCommand(&Str,TEXT("ADD")) )
	{
		UClass* Class;
		if( ParseObject<UClass>( Str, TEXT("CLASS="), Class, ANY_PACKAGE ) )
		{
			AActor* Default   = Class->GetDefaultActor();
			FVector Collision = Default->GetCylinderExtent();
			INT bSnap = 1;
			Parse(Str,TEXT("SNAP="),bSnap);
			if( bSnap )
			{
				Constraints.Snap( ClickLocation, FVector(0, 0, 0) );
			}
			FVector Location = ClickLocation + ClickPlane * (FBoxPushOut(ClickPlane,Collision) + 0.1);
			if( bSnap )
			{
				Constraints.Snap( Location, FVector(0, 0, 0) );
			}
			AddActor( Class, Location );
			RedrawLevelEditingViewports();
			return TRUE;
		}
	}
	else if( ParseCommand(&Str,TEXT("CREATE_BV_BOUNDINGBOX")) )
	{
		const FScopedTransaction Transaction( TEXT("Create Bounding Box Blocking Volume") );
		GWorld->GetBrush()->Modify();

		// Create a bounding box for the selected static mesh triangles and set the builder brush to match it

		TArray<FPoly*> SelectedPolys = GetSelectedPolygons();
		CreateBoundingBoxBuilderBrush( SelectedPolys );

		// Create the blocking volume

		GUnrealEd->Exec( TEXT("BRUSH ADDVOLUME CLASS=BlockingVolume") );

		// Set up collision on selected actors

		SetCollisionTypeOnSelectedActors( COLLIDE_BlockWeapons );

		// Clean up memory

		for( int x = 0 ; x < SelectedPolys.Num() ; ++x )
		{
			delete SelectedPolys(x);
		}

		SelectedPolys.Empty();

		// Finish up

		RedrawLevelEditingViewports();
		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("CREATE_BV_CONVEXVOLUME")) )
	{
		const FScopedTransaction Transaction( TEXT("Create Convex Blocking Volume") );
		GWorld->GetBrush()->Modify();

		// The rejection tolerance.  When figuring out which planes to cut the blocking volume cube with
		// the code will reject any planes that are less than "NormalTolerance" different in their normals.
		//
		// This cuts down on the number of planes that will be used for generating the cutting planes and,
		// as a side effect, eliminates duplicates.

		FLOAT NormalTolerance = 0.25f;
		Parse( Str, TEXT("NORMALTOLERANCE="), NormalTolerance );

		FVector NormalLimits( 1.0f, 1.0f, 1.0f );
		Parse( Str, TEXT("NLIMITX="), NormalLimits.X );
		Parse( Str, TEXT("NLIMITY="), NormalLimits.Y );
		Parse( Str, TEXT("NLIMITZ="), NormalLimits.Z );

		// Create a bounding box for the selected static mesh triangles and set the builder brush to match it

		TArray<FPoly*> SelectedPolys = GetSelectedPolygons();
		CreateBoundingBoxBuilderBrush( SelectedPolys );

		// Get a list of the polygons that make up the builder brush

		FPoly* poly;
		TArray<FPoly>* BuilderBrushPolys = new TArray<FPoly>( GWorld->GetBrush()->Brush->Polys->Element );

		// Create a list of valid splitting planes

		TArray<FPlane*> SplitterPlanes;

		for( int p = 0 ; p < SelectedPolys.Num() ; ++p )
		{
			// Get a splitting plane from the first poly in our selection

			poly = SelectedPolys(p);
			FPlane* SplittingPlane = new FPlane( poly->Vertices(0), poly->Normal );

			// Make sure this poly doesn't clip any other polys in the selection.  If it does, we can't use it for generating the convex volume.

			UBOOL bUseThisSplitter = TRUE;

			for( int pp = 0 ; pp < SelectedPolys.Num() && bUseThisSplitter ; ++pp )
			{
				FPoly* ppoly = SelectedPolys(pp);

				if( p != pp && !(poly->Normal - ppoly->Normal).IsNearlyZero() )
				{
					int res = ppoly->SplitWithPlaneFast( *SplittingPlane, NULL, NULL );

					if( res == SP_Split || res == SP_Front )
					{
						// Whoops, this plane clips polygons (and/or sits between static meshes) in the selection so it can't be used
						bUseThisSplitter = FALSE;
					}
				}
			}

			// If this polygons plane doesn't clip the selection in any way, we can carve the builder brush with it. Save it.

			if( bUseThisSplitter )
			{
				// Move the plane into the same coordinate space as the builder brush

				*SplittingPlane = SplittingPlane->TransformBy( GWorld->GetBrush()->WorldToLocal() );

				// Before keeping this plane, make sure there aren't any existing planes that have a normal within the rejection tolerance.

				UBOOL bAddPlaneToList = TRUE;

				for( int x = 0 ; x < SplitterPlanes.Num() ; ++x )
				{
					FPlane* plane = SplitterPlanes(x);

					if( plane->SafeNormal().Equals( SplittingPlane->SafeNormal(), NormalTolerance ) )
					{
						bAddPlaneToList = FALSE;
						break;
					}
				}

				// As a final test, make sure that this planes normal falls within the normal limits that were defined

				if( Abs( SplittingPlane->SafeNormal().X ) > NormalLimits.X )
				{
					bAddPlaneToList = FALSE;
				}
				if( Abs( SplittingPlane->SafeNormal().Y ) > NormalLimits.Y )
				{
					bAddPlaneToList = FALSE;
				}
				if( Abs( SplittingPlane->SafeNormal().Z ) > NormalLimits.Z )
				{
					bAddPlaneToList = FALSE;
				}

				// If this plane passed every test - it's a keeper!

				if( bAddPlaneToList )
				{
					SplitterPlanes.AddItem( SplittingPlane );
				}
				else
				{
					delete SplittingPlane;
				}
			}
		}

		// The builder brush is a bounding box at this point that fully surrounds the selected static meshes.
		// Now we will carve away at it using the splitting planes we collected earlier.  When this process
		// is complete, we will have a convex volume inside of the builder brush that can then be used to add
		// a blocking volume.

		TArray<FPoly> NewBuilderBrushPolys;

		for( int sp = 0 ; sp < SplitterPlanes.Num() ; ++sp )
		{
			FPlane* plane = SplitterPlanes(sp);

			// Carve the builder brush with each splitting plane we collected.  We place the results into
			// NewBuilderBrushPolys since we don't want to overwrite the original array just yet.

			UBOOL bNeedCapPoly = TRUE;

			for( int bp = 0 ; bp < BuilderBrushPolys->Num() ; ++bp )
			{
				FPoly* poly = &(*BuilderBrushPolys)(bp);

				FPoly Front, Back;
				int res = poly->SplitWithPlane( FVector( plane->X, plane->Y, plane->Z ) * plane->W, plane->SafeNormal(), &Front, &Back, TRUE );
				switch( res )
				{
					// Ignore these results.  We don't want them.
					case SP_Coplanar:
					case SP_Front:
						break;

					// In the case of a split, keep the polygon on the back side of the plane.
					case SP_Split:
					{
						NewBuilderBrushPolys.AddItem( Back );
						bNeedCapPoly = TRUE;
					}
					break;

					// By default, just keep the polygon that we had.
					default:
					{
						NewBuilderBrushPolys.AddItem( (*BuilderBrushPolys)(bp) );
					}
					break;
				}
			}

			// NewBuilderBrushPolys contains the newly clipped polygons so copy those into
			// the real array of polygons.

			BuilderBrushPolys = new TArray<FPoly>( NewBuilderBrushPolys );
			NewBuilderBrushPolys.Empty();

			// If any splitting occured, we need to generate a cap polygon to cover the hole.

			if( bNeedCapPoly )
			{
				// Create a large triangle polygon that covers the newly formed hole in the builder brush.

				FPoly* CappingPoly = CreateHugeTrianglePolygonOnPlane( plane );

				if( CappingPoly )
				{
					// Now we do the clipping the other way around.  We are going to use the polygons in the builder brush to
					// create planes which will clip the huge triangle polygon we just created.  When this process is over,
					// we will be left with a new polygon that covers the newly formed hole in the builder brush.

					for( int bp = 0 ; bp < BuilderBrushPolys->Num() ; ++bp )
					{
						FPoly* poly = &((*BuilderBrushPolys)(bp));
						FPlane* plane = new FPlane( poly->Vertices(0), poly->Vertices(1), poly->Vertices(2) );

						FPoly Front, Back;
						int res = CappingPoly->SplitWithPlane( FVector( plane->X, plane->Y, plane->Z ) * plane->W, plane->SafeNormal(), &Front, &Back, TRUE );
						switch( res )
						{
							case SP_Split:
							{
								*CappingPoly = Back;
							}
							break;
						}
					}

					// Add that new polygon into the builder brush polys as a capping polygon.

					BuilderBrushPolys->AddItem( *CappingPoly );
				}
			}
		}

		// Create a new builder brush from the freshly clipped polygons.

		GWorld->GetBrush()->Brush->Polys->Element.Empty();

		for( int x = 0 ; x < BuilderBrushPolys->Num() ; ++x )
		{
			GWorld->GetBrush()->Brush->Polys->Element.AddItem( (*BuilderBrushPolys)(x) );
		}

		GWorld->GetBrush()->ClearComponents();
		GWorld->GetBrush()->ConditionalUpdateComponents();

		// Create the blocking volume

		GUnrealEd->Exec( TEXT("BRUSH ADDVOLUME CLASS=BlockingVolume") );

		// Set up collision on selected actors

		SetCollisionTypeOnSelectedActors( COLLIDE_BlockWeapons );

		// Clean up memory

		for( int x = 0 ; x < SelectedPolys.Num() ; ++x )
		{
			delete SelectedPolys(x);
		}

		SelectedPolys.Empty();

		for( int x = 0 ; x < SplitterPlanes.Num() ; ++x )
		{
			delete SplitterPlanes(x);
		}

		SplitterPlanes.Empty();

		delete BuilderBrushPolys;

		// Finish up

		RedrawLevelEditingViewports();
		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("MIRROR")) )
	{
		FVector MirrorScale( 1, 1, 1 );
		GetFVECTOR( Str, MirrorScale );
		// We can't have zeroes in the vector
		if( !MirrorScale.X )		MirrorScale.X = 1;
		if( !MirrorScale.Y )		MirrorScale.Y = 1;
		if( !MirrorScale.Z )		MirrorScale.Z = 1;
		MirrorActors( MirrorScale );
		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("HIDE")) )
	{
		if( ParseCommand(&Str,TEXT("SELECTED")) ) // ACTOR HIDE SELECTED
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("HideSelectedActors") );
			edactHideSelected();
			SelectNone( TRUE, TRUE );
			return TRUE;
		}
		else if( ParseCommand(&Str,TEXT("UNSELECTED")) ) // ACTOR HIDE UNSELECTEED
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("HideUnselected") );
			edactHideUnselected();
			SelectNone( TRUE, TRUE );
			return TRUE;
		}
	}
	else if( ParseCommand(&Str,TEXT("UNHIDE")) ) // ACTOR UNHIDE ALL
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd("UnHideAll") );
		edactUnHideAll();
		return TRUE;
	}
	else if( ParseCommand(&Str, TEXT("APPLYTRANSFORM")) )
	{
		appErrorf(LocalizeSecure(LocalizeUnrealEd("Error_TriedToExecDeprecatedCmd"),Str));
	}
	else if( ParseCommand(&Str, TEXT("REPLACE")) )
	{
		UClass* Class;
		if( ParseCommand(&Str, TEXT("BRUSH")) ) // ACTOR REPLACE BRUSH
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("ReplaceSelectedBrushActors") );
			edactReplaceSelectedBrush();
			return TRUE;
		}
		else if( ParseObject<UClass>( Str, TEXT("CLASS="), Class, ANY_PACKAGE ) ) // ACTOR REPLACE CLASS=<class>
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("ReplaceSelectedNonBrushActors") );
			edactReplaceSelectedNonBrushWithClass( Class );
			return TRUE;
		}
	}
	else if( ParseCommand(&Str,TEXT("SELECT")) )
	{
		if( ParseCommand(&Str,TEXT("NONE")) ) // ACTOR SELECT NONE
		{
			return Exec( TEXT("SELECT NONE") );
		}
		else if( ParseCommand(&Str,TEXT("ALL")) ) // ACTOR SELECT ALL
		{
			if(ParseCommand(&Str, TEXT("FROMOBJ"))) // ACTOR SELECT ALL FROMOBJ
			{		
				UBOOL bHasStaticMeshes = FALSE;
				UBOOL bHasSpeedTrees = FALSE;
				TArray<UClass*> ClassesToSelect;

				for(FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
				{
					AActor* Actor = static_cast<AActor*>(*It);
					checkSlow(Actor->IsA(AActor::StaticClass()));

					if( Actor->IsA(AStaticMeshActor::StaticClass()) || Actor->IsA(ADynamicSMActor::StaticClass()) || Actor->IsA(AFracturedStaticMeshActor::StaticClass()) )
					{
						bHasStaticMeshes = TRUE;
					}
					else if(Actor->IsA(ASpeedTreeActor::StaticClass()))
					{
						bHasSpeedTrees = TRUE;
					}
					else
					{
						ClassesToSelect.AddUniqueItem(Actor->GetClass());
					}
				}

				const FScopedTransaction Transaction(*LocalizeUnrealEd("SelectAll"));
				if(bHasStaticMeshes)
				{
					edactSelectMatchingStaticMesh(FALSE);
				}

				if(bHasSpeedTrees)
				{
					GUnrealEd->SelectMatchingSpeedTrees();
				}

				if(ClassesToSelect.Num() > 0)
				{
					for(int Index = 0; Index < ClassesToSelect.Num(); ++Index)
					{
						edactSelectOfClass(ClassesToSelect(Index));
					}
				}

				return TRUE;
			}
			else
			{
				const FScopedTransaction Transaction( *LocalizeUnrealEd("SelectAll") );
				edactSelectAll();
				return TRUE;
			}
		}
		else if( ParseCommand(&Str,TEXT("INSIDE") ) ) // ACTOR SELECT INSIDE
		{
			appErrorf(LocalizeSecure(LocalizeUnrealEd("Error_TriedToExecDeprecatedCmd"),Str));
		}
		else if( ParseCommand(&Str,TEXT("INVERT") ) ) // ACTOR SELECT INVERT
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("SelectInvert") );
			edactSelectInvert();
			return TRUE;
		}
		else if( ParseCommand(&Str,TEXT("OFCLASS")) ) // ACTOR SELECT OFCLASS CLASS=<class>
		{
			UClass* Class;
			if( ParseObject<UClass>(Str,TEXT("CLASS="),Class,ANY_PACKAGE) )
			{
				const FScopedTransaction Transaction( *LocalizeUnrealEd("SelectOfClass") );
				edactSelectOfClass( Class );
			}
			else
			{
				Ar.Log( NAME_ExecWarning, TEXT("Missing class") );
			}
			return TRUE;
		}
		else if( ParseCommand(&Str,TEXT("OFSUBCLASS")) ) // ACTOR SELECT OFSUBCLASS CLASS=<class>
		{
			UClass* Class;
			if( ParseObject<UClass>(Str,TEXT("CLASS="),Class,ANY_PACKAGE) )
			{
				const FScopedTransaction Transaction( *LocalizeUnrealEd("SelectSubclassOfClass") );
				edactSelectSubclassOf( Class );
			}
			else
			{
				Ar.Log( NAME_ExecWarning, TEXT("Missing class") );
			}
			return TRUE;
		}
		else if( ParseCommand(&Str,TEXT("BASED")) ) // ACTOR SELECT BASED
		{
			edactSelectBased();
			return TRUE;
		}
		else if( ParseCommand(&Str,TEXT("BYPROPERTY")) ) // ACTOR SELECT BYPROPERTY
		{
			GEditor->SelectByPropertyColoration();
			return TRUE;
		}
		else if( ParseCommand(&Str,TEXT("DELETED")) ) // ACTOR SELECT DELETED
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("SelectDeleted") );
			edactSelectDeleted();
			return TRUE;
		}
		else if( ParseCommand(&Str,TEXT("MATCHINGSTATICMESH")) ) // ACTOR SELECT MATCHINGSTATICMESH
		{
			const UBOOL bAllClasses = ParseCommand( &Str, TEXT("ALLCLASSES") );
			const FScopedTransaction Transaction( *LocalizeUnrealEd("SelectMatchingStaticMesh") );
			edactSelectMatchingStaticMesh( bAllClasses );
			return TRUE;
		}
		else if( ParseCommand(&Str,TEXT("MATCHINGSKELETALMESH")) ) // ACTOR SELECT MATCHINGSKELETALMESH
		{
			const UBOOL bAllClasses = ParseCommand( &Str, TEXT("ALLCLASSES") );
			const FScopedTransaction Transaction( *LocalizeUnrealEd("SelectMatchingSkeletalMesh") );
			edactSelectMatchingSkeletalMesh( bAllClasses );
			return TRUE;
		}
		else if( ParseCommand(&Str,TEXT("KISMETREF")) ) // ACTOR SELECT KISMETREF
		{
			const UBOOL bReferenced = ParseCommand( &Str, TEXT("1") );
			const FScopedTransaction Transaction( *LocalizeUnrealEd("SelectKismetReferencedActors") );
			edactSelectKismetReferencedActors( bReferenced );
			return TRUE;
		}
		else
		{
			// Get actor name.
			FName ActorName(NAME_None);
			if ( Parse( Str, TEXT("NAME="), ActorName ) )
			{
				AActor* Actor = FindObject<AActor>( GWorld->CurrentLevel, *ActorName.ToString() );
				const FScopedTransaction Transaction( *LocalizeUnrealEd("SelectToggleSingleActor") );
				SelectActor( Actor, !(Actor && Actor->IsSelected()), FALSE, TRUE );
			}
			return TRUE;
		}
	}
	else if( ParseCommand(&Str,TEXT("DELETE")) )		// ACTOR SELECT DELETE
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd("DeleteActors") );
		edactDeleteSelected();
		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("UPDATE")) )		// ACTOR SELECT UPDATE
	{
		for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );

			Actor->PreEditChange(NULL);
			Actor->PostEditChange(NULL);
		}
		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("SET")) )
	{
		// @todo DB: deprecate the ACTOR SET exec.
		RedrawLevelEditingViewports();
		GCallbackEvent->Send( CALLBACK_RefreshEditor_LevelBrowser );
		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("RESET")) )
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd("ResetActors") );

		UBOOL Location=0;
		UBOOL Pivot=0;
		UBOOL Rotation=0;
		UBOOL Scale=0;
		if( ParseCommand(&Str,TEXT("LOCATION")) )
		{
			Location=1;
			ResetPivot();
		}
		else if( ParseCommand(&Str, TEXT("PIVOT")) )
		{
			Pivot=1;
			ResetPivot();
		}
		else if( ParseCommand(&Str,TEXT("ROTATION")) )
		{
			Rotation=1;
		}
		else if( ParseCommand(&Str,TEXT("SCALE")) )
		{
			Scale=1;
		}
		else if( ParseCommand(&Str,TEXT("ALL")) )
		{
			Location=Rotation=Scale=1;
			ResetPivot();
		}

		// Fires CALLBACK_LevelDirtied when falling out of scope.
		FScopedLevelDirtied		LevelDirtyCallback;

		for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );

			Actor->PreEditChange(NULL);
			Actor->Modify();

			if( Location ) 
			{
				Actor->Location  = FVector(0.f,0.f,0.f);
			}
			if( Location ) 
			{
				Actor->PrePivot  = FVector(0.f,0.f,0.f);
			}
			if( Pivot && Actor->IsABrush() )
			{
				ABrush* Brush = (ABrush*)(Actor);
				Brush->Location -= Brush->PrePivot;
				Brush->PrePivot = FVector(0.f,0.f,0.f);
				Brush->PostEditChange(NULL);
			}
			if( Scale ) 
			{
				Actor->DrawScale = 1.0f;
			}

			Actor->MarkPackageDirty();
			LevelDirtyCallback.Request();
		}

		RedrawLevelEditingViewports();
		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("DUPLICATE")) )
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd("DuplicateActors") );

		// if not specially handled by the current editing mode,
		if (!GEditorModeTools().GetCurrentMode()->HandleDuplicate())
		{
			// duplicate selected
			edactDuplicateSelected(TRUE);
		}
		RedrawLevelEditingViewports();
		return TRUE;
	}
	else if( ParseCommand(&Str, TEXT("ALIGN")) )
	{
		if( ParseCommand(&Str,TEXT("SNAPTOFLOOR")) )
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("SnapActorsToFloor") );

			UBOOL bAlign=0;
			ParseUBOOL( Str, TEXT("ALIGN="), bAlign );

			// Fires CALLBACK_LevelDirtied when falling out of scope.
			FScopedLevelDirtied		LevelDirtyCallback;

			for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
			{
				AActor* Actor = static_cast<AActor*>( *It );
				checkSlow( Actor->IsA(AActor::StaticClass()) );

				Actor->Modify();
				MoveActorToFloor(Actor,bAlign);
				Actor->InvalidateLightingCache();
				Actor->ForceUpdateComponents();

				Actor->MarkPackageDirty();
				LevelDirtyCallback.Request();
			}

			AActor* Actor = GetSelectedActors()->GetTop<AActor>();
			if( Actor )
			{
				SetPivot( Actor->Location, FALSE, FALSE, TRUE );
			}

			RedrawLevelEditingViewports();
			return TRUE;
		}
		else if( ParseCommand(&Str,TEXT("MOVETOGRID")) )
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("MoveActorToGrid") );

			// Update the pivot location.
			const FVector OldPivot = GetPivotLocation();
			const FVector NewPivot = OldPivot.GridSnap(Constraints.GetGridSize());
			const FVector Delta = NewPivot - OldPivot;

			SetPivot( NewPivot, FALSE, FALSE, TRUE );

			// Fires CALLBACK_LevelDirtied when falling out of scope.
			FScopedLevelDirtied		LevelDirtyCallback;

			for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
			{
				AActor* Actor = static_cast<AActor*>( *It );
				checkSlow( Actor->IsA(AActor::StaticClass()) );

				Actor->Modify();

				const FVector OldLocation = Actor->Location;

				GWorld->FarMoveActor( Actor, OldLocation+Delta, FALSE, FALSE, TRUE );
				Actor->InvalidateLightingCache();
				Actor->ForceUpdateComponents();

				Actor->MarkPackageDirty();
				LevelDirtyCallback.Request();
			}

			RedrawLevelEditingViewports();
			return TRUE;
		}
		else
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd("AlignBrushVertices") );
			edactAlignVertices();
			RedrawLevelEditingViewports();
			return TRUE;
		}
	}
	else if( ParseCommand(&Str,TEXT("TOGGLE")) )
	{
		if( ParseCommand(&Str,TEXT("LOCKMOVEMENT")) )			// ACTOR TOGGLE LOCKMOVEMENT
		{
			// Fires CALLBACK_LevelDirtied when falling out of scope.
			FScopedLevelDirtied		LevelDirtyCallback;

			for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
			{
				AActor* Actor = static_cast<AActor*>( *It );
				checkSlow( Actor->IsA(AActor::StaticClass()) );

				Actor->Modify();
				Actor->bLockLocation = !Actor->bLockLocation;

				Actor->MarkPackageDirty();
				LevelDirtyCallback.Request();
			}
		}

		RedrawLevelEditingViewports();
		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("LEVELCURRENT")) )
	{
		MakeSelectedActorsLevelCurrent();
		return TRUE;
	}
	else if( ParseCommand(&Str,TEXT("MOVETOCURRENT")) )
	{
		MoveSelectedActorsToCurrentLevel();
		return TRUE;
	}
	else if(ParseCommand(&Str, TEXT("FIND"))) //ACTOR FIND KISMET
	{
		if(ParseCommand(&Str, TEXT("KISMET")))
		{
			AActor *FirstActor = GEditor->GetSelectedActors()->GetTop<AActor>();
			
			if(FirstActor)
			{
				// Get the kismet sequence for the level the actor belongs to.
				USequence* RootSeq = GWorld->GetGameSequence(FirstActor->GetLevel());
				if(RootSeq && RootSeq->ReferencesObject(FirstActor))
				{
					WxKismet::FindActorReferences(FirstActor);
				}
			}

			return TRUE;
		}
	}
	else if(ParseCommand(&Str, TEXT("SYNCBROWSER")))
	{
		GApp->EditorFrame->SyncToGenericBrowser();
		return TRUE;
	}
	else if(ParseCommand(&Str, TEXT("DESELECT")))
	{
		//deselects everything in UnrealEd
		GUnrealEd->SelectNone(TRUE, TRUE);
		
		//Destroys any visible property windows associated with actors
		for(int Index = 0; Index < GUnrealEd->ActorProperties.Num(); ++Index)
		{
			GUnrealEd->ActorProperties(Index)->Destroy();
		}

		GUnrealEd->ActorProperties.Empty();
		return TRUE;
	}

	return FALSE;
}


UBOOL UUnrealEdEngine::Exec_Mode( const TCHAR* Str, FOutputDevice& Ar )
{
	Word1 = GEditorModeTools().GetCurrentModeID();  // To see if we should redraw
	Word2 = GEditorModeTools().GetCurrentModeID();  // Destination mode to set

	UBOOL DWord1;
	if( ParseCommand(&Str,TEXT("WIDGETCOORDSYSTEMCYCLE")) )
	{
		INT Wk = GEditorModeTools().CoordSystem;
		Wk++;

		if( Wk == COORD_Max )
		{
			Wk -= COORD_Max;
		}
		GEditorModeTools().CoordSystem = (ECoordSystem)Wk;
		GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
		GCallbackEvent->Send( CALLBACK_UpdateUI );
	}
	if( ParseCommand(&Str,TEXT("WIDGETMODECYCLE")) )
	{
		if( GEditorModeTools().GetShowWidget() )
		{
			const INT CurrentWk = GEditorModeTools().GetWidgetMode();
			INT Wk = CurrentWk;
			// don't allow cycling through non uniform scaling
			const INT MaxWidget = (INT)FWidget::WM_ScaleNonUniform;
			do
			{
				Wk++;
				// Roll back to the start if we go past FWidget::WM_Scale
				if( Wk >= MaxWidget )
				{
					Wk -= MaxWidget;
				}
			}
			while (!GEditorModeTools().GetCurrentMode()->UsesWidget((FWidget::EWidgetMode)Wk) && Wk != CurrentWk);
			GEditorModeTools().SetWidgetMode( (FWidget::EWidgetMode)Wk );
			GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
		}
	}
	if( ParseUBOOL(Str,TEXT("GRID="), DWord1) )
	{
		FinishAllSnaps();
		Constraints.GridEnabled = DWord1;
		Word1=MAXWORD;
		GCallbackEvent->Send( CALLBACK_UpdateUI );
	}
	if( ParseUBOOL(Str,TEXT("ROTGRID="), DWord1) )
	{
		FinishAllSnaps();
		Constraints.RotGridEnabled=DWord1;
		Word1=MAXWORD;
		GCallbackEvent->Send( CALLBACK_UpdateUI );
	}
	if( ParseUBOOL(Str,TEXT("SNAPVERTEX="), DWord1) )
	{
		FinishAllSnaps();
		Constraints.SnapVertices=DWord1;
		Word1=MAXWORD;
		GCallbackEvent->Send( CALLBACK_UpdateUI );
	}
	if( ParseUBOOL(Str,TEXT("ALWAYSSHOWTERRAIN="), DWord1) )
	{
		FinishAllSnaps();
		AlwaysShowTerrain=DWord1;
		Word1=MAXWORD;
	}
	if( ParseUBOOL(Str,TEXT("USEACTORROTATIONGIZMO="), DWord1) )
	{
		FinishAllSnaps();
		UseActorRotationGizmo=DWord1;
		Word1=MAXWORD;
	}
	if( ParseUBOOL(Str,TEXT("SHOWBRUSHMARKERPOLYS="), DWord1) )
	{
		FinishAllSnaps();
		bShowBrushMarkerPolys=DWord1;
		Word1=MAXWORD;
	}
	if( ParseUBOOL(Str,TEXT("SELECTIONLOCK="), DWord1) )
	{
		FinishAllSnaps();
		// If -1 is passed in, treat it as a toggle.  Otherwise, use the value as a literal assignment.
		if( DWord1 == -1 )
			GEdSelectionLock=(GEdSelectionLock == 0) ? 1 : 0;
		else
			GEdSelectionLock=DWord1;
		Word1=MAXWORD;
	}
	Parse(Str,TEXT("MAPEXT="), GApp->MapExt);
	if( ParseUBOOL(Str,TEXT("USESIZINGBOX="), DWord1) )
	{
		FinishAllSnaps();
		// If -1 is passed in, treat it as a toggle.  Otherwise, use the value as a literal assignment.
		if( DWord1 == -1 )
			UseSizingBox=(UseSizingBox == 0) ? 1 : 0;
		else
			UseSizingBox=DWord1;
		Word1=MAXWORD;
	}
	
	if(GCurrentLevelEditingViewportClient)
	{
		Parse( Str, TEXT("SPEED="), GCurrentLevelEditingViewportClient->CameraSpeed );
	}

	Parse( Str, TEXT("SNAPDIST="), Constraints.SnapDistance );
	//
	// Major modes:
	//
	if 		(ParseCommand(&Str,TEXT("CAMERAMOVE")))		Word2 = EM_Default;
	else if (ParseCommand(&Str,TEXT("TERRAINEDIT")))	Word2 = EM_TerrainEdit;
	else if	(ParseCommand(&Str,TEXT("GEOMETRY"))) 		Word2 = EM_Geometry;
	else if	(ParseCommand(&Str,TEXT("TEXTURE"))) 		Word2 = EM_Texture;
	else if (ParseCommand(&Str,TEXT("COVEREDIT")))		Word2 = EM_CoverEdit;

	if( Word2 != Word1 )
		GCallbackEvent->Send( CALLBACK_ChangeEditorMode, Word2 );

	// Reset the roll on all viewport cameras
	for(UINT ViewportIndex = 0;ViewportIndex < (UINT)ViewportClients.Num();ViewportIndex++)
	{
		if(ViewportClients(ViewportIndex)->ViewportType == LVT_Perspective)
			ViewportClients(ViewportIndex)->ViewRotation.Roll = 0;
	}

	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );

	return TRUE;
}
