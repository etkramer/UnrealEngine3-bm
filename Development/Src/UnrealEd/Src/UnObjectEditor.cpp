/*=============================================================================
	UnObjectEditor.cpp: Object editor implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineUIPrivateClasses.h"
#include "UnObjectEditor.h"
#include "UnrealEdPrivateClasses.h"

IMPLEMENT_CLASS(UObjectEditorViewportInput);

const static FLOAT	ObjectEditor_ZoomIncrement = 0.1f;
const static FLOAT	ObjectEditor_ZoomSpeed = 0.005f;
const static FLOAT	ObjectEditor_ZoomNotchThresh = 0.007f;
const static INT	ObjectEditor_ScrollBorderSize = 20;
const static FLOAT	ObjectEditor_ScrollBorderSpeed = 400.f;

static const TCHAR* GetActiveHandleText( ESelectionHandle Handle )
{
	switch ( Handle )
	{
		SHOW_CASE(HANDLE_Anchor);
		SHOW_CASE(HANDLE_Outline);
		SHOW_CASE(HANDLE_Left);
		SHOW_CASE(HANDLE_Right);
		SHOW_CASE(HANDLE_Top);
		SHOW_CASE(HANDLE_Bottom);
		SHOW_CASE(HANDLE_TopLeft);
		SHOW_CASE(HANDLE_TopRight);
		SHOW_CASE(HANDLE_BottomLeft);
		SHOW_CASE(HANDLE_BottomRight);
		SHOW_CASE(HANDLE_None);
	}

	return TEXT("");
}


/*-----------------------------------------------------------------------------
	FObjectViewportClick
-----------------------------------------------------------------------------*/

/** Constructors */
FObjectViewportClick::FObjectViewportClick()
: Location(0), Key(NAME_None), Event(IE_MAX), bControlDown(FALSE), bAltDown(FALSE), bShiftDown(FALSE)
{
}

FObjectViewportClick::FObjectViewportClick( FEditorObjectViewportClient* ViewportClient, FName InKey, EInputEvent InEvent, FLOAT X, FLOAT Y )
: Location(X,Y,0.f), Key(InKey), Event(InEvent)
{
	bControlDown = ViewportClient->Viewport->KeyState(KEY_LeftControl) || ViewportClient->Viewport->KeyState(KEY_RightControl);
	bShiftDown = ViewportClient->Viewport->KeyState(KEY_LeftShift) || ViewportClient->Viewport->KeyState(KEY_RightShift);
	bAltDown = ViewportClient->Viewport->KeyState(KEY_LeftAlt) || ViewportClient->Viewport->KeyState(KEY_RightAlt);
}

/*-----------------------------------------------------------------------------
	FObjectEditorInterface
-----------------------------------------------------------------------------*/

FObjectEditorInterface::FObjectEditorInterface()
{
	Selection = new( UObject::GetTransientPackage(), NAME_None, RF_Transactional ) USelection;
	Selection->AddToRoot();
}

FObjectEditorInterface::~FObjectEditorInterface()
{
	Selection->RemoveFromRoot();
	Selection = NULL;
}

/*-----------------------------------------------------------------------------
	FEditorObjectViewportClient
-----------------------------------------------------------------------------*/

FEditorObjectViewportClient::FEditorObjectViewportClient( FObjectEditorInterface* InObjectEditor )
: FEditorLevelViewportClient(UObjectEditorViewportInput::StaticClass()), MouseTracker(NULL), Selector(NULL), MouseTool(NULL)
{
	// No postprocess on editor object windows
	ShowFlags			&= ~SHOW_PostProcess;
	ObjectEditor		= InObjectEditor;

	// This window will be 2D/canvas only, so set the viewport type to None
	ViewportType = LVT_None;

	Origin2D			= FIntPoint(0, 0);
	Zoom2D				= 1.0f;
	MinZoom2D			= 0.1f;
	MaxZoom2D			= 5.f;

	bAllowScroll		= TRUE;
	ScrollAccum			= FVector2D(0,0);
		
	SetRealtime( false );

	GCallbackEvent->Register(CALLBACK_SelChange, this);
}

FEditorObjectViewportClient::~FEditorObjectViewportClient()
{
	GEngine->Client->CloseViewport(Viewport);
	Viewport = NULL;

	if ( MouseTracker != NULL )
	{
		delete MouseTracker;
		MouseTracker = NULL;
	}

	if ( Selector != NULL )
	{
		delete Selector;
		Selector = NULL;
	}

	GCallbackEvent->UnregisterAll(this);
}

void FEditorObjectViewportClient::InitializeClient()
{
	// create the mouse tracker
	CreateMouseTracker();

	// create the selection tool
	CreateSelectionTool();
	AddRenderModifier(Selector);
}

/** Creates the object selection tool */
void FEditorObjectViewportClient::CreateSelectionTool()
{
	if ( Selector == NULL )
	{
		Selector = new FObjectSelectionTool;
	}
}

/** Creates a mouse delta tracker */
void FEditorObjectViewportClient::CreateMouseTracker()
{
	if ( MouseTracker == NULL )
	{
		MouseTracker = new FMouseDragTracker;
	}
}


/**
 * Register a new render modifier with this viewport client.
 *
 * @param Modifier		Modifier to add to the list of modifers.
 * @param InsertIndex	Index to insert the modifer at.
 * @return				Whether or not we could add the modifier to the list.
 */
UBOOL FEditorObjectViewportClient::AddRenderModifier( class FObjectViewportRenderModifier* Modifier, INT InsertIndex /*= INDEX_NONE*/ )
{
	if ( Modifier == NULL )
	{
		return FALSE;
	}

	if ( RenderModifiers.FindItemIndex(Modifier) != INDEX_NONE )
	{
		return FALSE;
	}

	if(InsertIndex == INDEX_NONE || InsertIndex >= RenderModifiers.Num())
	{
		RenderModifiers.AddItem( Modifier );
	}
	else
	{
		RenderModifiers.InsertItem( Modifier, InsertIndex );
	}
	
	return TRUE;
}

/**
 * Remove a render modifier from this viewport client's list.
 */
UBOOL FEditorObjectViewportClient::RemoveRenderModifier( class FObjectViewportRenderModifier* Modifier )
{
	if ( Modifier == NULL )
	{
		return FALSE;
	}

	if ( RenderModifiers.FindItemIndex(Modifier) == INDEX_NONE )
	{
		return FALSE;
	}

	RenderModifiers.RemoveItem( Modifier );
	return TRUE;
}

/**
* Register a new input modifier with this viewport client.
*
* @param Modifier		Modifier to add to the list of modifers.
* @param InsertIndex	Index to insert the modifer at.
* @return				Whether or not we could add the modifier to the list.
*/
UBOOL FEditorObjectViewportClient::AddInputModifier(class FObjectViewportInputModifier* Modifier, INT InsertIndex /* = INDEX_NONE  */)
{
	if ( Modifier == NULL )
	{
		return FALSE;
	}

	if ( InputModifiers.FindItemIndex(Modifier) != INDEX_NONE )
	{
		return FALSE;
	}

	if(InsertIndex == INDEX_NONE || InsertIndex >= InputModifiers.Num())
	{
		InputModifiers.AddItem( Modifier );
	}
	else
	{
		InputModifiers.InsertItem( Modifier, InsertIndex );
	}

	return TRUE;
}

/**
 * Remove a input modifier from this viewport client's list.
 */
UBOOL FEditorObjectViewportClient::RemoveInputModifier( class FObjectViewportInputModifier* Modifier )
{
	if ( Modifier == NULL )
	{
		return FALSE;
	}

	if ( InputModifiers.FindItemIndex(Modifier) == INDEX_NONE )
	{
		return FALSE;
	}

	InputModifiers.RemoveItem( Modifier );
	return TRUE;
}

/**
 * Marks the viewport to be redrawn.
 */
void FEditorObjectViewportClient::RefreshViewport()
{
	Viewport->Invalidate();
}

void FEditorObjectViewportClient::Draw(FViewport* Viewport, FCanvas* Canvas)
{
	Canvas->PushAbsoluteTransform(FScaleMatrix(Zoom2D) * FTranslationMatrix(FVector(Origin2D.X,Origin2D.Y,0)));
	{
		// Render the background
		ObjectEditor->DrawBackground(Viewport,Canvas);

		// draw the objects in this viewport
		ObjectEditor->DrawObjects(Viewport,Canvas);

		// draw the viewport modifiers
		for ( INT i = 0; i < RenderModifiers.Num(); i++ )
		{
			FObjectViewportRenderModifier* Modifier = RenderModifiers(i);
			Modifier->Render(this,Canvas);
		}

		if ( MouseTracker->DragTool )
		{
			MouseTracker->DragTool->Render(Canvas);
		}

		// Let the selected tool render if it wants to, make sure to draw this last so it has a chance to draw on top
		// of everything else.
		if(MouseTool)
		{
			MouseTool->Render(Canvas);
		}
	}

	Canvas->PopTransform();
}

/**
 * Retrieve the vectors marking the region within the viewport available for the object editor to render objects. 
 *
 * @param	StartPos					receives the value of the origin of the rendering region
 * @param	EndPos						receives the value for the end point of the rendering region
 * @param	bIncludeVirtualAdjustments	specify FALSE to ignore any gutter or offsets which may be in use by the viewport
 */
void FEditorObjectViewportClient::GetClientRenderRegion( FVector2D& StartPos, FVector2D& EndPos, UBOOL bIncludeVirtualAdjustments/*=TRUE*/ )
{
	StartPos = Origin2D;
	EndPos = FVector2D(Viewport->GetSizeX(), Viewport->GetSizeY());
}

/**
* Converts the mouse coordinates to viewport coordinates by factoring in viewport offset and zoom.
*
* @param InMouseX		Mouse X Position
* @param InMouseY		Mouse Y Position
* @param OutViewportX	Output variable for the Viewport X position
* @param OutViewportY	Output variable for the Viewport Y position
*/
void FEditorObjectViewportClient::GetViewportPositionFromMousePosition(const INT InMouseX, const INT InMouseY, FLOAT& OutViewportX, FLOAT& OutViewportY )
{
	OutViewportX = (InMouseX - Origin2D.X) / Zoom2D;
	OutViewportY = (InMouseY - Origin2D.Y) / Zoom2D;
}

UBOOL FEditorObjectViewportClient::InputKey(FViewport* Viewport, INT ControllerId, FName Key, EInputEvent Event,FLOAT /*AmountDepressed*/,UBOOL /*Gamepad*/)
{
	const INT MouseX = Viewport->GetMouseX();
	const INT MouseY = Viewport->GetMouseY();

	if ( !Viewport->HasMouseCapture() )
	{
		Viewport->ShowCursor( TRUE );
		Viewport->LockMouseToWindow( FALSE );
	}

	// Handle our specially bound keys first.
	if (InputKeyUser (Key, Event))
	{
		return TRUE;
	}

	// Handle all input modifiers.
	for ( INT i = 0; i < InputModifiers.Num(); i++ )
	{
		FObjectViewportInputModifier* Modifier = InputModifiers(i);
		if ( Modifier != NULL && Modifier->HandleKeyInput(this,Key,Event) )
		{
			return TRUE;
		}
	}

	// Tell the viewports input handler about the keypress.
	if( Input->InputKey(ControllerId, Key, Event) )
	{
		return TRUE;
	}

	// Handle viewport screenshot.
	InputTakeScreenshot( Viewport, Key, Event );

	// Remember which keys and buttons are pressed down.
	const UBOOL bAltDown = Input->IsAltPressed();
	const UBOOL	bShiftDown = Input->IsShiftPressed();
	const UBOOL bControlDown = Input->IsCtrlPressed();
	const UBOOL bLeftMouseButtonDown = Viewport->KeyState(KEY_LeftMouseButton);
	const UBOOL bMiddleMouseButtonDown = Viewport->KeyState(KEY_MiddleMouseButton);
	const UBOOL bRightMouseButtonDown = Viewport->KeyState(KEY_RightMouseButton);
	const UBOOL bCaptureMouse = bLeftMouseButtonDown || bMiddleMouseButtonDown || bRightMouseButtonDown;

	// Convert the mouse position from 
	FLOAT HitX;
	FLOAT HitY;

	GetViewportPositionFromMousePosition(MouseX, MouseY, HitX, HitY);

	if( Key == KEY_LeftMouseButton || Key == KEY_MiddleMouseButton || Key == KEY_RightMouseButton )
	{
		switch( Event )
		{
		case IE_DoubleClick:
			{
				const FObjectViewportClick Click(this, Key, Event, HitX, HitY);
				HHitProxy* HitResult = Viewport->GetHitProxy(MouseX, MouseY);
				ProcessClick(HitResult,Click);
			}
			break;

		case IE_Pressed:
			{
				// See if we are not already tracking in some way.  If we are not, try to create a tool that tracks the mouse,
				// if there was no tool created, then let the drag tool take over.
				if(!MouseTool)
				{
					HHitProxy* HitResult = Viewport->GetHitProxy(MouseX, MouseY);
					MouseTool = CreateMouseTool(HitResult);
					
					if(MouseTool)
					{
						MouseTool->StartDrag(this, FVector(HitX, HitY, 0.0f));
					}
					else
					{
						MouseTracker->StartTracking(this, HitX, HitY);
						Viewport->LockMouseToWindow(TRUE);
						Viewport->ShowCursor(FALSE);
						bIsTracking = TRUE;
						RefreshViewport();
					}
				}

			}
			break;

		case IE_Released:
			{
				// Make sure to only handle the release event if all mouse buttons were let go.
				if(!bCaptureMouse)
				{
					if(MouseTool)
					{						
						// Process a click event if the mouse didn't move.  Call the click first because EndDrag may invalidate viewport.
						const UBOOL bNoMovement = MouseTool->GetAbsoluteDistance() < MINIMUM_MOUSEDRAG_PIXELS;

						if(bNoMovement == TRUE)
						{
							const FObjectViewportClick Click(this, Key, Event, HitX, HitY);
							HHitProxy* HitResult = Viewport->GetHitProxy(MouseX, MouseY);
							ProcessClick(HitResult, Click);
						}

						// Finish the mouse drag and delete the mousetool.
						MouseTool->EndDrag();
						delete MouseTool;
						MouseTool = NULL;
					}
					else if(bIsTracking == TRUE)
					{
						bIsTracking = FALSE;
				
						FVector MouseDelta = MouseTracker->GetDelta();
						if ( MouseTracker->EndTracking(this) )
						{
							if ( MouseDelta.Size() < MINIMUM_MOUSEDRAG_PIXELS && MouseTracker->DragTool == NULL )
							{
								const FObjectViewportClick Click(this, Key, Event, HitX, HitY);
								HHitProxy* HitResult = Viewport->GetHitProxy(MouseX, MouseY);
								ProcessClick(HitResult, Click);
							}
						}
					}
				}
			}
			break;
		}
	}
	else if ( (Key == KEY_MouseScrollDown || Key == KEY_MouseScrollUp) && Event == IE_Pressed )
	{
		// Mousewheel up/down zooms in/out.
		const FLOAT DeltaZoom = (Key == KEY_MouseScrollDown ? -ObjectEditor_ZoomIncrement : ObjectEditor_ZoomIncrement );

		if( (DeltaZoom < 0.f && Zoom2D > MinZoom2D) ||
			(DeltaZoom > 0.f && Zoom2D < MaxZoom2D) )
		{
			const FLOAT ViewCenterX = ((((FLOAT)Viewport->GetSizeX()) * 0.5f) - Origin2D.X)/Zoom2D;
			const FLOAT ViewCenterY = ((((FLOAT)Viewport->GetSizeY()) * 0.5f) - Origin2D.Y)/Zoom2D;

			Zoom2D = Clamp<FLOAT>(Zoom2D+DeltaZoom,MinZoom2D,MaxZoom2D);

			FLOAT DrawOriginX = ViewCenterX - ((Viewport->GetSizeX()*0.5f)/Zoom2D);
			FLOAT DrawOriginY = ViewCenterY - ((Viewport->GetSizeY()*0.5f)/Zoom2D);

			Origin2D.X = -(DrawOriginX * Zoom2D);
			Origin2D.Y = -(DrawOriginY * Zoom2D);

			ObjectEditor->ViewPosChanged();
			RefreshViewport();
		}
	}
	else if ( Key == KEY_Escape && Event == IE_Released )
	{
		// if the user presses escape, cancel any active tools without applying anything
		if ( MouseTool != NULL )
		{
			MouseTool->ResetTracking();
			delete MouseTool;
			MouseTool = NULL;
		}
		
		if ( bIsTracking )
		{
			MouseTracker->ResetTracking();
			bIsTracking = FALSE;
		}

		Viewport->LockMouseToWindow(FALSE);
		Viewport->ShowCursor(TRUE);
		return TRUE;
	}

	ObjectEditor->HandleKeyInput(Viewport, Key, Event);
	return TRUE;
}

/**
 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
 */
void FEditorObjectViewportClient::MouseMove(FViewport* Viewport, INT X, INT Y)
{
	//@todo - implement hot-tracking for highlighting the object that is being moused over...
	//			the selection handle stuff is taken care of by the selection tool

	// If there is a mouse tool active, make sure to route the mouse events to it.
	if(MouseTool)
	{
		MouseTool->MouseMove(Viewport, X, Y);
	}
	else
	{
		for ( INT i = 0; i < InputModifiers.Num(); i++ )
		{
			FObjectViewportInputModifier* Modifier = InputModifiers(i);
			if ( Modifier->HandleMouseMovement(this,X,Y) )
			{
				break;
			}
		}
	}

}

/**
 * Called when the user moves the mouse while this viewport is capturing mouse input (i.e. dragging)
 */
UBOOL FEditorObjectViewportClient::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad)
{
	// Accumulate and snap the mouse movement since the last mouse button click.
	MouseTracker->AddDelta(this, Key, Delta, FALSE);

	// If we are using a drag tool, paint the viewport so we can see it update.
	if ( MouseTracker->DragTool != NULL )
	{
		RefreshViewport();
	}

	return TRUE;
}

/**
 *	Creates a mouse tool depending on the current state of the editor and what key modifiers are held down.
 *  @param HitProxy	Pointer to the hit proxy currently under the mouse cursor.
 *  @return Returns a pointer to the newly created FMouseTool.
 */
FMouseTool* FEditorObjectViewportClient::CreateMouseTool(const HHitProxy* HitProxy)
{
	const UBOOL bBoxSelect = ShouldBeginBoxSelect();
	if(bBoxSelect == TRUE)
	{
		return new FMouseTool_ObjectBoxSelect(this);
	}

	return NULL;
}

/**
 * Called from the mouse drag tracker when the user is dragging the mouse and a drag tool is not active.
 *
 * @param	DragAmount		the translated mouse movement deltas
 * @param	bDragActive		TRUE if the mouse has been dragged more than the minimum drag amount (MINIMUM_MOUSEDRAG_PIXELS)
 */
void FEditorObjectViewportClient::MouseDrag( FVector& DragAmount, UBOOL bDragActive )
{
	for ( INT i = 0; i < InputModifiers.Num(); i++ )
	{
		FObjectViewportInputModifier* Modifier = InputModifiers(i);
		Modifier->OnMouseDrag(this, DragAmount, bDragActive);
	}
}

/**
 * Called when the user clicks anywhere in the viewport.
 *
 * @param	HitProxy	the hitproxy that was clicked on (may be null if no hit proxies were clicked on)
 * @param	Click		contains data about this mouse click
 */
void FEditorObjectViewportClient::ProcessClick( HHitProxy* HitProxy, const FObjectViewportClick& Click )
{
	LastClick = Click;
	if ( HitProxy == NULL )
	{
		ClickedBackground(Click);
	}
	else if ( HitProxy->IsA(HObject::StaticGetType()) )
	{
		ClickedObject(Click, ((HObject*)HitProxy)->Object);
	}
}

/**
 * Called when the user clicks on an empty region of the viewport.  Might clear all selected objects
 * or display a context menu.
 */
void FEditorObjectViewportClient::ClickedBackground( const FObjectViewportClick& Click )
{

	if( Click.GetKey() == KEY_RightMouseButton && !Click.IsControlDown() && !Viewport->KeyState(KEY_LeftMouseButton) )
	{	
		// Right click should only show the new object menu and should NOT deselect objects.
		ObjectEditor->ShowNewObjectMenu();
	}
	else if ( Click.GetKey() == KEY_LeftMouseButton )
	{
		if ( !Click.IsControlDown() && ObjectEditor->Selection->Num() > 0 )
		{
			ObjectEditor->Selection->BeginBatchSelectOperation();

			ObjectEditor->EmptySelection();

			ObjectEditor->Selection->EndBatchSelectOperation();
		}
	}
}

void FEditorObjectViewportClient::ClickedObject( const FObjectViewportClick& Click, UObject* ClickedObject )
{
	if( Click.GetKey() == KEY_RightMouseButton && !Click.IsControlDown() && !Viewport->KeyState(KEY_LeftMouseButton) )
	{
		if( !ObjectEditor->IsInSelection(ClickedObject) )
		{
			ObjectEditor->Selection->BeginBatchSelectOperation();

			ObjectEditor->EmptySelection();
			ObjectEditor->AddToSelection(ClickedObject);

			ObjectEditor->Selection->EndBatchSelectOperation();

			// Redraw the viewport so the user can see which item was right clicked on.
			Viewport->Draw();
		}

		ObjectEditor->ShowObjectContextMenu(ClickedObject);
	}
	else if ( Click.GetEvent() == IE_DoubleClick && Click.GetKey() == KEY_LeftMouseButton )
	{
		if ( ObjectEditor->Selection->Num() != 1 || !ObjectEditor->IsInSelection(ClickedObject) )
		{
			ObjectEditor->Selection->BeginBatchSelectOperation();

			if ( !Click.IsControlDown() )
			{
				ObjectEditor->EmptySelection();
			}
			ObjectEditor->AddToSelection(ClickedObject);

			ObjectEditor->Selection->EndBatchSelectOperation();
		}
		ObjectEditor->DoubleClickedObject(ClickedObject);
	}
	else
	{
		if ( Click.IsControlDown() || ObjectEditor->Selection->Num() != 1 || !ObjectEditor->IsInSelection(ClickedObject) )
		{
			ObjectEditor->Selection->BeginBatchSelectOperation();
			if ( Click.IsControlDown() )
			{
				ObjectEditor->ToggleSelection(ClickedObject);
			}
			else
			{
				ObjectEditor->EmptySelection();
				ObjectEditor->AddToSelection(ClickedObject);
			}

			ObjectEditor->Selection->EndBatchSelectOperation();
		}
	}
}

/**
 * @return		Returns the background color for the viewport.
 */
FLinearColor FEditorObjectViewportClient::GetBackgroundColor()
{
	return GEditor->C_OrthoBackground;
}

/**
 * Get the cursor that should be displayed in the viewport.
 */
EMouseCursor FEditorObjectViewportClient::GetCursor(FViewport* Viewport,INT X,INT Y)
{
	EMouseCursor MouseCursor = MC_Arrow;
	ESelectionHandle SelectionHandle = HANDLE_None;

	HHitProxy*	HitProxy = Viewport->GetHitProxy(X,Y);

	// Change the mouse cursor if the user is hovering over something they can interact with.
	if( HitProxy )
	{
		MouseCursor = HitProxy->GetMouseCursor();
		if ( HitProxy->IsA(HSelectionHandle::StaticGetType()) )
		{
			SelectionHandle = ((HSelectionHandle*)HitProxy)->Handle;
		}

	}

	// update the selector's active handle - if we are mousing over one of the selector's handles, let it know so that the mouse tracker will abort if the user starts dragging
	if ( SelectionHandle == HANDLE_None )
	{
		// not mousing over a selection handle at the moment
		Selector->ClearActiveHandle();
	}
	else
	{
		Selector->SetActiveHandle(SelectionHandle);
	}

	return MouseCursor;
}

/** 
 *	See if cursor is in 'scroll' region around the edge, and if so, scroll the view automatically. 
 *	Returns the distance that the view was moved.
 */
FIntPoint FEditorObjectViewportClient::DoScrollBorder(FLOAT DeltaTime)
{
	DeltaTime = Clamp(DeltaTime, 0.01f, 1.0f);

	if (bAllowScroll)
	{
		INT PosX = Viewport->GetMouseX();
		INT PosY = Viewport->GetMouseY();
		INT SizeX = Viewport->GetSizeX();
		INT SizeY = Viewport->GetSizeY();

		if(PosX < ObjectEditor_ScrollBorderSize)
		{
			ScrollAccum.X += (1.f - ((FLOAT)PosX/(FLOAT)ObjectEditor_ScrollBorderSize)) * ObjectEditor_ScrollBorderSpeed * DeltaTime;
		}
		else if(PosX > SizeX - ObjectEditor_ScrollBorderSize)
		{
			ScrollAccum.X -= ((FLOAT)(PosX - (SizeX - ObjectEditor_ScrollBorderSize))/(FLOAT)ObjectEditor_ScrollBorderSize) * ObjectEditor_ScrollBorderSpeed * DeltaTime;
		}
		else
		{
			ScrollAccum.X = 0.f;
		}

		FLOAT ScrollY = 0.f;
		if(PosY < ObjectEditor_ScrollBorderSize)
		{
			ScrollAccum.Y += (1.f - ((FLOAT)PosY/(FLOAT)ObjectEditor_ScrollBorderSize)) * ObjectEditor_ScrollBorderSpeed * DeltaTime;
		}
		else if(PosY > SizeY - ObjectEditor_ScrollBorderSize)
		{
			ScrollAccum.Y -= ((FLOAT)(PosY - (SizeY - ObjectEditor_ScrollBorderSize))/(FLOAT)ObjectEditor_ScrollBorderSize) * ObjectEditor_ScrollBorderSpeed * DeltaTime;
		}
		else
		{
			ScrollAccum.Y = 0.f;
		}

		// Apply integer part of ScrollAccum to origin, and save the rest.
		INT MoveX = appFloor(ScrollAccum.X);
		Origin2D.X += MoveX;
		ScrollAccum.X -= MoveX;

		INT MoveY = appFloor(ScrollAccum.Y);
		Origin2D.Y += MoveY;
		ScrollAccum.Y -= MoveY;

		// If view has changed, notify the app
		if( Abs<INT>(MoveX) > 0 || Abs<INT>(MoveY) > 0 )
		{
			ObjectEditor->ViewPosChanged();
		}
		return FIntPoint(MoveX, MoveY);
	}
	else
	{
		return FIntPoint(0, 0);
	}
}

void FEditorObjectViewportClient::Tick(FLOAT DeltaSeconds)
{
	FEditorLevelViewportClient::Tick(DeltaSeconds);

	// Auto-scroll display if moving/drawing etc. and near edge.
	UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);

	// If we have a mouse tool activated, then we are elgible to scroll the viewport.
	if(	MouseTool != NULL )
	{
		bAllowScroll = TRUE;

		// dragging objects....
		FIntPoint Delta = DoScrollBorder(DeltaSeconds);

		// Force a mouse move update
		MouseTool->MouseMove(Viewport, Viewport->GetMouseX(), Viewport->GetMouseY());
	}
	else
	{
		bAllowScroll = FALSE;
	}

	// Always invalidate the display - so mouse overs and stuff get a chance to redraw.
	RefreshViewport();
}

/**
 * Determines which axis InKey and InDelta most refer to and returns a corresponding FVector.  This
 * vector represents the mouse movement translated into the viewports/widgets axis space.
 *
 * @param	InNudge		If 1, this delta is coming from a keyboard nudge and not the mouse
 */
FVector FEditorObjectViewportClient::TranslateDelta( FName InKey, FLOAT InDelta, UBOOL InNudge )
{
	FVector vec;

	vec.X = InKey == KEY_MouseX ? InDelta / Zoom2D : 0.f;
	vec.Y = InKey == KEY_MouseY ? -InDelta / Zoom2D : 0.f;
	vec.Z = 0.f;

	return vec;
}

/*-----------------------------------------------------------------------------
	FCallbackEventDevice interface
-----------------------------------------------------------------------------*/
void FEditorObjectViewportClient::Send( ECallbackEventType InType )
{
	switch ( InType )
	{
	case CALLBACK_SelChange:
		RefreshViewport();
		break;
	}
}

/* ==========================================================================================================
	UObjectEditorViewportInput
========================================================================================================== */
UBOOL UObjectEditorViewportInput::Exec(const TCHAR* Cmd,FOutputDevice& Ar)
{
	// skip over the UEditorViewportInput version, which calls into the editor's exec functions
	return Super::Super::Exec(Cmd, Ar);
}
