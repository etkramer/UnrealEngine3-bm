//=============================================================================
// LocalPlayer
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class LocalPlayer extends Player
	within Engine
	config(Engine)
	native
	transient;

/** The controller ID which this player accepts input from. */
var int ControllerId;

/** The master viewport containing this player's view. */
var GameViewportClient ViewportClient;

/** The coordinates for the upper left corner of the master viewport subregion allocated to this player. 0-1 */
var vector2d Origin;

/** The size of the master viewport subregion allocated to this player. 0-1 */
var vector2d Size;

// BM1
var float ForegroundFOVScale;
var bool IsXrayEnabled;

/** Whether to override the post process settings or not */
var bool bOverridePostProcessSettings;

/** set when we've sent a split join request */
var const editconst transient bool bSentSplitJoin;

/** Chain of post process effects for this player view */
var const PostProcessChain PlayerPostProcess;
var const array<PostProcessChain> PlayerPostProcessChains;

var private native const pointer ViewState{FSceneViewStateInterface};

struct SynchronizedActorVisibilityHistory
{
	var pointer State;
	var pointer CriticalSection;
};

var private native transient const SynchronizedActorVisibilityHistory ActorVisibilityHistory;

/** The location of the player's view the previous frame. */
var transient vector LastViewLocation;

struct native CurrentPostProcessVolumeInfo
{
	/** Last pp settings used when blending to the next set of volume values. */
	var PostProcessSettings	LastSettings;
	/** The last post process volume that was applied to the scene */
	var PostProcessVolume LastVolumeUsed;
	/** Time when a new post process volume was set */
	var float BlendStartTime;
	/** Time when the settings blend was last updated. */
	var float LastBlendTime;
};
/** current state of post process info set in the scene */
var const noimport transient CurrentPostProcessVolumeInfo CurrentPPInfo;

/** The post process settings to override to */
var PostProcessSettings PostProcessSettingsOverride;
/** The start time of the post process override blend */
var float PPSettingsOverrideStartBlend;

cpptext
{
	/** Is object propagation currently overriding our view? */
	static UBOOL bOverrideView;
	static FVector OverrideLocation;
	static FRotator OverrideRotation;

	// Constructor.
	ULocalPlayer();

	/**
	 *	Rebuilds the PlayerPostProcessChain.
	 *	This should be called whenever the chain array has items inserted/removed.
	 */
	void RebuildPlayerPostProcessChain();

	/**
	 * Updates the post-process settings for the player's view.
	 * @param ViewLocation - The player's current view location.
	 */
	void UpdatePostProcessSettings(const FVector& ViewLocation);

	/**
	 * Calculate the view settings for drawing from this view actor
	 *
	 * @param	View - output view struct
	 * @param	ViewLocation - output actor location
	 * @param	ViewRotation - output actor rotation
	 * @param	Viewport - current client viewport
	 */
	FSceneView* CalcSceneView( FSceneViewFamily* ViewFamily, FVector& ViewLocation, FRotator& ViewRotation, FViewport* Viewport );

	// UObject interface.
	virtual void FinishDestroy();

	// FExec interface.
	virtual UBOOL Exec(const TCHAR* Cmd,FOutputDevice& Ar);

	void ExecMacro( const TCHAR* Filename, FOutputDevice& Ar );

}

/**
 * Creates an actor for this player.
 * @param URL - The URL the player joined with.
 * @param OutError - If an error occurred, returns the error description.
 * @return False if an error occurred, true if the play actor was successfully spawned.
 */
native final function bool SpawnPlayActor(string URL,out string OutError);

/** sends a splitscreen join command to the server to allow a splitscreen player to connect to the game
 * the client must already be connected to a server for this function to work
 * @note this happens automatically for all viewports that exist during the initial server connect
 * 	so it's only necessary to manually call this for viewports created after that
 * if the join fails (because the server was full, for example) all viewports on this client will be disconnected
 */
native final function SendSplitJoin();

/**
 * Tests the visibility state of an actor in the most recent frame of this player's view to complete rendering.
 * @param TestActor - The actor to check visibility for.
 * @return True if the actor was visible in the frame.
 */
native final function bool GetActorVisibility(Actor TestActor) const;

/**
 * Overrides the current post process settings.
 */
simulated function OverridePostProcessSettings( PostProcessSettings OverrideSettings, float StartBlendTime )
{
	// never allow post-process overrides in the front-end.
	if ( !class'WorldInfo'.static.IsMenuLevel() )
	{
		PostProcessSettingsOverride = OverrideSettings;
		bOverridePostProcessSettings = true;
		PPSettingsOverrideStartBlend = StartBlendTime;
	}
}

/**
 * Update the override post process settings
 */
simulated function UpdateOverridePostProcessSettings( PostProcessSettings OverrideSettings )
{
	PostProcessSettingsOverride = OverrideSettings;
}

/**
 * Clear the overriding of the post process settings.
 */
simulated function ClearPostProcessSettingsOverride()
{
	bOverridePostProcessSettings = false;
}

/**
 * Changes the ControllerId for this player; if the specified ControllerId is already taken by another player, changes the ControllerId
 * for the other player to the ControllerId currently in use by this player.
 *
 * @param	NewControllerId		the ControllerId to assign to this player.
 */
final function SetControllerId( int NewControllerId )
{
	local LocalPlayer OtherPlayer;
	local int CurrentControllerId;

	if ( ControllerId != NewControllerId )
	{
		`log(Name @ "changing ControllerId from" @ ControllerId @ "to" @ NewControllerId,,'PlayerManagement');

		// first, unregister the player's data stores if we already have a PlayerController.
		if ( Actor != None )
		{
			Actor.PreControllerIdChange();
		}

		CurrentControllerId = ControllerId;

		// set this player's ControllerId to -1 so that if we need to swap controllerIds with another player we don't
		// re-enter the function for this player.
		ControllerId = -1;

		// see if another player is already using this ControllerId; if so, swap controllerIds with them
		OtherPlayer = ViewportClient.FindPlayerByControllerId(NewControllerId);
		if ( OtherPlayer != None )
		{
			OtherPlayer.SetControllerId(CurrentControllerId);
		}

		ControllerId = NewControllerId;
		if ( Actor != None )
		{
			Actor.PostControllerIdChange();
		}
	}
}

/**
 * Add the given post process chain to the chain at the given index.
 *
 *	@param	InChain		The post process chain to insert.
 *	@param	InIndex		The position to insert the chain in the complete chain.
 *						If -1, insert it at the end of the chain.
 *	@param	bInClone	If TRUE, create a deep copy of the chains effects before insertion.
 *
 *	@return	boolean		TRUE if the chain was inserted
 *						FALSE if not
 */
native function bool InsertPostProcessingChain(PostProcessChain InChain, int InIndex, bool bInClone);

/**
 * Remove the post process chain at the given index.
 *
 *	@param	InIndex		The position to insert the chain in the complete chain.
 *
 *	@return	boolean		TRUE if the chain was removed
 *						FALSE if not
 */
native function bool RemovePostProcessingChain(int InIndex);

/**
 * Remove all post process chains.
 *
 *	@return	boolean		TRUE if the chain array was cleared
 *						FALSE if not
 */
native function bool RemoveAllPostProcessingChains();

/**
 *	Get the PPChain at the given index.
 *
 *	@param	InIndex				The index of the chain to retrieve.
 *
 *	@return	PostProcessChain	The post process chain if found; NULL if not.
 */
native function PostProcessChain GetPostProcessChain(int InIndex);

/**
 *	Forces the PlayerPostProcess chain to be rebuilt.
 *	This should be called if a PPChain is retrieved using the GetPostProcessChain,
 *	and is modified directly.
 */
native function TouchPlayerPostProcessChain();

defaultproperties
{
	bOverridePostProcessSettings=false
}
