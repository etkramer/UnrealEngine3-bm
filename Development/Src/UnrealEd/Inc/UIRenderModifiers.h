/*=============================================================================
     Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UIRENDERMODIFIERS_H__
#define __UIRENDERMODIFIERS_H__

// Docking Handle Constants
namespace FDockingConstants
{
	static const INT HandleRadius = 8;
	static const INT HandleGap = 8;
	static const INT HandleNumSides = 16;
};

/**
 * This class renders the overlay lines and dock handles for selected widgets. 
 */
class FRenderModifier_SelectedWidgetsOutline : public FUIEditor_RenderModifier
{
public:
	/**
	 * Constructor
	 *
	 * @param	InEditor	the editor window that holds the container to render the outline for
	 */
	FRenderModifier_SelectedWidgetsOutline( class WxUIEditorBase* InEditor );

	/**
	 * Renders the outline for the top-level object of a UI editor window.
	 */
	virtual void Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );

	/** Sets the selected widget for highlighting. */
	void SetSelectedWidget(UUIObject* InWidget);

	/** Sets the selected dock face for highlighting. */
	virtual void SetSelectedDockFace(EUIWidgetFace InFace);

	/** Clears the selected dock face and dock widget. */
	virtual void ClearSelections();
	/**
	 *	@return Returns the currently selected(highlighted) widget, if any.
	 */
	UUIObject* GetSelectedWidget() const
	{
		return SelectedWidget;
	}

	/**
	 *	@return Returns the currently selected(highlighted) dock handle.
	 */
	EUIWidgetFace GetSelectedDockHandle() const
	{
		return SelectedDockHandle;
	}

	/**
	 * Method for serializing UObject references that must be kept around through GC's.
	 *
	 * @param Ar The archive to serialize with
	 */
	virtual void Serialize(FArchive& Ar);

	/** 
	 * Generates a dock handle position for a object.
	 * 
	 * @param Widget				Widget we are getting a dock handle position for.
	 * @param InFace				Which face we are getting a position for.
	 * @param HandleRadius			Radius of the handle.
	 * @param HandleGap				Gap between the handle and the object's edge.
	 * @param OutPosition			Position of the dock handle.
	 */
	void GetDockHandlePosition(UUIObject* Widget, EUIWidgetFace InFace, INT HandleRadius, INT HandleGap, FIntPoint& OutPosition );

	/** 
	 * Generates a dock handle position for a scene.
	 * 
	 * @param Scene					Scene we are getting a dock handle position for.
	 * @param InFace				Which face we are getting a position for.
	 * @param HandleRadius			Radius of the handle.
	 * @param HandleGap				Gap between the handle and the object's edge.
	 * @param OutPosition			Position of the dock handle.
	 */
	void GetDockHandlePosition(UUIScene* Scene, EUIWidgetFace InFace, INT HandleRadius, INT HandleGap, FIntPoint& OutPosition);

	/** 
	 * Generates a handle position for a object.
	 * 
	 * @param RenderBounds	Render bounds of the object we are getting a dock handle for.
	 * @param InFace		Which face we are getting a position for.
	 * @param HandleRadius	Radius of the handle.
	 * @param HandleGap		Gap between the handle and the object's edge.
	 * @param OutPosition	Position of the dock handle.
	 */
	void GetHandlePosition(const FLOAT* RenderBounds, EUIWidgetFace InFace, INT HandleRadius, INT HandleGap, FIntPoint& OutPosition);

protected:

	/** 
	 * Renders a docking handle for the specified widget.
	 * 
	 * @param Widget	Widget to render the docking handle for.
	 * @param Face		Which face we are drawing the handle for.
	 * @param RI		Rendering interface to use for drawing the handle.
	 */
	virtual void RenderDockHandle( UUIObject* Widget, EUIWidgetFace InFace, FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );



	/** 
	 * Renders a docking link for ths specified face.
	 * 
	 * @param Widget					Widget to render the docking handle for.
 	 * @param InFace					Which face we are drawing the handle for.
 	 * @param RI						Rendering interface to use for drawing the handle.
	 * @param Start					Starting position for the link.
	 * @param DockHandleRadius		Radius for the docking handles.
	 * @param DockHandleGap			Gap between the handles and the edge of the widget.
	 */
	void RenderDockLink( UUIObject* Widget, EUIWidgetFace InFace, FCanvas* Canvas, const FVector& Start, INT DockHandleRadius, INT DockHandleGap );

	/** Member variable to store which widget is currently moused over. */
	UUIObject* SelectedWidget;

	/** Member variable to store which dock handle is currently moused over. */
	EUIWidgetFace SelectedDockHandle;

	/** whether this render modifier should render dock handles for the selected widgets */
	UBOOL bRenderDockHandles;
};

/**
 * This class renders the overlay lines and dock handles for a dock link target widget.
 */
class FRenderModifier_TargetWidgetOutline : public FRenderModifier_SelectedWidgetsOutline
{
public:
	struct FWidgetDockHandle
	{
		UUIObject* Widget;
		EUIWidgetFace Face;
		FIntPoint Position;
		UBOOL bValidTarget;

		// Used for sorting
		static INT Compare(FWidgetDockHandle* A, FWidgetDockHandle* B)
		{
			if(A->Position.X < B->Position.X)
			{
				return -1;
			}
			else if( A->Position.X > B->Position.X )
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	};

	struct FWidgetDockHandles
	{
		/** Redundant pointer to the Widget so we can quickly toggle visible of all of its 4 dock handles. */
		UUIObject* Widget;
		
		/** Widget Dock Handles */
		FWidgetDockHandle DockHandles[4];

		/** Whether or not this widget's dock handles are visible. */
		UBOOL bVisible;
	};

	/**
	* Constructor
	*
	* @param	InEditor	the editor window that holds the container to render the outline for
	*/
	FRenderModifier_TargetWidgetOutline( class WxUIEditorBase* InEditor );
	~FRenderModifier_TargetWidgetOutline();

	/** 
	 * Generates a list of all of the dock handles in the scene and sorts them by X position. 
	 *
	 * @param SourceWidget	The widget we are connecting a dock link from.
	 * @param SourceFace	The face we are connecting from.
	 */
	void GenerateSortedDockHandleList(UUIObject* SourceWidget, EUIWidgetFace SourceFace);

	/** Deallocates the sorted dock handle list. */
	void FreeSortedDockHandleList();

	/**
	 *	@return Returns the list of sorted dock handles for the scene.
	 */
	const TArray<FWidgetDockHandle*>* GetSortedDockHandleList() const
	{
		return &SortedDockHandles;
	}

	/**
	 * Generates a list of visible dock handles from the widgets passed in.  It combines the handles when they are too close to each other.
	 *
	 * @param Widgets A list of widgets to use for generating visible dock handles.
	 */
	void CalculateVisibleDockHandles(TArray<UUIObject*> &Widgets);

	/** Toggles all of the dock handles to not be visible */
	void ClearVisibleDockHandles();

	/**
	 *	Calculates dock handles which are close to the currently selected widget dock handle.
	 *  @return			The number of dock handles in the specified radius.
	 */
	TArray<FRenderModifier_TargetWidgetOutline::FWidgetDockHandle*>* CalculateCloseDockHandles();

	/**
	 * @return Returns the last generated list of close dock handles (CalculateCloseDockHandles must be called prior to this).
	 */
	TArray<FRenderModifier_TargetWidgetOutline::FWidgetDockHandle*>* GetCloseDockHandles()
	{
		return &CloseHandles;
	}

	/** Sets the selected dock face for highlighting. */
	virtual void SetSelectedDockFace(EUIWidgetFace InFace);

	/** Clears the selected dock face and dock widget. */
	virtual void ClearSelections();

	/**
	* Method for serializing UObject references that must be kept around through GC's.
	*
	* @param Ar The archive to serialize with
	*/
	virtual void Serialize(FArchive& Ar);

	/**
	* Renders all of the visible target dock handles.
	*/
	virtual void Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );

protected:

	/** 
	 * Renders a docking handle for the specified widget.
	 * 
	 * @param Widget	Widget to render the docking handle for.
	 * @param Face		Which face we are drawing the handle for.
	 * @param RI		Rendering interface to use for drawing the handle.
	 */
	virtual void RenderDockHandle( const FWidgetDockHandle& DockHandle, FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );

	TArray<FWidgetDockHandles*> SceneWidgetDockHandles;	
	TArray<FWidgetDockHandle*> SortedDockHandles;	
	TArray<FWidgetDockHandle*> CloseHandles;
};


/**
 * This class renders the overlay lines and focus chain handles for a selected widget.
 */
class FRenderModifier_FocusChainHandles : public FRenderModifier_SelectedWidgetsOutline
{
public:
	/**
	 * Constructor
	 *
	 * @param	InEditor	the editor window that holds the container to render the outline for
	 */
	FRenderModifier_FocusChainHandles( class WxUIEditorBase* InEditor );

	/**
	 * Renders the outline for the top-level object of a UI editor window.
	 */
	virtual void Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );

	/** Sets the selected dock face for highlighting. */
	virtual void SetSelectedDockFace(EUIWidgetFace InFace);

private:
	/** 
	 * Renders a focus chain link for ths specified face.
	 * 
	 * @param Widget				Widget to render the docking handle for.
	 * @param InFace				Which face we are drawing the handle for.
	 * @param RI					Rendering interface to use for drawing the handle.
	 * @param Start					Starting position for the link.
	 */
	void RenderLink( UUIObject* Widget, EUIWidgetFace InFace, FCanvas* Canvas, const FVector& InStart);

	/** 
	 * Renders a focus chain handle for the specified widget.
	 * 
	 * @param Widget	Widget to render the focus chain handle for.
	 * @param Face		Which face we are drawing the handle for.
	 * @param RI		Rendering interface to use for drawing the handle.
	 */
	virtual void RenderFocusChainHandle( UUIObject* Widget, EUIWidgetFace InFace, FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );

};


/**
 * This class renders the overlay lines and dock handles for a dock link target widget.
 */
class FRenderModifier_FocusChainTarget : public FUIEditor_RenderModifier
{
public:

	/**
	 * Constructor
	 *
	 * @param	InEditor	the editor window that holds the container to render the outline for
	 */
	FRenderModifier_FocusChainTarget( class WxUIEditorBase* InEditor );


	/**
	 * Sets the list of widgets that we currently have as targets (ie under the mouse cursor)
	 * @param Widgets	Currently targeted widgets
	 */
	void SetTargetWidgets(TArray<UUIObject*> &Widgets);

	/** Sets the selected widget for highlighting. */
	void SetSelectedWidget(UUIObject* InWidget)
	{
		SelectedWidget = InWidget;
	}

	/** @return Returns the top-most widget that is currently under the mouse cursor. */
	const UUIObject* GetSelectedWidget() const
	{
		return SelectedWidget;
	}

	/** @return Returns all of the widgets that are currently under the mouse cursor. */
	TArray<UUIObject*>& GetTargetWidgets()
	{
		return TargetWidgets;
	}

	/** Clears the selected widgets. */
	virtual void ClearSelections();

	/**
	 * Method for serializing UObject references that must be kept around through GC's.
	 *
	 * @param Ar The archive to serialize with
	 */
	virtual void Serialize(FArchive& Ar);

	/**
	 * Renders all of the visible target dock handles.
	 */
	virtual void Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas );

protected:

	/** Current object directly under the mouse cursor (The hitproxy object). */
	UUIObject* SelectedWidget;

	/** All objects under the mouse cursor. */
	TArray<UUIObject*> TargetWidgets;
};






#endif


// EOL





