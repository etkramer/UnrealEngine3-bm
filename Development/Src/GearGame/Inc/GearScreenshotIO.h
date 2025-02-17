/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

#ifndef __GEARSCREENSHOTIO_H__
#define __GEARSCREENSHOTIO_H__

/** The result of an async task. */
enum eAsyncTaskResult
{
	ATR_Working,
	ATR_Succeeded,
	ATR_Failed
};

/** Contains the state of an async state. */
struct AsyncTaskState
{
	/** True if this async task is running.  This is controlled from the main thread. */
	UBOOL bActive;
	/** The result of the async task.  This is controlled by the task from a separate thread. */
	volatile INT Result;
	/** The player for which this task is running. */
	INT PlayerID;
	/** The delegates to be called when the task completes. */
	TArray<FScriptDelegate>* CompleteDelegates;
};

/**
 * Handles disk access for screenshots.
 */
class FGearScreenshotIO : public FTickableObject
{
public:
	/**
	 * Starts an async task to save screenshots to disk.
	 *
	 * @param PlayerID The player requesting the save.
	 * @param Image The JPG to save to disk.
	 * @param Thumbnail The PNG to use as a thumbnail.
	 * @param Info The screenshot's info.
	 * @param InSaveCompleteDelegates The delegates to call when the attempt completes.
	 *
	 * @return True if the async task is started successfully.
	 */
	UBOOL Save(INT PlayerID, const TArray<BYTE>& Image, const TArray<BYTE>& Thumbnail, const FScreenshotInfo& Info, TArray<FScriptDelegate>* InSaveCompleteDelegates);

	/**
	 * Loads a screenshot from disk.
	 *
	 * @param PlayerID The player requesting the screenshot to be loaded.
	 * @param ID The ID of the screenshot to load.
	 * @param DeviceID The device from which to load the screenshot.
	 * @param Image The screenshot's JPG will be stored in this array.
	 * @param Info The screenshot's info will be stored in this parameter.
	 * @param InLoadCompleteDelegates The delegates to call when the attempt completes.
	 *
	 * @return True if the async task is started successfully.
	 */
	UBOOL Load(INT PlayerID, const FGuid& ID, INT DeviceID, TArray<BYTE>& Image, FScreenshotInfo& Info, TArray<FScriptDelegate>* InLoadCompleteDelegates);

	/**
	 * Deletes a screenshot from disk.
	 *
	 * @param PlayerID The player requesting the delete.
	 * @param ID The unique ID of the screenshot to delete.
	 * @param DeviceID The device from which to delete the screenshot.
	 *
	 * @return True if the screenshot was successfully deleted.
	 */
	UBOOL Delete(INT PlayerID, const FGuid& ID, INT DeviceID);

	/**
	 * Gets a list of screenshots on disk.
	 *
	 * @param PlayerID The player requesting the enumeration.
	 * @param Screenshots The list of screenshots will be stored in this array.
	 * @param InEnumerateCompleteDelegates The delegates to call when the enumeration completes.
	 *
	 * @return True if the async task is started successfully.
	 */
	UBOOL Enumerate(INT PlayerID, TArray<FSavedScreenshotInfo>& Screenshots, TArray<FScriptDelegate>* InEnumerateCompleteDelegates);

	/**
	 * Checks if the object is busy doing IO.
	 * IO attempts while the object is busy will fail.
	 *
	 * @return True if the object is busy, false if the object is free to perform a new operations.
	 */
	UBOOL IsBusy() const;

	// FTickableObject interface

	/**
	* Used to determine whether an object is ready to be ticked.
	*
	* @return	TRUE if tickable, FALSE otherwise
	*/
	virtual UBOOL IsTickable() const;

	/**
	* Used to determine if an object should be ticked when the game is paused.
	*
	* @return always TRUE as networking needs to be ticked even when paused
	*/
	virtual UBOOL IsTickableWhenPaused() const;

	/**
	* Called every frame to allow for updates.
	*
	* @param DeltaTime	Game time passed since the last call.
	*/
	virtual void Tick( FLOAT DeltaTime );

private:
	AsyncTaskState SaveScreenshotTask;
	AsyncTaskState LoadScreenshotTask;
	AsyncTaskState EnumerateScreenshotTask;

	void TickTask(AsyncTaskState* Task);

#if _XBOX
	void GetContentDisplayName(const FScreenshotInfo& Info, FString& DisplayName);

	UBOOL GetXContentData(DWORD PlayerID, const FGuid& ID, INT DeviceID, XCONTENT_DATA& ContentData);

	void GetContentFileName(const FGuid& ID, ANSICHAR FileName[XCONTENT_MAX_FILENAME_LENGTH]);

	UBOOL GetDeviceID(INT PlayerID, DWORD& DeviceID);
#endif
};

#endif
