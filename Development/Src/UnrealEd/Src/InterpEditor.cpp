/*=============================================================================
	InterpEditor.cpp: Interpolation editing
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "CurveEd.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"
#include "InterpEditor.h"
#include "UnLinkedObjDrawUtils.h"
#include "Properties.h"
#include "GenericBrowser.h"
#include "Kismet.h"

IMPLEMENT_CLASS(UInterpEdOptions);


static const FLOAT InterpEditor_ZoomIncrement = 1.2f;

static const FColor PositionMarkerLineColor(255, 222, 206);
static const FColor LoopRegionFillColor(80,255,80,24);
static const FColor Track3DSelectedColor(255,255,0);

/*-----------------------------------------------------------------------------
	UInterpEdTransBuffer / FInterpEdTransaction
-----------------------------------------------------------------------------*/

void UInterpEdTransBuffer::BeginSpecial(const TCHAR* SessionName)
{
	CheckState();
	if( ActiveCount++==0 )
	{
		// Cancel redo buffer.
		//debugf(TEXT("BeginTrans %s"), SessionName);
		if( UndoCount )
		{
			UndoBuffer.Remove( UndoBuffer.Num()-UndoCount, UndoCount );
		}

		UndoCount = 0;

		// Purge previous transactions if too much data occupied.
		while( GetUndoSize() > MaxMemory )
		{
			UndoBuffer.Remove( 0 );
		}

		// Begin a new transaction.
		GUndo = new(UndoBuffer)FInterpEdTransaction( SessionName, 1 );
	}
	CheckState();
}

void UInterpEdTransBuffer::EndSpecial()
{
	CheckState();
	check(ActiveCount>=1);
	if( --ActiveCount==0 )
	{
		GUndo = NULL;
	}
	CheckState();
}

void FInterpEdTransaction::SaveObject( UObject* Object )
{
	check(Object);

	if( Object->IsA( USeqAct_Interp::StaticClass() ) ||
		Object->IsA( UInterpData::StaticClass() ) ||
		Object->IsA( UInterpGroup::StaticClass() ) ||
		Object->IsA( UInterpTrack::StaticClass() ) ||
		Object->IsA( UInterpGroupInst::StaticClass() ) ||
		Object->IsA( UInterpTrackInst::StaticClass() ) ||
		Object->IsA( UInterpEdOptions::StaticClass() ) )
	{
		// Save the object.
		new( Records )FObjectRecord( this, Object, NULL, 0, 0, 0, 0, NULL, NULL );
	}
}

void FInterpEdTransaction::SaveArray( UObject* Object, FScriptArray* Array, INT Index, INT Count, INT Oper, INT ElementSize, STRUCT_AR Serializer, STRUCT_DTOR Destructor )
{
	// Never want this.
}

IMPLEMENT_CLASS(UInterpEdTransBuffer);

/*-----------------------------------------------------------------------------
 FInterpEdViewportClient
-----------------------------------------------------------------------------*/

FInterpEdViewportClient::FInterpEdViewportClient( class WxInterpEd* InInterpEd )
{
	InterpEd = InInterpEd;

	// This window will be 2D/canvas only, so set the viewport type to None
	ViewportType = LVT_None;

	// Set defaults for members.  These should be initialized by the owner after construction.
	bIsDirectorTrackWindow = FALSE;
	bWantFilterTabs = FALSE;
	bWantTimeline = FALSE;

	// Scroll bar starts at the top of the list!
	ThumbPos_Vert = 0;

	OldMouseX = 0;
	OldMouseY = 0;

	DistanceDragged = 0;

	BoxStartX = 0;
	BoxStartY = 0;
	BoxEndX = 0;
	BoxEndY = 0;

	bPanning = false;
	bMouseDown = false;
	bGrabbingHandle = false;
	bBoxSelecting = false;
	bTransactionBegun = false;
	bNavigating = false;
	bGrabbingMarker	= false;

	DragObject = NULL;

	SetRealtime( false );
}

FInterpEdViewportClient::~FInterpEdViewportClient()
{

}

UBOOL FInterpEdViewportClient::InputKey(FViewport* Viewport, INT ControllerId, FName Key, EInputEvent Event,FLOAT /*AmountDepressed*/,UBOOL /*Gamepad*/)
{
	const UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);
	const UBOOL bShiftDown = Viewport->KeyState(KEY_LeftShift) || Viewport->KeyState(KEY_RightShift);
	const UBOOL bAltDown = Viewport->KeyState(KEY_LeftAlt) || Viewport->KeyState(KEY_RightAlt);

	const INT HitX = Viewport->GetMouseX();
	const INT HitY = Viewport->GetMouseY();

	if( Key == KEY_LeftMouseButton )
	{
		switch( Event )
		{
		case IE_Pressed:
			{
				if(DragObject == NULL)
				{
					HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);

					if(HitResult)
					{
						if(HitResult->IsA(HInterpEdGroupTitle::StaticGetType()))
						{
							UInterpGroup* Group = ((HInterpTrackKeypointProxy*)HitResult)->Group;

							InterpEd->SetActiveTrack(Group, INDEX_NONE);

							InterpEd->ClearKeySelection();
						}
						else if(HitResult->IsA(HInterpEdGroupCollapseBtn::StaticGetType()))
						{
							UInterpGroup* Group = ((HInterpTrackKeypointProxy*)HitResult)->Group;

							InterpEd->SetActiveTrack(Group, INDEX_NONE);
							Group->bCollapsed = !Group->bCollapsed;

							// A group has been expanded or collapsed, so we need to update our scroll bar
							InterpEd->UpdateTrackWindowScrollBars();

							InterpEd->ClearKeySelection();
						}
						else if(HitResult->IsA(HInterpEdGroupLockCamBtn::StaticGetType()))
						{
							UInterpGroup* Group = ((HInterpEdGroupLockCamBtn*)HitResult)->Group;

							if(Group == InterpEd->CamViewGroup)
							{
								InterpEd->LockCamToGroup(NULL);
							}
							else
							{
								InterpEd->LockCamToGroup(Group);
							}
						}
						else if(HitResult->IsA(HInterpEdTrackTitle::StaticGetType()))
						{
							UInterpGroup* Group = ((HInterpTrackKeypointProxy*)HitResult)->Group;
							const INT TrackIndex = ((HInterpEdTrackTitle*)HitResult)->TrackIndex;

							InterpEd->SetActiveTrack(Group, TrackIndex);

							InterpEd->ClearKeySelection();
						}
						else if(HitResult->IsA(HInterpEdTrackTrajectoryButton::StaticGetType()))
						{
							UInterpGroup* Group = ((HInterpEdTrackGraphPropBtn*)HitResult)->Group;
							const INT TrackIndex = ((HInterpEdTrackGraphPropBtn*)HitResult)->TrackIndex;

							// Should always be a movement track
							UInterpTrackMove* MovementTrack = Cast<UInterpTrackMove>( Group->InterpTracks( TrackIndex )	);
							if( MovementTrack != NULL )
							{
								// Toggle the 3D trajectory for this track
								InterpEd->InterpEdTrans->BeginSpecial( *LocalizeUnrealEd( "InterpEd_Undo_ToggleTrajectory" ) );
								MovementTrack->Modify();
								MovementTrack->bHide3DTrack = !MovementTrack->bHide3DTrack;
								InterpEd->InterpEdTrans->EndSpecial();
							}
						}
						else if(HitResult->IsA(HInterpEdTrackGraphPropBtn::StaticGetType()))
						{
							UInterpGroup* Group = ((HInterpEdTrackGraphPropBtn*)HitResult)->Group;
							const INT TrackIndex = ((HInterpEdTrackGraphPropBtn*)HitResult)->TrackIndex;

							InterpEd->AddTrackToCurveEd(Group, TrackIndex);
						}
						else if(HitResult->IsA(HInterpEdEventDirBtn::StaticGetType()))
						{
							UInterpGroup* Group = ((HInterpEdEventDirBtn*)HitResult)->Group;
							const INT TrackIndex = ((HInterpEdEventDirBtn*)HitResult)->TrackIndex;
							EInterpEdEventDirection Dir = ((HInterpEdEventDirBtn*)HitResult)->Dir;

							UInterpTrackEvent* EventTrack = CastChecked<UInterpTrackEvent>( Group->InterpTracks(TrackIndex) );

							if(Dir == IED_Forward)
							{
								EventTrack->bFireEventsWhenForwards = !EventTrack->bFireEventsWhenForwards;
							}
							else
							{
								EventTrack->bFireEventsWhenBackwards = !EventTrack->bFireEventsWhenBackwards;
							}
						}
						else if(HitResult->IsA(HInterpTrackKeypointProxy::StaticGetType()))
						{
							UInterpGroup* Group = ((HInterpTrackKeypointProxy*)HitResult)->Group;
							const INT TrackIndex = ((HInterpTrackKeypointProxy*)HitResult)->TrackIndex;
							const INT KeyIndex = ((HInterpTrackKeypointProxy*)HitResult)->KeyIndex;

							if(!bCtrlDown)
							{
								InterpEd->ClearKeySelection();
								InterpEd->AddKeyToSelection(Group, TrackIndex, KeyIndex, !bShiftDown);
								if ( bShiftDown )
								{
									InterpEd->SetActiveTrack( Group, TrackIndex );
								}
							}
						}
						else if(HitResult->IsA(HInterpEdTrackBkg::StaticGetType()))
						{
							InterpEd->SetActiveTrack(NULL, INDEX_NONE);
						}
						else if(HitResult->IsA(HInterpEdTimelineBkg::StaticGetType()))
						{
							FLOAT NewTime = InterpEd->ViewStartTime + ((HitX - InterpEd->LabelWidth) / InterpEd->PixelsPerSec);
							if( InterpEd->bSnapToFrames && InterpEd->bSnapTimeToFrames )
							{
								NewTime = InterpEd->SnapTimeToNearestFrame( NewTime );
							}

							// When jumping to location by clicking, stop playback.
							InterpEd->Interp->Stop();
							SetRealtime( false );

							// Move to clicked on location
							InterpEd->SetInterpPosition(NewTime);

							// Act as if we grabbed the handle as well.
							bGrabbingHandle = true;
						}
						else if(HitResult->IsA(HInterpEdNavigatorBackground::StaticGetType()))
						{
							// Clicked on the navigator background, so jump directly to the position under the
							// mouse cursor and wait for a drag
							const FLOAT JumpToTime = ((HitX - InterpEd->LabelWidth)/InterpEd->NavPixelsPerSecond);
							const FLOAT ViewWindow = (InterpEd->ViewEndTime - InterpEd->ViewStartTime);

							InterpEd->ViewStartTime = JumpToTime - (0.5f * ViewWindow);
							InterpEd->ViewEndTime = JumpToTime + (0.5f * ViewWindow);
							InterpEd->SyncCurveEdView();

							bNavigating = true;
						}
						else if(HitResult->IsA(HInterpEdNavigator::StaticGetType()))
						{
							// Clicked on the navigator foreground, so just start the drag immediately without
							// jumping the timeline
							bNavigating = true;
						}
						else if(HitResult->IsA(HInterpEdMarker::StaticGetType()))
						{
							InterpEd->GrabbedMarkerType = ((HInterpEdMarker*)HitResult)->Type;

							InterpEd->BeginMoveMarker();
							bGrabbingMarker = true;
						}
						else if(HitResult->IsA(HInterpEdTab::StaticGetType()))
						{
							InterpEd->SetSelectedFilter(((HInterpEdTab*)HitResult)->Filter);

							Viewport->Invalidate();	
						}
						else if(HitResult->IsA(HInterpEdTrackDisableTrackBtn::StaticGetType()))
						{
							HInterpEdTrackDisableTrackBtn* TrackProxy = ((HInterpEdTrackDisableTrackBtn*)HitResult);

							if(TrackProxy->Group != NULL)
							{
								if(TrackProxy->TrackIndex != INDEX_NONE)
								{
									UInterpTrack* Track = TrackProxy->Group->InterpTracks(TrackProxy->TrackIndex);

									InterpEd->InterpEdTrans->BeginSpecial( *LocalizeUnrealEd( "InterpEd_Undo_ToggleTrackEnabled" ) );

									Track->Modify();
									Track->bDisableTrack = !Track->bDisableTrack;

									InterpEd->InterpEdTrans->EndSpecial();

									// Update the preview
									InterpEd->RefreshInterpPosition();
								}
							}
						}
						else if(HitResult->IsA(HInterpEdInputInterface::StaticGetType()))
						{
							HInterpEdInputInterface* Proxy = ((HInterpEdInputInterface*)HitResult);

							DragObject = Proxy->ClickedObject;
							DragData = Proxy->InputData;
							DragData.PixelsPerSec = InterpEd->PixelsPerSec;
							DragData.MouseStart = FIntPoint(HitX, HitY);
							DragData.bCtrlDown = bCtrlDown;
							DragData.bAltDown = bAltDown;
							DragData.bShiftDown = bShiftDown;
							Proxy->ClickedObject->BeginDrag(DragData);
						}
					}
					else
					{
						if(bCtrlDown && bAltDown)
						{
							BoxStartX = BoxEndX = HitX;
							BoxStartY = BoxEndY = HitY;

							bBoxSelecting = true;
						}
						else
						{
							bPanning = true;
						}
					}

					Viewport->LockMouseToWindow(true);

					bMouseDown = true;
					OldMouseX = HitX;
					OldMouseY = HitY;
					DistanceDragged = 0;
				}
			}
			break;
		case IE_DoubleClick:
			{
				HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);

				if(HitResult)
				{
					if(HitResult->IsA(HInterpEdGroupTitle::StaticGetType()))
					{
						UInterpGroup* Group = ((HInterpTrackKeypointProxy*)HitResult)->Group;

						Group->bCollapsed = !Group->bCollapsed;

						// A group has been expanded or collapsed, so we need to update our scroll bar
						InterpEd->UpdateTrackWindowScrollBars();
					}
				}
			}
			break;
		case IE_Released:
			{
				if(bBoxSelecting)
				{
					const INT MinX = Min(BoxStartX, BoxEndX);
					const INT MinY = Min(BoxStartY, BoxEndY);
					const INT MaxX = Max(BoxStartX, BoxEndX);
					const INT MaxY = Max(BoxStartY, BoxEndY);
					const INT TestSizeX = MaxX - MinX + 1;
					const INT TestSizeY = MaxY - MinY + 1;

					// Find how much (in time) 1.5 pixels represents on the screen.
					const FLOAT PixelTime = 1.5f/InterpEd->PixelsPerSec;

					// We read back the hit proxy map for the required region.
					TArray<HHitProxy*> ProxyMap;
					Viewport->GetHitProxyMap((UINT)MinX, (UINT)MinY, (UINT)MaxX, (UINT)MaxY, ProxyMap);

					TArray<FInterpEdSelKey>	NewSelection;

					// Find any keypoint hit proxies in the region - add the keypoint to selection.
					for(INT Y=0; Y < TestSizeY; Y++)
					{
						for(INT X=0; X < TestSizeX; X++)
						{
							HHitProxy* HitProxy = ProxyMap(Y * TestSizeX + X);

							if(HitProxy && HitProxy->IsA(HInterpTrackKeypointProxy::StaticGetType()))
							{
								UInterpGroup* Group = ((HInterpTrackKeypointProxy*)HitProxy)->Group;
								const INT TrackIndex = ((HInterpTrackKeypointProxy*)HitProxy)->TrackIndex;
								const INT KeyIndex = ((HInterpTrackKeypointProxy*)HitProxy)->KeyIndex;

								// Because AddKeyToSelection might invalidate the display, we just remember all the keys here and process them together afterwards.
								NewSelection.AddUniqueItem( FInterpEdSelKey(Group, TrackIndex, KeyIndex) );

								// Slight hack here. We select any other keys on the same track which are within 1.5 pixels of this one.
								UInterpTrack* Track = Group->InterpTracks(TrackIndex);
								const FLOAT SelKeyTime = Track->GetKeyframeTime(KeyIndex);

								for(INT i=0; i<Track->GetNumKeyframes(); i++)
								{
									const FLOAT KeyTime = Track->GetKeyframeTime(i);
									if( Abs(KeyTime - SelKeyTime) < PixelTime )
									{
										NewSelection.AddUniqueItem( FInterpEdSelKey(Group, TrackIndex, i) );
									}
								}
							}
						}
					}

					if(!bShiftDown)
					{
						InterpEd->ClearKeySelection();
					}

					for(INT i=0; i<NewSelection.Num(); i++)
					{
						InterpEd->AddKeyToSelection( NewSelection(i).Group, NewSelection(i).TrackIndex, NewSelection(i).KeyIndex, false );
					}
				}
				else if(DragObject)
				{
					HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);

					if(HitResult)
					{
						if(HitResult->IsA(HInterpEdInputInterface::StaticGetType()))
						{
							HInterpEdInputInterface* Proxy = ((HInterpEdInputInterface*)HitResult);
							
							//@todo: Do dropping.
						}
					}

					DragData.PixelsPerSec = InterpEd->PixelsPerSec;
					DragData.MouseCurrent = FIntPoint(HitX, HitY);
					DragObject->EndDrag(DragData);
					DragObject = NULL;
				}
				else if(DistanceDragged < 4)
				{
					HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);

					// If mouse didn't really move since last time, and we released over empty space, deselect everything.
					if(!HitResult)
					{
						InterpEd->ClearKeySelection();
					}
					else if(bCtrlDown && HitResult->IsA(HInterpTrackKeypointProxy::StaticGetType()))
					{
						UInterpGroup* Group = ((HInterpTrackKeypointProxy*)HitResult)->Group;
						const INT TrackIndex = ((HInterpTrackKeypointProxy*)HitResult)->TrackIndex;
						const INT KeyIndex = ((HInterpTrackKeypointProxy*)HitResult)->KeyIndex;

						const UBOOL bAlreadySelected = InterpEd->KeyIsInSelection(Group, TrackIndex, KeyIndex);
						if(bAlreadySelected)
						{
							InterpEd->RemoveKeyFromSelection(Group, TrackIndex, KeyIndex);
						}
						else
						{
							InterpEd->AddKeyToSelection(Group, TrackIndex, KeyIndex, !bShiftDown);
							if ( bShiftDown )
							{
								InterpEd->SetActiveTrack( Group, TrackIndex );
							}
						}
					}
				}

				if(bTransactionBegun)
				{
					InterpEd->EndMoveSelectedKeys();
					bTransactionBegun = false;
				}

				if(bGrabbingMarker)
				{
					InterpEd->EndMoveMarker();
					bGrabbingMarker = false;
				}

				Viewport->LockMouseToWindow(false);

				DistanceDragged = 0;

				bPanning = false;
				bMouseDown = false;
				bGrabbingHandle = false;
				bNavigating = false;
				bBoxSelecting = false;
			}
			break;
		}
	}
	else if( Key == KEY_RightMouseButton )
	{
		switch( Event )
		{
		case IE_Pressed:
			{
				const INT HitX = Viewport->GetMouseX();
				const INT HitY = Viewport->GetMouseY();
				HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);

				if(HitResult)
				{
					// User right-click somewhere in the track editor
					wxMenu* Menu = InterpEd->CreateContextMenu( Viewport, HitResult );
					if(Menu)
					{
						FTrackPopupMenu tpm( InterpEd, Menu );
						tpm.Show();
						delete Menu;
					}
				}
			}
			break;

		case IE_Released:
			{
				
			}
			break;
		}
	}
	
	if(Event == IE_Pressed)
	{
		if(Key == KEY_MouseScrollDown)
		{
			InterpEd->ZoomView( InterpEditor_ZoomIncrement, InterpEd->bZoomToScrubPos );
		}
		else if(Key == KEY_MouseScrollUp)
		{
			InterpEd->ZoomView( 1.0f / InterpEditor_ZoomIncrement, InterpEd->bZoomToScrubPos );
		}

		// Handle hotkey bindings.
		UUnrealEdOptions* UnrealEdOptions = GUnrealEd->GetUnrealEdOptions();

		if(UnrealEdOptions)
		{
			FString Cmd = UnrealEdOptions->GetExecCommand(Key, bAltDown, bCtrlDown, bShiftDown, TEXT("Matinee"));

			if(Cmd.Len())
			{
				Exec(*Cmd);
			}
		}
	}

	// Handle viewport screenshot.
	InputTakeScreenshot( Viewport, Key, Event );

	return TRUE;
}

// X and Y here are the new screen position of the cursor.
void FInterpEdViewportClient::MouseMove(FViewport* Viewport, INT X, INT Y)
{
	UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);

	INT DeltaX = OldMouseX - X;
	INT DeltaY = OldMouseY - Y;

	if(bMouseDown)
	{
		DistanceDragged += ( Abs<INT>(DeltaX) + Abs<INT>(DeltaY) );
	}

	OldMouseX = X;
	OldMouseY = Y;


	if(bMouseDown)
	{
		if(DragObject != NULL)
		{
			DragData.PixelsPerSec = InterpEd->PixelsPerSec;
			DragData.MouseCurrent = FIntPoint(X, Y);
			DragObject->ObjectDragged(DragData);
		}
		else if(bGrabbingHandle)
		{
			FLOAT NewTime = InterpEd->ViewStartTime + ((X - InterpEd->LabelWidth) / InterpEd->PixelsPerSec);
			if( InterpEd->bSnapToFrames && InterpEd->bSnapTimeToFrames )
			{
				NewTime = InterpEd->SnapTimeToNearestFrame( NewTime );
			}

			InterpEd->SetInterpPosition(NewTime);
		}
		else if(bBoxSelecting)
		{
			BoxEndX = X;
			BoxEndY = Y;
		}
		else if( bCtrlDown && InterpEd->Opt->SelectedKeys.Num() > 0 )
		{
			if(DistanceDragged > 4)
			{
				if(!bTransactionBegun)
				{
					InterpEd->BeginMoveSelectedKeys();
					bTransactionBegun = true;
				}

				FLOAT DeltaTime = -DeltaX / InterpEd->PixelsPerSec;
				InterpEd->MoveSelectedKeys(DeltaTime);
			}
		}
		else if(bNavigating)
		{
			FLOAT DeltaTime = -DeltaX / InterpEd->NavPixelsPerSecond;
			InterpEd->ViewStartTime += DeltaTime;
			InterpEd->ViewEndTime += DeltaTime;
			InterpEd->SyncCurveEdView();
		}
		else if(bGrabbingMarker)
		{
			FLOAT DeltaTime = -DeltaX / InterpEd->PixelsPerSec;
			InterpEd->UnsnappedMarkerPos += DeltaTime;

			if(InterpEd->GrabbedMarkerType == ISM_SeqEnd)
			{
				InterpEd->SetInterpEnd( InterpEd->SnapTime(InterpEd->UnsnappedMarkerPos, false) );
			}
			else if(InterpEd->GrabbedMarkerType == ISM_LoopStart || InterpEd->GrabbedMarkerType == ISM_LoopEnd)
			{
				InterpEd->MoveLoopMarker( InterpEd->SnapTime(InterpEd->UnsnappedMarkerPos, false), InterpEd->GrabbedMarkerType == ISM_LoopStart );
			}
		}
		else if(bPanning)
		{
			FLOAT DeltaTime = -DeltaX / InterpEd->PixelsPerSec;
			InterpEd->ViewStartTime -= DeltaTime;
			InterpEd->ViewEndTime -= DeltaTime;
			InterpEd->SyncCurveEdView();
		}
	}
}

UBOOL FInterpEdViewportClient::InputAxis(FViewport* Viewport, INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad)
{
	if ( Key == KEY_MouseX || Key == KEY_MouseY )
	{
		INT X = Viewport->GetMouseX();
		INT Y = Viewport->GetMouseY();
		MouseMove(Viewport, X, Y);
		return TRUE;
	}
	return FALSE;
}

EMouseCursor FInterpEdViewportClient::GetCursor(FViewport* Viewport,INT X,INT Y)
{
	EMouseCursor Result = MC_Cross;

	if(DragObject==NULL)
	{
		HHitProxy*	HitProxy = Viewport->GetHitProxy(X,Y);
		

		if(HitProxy)
		{
			Result = HitProxy->GetMouseCursor();
		}
	}
	else
	{
		Result = MC_NoChange;
	}

	return Result;
}

void FInterpEdViewportClient::Tick(FLOAT DeltaSeconds)
{
	// Only the main track window is allowed to tick the root object.  We never want the InterpEd object to be
	// ticked more than once per frame.
	if( !bIsDirectorTrackWindow )
	{
		InterpEd->TickInterp(DeltaSeconds);
	}

	// If curve editor is shown - sync us with it.
	if(InterpEd->CurveEd->IsShown())
	{
		InterpEd->ViewStartTime = InterpEd->CurveEd->StartIn;
		InterpEd->ViewEndTime = InterpEd->CurveEd->EndIn;
	}

	if(bNavigating || bPanning)
	{
		const INT ScrollBorderSize = 20;
		const FLOAT	ScrollBorderSpeed = 500.f;
		const INT PosX = Viewport->GetMouseX();
		const INT PosY = Viewport->GetMouseY();
		const INT SizeX = Viewport->GetSizeX();
		const INT SizeY = Viewport->GetSizeY();

		FLOAT DeltaTime = Clamp(DeltaSeconds, 0.01f, 1.0f);

		if(PosX < ScrollBorderSize)
		{
			ScrollAccum.X += (1.f - ((FLOAT)PosX/(FLOAT)ScrollBorderSize)) * ScrollBorderSpeed * DeltaTime;
		}
		else if(PosX > SizeX - ScrollBorderSize)
		{
			ScrollAccum.X -= ((FLOAT)(PosX - (SizeX - ScrollBorderSize))/(FLOAT)ScrollBorderSize) * ScrollBorderSpeed * DeltaTime;
		}
		else
		{
			ScrollAccum.X = 0.f;
		}

		// Apply integer part of ScrollAccum to the curve editor view position.
		const INT DeltaX = appFloor(ScrollAccum.X);
		ScrollAccum.X -= DeltaX;

		if(bNavigating)
		{
			DeltaTime = -DeltaX / InterpEd->NavPixelsPerSecond;
			InterpEd->ViewStartTime += DeltaTime;
			InterpEd->ViewEndTime += DeltaTime;

			InterpEd->SyncCurveEdView();
		}
		else
		{
			DeltaTime = -DeltaX / InterpEd->PixelsPerSec;
			InterpEd->ViewStartTime -= DeltaTime;
			InterpEd->ViewEndTime -= DeltaTime;
			InterpEd->SyncCurveEdView();
		}
	}

	Viewport->Invalidate();
}


void FInterpEdViewportClient::Serialize(FArchive& Ar) 
{ 
	Ar << Input; 

	// Drag object may be a instance of UObject, so serialize it if it is.
	if(DragObject && DragObject->GetUObject())
	{
		UObject* DragUObject = DragObject->GetUObject();
		Ar << DragUObject;
	}
}

/** Exec handler */
void FInterpEdViewportClient::Exec(const TCHAR* Cmd)
{
	const TCHAR* Str = Cmd;

	if(ParseCommand(&Str, TEXT("MATINEE")))
	{
		if(ParseCommand(&Str, TEXT("Undo")))
		{
			InterpEd->InterpEdUndo();
		}
		else if(ParseCommand(&Str, TEXT("Redo")))
		{
			InterpEd->InterpEdRedo();
		}
		else if(ParseCommand(&Str, TEXT("Cut")))
		{
			InterpEd->CopySelectedGroupOrTrack(TRUE);
		}
		else if(ParseCommand(&Str, TEXT("Copy")))
		{
			InterpEd->CopySelectedGroupOrTrack(FALSE);
		}
		else if(ParseCommand(&Str, TEXT("Paste")))
		{
			InterpEd->PasteSelectedGroupOrTrack();
		}
		else if(ParseCommand(&Str, TEXT("Play")))
		{
			InterpEd->StartPlaying( FALSE, TRUE );
		}
		else if(ParseCommand(&Str, TEXT("PlayReverse")))
		{
			InterpEd->StartPlaying( FALSE, FALSE );
		}
		else if(ParseCommand(&Str, TEXT("Stop")))
		{
			if(InterpEd->Interp->bIsPlaying)
			{
				InterpEd->StopPlaying();
			}
		}
		else if(ParseCommand(&Str, TEXT("Rewind")))
		{
			InterpEd->SetInterpPosition(0.f);
		}
		else if(ParseCommand(&Str, TEXT("TogglePlayPause")))
		{
			if(InterpEd->Interp->bIsPlaying)
			{
				InterpEd->StopPlaying();
			}
			else
			{
				// Start playback and retain whatever direction we were already playing
				InterpEd->StartPlaying( FALSE, TRUE );
			}
		}
		else if( ParseCommand( &Str, TEXT( "ZoomIn" ) ) )
		{
			const UBOOL bZoomToTimeCursorPos = TRUE;
			InterpEd->ZoomView( 1.0f / InterpEditor_ZoomIncrement, bZoomToTimeCursorPos );
		}
		else if( ParseCommand( &Str, TEXT( "ZoomOut" ) ) )
		{
			const UBOOL bZoomToTimeCursorPos = TRUE;
			InterpEd->ZoomView( InterpEditor_ZoomIncrement, bZoomToTimeCursorPos );
		}
		else if(ParseCommand(&Str, TEXT("DeleteSelection")))
		{
			InterpEd->DeleteSelectedKeys(TRUE);
		}
		else if(ParseCommand(&Str, TEXT("MarkInSection")))
		{
			InterpEd->MoveLoopMarker(InterpEd->Interp->Position, TRUE);
		}
		else if(ParseCommand(&Str, TEXT("MarkOutSection")))
		{
			InterpEd->MoveLoopMarker(InterpEd->Interp->Position, FALSE);
		}
		else if(ParseCommand(&Str, TEXT("CropAnimationBeginning")))
		{
			InterpEd->CropAnimKey(TRUE);
		}
		else if(ParseCommand(&Str, TEXT("CropAnimationEnd")))
		{
			InterpEd->CropAnimKey(FALSE);
		}
		else if(ParseCommand(&Str, TEXT("IncrementPosition")))
		{
			InterpEd->IncrementSelection();
		}
		else if(ParseCommand(&Str, TEXT("DecrementPosition")))
		{
			InterpEd->DecrementSelection();
		}
		else if(ParseCommand(&Str, TEXT("MoveToNextKey")))
		{
			InterpEd->SelectNextKey();
		}
		else if(ParseCommand(&Str, TEXT("MoveToPrevKey")))
		{
			InterpEd->SelectPreviousKey();
		}
		else if(ParseCommand(&Str, TEXT("SplitAnimKey")))
		{
			InterpEd->SplitAnimKey();
		}
		else if(ParseCommand(&Str, TEXT("ToggleSnap")))
		{
			InterpEd->SetSnapEnabled(!InterpEd->bSnapEnabled);
		}
		else if(ParseCommand(&Str, TEXT("ToggleSnapTimeToFrames")))
		{
			InterpEd->SetSnapTimeToFrames(!InterpEd->bSnapTimeToFrames);
		}
		else if(ParseCommand(&Str, TEXT("ToggleFixedTimeStepPlayback")))
		{
			InterpEd->SetFixedTimeStepPlayback( !InterpEd->bFixedTimeStepPlayback );
		}
		else if(ParseCommand(&Str, TEXT("TogglePreferFrameNumbers")))
		{
			InterpEd->SetPreferFrameNumbers( !InterpEd->bPreferFrameNumbers );
		}
		else if(ParseCommand(&Str, TEXT("ToggleShowTimeCursorPosForAllKeys")))
		{
			InterpEd->SetShowTimeCursorPosForAllKeys( !InterpEd->bShowTimeCursorPosForAllKeys );
		}
		else if(ParseCommand(&Str, TEXT("MoveActiveUp")))
		{
			InterpEd->MoveActiveUp();
		}
		else if(ParseCommand(&Str, TEXT("MoveActiveDown")))
		{
			InterpEd->MoveActiveDown();
		}
		else if(ParseCommand(&Str, TEXT("AddKey")))
		{
			InterpEd->AddKey();
		}
		else if(ParseCommand(&Str, TEXT("DuplicateSelectedKeys")) )
		{
			InterpEd->DuplicateSelectedKeys();
		}
		else if(ParseCommand(&Str, TEXT("ViewFitSequence")) )
		{
			InterpEd->ViewFitSequence();
		}
		else if(ParseCommand(&Str, TEXT("ViewFitToSelected")) )
		{
			InterpEd->ViewFitToSelected();
		}
		else if(ParseCommand(&Str, TEXT("ViewFitLoop")) )
		{
			InterpEd->ViewFitLoop();
		}
		else if(ParseCommand(&Str, TEXT("ViewFitLoopSequence")) )
		{
			InterpEd->ViewFitLoopSequence();
		}
		else if(ParseCommand(&Str, TEXT("ChangeKeyInterpModeAUTO")) )
		{
			InterpEd->ChangeKeyInterpMode(CIM_CurveAuto);
		}
		else if(ParseCommand(&Str, TEXT("ChangeKeyInterpModeAUTOCLAMPED")) )
		{
			InterpEd->ChangeKeyInterpMode(CIM_CurveAutoClamped);
		}
		else if(ParseCommand(&Str, TEXT("ChangeKeyInterpModeUSER")) )
		{
			InterpEd->ChangeKeyInterpMode(CIM_CurveUser);
		}
		else if(ParseCommand(&Str, TEXT("ChangeKeyInterpModeBREAK")) )
		{
			InterpEd->ChangeKeyInterpMode(CIM_CurveBreak);
		}
		else if(ParseCommand(&Str, TEXT("ChangeKeyInterpModeLINEAR")) )
		{
			InterpEd->ChangeKeyInterpMode(CIM_Linear);
		}
		else if(ParseCommand(&Str, TEXT("ChangeKeyInterpModeCONSTANT")) )
		{
			InterpEd->ChangeKeyInterpMode(CIM_Constant);
		}
	}
}

/*-----------------------------------------------------------------------------
 WxInterpEdVCHolder
 -----------------------------------------------------------------------------*/


BEGIN_EVENT_TABLE( WxInterpEdVCHolder, wxWindow )
	EVT_SIZE( WxInterpEdVCHolder::OnSize )
END_EVENT_TABLE()

WxInterpEdVCHolder::WxInterpEdVCHolder( wxWindow* InParent, wxWindowID InID, WxInterpEd* InInterpEd )
: wxWindow( InParent, InID )
{
	SetMinSize(wxSize(2, 2));

	// Create renderer viewport.
	InterpEdVC = new FInterpEdViewportClient( InInterpEd );
	InterpEdVC->Viewport = GEngine->Client->CreateWindowChildViewport(InterpEdVC, (HWND)GetHandle());
	InterpEdVC->Viewport->CaptureJoystickInput(false);

	// Create the vertical scroll bar.  We want this on the LEFT side, so the tracks line up in Matinee
	ScrollBar_Vert = new wxScrollBar(this, IDM_INTERP_VERT_SCROLL_BAR, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);

	// Setup the initial metrics for the scroll bar
	AdjustScrollBar();
}

WxInterpEdVCHolder::~WxInterpEdVCHolder()
{
	DestroyViewport();
}

/**
 * Destroys the viewport held by this viewport holder, disassociating it from the engine, etc.  Rentrant.
 */
void WxInterpEdVCHolder::DestroyViewport()
{
	if ( InterpEdVC )
	{
		GEngine->Client->CloseViewport(InterpEdVC->Viewport);
		InterpEdVC->Viewport = NULL;
		delete InterpEdVC;
		InterpEdVC = NULL;
	}
}



/**
 * Updates the scroll bar for the current state of the window's size and content layout.  This should be called
 *  when either the window size changes or the vertical size of the content contained in the window changes.
 */
void WxInterpEdVCHolder::AdjustScrollBar()
{
	if( InterpEdVC != NULL && ScrollBar_Vert != NULL )
	{
		// Grab the height of the client window
		wxRect rc = GetClientRect();
		const UINT ViewportHeight = rc.GetHeight();

		// Compute scroll bar layout metrics
		UINT ContentHeight = InterpEdVC->ComputeGroupListContentHeight();
		UINT ContentBoxHeight =	InterpEdVC->ComputeGroupListBoxHeight( ViewportHeight );


		// The current scroll bar position
		const INT ScrollBarPos = -InterpEdVC->ThumbPos_Vert;

		// The thumb size is the number of 'scrollbar units' currently visible
		const UINT ScrollBarThumbSize = ContentBoxHeight;

		// The size of a 'scrollbar page'.  This is how much to scroll when paging up and down.
		const UINT ScrollBarPageSize = ScrollBarThumbSize;


		// Configure the scroll bar's position and size
		wxRect rcSBV = ScrollBar_Vert->GetClientRect();
		ScrollBar_Vert->SetSize( 0, 0, rcSBV.GetWidth(), rc.GetHeight() );

		// Configure the scroll bar layout metrics
		ScrollBar_Vert->SetScrollbar(
			ScrollBarPos,         // Position
			ScrollBarThumbSize,   // Thumb size
			ContentHeight,        // Range
			ScrollBarPageSize );  // Page size
	}
}


void WxInterpEdVCHolder::OnSize( wxSizeEvent& In )
{
	// Update the window's content layout
	UpdateWindowLayout();
}


/**
 * Updates layout of the track editor window's content layout.  This is usually called in response to a window size change
 */
void WxInterpEdVCHolder::UpdateWindowLayout()
{
	if( InterpEdVC != NULL )
	{
		// The track window size has changed, so update our scroll bar
		AdjustScrollBar();

		wxRect rc = GetClientRect();

		// Make sure the track window's group list is positioned to the right of the scroll bar
		if( ScrollBar_Vert != NULL )
		{
			INT ScrollBarWidth = ScrollBar_Vert->GetClientRect().GetWidth();
			rc.x += ScrollBarWidth;
			rc.width -= ScrollBarWidth;
		}
		::MoveWindow( (HWND)InterpEdVC->Viewport->GetWindow(), rc.x, rc.y, rc.GetWidth(), rc.GetHeight(), 1 );
	}
}



/*-----------------------------------------------------------------------------
 WxInterpEd
 -----------------------------------------------------------------------------*/


UBOOL				WxInterpEd::bInterpTrackClassesInitialized = false;
TArray<UClass*>		WxInterpEd::InterpTrackClasses;

// On init, find all track classes. Will use later on to generate menus.
void WxInterpEd::InitInterpTrackClasses()
{
	if(bInterpTrackClassesInitialized)
		return;

	// Construct list of non-abstract gameplay sequence object classes.
	for(TObjectIterator<UClass> It; It; ++It)
	{
		if( It->IsChildOf(UInterpTrack::StaticClass()) && !(It->ClassFlags & CLASS_Abstract) )
		{
			InterpTrackClasses.AddItem(*It);
		}
	}

	bInterpTrackClassesInitialized = true;
}

BEGIN_EVENT_TABLE( WxInterpEd, WxTrackableFrame )
	EVT_SIZE( WxInterpEd::OnSize )
	EVT_CLOSE( WxInterpEd::OnClose )
	EVT_MENU( IDM_INTERP_FILE_EXPORT, WxInterpEd::OnMenuExport )
	EVT_MENU( IDM_INTERP_FILE_IMPORT, WxInterpEd::OnMenuImport )
	EVT_MENU( IDM_INTERP_ExportSoundCueInfo, WxInterpEd::OnExportSoundCueInfoCommand )
	EVT_MENU_RANGE( IDM_INTERP_NEW_TRACK_START, IDM_INTERP_NEW_TRACK_END, WxInterpEd::OnContextNewTrack )
	EVT_MENU_RANGE( IDM_INTERP_KEYMODE_LINEAR, IDM_INTERP_KEYMODE_CONSTANT, WxInterpEd::OnContextKeyInterpMode )
	EVT_MENU( IDM_INTERP_EVENTKEY_RENAME, WxInterpEd::OnContextRenameEventKey )
	EVT_MENU( IDM_INTERP_KEY_SETTIME, WxInterpEd::OnContextSetKeyTime )
	EVT_MENU( IDM_INTERP_KEY_SETVALUE, WxInterpEd::OnContextSetValue )
	EVT_MENU( IDM_INTERP_KEY_SETCOLOR, WxInterpEd::OnContextSetColor )
	EVT_MENU( IDM_INTERP_ANIMKEY_LOOP, WxInterpEd::OnSetAnimKeyLooping )
	EVT_MENU( IDM_INTERP_ANIMKEY_NOLOOP, WxInterpEd::OnSetAnimKeyLooping )
	EVT_MENU( IDM_INTERP_ANIMKEY_SETSTARTOFFSET, WxInterpEd::OnSetAnimOffset )
	EVT_MENU( IDM_INTERP_ANIMKEY_SETENDOFFSET, WxInterpEd::OnSetAnimOffset )
	EVT_MENU( IDM_INTERP_ANIMKEY_SETPLAYRATE, WxInterpEd::OnSetAnimPlayRate )
	EVT_MENU( IDM_INTERP_ANIMKEY_TOGGLEREVERSE, WxInterpEd::OnToggleReverseAnim )
	EVT_UPDATE_UI( IDM_INTERP_ANIMKEY_TOGGLEREVERSE, WxInterpEd::OnToggleReverseAnim_UpdateUI )

	EVT_MENU( IDM_INTERP_CAMERA_ANIM_EXPORT, WxInterpEd::OnContextSaveAsCameraAnimation )

	EVT_MENU( IDM_INTERP_SoundKey_SetVolume, WxInterpEd::OnSetSoundVolume )
	EVT_MENU( IDM_INTERP_SoundKey_SetPitch, WxInterpEd::OnSetSoundPitch )
	EVT_MENU( IDM_INTERP_KeyContext_SyncGenericBrowserToSoundCue, WxInterpEd::OnKeyContext_SyncGenericBrowserToSoundCue )
	EVT_MENU( IDM_INTERP_KeyContext_SetMasterVolume, WxInterpEd::OnKeyContext_SetMasterVolume )
	EVT_MENU( IDM_INTERP_KeyContext_SetMasterPitch, WxInterpEd::OnKeyContext_SetMasterPitch )
	EVT_MENU( IDM_INTERP_ParticleReplayKeyContext_SetClipIDNumber, WxInterpEd::OnParticleReplayKeyContext_SetClipIDNumber )
	EVT_MENU( IDM_INTERP_ParticleReplayKeyContext_SetDuration, WxInterpEd::OnParticleReplayKeyContext_SetDuration )
	EVT_MENU( IDM_INTERP_DIRKEY_SETTRANSITIONTIME, WxInterpEd::OnContextDirKeyTransitionTime )
	EVT_MENU( IDM_INTERP_TOGGLEKEY_FLIP, WxInterpEd::OnFlipToggleKey )
	EVT_MENU( IDM_INTERP_KeyContext_SetCondition_Always, WxInterpEd::OnKeyContext_SetCondition )
	EVT_MENU( IDM_INTERP_KeyContext_SetCondition_GoreEnabled, WxInterpEd::OnKeyContext_SetCondition )
	EVT_MENU( IDM_INTERP_KeyContext_SetCondition_GoreDisabled, WxInterpEd::OnKeyContext_SetCondition )

	EVT_MENU( IDM_INTERP_DeleteSelectedKeys, WxInterpEd::OnDeleteSelectedKeys )
	EVT_MENU( IDM_INTERP_GROUP_RENAME, WxInterpEd::OnContextGroupRename )
	EVT_MENU( IDM_INTERP_GROUP_DUPLICATE, WxInterpEd::OnContextNewGroup )
	EVT_MENU( IDM_INTERP_GROUP_DELETE, WxInterpEd::OnContextGroupDelete )
	EVT_MENU( IDM_INTERP_GROUP_CREATETAB, WxInterpEd::OnContextGroupCreateTab )
	EVT_MENU( IDM_INTERP_GROUP_DELETETAB, WxInterpEd::OnContextDeleteGroupTab )
	EVT_MENU( IDM_INTERP_GROUP_REMOVEFROMTAB, WxInterpEd::OnContextGroupRemoveFromTab )
	EVT_MENU_RANGE( IDM_INTERP_GROUP_SENDTOTAB_START, IDM_INTERP_GROUP_SENDTOTAB_END, WxInterpEd::OnContextGroupSendToTab )
	EVT_MENU_RANGE( IDM_INTERP_GROUP_MoveActiveGroupToFolder_Start, IDM_INTERP_GROUP_MoveActiveGroupToFolder_End, WxInterpEd::OnContextGroupChangeGroupFolder )
	EVT_MENU_RANGE( IDM_INTERP_GROUP_MoveGroupToActiveFolder_Start, IDM_INTERP_GROUP_MoveGroupToActiveFolder_End, WxInterpEd::OnContextGroupChangeGroupFolder )
	EVT_MENU( IDM_INTERP_GROUP_RemoveFromGroupFolder, WxInterpEd::OnContextGroupChangeGroupFolder )
	
	EVT_MENU( IDM_INTERP_TRACK_RENAME, WxInterpEd::OnContextTrackRename )
	EVT_MENU( IDM_INTERP_TRACK_DELETE, WxInterpEd::OnContextTrackDelete )
	EVT_MENU( IDM_INTERP_TOGGLE_CURVEEDITOR, WxInterpEd::OnToggleCurveEd )

	EVT_MENU_RANGE( IDM_INTERP_TRACK_FRAME_WORLD, IDM_INTERP_TRACK_FRAME_RELINITIAL, WxInterpEd::OnContextTrackChangeFrame )
	EVT_MENU( IDM_INTERP_TRACK_Show3DTrajectory, WxInterpEd::OnContextTrackShow3DTrajectory )
	EVT_MENU( IDM_INTERP_VIEW_ShowAll3DTrajectories, WxInterpEd::OnViewShowOrHideAll3DTrajectories )
	EVT_MENU( IDM_INTERP_VIEW_HideAll3DTrajectories, WxInterpEd::OnViewShowOrHideAll3DTrajectories )
	EVT_MENU( IDM_INTERP_ParticleReplayTrackContext_StartRecording, WxInterpEd::OnParticleReplayTrackContext_ToggleCapture )
	EVT_MENU( IDM_INTERP_ParticleReplayTrackContext_StopRecording, WxInterpEd::OnParticleReplayTrackContext_ToggleCapture )
	EVT_MENU( IDM_INTERP_NEW_FOLDER, WxInterpEd::OnContextNewGroup )
	EVT_MENU( IDM_INTERP_NEW_EMPTY_GROUP, WxInterpEd::OnContextNewGroup )
	EVT_MENU( IDM_INTERP_NEW_CAMERA_GROUP, WxInterpEd::OnContextNewGroup )
	EVT_MENU( IDM_INTERP_NEW_PARTICLE_GROUP, WxInterpEd::OnContextNewGroup )
	EVT_MENU( IDM_INTERP_NEW_SKELETAL_MESH_GROUP, WxInterpEd::OnContextNewGroup )
	EVT_MENU( IDM_INTERP_NEW_DIRECTOR_GROUP, WxInterpEd::OnContextNewGroup )
	EVT_MENU( IDM_INTERP_ADDKEY, WxInterpEd::OnMenuAddKey )
	EVT_COMBOBOX( IDM_INTERP_InitialInterpMode_ComboBox, WxInterpEd::OnChangeInitialInterpMode )
	EVT_MENU( IDM_INTERP_PLAY, WxInterpEd::OnMenuPlay )
	EVT_MENU( IDM_INTERP_PlayReverse, WxInterpEd::OnMenuPlay )
	EVT_MENU( IDM_INTERP_PLAYLOOPSECTION, WxInterpEd::OnMenuPlay )
	EVT_MENU( IDM_INTERP_STOP, WxInterpEd::OnMenuStop )
	EVT_MENU_RANGE( IDM_INTERP_SPEED_1, IDM_INTERP_SPEED_100, WxInterpEd::OnChangePlaySpeed )
	EVT_MENU( IDM_INTERP_ToggleGorePreview, WxInterpEd::OnToggleGorePreview )
	EVT_UPDATE_UI( IDM_INTERP_ToggleGorePreview, WxInterpEd::OnToggleGorePreview_UpdateUI )
	
	EVT_MENU( IDM_OPEN_BINDKEYS_DIALOG, WxInterpEd::OnOpenBindKeysDialog )

	EVT_MENU( IDM_INTERP_EDIT_UNDO, WxInterpEd::OnMenuUndo )
	EVT_MENU( IDM_INTERP_EDIT_REDO, WxInterpEd::OnMenuRedo )
	EVT_MENU( IDM_INTERP_EDIT_CUT, WxInterpEd::OnMenuCut )
	EVT_MENU( IDM_INTERP_EDIT_COPY, WxInterpEd::OnMenuCopy )
	EVT_MENU( IDM_INTERP_EDIT_PASTE, WxInterpEd::OnMenuPaste )

	EVT_UPDATE_UI ( IDM_INTERP_EDIT_UNDO, WxInterpEd::OnMenuEdit_UpdateUI )
	EVT_UPDATE_UI ( IDM_INTERP_EDIT_REDO, WxInterpEd::OnMenuEdit_UpdateUI )
	EVT_UPDATE_UI ( IDM_INTERP_EDIT_CUT, WxInterpEd::OnMenuEdit_UpdateUI )
	EVT_UPDATE_UI ( IDM_INTERP_EDIT_COPY, WxInterpEd::OnMenuEdit_UpdateUI )
	EVT_UPDATE_UI ( IDM_INTERP_EDIT_PASTE, WxInterpEd::OnMenuEdit_UpdateUI )

	EVT_MENU( IDM_INTERP_MOVEKEY_SETLOOKUP, WxInterpEd::OnSetMoveKeyLookupGroup )
	EVT_MENU( IDM_INTERP_MOVEKEY_CLEARLOOKUP, WxInterpEd::OnClearMoveKeyLookupGroup )

	EVT_MENU( IDM_INTERP_TOGGLE_SNAP, WxInterpEd::OnToggleSnap )
	EVT_UPDATE_UI( IDM_INTERP_TOGGLE_SNAP, WxInterpEd::OnToggleSnap_UpdateUI )
	EVT_MENU( IDM_INTERP_TOGGLE_SNAP_TIME_TO_FRAMES, WxInterpEd::OnToggleSnapTimeToFrames )
	EVT_UPDATE_UI( IDM_INTERP_TOGGLE_SNAP_TIME_TO_FRAMES, WxInterpEd::OnToggleSnapTimeToFrames_UpdateUI )
	EVT_MENU( IDM_INTERP_FixedTimeStepPlayback, WxInterpEd::OnFixedTimeStepPlaybackCommand )
	EVT_UPDATE_UI( IDM_INTERP_FixedTimeStepPlayback, WxInterpEd::OnFixedTimeStepPlaybackCommand_UpdateUI )
	EVT_MENU( IDM_INTERP_PreferFrameNumbers, WxInterpEd::OnPreferFrameNumbersCommand )
	EVT_UPDATE_UI( IDM_INTERP_PreferFrameNumbers, WxInterpEd::OnPreferFrameNumbersCommand_UpdateUI )
	EVT_MENU( IDM_INTERP_ShowTimeCursorPosForAllKeys, WxInterpEd::OnShowTimeCursorPosForAllKeysCommand )
	EVT_UPDATE_UI( IDM_INTERP_ShowTimeCursorPosForAllKeys, WxInterpEd::OnShowTimeCursorPosForAllKeysCommand_UpdateUI )
	EVT_MENU( IDM_INTERP_VIEW_FITSEQUENCE, WxInterpEd::OnViewFitSequence )
	EVT_MENU( IDM_INTERP_VIEW_FitViewToSelected, WxInterpEd::OnViewFitToSelected )
	EVT_MENU( IDM_INTERP_VIEW_FITLOOP, WxInterpEd::OnViewFitLoop )
	EVT_MENU( IDM_INTERP_VIEW_FITLOOPSEQUENCE, WxInterpEd::OnViewFitLoopSequence )
	EVT_COMBOBOX( IDM_INTERP_SNAPCOMBO, WxInterpEd::OnChangeSnapSize )
	EVT_MENU( IDM_INTERP_EDIT_INSERTSPACE, WxInterpEd::OnMenuInsertSpace )
	EVT_MENU( IDM_INTERP_EDIT_STRETCHSECTION, WxInterpEd::OnMenuStretchSection )
	EVT_MENU( IDM_INTERP_EDIT_DELETESECTION, WxInterpEd::OnMenuDeleteSection )
	EVT_MENU( IDM_INTERP_EDIT_SELECTINSECTION, WxInterpEd::OnMenuSelectInSection )
	EVT_MENU( IDM_INTERP_EDIT_DUPLICATEKEYS, WxInterpEd::OnMenuDuplicateSelectedKeys )
	EVT_MENU( IDM_INTERP_EDIT_SAVEPATHTIME, WxInterpEd::OnSavePathTime )
	EVT_MENU( IDM_INTERP_EDIT_JUMPTOPATHTIME, WxInterpEd::OnJumpToPathTime )
	EVT_MENU( IDM_INTERP_EDIT_REDUCEKEYS, WxInterpEd::OnMenuReduceKeys )
	EVT_MENU( IDM_INTERP_VIEW_Draw3DTrajectories, WxInterpEd::OnViewHide3DTracks )
	EVT_MENU( IDM_INTERP_VIEW_ZoomToTimeCursorPosition, WxInterpEd::OnViewZoomToScrubPos )
	EVT_MENU( IDM_INTERP_VIEW_ViewportFrameStats, WxInterpEd::OnToggleViewportFrameStats )

	EVT_MENU( IDM_INTERP_ExpandAllGroups, WxInterpEd::OnExpandAllGroups )
	EVT_MENU( IDM_INTERP_CollapseAllGroups, WxInterpEd::OnCollapseAllGroups )

	EVT_SPLITTER_SASH_POS_CHANGED( IDM_INTERP_GRAPH_SPLITTER, WxInterpEd::OnGraphSplitChangePos )
	EVT_SCROLL( WxInterpEd::OnScroll )
END_EVENT_TABLE()


/** Should NOT open an InterpEd unless InInterp has a valid InterpData attached! */
WxInterpEd::WxInterpEd( wxWindow* InParent, wxWindowID InID, USeqAct_Interp* InInterp )
	:	WxTrackableFrame( InParent, InID, *LocalizeUnrealEd("Matinee"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR )
	,	FDockingParent(this)
	,	bClosed( FALSE )
	,   PropertyWindow( NULL )
	,   CurveEd( NULL )
	,	TrackWindow(NULL)
	,	DirectorTrackWindow(NULL)
	,	ActiveGroup( NULL )
	,	CurveEdToggleMenuItem( NULL )
	,	ToolBar( NULL )
	,	MenuBar( NULL )
	,	bIsInitialized( FALSE )
	,   bViewportFrameStatsEnabled( TRUE )
{
	// Make sure we have a list of available track classes
	WxInterpEd::InitInterpTrackClasses();


	// NOTE: This should match the curve editor's label width!
	LabelWidth = 200;

	// 3D tracks should be visible by default
	bHide3DTrackView = FALSE;
	GConfig->GetBool( TEXT("Matinee"), TEXT("Hide3DTracks"), bHide3DTrackView, GEditorUserSettingsIni );

	// Zoom to scrub position defaults to off.  We want zoom to cursor position by default.
	bZoomToScrubPos = FALSE;
	GConfig->GetBool( TEXT("Matinee"), TEXT("ZoomToScrubPos"), bZoomToScrubPos, GEditorUserSettingsIni );

	// Setup 'viewport frame stats' preference
	bViewportFrameStatsEnabled = TRUE;
	GConfig->GetBool( TEXT("Matinee"), TEXT("ViewportFrameStats"), bViewportFrameStatsEnabled, GEditorUserSettingsIni );

	// We want to update the Attached array of each Actor, so that we can move attached things then their base moves.
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if(Actor && !Actor->bDeleteMe)
		{
			Actor->EditorUpdateBase();
		}
	}


	// Create options object.
	Opt = ConstructObject<UInterpEdOptions>( UInterpEdOptions::StaticClass(), INVALID_OBJECT, NAME_None, RF_Transactional );
	check(Opt);

	// Swap out regular UTransactor for our special one
	GEditor->ResetTransaction( *LocalizeUnrealEd("OpenMatinee") );

	NormalTransactor = GEditor->Trans;
	InterpEdTrans = new UInterpEdTransBuffer( 8*1024*1024 );
	GEditor->Trans = InterpEdTrans;

	// Set up pointers to interp objects
	Interp = InInterp;

	// Do all group/track instancing and variable hook-up.
	Interp->InitInterp();

	// Flag this action as 'being edited'
	Interp->bIsBeingEdited = TRUE;


	// Always start out with gore preview turned on in the editor!
	Interp->bShouldShowGore = TRUE;


	// Should always find some data.
	check(Interp->InterpData);
	IData = Interp->InterpData;


	// Repair any folder/group hierarchy problems in the data set
	RepairHierarchyProblems();


	PixelsPerSec = 1.f;
	TrackViewSizeX = 0;
	NavPixelsPerSecond = 1.0f;

	// Set initial zoom range
	ViewStartTime = 0.f;
	ViewEndTime = IData->InterpLength;

	bDrawSnappingLine = false;
	SnappingLinePosition = 0.f;
	UnsnappedMarkerPos = 0.f;


	// Set the default filter for the data
	if(IData->DefaultFilters.Num())
	{
		SetSelectedFilter(IData->DefaultFilters(0));
	}
	else
	{
		SetSelectedFilter(NULL);
	}

	// Slight hack to ensure interpolation data is transactional.
	Interp->SetFlags( RF_Transactional );
	IData->SetFlags( RF_Transactional );
	for(INT i=0; i<IData->InterpGroups.Num(); i++)
	{
		UInterpGroup* Group = IData->InterpGroups(i);
		Group->SetFlags( RF_Transactional );

		for(INT j=0; j<Group->InterpTracks.Num(); j++)
		{
			Group->InterpTracks(j)->SetFlags( RF_Transactional );
		}
	}


	FString ContentWarnings;

	// For each track let it save the state of the object its going to work on before being changed at all by Matinee.
	for(INT i=0; i<Interp->GroupInst.Num(); i++)
	{
		UInterpGroupInst* GrInst = Interp->GroupInst(i);
		GrInst->SaveGroupActorState();

		AActor* GroupActor = GrInst->GetGroupActor();
		if ( GroupActor )
		{
			// Save this actor's transformations if we need to (along with its children)
			Interp->ConditionallySaveActorState( GrInst, GroupActor );

			// Check for bStatic actors that have dynamic tracks associated with them and report a warning to the user
			if ( GroupActor->bStatic )
			{
				UBOOL bHasTrack = FALSE;
				FString TrackNames;

				for(INT TrackIdx=0; TrackIdx<GrInst->Group->InterpTracks.Num(); TrackIdx++)
				{
					if( GrInst->Group->InterpTracks(TrackIdx)->AllowStaticActors()==FALSE )
					{
						bHasTrack = TRUE;

						if( TrackNames.Len() > 0 )
						{
							TrackNames += ", ";
						}
						TrackNames += GrInst->Group->InterpTracks(TrackIdx)->GetClass()->GetDescription();
					}
				}

				if(bHasTrack)
				{
					// Warn if any groups with dynamic tracks are trying to act on bStatic actors!

					// Add to list of warnings of this type
					FString WarningString = FString::Printf( LocalizeSecure( LocalizeUnrealEd( "GroupOnStaticActor_F" ),
						*TrackNames,
						*GrInst->Group->GroupName.ToString(), 
						*GroupActor->GetName() ) );

					ContentWarnings += WarningString;
				}
			}


			// Also, we'll check for actors configured to use a non-interpolating physics mode (such as PHYS_None.)  If
			// the actor's group has tracks, then PHYS_None is going to prevent the object's properties from being
			// animated in game, although it may appear to animate in UnrealEd.  We'll report a warning to the user
			// about these actors.
			if ( GroupActor->Physics != PHYS_Interpolating )
			{
				UBOOL bHasPhysicsRelatedTrack = FALSE;
				FString PhysicsRelatedTrackNames;

				for( INT TrackIdx = 0; TrackIdx < GrInst->Group->InterpTracks.Num(); ++TrackIdx )
				{
					const UInterpTrack* CurTrack = GrInst->Group->InterpTracks( TrackIdx );

					// Is this is a 'movement' track?  If so, then we'll consider it worthy of our test
					if( CurTrack->IsA( UInterpTrackMove::StaticClass() ) )
					{
						bHasPhysicsRelatedTrack = TRUE;

						if( PhysicsRelatedTrackNames.Len() > 0 )
						{
							PhysicsRelatedTrackNames += ", ";
						}
						PhysicsRelatedTrackNames += CurTrack->GetClass()->GetDescription();
					}
				}

				// If we have at least one track, then we can assume that Matinee will be manipulating this object,
				// and that the physics mode should probably be set to Phys_Interpolating.
				if( bHasPhysicsRelatedTrack )
				{
					// Tracks are bound to an actor that is not configured to use interpolation physics!

					// Add to list of warnings of this type
					FString WarningString = FString::Printf( LocalizeSecure( LocalizeUnrealEd( "GroupOnNonInterpActor_F" ),
						*PhysicsRelatedTrackNames,
						*GrInst->Group->GroupName.ToString(), 
						*GroupActor->GetName() ) );

					ContentWarnings += WarningString;
				}
			}


			// Check for toggle tracks bound to non-toggleable light sources
			ALight* LightActor = Cast< ALight>( GroupActor );
			if( LightActor != NULL && !LightActor->IsToggleable() )
			{
				UBOOL bHasTrack = FALSE;
				FString TrackNames;

				for( INT TrackIdx = 0; TrackIdx < GrInst->Group->InterpTracks.Num(); ++TrackIdx )
				{
					if( GrInst->Group->InterpTracks( TrackIdx )->IsA( UInterpTrackToggle::StaticClass() ) )
					{
						bHasTrack = TRUE;

						if( TrackNames.Len() > 0 )
						{
							TrackNames += ", ";
						}
						TrackNames += GrInst->Group->InterpTracks( TrackIdx )->GetClass()->GetDescription();
					}
				}

				if( bHasTrack )
				{
					// Warn if any groups with toggle tracks are trying to act on non-toggleable light sources!

					// Add to list of warnings of this type
					FString WarningString = FString::Printf( LocalizeSecure( LocalizeUnrealEd( "InterpEd_ToggleTrackOnNonToggleableLight_F" ),
						*TrackNames,
						*GrInst->Group->GroupName.ToString(), 
						*GroupActor->GetName() ) );

					ContentWarnings += WarningString;
				}
			}
		}
	}


	// Is "force start pos" enabled?  If so, check for some common problems with use of that
	if( Interp->bForceStartPos && Interp->ForceStartPosition > 0.0f )
	{
		for( INT CurGroupIndex = 0; CurGroupIndex < Interp->InterpData->InterpGroups.Num(); ++CurGroupIndex )
		{
			const UInterpGroup* CurGroup = Interp->InterpData->InterpGroups( CurGroupIndex );

			for( INT CurTrackIndex = 0; CurTrackIndex < CurGroup->InterpTracks.Num(); ++CurTrackIndex )
			{
				const UInterpTrack* CurTrack = CurGroup->InterpTracks( CurTrackIndex );


				UBOOL bNeedWarning = FALSE;


				// @todo: Abstract these checks!  Should be accessor check in UInterpTrack!

				// @todo: These checks don't involve actors or group instances, so we should move them to
				//     the Map Check phase instead of Matinee startup!


				// Toggle tracks don't play nice with bForceStartPos since they currently cannot 'fast forward',
				// except in certain cases
				const UInterpTrackToggle* ToggleTrack = ConstCast< UInterpTrackToggle >( CurTrack );
				if( ToggleTrack != NULL )
				{
					for( INT CurKeyIndex = 0; CurKeyIndex < ToggleTrack->ToggleTrack.Num(); ++CurKeyIndex )
					{
						const FToggleTrackKey& CurKey = ToggleTrack->ToggleTrack( CurKeyIndex );

						// Trigger events will be skipped entirely when jumping forward
						if( !ToggleTrack->bFireEventsWhenJumpingForwards ||
							CurKey.ToggleAction == ETTA_Trigger )
						{
							// Is this key's time within the range that we'll be skipping over due to the Force Start
							// Position being set to a later time, we'll warn the user about that!
							if( CurKey.Time < Interp->ForceStartPosition )
							{
								// One warning per track is plenty!
								bNeedWarning = TRUE;
								break;
							}
						}
					}
				}


				// Visibility tracks don't play nice with bForceStartPos since they currently cannot 'fast forward'
				const UInterpTrackVisibility* VisibilityTrack = ConstCast< UInterpTrackVisibility >( CurTrack );
				if( VisibilityTrack != NULL && !VisibilityTrack->bFireEventsWhenJumpingForwards )
				{
					for( INT CurKeyIndex = 0; CurKeyIndex < VisibilityTrack->VisibilityTrack.Num(); ++CurKeyIndex )
					{
						const FVisibilityTrackKey& CurKey = VisibilityTrack->VisibilityTrack( CurKeyIndex );

						// Is this key's time within the range that we'll be skipping over due to the Force Start
						// Position being set to a later time, we'll warn the user about that!
						if( CurKey.Time < Interp->ForceStartPosition )
						{
							// One warning per track is plenty!
							bNeedWarning = TRUE;
							break;
						}
					}
				}


				// Sound tracks don't play nice with bForceStartPos since we can't start playing from the middle
				// of an audio clip (not supported, yet)
				const UInterpTrackSound* SoundTrack = ConstCast< UInterpTrackSound >( CurTrack );
				if( SoundTrack != NULL )
				{
					for( INT CurKeyIndex = 0; CurKeyIndex < SoundTrack->Sounds.Num(); ++CurKeyIndex )
					{
						const FSoundTrackKey& CurKey = SoundTrack->Sounds( CurKeyIndex );

						// Is this key's time within the range that we'll be skipping over due to the Force Start
						// Position being set to a later time, we'll warn the user about that!
						if( CurKey.Time < Interp->ForceStartPosition )
						{
							// One warning per track is plenty!
							bNeedWarning = TRUE;
							break;
						}
					}
				}


				// Event tracks are only OK if bFireEventsWhenJumpingForwards is also set, since that will go
				// back and fire off events between 0 and the ForceStartPosition
				const UInterpTrackEvent* EventTrack = ConstCast< UInterpTrackEvent >( CurTrack );
				if( EventTrack != NULL && ( EventTrack->bFireEventsWhenJumpingForwards == FALSE ) )
				{
					for( INT CurKeyIndex = 0; CurKeyIndex < EventTrack->EventTrack.Num(); ++CurKeyIndex )
					{
						const FEventTrackKey& CurKey = EventTrack->EventTrack( CurKeyIndex );

						// Is this key's time within the range that we'll be skipping over due to the Force Start
						// Position being set to a later time, we'll warn the user about that!
						if( CurKey.Time < Interp->ForceStartPosition )
						{
							// One warning per track is plenty!
							bNeedWarning = TRUE;
							break;
						}
					}
				}


				// FaceFX tracks don't play nice with bForceStartPos since they currently cannot 'fast forward'
				const UInterpTrackFaceFX* FaceFXTrack = ConstCast< UInterpTrackFaceFX >( CurTrack );
				if( FaceFXTrack != NULL )
				{
					for( INT CurKeyIndex = 0; CurKeyIndex < FaceFXTrack->FaceFXSeqs.Num(); ++CurKeyIndex )
					{
						const FFaceFXTrackKey& CurKey = FaceFXTrack->FaceFXSeqs( CurKeyIndex );

						// Is this key's time within the range that we'll be skipping over due to the Force Start
						// Position being set to a later time, we'll warn the user about that!
						if( CurKey.StartTime < Interp->ForceStartPosition )
						{
							// One warning per track is plenty!
							bNeedWarning = TRUE;
							break;
						}
					}
				}


				if( bNeedWarning )
				{
					FString WarningString =
						FString::Printf( LocalizeSecure( LocalizeUnrealEd( "InterpEd_TrackKeyAffectedByForceStartPosition_F" ),
							*CurTrack->TrackTitle,						// Friendly track type title
							*CurGroup->GroupName.ToString(),			// Group name
							Interp->ForceStartPosition ) );				// Force start pos value

					ContentWarnings += WarningString;
				}
			}
		}
	}


	// Did we have any warning messages to display?
	if( ContentWarnings.Len() > 0 )
	{
		// Tracks are bound to an actor that is not configured to use interpolation physics!
		appMsgf( AMT_OK, LocalizeSecure( LocalizeUnrealEd( "InterpEdWarningList_F" ), *ContentWarnings ) );
	}


	// Set position to the start of the interpolation.
	// Will position objects as the first frame of the sequence.
	Interp->UpdateInterp(0.f, true);

	ActiveGroup = NULL;
	ActiveTrackIndex = INDEX_NONE;

	CamViewGroup = NULL;

	bAdjustingKeyframe = false;
	bLoopingSection = false;
	bDragging3DHandle = false;

	PlaybackSpeed = 1.0f;
	PlaybackStartRealTime = 0.0;
	NumContinuousFixedTimeStepFrames = 0;


	// Update cam frustum colours.
	UpdateCamColours();

	// Setup property window
	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create( this, this );


	// Setup track windows
	{
		GraphSplitterWnd = new wxSplitterWindow(this, IDM_INTERP_GRAPH_SPLITTER, wxDefaultPosition, wxSize(100, 100), wxSP_3DBORDER|wxSP_FULLSASH );
		GraphSplitterWnd->SetMinimumPaneSize(40);

		// Setup track windows (separated by a splitter)
		TrackWindow = new WxInterpEdVCHolder( GraphSplitterWnd, -1, this );
		DirectorTrackWindow = new WxInterpEdVCHolder( GraphSplitterWnd, -1, this );

		// Load a default splitter position for the curve editor.
		const UBOOL bSuccess = GConfig->GetInt(TEXT("Matinee"), TEXT("SplitterPos"), GraphSplitPos, GEditorUserSettingsIni);
		if(bSuccess == FALSE)
		{
			GraphSplitPos = 240;
		}

		// Start off with the window unsplit, showing only the regular track window
		GraphSplitterWnd->Initialize( TrackWindow );
		TrackWindow->Show( TRUE );
		DirectorTrackWindow->Show( FALSE );

		// Setup track window defaults
		TrackWindow->InterpEdVC->bIsDirectorTrackWindow = FALSE;
		TrackWindow->InterpEdVC->bWantFilterTabs = TRUE;
		TrackWindow->InterpEdVC->bWantTimeline = TRUE;
		DirectorTrackWindow->InterpEdVC->bIsDirectorTrackWindow = TRUE;
	}


	// Create new curve editor setup if not already done
	if(!IData->CurveEdSetup)
	{
		IData->CurveEdSetup = ConstructObject<UInterpCurveEdSetup>( UInterpCurveEdSetup::StaticClass(), IData, NAME_None, RF_NotForClient | RF_NotForServer );
	}

	// Create graph editor to work on InterpData's CurveEd setup.
	CurveEd = new WxCurveEditor( this, -1, IData->CurveEdSetup );

	// Register this window with the Curve editor so we will be notified of various things.
	CurveEd->SetNotifyObject(this);

	// Set graph view to match track view.
	SyncCurveEdView();

	PosMarkerColor = PositionMarkerLineColor;
	RegionFillColor = LoopRegionFillColor;

	CurveEd->SetEndMarker(true, IData->InterpLength);
	CurveEd->SetPositionMarker(true, 0.f, PosMarkerColor);
	CurveEd->SetRegionMarker(true, IData->EdSectionStart, IData->EdSectionEnd, RegionFillColor);


	// Setup docked windows
	const wxString CurveEdWindowTitle( *LocalizeUnrealEd( "InterpEdCurveEditor" ) );
	{
		AddDockingWindow( CurveEd, FDockingParent::DH_Top, CurveEdWindowTitle.c_str() );
		AddDockingWindow(PropertyWindow, FDockingParent::DH_Bottom, *LocalizeUnrealEd("Properties"));

		SetDockHostSize( FDockingParent::DH_Left, 500 );

		AddDockingWindow( GraphSplitterWnd, FDockingParent::DH_None, NULL );


		// Load and apply docked window layout
		LoadDockingLayout();
	}

	// Create toolbar
	ToolBar = new WxInterpEdToolBar( this, -1 );
	SetToolBar( ToolBar );

	// Initialise snap settings.
	bSnapToKeys = FALSE;
	bSnapEnabled = FALSE;
	bSnapToFrames = FALSE;	
	bSnapTimeToFrames = FALSE;
	bFixedTimeStepPlayback = FALSE;
	bPreferFrameNumbers = TRUE;
	bShowTimeCursorPosForAllKeys = FALSE;

	// Load fixed time step setting
	GConfig->GetBool( TEXT("Matinee"), TEXT("FixedTimeStepPlayback"), bFixedTimeStepPlayback, GEditorUserSettingsIni );

	// Load 'prefer frame numbers' setting
	GConfig->GetBool( TEXT("Matinee"), TEXT("PreferFrameNumbers"), bPreferFrameNumbers, GEditorUserSettingsIni );

	// Load 'show time cursor pos for all keys' setting
	GConfig->GetBool( TEXT("Matinee"), TEXT("ShowTimeCursorPosForAllKeys"), bShowTimeCursorPosForAllKeys, GEditorUserSettingsIni );

	// Restore selected snap mode from INI.
	GConfig->GetBool( TEXT("Matinee"), TEXT("SnapEnabled"), bSnapEnabled, GEditorUserSettingsIni );
	GConfig->GetBool( TEXT("Matinee"), TEXT("SnapTimeToFrames"), bSnapTimeToFrames, GEditorUserSettingsIni );
	INT SelectedSnapMode = 3; // default 0.5 sec
	GConfig->GetInt(TEXT("Matinee"), TEXT("SelectedSnapMode"), SelectedSnapMode, GEditorUserSettingsIni );
	ToolBar->SnapCombo->SetSelection( SelectedSnapMode );
	wxCommandEvent In;
	In.SetInt(SelectedSnapMode);
	OnChangeSnapSize(In);

	// Update snap button & synchronize with curve editor
	SetSnapEnabled( bSnapEnabled );
	SetSnapTimeToFrames( bSnapTimeToFrames );
	SetFixedTimeStepPlayback( bFixedTimeStepPlayback );
	SetPreferFrameNumbers( bPreferFrameNumbers );
	SetShowTimeCursorPosForAllKeys( bShowTimeCursorPosForAllKeys );


	// We always default to Curve (Auto/Clamped) when we have no other settings
	InitialInterpMode = CIM_CurveAutoClamped;

	// Restore user's "initial curve interpolation mode" setting from their preferences file
	{
		// NOTE: InitialInterpMode now has a '2' suffix after a version bump to change the default
		INT DesiredInitialInterpMode = ( INT )InitialInterpMode;
		GConfig->GetInt( TEXT( "Matinee" ), TEXT( "InitialInterpMode2" ), DesiredInitialInterpMode, GEditorUserSettingsIni );

		// Update combo box
		ToolBar->InitialInterpModeComboBox->Select( DesiredInitialInterpMode );

		// Fire off a callback event for the combo box so that everything is properly refreshed
		wxCommandEvent In;
		In.SetInt( DesiredInitialInterpMode );
		OnChangeInitialInterpMode( In );
	}


	CurveEdToggleMenuItem = NULL;
	{
		// Create menu bar and 'Window' menu
		MenuBar = new WxInterpEdMenuBar(this);
		wxMenu* WindowMenu = AppendWindowMenu( MenuBar );
		SetMenuBar( MenuBar );

		// SetMenuBar should have caused our dockable WINDOWS to be added as checkable menu items to the Window menu,
		// so we'll go ahead and grab a pointer to the Curve Editor toggle menu item now.
		INT CurveEdToggleMenuItemIndex = WindowMenu->FindItem( CurveEdWindowTitle );
		CurveEdToggleMenuItem = WindowMenu->FindItem( CurveEdToggleMenuItemIndex );

		// Update button status
		bool bCurveEditorVisible = TRUE;		
		if( CurveEdToggleMenuItem != NULL )
		{
			bCurveEditorVisible = CurveEdToggleMenuItem->IsChecked();
		}
		ToolBar->ToggleTool( IDM_INTERP_TOGGLE_CURVEEDITOR, bCurveEditorVisible );
		MenuBar->ViewMenu->Check( IDM_INTERP_TOGGLE_CURVEEDITOR, bCurveEditorVisible );
	}

	// Will look at current selection to set active track
	ActorSelectionChange();

	// Load gradient texture for bars
	BarGradText = LoadObject<UTexture2D>(NULL, TEXT("EditorMaterials.MatineeGreyGrad"), NULL, LOAD_None, NULL);

	// If there is a Director group in this data, default to locking the camera to it.
	UInterpGroupDirector* DirGroup = IData->FindDirectorGroup();
	if(DirGroup)
	{
		LockCamToGroup(DirGroup);
	}

	for(INT i=0; i<GApp->EditorFrame->ViewportConfigData->GetViewportCount(); i++)
	{
		WxLevelViewportWindow* LevelVC = GApp->EditorFrame->ViewportConfigData->AccessViewport(i).ViewportWindow;
		if(LevelVC)
		{
			// If there is a director group, set the perspective viewports to realtime automatically.
			if(LevelVC->ViewportType == LVT_Perspective && LevelVC->AllowMatineePreview() )
			{				
				LevelVC->SetRealtime(TRUE);
			}

			// Turn on 'show camera frustums' flag
			LevelVC->ShowFlags |= SHOW_CamFrustums;
		}
	}

	// Update UI to reflect any change in realtime status
	GCallbackEvent->Send( CALLBACK_UpdateUI );

	// Load the desired window position from .ini file
	FWindowUtil::LoadPosSize(TEXT("Matinee"), this, -1, -1, 800,800);


	// OK, we're now initialized!
	bIsInitialized = TRUE;


	// Make sure any particle replay tracks are filled in with the correct state
	UpdateParticleReplayTracks();

	// Update visibility of director track window
	UpdateDirectorTrackWindowVisibility();
	
	// Now that we've filled in the track window's contents, reconfigure our scroll bar
	UpdateTrackWindowScrollBars();
}

WxInterpEd::~WxInterpEd()
{
}

/**
 * This function is called when the window has been selected from within the ctrl + tab dialog.
 */
void WxInterpEd::OnSelected()
{
	Raise();
}

void WxInterpEd::Serialize(FArchive& Ar)
{
	Ar << Interp;
	Ar << IData;
	Ar << NormalTransactor;
	Ar << Opt;

	// Check for non-NULL, as these references will be cleared in OnClose.
	if ( CurveEd )
	{
		CurveEd->CurveEdVC->Serialize(Ar);
	}
	if ( TrackWindow != NULL && TrackWindow->InterpEdVC != NULL )
	{
		TrackWindow->InterpEdVC->Serialize(Ar);
	}
	if ( DirectorTrackWindow != NULL && DirectorTrackWindow->InterpEdVC != NULL )
	{
		DirectorTrackWindow->InterpEdVC->Serialize(Ar);
	}
}


/** 
 * Starts playing the current sequence. 
 * @param bPlayLoop		Whether or not we should play the looping section.
 * @param bPlayForward	TRUE if we should play forwards, or FALSE for reverse
 */
void WxInterpEd::StartPlaying( UBOOL bPlayLoop, UBOOL bPlayForward )
{
	// Were we already in the middle of playback?
	const UBOOL bWasAlreadyPlaying = Interp->bIsPlaying;

	bAdjustingKeyframe = FALSE;

	bLoopingSection = bPlayLoop;
	if( bLoopingSection )
	{
		// If looping - jump to start of looping section.
		SetInterpPosition( IData->EdSectionStart );
	}

	// Start playing if we need to
	if( !bWasAlreadyPlaying )
	{
		// If 'snap time to frames' or 'fixed time step playback' is turned on, we'll make sure that we
		// start playback exactly on the closest frame
		if( bSnapToFrames && ( bSnapTimeToFrames || bFixedTimeStepPlayback ) )
		{
			SetInterpPosition( SnapTimeToNearestFrame( Interp->Position ) );
		}

		// Start playing
		Interp->bIsPlaying = TRUE;
		Interp->bReversePlayback = !bPlayForward;

		// Remember the real-time that we started playing the sequence
		PlaybackStartRealTime = appSeconds();
		NumContinuousFixedTimeStepFrames = 0;

		// Switch the Matinee windows to real-time so the track editor and curve editor update during playback
		TrackWindow->InterpEdVC->SetRealtime( TRUE );
		if( DirectorTrackWindow->IsShown() )
		{
			DirectorTrackWindow->InterpEdVC->SetRealtime( TRUE );
		}
	}
	else
	{
		// Switch playback directions if we need to
		if( Interp->bReversePlayback == bPlayForward )
		{
			Interp->ChangeDirection();

			// Reset our playback start time so fixed time step playback can gate frame rate properly
			PlaybackStartRealTime = appSeconds();
			NumContinuousFixedTimeStepFrames = 0;
		}
	}

	// Make sure fixed time step mode is set correctly based on whether we're currently 'playing' or not
	UpdateFixedTimeStepPlayback();
}

/** Stops playing the current sequence. */
void WxInterpEd::StopPlaying()
{
	// If already stopped, pressing stop again winds you back to the beginning.
	if(!Interp->bIsPlaying)
	{
		SetInterpPosition(0.f);
		return;
	}

	// Iterate over each group/track giving it a chance to stop things.
	for(INT i=0; i<Interp->GroupInst.Num(); i++)
	{
		UInterpGroupInst* GrInst = Interp->GroupInst(i);
		UInterpGroup* Group = GrInst->Group;

		check(Group->InterpTracks.Num() == GrInst->TrackInst.Num());
		for(INT j=0; j<Group->InterpTracks.Num(); j++)
		{
			UInterpTrack* Track = Group->InterpTracks(j);
			UInterpTrackInst* TrInst = GrInst->TrackInst(j);

			Track->PreviewStopPlayback(TrInst);
		}
	}

	// Set flag to indicate stopped
	Interp->bIsPlaying = FALSE;

	// Stop viewport being realtime
	TrackWindow->InterpEdVC->SetRealtime( FALSE );
	DirectorTrackWindow->InterpEdVC->SetRealtime( FALSE );

	// If the 'snap time to frames' option is enabled, we'll need to snap the time cursor position to
	// the nearest frame
	if( bSnapToFrames && bSnapTimeToFrames )
	{
		SetInterpPosition( SnapTimeToNearestFrame( Interp->Position ) );
	}

	// Make sure fixed time step mode is set correctly based on whether we're currently 'playing' or not
	UpdateFixedTimeStepPlayback();
}

/** Creates a popup context menu based on the item under the mouse cursor.
* @param	Viewport	FViewport for the FInterpEdViewportClient.
* @param	HitResult	HHitProxy returned by FViewport::GetHitProxy( ).
* @return	A new wxMenu with context-appropriate menu options or NULL if there are no appropriate menu options.
*/
wxMenu	*WxInterpEd::CreateContextMenu( FViewport *Viewport, const HHitProxy *HitResult )
{
	wxMenu	*Menu = NULL;

	if(HitResult->IsA(HInterpEdTrackBkg::StaticGetType()))
	{
		ClearKeySelection();
		SetActiveTrack(NULL, INDEX_NONE);				
		Viewport->Invalidate();

		Menu = new WxMBInterpEdBkgMenu( this );
	}
	else if(HitResult->IsA(HInterpEdGroupTitle::StaticGetType()))
	{
		UInterpGroup* Group = ((HInterpEdGroupTitle*)HitResult)->Group;

		ClearKeySelection();
		SetActiveTrack(Group, INDEX_NONE);
		Viewport->Invalidate();

		Menu = new WxMBInterpEdGroupMenu( this );		
	}
	else if(HitResult->IsA(HInterpEdTrackTitle::StaticGetType()))
	{
		UInterpGroup* Group = ((HInterpEdTrackTitle*)HitResult)->Group;
		const INT TrackIndex = ((HInterpEdTrackTitle*)HitResult)->TrackIndex;

		ClearKeySelection();
		SetActiveTrack(Group, TrackIndex);
		Viewport->Invalidate();

		Menu = new WxMBInterpEdTrackMenu( this );
	}
	else if(HitResult->IsA(HInterpTrackKeypointProxy::StaticGetType()))
	{
		UInterpGroup* Group = ((HInterpTrackKeypointProxy*)HitResult)->Group;
		const INT TrackIndex = ((HInterpTrackKeypointProxy*)HitResult)->TrackIndex;
		const INT KeyIndex = ((HInterpTrackKeypointProxy*)HitResult)->KeyIndex;

		const UBOOL bAlreadySelected = KeyIsInSelection(Group, TrackIndex, KeyIndex);
		if(bAlreadySelected)
		{
			Menu = new WxMBInterpEdKeyMenu( this );
		}
	}
	else if(HitResult->IsA(HInterpEdTab::StaticGetType()))
	{
		UInterpFilter* Filter = ((HInterpEdTab*)HitResult)->Filter;

		SetSelectedFilter(Filter);
		Viewport->Invalidate();

		Menu = new WxMBInterpEdTabMenu( this );
	}
	else if(HitResult->IsA(HInterpEdGroupCollapseBtn::StaticGetType()))
	{
		// Use right-clicked on the 'Expand/Collapse' track editor widget for a group
		Menu = new WxMBInterpEdCollapseExpandMenu( this );
	}

	return Menu;
}

void WxInterpEd::OnSize( wxSizeEvent& In )
{
	In.Skip();
}

void WxInterpEd::OnClose( wxCloseEvent& In )
{
	// Re-instate regular transactor
	check( GEditor->Trans == InterpEdTrans );
	check( NormalTransactor->IsA( UTransBuffer::StaticClass() ) );

	GEditor->ResetTransaction( *LocalizeUnrealEd("ExitMatinee") );
	GEditor->Trans = NormalTransactor;

	// Detach editor camera from any group and clear any previewing stuff.
	LockCamToGroup(NULL);

	// Restore the saved state of any actors we were previewing interpolation on.
	for(INT i=0; i<Interp->GroupInst.Num(); i++)
	{
		// Restore Actors to the state they were in when we opened Matinee.
		Interp->GroupInst(i)->RestoreGroupActorState();

		// Call TermTrackInst, but don't actually delete them. Leave them for GC.
		// Because we don't delete groups/tracks so undo works, we could end up deleting the Outer of a valid object.
		Interp->GroupInst(i)->TermGroupInst(false);

		// Set any manipulated cameras back to default frustum colours.
		ACameraActor* Cam = Cast<ACameraActor>(Interp->GroupInst(i)->GroupActor);
		if(Cam && Cam->DrawFrustum)
		{
			ACameraActor* DefCam = (ACameraActor*)(Cam->GetClass()->GetDefaultActor());
			Cam->DrawFrustum->FrustumColor = DefCam->DrawFrustum->FrustumColor;
		}
	}

	// Restore the bHidden state of all actors with visibility tracks
	Interp->RestoreActorVisibilities();

	// Movement tracks no longer save/restore relative actor positions.  Instead, the SeqAct_interp
	// stores actor positions/orientations so they can be precisely restored on Matinee close.
	// Note that this must happen before Interp's GroupInst array is emptied.
	Interp->RestoreActorTransforms();

	Interp->GroupInst.Empty();
	Interp->InterpData = NULL;

	// Indicate action is no longer being edited.
	Interp->bIsBeingEdited = FALSE;

	// Reset interpolation to the beginning when quitting.
	Interp->bIsPlaying = FALSE;
	Interp->Position = 0.f;

	bAdjustingKeyframe = FALSE;

	// When they close the window - change the mode away from InterpEdit.
	if( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit )
	{
		FEdModeInterpEdit* InterpEditMode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();

		// Only change mode if this window closing wasn't instigated by someone changing mode!
		if( !InterpEditMode->bLeavingMode )
		{
			InterpEditMode->InterpEd = NULL;
			GEditorModeTools().SetCurrentMode( EM_Default );
		}
	}

	// Undo any weird settings to editor level viewports.
	for(INT i=0; i<GApp->EditorFrame->ViewportConfigData->GetViewportCount(); i++)
	{
		WxLevelViewportWindow* LevelVC = GApp->EditorFrame->ViewportConfigData->AccessViewport(i).ViewportWindow;
		if(LevelVC)
		{
			// Turn off realtime when exiting.
			if(LevelVC->ViewportType == LVT_Perspective && LevelVC->AllowMatineePreview() )
			{				
				LevelVC->SetRealtime(FALSE);
			}

			// Turn off 'show camera frustums' flag.
			LevelVC->ShowFlags &= ~SHOW_CamFrustums;
		}
	}

	// Un-highlight selected track.
	if(ActiveGroup && ActiveTrackIndex != INDEX_NONE)
	{
		UInterpTrack* Track = ActiveGroup->InterpTracks(ActiveTrackIndex);
		IData->CurveEdSetup->ChangeCurveColor(Track, ActiveGroup->GroupColor);
	}


	// Make sure benchmarking mode is disabled (we may have turned it on for 'fixed time step playback')
	GIsBenchmarking = FALSE;
	

	// Update UI to reflect any change in realtime status
	GCallbackEvent->Send( CALLBACK_UpdateUI );

	// Redraw viewport as well, to show reset state of stuff.
	GCallbackEvent->Send( CALLBACK_RedrawAllViewports );


	// Save window position/size
	FWindowUtil::SavePosSize(TEXT("Matinee"), this);

	// Save the default splitter position for the curve editor.
	GConfig->SetInt(TEXT("Matinee"), TEXT("SplitterPos"), GraphSplitPos, GEditorUserSettingsIni);

	// Save docking layout.
	SaveDockingLayout();


	// Make sure our dock windows are unbound since we're about to destroy them
	UnbindDockingWindow( PropertyWindow );
	UnbindDockingWindow( CurveEd );

	// Kill our dockable windows, so that we don't end up trying to draw it's (possibly floating) viewport
	// again after the InterpEd is gone
	PropertyWindow->Destroy();
	PropertyWindow = NULL;
	CurveEd->Destroy();
	CurveEd = NULL;

	// Clear references to serialized members so they won't be serialized in the time
	// between the window closing and wx actually deleting it.
	bClosed = TRUE;
	Interp = NULL;
	IData = NULL;
	NormalTransactor = NULL;
	Opt = NULL;
	CurveEd = NULL;

	// Destroy the viewport, disassociating it from the engine, etc.
	TrackWindow->DestroyViewport();
	TrackWindow = NULL;
	DirectorTrackWindow->DestroyViewport();
	DirectorTrackWindow = NULL;

	// Destroy the window.
	this->Destroy();
}


/** Handle scrolling */
void WxInterpEd::OnScroll(wxScrollEvent& In)
{
	wxScrollBar* InScrollBar = wxDynamicCast(In.GetEventObject(), wxScrollBar);
	if (InScrollBar && DirectorTrackWindow != NULL && TrackWindow != NULL ) 
	{
		// Figure out which scroll bar has changed
		UBOOL bIsDirectorTrackWindow = ( InScrollBar == DirectorTrackWindow->ScrollBar_Vert );

		if( bIsDirectorTrackWindow )
		{
			DirectorTrackWindow->InterpEdVC->ThumbPos_Vert = -In.GetPosition();

			if( DirectorTrackWindow->IsShown() )
			{
				if (DirectorTrackWindow->InterpEdVC->Viewport)
				{
					// Force it to draw so the view change is seen
					DirectorTrackWindow->InterpEdVC->Viewport->Invalidate();
					DirectorTrackWindow->InterpEdVC->Viewport->Draw();
				}
			}
		}
		else
		{
			TrackWindow->InterpEdVC->ThumbPos_Vert = -In.GetPosition();

			if (TrackWindow->InterpEdVC->Viewport)
			{
				// Force it to draw so the view change is seen
				TrackWindow->InterpEdVC->Viewport->Invalidate();
				TrackWindow->InterpEdVC->Viewport->Draw();
			}
		}
	}
}

void WxInterpEd::DrawTracks3D(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	for(INT i=0; i<Interp->GroupInst.Num(); i++)
	{
		UInterpGroupInst* GrInst = Interp->GroupInst(i);
		check( GrInst->Group );
		check( GrInst->TrackInst.Num() == GrInst->Group->InterpTracks.Num() );

		// In 3D viewports, Don't draw path if we are locking the camera to this group.
		//if( !(View->Family->ShowFlags & SHOW_Orthographic) && (Group == CamViewGroup) )
		//	continue;

		for(INT j=0; j<GrInst->TrackInst.Num(); j++)
		{
			UInterpTrackInst* TrInst = GrInst->TrackInst(j);
			UInterpTrack* Track = GrInst->Group->InterpTracks(j);

			//UBOOL bTrackSelected = ((GrInst->Group == ActiveGroup) && (j == ActiveTrackIndex));
			UBOOL bTrackSelected = (GrInst->Group == ActiveGroup);
			FColor TrackColor = bTrackSelected ? Track3DSelectedColor : GrInst->Group->GroupColor;

			Track->Render3DTrack( TrInst, View, PDI, j, TrackColor, Opt->SelectedKeys);
		}
	}
}

void WxInterpEd::DrawModeHUD(FEditorLevelViewportClient* ViewportClient,FViewport* Viewport,const FSceneView* View,FCanvas* Canvas)
{
	// If 'frame stats' are turned on and this viewport is configured for Matinee preview, then draw some text
	if( ViewportClient->AllowMatineePreview() && IsViewportFrameStatsEnabled() )
	{
		INT XL, YL;
		INT YPos = 3;
		INT XPos = 5;

		// Title
		{
			FString StatsString = *LocalizeUnrealEd( "Matinee" );
			DrawShadowedString( Canvas, XPos, YPos, *StatsString, GEngine->SmallFont, FLinearColor::White );
			StringSize( GEngine->SmallFont, XL, YL, *StatsString );
			XPos += XL;
		}

		// Frame counts
		{
			FString StatsString =
				FString::Printf(
					TEXT("%3.1f / %3.1f %s"),
					(1.0 / SnapAmount) * Interp->Position,
					(1.0 / SnapAmount) * IData->InterpLength,
					*LocalizeUnrealEd("InterpEd_TimelineInfo_Frames") );
			XPos += 16;
			DrawShadowedString( Canvas, XPos, YPos, *StatsString, GEngine->TinyFont, FLinearColor( 0, 1, 0 ) );
			StringSize( GEngine->SmallFont, XL, YL, *StatsString );
			XPos += XL;
		}

		// SMTPE-style timecode
		if( bSnapToFrames )
		{
			FString StatsString = MakeTimecodeString( Interp->Position );
			XPos += 12;
			DrawShadowedString( Canvas, XPos, YPos, *StatsString, GEngine->TinyFont, FLinearColor( 1, 1, 0 ) );
			StringSize( GEngine->SmallFont, XL, YL, *StatsString );
			XPos += XL;
		}
	}

	// Show a notification if we are adjusting a particular keyframe.
	if(bAdjustingKeyframe)
	{
		check(Opt->SelectedKeys.Num() == 1);

		FInterpEdSelKey& SelKey = Opt->SelectedKeys(0);
		FString AdjustNotify = FString::Printf( LocalizeSecure(LocalizeUnrealEd("AdjustKey_F"), SelKey.KeyIndex) );

		INT XL, YL;
		StringSize(GEngine->SmallFont, XL, YL, *AdjustNotify);
		DrawShadowedString(Canvas, 5, Viewport->GetSizeY() - (3 + YL) , *AdjustNotify, GEngine->SmallFont, FLinearColor( 1, 0, 0 ) );
	}
}


///////////////////////////////////////////////////////////////////////////////////////
// Properties window NotifyHook stuff

void WxInterpEd::NotifyDestroy( void* Src )
{

}

void WxInterpEd::NotifyPreChange( void* Src, UProperty* PropertyAboutToChange )
{

}

void WxInterpEd::NotifyPostChange( void* Src, UProperty* PropertyThatChanged )
{
	CurveEd->CurveChanged();

	// Dirty the track window viewports
	InvalidateTrackWindowViewports();

	// If we are changing the properties of a Group, propagate changes to the GroupAnimSets array to the Actors being controlled by this group.
	if(ActiveGroup && ActiveTrackIndex == INDEX_NONE && ActiveGroup->HasAnimControlTrack())
	{
		for(INT i=0; i<Interp->GroupInst.Num(); i++)
		{
			if(Interp->GroupInst(i)->Group == ActiveGroup)
			{
				AActor* Actor = Interp->GroupInst(i)->GetGroupActor();
				if(Actor)
				{
					Actor->PreviewBeginAnimControl(ActiveGroup->GroupAnimSets);
				}
			}
		}

		// Update to current position - so changes in AnimSets take affect now.
		RefreshInterpPosition();
	}
}

void WxInterpEd::NotifyExec( void* Src, const TCHAR* Cmd )
{
	
}

///////////////////////////////////////////////////////////////////////////////////////
// Curve editor notify stuff

/** Implement Curve Editor notify interface, so we can back up state before changes and support Undo. */
void WxInterpEd::PreEditCurve(TArray<UObject*> CurvesAboutToChange)
{
	InterpEdTrans->BeginSpecial(*LocalizeUnrealEd("CurveEdit"));

	// Call Modify on all tracks with keys selected
	for(INT i=0; i<CurvesAboutToChange.Num(); i++)
	{
		// If this keypoint is from an InterpTrack, call Modify on it to back up its state.
		UInterpTrack* Track = Cast<UInterpTrack>( CurvesAboutToChange(i) );
		if(Track)
		{
			Track->Modify();
		}
	}
}

void WxInterpEd::PostEditCurve()
{
	InterpEdTrans->EndSpecial();
}

void WxInterpEd::MovedKey()
{
	// Update interpolation to the current position - but thing may have changed due to fiddling on the curve display.
	RefreshInterpPosition();
}

void WxInterpEd::DesireUndo()
{
	InterpEdUndo();
}

void WxInterpEd::DesireRedo()
{
	InterpEdRedo();
}


/**
 * FCurveEdNotifyInterface: Called by the Curve Editor when a Curve Label is clicked on
 *
 * @param	CurveObject	The curve object whose label was clicked on
 */
void WxInterpEd::OnCurveLabelClicked( UObject* CurveObject )
{
	check( CurveObject != NULL );

	// Is this curve an interp track?
	UInterpTrack* Track = Cast<UInterpTrack>( CurveObject );
	if( Track != NULL )
	{
		UInterpGroup* Group = CastChecked< UInterpGroup >( Track->GetOuter() );
		
		// Find the index for the track that was selected
		INT CurTrackIndex;
		for( CurTrackIndex = 0; CurTrackIndex < Group->InterpTracks.Num(); ++CurTrackIndex )
		{
			if( Group->InterpTracks( CurTrackIndex ) == Track )
			{
				// Found the track index!
				break;
			}
		}

		// Did we found a track?
		if( CurTrackIndex < Group->InterpTracks.Num() )
		{
			// Select the track!
			SetActiveTrack( Group, CurTrackIndex );
			ClearKeySelection();
		}
	}
}



/**
 * Either shows or hides the director track window by splitting/unsplitting the parent window
 */
void WxInterpEd::UpdateDirectorTrackWindowVisibility()
{
	// Do we have a director group?  If so, then the director track window will be implicitly visible!
	UInterpGroupDirector* DirGroup = IData->FindDirectorGroup();
	UBOOL bWantDirectorTrackWindow = ( DirGroup != NULL );

	// Show director track window by splitting the window
	if( bWantDirectorTrackWindow && !GraphSplitterWnd->IsSplit() )
	{
		GraphSplitterWnd->SplitHorizontally( DirectorTrackWindow, TrackWindow, GraphSplitPos );
		DirectorTrackWindow->Show( TRUE );

		// When both windows are visible, we'll draw the tabs in the director track window and the timeline in the
		// regular track window
		DirectorTrackWindow->InterpEdVC->bWantFilterTabs = TRUE;
		DirectorTrackWindow->InterpEdVC->bWantTimeline = FALSE;
		TrackWindow->InterpEdVC->bWantFilterTabs = FALSE;
		TrackWindow->InterpEdVC->bWantTimeline = TRUE;
	}
	// Hide the director track window
	else if( !bWantDirectorTrackWindow && GraphSplitterWnd->IsSplit() )
	{
		GraphSplitterWnd->Unsplit( DirectorTrackWindow );
		DirectorTrackWindow->Show( FALSE );

		// When only the regular track window is visible, we'll draw both the tabs and timeline in that window!
		TrackWindow->InterpEdVC->bWantFilterTabs = TRUE;
		TrackWindow->InterpEdVC->bWantTimeline = TRUE;
	}
}



/**
 * Locates the specified group's parent group folder, if it has one
 *
 * @param ChildGroup The group who's parent we should search for
 *
 * @return Returns the parent group pointer or NULL if one wasn't found
 */
UInterpGroup* WxInterpEd::FindParentGroupFolder( UInterpGroup* ChildGroup )
{
	// Does this group even have a parent?
	if( ChildGroup->bIsParented )
	{
		check( !ChildGroup->bIsFolder );

		// Find the child group list index
		INT ChildGroupIndex = -1;
		if( IData->InterpGroups.FindItem( ChildGroup, ChildGroupIndex ) )
		{
			// Iterate backwards in the group list starting at the child group index, looking for its parent
			for( INT CurGroupIndex = ChildGroupIndex - 1; CurGroupIndex >= 0; --CurGroupIndex )
			{
				UInterpGroup* CurGroup = IData->InterpGroups( CurGroupIndex );

				// Just skip the director group if we find it; it's not allowed to be a parent
				if( !CurGroup->IsA( UInterpGroupDirector::StaticClass() ) )
				{
					// Is the current group a top level folder?
					if( !CurGroup->bIsParented )
					{
						check( CurGroup->bIsFolder );

						// Found it!
						return CurGroup;
					}
				}
			}
		}
	}

	// Not found
	return NULL;
}



/**
 * Counts the number of children that the specified group folder has
 *
 * @param GroupFolder The group who's children we should count
 *
 * @return Returns the number of child groups
 */
INT WxInterpEd::CountGroupFolderChildren( UInterpGroup* const GroupFolder ) const
{
	INT ChildCount = 0;

	// Child groups currently don't support containing their own children
	if( GroupFolder->bIsFolder && !GroupFolder->bIsParented )
	{
		INT StartIndex = IData->InterpGroups.FindItemIndex( GroupFolder ) + 1;
		for( INT CurGroupIndex = StartIndex; CurGroupIndex < IData->InterpGroups.Num(); ++CurGroupIndex	)
		{
			UInterpGroup* CurGroup = IData->InterpGroups( CurGroupIndex );

			// Children always appear sequentially after their parent in the array, so if we find an unparented item, then 
			// we know we've reached the last child
			if( CurGroup->bIsParented )
			{
				// Found a child!
				++ChildCount;
			}
			else
			{
				// No more children
				break;
			}
		}
	}

	return ChildCount;
}


/**
 * Fixes up any problems in the folder/group hierarchy caused by bad parenting in previous builds
 */
void WxInterpEd::RepairHierarchyProblems()
{
	UBOOL bAnyRepairsMade = FALSE;

	UBOOL bPreviousGroupWasFolder = FALSE;
	UBOOL bPreviousGroupWasParented = FALSE;

	for( INT CurGroupIndex = 0; CurGroupIndex < IData->InterpGroups.Num(); ++CurGroupIndex )
	{
		UInterpGroup* CurGroup = IData->InterpGroups( CurGroupIndex );
		if( CurGroup != NULL )
		{
			if( CurGroup->bIsFolder )
			{
				// This is a folder group.
				
				// Folders are never allowed to be parented
				if( CurGroup->bIsParented )
				{
					// Repair parenting problem
					CurGroup->bIsParented = FALSE;
					bAnyRepairsMade = TRUE;
				}
			}
			else if( CurGroup->bIsParented )
			{
				// This group is parented to a folder
				
				// Make sure the previous group in the list was either a folder OR a parented group
				if( !bPreviousGroupWasFolder && !bPreviousGroupWasParented )
				{
					// Uh oh, the current group thinks its parented but the previous item is not a folder
					// or another parented group.  This means the current group thinks its parented to
					// another root group.  No good!  We'll unparent the group to fix this.
					CurGroup->bIsParented = FALSE;
					bAnyRepairsMade = TRUE;
				}
			}

			// If this is a 'director group', its never allowed to be parented (or act as a folder)
			if( CurGroup->IsA( UInterpGroupDirector::StaticClass() ) )
			{
				if( CurGroup->bIsParented )
				{
					// Director groups cannot be parented
					CurGroup->bIsParented = FALSE;
					bAnyRepairsMade = TRUE;
				}

				if( CurGroup->bIsFolder )
				{
					// Director groups cannot act as a folder
					CurGroup->bIsFolder = FALSE;
					bAnyRepairsMade = TRUE;
				}
			}

			// Keep track of this group's status for the next iteration's tests
			bPreviousGroupWasFolder	 = CurGroup->bIsFolder;
			bPreviousGroupWasParented = CurGroup->bIsParented;
		}
		else
		{
			// Bad group pointer, so remove this element from the list
			IData->InterpGroups.Remove( 0 );
			--CurGroupIndex;
			bAnyRepairsMade = TRUE;
		}
	}


	if( bAnyRepairsMade )
	{
		// Dirty the package so that editor changes will be saved
		IData->MarkPackageDirty();

		// Notify the user
		appMsgf( AMT_OK, *LocalizeUnrealEd( "InterpEd_HierachyRepairsNotification" ) );
	}
}



///////////////////////////////////////////////////////////////////////////////////////
// FDockingParent Interface

/**
 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
 *  @return A string representing a name to use for this docking parent.
 */
const TCHAR* WxInterpEd::GetDockingParentName() const
{
	return TEXT("Matinee");
}

/**
 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
 */
const INT WxInterpEd::GetDockingParentVersion() const
{
	// NOTE: Version 0 supported a dockable Property window
	// NOTE: Version 1 added support for a dockable Curve Editor window
	return 1;
}


///////////////////////////////////////////////////////////////////////////////////////
// WxCameraAnimEd editor

WxCameraAnimEd::WxCameraAnimEd( wxWindow* InParent, wxWindowID InID, class USeqAct_Interp* InInterp )
:	WxInterpEd(InParent, InID, InInterp)
{
	SetTitle(*LocalizeUnrealEd("MatineeCamAnimMode"));
}

WxCameraAnimEd::~WxCameraAnimEd()
{

}

/** Creates a popup context menu based on the item under the mouse cursor.
* @param	Viewport	FViewport for the FInterpEdViewportClient.
* @param	HitResult	HHitProxy returned by FViewport::GetHitProxy( ).
* @return	A new wxMenu with context-appropriate menu options or NULL if there are no appropriate menu options.
*/
wxMenu	*WxCameraAnimEd::CreateContextMenu( FViewport *Viewport, const HHitProxy *HitResult )
{
	wxMenu	*Menu = NULL;

	if(HitResult->IsA(HInterpEdTrackBkg::StaticGetType()))
	{
		// no menu, explicitly ignore this case
	}
	else if(HitResult->IsA(HInterpEdGroupTitle::StaticGetType()))
	{
		UInterpGroup* Group = ((HInterpEdGroupTitle*)HitResult)->Group;

		ClearKeySelection();
		SetActiveTrack(Group, INDEX_NONE);
		Viewport->Invalidate();

		Menu = new WxMBCameraAnimEdGroupMenu( this );		
	}
	else 
	{
		// let our parent handle it
		Menu = WxInterpEd::CreateContextMenu(Viewport, HitResult);
	}

	return Menu;
}

void WxCameraAnimEd::OnClose(wxCloseEvent& In)
{
	// delete the attached objects.  this should take care of the temp camera actor.
	TArray<UObject**> ObjectVars;
	Interp->GetObjectVars(ObjectVars);
	for (INT Idx=0; Idx<ObjectVars.Num(); ++Idx)
	{
		AActor* Actor = Cast<AActor>(*(ObjectVars(Idx)));

		// prevents a NULL in the selection
		if (Actor->IsSelected())
		{
			GEditor->GetSelectedActors()->Deselect(Actor);
		}

		GWorld->DestroyActor(Actor);
	}

	// need to destroy all of the temp kismet stuff
	TArray<USequenceObject*> SeqsToDelete;
	{
		SeqsToDelete.AddItem(Interp);

		if (Interp->InterpData)
		{
			// delete everything linked to the temp interp.  this should take care of
			// both the interpdata and the cameraactor's seqvar_object
			for (INT Idx=0; Idx<Interp->VariableLinks.Num(); ++Idx)
			{
				FSeqVarLink* Link = &Interp->VariableLinks(Idx);

				for (INT VarIdx=0; VarIdx<Link->LinkedVariables.Num(); ++VarIdx)
				{
					SeqsToDelete.AddItem(Link->LinkedVariables(VarIdx));
				}
			}
		}
	}
	USequence* RootSeq = Interp->GetRootSequence();
	RootSeq->RemoveObjects(SeqsToDelete);

	// update all kismet property windows, so we don't have a property window with a dangling 
	// ref to the seq objects we just deleted
	for(INT i=0; i<GApp->KismetWindows.Num(); i++)
	{
		WxKismet* const KismetWindow = GApp->KismetWindows(i);
		for (INT SeqIdx=0; SeqIdx<SeqsToDelete.Num(); SeqIdx++)
		{
			KismetWindow->RemoveFromSelection(SeqsToDelete(SeqIdx));
		}
		KismetWindow->UpdatePropertyWindow();
	}

	// make sure destroyed actors get flushed asap
	GWorld->GetWorldInfo()->ForceGarbageCollection();

	// Fill in AnimLength parameter, in case it changed during editing
	if ( Interp && Interp->InterpData && (Interp->InterpData->InterpGroups.Num() > 0) )
	{
		UCameraAnim* const CamAnim = Cast<UCameraAnim>(Interp->InterpData->InterpGroups(0)->GetOuter());
		if (CamAnim)
		{
			if (CamAnim->AnimLength != Interp->InterpData->InterpLength)
			{
				CamAnim->AnimLength = Interp->InterpData->InterpLength;
				CamAnim->MarkPackageDirty(TRUE);
			}
		}
	}

	// update generic browser to reflect any changes we've made
	WxGenericBrowser* GBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
	GBrowser->Update();	
	
	// clean up any open property windows, in case one of them points to something we just deleted
	GUnrealEd->UpdatePropertyWindows();

	WxInterpEd::OnClose(In);
}

void WxCameraAnimEd::UpdateCameraToGroup()
{
	// Make sure camera's PP settings get applied additively.
	AActor* ViewedActor = GetViewedActor();
	if(ViewedActor)
	{
		ACameraActor* Cam = Cast<ACameraActor>(ViewedActor);
		ACameraActor const* const DefaultCamActor = ACameraActor::StaticClass()->GetDefaultObject<ACameraActor>();
		FPostProcessSettings const* const DefaultCamActorPPSettings = DefaultCamActor ? &DefaultCamActor->CamOverridePostProcess : NULL;

		// Move any perspective viewports to coincide with moved actor.
		for(INT i=0; i<GApp->EditorFrame->ViewportConfigData->GetViewportCount(); i++)
		{
			WxLevelViewportWindow* LevelVC = GApp->EditorFrame->ViewportConfigData->AccessViewport(i).ViewportWindow;
			if (LevelVC && (LevelVC->ViewportType == LVT_Perspective) && LevelVC->AllowMatineePreview() )
			{
				if( (Cam != NULL) && (DefaultCamActorPPSettings != NULL) )
				{
					LevelVC->bAdditivePostProcessSettings = TRUE;
					LevelVC->AdditivePostProcessSettings = Cam->CamOverridePostProcess;

					// subtract defaults to leave only the additive portion
					LevelVC->AdditivePostProcessSettings.AddInterpolatable(*DefaultCamActorPPSettings, -1.f);
				}
				else
				{
					LevelVC->bAdditivePostProcessSettings = FALSE;
				}
			}
		}
	}

	WxInterpEd::UpdateCameraToGroup();
}




