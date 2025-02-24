/**
 * LevelStreaming
 *
 * Abstract base class of container object encapsulating data required for streaming and providing 
 * interface for when a level should be streamed in and out of memory.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class LevelStreaming extends Object
	abstract
	editinlinenew
	native;

/** Name of the level package name used for loading.																		*/
var() editconst const name							PackageName;

/** Pointer to Level object if currently loaded/ streamed in.																*/
var transient const	level							LoadedLevel;

/** Offset applied to actors after loading.																					*/
var() const			vector							Offset;

/** Current/ old offset required for changing the offset at runtime, e.g. in the Editor.									*/
var const			vector							OldOffset;	

/** Whether the level is currently visible/ associated with the world														*/
var const transient bool							bIsVisible;

// BM1
var const transient bool bIsLevelHidden;

/** Whether we currently have a load request pending.																		*/
var const transient	bool							bHasLoadRequestPending;

/** Whether we currently have an unload request pending.																	*/
var const transient bool							bHasUnloadRequestPending;

/** Whether this level should be visible in the Editor																		*/
var() const			bool							bShouldBeVisibleInEditor;

/** Whether this level's bounding box should be visible in the Editor.														*/
var const			bool							bBoundingBoxVisible;

/** Whether this level is locked; that is, its actors are read-only.														*/
var() const			bool							bLocked;

/** Whether this level is fully static - if it is, then assumptions can be made about it, ie it doesn't need to be reloaded since nothing could have changed */
var() const			bool							bIsFullyStatic;

/** Whether the level should be loaded																						*/
var	const transient bool bShouldBeLoaded;

/** Whether the level should be visible if it is loaded																		*/
var const transient bool bShouldBeVisible;

// BM1
var const transient bool bShouldBeLevelHidden;

/** Whether we want to force a blocking load																				*/
var transient		bool							bShouldBlockOnLoad;

// BM1
var transient bool bShouldBeVisibleInLevelBrowser;
var transient bool bExpandedInLevelBrowser;
var transient bool bIsParent;
var transient bool bIsChild;

/** Whether this level streaming object's level should be unloaded and the object be removed from the level list.			*/
var const transient bool							bIsRequestingUnloadAndRemoval;

// BM1
var(Rocksteady) bool bExcludeFromPathBuilding;
var transient bool bExcludeFromPathBuilding_Old_bShouldBeVisibleInEditor;
var(Rocksteady) bool bLightPerRoom;
var(Rocksteady) bool ForceShadowVolumes;

/** The level's color; used to make the level easily identifiable in the level browser, for actor level visulization, etc.	*/
var() const			color							DrawColor;

/** The level streaming volumes bound to this level.																		*/
var() const editconst array<LevelStreamingVolume>	EditorStreamingVolumes;

/** Cooldown time in seconds between volume-based unload requests.  Used in preventing spurious unload requests.			*/
var() float											MinTimeBetweenVolumeUnloadRequests;

/** Time of last volume unload request.  Used in preventing spurious unload requests.										*/
var const transient float							LastVolumeUnloadRequestTime;

// BM1
var(Rocksteady) name AlternativeMapLevel;
var(Rocksteady) string PersonResponsible;
var(Rocksteady) name ConditionalStartFlag;
var(Rocksteady) name ConditionalLoadFlag;

cpptext
{
	/**
	 * Returns whether this level should be present in memory which in turn tells the 
	 * streaming code to stream it in. Please note that a change in value from FALSE 
	 * to TRUE only tells the streaming code that it needs to START streaming it in 
	 * so the code needs to return TRUE an appropriate amount of time before it is 
	 * needed.
	 *
	 * @param ViewLocation	Location of the viewer
	 * @return TRUE if level should be loaded/ streamed in, FALSE otherwise
	 */
	virtual UBOOL ShouldBeLoaded( const FVector& ViewLocation );

	/**
	 * Returns whether this level should be visible/ associated with the world if it is
	 * loaded.
	 * 
	 * @param ViewLocation	Location of the viewer
	 * @return TRUE if the level should be visible, FALSE otherwise
	 */
	virtual UBOOL ShouldBeVisible( const FVector& ViewLocation );
	
	// UObject interface.
	virtual void PostEditChange(FEditPropertyChain& PropertyThatChanged);
}

defaultproperties
{
	bShouldBeVisibleInEditor=TRUE
	DrawColor=(R=255,G=255,B=255,A=255)

	MinTimeBetweenVolumeUnloadRequests=2.0
}
