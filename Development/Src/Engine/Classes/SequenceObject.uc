class SequenceObject extends Object
    abstract
    native(Sequence);

cpptext
{
	virtual void CheckForErrors() {};

	/**
	 * Notification that this object has been connected to another sequence object via a link.  Called immediately after
	 * the designer creates a link between two sequence objects.
	 *
	 * @param	connObj		the object that this op was just connected to.
	 * @param	connIdx		the index of the connection that was created.  Depends on the type of sequence op that is being connected.
	 */
	virtual void OnConnect(USequenceObject *connObj,INT connIdx) {}

	// USequenceObject interface
	virtual void DrawSeqObj(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, INT MouseOverConnType, INT MouseOverConnIndex, FLOAT MouseOverTime) {};
	virtual void DrawLogicLinks(FCanvas* Canvas, UBOOL bCurves, TArray<USequenceObject*> &SelectedSeqObjs, USequenceObject* MouseOverSeqObj, INT MouseOverConnType, INT MouseOverConnIndex) {};
	virtual void DrawVariableLinks(FCanvas* Canvas, UBOOL bCurves, TArray<USequenceObject*> &SelectedSeqObjs, USequenceObject* MouseOverSeqObj, INT MouseOverConnType, INT MouseOverConnIndex) {};
	virtual void OnCreated()
	{
		ObjInstanceVersion = eventGetObjClassVersion();
	};
	virtual void OnDelete() {}
	virtual void OnSelected() {};

	virtual void OnExport();
	/**
	 * Called when a copy of this object is made in the editor via cut and paste
	 */
	virtual void OnPasted(){};

	virtual FIntRect GetSeqObjBoundingBox();
	void SnapPosition(INT Gridsize, INT MaxSequenceSize);
	FString GetSeqObjFullName();

	/**
	 * Traverses the ParentSequence chain until a non-sequence object is found, starting with this object.
	 *
	 * @erturn	a pointer to the first object (including this one) in the ParentSequence chain that does
	 *			has a NULL ParentSequence.
	 */
	USequence* GetRootSequence( UBOOL bOuterFallback=FALSE );
	/**
	 * Traverses the ParentSequence chain until a non-sequence object is found, starting with this object.
	 *
	 * @erturn	a pointer to the first object (including this one) in the ParentSequence chain that does
	 *			has a NULL ParentSequence.
	 */
	const USequence* GetRootSequence( UBOOL bOuterFallback=FALSE ) const;
	/**
	 * Traverses the ParentSequence chain until a non-sequence object is found, starting with this object's ParentSequence.
	 *
	 * @erturn	a pointer to the first object (not including this one) in the ParentSequence chain that does
	 *			has a NULL ParentSequence.
	 */
	USequence* GetParentSequenceRoot( UBOOL bOuterFallback=FALSE ) const;

	FIntPoint GetTitleBarSize(FCanvas* Canvas);
	FColor GetBorderColor(UBOOL bSelected, UBOOL bMouseOver);

	/** Gives op a chance to customize the title bar text.  e.g. to include important data.  Returns string to display in the title bar. */
	virtual FString GetDisplayTitle() const;
	virtual void DrawTitleBar(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, const FIntPoint& Pos, const FIntPoint& Size);

	virtual void UpdateObject()
	{
		// set the new instance version to match the class version
		const INT ObjClassVersion = eventGetObjClassVersion();
		const UBOOL bDirty = ObjInstanceVersion != ObjClassVersion;
		ObjInstanceVersion = ObjClassVersion;
		if ( bDirty )
		{
			MarkPackageDirty();
		}
	}

	virtual void DrawKismetRefs( FViewport* Viewport, const FSceneView* View, FCanvas* Canvas ) {}

	virtual void PostLoad();
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	/**
	 * Get the name of the class to use for handling user interaction events (such as mouse-clicks) with this sequence object
	 * in the kismet editor.
	 *
	 * @return	a string containing the path name of a class in an editor package which can handle user input events for this
	 *			sequence object.
	 */
	virtual const FString GetEdHelperClassName() const
	{
		return FString( TEXT("UnrealEd.SequenceObjectHelper") );
	}

	virtual UBOOL IsPendingKill() const;

	/**
	 * Provides a way for non-deletable SequenceObjects (those with bDeletable=false) to be removed programatically.  The
	 * user will not be able to remove this object from the sequence via the UI, but calls to RemoveObject will succeed.
	 */
	virtual UBOOL IsDeletable() const { return bDeletable; }

	/**
	 * Returns whether this SequenceObject can exist in a sequence without being linked to anything else (i.e. does not require
	 * another sequence object to activate it)
	 */
	virtual UBOOL IsStandalone() const { return FALSE; }

	/** called when the level that contains this sequence object is being removed/unloaded */
	virtual void CleanUp() {}

	/**
	 * Builds a list of objects which have this object in their archetype chain.
	 *
	 * All archetype propagation for sequence objects would be handled by prefab code, so this version just skips the iteration.
	 *
	 * @param	Instances	receives the list of objects which have this one in their archetype chain
	 */
	virtual void GetArchetypeInstances( TArray<UObject*>& Instances );

	/**
	 * Serializes all objects which have this object as their archetype into GMemoryArchive, then recursively calls this function
	 * on each of those objects until the full list has been processed.
	 * Called when a property value is about to be modified in an archetype object.
	 *
	 * Since archetype propagation for sequence objects is handled by the prefab code, this version simply routes the call
	 * to the owning prefab so that it can handle the propagation at the appropriate time.
	 *
	 * @param	AffectedObjects		unused
	 */
	virtual void SaveInstancesIntoPropagationArchive( TArray<UObject*>& AffectedObjects );

	/**
	 * De-serializes all objects which have this object as their archetype from the GMemoryArchive, then recursively calls this function
	 * on each of those objects until the full list has been processed.
	 *
	 * Since archetype propagation for sequence objects is handled by the prefab code, this version simply routes the call
	 * to the owning prefab so that it can handle the propagation at the appropriate time.
	 *
	 * @param	AffectedObjects		unused
	 */
	virtual void LoadInstancesFromPropagationArchive( TArray<UObject*>& AffectedObjects );

	/**
	 * Determines whether this object is contained within a UPrefab.
	 *
	 * @param	OwnerPrefab		if specified, receives a pointer to the owning prefab.
	 *
	 * @return	TRUE if this object is contained within a UPrefab; FALSE if it IS a UPrefab or isn't contained within one.
	 */
	virtual UBOOL IsAPrefabArchetype( class UObject** OwnerPrefab=NULL ) const;

	/**
	 * @return	TRUE if the object is a UPrefabInstance or part of a prefab instance.
	 */
	virtual UBOOL IsInPrefabInstance() const;

	virtual void Initialize() {}
	virtual void PrePathBuild(  AScout* Scout ) {}
	virtual void PostPathBuild( AScout* Scout ) {}
}

var const int ObjInstanceVersion;
var noimport const Sequence ParentSequence;
var editoronly int ObjPosX;
var editoronly int ObjPosY;
var editoronly string ObjName;
var editoronly string ObjCategory;
var editoronly Color ObjColor;
var Color ObjTitleColor;
var() string ObjComment;
var bool bDeletable;
var bool bDrawFirst;
var bool bDrawLast;
var() bool bOutputObjCommentToScreen;
var bool bSuppressAutoComment;
var int DrawWidth;
var int DrawHeight;

// Export USequenceObject::execScriptLog(FFrame&, void* const)
native final function ScriptLog(string LogText, optional bool bWarning = true);

// Export USequenceObject::execGetWorldInfo(FFrame&, void* const)
native final function WorldInfo GetWorldInfo();

event bool IsValidLevelSequenceObject()
{
    return true;
    //return ReturnValue;    
}

event bool IsPastingIntoLevelSequenceAllowed()
{
    return IsValidLevelSequenceObject();
    //return ReturnValue;    
}

event bool IsValidUISequenceObject(optional UIScreenObject TargetObject)
{
    return false;
    //return ReturnValue;    
}

event bool IsPastingIntoUISequenceAllowed()
{
    return IsValidUISequenceObject();
    //return ReturnValue;    
}

static event int GetObjClassVersion()
{
    return 1;
    //return ReturnValue;    
}

defaultproperties
{
    ObjTitleColor=(R=112,G=112,B=112,A=255)
    bDeletable=true
}
