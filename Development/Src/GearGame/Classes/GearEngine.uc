/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearEngine extends GameEngine
	native;

/** Total number of bytes needed by a device to support 3 checkpoints being saved */
const MAX_DATASIZE_FOR_ALL_CHECKPOINTS = 10000000;

cpptext
{
	virtual UBOOL LoadMap(const FURL& URL, class UPendingLevel* Pending, FString& Error);
	virtual void Tick(FLOAT DeltaSeconds);
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
};

/** Information set to trigger a checkpoint load/save on the next tick. */
enum ECheckpointAction
{
	Checkpoint_Default,
	Checkpoint_Load,
	Checkpoint_Save,

	/** indicates that the save-game currently in use should be deleted */
	Checkpoint_DeleteAll,
};

var							vector		PendingCheckpointLocation;
var					ECheckpointAction	PendingCheckpointAction;

/** current checkpoint in use (if any) */
var							Checkpoint	CurrentCheckpoint;

/** user and device ID to use when checkpointing on xbox */
var							int			CurrentUserID;
var		private{private}	int			CurrentDeviceID;

/** whether to actually write checkpoint data to the storage device or simply keep it in memory */
var							bool		bShouldWriteCheckpointToDisk;

/**
 * Tracks whether the current value of CurrentDeviceID is expected to be valid; used for deterining what action to take when
 * IsCurrentDeviceValid() returns FALSE.
 */
var	transient	const		bool		bHasSelectedValidStorageDevice;

/** if this is > 0 and we fall below this many bytes free memory, force the player to lose the game and do a map reload */
var config int FailureMemoryHackThreshold;

/* == Natives == */
/**
 * Accessor for setting the value of CurrentDeviceID
 *
 * @param	NewDeviceID			the new value to set CurrentDeviceID to
 * @param	bProfileSignedOut	Controls whether the previous value of CurrentDeviceID is considered; specify TRUE when setting
 *								CurrentDeviceID to an invalid value as a result of a profile being signed out and
 *								bHasSelectedValidStorageDevice will be reset to FALSE.
 */
native final function SetCurrentDeviceID( int NewDeviceID, optional bool bProfileSignedOut );

/**
 * Accessor for getting the value of CurrentDeviceID
 */
native final function int GetCurrentDeviceID() const;

/**
 * Wrapper for checking whether a previously selected storage device has been removed.
 *
 * @return	TRUE if the currently selected device is invalid but bHasSelectedValidStorageDevice is TRUE.
 */
native final function bool HasStorageDeviceBeenRemoved() const;

/**
 * Loads the desired checkpoint slot from disk/memory card/etc and stores it in CurrentCheckpoint for later use
 * if there is no checkpoint data to load, CurrentCheckpoint will be set to NULL
 *
 * @param	SlotIndex	indicates the slot to load the checkpoint data from; when a value is provided for EnumResult, the value of SlotIndex
 *						must be INDEX_NONE.
 * @param	EnumResult	receives the collection of metadata for all existing checkpoints.  If a value is passed in, SlotIndex must be INDEX_NONE
 *						and CurrentCheckpoint will not be affected.
 *
 * @return	TRUE if the file was successfully loaded or didn't exist; FALSE is only returned if the data existed but was corrupt
 *			(i.e. for use for UI errors and such)
 */
native final function bool FindCheckpointData(int SlotIndex, optional out CheckpointEnumerationResult EnumResult);

/** Checkpoint save/load accessors */
protected native final function SaveCheckpoint(vector CheckpointLocation);
protected native final function LoadCheckpoint();

/**
 * Deletes the save game containing all checkpoints.  This function is triggered by setting PendingCheckpointAction to Checkpoint_DeleteAll.
 */
protected native final function bool DeleteCheckpoints( optional out int ResultCode );

/** @return whether our free memory is too low so we must force failure and a map reload to reset things */
native final function bool NeedMemoryHack();

/**
 * Checks whether the currently selected device is valid (i.e. connected, has enough space, etc.)
 *
 * @param	SizeNeeded	optionally specify a minimum size that must be available on the device.
 *
 * @return	TRUE if the current device is valid and contains enough space.
 */
event bool IsCurrentDeviceValid( optional int SizeNeeded )
{
	local OnlineSubsystem OnlineSub;
	local bool bResult;

	OnlineSub = GetOnlineSubsystem();
	if ( OnlineSub != None && OnlineSub.PlayerInterfaceEx != None )
	{
		bResult = OnlineSub.PlayerInterfaceEx.IsDeviceValid(CurrentDeviceID, SizeNeeded);
	}

	return bResult;
}

/**
 * Wrapper for checking whether saving user content (checkpoints, screenshots) to a storage device is allowed.
 *
 * @param	bIgnoreDeviceStatus		specify TRUE to skip checking whether saving to a storage device is actually possible.
 *
 * @return	TRUE if saving checkpoint data and screenshots (and any other profile-specific content) is allowed.
 */
event bool AreStorageWritesAllowed( optional bool bIgnoreDeviceStatus, optional int RequiredSize=MAX_DATASIZE_FOR_ALL_CHECKPOINTS )
{
	return	bShouldWriteCheckpointToDisk &&
			(bIgnoreDeviceStatus || IsCurrentDeviceValid(RequiredSize));
}

defaultproperties
{
	CurrentDeviceID=-1
	bShouldWriteCheckpointToDisk=true
}
