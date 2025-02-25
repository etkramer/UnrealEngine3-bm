/*=============================================================================
	UnEdModeInterpolation : Editor mode for setting up interpolation sequences.

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"
#include "InterpEditor.h"

static const FLOAT	CurveHandleScale = 0.5f;

//////////////////////////////////////////////////////////////////////////
// FEdModeInterpEdit
//////////////////////////////////////////////////////////////////////////

FEdModeInterpEdit::FEdModeInterpEdit()
{
	ID = EM_InterpEdit;
	Desc = *LocalizeUnrealEd("InterpEdit");

	Interp = NULL;
	InterpEd = NULL;

	Tools.AddItem( new FModeTool_InterpEdit() );
	SetCurrentTool( MT_InterpEdit );

	Settings = NULL;

	bLeavingMode = false;
}

FEdModeInterpEdit::~FEdModeInterpEdit()
{
}



UBOOL FEdModeInterpEdit::InputKey( FEditorLevelViewportClient* ViewportClient, FViewport* Viewport, FName Key, EInputEvent Event )
{
	// Enter key drops new key frames
	if( Key == KEY_Enter &&
		( Event == IE_Pressed || Event == IE_Repeat ) &&
		( !ViewportClient->Input->IsShiftPressed() &&
		  !ViewportClient->Input->IsAltPressed() &&
		  !ViewportClient->Input->IsCtrlPressed() ) )
	{
		if( InterpEd != NULL )
		{
			// Add a new key!
			InterpEd->AddKey();
		}

		return TRUE;
	}

	return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}



void FEdModeInterpEdit::Enter()
{
	FEdMode::Enter();
}

void FEdModeInterpEdit::Exit()
{
	Interp = NULL;

	// If there is one, close the Interp Editor and clear pointers.
	if(InterpEd != NULL)
	{
		bLeavingMode = true; // This is so the editor being closed doesn't try and change the mode again!

		InterpEd->Close(true);

		bLeavingMode = false;
	}

	InterpEd = NULL;

	FEdMode::Exit();
}

// We see if we have 
void FEdModeInterpEdit::ActorMoveNotify()
{
	if(!InterpEd)
		return;

	InterpEd->ActorModified();
}


void FEdModeInterpEdit::CamMoveNotify(FEditorLevelViewportClient* ViewportClient)
{
	if(!InterpEd)
		return;

	if( ViewportClient->AllowMatineePreview() )
	{
		InterpEd->CamMoved(ViewportClient->ViewLocation, ViewportClient->ViewRotation);
	}
}

void FEdModeInterpEdit::ActorSelectionChangeNotify()
{
	if(!InterpEd)
		return;

	InterpEd->ActorSelectionChange();
}

void FEdModeInterpEdit::ActorPropChangeNotify()
{
	if(!InterpEd)
		return;

	InterpEd->ActorModified();
}

// Set the currently edited SeqAct_Interp and open timeline window. Should always be called after we change to EM_InterpEdit.
void FEdModeInterpEdit::InitInterpMode(USeqAct_Interp* EditInterp)
{
	check(EditInterp);
	check(!InterpEd);

	Interp = EditInterp;

	InterpEd = ((UUnrealEdEngine *)GEditor)->CreateInterpEditor( GApp->EditorFrame, -1, Interp );
	InterpEd->Show(1);
}

WxCameraAnimEd* FEdModeInterpEdit::InitCameraAnimMode(class USeqAct_Interp* EditInterp)
{
	check(EditInterp);
	check(!InterpEd);

	// open up the matinee window
	WxCameraAnimEd* CamAnimEd = ((UUnrealEdEngine *)GEditor)->CreateCameraAnimEditor( GApp->EditorFrame, -1, EditInterp );
	CamAnimEd->Show(1);

	Interp = EditInterp;
	InterpEd = CamAnimEd;

	return CamAnimEd;
}

void FEdModeInterpEdit::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View, Viewport, PDI);

	check( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit );

	if(InterpEd  && !InterpEd->bHide3DTrackView)
	{
		InterpEd->DrawTracks3D(View, PDI);
	}
}

void FEdModeInterpEdit::DrawHUD(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,const FSceneView* View,FCanvas* Canvas)
{
	FEdMode::DrawHUD(ViewportClient,Viewport,View,Canvas);

	if(InterpEd)
		InterpEd->DrawModeHUD(ViewportClient,Viewport,View,Canvas);
}

UBOOL FEdModeInterpEdit::AllowWidgetMove()
{
	FModeTool_InterpEdit* InterpTool = (FModeTool_InterpEdit*)FindTool(MT_InterpEdit);

	if(InterpTool->bMovingHandle)
	{
		return false;
	}
	else
	{
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
// FModeTool_InterpEdit
//////////////////////////////////////////////////////////////////////////

FModeTool_InterpEdit::FModeTool_InterpEdit()
{
	ID = MT_InterpEdit;

	bMovingHandle = false;
	DragGroup = false;
	DragTrackIndex = false;
	DragKeyIndex = false;
	bDragArriving = false;
}

FModeTool_InterpEdit::~FModeTool_InterpEdit()
{
}

UBOOL FModeTool_InterpEdit::MouseMove(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,INT x, INT y)
{
	check( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit );

	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();

	return 1;
}

/**
 * @return		TRUE if the key was handled by this editor mode tool.
 */
UBOOL FModeTool_InterpEdit::InputKey(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,FName Key,EInputEvent Event)
{
	check( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit );

	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
	if( !mode->InterpEd )
	{
		// Abort cleanly on InerpEd not yet being assigned.  This can occasionally be the case when receiving
		// modifier key release events when changing into interp edit mode.
		return FALSE;
	}

	UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);
	UBOOL bAltDown = Viewport->KeyState(KEY_LeftAlt) || Viewport->KeyState(KEY_RightAlt);

	if( Key == KEY_LeftMouseButton )
	{

		if( Event == IE_Pressed)
		{
			INT HitX = ViewportClient->Viewport->GetMouseX();
			INT HitY = ViewportClient->Viewport->GetMouseY();
			HHitProxy*	HitResult = ViewportClient->Viewport->GetHitProxy(HitX, HitY);

			if(HitResult)
			{
				if( HitResult->IsA(HInterpTrackKeypointProxy::StaticGetType()) )
				{
					HInterpTrackKeypointProxy* KeyProxy = (HInterpTrackKeypointProxy*)HitResult;
					UInterpGroup* Group = KeyProxy->Group;
					INT TrackIndex = KeyProxy->TrackIndex;
					INT KeyIndex = KeyProxy->KeyIndex;

					// This will invalidate the display - so we must not access the KeyProxy after this!
					mode->InterpEd->SetActiveTrack(Group, TrackIndex);

					if(!bCtrlDown)
						mode->InterpEd->ClearKeySelection();

					mode->InterpEd->AddKeyToSelection(Group, TrackIndex, KeyIndex, true);
				}
				else if( HitResult->IsA(HInterpTrackKeyHandleProxy::StaticGetType()) )
				{
					// If we clicked on a 3D track handle, remember which key.
					HInterpTrackKeyHandleProxy* KeyProxy = (HInterpTrackKeyHandleProxy*)HitResult;
					DragGroup = KeyProxy->Group;
					DragTrackIndex = KeyProxy->TrackIndex;
					DragKeyIndex = KeyProxy->KeyIndex;
					bDragArriving = KeyProxy->bArriving;

					bMovingHandle = true;

					mode->InterpEd->BeginDrag3DHandle(DragGroup, DragTrackIndex);
				}
			}
		}
		else if( Event == IE_Released)
		{
			if(bMovingHandle)
			{
				mode->InterpEd->EndDrag3DHandle();
				bMovingHandle = false;
			}
		}
	}

	// Handle keys
	if( Event == IE_Pressed )
	{
		if( Key == KEY_Delete )
		{
			// Swallow 'Delete' key to avoid deleting stuff when trying to interpolate it!
			return TRUE;
		}
		else if( mode->InterpEd->ProcessKeyPress( Key, bCtrlDown, bAltDown ) )
		{
			return TRUE;
		}
	}

	return FModeTool::InputKey(ViewportClient, Viewport, Key, Event);
}

/**
 * @return		TRUE if the delta was handled by this editor mode tool.
 */
UBOOL FModeTool_InterpEdit::InputDelta(FEditorLevelViewportClient* InViewportClient,FViewport* InViewport,FVector& InDrag,FRotator& InRot,FVector& InScale)
{
	check( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit );

	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
	check(mode->InterpEd);

	UBOOL bShiftDown = InViewport->KeyState(KEY_LeftShift) || InViewport->KeyState(KEY_RightShift);

	FVector InputDeltaDrag( InDrag );

	// If we are grabbing a 'handle' on the movement curve, pass that info to Matinee
	if(bMovingHandle)
	{
		// We seem to need this - not sure why...
		if ( InViewportClient->ViewportType == LVT_OrthoXY )
		{
			InputDeltaDrag.X = InDrag.Y;
			InputDeltaDrag.Y = -InDrag.X;
		}

		mode->InterpEd->Move3DHandle( DragGroup, DragTrackIndex, DragKeyIndex, bDragArriving, InputDeltaDrag * (1.f/CurveHandleScale) );

		return 1;
	}
	// If shift is downOnly do 'move initial position' if dragging the widget
	else if(bShiftDown && InViewportClient->Widget->GetCurrentAxis() != AXIS_None)
	{
		mode->InterpEd->MoveInitialPosition( InputDeltaDrag, InRot );

		return 1;
	}

	InViewportClient->Viewport->Invalidate();

	return 0;
}

void FModeTool_InterpEdit::SelectNone()
{
	check( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit );

	FEdModeInterpEdit* mode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();

}

//////////////////////////////////////////////////////////////////////////
// WxModeBarInterpEdit
//////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( WxModeBarInterpEdit, WxModeBar )
END_EVENT_TABLE()

WxModeBarInterpEdit::WxModeBarInterpEdit( wxWindow* InParent, wxWindowID InID )
	: WxModeBar( (wxWindow*)InParent, InID )
{
	SetSize( wxRect( -1, -1, 400, 1) );
	LoadData();
	Show(0);
}

WxModeBarInterpEdit::~WxModeBarInterpEdit()
{
}
