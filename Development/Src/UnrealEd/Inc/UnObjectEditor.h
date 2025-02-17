/*=============================================================================
	UnObjectEditor.h: Object editor class declarations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNOBJECTEDITOR_H__
#define __UNOBJECTEDITOR_H__

// Forward declarations
struct FObjectSelectionTool;
class FDragTool_ObjectBoxSelect;
struct FMouseDragTracker;
struct HSelectionHandle;
struct FObjectViewportClick;
class FObjectEditorInterface;
class FEditorObjectViewportClient;
struct FMouseDragParameters;
class FObjectViewportModifier;
class FObjectViewportRenderModifier;
class FObjectViewportInputModifier;

const FLOAT MINIMUM_MOUSEDRAG_PIXELS = 4.f;

#define SHOW_CASE(val) case val: return TEXT(#val);

/** This enum represents the types of handles supported by the object selection tool */
enum ESelectionHandle
{
	/** INVALID */
	HANDLE_None							=0,

	/** handle for resizing the object using the left face of the selection box */
	HANDLE_Left							=0x00000001,		
	/** handle for resizing the object using the top face of the selection box */
	HANDLE_Top							=0x00000002,
	/** handle for resizing the object using the right face of the selection box */
	HANDLE_Right						=0x00000004,
	/** handle for resizing the object using the bottom face of the selection box */
	HANDLE_Bottom						=0x00000008,
	/** handle for moving the object, if no other handle is selected */
	HANDLE_Outline						=0x00000010,
	/** handle for moving the animation pivot of the object */
	HANDLE_Anchor						=0x00000020,
	/** handle for rotating the object around its pivot */
	HANDLE_Rotation						=0x00000040,

	/** handle for resizing the object using the top-left corner of the selection box */
	HANDLE_TopLeft						=HANDLE_Top|HANDLE_Left,
	/** handle for resizing the object using the top-right corner of the selection box */
	HANDLE_TopRight						=HANDLE_Top|HANDLE_Right,
	/** handle for resizing the object using the bottom-left corner of the selection box */
	HANDLE_BottomLeft					=HANDLE_Bottom|HANDLE_Left,
	/** handle for resizing the object using the bottom-right corner of the selection box */
	HANDLE_BottomRight					=HANDLE_Bottom|HANDLE_Right,
};

/**
 * Calculates information about a clicked location in the object editor
 */
struct FObjectViewportClick
{
public:
	/** Constructors */
	FObjectViewportClick();
	FObjectViewportClick(FEditorObjectViewportClient* ViewportClient,FName InKey,EInputEvent InEvent,FLOAT X,FLOAT Y);

	const FVector&	GetLocation()	const	{ return Location; }
	const FName&	GetKey()		const	{ return Key; }
	EInputEvent		GetEvent()		const	{ return Event; }

	UBOOL	IsControlDown()	const			{ return bControlDown; }
	UBOOL	IsShiftDown()	const			{ return bShiftDown; }
	UBOOL	IsAltDown()		const			{ return bAltDown; }

private:
	/** the location of the click */
	FVector		Location;
	FName		Key;
	EInputEvent	Event;
	UBOOL		bControlDown,
				bShiftDown,
				bAltDown;
};

/**
 * Abstract base class for classes which modify the viewport's appearance and/or behavior.
 */
class FObjectViewportModifier
{
public:
	/** Destructor */
	virtual ~FObjectViewportModifier() {}

	/**
	 * Called once each frame.
	 *
	 * @param	DeltaSeconds	the number of seconds that have passed since the last tick
	 */
	void Tick( FLOAT DeltaSeconds ) {}
};

/**
 * This class provides an interface for rendering additional information to the viewport.
 */
class FObjectViewportRenderModifier : public FObjectViewportModifier
{
public:

	/**
	 * Allow the viewport modifier to render to the viewport
	 */
	virtual void Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )=0;
};

/**
 * This class provides an interface for intercepting and/or overriding input received by an object viewport.
 */
class FObjectViewportInputModifier : public FObjectViewportModifier
{
public:
	/**
	 * Provides a way for viewport modifiers to perform special logic for key presses and override the viewport client's default behavior.
	 *
	 * @param	ObjectVC	the viewport client for the viewport that is processing the input
	 * @param	Key			the key that triggered the input event (KEY_LeftMouseButton, etc.)
	 * @param	Event		the type of event (press, release, etc.)
	 *
	 * @return	TRUE to override the viewport client's default handling of this event.
	 */
	virtual UBOOL HandleKeyInput(FEditorObjectViewportClient* ObjectVC, FName Key, EInputEvent Event)=0;

	/**
	 * Provides a way for viewport modifiers to override mouse movement.  This is only called when the viewport is not capturing
	 * the mouse.
	 *
	 * @param	ObjectVC	the viewport client for the viewport that is processing the input
	 * @param	X			the number of pixels the mouse was moved along the horizontal axis
	 * @param	Y			the number of pixels the mouse was moved along the vertical axis
	 *
	 * @return	TRUE to override the viewport client's default handling of this event.
	 */
	virtual UBOOL HandleMouseMovement(FEditorObjectViewportClient* ObjectVC, INT X, INT Y)=0;

	/**
	 * Notifies the viewport modifier about that mouse movement that occurs when the viewport is capturing mouse input.
	 *
	 * @param	ObjectVC		the viewport client for the viewport that is processing the input
	 * @param	DragAmount		the translated mouse movement deltas
	 * @param	bDragActive		TRUE if the mouse has been dragged more than the minimum drag amount (MINIMUM_MOUSEDRAG_PIXELS)
	 */
	virtual void OnMouseDrag( FEditorObjectViewportClient* ObjectVC, const FVector& DragAmount, UBOOL bDragActive )=0;
};

#include "UnObjectDragTools.h"

/**
 * Provides an interface for sending messages from an object editor workspace, such as
 * hit detection, input events, and "selection" of objects.
 */
class FObjectEditorInterface
{
public:
	/* =======================================
		Input methods and notifications
	   =====================================*/

	/**
	 * Displays a context menu that allows placement of new objects.
	 * Called when the user right-clicks on an empty region of the viewport.
	 */
	virtual void ShowNewObjectMenu() {}

	/**
	 * Displays a context menu for the currently selected objects.
	 * Called when the user right-clicks on an object in the viewport.
	 *
	 * @param	ClickedObject	the object that was right-clicked on
	 */
	virtual void ShowObjectContextMenu( UObject* ClickedObject ) {}

	/**
	 * Called when the user double-clicks an object in the viewport
	 *
	 * @param	Obj		the object that was double-clicked on
	 */
	virtual void DoubleClickedObject(UObject* Obj) {}

	/**
	 * Called when the user left-clicks on an empty region of the viewport
	 *
	 * @return	TRUE to deselect all selected objects
	 */
	virtual UBOOL ClickOnBackground() {return TRUE;}

	/**
	 * Called when the user releases the mouse after dragging the selection tool's rotation handle.
	 *
	 * @param	Anchor			The anchor to rotate the selected objects around.
	 * @param	DeltaRotation	the amount of rotation to apply to objects
	 */
	virtual void RotateSelectedObjects( const FVector& Anchor, const FRotator& DeltaRotation ) {}

	/**
	 * Called when the user performs a draw operation while objects are selected.
	 *
	 * @param	Delta	the number of pixels to move the objects
	 */
	virtual void MoveSelectedObjects( const FVector& Delta ) {};

	/**
	 * Called when the user performs a draw operation while objects are selected.  This version is used for editors that use
	 * the object selection tool.
	 *
	 * @param	Handle	the handle that was used to move the objects; determines whether the objects should be resized or moved.
	 * @param	Delta	the number of pixels to move the objects
	 * @param   bPreserveProportion		Whether or not we should resize objects while keeping their original proportions.
	 */
	virtual void MoveSelectedObjects( ESelectionHandle Handle, const FVector& Delta, UBOOL bPreserveProportion ) {};

	/**
	 * Hook for child classes to perform special logic for key presses and override the viewport client's default behavior.
	 *
	 * @param	Viewport	the viewport processing the input
	 * @param	Key			the key that triggered the input event (KEY_LeftMouseButton, etc.)
	 * @param	Event		the type of event (press, release, etc.)
	 *
	 * @return	TRUE to override the viewport client's default handling of this event.
	 */
	virtual UBOOL HandleKeyInput(FViewport* Viewport, FName Key, EInputEvent Event)=0;

	/**
	 * Called once when the user mouses over an new object.  Child classes should implement
	 * this function if they need to perform any special logic when an object is moused over
	 *
	 * @param	Obj		the object that is currently under the mouse cursor
	 */
	virtual void OnMouseOver(UObject* Obj) {}

	/* =======================================
		General methods and notifications
	   =====================================*/

	/**
	 * Get the package that owns the objects that are being modified in this viewport
	 */
	virtual UObject* GetPackage() const =0;

	/**
	 * Render the background for this object editor.
	 */
	virtual void DrawBackground( FViewport* Viewport, FCanvas* Canvas )=0;

	/**
	 * Child classes should implement this function to render the objects that are currently visible.
	 */
	virtual void DrawObjects(FViewport* Viewport, FCanvas* Canvas)=0;

	/**
	 * Render outlines for selected objects which are being moved / resized.
	 */
	virtual void DrawDragSelection( FCanvas* Canvas, class FMouseTool_DragSelection* DragTool ) {};

	/**
	 * Called when the viewable region is changed, such as when the user pans or zooms the workspace.
	 */
	virtual void ViewPosChanged() {}

	/**
	 * Called when something happens in the linked object drawing that invalidates the property window's
	 * current values, such as selecting a new object.
	 * Child classes should implement this function to update any property windows which are being displayed.
	 */
	virtual void UpdatePropertyWindow() {}

	/* =======================================
		Selection methods and notifications
	   =====================================*/

	/**
	 * Selects all objects within the specified region and notifies the viewport client so that it can
	 * reposition the selection overlay.
	 *
	 * @param	NewSelectionRegion	the box to use for choosing which objects to select.  Generally, all objects that fall within the bounds
	 *								of this box should be selected
	 *
	 * @return	TRUE if the selected objects were changed
	 */
	virtual UBOOL BoxSelect( FBox NewSelectionRegion )=0;

	/**
	 * Empty the list of selected objects.
	 */
	virtual void EmptySelection()
	{
		Selection->BeginBatchSelectOperation();
		Selection->DeselectAll();
		Selection->EndBatchSelectOperation();
	}

	/**
	 * Add the specified object to the list of selected objects
	 */
	virtual void AddToSelection( UObject* Obj )
	{
		Selection->Select(Obj);
	}

	/**
	 * Remove the specified object from the list of selected objects.
	 */
	virtual void RemoveFromSelection( UObject* Obj )
	{
		Selection->Deselect(Obj);
	}

	/**
	 * Toggles the selection state of the specified object.
	 */
	void ToggleSelection( UObject* Obj )
	{
		if ( IsInSelection(Obj) )
		{
			RemoveFromSelection(Obj);
		}
		else
		{
			AddToSelection(Obj);
		}
	}

	/**
	 * Checks whether the specified object is currently selected.
	 * Child classes should implement this to check for their specific
	 * object class.
	 *
	 * @return	TRUE if the specified object is currently selected
	 */
	virtual UBOOL IsInSelection( UObject* Obj ) const
	{
		return Selection->IsSelected(Obj);
	}

	/**
	 * Returns the number of selected objects
	 */
	virtual INT GetNumSelected() const
	{
		return Selection->CountSelections<UObject>();
	}

	/**
	 * Checks whether we have any objects selected.
	 */
	UBOOL HaveObjectsSelected() const { return GetNumSelected() > 0; }

	/**
	 * Determines whether handles for manipulating the selected objects should be rendered
	 */
	virtual UBOOL ShouldRenderSelectionHandles() const { return TRUE; }

	USelection*			Selection;

	/** Constructor */
	FObjectEditorInterface();

	/** Destructor */
	virtual ~FObjectEditorInterface();
};

class FEditorObjectViewportClient : public FEditorLevelViewportClient, public FCallbackEventDevice
{
public:
	FEditorObjectViewportClient( FObjectEditorInterface* InObjectEditor);
	virtual ~FEditorObjectViewportClient();

	////////////////////////////
	// FViewportClient interface

	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);

	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE);

	virtual UBOOL InputKeyUser(FName Key,EInputEvent Event) { return FALSE; }

	/**
	 * Called when the user moves the mouse while this viewport is capturing mouse input (i.e. dragging)
	 */
	virtual UBOOL InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad=FALSE);

	/**
	 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
	 */
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y);

	/**
	 * Get the cursor that should be displayed in the viewport.
	 */
	virtual EMouseCursor GetCursor(FViewport* Viewport,INT X,INT Y);


	////////////////////////////
	// FEditorLevelViewportClient interface
	virtual void Tick(FLOAT DeltaSeconds);

	/**
	 * Determines which axis InKey and InDelta most refer to and returns a corresponding FVector.  This
	 * vector represents the mouse movement translated into the viewports/widgets axis space.
	 *
	 * @param	InNudge		If 1, this delta is coming from a keyboard nudge and not the mouse
	 */
	virtual FVector TranslateDelta( FName InKey, FLOAT InDelta, UBOOL InNudge );


	////////////////////////////
	// FCallbackEventDevice interface
	virtual void Send( ECallbackEventType InType );

	////////////////////////////
	// FEditorObjectViewportClient interface

	/** Initialize this client's workers */
	virtual void InitializeClient();

	/**
	 * Marks the viewport to be redrawn.
	 */
	virtual void RefreshViewport();

	/**
	 * Register a new render modifier with this viewport client.
	 *
	 * @param Modifier		Modifier to add to the list of modifers.
	 * @param InsertIndex	Index to insert the modifer at.
	 * @return				Whether or not we could add the modifier to the list.
	 */
	virtual UBOOL AddRenderModifier( class FObjectViewportRenderModifier* Modifier, INT InsertIndex = INDEX_NONE );

	/**
	 * Remove a render modifier from this viewport client's list.
	 */
	virtual UBOOL RemoveRenderModifier( class FObjectViewportRenderModifier* Modifier );

	/**
	 * Register a new input modifier with this viewport client.
	 *
	 * @param Modifier		Modifier to add to the list of modifers.
	 * @param InsertIndex	Index to insert the modifer at.
	 * @return				Whether or not we could add the modifier to the list.
	 */
	virtual UBOOL AddInputModifier( class FObjectViewportInputModifier* Modifier, INT InsertIndex = INDEX_NONE );

	/**
	 * Remove a input modifier from this viewport client's list.
	 */
	virtual UBOOL RemoveInputModifier( class FObjectViewportInputModifier* Modifier );

	/**
	 * Called when the user clicks anywhere in the viewport.
	 *
	 * @param	HitProxy	the hitproxy that was clicked on (may be null if no hit proxies were clicked on)
	 * @param	Click		contains data about this mouse click
	 */
	virtual void ProcessClick(HHitProxy* HitProxy,const FObjectViewportClick& Click);

	/**
	 * Called when the user clicks on an empty region of the viewport.  Might clear all selected objects
	 * or display a context menu.
	 */
	virtual void ClickedBackground( const FObjectViewportClick& Click );

	/**
	 * Called when the user clicks on an object in the viewport.
	 */
	virtual void ClickedObject( const FObjectViewportClick& Click, UObject* ClickedObject );

	/**
	 * @return		Returns the background color for the viewport.
	 */
	virtual FLinearColor GetBackgroundColor();

	/**
	 * Retrieve the vectors marking the region within the viewport available for the object editor to render objects. 
	 *
	 * @param	StartPos					receives the value of the origin of the rendering region
	 * @param	EndPos						receives the value for the end point of the rendering region
	 * @param	bIncludeVirtualAdjustments	specify FALSE to ignore any gutter or offsets which may be in use by the viewport
	 */
	virtual void GetClientRenderRegion( FVector2D& StartPos, FVector2D& EndPos, UBOOL bIncludeVirtualAdjustments=TRUE );

	/**
	* Converts the mouse coordinates to viewport coordinates by factoring in viewport offset and zoom.
	*
	* @param InMouseX		Mouse X Position
	* @param InMouseY		Mouse Y Position
	* @param OutViewportX	Output variable for the Viewport X position
	* @param OutViewportY	Output variable for the Viewport Y position
	*/
	void GetViewportPositionFromMousePosition(const INT InMouseX, const INT InMouseY, FLOAT& OutViewportX, FLOAT& OutViewportY );

	// Mouse dragging methods

	/**
	 * Determine whether the required buttons are pressed to begin a box selection.
	 */
	virtual UBOOL ShouldBeginBoxSelect() const=0;

	/**
	 * Create the appropiate drag tool.  Called when the user has dragged the mouse at least the minimum number of pixels.
	 *
	 * @return	a pointer to the appropriate drag tool given the currently pressed buttons, or NULL if no drag tool should be active
	 */
	virtual FObjectDragTool* CreateCustomDragTool() { return NULL; }

	/**
	 *	Creates a mouse tool depending on the current state of the editor and what key modifiers are held down.
	 *  @param HitProxy	Pointer to the hit proxy currently under the mouse cursor.
	 *  @return Returns a pointer to the newly created FMouseTool.
	 */
	virtual FMouseTool* CreateMouseTool(const HHitProxy* HitProxy);


	/**
	 * Called from the mouse drag tracker when the user is dragging the mouse and a drag tool is not active.
	 *
	 * @param	DragAmount		the translated mouse movement deltas
	 * @param	bDragActive		TRUE if the mouse has been dragged more than the minimum drag amount (MINIMUM_MOUSEDRAG_PIXELS)
	 */
	virtual void MouseDrag( FVector& DragAmount, UBOOL bDragActive );

	/** 
	 * See if cursor is in 'scroll' region around the edge, and if so, scroll the view automatically. 
	 * Returns the distance that the view was moved.
	 */
	FIntPoint DoScrollBorder(FLOAT DeltaSeconds);



protected:
	/** Creates the object selection tool */
	virtual void CreateSelectionTool();

	/** Creates a mouse delta tracker */
	virtual void CreateMouseTracker();

public:
	/** the editor that opened this viewport client */
	FObjectEditorInterface* ObjectEditor;

	/** displays outlines and handles for the currently selected objects */
	FObjectSelectionTool* Selector;

	/** tracks mouse movement */
	FMouseDragTracker* MouseTracker;

	/** information about the last click in the viewport */
	FObjectViewportClick LastClick;

	/** modifiers that can alter the appearance of the viewport */
	TArray<FObjectViewportRenderModifier*>	RenderModifiers;

	/** modifiers that can alter the behavior of the viewport */
	TArray<FObjectViewportInputModifier*>	InputModifiers;

	/**
	 * The position in the viewport for the upper left corner of the editor window.  Always 0,0 unless zoomed in.
	 */
	FVector2D Origin2D;

	/** Currently active mouse dragging tool. */
	FMouseTool*	MouseTool;

	/**
	 * A percentage value representing the amount of zoom to apply to the editor window. 1.0 is normal zoom.
	 */
	FLOAT Zoom2D;

	/** The minimum and maximum possible values for Zoom2D */
	FLOAT MinZoom2D, MaxZoom2D;

	FVector2D ScrollAccum;

	/** Indicates whether scrolling the editor window using mouse drag is allowed */
	UBOOL bAllowScroll;
};

#endif	// __UNOBJECTEDITOR_H__
