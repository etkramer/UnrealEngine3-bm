/*
UnErrorChecking.cpp
Actor Error checking functions
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EnginePhysicsClasses.h"
#include "EngineSequenceClasses.h"

/**
 * Special archive for finding references from a map to objects contained within an editor-only package.
 */
class FEditorContentReferencersArchive : public FArchive
{
public:
	FEditorContentReferencersArchive( const TArray<UPackage*>& inEditorContentPackages )
	: EditorContentPackages(inEditorContentPackages)
	{
		ArIsObjectReferenceCollector = TRUE;
		ArIsPersistent = TRUE;
		ArIgnoreClassRef = TRUE;
	}

	/** 
	 * UObject serialize operator implementation
	 *
	 * @param Object	reference to Object reference
	 * @return reference to instance of this class
	 */
	FArchive& operator<<( UObject*& Object )
	{
		// Avoid duplicate entries.
		if ( Object != NULL && !SerializedObjects.HasKey(Object) )
		{
			SerializedObjects.AddItem(Object);
			if ( !Object->IsA(UField::StaticClass()) 
			&&	(Object->NeedsLoadForClient() || Object->NeedsLoadForServer()) )
			{
				if (EditorContentPackages.ContainsItem(Object->GetOutermost())
				&&	Object->GetOutermost() != Object )
				{
					ReferencedEditorOnlyObjects.AddItem(Object);
				}

				Object->Serialize(*this);
			}
		}
		
		return *this;
	}

	/** the list of objects within the editor-only packages that are referenced by this map */
	TLookupMap<UObject*> ReferencedEditorOnlyObjects;

protected:

	/** the list of packages that will not be loaded in-game */
	TArray<UPackage*> EditorContentPackages;

	/** prevents objects from being serialized more than once */
	TLookupMap<UObject*> SerializedObjects;
};

void AWorldInfo::CheckForErrors()
{
	Super::CheckForErrors();

	if ( GWorld->GetWorldInfo() != this )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("Duplicate level info!") ), MCACTION_NONE, TEXT("DuplicateLevelInfo") );
	}
	else
	{
		if ( !bPathsRebuilt )
		{
			// Don't specify a pointer to this actor when simply warning about paths needing a rebuild.
			GWarn->MapCheck_Add( MCTYPE_ERROR, NULL, *FString::Printf(TEXT("Paths need to be rebuilt!") ), MCACTION_NONE, TEXT("RebuildPaths") );
		}
	}

	if( bMapNeedsLightingFullyRebuilt == TRUE )
	{
		GWarn->MapCheck_Add( MCTYPE_ERROR, this, TEXT("Maps need lighting rebuilt"), MCACTION_NONE, TEXT("RebuildLighting") );
	}

	// pass to MapInfo
	if (MyMapInfo != NULL)
	{
		MyMapInfo->CheckForErrors();
	}

	// Warn about KillZ not being overridden for this map.
	if( KillZ == GetDefault<AWorldInfo>()->KillZ )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, TEXT("Map should have KillZ set."), MCACTION_NONE );
	}

	for ( INT LevelIndex = 0; LevelIndex < GWorld->Levels.Num(); LevelIndex++ )
	{
		USequence* LevelSequence = GWorld->Levels(LevelIndex)->GetGameSequence();
		if ( LevelSequence != NULL )
		{
			LevelSequence->CheckForErrors();
		}
	}
}


void AActor::CheckForDeprecated()
{
	if ( GetClass()->ClassFlags & CLASS_Deprecated )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s is obsolete and must be removed! (Class is deprecated)"), *GetName() ), MCACTION_DELETE, TEXT("ActorIsObselete") );
	}

	if ( GetClass()->ClassFlags & CLASS_Abstract )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s is obsolete and must be removed! (Class is abstract)"), *GetName() ), MCACTION_DELETE, TEXT("ActorIsObselete"));
	}
}

void AActor::CheckForErrors()
{
	if ( GetClass()->ClassFlags & CLASS_Deprecated )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s is obsolete and must be removed! (Class is deprecated)"), *GetName() ), MCACTION_DELETE, TEXT("ActorIsObselete") );
		return;
	}
	if ( GetClass()->ClassFlags & CLASS_Abstract )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s is obsolete and must be removed! (Class is abstract)"), *GetName() ), MCACTION_DELETE, TEXT("ActorIsObselete") );
		return;
	}
	if ( GetClass()->GetDefaultActor()->bStatic && !bStatic )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s bStatic false, but is bStatic by default - map will fail in netplay"), *GetName() ), MCACTION_NONE, TEXT("StaticFalse") );
	}
	if ( GetClass()->GetDefaultActor()->bNoDelete && !bNoDelete )
	{	
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s bNoDelete false, but is bNoDelete by default - map will fail in netplay"), *GetName() ), MCACTION_NONE, TEXT("NoDeleteFalse") );
	}
	if( bStatic && Physics != PHYS_None)
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s bStatic true, but has Physics set to something other than PHYS_None!"), *GetName() ), MCACTION_NONE, TEXT("StaticPhysNone") );
	}
	if( (DrawScale * DrawScale3D.X * DrawScale3D.Y * DrawScale3D.Z) == 0 )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s has invalid DrawScale/ DrawScale3D"), *GetName() ), MCACTION_NONE, TEXT("InvalidDrawscale") );
	}

	// Route error checking to components.
	for ( INT ComponentIndex = 0 ; ComponentIndex < Components.Num() ; ++ComponentIndex )
	{
		UActorComponent* ActorComponent = Components( ComponentIndex );
		if (ActorComponent != NULL)
		{
			ActorComponent->CheckForErrors();
		}
	}

	if( Physics == PHYS_RigidBody && !CollisionComponent && !IsA(ARB_ConstraintActor::StaticClass()) )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s is set to PHYS_RigidBody but has no CollisionComponent"), *GetName() ), MCACTION_NONE, TEXT("PhysRigidBodyNoCollisionComp") );
	}
}

void APhysicsVolume::CheckForErrors()
{
	Super::CheckForErrors();

	// @FIXME - temporary check for old placeable water volumes
	if ( bWaterVolume && ((GetClass()->ClassFlags & CLASS_Placeable) == 0) )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s is not placeable and should be removed!!!"), *GetName() ), MCACTION_DELETE, TEXT("NotPlaceable") );
	}
	if ( bPainCausing && !DamageType )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s causes damage, but has no damagetype defined."), *GetName() ), MCACTION_NONE, TEXT("NoDamageType") );
	}
}

void ANote::CheckForErrors()
{
	Super::CheckForErrors();

	if( Text.Len() > 0 )
	{
		GWarn->MapCheck_Add( MCTYPE_NOTE, this, *Text );
	}
}

void ABrush::CheckForErrors()
{
	Super::CheckForErrors();

	if( !BrushComponent )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s : Brush has NULL BrushComponent property - please delete!"), *GetName() ), MCACTION_DELETE, TEXT("BrushComponentNull") );
	}
	else
	{
		// NOTE : don't report NULL texture references on the builder brueh - it doesn't matter there
		// NOTE : don't check volume brushes since those are forced to have the NULL texture on all polys
		if( Brush && !this->IsABuilderBrush() && !IsVolumeBrush() )
		{
			// Check for missing textures
			for( INT x = 0 ; x < Brush->Polys->Element.Num() ; ++x )
			{
				FPoly* Poly = &(Brush->Polys->Element(x));

				if( !Poly->Material )
				{
					GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s : Brush has NULL material reference(s)"), *GetName() ), MCACTION_NONE, TEXT("BrushNullMaterial") );
					break;
				}
			}
		
			// A brush without any polygons in it isn't useful.  Should be deleted.
			if( Brush->Polys->Element.Num() == 0 )
			{
				GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s : Brush has zero polygons - please delete!"), *GetName() ), MCACTION_DELETE, TEXT("BrushZeroPolygons") );
			}

			// Check for non-coplanar polygons.
			for( INT x = 0 ; x < Brush->Polys->Element.Num() ; ++x )
			{
				FPoly* Poly = &(Brush->Polys->Element(x));
				UBOOL	Coplanar = 1;

				for(INT VertexIndex = 0;VertexIndex < Poly->Vertices.Num();VertexIndex++)
				{
					if(Abs(FPlane(Poly->Vertices(0),Poly->Normal).PlaneDot(Poly->Vertices(VertexIndex))) > THRESH_POINT_ON_PLANE)
					{
						GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s : Brush has non-coplanar polygons"), *GetName() ), MCACTION_NONE, TEXT("NonCoPlanarPolys") );
						Coplanar = 0;
						break;
					}
				}

				if(!Coplanar)
				{
					break;
				}
			}
		}
	}
}

/**
 * Function that gets called from within Map_Check to allow this actor to check itself
 * for any potential errors and register them with map check dialog.
 */
void AStaticMeshActor::CheckForErrors()
{
	Super::CheckForErrors();

	if( StaticMeshComponent == NULL )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s : Static mesh actor has NULL StaticMeshComponent property - please delete!"), *GetName() ), MCACTION_DELETE, TEXT("StaticMeshComponent") );
	}
	else if( StaticMeshComponent->StaticMesh == NULL )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s : Static mesh actor has NULL StaticMesh property"), *GetName() ), MCACTION_DELETE, TEXT("StaticMeshNull") );
	}
	else
	{
		FMemMark Mark( GMainThreadMemStack );
		FCheckResult* ActorResult =
			GWorld->Hash->ActorRadiusCheck(
				GMainThreadMemStack,	// Memory stack
				Location,				// Location to test
				1.0f,					// Radius
				TRACE_Actors );			// Flags

		for( FCheckResult* HitResult = ActorResult ; HitResult ; HitResult = HitResult->GetNext() )
		{
			AActor* CurTouchingActor = HitResult->Actor;
			if( CurTouchingActor != NULL )
			{
				AStaticMeshActor *A = Cast<AStaticMeshActor>( CurTouchingActor );
				if ( A && (A != this) && (A->Location - Location).IsNearlyZero() && A->StaticMeshComponent 
					&& (A->StaticMeshComponent->StaticMesh == StaticMeshComponent->StaticMesh) && (A->Rotation == Rotation) 
					&&  (A->DrawScale3D == DrawScale3D) )
				{
					GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s in the same location as %s"), *GetName(), *A->GetName() ) );
				}
			}
		}

		Mark.Pop();
	}
}

/**
 * Function that gets called from within Map_Check to allow this actor to check itself
 * for any potential errors and register them with map check dialog.
 */
void ADynamicSMActor::CheckForErrors()
{
	Super::CheckForErrors();
	if( StaticMeshComponent == NULL )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s : DynamicSMActor has NULL StaticMeshComponent property - please delete!"), *GetName() ), MCACTION_DELETE, TEXT("DynamicStaticMeshComponent"));
	}
	else if( StaticMeshComponent->StaticMesh == NULL )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s : DynamicSMActor has a StaticMeshComponent with NULL StaticMesh property"), *GetName() ), MCACTION_DELETE, TEXT("DynamicStaticMeshNull") );
	}
}


void APawn::CheckForErrors()
{
	Super::CheckForErrors();
}

void ANavigationPoint::CheckForErrors()
{
	Super::CheckForErrors();

	// NOTE: Map_Check will test for navigation point GUID collisions itself, so we don't need to worry
	//   about that here.

	if( !GWorld->GetWorldInfo()->bHasPathNodes )
	{
		return;
	}

	if (CylinderComponent == NULL)
	{
		GWarn->MapCheck_Add(MCTYPE_ERROR, this, TEXT("NavigationPoint has NULL CylinderComponent - please delete!"), MCACTION_DELETE, TEXT("CylinderComponentNull"));
	}
	if ( !bDestinationOnly && !GWorld->GetWorldInfo()->bNoPathWarnings )
	{
		if( PathList.Num() == 0 )
		{
			GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("No paths from %s"), *GetName() ), MCACTION_NONE, TEXT("NoPathsFromPoint") );
		}
		else 
		if( PathList.Num() == 1 )
		{
			UReachSpec* Spec = PathList(0);
			if( Spec && Spec->bSkipPrune )
			{
				ANavigationPoint* End = (ANavigationPoint*)~Spec->End;
				if( End && End->PathList.Num() == 1 )
				{
					UReachSpec* Back = End->PathList(0);
					if( Back && Back->bSkipPrune )
					{
						ANavigationPoint* Start = (ANavigationPoint*)~Back->End;
						if( Start && Start == this )
						{
							GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("Only special action path to world: %s"), *End->GetName()), MCACTION_NONE, TEXT("OnlySpecialPathsFromPoint")) ;
						}
					}
				}
			}
		}
	}
	if ( ExtraCost < 0 )
	{
		GWarn->MapCheck_Add( MCTYPE_ERROR, this, TEXT("Extra Cost cannot be less than zero!"), MCACTION_NONE, TEXT("ExtraCostZero") );
	}
	if ( Location.Z > GWorld->GetWorldInfo()->StallZ )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, TEXT("NavigationPoint above level's StallZ!"), MCACTION_NONE, TEXT("NavPointAboveStallZ") );
	}
}

void ALight::CheckForErrors()
{
	Super::CheckForErrors();
	if( LightComponent == NULL )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s : Light actor has NULL LightComponent property - please delete!"), *GetName() ), MCACTION_DELETE, TEXT("LightComponentNull") );
	}
}

void APortalTeleporter::CheckForErrors()
{
	Super::CheckForErrors();
	
	if (Cast<USceneCapturePortalComponent>(SceneCapture) == NULL)
	{
		GWarn->MapCheck_Add(MCTYPE_WARNING, this, TEXT("No or invalid scene capture component - please delete"), MCACTION_DELETE, TEXT("InvalidSceneCaptureComponent"));
	}

	if (SisterPortal == NULL)
	{
		GWarn->MapCheck_Add(MCTYPE_WARNING, this, TEXT("No SisterPortal"));
	}
}

void ALevelStreamingVolume::CheckForErrors()
{
	Super::CheckForErrors();

	// Streaming level volumes are not permitted outside the persistent level.
	if ( GetLevel() != GWorld->PersistentLevel )
	{
		GWarn->MapCheck_Add(MCTYPE_WARNING, this, TEXT("LevelStreamingVolume is not in the persistent level - please delete"), MCACTION_DELETE, TEXT("LevelStreamingVolume"));
	}
}

/** Utility function that indicates whether this DynamicBlockingVolume is currently blocking the player. */
static UBOOL BlocksPlayer(ADynamicBlockingVolume* BV)
{
	if(BV->bCollideActors && BV->bBlockActors && BV->CollisionComponent && BV->CollisionComponent->CollideActors && BV->CollisionComponent->BlockActors && BV->CollisionComponent->BlockNonZeroExtent)
	{
		return TRUE;
	}

	return FALSE;
}

void ADynamicBlockingVolume::CheckForErrors()
{
	Super::CheckForErrors();

	// See if this volume is not blocking the player, but is blocking its physics (eg ragdoll).
	if(!BlocksPlayer(this) && CollisionComponent && CollisionComponent->BlockRigidBody)
	{
		GWarn->MapCheck_Add(MCTYPE_WARNING, this, TEXT("DynamicBlockingVolume does not block players but does block ragdolls (BlockRigidBody is TRUE)."), MCACTION_NONE, TEXT("DynamicBVNotPlayerButRB"));
	}
}

