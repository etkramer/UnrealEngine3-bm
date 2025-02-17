/*=============================================================================
	UnLinkedObjEditor.h: Base class for boxes-and-lines editing
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNLINKEDOBJEDITOR_H__
#define __UNLINKEDOBJEDITOR_H__

#include "TrackableWindow.h"

/**
 * Provides notifications of messages sent from a linked object editor workspace.
 */
class FLinkedObjEdNotifyInterface
{
public:
	/* =======================================
		Input methods and notifications
	   =====================================*/

	/**
	 * Called when the user right-clicks on an empty region of the viewport
	 */
	virtual void OpenNewObjectMenu() {}

	/**
	 * Called when the user right-clicks on an object in the viewport
	 */
	virtual void OpenObjectOptionsMenu() {}

	virtual void ClickedLine(struct FLinkedObjectConnector &Src, struct FLinkedObjectConnector &Dest) {}
	virtual void DoubleClickedLine(struct FLinkedObjectConnector &Src, struct FLinkedObjectConnector &Dest) {}

	/**
	 * Called when the user double-clicks an object in the viewport
	 *
	 * @param	Obj		the object that was double-clicked on
	 */
	virtual void DoubleClickedObject(UObject* Obj) {}

	/**
	 * Called whent he use double-clicks an object connector.
	 */
	virtual void DoubleClickedConnector(struct FLinkedObjectConnector& Connector) {}
																			   
	/**
	 * Called when the user left-clicks on an empty region of the viewport
	 *
	 * @return	TRUE to deselect all selected objects
	 */
	virtual UBOOL ClickOnBackground() {return TRUE;}

	/**
	 *	Called when the user left-clicks on a connector.
	 *
	 *	@return TRUE to deselect all selected objects
	 */
	virtual UBOOL ClickOnConnector(UObject* InObj, INT InConnType, INT InConnIndex) {return TRUE;}

	/**
	 * Called when the user performs a draw operation while objects are selected.
	 *
	 * @param	DeltaX	the X value to move the objects
	 * @param	DeltaY	the Y value to move the objects
	 */
	virtual void MoveSelectedObjects( INT DeltaX, INT DeltaY )=0;

	/**
	 * Hook for child classes to perform special logic for key presses.  Called for all key events, event
	 * those which cause other functions to be called (such as ClickOnBackground, etc.)
	 */
	virtual void EdHandleKeyInput(FViewport* Viewport, FName Key, EInputEvent Event)=0;

	/**
	 * Called once when the user mouses over an new object.  Child classes should implement
	 * this function if they need to perform any special logic when an object is moused over
	 *
	 * @param	Obj		the object that is currently under the mouse cursor
	 */
	virtual void OnMouseOver(UObject* Obj) {}

	/**
	 * Called when the user clicks something in the workspace that uses a special hit proxy (HLinkedObjProxySpecial),
	 * Hook for preventing AddToSelection/SetSelectedConnector from being called for the clicked element.
	 *
	 * @return	FALSE to prevent any selection methods from being called for the clicked element
	 */
	virtual UBOOL SpecialClick( INT NewX, INT NewY, INT SpecialIndex, FViewport* Viewport ) { return 0; }

	/**
	 * Called when the user drags an object that uses a special hit proxy (HLinkedObjProxySpecial)
	 * (corresponding method for non-special objects is MoveSelectedObjects)
	 */
	virtual void SpecialDrag( INT DeltaX, INT DeltaY, INT NewX, INT NewY, INT SpecialIndex ) {}

	/* =======================================
		General methods and notifications
	   =====================================*/

	/**
	 * Child classes should implement this function to render the objects that are currently visible.
	 */
	virtual void DrawObjects(FViewport* Viewport, FCanvas* Canvas)=0;

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

	/**
	 * Called when one or more objects changed
	 */
	virtual void NotifyObjectsChanged() {}

	/**
	 * Called once the user begins a drag operation.  Child classes should implement this method
	 * if they the position of the selected objects is part of the object's state (and thus, should
	 * be recorded into the transaction buffer)
	 */
	virtual void BeginTransactionOnSelected() {}

	/**
	 * Called when the user releases the mouse button while a drag operation is active.
	 * Child classes should implement this only if they also implement BeginTransactionOnSelected.
	 */
	virtual void EndTransactionOnSelected() {}

	/* =======================================
		Selection methods and notifications
	   =====================================*/

	/**
	 * Empty the list of selected objects.
	 */
	virtual void EmptySelection() {}

	/**
	 * Add the specified object to the list of selected objects
	 */
	virtual void AddToSelection( UObject* Obj ) {}

	/**
	 * Remove the specified object from the list of selected objects.
	 */
	virtual void RemoveFromSelection( UObject* Obj ) {}

	/**
	 * Checks whether the specified object is currently selected.
	 * Child classes should implement this to check for their specific
	 * object class.
	 *
	 * @return	TRUE if the specified object is currently selected
	 */
	virtual UBOOL IsInSelection( UObject* Obj ) const { return FALSE; }

	/**
	 * Returns the number of selected objects
	 */
	virtual INT GetNumSelected() const { return 0; }

	/**
	 * Checks whether we have any objects selected.
	 */
	UBOOL HaveObjectsSelected() const { return GetNumSelected() > 0; }

	/**
	 * Called once the mouse is released after moving objects.
	 * Snaps the selected objects to the grid.
	 */
	virtual void PositionSelectedObjects() {}

	/**
	 * Called when the user right-clicks on an object connector in the viewport.
	 */
	virtual void OpenConnectorOptionsMenu() {}

	/**
	 * Child classes should implement this function to render the lines between linked objects.
	 */
	virtual UBOOL DrawCurves() { return false; }

	// Selection-related methods

	/**
	 * Called when the user clicks on an unselected link connector.
	 *
	 * @param	Connector	the link connector that was clicked on
	 */
	virtual void SetSelectedConnector( struct FLinkedObjectConnector& Connector ) {}

	/**
	 * Gets the position of the selected connector.
	 *
	 * @return	an FIntPoint that represents the position of the selected connector, or (0,0)
	 *			if no connectors are currently selected
	 */
	virtual FIntPoint GetSelectedConnLocation(FCanvas* Canvas) { return FIntPoint(0,0); }

	/**
	 * Gets the EConnectorHitProxyType for the currently selected connector
	 *
	 * @return	the type for the currently selected connector, or 0 if no connector is selected
	 */
	virtual INT GetSelectedConnectorType() { return 0; }

	/**
	 * Called when the user mouses over a linked object connector.
	 * Checks whether the specified connector should be highlighted (i.e. whether this connector is valid for linking)
	 */
	virtual UBOOL ShouldHighlightConnector(struct FLinkedObjectConnector& Connector) { return TRUE; }

	/**
	 * Gets the color to use for drawing the pending link connection.  Only called when the user
	 * is connecting a link to a link connector.
	 */
	virtual FColor GetMakingLinkColor() { return FColor(0,0,0); }

	/**
	 * Called when the user releases the mouse over a link connector during a link connection operation.
	 * Make a connection between selected connector and an object or another connector.
	 *
	 * @param	Connector	the connector corresponding to the endpoint of the new link
	 */
	virtual void MakeConnectionToConnector( struct FLinkedObjectConnector& Connector ) {}

	/**
	 * Called when the user releases the mouse over an object during a link connection operation.
	 * Makes a connection between the current selected connector and the specified object.
	 *
	 * @param	Obj		the object target corresponding to the endpoint of the new link connection
	 */
	virtual void MakeConnectionToObject( UObject* Obj ) {}

	/**
	 * Called when the user releases the mouse over a link connector and is holding the ALT key.
	 * Commonly used as a shortcut to breaking connections.
	 *
	 * @param	Connector	The connector that was ALT+clicked upon.
	 */
	virtual void AltClickConnector(struct FLinkedObjectConnector& Connector) {}
};

class FLinkedObjViewportClient : public FEditorLevelViewportClient
{
public:
	FLinkedObjViewportClient( FLinkedObjEdNotifyInterface* InEdInterface );

	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);
	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE);
	virtual UBOOL InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad=FALSE);
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y);
	virtual void Tick(FLOAT DeltaSeconds);

	virtual EMouseCursor GetCursor(FViewport* Viewport,INT X,INT Y);

	/** 
	 * See if cursor is in 'scroll' region around the edge, and if so, scroll the view automatically. 
	 * Returns the distance that the view was moved.
	 */
	FIntPoint DoScrollBorder(FLOAT DeltaSeconds);

	/**
	 * Sets whether or not the viewport should be invalidated in Tick().
	 */
	void SetRedrawInTick(UBOOL bInAlwaysDrawInTick);

	FLinkedObjEdNotifyInterface* EdInterface;

	FIntPoint Origin2D;
	FLOAT Zoom2D;
	FLOAT MinZoom2D, MaxZoom2D;
	INT NewX, NewY; // Location for creating new object

	INT OldMouseX, OldMouseY;
	FVector2D BoxOrigin2D;
	INT BoxStartX, BoxStartY;
	INT BoxEndX, BoxEndY;
	INT DistanceDragged;
	FLOAT DeltaXFraction;	// Fractional delta X, which can add up when zoomed in
	FLOAT DeltaYFraction;	// Fractional delta Y, which can add up when zoomed in

	FVector2D ScrollAccum;

	UBOOL bTransactionBegun;
	UBOOL bMouseDown;

	UBOOL bMakingLine;
	UBOOL bBoxSelecting;
	UBOOL bSpecialDrag;
	UBOOL bAllowScroll;

protected:

	/** Handle mouse over events */
	void OnMouseOver( INT X, INT Y );

	/** If TRUE, invert mouse panning behaviour. */
	UBOOL bInvertMousePan;

	/** If TRUE, invalidate the viewport in Tick().  Used for mouseover support, etc. */
	UBOOL bAlwaysDrawInTick;

public:
	// For mouse over stuff.
	UObject* MouseOverObject;
	INT MouseOverConnType;
	INT MouseOverConnIndex;
	DOUBLE MouseOverTime;

	INT SpecialIndex;
};

/*-----------------------------------------------------------------------------
	WxLinkedObjVCHolder
-----------------------------------------------------------------------------*/

class WxLinkedObjVCHolder : public wxWindow
{
public:
	WxLinkedObjVCHolder( wxWindow* InParent, wxWindowID InID, FLinkedObjEdNotifyInterface* InEdInterface );
	~WxLinkedObjVCHolder();

	void OnSize( wxSizeEvent& In );

	FLinkedObjViewportClient* LinkedObjVC;

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxLinkedObjEd
-----------------------------------------------------------------------------*/

class WxLinkedObjEd : public WxTrackableFrame, public FNotifyHook, public FLinkedObjEdNotifyInterface, public FSerializableObject, public FDockingParent
{
public:
	/** Default constructor for two-step dynamic window creation */
	WxLinkedObjEd( wxWindow* InParent, wxWindowID InID, const TCHAR* InWinName );
	virtual ~WxLinkedObjEd();

	/**
	 * This function is called when the window has been selected from within the ctrl + tab dialog.
	 */
	virtual void OnSelected();

    /**
     * Creates the controls for this window.  If bTreeControl is TRUE, the bottom portion of
	 * the screen is split with a property window on the left and a tree control on the right.
	 * If bTreeControl is FALSE, the bottom portion of the screen will contain only a property window.
	 *
	 * @param	bTreeControl	If TRUE, create a property window and tree control; If FALSE, create just a property window.
     */
	virtual void CreateControls( UBOOL bTreeControl );
	virtual void InitEditor()=0;

	/** 
	 * Saves Window Properties
	 */ 
	void SaveProperties();

	/**
	 * Loads Window Properties
	 */
	void LoadProperties();

	/**
	 * @return Returns the name of the inherited class, so we can generate .ini entries for all LinkedObjEd instances.
	 */
	virtual const TCHAR* GetConfigName() const = 0;

	void OnSize( wxSizeEvent& In );
	void RefreshViewport();

	// FLinkedObjEdNotifyInterface interface
	virtual void DrawObjects(FViewport* Viewport, FCanvas* Canvas);
	virtual UBOOL DrawCurves() { return bDrawCurves; }

	/**
	 * Creates the tree control for this linked object editor.  Only called if TRUE is specified for bTreeControl
	 * in the constructor.
	 *
	 * @param	TreeParent	the window that should be the parent for the tree control
	 */
	virtual void CreateTreeControl( wxWindow* TreeParent );


	// FNotifyHook interface

	/**
	 * Called when the property window is destroyed.
	 *
	 * @param	Src		a pointer to the property window being destroyed
	 */
	virtual void NotifyDestroy( void* Src );

	/**
	 * Called when a property value has been changed, before the new value has been applied.
	 *
	 * @param	Src						a pointer the property window containing the property that was changed
	 * @param	PropertyAboutToChange	a pointer to the UProperty that is being changed
	 */
	virtual void NotifyPreChange( void* Src, UProperty* PropertyAboutToChange );

	/**
	 * Called when a property value has been changed, after the new value has been applied.
	 *
	 * @param	Src						a pointer the property window containing the property that was changed
	 * @param	PropertyThatChanged		a pointer to the UProperty that has been changed
	 */
	virtual void NotifyPostChange( void* Src, UProperty* PropertyThatChanged );

	/**
	 * Called when the the propert window wants to send an exec command.  Seems to be currently unused.
	 */
	virtual void NotifyExec( void* Src, const TCHAR* Cmd );

	/**
	 * Used to serialize any UObjects contained that need to be to kept around.
	 *
	 * @param Ar The archive to serialize with
	 */
	virtual void Serialize(FArchive& Ar);

	WxPropertyWindow* PropertyWindow;

	/** Container window for the 'graph' viewport.  This is usually a dockable window that wraps LinkedObjVC. */
	WxLinkedObjVCHolder* GraphWindow;

	FLinkedObjViewportClient* LinkedObjVC;

	wxTreeCtrl* TreeControl;
	wxImageList* TreeImages;
	UTexture2D*	BackgroundTexture;
	FString WinNameString;
	UBOOL bDrawCurves;

protected:
	/**
	 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
	 *  @return A string representing a name to use for this docking parent.
	 */
	virtual const TCHAR* GetDockingParentName() const;

	/**
	 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
	 */
	virtual const INT GetDockingParentVersion() const;

	DECLARE_EVENT_TABLE()
};

#endif	// __UNLINKEDOBJEDITOR_H__
