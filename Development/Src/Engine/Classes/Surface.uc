/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Surface extends Object
	native
	abstract;

native noexport final function float GetSurfaceWidth();
native noexport final function float GetSurfaceHeight();

cpptext
{
	virtual FLOAT GetSurfaceWidth() const PURE_VIRTUAL(USurface::GetSurfaceWidth,return 0;);
	virtual FLOAT GetSurfaceHeight() const PURE_VIRTUAL(USurface::GetSurfaceHeight,return 0;);
}