/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UIWIDGETTOOL_H__
#define __UIWIDGETTOOL_H__

/* ==========================================================================================================
	Includes
========================================================================================================== */
#include "UIRenderModifiers.h"	// Selection and Focus Chain tool


/**
 *	UI Editor Widget Tool, used for selection tool, widget creation tool, etc.
 */
class FUIWidgetTool
{
public:
	FUIWidgetTool(class WxUIEditorBase *InEditor) : 
    UIEditor(InEditor)
    {

    }
	virtual ~FUIWidgetTool() {}

	/** Called when this tool is 'activated'. */
	virtual void EnterToolMode()
	{
		// Intentionally empty.
	}

	/** Called when this tool is 'deactivated'. */
	virtual void ExitToolMode()
	{
		// Intentionally empty.
	}

	/** Creates a mouse tool for the current widget mode. */
	virtual FMouseTool* CreateMouseTool() const
	{
		return NULL;
	}

	/**
	 * Check a key event received by the viewport.
	 * If the viewport client uses the event, it should return true to consume it.
	 * @param	Viewport - The viewport which the key event is from.
	 * @param	ControllerId - The controller which the key event is from.
	 * @param	Key - The name of the key which an event occured for.
	 * @param	Event - The type of event which occured.
	 * @param	AmountDepressed - For analog keys, the depression percent.
	 * @return	True to consume the key event, false to pass it on.
	 */
	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed)
	{
		return FALSE;
	}

	/**
	 * Called when the user clicks anywhere in the viewport.
	 *
	 * @param	HitProxy	the hitproxy that was clicked on (may be null if no hit proxies were clicked on)
	 * @param	Click		contains data about this mouse click
	 * @return				Whether or not we consumed the click.
	 */
	virtual UBOOL ProcessClick (HHitProxy* HitProxy, const FObjectViewportClick& Click)
	{
		return FALSE;
	}

	/**
	 * Called when the user moves the mouse while this viewport is capturing mouse input (i.e. dragging)
	 */
	virtual void InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime)
	{
		// Intentionally empty.
	}

	/**
	 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
	 */
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y)
	{
		// Intentionally empty.
	}

protected:

	class WxUIEditorBase *UIEditor;
};


/* ==========================================================================================================
	"Widget Selection" Mouse Mode - moving, resizing, rotating widgets.
========================================================================================================== */

class WxSelectionToolProxy : public wxWindow
{
public:
	WxSelectionToolProxy(wxWindow* InParent, class FUIWidgetTool_Selection* InParentTool, class WxUIEditorBase* InEditor);

protected:
	/**
	 * Breaks the dock link starting from the handle that was right clicked.
	 */
	void OnContext_BreakDockLink( wxCommandEvent& Event );

	/**
	 * Breaks all of the dock links for the widget that was right clicked.
	 */
	void OnContext_BreakAllDockLinks( wxCommandEvent& Event );

	/**
	 * Connects the selected dock link.
	 */
	void OnContext_ConnectDockLink( wxCommandEvent& Event );

	/**
	 * Shows the docking editor.
	 */
	void OnContext_DockingEditor( wxCommandEvent& Event );

	/** Tool that spawned this menu. */
	class FUIWidgetTool_Selection* ParentTool;

	/** UIEditor that owns the parent tool. */
	class WxUIEditorBase* UIEditor;

	DECLARE_EVENT_TABLE()
};

/**
 *	UI Editor Widget Tool, this is the default manipulate widget tool.
 */
class FUIWidgetTool_Selection : public FUIWidgetTool, public FSerializableObject
{
public:

	FUIWidgetTool_Selection(class WxUIEditorBase* InEditor);
	virtual ~FUIWidgetTool_Selection() {}

	/**
	 * Method for serializing UObject references that must be kept around through GC's.
	 * 
	 * @param Ar The archive to serialize with
	 */
	virtual void Serialize(FArchive& Ar);

	/** Called when this tool is 'activated'. */
	virtual void EnterToolMode();

	/** Called when this tool is 'deactivated'. */
	virtual void ExitToolMode();

	/**
	 * Check a key event received by the viewport.
	 * If the viewport client uses the event, it should return true to consume it.
	 * @param	Viewport - The viewport which the key event is from.
	 * @param	ControllerId - The controller which the key event is from.
	 * @param	Key - The name of the key which an event occured for.
	 * @param	Event - The type of event which occured.
	 * @param	AmountDepressed - For analog keys, the depression percent.
	 * @return	True to consume the key event, false to pass it on.
	 */
	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed);

	/**
	 * Called when the user clicks anywhere in the viewport.
	 *
	 * @param	HitProxy	the hitproxy that was clicked on (may be null if no hit proxies were clicked on)
	 * @param	Click		contains data about this mouse click
	 * @return				Whether or not we consumed the click.
	 */
	virtual UBOOL ProcessClick (HHitProxy* HitProxy, const FObjectViewportClick& Click);

	/**
	 * Called when the user moves the mouse while this viewport is capturing mouse input (i.e. dragging)
	 */
	virtual void InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime);

	/**
	 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
	 */
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y);

	/** displays outlines and docking handles around all selected widgets. */
	class FRenderModifier_SelectedWidgetsOutline	SelectedWidgetsOutline;

	/** Displays docking handles and an outline for a target widget. */
	class FRenderModifier_TargetWidgetOutline		TargetWidgetOutline;

	/** Proxy to handle receiving wx events. */
	class WxSelectionToolProxy* MenuProxy;

protected:
	/**
	 * Called when the user right-clicks on a dock handle in the viewport, shows a context menu for dock handles.
	 */
	void ShowDockHandleContextMenu();
};

/* ==========================================================================================================
	"Edit Focus Chain" Mouse Mode - assign manual navigation links between widgets
========================================================================================================== */
class WxFocusChainToolProxy : public wxWindow
{
public:
	WxFocusChainToolProxy(wxWindow* InParent, class FUIWidgetTool_FocusChain* InParentTool);

protected:
	/**
	 * Breaks the focus chain link starting from the handle that was right clicked.
	 */
	void OnContext_BreakFocusChainLink(wxCommandEvent& Event);

	/**
	 * Breaks all of the focus chain links connected to the current object.
	 */
	void OnContext_BreakAllFocusChainLinks(wxCommandEvent& Event);

	/**
	 * Sets a focus chain to NULL.
	 */
	void OnContext_SetFocusChainToNull(wxCommandEvent& Event);
	
	/**
 	 * Sets all focus chains to NULL.
	 */
	void OnContext_SetAllFocusChainsToNull(wxCommandEvent& Event);

	/**
	 * Connects the selected focus chain link.
	 */
	void OnContext_ConnectFocusChainLink( wxCommandEvent& Event );

	/** Tool that spawned this menu. */
	class FUIWidgetTool_FocusChain* ParentTool;

	DECLARE_EVENT_TABLE()
};


/**
 *	UI Editor Widget Tool, this is the default manipulate widget tool.
 */
class FUIWidgetTool_FocusChain : public FUIWidgetTool, public FSerializableObject
{
public:

	FUIWidgetTool_FocusChain(class WxUIEditorBase* InEditor);

	/** Called when this tool is 'activated'. */
	virtual void EnterToolMode();

	/** Called when this tool is 'deactivated'. */
	virtual void ExitToolMode();

	/**
	 * Check a key event received by the viewport.
	 * If the viewport client uses the event, it should return true to consume it.
	 * @param	Viewport - The viewport which the key event is from.
	 * @param	ControllerId - The controller which the key event is from.
	 * @param	Key - The name of the key which an event occured for.
	 * @param	Event - The type of event which occured.
	 * @param	AmountDepressed - For analog keys, the depression percent.
	 * @return	True to consume the key event, false to pass it on.
	 */
	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed);

	/**
	 * Called when the user clicks anywhere in the viewport.
	 *
	 * @param	HitProxy	the hitproxy that was clicked on (may be null if no hit proxies were clicked on)
	 * @param	Click		contains data about this mouse click
	 * @return				Whether or not we consumed the click.
	 */
	virtual UBOOL ProcessClick (HHitProxy* HitProxy, const FObjectViewportClick& Click);

	/**
	 * Called when the user moves the mouse while this viewport is capturing mouse input (i.e. dragging)
	 */
	virtual void InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime);

	/**
	 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
	 */
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y);


	/**
	 * Method for serializing UObject references that must be kept around through GC's.
	 *
	 * @param Ar The archive to serialize with
	 */
	virtual void Serialize(FArchive& Ar);


	/** Render modifier that draws the focus chain handles for selected widgets. */
	class FRenderModifier_FocusChainHandles RenderModifier;

	/** Render modifier that displays which widgets the focus chain mouse tool is targeting. */
	class FRenderModifier_FocusChainTarget TargetRenderModifier;

	/** Proxy to receive wx menu events. */
	WxFocusChainToolProxy* MenuProxy;

protected:
	/**
	 * Called when the user right-clicks on a dock handle in the viewport, shows a context menu for dock handles.
	 */
	void ShowFocusChainHandleContextMenu();
};

/* ==========================================================================================================
	"Create Widget" Mouse Mode - add widgets at the mouse location and set widget's initial size
	by dragging the mouse
========================================================================================================== */
/**
 *	UI Editor Widget Tool, this is the widget creation tool.
 */
class FUIWidgetTool_CustomCreate : public FUIWidgetTool_Selection
{
public:

	FUIWidgetTool_CustomCreate(class WxUIEditorBase* InEditor, INT InWidgetIdx);

	/** Called when this tool is 'activated'. */
	virtual void EnterToolMode();

	/** Called when this tool is 'deactivated'. */
	virtual void ExitToolMode();

	/** Creates a mouse tool for the current widget mode. */
	virtual FMouseTool* CreateMouseTool() const;

	/**
	 * Check a key event received by the viewport.
	 * If the viewport client uses the event, it should return true to consume it.
	 * @param	Viewport - The viewport which the key event is from.
	 * @param	ControllerId - The controller which the key event is from.
	 * @param	Key - The name of the key which an event occured for.
	 * @param	Event - The type of event which occured.
	 * @param	AmountDepressed - For analog keys, the depression percent.
	 * @return	True to consume the key event, false to pass it on.
	 */
	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed);

	/**
	 * Called when the user clicks anywhere in the viewport.
	 *
	 * @param	HitProxy	the hitproxy that was clicked on (may be null if no hit proxies were clicked on)
	 * @param	Click		contains data about this mouse click
	 * @return				Whether or not we consumed the click.
	 */
	virtual UBOOL ProcessClick (HHitProxy* HitProxy, const FObjectViewportClick& Click);

	/**
	 * Called when the user moves the mouse while this viewport is capturing mouse input (i.e. dragging)
	 */
	virtual void InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime);

	/**
	 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
	 */
	virtual void MouseMove(FViewport* Viewport, INT X, INT Y);

protected:

	/** Widget index, this is the widget type we will be creating. */
	INT WidgetIdx;
};


/* ==========================================================================================================
	Widget specified mouse mode - context sensitive mouse mode for performing more interactive editing of
	specific widget types.  Not yet implemented.
========================================================================================================== */
/**
 * Base class for all widget custom editor mode tools.  T must be a class derived from UIScreenObject.
 */
template< class T >
class TUIWidgetTool_WidgetEditor : public FUIWidgetTool, public FSerializableObject
{
public:
	/* === FUIWidgetTool_WidgetEditor interface === */
	/**
	 * Constructor
	 *
	 * @param	InEditor	the UI editor window that contains our widget
	 */
	TUIWidgetTool_WidgetEditor( class WxUIEditorBase* InEditor )
	: FUIWidgetTool(InEditor), FSerializableObject(), CurrentWidget(NULL)
	{
		// we shouldn't be able to invoke the widget editor tool if we don't have exactly one widget selected
		check(InEditor->GetNumSelected() == 1);
		
		TArray<UUIObject*> SelectedWidgets;
		InEditor->GetSelectedWidgets(SelectedWidgets);

		// this widget editor tool shouldn't be invoked if the widget wasn't of the correct type.
		CurrentWidget = CastChecked<T>(SelectedWidgets(0));
	}

	/* === FSerializableObject interface === */
	/**
	 * Method for serializing UObject references that must be kept around through GC's.
	 * 
	 * @param Ar The archive to serialize with
	 */
	virtual void Serialize(FArchive& Ar)
	{
		Ar << (UObject*&)CurrentWidget;
	}

protected:
	/**
	 * The widget being edited by this widget editor mode tool.
	 */
	T* CurrentWidget;
};

/**
 * Custom tool mode editor for UIList widgets.
 */
class FUIWidgetTool_ListEditor : public TUIWidgetTool_WidgetEditor<UUIList>
{
public:
	/* === FUIWidgetTool_ListEditor interface === */
	/**
	 * Constructor
	 *
	 * @param	InEditor	the UI editor window that contains our widget
	 */
	FUIWidgetTool_ListEditor( class WxUIEditorBase* InEditor );

	/* === FUIWidgetTool_WidgetEditor interface === */
	/** Called when this tool is 'activated'. */
	virtual void EnterToolMode();

	/** Called when this tool is 'deactivated'. */
	virtual void ExitToolMode();
	//@todo
};

#endif



// EOF



