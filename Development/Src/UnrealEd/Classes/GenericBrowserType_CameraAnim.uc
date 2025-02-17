/**
 * Generic browser type for Camera animations
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GenericBrowserType_CameraAnim extends GenericBrowserType
	native;

cpptext
{
	/**
	 * Initialize the supported classes for this browser type.
	 */
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );
}

DefaultProperties
{
	Description="Camera Animation"
}
