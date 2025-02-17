/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UnrealEdEngine extends EditorEngine
	native
	config(Engine)
	noexport
	transient;

var const noexport	pointer NotifyVtbl;

/** Global instance of the editor options class. */
var const UnrealEdOptions  EditorOptionsInst;

/**
 * Manager responsible for creating and configuring browser windows
 */
var const BrowserManager BrowserManager;

/**
 * Manager responsible for configuring and rendering thumbnails
 */
var const ThumbnailManager ThumbnailManager;

/**
 * Holds the name of the browser manager class to instantiate
 */
var config string BrowserManagerClassName;

/**
 * Holds the name of the class to instantiate
 */
var config string ThumbnailManagerClassName;

/** The current autosave number, appended to the autosave map name, wraps after 10 */
var const config int			AutoSaveIndex;
/** The number of 10-sec intervals that have passed since last autosave. */
var const float					AutosaveCount;
/** Is autosaving enabled? */
var(Advanced) config bool		AutoSave;
/** How often to save out to disk */
var(Advanced) config int		AutosaveTimeMinutes;

/** A buffer for implementing material expression copy/paste. */
var const Material				MaterialCopyPasteBuffer;

/** A buffer for implementing matinee track/group copy/paste. */
var const Object				MatineeCopyPasteBuffer;


/** Used for copy/pasting sockets between skeletal meshes. */
struct native SkelSocketCopyInfo
{
	var name		SocketName;
	var name		BoneName;
	var vector		RelativeLocation;
	var rotator		RelativeRotation;
	var vector		RelativeScale;
};

/** A buffer for implementing socket copy/paste. */
var const native array<SkelSocketCopyInfo>		SkelSocketPasteBuffer;

/** Global list of instanced animation compression algorithms. */
var array<AnimationCompressionAlgorithm>	AnimationCompressionAlgorithms;

/** Array of packages to be fully loaded at Editor startup. */
var config array<string> PackagesToBeFullyLoadedAtStartup;

/** class names of Kismet objects to hide in the menus (i.e. because they aren't applicable for this game) */
var config array<name> HiddenKismetClassNames;

/** Used during asset renaming/duplication to specify class-specific package/group targets. */
struct native ClassMoveInfo
{
	/** The type of asset this MoveInfo applies to. */
	var config string ClassName;
	/** The target package info which assets of this type are moved/duplicated. */
	var config string PackageName;
	/** The target group info which assets of this type are moved/duplicated. */
	var config string GroupName;
	/** If TRUE, this info is applied when moving/duplicating assets. */
	var config bool bActive;
};

/** Used during asset renaming/duplication to specify class-specific package/group targets. */
var config array<ClassMoveInfo>	ClassRelocationInfo;
