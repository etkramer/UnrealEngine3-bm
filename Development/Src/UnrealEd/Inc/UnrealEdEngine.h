/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** Used during asset renaming/duplication to specify class-specific package/group targets. */
struct FClassMoveInfo
{
public:
	/** The type of asset this MoveInfo applies to. */
	FStringNoInit ClassName;
	/** The target package info which assets of this type are moved/duplicated. */
	FStringNoInit PackageName;
	/** The target group info which assets of this type are moved/duplicated. */
	FStringNoInit GroupName;
	/** If TRUE, this info is applied when moving/duplicating assets. */
	BITFIELD bActive:1;

	FClassMoveInfo() {}
    FClassMoveInfo(EEventParm)
    {
        appMemzero(this, sizeof(FClassMoveInfo));
    }
};

/** Used for copy/pasting sockets between skeletal meshes. */
struct FSkelSocketCopyInfo
{
	FName			SocketName;
	FName			BoneName;
	FVector			RelativeLocation;
	FRotator		RelativeRotation;
	FVector			RelativeScale;
};

class UUnrealEdEngine : public UEditorEngine, public FNotifyHook
{
	DECLARE_CLASS(UUnrealEdEngine,UEditorEngine,CLASS_Transient|CLASS_Config|CLASS_NoExport,UnrealEd)

protected:
	/** Global instance of the editor options class. */
	UUnrealEdOptions* EditorOptionsInst;

	/**
	 * Manager responsible for creating and configuring browser windows
	 */
	UBrowserManager* BrowserManager;
	/** Handles all thumbnail rendering and configuration. */
	UThumbnailManager* ThumbnailManager;

public:
	/**
	 * Holds the name of the browser manager class to instantiate
	 */
	FStringNoInit BrowserManagerClassName;

	/**
	 * Holds the name of the class to instantiate
	 */
	FStringNoInit ThumbnailManagerClassName;

	/** The current autosave number, appended to the autosave map name, wraps after 10 */
	INT				AutoSaveIndex;
	/** The number of seconds since the last autosave. */
	FLOAT			AutosaveCount;
	/** Is autosaving enabled? */
	BITFIELD		AutoSave:1;
	/** How often to save out to disk */
	INT				AutosaveTimeMinutes;

	/** A buffer for implementing material expression copy/paste. */
	UMaterial*		MaterialCopyPasteBuffer;

	/** A buffer for implementing matinee track/group copy/paste. */
	UObject*		MatineeCopyPasteBuffer;

	/** A buffer for implementing socket copy/paste. */
	TArray<FSkelSocketCopyInfo>	SkelSocketPasteBuffer;

	/** Global list of instanced animation compression algorithms. */
	TArray<class UAnimationCompressionAlgorithm*>	AnimationCompressionAlgorithms;

	/** Array of packages to be fully loaded at Editor startup. */
	TArrayNoInit<FString>	PackagesToBeFullyLoadedAtStartup;

	/** class names of Kismet objects to hide in the menus (i.e. because they aren't applicable for this game) */
	TArray<FName> HiddenKismetClassNames;

	/** Used during asset renaming/duplication to specify class-specific package/group targets. */
	TArray<FClassMoveInfo>	ClassRelocationInfo;

	// FNotify interface.
	void NotifyDestroy( void* Src );
	void NotifyPreChange( void* Src, UProperty* PropertyAboutToChange );
	void NotifyPostChange( void* Src, UProperty* PropertyThatChanged );
	void NotifyExec( void* Src, const TCHAR* Cmd );

	// Selection.
	virtual void SelectActor(AActor* Actor, UBOOL bSelect, FViewportClient* InViewportClient, UBOOL bNotify, UBOOL bSelectEvenIfHidden=FALSE);

	/**
	 * Selects or deselects a BSP surface in the persistent level's UModel.  Does nothing if GEdSelectionLock is TRUE.
	 *
	 * @param	InModel					The model of the surface to select.
	 * @param	iSurf					The index of the surface in the persistent level's UModel to select/deselect.
	 * @param	bSelected				If TRUE, select the surface; if FALSE, deselect the surface.
	 * @param	bNoteSelectionChange	If TRUE, call NoteSelectionChange().
	 */
	virtual void SelectBSPSurf(UModel* InModel, INT iSurf, UBOOL bSelected, UBOOL bNoteSelectionChange);

	/**
	 * Deselect all actors.  Does nothing if GEdSelectionLock is TRUE.
	 *
	 * @param	bNoteSelectionChange		If TRUE, call NoteSelectionChange().
	 * @param	bDeselectBSPSurfs			If TRUE, also deselect all BSP surfaces.
	 */
	virtual void SelectNone(UBOOL bNoteSelectionChange, UBOOL bDeselectBSPSurfs);

	/**
	* Updates the mouse position status bar field.
	*
	* @param PositionType	Mouse position type, used to decide how to generate the status bar string.
	* @param Position		Position vector, has the values we need to generate the string.  These values are dependent on the position type.
	*/
	virtual void UpdateMousePositionText( EMousePositionType PositionType, const FVector &Position );


	// General functions.
	virtual void UpdatePropertyWindows();

	virtual void NoteSelectionChange();
	virtual void NoteActorMovement();
	virtual void FinishAllSnaps();

	/**
	 * Cleans up after major events like e.g. map changes.
	 *
	 * @param	ClearSelection	Whether to clear selection
	 * @param	Redraw			Whether to redraw viewports
	 * @param	TransReset		Human readable reason for resetting the transaction system
	 */
	virtual void Cleanse( UBOOL ClearSelection, UBOOL Redraw, const TCHAR* TransReset );

	/**
	 * Replaces the specified actor with a new actor of the specified class.  The new actor
	 * will be selected if the current actor was selected.
	 * 
	 * @param	CurrentActor			The actor to replace.
	 * @param	NewActorClass			The class for the new actor.
	 * @param	Archetype				The template to use for the new actor.
	 * @param	bNoteSelectionChange	If TRUE, call NoteSelectionChange if the new actor was created successfully.
	 * @return							The new actor.
	 */
	virtual AActor* ReplaceActor( AActor* CurrentActor, UClass* NewActorClass, UObject* Archetype, UBOOL bNoteSelectionChange );

	/**
	 * Creates an archetype based on the parameters specified.  If PackageName or
	 * ArchetypeName are not specified, displays a prompt to the user.
	 *
	 * @param	ArchetypeBase	the object to create the archetype from
	 * @param	ArchetypeName	name for the archetype
	 * @param	PackageName		package for the archetype
	 * @param	GroupName		group for the archetype
	 *
	 * @return	a pointer to the archetype that was created
	 */
	virtual UObject* Archetype_CreateFromObject( UObject* ArchetypeBase, FString& ArchetypeName, FString& PackageName, FString& GroupName );

	virtual void Prefab_UpdateFromInstance( class APrefabInstance* Instance, TArray<UObject*>& NewObjects );

	virtual class UPrefab* Prefab_NewPrefab( TArray<UObject*> InObjects );

	virtual void Prefab_CreatePreviewImage( class UPrefab* InPrefab );
	
	virtual void Prefab_ConvertToNormalActors( class APrefabInstance* Instance );

	/**
	 * Returns whether or not the map build in progressed was cancelled by the user. 
	 */
	virtual UBOOL GetMapBuildCancelled() const;

	/**
	 * Sets the flag that states whether or not the map build was cancelled.
	 *
	 * @param InCancelled	New state for the cancelled flag.
	 */
	virtual void SetMapBuildCancelled( UBOOL InCancelled );

	/**
	*  Cancels color pick mode and resets all property windows to not handle color picking.
	*/
	virtual void CancelColorPickMode();

	/**
	 * Picks a color from the current viewport by getting the viewport buffer and then sampling a pixel at the click position.
	 *
	 * @param Viewport	Viewport to sample color from.
	 * @param ClickX	X position of pixel to sample.
	 * @param ClickY	Y position of pixel to sample.
	 *
	 */
	virtual void PickColorFromViewport(FViewport* Viewport, INT ClickX, INT ClickY);

	/**
	 * @return Returns the global instance of the editor options class.
	 */
	UUnrealEdOptions* GetUnrealEdOptions();

	// UnrealEdSrv stuff.
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
	UBOOL Exec_Edit( const TCHAR* Str, FOutputDevice& Ar );
	UBOOL Exec_Pivot( const TCHAR* Str, FOutputDevice& Ar );
	UBOOL Exec_Actor( const TCHAR* Str, FOutputDevice& Ar );
	UBOOL Exec_Mode( const TCHAR* Str, FOutputDevice& Ar );
	UBOOL Exec_SkeletalMesh( const TCHAR* Str, FOutputDevice& Ar );

	virtual FVector GetPivotLocation();
	/**
	 * Sets the editor's pivot location, and optionally the pre-pivots of actors.
	 *
	 * @param	NewPivot				The new pivot location
	 * @param	bSnapPivotToGrid		If TRUE, snap the new pivot location to the grid.
	 * @param	bAssignToPrePivot		If TRUE, assign the new pivot location to the pre-pivot of selected actors.
	 * @param	bIgnoreAxis				If TRUE, leave the existing pivot unaffected for components of NewPivot that are 0.
	 */
	virtual void SetPivot(FVector NewPivot, UBOOL bSnapPivotToGrid, UBOOL bAssignToPrePivot, UBOOL bIgnoreAxis);
	virtual void ResetPivot();

	// Editor actor virtuals from UnEdAct.cpp.
	/**
	 * Select all actors except hidden actors.
	 */
	virtual void edactSelectAll();

	/**
	 * Invert the selection of all actors.
	 */
	virtual void edactSelectInvert();

	/**
	 * Select any actors based on currently selected actors 
	 */
	virtual void edactSelectBased();

	/**
	 * Select all actors in a particular class.
	 */
	virtual void edactSelectOfClass( UClass* Class );

	/**
	 * Select all actors in a particular class and its subclasses.
	 */
	virtual void edactSelectSubclassOf( UClass* Class );
	
	/**
	 * Select all actors in a level that are marked for deletion.
	 */
	virtual void edactSelectDeleted();
	
	/**
	 * Select all actors that have the same static mesh assigned to them as the selected ones.
	 *
	 * @param bAllClasses		If TRUE, also select non-AStaticMeshActor actors whose meshes match.
	 */
	virtual void edactSelectMatchingStaticMesh(UBOOL bAllClasses);

	/**
	 * Select all actors that have the same skeletal mesh assigned to them as the selected ones.
	 *
	 * @param bAllClasses		If TRUE, also select non-ASkeletalMeshActor actors whose meshes match.
	 */
	virtual void edactSelectMatchingSkeletalMesh(UBOOL bAllClasses);

	/**
	 * Selects actors in the current level based on whether or not they're referenced by Kismet.
	 *
	 * @param	bReferenced		If TRUE, select actors referenced by Kismet; if FALSE, select unreferenced actors.
	 */
	virtual void edactSelectKismetReferencedActors(UBOOL bReferenced);

	/** Selects all actors that have the same SpeedTree assigned to them as the selected ones. */
	virtual void SelectMatchingSpeedTrees();

	/**
	 * Deletes all selected actors.  bIgnoreKismetReferenced is ignored when bVerifyDeletionCanHappen is TRUE.
	 *
	 * @param		bVerifyDeletionCanHappen	[opt] If TRUE (default), verify that deletion can be performed.
	 * @param		bIgnoreKismetReferenced		[opt] If TRUE, don't delete actors referenced by Kismet.
	 * @return									TRUE unless the delete operation was aborted.
	 */
	virtual UBOOL edactDeleteSelected(UBOOL bVerifyDeletionCanHappen=TRUE, UBOOL bIgnoreKismetReferenced=FALSE);
	
	/**
	 * Create archetypes of the selected actors, and replace those actors with instances of
	 * the archetype class.
	 */
	virtual void edactArchetypeSelected();

	/**
	 * Create a prefab from the selected actors, and replace those actors with an instance of that prefab.
	 */
	virtual void edactPrefabSelected();

	/**
	 * Add the selected prefab at the clicked location.
	 */
	virtual void edactAddPrefab();

	/**
	 * Select all Actors that make up the selected PrefabInstance.
	 */
	virtual void edactSelectPrefabActors();

	/**
	 * Update a prefab from the selected PrefabInstance.
	 */
	virtual void edactUpdatePrefabFromInstance();

	/**
	 * Reset a prefab instance from the prefab.
	 */
	virtual void edactResetInstanceFromPrefab();

	/**
	 * Convert a prefab instance back into normal actors.
	 */
	virtual void edactPrefabInstanceToNormalActors();

	/**
	 * Copy selected actors to the clipboard.  Does not copy PrefabInstance actors or parts of Prefabs.
	 *
	 * @param	bReselectPrefabActors	If TRUE, reselect any actors that were deselected prior to export as belonging to prefabs.
	 * @param	bClipPadCanBeUsed		If TRUE, the clip pad is available for use if the user is holding down SHIFT.
	 */
	virtual void edactCopySelected(UBOOL bReselectPrefabActors, UBOOL bClipPadCanBeUsed);

	/** 
	 * Duplicates selected actors.  Handles the case where you are trying to duplicate PrefabInstance actors.
	 *
	 * @param	bUseOffset		Should the actor locations be offset after they are created?
	*/
	virtual void edactDuplicateSelected(UBOOL bUseOffset);

	/**
	 * Paste selected actors from the clipboard.
	 *
	 * @param	bDuplicate			Is this a duplicate operation (as opposed to a real paste)?
	 * @param	bOffsetLocations	Should the actor locations be offset after they are created?
	 * @param	bClipPadCanBeUsed		If TRUE, the clip pad is available for use if the user is holding down SHIFT.
	 */
	virtual void edactPasteSelected(UBOOL bDuplicate, UBOOL bOffsetLocations, UBOOL bClipPadCanBeUsed);

	/**
	 * Replace all selected brushes with the default brush.
	 */
	virtual void edactReplaceSelectedBrush();

	/**
	 * Replace all selected non-brush actors with the specified class.
	 */
	virtual void edactReplaceSelectedNonBrushWithClass(UClass* Class);

	/**
	 * Replace all actors of the specified source class with actors of the destination class.
	 *
	 * @param	SrcClass	The class of actors to replace.
	 * @param	DstClass	The class to replace with.
	 */
	virtual void edactReplaceClassWithClass(UClass* SrcClass, UClass* DstClass);

	/**
	 * Align all vertices with the current grid.
	 */
	virtual void edactAlignVertices();

	/**
	 * Hide selected actors (set their bHiddenEd=true).
	 */
	virtual void edactHideSelected();

	/**
	 * Hide unselected actors (set their bHiddenEd=true).
	 */
	virtual void edactHideUnselected();

	/**
	 * UnHide selected actors (set their bHiddenEd=true).
	 */
	virtual void edactUnHideAll();

	/**
	 * Redraws all level editing viewport clients.
	 *
	 * @param	bInvalidateHitProxies		[opt] If TRUE (the default), invalidates cached hit proxies too.
	 */
	virtual void RedrawLevelEditingViewports(UBOOL bInvalidateHitProxies=TRUE);

	/**
	 * Pastes clipboard text into a clippad entry
	 */
	virtual void PasteClipboardIntoClipPad();

	// Hook replacements.
	void ShowActorProperties();
	void ShowWorldProperties();

	/**
	 * Checks to see if any worlds are dirty (that is, they need to be saved.)
	 *
	 * @return TRUE if any worlds are dirty
	 */
	UBOOL AnyWorldsAreDirty() const;

	// Misc
	void AttemptLevelAutosave();

	void SetCurrentClass( UClass* InClass );
	virtual void GetPackageList( TArray<UPackage*>* InPackages, UClass* InClass );

	/**
	 * Checks the state of the selected actors and notifies the user of any potentially unknown destructive actions which may occur as
	 * the result of deleting the selected actors.  In some cases, displays a prompt to the user to allow the user to choose whether to
	 * abort the deletion.
	 *
	 * @param	bOutIgnoreKismetReferenced	[out] Set only if it's okay to delete actors; specifies if the user wants Kismet-refernced actors not deleted.
	 * @return								FALSE to allow the selected actors to be deleted, TRUE if the selected actors should not be deleted.
	 */
	virtual UBOOL ShouldAbortActorDeletion(UBOOL& bOutIgnoreKismetReferenced) const;

	/**
	* Closes the main editor frame.
	*/ 
	virtual void CloseEditor();
	virtual void ShowUnrealEdContextMenu();
	virtual void ShowUnrealEdContextSurfaceMenu();
	virtual void ShowUnrealEdContextCoverSlotMenu(class ACoverLink *Link, FCoverSlot &Slot);

	/**
	 * If all selected actors belong to the same level, that level is made the current level.
	 */
	void MakeSelectedActorsLevelCurrent();

	/** Returns the thumbnail manager and creates it if missing */
	UThumbnailManager* GetThumbnailManager(void);

	/** Returns the browser manager and creates it if missing */
	UBrowserManager* GetBrowserManager(void);

	/**
	 * Returns the named browser, statically cast to the specified type.
	 *
	 * @param BrowserName	Browser name; the same name that would be passed to UBrowserManager::GetBrowserPane.
	 */
	template<class BrowserType>
	BrowserType* GetBrowser(const TCHAR* BrowserName)
	{
		return static_cast<BrowserType*>( GetBrowserManager()->GetBrowserPane( BrowserName ) );
	}

	/** @return Returns whether or not the user is able to autosave. */
	UBOOL CanAutosave() const;
	
	/** @return Returns whether or not autosave is going to occur within the next minute. */
	UBOOL AutoSaveSoon() const;

	/** @return Returns the amount of time until the next autosave in seconds. */
	INT GetTimeTillAutosave() const;
	/**
	 * Resets the autosave timer.
	 */
	virtual void ResetAutosaveTimer();

	/**
	 * Increases the amount of time before the next autosave
	 *
	 * @param InSeconds		Number of seconds to add to the autosave countdown timer
	 */
	virtual void BumpAutosaveTimer( const FLOAT InSeconds );

	// UObject interface.
	virtual void FinishDestroy();
	/** Serializes this object to an archive. */
	virtual void Serialize( FArchive& Ar );

	// UEngine Interface.
	virtual void Init();
	virtual void Tick(FLOAT DeltaSeconds);


	/**
	 * Creates an embedded Play In Editor viewport window (if possible)
	 *
	 * @param ViewportClient The viewport client the new viewport will be associated with
	 * @param InPlayInViewportIndex Viewport index to play in, or -1 for "don't care"
	 *
	 * @return TRUE if successful
	 */
	virtual UBOOL CreateEmbeddedPIEViewport( UGameViewportClient* ViewportClient, INT InPlayInViewportIndex );


	/**
	 * Returns whether saving the specified package is allowed
	 */
	virtual UBOOL CanSavePackage( UPackage* PackageToSave );

	/**
	 * Looks at all currently loaded packages and prompts the user to save them
	 * if their "bDirty" flag is set.
	 *
	 * @param	bShouldSaveMap				TRUE if the function should save the map first before other packages.
	 * @param	bForcePackagesToFullyLoad	If TRUE, fully load dirty packages without first prompting the user.
	 * @return								TRUE on success, FALSE on fail.
	 */
	virtual UBOOL SaveDirtyPackages(UBOOL bShouldSaveMap, UBOOL bForcePackagesToFullyLoad);

	/**
	 * Creates an editor derived from the wxInterpEd class.
	 *
	 * @return		A heap-allocated WxInterpEd instance.
	 */
	virtual WxInterpEd	*CreateInterpEditor( wxWindow* InParent, wxWindowID InID, class USeqAct_Interp* InInterp );

	/**
	 * Creates an editor derived from the WxCameraAnimEd class.
	 *
	 * @return		A heap-allocated WxCameraAnimEd instance.
	 */
	virtual WxCameraAnimEd	*CreateCameraAnimEditor( wxWindow* InParent, wxWindowID InID, class USeqAct_Interp* InInterp );

	virtual void SentinelStatRender(FEditorLevelViewportClient* ViewportClient, const FSceneView* View, FCanvas* Canvas);
	virtual void SentinelStatRender3D(const FSceneView* View,class FPrimitiveDrawInterface* PDI);
	virtual void SentinelMouseMove(FEditorLevelViewportClient* ViewportClient, INT X, INT Y);
	virtual void SentinelInputKey(FEditorLevelViewportClient* ViewportClient, FName Key,EInputEvent Event);

	friend class WxGenericBrowser;
};
