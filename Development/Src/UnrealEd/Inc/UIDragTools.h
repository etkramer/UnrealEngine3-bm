/*=============================================================================
       Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


#ifndef __UIDRAGTOOLS_H__
#define __UIDRAGTOOLS_H__


/**
 * This class creates a new instance of a UI widget based on the current tool mode and
 * resizes it until the user lets go of the mouse.
 */
class FMouseTool_ObjectDragCreate : public FMouseTool
{
public:
	/** Constructor */
	FMouseTool_ObjectDragCreate( FEditorObjectViewportClient* InObjectVC, WxUIEditorBase* InUIEditor );

	/** Destructor */
	virtual ~FMouseTool_ObjectDragCreate();

	/**
	* Creates a instance of a widget based on the current tool mode and starts the drag process.
	*
	* @param	InViewportClient	the viewport client to draw the selection box in
	* @param	Start				Where the mouse was when the drag started
	*/
	virtual void StartDrag(FEditorLevelViewportClient* InViewportClient, const FVector& InStart);

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
	void MouseMove(FViewport* Viewport, INT X, INT Y);

	/** 
	 * Generates a string that describes the current drag operation to the user.
	 * 
	 * @param OutString		Storage class for the generated string, should have the current delta in it in some way.
	 */
	virtual void GetDeltaStatusString(FString &OutString) const;

private:

	/** pointer to the UI editor object so we can create widgets */
	WxUIEditorBase* UIEditor;

	/** Widget we are currently dragging. */
	UUIObject* DragWidget;
};

/**
 * This class allows the user to drag a link from a dock handle to another widget's dock handle to create a 
 * docking relationship.
 */
class FMouseTool_DockLinkCreate : public FMouseTool
{
public:
	/** Constructor */
	FMouseTool_DockLinkCreate( FEditorObjectViewportClient* InObjectVC, WxUIEditorBase* InUIEditor, class FUIWidgetTool_Selection* InParentWidgetTool );

	/** Destructor */
	virtual ~FMouseTool_DockLinkCreate();

	/**
	 * Creates a instance of a widget based on the current tool mode and starts the drag process.
	 *
	 * @param	InViewportClient	the viewport client to draw the selection box in
	 * @param	Start				Where the mouse was when the drag started
	 */
	virtual void StartDrag(FEditorLevelViewportClient* InViewportClient, const FVector& InStart);

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
	 *	Callback allowing the drag tool to render some data to the viewport.
	 */
	virtual void Render(FCanvas* Canvas);

	/** 
	 * Generates a string that describes the current drag operation to the user.
	 * 
	 * This version blanks out the string because we do not want to display any info.
	 *
	 * @param OutString		Storage class for the generated string, should have the current delta in it in some way.
	 */
	virtual void GetDeltaStatusString(FString &OutString) const
	{
		OutString = TEXT("");
	}
protected:

	/** pointer to the UI editor object so we can create widgets */
	WxUIEditorBase* UIEditor;

	/** Widget we are currently dragging. */
	UUIObject* DragWidget;

	/** Dock face that we are dragging from. */
	EUIWidgetFace DockFace;

	/** Starting position of the link spline. */
	FIntPoint SplineStart;

	/** Current target widget for the link. */
	UUIObject* TargetWidget;

	/** Current target widget dock face for the link. */
	EUIWidgetFace TargetFace;

	class FUIWidgetTool_Selection* ParentWidgetTool;
};



/**
 * This class allows the user to drag a link from a focus chain handle to another widget, making that widget the next to get focus.
 */
class FMouseTool_FocusChainCreate : public FMouseTool
{
public:
	/** Constructor */
	FMouseTool_FocusChainCreate( FEditorObjectViewportClient* InObjectVC, WxUIEditorBase* InUIEditor, class FUIWidgetTool_FocusChain* InParentWidgetTool );

	/** Destructor */
	virtual ~FMouseTool_FocusChainCreate();

	/**
	 * Creates a instance of a widget based on the current tool mode and starts the drag process.
	 *
	 * @param	InViewportClient	the viewport client to draw the selection box in
	 * @param	Start				Where the mouse was when the drag started
	 */
	virtual void StartDrag(FEditorLevelViewportClient* InViewportClient, const FVector& InStart);

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
	 *	Callback allowing the drag tool to render some data to the viewport.
	 */
	virtual void Render(FCanvas* Canvas);

	/** 
	 * Generates a string that describes the current drag operation to the user.
	 * 
	 * This version blanks out the string because we do not want to display any info.
	 *
	 * @param OutString		Storage class for the generated string, should have the current delta in it in some way.
	 */
	virtual void GetDeltaStatusString(FString &OutString) const
	{
		OutString = TEXT("");
	}

protected:

	/** pointer to the UI editor object so we can create widgets */
	WxUIEditorBase* UIEditor;

	/** Widget we are currently dragging. */
	UUIObject* DragWidget;

	/** Dock face that we are dragging from. */
	EUIWidgetFace DragFace;

	/** Starting position of the link spline. */
	FIntPoint SplineStart;

	/** Current target widget for the link. */
	UUIObject* TargetWidget;

	class FUIWidgetTool_FocusChain* ParentWidgetTool;
};


/**
 * A mouse tool used to resize the columns or rows in the selected UIList.
 */
class FMouseTool_ResizeListCell : public FMouseTool
{
public:
	/** Constructor */
	FMouseTool_ResizeListCell( FEditorObjectViewportClient* InObjectVC, WxUIEditorBase* InUIEditor, class UUIList* InSelectedList, INT InResizeCell );

	/** Destructor */
	virtual ~FMouseTool_ResizeListCell();

	/**
	 * Begins resizing the 
	 *
	 * @param	InViewportClient	the viewport client to draw the selection box in
	 * @param	Start				Where the mouse was when the drag started
	 */
	virtual void StartDrag(FEditorLevelViewportClient* InViewportClient, const FVector& InStart);

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
	void MouseMove(FViewport* Viewport, INT X, INT Y);

	/**
	 * @return	Returns which mouse cursor to display while this tool is active.
	 */
	virtual EMouseCursor GetCursor() const;

	/** 
	 * Generates a string that describes the current drag operation to the user.
	 * 
	 * @param OutString		Storage class for the generated string, should have the current delta in it in some way.
	 */
	virtual void GetDeltaStatusString(FString &OutString) const;

private:

	/** pointer to the UI editor object so we can undo/redo this operation */
	WxUIEditorBase* UIEditor;

	class FScopedObjectStateChange* CellResizePropagator;

	/** Widget we are currently dragging. */
	class UUIList* SelectedList;

	/** the row or column being resized */
	INT ResizeCell;
};


#endif


// EOL




