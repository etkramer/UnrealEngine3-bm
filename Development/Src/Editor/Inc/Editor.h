/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __EDITOR_H__
#define __EDITOR_H__

/*-----------------------------------------------------------------------------
	Dependencies.
-----------------------------------------------------------------------------*/

#include "../../UnrealEd/Inc/XWindow.h"
#include "Engine.h"

struct FBuilderPoly
{
	TArray<INT> VertexIndices;
	INT Direction;
	FName ItemName;
	INT PolyFlags;
	FBuilderPoly()
	: VertexIndices(), Direction(0), ItemName(NAME_None)
	{}
};

#define ENUMS_ONLY 1
#include "EngineAnimClasses.h"
#undef ENUMS_ONLY

#include "EngineUserInterfaceClasses.h"
#include "EditorClasses.h"
#include "UnContentCookers.h"
#include "EditorCommandlets.h"

#define CAMERA_ROTATION_SPEED		32.0f
#define CAMERA_ZOOM_DAMPEN			200.f
#define CAMERA_ZOOM_DIV				15000.0f

#include "UnEdViewport.h"
#include "UnEdComponents.h"
#include "FEdObjectPropagator.h"

/*-----------------------------------------------------------------------------
	Forward declarations.
-----------------------------------------------------------------------------*/

class FGeomBase;
class FGeomVertex;
class FGeomEdge;
class FGeomPoly;
class FGeomObject;
class WxGeomModifiers;

/*-----------------------------------------------------------------------------
	Editor public.
-----------------------------------------------------------------------------*/

/** Max Unrealed->Editor Exec command string length. */
#define MAX_EDCMD 512

/** The editor object. */
extern class UEditorEngine* GEditor;

/** Texture alignment. */
enum ETAxis
{
    TAXIS_X                 = 0,
    TAXIS_Y                 = 1,
    TAXIS_Z                 = 2,
    TAXIS_WALLS             = 3,
    TAXIS_AUTO              = 4,
};

// Object propagation destination
const INT OPD_None				= 0;
const INT OPD_LocalStandalone	= 1;
const INT OPD_ConsoleStart		= 2; // more after this are okay, means which console in the GConsoleSupportContainer


/**
 * MapChangeEventFlags defines flags passed to CALLBACK_MapChange global events
 */
namespace MapChangeEventFlags
{
	/** MapChangeEventFlags::Type */
	typedef DWORD Type;

	/** Default flags */
	const Type Default = 0;

	/** Set when a new map is created, loaded from disk, imported, etc. */
	const Type NewMap = 1 << 0;

	/** Set when a map rebuild occurred */
	const Type MapRebuild = 1 << 1;

	/** Set when a world was destroyed (torn down) */
	const Type WorldTornDown = 1 << 2;
}


/**
 * Import the entire default properties block for the class specified
 * 
 * @param	Class		the class to import defaults for
 * @param	Text		buffer containing the text to be imported
 * @param	Warn		output device for log messages
 * @param	Depth		current nested subobject depth
 * @param	LineNumber	the starting line number for the defaultproperties block (used for log messages)
 *
 * @return	NULL if the default values couldn't be imported
 */
const TCHAR* ImportDefaultProperties(
	UClass*				Class,
	const TCHAR*		Text,
	FFeedbackContext*	Warn,
	INT					Depth,
	INT					LineNumber
	);

/**
 * Parse and import text as property values for the object specified.
 * 
 * @param	DestData			the location to import the property values to
 * @param	SourceText			pointer to a buffer containing the values that should be parsed and imported
 * @param	ObjectStruct		the struct for the data we're importing
 * @param	SubobjectRoot		the original object that ImportObjectProperties was called for.
 *								if SubobjectOuter is a subobject, corresponds to the first object in SubobjectOuter's Outer chain that is not a subobject itself.
 *								if SubobjectOuter is not a subobject, should normally be the same value as SubobjectOuter
 * @param	SubobjectOuter		the object corresponding to DestData; this is the object that will used as the outer when creating subobjects from definitions contained in SourceText
 * @param	Warn				ouptut device to use for log messages
 * @param	Depth				current nesting level
 * @param	LineNumber			used when importing defaults during script compilation for tracking which line the defaultproperties block begins on
 * @param	InstanceGraph		contains the mappings of instanced objects and components to their templates; used when recursively calling ImportObjectProperties; generally
 *								not necessary to specify a value when calling this function from other code
 *
 * @return	NULL if the default values couldn't be imported
 */
const TCHAR* ImportObjectProperties(
	BYTE*				DestData,
	const TCHAR*		SourceText,
	UStruct*			ObjectStruct,
	UObject*			SubobjectRoot,
	UObject*			SubobjectOuter,
	FFeedbackContext*	Warn,
	INT					Depth,
	INT					LineNumber = INDEX_NONE,
	FObjectInstancingGraph* InstanceGraph=NULL
	);

//
// GBuildStaticMeshCollision - Global control for building static mesh collision on import.
//

extern UBOOL GBuildStaticMeshCollision;

//
// Creating a static mesh from an array of triangles.
//
UStaticMesh* CreateStaticMesh(TArray<FStaticMeshTriangle>& Triangles,TArray<FStaticMeshElement>& Materials,UObject* Outer,FName Name);

//
// Merges SourceMesh into DestMesh.
//
void MergeStaticMesh(UStaticMesh* DestMesh, UStaticMesh* SourceMesh, const FVector& InOffset, const FRotator& InRotation, FLOAT ScaleFactor, const FVector& ScaleFactor3D);

//
// Converting models to static meshes.
//
void GetBrushTriangles(TArray<FStaticMeshTriangle>& Triangles,TArray<FStaticMeshElement>& Materials,AActor* Brush,UModel* Model);
UStaticMesh* CreateStaticMeshFromBrush(UObject* Outer,FName Name,ABrush* Brush,UModel* Model);

/**
 * Converts a static mesh to a brush.
 *
 * @param	Model					[out] The target brush.  Must be non-NULL.
 * @param	StaticMeshActor			The source static mesh.  Must be non-NULL.
 */
void CreateModelFromStaticMesh(UModel* Model,AStaticMeshActor* StaticMeshActor);

#include "UnEdModeTools.h"
#include "UnWidget.h"

#include "UnEdModes.h"
#include "UnGeom.h"					// Support for the editors "geometry mode"

/**
 * Sets GWorld to the passed in PlayWorld and sets a global flag indicating that we are playing
 * in the Editor.
 *
 * @param	PlayInEditorWorld		PlayWorld
 * @return	the original GWorld
 */
UWorld* SetPlayInEditorWorld( UWorld* PlayInEditorWorld );

/**
 * Restores GWorld to the passed in one and reset the global flag indicating whether we are a PIE
 * world or not.
 *
 * @param EditorWorld	original world being edited
 */
void RestoreEditorWorld( UWorld* EditorWorld );

/*-----------------------------------------------------------------------------
	FScan.
-----------------------------------------------------------------------------*/

typedef void (*POLY_CALLBACK)( UModel* Model, INT iSurf );

/*-----------------------------------------------------------------------------
	FConstraints.
-----------------------------------------------------------------------------*/

//
// General purpose movement/rotation constraints.
//
class FConstraints
{
public:
	// Functions.
	virtual void Snap( FVector& Point, const FVector& GridBase )=0;
	virtual void SnapScale( FVector& Point, const FVector& GridBase )=0;
	virtual void Snap( FRotator& Rotation )=0;
	virtual UBOOL Snap( FVector& Location, FVector GridBase, FRotator& Rotation )=0;
};

/*-----------------------------------------------------------------------------
	FConstraints.
-----------------------------------------------------------------------------*/

//
// General purpose movement/rotation constraints.
//
class FEditorConstraints : public FConstraints
{
public:
	enum { MAX_GRID_SIZES=11 };

	// Variables.
	BITFIELD	GridEnabled:1;				// Grid on/off.
	BITFIELD	SnapScaleEnabled:1;			// Snap Scaling to Grid on/off.
	BITFIELD	SnapVertices:1;				// Snap to nearest vertex within SnapDist, if any.
	INT			ScaleGridSize;				// Integer percentage amount to snap scaling to.
	FLOAT		SnapDistance;				// Distance to check for snapping.
	FLOAT		GridSizes[MAX_GRID_SIZES];	// Movement grid size steps.
	INT			CurrentGridSz;				// Index into GridSizes.
	BITFIELD	RotGridEnabled:1;			// Rotation grid on/off.
	FRotator	RotGridSize;				// Rotation grid.

	FLOAT GetGridSize();
	void SetGridSz( INT InIndex );

	// Functions.
	virtual void Snap( FVector& Point, const FVector& GridBase );
	virtual void SnapScale( FVector& Point, const FVector& GridBase );
	virtual void Snap( FRotator& Rotation );
	virtual UBOOL Snap( FVector& Location, FVector GridBase, FRotator& Rotation );
};

/*-----------------------------------------------------------------------------
	UEditorEngine definition.
-----------------------------------------------------------------------------*/

class UEditorEngine : public UEngine, public FCallbackEventDevice
{
	DECLARE_CLASS(UEditorEngine,UEngine,CLASS_Transient|CLASS_Config|CLASS_NoExport,Editor)
	NO_DEFAULT_CONSTRUCTOR(UEditorEngine)

	// Objects.
	UModel*						TempModel;
	class UTransactor*			Trans;
	class UTextBuffer*			Results;
	TArray<class WxPropertyWindowFrame*>	ActorProperties;
	class WxPropertyWindowFrame*	LevelProperties;

	// Graphics.
	UTexture2D *Bad;
	UTexture2D *Bkgnd, *BkgndHi, *BadHighlight, *MaterialArrow, *MaterialBackdrop;

	// Audio
	USoundCue *				PreviewSoundCue;
	UAudioComponent	*		PreviewAudioComponent;

	// Static Meshes
	UStaticMesh* TexPropCube;
	UStaticMesh* TexPropSphere;
	UStaticMesh* TexPropPlane;
	UStaticMesh* TexPropCylinder;

	// Toggles.
	BITFIELD				FastRebuild:1;
	BITFIELD				Bootstrapping:1;
	BITFIELD				IsImportingT3D:1;

	// Variables.
	INT						TerrainEditBrush;
	DWORD					ClickFlags;
	UPackage*				ParentContext;
	FVector					ClickLocation;				// Where the user last clicked in the world
	FPlane					ClickPlane;
	FVector					MouseMovement;				// How far the mouse has moved since the last button was pressed down
	TArray<FEditorLevelViewportClient*> ViewportClients;

	/** Distance to far clipping plane for perspective viewports.  If <= NEAR_CLIPPING_PLANE, far plane is at infinity. */
	FLOAT					FarClippingPlane;

	// Setting for the detail mode to show in the editor viewports
	EDetailMode				DetailMode;

	// Constraints.
	FEditorConstraints		Constraints;

	// Advanced.
	BITFIELD				UseSizingBox:1;		// Shows sizing information in the top left corner of the viewports
	BITFIELD				UseAxisIndicator:1;	// Displays an axis indictor in the bottom left corner of the viewports
	FLOAT					FOVAngle;
	BITFIELD				GodMode:1;
	/** The location to autosave to. */
	FStringNoInit			AutoSaveDir;
	BITFIELD				InvertwidgetZAxis;
	FStringNoInit			GameCommandLine;
	/** the list of package names to compile when building scripts */
	TArrayNoInit<FString>	EditPackages;
	/** the base directory to use for finding .uc files to compile*/
	FStringNoInit			EditPackagesInPath;
	/** the directory to save compiled .u files to */
	FStringNoInit			EditPackagesOutPath;
	/** the directory to save compiled .u files to when script is compiled with the -FINAL_RELEASE switch */
	FStringNoInit			FRScriptOutputPath;
	/** If TRUE, always show the terrain in the overhead 2D view. */
	BITFIELD				AlwaysShowTerrain:1;
	/** If TRUE, use the gizmo for rotating actors. */
	BITFIELD				UseActorRotationGizmo:1;
	/** If TRUE, show translucent marker polygons on the builder brush and volumes. */
	BITFIELD				bShowBrushMarkerPolys:1;
	/** If TRUE, use Maya camera controls. */
	BITFIELD				bUseMayaCameraControls:1;
	/** If TRUE, parts of prefabs cannot be individually selected/edited. */
	BITFIELD				bPrefabsLocked:1;
	/** If TRUE, socket snapping is enabled in the main level viewports. */
	BITFIELD				bEnableSocketSnapping:1;
	
	FString					HeightMapExportClassName;

	/** Array of actor factories created at editor startup and used by context menu etc. */
	TArray<UActorFactory*>	ActorFactories;

	/** The name of the file currently being opened in the editor. "" if no file is being opened.										*/
	FStringNoInit			UserOpenedFile;

	/** Additional per-user/per-game options set in the .ini file. Should be in the form "?option1=X?option2?option3=Y"					*/
	FStringNoInit			InEditorGameURLOptions;
	/** A pointer to a UWorld that is the duplicated/saved-loaded to be played in with "Play From Here" 								*/
	UWorld*					PlayWorld;
	/** An optional location for the starting location for "Play From Here"																*/
	FVector					PlayWorldLocation;
	/** An optional rotation for the starting location for "Play From Here"																*/
	FRotator				PlayWorldRotation;
	/** Has a request for "Play From Here" been made?													 								*/
	BITFIELD				bIsPlayWorldQueued:1;
	/** Did the request include the optional location and rotation?										 								*/
	BITFIELD				bHasPlayWorldPlacement:1;
	/** Cache of the world package's dirty flag, so that it can be restored after Play Form Here. */
	BITFIELD				bWorldPackageWasDirty:1;
	/** Where did the person want to play? Where to play the game - -1 means in editor, 0 or more is an index into the GConsoleSupportContainer	*/
	INT						PlayWorldDestination;

	/** When set to anything other than -1, indicates a specific In-Editor viewport index that PIE should use */
	INT						PlayInEditorViewportIndex;

	/** The pointer to the propagator to use for sending to the PIE window																*/
	FObjectPropagator*		InEditorPropagator;
	/** The pointer to the propagator to use for sending to a remote target (console or standalone game on this PC						*/
	FObjectPropagator*		RemotePropagator;

	/** Are we currently pushing the perspective view to the object propagator? */
	BITFIELD				bIsPushingView:1;

	/** Are we currently handling color picking? */
	BITFIELD				bColorPickModeEnabled:1;

	/** Issued by code requesting that decals be reattached. */
	BITFIELD				bDecalUpdateRequested:1;

	/** Temporary render target that can be used by the editor. */
	UTextureRenderTarget2D*	ScratchRenderTarget;

	/** Display StreamingBounds for textures */
	UTexture2D*				StreamingBoundsTexture;

	/** Global instance of the editor user settings class. */
	UEditorUserSettings* UserSettings;

	// Constructor.
	void StaticConstructor();

	// UObject interface.
	virtual void FinishDestroy();
	/** Serializes this object to an archive. */
	virtual void Serialize( FArchive& Ar );

	// UEngine interface.
	virtual void Init();

	/**
	 * Issued by code requesting that decals be reattached.
	 */
	virtual void IssueDecalUpdateRequest();

	/**
	 * Initializes the Editor.
	 */
	void InitEditor();
	
	/**
	 * Constructs a default cube builder brush, this function MUST be called at the AFTER UEditorEngine::Init in order to guarantee builder brush and other required subsystems exist.
	 */
	void InitBuilderBrush();

	virtual void Tick( FLOAT DeltaSeconds );
	void SetClientTravel( const TCHAR* NextURL, ETravelType TravelType ) {}

	/** Get some viewport. Will be GameViewport in game, and one of the editor viewport windows in editor. */
	virtual FViewport* GetAViewport();

	/** Returns the global instance of the editor user settings class. */
	const UEditorUserSettings& GetUserSettings()
	{
		if( UserSettings == NULL )
		{
			UserSettings = ConstructObject<UEditorUserSettings>( UEditorUserSettings::StaticClass() );
		}

		check( UserSettings != NULL );
		return *UserSettings;
	}

	/** Returns the global instance of the editor user settings class. */
	UEditorUserSettings& AccessUserSettings()
	{
		if( UserSettings == NULL )
		{
			UserSettings = ConstructObject<UEditorUserSettings>( UEditorUserSettings::StaticClass() );
		}

		check( UserSettings != NULL );
		return *UserSettings;
	}

	/** Saves the user settings to disk. */
	void SaveUserSettings()
	{
		AccessUserSettings().SaveConfig();
	}


	/**
	 * Callback for when a editor property changed.
	 *
	 * @param	PropertyThatChanged	Property that changed and initiated the callback.
	 */
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	/** Used for generating status bar text */
	enum EMousePositionType
	{
		MP_None,
		MP_WorldspacePosition,
		MP_Translate,
		MP_Rotate,
		MP_Scale,
		MP_CameraSpeed,
		MP_NoChange
	};

	/**
	* Updates the mouse position status bar field.
	*
	* @param PositionType	Mouse position type, used to decide how to generate the status bar string.
	* @param Position		Position vector, has the values we need to generate the string.  These values are dependent on the position type.
	*/
	virtual void UpdateMousePositionText( EMousePositionType PositionType, const FVector &Position ) { check(0); }

	/**
	 * Returns whether or not the map build in progressed was cancelled by the user. 
	 */
	virtual UBOOL GetMapBuildCancelled() const
	{
		return FALSE;
	}

	/**
	 * Sets the flag that states whether or not the map build was cancelled.
	 *
	 * @param InCancelled	New state for the cancelled flag.
	 */
	virtual void SetMapBuildCancelled( UBOOL InCancelled )
	{
		// Intentionally empty.
	}

	/**
	 * Returns whether or not the actor passed in should draw as wireframe.
	 *
	 * @param InActor	Actor that is being drawn.
	 */
	virtual UBOOL ShouldDrawBrushWireframe( AActor* InActor );

	// Color picking mode

	/**
	* Picks a color from the current viewport by getting the viewport buffer and then sampling a pixel at the click position.
	*
	* @param Viewport	Viewport to sample color from.
	* @param ClickX	X position of pixel to sample.
	* @param ClickY	Y position of pixel to sample.
	*
	*/
	virtual void PickColorFromViewport(FViewport* Viewport, INT ClickX, INT ClickY)
	{
		// Intentionally empty.
	}

	/**
	 *  Cancels color pick mode and resets all property windows to not handle color picking.
	 */
	virtual void CancelColorPickMode()
	{
		// Intentionally empty.
	}

	/**
	* Sets whether color picking is enabled.
	*/
	void SetColorPickModeEnabled( UBOOL EnableColorPick )
	{
		bColorPickModeEnabled = EnableColorPick;
	}

	/**
	* Returns whether color picking is enabled.
	*/
	UBOOL GetColorPickModeEnabled() const
	{
		return bColorPickModeEnabled;
	}

	// UnEdSrv.cpp
	virtual UBOOL SafeExec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
	UBOOL Exec_StaticMesh( const TCHAR* Str, FOutputDevice& Ar );
	UBOOL Exec_Brush( const TCHAR* Str, FOutputDevice& Ar );
	UBOOL Exec_Paths( const TCHAR* Str, FOutputDevice& Ar );
	UBOOL Exec_Poly( const TCHAR* Str, FOutputDevice& Ar );
	UBOOL Exec_Obj( const TCHAR* Str, FOutputDevice& Ar );
	UBOOL Exec_Class( const TCHAR* Str, FOutputDevice& Ar );
	UBOOL Exec_Camera( const TCHAR* Str, FOutputDevice& Ar );
	UBOOL Exec_Transaction(const TCHAR* Str, FOutputDevice& Ar);

	UBOOL BakeAnimSetsInLevel(ULevel* InLevel);
	UBOOL PruneAnimSetsInLevel(ULevel* InLevel);

	/**
	 * Executes each line of text in a file sequentially, as if each were a separate command
	 *
	 * @param InFilename The name of the file to load and execute
	 * @param Ar Output device
	 */
	void ExecFile( const TCHAR* InFilename, FOutputDevice& Ar );

	// Transaction interfaces.
	INT BeginTransaction(const TCHAR* SessionName);
	INT EndTransaction();
	void ResetTransaction(const TCHAR* Action);
	void CancelTransaction(INT Index);
	UBOOL UndoTransaction();
	UBOOL RedoTransaction();
	UBOOL IsTransactionActive();


	/**
	 * Rebuilds the map.
	 *
	 * @param bBuildAllVisibleMaps	Whether or not to rebuild all visible maps, if FALSE only the current level will be built.
	 */
	void RebuildMap(UBOOL bBuildAllVisibleMaps);

	/**
	 * @return	A pointer to the named actor or NULL if not found.
	 */
	AActor* SelectNamedActor(const TCHAR *TargetActorName);

	/**
	 * Moves all viewport cameras to the target actor.
	 * @param	Actor					Target actor.
	 * @param	bActiveViewportOnly		If TRUE, move/reorient only the active viewport.
	 */
	void MoveViewportCamerasToActor(AActor& Actor,  UBOOL bActiveViewportOnly);

	/** 
	 * Moves an actor to the floor.  Optionally will align with the trace normal.
	 * @param InActor			Actor to move to the floor.
	 * @param InAlign			Whether or not to rotate the actor to align with the trace normal.
	 * @return					Whether or not the actor was moved.
	 */
	UBOOL MoveActorToFloor( AActor* InActor, UBOOL InAlign );

	/**
	* Moves all viewport cameras to focus on the provided array of actors.
	* @param	Actors					Target actors.
	* @param	bActiveViewportOnly		If TRUE, move/reorient only the active viewport.
	*/
	void MoveViewportCamerasToActor(const TArray<AActor*> &Actors, UBOOL bActiveViewportOnly);

	/**
	 * Snaps the view of the camera to that of the provided actor.
	 *
	 * @param	Actor	The actor the camera is going to be snapped to.
	 */
	void SnapViewToActor(const AActor &Actor);


	// Pivot handling.
	virtual FVector GetPivotLocation() { return FVector(0,0,0); }
	/** Sets the editor's pivot location, and optionally the pre-pivots of actors.
	 *
	 * @param	NewPivot				The new pivot location
	 * @param	bSnapPivotToGrid		If TRUE, snap the new pivot location to the grid.
	 * @param	bAssignToPrePivot		If TRUE, assign the new pivot location to the pre-pivot of selected actors.
	 * @param	bIgnoreAxis				If TRUE, leave the existing pivot unaffected for components of NewPivot that are 0.
	 */
	virtual void SetPivot(FVector NewPivot, UBOOL bSnapPivotToGrid, UBOOL bAssignToPrePivot, UBOOL bIgnoreAxis) {}
	virtual void ResetPivot() {}


	// General functions.
	/**
	 * Cleans up after major events like e.g. map changes.
	 *
	 * @param	ClearSelection	Whether to clear selection
	 * @param	Redraw			Whether to redraw viewports
	 * @param	TransReset		Human readable reason for resetting the transaction system
	 */
	virtual void Cleanse( UBOOL ClearSelection, UBOOL Redraw, const TCHAR* TransReset );
	virtual void FinishAllSnaps() { }

	/**
	 * Redraws all level editing viewport clients.
	 *
	 * @param	bInvalidateHitProxies		[opt] If TRUE (the default), invalidates cached hit proxies too.
	 */
	virtual void RedrawLevelEditingViewports(UBOOL bInvalidateHitProxies=TRUE) {}
	virtual void NoteSelectionChange() { check(0); }

	/**
	 * Adds an actor to the world at the specified location.
	 *
	 * @param	Class		A non-abstract, non-transient, placeable class.  Must be non-NULL.
	 * @param	Location	The world-space location to spawn the actor.
	 * @param	bSilent		If TRUE, suppress logging (optional, defaults to FALSE).
	 * @result				A pointer to the newly added actor, or NULL if add failed.
	 */
	virtual AActor* AddActor(UClass* Class, const FVector& Location, UBOOL bSilent = FALSE);

	virtual void NoteActorMovement() { check(0); }
	virtual UTransactor* CreateTrans();

	/** 
	 * Returns an audio component linked to the current scene that it is safe to play a sound on
	 *
	 * @param	SoundCue	A sound cue to attach to the audio component
	 * @param	SoundNode	A sound node that is attached to the audio component when the sound cue is NULL
	 */
	UAudioComponent* GetPreviewAudioComponent( USoundCue* SoundCue, USoundNode* SoundNode );

	/** 
	 * Stop any sounds playing on the preview audio component and allowed it to be garbage collected
	 */
	void ClearPreviewAudioComponents( void );

	/**
	 * Redraws all editor viewport clients.
	 *
	 * @param	bInvalidateHitProxies		[opt] If TRUE (the default), invalidates cached hit proxies too.
	 */
	void RedrawAllViewports(UBOOL bInvalidateHitProxies=TRUE);

	/**
	 * Invalidates all viewports parented to the specified view.
	 *
	 * @param	InParentView				The parent view whose child views should be invalidated.
	 * @param	bInvalidateHitProxies		[opt] If TRUE (the default), invalidates cached hit proxies too.
	 */
	void InvalidateChildViewports(FSceneViewStateInterface* InParentView, UBOOL bInvalidateHitProxies=TRUE);

	/**
	 * Looks for an appropriate actor factory for the specified UClass.
	 *
	 * @param	InClass		The class to find the factory for.
	 * @return				A pointer to the factory to use.  NULL if no factories support this class.
	 */
	UActorFactory* FindActorFactory( const UClass* InClass );

	/**
	 * Uses the supplied factory to create an actor at the clicked location ands add to level.
	 *
	 * @param	Factory					The factor to create the actor from.  Must be non-NULL.
	 * @param	bIgnoreCanCreate		[opt] If TRUE, don't call UActorFactory::CanCreateActor.  Default is FALSE.
	 * @param	bUseSurfaceOrientation	[opt] If TRUE, align new actor's orientation to the underlying surface normal.  Default is FALSE.
	 * @return							A pointer to the new actor, or NULL on fail.
	 */
	AActor* UseActorFactory( UActorFactory* Factory, UBOOL bIgnoreCanCreate=FALSE, UBOOL bUseSurfaceOrientation=FALSE ); 

	/** replaces the selected Actors with the same number of a different kind of Actor
	 * if a Factory is specified, it is used to spawn the requested Actors, otherwise NewActorClass is used (one or the other must be specified)
	 * note that only Location, Rotation, Tag, and Group are copied from the old Actors
	 * @param Factory - the Factory to use to create Actors
	 */
	void ReplaceSelectedActors(UActorFactory* Factory, UClass* NewActorClass);

	/**
	 * Copy selected actors to the clipboard.  Does not copy PrefabInstance actors or parts of Prefabs.
	 *
	 * @param	bReselectPrefabActors	If TRUE, reselect any actors that were deselected prior to export as belonging to prefabs.
	 * @param	bClipPadCanBeUsed		If TRUE, the clip pad is available for use if the user is holding down SHIFT.
	 */
	virtual void edactCopySelected(UBOOL bReselectPrefabActors, UBOOL bClipPadCanBeUsed) {}

	/**
	 * Paste selected actors from the clipboard.
	 *
	 * @param	bDuplicate			Is this a duplicate operation (as opposed to a real paste)?
	 * @param	bOffsetLocations	Should the actor locations be offset after they are created?
	 * @param	bClipPadCanBeUsed		If TRUE, the clip pad is available for use if the user is holding down SHIFT.
	 */
	virtual void edactPasteSelected(UBOOL bDuplicate, UBOOL bOffsetLocations, UBOOL bClipPadCanBeUsed) {}

	/** 
	 * Duplicates selected actors.  Handles the case where you are trying to duplicate PrefabInstance actors.
	 *
	 * @param	bUseOffset		Should the actor locations be offset after they are created?
	 */
	virtual void edactDuplicateSelected(UBOOL bOffsetLocations) {}

	/**
	 * Deletes all selected actors.  bIgnoreKismetReferenced is ignored when bVerifyDeletionCanHappen is TRUE.
	 *
	 * @param		bVerifyDeletionCanHappen	[opt] If TRUE (default), verify that deletion can be performed.
	 * @param		bIgnoreKismetReferenced		[opt] If TRUE, don't delete actors referenced by Kismet.
	 * @return									TRUE unless the delete operation was aborted.
	 */
	virtual UBOOL edactDeleteSelected(UBOOL bVerifyDeletionCanHappen=TRUE, UBOOL bIgnoreKismetReferenced=FALSE) { return TRUE; }

	/**
	 * Checks the state of the selected actors and notifies the user of any potentially unknown destructive actions which may occur as
	 * the result of deleting the selected actors.  In some cases, displays a prompt to the user to allow the user to choose whether to
	 * abort the deletion.
	 *
	 * @param	bOutIgnoreKismetReferenced	[out] Set only if it's okay to delete actors; specifies if the user wants Kismet-refernced actors not deleted.
	 * @return								FALSE to allow the selected actors to be deleted, TRUE if the selected actors should not be deleted.
	 */
	virtual UBOOL ShouldAbortActorDeletion(UBOOL& bOutIgnoreKismetReferenced) const { return FALSE; }

	/**
	 * Create archetypes of the selected actors, and replace those actors with instances of
	 * the archetype class.
	 */
	virtual void edactArchetypeSelected() {};

	/** Create a prefab from the selected actors, and replace those actors with an instance of that prefab. */
	virtual void edactPrefabSelected() {};

	/** Add the selected prefab at the clicked location. */
	virtual void edactAddPrefab() {};

	/** Select all Actors that make up the selected PrefabInstance. */
	virtual void edactSelectPrefabActors() {};

	// Editor CSG virtuals from UnEdCsg.cpp.
	virtual void csgRebuild();

	// Editor EdPoly/BspSurf assocation virtuals from UnEdCsg.cpp.
	virtual INT polyFindMaster( UModel* Model, INT iSurf, FPoly& Poly );
	virtual void polyUpdateMaster( UModel* Model, INT iSurf, INT UpdateTexCoords );
	virtual void polyGetLinkedPolys( ABrush* InBrush, FPoly* InPoly, TArray<FPoly>* InPolyList );
	virtual void polyGetOuterEdgeList( TArray<FPoly>* InPolyList, TArray<FEdge>* InEdgeList );
	virtual void polySplitOverlappingEdges( TArray<FPoly>* InPolyList, TArray<FPoly>* InResult );

	// Bsp Poly search virtuals from UnEdCsg.cpp.
	virtual void polySetAndClearPolyFlags( UModel* Model, DWORD SetBits, DWORD ClearBits, INT SelectedOnly, INT UpdateMaster );

	// Selection.
	virtual void SelectActor(AActor* Actor, UBOOL InSelected, FViewportClient* InViewportClient, UBOOL bNotify, UBOOL bSelectEvenIfHidden=FALSE) {}

	/**
	 * Selects or deselects a BSP surface in the persistent level's UModel.  Does nothing if GEdSelectionLock is TRUE.
	 *
	 * @param	InModel					The model of the surface to select.
	 * @param	iSurf					The index of the surface in the persistent level's UModel to select/deselect.
	 * @param	bSelected				If TRUE, select the surface; if FALSE, deselect the surface.
	 * @param	bNoteSelectionChange	If TRUE, call NoteSelectionChange().
	 */
	virtual void SelectBSPSurf(UModel* InModel, INT iSurf, UBOOL bSelected, UBOOL bNoteSelectionChange) {}

	/**
	 * Deselect all actors.  Does nothing if GEdSelectionLock is TRUE.
	 *
	 * @param	bNoteSelectionChange		If TRUE, call NoteSelectionChange().
	 * @param	bDeselectBSPSurfs			If TRUE, also deselect all BSP surfaces.
	 */
	virtual void SelectNone(UBOOL bNoteSelectionChange, UBOOL bDeselectBSPSurfs) {}

	// Bsp Poly selection virtuals from UnEdCsg.cpp.
	virtual void polySelectAll ( UModel* Model );
	virtual void polySelectMatchingGroups( UModel* Model );
	virtual void polySelectMatchingItems( UModel* Model );
	virtual void polySelectCoplanars( UModel* Model );
	virtual void polySelectAdjacents( UModel* Model );
	virtual void polySelectAdjacentWalls( UModel* Model );
	virtual void polySelectAdjacentFloors( UModel* Model );
	virtual void polySelectAdjacentSlants( UModel* Model );
	virtual void polySelectMatchingBrush( UModel* Model );

	/**
	 * Selects surfaces whose material matches that of any selected surfaces.
	 *
	 * @param	bCurrentLevelOnly		If TRUE, select
	 */
	virtual void polySelectMatchingMaterial(UBOOL bCurrentLevelOnly);

	virtual void polySelectReverse( UModel* Model );
	virtual void polyMemorizeSet( UModel* Model );
	virtual void polyRememberSet( UModel* Model );
	virtual void polyXorSet( UModel* Model );
	virtual void polyUnionSet( UModel* Model );
	virtual void polyIntersectSet( UModel* Model );
	virtual void polySelectZone( UModel *Model );

	// Poly texturing virtuals from UnEdCsg.cpp.
	virtual void polyTexPan( UModel* Model, INT PanU, INT PanV, INT Absolute );
	virtual void polyTexScale( UModel* Model,FLOAT UU, FLOAT UV, FLOAT VU, FLOAT VV, UBOOL Absolute );

	// Map brush selection virtuals from UnEdCsg.cpp.
	virtual void mapSelectOperation( ECsgOper CSGOper );
	virtual void mapSelectFlags( DWORD Flags );
	virtual void mapBrushGet();
	virtual void mapBrushPut();
	virtual void mapSendToFirst();
	virtual void mapSendToLast();
	virtual void mapSendToSwap();
	virtual void mapSetBrush(enum EMapSetBrushFlags PropertiesMask, WORD BrushColor, FName Group, DWORD SetPolyFlags, DWORD ClearPolyFlags, DWORD CSGOper, INT DrawType );

	// Bsp virtuals from UnBsp.cpp.
	virtual void bspRepartition( INT iNode, UBOOL Simple );
	virtual INT bspNodeToFPoly( UModel* Model, INT iNode, FPoly* EdPoly );
	virtual void bspCleanup( UModel* Model );
	virtual void bspBuildFPolys( UModel* Model, UBOOL SurfLinks, INT iNode );
	virtual void bspMergeCoplanars( UModel* Model, UBOOL RemapLinks, UBOOL MergeDisparateTextures );
	/**
	 * Performs any CSG operation between the brush and the world.
	 *
	 * @param	Actor							The brush actor to apply.
	 * @param	Model							The model to apply the CSG operation to; typically the world's model.
	 * @param	PolyFlags						PolyFlags to set on brush's polys.
	 * @param	CSGOper							The CSG operation to perform.
	 * @param	bBuildBounds					If TRUE, updates bounding volumes on Model for CSG_Add or CSG_Subtract operations.
	 * @param	bMergePolys						If TRUE, coplanar polygons are merged for CSG_Intersect or CSG_Deintersect operations.
	 * @param	bReplaceNULLMaterialRefs		If TRUE, replace NULL material references with a reference to the GB-selected material.
	 * @return									0 if nothing happened, 1 if the operation was error-free, or 1+N if N CSG errors occurred.
	 */
	virtual INT bspBrushCSG( ABrush* Actor, UModel* Model, DWORD PolyFlags, ECsgOper CSGOper, UBOOL bBuildBounds, UBOOL bMergePolys, UBOOL bReplaceNULLMaterialRefs );
	virtual void bspOptGeom( UModel* Model );

	/**
	 * Builds lighting information depending on passed in options.
	 *
	 * @param	Options		Options determining on what and how lighting is built
	 */
	void BuildLighting(const class FLightingBuildOptions& Options);

	/**
	 * Builds fluid surface data
	 */
	void BuildFluidSurfaces();

	/**
	 * Assembles a list of worlds whose PIE packages need resaving.
	 *
	 * @param	FilenamePrefix				The PIE filename prefix.
	 * @param	OutWorldsNeedingPIESave		[out] The list worlds that need to be saved for PIE.
	 */
	void GetWorldsNeedingPIESave(const TCHAR* FilenamePrefix, TArray<UWorld*>& OutWorldsNeedingPIESave) const;

	/**
	 * Creates a fully qualified PIE package file name, given an original package file name
	 */
	FString MakePIEFileName( const TCHAR* FilenamePrefix, const FFilename& PackageFileName ) const;

	/**
	 * Checks to see if we need to delete any PIE files from disk, usually because we're closing a map (or shutting
	 * down) and the in memory map data has been modified since PIE packages were generated last
	 */
	void PurgePIEDataForDirtyPackagesIfNeeded();


	/**
	 * Open a PSA file with the given name, and import each sequence into the supplied AnimSet.
	 * This is only possible if each track expected in the AnimSet is found in the target file. If not the case, a warning is given and import is aborted.
	 * If AnimSet is empty (ie. TrackBoneNames is empty) then we use this PSA file to form the track names array.
	 */
	static void ImportPSAIntoAnimSet( class UAnimSet* AnimSet, const TCHAR* Filename, class USkeletalMesh* FillInMesh );

	/**
	 * Open a COLLADA file with the given name, and import each sequence into the supplied AnimSet.
	 * This is only possible if each track expected in the AnimSet is found in the target file. If not the case, a warning is given and import is aborted.
	 * If AnimSet is empty (ie. TrackBoneNames is empty) then we use this COLLADA file to form the track names array.
	 */
	static void ImportCOLLADAANIMIntoAnimSet( UAnimSet* AnimSet, const TCHAR* Filename, class USkeletalMesh* FillInMesh );

	// Visibility.
	virtual void TestVisibility( UModel* Model, int A, int B );

	// Scripts.
	virtual UBOOL MakeScripts( UClass* BaseClass, FFeedbackContext* Warn, UBOOL MakeAll, UBOOL Booting, UBOOL MakeSubclasses, UPackage* LimitOuter = NULL, UBOOL bParseOnly=0 );

	// Object management.
	virtual void RenameObject(UObject* Object,UObject* NewOuter,const TCHAR* NewName, ERenameFlags Flags=REN_None);

	// Level management.
	void AnalyzeLevel(ULevel* Level,FOutputDevice& Ar);

	/**
	 * Pastes clipboard text into a clippad entry
	 */
	virtual void PasteClipboardIntoClipPad() {}

	/**
	 * Removes all components from the current level's scene.
	 */
	void ClearComponents();

	/**
	 * Updates all components in the current level's scene.
	 */
	void UpdateComponents();

	/**
	 * Check for any PrefabInstances which are out of date.  For any PrefabInstances which have a TemplateVersion less than its PrefabTemplate's
	 * PrefabVersion, propagates the changes to the source Prefab to the PrefabInstance.
	 */
	void UpdatePrefabs();

	/** Util that looks for and fixes any incorrect ParentSequence pointers in Kismet objects in memory. */
	void FixKismetParentSequences();


	/**
	 * FCallbackEventDevice interface
	 */
	virtual void Send( ECallbackEventType Event );
	virtual void Send( ECallbackEventType Event, DWORD Param );


	/**
	 * Makes a request to start a play from editor session (in editor or on a remote platform)
	 * @param	StartLocation			If specified, this is the location to play from (via a Teleporter - Play From Here)
	 * @param	StartRotation			If specified, this is the rotation to start playing at
	 * @param	DestinationConsole		Where to play the game - -1 means in editor, 0 or more is an index into the GConsoleSupportContainer
	 * @param	InPlayInViewportIndex	Viewport index to play the game in, or -1 to spawn a standalone PIE window
	 */
	virtual void PlayMap( FVector* StartLocation = NULL, FRotator* StartRotation = NULL, INT DestinationConsole = -1, INT InPlayInViewportIndex = -1 );

	/**
	 * Kicks off a "Play From Here" request that was most likely made during a transaction
	 */
	virtual void StartQueuedPlayMapRequest();

	/**
	 * Saves play in editor levels and also fixes up references in AWorldInfo to other levels.
	 *
	 * @param	Prefix				Prefix used to save files to disk.
	 * @param	bSaveAllPackages	Do we save all non-map packages as well? Useful for PlayOnXenon which needs to copy updated packages to the Xenon
	 *
	 * @return	False if the save failed and the user wants to abort what they were doing
	 */
	virtual UBOOL SavePlayWorldPackages( const TCHAR* Prefix, UBOOL bSaveAllPackages );

	/**
	 * Builds a URL for game spawned by the editor (not including map name!). Has stuff like the teleporter, spectating, etc.
	 *
	 * @return	The URL for the game; does not include the map name (different per platform)
	 */
	virtual FString BuildPlayWorldURL(const TCHAR* MapName);

	/**
	 * Spawns a teleporter in the given world
	 * @param	World		The World to spawn in (for PIE this may not be GWorld)
	 * @param	Teleporter	A reference to the resulting Teleporter actor
	 *
	 * @return	If the spawn failed
	 */
	virtual UBOOL SpawnPlayFromHereTeleporter(UWorld* World, ATeleporter*& Teleporter);

	/**
	 * Starts a Play In Editor session
	 */
	virtual void PlayInEditor();

	/**
	 * Sends the level over to a platform (one of the platorms in the GConsoleSupportContainer)
	 * @param	ConsoleIndex	The index into the GConsoleSupportContainer of which platform to play on
	 */
	virtual void PlayOnConsole(INT ConsoleIndex);

	/**
	 * Kills the Play From Here session
	 */
	virtual void EndPlayMap();

	/**
	 * Creates an embedded Play In Editor viewport window (if possible)
	 *
	 * @param ViewportClient The viewport client the new viewport will be associated with
	 * @param InPlayInViewportIndex Viewport index to play in, or -1 for "don't care"
	 *
	 * @return TRUE if successful
	 */
	virtual UBOOL CreateEmbeddedPIEViewport( UGameViewportClient* ViewportClient, INT InPlayInViewportIndex )
	{
		// Override this in derived classes
		return FALSE;
	}

	/**
	 * Sets where the object propagation in the editor goes (PIE, a console, etc)
	 * @param	Destination		The enum of where to send propagations (see top of this file for values
	 */
	void SetObjectPropagationDestination(INT Destination);

	/**
	 * The support DLL may have changed the IP address to propagate, so update the IP address
	 */
	void UpdateObjectPropagationIPAddress();

	/**
	 * Disables any realtime viewports that are currently viewing the level.  This will not disable
	 * things like preview viewports in Cascade, etc. Typically called before running the game.
	 */
	void DisableRealtimeViewports();

	/**
	 * Restores any realtime viewports that have been disabled by DisableRealtimeViewports. This won't
	 * disable viewporst that were realtime when DisableRealtimeViewports has been called and got
	 * latter toggled to be realtime.
	 */
	void RestoreRealtimeViewports();

	/**
	 *	Returns pointer to a temporary render target.
	 *	If it has not already been created, does so here.
	 */
	UTextureRenderTarget2D* GetScratchRenderTarget();

	/**
	 * Resets the autosave timer.
	 */
	virtual void ResetAutosaveTimer() {}

	/**
	 * Increases the amount of time before the next autosave
	 *
	 * @param InSeconds		Number of seconds to add to the autosave countdown timer
	 */
	virtual void BumpAutosaveTimer( const FLOAT InSeconds ) { }

	/**
	 * Handles freezing/unfreezing of rendering
	 */
	virtual void ProcessToggleFreezeCommand();

	/**
	 * Handles frezing/unfreezing of streaming
	 */
	virtual void ProcessToggleFreezeStreamingCommand();

	// Editor specific

	/**
	* Closes the main editor frame.
	*/ 
	virtual void CloseEditor() {}
	virtual void ShowUnrealEdContextMenu() {}
	virtual void ShowUnrealEdContextSurfaceMenu() {}
	virtual void ShowUnrealEdContextCoverSlotMenu(class ACoverLink *Link, FCoverSlot &Slot) {}
	virtual void GetPackageList( TArray<UPackage*>* InPackages, UClass* InClass ) {}

	/**
	 *	Get/Set the ParticleSystemRealTime flag in the thumbnail manager
	 */
	virtual UBOOL GetPSysRealTimeFlag()	{ return FALSE;	}
	virtual void SetPSysRealTimeFlag(UBOOL bPSysRealTime)	{};

	/**
	 * Returns the number of currently selected actors.
	 *
	 */
	UBOOL GetSelectedActorCount() const;

	/**
	 * Returns the set of selected actors.
	 */
	class USelection* GetSelectedActors() const;

	/**
	 * Returns an FSelectionIterator that iterates over the set of selected actors.
	 */
	class FSelectionIterator GetSelectedActorIterator() const;

	/**
	 * Returns the set of selected non-actor objects.
	 */
	class USelection* GetSelectedObjects() const;

	/**
	 * Returns the appropriate selection set for the specified object class.
	 */
	class USelection* GetSelectedSet( const UClass* Class ) const;

	/**
	 * Clears out the current map, if any, and creates a new blank map.
	 *
	 * @param	bAdditiveGeom	If FALSE, create a large additive brush that surrounds the world.
	 */
	void NewMap(UBOOL bAdditiveGeom);

	/**
	 * Exports the current map to the specified filename.
	 *
	 * @param	InFilename					Filename to export the map to.
	 * @param	bExportSelectedActorsOnly	If TRUE, export only the selected actors.
	 */
	void ExportMap(const TCHAR* InFilename, UBOOL bExportSelectedActorsOnly);

	/**
	 * Iterates over objects belonging to the specified packages and reports
	 * direct references to objects in the trashcan packages.  Only looks
	 * at loaded objects.  Output is to the specified arrays -- the i'th
	 * element of OutObjects refers to the i'th element of OutTrashcanObjects.
	 *
	 * @param	Packages			Only objects in these packages are considered when looking for trashcan references.
	 * @param	OutObjects			[out] Receives objects that directly reference objects in the trashcan.
	 * @param	OutTrashcanObject	[out] Receives the list of referenced trashcan objects.
	 */
	void CheckForTrashcanReferences(const TArray<UPackage*>& SelectedPackages, TArray<UObject*>& OutObjects, TArray<UObject*>& OutTrashcanObjects);

	/**
	 * Checks loaded levels for references to objects in the trashcan and
	 * reports to the Map Check dialog.
	 */
	void CheckLoadedLevelsForTrashcanReferences();

	/**
	 * Deselects all selected prefab instances or actors belonging to prefab instances.  If a prefab
	 * instance is selected, or if all actors in the prefab are selected, record the prefab.
	 *
	 * @param	OutPrefabInstances		[out] The set of prefab instances that were selected.
	 * @param	bNotify					If TRUE, call NoteSelectionChange if any actors were deselected.
	 */
	void DeselectActorsBelongingToPrefabs(TArray<APrefabInstance*>& OutPrefabInstances, UBOOL bNotify);

	/**
	 * Moves selected actors to the current level.
	 */
	void MoveSelectedActorsToCurrentLevel();

	/**
	 * Copies selected actors to the clipboard.  Supports copying actors from multiple levels.
	 * NOTE: Doesn't support copying prefab instance actors!
	 *
	 * @param bShouldCut If TRUE, deletes the selected actors after copying them to the clipboard
	 * @param bShouldClipPadCanBeUsed If TRUE and the SHIFT key is held down, the actors will be copied to the clip pad
	 */
	void CopySelectedActorsToClipboard( UBOOL bShouldCut, UBOOL bClipPadCanBeUsed );

	/**
	 * Computes a color to use for property coloration for the given object.
	 *
	 * @param	Object		The object for which to compute a property color.
	 * @param	OutColor	[out] The returned color.
	 * @return				TRUE if a color was successfully set on OutColor, FALSE otherwise.
	 */
	virtual UBOOL GetPropertyColorationColor(class UObject* Object, FColor& OutColor);

	/**
	 * Sets property value and property chain to be used for property-based coloration.
	 *
	 * @param	PropertyValue		The property value to color.
	 * @param	Property			The property to color.
	 * @param	CommonBaseClass		The class of object to color.
	 * @param	PropertyChain		The chain of properties from member to lowest property.
	 */
	virtual void SetPropertyColorationTarget(const FString& PropertyValue, class UProperty* Property, class UClass* CommonBaseClass, class FEditPropertyChain* PropertyChain);

	/**
	 * Accessor for current property-based coloration settings.
	 *
	 * @param	OutPropertyValue	[out] The property value to color.
	 * @param	OutProperty			[out] The property to color.
	 * @param	OutCommonBaseClass	[out] The class of object to color.
	 * @param	OutPropertyChain	[out] The chain of properties from member to lowest property.
	 */
	virtual void GetPropertyColorationTarget(FString& OutPropertyValue, UProperty*& OutProperty, UClass*& OutCommonBaseClass, FEditPropertyChain*& OutPropertyChain);

	/**
	 * Selects actors that match the property coloration settings.
	 */
	void SelectByPropertyColoration();

	/**
	 *	Sets the texture to use for displaying StreamingBounds.
	 *
	 *	@param	InTexture	The source texture for displaying StreamingBounds.
	 *						Pass in NULL to disable displaying them.
	 */
	void SetStreamingBoundsTexture(UTexture2D* InTexture);

	/** 
	 *	Create a new instance of a prefab in the level. 
	 *
	 *	@param	Prefab		The prefab to create an instance of.
	 *	@param	Location	Location to create the new prefab at.
	 *	@param	Rotation	Rotation to create the new prefab at.
	 *	@return				Pointer to new PrefabInstance actor in the level, or NULL if it fails.
	 */
	class APrefabInstance* Prefab_InstancePrefab(class UPrefab* Prefab, const FVector& Location, const FRotator& Rotation) const;

	/**
	 * Warns the user of any hidden streaming levels, and prompts them with a Yes/No dialog
	 * for whether they wish to continue with the operation.  No dialog is presented if all
	 * streaming levels are visible.  The return value is TRUE if no levels are hidden or
	 * the user selects "Yes", or FALSE if the user selects "No".
	 *
	 * @param	AdditionalMessage		An additional message to include in the dialog.  Can be NULL.
	 * @return							FALSE if the user selects "No", TRUE otherwise.
	 */
	UBOOL WarnAboutHiddenLevels(const TCHAR* AdditionalMessage) const;

	void ApplyDeltaToActor(AActor* InActor, UBOOL bDelta, const FVector* InTranslation, const FRotator* InRotation, const FVector* InScaling, UBOOL bAltDown=FALSE, UBOOL bShiftDown=FALSE, UBOOL bControlDown=FALSE) const;

	/** called after script compilation to allow for game specific post-compilation steps */
	virtual void PostScriptCompile() {}

	/** Hook for sentinel stats tool to render things in viewport. */
	virtual void SentinelStatRender(FEditorLevelViewportClient* ViewportClient, const FSceneView* View, FCanvas* Canvas) {}
	/** Hook for sentinel stats tool to render things in 3D viewport. */
	virtual void SentinelStatRender3D(const FSceneView* View,class FPrimitiveDrawInterface* PDI) {}
	/** Hook for sentinel to be informed about mouse movements (for tool tip) */
	virtual void SentinelMouseMove(FEditorLevelViewportClient* ViewportClient, INT X, INT Y) {}
	/** Hook for sentinel to be informed about key input */
	virtual void SentinelInputKey(FEditorLevelViewportClient* ViewportClient, FName Key,EInputEvent Event) {}


	/** This allows the indiv game to return the set of decal materials it uses based off its internal objects (e.g. PhysicalMaterials)**/
	virtual class TArray<UMaterialInterface*> GetDecalMaterialsFromGame( UObject* InObject ) const { TArray<UMaterialInterface*> Retval; return Retval; }


private:
	//////////////////////
	// Map execs

	UBOOL Map_Rotgrid(const TCHAR* Str, FOutputDevice& Ar);
	UBOOL Map_Select(const TCHAR* Str, FOutputDevice& Ar);
	UBOOL Map_Brush(const TCHAR* Str, FOutputDevice& Ar);
	UBOOL Map_Sendto(const TCHAR* Str, FOutputDevice& Ar);
	UBOOL Map_Rebuild(const TCHAR* Str, FOutputDevice& Ar);
	UBOOL Map_Load(const TCHAR* Str, FOutputDevice& Ar);
	UBOOL Map_Import(const TCHAR* Str, FOutputDevice& Ar, UBOOL bNewMap);
	UBOOL Map_Check(const TCHAR* Str, FOutputDevice& Ar, UBOOL bCheckDeprecatedOnly, UBOOL bClearExistingMessages);
	UBOOL Map_Scale(const TCHAR* Str, FOutputDevice& Ar);
	UBOOL Map_Setbrush(const TCHAR* Str, FOutputDevice& Ar);
};

/*-----------------------------------------------------------------------------
	Parameter parsing functions.
-----------------------------------------------------------------------------*/

UBOOL GetFVECTOR( const TCHAR* Stream, const TCHAR* Match, FVector& Value );
UBOOL GetFVECTOR( const TCHAR* Stream, FVector& Value );
const TCHAR* GetFVECTORSpaceDelimited( const TCHAR* Stream, FVector& Value );
UBOOL GetFROTATOR( const TCHAR* Stream, const TCHAR* Match, FRotator& Rotation, INT ScaleFactor );
UBOOL GetFROTATOR( const TCHAR* Stream, FRotator& Rotation, int ScaleFactor );
const TCHAR* GetFROTATORSpaceDelimited( const TCHAR* Stream, FRotator& Rotation, INT ScaleFactor );
UBOOL GetBEGIN( const TCHAR** Stream, const TCHAR* Match );
UBOOL GetEND( const TCHAR** Stream, const TCHAR* Match );
UBOOL GetSUBSTRING(const TCHAR*	Stream, const TCHAR* Match, TCHAR* Value, INT MaxLen);
TCHAR* SetFVECTOR( TCHAR* Dest, const FVector* Value );

class FReimportHandler;

/** Reimport manager for package resources with associated source files on disk. */
class FReimportManager
{
protected:
	/** FReimportHandlers add themselves */
	friend class FReimportHandler;
	/** Reimport handlers registered with this manager */
	TArray<FReimportHandler*>	Handlers;
public:
	/** Constructor */
	FReimportManager();
	/** Singleton function
	* @return Singleton instance of this manager
	*/
	static FReimportManager* Instance();

	/**
	* Reimports specified resource from its source, if the meta-data exists
	* @param Package texture to reimport
	* @return TRUE if handled
	*/
	virtual UBOOL Reimport( UObject* Obj );
};

/** 
* Reimport handler for package resources with associated source files on disk.
*/
class FReimportHandler
{
public:
	/** Constructor. Add self to manager */
	FReimportHandler(){ FReimportManager::Instance()->Handlers.AddItem(this); }
	/** Destructor. Remove self from manager */
	virtual ~FReimportHandler(){ if(GConfig) FReimportManager::Instance()->Handlers.RemoveItem(this); }
	/**
	* Reimports specified resource from its source, if the meta-data exists
	* @param Package texture to reimport
	* @return TRUE if handled
	*/
	virtual UBOOL Reimport( UObject* Obj ) = 0;
};

/*-----------------------------------------------------------------------------
	Cooking helpers.
-----------------------------------------------------------------------------*/

/** 
 * Info used to setup the rows of the sound quality previewer
 */
class FPreviewInfo
{
public:
	FPreviewInfo( INT Quality );
	~FPreviewInfo( void ) { Cleanup(); }

	void Cleanup( void );

	INT			QualitySetting;

	DWORD		OriginalSize;

	DWORD		OggVorbisSize;
	SWORD*		DecompressedOggVorbis;
	DWORD		XMASize;
	DWORD		PS3Size;

	SWORD*		DecompressedXMA;
	SWORD*		DecompressedPS3;
};

/**
 * Cooks SoundNodeWave to a specific platform
 *
 * @param	SoundNodeWave			Wave file to cook
 * @param	SoundCooker				Platform specific cooker object to cook with
 * @param	DestinationData			Destination bulk data
 */
UBOOL CookSoundNodeWave( USoundNodeWave* SoundNodeWave, class FConsoleSoundCooker* SoundCooker, FByteBulkData& DestinationData );

/**
 * Cooks SoundNodeWave to all available platforms
 *
 * @param	SoundNodeWave			Wave file to cook
 * @param	Platform				Platform to cook for, PLATFORM_Unknown for all platforms
 */
UBOOL CookSoundNodeWave( USoundNodeWave* SoundNodeWave, UE3::EPlatformType Platform = UE3::PLATFORM_Unknown );

/**
 * Compresses SoundNodeWave for all available platforms, and then decompresses to PCM 
 *
 * @param	SoundNodeWave			Wave file to compress
 * @param	PreviewInfo				Compressed stats and decompressed data
 */
void SoundNodeWaveQualityPreview( USoundNodeWave* SoundNode, FPreviewInfo * PreviewInfo );

#endif // __EDITOR_H__
