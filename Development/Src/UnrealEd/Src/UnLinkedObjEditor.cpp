/*=============================================================================
	UnLinkedEdInterface.cpp: Base class for boxes-and-lines editing
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "UnLinkedObjEditor.h"
#include "UnLinkedObjDrawUtils.h"
#include "Properties.h"
#include "EngineSequenceClasses.h"

const static FLOAT	LinkedObjectEditor_ZoomIncrement = 0.1f;
const static FLOAT	LinkedObjectEditor_ZoomSpeed = 0.005f;
const static FLOAT	LinkedObjectEditor_ZoomNotchThresh = 0.007f;
const static INT	LinkedObjectEditor_ScrollBorderSize = 20;
const static FLOAT	LinkedObjectEditor_ScrollBorderSpeed = 400.f;

/*-----------------------------------------------------------------------------
	FLinkedObjViewportClient
-----------------------------------------------------------------------------*/

FLinkedObjViewportClient::FLinkedObjViewportClient( FLinkedObjEdNotifyInterface* InEdInterface )
	:	bInvertMousePan( FALSE )
	,	bAlwaysDrawInTick( FALSE )
{
	// No postprocess.
	ShowFlags			&= ~SHOW_PostProcess;

	// This window will be 2D/canvas only, so set the viewport type to None
	ViewportType = LVT_None;

	EdInterface			= InEdInterface;

	Origin2D			= FIntPoint(0, 0);
	Zoom2D				= 1.0f;
	MinZoom2D			= 0.1f;
	MaxZoom2D			= 1.f;

	bMouseDown			= false;
	OldMouseX			= 0;
	OldMouseY			= 0;

	BoxStartX			= 0;
	BoxStartY			= 0;
	BoxEndX				= 0;
	BoxEndY				= 0;

	DeltaXFraction		= 0.0f;
	DeltaYFraction		= 0.0f;

	ScrollAccum			= FVector2D(0,0);

	DistanceDragged		= 0;

	bTransactionBegun	= false;
	bMakingLine			= false;
	bSpecialDrag		= false;
	bBoxSelecting		= false;
	bAllowScroll		= true;

	MouseOverObject		= NULL;
	MouseOverTime		= 0.f;

	SpecialIndex		= 0;
		
	SetRealtime( FALSE );

	// Load user settings from the editor user settings .ini.
	GConfig->GetBool( TEXT("LinkedObjectEditor"), TEXT("InvertMousePan"), bInvertMousePan, GEditorUserSettingsIni );
}

void FLinkedObjViewportClient::Draw(FViewport* Viewport, FCanvas* Canvas)
{
	Canvas->PushAbsoluteTransform(FScaleMatrix(Zoom2D) * FTranslationMatrix(FVector(Origin2D.X,Origin2D.Y,0)));
	{
		// Erase background
		Clear(Canvas, FColor(197,197,197) );

		EdInterface->DrawObjects( Viewport, Canvas );

		// Draw new line
		if(bMakingLine && !Canvas->IsHitTesting())
		{
			FIntPoint StartPoint = EdInterface->GetSelectedConnLocation(Canvas);
			FIntPoint EndPoint( (NewX - Origin2D.X)/Zoom2D, (NewY - Origin2D.Y)/Zoom2D );
			INT ConnType = EdInterface->GetSelectedConnectorType();
			FColor LinkColor = EdInterface->GetMakingLinkColor();

			if(EdInterface->DrawCurves())// && (ConnType != LOC_VARIABLE) && (ConnType != LOC_EVENT))
			{
				FLOAT Tension;
				if(ConnType == LOC_INPUT || ConnType == LOC_OUTPUT)
				{
					Tension = Abs<INT>(StartPoint.X - EndPoint.X);
				}
				else
				{
					Tension = Abs<INT>(StartPoint.Y - EndPoint.Y);
				}


				if(ConnType == LOC_INPUT)
				{
					FLinkedObjDrawUtils::DrawSpline(Canvas, StartPoint, Tension*FVector2D(-1,0), EndPoint, Tension*FVector2D(-1,0), LinkColor, false);
				}
				else if(ConnType == LOC_OUTPUT)
				{
					FLinkedObjDrawUtils::DrawSpline(Canvas, StartPoint, Tension*FVector2D(1,0), EndPoint, Tension*FVector2D(1,0), LinkColor, false);
				}
				else if(ConnType == LOC_VARIABLE)
				{
					FLinkedObjDrawUtils::DrawSpline(Canvas, StartPoint, Tension*FVector2D(0,1), EndPoint, FVector2D(0,0), LinkColor, false);
				}
				else
				{
					FLinkedObjDrawUtils::DrawSpline(Canvas, StartPoint, Tension*FVector2D(0,1), EndPoint, Tension*FVector2D(0,1), LinkColor, false);
				}
			}
			else
			{
				DrawLine2D(Canvas, StartPoint, EndPoint, LinkColor );
			}
		}

	}
	Canvas->PopTransform();

	Canvas->PushAbsoluteTransform(FTranslationMatrix(FVector(Origin2D.X,Origin2D.Y,0)));
	{
		// Draw the box select box
		if(bBoxSelecting)
		{
			INT MinX = (Min(BoxStartX, BoxEndX) - BoxOrigin2D.X);
			INT MinY = (Min(BoxStartY, BoxEndY) - BoxOrigin2D.Y);
			INT MaxX = (Max(BoxStartX, BoxEndX) - BoxOrigin2D.X);
			INT MaxY = (Max(BoxStartY, BoxEndY) - BoxOrigin2D.Y);

			DrawLine2D(Canvas,FVector2D(MinX, MinY), FVector2D(MaxX, MinY), FColor(255,0,0));
			DrawLine2D(Canvas,FVector2D(MaxX, MinY), FVector2D(MaxX, MaxY), FColor(255,0,0));
			DrawLine2D(Canvas,FVector2D(MaxX, MaxY), FVector2D(MinX, MaxY), FColor(255,0,0));
			DrawLine2D(Canvas,FVector2D(MinX, MaxY), FVector2D(MinX, MinY), FColor(255,0,0));
		}
	}
	Canvas->PopTransform();
}

UBOOL FLinkedObjViewportClient::InputKey(FViewport* Viewport, INT ControllerId, FName Key, EInputEvent Event,FLOAT /*AmountDepressed*/,UBOOL /*Gamepad*/)
{
	const UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);
	const UBOOL bShiftDown = Viewport->KeyState(KEY_LeftShift) || Viewport->KeyState(KEY_RightShift);
	const UBOOL bAltDown = Viewport->KeyState(KEY_LeftAlt) || Viewport->KeyState(KEY_RightAlt);

	const INT HitX = Viewport->GetMouseX();
	const INT HitY = Viewport->GetMouseY();

	if ( !Viewport->HasMouseCapture() )
	{
		Viewport->ShowCursor( TRUE );
		Viewport->LockMouseToWindow( FALSE );
	}

	static EInputEvent LastEvent = IE_Pressed;

	if( Key == KEY_LeftMouseButton )
	{
		switch( Event )
		{
		case IE_Pressed:
		case IE_DoubleClick:
			{
				DeltaXFraction = 0.0f;
				DeltaYFraction = 0.0f;
				HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);

				if(HitResult)
				{
					if (HitResult->IsA(HLinkedObjLineProxy::StaticGetType()))
					{
						// clicked on a line
						HLinkedObjLineProxy *LineProxy = (HLinkedObjLineProxy*)HitResult;
						if (Event == IE_Pressed)
						{
							EdInterface->ClickedLine(LineProxy->Src,LineProxy->Dest);
						}
						else
						if (Event == IE_DoubleClick)
						{
							EdInterface->DoubleClickedLine(LineProxy->Src,LineProxy->Dest);
						}
					}
					else if(HitResult->IsA(HLinkedObjProxy::StaticGetType()))
					{
						UObject* Obj = ((HLinkedObjProxy*)HitResult)->Obj;

						if(!bCtrlDown)
						{
							EdInterface->EmptySelection();
							EdInterface->AddToSelection(Obj);

							EdInterface->UpdatePropertyWindow();

							if(Event == IE_DoubleClick)
							{
								EdInterface->DoubleClickedObject(Obj);
								bMouseDown = false;
								return TRUE;
							}
						}
					}
					else if(HitResult->IsA(HLinkedObjConnectorProxy::StaticGetType()))
					{
						HLinkedObjConnectorProxy* ConnProxy = (HLinkedObjConnectorProxy*)HitResult;
						EdInterface->SetSelectedConnector( ConnProxy->Connector );

						EdInterface->EmptySelection();
						EdInterface->UpdatePropertyWindow();

						if (bAltDown)
						{
							// break the connectors
							EdInterface->AltClickConnector( ConnProxy->Connector );
						}
						else
						{
							if (Event == IE_DoubleClick)
							{
								EdInterface->DoubleClickedConnector(ConnProxy->Connector);
							}
							else
							{
								bMakingLine = true;
								NewX = HitX;
								NewY = HitY;
							}
						}
					}
					else if(HitResult->IsA(HLinkedObjProxySpecial::StaticGetType()))
					{
						HLinkedObjProxySpecial* SpecialProxy = (HLinkedObjProxySpecial*)HitResult;

						// Copy properties out of SpecialProxy first, in case it gets invalidated!
						INT ProxyIndex = SpecialProxy->SpecialIndex;
						UObject* ProxyObj = SpecialProxy->Obj;

						FIntPoint MousePos( (HitX - Origin2D.X)/Zoom2D, (HitY - Origin2D.Y)/Zoom2D );

						// If object wasn't selected already OR 
						// we didn't handle it all in special click - change selection
						if( !EdInterface->IsInSelection(ProxyObj) || 
							!EdInterface->SpecialClick(  MousePos.X, MousePos.Y, ProxyIndex, Viewport ) )
						{
							bSpecialDrag = true;
							SpecialIndex = ProxyIndex;

							// Slightly quirky way of avoiding selecting the same thing again.
							if(!(EdInterface->GetNumSelected() == 1 && EdInterface->IsInSelection(ProxyObj)))
							{
								EdInterface->EmptySelection();
								EdInterface->AddToSelection(ProxyObj);
								EdInterface->UpdatePropertyWindow();
							}

							// For supporting undo 
							EdInterface->BeginTransactionOnSelected();
							bTransactionBegun = true;
						}
					}
				}
				else
				{
					if(bCtrlDown && bAltDown)
					{
						BoxOrigin2D = Origin2D;
						BoxStartX = BoxEndX = HitX;
						BoxStartY = BoxEndY = HitY;

						bBoxSelecting = true;
					}
				}

				OldMouseX = HitX;
				OldMouseY = HitY;
				DistanceDragged = 0;
				bMouseDown = true;

				if( !bMakingLine && !bBoxSelecting && !bSpecialDrag && !(bCtrlDown && EdInterface->HaveObjectsSelected()) && bAllowScroll )
				{
				}
				else
				{
					Viewport->LockMouseToWindow(true);
				}

				Viewport->Invalidate();
			}
			break;

		case IE_Released:
			{
				if(bMakingLine)
				{
					Viewport->Invalidate();
					HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);

					if(HitResult)
					{
						if( HitResult->IsA(HLinkedObjConnectorProxy::StaticGetType()) )
						{
							HLinkedObjConnectorProxy* EndConnProxy = (HLinkedObjConnectorProxy*)HitResult;

							if( DistanceDragged < 4 )
							{
								HLinkedObjConnectorProxy* ConnProxy = (HLinkedObjConnectorProxy*)HitResult;
								UBOOL bDoDeselect = EdInterface->ClickOnConnector(EndConnProxy->Connector.ConnObj, EndConnProxy->Connector.ConnType, EndConnProxy->Connector.ConnIndex);
								if(bDoDeselect && LastEvent != IE_DoubleClick)
								{
									EdInterface->EmptySelection();
									EdInterface->UpdatePropertyWindow();
								}
							}
							else if ( bAltDown )
							{
								EdInterface->AltClickConnector( EndConnProxy->Connector );
							}
							else
							{
								EdInterface->MakeConnectionToConnector( EndConnProxy->Connector );
							}
						}
						else if( HitResult->IsA(HLinkedObjProxy::StaticGetType()) )
						{
							UObject* Obj = ((HLinkedObjProxy*)HitResult)->Obj;

							EdInterface->MakeConnectionToObject( Obj );
						}
					}
				}
				else if( bBoxSelecting )
				{
					// When box selecting, the region that user boxed can be larger than the size of the viewport
					// so we use the viewport as a max region and loop through the box, rendering different chunks of it
					// and reading back its hit proxy map to check for objects.
					TArray<UObject*> NewSelection;
					
					// Save the current origin since we will be modifying it.
					FVector2D SavedOrigin2D;
					
					SavedOrigin2D.X = Origin2D.X;
					SavedOrigin2D.Y = Origin2D.Y;

					// Calculate the size of the box and its extents.
					const INT MinX = Min(BoxStartX, BoxEndX);
					const INT MinY = Min(BoxStartY, BoxEndY);
					const INT MaxX = Max(BoxStartX, BoxEndX) + 1;
					const INT MaxY = Max(BoxStartY, BoxEndY) + 1;

					const INT ViewX = Viewport->GetSizeX()-1;
					const INT ViewY = Viewport->GetSizeY()-1;

					const INT BoxSizeX = MaxX - MinX;
					const INT BoxSizeY = MaxY - MinY;

					const FLOAT BoxMinX = MinX-BoxOrigin2D.X;
					const FLOAT BoxMinY = MinY-BoxOrigin2D.Y;
					const FLOAT BoxMaxX = BoxMinX + BoxSizeX;
					const FLOAT BoxMaxY = BoxMinY + BoxSizeY;

					// Loop through 'tiles' of the box using the viewport size as our maximum tile size.
					INT TestSizeX = Min(ViewX, BoxSizeX);
					INT TestSizeY = Min(ViewY, BoxSizeY);

					FLOAT TestStartX = BoxMinX;
					FLOAT TestStartY = BoxMinY;

					while(TestStartX < BoxMaxX)
					{
						TestStartY = BoxMinY;
						TestSizeY = Min(ViewY, BoxSizeY);
				
						while(TestStartY < BoxMaxY)
						{
							// We read back the hit proxy map for the required region.
							Origin2D.X = -TestStartX;
							Origin2D.Y = -TestStartY;

							TArray<HHitProxy*> ProxyMap;
							Viewport->Invalidate();
							Viewport->GetHitProxyMap((UINT)0, (UINT)0, (UINT)TestSizeX, (UINT)TestSizeY, ProxyMap);

							// Find any keypoint hit proxies in the region - add the keypoint to selection.
							for(INT Y=0; Y < TestSizeY; Y++)
							{
								for(INT X=0; X < TestSizeX; X++)
								{
									HHitProxy* HitProxy = NULL;							
									INT ProxyMapIndex = Y * TestSizeX + X; // calculate location in proxy map
									if(ProxyMapIndex < ProxyMap.Num()) // If within range, grab the hit proxy from there
									{
										HitProxy = ProxyMap(ProxyMapIndex);
									}

									UObject* SelObject = NULL;

									// If we got one, add it to the NewSelection list.
									if(HitProxy)
									{

										if(HitProxy->IsA(HLinkedObjProxy::StaticGetType()))
										{
											SelObject = ((HLinkedObjProxy*)HitProxy)->Obj;
										}
										// Special case for the little resizer triangles in the bottom right corner of comment frames
										else if(HitProxy->IsA(HLinkedObjProxySpecial::StaticGetType()))
										{
											SelObject = ((HLinkedObjProxySpecial*)HitProxy)->Obj;

											if(!SelObject->IsA(USequenceFrame::StaticClass()))
											{
												SelObject = NULL;
											}
										}

										if(SelObject)
										{
											// Don't want to call AddToSelection here because that might invalidate the display and we'll crash.
											NewSelection.AddUniqueItem( SelObject );
										}
									}
								}
							}

							TestStartY += ViewY;
							TestSizeY = Min(ViewY, appTrunc(BoxMaxY - TestStartY));
						}

						TestStartX += ViewX;
						TestSizeX = Min(ViewX, appTrunc(BoxMaxX - TestStartX));
					}

		
					// restore the original viewport settings
					Origin2D.X = SavedOrigin2D.X;
					Origin2D.Y = SavedOrigin2D.Y;

					// If shift is down, don't empty, just add to selection.
					if(!bShiftDown)
					{
						EdInterface->EmptySelection();
					}
					
					// Iterate over array adding each to selection.
					for(INT i=0; i<NewSelection.Num(); i++)
					{
						EdInterface->AddToSelection( NewSelection(i) );
					}

					EdInterface->UpdatePropertyWindow();
				}
				else
				{
					HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);

					// If mouse didn't really move since last time, and we released over empty space, deselect everything.
					if( !HitResult && DistanceDragged < 4 )
					{
						NewX = HitX;
						NewY = HitY;
						const UBOOL bDoDeselect = EdInterface->ClickOnBackground() && !bCtrlDown;
						if(bDoDeselect && LastEvent != IE_DoubleClick)
						{
							EdInterface->EmptySelection();
							EdInterface->UpdatePropertyWindow();
						}
					}
					else if(bCtrlDown)
					{
						if(DistanceDragged < 4)
						{
							if( HitResult && HitResult->IsA(HLinkedObjProxy::StaticGetType()) )
							{
								UObject* Obj = ((HLinkedObjProxy*)HitResult)->Obj;
								UBOOL bAlreadySelected = EdInterface->IsInSelection(Obj);

								if(!bAlreadySelected)
								{
									EdInterface->AddToSelection(Obj);
								}
								else
								{
									EdInterface->RemoveFromSelection(Obj);
								}

								EdInterface->UpdatePropertyWindow();
							}
						}
						else
						{
							EdInterface->PositionSelectedObjects();
						}
					} 
				}

				if(bTransactionBegun)
				{
					EdInterface->EndTransactionOnSelected();
					bTransactionBegun = false;
				}

				bMouseDown = false;
				bMakingLine = false;
				bSpecialDrag = false;
				bBoxSelecting = false;

				Viewport->LockMouseToWindow(false);
				Viewport->Invalidate();
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
				NewX = Viewport->GetMouseX();
				NewY = Viewport->GetMouseY();
				DeltaXFraction = 0.0f;
				DeltaYFraction = 0.0f;
				DistanceDragged = 0;
			}
			break;

		case IE_Released:
			{
				Viewport->Invalidate();

				if(bMakingLine || Viewport->KeyState(KEY_LeftMouseButton))
					break;

				INT HitX = Viewport->GetMouseX();
				INT HitY = Viewport->GetMouseY();

				// If right clicked and dragged - don't pop up menu. Have to click and release in roughly the same spot.
				if( Abs(HitX - NewX) + Abs(HitY - NewY) > 4 || DistanceDragged > 4)
					break;

				HHitProxy*	HitResult = Viewport->GetHitProxy(HitX,HitY);

				wxMenu* menu = NULL;
				if(!HitResult)
				{
					EdInterface->OpenNewObjectMenu();
				}
				else
				{
					if( HitResult->IsA(HLinkedObjConnectorProxy::StaticGetType()) )
					{
						HLinkedObjConnectorProxy* ConnProxy = (HLinkedObjConnectorProxy*)HitResult;

						// First select the connector and deselect any objects.
						EdInterface->SetSelectedConnector( ConnProxy->Connector );
						EdInterface->EmptySelection();
						EdInterface->UpdatePropertyWindow();
						Viewport->Invalidate();

						// Then open connector options menu.
						EdInterface->OpenConnectorOptionsMenu();
					}
					else if( HitResult->IsA(HLinkedObjProxy::StaticGetType()) )
					{
						// When right clicking on an unselected object, select it only before opening menu.
						UObject* Obj = ((HLinkedObjProxy*)HitResult)->Obj;

						if( !EdInterface->IsInSelection(Obj) )
						{
							EdInterface->EmptySelection();
							EdInterface->AddToSelection(Obj);
							EdInterface->UpdatePropertyWindow();
							Viewport->Invalidate();
						}
					
						EdInterface->OpenObjectOptionsMenu();
					}
				}
			}
			break;
		}
	}
	else if ( (Key == KEY_MouseScrollDown || Key == KEY_MouseScrollUp) && Event == IE_Pressed )
	{
		// Mousewheel up/down zooms in/out.
		const FLOAT DeltaZoom = (Key == KEY_MouseScrollDown ? -LinkedObjectEditor_ZoomIncrement : LinkedObjectEditor_ZoomIncrement );

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

			EdInterface->ViewPosChanged();
			Viewport->Invalidate();
		}
	}

	EdInterface->EdHandleKeyInput(Viewport, Key, Event);

	LastEvent = Event;

	// Hide and lock mouse cursor if we're capturing mouse input.
	// But don't hide mouse cursor if drawing lines and other special cases.
	UBOOL bCanLockMouse = (Key == KEY_LeftMouseButton) || (Key == KEY_RightMouseButton);
	if ( Viewport->HasMouseCapture() && bCanLockMouse )
	{
		Viewport->LockMouseToWindow( TRUE );
		UBOOL bHideCursor = !bMakingLine && !bBoxSelecting && !bSpecialDrag && !(bCtrlDown && EdInterface->HaveObjectsSelected()) && bAllowScroll;
		Viewport->ShowCursor( !bHideCursor );
	}

	// Handle viewport screenshot.
	InputTakeScreenshot( Viewport, Key, Event );

	return TRUE;
}

void FLinkedObjViewportClient::MouseMove(FViewport* Viewport, INT X, INT Y)
{
#if !USE_NEW_MOUSECAPTURE
	UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);
	UBOOL bShiftDown = Viewport->KeyState(KEY_LeftShift) || Viewport->KeyState(KEY_RightShift);
#endif

	INT DeltaX = X - OldMouseX;
	INT DeltaY = Y - OldMouseY;
	OldMouseX = X;
	OldMouseY = Y;

#if !USE_NEW_MOUSECAPTURE
	if(bMouseDown)
	{
		DistanceDragged += ( Abs<INT>(DeltaX) + Abs<INT>(DeltaY) );
	}
	if(bMouseDown)
	{
		if(bMakingLine)
		{
			NewX = X;
			NewY = Y;
		}
		else if(bBoxSelecting)
		{
			BoxEndX = X + (BoxOrigin2D.X-Origin2D.X);
			BoxEndY = Y + (BoxOrigin2D.Y-Origin2D.Y);
		}
		else if(bSpecialDrag)
		{
			FIntPoint MousePos( (X - Origin2D.X)/Zoom2D, (Y - Origin2D.Y)/Zoom2D );

			EdInterface->SpecialDrag( DeltaX * (1.f/Zoom2D), DeltaY * (1.f/Zoom2D), MousePos.X, MousePos.Y, SpecialIndex );
		}
		else if( bCtrlDown && EdInterface->HaveObjectsSelected() )
		{
			EdInterface->MoveSelectedObjects( DeltaX * (1.f/Zoom2D), DeltaY * (1.f/Zoom2D) );

			// If haven't started a transaction, and moving some stuff, and have moved mouse far enough, start transaction now.
			if(!bTransactionBegun && DistanceDragged > 4)
			{
				EdInterface->BeginTransactionOnSelected();
				bTransactionBegun = true;
			}
		}

		Viewport->InvalidateDisplay();
	}
#endif

	// Do mouse-over stuff (if mouse button is not held).
	OnMouseOver( X, Y );
}

/** Handle mouse over events */
void FLinkedObjViewportClient::OnMouseOver( INT X, INT Y )
{
	// Do mouse-over stuff (if mouse button is not held).
	UObject *NewMouseOverObject = NULL;
	INT NewMouseOverConnType = -1;
	INT NewMouseOverConnIndex = INDEX_NONE;
	HHitProxy*	HitResult = NULL;

	if(!bMouseDown || bMakingLine)
	{
		HitResult = Viewport->GetHitProxy(X,Y);
	}

	if( HitResult )
	{
		if( HitResult->IsA(HLinkedObjProxy::StaticGetType()) )
		{
			NewMouseOverObject = ((HLinkedObjProxy*)HitResult)->Obj;
		}
		else if( HitResult->IsA(HLinkedObjConnectorProxy::StaticGetType()) )
		{
			NewMouseOverObject = ((HLinkedObjConnectorProxy*)HitResult)->Connector.ConnObj;
			NewMouseOverConnType = ((HLinkedObjConnectorProxy*)HitResult)->Connector.ConnType;
			NewMouseOverConnIndex = ((HLinkedObjConnectorProxy*)HitResult)->Connector.ConnIndex;

			if( !EdInterface->ShouldHighlightConnector(((HLinkedObjConnectorProxy*)HitResult)->Connector) )
			{
				NewMouseOverConnType = -1;
				NewMouseOverConnIndex = INDEX_NONE;
			}
		}
		else if (HitResult->IsA(HLinkedObjLineProxy::StaticGetType()) && !bMakingLine)		// don't mouse-over lines when already creating a line
		{
			HLinkedObjLineProxy *LineProxy = (HLinkedObjLineProxy*)HitResult;
			NewMouseOverObject = LineProxy->Src.ConnObj;
			NewMouseOverConnType = LineProxy->Src.ConnType;
			NewMouseOverConnIndex = LineProxy->Src.ConnIndex;
		}

	}

	if(	NewMouseOverObject != MouseOverObject || 
		NewMouseOverConnType != MouseOverConnType ||
		NewMouseOverConnIndex != MouseOverConnIndex )
	{
		MouseOverObject = NewMouseOverObject;
		MouseOverConnType = NewMouseOverConnType;
		MouseOverConnIndex = NewMouseOverConnIndex;
		MouseOverTime = appSeconds();

		Viewport->InvalidateDisplay();
		EdInterface->OnMouseOver(MouseOverObject);
	}
}


UBOOL FLinkedObjViewportClient::InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad)
{
#if USE_NEW_MOUSECAPTURE
	UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);
	UBOOL bShiftDown = Viewport->KeyState(KEY_LeftShift) || Viewport->KeyState(KEY_RightShift);

	// DeviceDelta is not constricted to mouse locks
	FLOAT DeviceDeltaX = (Key == KEY_MouseX) ? Delta : 0;
	FLOAT DeviceDeltaY = (Key == KEY_MouseY) ? -Delta : 0;

	// Mouse variables represent the actual (potentially constrained) location of the mouse
	INT MouseX = Viewport->GetMouseX();	
	INT MouseY = Viewport->GetMouseY();
	INT MouseDeltaX = MouseX - OldMouseX;
	INT MouseDeltaY = MouseY - OldMouseY;
	OldMouseX = MouseX;
	OldMouseY = MouseY;

	// Accumulate delta fractions, since these will get dropped when truncated to INT.
	DeltaXFraction += MouseDeltaX * (1.f/Zoom2D) - INT(MouseDeltaX * (1.f/Zoom2D));
	DeltaYFraction += MouseDeltaY * (1.f/Zoom2D) - INT(MouseDeltaY * (1.f/Zoom2D));
	INT DeltaXAdd = INT(DeltaXFraction);
	INT DeltaYAdd = INT(DeltaYFraction);
	DeltaXFraction -= DeltaXAdd;
	DeltaYFraction -= DeltaYAdd;

	UBOOL bLeftMouseButtonDown = Viewport->KeyState(KEY_LeftMouseButton);
	UBOOL bRightMouseButtonDown = Viewport->KeyState(KEY_RightMouseButton);

	if( Key == KEY_MouseX || Key == KEY_MouseY )
	{
		DistanceDragged += Abs(MouseDeltaX) + Abs(MouseDeltaY);
	}

	// If holding both buttons, we are zooming.
	if(bLeftMouseButtonDown && bRightMouseButtonDown)
	{
		if(Key == KEY_MouseY)
		{
			const FLOAT ZoomDelta = -Zoom2D * Delta * LinkedObjectEditor_ZoomSpeed;

			const FLOAT ViewCenterX = ((((FLOAT)Viewport->GetSizeX()) * 0.5f) - (FLOAT)Origin2D.X)/Zoom2D;
			const FLOAT ViewCenterY = ((((FLOAT)Viewport->GetSizeY()) * 0.5f) - (FLOAT)Origin2D.Y)/Zoom2D;

			Zoom2D = Clamp<FLOAT>(Zoom2D+ZoomDelta,MinZoom2D,MaxZoom2D);

			// We have a 'notch' around 1.f to make it easy to get back to normal zoom factor.
			if( Abs(Zoom2D - 1.f) < LinkedObjectEditor_ZoomNotchThresh )
			{
				Zoom2D = 1.f;
			}

			const FLOAT DrawOriginX = ViewCenterX - ((Viewport->GetSizeX()*0.5f)/Zoom2D);
			const FLOAT DrawOriginY = ViewCenterY - ((Viewport->GetSizeY()*0.5f)/Zoom2D);

			Origin2D.X = -appRound(DrawOriginX * Zoom2D);
			Origin2D.Y = -appRound(DrawOriginY * Zoom2D);

			EdInterface->ViewPosChanged();
			Viewport->Invalidate();
		}
	}
	else if(bLeftMouseButtonDown || bRightMouseButtonDown)
	{
		UBOOL bInvalidate = FALSE;
		if(bMakingLine)
		{
			NewX = MouseX;
			NewY = MouseY;
			bInvalidate = TRUE;
		}
		else if(bBoxSelecting)
		{
			BoxEndX = MouseX + (BoxOrigin2D.X - Origin2D.X);
			BoxEndY = MouseY + (BoxOrigin2D.Y - Origin2D.Y);
			bInvalidate = TRUE;
		}
		else if(bSpecialDrag)
		{
			FIntPoint MousePos( (MouseX - Origin2D.X)/Zoom2D, (MouseY - Origin2D.Y)/Zoom2D );
			EdInterface->SpecialDrag( MouseDeltaX * (1.f/Zoom2D) + DeltaXAdd, MouseDeltaY * (1.f/Zoom2D) + DeltaYAdd, MousePos.X, MousePos.Y, SpecialIndex );
			bInvalidate = TRUE;
		}
		else if( bCtrlDown && EdInterface->HaveObjectsSelected() )
		{
			EdInterface->MoveSelectedObjects( MouseDeltaX * (1.f/Zoom2D) + DeltaXAdd, MouseDeltaY * (1.f/Zoom2D) + DeltaYAdd );

			// If haven't started a transaction, and moving some stuff, and have moved mouse far enough, start transaction now.
			if(!bTransactionBegun && DistanceDragged > 4)
			{
				EdInterface->BeginTransactionOnSelected();
				bTransactionBegun = true;
			}
			bInvalidate = TRUE;
		}
		else if(bAllowScroll && Viewport->HasMouseCapture())
		{
			if ( !bInvertMousePan )
			{
				DeviceDeltaX = -DeviceDeltaX;
				DeviceDeltaY = -DeviceDeltaY;
			}

			Origin2D.X += DeviceDeltaX;
			Origin2D.Y += DeviceDeltaY;
			EdInterface->ViewPosChanged();
			bInvalidate = TRUE;
		}

		OnMouseOver( MouseX, MouseY );

		if ( bInvalidate )
		{
			Viewport->InvalidateDisplay();
		}
	}
#else
	if( Key == KEY_MouseX || Key == KEY_MouseY )
	{
		DistanceDragged += Abs(Delta);
	}

	// If holding both buttons, we are zooming.
	if(Viewport->KeyState(KEY_RightMouseButton) && Viewport->KeyState(KEY_LeftMouseButton))
	{
		if(Key == KEY_MouseY)
		{
			const FLOAT ZoomDelta = -Zoom2D * Delta * LinkedObjectEditor_ZoomSpeed;

			const FLOAT ViewCenterX = ((((FLOAT)Viewport->GetSizeX()) * 0.5f) - (FLOAT)Origin2D.X)/Zoom2D;
			const FLOAT ViewCenterY = ((((FLOAT)Viewport->GetSizeY()) * 0.5f) - (FLOAT)Origin2D.Y)/Zoom2D;

			Zoom2D = Clamp<FLOAT>(Zoom2D+ZoomDelta,MinZoom2D,MaxZoom2D);

			// We have a 'notch' around 1.f to make it easy to get back to normal zoom factor.
			if( Abs(Zoom2D - 1.f) < LinkedObjectEditor_ZoomNotchThresh )
			{
				Zoom2D = 1.f;
			}

			const FLOAT DrawOriginX = ViewCenterX - ((Viewport->GetSizeX()*0.5f)/Zoom2D);
			const FLOAT DrawOriginY = ViewCenterY - ((Viewport->GetSizeY()*0.5f)/Zoom2D);

			Origin2D.X = -appRound(DrawOriginX * Zoom2D);
			Origin2D.Y = -appRound(DrawOriginY * Zoom2D);

			EdInterface->ViewPosChanged();
			Viewport->Invalidate();
		}
	}
	// If not, we are panning.
	else
	{
		if(bAllowScroll)
		{
			INT DeltaX = (Key == KEY_MouseX) ? -Delta : 0;
			INT DeltaY = (Key == KEY_MouseY) ? Delta : 0;

			if ( bInvertMousePan )
			{
				DeltaX = -DeltaX;
				DeltaY = -DeltaY;
			}

			Origin2D.X += DeltaX;
			Origin2D.Y += DeltaY;
			EdInterface->ViewPosChanged();
			Viewport->InvalidateDisplay();
		}
	}
#endif

	return TRUE;
}

EMouseCursor FLinkedObjViewportClient::GetCursor(FViewport* Viewport,INT X,INT Y)
{
	return MC_Arrow;
}

/** 
 *	See if cursor is in 'scroll' region around the edge, and if so, scroll the view automatically. 
 *	Returns the distance that the view was moved.
 */
FIntPoint FLinkedObjViewportClient::DoScrollBorder(FLOAT DeltaTime)
{
	FIntPoint Result( 0, 0 );

	if (bAllowScroll)
	{
		const INT PosX = Viewport->GetMouseX();
		const INT PosY = Viewport->GetMouseY();
		const INT SizeX = Viewport->GetSizeX();
		const INT SizeY = Viewport->GetSizeY();

		DeltaTime = Clamp(DeltaTime, 0.01f, 1.0f);

		if(PosX < LinkedObjectEditor_ScrollBorderSize)
		{
			ScrollAccum.X += (1.f - ((FLOAT)PosX/(FLOAT)LinkedObjectEditor_ScrollBorderSize)) * LinkedObjectEditor_ScrollBorderSpeed * DeltaTime;
		}
		else if(PosX > SizeX - LinkedObjectEditor_ScrollBorderSize)
		{
			ScrollAccum.X -= ((FLOAT)(PosX - (SizeX - LinkedObjectEditor_ScrollBorderSize))/(FLOAT)LinkedObjectEditor_ScrollBorderSize) * LinkedObjectEditor_ScrollBorderSpeed * DeltaTime;
		}
		else
		{
			ScrollAccum.X = 0.f;
		}

		FLOAT ScrollY = 0.f;
		if(PosY < LinkedObjectEditor_ScrollBorderSize)
		{
			ScrollAccum.Y += (1.f - ((FLOAT)PosY/(FLOAT)LinkedObjectEditor_ScrollBorderSize)) * LinkedObjectEditor_ScrollBorderSpeed * DeltaTime;
		}
		else if(PosY > SizeY - LinkedObjectEditor_ScrollBorderSize)
		{
			ScrollAccum.Y -= ((FLOAT)(PosY - (SizeY - LinkedObjectEditor_ScrollBorderSize))/(FLOAT)LinkedObjectEditor_ScrollBorderSize) * LinkedObjectEditor_ScrollBorderSpeed * DeltaTime;
		}
		else
		{
			ScrollAccum.Y = 0.f;
		}

		// Apply integer part of ScrollAccum to origin, and save the rest.
		const INT MoveX = appFloor(ScrollAccum.X);
		Origin2D.X += MoveX;
		ScrollAccum.X -= MoveX;

		const INT MoveY = appFloor(ScrollAccum.Y);
		Origin2D.Y += MoveY;
		ScrollAccum.Y -= MoveY;

		// Update the box selection if necessary
		if (bBoxSelecting)
		{
			BoxEndX += MoveX;
			BoxEndY += MoveY;
		}

		// If view has changed, notify the app and redraw the viewport.
		if( Abs<INT>(MoveX) > 0 || Abs<INT>(MoveY) > 0 )
		{
			EdInterface->ViewPosChanged();
			Viewport->Invalidate();
		}
		Result = FIntPoint(MoveX, MoveY);
	}

	return Result;
}

/**
 * Sets whether or not the viewport should be invalidated in Tick().
 */
void FLinkedObjViewportClient::SetRedrawInTick(UBOOL bInAlwaysDrawInTick)
{
	bAlwaysDrawInTick = bInAlwaysDrawInTick;
}

void FLinkedObjViewportClient::Tick(FLOAT DeltaSeconds)
{
	FEditorLevelViewportClient::Tick(DeltaSeconds);

	// Auto-scroll display if moving/drawing etc. and near edge.
	UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);
	if(	bMouseDown )		
	{
		// If holding both buttons, we are zooming.
		if(Viewport->KeyState(KEY_RightMouseButton) && Viewport->KeyState(KEY_LeftMouseButton))
		{
		}
		else if(bMakingLine || bBoxSelecting)
		{
			DoScrollBorder(DeltaSeconds);
		}
		else if(bSpecialDrag)
		{
			FIntPoint Delta = DoScrollBorder(DeltaSeconds);
			if(Delta.Size() > 0)
			{
				EdInterface->SpecialDrag( -Delta.X * (1.f/Zoom2D), -Delta.Y * (1.f/Zoom2D), 0, 0, SpecialIndex ); // TODO fix mouse position in this case.
			}
		}
		else if(bCtrlDown && EdInterface->HaveObjectsSelected())
		{
			FIntPoint Delta = DoScrollBorder(DeltaSeconds);

			// In the case of dragging boxes around, we move them as well when dragging at the edge of the screen.
			EdInterface->MoveSelectedObjects( -Delta.X * (1.f/Zoom2D), -Delta.Y * (1.f/Zoom2D) );

			DistanceDragged += ( Abs<INT>(Delta.X) + Abs<INT>(Delta.Y) );

			if(!bTransactionBegun && DistanceDragged > 4)
			{
				EdInterface->BeginTransactionOnSelected();
				bTransactionBegun = true;
			}
		}
	}

	if ( bAlwaysDrawInTick )
	{
		Viewport->InvalidateDisplay();
	}
}

/*-----------------------------------------------------------------------------
WxLinkedObjVCHolder
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxLinkedObjVCHolder, wxWindow )
EVT_SIZE( WxLinkedObjVCHolder::OnSize )
END_EVENT_TABLE()

WxLinkedObjVCHolder::WxLinkedObjVCHolder( wxWindow* InParent, wxWindowID InID, FLinkedObjEdNotifyInterface* InEdInterface )
: wxWindow( InParent, InID )
{
	LinkedObjVC = new FLinkedObjViewportClient( InEdInterface );
	LinkedObjVC->Viewport = GEngine->Client->CreateWindowChildViewport(LinkedObjVC, (HWND)GetHandle());
	LinkedObjVC->Viewport->CaptureJoystickInput(false);
}

WxLinkedObjVCHolder::~WxLinkedObjVCHolder()
{
	GEngine->Client->CloseViewport(LinkedObjVC->Viewport);
	LinkedObjVC->Viewport = NULL;
	delete LinkedObjVC;
}

void WxLinkedObjVCHolder::OnSize( wxSizeEvent& In )
{
	wxRect rc = GetClientRect();

	::MoveWindow( (HWND)LinkedObjVC->Viewport->GetWindow(), 0, 0, rc.GetWidth(), rc.GetHeight(), 1 );
}

/*-----------------------------------------------------------------------------
WxLinkedObjEd
-----------------------------------------------------------------------------*/


BEGIN_EVENT_TABLE( WxLinkedObjEd, WxTrackableFrame )
EVT_SIZE( WxLinkedObjEd::OnSize )
END_EVENT_TABLE()


WxLinkedObjEd::WxLinkedObjEd( wxWindow* InParent, wxWindowID InID, const TCHAR* InWinName ) : 
WxTrackableFrame( InParent, InID, InWinName, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR ),
FDockingParent(this)
, PropertyWindow(NULL), GraphWindow(NULL), LinkedObjVC(NULL), TreeControl(NULL), TreeImages(NULL), BackgroundTexture(NULL)
{
	bDrawCurves = false;

	WinNameString = FString(InWinName);

	SetTitle( *WinNameString );
	SetName(InWinName);
}

/**
 * This function is called when the window has been selected from within the ctrl + tab dialog.
 */
void WxLinkedObjEd::OnSelected()
{
	this->Raise();
}

void WxLinkedObjEd::CreateControls( UBOOL bTreeControl )
{
	GraphWindow = new WxLinkedObjVCHolder( this, -1, this );
	LinkedObjVC = GraphWindow->LinkedObjVC;

	if(bTreeControl)
	{
		PropertyWindow = new WxPropertyWindow;
		PropertyWindow->Create( this, this );
		AddDockingWindow(PropertyWindow, FDockingParent::DH_Bottom, *LocalizeUnrealEd("Properties"));

		CreateTreeControl(this);
		check(TreeControl);
		AddDockingWindow(TreeControl, FDockingParent::DH_Bottom, *LocalizeUnrealEd("Sequences"));
	}
	else
	{
		PropertyWindow = new WxPropertyWindow;
		PropertyWindow->Create( this, this );
		AddDockingWindow(PropertyWindow, FDockingParent::DH_Bottom, *LocalizeUnrealEd("Properties"));
	}

	// Add main docking pane.
	AddDockingWindow( GraphWindow, FDockingParent::DH_None, NULL );

	// Try to load a existing layout for the docking windows.
	LoadDockingLayout();
}

WxLinkedObjEd::~WxLinkedObjEd()
{
}

/** 
* Saves Window Properties
*/ 
void WxLinkedObjEd::SaveProperties()
{
	FString KeyName;
	const TCHAR* ConfigName = GetConfigName();

	// Save Window Position and Size
	KeyName = FString::Printf(TEXT("%s_PosSize"), ConfigName);
	FWindowUtil::SavePosSize(KeyName, this);

	// Save Docking Layout
	SaveDockingLayout();
}

/**
* Loads Window Properties
*/
void WxLinkedObjEd::LoadProperties()
{
	FString KeyName;
	const TCHAR* ConfigName = GetConfigName();

	// Load Window Position and Size
	KeyName = FString::Printf(TEXT("%s_PosSize"), ConfigName);
	FWindowUtil::LoadPosSize(KeyName, this, 256, 256, 1024, 768);
	Refresh();

	Layout();
}

/**
 * Creates the tree control for this linked object editor.  Only called if TRUE is specified for bTreeControl
 * in the constructor.
 *
 * @param	TreeParent	the window that should be the parent for the tree control
 */
void WxLinkedObjEd::CreateTreeControl( wxWindow* TreeParent )
{
	TreeImages = new wxImageList( 16, 15 );
	TreeControl = new wxTreeCtrl( TreeParent, IDM_LINKEDOBJED_TREE, wxDefaultPosition, wxSize(100,100), wxTR_HAS_BUTTONS, wxDefaultValidator, TEXT("TreeControl") );
	TreeControl->AssignImageList(TreeImages);
}

/**
 * Used to serialize any UObjects contained that need to be to kept around.
 *
 * @param Ar The archive to serialize with
 */
void WxLinkedObjEd::Serialize(FArchive& Ar)
{
	// Need to call Serialize(Ar) on super class in case we ever move inheritance from FSerializeObject up the chain.
	if( !Ar.IsLoading() && !Ar.IsSaving() )
	{
		Ar << BackgroundTexture;
	}
}

void WxLinkedObjEd::OnSize( wxSizeEvent& In )
{
	if ( LinkedObjVC && LinkedObjVC->Viewport )
	{
		LinkedObjVC->Viewport->Invalidate();
	}
	In.Skip();
}

void WxLinkedObjEd::RefreshViewport()
{
	LinkedObjVC->Viewport->Invalidate();
}

void WxLinkedObjEd::DrawObjects(FViewport* Viewport, FCanvas* Canvas)
{
	// draw the background texture if specified
	if (BackgroundTexture != NULL)
	{
		Canvas->PushAbsoluteTransform(FMatrix::Identity);

		Clear(Canvas, FColor(161,161,161) );

		const INT ViewWidth = GraphWindow->GetSize().x;
		const INT ViewHeight = GraphWindow->GetSize().y;

		// draw the texture to the side, stretched vertically
		DrawTile(Canvas, ViewWidth - BackgroundTexture->SizeX, 0,
					  BackgroundTexture->SizeX, ViewHeight,
					  0.f, 0.f,
					  1.f, 1.f,
					  FLinearColor::White,
					  BackgroundTexture->Resource );

		// stretch the left part of the texture to fill the remaining gap
		if (ViewWidth > BackgroundTexture->SizeX)
		{
			DrawTile(Canvas, 0, 0,
						  ViewWidth - BackgroundTexture->SizeX, ViewHeight,
						  0.f, 0.f,
						  0.1f, 0.1f,
						  FLinearColor::White,
						  BackgroundTexture->Resource );
		}

		Canvas->PopTransform();
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// Properties window NotifyHook stuff

void WxLinkedObjEd::NotifyDestroy( void* Src )
{

}

void WxLinkedObjEd::NotifyPreChange( void* Src, UProperty* PropertyAboutToChange )
{
	GEditor->BeginTransaction( *LocalizeUnrealEd("EditLinkedObj") );

	for ( WxPropertyWindow::TObjectIterator Itor( PropertyWindow->ObjectIterator() ) ; Itor ; ++Itor )
	{
		(*Itor)->Modify();
	}
}

void WxLinkedObjEd::NotifyPostChange( void* Src, UProperty* PropertyThatChanged )
{
	GEditor->EndTransaction();

	RefreshViewport();
}

void WxLinkedObjEd::NotifyExec( void* Src, const TCHAR* Cmd )
{
	GUnrealEd->NotifyExec(Src, Cmd);
}

//////////////////////////////////////////////////////////////////////////
// FDockingParent Interface
//////////////////////////////////////////////////////////////////////////

/**
 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
 *  @return A string representing a name to use for this docking parent.
 */
const TCHAR* WxLinkedObjEd::GetDockingParentName() const
{
	return GetConfigName();
}

/**
 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
 */
const INT WxLinkedObjEd::GetDockingParentVersion() const
{
	return 0;
}
