/*=============================================================================
	UnObjectDragTools.cpp: Object editor drag helper class implementations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Includes
#include "UnrealEd.h"
#include "EngineUIPrivateClasses.h"
#include "UnObjectEditor.h"

/* ==========================================================================================================
	FMouseDragParameters
========================================================================================================== */

/**
 * Sets up the values for this struct
 */
void FMouseDragParameters::SetValues( FEditorObjectViewportClient* ViewportClient )
{
	if ( ViewportClient != NULL && ViewportClient->Viewport != NULL )
	{
		// use the Viewport to determine whether the system keys are pressed, rather than the viewport client's Input.  If the user clicked on this viewport
		// while the system keys were held down, the viewport's Input won't know about it, so by using ViewportClient->Viewport we can begin a drag operation
		// immediately instead of requiring the user to focus this viewport before pressing the system keys

		bAltPressed = IsAltDown(ViewportClient->Viewport);
		bShiftPressed = IsShiftDown(ViewportClient->Viewport);
		bCtrlPressed = IsCtrlDown(ViewportClient->Viewport);
		bLeftMouseButtonDown = ViewportClient->Viewport->KeyState(KEY_LeftMouseButton);
		bRightMouseButtonDown = ViewportClient->Viewport->KeyState(KEY_RightMouseButton);
		bMiddleMouseButtonDown = ViewportClient->Viewport->KeyState(KEY_MiddleMouseButton);
	}
	else
	{
		bAltPressed = FALSE;
		bShiftPressed = FALSE;
		bCtrlPressed = FALSE;
		bLeftMouseButtonDown = FALSE;
		bRightMouseButtonDown = FALSE;
		bMiddleMouseButtonDown = FALSE;
	}
}

/* ==========================================================================================================
	FMouseDragTracker
========================================================================================================== */
FMouseDragTracker::FMouseDragTracker()
: DragTool(NULL)
, TransCount(0)
, Start(0,0,0)
, StartSnapped(0,0,0)
, End(0,0,0)
, EndSnapped(0,0,0)
{ }

void FMouseDragTracker::StartTracking( FEditorObjectViewportClient* InViewportClient, const INT InX, const INT InY )
{
	StartSnapped = Start = FVector( InX, InY, 0 );

	// remember which buttons were pressed down
	DragParams.SetValues(InViewportClient);

	End = Start;
	EndSnapped = StartSnapped;
}

/**
 * Adds delta movement into the tracker.
 *
 * @param	InViewportClient	the viewport client where this movement is occurring
 * @aram	InKey				the key that caused the movement
 * @param	InDelta				the number of pixels in this movement
 * @param	InNudge				TRUE if this is a delta movement simulated by keyboard shortcuts (pressing left/right, etc.)
 */

void FMouseDragTracker::AddDelta( FEditorObjectViewportClient* InViewportClient, const FName InKey, const INT InDelta, UBOOL InNudge )
{
	// if we aren't holding any mouse buttons down and this isn't a keyboard movement, abort
	if( !DragParams.bLeftMouseButtonDown && !DragParams.bMiddleMouseButtonDown && !DragParams.bRightMouseButtonDown && !InNudge )
	{
		return;
	}

	const FVector CurrentDelta = GetDelta();
	const UBOOL bActiveDrag = CurrentDelta.Size() >= MINIMUM_MOUSEDRAG_PIXELS;
	if ( DragTool == NULL && bActiveDrag ) 
	{
		// determine if we should invoke a drag tool
		DragTool = InViewportClient->CreateCustomDragTool();

		// If no other drag tool was created, create a camera move drag tool.
		if ( DragTool == NULL )
		{
			DragTool = new FDragTool_MoveCamera(InViewportClient);
		}

		DragTool->StartDrag( InViewportClient, Start );
		DragTool->AddDelta(CurrentDelta);
	}

	// vDelta is a vector representing how much the mouse has moved
	FVector vDelta = InViewportClient->TranslateDelta( InKey, InDelta, InNudge );
	if ( InViewportClient->Selector->ActiveHandle != HANDLE_None )
	{
		// remove any deltas that shouldn't be applied to this resize handle
		switch ( InViewportClient->Selector->ActiveHandle )
		{
		case HANDLE_Left:
		case HANDLE_Right:
			vDelta.Y = 0;
			break;

		case HANDLE_Top:
		case HANDLE_Bottom:
			vDelta.X = 0;
			break;
		}

	}

	End += vDelta;
	EndSnapped = End;

	if( DragTool )
	{
		DragTool->AddDelta( vDelta );
	}
	else
	{
		InViewportClient->MouseDrag(vDelta, bActiveDrag);
	}
}

/**
 * Called when a mouse button has been released.  If there are no other
 * mouse buttons being held down, the internal information is reset.
 */

UBOOL FMouseDragTracker::EndTracking( FEditorObjectViewportClient* InViewportClient )
{
	Start = StartSnapped = End = EndSnapped = FVector(0,0,0);
	DragParams.SetValues(NULL);

	if( DragTool != NULL )
	{
		FObjectDragTool* pDragTool = DragTool;
		DragTool = NULL;

		pDragTool->EndDrag();
		delete pDragTool;
		return FALSE;
	}

	return TRUE;
}

/**
 * Resets the mouse drag tracker.  Used to release an active mouse tool without applying whatever changes the tool
 * is designed for.
 *
 * @return	FALSE if there was an active drag tool that had to be reset as well; TRUE if there was no active drag tool.
 */
UBOOL FMouseDragTracker::ResetTracking()
{
	Start = StartSnapped = End = EndSnapped = FVector(0,0,0);
	DragParams.SetValues(NULL);
	if( DragTool != NULL )
	{
		FObjectDragTool* pDragTool = DragTool;
		DragTool = NULL;

		pDragTool->ResetTracking();
		delete pDragTool;
		return FALSE;
	}

	return TRUE;
}

/**
 * Retrieve information about the current drag delta.
 */

FVector FMouseDragTracker::GetDelta() const
{
	return (End - Start);
}

FVector FMouseDragTracker::GetDeltaSnapped() const
{
	return (EndSnapped - StartSnapped);
}

/**
 * Updates the EndSnapped variable so that it is a snapped version of End using the GridSize and GridOrigin parameters passed in.
 * @param GridSize		Size of the grid.
 * @param GridOrigin	Origin of the grid.
 */
void FMouseDragTracker::Snap(FLOAT GridSize, const FVector& GridOrigin)
{
	EndSnapped = (End - GridOrigin).GridSnap(GridSize);
}

void FMouseDragTracker::ReduceBy( const FVector& In )
{
	End -= In;
	EndSnapped -= In;
}

/* ==========================================================================================================
	FObjectSelectionTool
========================================================================================================== */

/** Constructor */
FObjectSelectionTool::FObjectSelectionTool()
: HandleMaterial(NULL), AnchorMaterial(NULL), OutlineMaterial(NULL), HandleSize(12), AnchorSize(24)
, AnchorLocation(0,0), AnchorOffset(0,0), StartLocation(0,0), EndLocation(0,0), ActiveHandle(HANDLE_None)
{
	HandleMaterial = LoadObject<UTexture2D>(NULL, TEXT("EditorResources.BSPVertex"), NULL, LOAD_None, NULL);
	AnchorMaterial = LoadObject<UTexture2D>(NULL, TEXT("EditorMaterials.Anchor"), NULL, LOAD_None, NULL);

	HandleColor = FColor(255,64,64,200);
	AnchorColor = FColor(255,129,129,255);

	HandleActiveColor = FColor(255,255,129,255);
	AnchorActiveColor = FColor(255,255,255,200);

	OutlineColor = FColor(255,255,255,255);
	OutlineActiveColor = FColor(255, 0, 0);
}

/** Destructor */
FObjectSelectionTool::~FObjectSelectionTool()
{
}

/**
 * Sets the rendering offset of the anchor.  Tries to snap the anchor to various positions if its close enough, otherwise just sets the offset equal to the delta passed in.
 *
 * @param Delta		Offset to set the anchor's rendering offset to.  Usually this is just the change in mouse position since the user started dragging.
 */
void FObjectSelectionTool::SetAnchorOffset(const FVector2D &Delta)
{
	// if the anchor position+offset is within a certain pixel distance threshold of the center of the selection box or any of the handles,
	// then snap the anchor offset to that position.  Otherwise just set the offset to the delta value.
	const FLOAT Threshold = 16.0f;
	const FLOAT ThresholdSquared = Threshold * Threshold;
	const FVector2D AnchorPos = AnchorLocation + Delta;
	const FVector2D SelectionCenter = (StartLocation + EndLocation) * 0.5f;
	UBOOL bSnappedPosition = FALSE;

	// Try to snap to the center of the selection first.
	if((AnchorPos - SelectionCenter).SizeSquared() < ThresholdSquared)
	{
		AnchorOffset = SelectionCenter - AnchorLocation;
		bSnappedPosition = TRUE;
	}


	// Go through all of the handle positions and try to snap to those.
	ESelectionHandle Handles[8] = {HANDLE_TopLeft, HANDLE_Left, HANDLE_BottomLeft, HANDLE_Bottom, HANDLE_BottomRight, HANDLE_Right, HANDLE_TopRight, HANDLE_Top};

	for(INT HandleIdx=0; HandleIdx<8; HandleIdx++)
	{
		FVector2D HandlePos;
		GetHandlePos(Handles[HandleIdx], HandlePos);

		if((AnchorPos - HandlePos).SizeSquared() < ThresholdSquared)
		{
			AnchorOffset = HandlePos - AnchorLocation;
			bSnappedPosition = TRUE;
		}
	}

	// If we were unable to snap to anything, just set the anchor offset equal to the current delta.
	if(bSnappedPosition == FALSE)
	{
		AnchorOffset = Delta;
	}
}

/**
 * Returns the position of a resizing handle.
 *
 * @param Handle	Handle to return the position for.
 * @param OutPos	Storage vector to hold the handle's position.
 */
void FObjectSelectionTool::GetHandlePos(ESelectionHandle Handle, FVector2D &OutPos)
{
	switch ( Handle )
	{
	case HANDLE_Left:
		OutPos.X = StartLocation.X;
		OutPos.Y = StartLocation.Y + ((EndLocation.Y - StartLocation.Y) / 2);
		break;
	case HANDLE_TopLeft:
		OutPos.X = StartLocation.X;
		OutPos.Y = StartLocation.Y;
		break;
	case HANDLE_BottomLeft:
		OutPos.X = StartLocation.X;
		OutPos.Y = EndLocation.Y;
		break;
	case HANDLE_Right:
		OutPos.X = EndLocation.X;
		OutPos.Y = StartLocation.Y + ((EndLocation.Y - StartLocation.Y) / 2);
		break;
	case HANDLE_BottomRight:
		OutPos.X = EndLocation.X;
		OutPos.Y = EndLocation.Y;
		break;
	case HANDLE_TopRight:
		OutPos.X = EndLocation.X;
		OutPos.Y = StartLocation.Y;
		break;
	case HANDLE_Top:
		OutPos.X = StartLocation.X + ((EndLocation.X - StartLocation.X)/2);
		OutPos.Y = StartLocation.Y;
		break;
	case HANDLE_Bottom:
		OutPos.X = StartLocation.X + ((EndLocation.X - StartLocation.X) / 2);
		OutPos.Y = EndLocation.Y;
		break;
	default:
		OutPos.X = OutPos.Y = 0.0f;
		break;
	}
}

// FObjectViewportRenderModifier interface.
void FObjectSelectionTool::Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	if ( ObjectVC->ObjectEditor->HaveObjectsSelected()
	&&	(ObjectVC->ObjectEditor->ShouldRenderSelectionHandles() || Canvas->IsHitTesting()) )
	{
		RenderSelection(ObjectVC, Canvas);
	}
}

/**
 * Renders interactive handles for manipulating the selected widgets, as well as an interactive outline which is used for moving the selected widgets.
 */
void FObjectSelectionTool::RenderSelection( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	TArray<UUIObject*> Widgets;
	ObjectVC->ObjectEditor->Selection->GetSelectedObjects(Widgets);

	// ensure that the selection handles always render on top of the selected widgets,
	INT SelectionZDepth=MAXINT;
	for ( INT WidgetIndex = 0; WidgetIndex < Widgets.Num(); WidgetIndex++ )
	{
		SelectionZDepth = Min(SelectionZDepth, appTrunc(Widgets(WidgetIndex)->GetZDepth()));
	}

	Canvas->PushDepthSortKey(SelectionZDepth - 1);

	Canvas->SetHitProxy( new HSelectionHandle(HANDLE_Outline) );

	const FLinearColor OutlineDrawColor = ActiveHandle == HANDLE_Outline ? OutlineActiveColor : OutlineColor;

	const FLOAT Offset = 2 / ObjectVC->Zoom2D;

	// Top Line.
	DrawTile(Canvas, StartLocation.X - Offset, StartLocation.Y - Offset, EndLocation.X - StartLocation.X + Offset*2, Offset,
		0.0f, 0.0f, 0.0f, 0.0f, OutlineDrawColor, OutlineMaterial ? OutlineMaterial->Resource : NULL);

	// Bottom Line.
	DrawTile(Canvas, StartLocation.X - Offset, EndLocation.Y - Offset, EndLocation.X - StartLocation.X + Offset*2, Offset,
		0.0f, 0.0f, 0.0f, 0.0f, OutlineDrawColor, OutlineMaterial ? OutlineMaterial->Resource : NULL);

	// Left Line.
	DrawTile(Canvas, StartLocation.X - Offset, StartLocation.Y - Offset, Offset, EndLocation.Y - StartLocation.Y + Offset*2,
		0.0f, 0.0f, 0.0f, 0.0f, OutlineDrawColor, OutlineMaterial ? OutlineMaterial->Resource : NULL);

	// Right Line.
	DrawTile(Canvas, EndLocation.X - Offset, StartLocation.Y - Offset, Offset, EndLocation.Y - StartLocation.Y + Offset*2,
		0.0f, 0.0f, 0.0f, 0.0f, OutlineDrawColor, OutlineMaterial ? OutlineMaterial->Resource : NULL);

	// now render the handles
	RenderHandle(ObjectVC, Canvas, HANDLE_Left, 0);
	RenderHandle(ObjectVC, Canvas, HANDLE_TopLeft, 0);
	RenderHandle(ObjectVC, Canvas, HANDLE_Top, 0);
	RenderHandle(ObjectVC, Canvas, HANDLE_TopRight, 0);
	RenderHandle(ObjectVC, Canvas, HANDLE_Right, 0);
	RenderHandle(ObjectVC, Canvas, HANDLE_BottomRight, 0);
	RenderHandle(ObjectVC, Canvas, HANDLE_Bottom, 0);
	RenderHandle(ObjectVC, Canvas, HANDLE_BottomLeft, 0);

	FLOAT OriginalAlphaModulation = Canvas->AlphaModulate;
	if ( IsAnchorTransformable(Widgets) )
	{
		Canvas->SetHitProxy( new HSelectionHandle(HANDLE_Anchor) );
	}
	else
	{
		Canvas->AlphaModulate *= 0.4f;
	}

	if( Widgets.Num() == 1 )
	{
		Canvas->PushRelativeTransform(Widgets(0)->GenerateTransformMatrix());
	}

	// next draw the anchor
	const FLOAT AnchorDrawSize = AnchorSize / ObjectVC->Zoom2D;

	DrawTile(
		Canvas,
		AnchorLocation.X+AnchorOffset.X-AnchorDrawSize/2, AnchorLocation.Y+AnchorOffset.Y-AnchorDrawSize/2, AnchorDrawSize, AnchorDrawSize, 
		0.f, 0.f, 1.f, 1.f,
		ActiveHandle == HANDLE_Anchor ? AnchorActiveColor : AnchorColor,
		AnchorMaterial->Resource
		);

	Canvas->SetHitProxy(NULL);
	Canvas->AlphaModulate = OriginalAlphaModulation;

	if( Widgets.Num() == 1 )
	{
		Canvas->PopTransform();
	}

	Canvas->PopDepthSortKey();
}

/**
 * Render the specified handle.  HANDLE_Outline and HANDLE_Anchor are ignored (rendered by RenderSelection)
 */
void FObjectSelectionTool::RenderHandle(FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas, ESelectionHandle Handle, FLOAT Rotation )
{
	FVector2D DrawPos(0.0f,0.0f);

	GetHandlePos(Handle, DrawPos);

	FLOAT HandleDrawSize = HandleSize / ObjectVC->Zoom2D;
	
	DrawPos.X -= HandleDrawSize / 2;
	DrawPos.Y -= HandleDrawSize / 2;

	Canvas->SetHitProxy( new HSelectionHandle(Handle, Rotation) );
	DrawTile(Canvas,DrawPos.X, DrawPos.Y, HandleDrawSize, HandleDrawSize, 0.f, 0.f, 1.f, 1.f, Handle == ActiveHandle ? HandleActiveColor : HandleColor, HandleMaterial->Resource);
	Canvas->SetHitProxy(NULL);
}

void FObjectSelectionTool::SetPosition( FVector2D Start, FVector2D End )
{
	StartLocation = Start;
	EndLocation = End;
}

/**
 * Sets the currently active handle, indicating that any mouse/keyboard input events should be applied to this handle.
 */
void FObjectSelectionTool::SetActiveHandle( ESelectionHandle Handle )
{
	ActiveHandle = Handle;
}

/**
 * Get the cursor to use when mousing over this hit proxy.
 */
EMouseCursor HSelectionHandle::GetMouseCursor()
{
	EMouseCursor Result = MC_Arrow;

	switch ( Handle )
	{
	case HANDLE_Outline:
		Result = MC_SizeAll;
		break;
	case HANDLE_Left:
		Result = MC_SizeLeftRight;
		break;
	case HANDLE_Right:
		Result = MC_SizeLeftRight;
		break;
	case HANDLE_Top:
		Result = MC_SizeUpDown;
		break;
	case HANDLE_Bottom:
		Result = MC_SizeUpDown;
		break;
	case HANDLE_TopLeft:
		Result = MC_SizeUpLeftDownRight;
		break;
	case HANDLE_BottomRight:
		Result = MC_SizeUpLeftDownRight;
		break;
	case HANDLE_TopRight:
		Result = MC_SizeUpRightDownLeft;
		break;
	case HANDLE_BottomLeft:
		Result = MC_SizeUpRightDownLeft;
		break;
	case HANDLE_Anchor:
		Result = MC_Cross;
		break;
	}

	return Result;
}

FObjectDragTool::FObjectDragTool( FEditorObjectViewportClient* InObjectVC ) : 
ObjectVC(InObjectVC),
GridSize(8),
GridOrigin(FVector(0,0,0)),
AbsoluteDistance(0.0f)
{

}

FObjectDragTool::~FObjectDragTool()
{
	ObjectVC = NULL;
}

/**
* Starts a mouse drag box selection.
*
* @param	InViewportClient	the viewport client to draw the selection box in
* @param	Start				Where the mouse was when the drag started
*/
void FObjectDragTool::StartDrag(FEditorLevelViewportClient* InViewportClient, const FVector& InStart)
{
	ViewportClient = InViewportClient;
	Start = InStart;
	AbsoluteDistance = 0.0f;
	LastMousePosition = InStart;

	// Snap to constraints.
	if( bUseSnapping )
	{
		Start = (Start - GridOrigin).GridSnap(GridSize) + GridOrigin;
	}
	End = EndWk = Start;
	
	// Store button state when the drag began.
	bAltDown = ViewportClient->Input->IsAltPressed();
	bShiftDown = ViewportClient->Input->IsShiftPressed();
	bControlDown = ViewportClient->Input->IsCtrlPressed();
	bLeftMouseButtonDown = ViewportClient->Viewport->KeyState(KEY_LeftMouseButton);
	bRightMouseButtonDown = ViewportClient->Viewport->KeyState(KEY_RightMouseButton);
	bMiddleMouseButtonDown = ViewportClient->Viewport->KeyState(KEY_MiddleMouseButton);
}


/**
 * Resets this drag tool's tracking variables.  Used to release an active drag tool without applying whatever changes the drag tool
 * is designed to handle.
 */
UBOOL FObjectDragTool::ResetTracking()
{
	Start = End = EndWk = LastMousePosition = FVector(EC_EventParm);
	bAltDown = bControlDown = bShiftDown = bLeftMouseButtonDown = bMiddleMouseButtonDown = bRightMouseButtonDown = FALSE;
	AbsoluteDistance = 0.f;
	ObjectVC = NULL;
	return TRUE;
}


/**
 * Updates the drag tool's end location with the specified delta.  The end location is
 * snapped to the editor constraints if bUseSnapping is TRUE.
 *
 * @param	InDelta		A delta of mouse movement.
 */
void FObjectDragTool::AddDelta( const FVector& InDelta )
{
	EndWk += InDelta;

	// Snap to constraints.
	if( bUseSnapping )
	{
		End = (EndWk - GridOrigin).GridSnap(GridSize) + GridOrigin;
	}
	else
	{
		End = EndWk;
	}

	AbsoluteDistance += InDelta.Size();
}

/** 
 * Generates a string that describes the current drag operation to the user.
 * 
 * @param OutString		Storage class for the generated string, should have the current delta in it in some way.
 */
void FObjectDragTool::GetDeltaStatusString(FString &OutString) const
{
	FVector Delta;

	Delta = End-Start;

	OutString = FString::Printf(TEXT("%s: (%.2f, %.2f)"), *LocalizeUnrealEd("Delta"), Delta.X, Delta.Y);
}

/* ==========================================================================================================
	FMouseTool
========================================================================================================== */

/** Constructor */
FMouseTool::FMouseTool( FEditorObjectViewportClient* InObjectVC )
: FObjectDragTool(InObjectVC)
{ 
	
}

/**
 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
 */
void FMouseTool::MouseMove(FViewport* Viewport, INT X, INT Y)
{
	if ( ObjectVC != NULL )
	{
		FLOAT ViewportX;
		FLOAT ViewportY;

		ObjectVC->GetViewportPositionFromMousePosition(X, Y, ViewportX, ViewportY);

		EndWk.X = ViewportX;
		EndWk.Y = ViewportY;

		// Snap to constraints.
		if( bUseSnapping )
		{
			End = (EndWk - GridOrigin).GridSnap(GridSize) + GridOrigin;
		}
		else
		{
			End = EndWk;
		}

		// Update the mouse distance variable.
		FVector CurrentMousePos(ViewportX, ViewportY, 0);
		AbsoluteDistance += (CurrentMousePos - LastMousePosition).Size();
		LastMousePosition = CurrentMousePos;
	}
}

/* ==========================================================================================================
	FMouseTool_ObjectBoxSelect
========================================================================================================== */

/** Constructor */
FMouseTool_ObjectBoxSelect::FMouseTool_ObjectBoxSelect( FEditorObjectViewportClient* InObjectVC )
: FMouseTool(InObjectVC)
{ }

/** Destructor */
FMouseTool_ObjectBoxSelect::~FMouseTool_ObjectBoxSelect()
{
	ObjectVC = NULL;
}

/**
 * Starts a mouse drag behavior.  Called when the user begins dragging the mouse on an empty area of the workspace.
 *
 * @param InStart		Where the mouse was when the drag started
 */

void FMouseTool_ObjectBoxSelect::StartDrag( FEditorLevelViewportClient* InViewportClient, const FVector& InStart )
{
	FMouseTool::StartDrag(InViewportClient, InStart);

	// use the Viewport to determine whether the system keys are pressed, rather than the viewport client's Input.  If the user clicked on this viewport
	// while the system keys were held down, the viewport's Input won't know about it, so by using ViewportClient->Viewport we can begin a drag operation
	// immediately instead of requiring the user to focus this viewport before pressing the system keys
	bShiftDown = IsShiftDown(ViewportClient->Viewport);

	// If the user is selecting, but isn't hold down SHIFT, remove all current selections.

	// If the user is selecting, but isn't hold down SHIFT, remove all current selections.
	if ( !bShiftDown && ObjectVC->ObjectEditor->HaveObjectsSelected() )
	{
		ObjectVC->ObjectEditor->EmptySelection();
	}
}

/**
 * Allow this drag tool to render to the screen
 */
void FMouseTool_ObjectBoxSelect::Render( FCanvas* Canvas )
{
	DrawBox2D(Canvas,FVector2D(Start.X,Start.Y), FVector2D(End.X,End.Y), FColor(255,0,0));
}

/**
 * Ends the mouse drag movement.  Called when the user releases the mouse button while this drag tool is active.
 */
void FMouseTool_ObjectBoxSelect::EndDrag()
{
	// Create a bounding box based on the start/end points (normalizes the points).
	FBox SelBBox(0);
	SelBBox += Start;
	SelBBox += End;

	if ( ObjectVC->ObjectEditor->BoxSelect(SelBBox) )
	{
		GEditor->NoteSelectionChange();
	}

	// Clean up.
	FMouseTool::EndDrag();
}

/** 
 * Generates a string that describes the current drag operation to the user.
 * 
 * @param OutString		Storage class for the generated string, should have the current delta in it in some way.
 */
void FMouseTool_ObjectBoxSelect::GetDeltaStatusString(FString &OutString) const
{
	FVector Delta = End-Start;

	OutString = FString::Printf(LocalizeSecure(LocalizeUnrealEd("ObjectEditor_MouseDelta_SizeF"), Delta.X, Delta.Y));
}

/* ==========================================================================================================
	FMouseTool_DragSelection
========================================================================================================== */

/** Constructor */
FMouseTool_DragSelection::FMouseTool_DragSelection( FEditorObjectViewportClient* InObjectVC )
: FMouseTool(InObjectVC),
  bPreserveProportion(FALSE)
{
	
}

/** Destructor */
FMouseTool_DragSelection::~FMouseTool_DragSelection()
{
	
}

/**
 * Starts a mouse drag behavior.
 *
 * @param InStart		Where the mouse was when the drag started
 */

void FMouseTool_DragSelection::StartDrag( FEditorLevelViewportClient* InViewportClient, const FVector& InStart )
{
	FObjectDragTool::StartDrag(InViewportClient, InStart);


	/** If they held shift down while starting to drag, then we preserve the widget's proportion. */

	if(IsShiftDown(ObjectVC->Viewport) == TRUE)
	{
		bPreserveProportion = TRUE;
	}
	else
	{
		bPreserveProportion = FALSE;
	}

	SelectionHandle = ObjectVC->Selector->ActiveHandle;
}

/**
 * Ends the mouse drag movement.  Called when the user releases the mouse button while this drag tool is active.
 */
void FMouseTool_DragSelection::EndDrag()
{
	if(SelectionHandle == HANDLE_Rotation)
	{
		// create a position vector of the current location of the anchor handle
		FVector Anchor = FVector(ObjectVC->Selector->AnchorLocation.X, ObjectVC->Selector->AnchorLocation.Y, 0.0f);

		// there might be an easier way to do this, but this works, so leave it

		// get the rotation (relative to the anchor handle) of the point where we started dragging the mouse
		FRotator DragStartLocation = FVector(Anchor - Start).Rotation();

		// get the rotation (relative to the anchor handle) of the point where we released the mouse
		FRotator DragEndLocation = FVector(Anchor - End).Rotation();

		// calculate the difference between these two rotations
		FRotator DragDelta = DragEndLocation - DragStartLocation;
		
		ObjectVC->ObjectEditor->RotateSelectedObjects( Anchor, DragDelta );
	}
	else
	{
		ObjectVC->ObjectEditor->MoveSelectedObjects( SelectionHandle, GetDelta(), bPreserveProportion );
	}


	// Clean up.
	FMouseTool::EndDrag();
}

/**
 * Resets this drag tool's tracking variables.  Used to release an active drag tool without applying whatever changes the drag tool
 * is designed to handle.
 */
UBOOL FMouseTool_DragSelection::ResetTracking()
{
	SelectionHandle = HANDLE_None;
	return FMouseTool::ResetTracking();
}

/**
 * Allow this drag tool to render to the screen
 */
void FMouseTool_DragSelection::Render(FCanvas* Canvas)
{
	ObjectVC->ObjectEditor->DrawDragSelection( Canvas, this );
}

/**
 * Called when the user moves the mouse while this viewport is not capturing mouse input (i.e. not dragging).
 */
void FMouseTool_DragSelection::MouseMove(FViewport* Viewport, INT X, INT Y)
{
	FMouseTool::MouseMove(Viewport, X, Y);

	// If we are dragging the anchor, then update the anchor's position as we move the mouse.
	if(SelectionHandle == HANDLE_Anchor)
	{
		//@fixme ronp - think this is where I was handling moving the anchor position freely
		FLOAT ViewportX, ViewportY;
		ObjectVC->GetViewportPositionFromMousePosition(X, Y, ViewportX, ViewportY);
	}
}


/**
 * Returns the angle of the vector for the current mouse drag operation, in degrees.
 *
 * @return a float angle in DEGREES between 0 and 360.
 */
FLOAT FMouseTool_DragSelection::GetRotationDelta() const
{
	FVector Anchor = FVector(ObjectVC->Selector->AnchorLocation.X, ObjectVC->Selector->AnchorLocation.Y, 0.0f);
	FLOAT StartAngle = GetRotationAngle(Anchor, Start);
	FLOAT EndAngle = GetRotationAngle(Anchor, End);

	return EndAngle-StartAngle;
}

/** 
 * Generates a string that describes the current drag operation to the user.
 * 
 * @param OutString		Storage class for the generated string, should have the current delta in it in some way.
 */
void FMouseTool_DragSelection::GetDeltaStatusString(FString &OutString) const
{
	FVector Delta = End-Start;

	if(SelectionHandle == HANDLE_Anchor || SelectionHandle == HANDLE_Outline)	// Movement
	{
		OutString = FString::Printf(LocalizeSecure(LocalizeUnrealEd("ObjectEditor_MouseDelta_MoveF"), Delta.X, Delta.Y));
	}
	else if(SelectionHandle == HANDLE_Rotation)									// Rotation
	{
		// Calculate the angle from the anchor to the start position and end position
		FVector Anchor = FVector(ObjectVC->Selector->AnchorLocation.X, ObjectVC->Selector->AnchorLocation.Y, 0.0f);
		FLOAT StartAngle = GetRotationAngle(Anchor, Start);
		FLOAT EndAngle = GetRotationAngle(Anchor, End);

		OutString = FString::Printf(LocalizeSecure(LocalizeUnrealEd("ObjectEditor_MouseDelta_RotationF"), EndAngle-StartAngle, 176));
	}
	else
	{
		if(SelectionHandle == HANDLE_Top || SelectionHandle == HANDLE_Bottom)
		{
			Delta.X = 0.0f;
		}
		else if(SelectionHandle == HANDLE_Left || SelectionHandle == HANDLE_Right)
		{
			Delta.Y = 0.0f;
		}
		
		OutString = FString::Printf(LocalizeSecure(LocalizeUnrealEd("ObjectEditor_MouseDelta_SizeF"), Delta.X, Delta.Y));
	}
}

/**
 *	Returns a angle of rotation around a anchor at a specified point.
 *
 *	@param	Anchor		Anchor of rotation.
 *	@param	Point		A point that we will calculate the angle of rotation at, assuming the origin of rotation is (1,0)
 *
 * @return a float angle in DEGREES between 0 and 360.
 */
FLOAT FMouseTool_DragSelection::GetRotationAngle(const FVector &Anchor, const FVector &Point) const
{
	FVector Delta = (Anchor - Point);
	Delta.Z = 0.0f;
	FRotator Rotator = Delta.Rotation();

	return Rotator.Yaw / 65535.f * 360.0f;
}

/**
 *	This version returns a different mouse cursor depending on which handle we are currently operating on.
 *
 * @return	Returns which mouse cursor to display while this tool is active.
 */
EMouseCursor FMouseTool_DragSelection::GetCursor() const 
{
	EMouseCursor Cursor = MC_Arrow;

	switch(SelectionHandle)
	{
	case HANDLE_Left: 
		Cursor = MC_SizeLeftRight;
		break;
	case HANDLE_Right:
		Cursor = MC_SizeLeftRight;
		break;
	case HANDLE_Top: 
		Cursor = MC_SizeUpDown;
		break;
	case HANDLE_Bottom:
		Cursor = MC_SizeUpDown;
		break;
	case HANDLE_TopRight: 
		Cursor = MC_SizeUpRightDownLeft;
		break;
	case HANDLE_BottomLeft:
		Cursor = MC_SizeUpRightDownLeft;
		break;
	case HANDLE_BottomRight:
		Cursor = MC_SizeUpLeftDownRight;
		break;
	case HANDLE_TopLeft:
		Cursor = MC_SizeUpLeftDownRight;
		break;
	case HANDLE_Anchor: case HANDLE_Outline:
		Cursor = MC_SizeAll;
		break;
	case HANDLE_Rotation:
		{
			Cursor = MC_NoChange;

			// Get a custom cursor depending on the location of the mouse cursor relative to the rotation anchor.
			static ANSICHAR* RotateImages [] = {"Rotate_Right", "Rotate_TopRight", "Rotate_Top", "Rotate_TopLeft", "Rotate_Left", "Rotate_BottomLeft", "Rotate_Bottom", "Rotate_BottomRight"};
			const FVector Anchor = FVector(ObjectVC->Selector->AnchorLocation.X, ObjectVC->Selector->AnchorLocation.Y, 0.0f);
			const FLOAT AngleStep = 360.0f / 8.0f;
			const FLOAT Angle = -GetRotationAngle(Anchor, End) + 180.0f + AngleStep / 2;
			INT CursorIdx = appTrunc(Angle / AngleStep);
			if(CursorIdx > 7 || CursorIdx < 0)
			{
				CursorIdx = 0;
			}

			WxCursor &Cursor = FCursorManager::GetCursor(RotateImages[CursorIdx]);
			wxSetCursor(Cursor);
		}
		break;
	};

	return Cursor;
}

/* ==========================================================================================================
	FDragTool_MoveCamera
========================================================================================================== */
/** Constructor */
FDragTool_MoveCamera::FDragTool_MoveCamera( FEditorObjectViewportClient* InObjectVC ) : 
FObjectDragTool(InObjectVC)
{

}

/**
* Starts a mouse drag behavior.  The start location is snapped to the editor constraints if bUseSnapping is TRUE.
*
* @param	InViewportClient	The viewport client in which the drag event occurred.
* @param	InStart				Where the mouse was when the drag started.
*/
void FDragTool_MoveCamera::StartDrag(FEditorLevelViewportClient* InViewportClient, const FVector& InStart)
{
	FObjectDragTool::StartDrag(InViewportClient, InStart);
}

/**
* Updates the camera's position depending on the current movement mode.
*
* @param	InDelta		A delta of mouse movement.
*/
void FDragTool_MoveCamera::AddDelta( const FVector& InDelta )
{
	// Calculate which mode we are in, we zoom if both the left and right mouse buttons are down.  Translate otherwise.
	if( ObjectVC->Viewport->KeyState(KEY_RightMouseButton) && ObjectVC->Viewport->KeyState(KEY_LeftMouseButton) )
	{
		MoveMode = MM_Zoom;
	}
	else
	{
		MoveMode = MM_Translate;
	}

	switch(MoveMode)
	{
	case MM_Translate:
		ObjectVC->Origin2D.X -= InDelta.X * ObjectVC->Zoom2D;
		ObjectVC->Origin2D.Y -= InDelta.Y * ObjectVC->Zoom2D;
		break;
	case MM_Zoom:
		{
			FLOAT DeltaZoom = InDelta.Y * 0.005f * ObjectVC->Zoom2D;

			const FLOAT ViewCenterX = ((ObjectVC->Viewport->GetSizeX() * 0.5f) - ObjectVC->Origin2D.X) / ObjectVC->Zoom2D;
			const FLOAT ViewCenterY = ((ObjectVC->Viewport->GetSizeY() * 0.5f) - ObjectVC->Origin2D.Y) / ObjectVC->Zoom2D;

			ObjectVC->Zoom2D = Clamp<FLOAT>(ObjectVC->Zoom2D+DeltaZoom,ObjectVC->MinZoom2D,ObjectVC->MaxZoom2D);

			FLOAT DrawOriginX = (ObjectVC->Viewport->GetSizeX() * 0.5f) - ViewCenterX * ObjectVC->Zoom2D;
			FLOAT DrawOriginY = (ObjectVC->Viewport->GetSizeY() * 0.5f) - ViewCenterY * ObjectVC->Zoom2D;

			ObjectVC->Origin2D.X = DrawOriginX;
			ObjectVC->Origin2D.Y = DrawOriginY;

		}
		break;
	default:
		break;
	}

	ObjectVC->ObjectEditor->ViewPosChanged();
	ObjectVC->RefreshViewport();
}

