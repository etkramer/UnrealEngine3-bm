/*=============================================================================
	SpeedTreeEditor.cpp: SpeedTree editor implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "MouseDeltaTracker.h"
#include "Properties.h"
#include "PropertyUtils.h"
#include "..\..\Launch\Resources\resource.h"
#include "SpeedTree.h"

#if WITH_SPEEDTREE

static const FLOAT	LightRotationSpeed = 40.0f;


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FSpeedTreeEditorViewportClient::FSpeedTreeEditorViewportClient

FSpeedTreeEditorViewportClient::FSpeedTreeEditorViewportClient(class WxSpeedTreeEditor* Editor) :
	SpeedTreeEditor(Editor)
{
	EditorComponent = ConstructObject<UEditorComponent>(UEditorComponent::StaticClass( ));
	EditorComponent->bDrawPivot = false;
	EditorComponent->bDrawGrid = false;
	FPreviewScene::AddComponent(EditorComponent, FMatrix::Identity);

	SpeedTreeComponent = ConstructObject<USpeedTreeComponent>(USpeedTreeComponent::StaticClass( ));
	SpeedTreeComponent->SpeedTree = Editor->SpeedTree;
	FPreviewScene::AddComponent(SpeedTreeComponent, FMatrix::Identity);

	FLOAT SphereRadius = SpeedTreeEditor->SpeedTree->SRH->GetBounds( ).SphereRadius;
	ViewLocation = -FVector(0, SphereRadius / (75.0f * (FLOAT)PI / 360.0f), -SphereRadius * 0.5f);
	ViewRotation = FRotator(0, 16384, 0);

	bDrawAxes = 0;
	NearPlane = 1.0f;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FSpeedTreeEditorViewportClient::InputAxis

UBOOL FSpeedTreeEditorViewportClient::InputAxis(FViewport* Viewport, INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime)
{
	// See if we are holding down the 'rotate light' key
	UBOOL bLightMoveDown = Viewport->KeyState(KEY_L);

	// If so, use mouse movement to rotate light direction,
	if (bLightMoveDown)
	{
		// Look at which axis is being dragged and by how much
		FLOAT DragX = (Key == KEY_MouseX) ? Delta : 0.0f;
		FLOAT DragY = (Key == KEY_MouseY) ? Delta : 0.0f;

		FRotator LightDir = GetLightDirection( );

		LightDir.Yaw += -DragX * LightRotationSpeed;
		LightDir.Pitch += -DragY * LightRotationSpeed;

		SetLightDirection(LightDir);
	}
	// If we are not moving light, use the MouseDeltaTracker to update camera.
	else
	{
		if ((Key == KEY_MouseX || Key == KEY_MouseY) && Delta != 0.0f)
		{
			MouseDeltaTracker->AddDelta(this, Key, Delta, 0);
			FVector DragDelta = MouseDeltaTracker->GetDelta( );

			GEditor->MouseMovement += DragDelta;

			if (!DragDelta.IsZero( ))
			{
				// Convert the movement delta into drag/rotation deltas
				FVector Drag;
				FRotator Rot;
				FVector Scale;
				MouseDeltaTracker->ConvertMovementDeltaToDragRot(this, DragDelta, Drag, Rot, Scale);
				MoveViewportCamera(Drag, Rot);
				MouseDeltaTracker->ReduceBy(DragDelta);
			}
		}
	}

	SpeedTreeComponent->ConditionalTick(DeltaTime);
	Viewport->Invalidate( );

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Message Map

IMPLEMENT_DYNAMIC_CLASS(WxSpeedTreeEditor,wxFrame)
BEGIN_EVENT_TABLE(WxSpeedTreeEditor, wxFrame)
	EVT_SIZE(WxSpeedTreeEditor::OnSize)
	EVT_PAINT(WxSpeedTreeEditor::OnPaint)
END_EVENT_TABLE( )


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WxSpeedTreeEditor::WxSpeedTreeEditor

WxSpeedTreeEditor::WxSpeedTreeEditor(wxWindow* Parent, wxWindowID wxID, USpeedTree* SpeedTree) : 
	wxFrame(Parent, wxID, *LocalizeUnrealEd("SpeedTreeEditor"), wxDefaultPosition, wxDefaultSize, wxFRAME_FLOAT_ON_PARENT | wxDEFAULT_FRAME_STYLE | wxFRAME_NO_TASKBAR),
	SpeedTree(SpeedTree)
{
	SplitterWnd = new wxSplitterWindow(this, ID_SPLITTERWINDOW, wxDefaultPosition, wxSize(100, 100), wxSP_3D | wxSP_3DBORDER | wxSP_FULLSASH);

	// Create property window
	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create(SplitterWnd, NULL);
	PropertyWindow->SetObject(SpeedTree, TRUE, FALSE, TRUE);

	// Create viewport
	ViewportHolder = new WxViewportHolder(SplitterWnd, -1, 0);
	ViewportClient = new FSpeedTreeEditorViewportClient(this);
	ViewportClient->Viewport = GEngine->Client->CreateWindowChildViewport(ViewportClient, (HWND)ViewportHolder->GetHandle( ));
	ViewportClient->Viewport->CaptureJoystickInput(false);
	ViewportHolder->SetViewport(ViewportClient->Viewport);
	ViewportHolder->Show( );

	FWindowUtil::LoadPosSize(TEXT("SpeedTreeEditor"), this, 64, 64, 800, 450);

	wxRect rc = GetClientRect( );

	SplitterWnd->SplitVertically(ViewportHolder, PropertyWindow, rc.GetWidth( ) - 350);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WxSpeedTreeEditor::~WxSpeedTreeEditor

WxSpeedTreeEditor::~WxSpeedTreeEditor( )
{
	FWindowUtil::SavePosSize(TEXT("SpeedTreeEditor"), this);

	GEngine->Client->CloseViewport(ViewportClient->Viewport);
	ViewportClient->Viewport = NULL;
	delete ViewportClient;
	delete PropertyWindow;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WxSpeedTreeEditor::OnSize

void WxSpeedTreeEditor::OnSize(wxSizeEvent& wxIn)
{
	wxPoint wxOrigin = GetClientAreaOrigin( );
	wxRect wxRect = GetClientRect( );
	wxRect.y -= wxOrigin.y;
	SplitterWnd->SetSize(wxRect);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WxSpeedTreeEditor::Serialize

void WxSpeedTreeEditor::Serialize(FArchive& Ar)
{
	Ar << SpeedTree;
	check(ViewportClient);
	ViewportClient->Serialize(Ar);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WxSpeedTreeEditor::OnPaint

void WxSpeedTreeEditor::OnPaint(wxPaintEvent& wxIn)
{
	wxPaintDC dc(this);
	ViewportClient->Viewport->Invalidate( );
}

#endif

