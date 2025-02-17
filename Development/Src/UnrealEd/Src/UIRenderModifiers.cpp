/*=============================================================================
	UIRenderModifiers.cpp: Class implementations for UI editor render modifiers
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/* ==========================================================================================================
	Includes
========================================================================================================== */
#include "UnrealEd.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineUIPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnLinkedObjDrawUtils.h"

#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UIWidgetTool.h"
#include "UIDragTools.h"
#include "UIRenderModifiers.h"

/* ==========================================================================================================
	"Selected Widget" render modifier:
	Renders a non-interactive outline around the selected widget, as well as the interactive docking handles
========================================================================================================== */
FRenderModifier_SelectedWidgetsOutline::FRenderModifier_SelectedWidgetsOutline( WxUIEditorBase* InEditor )
: FUIEditor_RenderModifier(InEditor), SelectedWidget(NULL), SelectedDockHandle(UIFACE_MAX), bRenderDockHandles(TRUE)
{
}

void FRenderModifier_SelectedWidgetsOutline::Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	TArray<UUIObject*> SelectedWidgets;
	UIEditor->GetSelectedWidgets(SelectedWidgets);

	const INT NumWidgets = SelectedWidgets.Num();

	// Loop through all selected widgets and render a selection box around them,
	// and their docking handles.
	for(INT WidgetIdx = 0; WidgetIdx < NumWidgets; WidgetIdx++)
	{
		UUIObject* Widget = SelectedWidgets(WidgetIdx);

		Canvas->PushDepthSortKey(appTrunc(Widget->GetZDepth()));
		Canvas->PushRelativeTransform(Widget->GenerateTransformMatrix());
		
		if ( UIEditor->EditorOptions->bRenderSelectionOutline )
		{
			// Render a box outline around this widget because it is selected.
			FColor BorderColor(64,64,255);

			FVector Expand(2 / ObjectVC->Zoom2D,2 / ObjectVC->Zoom2D,2 / ObjectVC->Zoom2D);
			FVector Start, End;

			FLOAT MinX = 0.0f, MaxX = 0.0f, MinY = 0.0f, MaxY = 0.0f;
			Widget->GetPositionExtents( MinX, MaxX, MinY, MaxY, FALSE, TRUE );

			// Top Line.
			DrawTile(Canvas,MinX - Expand.X, MinY - Expand.Y, Expand.X * 2 + MaxX - MinX, Expand.Y * 2,
				0.0f, 0.0f, 0.0f, 0.0f, BorderColor);

			// Bottom Line.
			DrawTile(Canvas,MinX - Expand.X, MaxY - Expand.Y, Expand.X * 2 + MaxX - MinX, Expand.Y * 2,
				0.0f, 0.0f, 0.0f, 0.0f, BorderColor);

			// Left Line.
			DrawTile(Canvas,MinX - Expand.X, MinY - Expand.Y, Expand.X * 2, Expand.Y * 2 + MaxY - MinY,
				0.0f, 0.0f, 0.0f, 0.0f, BorderColor);

			// Right Line.
			DrawTile(Canvas,MaxX - Expand.X, MinY - Expand.Y, Expand.X * 2, Expand.Y * 2 + MaxY - MinY,
				0.0f, 0.0f, 0.0f, 0.0f, BorderColor);
		}

		// Draw docking handles for this object.
		if ( bRenderDockHandles && UIEditor->EditorOptions->bShowDockHandles && !Widget->IsPrivateBehaviorSet( UCONST_PRIVATE_NotDockable ))
		{
			RenderDockHandle(Widget, UIFACE_Top,	ObjectVC, Canvas);
			RenderDockHandle(Widget, UIFACE_Bottom,	ObjectVC, Canvas);
			RenderDockHandle(Widget, UIFACE_Left,	ObjectVC, Canvas);
			RenderDockHandle(Widget, UIFACE_Right,	ObjectVC, Canvas);
		}
		Canvas->PopTransform();
		Canvas->PopDepthSortKey();
	}
}

void FRenderModifier_SelectedWidgetsOutline::SetSelectedWidget(UUIObject* InWidget)
{
	SelectedWidget = InWidget;
}

/** Sets the selected dock face for highlighting. */
void FRenderModifier_SelectedWidgetsOutline::SetSelectedDockFace(EUIWidgetFace InFace)
{
	// Only update if we are changing state.
	// If we changed to a blank face, then clear the tooltip. Otherwise, update the tooltip with some information
	// about the currently highlighted dock handle.
	if(SelectedDockHandle != InFace)
	{
		if(InFace != UIFACE_MAX && SelectedWidget != NULL)
		{
			FString DockString;
			FString TempString;
			TArray<FString> TempStringArray;

			UIEditor->GetDockHandleString(SelectedWidget, InFace,DockString);
			TempString = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_Source")), *DockString));
			TempStringArray.AddItem(TempString);

			const UUIObject* TargetWidget = SelectedWidget->DockTargets.GetDockTarget(InFace);
			const EUIWidgetFace TargetFace = SelectedWidget->DockTargets.GetDockFace(InFace);

			UIEditor->GetDockHandleString(TargetWidget, TargetFace,DockString);
			TempString = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_Target")), *DockString));

			TempStringArray.AddItem(TempString);

			UIEditor->TargetTooltip->SetStringList(TempStringArray);
		}
		else
		{
			UIEditor->TargetTooltip->ClearStringList();
		}

		SelectedDockHandle = InFace;
	}
}

/** Clears the selected dock face and dock widget. */
void FRenderModifier_SelectedWidgetsOutline::ClearSelections()
{
	SelectedWidget = NULL;
	SelectedDockHandle = UIFACE_MAX;
	UIEditor->TargetTooltip->ClearStringList();
}

/**
* Method for serializing UObject references that must be kept around through GC's.
*
* @param Ar The archive to serialize with
*/
void FRenderModifier_SelectedWidgetsOutline::Serialize(FArchive& Ar)
{
	Ar << SelectedWidget;
}

/** 
* Renders a docking handle for the specified widget.
* 
* @param Widget	Widget to render the docking handle for.
* @param Face		Which face we are drawing the handle for.
* @param RI		Rendering interface to use for drawing the handle.
*/
void FRenderModifier_SelectedWidgetsOutline::RenderDockHandle( UUIObject* Widget, EUIWidgetFace InFace, FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	const FColor DockHandlerColor(255,128,0);
	const FColor DockHandlerHighlightColor(255,255,0);
	const FColor DockHandlerBorderColor(0,0,0);
	const FLOAT Zoom = ObjectVC->Zoom2D < 1.0f ? 1.0f : ObjectVC->Zoom2D;
	const FLOAT DockHandleRadius = FDockingConstants::HandleRadius / Zoom;
	const FLOAT DockHandleGap = FDockingConstants::HandleGap;
	const INT NumSides = FDockingConstants::HandleNumSides;

	// Get a position for this dock handle.
	FIntPoint DockPosition;
	FLOAT RenderBounds[4];
	Widget->GetPositionExtents( RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Right], RenderBounds[UIFACE_Top], RenderBounds[UIFACE_Bottom], FALSE, TRUE );

	GetHandlePosition(RenderBounds, InFace, DockHandleRadius, DockHandleGap, DockPosition);
	
	//Render the dock link for this object.
	RenderDockLink(Widget, InFace, Canvas, DockPosition, DockHandleRadius, DockHandleGap);

	Canvas->SetHitProxy( new HUIDockHandleHitProxy(Widget, InFace) );
	{
		// Draw the dock handle circle, if the mouse is over the dock handle, then draw it in a different color.
		const UBOOL bHandleHighlighted = (SelectedWidget == Widget && SelectedDockHandle == InFace);

		FLinkedObjDrawUtils::DrawNGon(Canvas, DockPosition, DockHandlerBorderColor, NumSides, DockHandleRadius + (1 / Zoom));

		if(bHandleHighlighted == TRUE)
		{
			FLinkedObjDrawUtils::DrawNGon(Canvas, DockPosition, DockHandlerHighlightColor, NumSides, DockHandleRadius);
		}
		else
		{
			FLinkedObjDrawUtils::DrawNGon(Canvas, DockPosition, DockHandlerColor, NumSides, DockHandleRadius);
		}

		//Draw a X on top of the handle if the widget is docked.
		const UBOOL bWidgetDocked = (Widget->DockTargets.IsDocked(InFace));
		if( bWidgetDocked == TRUE )
		{
			UTexture2D* tex = GApp->MaterialEditor_Delete;
			DrawTile(Canvas,DockPosition.X - DockHandleRadius / 2,DockPosition.Y - DockHandleRadius / 2, DockHandleRadius, DockHandleRadius, 0.0f,0.0f,1.0f,1.0f, FLinearColor::White, tex->Resource );
		}
	}
	Canvas->SetHitProxy( NULL );
}


/** 
 * Generates a dock handle position for a object.
 * 
 * @param Widget				Widget we are getting a dock handle position for.
 * @param InFace				Which face we are getting a position for.
 * @param HandleRadius			Radius of the handle.
 * @param HandleGap				Gap between the handle and the object's edge.
 * @param OutPosition			Position of the dock handle.
 */
void FRenderModifier_SelectedWidgetsOutline::GetDockHandlePosition(UUIObject* Widget, EUIWidgetFace InFace, INT HandleRadius, INT HandleGap, FIntPoint& OutPosition )
{
	if(Widget == NULL)
	{
		if(UIEditor->OwnerScene != NULL)
		{
			GetDockHandlePosition(UIEditor->OwnerScene, InFace, HandleRadius, HandleGap, OutPosition);
		}
	}
	else
	{
		FLOAT RenderBounds[4];
		Widget->GetPositionExtents( RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Right], RenderBounds[UIFACE_Top], RenderBounds[UIFACE_Bottom], FALSE, TRUE );
		GetHandlePosition(RenderBounds, InFace, HandleRadius, HandleGap, OutPosition);
	}	
}

/** 
 * Generates a dock handle position for a scene.
 * 
 * @param Scene					Scene we are getting a dock handle position for.
 * @param InFace				Which face we are getting a position for.
 * @param HandleRadius			Radius of the handle.
 * @param HandleGap				Gap between the handle and the object's edge.
 * @param OutPosition			Position of the dock handle.
 */
void FRenderModifier_SelectedWidgetsOutline::GetDockHandlePosition(UUIScene* Scene, EUIWidgetFace InFace, INT HandleRadius, INT HandleGap, FIntPoint& OutPosition)
{
	FLOAT RenderBounds[4];

	for(INT FaceIdx=0; FaceIdx < 4; FaceIdx++)
	{
		RenderBounds[FaceIdx] = Scene->GetPosition((EUIWidgetFace)FaceIdx, EVALPOS_PixelViewport, TRUE);
	}
	
	GetHandlePosition(RenderBounds, InFace, HandleRadius, HandleGap, OutPosition);
}


/** 
 * Generates a handle position for a object.
 * 
 * @param RenderBounds	Render bounds of the object we are getting a dock handle for.
 * @param InFace		Which face we are getting a position for.
 * @param HandleRadius	Radius of the handle.
 * @param HandleGap		Gap between the handle and the object's edge.
 * @param OutPosition	Position of the dock handle.
 */
void FRenderModifier_SelectedWidgetsOutline::GetHandlePosition(const FLOAT* RenderBounds, EUIWidgetFace InFace, INT HandleRadius, INT HandleGap, FIntPoint& OutPosition)
{
	switch(InFace)
	{
	case UIFACE_Top:
		{
			OutPosition.X = (RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left]) / 2 + RenderBounds[UIFACE_Left];
			OutPosition.Y = RenderBounds[UIFACE_Top] + HandleRadius + HandleGap;

			// Make sure that we can clearly see all of the dock handles for a selected widget.
			FLOAT Distance = Abs(RenderBounds[UIFACE_Top] - RenderBounds[UIFACE_Bottom]);

			const FLOAT DistThreshold = (HandleRadius*2.0f + HandleGap*2.0f) * 2.0f;
			if(Distance < DistThreshold)
			{
				OutPosition.Y -= (DistThreshold - Distance) / 2;
			}
		}

		break;

	case UIFACE_Bottom:
		{
			OutPosition.X = (RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left]) / 2 + RenderBounds[UIFACE_Left];
			OutPosition.Y = RenderBounds[UIFACE_Bottom] - HandleRadius - HandleGap;

			// Make sure that we can clearly see all of the dock handles for a selected widget.
			FLOAT Distance = Abs(RenderBounds[UIFACE_Top] - RenderBounds[UIFACE_Bottom]);
			const FLOAT DistThreshold = (HandleRadius*2.0f + HandleGap*2.0f) * 2.0f;
			if(Distance < DistThreshold)
			{
				OutPosition.Y += (DistThreshold - Distance) / 2;
			}
		}
		break;

	case UIFACE_Left:
		{
			OutPosition.X = RenderBounds[UIFACE_Left] + HandleRadius + HandleGap;
			OutPosition.Y = (RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top]) / 2 + RenderBounds[UIFACE_Top];

			// Make sure that we can clearly see all of the dock handles for a selected widget.
			FLOAT Distance = Abs(RenderBounds[UIFACE_Left] - RenderBounds[UIFACE_Right]);
			const FLOAT DistThreshold = (HandleRadius*2.0f + HandleGap*2.0f) * 2.0f;
			if(Distance < DistThreshold)
			{
				OutPosition.X -= (DistThreshold - Distance) / 2;
			}
		}

		break;

	case UIFACE_Right:
		{
			OutPosition.X = RenderBounds[UIFACE_Right] - HandleRadius - HandleGap;
			OutPosition.Y = (RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top]) / 2 + RenderBounds[UIFACE_Top];

			// Make sure that we can clearly see all of the dock handles for a selected widget.
			FLOAT Distance = Abs(RenderBounds[UIFACE_Left] - RenderBounds[UIFACE_Right]);
			const FLOAT DistThreshold = (HandleRadius*2.0f + HandleGap*2.0f) * 2.0f;
			if(Distance < DistThreshold)
			{
				OutPosition.X += (DistThreshold - Distance) / 2;
			}
		}
		break;

	default:
		break;
	}
}

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
void FRenderModifier_SelectedWidgetsOutline::RenderDockLink( UUIObject* Widget, EUIWidgetFace InFace, FCanvas* Canvas, const FVector& InStart, INT DockHandleRadius, INT DockHandleGap )
{
	// Make sure that we have a dock target for this face.
	const EUIWidgetFace TargetFace = Widget->DockTargets.GetDockFace(InFace);
	UUIObject* TargetWidget = Widget->DockTargets.GetDockTarget(InFace);

	if( Widget->DockTargets.IsDocked(InFace) )
	{
		// Calculate end position for the link and the end arrow direction.
		FIntPoint End;
		GetDockHandlePosition(TargetWidget, TargetFace, DockHandleRadius, DockHandleGap, End);

		FVector TransformedEndPoint = TargetWidget != NULL ? TargetWidget->Project(FVector(End.X, End.Y, 0.f)) : UIEditor->OwnerScene->Project(FVector(End.X, End.Y, 0.f));
		End.X = appRound(TransformedEndPoint.X);
		End.Y = appRound(TransformedEndPoint.Y);

		// we also need to transform the start point into screen space so that the spline will take the view perspective into account.
		FVector TransformedStartPoint = Widget->Project(InStart);

		// Calculate end arrow direction.
		const INT MinDist = 7;
		FVector2D StartDir(0,0);
		const FIntPoint Start(TransformedStartPoint.X, TransformedStartPoint.Y);
		const FIntPoint Diff(End-Start);
		FVector2D EndDir(0,0);
		FLOAT Tension = (Start-End).Size();

		switch(TargetFace)
		{
		case UIFACE_Bottom:
			EndDir[1] = 1;

			if(Abs<INT>(Diff.Y) < MinDist)
			{
				Tension *= 1.5f;
			}

			break;

		case UIFACE_Top:
			EndDir[1] = -1;

			if(Abs<INT>(Diff.Y) < MinDist)
			{
				Tension *= 1.5f;
			}

			break;

		case UIFACE_Right: 
			EndDir[0] = 1;

			if(Abs<INT>(Diff.X) < MinDist)
			{
				Tension *= 1.5f;
			}

			break;

		case UIFACE_Left:
			EndDir[0] = -1;

			if(Abs<INT>(Diff.X) < MinDist)
			{
				Tension *= 1.5f;
			}
			
			break;

		default:
			return;
		}

		/** @todo: Leave this code here please. - ASM
			switch(TargetFace)
			{
			case UIFACE_Bottom: case UIFACE_Top:
				if(Diff.Y > MinDist)
				{
					EndDir[1] = -1;
				}
				else if(Diff.Y < -MinDist)
				{
					EndDir[1] = 1;
				}
				else
				{
					if(Diff.X > MinDist)
					{
						EndDir[0] = -1;
					}
					else
					{
						EndDir[0] = 1;
					}
				}
				break;

			case UIFACE_Right: case UIFACE_Left:
				if(Diff.X > MinDist)
				{
					EndDir[0] = -1;
				}
				else if(Diff.X < -MinDist)
				{
					EndDir[0] = 1;
				}
				else
				{
					if(Diff.Y > MinDist)
					{
						EndDir[1] = -1;
					}
					else
					{
						EndDir[1] = 1;
					}
				}
				break;

			default:
				return;
			}
		*/


		// since we're in the render loop, the canvas's TranformStack already contains the widget's transform, but since we had
		// to transform the start and end points using Project (which also includes the widget's transform), we need to pull out
		// all widget transforms before rendering the spline.
		Canvas->PushAbsoluteTransform(FScaleMatrix(UIEditor->ObjectVC->Zoom2D) * FTranslationMatrix(FVector(UIEditor->ObjectVC->Origin2D,0)));

		// Finally render the spline.
		const FColor LineColor(0,255,255);
		FLinkedObjDrawUtils::DrawSpline(Canvas, Start, StartDir, End, -Tension * EndDir, LineColor, TRUE, FALSE);
		Canvas->PopTransform();
	}
}


/* ==========================================================================================================
	"Target Widget" render modifier - renders an outline and focus chain handles for widgets being moused over
	while a mouse tool is active, such as the focus chain or dock link mouse tools
========================================================================================================== */
/**
 * Constructor
 *
 * @param	InEditor	the editor window that holds the container to render the outline for
 */
FRenderModifier_TargetWidgetOutline::FRenderModifier_TargetWidgetOutline( WxUIEditorBase* InEditor )
: FRenderModifier_SelectedWidgetsOutline(InEditor)
{
	bRenderDockHandles = FALSE;
}

FRenderModifier_TargetWidgetOutline::~FRenderModifier_TargetWidgetOutline()
{
	FreeSortedDockHandleList();
}


/** 
 * Generates a list of all of the dock handles in the scene and sorts them by X position. 
 *
 * @param SourceWidget	The widget we are connecting a dock link from.
 * @param SourceFace	The face we are connecting from.
 */
void FRenderModifier_TargetWidgetOutline::GenerateSortedDockHandleList(UUIObject* SourceWidget, EUIWidgetFace SourceFace)
{
	FreeSortedDockHandleList();

	// Add dock handles for the scene.
	{
		UUIScene* Scene = UIEditor->OwnerScene;
		FWidgetDockHandles* WidgetDockHandles = new FWidgetDockHandles;

		WidgetDockHandles->bVisible = FALSE;
		WidgetDockHandles->Widget = NULL;

		// Add a item for each dock handle, each dock handle checks to see if it is a valid target.
		// If not, it wont be added to the sorted dock handles list and will also have its validtarget flag set to false.
		const EUIWidgetFace Faces[4] = {UIFACE_Top, UIFACE_Bottom, UIFACE_Left, UIFACE_Right};

		for(INT DockFaceIdx = 0; DockFaceIdx < 4; DockFaceIdx++)
		{
			WidgetDockHandles->DockHandles[DockFaceIdx].Widget = NULL;
			WidgetDockHandles->DockHandles[DockFaceIdx].Face = Faces[DockFaceIdx];
			WidgetDockHandles->DockHandles[DockFaceIdx].bValidTarget = TRUE;

			if(WidgetDockHandles->DockHandles[DockFaceIdx].bValidTarget == TRUE)
			{
				GetDockHandlePosition(Scene, Faces[DockFaceIdx], FDockingConstants::HandleRadius, 0, WidgetDockHandles->DockHandles[DockFaceIdx].Position);

				SortedDockHandles.AddItem(&WidgetDockHandles->DockHandles[DockFaceIdx]);
			}
		}

		SceneWidgetDockHandles.AddItem(WidgetDockHandles);
	}

	// Add dock handles for all of the scene's widgets.
	TArray<UUIObject*> Widgets = UIEditor->OwnerScene->GetChildren( TRUE );

	for( INT DockIdx = 0; DockIdx < Widgets.Num(); DockIdx++ )
	{
		FWidgetDockHandles* WidgetDockHandles = new FWidgetDockHandles;

		WidgetDockHandles->bVisible = FALSE;
		WidgetDockHandles->Widget = Widgets(DockIdx);

		// Add a item for each dock handle, each dock handle checks to see if it is a valid target.
		// If not, it wont be added to the sorted dock handles list and will also have its validtarget flag set to false.
		const EUIWidgetFace Faces[4] = {UIFACE_Top, UIFACE_Bottom, UIFACE_Left, UIFACE_Right};

		for(INT DockFaceIdx = 0; DockFaceIdx < 4; DockFaceIdx++)
		{
			WidgetDockHandles->DockHandles[DockFaceIdx].Widget = WidgetDockHandles->Widget;
			WidgetDockHandles->DockHandles[DockFaceIdx].Face = Faces[DockFaceIdx];
			WidgetDockHandles->DockHandles[DockFaceIdx].bValidTarget = SourceWidget->IsValidDockTarget(SourceFace, WidgetDockHandles->DockHandles[DockFaceIdx].Widget, WidgetDockHandles->DockHandles[DockFaceIdx].Face);

			if(WidgetDockHandles->DockHandles[DockFaceIdx].bValidTarget == TRUE)
			{
				GetDockHandlePosition(WidgetDockHandles->Widget, Faces[DockFaceIdx], FDockingConstants::HandleRadius, 0, WidgetDockHandles->DockHandles[DockFaceIdx].Position);

				SortedDockHandles.AddItem(&WidgetDockHandles->DockHandles[DockFaceIdx]);
			}
		}
	
		SceneWidgetDockHandles.AddItem(WidgetDockHandles);
	}


	Sort<FWidgetDockHandle*,FRenderModifier_TargetWidgetOutline::FWidgetDockHandle>(&SortedDockHandles(0),SortedDockHandles.Num());
}

void FRenderModifier_TargetWidgetOutline::FreeSortedDockHandleList()
{
	
	INT WidgetCount = SceneWidgetDockHandles.Num();


	for( INT WidgetIdx = 0; WidgetIdx < WidgetCount; WidgetIdx++ )
	{
		FWidgetDockHandles* WidgetHandles = SceneWidgetDockHandles(WidgetIdx);

		delete WidgetHandles;
	}

	SceneWidgetDockHandles.Empty();
	SortedDockHandles.Empty();
}


void FRenderModifier_TargetWidgetOutline::CalculateVisibleDockHandles(TArray<UUIObject*> &Widgets)
{

	for(INT WidgetHandlesIdx = 0; WidgetHandlesIdx < SceneWidgetDockHandles.Num(); WidgetHandlesIdx++)
	{
		FWidgetDockHandles* WidgetHandles = SceneWidgetDockHandles(WidgetHandlesIdx);
		UBOOL bWidgetMatch = FALSE;

		for( INT WidgetIdx = 0; WidgetIdx < Widgets.Num(); WidgetIdx++ )
		{
			UUIObject* Widget = Widgets(WidgetIdx);

			bWidgetMatch = (WidgetHandles->Widget == Widget);

			if( bWidgetMatch == TRUE)
			{
				break;
			}
		}

		WidgetHandles->bVisible = bWidgetMatch;
	}


}

/** Sets the selected dock face for highlighting. */
void FRenderModifier_TargetWidgetOutline::SetSelectedDockFace(EUIWidgetFace InFace)
{
	// Only update when we actually change state because it is an expensive operation to calculate
	// close dock handles.
	if(SelectedDockHandle != InFace)
	{
		SelectedDockHandle = InFace;

		if(SelectedDockHandle != UIFACE_MAX)
		{
			// Calculate all handles within the proximity of the handle we selected, then build a list of all of their names.
			TArray<FString> TempStringList;
			const TArray<FRenderModifier_TargetWidgetOutline::FWidgetDockHandle*>* CloseHandles = CalculateCloseDockHandles();
			const INT NumHandles = CloseHandles->Num();
			for(INT HandleIdx = 0; HandleIdx < NumHandles; HandleIdx++)
			{
				const FWidgetDockHandle* Handle = (*CloseHandles)(HandleIdx);

				FString TempString;
				UIEditor->GetDockHandleString(Handle->Widget, Handle->Face, TempString);

				TempStringList.AddItem(TempString);

			}

			UIEditor->TargetTooltip->SetStringList(TempStringList);
		}
		else
		{
			UIEditor->TargetTooltip->ClearStringList();
		}
	}
	
}


/** Clears the selected dock face and dock widget. */
void FRenderModifier_TargetWidgetOutline::ClearSelections()
{
	ClearVisibleDockHandles();
	SelectedWidget = NULL;
	SelectedDockHandle = UIFACE_MAX;

	UIEditor->TargetTooltip->ClearStringList();
}

/** Toggles all of the dock handles to not be visible */
void FRenderModifier_TargetWidgetOutline::ClearVisibleDockHandles()
{
	for(INT WidgetHandlesIdx = 0; WidgetHandlesIdx < SceneWidgetDockHandles.Num(); WidgetHandlesIdx++)
	{
		FWidgetDockHandles* WidgetHandles = SceneWidgetDockHandles(WidgetHandlesIdx);

		WidgetHandles->bVisible = FALSE;
	}
}
/**
 *	Calculates dock handles which are close to the currently selected widget dock handle.
 *  @return			The number of dock handles in the specified radius.
 */
TArray<FRenderModifier_TargetWidgetOutline::FWidgetDockHandle*>* FRenderModifier_TargetWidgetOutline::CalculateCloseDockHandles()
{
	if(SelectedDockHandle != UIFACE_MAX)
	{
		FIntPoint SelectedHandlePosition;
		GetDockHandlePosition(SelectedWidget, SelectedDockHandle, FDockingConstants::HandleRadius, 0, SelectedHandlePosition);

		const INT TotalHandles = SortedDockHandles.Num();
		const FLOAT SearchRadius = (FDockingConstants::HandleRadius * 1.5f);
		const FLOAT DistanceThreshold = SearchRadius * SearchRadius;
		const FLOAT MinX = (INT)SelectedHandlePosition.X - SearchRadius;
		const FLOAT MaxX = (INT)SelectedHandlePosition.X + SearchRadius;

		CloseHandles.Empty();

		// Generate a list of docking handles within our selection threshold.  The list of handles is sorted by X position,
		// so we can exit early if the X position of a handle is greater than our maximum X.
		for(INT HandleIdx = 0; HandleIdx < TotalHandles; HandleIdx++)
		{
			FRenderModifier_TargetWidgetOutline::FWidgetDockHandle* Handle = SortedDockHandles(HandleIdx);

			if(Handle->Position.X > MaxX)
			{
				break;
			}

			if(Handle->Position.X > MinX)
			{
				FLOAT DistX = Handle->Position.X - SelectedHandlePosition.X; 
				FLOAT DistY = Handle->Position.Y - SelectedHandlePosition.Y;
				FLOAT Dist = DistX * DistX + DistY * DistY;

				if(Dist < DistanceThreshold)
				{
					CloseHandles.AddItem(Handle);
				}
			}
		}
	}
	else
	{
		CloseHandles.Empty();
	}
	

	return &CloseHandles;
}


/**
 * Method for serializing UObject references that must be kept around through GC's.
 *
 * @param Ar The archive to serialize with
 */
void FRenderModifier_TargetWidgetOutline::Serialize(FArchive& Ar)
{
	INT NumHandles = SceneWidgetDockHandles.Num();

	for(INT HandleIdx = 0; HandleIdx < NumHandles; HandleIdx++)
	{
		FWidgetDockHandles* WidgetDockHandles = SceneWidgetDockHandles(HandleIdx);

		Ar << WidgetDockHandles->Widget;
	}

	Ar << SelectedWidget;


	FRenderModifier_SelectedWidgetsOutline::Serialize(Ar);
}

/**
 * Renders all of the visible dock handles.
 */
void FRenderModifier_TargetWidgetOutline::Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
//	FRenderModifier_SelectedWidgetsOutline::Render(ObjectVC, Canvas);

	INT NumHandles = SceneWidgetDockHandles.Num();

	for(INT HandleIdx = 0; HandleIdx < NumHandles; HandleIdx++)
	{
		const FWidgetDockHandles* WidgetDockHandles = SceneWidgetDockHandles(HandleIdx);

		if(WidgetDockHandles->bVisible == TRUE)
		{
			if ( WidgetDockHandles->Widget != NULL )
			{
				Canvas->PushDepthSortKey(-MAXINT+2);
				Canvas->PushRelativeTransform(WidgetDockHandles->Widget->GenerateTransformMatrix());
			}
			for( INT DockIdx = 0; DockIdx < 4; DockIdx++)
			{

				// Only draw the dock handle if it is a valid target for our dock link.
				if(WidgetDockHandles->DockHandles[DockIdx].bValidTarget == TRUE)
				{
					RenderDockHandle(WidgetDockHandles->DockHandles[DockIdx], ObjectVC, Canvas);
				}
			}
			if ( WidgetDockHandles->Widget != NULL )
			{
				Canvas->PopTransform();
				Canvas->PopDepthSortKey();
			}
		}
	}
}


/** 
 * Renders a docking handle for this widget, it uses a different color to let the user know its a target widget.
 * 
 * @param Widget	Widget to render the docking handle for.
 * @param Face		Which face we are drawing the handle for.
 * @param RI		Rendering interface to use for drawing the handle.
 */
void FRenderModifier_TargetWidgetOutline::RenderDockHandle(const FWidgetDockHandle& DockHandle, FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	const FColor DockHandlerColor(255,32,0);
	const FColor DockHandlerHighlightColor(255,255,0);
	const FColor DockHandlerBorderColor(0,0,0);
	const FLOAT Zoom = ObjectVC->Zoom2D < 1.0f ? 1.0f : ObjectVC->Zoom2D;
	const FLOAT DockHandleRadius = FDockingConstants::HandleRadius / Zoom;
	const INT NumSides = FDockingConstants::HandleNumSides;

	Canvas->SetHitProxy( new HUIDockHandleHitProxy(DockHandle.Widget, DockHandle.Face) );
	{
		// Draw the dock handle circle, if the mouse is over the dock handle, then draw it in a different color.
		const UBOOL bHandleHighlighted = (SelectedDockHandle == DockHandle.Face && SelectedWidget == DockHandle.Widget);

		FLinkedObjDrawUtils::DrawNGon(Canvas, DockHandle.Position, DockHandlerBorderColor, NumSides, DockHandleRadius+1);

		if(bHandleHighlighted == TRUE)
		{
			FLinkedObjDrawUtils::DrawNGon(Canvas, DockHandle.Position, DockHandlerHighlightColor, NumSides, DockHandleRadius);
		}
		else
		{
			FLinkedObjDrawUtils::DrawNGon(Canvas, DockHandle.Position, DockHandlerColor, NumSides, DockHandleRadius);
		}

	}
	Canvas->SetHitProxy( NULL );
}



/* ==========================================================================================================
	"Focus Chain Target" render modifier - similar to the FRenderModifier_TargetWidgetOutline class; renders
	focus chain handles for the widget under the mouse when the focus chain mouse tool is active.
========================================================================================================== */
/**
 * Constructor
 *
 * @param	InEditor	The UI editor that this rendermodifier belongs to.
 */
FRenderModifier_FocusChainTarget::FRenderModifier_FocusChainTarget( WxUIEditorBase* InEditor )
: FUIEditor_RenderModifier(InEditor), SelectedWidget(NULL)
{
}


/**
 * Sets the list of widgets that we currently have as targets (ie under the mouse cursor)
 * @param Widgets	Currently targeted widgets
 */
void FRenderModifier_FocusChainTarget::SetTargetWidgets(TArray<UUIObject*> &Widgets)
{
	TargetWidgets = Widgets;

	if(TargetWidgets.Num())
	{
		// Generate a tooltip with the names of all of the widgets the mouse is currently over.
		TArray<FString> TempStringList;
		const INT NumHitWidgets = TargetWidgets.Num();
		for(INT WidgetIdx = 0; WidgetIdx < NumHitWidgets; WidgetIdx++)
		{
			const UUIObject* Widget = TargetWidgets(WidgetIdx);
			TempStringList.AddItem(*Widget->GetName());
		}

		UIEditor->TargetTooltip->SetStringList(TempStringList);
	}
	else
	{
		UIEditor->TargetTooltip->ClearStringList();
	}

}

/** Clears the selected dock face and dock widget. */
void FRenderModifier_FocusChainTarget::ClearSelections()
{
	SelectedWidget = NULL;
	TargetWidgets.Empty();

	UIEditor->TargetTooltip->ClearStringList();
}


/**
 * Method for serializing UObject references that must be kept around through GC's.
 *
 * @param Ar The archive to serialize with
 */
void FRenderModifier_FocusChainTarget::Serialize(FArchive& Ar)
{
	Ar << SelectedWidget << TargetWidgets;
}

/**
 * Renders an outline around all of our target widgets.
 */
void FRenderModifier_FocusChainTarget::Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	INT NumWidgets = TargetWidgets.Num();

	for(INT WidgetIdx = 0; WidgetIdx < NumWidgets; WidgetIdx++)
	{
		UUIObject* Widget = TargetWidgets(WidgetIdx);

		Canvas->PushDepthSortKey(-MAXINT+2);
		Canvas->PushRelativeTransform(Widget->GenerateTransformMatrix());
		// Render a box outline around this widget because it is selected.
		FColor BorderColor(255,255,64);

		FVector Expand(2 / ObjectVC->Zoom2D,2 / ObjectVC->Zoom2D,2 / ObjectVC->Zoom2D);
		FVector Start, End;

		// Top Line.
		FLOAT MinX = 0.0f, MaxX = 0.0f, MinY = 0.0f, MaxY = 0.0f;
		Widget->GetPositionExtents( MinX, MaxX, MinY, MaxY, FALSE, TRUE );

		Start.X = MinX;
		Start.Y = MinY;

		End.X = MaxX;
		End.Y = MinY;

		DrawTile(Canvas,Start.X - Expand.X, Start.Y - Expand.Y, Expand.X * 2 + End.X - Start.X, Expand.Y * 2 + End.Y - Start.Y, 0.0f, 0.0f, 0.0f, 0.0f, BorderColor);

		// Bottom Line.
		Start.X = MinX;
		Start.Y = MaxY;

		End.X = MaxX;
		End.Y = MaxY;

		DrawTile(Canvas,Start.X - Expand.X, Start.Y - Expand.Y, Expand.X * 2 + End.X - Start.X, Expand.Y * 2 + End.Y - Start.Y, 0.0f, 0.0f, 0.0f, 0.0f, BorderColor);


		// Left Line.
		Start.X = MinX;
		Start.Y = MinY;

		End.X = MinX;
		End.Y = MaxY;

		DrawTile(Canvas,Start.X - Expand.X, Start.Y - Expand.Y, Expand.X * 2 + End.X - Start.X, Expand.Y * 2 + End.Y - Start.Y, 0.0f, 0.0f, 0.0f, 0.0f, BorderColor);

		// Right Line.
		Start.X = MaxX;
		Start.Y = MinY;

		End.X = MaxX;
		End.Y = MaxY;

		DrawTile(Canvas,Start.X - Expand.X, Start.Y - Expand.Y, Expand.X * 2 + End.X - Start.X, Expand.Y * 2 + End.Y - Start.Y, 0.0f, 0.0f, 0.0f, 0.0f, BorderColor);
		Canvas->PopTransform();
		Canvas->PopDepthSortKey();
	}
}


/* ==========================================================================================================
	"Selected Widget" render modifier - renders the selection box and anchors
========================================================================================================== */

// Docking Constants
namespace FFocusChainConstants
{
	static const INT HandleRadius = 8;
	static const INT HandleGap = 8;
	static const INT HandleNumSides = 16;
};


/**
 * Constructor
 *
 * @param	InEditor	the editor window that holds the container to render the outline for
 */
FRenderModifier_FocusChainHandles::FRenderModifier_FocusChainHandles( class WxUIEditorBase* InEditor ) : 
FRenderModifier_SelectedWidgetsOutline(InEditor)
{
	bRenderDockHandles = FALSE;
}



/** Sets the selected dock face for highlighting. */
void FRenderModifier_FocusChainHandles::SetSelectedDockFace(EUIWidgetFace InFace)
{
	// Only update if we are changing state.
	// If we changed to a blank face, then clear the tooltip. Otherwise, update the tooltip with some information
	// about the currently highlighted dock handle.
	if(SelectedDockHandle != InFace)
	{
		if(InFace != UIFACE_MAX && SelectedWidget != NULL)
		{
			FString DockString;
			FString TempString;
			TArray<FString> TempStringArray;

			UIEditor->GetDockHandleString(SelectedWidget, InFace,DockString);
			TempString = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_Source")), *DockString));
			TempStringArray.AddItem(TempString);

			const UUIObject* TargetWidget = SelectedWidget->GetNavigationTarget(InFace);
			if(TargetWidget == NULL)
			{
				TempString = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_Target")), *LocalizeUI(TEXT("UIEditor_Face_None"))));
			}
			else
			{
				TempString = FString::Printf(LocalizeSecure(LocalizeUI(TEXT("UIEditor_Target")), *TargetWidget->GetName()));
			}
			
		
			TempStringArray.AddItem(TempString);

			UIEditor->TargetTooltip->SetStringList(TempStringArray);
		}
		else
		{
			UIEditor->TargetTooltip->ClearStringList();
		}

		SelectedDockHandle = InFace;
	}
}

/**
 * Renders the outline for the top-level object of a UI editor window.
 */
void FRenderModifier_FocusChainHandles::Render( FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	FRenderModifier_SelectedWidgetsOutline::Render(ObjectVC, Canvas);

	// Loop through all selected widgets and render their focus chain handles.
	TArray<UUIObject*> SelectedWidgets;
	UIEditor->GetSelectedWidgets(SelectedWidgets);

	const INT NumWidgets = SelectedWidgets.Num();

	for(INT WidgetIdx = 0; WidgetIdx < NumWidgets; WidgetIdx++)
	{
		UUIObject* Widget = SelectedWidgets(WidgetIdx);

		if( !Widget->IsPrivateBehaviorSet( UCONST_PRIVATE_NotFocusable ) )
		{
			Canvas->PushDepthSortKey(appTrunc(Widget->GetZDepth()));
			Canvas->PushRelativeTransform(Widget->GenerateTransformMatrix());
			RenderFocusChainHandle(Widget, UIFACE_Top, ObjectVC, Canvas);
			RenderFocusChainHandle(Widget, UIFACE_Bottom, ObjectVC, Canvas);
			RenderFocusChainHandle(Widget, UIFACE_Left, ObjectVC, Canvas);
			RenderFocusChainHandle(Widget, UIFACE_Right, ObjectVC, Canvas);
			Canvas->PopTransform();
			Canvas->PopDepthSortKey();
		}
	}
}

/** 
* Renders a focus chain handle for the specified widget.
* 
* @param Widget	Widget to render the focus chain handle for.
* @param Face		Which face we are drawing the handle for.
* @param RI		Rendering interface to use for drawing the handle.
*/
void FRenderModifier_FocusChainHandles::RenderFocusChainHandle( UUIObject* Widget, EUIWidgetFace InFace, FEditorObjectViewportClient* ObjectVC, FCanvas* Canvas )
{
	const FColor FocusChainHandlerColor(128,0,128);
	const FColor FocusChainHandlerHighlightColor(192,0,192);
	const FColor FocusChainHandlerBorderColor(0,0,0);
	const FLOAT Zoom = ObjectVC->Zoom2D < 1.0f ? 1.0f : ObjectVC->Zoom2D;
	const FLOAT FocusChainHandleRadius = FFocusChainConstants::HandleRadius / Zoom;
	const FLOAT FocusChainHandleGap = FFocusChainConstants::HandleGap / Zoom;

	// Get a position for the focus chain handle.
	FIntPoint HandlePosition;
	GetHandlePosition(Widget->RenderBounds, InFace, FocusChainHandleRadius, FocusChainHandleGap, HandlePosition);

	//Render the dock link for this object.
	RenderLink(Widget, InFace, Canvas, HandlePosition);

	Canvas->SetHitProxy( new HUIFocusChainHandleHitProxy(Widget, InFace) );
	{
		// Draw the dock handle circle, if the mouse is over the dock handle, then draw it in a different color.
		const UBOOL bHandleHighlighted = (SelectedWidget == Widget && SelectedDockHandle == InFace);
		FColor DockColor;

		if(bHandleHighlighted == TRUE)
		{
			DockColor = FocusChainHandlerHighlightColor;
		}
		else
		{
			DockColor = FocusChainHandlerColor;
		}

		FVector2D LogicLeft(HandlePosition.X - FocusChainHandleRadius, HandlePosition.Y);
		FVector2D LogicTop(HandlePosition.X, HandlePosition.Y - FocusChainHandleRadius);
		FVector2D LogicRight(HandlePosition.X + FocusChainHandleRadius, HandlePosition.Y);
		FVector2D LogicBottom(HandlePosition.X, HandlePosition.Y + FocusChainHandleRadius);

		// Draw Triangle Borders
		const FLOAT BorderSize = 2 / Zoom;
		DrawTriangle2D( Canvas,
			FVector2D(LogicLeft.X - BorderSize,		LogicLeft.Y), FVector2D(0,0),
			FVector2D(LogicTop.X,					LogicTop.Y - BorderSize), FVector2D(0,0),
			FVector2D(LogicRight.X + BorderSize,	LogicRight.Y), FVector2D(0,0),
			FocusChainHandlerBorderColor );

		DrawTriangle2D( Canvas,
			FVector2D(LogicLeft.X - BorderSize,		LogicLeft.Y), FVector2D(0,0),
			FVector2D(LogicBottom.X,				LogicBottom.Y + BorderSize), FVector2D(0,0),
			FVector2D(LogicRight.X + BorderSize,	LogicRight.Y), FVector2D(0,0),
			FocusChainHandlerBorderColor );

		DrawTriangle2D( Canvas,
			FVector2D(LogicLeft.X,	LogicLeft.Y), FVector2D(0,0),
			FVector2D(LogicTop.X,	LogicTop.Y), FVector2D(0,0),
			FVector2D(LogicRight.X,	LogicRight.Y), FVector2D(0,0),
			DockColor );

		DrawTriangle2D( Canvas,
			FVector2D(LogicLeft.X,		LogicLeft.Y), FVector2D(0,0),
			FVector2D(LogicBottom.X,	LogicBottom.Y), FVector2D(0,0),
			FVector2D(LogicRight.X,		LogicRight.Y), FVector2D(0,0),
			DockColor );

		//Draw a X on top of the handle if the widget focus link is connected.
		const UBOOL bWidgetHandleConnected = Widget->GetNavigationTarget(InFace) != NULL;
		if( bWidgetHandleConnected == TRUE )
		{
			UTexture2D* tex = GApp->MaterialEditor_Delete;
			DrawTile(Canvas,HandlePosition.X - FocusChainHandleRadius / 2,HandlePosition.Y - FocusChainHandleRadius / 2, FocusChainHandleRadius, FocusChainHandleRadius, 0.0f,0.0f,1.0f,1.0f, FLinearColor::White, tex->Resource );
		}
	}
	Canvas->SetHitProxy( NULL );
}


/** 
 * Renders a focus chain link for the specified face.
 * 
 * @param Widget				Widget to render the docking handle for.
 * @param InFace				Which face we are drawing the handle for.
 * @param RI					Rendering interface to use for drawing the handle.
 * @param Start					Starting position for the link.
 */
void FRenderModifier_FocusChainHandles::RenderLink( UUIObject* Widget, EUIWidgetFace InFace, FCanvas* Canvas, const FVector& InStart)
{
	// Make sure that we have a focus target for this face.
	const UUIObject* TargetWidget = Widget->GetNavigationTarget(InFace);
	if(TargetWidget != NULL)
	{

		FVector2D WidgetX(TargetWidget->RenderBounds[UIFACE_Right], TargetWidget->RenderBounds[UIFACE_Left]);
		FVector2D WidgetY(TargetWidget->RenderBounds[UIFACE_Bottom], TargetWidget->RenderBounds[UIFACE_Top]);
		FVector TransformedEndPoint = TargetWidget->Project(FVector(
			WidgetX.GetMin() + ((WidgetX.GetMax() - WidgetX.GetMin()) * 0.5f),
			WidgetY.GetMin() + ((WidgetY.GetMax() - WidgetY.GetMin()) * 0.5f),
			0.f));

		FIntPoint End(appRound(TransformedEndPoint.X), appRound(TransformedEndPoint.Y));

		// Calculate start arrow direction.
		FVector2D StartDir(0,0);

		switch(InFace)
		{
		case UIFACE_Bottom: case UIFACE_Top:
			if(InStart.Y < End.Y)
			{
				StartDir[1] = 1;
			}
			else
			{
				StartDir[1] = -1;
			}
			
			break;

		case UIFACE_Right: case UIFACE_Left:
			if(InStart.X < End.X)
			{
				StartDir[0] = 1;
			}
			else
			{
				StartDir[0] = -1;
			}
			break;

		default:
			return;
		}


		// Finally render the spline.
		FVector TransformedStartPoint = Widget->Project(InStart);
		const FIntPoint Start(appTrunc(TransformedStartPoint.X), appTrunc(TransformedStartPoint.Y));
		const FColor LineColor(0,255,255);
		const FLOAT Tension = (Start-End).Size();

		// since we're in the render loop, the canvas's TranformStack already contains the widget's transform, but since we had
		// to transform the start and end points using Project (which also includes the widget's transform), we need to pull out
		// all widget transforms before rendering the spline.
		Canvas->PushAbsoluteTransform(FScaleMatrix(UIEditor->ObjectVC->Zoom2D) * FTranslationMatrix(FVector(UIEditor->ObjectVC->Origin2D,0)));
		FLinkedObjDrawUtils::DrawSpline(Canvas, Start, Tension * StartDir, End, Tension * StartDir, LineColor, true);
		Canvas->PopTransform();
	}
}


// EOL





