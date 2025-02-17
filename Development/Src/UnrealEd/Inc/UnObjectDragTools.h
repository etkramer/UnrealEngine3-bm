/*=============================================================================
	UnObjectDragTools.h: Object editor drag helper class declarations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNOBJECTDRAGTOOLS_H__
#define __UNOBJECTDRAGTOOLS_H__

#include "UnEdDragTools.h"

// Forward declarations
struct FObjectSelectionTool;
class FMouseTool_ObjectBoxSelect;
struct FMouseDragTracker;
struct HSelectionHandle;
struct FObjectViewportClick;
class FObjectEditorInterface;
class FEditorObjectViewportClient;
struct FMouseDragParameters;
class FObjectDragTool;

/**
 * Tracks information about the 
 */
struct FMouseDragParameters
{
	BITFIELD	bAltPressed:1;
	BITFIELD	bCtrlPressed:1;
	BITFIELD	bShiftPressed:1;
	BITFIELD	bLeftMouseButtonDown:1;
	BITFIELD	bRightMouseButtonDown:1;
	BITFIELD	bMiddleMouseButtonDown:1;

	/** Constructor */
	FMouseDragParameters()
	: bAltPressed(FALSE), bShiftPressed(FALSE), bCtrlPressed(FALSE)
	, bLeftMouseButtonDown(FALSE), bRightMouseButtonDown(FALSE), bMiddleMouseButtonDown(FALSE)
	{ }

	/**
	 * Sets up the values for this struct
	 */
	void SetValues( FEditorObjectViewportClient* ViewportClient );
};

/**
 * Keeps track of mouse movement deltas in the object editor viewport.  Based on FMouseDeltaTracker.
 */
struct FMouseDragTracker
{
public:
	FMouseDragTracker();

//	void DetermineCurrentAxis( FEditorLevelViewportClient* InViewportClient );
	void StartTracking( FEditorObjectViewportClient* InViewportClient, const INT InX, const INT InY );

	/**
	 * Called when a mouse button has been released.  If there are no other
	 * mouse buttons being held down, the internal information is reset.
	 */
	UBOOL EndTracking( FEditorObjectViewportClient* InViewportClient );

	/**
	 * Resets the mouse drag tracker.  Used to release an active mouse tool without applying whatever changes the tool
	 * is designed for.
	 *
	 * @return	FALSE if there was an active drag tool that had to be reset as well; TRUE if there was no active drag tool.
	 */
	UBOOL ResetTracking();

	/**
	 * Adds delta movement into the tracker.
	 */
	void AddDelta( FEditorObjectViewportClient* InViewportClient, const FName InKey, const INT InDelta, UBOOL InNudge );
	

	/**
	 * Updates the EndSnapped variable so that it is a snapped version of End using the GridSize and GridOrigin parameters passed in.
	 * @param GridSize		Size of the grid.
	 * @param GridOrigin	Origin of the grid.
	 */
	void Snap(FLOAT GridSize, const FVector& GridOrigin);

	/**
	 * Retrieve information about the current drag delta.
	 */
	FVector GetDelta() const;
	FVector GetDeltaSnapped() const;

//	/**
//	 * Converts the delta movement to drag/rotation/scale based on the viewport type or widget axis.
//	 */
//	void ConvertMovementDeltaToDragRot( FEditorObjectViewportClient* InViewportClient, const FVector& InDragDelta, FVector& InDrag, FRotator& InRotation, FVector& InScale );
	void ReduceBy( const FVector& In );

	/** The start/end positions of the current mouse drag, snapped and unsnapped. */
	FVector Start, StartSnapped, End, EndSnapped;
	
	/**
	 * If there is a dragging tool being used, this will point to it.
	 * Gets newed/deleted in StartTracking/EndTracking.
	 */
	FObjectDragTool* DragTool;

	/** Count how many transactions we've started. */
	INT TransCount;

	/**
	 * The state of the various control buttons when this mouse drag began
	 */
	FMouseDragParameters DragParams;
};


/**
 * This tool renders the selection outline and handles for the currently selected object.
 * Design based on FWidget.
 */
struct FObjectSelectionTool : public FObjectViewportRenderModifier
{
	// FObjectViewportRenderModifier interface.
	virtual void Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );

	/** Constructor */
	FObjectSelectionTool();

	/** Destructor */
	virtual ~FObjectSelectionTool();

	/**
	 * Returns the position of a resizing handle.
	 *
	 * @param Handle	Handle to return the position for.
	 * @param OutPos	Storage vector to hold the handle's position.
	 */
	void GetHandlePos(ESelectionHandle Handle, FVector2D &OutPos);

	/** Renders this selection's handles and outlines */
	virtual void RenderSelection( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );

	/**
	 * Render the specified handle.  HANDLE_Outline and HANDLE_Anchor are ignored (rendered by RenderSelection)
	 */
	virtual void RenderHandle( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas, ESelectionHandle Handle, FLOAT Rotation );

	/**
	 * Sets the rendering offset of the anchor.  Tries to snap the anchor to various positions if its close enough, otherwise just sets the offset equal to the delta passed in.
	 *
	 * @param Delta		Offset to set the anchor's rendering offset to.  Usually this is just the change in mouse position since the user started dragging.
	 */
	void SetAnchorOffset(const FVector2D &Delta);

	/**
	 * Returns whether the user is allowed to move the anchor handle.
	 */
	virtual UBOOL IsAnchorTransformable( const TArray<class UUIObject*>& SelectedObjects ) const { return TRUE; }

	/**
	 * Sets the currently active handle, indicating that any mouse/keyboard input events should be applied to this handle.
	 */
	virtual void SetActiveHandle( ESelectionHandle Handle );

	/**
	 * Clears the active handle, indicating that mouse movements should not interact with this selection tool
	 */
	void ClearActiveHandle()
	{
		ActiveHandle = HANDLE_None;
	}

	virtual void SetPosition( FVector2D Start, FVector2D End );

	/** the location of the upper left point of this selection region */
	FVector2D StartLocation;

	/** the locaiton of the lower right of this selection region */
	FVector2D EndLocation;

	/** the location of the animation pivot for this selection region */
	FVector2D AnchorLocation;

	/** Offset from the anchor location to render the anchor at. */
	FVector2D AnchorOffset;

	/** the default size for the selection handles */
	INT HandleSize, AnchorSize;

	/** Materials and colors to be used when drawing the selection tool */
	UTexture2D *OutlineMaterial, *HandleMaterial, *AnchorMaterial;
	FColor OutlineColor, HandleColor, AnchorColor;

	/** the color for the various handles when they're being moused over */
	FColor OutlineActiveColor, HandleActiveColor, AnchorActiveColor;


	/**
	 * The currently active handle.  HANDLE_None if there is no active handle.
	 */
	ESelectionHandle ActiveHandle;
};

/**
 * A modified version of FDragTool with some extra information that is used by object editor viewports.
 */
class FObjectDragTool : public FDragTool
{
public:
	FObjectDragTool( FEditorObjectViewportClient* InObjectVC );
	virtual ~FObjectDragTool();
	/**
	* Starts a mouse drag box selection.
	*
	* @param	InViewportClient	the viewport client to draw the selection box in
	* @param	Start				Where the mouse was when the drag started
	*/
	virtual void StartDrag(FEditorLevelViewportClient* InViewportClient, const FVector& Start);

	/**
	 * Resets this drag tool's tracking variables.  Used to release an active drag tool without applying whatever changes the drag tool
	 * is designed to handle.
	 */
	virtual UBOOL ResetTracking();

	/**
	* Updates the drag tool's end location with the specified delta.  The end location is
	* snapped to the editor constraints if bUseSnapping is TRUE.
	*
	* @param	InDelta		A delta of mouse movement.
	*/
	virtual void AddDelta( const FVector& InDelta );

	/** Sets the grid origin for snapping. */
	void SetGridOrigin(const FVector& InGridOrigin)
	{
		GridOrigin = InGridOrigin;
	}

	/** Sets the grid size for snapping. */
	void SetGridSize(INT InGridSize)
	{
		GridSize = InGridSize;
	}

	/** Sets whether or not this tool should snap to grid. */
	void SetUseSnapping(UBOOL InUseSnapping)
	{
		bUseSnapping = InUseSnapping;
	}

	/** @return Distance mouse has traveled since dragging started. */
	FLOAT GetAbsoluteDistance() const
	{
		return AbsoluteDistance;
	}

	/** 
	 * Generates a string that describes the current drag operation to the user.
	 * 
	 * @param OutString		Storage class for the generated string, should have the current delta in it in some way.
	 */
	virtual void GetDeltaStatusString(FString &OutString) const;

protected:

	/** Origin of the grid, used for snapping. */
	FVector GridOrigin;

	/** Size of the grid, used for snapping. */
	INT GridSize;

	/** Distance mouse has traveled since dragging started. */
	FLOAT AbsoluteDistance;

	/** Last position of the mouse. */
	FVector LastMousePosition;

	/** pointer to the object editor interface that contains the viewport */
	class FEditorObjectViewportClient* ObjectVC;
};



/**
* This class, while similar to a drag tool, is used to let the user interact by dragging inside a viewport WITHOUT capturing
* the mouse, so the cursor is still visible.
*/
class FMouseTool : public FObjectDragTool
{
public:
	/** Constructor */
	FMouseTool( FEditorObjectViewportClient* InObjectVC );

	/**
	 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
	 */
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y);

	/**
	 * Returns the starting point of this drag operation
	 */
	FVector GetStartPoint() const
	{
		return Start;
	}

	/**
	 * Returns the end point of this drag operation
	 */
	FVector GetEndPoint() const
	{
		return End;
	}

	/**
	 * @return	Returns which mouse cursor to display while this tool is active.
	 */
	virtual EMouseCursor GetCursor() const 
	{
		return MC_Arrow;
	}
};


/**
 * Draws a box in the current viewport and when the mouse button is released,
 * it selects/unselects everything inside of it.
 */
class FMouseTool_ObjectBoxSelect : public FMouseTool
{
public:
	/** Constructor */
	FMouseTool_ObjectBoxSelect( FEditorObjectViewportClient* InObjectVC );

	/** Destructor */
	virtual ~FMouseTool_ObjectBoxSelect();

	/**
	 * Starts a mouse drag box selection.
	 *
	 * @param	InViewportClient	the viewport client to draw the selection box in
	 * @param	Start				Where the mouse was when the drag started
	 */
	virtual void StartDrag(FEditorLevelViewportClient* InViewportClient, const FVector& Start);

	/**
	 * Ends the mouse drag movement.  Called when the user releases the mouse button while this drag tool is active.
	 */
	virtual void EndDrag();

	/**
	 * Allow this drag tool to render to the screen
	 */
	virtual void Render( FCanvas* Canvas );

	/** 
	 * Generates a string that describes the current drag operation to the user.
	 * 
	 * @param OutString		Storage class for the generated string, should have the current delta in it in some way.
	 */
	virtual void GetDeltaStatusString(FString &OutString) const;
};

/**
 * This class tracks drag movements involving the object selection tool.  It is invoked when the user begins dragging
 * any of the selection tool's handles.
 */
class FMouseTool_DragSelection : public FMouseTool
{
public:
	/** Constructor */
	FMouseTool_DragSelection( FEditorObjectViewportClient* InObjectVC );

	/** Destructor */
	virtual ~FMouseTool_DragSelection();

	/**
	 * Starts a mouse drag movement.
	 *
	 * @param	InViewportClient	the viewport client to draw the selection box in
	 * @param	Start				Where the mouse was when the drag started
	 */
	virtual void StartDrag( FEditorLevelViewportClient* InViewportClient, const FVector& Start );

	/**
	 * Ends the mouse drag movement.  Called when the user releases the mouse button while this drag tool is active.
	 */
	virtual void EndDrag();

	/**
	 * Resets this drag tool's tracking variables.  Used to release an active drag tool without applying whatever changes the drag tool
	 * is designed to handle.
	 */
	virtual UBOOL ResetTracking();

	/**
	 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
	 */
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y);

	/**
	 * Allow this drag tool to render to the screen
	 */
	virtual void Render( FCanvas* Canvas );

	/**
	 * Retrieve information about the current drag delta.
	 */
	FVector GetDelta() const
	{
		return (End - Start);
	}

	/**
	 * Returns the current rotation delta.
	 */
	FLOAT GetRotationDelta() const;

	/**
	 *	Returns a angle of rotation around a anchor at a specified point.
	 *
	 *	@param	Anchor		Anchor of rotation.
	 *	@param	Point		A point that we will calculate the angle of rotation at, assuming the origin of rotation is (1,0)
	 *
	 * @return a float angle in DEGREES between 0 and 360.
	 */
	FLOAT GetRotationAngle(const FVector &Anchor, const FVector &Point) const;

	/** 
	 * Generates a string that describes the current drag operation to the user.
	 * 
	 * @param OutString		Storage class for the generated string, should have the current delta in it in some way.
	 */
	virtual void GetDeltaStatusString(FString &OutString) const;

	/**
	 * @return	Returns which mouse cursor to display while this tool is active.
	 */
	virtual EMouseCursor GetCursor() const;

	/** the handle that was active when this drag tool was invoked */
	ESelectionHandle SelectionHandle;

	/** Whether or not we are supposed to preserve proportion when resizing. */
	UBOOL bPreserveProportion;
};

/**
 * Detects mouse clicks on selection handles
 */
struct HSelectionHandle : public HHitProxy
{
	DECLARE_HIT_PROXY(HSelectionHandle,HHitProxy);

	/** the handle that is selected when this hit proxy is activated */
	ESelectionHandle		Handle;

	/** the rotation angle of the handle used in determining the appropriate mouse cursor */
	FLOAT					Rotation;

	/** Constructor */
	HSelectionHandle( ESelectionHandle InHandle, FLOAT InRotation = 0.0f )
	: HHitProxy(HPP_UI), Handle(InHandle), Rotation(InRotation)
	{ }

	/**
	* Get the cursor to use when mousing over this hit proxy.
	*/
	virtual EMouseCursor GetMouseCursor();
};


/** Controls the movement of the viewport camera. */
class FDragTool_MoveCamera : public FObjectDragTool
{
public:
	/** Constructor */
	FDragTool_MoveCamera( FEditorObjectViewportClient* InObjectVC );

	/**
	 * Starts a mouse drag behavior.  The start location is snapped to the editor constraints if bUseSnapping is TRUE.
	 *
	 * @param	InViewportClient	The viewport client in which the drag event occurred.
	 * @param	InStart				Where the mouse was when the drag started.
	 */
	virtual void StartDrag(FEditorLevelViewportClient* InViewportClient, const FVector& InStart);

	/**
	* Updates the drag tool's end location with the specified delta.  The end location is
	* snapped to the editor constraints if bUseSnapping is TRUE.
	*
	* @param	InDelta		A delta of mouse movement.
	*/
	virtual void AddDelta( const FVector& InDelta );

private:
	enum EMoveMode
	{
		MM_Translate,
		MM_Zoom
	};

	EMoveMode MoveMode;
};

#endif // __UNOBJECTDRAGTOOLS_H__
