/*=============================================================================
	UIWidgetTool.cpp: Implementation for standard widget editor modes
        Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/* ==========================================================================================================
	Includes
========================================================================================================== */
#include "UnrealEd.h"
#include "EngineUIPrivateClasses.h"

#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UIWidgetTool.h"
#include "UIDragTools.h"

#include "ScopedTransaction.h"
#include "ScopedObjectStateChange.h"


/* ==========================================================================================================
	"Widget Selection" Mouse Mode - moving, resizing, rotating widgets.
========================================================================================================== */
BEGIN_EVENT_TABLE(WxSelectionToolProxy, wxWindow)
	EVT_MENU_RANGE(IDM_UI_DOCKTARGET_BEGIN, IDM_UI_DOCKTARGET_END, WxSelectionToolProxy::OnContext_ConnectDockLink)
	EVT_MENU(IDM_UI_BREAKDOCKLINK, WxSelectionToolProxy::OnContext_BreakDockLink)
	EVT_MENU(IDM_UI_BREAKALLDOCKLINKS, WxSelectionToolProxy::OnContext_BreakAllDockLinks)
	EVT_MENU(IDM_UI_DOCKINGEDITOR, WxSelectionToolProxy::OnContext_DockingEditor)
END_EVENT_TABLE()

WxSelectionToolProxy::WxSelectionToolProxy(wxWindow* InParent, FUIWidgetTool_Selection* InParentTool, class WxUIEditorBase* InEditor) : 
wxWindow(InParent, wxID_ANY),
ParentTool(InParentTool),
UIEditor(InEditor)
{

}

/**
 * Breaks the dock link starting from the handle that was right clicked.
 */
void WxSelectionToolProxy::OnContext_BreakDockLink(wxCommandEvent& Event )
{
	FScopedTransaction ScopedTransaction( *LocalizeUI(TEXT("UIEditor_MenuItem_BreakDockLink")) );

	
	WxUIHandleContextMenu* Menu = wxDynamicCast(Event.GetEventObject(), WxUIHandleContextMenu);
	UUIObject* Widget = Menu->ActiveWidget;
	EUIWidgetFace DockFace = Menu->ActiveFace;

	FScopedObjectStateChange DockingChangeNotification(Widget);

	Menu->ActiveWidget->SetDockTarget(DockFace, NULL, UIFACE_MAX);
	UIEditor->RefreshPositionPanelValues();
	UIEditor->RefreshDockingPanelValues();
}


/**
 * Breaks the dock link starting from the handle that was right clicked.
 */
void WxSelectionToolProxy::OnContext_BreakAllDockLinks(wxCommandEvent& Event )
{
	FScopedTransaction ScopedTransaction( *LocalizeUI(TEXT("UIEditor_MenuItem_BreakDockLink")) );
	WxUIHandleContextMenu* Menu = wxDynamicCast(Event.GetEventObject(), WxUIHandleContextMenu);
	UUIObject* Widget = Menu->ActiveWidget;

	FScopedObjectStateChange NavTargetChangeNotification(Widget);
	Menu->ActiveWidget->SetDockTarget(UIFACE_Top, NULL, UIFACE_MAX);
	Menu->ActiveWidget->SetDockTarget(UIFACE_Bottom, NULL, UIFACE_MAX);
	Menu->ActiveWidget->SetDockTarget(UIFACE_Left, NULL, UIFACE_MAX);
	Menu->ActiveWidget->SetDockTarget(UIFACE_Right, NULL, UIFACE_MAX);
	UIEditor->RefreshPositionPanelValues();
	UIEditor->RefreshDockingPanelValues();
}


/**
 * Connects the selected dock link.
 */
void WxSelectionToolProxy::OnContext_ConnectDockLink( wxCommandEvent& Event )
{
	// Get the HandleIdx from the event's ID, and then make sure it is still a valid index into the sorted dock handle array.
	// If it is, set the dock target for the selected widget using that info.

	const INT HandleIdx = Event.GetId() - IDM_UI_DOCKTARGET_BEGIN;
	const TArray<FRenderModifier_TargetWidgetOutline::FWidgetDockHandle*>* Handles = ParentTool->TargetWidgetOutline.GetCloseDockHandles();
	const UBOOL bValidId = HandleIdx < Handles->Num();
	if( bValidId == TRUE)
	{
		const FRenderModifier_TargetWidgetOutline::FWidgetDockHandle* Handle = (*Handles)(HandleIdx);
		UUIObject* SelectedWidget = ParentTool->SelectedWidgetsOutline.GetSelectedWidget();
		const EUIWidgetFace SelectedFace = ParentTool->SelectedWidgetsOutline.GetSelectedDockHandle();
		UUIScreenObject* TargetWidget = Handle->Widget;
		const EUIWidgetFace TargetFace = Handle->Face;

		if(SelectedWidget != NULL && SelectedFace != UIFACE_MAX)
		{
			FScopedTransaction ScopedTransaction( *LocalizeUI(TEXT("UIEditor_MenuItem_ConnectDockLink")) );
			FScopedObjectStateChange NavTargetChangeNotification(SelectedWidget);

			if(TargetWidget == NULL && TargetFace != UIFACE_MAX)
			{
				TargetWidget = UIEditor->OwnerScene;
			}


			SelectedWidget->SetDockTarget(SelectedFace, TargetWidget, TargetFace);
			SelectedWidget->UpdateScene();
			UIEditor->RefreshPositionPanelValues();
			UIEditor->RefreshDockingPanelValues();
		}
	}
}

void WxSelectionToolProxy::OnContext_DockingEditor( wxCommandEvent& Event )
{
	// Forward this event to our parent.
	Event.SetEventObject(GetParent());

	GetParent()->ProcessEvent(Event);
}

/** 
 * FUIWidgetTool_Selection
 */
FUIWidgetTool_Selection::FUIWidgetTool_Selection(class WxUIEditorBase* InEditor) : 
FUIWidgetTool(InEditor),
SelectedWidgetsOutline(InEditor),
TargetWidgetOutline(InEditor),
MenuProxy(NULL)
{ 
}

/**
 * Method for serializing UObject references that must be kept around through GC's.
 * 
 * @param Ar The archive to serialize with
 */
void FUIWidgetTool_Selection::Serialize(FArchive& Ar)
{
	SelectedWidgetsOutline.Serialize(Ar);
	TargetWidgetOutline.Serialize(Ar);
}


/** Called when this tool is 'activated'. */
void FUIWidgetTool_Selection::EnterToolMode()
{
	// Add render modifiers.

	// add a modifier for rendering the dock handles and non-interactive outline for the currently selected widget.  Make sure the 
	// modifier is inserted into the list below the Selector (which renders the resize and movements handles) because the docking handles
	// are larger than the resize handles.
	const INT InsertIndex = UIEditor->ObjectVC->RenderModifiers.FindItemIndex(UIEditor->ObjectVC->Selector);
	UIEditor->ObjectVC->AddRenderModifier(&SelectedWidgetsOutline, InsertIndex);

	// insert a modifier for rendering the dock handles and non-interactive outline for widgets that are being moused over when the user
	// is attempting to connect a dock link.
	UIEditor->ObjectVC->AddRenderModifier(&TargetWidgetOutline);

	// Add a proxy to receive wx events
	MenuProxy = new WxSelectionToolProxy(UIEditor, this, UIEditor);
}

/** Called when this tool is 'deactivated'. */
void FUIWidgetTool_Selection::ExitToolMode()
{
	// Remove render modifiers.
	UIEditor->ObjectVC->RemoveRenderModifier(&TargetWidgetOutline);
	UIEditor->ObjectVC->RemoveRenderModifier(&SelectedWidgetsOutline);


	// We need to remove the menu proxy from its parent so we can be deleted safely.
	if(MenuProxy != NULL)
	{
		MenuProxy->Destroy();
		MenuProxy = NULL;
	}
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
UBOOL FUIWidgetTool_Selection::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed)
{
	UBOOL bConsumeInput = FALSE;

	// See if we are interacting with a docking widget handle, if so we handle all input here.
	if( Key == KEY_LeftMouseButton )
	{
		if(Event == IE_Pressed)
		{
			const INT MouseX = Viewport->GetMouseX();
			const INT MouseY = Viewport->GetMouseY();

			// See if they clicked on a dock handle, if so, let start the link dragging process.
			HHitProxy* HitProxy = Viewport->GetHitProxy(MouseX, MouseY);

			if(HitProxy != NULL)
			{
				if(HitProxy->IsA(HUIDockHandleHitProxy::StaticGetType()) == TRUE && SelectedWidgetsOutline.GetSelectedWidget() != NULL)
				{
					HUIDockHandleHitProxy* DockHandleProxy = (HUIDockHandleHitProxy*)(HitProxy);

					UIEditor->ObjectVC->MouseTool= new FMouseTool_DockLinkCreate(UIEditor->ObjectVC, UIEditor, this);

					UIEditor->TargetTooltip->ClearStringList();

					// Tell the MouseTool to start dragging.
					FLOAT HitX;
					FLOAT HitY;
					UIEditor->ObjectVC->GetViewportPositionFromMousePosition(MouseX, MouseY, HitX, HitY);
					UIEditor->ObjectVC->MouseTool->StartDrag(UIEditor->ObjectVC, FVector(HitX, HitY, 0));

					bConsumeInput = TRUE;
				}
			}
		}
	}

	return bConsumeInput;
}

/**
 * Called when the user clicks anywhere in the viewport.
 *
 * @param	HitProxy	the hitproxy that was clicked on (may be null if no hit proxies were clicked on)
 * @param	Click		contains data about this mouse click
 * @return				Whether or not we consumed the click.
 */
UBOOL FUIWidgetTool_Selection::ProcessClick (HHitProxy* HitProxy, const FObjectViewportClick& Click)
{
	UBOOL bHandledClick = FALSE;

	if( Click.GetKey() == KEY_RightMouseButton)
	{
		// See if they right clicked on a widget dock handle, if so, display the dock handle
		// context menu.
		if(HitProxy != NULL)
		{
			if(HitProxy->IsA(HUIDockHandleHitProxy::StaticGetType()) == TRUE)
			{
				HUIDockHandleHitProxy* DockHandleProxy = (HUIDockHandleHitProxy*)(HitProxy);

				ShowDockHandleContextMenu();
				bHandledClick = TRUE;
			}
		}
	}

	return bHandledClick;
}

/**
 * Called when the user moves the mouse while this viewport is capturing mouse input (i.e. dragging)
 */
void FUIWidgetTool_Selection::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime)
{
	if(UIEditor->ObjectVC->MouseTool != NULL)
	{
		// If we are dragging, we keep track of a 'target' widget and face for the mouse tools.
		UBOOL bClearTarget = TRUE;

		INT X = Viewport->GetMouseX();
		INT Y = Viewport->GetMouseY();
		HHitProxy* HitProxy = Viewport->GetHitProxy(X, Y);

		if(HitProxy != NULL)
		{
			// We first check to see if we have moused over a widget.  
			if(HitProxy->IsA(HUIDockHandleHitProxy::StaticGetType()) == TRUE)
			{
				HUIDockHandleHitProxy* DockHandleProxy = (HUIDockHandleHitProxy*)(HitProxy);
				UUIObject* Widget = DockHandleProxy->UIObject;

				const UUIObject* SelectedWidget = SelectedWidgetsOutline.GetSelectedWidget();
				const EUIWidgetFace DockHandle = DockHandleProxy->DockFace;

				// Make sure we aren't setting the target widget to the same widget as the selected widget.
				if(SelectedWidget != NULL && SelectedWidget != Widget && DockHandle != UIFACE_MAX)
				{	
					TargetWidgetOutline.SetSelectedWidget(Widget);
					TargetWidgetOutline.SetSelectedDockFace(DockHandle);

					bClearTarget = FALSE;
				}
			}
		}

		// See if we moused over the scene or any widgets...
		if(bClearTarget == TRUE)
		{
			// Build a list of widgets currently under the mouse cursor, we will add this to our 
			// target widget outline container.
			const UUIObject* SelectedWidget = SelectedWidgetsOutline.GetSelectedWidget();

			const FVector2D Point(X, Y);
			TArray<UUIObject*> Children = UIEditor->OwnerScene->GetChildren( TRUE );
			TArray<UUIObject*> HitWidgets;

			for(INT ChildIdx = 0; ChildIdx < Children.Num(); ChildIdx++)
			{
				UUIObject* ChildWidget = Children(ChildIdx);
				const UBOOL bUnderMouse = ChildWidget->ContainsPoint(Point);

				if( bUnderMouse && SelectedWidget != ChildWidget )
				{
					HitWidgets.AddItem(ChildWidget);
				}
			}

			// Check to see if the user is moused over the scene extents.
			if(UIEditor->OwnerScene->ContainsPoint(Point) == TRUE)
			{
				HitWidgets.AddUniqueItem(NULL);
			}

			// Make sure we have atleast one target widget under us that isn't the selected widget.
			if(HitWidgets.Num() > 0)
			{	
				TargetWidgetOutline.CalculateVisibleDockHandles(HitWidgets);

				// Clear the dock handle member because we are not on one.
				TargetWidgetOutline.SetSelectedWidget(NULL);
				TargetWidgetOutline.SetSelectedDockFace(UIFACE_MAX);	

				bClearTarget = FALSE;
			}
		}

		// Clear the target widget if nothing was moused over.
		if( bClearTarget == TRUE )
		{	
			TargetWidgetOutline.ClearSelections();
		}
	}
	else
	{
		TargetWidgetOutline.ClearSelections();
	}
}

/**
 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
 */
void FUIWidgetTool_Selection::MouseMove(FViewport* Viewport, INT X, INT Y)
{
	if(UIEditor->ObjectVC->MouseTool == NULL)
	{
		// Use hitproxy's to see if we are over any dock handles, if we are then set the
		// render modifier state to reflect the fact that the handle is highlighted.
		HHitProxy* HitProxy = Viewport->GetHitProxy(X, Y);
		UBOOL bClearSelections = TRUE;

		if(HitProxy != NULL)
		{
			if(HitProxy->IsA(HUIDockHandleHitProxy::StaticGetType()) == TRUE)
			{
				HUIDockHandleHitProxy* DockHandleProxy = (HUIDockHandleHitProxy*)(HitProxy);

				SelectedWidgetsOutline.SetSelectedWidget(DockHandleProxy->UIObject);
				SelectedWidgetsOutline.SetSelectedDockFace(DockHandleProxy->DockFace);

				bClearSelections = FALSE;
			}
		}

		if(bClearSelections == TRUE)
		{
			SelectedWidgetsOutline.ClearSelections();
		}
	}
}

/**
 * Called when the user right-clicks on a dock handle in the viewport, shows a context menu for dock handles.
 */
void FUIWidgetTool_Selection::ShowDockHandleContextMenu( )
{
	UUIObject* SelectedWidget = SelectedWidgetsOutline.GetSelectedWidget();
	EUIWidgetFace SelectedFace = SelectedWidgetsOutline.GetSelectedDockHandle();

	const UBOOL bValidWidget = (SelectedWidget != NULL);
	const UBOOL bValidFace = (SelectedFace != UIFACE_MAX);

	if(bValidWidget && bValidFace)
	{
		UIEditor->SynchronizeSelectedWidgets(UIEditor);

		WxUIHandleContextMenu menu(MenuProxy);
		menu.Create( SelectedWidget, SelectedFace );
		menu.Append( IDM_UI_BREAKDOCKLINK, *LocalizeUI(TEXT("UIEditor_MenuItem_BreakDockLink")) );
		menu.Append( IDM_UI_BREAKALLDOCKLINKS, *LocalizeUI(TEXT("UIEditor_MenuItem_BreakAllDockLinks")) );
		menu.AppendSeparator();
		menu.Append( IDM_UI_DOCKINGEDITOR, *LocalizeUI(TEXT("UIEditor_MenuItem_DockingEditor")) );

		// If we do not have a dock link on this dock handle, then disable the break dock link item.
		// Otherwise, enable the item and generate a label for the menu item that says which widget/face we are breaking the link to.
		UUIObject* TargetWidget = SelectedWidget->DockTargets.GetDockTarget(SelectedFace);
		EUIWidgetFace TargetFace = SelectedWidget->DockTargets.GetDockFace(SelectedFace);

		if(!SelectedWidget->DockTargets.IsDocked(SelectedFace))
		{
			menu.SetLabel(IDM_UI_BREAKDOCKLINK, *LocalizeUI(TEXT("UIEditor_MenuItem_BreakDockLink")));
			menu.Enable(IDM_UI_BREAKDOCKLINK, false);
		}
		else
		{
			FString DockLabel;

			UIEditor->GetDockHandleString(TargetWidget, TargetFace, DockLabel);

			FString MenuLabel = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_MenuItem_BreakDockLinkTo")), *DockLabel));
			menu.SetLabel(IDM_UI_BREAKDOCKLINK, *MenuLabel);
			menu.Enable(IDM_UI_BREAKDOCKLINK, true);
		}

		FTrackPopupMenu tpm( MenuProxy, &menu );
		tpm.Show();
	}
}

/* ==========================================================================================================
	"Edit Focus Chain" Mouse Mode - assign manual navigation links between widgets
========================================================================================================== */
BEGIN_EVENT_TABLE(WxFocusChainToolProxy, wxWindow)
	EVT_MENU_RANGE(IDM_UI_DOCKTARGET_BEGIN, IDM_UI_DOCKTARGET_END, WxFocusChainToolProxy::OnContext_ConnectFocusChainLink)
	
	EVT_MENU(IDM_UI_SETFOCUSCHAINTONULL, WxFocusChainToolProxy::OnContext_SetFocusChainToNull)
	EVT_MENU(IDM_UI_SETALLFOCUSCHAINTONULL, WxFocusChainToolProxy::OnContext_SetAllFocusChainsToNull)

	EVT_MENU(IDM_UI_BREAKFOCUSCHAINLINK, WxFocusChainToolProxy::OnContext_BreakFocusChainLink)
	EVT_MENU(IDM_UI_BREAKALLFOCUSCHAINLINKS, WxFocusChainToolProxy::OnContext_BreakAllFocusChainLinks)
END_EVENT_TABLE()

WxFocusChainToolProxy::WxFocusChainToolProxy(wxWindow* InParent, FUIWidgetTool_FocusChain* InParentTool) : 
wxWindow(InParent, wxID_ANY),
ParentTool(InParentTool)
{
}

/**
 * Breaks the focus chain link starting from the handle that was right clicked.
 */
void WxFocusChainToolProxy::OnContext_BreakFocusChainLink(wxCommandEvent& Event)
{
	FScopedTransaction ScopedTransaction( *LocalizeUI(TEXT("UIEditor_MenuItem_BreakFocusChainLink")) );
	WxUIHandleContextMenu* Menu = wxDynamicCast(Event.GetEventObject(), WxUIHandleContextMenu);

	UUIObject* Widget = Menu->ActiveWidget;
	EUIWidgetFace Face = Menu->ActiveFace;

	FScopedObjectStateChange NavTargetChangeNotification(Widget);
	Widget->SetForcedNavigationTarget(Face, NULL);
}

/**
 * Breaks all of the focus chain links connected to the current object.
 */
void WxFocusChainToolProxy::OnContext_BreakAllFocusChainLinks(wxCommandEvent& Event)
{
	FScopedTransaction ScopedTransaction( *LocalizeUI(TEXT("UIEditor_MenuItem_BreakFocusChainLink")) );
	WxUIHandleContextMenu* Menu = wxDynamicCast(Event.GetEventObject(), WxUIHandleContextMenu);

	UUIObject* Widget = Menu->ActiveWidget;

	FScopedObjectStateChange NavTargetChangeNotification(Widget);
	Widget->SetForcedNavigationTarget(UIFACE_Top, NULL, FALSE);
	Widget->SetForcedNavigationTarget(UIFACE_Bottom, NULL, FALSE);
	Widget->SetForcedNavigationTarget(UIFACE_Left, NULL, FALSE);
	Widget->SetForcedNavigationTarget(UIFACE_Right, NULL, FALSE);
}

/**
 * Sets a focus chain to NULL.
 */
void WxFocusChainToolProxy::OnContext_SetFocusChainToNull(wxCommandEvent& Event)
{
	FScopedTransaction ScopedTransaction( *LocalizeUI(TEXT("UIEditor_MenuItem_BreakFocusChainLink")) );
	WxUIHandleContextMenu* Menu = wxDynamicCast(Event.GetEventObject(), WxUIHandleContextMenu);

	UUIObject* Widget = Menu->ActiveWidget;
	EUIWidgetFace Face = Menu->ActiveFace;

	FScopedObjectStateChange NavTargetChangeNotification(Widget);
	Widget->SetForcedNavigationTarget(Face, NULL, TRUE);
}

/**
 * Sets all focus chains to NULL.
 */
void WxFocusChainToolProxy::OnContext_SetAllFocusChainsToNull(wxCommandEvent& Event)
{
	FScopedTransaction ScopedTransaction( *LocalizeUI(TEXT("UIEditor_MenuItem_BreakFocusChainLink")) );
	WxUIHandleContextMenu* Menu = wxDynamicCast(Event.GetEventObject(), WxUIHandleContextMenu);

	UUIObject* Widget = Menu->ActiveWidget;

	FScopedObjectStateChange NavTargetChangeNotification(Widget);
	Widget->SetForcedNavigationTarget(UIFACE_Top, NULL, TRUE);
	Widget->SetForcedNavigationTarget(UIFACE_Left, NULL, TRUE);
	Widget->SetForcedNavigationTarget(UIFACE_Right, NULL, TRUE);
	Widget->SetForcedNavigationTarget(UIFACE_Bottom, NULL, TRUE);
}

/**
 * Connects the selected dock link.
 */
void WxFocusChainToolProxy::OnContext_ConnectFocusChainLink(wxCommandEvent& Event)
{
	// Get the HandleIdx from the event's ID, and then make sure it is still a valid index into the sorted dock handle array.
	// If it is, set the dock target for the selected widget using that info.
	const INT WidgetIdx = Event.GetId() - IDM_UI_DOCKTARGET_BEGIN;
	const TArray<UUIObject*>& TargetWidgets = ParentTool->TargetRenderModifier.GetTargetWidgets();
	const UBOOL bValidId = WidgetIdx < TargetWidgets.Num();
	if( bValidId == TRUE)
	{
		const UUIObject* Widget = TargetWidgets(WidgetIdx);
		UUIObject* SelectedWidget = ParentTool->RenderModifier.GetSelectedWidget();
		const EUIWidgetFace SelectedFace = ParentTool->RenderModifier.GetSelectedDockHandle();

		if(SelectedWidget != NULL && SelectedFace != UIFACE_MAX)
		{
			FScopedTransaction ScopedTransaction( *LocalizeUI(TEXT("UIEditor_MenuItem_ConnectFocusChainLink")) );
			FScopedObjectStateChange NavTargetChangeNotification(SelectedWidget);
			SelectedWidget->SetForcedNavigationTarget(SelectedFace,const_cast<UUIObject*>(Widget));
	    }
    }
}




/* ==========================================================================================================
FUIWidgetTool_FocusChain
========================================================================================================== */
FUIWidgetTool_FocusChain::FUIWidgetTool_FocusChain(class WxUIEditorBase* InEditor) : 
FUIWidgetTool(InEditor),
RenderModifier(InEditor),
TargetRenderModifier(InEditor)
{

}


/** Called when this tool is 'activated'. */
void FUIWidgetTool_FocusChain::EnterToolMode()
{
	// add a modifier for rendering the focus chain links (little purple diamonds).  We insert it into the list before the modifier that
	// renders the focus chain target so that the focus chain target outline is always rendered over the focus chain source's outline.  Also,
	// make sure the modifier is inserted into the list below the Selector (which renders the resize and movements handles) because
	// the focus chain handles are larger than the resize handles.
	const INT InsertIndex = UIEditor->ObjectVC->RenderModifiers.FindItemIndex(UIEditor->ObjectVC->Selector);
	UIEditor->ObjectVC->AddRenderModifier(&RenderModifier, InsertIndex);

	// insert a modifier for rendering the outline for widgets that are being moused over when the user is attempting to create a focus
	// link.
	UIEditor->ObjectVC->AddRenderModifier(&TargetRenderModifier);

	// Add a proxy to receive wx events
	MenuProxy = new WxFocusChainToolProxy(UIEditor, this);
}

/** Called when this tool is 'deactivated'. */
void FUIWidgetTool_FocusChain::ExitToolMode()
{
	UIEditor->ObjectVC->RemoveRenderModifier(&TargetRenderModifier);
	UIEditor->ObjectVC->RemoveRenderModifier(&RenderModifier);

	// We need to remove the menu proxy from its parent so we can be deleted safely.
	if(MenuProxy != NULL)
	{
		MenuProxy->Destroy();
		MenuProxy = NULL;
	}
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
UBOOL FUIWidgetTool_FocusChain::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed)
{
	UBOOL bConsumeInput = FALSE;

	// See if we are interacting with a docking widget handle, if so we handle all input here.
	if( Key == KEY_LeftMouseButton )
	{
		if(Event == IE_Pressed)
		{
			const INT MouseX = Viewport->GetMouseX();
			const INT MouseY = Viewport->GetMouseY();

			// See if they clicked on a dock handle, if so, let start the link dragging process.
			HHitProxy* HitProxy = Viewport->GetHitProxy(MouseX, MouseY);

			if(HitProxy != NULL)
			{
				if(HitProxy->IsA(HUIFocusChainHandleHitProxy::StaticGetType()) == TRUE && RenderModifier.GetSelectedWidget() != NULL)
				{
					HUIFocusChainHandleHitProxy* DockHandleProxy = (HUIFocusChainHandleHitProxy*)(HitProxy);

					UIEditor->ObjectVC->MouseTool= new FMouseTool_FocusChainCreate(UIEditor->ObjectVC, UIEditor, this);
					UIEditor->TargetTooltip->ClearStringList();

					// Tell the MouseTool to start dragging.
					FLOAT HitX;
					FLOAT HitY;
					UIEditor->ObjectVC->GetViewportPositionFromMousePosition(MouseX, MouseY, HitX, HitY);
					UIEditor->ObjectVC->MouseTool->StartDrag(UIEditor->ObjectVC, FVector(HitX, HitY, 0));

					bConsumeInput = TRUE;
				}
			}
		}
	}

	return bConsumeInput;
}

/**
 * Called when the user clicks anywhere in the viewport.
 *
 * @param	HitProxy	the hitproxy that was clicked on (may be null if no hit proxies were clicked on)
 * @param	Click		contains data about this mouse click
 * @return				Whether or not we consumed the click.
 */
UBOOL FUIWidgetTool_FocusChain::ProcessClick (HHitProxy* HitProxy, const FObjectViewportClick& Click)
{
	UBOOL bHandledClick = FALSE;

	if( Click.GetKey() == KEY_RightMouseButton)
	{
		// See if they right clicked on a widget dock handle, if so, display the dock handle
		// context menu.
		if(HitProxy != NULL)
		{
			if(HitProxy->IsA(HUIFocusChainHandleHitProxy::StaticGetType()) == TRUE)
			{
				HUIFocusChainHandleHitProxy* DockHandleProxy = (HUIFocusChainHandleHitProxy*)(HitProxy);

				ShowFocusChainHandleContextMenu();
				bHandledClick = TRUE;
			}
		}
	}

	return bHandledClick;
}

/**
 * Called when the user moves the mouse while this viewport is capturing mouse input (i.e. dragging)
 */
void FUIWidgetTool_FocusChain::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime)
{
	if(UIEditor->ObjectVC->MouseTool != NULL)
	{
		// If we are dragging, we keep track of a 'target' widget and face for the mouse tools.
		UBOOL bClearTarget = TRUE;

		INT X = Viewport->GetMouseX();
		INT Y = Viewport->GetMouseY();
		HHitProxy* HitProxy = Viewport->GetHitProxy(X, Y);

		if(HitProxy != NULL)
		{
			if(HitProxy->IsA(HObject::StaticGetType()) == TRUE)
			{
				HUIHitProxy* UIProxy = (HUIHitProxy*)HitProxy;
				UUIObject* Widget = Cast<UUIObject>(UIProxy->UIObject);
				
				const UBOOL bHasSelectedWidget = RenderModifier.GetSelectedWidget() != NULL;
				const UBOOL bHasSelectedFace = RenderModifier.GetSelectedDockHandle() != UIFACE_MAX;
				if(Widget != NULL && bHasSelectedWidget && bHasSelectedFace)
				{
					// Build a list of widgets currently under the mouse cursor, we will add this to our 
					// target widget outline container.
					const UUIObject* SelectedWidget = RenderModifier.GetSelectedWidget();

					const FVector2D Point(X, Y);
					TArray<UUIObject*> Children = UIEditor->OwnerScene->GetChildren( TRUE );
					TArray<UUIObject*> HitWidgets;

					for(INT ChildIdx = 0; ChildIdx < Children.Num(); ChildIdx++)
					{
						UUIObject* ChildWidget = Children(ChildIdx);
						const UBOOL bUnderMouse = ChildWidget->ContainsPoint(Point);

						// See if this widget is under the mouse, not the selected widget and it can receive focus.
						if( bUnderMouse && SelectedWidget != ChildWidget && !ChildWidget->IsPrivateBehaviorSet( UCONST_PRIVATE_NotFocusable ) )
						{
							HitWidgets.AddItem(ChildWidget);
						}
					}

					
					// Make sure we have atleast one target widget under us that isn't the selected widget.
					if(HitWidgets.Num() > 0)
					{	
							
						TargetRenderModifier.SetTargetWidgets(HitWidgets);
						TargetRenderModifier.SetSelectedWidget(Widget);

						bClearTarget = FALSE;
					}
					
				}
			}
		}

		// Clear the target widget if nothing was moused over.
		if( bClearTarget == TRUE )
		{	
			TargetRenderModifier.ClearSelections();
		}
	}
	else
	{
		TargetRenderModifier.ClearSelections();
	}
}

/**
 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
 */
void FUIWidgetTool_FocusChain::MouseMove(FViewport* Viewport, INT X, INT Y)
{
	if(UIEditor->ObjectVC->MouseTool == NULL)
	{
		// Use hitproxy's to see if we are over any dock handles, if we are then set the
		// render modifier state to reflect the fact that the handle is highlighted.
		HHitProxy* HitProxy = Viewport->GetHitProxy(X, Y);
		UBOOL bClearSelections = TRUE;

		if(HitProxy != NULL)
		{
			if(HitProxy->IsA(HUIFocusChainHandleHitProxy::StaticGetType()) == TRUE)
			{
				HUIFocusChainHandleHitProxy* DockHandleProxy = (HUIFocusChainHandleHitProxy*)(HitProxy);

				RenderModifier.SetSelectedWidget(DockHandleProxy->UIObject);
				RenderModifier.SetSelectedDockFace(DockHandleProxy->DockFace);

				bClearSelections = FALSE;
			}
		}

		if(bClearSelections == TRUE)
		{
			RenderModifier.ClearSelections();
		}
	}
}

/**
 * Method for serializing UObject references that must be kept around through GC's.
 *
 * @param Ar The archive to serialize with
 */
void FUIWidgetTool_FocusChain::Serialize(FArchive& Ar)
{
	RenderModifier.Serialize(Ar);
	TargetRenderModifier.Serialize(Ar);
}

/**
 * Called when the user right-clicks on a dock handle in the viewport, shows a context menu for dock handles.
 */
void FUIWidgetTool_FocusChain::ShowFocusChainHandleContextMenu()
{
	UUIObject* SelectedWidget = RenderModifier.GetSelectedWidget();
	EUIWidgetFace SelectedFace = RenderModifier.GetSelectedDockHandle();

	const UBOOL bValidWidget = (SelectedWidget != NULL);
	const UBOOL bValidFace = (SelectedFace != UIFACE_MAX);

	if(bValidWidget && bValidFace)
	{
		UIEditor->SynchronizeSelectedWidgets(UIEditor);

		const UUIObject* TargetWidget = SelectedWidget->GetNavigationTarget(SelectedFace, NAVLINK_Manual);
		const UUIObject* AutomaticWidget = SelectedWidget->GetNavigationTarget(SelectedFace, NAVLINK_Automatic);
		const UUIObject* MaxTargetWidget = SelectedWidget->GetNavigationTarget(SelectedFace, NAVLINK_MAX);
		const UBOOL bOverrideTarget = (AutomaticWidget != NULL && MaxTargetWidget == NULL);
		WxUIHandleContextMenu menu(MenuProxy);

		menu.Create( SelectedWidget, SelectedFace );
		
		menu.Append( IDM_UI_SETFOCUSCHAINTONULL, *LocalizeUI(TEXT("UIEditor_MenuItem_SetFocusChainToNull")) );
		menu.Enable( IDM_UI_SETFOCUSCHAINTONULL, bOverrideTarget == FALSE);
		menu.Append( IDM_UI_SETALLFOCUSCHAINTONULL, *LocalizeUI(TEXT("UIEditor_MenuItem_SetAllFocusChainToNull")) );
		menu.AppendSeparator();

		menu.Append( IDM_UI_BREAKFOCUSCHAINLINK, *LocalizeUI(TEXT("UIEditor_MenuItem_BreakFocusChainLink")) );

		// If we do not have a focus chain link on this focus chain handle, then disable the break focus chain link item.
		// Otherwise, enable the item and generate a label for the menu item that says which widget/face we are breaking the link to.
	

		if(TargetWidget == NULL && bOverrideTarget == FALSE)
		{
			menu.SetLabel(IDM_UI_BREAKFOCUSCHAINLINK, *LocalizeUI(TEXT("UIEditor_MenuItem_BreakFocusChainLink")));
			menu.Enable(IDM_UI_BREAKFOCUSCHAINLINK, false);
		}
		else
		{
			FString MenuLabel = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_MenuItem_BreakFocusChainLinkTo")), *TargetWidget->GetName()));
			menu.SetLabel(IDM_UI_BREAKFOCUSCHAINLINK, *MenuLabel);
			menu.Enable(IDM_UI_BREAKFOCUSCHAINLINK, true);
		}

		menu.Append( IDM_UI_BREAKALLFOCUSCHAINLINKS, *LocalizeUI(TEXT("UIEditor_MenuItem_BreakAllFocusChainLinks")) );


		FTrackPopupMenu tpm( MenuProxy, &menu );
		tpm.Show();
	}
}

/* ==========================================================================================================
	"Create Widget" Mouse Mode - add widgets at the mouse location and set widget's initial size
	by dragging the mouse
========================================================================================================== */
FUIWidgetTool_CustomCreate::FUIWidgetTool_CustomCreate(class WxUIEditorBase* InEditor, INT InWidgetIdx) : 
FUIWidgetTool_Selection(InEditor),
WidgetIdx(InWidgetIdx)
{
}

/** Called when this tool is 'activated'. */
void FUIWidgetTool_CustomCreate::EnterToolMode()
{
	FUIWidgetTool_Selection::EnterToolMode();
}

/** Called when this tool is 'deactivated'. */
void FUIWidgetTool_CustomCreate::ExitToolMode()
{
	FUIWidgetTool_Selection::ExitToolMode();
}


FMouseTool* FUIWidgetTool_CustomCreate::CreateMouseTool() const
{
	FMouseTool* MouseTool = NULL;
	FEditorObjectViewportClient* ViewportClient = UIEditor->ObjectVC;
	const UBOOL bNoModifiers = (!ViewportClient->Input->IsAltPressed() && !ViewportClient->Input->IsCtrlPressed() && !ViewportClient->Input->IsShiftPressed());
	const UBOOL bLeftMouseButtonOnly = (ViewportClient->Viewport->KeyState(KEY_LeftMouseButton) == TRUE) && (ViewportClient->Viewport->KeyState(KEY_RightMouseButton) == FALSE);

	if( bLeftMouseButtonOnly == TRUE && bNoModifiers == TRUE)
	{
		MouseTool =  new FMouseTool_ObjectDragCreate(ViewportClient, UIEditor);
	}

	return MouseTool;
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
UBOOL FUIWidgetTool_CustomCreate::InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed)
{
	UBOOL bConsumeInput = FALSE;

	// See if we are interacting with a docking widget handle, if so we handle all input here.
	if( Key == KEY_LeftMouseButton )
	{
		if(Event == IE_Pressed)
		{
			const INT MouseX = Viewport->GetMouseX();
			const INT MouseY = Viewport->GetMouseY();

			// See if they clicked on a dock handle, if so, let start the link dragging process.
			HHitProxy* HitProxy = Viewport->GetHitProxy(MouseX, MouseY);

			if(HitProxy != NULL)
			{
				if(HitProxy->IsA(HUIDockHandleHitProxy::StaticGetType()) == TRUE && SelectedWidgetsOutline.GetSelectedWidget() != NULL)
				{
					HUIDockHandleHitProxy* DockHandleProxy = (HUIDockHandleHitProxy*)(HitProxy);

					UIEditor->ObjectVC->MouseTool= new FMouseTool_DockLinkCreate(UIEditor->ObjectVC, UIEditor, this);

					UIEditor->TargetTooltip->ClearStringList();

					// Tell the MouseTool to start dragging.
					FLOAT HitX;
					FLOAT HitY;
					UIEditor->ObjectVC->GetViewportPositionFromMousePosition(MouseX, MouseY, HitX, HitY);
					UIEditor->ObjectVC->MouseTool->StartDrag(UIEditor->ObjectVC, FVector(HitX, HitY, 0));

					bConsumeInput = TRUE;
				}
			}
		}
	}

	if(bConsumeInput == FALSE)
	{
		bConsumeInput = FUIWidgetTool_Selection::InputKey(Viewport, ControllerId, Key, Event, AmountDepressed);
	}

	return bConsumeInput;
}

/**
 * Called when the user clicks anywhere in the viewport.
 *
 * @param	HitProxy	the hitproxy that was clicked on (may be null if no hit proxies were clicked on)
 * @param	Click		contains data about this mouse click
 * @return				Whether or not we consumed the click.
 */
UBOOL FUIWidgetTool_CustomCreate::ProcessClick (HHitProxy* HitProxy, const FObjectViewportClick& Click)
{
	UBOOL bHandledClick = FALSE;


	if( Click.GetKey() == KEY_RightMouseButton)
	{
		// See if they right clicked on a widget dock handle, if so, display the dock handle
		// context menu.
		if(HitProxy != NULL)
		{
			if(HitProxy->IsA(HUIDockHandleHitProxy::StaticGetType()) == TRUE)
			{
				HUIDockHandleHitProxy* DockHandleProxy = (HUIDockHandleHitProxy*)(HitProxy);

				ShowDockHandleContextMenu();
				bHandledClick = TRUE;
			}
		}
	}

	return bHandledClick;
}

/**
 * Called when the user moves the mouse while this viewport is capturing mouse input (i.e. dragging)
 */
void FUIWidgetTool_CustomCreate::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime)
{
	TargetWidgetOutline.ClearSelections();
}

/**
 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
 */
void FUIWidgetTool_CustomCreate::MouseMove(FViewport* Viewport, INT X, INT Y)
{
	FUIWidgetTool_Selection::MouseMove(Viewport, X, Y);
}


/* ==========================================================================================================
	Widget specified mouse mode - context sensitive mouse mode for performing more interactive editing of
	specific widget types.  Not yet implemented.
========================================================================================================== */
/*==================================================================
	FUIWidgetTool_ListEditor
==================================================================*/
FUIWidgetTool_ListEditor::FUIWidgetTool_ListEditor( WxUIEditorBase* InEditor )
: TUIWidgetTool_WidgetEditor<UUIList>(InEditor)
{
}

/** Called when this tool is 'activated'. */
void FUIWidgetTool_ListEditor::EnterToolMode()
{
	//@todo
	FUIWidgetTool::EnterToolMode();
}

/** Called when this tool is 'deactivated'. */
void FUIWidgetTool_ListEditor::ExitToolMode()
{
	//@todo
	FUIWidgetTool::ExitToolMode();
}




// EOF



