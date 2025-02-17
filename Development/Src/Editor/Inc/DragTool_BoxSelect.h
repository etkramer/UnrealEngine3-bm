/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DRAGTOOL_BOXSELECT_H__
#define __DRAGTOOL_BOXSELECT_H__

#include "UnEdDragTools.h"

/**
 * Draws a box in the current viewport and when the mouse button is released,
 * it selects/unselects everything inside of it.
 */
class FDragTool_BoxSelect : public FDragTool
{
public:
	/**
	* Ends a mouse drag behavior (the user has let go of the mouse button).
	*/
	virtual void EndDrag();
	virtual void Render3D(const FSceneView* View,FPrimitiveDrawInterface* PDI);
};

#endif // __DRAGTOOL_BOXSELECT_H__
