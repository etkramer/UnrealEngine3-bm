/*=============================================================================
	UnLevel.cpp: Level-related functions
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"
#include "EngineSequenceClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineMaterialClasses.h"
#include "EnginePhysicsClasses.h"
#include "UnOctree.h"
#include "UnTerrain.h"
#include "ScenePrivate.h"

IMPLEMENT_CLASS(ULineBatchComponent);

/*-----------------------------------------------------------------------------
	ULevelBase implementation.
-----------------------------------------------------------------------------*/

ULevelBase::ULevelBase( const FURL& InURL )
: Actors( this )
, URL( InURL )

{}

/**
 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
 * properties for native- only classes.
 */
void ULevelBase::StaticConstructor()
{
	UClass* TheClass = GetClass();
	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( ULevelBase, Actors ) );
}

void ULevelBase::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);
	Ar << Actors;
	Ar << URL;
}
IMPLEMENT_CLASS(ULevelBase);


/*-----------------------------------------------------------------------------
	ULevel implementation.
-----------------------------------------------------------------------------*/

//@deprecated with VER_SPLIT_SOUND_FROM_TEXTURE_STREAMING
struct FStreamableResourceInstanceDeprecated
{
	FSphere BoundingSphere;
	FLOAT TexelFactor;
	friend FArchive& operator<<( FArchive& Ar, FStreamableResourceInstanceDeprecated& ResourceInstance )
	{
		Ar << ResourceInstance.BoundingSphere;
		Ar << ResourceInstance.TexelFactor;
		return Ar;
	}
};
//@deprecated with VER_SPLIT_SOUND_FROM_TEXTURE_STREAMING
struct FStreamableResourceInfoDeprecated
{
	UObject* Resource;
	TArray<FStreamableResourceInstanceDeprecated> ResourceInstances;
	friend FArchive& operator<<( FArchive& Ar, FStreamableResourceInfoDeprecated& ResourceInfo )
	{
		Ar << ResourceInfo.Resource;
		Ar << ResourceInfo.ResourceInstances;
		return Ar;
	}
};
//@deprecated with VER_RENDERING_REFACTOR
struct FStreamableSoundInstanceDeprecated
{
	FSphere BoundingSphere;
	friend FArchive& operator<<( FArchive& Ar, FStreamableSoundInstanceDeprecated& SoundInstance )
	{
		Ar << SoundInstance.BoundingSphere;
		return Ar;
	}
};
//@deprecated with VER_RENDERING_REFACTOR
struct FStreamableSoundInfoDeprecated
{
	USoundNodeWave*	SoundNodeWave;
	TArray<FStreamableSoundInstanceDeprecated> SoundInstances;
	friend FArchive& operator<<( FArchive& Ar, FStreamableSoundInfoDeprecated& SoundInfo )
	{
		Ar << SoundInfo.SoundNodeWave;
		Ar << SoundInfo.SoundInstances;
		return Ar;
	}
};
//@deprecated with VER_RENDERING_REFACTOR
struct FStreamableTextureInfoDeprecated
{
	UTexture*							Texture;
	TArray<FStreamableTextureInstance>	TextureInstances;
	friend FArchive& operator<<( FArchive& Ar, FStreamableTextureInfoDeprecated& TextureInfo )
	{
		Ar << TextureInfo.Texture;
		Ar << TextureInfo.TextureInstances;
		return Ar;
	}
};


IMPLEMENT_CLASS(ULevel);

ULevel::ULevel( const FURL& InURL )
:	ULevelBase( InURL )
{
}

/**
 * Static constructor called once per class during static initialization via IMPLEMENT_CLASS
 * macro. Used to e.g. emit object reference tokens for realtime garbage collection or expose
 * properties for native- only classes.
 */
void ULevel::StaticConstructor()
{
	UClass* TheClass = GetClass();
	TheClass->EmitObjectReference( STRUCT_OFFSET( ULevel, Model ) );
	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( ULevel, ModelComponents ) );
	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( ULevel, GameSequences ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( ULevel, NavListStart ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( ULevel, NavListEnd ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( ULevel, CoverListStart ) );
	TheClass->EmitObjectReference( STRUCT_OFFSET( ULevel, CoverListEnd ) );
	TheClass->EmitObjectArrayReference( STRUCT_OFFSET( ULevel, CrossLevelActors ) );
}

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void ULevel::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );
	for( TMap<UTexture2D*,TArray<FStreamableTextureInstance> >::TIterator It(TextureToInstancesMap); It; ++It )
	{
		UTexture2D* Texture2D = It.Key();
		AddReferencedObject( ObjectArray, Texture2D );
	}
}

void ULevel::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	Ar << Model;

	Ar << ModelComponents;

	Ar << GameSequences;

	if( !Ar.IsTransacting() )
	{
		Ar << TextureToInstancesMap;

		CachedPhysBSPData.BulkSerialize(Ar);
    
	    Ar << CachedPhysSMDataMap;
	    Ar << CachedPhysSMDataStore;
	    Ar << CachedPhysPerTriSMDataMap;
	    Ar << CachedPhysPerTriSMDataStore;
        Ar << CachedPhysBSPDataVersion;
	    Ar << CachedPhysSMDataVersion;
		Ar << ForceStreamTextures;
	}

	// Mark archive and package as containing a map if we're serializing to disk.
	if( !HasAnyFlags( RF_ClassDefaultObject ) && Ar.IsPersistent() )
	{
		Ar.ThisContainsMap();
		GetOutermost()->ThisContainsMap();
	}

	// serialize the nav list
	Ar << NavListStart;
	Ar << NavListEnd;
	// and cover
	Ar << CoverListStart;
	Ar << CoverListEnd;
	// serialize the list of cross level actors
	Ar << CrossLevelActors;
}


/**
 * Sorts the actor list by net relevancy and static behaviour. First all net relevant static
 * actors, then all remaining static actors and then the rest. This is done to allow the dynamic
 * and net relevant actor iterators to skip large amounts of actors.
 */
void ULevel::SortActorList()
{		
	INT				StartIndex	= 0;
	TArray<AActor*> NewActors;

	// The world info and default brush have fixed actor indices.
	NewActors.AddItem(Actors(StartIndex++));
	NewActors.AddItem(Actors(StartIndex++));

	// Static not net relevant actors.
	for( INT ActorIndex=StartIndex; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if( Actor && !Actor->bDeleteMe && Actor->bStatic && !Actor->bAlwaysRelevant )
		{
			NewActors.AddItem( Actor );
		}
	}
	iFirstNetRelevantActor=NewActors.Num();

	// Static net relevant actors.
	for( INT ActorIndex=StartIndex; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);		
		if( Actor && !Actor->bDeleteMe && Actor->bStatic && Actor->bAlwaysRelevant )
		{
			NewActors.AddItem( Actor );
		}
	}
	iFirstDynamicActor=NewActors.Num();

	// Remaining (dynamic, potentially net relevant actors)
	for( INT ActorIndex=StartIndex; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);			
		if( Actor && !Actor->bDeleteMe && !Actor->bStatic )
		{
			NewActors.AddItem( Actor );
		}
	}

	// Replace with sorted list.
	Actors = NewActors;

	// Don't use sorted optimization outside of gameplay so we can safely shuffle around actors e.g. in the Editor
	// without there being a chance to break code using dynamic/ net relevant actor iterators.
	if( !GIsGame )
	{
		iFirstNetRelevantActor	= 0;
		iFirstDynamicActor		= 0;
	}
}

/**
 * Makes sure that all light components have valid GUIDs associated.
 */
void ULevel::ValidateLightGUIDs()
{
	for( TObjectIterator<ULightComponent> It; It; ++It )
	{
		ULightComponent*	LightComponent	= *It;
		UBOOL				IsInLevel		= LightComponent->IsIn( this );

		if( IsInLevel )
		{
			LightComponent->ValidateLightGUIDs();
		}
	}
}

/** 
 * Associate teleporters with the portal volume they are in
 */
void ULevel::AssociatePortals( void )
{
	check( GWorld );

	for( TObjectIterator<APortalTeleporter> It; It; ++It )
	{
		APortalTeleporter* Teleporter = static_cast<APortalTeleporter*>( *It );
		APortalVolume* Volume = GWorld->GetWorldInfo()->GetPortalVolume( Teleporter->Location );

		if( Volume )
		{
			Volume->Portals.AddUniqueItem( Teleporter );
		}
	}
}

/**
 * Presave function, gets called once before the level gets serialized (multiple times) for saving.
 * Used to rebuild streaming data on save.
 */
void ULevel::PreSave()
{
	Super::PreSave();

	if( !IsTemplate() )
	{
		UPackage* Package = CastChecked<UPackage>(GetOutermost());

		ValidateLightGUIDs();

		// Don't rebuild streaming data if we're saving out a cooked package as the raw data required has already been stripped.
		if( !(Package->PackageFlags & PKG_Cooked) )
		{
			BuildStreamingData();
		}

		// Build bsp-trimesh data for physics engine
		BuildPhysBSPData();

		// clean up the nav list
		GWorld->RemoveLevelNavList(this);
        // if one of the pointers are NULL then eliminate both to prevent possible crashes during gameplay before paths are rebuilt
        if (NavListStart == NULL || NavListEnd == NULL)
        {
            if (HasPathNodes())
            {
                //@todo - add a message box (but make sure it doesn't pop up for autosaves?)
                debugf(NAME_Warning,TEXT("PATHING NEEDS TO BE REBUILT FOR %s"),*GetPathName());
            }
            NavListStart = NULL;
            NavListEnd = NULL;
        }

		// Associate portal teleporters with portal volumes
		AssociatePortals();

		// Clear out any crosslevel references
		for( INT ActorIdx = 0; ActorIdx < Actors.Num(); ActorIdx++ )
		{
			AActor *Actor = Actors(ActorIdx);
			if( Actor != NULL )
			{
				Actor->ClearCrossLevelReferences();
			}
		}

		// Build the list of cross level actors
		CrossLevelActors.Empty();
		for( INT ActorIdx = 0; ActorIdx < Actors.Num(); ActorIdx++ )
		{
			AActor *Actor = Actors(ActorIdx);
			if( Actor != NULL && !Actor->IsPendingKill() )
			{
				TArray<FActorReference*> ActorRefs;
				Actor->GetActorReferences(ActorRefs,TRUE);
				Actor->GetActorReferences(ActorRefs,FALSE);
				if( ActorRefs.Num() > 0 )
				{
					// and null the cross level references
					UBOOL bHasCrossLevelRef = FALSE;
					for( INT Idx = 0; Idx < ActorRefs.Num(); Idx++ )
					{
						if( ActorRefs(Idx)->Actor == NULL || Cast<ULevel>(ActorRefs(Idx)->Actor->GetOuter()) != this )
						{
							bHasCrossLevelRef = TRUE;
							ActorRefs(Idx)->Actor = NULL;
						}
						else
						{
							ActorRefs(Idx)->Guid = FGuid(EC_EventParm);
						}
					}
					if( bHasCrossLevelRef )
					{
						CrossLevelActors.AddItem(Actor);
					}
				}
			}
		}
	}
}

/**
 * Removes existing line batch components from actors and associates streaming data with level.
 */
void ULevel::PostLoad()
{
	Super::PostLoad();

	//@todo: investigate removal of code cleaning up existing LineBatchComponents.
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if(Actor)
		{
			for( INT ComponentIndex=0; ComponentIndex<Actor->Components.Num(); ComponentIndex++ )
			{
				UActorComponent* Component = Actor->Components(ComponentIndex);
				if( Component && Component->IsA(ULineBatchComponent::StaticClass()) )
				{
					check(!Component->IsAttached());
					Actor->Components.Remove(ComponentIndex--);
				}
			}
		}
	}

	// reattach decals to receivers after level has been fully loaded
	GEngine->IssueDecalUpdateRequest();
}

/**
 * Clears all components of actors associated with this level (aka in Actors array) and 
 * also the BSP model components.
 */
void ULevel::ClearComponents()
{
	bAreComponentsCurrentlyAttached = FALSE;

	// Remove the model components from the scene.
	for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
	{
		if(ModelComponents(ComponentIndex))
		{
			ModelComponents(ComponentIndex)->ConditionalDetach();
		}
	}

	// Remove the actors' components from the scene.
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if( Actor )
		{
			Actor->ClearComponents();
		}
	}

	// Iterate over all textures with distance based streaming referenced by the level and reset their last render
	// time. This avoids the case where a level is removed but its textures are being streamed in as they are no 
	// longer handled by the distance based streaming code and therefore fall back to using streaming based on
	// last render time.
	for( TMap<UTexture2D*,TArray<FStreamableTextureInstance> >::TIterator It(TextureToInstancesMap); It; ++It )
	{
		UTexture2D* Texture = It.Key();
		if( Texture && Texture->Resource )
		{
			// Reset last render time. This is not thread safe but sufficient for our purposes.
			Texture->Resource->LastRenderTime = 0;
		}
	}
}

/**
 * A TMap key type used to sort BSP nodes by locality and zone.
 */
struct FModelComponentKey
{
	UINT	ZoneIndex;
	UINT	X;
	UINT	Y;
	UINT	Z;
	DWORD	MaskedPolyFlags;
	DWORD	LightingChannels;

	friend UBOOL operator==(const FModelComponentKey& A,const FModelComponentKey& B)
	{
		return	A.ZoneIndex == B.ZoneIndex 
			&&	A.X == B.X 
			&&	A.Y == B.Y 
			&&	A.Z == B.Z 
			&&	A.MaskedPolyFlags == B.MaskedPolyFlags
			&&	A.LightingChannels == B.LightingChannels;
	}

	friend DWORD GetTypeHash(const FModelComponentKey& Key)
	{
		return appMemCrc(&Key,sizeof(Key),0);
	}
};

/**
 * Updates all components of actors associated with this level (aka in Actors array) and 
 * creates the BSP model components.
 */
void ULevel::UpdateComponents()
{
	// Update all components in one swoop.
	IncrementalUpdateComponents( 0 );
}


/**
 * Incrementally updates all components of actors associated with this level.
 *
 * @param NumComponentsToUpdate	Number of components to update in this run, 0 for all
 */
void ULevel::IncrementalUpdateComponents( INT NumComponentsToUpdate )
{
	// A value of 0 means that we want to update all components.
	UBOOL bForceUpdateAllActors = FALSE;
	if( NumComponentsToUpdate == 0 )
	{
		NumComponentsToUpdate = Actors.Num();
		bForceUpdateAllActors = TRUE;
	}
	// Only the game can use incremental update functionality.
	else
	{
		checkMsg(!GIsEditor && GIsGame,TEXT("Cannot call IncrementalUpdateComponents with non 0 argument in the Editor/ commandlets."));
	}

	// Do BSP on the first pass.
	if( CurrentActorIndexForUpdateComponents == 0 )
	{
		UpdateModelComponents();
	}

	// Do as many Actor's as we were told, with the exception of static mesh collection actors. They contain a variable number of 
	// RBPhys components that can take more time than we are anticipating at a higher level. Unless we do a force full update we
	// only do up to the first SMCA and only one SMCA at a time.
	UBOOL bShouldBailOutEarly = FALSE;
	NumComponentsToUpdate = Min( NumComponentsToUpdate, Actors.Num() - CurrentActorIndexForUpdateComponents );
	for( INT i=0; i<NumComponentsToUpdate && !bShouldBailOutEarly; i++ )
	{
		AActor* Actor = Actors(CurrentActorIndexForUpdateComponents++);
		if( Actor )
		{
			// Request an early bail out if we encounter a SMCA... unless we force update all actors.
			bShouldBailOutEarly = Cast<AStaticMeshCollectionActor>(Actor) ? !bForceUpdateAllActors : FALSE; 

			// Always do at least one and keep going as long as its not a SMCA
			if( !bShouldBailOutEarly || i == 0 )  
			{
				Actor->ClearComponents();
				Actor->ConditionalUpdateComponents();
				// Shrink various components arrays for static actors to avoid waste due to array slack.
				if( Actor->bStatic )
				{
					Actor->Components.Shrink();
					Actor->AllComponents.Shrink();
				}
			}
			else
			{
				// Rollback since we didn't actually process the actor
				CurrentActorIndexForUpdateComponents--;
				break;
			}		
		}
	}

	// See whether we are done.
	if( CurrentActorIndexForUpdateComponents == Actors.Num() )
	{
		CurrentActorIndexForUpdateComponents	= 0;
		bAreComponentsCurrentlyAttached			= TRUE;
	}
	// Only the game can use incremental update functionality.
	else
	{
		check(!GIsEditor && GIsGame);
	}
}



/**
 * Updates the model components associated with this level
 */
void ULevel::UpdateModelComponents()
{
	// Create/update the level's BSP model components.
	if(!ModelComponents.Num())
	{
		// Update the model vertices and edges.
		Model->UpdateVertices();
		Model->BuildShadowData();
		Model->InvalidSurfaces = 0;

	    // Clear the model index buffers.
	    Model->MaterialIndexBuffers.Empty();

		TMap< FModelComponentKey, TArray<WORD> > ModelComponentMap;

		// Sort the nodes by zone, grid cell and masked poly flags.
		for(INT NodeIndex = 0;NodeIndex < Model->Nodes.Num();NodeIndex++)
		{
			FBspNode& Node = Model->Nodes(NodeIndex);
			FBspSurf& Surf = Model->Surfs(Node.iSurf);

			if(Node.NumVertices > 0)
			{
				for(INT BackFace = 0;BackFace < ((Surf.PolyFlags & PF_TwoSided) ? 2 : 1);BackFace++)
				{
					// Calculate the bounding box of this node.
					FBox NodeBounds(0);
					for(INT VertexIndex = 0;VertexIndex < Node.NumVertices;VertexIndex++)
					{
						NodeBounds += Model->Points(Model->Verts(Node.iVertPool + VertexIndex).pVertex);
					}

					// Create a sort key for this node using the grid cell containing the center of the node's bounding box.
#define MODEL_GRID_SIZE_XY	2048.0f
#define MODEL_GRID_SIZE_Z	4096.0f
					FModelComponentKey Key;
					Key.ZoneIndex		= Model->NumZones ? Node.iZone[1 - BackFace] : INDEX_NONE;
					Key.X				= appFloor(NodeBounds.GetCenter().X / MODEL_GRID_SIZE_XY);
					Key.Y				= appFloor(NodeBounds.GetCenter().Y / MODEL_GRID_SIZE_XY);
					Key.Z				= appFloor(NodeBounds.GetCenter().Z / MODEL_GRID_SIZE_Z);
					Key.MaskedPolyFlags = Surf.PolyFlags & PF_ModelComponentMask;
					Key.LightingChannels = Surf.LightingChannels.Bitfield;
					// Don't accept lights if material is unlit.
					if( Surf.Material && Surf.Material->GetMaterial() && Surf.Material->GetMaterial()->LightingModel == MLM_Unlit )
					{
						Key.MaskedPolyFlags	= Key.MaskedPolyFlags & (~PF_AcceptsLights);
					}
			
					// Find an existing node list for the grid cell.
					TArray<WORD>* ComponentNodes = ModelComponentMap.Find(Key);
					if(!ComponentNodes)
					{
						// This is the first node we found in this grid cell, create a new node list for the grid cell.
						ComponentNodes = &ModelComponentMap.Set(Key,TArray<WORD>());
					}

					// Add the node to the grid cell's node list.
					ComponentNodes->AddUniqueItem(NodeIndex);
				}
			}
		}

		// Create a UModelComponent for each grid cell's node list.
		for(TMap< FModelComponentKey, TArray<WORD> >::TConstIterator It(ModelComponentMap);It;++It)
		{
			const FModelComponentKey&	Key		= It.Key();
			const TArray<WORD>&			Nodes	= It.Value();	
			for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
			{
				Model->Nodes(Nodes(NodeIndex)).ComponentIndex = ModelComponents.Num();							
				Model->Nodes(Nodes(NodeIndex)).ComponentNodeIndex = NodeIndex;
			}
			
			UModelComponent* ModelComponent = new(this) UModelComponent(Model,Key.ZoneIndex,ModelComponents.Num(),Key.MaskedPolyFlags,Key.LightingChannels,Nodes);
			ModelComponents.AddItem(ModelComponent);

			for(INT NodeIndex = 0;NodeIndex < Nodes.Num();NodeIndex++)
			{
				Model->Nodes(Nodes(NodeIndex)).ComponentElementIndex = INDEX_NONE;
				
				const WORD								Node	 = Nodes(NodeIndex);
				const TIndirectArray<FModelElement>&	Elements = ModelComponent->GetElements();
				for( INT ElementIndex=0; ElementIndex<Elements.Num(); ElementIndex++ )
				{
					if( Elements(ElementIndex).Nodes.FindItemIndex( Node ) != INDEX_NONE )
					{
						Model->Nodes(Nodes(NodeIndex)).ComponentElementIndex = ElementIndex;
						break;
					}
				}
			}
		}
	}
	else
	{
		for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
		{
			if(ModelComponents(ComponentIndex))
			{
				ModelComponents(ComponentIndex)->ConditionalDetach();
			}
		}
	}

	// Update model components.
	for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
	{
		if(ModelComponents(ComponentIndex))
		{
			ModelComponents(ComponentIndex)->ConditionalAttach(GWorld->Scene,NULL,FMatrix::Identity);
		}
	}

	// Initialize the model's index buffers.
	for(TMap<UMaterialInterface*,TScopedPointer<FRawIndexBuffer32> >::TIterator IndexBufferIt(Model->MaterialIndexBuffers);
		IndexBufferIt;
		++IndexBufferIt)
	{
		BeginInitResource(IndexBufferIt.Value());
	}
}


/** Called before an Undo action occurs */
void ULevel::PreEditUndo()
{
	Super::PreEditUndo();

	// Release the model's resources.
	Model->BeginReleaseResources();
	Model->ReleaseResourcesFence.Wait();

	// Detach existing model components.  These are left in the array, so they are saved for undoing the undo.
	for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
	{
		if(ModelComponents(ComponentIndex))
		{
			ModelComponents(ComponentIndex)->ConditionalDetach();
		}
	}

	// Wait for the components to be detached.
	FlushRenderingCommands();
}


/** Called after an Undo action occurs */
void ULevel::PostEditUndo()
{
	Super::PostEditUndo();

	Model->UpdateVertices();
	// Update model components that were detached earlier
	UpdateModelComponents();
}



/**
 * Invalidates the cached data used to render the level's UModel.
 */
void ULevel::InvalidateModelGeometry()
{
	// Save the level/model state for transactions.
	Model->Modify();
	Modify();

	// Begin releasing the model's resources.
	Model->BeginReleaseResources();

	// Remove existing model components.
	for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
	{
		if(ModelComponents(ComponentIndex))
		{
			ModelComponents(ComponentIndex)->Modify();
			ModelComponents(ComponentIndex)->ConditionalDetach();
		}
	}
	ModelComponents.Empty();
}

/**
 * Discards the cached data used to render the level's UModel.  Assumes that the
 * faces and vertex positions haven't changed, only the applied materials.
 */
void ULevel::InvalidateModelSurface()
{
	Model->InvalidSurfaces = TRUE;
}

void ULevel::CommitModelSurfaces()
{
	if(Model->InvalidSurfaces)
	{
		// Begin releasing the model's resources.
		Model->BeginReleaseResources();

		// Wait for the model's resources to be released.
		FlushRenderingCommands();

		// Clear the model index buffers.
		Model->MaterialIndexBuffers.Empty();

		// Update the model vertices.
		Model->UpdateVertices();

		// Update the model components.
		for(INT ComponentIndex = 0;ComponentIndex < ModelComponents.Num();ComponentIndex++)
		{
			if(ModelComponents(ComponentIndex))
			{
				ModelComponents(ComponentIndex)->CommitSurfaces();
			}
		}
		Model->InvalidSurfaces = 0;
		
		// Initialize the model's index buffers.
		for(TMap<UMaterialInterface*,TScopedPointer<FRawIndexBuffer32> >::TIterator IndexBufferIt(Model->MaterialIndexBuffers);
			IndexBufferIt;
			++IndexBufferIt)
		{
			BeginInitResource(IndexBufferIt.Value());
		}
	}
}

/**
 * Rebuilds static streaming data.	
 */
void ULevel::BuildStreamingData(UTexture2D* TargetTexture)
{
#if !CONSOLE
	DOUBLE StartTime = appSeconds();

	// Start from scratch.
	TextureToInstancesMap.Empty();
	ForceStreamTextures.Empty();

	for( TObjectIterator<UPrimitiveComponent> It; It; ++It )
	{
		const UPrimitiveComponent* const Primitive = *It;

		// Only handle attached primitives in this level.
		if(Primitive->IsAttached() && Primitive->IsIn(this))
		{
			const AActor* const Owner				= Primitive->GetOwner();
			const UBOOL bIsStaticMeshComponent		= Primitive->IsA(UStaticMeshComponent::StaticClass());
			const UBOOL bIsSkeletalMeshComponent	= Primitive->IsA(USkeletalMeshComponent::StaticClass());
			const UBOOL bIsStatic					= !Owner || Owner->bStatic;
			const UBOOL bIsLevelPlacedKActor		= Owner && Owner->bNoDelete && Owner->IsA(AKActor::StaticClass());
			const UBOOL bUseAllStaticMeshComponents	= bIsStaticMeshComponent && (!Owner || Owner->bConsiderAllStaticMeshComponentsForStreaming);
			const UBOOL bStreamNonWorldTextures		= bIsStatic || bIsLevelPlacedKActor || bUseAllStaticMeshComponents;

			// Ask the primitive to enumerate the streaming textures it uses.
			TArray<FStreamingTexturePrimitiveInfo> PrimitiveStreamingTextures;
			Primitive->GetStreamingTextureInfo(PrimitiveStreamingTextures);

			// If the primitive isn't a skeletal mesh, add its streaming textures to the texture instance map.
			if(!bIsSkeletalMeshComponent)
			{
				for(INT TextureIndex = 0;TextureIndex < PrimitiveStreamingTextures.Num();TextureIndex++)
				{
					const FStreamingTexturePrimitiveInfo& PrimitiveStreamingTexture = PrimitiveStreamingTextures(TextureIndex);
					UTexture2D* Texture2D = Cast<UTexture2D>(PrimitiveStreamingTexture.Texture);

					// Only handle 2D textures that match the target texture.
					const UBOOL bIsTargetTexture = (!TargetTexture || TargetTexture == Texture2D);
					if(Texture2D && bIsTargetTexture)
					{
						// Only stream textures that are world textures.
						const UBOOL bIsLightOrShadowMap		= Texture2D->IsA(UShadowMapTexture2D::StaticClass()) || Texture2D->IsA(ULightMapTexture2D::StaticClass());
						const UBOOL bIsWorldTexture			= 
							Texture2D->LODGroup == TEXTUREGROUP_World ||
							Texture2D->LODGroup == TEXTUREGROUP_WorldNormalMap ||
							Texture2D->LODGroup == TEXTUREGROUP_WorldSpecular;
						const UBOOL bShouldStreamTexture	= (bStreamNonWorldTextures || bIsWorldTexture) && !bIsLightOrShadowMap;
						if( bShouldStreamTexture )
						{
							// Texture instance information.
							FStreamableTextureInstance TextureInstance;
							TextureInstance.BoundingSphere	= PrimitiveStreamingTexture.Bounds;
							TextureInstance.TexelFactor		= PrimitiveStreamingTexture.TexelFactor;

							// See whether there already is an instance in the level.
							TArray<FStreamableTextureInstance>* TextureInstances = TextureToInstancesMap.Find( Texture2D );

							// We have existing instances.
							if( TextureInstances )
							{
								// Add to the array.
								TextureInstances->AddItem( TextureInstance );
							}
							// This is the first instance.
							else
							{
								// Create array with current instance as the only entry.
								TArray<FStreamableTextureInstance> NewTextureInstances;
								NewTextureInstances.AddItem( TextureInstance );
								// And set it .
								TextureToInstancesMap.Set( Texture2D, NewTextureInstances );
							}
						}
					}
				}
			}

			// If the primitive is set to force its textures to be resident, add them to the ForceStreamTextures set.
			if(Primitive->bForceMipStreaming)
			{
				for(INT TextureIndex = 0;TextureIndex < PrimitiveStreamingTextures.Num();TextureIndex++)
				{
					UTexture2D* Texture2D = Cast<UTexture2D>(PrimitiveStreamingTextures(TextureIndex).Texture);
					if(Texture2D)
					{
						ForceStreamTextures.Set(Texture2D,TRUE);
					}
				}
			}
		}
	}

	debugf(TEXT("ULevel::BuildStreamingData took %.3f seconds."), appSeconds() - StartTime);
#else
	appErrorf(TEXT("ULevel::BuildStreamingData should not be called on a console"));
#endif
}

/**
 *	Retrieves the array of streamable texture isntances.
 *
 */
TArray<FStreamableTextureInstance>* ULevel::GetStreamableTextureInstances(UTexture2D*& TargetTexture)
{
	typedef TArray<FStreamableTextureInstance>	STIA_Type;
	for (TMap<UTexture2D*,STIA_Type>::TIterator It(TextureToInstancesMap); It; ++It)
	{
		TArray<FStreamableTextureInstance>& TSIA = It.Value();
		TargetTexture = It.Key();
		return &TSIA;
	}		

	return NULL;
}

/**
 * Returns the default brush for this level.
 *
 * @return		The default brush for this level.
 */
ABrush* ULevel::GetBrush() const
{
	checkMsg( Actors.Num() >= 2, *GetName() );
	ABrush* DefaultBrush = Cast<ABrush>( Actors(1) );
	checkMsg( DefaultBrush != NULL, *GetName() );
	checkMsg( DefaultBrush->BrushComponent, *GetName() );
	checkMsg( DefaultBrush->Brush != NULL, *GetName() );
	return DefaultBrush;
}

/**
 * Returns the world info for this level.
 *
 * @return		The AWorldInfo for this level.
 */
AWorldInfo* ULevel::GetWorldInfo() const
{
	check( Actors.Num() >= 2 );
	AWorldInfo* WorldInfo = Cast<AWorldInfo>( Actors(0) );
	check( WorldInfo != NULL );
	return WorldInfo;
}

/**
 * Returns the sequence located at the index specified.
 *
 * @return	a pointer to the USequence object located at the specified element of the GameSequences array.  Returns
 *			NULL if the index is not a valid index for the GameSequences array.
 */
USequence* ULevel::GetGameSequence() const
{
	USequence* Result = NULL;

	if( GameSequences.Num() )
	{
		Result = GameSequences(0);
	}

	return Result;
}

/**
 * Initializes all actors after loading completed.
 *
 * @param bForDynamicActorsOnly If TRUE, this function will only act on non static actors
 */
void ULevel::InitializeActors(UBOOL bForDynamicActorsOnly)
{
	UBOOL			bIsServer				= GWorld->IsServer();
	APhysicsVolume*	DefaultPhysicsVolume	= GWorld->GetDefaultPhysicsVolume();

	// Kill non relevant client actors, initialize render time, set initial physic volume, initialize script execution and rigid body physics.
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if( Actor && ( !bForDynamicActorsOnly || !Actor->bStatic ) )
		{
			// Kill off actors that aren't interesting to the client.
			if( !bIsServer && !Actor->bScriptInitialized )
			{
				if (Actor->bStatic || Actor->bNoDelete)
				{
					if (!Actor->bExchangedRoles)
					{
						Exchange( Actor->Role, Actor->RemoteRole );
						Actor->bExchangedRoles = TRUE;
					}
				}
				else
				{
					GWorld->DestroyActor( Actor );
				}
			}

			if( !Actor->ActorIsPendingKill() )
			{
				Actor->LastRenderTime	= 0.f;
				Actor->PhysicsVolume	= DefaultPhysicsVolume;
				Actor->Touching.Empty();
				// don't reinitialize actors that have already been initialized (happens for actors that persist through a seamless level change)
				if (!Actor->bScriptInitialized || Actor->GetStateFrame() == NULL)
				{
					Actor->InitExecution();
				}
			}
		}
	}
}

/**
 * Routes pre and post begin play to actors and also sets volumes.
 *
 * @param bForDynamicActorsOnly If TRUE, this function will only act on non static actors
 *
 * @todo seamless worlds: this doesn't correctly handle volumes in the multi- level case
 */
void ULevel::RouteBeginPlay(UBOOL bForDynamicActorsOnly)
{
	// this needs to only be done once, so when we do this again for reseting
	// dynamic actors, we can't do it again
	if (!bForDynamicActorsOnly)
	{
		GWorld->AddLevelNavList( this, TRUE );
	}

	// Send PreBeginPlay, set zones and collect volumes.
	TArray<AVolume*> LevelVolumes;		
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if( Actor && ( !bForDynamicActorsOnly || !Actor->bStatic ) )
		{
			if( !Actor->bScriptInitialized && (!Actor->bStatic || Actor->bRouteBeginPlayEvenIfStatic) )
			{
				Actor->PreBeginPlay();
			}
			AVolume* Volume = Actor->GetAVolume();
			if( Volume )
			{
				LevelVolumes.AddItem(Volume);
			}
		}
	}

	// Send set volumes, beginplay on components, and postbeginplay.
	for( INT ActorIndex=0; ActorIndex<Actors.Num(); ActorIndex++ )
	{
		AActor* Actor = Actors(ActorIndex);
		if( Actor && ( !bForDynamicActorsOnly || !Actor->bStatic ) )
		{
			if( !Actor->bScriptInitialized )
			{
				Actor->SetVolumes( LevelVolumes );
			}

			if( !Actor->bStatic || Actor->bRouteBeginPlayEvenIfStatic )
			{
#ifdef DEBUG_CHECKCOLLISIONCOMPONENTS
				INT NumCollisionComponents = 0;
#endif
				// Call BeginPlay on Components.
				for(INT ComponentIndex = 0;ComponentIndex < Actor->Components.Num();ComponentIndex++)
				{
					UActorComponent* ActorComponent = Actor->Components(ComponentIndex);
					if( ActorComponent && ActorComponent->IsAttached() )
					{
						ActorComponent->ConditionalBeginPlay();
#ifdef DEBUG_CHECKCOLLISIONCOMPONENTS
						UPrimitiveComponent *C = Cast<UPrimitiveComponent>(ActorComponent);
						if ( C && C->ShouldCollide() )
						{
							NumCollisionComponents++;
							if( NumCollisionComponents > 1 )
							{
								debugf(TEXT("additional collision component %s owned by %s"), *C->GetName(), *GetName());
							}
						}
#endif
					}
				}
			}
			if( !Actor->bScriptInitialized )
			{
				if( !Actor->bStatic || Actor->bRouteBeginPlayEvenIfStatic )
				{
					Actor->PostBeginPlay();
				}
				// Set script initialized if we skip routing begin play as some code relies on it.
				else
				{
					Actor->bScriptInitialized = TRUE;
				}
			}
		}
	}
}

UBOOL ULevel::HasAnyActorsOfType(UClass *SearchType)
{
	// just search the actors array
	for (INT Idx = 0; Idx < Actors.Num(); Idx++)
	{
		AActor *Actor = Actors(Idx);
		// if valid, not pending kill, and
		// of the correct type
		if (Actor != NULL &&
			!Actor->IsPendingKill() &&
			Actor->IsA(SearchType))
		{
			return TRUE;
		}
	}
	return FALSE;
}

UBOOL ULevel::HasPathNodes()
{
	// if this is the editor
	if (GIsEditor)
	{
		// check the actor list, as paths may not be rebuilt
		return HasAnyActorsOfType(ANavigationPoint::StaticClass());
	}
	else
	{
		// otherwise check the nav list pointers
		return (NavListStart != NULL && NavListEnd != NULL);
	}
}

//debug
//pathdebug
#if 0 && !PS3 && !FINAL_RELEASE
#define CHECKNAVLIST(b, x, n) \
		if( !GIsEditor && ##b ) \
		{ \
			debugf(*##x); \
			for (ANavigationPoint *T = GWorld->GetFirstNavigationPoint(); T != NULL; T = T->nextNavigationPoint) \
			{ \
				T->ClearForPathFinding(); \
			} \
			UWorld::VerifyNavList(*##x, ##n); \
		}
#else
#define CHECKNAVLIST(b, x, n)
#endif

void ULevel::AddToNavList( ANavigationPoint *Nav, UBOOL bDebugNavList )
{
	if (Nav != NULL)
	{
		CHECKNAVLIST(bDebugNavList, FString::Printf(TEXT("ADD %s to nav list %s"), *Nav->GetFullName(), *GetFullName()), Nav );

		UBOOL bNewList = FALSE;

		// if the list is currently invalid,
		if (NavListStart == NULL || NavListEnd == NULL)
		{
			// set the new nav as the start/end of the list
			NavListStart = Nav;
			NavListEnd = Nav;
			Nav->nextNavigationPoint = NULL;
			bNewList = TRUE;
		}
		else
		{
			// otherwise insert the nav at the end
			ANavigationPoint* Next = NavListEnd->nextNavigationPoint;
			NavListEnd->nextNavigationPoint = Nav;
			NavListEnd = Nav;
			Nav->nextNavigationPoint = Next;
		}
		// add to the cover list as well
		ACoverLink *Link = Cast<ACoverLink>(Nav);
		if (Link != NULL)
		{
			if (CoverListStart == NULL || CoverListEnd == NULL)
			{
				CoverListStart = Link;
				CoverListEnd = Link;
				Link->NextCoverLink = NULL;
			}
			else
			{
				ACoverLink* Next = CoverListEnd->NextCoverLink;
				CoverListEnd->NextCoverLink = Link;
				CoverListEnd = Link;
				Link->NextCoverLink = Next;
			}
		}

		if (bNewList && GIsGame)
		{
			GWorld->AddLevelNavList(this,bDebugNavList);
			debugfSuppressed(NAME_DevPath, TEXT(">>>  ADDED %s to world nav list because of %s"), *GetFullName(), *Nav->GetFullName());
		}

		CHECKNAVLIST(bDebugNavList, FString::Printf(TEXT(">>> ADDED %s to nav list"), *Nav->GetFullName()), Nav );
	}
}

void ULevel::RemoveFromNavList( ANavigationPoint *Nav, UBOOL bDebugNavList )
{
	if( GIsEditor && !GIsGame )
	{
		// skip if in the editor since this shouldn't be reliably used (build paths only)
		return;
	}

	if (Nav != NULL)
	{
		CHECKNAVLIST(bDebugNavList, FString::Printf(TEXT("REMOVE %s from nav list"), *Nav->GetFullName()), Nav );

		AWorldInfo *Info = GWorld->GetWorldInfo();

		// navigation point
		{
			// this is the nav that was pointing to this nav in the linked list
			ANavigationPoint *PrevNav = NULL;

			// remove from the world list
			// first check to see if this is the head of the world nav list
			if (Info->NavigationPointList == Nav)
			{
				// adjust to the next
				Info->NavigationPointList = Nav->nextNavigationPoint;
			}
			else
			{
				// otherwise hunt through the list for it
				for (ANavigationPoint *ChkNav = Info->NavigationPointList; ChkNav != NULL; ChkNav = ChkNav->nextNavigationPoint)
				{
					if (ChkNav->nextNavigationPoint == Nav)
					{
						// remove from the list
						PrevNav = ChkNav;
						ChkNav->nextNavigationPoint = Nav->nextNavigationPoint;
						break;
					}
				}
			}

			// check to see if it was the head of the level list
			if (Nav == NavListStart)
			{
				NavListStart = Nav->nextNavigationPoint;
			}

			// check to see if it was the end of the level list
			if (Nav == NavListEnd)
			{
				// if the previous nav is in this level
				if (PrevNav != NULL &&
					PrevNav->GetLevel() == this)
				{
					// then set the end to that
					NavListEnd = PrevNav;
				}
				// otherwise null the end
				else
				{
					NavListEnd = NULL;
				}
			}
		}

		// update the cover list as well (MIRROR NavList* update!)
		ACoverLink *Link = Cast<ACoverLink>(Nav);
		if (Link != NULL)
		{
			// this is the nav that was pointing to this nav in the linked list
			ACoverLink *PrevLink = NULL;

			// remove from the world list
			// first check to see if this is the head of the world nav list
			if (Info->CoverList == Link)
			{
				// adjust to the next
				Info->CoverList = Link->NextCoverLink;
			}
			else
			{
				// otherwise hunt through the list for it
				for (ACoverLink *ChkLink = Info->CoverList; ChkLink != NULL; ChkLink = ChkLink->NextCoverLink)
				{
					if (ChkLink->NextCoverLink == Link)
					{
						// remove from the list
						PrevLink = ChkLink;
						ChkLink->NextCoverLink = Link->NextCoverLink;
						break;
					}
				}
			}

			// check to see if it was the head of the level list
			if (Link == CoverListStart)
			{
				CoverListStart = Link->NextCoverLink;
			}

			// check to see if it was the end of the level list
			if (Link == CoverListEnd)
			{
				// if the previous nav is in this level
				if (PrevLink != NULL &&
					PrevLink->GetLevel() == this)
				{
					// then set the end to that
					CoverListEnd = PrevLink;
				}
				// otherwise null the end
				else
				{
					CoverListEnd = NULL;
				}
			}
		}

		CHECKNAVLIST(bDebugNavList, FString::Printf(TEXT(">>> REMOVED %s from nav list"), *Nav->GetFullName()), Nav );
	}
}

#undef CHECKNAVLIST

void ULevel::ResetNavList()
{
	NavListStart = NULL;
	NavListEnd = NULL;
	CoverListStart = NULL;
	CoverListEnd = NULL;
}

/*-----------------------------------------------------------------------------
	ULineBatchComponent implementation.
-----------------------------------------------------------------------------*/

/** Represents a LineBatchComponent to the scene manager. */
class FLineBatcherSceneProxy : public FPrimitiveSceneProxy
{
 public:
	FLineBatcherSceneProxy(const ULineBatchComponent* InComponent):
		FPrimitiveSceneProxy(InComponent), Lines(InComponent->BatchedLines)
	{
		ViewRelevance.bDynamicRelevance = TRUE;
		for(INT LineIndex = 0;LineIndex < Lines.Num();LineIndex++)
		{
			const ULineBatchComponent::FLine& Line = Lines(LineIndex);
			ViewRelevance.SetDPG(Line.DepthPriority,TRUE);
		}
	}

	/** 
	 * Draw the scene proxy as a dynamic element
	 *
	 * @param	PDI - draw interface to render to
	 * @param	View - current view
	 * @param	DPGIndex - current depth priority 
	 * @param	Flags - optional set of flags from EDrawDynamicElementFlags
	 */
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags)
	{
		for (INT i = 0; i < Lines.Num(); i++)
		{
			PDI->DrawLine(Lines(i).Start, Lines(i).End, Lines(i).Color, Lines(i).DepthPriority);
		}
	}

	/**
	 *  Returns a struct that describes to the renderer when to draw this proxy.
	 *	@param		Scene view to use to determine our relevence.
	 *  @return		View relevance struct
	 */
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		return ViewRelevance;
	}
	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() + Lines.GetAllocatedSize() ); }

 private:
	 TArray<ULineBatchComponent::FLine> Lines;
	 FPrimitiveViewRelevance ViewRelevance;
};

void ULineBatchComponent::DrawLine(const FVector& Start,const FVector& End,const FLinearColor& Color,BYTE DepthPriority)
{
	new(BatchedLines) FLine(Start,End,Color,DefaultLifeTime,DepthPriority);
	BeginDeferredReattach();
}

void ULineBatchComponent::DrawLine(const FVector& Start,const FVector& End,FColor Color,FLOAT LifeTime,BYTE DepthPriority)
{
	new(BatchedLines) FLine(Start,End,Color,LifeTime,DepthPriority);
	BeginDeferredReattach();
}

/** Provide many lines to draw - faster than calling DrawLine many times. */
void ULineBatchComponent::DrawLines(const TArray<FLine>& InLines)
{
	BatchedLines.Append(InLines);
	BeginDeferredReattach();
}

/** Draw a box. */
void ULineBatchComponent::DrawBox(const FBox& Box, const FMatrix& TM, const FColor& Color, BYTE DepthPriorityGroup)
{
	FVector	B[2],P,Q;
	INT ai,aj;
	const FMatrix& L2W = TM;
	B[0]=Box.Min;
	B[1]=Box.Max;

	for( ai=0; ai<2; ai++ ) for( aj=0; aj<2; aj++ )
	{
		P.X=B[ai].X; Q.X=B[ai].X;
		P.Y=B[aj].Y; Q.Y=B[aj].Y;
		P.Z=B[0].Z; Q.Z=B[1].Z;
		new(BatchedLines) FLine(TM.TransformFVector(P), TM.TransformFVector(Q), Color, DefaultLifeTime, DepthPriorityGroup);

		P.Y=B[ai].Y; Q.Y=B[ai].Y;
		P.Z=B[aj].Z; Q.Z=B[aj].Z;
		P.X=B[0].X; Q.X=B[1].X;
		new(BatchedLines) FLine(TM.TransformFVector(P), TM.TransformFVector(Q), Color, DefaultLifeTime, DepthPriorityGroup);

		P.Z=B[ai].Z; Q.Z=B[ai].Z;
		P.X=B[aj].X; Q.X=B[aj].X;
		P.Y=B[0].Y; Q.Y=B[1].Y;
		new(BatchedLines) FLine(TM.TransformFVector(P), TM.TransformFVector(Q), Color, DefaultLifeTime, DepthPriorityGroup);
	}

	BeginDeferredReattach();
}

void ULineBatchComponent::Tick(FLOAT DeltaTime)
{
	SCOPE_CYCLE_COUNTER( STAT_LineBatchComponentTickTime );

	UBOOL bNeedsUpdate = FALSE;
	// Update the life time of batched lines, removing the lines which have expired.
	for(INT LineIndex = 0;LineIndex < BatchedLines.Num();LineIndex++)
	{
		FLine& Line = BatchedLines(LineIndex);
		if(Line.RemainingLifeTime > 0.0f)
		{
			Line.RemainingLifeTime -= DeltaTime;
			if(Line.RemainingLifeTime <= 0.0f)
			{
				// The line has expired, remove it.
				BatchedLines.Remove(LineIndex--);
				bNeedsUpdate = TRUE;
			}
		}
	}
	if (bNeedsUpdate)
	{
		BeginDeferredReattach();
	}
}

/**
 * Creates a new scene proxy for the line batcher component.
 * @return	Pointer to the FLineBatcherSceneProxy
 */
FPrimitiveSceneProxy* ULineBatchComponent::CreateSceneProxy()
{
	return new FLineBatcherSceneProxy(this);
}

