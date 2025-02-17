/**
 * This class manages screenshots on disk.
 * Functionality includes saving, enumerating, deleting, loading, and uploading.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearScreenshotManager extends Object
	within GearPC
	native
	config(Pawn)
	transient;

/** Keeps a pointer to the class that handles disk I/O. */
var native transient const private pointer ScreenshotIO{class FGearScreenshotIO};

/** These are IDs needed for uploading screenshots.  They come from the auto-generated XNA .cpp file. */
var config int ScreenshotContentType;
var config int ScreenshotBaseRecordId;

/** Delegates that get called when a screenshot finishes saving to disk. */
var array<delegate<OnSaveScreenshotComplete> > SaveScreenshotCompleteDelegates;

/** Delegates that get called when enumerating screenshots completes. */
var array<delegate<OnEnumerateScreenshotsComplete> > EnumerateScreenshotsCompleteDelegates;

/** Delegates that get called when a screenshot finishes loading from disk. */
var array<delegate<OnLoadScreenshotComplete> > LoadScreenshotCompleteDelegates;

/** Delegates that get called when a screenshot finished uploading. */
var array<delegate<OnUploadScreenshotComplete> > UploadScreenshotCompleteDelegates;

/**
 * Initializes the object.
 *
 * @return True if the initializiation succeeded.
 */
native function bool Init();

/**
 * Delegate fired when saving a screenshot completes.
 *
 * @param bWasSuccessful True if the save completed successfully.
 */
delegate OnSaveScreenshotComplete(bool bWasSuccessful);

/**
 * Starts an async task which saves the screenshots to disk.
 * If the function returns true, then any OnSaveScreenshotComplete delegates will be called when the save completes.
 *
 * @param Image The pre-compressed JPG file to save to disk.
 * @param Thumbnail The pre-compressed PNG file to use as a thumbnail.
 * @param Info Info to save along with the screenshot.  This can be used for UI display, and is converted to metadata for upload.
 *
 * @return True if the async operation is started successfully.  The save is not complete until the delegate is called.
 */
native function bool SaveScreenshot(array<byte> Image, array<byte> Thumbnail, const out ScreenshotInfo Info);

/**
 * Deletes a screenshot from disk.
 *
 * @param ID The unique ID of the screenshot to delete.
 * @param DeviceID The device from which to delete the screenshot.
 *
 * @return True if the delete succeeds.
 */
native function bool DeleteScreenshot(guid ID, int DeviceID);

/**
 * Delegate fired when enumerating screenshots completes.
 *
 * @param bWasSuccessful True if the enumeration completed successfully.
 */
delegate OnEnumerateScreenshotsComplete(bool bWasSuccessful);

/**
 * Gets a list of screenshots on disk.
 *
 * @param Screenshots If the function succeeds, the array will contain a list of this player's screenshots on disk.
 *
 * @return True if the enumeration succeeds.
 */
native function bool EnumerateScreenshots(out array<SavedScreenshotInfo> Screenshots);

/**
 * Delegate fired when saving a screenshot completes.
 *
 * @param bWasSuccessful True if the save completed successfully.
 */
delegate OnLoadScreenshotComplete(bool bWasSuccessful);

/**
 * Loads a screenshot from disk.
 *
 * @param ID The unique ID of the screenshot to load.
 * @param DeviceID The device from which to load the screenshot.
 * @param Image If the function succeeds, this will contain the JPG file for this screenshot.
 * @param Info If the function succeeds, this will contain the info saved along with the screenshot.
 *
 * @return True if the screenshot was loaded successfully.
 */
native function bool LoadScreenshot(const out Guid ID, int DeviceID, out array<byte> Image, out ScreenshotInfo Info);

/**
 * Delegate fired when uploading a screenshot completes.
 *
 * @param bWasSuccessful True if the upload completed successfully.
 */
delegate private OnUploadScreenshotComplete(bool bWasSuccessful);

/**
 * Wrapper for getting a reference to the LocalPlayer associated with this screenshot manager.
 */
native final function LocalPlayer GetPlayerOwner() const;

/**
 * Wrapper for getting the controller id of the player that owns this screenshot manager.
 *
 * @return	the controller ID for the player associated with this screenshot manager, or 255 if the owning player doesn't have a valid
 *			local player object.
 */
native final function int GetPlayerControllerId() const;

/**
 * Clears all external references and any references from this objec to others.
 *
 * @todo ronp - we could set ScreenshotIO to NULL [would need to be a native function that we call], then we could use that as a condition
 *				to prevent anyone creating external references to this object once it's been detached from the player.
 */
function Cleanup()
{
	local OnlineCommunityContentInterface CommunityContent;

	SaveScreenshotCompleteDelegates.Length = 0;
	UploadScreenshotCompleteDelegates.Length = 0;

	if ( OnlineSub != None )
	{
		CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
		if ( CommunityContent != None )
		{
			CommunityContent.ClearUploadContentCompleteDelegate(UploadScreenshotComplete);
		}
	}

	// make sure we close the connection
	CloseCommunityContentConnection();
}

/** Close the connection to the content server. */
function CloseCommunityContentConnection()
{
	local OnlineCommunityContentInterface CommunityContent;

	`log(`location);

	if ( OnlineSub != None )
	{
		CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
		if ( CommunityContent != None )
		{
			CommunityContent.ClearUploadContentCompleteDelegate(UploadScreenshotComplete);
			CommunityContent.Exit();
		}
	}
}

/** Internal function called when uploading a screenshot completes. */
function UploadScreenshotComplete(bool bWasSuccessful,CommunityContentFile UploadedFile)
{
	local int Index;
	local array<delegate<OnUploadScreenshotComplete> > LocalCopy;
	local delegate<OnUploadScreenshotComplete> UploadDelegate;

	`Log("Screenshot upload completed for content id: "$UploadedFile.ContentId$", result: "$bWasSuccessful);

	if(bWasSuccessful)
	{
		// Unlock the eGA_Photojournalist achievement
		ClientUnlockAchievement( eGA_Photojournalist );
	}

	LocalCopy = UploadScreenshotCompleteDelegates;
	for(Index = 0; Index < LocalCopy.length; Index++)
	{
		UploadDelegate = LocalCopy[Index];
		UploadDelegate(bWasSuccessful);
	}
}

/** Internal function used to convert ScreenshotInfo to the metadata format needed for uploading content */
function ScreenshotInfoToMetadata(const out ScreenshotInfo Info, out CommunityContentMetadata Metadata)
{
	local array<BYTE> InfoBlob;
	local int RecordId;
	CreateInfoBlob(Info, InfoBlob);
	//Set IDs.
	Metadata.ContentType = ScreenshotContentType;
	RecordId = ScreenshotBaseRecordId;
	// Set Metadata.
	AddMetadataString(Metadata, RecordId++, Info.Description);
	AddMetadataInt(Metadata, RecordId++, Info.Rating);
	AddMetadataInt(Metadata, RecordId++, Info.GameTypeId);
	AddMetadataInt(Metadata, RecordId++, Info.MatchType);
	AddMetadataString(Metadata, RecordId++, Info.MapName);
	AddMetadataFloat(Metadata, RecordId++, Info.WorldTime);
	AddMetadataDateTime(Metadata, RecordId++, Info.Realtime);
	AddMetadataGuid(Metadata, RecordId++, Info.MatchId);
	AddMetadataBlob(Metadata, RecordId++, InfoBlob);
}

/**
 * Uploads a saved screenshot.
 * If the function returns true, then any OnUploadScreenshotComplete delegates will be called when the save completes.
 *
 * @param Image The raw bytes of the image to upload.
 * @param Info Info for the screenshot being uploaded.
 *
 * @return True if the async task is started successfully.  The upload is not complete until the delegate is called.
 */
function bool UploadScreenshot(const out array<byte> Image, ScreenshotInfo Info)
{
	local OnlineCommunityContentInterface CommunityContent;
	local CommunityContentMetadata Metadata;
	local bool bUploading;
	local bool bWasSuccessful;
	bUploading = false;
	if(OnlineSub != None)
	{
		CommunityContent = OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent'));
		if (CommunityContent != None)
		{
			bWasSuccessful = CommunityContent.Init();
			if(bWasSuccessful)
			{
				CommunityContent.AddUploadContentCompleteDelegate(UploadScreenshotComplete);
				ScreenshotInfoToMetadata(Info, Metadata);
				bWasSuccessful = CommunityContent.UploadContent(GetPlayerControllerId(), Image, Metadata);
				if(bWasSuccessful)
				{
					`log("UploadScreenshot: Initiated upload");
					bUploading = true;
				}
				else
				{
					CommunityContent.ClearUploadContentCompleteDelegate(UploadScreenshotComplete);
					CommunityContent.Exit();
					`log("UploadScreenshot: CommunityContent.UploadContent failed");
				}
			}
			else
			{
				`log("UploadScreenshot: CommunityContent.Init failed");
			}
		}
		else
		{
			`log("UploadScreenshot: Couldn't get the CommunityContent interface");
		}
	}
	else
	{
		`log("UploadScreenshot: Couldn't get the OnlineSubsystem interface");
	}
	return bUploading;
}

/**
 * Adds the delegate to the list that will be notified when the task completes
 *
 * @param SaveScreenshotCompleteDelegate the delegate to use for notifications
 */
function AddSaveScreenshotCompleteDelegate(delegate<OnSaveScreenshotComplete> SaveScreenshotCompleteDelegate)
{
	// Add this delegate to the array if not already present
	if (SaveScreenshotCompleteDelegates.Find(SaveScreenshotCompleteDelegate) == INDEX_NONE)
	{
		SaveScreenshotCompleteDelegates[SaveScreenshotCompleteDelegates.Length] = SaveScreenshotCompleteDelegate;
	}
}

/**
 * Removes the delegate from the list of notifications
 *
 * @param SaveScreenshotCompleteDelegate the delegate to use for notifications
 */
function ClearSaveScreenshotCompleteDelegate(delegate<OnSaveScreenshotComplete> SaveScreenshotCompleteDelegate)
{
	local int RemoveIndex;

	// Remove this delegate from the array if found
	RemoveIndex = SaveScreenshotCompleteDelegates.Find(SaveScreenshotCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		SaveScreenshotCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Adds the delegate to the list that will be notified when the task completes
 *
 * @param EnumerateScreenshotsCompleteDelegate the delegate to use for notifications
 */
function AddEnumerateScreenshotsCompleteDelegate(delegate<OnEnumerateScreenshotsComplete> EnumerateScreenshotsCompleteDelegate)
{
	// Add this delegate to the array if not already present
	if (EnumerateScreenshotsCompleteDelegates.Find(EnumerateScreenshotsCompleteDelegate) == INDEX_NONE)
	{
		EnumerateScreenshotsCompleteDelegates[EnumerateScreenshotsCompleteDelegates.Length] = EnumerateScreenshotsCompleteDelegate;
	}
}

/**
 * Removes the delegate from the list of notifications
 *
 * @param EnumerateScreenshotsCompleteDelegate the delegate to use for notifications
 */
function ClearEnumerateScreenshotsCompleteDelegate(delegate<OnEnumerateScreenshotsComplete> EnumerateScreenshotsCompleteDelegate)
{
	local int RemoveIndex;

	// Remove this delegate from the array if found
	RemoveIndex = EnumerateScreenshotsCompleteDelegates.Find(EnumerateScreenshotsCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		EnumerateScreenshotsCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Adds the delegate to the list that will be notified when the task completes
 *
 * @param LoadScreenshotCompleteDelegate the delegate to use for notifications
 */
function AddLoadScreenshotCompleteDelegate(delegate<OnLoadScreenshotComplete> LoadScreenshotCompleteDelegate)
{
	// Add this delegate to the array if not already present
	if (LoadScreenshotCompleteDelegates.Find(LoadScreenshotCompleteDelegate) == INDEX_NONE)
	{
		LoadScreenshotCompleteDelegates[LoadScreenshotCompleteDelegates.Length] = LoadScreenshotCompleteDelegate;
	}
}

/**
 * Removes the delegate from the list of notifications
 *
 * @param LoadScreenshotCompleteDelegate the delegate to use for notifications
 */
function ClearLoadScreenshotCompleteDelegate(delegate<OnLoadScreenshotComplete> LoadScreenshotCompleteDelegate)
{
	local int RemoveIndex;

	// Remove this delegate from the array if found
	RemoveIndex = LoadScreenshotCompleteDelegates.Find(LoadScreenshotCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		LoadScreenshotCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/**
 * Adds the delegate to the list that will be notified when the task completes
 *
 * @param UploadScreenshotCompleteDelegate the delegate to use for notifications
 */
function AddUploadScreenshotCompleteDelegate(delegate<OnUploadScreenshotComplete> UploadScreenshotCompleteDelegate)
{
	// Add this delegate to the array if not already present
	if (UploadScreenshotCompleteDelegates.Find(UploadScreenshotCompleteDelegate) == INDEX_NONE)
	{
		UploadScreenshotCompleteDelegates[UploadScreenshotCompleteDelegates.Length] = UploadScreenshotCompleteDelegate;
	}
}

/**
 * Removes the delegate from the list of notifications
 *
 * @param UploadScreenshotCompleteDelegate the delegate to use for notifications
 */
function ClearUploadScreenshotCompleteDelegate(delegate<OnUploadScreenshotComplete> UploadScreenshotCompleteDelegate)
{
	local int RemoveIndex;

	// Remove this delegate from the array if found
	RemoveIndex = UploadScreenshotCompleteDelegates.Find(UploadScreenshotCompleteDelegate);
	if (RemoveIndex != INDEX_NONE)
	{
		UploadScreenshotCompleteDelegates.Remove(RemoveIndex,1);
	}
}

/** Convert a JPG from a byte array to a texture */
native static function ScreenshotToTexture(const out array<byte> Image, out Texture2DDynamic Texture);

/** Internal function used to convert a GUID to a string */
native static function GuidToString(const Guid InGuid, out string OutString);

/** Internal function used to convert a string to a blob of ANSI chars */
native function StringToAnsiBlob(const out string InString, out array<byte> OutBlob);

/** Internal functions used for writing XML elements to a string */
native function XmlWriteStartTag(out string Xml, string TagName, int Indent);
native function XmlBeginStartTag(out string Xml, string TagName, int Indent);
native function XmlEndStartTag(out string Xml, bool Close);
native function XmlWriteEndTag(out string Xml, string TagName, int Indent = -1);
native function XmlWriteAttributeString(out string Xml, string Attribute, string Value);
native function XmlWriteAttributeBool(out string Xml, string Attribute, bool Value);
native function XmlWriteAttributeInt(out string Xml, string Attribute, int Value);
native function XmlWriteAttributeFloat(out string Xml, string Attribute, float Value);
native function XmlWriteAttributeQword(out string Xml, string Attribute, int Value1, int Value2);
native function XmlWriteContent(out string Xml, string Content);

/** Internal function that writes the a players and his info to an XML string */
function WritePlayerXml(out string Xml, const out ScreenshotPlayerInfo Info, int Indent)
{
	local Box PlayerBox;
	local vector PlayerLocation;
	local UniqueNetId BotXuid;
	PlayerBox = Info.ScreenSpaceBoundingBox;
	PlayerLocation = Info.Location;
	//player
	XmlBeginStartTag(Xml, "player", Indent++);
	if(Info.Xuid == BotXuid)
	{
		// they actually want -1 for bots
		XmlWriteAttributeQword(Xml, "xuid", -1, -1);
		XmlWriteAttributeString(Xml, "name", Info.Nick);
	}
	else
	{
		XmlWriteAttributeQword(Xml, "xuid", Info.Xuid.Uid.A, Info.Xuid.Uid.B);
	}
	XmlWriteAttributeBool(Xml, "visible", Info.IsVisible);
	XmlEndStartTag(Xml, false);
	//screen
	if(Info.IsVisible)
	{
		XmlBeginStartTag(Xml, "screen", Indent);
		XmlWriteAttributeInt(Xml, "xmin", FFloor(PlayerBox.Min.X));
		XmlWriteAttributeInt(Xml, "ymin", FFloor(PlayerBox.Min.Y));
		XmlWriteAttributeInt(Xml, "xmax", FFloor(PlayerBox.Max.X));
		XmlWriteAttributeInt(Xml, "ymax", FFloor(PlayerBox.Max.Y));
		XmlEndStartTag(Xml, true);
	}
	//location
	XmlBeginStartTag(Xml, "location", Indent);
	XmlWriteAttributeFloat(Xml, "x", PlayerLocation.X);
	XmlWriteAttributeFloat(Xml, "y", PlayerLocation.Y);
	XmlWriteAttributeFloat(Xml, "z", PlayerLocation.Z);
	XmlEndStartTag(Xml, true);
	XmlWriteEndTag(Xml, "player", --Indent);
}

/** Internal function that writes the list of players and their info to an XML string */
function WritePlayersXml(out string Xml, const out array<ScreenshotPlayerInfo> Players, int Indent)
{
	local int Index;
	local ScreenshotPlayerInfo Info;
	XmlWriteStartTag(Xml, "players", Indent++);
	for(Index = 0; Index < Players.length; Index++)
	{
		Info = Players[Index];
		WritePlayerXml(Xml, Info, Indent);
	}
	XmlWriteEndTag(Xml, "players", --Indent);
}

/** Internal function that writes the camera info to an XML string */
function WriteCameraXml(out string Xml, vector CamLoc, rotator CamRot, int Indent)
{
	//camera
	XmlWriteStartTag(Xml, "camera", Indent++);
	//location
	XmlBeginStartTag(Xml, "location", Indent);
	XmlWriteAttributeFloat(Xml, "x", CamLoc.X);
	XmlWriteAttributeFloat(Xml, "y", CamLoc.Y);
	XmlWriteAttributeFloat(Xml, "z", CamLoc.Z);
	XmlEndStartTag(Xml, true);
	//rotation
	XmlBeginStartTag(Xml, "rotation", Indent);
	XmlWriteAttributeInt(Xml, "pitch", CamRot.Pitch);
	XmlWriteAttributeInt(Xml, "yaw", CamRot.Yaw);
	XmlWriteAttributeInt(Xml, "roll", CamRot.Roll);
	XmlEndStartTag(Xml, true);
	XmlWriteEndTag(Xml, "camera", --Indent);
}

/** Internal function that converts a screenshot's info to a blob containing XML */
function CreateInfoBlob(const out ScreenshotInfo Info, out array<BYTE> Blob)
{
	local string Xml;
	local int Indent;
	Indent = 0;
	//info
	XmlWriteStartTag(Xml, "info", Indent++);
	//players
	WritePlayersXml(Xml, Info.Players, Indent);
	//camera
	WriteCameraXml(Xml, Info.CamLoc, Info.CamRot, Indent);
	XmlWriteEndTag(Xml, "info", --Indent);
	//`log("Info blob:"$Xml);
	StringToAnsiBlob(Xml, Blob);
}

/** Internal function that adds a string value to the list of metadata for upload */
static function AddMetadataString(out CommunityContentMetadata Metadata, int RecordId, string Value)
{
	local SettingsData Data;
	local int Index;
	Index = Metadata.MetadataItems.length;
	class'Settings'.static.SetSettingsDataString(Data,Value);
	Metadata.MetadataItems.Add(1);
	Metadata.MetadataItems[Index].PropertyId = RecordId;
	Metadata.MetadataItems[Index].Data = Data;
}

/** Internal function that adds an int value to the list of metadata for upload */
static function AddMetadataInt(out CommunityContentMetadata Metadata, int RecordId, int Value)
{
	local SettingsData Data;
	local int Index;
	Index = Metadata.MetadataItems.length;
	class'Settings'.static.SetSettingsDataInt(Data,Value);
	Metadata.MetadataItems.Add(1);
	Metadata.MetadataItems[Index].PropertyId = RecordId;
	Metadata.MetadataItems[Index].Data = Data;
}

/** Internal function that adds a float value to the list of metadata for upload */
static function AddMetadataFloat(out CommunityContentMetadata Metadata, int RecordId, float Value)
{
	local SettingsData Data;
	local int Index;
	Index = Metadata.MetadataItems.length;
	class'Settings'.static.SetSettingsDataFloat(Data,Value);
	Metadata.MetadataItems.Add(1);
	Metadata.MetadataItems[Index].PropertyId = RecordId;
	Metadata.MetadataItems[Index].Data = Data;
}

/** Internal function that adds a guid value to the list of metadata for upload */
static function AddMetadataGuid(out CommunityContentMetadata Metadata, int RecordId, Guid Value)
{
	local SettingsData Data;
	local int Index;
	local string ValueString;
	Index = Metadata.MetadataItems.length;
	GuidToString(Value, ValueString);
	class'Settings'.static.SetSettingsDataString(Data,ValueString);
	Metadata.MetadataItems.Add(1);
	Metadata.MetadataItems[Index].PropertyId = RecordId;
	Metadata.MetadataItems[Index].Data = Data;
}

/** Internal function that adds a blob value to the list of metadata for upload */
static function AddMetadataBlob(out CommunityContentMetadata Metadata, int RecordId, out array<BYTE> Value)
{
	local SettingsData Data;
	local int Index;
	Index = Metadata.MetadataItems.length;
	class'Settings'.static.SetSettingsDataBlob(Data,Value);
	Metadata.MetadataItems.Add(1);
	Metadata.MetadataItems[Index].PropertyId = RecordId;
	Metadata.MetadataItems[Index].Data = Data;
}

/** Internal function that adds a datetime value to the list of metadata for upload */
static function AddMetadataDateTime(out CommunityContentMetadata Metadata, int RecordId, DateTime Value)
{
	local SettingsData Data;
	local int Index;
	Index = Metadata.MetadataItems.length;
	class'Settings'.static.SetSettingsDataDateTime(Data,Value.A,Value.B);
	Metadata.MetadataItems.Add(1);
	Metadata.MetadataItems[Index].PropertyId = RecordId;
	Metadata.MetadataItems[Index].Data = Data;
}


