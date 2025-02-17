/**
 * This class handles hotkey binding management for the editor.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class EditorUserSettings extends Object
	hidecategories(Object)
	config(EditorUserSettings)
	native;


/** True if WASD keys should be remapped to flight controls while the right mouse button is held down */
var(Options) config bool AllowFlightCameraToRemapKeys;
/** The background color for material preview thumbnails in Generic Browser  */
var(Options) config Color PreviewThumbnailBackgroundColor;
/** The background color for translucent material preview thumbnails in Generic Browser */
var(Options) config Color PreviewThumbnailTranslucentMaterialBackgroundColor;
