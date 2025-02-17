/*=============================================================================
	UnUIPrefab.cpp: UI prefab implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


// Includes
#include "EnginePrivate.h"
#include "EnginePrefabClasses.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"
#include "ScopedObjectStateChange.h"

IMPLEMENT_CLASS(UUIPrefab);
IMPLEMENT_CLASS(UUIPrefabInstance);
IMPLEMENT_CLASS(UUIPrefabScene);

static void GenerateExclusionSet( TMap<UObject*,UObject*>& SourceMap, TLookupMap<UObject*>& ExclusionSet )
{
	for ( TMap<UObject*,UObject*>::TIterator It(SourceMap); It; ++It )
	{
		ExclusionSet.AddItem(It.Key());
		ExclusionSet.AddItem(It.Value());
	}
}

/* ==========================================================================================================
	UUIPrefabScene
========================================================================================================== */
/* === FCallbackEventDevice interface === */
/**
 * Handles validating that this scene's UIPrefab is still a valid widget after each undo is performed.
 */
void UUIPrefabScene::Send( ECallbackEventType InType )
{
	if ( InType == CALLBACK_Undo && Children.Num() > 0 )
	{
		UUIPrefab* Prefab = Cast<UUIPrefab>(Children(0));
		checkSlow(Prefab);

		if ( Prefab->OwnerScene == NULL )
		{
			// if the prefab's OwnerScene is NULL, it probably means that the user performed an undo action which "undid" the creation of the UIPrefab;
			// in this case we want to close our editor window immediately as the next frame we'll start crashing hard.

			// @todo ronp - ideally the EditorUISceneClient would call Close on the scene window, but since CloseScene is called from
			// the scene window's OnClose handler, we'll first need to verify that it's safe to call Close on the editor window twice
			/*SceneClient->CloseScene(this);*/
			Children.Empty();
			RequestSceneUpdate(TRUE,TRUE,TRUE,TRUE);
			RequestFormattingUpdate();
		}
	}
}

/* === UUIScreenObject interface === */
/**
 * Notification that this scene becomes the active scene.  Called after other activation methods have been called
 * and after focus has been set on the scene.
 *
 * This version registers this UIPrefabScene as an observer for CALLBACK_Undo events.
 *
 * @param	bInitialActivation		TRUE if this is the first time this scene is being activated; FALSE if this scene has become active
 *									as a result of closing another scene or manually moving this scene in the stack.
 */
void UUIPrefabScene::OnSceneActivated( UBOOL bInitialActivation )
{
	Super::OnSceneActivated(bInitialActivation);

	GCallbackEvent->Register(CALLBACK_Undo, this);
}

/**
 * Called just after this scene is removed from the active scenes array; unregisters this scene as an observer for undo callbacks.
 */
void UUIPrefabScene::Deactivate()
{
	for ( INT ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++ )
	{
		UUIPrefab* PrefabChild = Cast<UUIPrefab>(Children(ChildIndex));
		if ( PrefabChild != NULL )
		{
			FLOAT CurrentValue = PrefabChild->OriginalWidth.GetValue(this);
			PrefabChild->OriginalWidth.SetValue(this, PrefabChild->GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport));
			if ( !ARE_FLOATS_EQUAL(CurrentValue, PrefabChild->OriginalWidth.GetValue(this)) )
			{
				PrefabChild->MarkPackageDirty();
			}

			CurrentValue = PrefabChild->OriginalHeight.GetValue(this);
			PrefabChild->OriginalHeight.SetValue(this, PrefabChild->GetBounds(UIORIENT_Vertical, EVALPOS_PixelViewport));
			if ( !ARE_FLOATS_EQUAL(CurrentValue, PrefabChild->OriginalHeight.GetValue(this)) )
			{
				PrefabChild->MarkPackageDirty();
			}
		}
	}

	GCallbackEvent->UnregisterAll(this);
	Super::Deactivate();
}

/**
 * Insert a widget at the specified location
 *
 * @param	NewChild		the widget to insert; it must be a UIPrefab.
 * @param	InsertIndex		the position to insert the widget.  If not specified, the widget is insert at the end of
 *							the list
 * @param	bRenameExisting	controls what happens if there is another widget in this widget's Children list with the same tag as NewChild.
 *							if TRUE, renames the existing widget giving a unique transient name.
 *							if FALSE, does not add NewChild to the list and returns FALSE.
 *
 * @return	the position that that the child was inserted in, or INDEX_NONE if the widget was not inserted
 */
INT UUIPrefabScene::InsertChild(UUIObject* NewChild, INT InsertIndex/*=INDEX_NONE*/, UBOOL bRenameExisting/*=TRUE*/)
{
	if ( NewChild == NULL )
	{
		return INDEX_NONE;
	}

	if ( InsertIndex == INDEX_NONE )
	{
		InsertIndex = Children.Num();
	}

	// can't add NewChild to our Children array if it already has a parent
	if ( NewChild->GetParent() != NULL )
	{
		return INDEX_NONE;
	}

	INT Result = INDEX_NONE;

	// a UIPrefabScene is only used as a container for editing UIPrefabs.  So the only child it should ever have is
	// the UIPrefab itself.  However, dragging a mouse tool to create a widget always uses the scene as the parent for
	// the new widget, so if we already have a UIPrefab in the Children array, just route the InsertChild call to it.
	if ( NewChild->IsA(UUIPrefab::StaticClass()) ) 
	{
		if ( Children.Num() == 0 )
		{
			Result = Super::InsertChild(NewChild, InsertIndex, bRenameExisting);
		}
	}
	else if ( Children.Num() == 1 && Children(0)->IsA(UUIPrefab::StaticClass()) && NewChild != Children(0) )
	{
		Result = Children(0)->InsertChild(NewChild, InsertIndex, bRenameExisting);
	}

	return Result;
}

/**
 * Returns the default parent to use when placing widgets using the UI editor.  This widget is used when placing
 * widgets by dragging their outline using the mouse, for example.
 *
 * @return	a pointer to the widget that will contain newly placed widgets when a specific parent widget has not been
 *			selected by the user.
 */
UUIScreenObject* UUIPrefabScene::GetEditorDefaultParentWidget()
{
	UUIScreenObject* Result = NULL;
	if ( Children.Num() == 1 && Children(0)->IsA(UUIPrefab::StaticClass()) )
	{
		Result = Children(0);
	}
	else
	{
		Result = Super::GetEditorDefaultParentWidget();
	}

	return Result;
}

/**
 * Presave function. Gets called once before an object gets serialized for saving. This function is necessary
 * for save time computation as Serialize gets called three times per object from within UObject::SavePackage.
 *
 * @warning: Objects created from within PreSave will NOT have PreSave called on them!!!
 *
 * This version determines determines which sequences in this scene contains sequence ops that are capable of executing logic,
 * and marks sequence objects with the RF_NotForClient|RF_NotForServer if the op isn't linked to anything.
 */
void UUIPrefabScene::PreSave()
{
	// skip the UIScene version
	Super::Super::PreSave();
}

/* ==========================================================================================================
	UUIPrefab
========================================================================================================== */
/* === UUIPrefab interface === */
/**
 * Creates archetypes for the specified widgets and adds the archetypes to this UIPrefab.
 *
 * @param	WidgetPairs				[in]	the widgets to create archetypes for, along with their screen positions (in pixels)
 *									[out]	receives the list of archetypes that were created from the WidgetInstances
 *
 * @return	TRUE if archetypes were created and added to this UI archetype successfully.  FALSE if this UIPrefab
 *			is not an archetype, if the widgets specified are already archetypes, or couldn't other
 *			be created.
 */
UBOOL UUIPrefab::CreateWidgetArchetypes( TArray<FArchetypeInstancePair>& WidgetPairs, const FBox& BoundingRegion )
{
	// if this wrapper is an instance, can't add archetypes to it
	if ( !HasAnyFlags(RF_ArchetypeObject) )
	{
		return FALSE;
	}

	// if no widgets were specified, can't create archetypes
	if ( WidgetPairs.Num() == 0 )
	{
		return FALSE;
	}

	// adding archetypes for the specified instances is all-or-nothing; if any of the widgets in the list are not valid for becoming
	// members of this UI archetype, don't add any
	for ( INT Idx = 0; Idx < WidgetPairs.Num(); Idx++ )
	{
		UUIObject* Instance = WidgetPairs(Idx).WidgetInstance;

		// if any of the widgets are NULL, bail
		if ( Instance == NULL )
		{
			return FALSE;
		}

		// if any widget has been deleted and is waiting for garbage collection, can't create an archetype
		if ( Instance->IsPendingKill() )
		{
			return FALSE;
		}

		// if any widget is already an archetype, don't make an archetype
		if ( Instance->HasAnyFlags(RF_ArchetypeObject) )
		{
			return FALSE;
		}

		// check for duplicates
		for ( INT CheckIndex = 0; CheckIndex < WidgetPairs.Num(); CheckIndex++ )
		{
			if ( CheckIndex != Idx && Instance == WidgetPairs(CheckIndex).WidgetInstance )
			{
				return FALSE;
			}
		}
	}

	// build a mapping of widget classes to instance count, which is initialized using the existing archetypes in this wrapper
	TMap<UClass*,INT> WidgetTypeCounts;
	for ( INT ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++ )
	{
		UUIObject* ExistingChild = Children(ChildIndex);
		INT* ClassCount = WidgetTypeCounts.Find(ExistingChild->GetClass());
		if ( ClassCount != NULL )
		{
			(*ClassCount)++;
		}
		else
		{
			WidgetTypeCounts.Set(ExistingChild->GetClass(), 1);
		}
	}

	// this will be used to track all of the objects created as create the archetypes for this prefab
	TMap<UObject*,UObject*> DuplicationGraph;

	// the widgets have already been removed from their parent, so won't have a valid value for OwnerScene.  The only way to find out which
	// scene they're coming from is to iterate up the Outer chain to find a UIScene.  We only need to do this once as all of the widgets
	// in the WidgetInstances array should be in the same scene (but we'll assert on this just in case)
	UUIScene* SceneInstance=NULL;

	// In order for the archetype widget's positions to be identical to the original widget's positions when 
	// an instance of this UIPrefab is placed, their scale types need to be PercentageOwner.  To convert the
	// positions of the source widget instances into percentage values (relative to the bounding region of the
	// source widgets), we make the UIPrefab look like it is in the scene which contains the source widget instances
	// with a bounding region which matches the bounding region of all selected widgets.
	Position.SetRawPositionValue(UIFACE_Left,	BoundingRegion.Min.X,	EVALPOS_PixelViewport);
	Position.SetRawPositionValue(UIFACE_Top,	BoundingRegion.Min.Y,	EVALPOS_PixelViewport);
	Position.SetRawPositionValue(UIFACE_Right,	OriginalWidth.Value,	FUIScreenValue_Extent::TranslateScaleType(static_cast<EUIExtentEvalType>(OriginalWidth.ScaleType)));
	Position.SetRawPositionValue(UIFACE_Bottom,	OriginalHeight.Value,	FUIScreenValue_Extent::TranslateScaleType(static_cast<EUIExtentEvalType>(OriginalHeight.ScaleType)));

	TArray<UUIObject*> NewArchetypes;
	for ( INT InstanceIndex = 0; InstanceIndex < WidgetPairs.Num(); InstanceIndex++ )
	{
		FArchetypeInstancePair& Pair = WidgetPairs(InstanceIndex);
		UUIObject* SourceWidget = Pair.WidgetInstance;

		// create the archetype for the specified widget
		FName WidgetName = GenerateUniqueArchetypeName(WidgetTypeCounts, SourceWidget->GetClass());

		FObjectDuplicationParameters Params(SourceWidget, this);
		Params.FlagMask = RF_PropagateToSubObjects;
		Params.CreatedObjects = &DuplicationGraph;
		Params.DestName = WidgetName;
		Params.ApplyFlags = RF_Public|RF_ArchetypeObject|RF_Transactional;

		// if we don't yet have a reference to the scene which contained these widgets, do that now
		if ( SceneInstance == NULL )
		{
			for ( UObject* CheckOuter = SourceWidget->GetOuter(); CheckOuter; CheckOuter = CheckOuter->GetOuter() )
			{
				SceneInstance = Cast<UUIScene>(CheckOuter);
				if ( SceneInstance != NULL )
				{
					break;
				}
			}

			check(SceneInstance);
		}

		// seed the duplication map with all widgets that we've instanced so far
		Params.DuplicationSeed = DuplicationGraph;
		Params.DuplicationSeed.Set(SceneInstance, this);

		UUIObject* ArchetypeWidget = Cast<UUIObject>(StaticDuplicateObjectEx(Params));
		ArchetypeWidget->Created(this);
		ArchetypeWidget->CreateDefaultStates();
		if ( ArchetypeWidget->EventProvider != NULL )
		{
			ArchetypeWidget->EventProvider->InitializeEventProvider();
		}

		// clear any other properties that should not be transferred to the new archetype
		ArchetypeWidget->WidgetTag = WidgetName;
		ArchetypeWidget->WidgetID = FWIDGET_ID(EC_EventParm);

		// Clear any references in this archetype to non-public objects in other packages (eg. this will clear pointers to the widget's Owner, etc.)
		TMap<UObject*,UObject*> TempMap;

		// references to the owning scene won't be automatically cleared by the FArchiveReplaceObjectRef archive, since UIScenes are RF_Public
		// so set the replacement value for the scene to NULL to force these references to be cleared
		TempMap.Set(SourceWidget->GetScene(), NULL);
		FArchiveReplaceObjectRef<UObject> ReplaceAr(ArchetypeWidget, TempMap, TRUE, TRUE, TRUE);

		TArray<UUIObject*> ArchetypeChildren;
		ArchetypeWidget->GetChildren(ArchetypeChildren, TRUE);
		for ( INT ChildIndex = 0; ChildIndex < ArchetypeChildren.Num(); ChildIndex++ )
		{
			UUIObject* ArcChild = ArchetypeChildren(ChildIndex);
			if ( ArcChild->EventProvider != NULL )
			{
				ArcChild->EventProvider->InitializeEventProvider();
				ArcChild->WidgetTag = ArcChild->GetFName();
				ArcChild->WidgetID = FWIDGET_ID(EC_EventParm);
			}
		}

		InsertChild(ArchetypeWidget);
		NewArchetypes.AddItem(ArchetypeWidget);
		Pair.WidgetArchetype = ArchetypeWidget;

		// initialize the widget's position based on the render bounds associated with the source instance; this assumes that the caller
		// assigned a valid UIScene to this UIPrefab's OwnerScene reference
		{

			// We need to adjust the widget archetypes' positions so that they are in the same position (relative to each other and the bounding
			// box of all widgets going into the archetype).  Since the widget positions are in absolute pixels, we'll need to leverage the conversion
			// capability of UUIScreenObject::SetPosition; however, because the widgets aren't initialized by this point, SetPosition won't work correctly
			// since it can't iterate up the owner chain to find the owning scene.  So to workaround this, we'll just set the OwnerScene and Owner properties
			// of the UIPrefab and the new widget archetype during the call to SetPosition.
			ArchetypeWidget->Owner = this;
			OwnerScene = ArchetypeWidget->OwnerScene = SceneInstance;

			// first, we need to change the widget's scale type to be relative to owner, allowing for either pixels or percentage values
			for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
			{
				// clear the bResolved flag in case the source instance was pending any position updates
				ArchetypeWidget->DockTargets.MarkResolved(FaceIndex,0);
				ArchetypeWidget->Position.SetRawScaleType(FaceIndex, EVALPOS_PercentageOwner);
			}

			// assign the position of the archetype widget, which will automatically be converted into a percentage value
			ArchetypeWidget->SetPosition( Pair.ArchetypeBounds[UIFACE_Left], Pair.ArchetypeBounds[UIFACE_Top], Pair.ArchetypeBounds[UIFACE_Right],
				Pair.ArchetypeBounds[UIFACE_Bottom], EVALPOS_PixelViewport, TRUE, FALSE);

			// now clear these references
			ArchetypeWidget->Owner = NULL;
			OwnerScene = ArchetypeWidget->OwnerScene = NULL;
		}

		// make sure that reparenting is enabled on this archetype, in case we're creating an archetype from instances of another archetype
		ArchetypeWidget->SetPrivateBehavior(UCONST_PRIVATE_EditorNoReparent, FALSE, TRUE);
	}

	return TRUE;
}

void InitializePrefabInstance( UUIPrefabInstance* PrefInst )
{
	check(PrefInst);
	check(PrefInst->SourcePrefab);

	// duplicate the EventProvider
	FObjectDuplicationParameters Params(PrefInst->SourcePrefab->EventProvider, PrefInst);
	Params.bMigrateArchetypes = TRUE;
	Params.FlagMask = RF_Transactional;

	TMap<UObject*,UObject*> CreatedObjects;
	Params.CreatedObjects = &CreatedObjects;

	PrefInst->EventProvider = CastChecked<UUIComp_Event>(UObject::StaticDuplicateObjectEx(Params));
	PrefInst->Created(CastChecked<UUIScreenObject>(PrefInst->GetOuter()));

	// now duplicate the UIPrefab's states
	for ( INT StateIndex = 0; StateIndex < PrefInst->SourcePrefab->InactiveStates.Num(); StateIndex++ )
	{
		UUIState* SourceState = PrefInst->SourcePrefab->InactiveStates(StateIndex);
		Params.DestClass = SourceState->GetClass();
		Params.SourceObject = SourceState;

		UUIState* InstancedState = CastChecked<UUIState>(UObject::StaticDuplicateObjectEx(Params));
		InstancedState->Created();
		InstancedState->StateSequence->ParentSequence = PrefInst->EventProvider->EventContainer;
		PrefInst->InactiveStates.AddItem(InstancedState);
	}
	
	// now add all objects which were created during duplication to the UIPrefabInstance's arch->inst map
	for ( TMap<UObject*,UObject*>::TIterator It(CreatedObjects); It; ++It )
	{
		UObject* Arc = It.Key();
		UObject* Inst = It.Value();

		PrefInst->ArchetypeToInstanceMap.Set(Arc, Inst);
	}
}

/**
 * Creates an instance of this UIPrefab; does NOT add the new UIPrefabInstance to the specified DestinationOwner's
 * Children array.
 *
 * @param	DestinationOwner	the widget to use as the parent for the new PrefabInstance
 * @param	DestinationName		the name to use for the new PrefabInstance.
 *
 * @return	a pointer to a UIPrefabInstance created from this UIPrefab
 */
UUIPrefabInstance* UUIPrefab::InstancePrefab( UUIScreenObject* DestinationOwner, FName DestinationName )
{
	UUIPrefabInstance* PrefabInstance = NULL;
	if ( DestinationOwner != NULL )
	{
		// create the PrefabInstance
		FObjectInstancingGraph InstanceGraph;
		InstanceGraph.EnableComponentInstancing(FALSE);
		InstanceGraph.EnableObjectInstancing(FALSE);

		PrefabInstance = ConstructObject<UUIPrefabInstance>(UUIPrefabInstance::StaticClass(), DestinationOwner,
			GIsGame ? DestinationName : NAME_None, RF_Transactional, NULL, NULL, &InstanceGraph);
		check(PrefabInstance);

		// transact this object before we create any references to objects so that users will be able to delete
		// a newly created prefab after undoing its creation
		PrefabInstance->Modify();

		if ( !GIsGame )
		{
			// rather than creating the prefab instance using the specified name, rename the widget after calling Modify()
			// so that if the user undos the creation of the prefab instance they can still use the same name if they choose
			// to create another prefab instance
			PrefabInstance->Rename( DestinationName != NAME_None ? *DestinationName.ToString() : NULL, NULL, REN_ForceNoResetLoaders);
		}

		// Initialize the links back to this UIPrefab
		PrefabInstance->SourcePrefab = this;
		PrefabInstance->PrefabInstanceVersion = PrefabVersion;
		PrefabInstance->ArchetypeToInstanceMap.Set(this, PrefabInstance);

		// we need to make sure that we've created all subobjects that the UIPrefabInstance needs or would eventually create,
		// and we want those subobjects to be based on the corresponding subobject in this UIPrefab
		InitializePrefabInstance(PrefabInstance);

		// this will be used to track all of the objects created when we instance this prefab
		TMap<UObject*,UObject*> DuplicationGraph;
		DuplicationGraph.Set(this, PrefabInstance);
		for ( TMap<UObject*, UObject*>::TIterator It(PrefabInstance->ArchetypeToInstanceMap); It; ++It )
		{
			DuplicationGraph.Set(It.Key(), It.Value());
		}

		// store the child widgets we create in this array; if duplication fails for any children, the entire operation fails
		TArray<UUIObject*> ChildInstances;

		// Instance all of the children of this Prefab
		for ( INT ChildIndex = 0; ChildIndex < Children.Num(); ChildIndex++ )
		{
			UUIObject* ChildArchetype = Children(ChildIndex);

			FObjectDuplicationParameters Params(ChildArchetype, PrefabInstance);
			Params.bMigrateArchetypes = TRUE;
			Params.FlagMask = RF_Transactional;

			TMap<UObject*,UObject*> CreatedObjects;
			Params.CreatedObjects = &CreatedObjects;

			// seed the duplication map with all widgets that we've instanced so far
			Params.DuplicationSeed = DuplicationGraph;
			Params.DestName = ChildArchetype->GetFName();

			UUIObject* ChildInstance = CastChecked<UUIObject>(StaticDuplicateObjectEx(Params));
			if ( ChildInstance != NULL )
			{
				ChildInstance->Modify();

				// have to manually set the new widget's WidgetTag, since it will inherit the one from the archetype
				// (which will look like ClassName_Arc_##) and until the widget is renamed it won't update its WidgetTag
				// (because it's not NAME_None)
				ChildInstance->WidgetTag = ChildArchetype->GetFName();
				ChildInstances.AddItem(ChildInstance);
				for ( TMap<UObject*,UObject*>::TIterator It(CreatedObjects); It; ++It )
				{
					UObject* Arc = It.Key();
					UObject* Inst = It.Value();

					DuplicationGraph.Set(Arc,Inst);
					PrefabInstance->ArchetypeToInstanceMap.Set(Arc, Inst);

					// this should actually be unnecessary, but ya never know and it doesn't hurt anything.
					Inst->ClearFlags(RF_ArchetypeObject|RF_Public);
				}
			}
			else
			{
				// clear the result so the operation fails
				PrefabInstance = NULL;
				break;
			}
		}

		if ( PrefabInstance != NULL )
		{
			for ( INT ChildIndex = 0; ChildIndex < ChildInstances.Num(); ChildIndex++ )
			{
				UUIObject* ChildInstance = ChildInstances(ChildIndex);

				ChildInstance->Created(PrefabInstance);
				PrefabInstance->InsertChild(ChildInstance);
				ChildInstance->SetPrivateBehavior(UCONST_PRIVATE_EditorNoReparent, TRUE, TRUE);
			}

#if _DEBUG
			if ( !GIsGame )
			{
				// for debugging
				debugf(TEXT("Instanced objects (%i)"), DuplicationGraph.Num());
				INT i = 0;
				for ( TMap<UObject*,UObject*>::TIterator It(DuplicationGraph); It; ++It )
				{
					debugf(TEXT("\t%i) %s: %s"), i++, *It.Key()->GetFullName(), *It.Value()->GetFullName());
				}
			}
#endif
		}
	}

	// Now fix up references in new objects so they point to new instanced objects instead of archetype objects.
	ConvertObjectReferences(PrefabInstance->ArchetypeToInstanceMap, FALSE);

	return PrefabInstance;
}

/**
 * Notifies all instances of this UIPrefab to serialize their current property values against this UIPrefab.
 * Called just before something is modified in a widget contained in this UIPrefab.
 */
void UUIPrefab::SavePrefabInstances()
{
	// update all instances of this UIPrefab
	TArray<UObject*> PrefabInstances;
	GetArchetypeInstances(PrefabInstances);

	// first, serialize all prefab instances
	for ( INT InstIndex = 0; InstIndex < PrefabInstances.Num(); InstIndex++ )
	{
		UUIPrefabInstance* Inst = CastChecked<UUIPrefabInstance>(PrefabInstances(InstIndex));
		Inst->SavePrefabDifferences();
	}
}

/**
 * Notifies all instances of this UIPrefab to re-initialize and reload their property data.  Called just after
 * something is modified in a widget contained in this UIPrefab.
 */
void UUIPrefab::LoadPrefabInstances()
{
	// update all instances of this UIPrefab
	TArray<UObject*> PrefabInstances;
	GetArchetypeInstances(PrefabInstances);

	// first, serialize all prefab instances
	for ( INT InstIndex = 0; InstIndex < PrefabInstances.Num(); InstIndex++ )
	{
		UUIPrefabInstance* Inst = CastChecked<UUIPrefabInstance>(PrefabInstances(InstIndex));
		Inst->UpdateUIPrefabInstance();
	}
}
/**
 * Remaps object references within the specified set of objects.  Iterates through the specified map, applying an
 * FReplaceObjectReferences archive to each value in the map, using the map itself as the replacement map.
 *
 * @param	ReplacementMap		map of archetype => instances or instances => archetypes to use for replacing object
 *								references
 * @param	bNullPrivateRefs	should we null references to any private objects
 * @param	SourceObjects		if specified, applies the replacement archive on these objects instead of the values (objects)
 *								in the ReplacementMap.
 */
void UUIPrefab::ConvertObjectReferences( TMap<UObject*,UObject*>& ReplacementMap, UBOOL bNullPrivateRefs, TArray<UObject*>* pSourceObjects/*=NULL*/ )
{
	TArray<UObject*> MappedObjects;
	TArray<UObject*>& SourceObjects = pSourceObjects != NULL
		? *pSourceObjects
		: MappedObjects;

	if ( pSourceObjects == NULL )
	{
		// if no alternate list of objects was specified, generate one from the map itself
		ReplacementMap.GenerateValueArray(SourceObjects);
	}

	//@todo ronp - need an exclusion set here?
	for ( INT ObjIndex = 0; ObjIndex < SourceObjects.Num(); ObjIndex++ )
	{
		UObject* CurrentObj = SourceObjects(ObjIndex);

		// replace any references to actors in the set with references to the archetype version of that object
		FUIPrefabReplaceObjectRefArc RemapAr(CurrentObj, ReplacementMap, NULL, bNullPrivateRefs, TRUE, TRUE);
	}
}

/**
 * Generates a name for the widget specified in the format 'WidgetClass_Arc_##', where ## corresponds to the number of widgets of that
 * class already contained by this wrapper (though not completely representative, since other widget of that class may have been previously
 * removed).
 *
 * @param	WidgetTypeCounts	contains the number of widgets of each class contained by this wrapper
 * @param	WidgetInstance		the widget class to generate a unique archetype name for
 *
 * @return	a widget archetype name guaranteed to be unique within the scope of this wrapper.
 */
FName UUIPrefab::GenerateUniqueArchetypeName( TMap<UClass*,INT>& WidgetTypeCounts, UClass* WidgetClass ) const
{
	check(WidgetClass);
	check(WidgetClass->IsChildOf(UUIObject::StaticClass()));

	INT* pCurrentClassCount = WidgetTypeCounts.Find(WidgetClass);
	if ( pCurrentClassCount == NULL )
	{
		// first widget of this class in the map
		pCurrentClassCount = &WidgetTypeCounts.Set(WidgetClass, 0);
	}

	FName ArchetypeName = *FString::Printf(TEXT("%s_Arc_%i"), *WidgetClass->GetName(), (*pCurrentClassCount)++);
	INT ExistingChildIndex;
	while ( (ExistingChildIndex=FindChildIndex(ArchetypeName)) != INDEX_NONE )
	{
		// we already have a child widget using that name - increase the class count and try again
		ArchetypeName = *FString::Printf(TEXT("%s_Arc_%i"), *WidgetClass->GetName(), (*pCurrentClassCount)++);
	}

	return ArchetypeName;
}
	
/**
 * Determines whether the widget is allowed to be added to a UI prefab.
 *
 * @param	Widget	the widget to check
 *
 * @return	TRUE if this widget is allowed to be added to a UI prefab.
 */
UBOOL UUIPrefab::SupportsWidgetType( UUIObject* Widget )
{
	UBOOL bResult = TRUE;

	if ( Widget != NULL )
	{
		if ( Widget->IsA(UUICalloutButtonPanel::StaticClass()) || Widget->IsA(UUICalloutButton::StaticClass()) )
		{
			bResult = FALSE;
		}
	}

	return bResult;
}

/* === UUIScreenObject interface === */
/**
 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
 * once the scene has been completely initialized.
 * For widgets added at runtime, called after the widget has been inserted into its parent's
 * list of children.
 *
 * @param	inOwnerScene	the scene to add this widget to.
 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
 *							is being added to the scene's list of children.
 */
void UUIPrefab::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	Super::Initialize(inOwnerScene, inOwner);
}

/**
 * Returns the default parent to use when placing widgets using the UI editor.  This widget is used when placing
 * widgets by dragging their outline using the mouse, for example.
 *
 * @return	a pointer to the widget that will contain newly placed widgets when a specific parent widget has not been
 *			selected by the user.
 */
UUIScreenObject* UUIPrefab::GetEditorDefaultParentWidget()
{
	return this;
}

/* === UObject interface === */
/**
 * Called just before just object is saved to disk.  Updates the value of InternalPrefabVersion to match PrefabVersion,
 * and prevents the sequence objects contained in this prefab from being marked RF_NotForServer|RF_NotForClient.
 */
void UUIPrefab::PreSave()
{
	Super::PreSave();

	// update the InternalPrefabVersion so that it's up to date the next time this prefab is loaded
	InternalPrefabVersion = PrefabVersion;
	if ( EventProvider != NULL && EventProvider->EventContainer != NULL )
	{
		EventProvider->EventContainer->CalculateSequenceLoadFlags();
	}

	// this handles removal of widgets that have deprecated classes when this package is resaved via a commandlet
	if ( GIsUCC )
	{
		if ( GetClass()->HasAnyClassFlags(CLASS_Deprecated) )
		{
			ClearFlags(RF_Standalone);
		}
		else
		{
			TArray<UUIScreenObject*> DeprecatedObjects;
			FindDeprecatedWidgets(DeprecatedObjects);
			for ( INT ObjIndex = 0; ObjIndex < DeprecatedObjects.Num(); ObjIndex++ )
			{
				UUIScreenObject* DeprecatedObj = DeprecatedObjects(ObjIndex);

				// for now, we'll just remove the object.  another alternative would be to reparent
				// its children, but that becomes a lot more complex because some of those children
				// might be deprecated or might have the EditorNoReparent flag set

				UUIScreenObject* WidgetOwner = Cast<UUIScreenObject>(DeprecatedObj->GetOuter());
				if ( WidgetOwner != NULL )
				{
					WidgetOwner->RemoveChild(Cast<UUIObject>(DeprecatedObj));

					// set the RF_Transient flag in case there are redirectors referencing this object
					TArray<UObject*> Subobjects;
					FArchiveObjectReferenceCollector Collector(
						&Subobjects,
						DeprecatedObj,
						FALSE,
						TRUE,
						TRUE,
						FALSE
						);

					DeprecatedObj->Serialize(Collector);
					for ( INT SubobjectIndex = 0; SubobjectIndex < Subobjects.Num(); SubobjectIndex++ )
					{
						Subobjects(SubobjectIndex)->SetFlags(RF_Transient);
					}

					DeprecatedObj->SetFlags(RF_Transient);
				}
			}
		}
	}
}

/**
 * Note that the object has been modified.  If we are currently recording into the 
 * transaction buffer (undo/redo), save a copy of this object into the buffer and 
 * marks the package as needing to be saved.
 *
 * @param	bAlwaysMarkDirty	if TRUE, marks the package dirty even if we aren't
 *								currently recording an active undo/redo transaction
 */
void UUIPrefab::Modify( UBOOL bAlwaysMarkDirty/*=FALSE*/ )
{
	Super::Modify(bAlwaysMarkDirty);

	// increment the PrefabVersion if this is the first modification to this UIPrefab since it was loaded (ignore when playing in editor)
	if ( PrefabVersion == InternalPrefabVersion
	&& (GetOutermost()->PackageFlags & PKG_PlayInEditor) == 0 )
	{
		PrefabVersion++;
	}
}

/**
 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser.
 *
 * This version prints the number of widgets contained in this prefab.
 */
FString UUIPrefab::GetDesc()
{
	return FString::Printf(TEXT("%d total children"), GetObjectCount() - 1);
}

/**
 * Builds a list of UIPrefabInstances which have this UIPrefab as their SourcePrefab.
 *
 * @param	Instances	receives the list of UIPrefabInstances which have this UIPrefab as their SourcePrefab.
 */
void UUIPrefab::GetArchetypeInstances( TArray<UObject*>& Instances )
{
	Instances.Empty();
	if ( HasAnyFlags(RF_ClassDefaultObject) )
	{
		// ignore if we're the CDO
	}
	else
	{
		// editing an archetype object - objects of child classes won't be affected
		for ( TObjectIterator<UUIPrefabInstance> It; It; ++It )
		{
			UUIPrefabInstance* Inst = *It;
			if ( Inst->SourcePrefab == this && !Inst->IsPendingKill() )
			{
				Instances.AddItem(Inst);
			}
		}
	}
}

/**
 * Increments the value of ModificationCounter.  If the previous value was 0, notifies all instances of this UIPrefab
 * to serialize their current property values against this UIPrefab.  
 * Called just before something is modified in a widget contained in this UIPrefab.
 *
 * @param	AffectedObjects		ignored
 */
void UUIPrefab::SaveInstancesIntoPropagationArchive( TArray<UObject*>& AffectedObjects )
{
	checkSlow(ModificationCounter>=0);

	Modify();
	if ( ModificationCounter++ == 0 )
	{
		SavePrefabInstances();
	}
}

/**
 * Decrements the value of ModificationCounter.  If the new value is 0, notifies all instances of this UIPrefab to
 * re-initialize and reload their property data.  Called just after something is modified in a widget contained in this UIPrefab.
 *
 * @param	AffectedObjects		ignored
 */
void UUIPrefab::LoadInstancesFromPropagationArchive( TArray<UObject*>& AffectedObjects )
{
	checkSlow(ModificationCounter>0);

	Modify();
	if ( --ModificationCounter == 0 )
	{
		LoadPrefabInstances();
	}
}


/* ==========================================================================================================
	UUIPrefabInstance
========================================================================================================== */
/* === UIPrefabInstance interface === */
/**
 * Convert this prefab instance to look like the Prefab archetype version of it (by changing object refs to archetype refs and
 * converting positions to local space). Then serialise it, so we only get things that are unique to this instance. We store this
 * archive in the PrefabInstance.
 */
void UUIPrefabInstance::SavePrefabDifferences()
{
	// ensure that our ArchetypeToInstanceMap isn't missing any archetypes
	for ( TMap<UObject*, UObject*>::TIterator It(ArchetypeToInstanceMap); It; ++It )
	{
		UObject*& ObjArc = It.Key();
		UObject*& ObjInst = It.Value();
		if ( ObjArc == NULL && ObjInst != NULL )
		{
			if ( ObjInst == this )
			{
				ObjArc = SourcePrefab;
				debugf(TEXT("Archetype for %s was missing from ArchetypeToInstanceMap; restoring archetype %s"), *ObjInst->GetFullName(), *ObjArc->GetFullName());
			}
			else if ( ObjInst->GetArchetype()->IsIn(SourcePrefab) )
			{
				ObjArc = ObjInst->GetArchetype();
				debugf(TEXT("Archetype for %s was missing from ArchetypeToInstanceMap; restoring archetype %s"), *ObjInst->GetFullName(), *ObjArc->GetFullName());
			}
		}
	}

	// Create a mapping of widget instance => widget archetype by inverting the Arch-to-Inst map
	TMap<UObject*,UObject*> InstanceToArchetypeMap;
	APrefabInstance::CreateInverseMap(InstanceToArchetypeMap, ArchetypeToInstanceMap);

	// Set archive to write mode.
	FUIPrefabUpdateArc UpdateAr;
	UpdateAr.SetPortFlags(PPF_DeepCompareInstances);
	UpdateAr.SetPersistant(TRUE);
	UpdateAr.ActivateWriter();

	// because we need to copy the serialized bytes from the UIPrefabUpdateArc into this UIPrefabArcserialize this UIPrefabInstance into the archive as well (which 
	UpdateAr.SetRootObject(this);
	UpdateAr.InstanceGraph->SetDestinationRoot(this, SourcePrefab);

	// initialize the exclusion map to prevent the replacement archive from serializing the same objects more than once
	TLookupMap<UObject*> ExclusionSet;
	GenerateExclusionSet(ArchetypeToInstanceMap, ExclusionSet);

	for ( TMap<UObject*,UObject*>::TIterator InstIt(ArchetypeToInstanceMap); InstIt; ++InstIt )
	{
		UObject* Arc = InstIt.Key();
		UObject* Inst = InstIt.Value();
		
		if ( Inst != this )
		{
			if ( Inst != NULL )
			{
				Inst->PreSerializeIntoPropagationArchive();

				// Temporarily turn references to other objects in the prefab into archetype references, so it matches the archetype.
				FUIPrefabReplaceObjectRefArc(Inst, InstanceToArchetypeMap, &ExclusionSet, FALSE, TRUE, TRUE);
				//FArchiveReplaceObjectRef<UObject>(Inst, InstanceToArchetypeMap, FALSE, TRUE, TRUE);

				// Now save object. This will save properties which we have actually changed since we instanced the actor.
				((FReloadObjectArc&)UpdateAr) << Inst;

				// recopy the value of Inst, in case it was modified by the << operator
				Inst = InstIt.Value();

				// Turn back to normal.
				FUIPrefabReplaceObjectRefArc(Inst, ArchetypeToInstanceMap, &ExclusionSet, FALSE, TRUE, TRUE);
				//FArchiveReplaceObjectRef<UObject>(Inst, ArchetypeToInstanceMap, FALSE, TRUE, TRUE);

				Inst->PostSerializeIntoPropagationArchive();
			}
			else if ( Arc != NULL )
			{
				// If Inst is NULL, but Arc is not, then the user probably removed one of widgets that was instanced from SourcePrefab
				//@todo - do I need to serialize the NULL reference?
				//((FReloadObjectArc&)UpdateAr) << Inst;
			}
		}
	}

	PreSerializeIntoPropagationArchive();

	// Temporarily turn references to other objects in the prefab into archetype references, so it matches the archetype.
	FUIPrefabReplaceObjectRefArc(this, InstanceToArchetypeMap, &ExclusionSet, FALSE, TRUE, TRUE);
	//FArchiveReplaceObjectRef<UObject>(this, InstanceToArchetypeMap, FALSE, TRUE, TRUE);

	// we'll be re-initializing these arrays from the update archive, so we'll need to clear them
	// before serializing the UIPrefabInstance so that they don't cause objects to hang around that have
	// been deleted (because these arrays will be serialized as well)
	PI_CompleteObjects.Empty();
	PI_ReferencedObjects.Empty();
	PI_ObjectMap.Empty();

	// we'll also be re-initializing these arrays from the update archive, so we'll need to clear them
	// so that we don't serialize this data twice
	PI_Bytes.Empty();
	PI_SavedNames.Empty();

	PI_DataOffset = UpdateAr.Tell();
	Serialize(UpdateAr);

	// Keep information from this archive.
	CopyFromArchive(&UpdateAr);

	// Copying data from the UIPrefabUpdateArc changed our own data, so we'll need to serialize it into the archive again
// 	UpdateAr.Seek(PI_DataOffset);
// 	Serialize(UpdateAr);

	// finally, re-copy the data from the archive so that we have all of it in our persistent archives.
// 	CopyFromArchive(&UpdateAr);
	TArray<UObject*> CompleteObjects = PI_CompleteObjects, ReferencedObjects = PI_ReferencedObjects;

	// Turn back to normal.
	FUIPrefabReplaceObjectRefArc(this, ArchetypeToInstanceMap, &ExclusionSet, FALSE, TRUE, TRUE);
	//FArchiveReplaceObjectRef<UObject>(this, ArchetypeToInstanceMap, FALSE, TRUE, TRUE);

	// the ReplaceObjecRef archive will remap some references that we don't want mapped, so restore those now.
	//@todo - I think this is handled now in UUIPrefabInstance::Serialize() - this should be unnecessary
	PI_CompleteObjects = CompleteObjects;
	PI_ReferencedObjects = ReferencedObjects;

	PostSerializeIntoPropagationArchive();
}

/**
 * Generates a list of all widget and sequence archetypes contained in the specified parent which have never been instanced into this
 * UIPrefabInstance (and thus, have been newly added since this UIPrefabInstance was last saved), recursively.
 *
 * @param	ParentArchetype			a pointer to a widget archetype; must be contained within this UIPrefabInstance's SourcePrefab.
 * @param	NewWidgetArchetypes		receives the list of archetype widgets contained within ParentArchetype which have never been instanced into this
 *									UIPrefabInstance.
 * @param	NewSequenceArchetypes	receives the list of sequence object archetypes contained within ParentArchetype which do not exist in this UIPrefabInstance;
 *									will not include sequence objects contained in widgets that are also newly added.
 */
void UUIPrefabInstance::FindNewArchetypes( UUIObject* ParentArchetype, TArray<UUIObject*>& NewWidgetArchetypes, TArray<USequenceObject*>& NewSequenceArchetypes )
{
	check(ParentArchetype);
	check(ParentArchetype->IsInUIPrefab()||ParentArchetype->IsA(UUIPrefab::StaticClass()));

	TArray<UUIObject*> ArcChildren;
	ParentArchetype->GetChildren(ArcChildren, FALSE);
	for ( INT ChildIndex = 0; ChildIndex < ArcChildren.Num(); ChildIndex++ )
	{
		UUIObject* WidgetArchetype = ArcChildren(ChildIndex);
		if ( !ArchetypeToInstanceMap.HasKey(WidgetArchetype) )
		{
			// if there is no corresponding instance for this archetype in our map, it means that this archetype was added since the last time we
			// updated this UIPrefabInstance - add it to the array of archetypes we'll be instancing in the next step
			NewWidgetArchetypes.AddUniqueItem(WidgetArchetype);
		}
		else if ( ArchetypeToInstanceMap.Find(WidgetArchetype) != NULL )
		{
			// this widget archetype already has a corresponding instance in this UIPrefabInstance; first check the widget to see if any new sequence
			// objects have been created.
			if ( WidgetArchetype->EventProvider != NULL && WidgetArchetype->EventProvider->EventContainer != NULL )
			{
				if ( !ArchetypeToInstanceMap.HasKey(WidgetArchetype->EventProvider->EventContainer) )
				{
					check(ArchetypeToInstanceMap.HasKey(WidgetArchetype->EventProvider));
					NewSequenceArchetypes.AddUniqueItem(WidgetArchetype->EventProvider->EventContainer);

					//@todo ronp uiprefabs: hmmm, if the global sequence hasn't been instanced, then it seems like we'd still need 
					// to double-check that all of the state sequences have been.
				}
				else if ( ArchetypeToInstanceMap.Find(WidgetArchetype->EventProvider->EventContainer) != NULL )
				{
					// first, check the widget's global sequence
					FindNewSequenceArchetypes(WidgetArchetype, WidgetArchetype->EventProvider->EventContainer, NewSequenceArchetypes);

					// then check each of its state sequences
					for ( INT StateIndex = 0; StateIndex < WidgetArchetype->InactiveStates.Num(); StateIndex++ )
					{
						UUIState* StateArc = WidgetArchetype->InactiveStates(StateIndex);

						// skip any state sequences which are in the global sequence's list of SequenceObjects, as they will be processed when
						// FindNewSequenceArchetypes is called on the global sequence.
						if ( StateArc->StateSequence != NULL && !WidgetArchetype->EventProvider->EventContainer->SequenceObjects.ContainsItem(StateArc->StateSequence) )
						{
							if ( !ArchetypeToInstanceMap.HasKey(StateArc->StateSequence) )
							{
// 								check(ArchetypeToInstanceMap.HasKey(StateArc));
								NewSequenceArchetypes.AddUniqueItem(StateArc->StateSequence);
							}
							else if ( ArchetypeToInstanceMap.Find(StateArc->StateSequence) != NULL )
							{
								FindNewSequenceArchetypes(WidgetArchetype, StateArc->StateSequence, NewSequenceArchetypes);
							}
						}
					}
				}
			}
			
			// now check its children to see if there are any new widgets there
			FindNewArchetypes(WidgetArchetype, NewWidgetArchetypes, NewSequenceArchetypes);
		}
	}
}

/**
 * Helper method for recursively finding sequence archetype objects which have been added since this UIPrefabInstance was last updated.
 *
 * @param	ParentArchetype			a pointer to a widget archetype; must be contained within this UIPrefabInstance's SourcePrefab.
 * @param	SequenceToCheck			the sequence to search for new archetypes in
 * @param	NewSequenceArchetypes	receives the list of sequence object archetypes which do not exist in this UIPrefabInstance
 */
void UUIPrefabInstance::FindNewSequenceArchetypes( UUIObject* ParentArchetype, USequence* SequenceToCheck, TArray<USequenceObject*>& NewSequenceArchetypes )
{
	checkSlow(SequenceToCheck);
	for ( INT SeqObjIndex = 0; SeqObjIndex < SequenceToCheck->SequenceObjects.Num(); SeqObjIndex++ )
	{
		USequenceObject* SeqObj = SequenceToCheck->SequenceObjects(SeqObjIndex);
		if ( !SeqObj->HasAnyFlags(RF_Transient) && SeqObj->IsIn(ParentArchetype) )
		{
			if ( !ArchetypeToInstanceMap.HasKey(SeqObj) )
			{
				NewSequenceArchetypes.AddUniqueItem(SeqObj);
			}
			else if ( ArchetypeToInstanceMap.Find(SeqObj) != NULL )
			{
				USequence* Sequence = Cast<USequence>(SeqObj);
				if ( Sequence != NULL )
				{
					FindNewSequenceArchetypes(ParentArchetype, Sequence, NewSequenceArchetypes);
				}
			}
		}
	}

}


/**
 * This struct handles propagation of changes to the parent-child hierarchy of a UIPrefab to its instances.
 */
struct FUIPrefabInstanceHierarchyValidator
{
	/** The UIPrefabInstance that is being updated */
	UUIPrefabInstance*	PrefabInstance;

	/** Used as the exclusion set when reparenting widgets */
	TArray<UUIObject*>	AllChildInstances;

	/**
	 * A mapping of widget archetype => widget instance for all widget archetypes which have been re-parented since
	 * the PrefabInstance was last updated.
	 */
	TMap<UUIObject*,UUIObject*>	ReparentedWidgets;

public:
	/** Constructor */
	FUIPrefabInstanceHierarchyValidator( UUIPrefabInstance* InPrefabInstance )
	: PrefabInstance(InPrefabInstance)
	{
		check(PrefabInstance);
		check(PrefabInstance->SourcePrefab);
	}

	/**
	 * Main entry point for this struct's functionality.  Begins the operation (checking the parent-child relationships of the source
	 * UIPrefab, reparenting any children in the UIPrefabInstance, etc.)
	 */
	void BeginHierarchyValidation()
	{
		AllChildInstances.Empty(PrefabInstance->GetObjectCount());

		// find all widget instances whose archetype was re-parented
		ValidateChildren(PrefabInstance);

		// now reparent those instances to the correct parent
		ReparentInstances();
	}

protected:

	/**
	 * Finds all children of the specified widget that do not have the correct parent (based on whether
	 * the widget's archetype has been reparented) and adds them to the ReparentedWidgets map.
	 *
	 * @param	CurrentParent	the widget instance to check children for
	 */
	void ValidateChildren( UUIObject* CurrentParent )
	{
		check(CurrentParent);

		// set the current parent to the UIPrefab itself and validate the children of the UIPrefabInstance
		UUIObject* CurrentParentArchetype = CurrentParent == PrefabInstance ? PrefabInstance->SourcePrefab : CurrentParent->GetArchetype<UUIObject>();

		// grab the immediate children of the current parent
		TArray<UUIObject*> ChildInstances;
		CurrentParent->GetChildren(ChildInstances, FALSE);

		for ( INT ChildIndex = ChildInstances.Num() - 1; ChildIndex >= 0; ChildIndex-- )
		{
			if ( ChildInstances(ChildIndex)->HasAnyFlags(RF_ArchetypeObject) )
			{
				// sometimes we can end up with widget archetypes in our Children array so just do a quick check for those here.
				CurrentParent->Children.Remove(ChildIndex);
				continue;
			}

			UUIObject* ChildArchetype = ChildInstances(ChildIndex)->GetArchetype<UUIObject>();
			checkSlow(ChildArchetype);

			// for each child, check whether its archetype's Outer is the current parent's archetype
			if ( ChildArchetype->GetOuter() != CurrentParentArchetype )
			{
				if ( ChildArchetype->IsIn(PrefabInstance->SourcePrefab) )
				{
					// if not, it indicates that this archetype has been reparented; add the widget to the list of widgets that must be fixed (we'll do this later)
					ReparentedWidgets.Set(ChildArchetype, ChildInstances(ChildIndex));
					AllChildInstances.AddItem(ChildInstances(ChildIndex));
				}

				// remove any children that we added to the list of archetypes that have been reparented, or
				// were not instanced from the SourcePrefab (i.e. their ObjectArchetype is not contained within the SourcePrefab)
				ChildInstances.Remove(ChildIndex);
			}
		}

		// at this point, ChildInstances should only contain widgets which were instanced from the SourcePrefab and don't need to be re-parented;
		// recursively validate their children as well
		for ( INT ChildIndex = 0; ChildIndex < ChildInstances.Num(); ChildIndex++ )
		{
			AllChildInstances.AddItem(ChildInstances(ChildIndex));
			ValidateChildren(ChildInstances(ChildIndex));
		}
	}

	/**
	 * Reparents any widgets whose archetypes have been reparented, as determined in ValidateChildren().
	 */
	void ReparentInstances()
	{
		for ( TMap<UUIObject*,UUIObject*>::TIterator It(ReparentedWidgets); It; ++It )
		{
			UUIObject* Arc = It.Key();
			UUIObject* Inst = It.Value();
			check(Inst);

			UUIObject* Arc_NewParent = Cast<UUIObject>(Arc->GetOuter());
			UUIObject* Inst_OldParent = Inst->GetOwner();
			UUIObject* Inst_NewParent = (Arc_NewParent == PrefabInstance->SourcePrefab)
				? PrefabInstance->SourcePrefab
				: CastChecked<UUIObject>(PrefabInstance->ArchetypeToInstanceMap.FindRef(Arc_NewParent));

			checkf(Inst_NewParent, TEXT("%s: No instance found corresponding to '%s' while attempting to reparent '%s'"),
				*PrefabInstance->GetFullName(), *Arc_NewParent->GetFullName(), *Inst->GetFullName());

			// at this point, we now have the reparented instance's current and new parent widgets - we're ready to move it
			ReparentWidget(Inst, Inst_OldParent, Inst_NewParent, AllChildInstances);
		}
	}

	/**
	 * Performs the actual reparenting operation, preserving the widget's current screen position.
	 */
	static void ReparentWidget( UUIObject* WidgetInstance, UUIObject* OldParent, UUIObject* NewParent, TArray<UUIObject*>& ExclusionSet )
	{
		// Save the widget's position before modifying its parent.
		FLOAT Position[4];
		Position[UIFACE_Left] = WidgetInstance->GetPosition(UIFACE_Left,EVALPOS_PixelViewport,TRUE);
		Position[UIFACE_Top] = WidgetInstance->GetPosition(UIFACE_Top,EVALPOS_PixelViewport,TRUE);
		Position[UIFACE_Right] = WidgetInstance->GetPosition(UIFACE_Right,EVALPOS_PixelViewport,TRUE);
		Position[UIFACE_Bottom] = WidgetInstance->GetPosition(UIFACE_Bottom,EVALPOS_PixelViewport,TRUE);

		UBOOL bSuccessful = FALSE;
		if ( OldParent != NULL && OldParent != NewParent )
		{
			// remember the index for this child into its current parent's Children array, in case the new
			// parent does not accept it
			INT CurrentIndex = OldParent->Children.FindItemIndex(WidgetInstance);
			check(CurrentIndex != INDEX_NONE);

			if ( OldParent->RemoveChild(WidgetInstance, &ExclusionSet) )
			{
				// If NewParent's Children array was never serialized (because it had not been modified since NewParent was instanced)
				// then WidgetInstance will already exist in NewParent's Children array (as a result of updating the UIPrefabInstance
				// against its SourcePrefab), but it will not be properly initialized.  So remove that entry now.
				INT ExistingIndex = NewParent->Children.FindItemIndex(WidgetInstance);
				if ( ExistingIndex != INDEX_NONE )
				{
					NewParent->Children.Remove(ExistingIndex);
				}
				
				
				// now we can insert the child using the normal path
				if ( NewParent->InsertChild(WidgetInstance, ExistingIndex) != INDEX_NONE )
				{
					bSuccessful = TRUE;
				}
				else
				{
					OldParent->InsertChild(WidgetInstance,CurrentIndex);
				}
			}
		}
		else if ( NewParent->InsertChild(WidgetInstance) != INDEX_NONE )
		{
			bSuccessful = TRUE;
		}

		// Reset the widget's position to its old position.
		if(bSuccessful == TRUE)
		{
			WidgetInstance->SetPosition(Position[UIFACE_Left], Position[UIFACE_Top], Position[UIFACE_Right], Position[UIFACE_Bottom], EVALPOS_PixelViewport, TRUE);
		}
	}
};

/**
 * Helper method for easily grabbing the widget that contains the specified SeqObj
 */
UUIObject* GetSequenceObjectOwner( USequenceObject* SeqObj )
{
	check(SeqObj);

	UUIObject* WidgetOwner=NULL;

	for ( UObject* CurrentOuter = SeqObj->GetOuter(); CurrentOuter && WidgetOwner == NULL; CurrentOuter = CurrentOuter->GetOuter() )
	{
		WidgetOwner = Cast<UUIObject>(CurrentOuter);
	}

	return WidgetOwner;
}

/**
 * Reinitializes this UIPrefabInstance against its SourcePrefab.  The main purpose of this function (over standard archetype propagation)
 * is to convert all inter-object references within the PrefabInstance into references to their archetypes.  This is the only way that
 * changes to object references in a UIPrefab can be propagated to UIPrefabInstances, since otherwise the UIPrefabInstance would serialize
 * those object references.
 *
 * This will destroy/create objects as necessary.
 *
 * @todo - could refactor each step into a separate function...but it's unclear whether that would be easier to follow and maintain that having it all together.
 */
void UUIPrefabInstance::UpdateUIPrefabInstance()
{
	// Do nothing if template is NULL.
	if ( SourcePrefab == NULL )
	{
		return;
	}

	TLookupMap<UObject*> ExclusionSet;
	GenerateExclusionSet(ArchetypeToInstanceMap, ExclusionSet);

//==================================
// STEP 1: Update all instances (ignoring any widgets we just instanced)
//==================================
	{

		// Next we load the object data previously serialized in SavePrefabDifferences().  This operation first re-initializes the instances' property
		// data with the values from its archetype, thus propagating any changes made to the archetype.
		FUIPrefabUpdateArc UpdateAr;
		CopyToArchive(&UpdateAr);
		UpdateAr.SetPersistant(TRUE);
		UpdateAr.ActivateReader();

		UpdateAr.SetRootObject(this);
		UpdateAr.InstanceGraph->SetDestinationRoot(this, SourcePrefab);

		// Iterate over each object in the instance propagating any changes made to its archetype
		for ( TMap<UObject*,UObject*>::TIterator InstIt(ArchetypeToInstanceMap); InstIt; ++InstIt )
		{
			UObject* Arc = InstIt.Key();		// for debugging purposes only
			UObject* Inst = InstIt.Value();

			// if Arc is NULL, it means that the archetype was removed from the source prefab.  Ignore it for now - we'll remove those in step 2
			if ( Inst != NULL && Inst != this )
			{
				// calls Modify(), among other things
				Inst->PreSerializeFromPropagationArchive();

				// re-init properties to the values from its archetype, then read in the data previously serialized for this object.
				((FReloadObjectArc&)UpdateAr) << Inst;

				// recopy the value of Inst, in case it was modified by the << operator
				Inst = InstIt.Value();

				// convert any archetype references into references to the corresponding instance contained in this UIPrefabInstance.
				FUIPrefabReplaceObjectRefArc(Inst, ArchetypeToInstanceMap, &ExclusionSet, FALSE, TRUE, TRUE);

				Inst->PostSerializeFromPropagationArchive();
			}
		}

		UpdateAr.Seek(PI_DataOffset);

		// calls Modify(), among other things
		PreSerializeFromPropagationArchive();

		// re-init properties to the values from its archetype, then read in the data previously serialized for this object.
		UpdateAr.InstanceGraph->EnableObjectInstancing(FALSE);
		UpdateAr.InstanceGraph->EnableComponentInstancing(FALSE);

		SafeInitProperties((BYTE*)this, GetClass()->GetPropertiesSize(), GetClass(), (BYTE*)SourcePrefab, UUIObject::StaticClass()->GetPropertiesSize(), this, this, UpdateAr.InstanceGraph);
		Serialize(UpdateAr);
		PostLoad();

		// Running the replace archive on this object will remap the objects in the following arrays, which we do NOT want
		TArray<UObject*> CompleteObjects = PI_CompleteObjects;
		TArray<UObject*> ReferencedObjects = PI_ReferencedObjects;

		// convert any archetype references into references to the corresponding instance contained in this UIPrefabInstance.
		FUIPrefabReplaceObjectRefArc(this, ArchetypeToInstanceMap, &ExclusionSet, FALSE, TRUE, TRUE);

		// the ReplaceObjecRef archive will remap some references that we don't want mapped, so restore those now.
		PI_CompleteObjects = CompleteObjects;
		PI_ReferencedObjects = ReferencedObjects;

		PostSerializeFromPropagationArchive();

		// Fix up references in the newly created objects, and null out references to objects that have been destroyed.
		// This will also catch any references that were set by PostSerializeFromPropagationArchive()
		for ( TMap<UObject*,UObject*>::TIterator InstIt(ArchetypeToInstanceMap); InstIt; ++InstIt )
		{
			UObject* WidgetArchetype = InstIt.Key();
			UObject* WidgetInstance = InstIt.Value();

			// if WidgetArchetype is NULL, it means that the archetype was removed from the source prefab.  Ignore it for now - we'll remove those in step 2
			if ( WidgetInstance != NULL && WidgetInstance != this )
			{
				// replace any references to archetype objects in the set with references to the instance version of that object
				FUIPrefabReplaceObjectRefArc(WidgetInstance, ArchetypeToInstanceMap, &ExclusionSet, FALSE, TRUE, TRUE);
			}
		}

		{
			UpdateAr.InstanceGraph->EnableObjectInstancing(TRUE);
			UpdateAr.InstanceGraph->EnableComponentInstancing(TRUE);

			for ( INT ObjIndex = 0; ObjIndex < UpdateAr.LoadedObjects.Num(); ObjIndex++ )
			{
				UObject* ReloadedObject = UpdateAr.LoadedObjects(ObjIndex);

				// serializing the stored data for this object should have replaced all of its original instanced object references
				// but there might have been new subobjects added to the object's archetype in the meantime (in the case of updating 
				// an prefab from a prefab instance, for example), so enable subobject instancing and instance those now.
				ReloadedObject->InstanceSubobjectTemplates(UpdateAr.InstanceGraph);

				// components which were identical to their archetypes weren't stored into this archive's object data, so re-instance those components now
				ReloadedObject->InstanceComponentTemplates(UpdateAr.InstanceGraph);
			}
		}
	}

//==================================
// STEP 2: Remove obsolete instances.
//==================================

	// For any widget archetypes which have been removed from the UIPrefab, remove the corresponding widget instance from this UIPrefabInstance.
	{
		// There are two indications that a widget archetype has been removed:
		// 1. when updating a UIPrefabInstance for the first time after loading it, there will be a NULL entry in the arc->inst map for each widget that was removed.
		// 2. when updating a UIPrefabInstance as a result of modifying its source UIPrefab, the removed archetype will still be in memory (so it won't have a NULL entry
		//		in the map), but it will no longer be part of the source UIPrefab's Children array.

		UBOOL bStaleSubobjects = FALSE;

		// first, we'll build a list of widget instances whose archetype no longer exists in the UIPrefab's Children array.
		TArray<UObject*> InstancesToRemove;
		TArray<UUIObject*> WidgetInstancesToRemove;
		TArray<USequenceObject*> SequenceInstancesToRemove;
		
		for ( TMap<UObject*,UObject*>::TIterator It(ArchetypeToInstanceMap); It; ++It )
		{
			UObject* Arc = It.Key();
			UObject* Inst = It.Value();

			if ( Inst != this )
			{
				if ( Inst != NULL )
				{
					if ( Arc == NULL )
					{
						// no longer exists in the source prefab, remove it from the map
						It.RemoveCurrent();

						// now add it to the array of widgets we'll need to remove from our Children array
						if ( Inst->IsA(UUIObject::StaticClass()) )
						{
							WidgetInstancesToRemove.AddItem(static_cast<UUIObject*>(Inst));
						}
						else if ( Inst->IsA(USequenceObject::StaticClass()) )
						{
							SequenceInstancesToRemove.AddItem(static_cast<USequenceObject*>(Inst));
						}
						else
						{
							InstancesToRemove.AddItem(Inst);
						}

						bStaleSubobjects = TRUE;
					}
					else
					{
						if ( Inst->IsA(UUIObject::StaticClass()) )
						{
							UUIObject* WidgetInst = static_cast<UUIObject*>(Inst);
							UUIObject* WidgetArc = CastChecked<UUIObject>(Arc);
							checkSlow(WidgetArc==WidgetInst->GetArchetype());

							if ( !SourcePrefab->ContainsChild(WidgetArc, TRUE) )
							{
								// no longer exists in the source prefab, remove it from the map
								It.RemoveCurrent();
								WidgetInstancesToRemove.AddItem(WidgetInst);
								bStaleSubobjects = TRUE;
							}
						}
						else if ( Inst->IsA(USequenceObject::StaticClass()) )
						{
							USequenceObject* SeqInst = static_cast<USequenceObject*>(Inst);
							USequenceObject* SeqArc = CastChecked<USequenceObject>(Arc);
							checkSlow(SeqArc == SeqInst->GetArchetype());

							if ( SeqArc->ParentSequence == NULL )
							{
								UBOOL bRemoveSequenceObject = TRUE;

								UUIObject* OwnerWidget = GetSequenceObjectOwner(SeqArc);
								check(OwnerWidget);
								if (OwnerWidget->EventProvider != NULL
								&&	OwnerWidget->EventProvider->EventContainer != NULL

								// if this SequenceObject is the widget archetype's global sequence
								&&	(OwnerWidget->EventProvider->EventContainer == SeqArc

								// or the widget archetype's global sequence contains this sequence object in its SequenceObjects array
								||	OwnerWidget->EventProvider->EventContainer->ContainsSequenceObject(SeqArc)) )
								{
									// don't remove it
									bRemoveSequenceObject = FALSE;
								}

								for ( INT StateIndex = 0; bRemoveSequenceObject && StateIndex < OwnerWidget->InactiveStates.Num(); StateIndex++ )
								{
									UUIState* StateArc = OwnerWidget->InactiveStates(StateIndex);
									if (StateArc->StateSequence != NULL 
									
									// if this sequence object is one of the widget archetype's state sequences
									&&	(StateArc->StateSequence == SeqArc

									// or this state sequence contains this sequence object in its SequenceObjects array
									||	StateArc->StateSequence->ContainsSequenceObject(SeqArc)) )
									{
										// don't remove it
										bRemoveSequenceObject = FALSE;
									}
								}

								if ( bRemoveSequenceObject )
								{
									It.RemoveCurrent();
									SequenceInstancesToRemove.AddItem(SeqInst);
									bStaleSubobjects = TRUE;
								}
							}
						}
					}
				}
				else if ( Arc == NULL )
				{
					// if both key and value are NULL, remove the element
					It.RemoveCurrent();
				}
			}
		}

		// now at this point, all widget instances of removed archetypes have been removed from the map.  However, if the UIPrefabInstance was updated as a result of
		// a modification to the source UIPrefab (meaning all objects are still in memory), then the arc->inst map will still contain the subobjects of the removed widget
		// instances (when loading from disk, the map will contain a NULL key for all subobjects as well, so they should already be gone). 
		if ( bStaleSubobjects )
		{
			for ( INT RemovalIndex = 0; RemovalIndex < WidgetInstancesToRemove.Num(); RemovalIndex++ )
			{
				UUIObject* WidgetInst = WidgetInstancesToRemove(RemovalIndex);
				PI_CompleteObjects.RemoveItem(WidgetInst);
				WidgetInst->RemoveFromRoot();
				WidgetInst->ClearFlags(RF_DisregardForGC);

				for ( TMap<UObject*,UObject*>::TIterator It(ArchetypeToInstanceMap); It; ++It )
				{
					UObject* Inst = It.Value();

					// if this is a widget, it should have already been removed from the map by the previous loop
					if ( Inst != NULL && !Inst->IsA(UUIScreenObject::StaticClass()) && Inst->IsIn(WidgetInst) )
					{
						// this is a subobject of a widget instance that we've removed
						// hmmm, note that it might make sense to move this to a separate function
						It.RemoveCurrent();
						PI_CompleteObjects.RemoveItem(Inst);

						Inst->RemoveFromRoot();
						Inst->ClearFlags(RF_DisregardForGC);
					}
				}
			}
			for ( INT RemovalIndex = 0; RemovalIndex < SequenceInstancesToRemove.Num(); RemovalIndex++ )
			{
				USequenceObject* SeqInst = SequenceInstancesToRemove(RemovalIndex);
				PI_CompleteObjects.RemoveItem(SeqInst);
				SeqInst->RemoveFromRoot();
				SeqInst->ClearFlags(RF_DisregardForGC);

				if ( SeqInst->IsA(UUIStateSequence::StaticClass()) )
				{
					// for state sequences, we'll also need to remove the state instance
					ArchetypeToInstanceMap.Remove(SeqInst->GetOuter());
					PI_CompleteObjects.RemoveItem(SeqInst->GetOuter());
					SeqInst->GetOuter()->RemoveFromRoot();
					SeqInst->GetOuter()->ClearFlags(RF_DisregardForGC);
				}
				for ( TMap<UObject*,UObject*>::TIterator It(ArchetypeToInstanceMap); It; ++It )
				{
					UObject* Inst = It.Value();

					// if this is a widget, it should have already been removed from the map by the previous loop
					if ( Inst != NULL && Inst->IsIn(SeqInst) )
					{
						// this is a subobject of a widget instance that we've removed
						// hmmm, note that it might make sense to move this to a separate function
						It.RemoveCurrent();
						PI_CompleteObjects.RemoveItem(Inst);
						Inst->RemoveFromRoot();
						Inst->ClearFlags(RF_DisregardForGC);
					}
				}
			}
			for ( INT RemovalIndex = 0; RemovalIndex < InstancesToRemove.Num(); RemovalIndex++ )
			{
				UObject* ObjInst = InstancesToRemove(RemovalIndex);
				PI_CompleteObjects.RemoveItem(ObjInst);
				ObjInst->RemoveFromRoot();
				ObjInst->ClearFlags(RF_DisregardForGC);
				
				for ( TMap<UObject*,UObject*>::TIterator It(ArchetypeToInstanceMap); It; ++It )
				{
					UObject* Inst = It.Value();

					// if this is a widget, it should have already been removed from the map by the previous loop
					if ( Inst != NULL && Inst->IsIn(ObjInst) )
					{
						// this is a subobject of a widget instance that we've removed
						// hmmm, note that it might make sense to move this to a separate function
						It.RemoveCurrent();
						PI_CompleteObjects.RemoveItem(Inst);
						Inst->RemoveFromRoot();
						Inst->ClearFlags(RF_DisregardForGC);
					}
				}
			}
		}

		// ok, now the arc->inst map should be completely up to date and no longer contain any objects which are instances of an archetype that was removed from
		// the source UIPrefab.  Now we remove the instances from our own children array
		for ( INT RemovalIndex = 0; RemovalIndex < WidgetInstancesToRemove.Num(); RemovalIndex++ )
		{
			UUIObject* WidgetInst = WidgetInstancesToRemove(RemovalIndex);
			
			UBOOL bRemoveChild = TRUE;

			// if this widget's parent is supposed to be removed, we don't need to remove this one
			for ( UUIObject* OwnerWidget = WidgetInst->GetOwner(); OwnerWidget; OwnerWidget = OwnerWidget->GetOwner() )
			{
				if ( WidgetInstancesToRemove.ContainsItem(OwnerWidget) )
				{
					bRemoveChild = FALSE;
					break;
				}
			}

			if ( bRemoveChild )
			{
				RemoveChild(WidgetInst, &WidgetInstancesToRemove);
			}
		}

		for ( INT RemovalIndex = 0; RemovalIndex < SequenceInstancesToRemove.Num(); RemovalIndex++ )
		{
			USequenceObject* SeqInst = SequenceInstancesToRemove(RemovalIndex);
			if ( SeqInst->ParentSequence != NULL )
			{
				SeqInst->ParentSequence->RemoveObject(SeqInst);
			}
			else
			{
				IUIEventContainer* OwnerContainer = static_cast<IUIEventContainer*>(SeqInst->GetOuter()->GetInterfaceAddress(UUIEventContainer::StaticClass()));
				if ( OwnerContainer != NULL )
				{
					OwnerContainer->RemoveSequenceObject(SeqInst);
				}
			}

			// clear the reference from the event component
			UUIComp_Event* EventOwner = Cast<UUIComp_Event>(SeqInst->GetOuter());
			if ( EventOwner && EventOwner->EventContainer == SeqInst )
			{
				EventOwner->EventContainer = NULL;
			}
		}

		// Finally, clean up our other members
		PI_CompleteObjects.RemoveItem(NULL);
		PI_ReferencedObjects.RemoveItem(NULL);
		PI_ObjectMap.Remove(NULL);
	}

//==================================
// STEP 3: Generate duplication seed
//==================================
	{
		// this graph will be used to instance any widget archetypes which were added since the last time this UIPrefabInstance was updated.  These new widget archetypes
		// may contain references to existing archetypes (which already have a corresponding instance in this UIPrefabInstance), so we'll need to seed the duplication graph
		// with all the existing widget instances so that any references to previously existing archetypes are converted to references to the instance of that archetype, rather
		// than causing a new instance of that archetype to be created.
		TMap<UObject*,UObject*> DuplicationGraph = ArchetypeToInstanceMap;
		DuplicationGraph.Set(SourcePrefab, this);

//==================================
// STEP 4: Instance newly added archetypes
//==================================
		// this array will contain all archetype widgets which exist in the source UIPrefab, but have never been instanced into this UIPrefabInstance.
		TArray<UUIObject*> NewArchetypes;
		TArray<USequenceObject*> NewSequenceArchetypes;
		FindNewArchetypes(SourcePrefab, NewArchetypes, NewSequenceArchetypes);

		if ( NewArchetypes.Num() > 0 || NewSequenceArchetypes.Num() > 0 )
		{
			// because of the way that instancing works, any new archetypes may have been instanced as part of the propagation
			TArray<UObject*> InstancedObjects;
			FArchiveObjectReferenceCollector SubobjectCollectionArc(&InstancedObjects, this, FALSE, TRUE, TRUE, FALSE);
			Serialize(SubobjectCollectionArc);

			// Instance all of the newly added archetypes
			for ( INT ArcIndex = 0; ArcIndex < NewArchetypes.Num(); ArcIndex++ )
			{
				UBOOL bShouldInstanceArchetype = TRUE;

				UUIObject* NewArchetype = NewArchetypes(ArcIndex);
				for ( INT InstanceIndex = 0; InstanceIndex < InstancedObjects.Num(); InstanceIndex++ )
				{
					UObject* ObjInstance = InstancedObjects(InstanceIndex);
					if ( ObjInstance->GetArchetype() == NewArchetype )
					{
						bShouldInstanceArchetype = FALSE;
						ArchetypeToInstanceMap.Set(NewArchetype, ObjInstance);
						break;
					}
				}

				if ( !bShouldInstanceArchetype )
				{
					continue;
				}

				// figure out which widget instance should be used as the Outer for the new widget we're about to create
				UUIObject* WidgetParent=this;
				for ( UObject* ExistingArcParent = NewArchetype->GetOuter(); bShouldInstanceArchetype && ExistingArcParent; ExistingArcParent = ExistingArcParent->GetOuter() )
				{
					if ( ExistingArcParent == SourcePrefab )
					{
						// none of the objects in this archetype's Outer chain have corresponding instances in this UIPrefabInstance, so we'll just
						// use the UIPrefabInstance itself as the new widget instance's parent.
						// hmmm, it seems like the only time we should get here is when NewArchetype->Outer is SourcePrefab...otherwise, it means that
						// we have several new archetypes and they are out of order (i.e. we're trying to instance an archetype before we've instanced its Outer)
						break;
					}
					else
					{
						// see if we have a corresponding instance for this Outer
						UObject** ExistingInstanceParent = ArchetypeToInstanceMap.Find(ExistingArcParent);
						if ( ExistingInstanceParent != NULL )
						{
							// found an entry - this means that this archetype (ExistingArcParent) has previously been instanced into this UIPrefabInstance.
							// but the user may have since manually removed that widget instance from the UIPrefabInstance, so we check for that next.
							if ( (*ExistingInstanceParent) != NULL )
							{
								// found it!  this is the object that should be used as the outer for the new widget instance
								WidgetParent = CastChecked<UUIObject>(*ExistingInstanceParent);
								break;
							}
						}
						else
						{
							// found an entry, but the value is NULL which indicates that the user manually removed this widget instance.
							// it's not really clear what the best course of action is here, but for now we'll just bail (even though this means
							// we might be losing widget instances if the user reparented existing widget archetypes under this newly added widget archetype)
							bShouldInstanceArchetype = FALSE;
							break;
						}
					}
				}

				// this should only be FALSE if the we determined that the user manually removed from this UIPrefabInstance the widget that would have been
				// the parent for this newly added widget archetype.
				if ( !bShouldInstanceArchetype )
				{
					continue;
				}

				checkSlow(WidgetParent);

				FObjectDuplicationParameters Params(NewArchetype, WidgetParent);
				Params.bMigrateArchetypes = TRUE;	// this indicates that we want the value of the duplicates ObjectArchetype to point to the object used
													// as the source in the duplication
				Params.FlagMask = RF_Transactional;

				TMap<UObject*,UObject*> CreatedObjects;
				Params.CreatedObjects = &CreatedObjects;

				// seed the duplication map with all widgets that we've instanced so far
				Params.DuplicationSeed = DuplicationGraph;

				// instance the archetype
				UUIObject* ChildInstance = CastChecked<UUIObject>(StaticDuplicateObjectEx(Params));
				if ( ChildInstance != NULL )
				{
					ChildInstance->Modify();

					// have to manually set the new widget's WidgetTag, since it will inherit the one from the archetype
					// (which will look like ClassName_Arc_##) and until the widget is renamed it won't update its WidgetTag
					// (because it's not NAME_None)
					ChildInstance->WidgetTag = ChildInstance->GetFName();

					// now go through and add all objects that were created to the duplication graph so that any references to those objects
					// in other newly added archetypes don't cause them to be instanced multiple times
					for ( TMap<UObject*,UObject*>::TIterator It(CreatedObjects); It; ++It )
					{
						UObject* Arc = It.Key();
						UObject* Inst = It.Value();

						DuplicationGraph.Set(Arc,Inst);
						ArchetypeToInstanceMap.Set(Arc, Inst);
						InstancedObjects.AddUniqueItem(Inst);

						// this should actually be unnecessary, but ya never know and it doesn't hurt anything.
						Inst->ClearFlags(RF_ArchetypeObject|RF_Public);
					}

					// after we were reinitialized against SourcePrefab, we might have the new archetype in our Children array already if this instance's
					// Children array still matches the SourcePrefab version, so remove it now
					WidgetParent->Children.RemoveItem(NewArchetype);

					ChildInstance->Created(WidgetParent);
					WidgetParent->InsertChild(ChildInstance);
					ChildInstance->SetPrivateBehavior(UCONST_PRIVATE_EditorNoReparent, TRUE, TRUE);
				}
			}

			// Instance all of the newly added sequence object archetypes
			for ( INT ArcIndex = 0; ArcIndex < NewSequenceArchetypes.Num(); ArcIndex++ )
			{
				UBOOL bShouldInstanceArchetype = TRUE;
				USequenceObject* NewArchetype = NewSequenceArchetypes(ArcIndex);
				for ( INT InstanceIndex = 0; InstanceIndex < InstancedObjects.Num(); InstanceIndex++ )
				{
					UObject* ObjInstance = InstancedObjects(InstanceIndex);
					if ( ObjInstance->GetArchetype() == NewArchetype )
					{
						bShouldInstanceArchetype = FALSE;
						ArchetypeToInstanceMap.Set(NewArchetype, ObjInstance);

						if ( ObjInstance->IsA(UUIStateSequence::StaticClass()) )
						{
							ArchetypeToInstanceMap.Set(NewArchetype->GetOuter(), ObjInstance->GetOuter());
						}
						break;
					}
				}

				if ( !bShouldInstanceArchetype )
				{
					continue;
				}

				UObject* ExistingArcParent = NewArchetype->GetOuter();
				UObject* NewInstanceParent = NULL;

				// see if we have a corresponding instance for this Outer
				UObject** ExistingInstanceParent = ArchetypeToInstanceMap.Find(ExistingArcParent);
				if ( ExistingInstanceParent != NULL )
				{
					// found an entry - this means that this archetype (ExistingArcParent) has previously been instanced into this UIPrefabInstance.
					// but the user may have since manually removed that sequence object instance from the UIPrefabInstance, so we check for that next.
					if ( (*ExistingInstanceParent) != NULL )
					{
						// found it!  this is the object that should be used as the outer for the new widget instance
						NewInstanceParent = *ExistingInstanceParent;
					}
					else
					{
						bShouldInstanceArchetype = FALSE;
					}
				}
				else
				{
					// found an entry, but the value is NULL which indicates that the user manually removed this widget instance.
					// it's not really clear what the best course of action is here, but for now we'll just bail (even though this means
					// we might be losing widget instances if the user reparented existing widget archetypes under this newly added widget archetype)
					bShouldInstanceArchetype = FALSE;
				}

				// this should only be FALSE if the we determined that the user manually removed from this UIPrefabInstance the sequence that would have been
				// the parent for this newly added SequenceObject archetype.
				if ( !bShouldInstanceArchetype )
				{
					continue;
				}

				checkSlow(NewInstanceParent);

				FObjectDuplicationParameters Params(NewArchetype, NewInstanceParent);
				Params.bMigrateArchetypes = TRUE;	// this indicates that we want the value of the duplicates ObjectArchetype to point to the object used
													// as the source in the duplication
				Params.FlagMask = RF_Transactional|RF_Public;

				TMap<UObject*,UObject*> CreatedObjects;
				Params.CreatedObjects = &CreatedObjects;

				// seed the duplication map with all widgets that we've instanced so far
				Params.DuplicationSeed = DuplicationGraph;

				// instance the archetype
				USequenceObject* NewSeqInstance = CastChecked<USequenceObject>(StaticDuplicateObjectEx(Params));
				if ( NewSeqInstance != NULL )
				{
					// now go through and add all objects that were created to the duplication graph so that any references to those objects
					// in other newly added archetypes don't cause them to be instanced multiple times
					for ( TMap<UObject*,UObject*>::TIterator It(CreatedObjects); It; ++It )
					{
						UObject* Arc = It.Key();
						UObject* Inst = It.Value();

						DuplicationGraph.Set(Arc,Inst);
						ArchetypeToInstanceMap.Set(Arc, Inst);
						InstancedObjects.AddUniqueItem(Inst);
					}

					NewSeqInstance->OnCreated();
					USequence* OwnerSequence = Cast<USequence>(NewInstanceParent);
					if ( OwnerSequence != NULL )
					{
						// after we were reinitialized against SourcePrefab, we might have the new archetype in our Children array already if this instance's
						// Children array still matches the SourcePrefab version, so remove it now
						OwnerSequence->SequenceObjects.RemoveItem(NewArchetype);

						OwnerSequence->AddSequenceObject(NewSeqInstance);
					}
					else
					{
						// the only sequence objects that should have an Outer of an event component or a UIState are the
						// widget's global and state sequences; the properties that reference those objects are marked 'instanced'
						// however, so we shouldn't ever need to do anything special
						checkf(!NewInstanceParent->IsA(UUIComp_Event::StaticClass()), TEXT("NewSeqInstance: %s     NewInstanceParent: %s    SourceObject: %s"),
							*NewSeqInstance->GetFullName(), *NewInstanceParent->GetFullName(), *Params.SourceObject->GetFullName());
						checkf(!NewInstanceParent->IsA(UUIState::StaticClass()), TEXT("NewSeqInstance: %s     NewInstanceParent: %s    SourceObject: %s"),
							*NewSeqInstance->GetFullName(), *NewInstanceParent->GetFullName(), *Params.SourceObject->GetFullName());
					}

				}
			}
		}
	}

//==================================
// STEP 5: Determine whether any parent-child relationships were changed in the UIPrefab, and update this UIPrefabInstance's children arrays accordingly.
//==================================
	{
		FUIPrefabInstanceHierarchyValidator HierarchyValidator(this);
		HierarchyValidator.BeginHierarchyValidation();
	}

//==================================
// STEP 6: If this UIPrefabInstance is currently open in a UI editor window, reinitialize all widgets to ensure that they have the correct Owner, OwnerScene, etc.
//==================================

	TArray<UUIObject*> PrefabInstanceChildren;
	GetChildren(PrefabInstanceChildren, TRUE);

	UUIScene* InstanceOwnerScene = GetScene();
	if ( InstanceOwnerScene != NULL && InstanceOwnerScene->SceneClient != NULL && InstanceOwnerScene->IsInitialized() )
	{
		InitializePlayerTracking();
		BYTE CustomInputMask = GetInputMask(FALSE,TRUE);
		if ( CustomInputMask != 0 )
		{
			SetInputMask(CustomInputMask, FALSE, TRUE);
			SetInputMask(CustomInputMask, TRUE, FALSE);
		}

		Initialize(GetScene(), GetOwner());
		eventPostInitialize();

		RequestFormattingUpdate();
		RequestSceneUpdate(TRUE, TRUE, TRUE, TRUE);

		for ( INT ChildIndex = 0; ChildIndex < PrefabInstanceChildren.Num(); ChildIndex++ )
		{
			UUIObject* Widget = PrefabInstanceChildren(ChildIndex);

			// the StateStack array is transient, so if we've been reloaded from a propagation archive (i.e. our source prefab was updated or something)
			// we need to reactivate our initial state
			// activate the initial state for this widget which initializes this widget's style
			Widget->ActivateInitialState(0);

			IUIDataStoreSubscriber* Subscriber = (IUIDataStoreSubscriber*)Widget->GetInterfaceAddress(IUIDataStoreSubscriber::UClassType::StaticClass());
			if ( Subscriber != NULL )
			{
				Subscriber->RefreshSubscriberValue(UCONST_TOOLTIP_BINDING_INDEX);
				Subscriber->RefreshSubscriberValue(UCONST_CONTEXTMENU_BINDING_INDEX);
				Subscriber->RefreshSubscriberValue();
			}
		}
	}

	// Finally, set the instance version to be the same as the template version.
	PrefabInstanceVersion = SourcePrefab->PrefabVersion;
}

/**
 * Iterates through the ArchetypeToInstanceMap and verifies that the archetypes for each of this PrefabInstance's actors exist.
 * For any actors contained by this PrefabInstance that do not have a corresponding archetype, removes the actor from the
 * ArchetypeToInstanceMap.  This is normally caused by adding a new actor to a PrefabInstance, updating the source Prefab, then loading
 * a new map without first saving the package containing the updated Prefab.  When the original map is reloaded, though it contains
 * an entry for the newly added actor, the source Prefab's linker does not contain an entry for the corresponding archetype.
 *
 * @return	TRUE if each pair in the ArchetypeToInstanceMap had a valid archetype.  FALSE if one or more archetypes were NULL.
 */
UBOOL UUIPrefabInstance::HasMissingArchetypes()
{
	UBOOL bMissingArchetypes = FALSE;

	for ( TMap<UObject*,UObject*>::TIterator It(ArchetypeToInstanceMap); It; ++It )
	{
		UObject* WidgetArchetype = It.Key();
		UObject* WidgetInstance = It.Value();
		if ( WidgetArchetype == NULL && WidgetInstance != NULL )
		{
			// this prefab member doesn't have a corresponding archetype
			bMissingArchetypes = TRUE;

			// notify the user that this prefab member was removed
			warnf(NAME_Warning, LocalizeSecure(LocalizeUnrealEd(TEXT("Prefab_MissingArchetypes")), *WidgetInstance->GetPathName(), *GetPathName()));

			// now remove it
			It.RemoveCurrent();
		}
	}

	return bMissingArchetypes;
}

/** Copy information to a FUIPrefabUpdateArc from this PrefabInstance for updating a PrefabInstance with. */
void UUIPrefabInstance::CopyToArchive( FUIPrefabUpdateArc* InArc )
{
	check(InArc);

	// package versions weren't always saved with the PrefabInstance data, so if the package versions match the default values,
	// then we'll attempt to pull the correct versions from the PrefabInstance's Linker.  This won't work if the PrefabInstance
	// has already been detached from its linker for whatever reason.
	if ( PI_PackageVersion == INDEX_NONE )
	{
		PI_PackageVersion = GetLinkerVersion();
	}

	if ( PI_LicenseePackageVersion == INDEX_NONE )
	{
		PI_LicenseePackageVersion = GetLinkerLicenseeVersion();
	}

	// when reading the stored property data from this prefab instance's PI_Bytes array, we need to make sure
	// that the archive's version is the same as the version used when the package containing this prefab instance
	// was saved, so that any compatibility code works as expected.
	InArc->SetVer( PI_PackageVersion );
	InArc->SetLicenseeVer( PI_LicenseePackageVersion );

	InArc->Bytes				= PI_Bytes;
	InArc->CompleteObjects		= PI_CompleteObjects;
	InArc->ReferencedObjects	= PI_ReferencedObjects;
	InArc->SavedNames			= PI_SavedNames;
	InArc->ObjectMap			= PI_ObjectMap;
}

/** Copy information from a FUIPrefabUpdateArc into this PrefabInstance for saving etc. */
void UUIPrefabInstance::CopyFromArchive( const FUIPrefabUpdateArc* InArc )
{
	check(InArc);

	PI_PackageVersion			= GPackageFileVersion;
	PI_LicenseePackageVersion	= GPackageFileLicenseeVersion;

	PI_Bytes				= InArc->Bytes;
	PI_SavedNames			= InArc->SavedNames;
	PI_ObjectMap			= InArc->ObjectMap;

	PI_CompleteObjects.Empty(InArc->CompleteObjects.Num());
	InArc->CompleteObjects.GenerateKeyArray(PI_CompleteObjects);

	PI_ReferencedObjects.Empty(InArc->ReferencedObjects.Num());
	InArc->ReferencedObjects.GenerateKeyArray(PI_ReferencedObjects);
}

/* === UIObject interface === */

/* === UUIScreenObject interface === */
/**
 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
 * once the scene has been completely initialized.
 * For widgets added at runtime, called after the widget has been inserted into its parent's
 * list of children.
 *
 * @param	inOwnerScene	the scene to add this widget to.
 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
 *							is being added to the scene's list of children.
 */
void UUIPrefabInstance::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	Super::Initialize(inOwnerScene, inOwner);
}

/**
 * Called immediately after a child has been removed from this screen object.
 *
 * @param	WidgetOwner		the screen object that the widget was removed from.
 * @param	OldChild		the widget that was removed
 * @param	ExclusionSet	used to indicate that multiple widgets are being removed in one batch; useful for preventing references
 *							between the widgets being removed from being severed.
 */
void UUIPrefabInstance::NotifyRemovedChild( UUIScreenObject* WidgetOwner, UUIObject* OldChild, TArray<UUIObject*>* ExclusionSet/*=NULL*/ )
{
	// we only care about this if we're in the editor
	if ( GIsEditor && !GIsGame && OldChild != NULL )
	{
		// if this widget was instanced from a widget in our source prefab, we'll need to remove it from our arrays
		UObject* ChildArchetype = OldChild->GetArchetype();
		if ( !ChildArchetype->HasAnyFlags(RF_ClassDefaultObject) &&	ArchetypeToInstanceMap.HasKey(ChildArchetype) )
		{
			// now get a list of all objects contained within the child being removed
			TArray<UObject*> ObjectsToRemove;
			FArchiveObjectReferenceCollector RefCollectorArc(&ObjectsToRemove, OldChild, FALSE, TRUE, TRUE, FALSE);
			Serialize(RefCollectorArc);

			ObjectsToRemove.AddItem(OldChild);
			for ( INT ObjIndex = 0; ObjIndex < ObjectsToRemove.Num(); ObjIndex++ )
			{
				UObject* Object = ObjectsToRemove(ObjIndex);
				UObject* Arc = Object->GetArchetype();
				if ( ArchetypeToInstanceMap.Find(Arc) != NULL )
				{
					ArchetypeToInstanceMap.Set(Arc, NULL);
					PI_CompleteObjects.RemoveItem(Object);
					PI_ReferencedObjects.RemoveItem(Object);
					PI_ObjectMap.Remove(Object);
				}
			}

			ArchetypeToInstanceMap.Set(ChildArchetype, NULL);
		}
	}

	Super::NotifyRemovedChild(WidgetOwner, OldChild, ExclusionSet);
}

/* === UObject interface === */
void UUIPrefabInstance::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// If the archive is a ReplaceObjectRef archive, and its SearchObject is this UIPrefabInstance
	// or one of its subobjects, don't serialize the arc->inst or object maps, as this will clobber
	// references that should not be changed
	UBOOL bSerializeMaps = Ar.GetArchiveName() != TEXT("ReplaceObjectRef");
	if ( !bSerializeMaps )
	{
		const UObject* SearchObject = ((const FArchiveReplaceObjectRef<UObject>*)&Ar)->GetSearchObject();
		if ( SearchObject->HasAnyFlags(RF_ArchetypeObject) )
		{
			bSerializeMaps = SearchObject != SourcePrefab && !SearchObject->IsIn(SourcePrefab);
		}
		else
		{
			bSerializeMaps = SearchObject != this && !SearchObject->IsIn(this);
		}
	}

	if ( bSerializeMaps )
	{
		// put these one two separate lines for easier debugging
		Ar << ArchetypeToInstanceMap;
		Ar << PI_ObjectMap;
	}
}

#if REQUIRES_SAMECLASS_ARCHETYPE
/**
 * Provides PrefabInstance & UIPrefabInstanc objects with a way to override incorrect behavior in ConditionalPostLoad()
 * until different-class archetypes are supported.
 *
 * @fixme - temporary hack; correct fix would be to support archetypes of a different class
 *
 * @return	pointer to an object instancing graph to use for logic in ConditionalPostLoad().
 */
FObjectInstancingGraph* UUIPrefabInstance::GetCustomPostLoadInstanceGraph()
{
	// UIPrefabInstance's ObjectArchetype is not the source Prefab (because archetypes must be the same class)
	// so ConditionalPostLoadSubobjects causes instanced components to be replaced.  Here we supply
	// ConditionalPostLoadSubobjects with an object instance graph that is pre-initialized with
	// the source prefab as the source root
	FObjectInstancingGraph* Result = new FObjectInstancingGraph(this, SourcePrefab);
	Result->SetLoadingObject(TRUE);

	{
		// now we fixup any existing instances.  this fixup code was added on 2/5/08, engine version 538 (VER_GLOBAL_SHADER_FILE)
		// for packages with a version higher than this, this code shouldn't find anything
		TMap<UObject*,UObject*> ReplacementMap;
		TArray<UUIComp_Event*> OrphanedComps;
		
		// search for components that were previously incorrectly instanced
		for ( INT Idx = 0; Idx < PI_CompleteObjects.Num(); Idx++ )
		{
			UUIComp_Event* EventComp = Cast<UUIComp_Event>(PI_CompleteObjects(Idx));
			if ( EventComp != NULL )
			{
				UUIScreenObject* EventCompOuter = EventComp->GetOwner();
				if ( EventCompOuter == NULL )
				{
					PI_CompleteObjects.Remove(Idx--);
				}
				else if ( EventCompOuter->EventProvider != EventComp )
				{
					if ( EventCompOuter->EventProvider == NULL || EventComp == NULL )
					{
						INT Result = appMsgf(AMT_YesNoCancel, TEXT("NULL EventComponent detected EventComp: %s     EventProvider: %s.    Replace existing, remove from prefab, or ignore?"), *EventComp->GetFullName(), *EventCompOuter->EventProvider->GetFullName());
						if ( Result == 0 )
						{
							EventCompOuter->EventProvider = EventComp;
						}
						else if ( Result == 1 )
						{
							PI_CompleteObjects.Remove(Idx--);
							// remove from the PI_CompleteObjects array
							PI_CompleteObjects.RemoveItem(EventComp);
							// remove from the referenced object array
							PI_ReferencedObjects.RemoveItem(EventComp);
							// remove from the object map
							PI_ObjectMap.Remove(EventComp);

							// remove from the arc => inst map
							UObject* const* ObjArc = ArchetypeToInstanceMap.FindKey(EventComp);
							if ( ObjArc )
							{
								ArchetypeToInstanceMap.Remove(*ObjArc);
							}
							continue;
						}
						else if ( Result == 2 )
						{
							continue;
						}
					}
					// we have two versions of the event component, which means that it was re-instanced by ConditionalPostLoadSubobjects
					if ( EventComp->GetArchetype()->IsAPrefabArchetype()
					&&	!EventCompOuter->EventProvider->GetArchetype()->IsAPrefabArchetype() )
					{
						OrphanedComps.AddUniqueItem(EventCompOuter->EventProvider);
						ReplacementMap.Set(EventCompOuter->EventProvider, EventComp);

						// also add the sequence in order to replace any ParentSequence references in child widget event components
						if ( EventCompOuter->EventProvider->EventContainer != NULL && EventComp->EventContainer != NULL )
						{
							ReplacementMap.Set(EventCompOuter->EventProvider->EventContainer, EventComp->EventContainer);
						}
					}
					else if ( !EventComp->GetArchetype()->IsAPrefabArchetype()
					&&	EventCompOuter->EventProvider->GetArchetype()->IsAPrefabArchetype() )
					{
						OrphanedComps.AddUniqueItem(EventComp);
						ReplacementMap.Set(EventComp, EventCompOuter->EventProvider);
						if ( EventCompOuter->EventProvider->EventContainer != NULL && EventComp->EventContainer != NULL )
						{
							ReplacementMap.Set(EventComp->EventContainer, EventCompOuter->EventProvider->EventContainer);
						}
					}
				}
			}
		}

		for ( INT CompIndex = 0; CompIndex < OrphanedComps.Num(); CompIndex++ )
		{
			UUIComp_Event* EventComp = OrphanedComps(CompIndex);
			UUIScreenObject* EventOwner = EventComp->GetOwner();

			// so what's happened is that EventComp is the component that was instanced from the source prefab, but due
			// to a bug related to the fact that a UIPrefabInstance's archetype is not a UIPrefab, the component was re-instanced.
			// so now we fix it all up

			TArray<UObject*> ObjectsToRemove;
			FArchiveObjectReferenceCollector Collector(&ObjectsToRemove, EventComp, FALSE, TRUE, TRUE, TRUE);
			EventComp->Serialize(Collector);

			ObjectsToRemove.AddUniqueItem(EventComp);

			// ok now we have a list of all objects that will be removed - remove them all from this prefabinstance
			for ( INT ObjIndex = 0; ObjIndex < ObjectsToRemove.Num(); ObjIndex++ )
			{
				UObject* Obj = ObjectsToRemove(ObjIndex);

				// make sure it doesn't get saved again
				Obj->SetFlags(RF_Transient);

				// clear any flags that would prevent it from being eliminated
				Obj->ClearFlags(RF_Standalone);
				Obj->RemoveFromRoot();
// 				Obj->MarkPendingKill();

				// remove from the PI_CompleteObjects array
				PI_CompleteObjects.RemoveItem(Obj);
				// remove from the referenced object array
				PI_ReferencedObjects.RemoveItem(Obj);
				// remove from the object map
				PI_ObjectMap.Remove(Obj);

				// remove from the arc => inst map
				UObject* const* ObjArc = ArchetypeToInstanceMap.FindKey(Obj);
				if ( ObjArc )
				{
					ArchetypeToInstanceMap.Remove(*ObjArc);
				}
			}
		}

		// now replace any references to the incorrectly instanced component with the correct one
		if ( ReplacementMap.Num() > 0 )
		{
			FArchiveReplaceObjectRef<UObject> ReplaceAr(
				this,				// the object to serialize from
				ReplacementMap,		// map of objects to replace
				FALSE,				// null private refs?
				TRUE,				// skip Outer member (i.e. disallow replacing the object's Outer?)
				TRUE,				// skip archetype refs (i.e. disallow replacing the object's ObjectArchetype?)
				TRUE				// don't automatically begin serialization.
			);
			ReplaceAr.SerializeSearchObject();
		}

		TLookupMap<UObject*> ArcsToCheck;

		// now fixup any archetype references so that changes to the source prefab are propagated correctly
		for ( TMap<UObject*,UObject*>::TIterator It(ArchetypeToInstanceMap); It; ++It )
		{
			UObject* Arc = It.Key();
			UObject* Obj = It.Value();
			
			if ( Obj != NULL )
			{
				if ( Obj != this && Arc != NULL && Obj->GetArchetype() != Arc )
				{
					Obj->SetArchetype(Arc);
				}
			}
			else if ( Arc != NULL )
			{
				ArcsToCheck.AddItem(Arc);
			}
		}

		for ( INT ObjIndex = 0; ObjIndex < PI_CompleteObjects.Num() && ArcsToCheck.Num() > 0; ObjIndex++ )
		{
			UObject* InnerObj = PI_CompleteObjects(ObjIndex);
			for ( INT ArcIndex = 0; ArcIndex < ArcsToCheck.Num(); ArcIndex++ )
			{
				UObject* Arc = ArcsToCheck(ArcIndex);
				if ( InnerObj->GetArchetype() == Arc )
				{
					MarkPackageDirty();
					ArchetypeToInstanceMap.Set(Arc, InnerObj);
					ArcsToCheck.Remove(ArcIndex);
					break;
				}
			}
		}
	}

	return Result;
}
#endif

void UUIPrefabInstance::PreSave()
{
	Super::PreSave();

	// Don't do this for default object.
	if( !IsTemplate() )
	{
		// If we are cooking...
		if ( GIsCooking )
		{
			// In cooking, clear all data as it's only needed in the editor
			ArchetypeToInstanceMap.Empty();
			PI_Bytes.Empty();
			PI_CompleteObjects.Empty();
			PI_ReferencedObjects.Empty();
			PI_SavedNames.Empty();
			PI_ObjectMap.Empty();
		}
		else
		{
			// remove all NULL entries - these are typically caused by having RF_Transient objects in the map then reloading from disk.
			for( TMap<UObject*,UObject*>::TIterator It(ArchetypeToInstanceMap); It; ++It )
			{
				if ( It.Key() == NULL && It.Value() == NULL )
				{
					It.RemoveCurrent();
				}
			}

			// Update the archive containing the differences between this Actor and its Prefab defaults.
			SavePrefabDifferences();
		}
	}
}

/**
 * Serializes the unrealscript property data located at Data.  When saving, only saves those properties which differ from the corresponding
 * value in the specified 'DiffObject' (usually the object's archetype).
 *
 * @param	Ar				the archive to use for serialization
 * @param	DiffObject		the object to use for determining which properties need to be saved (delta serialization);
 *							if not specified, the ObjectArchetype is used
 * @param	DefaultsCount	maximum number of bytes to consider for delta serialization; any properties which have an Offset+ElementSize greater
 *							that this value will NOT use delta serialization when serializing data;
 *							if not specified, the result of DiffObject->GetClass()->GetPropertiesSize() will be used.
 */
void UUIPrefabInstance::SerializeScriptProperties( FArchive& Ar, UObject* DiffObject/*=NULL*/, INT DiffCount/*=0*/ ) const
{
	if ( DiffObject == NULL && DiffCount == 0 )
	{
		// Since SourcePrefab is a different class, it can't be our archetype.  However, in order for changes to our SourcePrefab
		// to propagate correctly to this UIPrefabInstance, we'll need to use delta serialization for all properties common
		// to both the UIPrefab and UIPrefabInstance classes (basically everything down to UIObject) to serialize only the property 
		// that is either unique to this class or different from the value in SourcePrefab.
		DiffObject = SourcePrefab;
		DiffCount = UUIObject::StaticClass()->GetPropertiesSize();
	}
	Super::SerializeScriptProperties(Ar, DiffObject, DiffCount);
}

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void UUIPrefabInstance::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );

	//@todo rtgc: this can be removed once we have proper TMap support in script/ RTGC
	for( TMap<UObject*,UObject*>::TIterator It(ArchetypeToInstanceMap); It; ++It )
	{
		AddReferencedObject( ObjectArray, It.Key() );
		AddReferencedObject( ObjectArray, It.Value() );
	}

	for( TMap<UObject*,INT>::TIterator It(PI_ObjectMap); It; ++It )
	{
		AddReferencedObject( ObjectArray, It.Key() );
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
UBOOL UUIPrefabInstance::GetNativePropertyValues( TMap<FString,FString>& out_PropertyValues, DWORD ExportFlags/*=0*/ ) const
{
	UBOOL bResult = Super::GetNativePropertyValues(out_PropertyValues, ExportFlags);

	INT Count=0, LongestKeyName = 0;
	TMap<FString,FString> PropertyValues;
	PropertyValues.Empty(ArchetypeToInstanceMap.Num());
	for ( TMap<UObject*,UObject*>::TConstIterator It(ArchetypeToInstanceMap); It; ++It )
	{
		const UObject* Arc = It.Key();
		const UObject* Inst = It.Value();

		const FString KeyName = FString::Printf(TEXT("%s'%s'"), Arc ? *Arc->GetClass()->GetName() : TEXT("NULL"), *Arc->GetPathName());
		const FString ValueName = FString::Printf(TEXT("%s'%s'"), Inst ? *Inst->GetClass()->GetName() : TEXT("NULL"), *Inst->GetPathName());

		LongestKeyName = Max(LongestKeyName, KeyName.Len());
		PropertyValues.Set(*KeyName, *ValueName);
	}
	for ( TMap<FString,FString>::TConstIterator It(PropertyValues); It; ++It )
	{
		const FString PropertyName = FString::Printf(TEXT("ArchetypeToInstanceMap[%i]:"), Count++);
		const FString& ArcName = It.Key();
		const FString& InstName = It.Value();

		out_PropertyValues.Set(*PropertyName, *FString::Printf(TEXT("%s => %s"), *ArcName.RightPad(LongestKeyName), *InstName));
		bResult = TRUE;
	}

	Count = 0, LongestKeyName = 0;
	PropertyValues.Empty(PI_ObjectMap.Num());
	for ( TMap<UObject*,INT>::TConstIterator It(PI_ObjectMap); It; ++It )
	{
		const UObject* Obj = It.Key();
		const INT Offset = It.Value();

		const FString KeyName = FString::Printf(TEXT("%s'%s'"), Obj ? *Obj->GetClass()->GetName() : TEXT("NULL"), *Obj->GetPathName());
		const FString ValueName = FString::Printf(TEXT("%i"), Offset);

		LongestKeyName = Max(LongestKeyName, KeyName.Len());
		PropertyValues.Set(*KeyName, *ValueName);
	}
	for ( TMap<FString,FString>::TConstIterator It(PropertyValues); It; ++It )
	{
		const FString PropertyName = FString::Printf(TEXT("PI_ObjectMap[%i]:"), Count++);
		const FString& KeyName = It.Key();
		const FString& ValueName = It.Value();

		out_PropertyValues.Set(*PropertyName, *FString::Printf(TEXT("%s => %s"), *KeyName.RightPad(LongestKeyName), *ValueName));
		bResult = TRUE;
	}

	return bResult || ArchetypeToInstanceMap.Num() > 0 || PI_ObjectMap.Num() > 0;
}

/**
 * Converts all widgets in this UIPrefabInstance into normal widgets and removes the UIPrefabInstance from the scene.
 */
void UUIPrefabInstance::DetachFromSourcePrefab()
{
	UUIScreenObject* ParentWidget = GetParent();
	if ( ParentWidget != NULL )
	{
		TArray<UUIObject*> DirectChildren;
		GetChildren(DirectChildren, FALSE);

		TArray<FScopedObjectStateChange> Notifiers;
		for ( INT ChildIndex = DirectChildren.Num() - 1; ChildIndex >= 0; ChildIndex-- )
		{
			new(Notifiers) FScopedObjectStateChange(DirectChildren(ChildIndex));
		}
		new(Notifiers) FScopedObjectStateChange(this);
		new(Notifiers) FScopedObjectStateChange(ParentWidget);


		TArray<UUIObject*> AllChildren;
		GetChildren(AllChildren, TRUE);

		TArray<FArchetypeInstancePair> WidgetPairs;
		WidgetPairs.AddZeroed(DirectChildren.Num());

		// In order for the widget's positions to remain the same after they're removed from the prefab instance,
		// first need to get a list of pixel positions for all children of this prefab instance.  Once we've reparented
		// all children to the parent of this uiprefab, we'll apply those positions to each child and let SetPositionValue
		// do its thing.
		for ( INT ChildIndex = 0; ChildIndex < DirectChildren.Num(); ChildIndex++ )
		{
			UUIObject* ChildWidget = DirectChildren(ChildIndex);
			FArchetypeInstancePair& Pair = WidgetPairs(ChildIndex);
			
			Pair.WidgetInstance = ChildWidget;
			for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
			{
				Pair.InstanceBounds[FaceIndex] = ChildWidget->GetPosition(FaceIndex, EVALPOS_PixelViewport, TRUE);
			}
		}

		// remove this prefab instance from the parent widget
		ParentWidget->RemoveChild(this, &AllChildren);

		// now we remove all children from this prefab instance
		RemoveChildren(DirectChildren);

		// then add those widgets as children of our old parent
		for ( INT ChildIndex = 0; ChildIndex < DirectChildren.Num(); ChildIndex++ )
		{
			UUIObject* Child = DirectChildren(ChildIndex);
			ParentWidget->InsertChild(Child);
		}

		// finally, reapply the original positions to the widgets now that they have a new parent
		for ( INT ChildIndex = 0; ChildIndex < WidgetPairs.Num(); ChildIndex++ )
		{
			FArchetypeInstancePair& Pair = WidgetPairs(ChildIndex);
			Pair.WidgetInstance->SetPosition(
				Pair.InstanceBounds[UIFACE_Left], Pair.InstanceBounds[UIFACE_Top],
				Pair.InstanceBounds[UIFACE_Right], Pair.InstanceBounds[UIFACE_Bottom],
				EVALPOS_PixelViewport, TRUE
				);
		}

		// now cleanup the prefab instance.
		ArchetypeToInstanceMap.Empty();
		PI_Bytes.Empty();
		PI_CompleteObjects.Empty();
		PI_ReferencedObjects.Empty();
		PI_SavedNames.Empty();
		PI_ObjectMap.Empty();
		SourcePrefab = NULL;
	}
}

// EOL




