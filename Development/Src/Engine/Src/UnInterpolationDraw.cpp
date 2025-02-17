/*=============================================================================
	UnInterpolationDraw.cpp: Code for supporting interpolation of properties in-game.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnFaceFXSupport.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"
#include "EngineAnimClasses.h"
#include "UnInterpolationHitProxy.h"

#if WITH_FACEFX
using namespace OC3Ent;
using namespace Face;
#endif

static const INT	KeyHalfTriSize = 6;
static const FColor KeyNormalColor(0,0,0);
static const FColor KeyCurveColor(100,0,0);
static const FColor KeyLinearColor(0,100,0);
static const FColor KeyConstantColor(0,0,100);
static const FColor	KeySelectedColor(255,128,0);
static const FColor KeyLabelColor(225,225,225);
static const INT	KeyVertOffset = 3;

static const FLOAT	DrawTrackTimeRes = 0.1f;
static const FLOAT	CurveHandleScale = 0.5f;


enum EInterpTrackAnimControlDragType
{
	ACDT_AnimBlockLeftEdge = 0,
	ACDT_AnimBlockRightEdge
};

enum EInterpTrackParticleReplayDragType
{
	PRDT_LeftEdge = 10,
	PRDT_RightEdge
};

	
/*-----------------------------------------------------------------------------
  UInterpTrack
-----------------------------------------------------------------------------*/

void UInterpTrack::DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params )
{
	INT NumKeys = GetNumKeyframes();

	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());

	for(INT i=0; i<NumKeys; i++)
	{
		FLOAT KeyTime = GetKeyframeTime(i);

		INT PixelPos = appTrunc((KeyTime - Params.StartTime) * Params.PixelsPerSec);

		FIntPoint A(PixelPos - KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
		FIntPoint B(PixelPos + KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
		FIntPoint C(PixelPos,					Params.TrackHeight - KeyVertOffset - KeyHalfTriSize);

		UBOOL bKeySelected = false;
		for(INT j=0; j<Params.SelectedKeys.Num() && !bKeySelected; j++)
		{
			if( Params.SelectedKeys(j).Group == Group && 
				Params.SelectedKeys(j).TrackIndex == Params.TrackIndex && 
				Params.SelectedKeys(j).KeyIndex == i )
				bKeySelected = true;
		}

		FColor KeyColor = GetKeyframeColor(i);
		
		if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HInterpTrackKeypointProxy(Group, Params.TrackIndex, i) );
		if(bKeySelected)
		{
			DrawTriangle2D(Canvas, A+FIntPoint(-2,1), FVector2D(0,0), B+FIntPoint(2,1), FVector2D(0,0), C+FIntPoint(0,-2), FVector2D(0,0), KeySelectedColor );
		}

		DrawTriangle2D(Canvas, A, FVector2D(0,0), B, FVector2D(0,0), C, FVector2D(0,0), KeyColor );
		if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );
	}
}

/** Return color to draw each keyframe in Matinee. */
FColor UInterpTrack::GetKeyframeColor(INT KeyIndex)
{
	return KeyNormalColor;
}

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrack::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Float_Mat"), NULL, LOAD_None, NULL );
}

/*-----------------------------------------------------------------------------
  UInterpTrackMove
-----------------------------------------------------------------------------*/

/** Return color to draw each keyframe in Matinee. */
FColor UInterpTrackMove::GetKeyframeColor(INT KeyIndex)
{
	if( KeyIndex < 0 || KeyIndex >= PosTrack.Points.Num() )
	{
		return KeyNormalColor;
	}

	if( PosTrack.Points(KeyIndex).IsCurveKey() )
	{
		return KeyCurveColor;
	}
	else if( PosTrack.Points(KeyIndex).InterpMode == CIM_Linear )
	{
		return KeyLinearColor;
	}
	else
	{
		return KeyConstantColor;
	}
}


void UInterpTrackMove::DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params )
{
	INT NumKeys = GetNumKeyframes();

	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());

	for(INT KeyIndex=0; KeyIndex<NumKeys; KeyIndex++)
	{
		FLOAT KeyTime = GetKeyframeTime(KeyIndex);

		INT PixelPos = appTrunc((KeyTime - Params.StartTime) * Params.PixelsPerSec);

		FIntPoint A(PixelPos - KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
		FIntPoint B(PixelPos + KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
		FIntPoint C(PixelPos,					Params.TrackHeight - KeyVertOffset - KeyHalfTriSize);

		UBOOL bKeySelected = false;
		for(INT SelectedKeyIndex=0; SelectedKeyIndex<Params.SelectedKeys.Num() && !bKeySelected; SelectedKeyIndex++)
		{
			if( Params.SelectedKeys(SelectedKeyIndex).Group == Group && 
				Params.SelectedKeys(SelectedKeyIndex).TrackIndex == Params.TrackIndex && 
				Params.SelectedKeys(SelectedKeyIndex).KeyIndex == KeyIndex )
				bKeySelected = true;
		}

		FColor KeyColor = GetKeyframeColor(KeyIndex);
		
		if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HInterpTrackKeypointProxy(Group, Params.TrackIndex, KeyIndex) );
		if(bKeySelected)
		{
			DrawTriangle2D(Canvas, A+FIntPoint(-2,1), FVector2D(0,0), B+FIntPoint(2,1), FVector2D(0,0), C+FIntPoint(0,-2), FVector2D(0,0), KeySelectedColor );
		}

		DrawTriangle2D(Canvas, A, FVector2D(0,0), B, FVector2D(0,0), C, FVector2D(0,0), KeyColor );
		if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );

		// Draw lookup name if one exists for this key.
		FName LookupName = GetLookupKeyGroupName(KeyIndex);
		if(LookupName != NAME_None)
		{
			INT XL, YL;
			FString Str = LookupName.ToString();
			StringSize( GEngine->SmallFont, XL, YL, *Str );
			DrawShadowedString(Canvas, PixelPos - XL / 2, Params.TrackHeight - YL - KeyVertOffset - KeyHalfTriSize - 2, *Str, GEngine->SmallFont, KeyLabelColor );
		}
	}
}


/**
*	For drawing track information into the 3D scene.
*	TimeRes is how often to draw an event (eg. resoltion of spline path) in seconds.
*/
void UInterpTrackMove::Render3DTrack(UInterpTrackInst* TrInst, 
									 const FSceneView* View, 
									 FPrimitiveDrawInterface* PDI, 
									 INT TrackIndex, 
									 const FColor& TrackColor, 
									 TArray<class FInterpEdSelKey>& SelectedKeys)
{
	// Draw nothing if no points.
	if(PosTrack.Points.Num() == 0)
	{
		return;
	}

	// if we dont want to draw the track, return.
	if(bHide3DTrack)
	{
		return;
	}

	UBOOL bHitTesting = PDI->IsHitTesting();

	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());

	FVector OldKeyPos(0);
	FLOAT OldKeyTime = 0.f;

	for(INT i=0; i<PosTrack.Points.Num(); i++)
	{
		FLOAT NewKeyTime = PosTrack.Points(i).InVal;

		FVector NewKeyPos(0);
		FRotator NewKeyRot(0,0,0);
		GetLocationAtTime(TrInst, NewKeyTime, NewKeyPos, NewKeyRot);

		// If not the first keypoint, draw a line to the last keypoint.
		if(i>0)
		{
			INT NumSteps = appCeil( (NewKeyTime - OldKeyTime)/DrawTrackTimeRes );
			FLOAT DrawSubstep = (NewKeyTime - OldKeyTime)/NumSteps;

			// Find position on first keyframe.
			FLOAT OldTime = OldKeyTime;

			FVector OldPos(0);
			FRotator OldRot(0,0,0);
			GetLocationAtTime(TrInst, OldKeyTime, OldPos, OldRot);
		
			// For constant interpolation - don't draw ticks - just draw dotted line.
			if(PosTrack.Points(i-1).InterpMode == CIM_Constant)
			{
				DrawDashedLine(PDI,OldPos, NewKeyPos, TrackColor, 20, SDPG_Foreground);
			}
			else
			{
				// Then draw a line for each substep.
				for(INT j=1; j<NumSteps+1; j++)
				{
					FLOAT NewTime = OldKeyTime + j*DrawSubstep;

					FVector NewPos(0);
					FRotator NewRot(0,0,0);
					GetLocationAtTime(TrInst, NewTime, NewPos, NewRot);

					PDI->DrawLine(OldPos, NewPos, TrackColor, SDPG_Foreground);

					// Don't draw point for last one - its the keypoint drawn above.
					if(j != NumSteps)
					{
						PDI->DrawPoint(NewPos, TrackColor, 3.f, SDPG_Foreground);
					}

					OldTime = NewTime;
					OldPos = NewPos;
				}
			}
		}

		OldKeyTime = NewKeyTime;
		OldKeyPos = NewKeyPos;
	}

	// Draw keypoints on top of curve
	for(INT i=0; i<PosTrack.Points.Num(); i++)
	{
		// Find if this key is one of the selected ones.
		UBOOL bKeySelected = false;
		for(INT j=0; j<SelectedKeys.Num() && !bKeySelected; j++)
		{
			if( SelectedKeys(j).Group == Group && 
				SelectedKeys(j).TrackIndex == TrackIndex && 
				SelectedKeys(j).KeyIndex == i )
				bKeySelected = true;
		}

		// Find the time, position and orientation of this Key.
		FLOAT NewKeyTime = PosTrack.Points(i).InVal;

		FVector NewKeyPos(0);
		FRotator NewKeyRot(0,0,0);
		GetLocationAtTime(TrInst, NewKeyTime, NewKeyPos, NewKeyRot);

		UInterpTrackInstMove* MoveTrackInst = CastChecked<UInterpTrackInstMove>(TrInst);
		FMatrix RefTM = GetMoveRefFrame(MoveTrackInst);

		FColor KeyColor = bKeySelected ? KeySelectedColor : TrackColor;

		if(bHitTesting) PDI->SetHitProxy( new HInterpTrackKeypointProxy(Group, TrackIndex, i) );
		PDI->DrawPoint(NewKeyPos, KeyColor, 6.f, SDPG_Foreground);

		// If desired, draw directional arrow at each keyframe.
		if(bShowArrowAtKeys)
		{
			FRotationTranslationMatrix ArrowToWorld(NewKeyRot,NewKeyPos);
			DrawDirectionalArrow(PDI, FScaleMatrix(FVector(16.f,16.f,16.f)) * ArrowToWorld, KeyColor, 3.f, 1.f, SDPG_Foreground );
		}
		if(bHitTesting) PDI->SetHitProxy( NULL );

		UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
		USeqAct_Interp* Seq = CastChecked<USeqAct_Interp>( GrInst->GetOuter() );
		UInterpGroupInst* FirstGrInst = Seq->FindFirstGroupInst(Group);

		// If a selected key, and this is the 'first' instance of this group, draw handles.
		if(bKeySelected && (GrInst == FirstGrInst))
		{
			FVector ArriveTangent = PosTrack.Points(i).ArriveTangent;
			FVector LeaveTangent = PosTrack.Points(i).LeaveTangent;

			BYTE PrevMode = (i > 0)							? GetKeyInterpMode(i-1) : 255;
			BYTE NextMode = (i < PosTrack.Points.Num()-1)	? GetKeyInterpMode(i)	: 255;

			// If not first point, and previous mode was a curve type.
			if(PrevMode == CIM_CurveAuto || PrevMode == CIM_CurveAutoClamped || PrevMode == CIM_CurveUser || PrevMode == CIM_CurveBreak)
			{
				FVector HandlePos = NewKeyPos - RefTM.TransformNormal(ArriveTangent * CurveHandleScale);
				PDI->DrawLine(NewKeyPos, HandlePos, FColor(255,255,255), SDPG_Foreground);

				if(bHitTesting) PDI->SetHitProxy( new HInterpTrackKeyHandleProxy(Group, TrackIndex, i, true) );
				PDI->DrawPoint(HandlePos, FColor(255,255,255), 5.f, SDPG_Foreground);
				if(bHitTesting) PDI->SetHitProxy( NULL );
			}

			// If next section is a curve, draw leaving handle.
			if(NextMode == CIM_CurveAuto || NextMode == CIM_CurveAutoClamped || NextMode == CIM_CurveUser || NextMode == CIM_CurveBreak)
			{
				FVector HandlePos = NewKeyPos + RefTM.TransformNormal(LeaveTangent * CurveHandleScale);
				PDI->DrawLine(NewKeyPos, HandlePos, FColor(255,255,255), SDPG_Foreground);

				if(bHitTesting) PDI->SetHitProxy( new HInterpTrackKeyHandleProxy(Group, TrackIndex, i, false) );
				PDI->DrawPoint(HandlePos, FColor(255,255,255), 5.f, SDPG_Foreground);
				if(bHitTesting) PDI->SetHitProxy( NULL );
			}
		}
	}
}

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackMove::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Move_Mat"), NULL, LOAD_None, NULL );
}

/*-----------------------------------------------------------------------------
	UInterpTrackFloatBase
-----------------------------------------------------------------------------*/

/** Return color to draw each keyframe in Matinee. */
FColor UInterpTrackFloatBase::GetKeyframeColor(INT KeyIndex)
{
	if( KeyIndex < 0 || KeyIndex >= FloatTrack.Points.Num() )
	{
		return KeyNormalColor;
	}

	if( FloatTrack.Points(KeyIndex).IsCurveKey() )
		return KeyCurveColor;
	else if( FloatTrack.Points(KeyIndex).InterpMode == CIM_Linear )
		return KeyLinearColor;
	else
		return KeyConstantColor;
}

/*-----------------------------------------------------------------------------
	UInterpTrackFloatProp
-----------------------------------------------------------------------------*/

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackFloatProp::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Float_Mat"), NULL, LOAD_None, NULL );
}


/*-----------------------------------------------------------------------------
	UInterpTrackToggle
-----------------------------------------------------------------------------*/
UMaterial* UInterpTrackToggle::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Toggle_Mat"), NULL, LOAD_None, NULL );
}

void UInterpTrackToggle::DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params )
{
	INT NumKeys = GetNumKeyframes();

	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());

	// Draw the 'on' blocks in green
	INT LastPixelPos = -1;
	UBOOL bLastPosWasOn = FALSE;
	for (INT i=0; i<NumKeys; i++)
	{
		FLOAT KeyTime = GetKeyframeTime(i);
		INT PixelPos = appTrunc((KeyTime - Params.StartTime) * Params.PixelsPerSec);

		FToggleTrackKey& Key = ToggleTrack(i);
		if ((Key.ToggleAction == ETTA_Off) && bLastPosWasOn)
		{
			DrawTile(Canvas, LastPixelPos, KeyVertOffset, PixelPos - LastPixelPos, Params.TrackHeight - (2 * KeyVertOffset), 
				0.0f, 0.0f, 0.0f, 0.0f, FLinearColor(0.0f, 1.0f, 0.0f));
		}

		LastPixelPos = PixelPos;
		bLastPosWasOn = (Key.ToggleAction == ETTA_On) ? TRUE : FALSE;
	}

	// Draw the keyframe points after, so they are on top
	for (INT i=0; i<NumKeys; i++)
	{
		FLOAT KeyTime = GetKeyframeTime(i);

		INT PixelPos = appTrunc((KeyTime - Params.StartTime) * Params.PixelsPerSec);

		FIntPoint A, A_Offset;
		FIntPoint B, B_Offset;
		FIntPoint C, C_Offset;

		FToggleTrackKey& Key = ToggleTrack(i);
		if (Key.ToggleAction == ETTA_Off)
		{
			// Point the triangle down...
			A = FIntPoint(PixelPos - KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset - KeyHalfTriSize);
			B = FIntPoint(PixelPos + KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset - KeyHalfTriSize);
			C = FIntPoint(PixelPos,						Params.TrackHeight - KeyVertOffset);

			A_Offset = FIntPoint(-2,-2);
			B_Offset = FIntPoint( 2,-2);
			C_Offset = FIntPoint( 0, 1);
		}
		else
		if (Key.ToggleAction == ETTA_Trigger)
		{
			// Point the triangle up
			A = FIntPoint(PixelPos - KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
			B = FIntPoint(PixelPos + KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
			C = FIntPoint(PixelPos,						Params.TrackHeight - KeyVertOffset - KeyHalfTriSize);

			A_Offset = FIntPoint(-2, 1);
			B_Offset = FIntPoint( 2, 1);
			C_Offset = FIntPoint( 0,-2);

			DrawTile(Canvas, PixelPos - 4, KeyVertOffset, 7, Params.TrackHeight - (2 * KeyVertOffset), 
				0.0f, 0.0f, 0.0f, 0.0f, FLinearColor(1.0f, 0.0f, 0.0f));
		}
		else
		{
			// Point the triangle up
			A = FIntPoint(PixelPos - KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
			B = FIntPoint(PixelPos + KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
			C = FIntPoint(PixelPos,						Params.TrackHeight - KeyVertOffset - KeyHalfTriSize);

			A_Offset = FIntPoint(-2, 1);
			B_Offset = FIntPoint( 2, 1);
			C_Offset = FIntPoint( 0,-2);
		}

		UBOOL bKeySelected = false;
		for (INT j=0; j<Params.SelectedKeys.Num() && !bKeySelected; j++)
		{
			if (Params.SelectedKeys(j).Group == Group && 
				Params.SelectedKeys(j).TrackIndex == Params.TrackIndex && 
				Params.SelectedKeys(j).KeyIndex == i)
			{
				bKeySelected = true;
			}
		}

		FColor KeyColor = GetKeyframeColor(i);
		
		if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HInterpTrackKeypointProxy(Group, Params.TrackIndex, i) );
		if (bKeySelected)
		{
			DrawTriangle2D(Canvas, A + A_Offset, FVector2D(0,0), B + B_Offset, FVector2D(0,0), C + C_Offset, FVector2D(0,0), KeySelectedColor );
		}

		DrawTriangle2D(Canvas, A, FVector2D(0,0), B, FVector2D(0,0), C, FVector2D(0,0), KeyColor );
		if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );
	}
}

/*-----------------------------------------------------------------------------
	UInterpTrackVectorBase
-----------------------------------------------------------------------------*/

/** Return color to draw each keyframe in Matinee. */
FColor UInterpTrackVectorBase::GetKeyframeColor(INT KeyIndex)
{
	if( KeyIndex < 0 || KeyIndex >= VectorTrack.Points.Num() )
	{
		return KeyNormalColor;
	}

	if( VectorTrack.Points(KeyIndex).IsCurveKey() )
		return KeyCurveColor;
	else if( VectorTrack.Points(KeyIndex).InterpMode == CIM_Linear )
		return KeyLinearColor;
	else
		return KeyConstantColor;
}

/*-----------------------------------------------------------------------------
	UInterpTrackVectorProp
-----------------------------------------------------------------------------*/

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackVectorProp::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Vector_Mat"), NULL, LOAD_None, NULL );
}

/*-----------------------------------------------------------------------------
	UInterpTrackColorProp
-----------------------------------------------------------------------------*/

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackColorProp::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_ColorTrack_Mat"), NULL, LOAD_None, NULL );
}

/*-----------------------------------------------------------------------------
	UInterpTrackEvent
-----------------------------------------------------------------------------*/

/** 
*	Assumes RI->Origin is in the correct place to start drawing this track.
*	SelectedKeyIndex is INDEX_NONE if no key is selected.
*/
void UInterpTrackEvent::DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params )
{
	Super::DrawTrack( Canvas, Params );

	for(INT i=0; i<EventTrack.Num(); i++)
	{
		FLOAT KeyTime = EventTrack(i).Time;
		INT PixelPos = appTrunc((KeyTime - Params.StartTime) * Params.PixelsPerSec);

		INT XL, YL;
		FString Str = EventTrack(i).EventName.ToString();
		StringSize( GEngine->SmallFont, XL, YL, *Str );
		DrawShadowedString(Canvas, PixelPos + 2, Params.TrackHeight - YL - KeyVertOffset, *Str, GEngine->SmallFont, KeyLabelColor );
	}
}

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackEvent::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Event_Mat"), NULL, LOAD_None, NULL );
}

/*-----------------------------------------------------------------------------
	UInterpTrackDirector
-----------------------------------------------------------------------------*/

/** 
*	Assumes RI->Origin is in the correct place to start drawing this track.
*	SelectedKeyIndex is INDEX_NONE if no key is selected.
*/
void UInterpTrackDirector::DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params )
{
	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());
	UInterpData* Data = CastChecked<UInterpData>(Group->GetOuter());

	// Draw background coloured blocks for camera sections
	for(INT i=0; i<CutTrack.Num(); i++)
	{
		FLOAT KeyTime = CutTrack(i).Time;

		FLOAT NextKeyTime;
		if(i < CutTrack.Num()-1)
		{
			NextKeyTime = ::Min( CutTrack(i+1).Time, Data->InterpLength );
		}
		else
		{
			NextKeyTime = Data->InterpLength;
		}

		// Find the group we are cutting to.
		INT CutGroupIndex = Data->FindGroupByName( CutTrack(i).TargetCamGroup );

		// If its valid, and its not this track, draw a box over duration of shot.
		if((CutGroupIndex != INDEX_NONE) && (CutTrack(i).TargetCamGroup != Group->GroupName))
		{
			INT StartPixelPos = appTrunc((KeyTime - Params.StartTime) * Params.PixelsPerSec);
			INT EndPixelPos = appTrunc((NextKeyTime - Params.StartTime) * Params.PixelsPerSec);

			DrawTile(Canvas, StartPixelPos, KeyVertOffset, EndPixelPos - StartPixelPos, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset), 0.f, 0.f, 1.f, 1.f, Data->InterpGroups(CutGroupIndex)->GroupColor );
		}
	}

	// Use base-class to draw key triangles
	Super::DrawTrack( Canvas, Params );

	// Draw group name for each shot.
	for(INT i=0; i<CutTrack.Num(); i++)
	{
		FLOAT KeyTime = CutTrack(i).Time;
		INT PixelPos = appTrunc((KeyTime - Params.StartTime) * Params.PixelsPerSec);

		INT XL, YL;
		FString Str = CutTrack(i).TargetCamGroup.ToString();
		StringSize( GEngine->SmallFont, XL, YL, *Str );
		DrawShadowedString(Canvas, PixelPos + 2, Params.TrackHeight - YL - KeyVertOffset, *Str, GEngine->SmallFont, KeyLabelColor );
	}
}

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackDirector::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Director_Mat"), NULL, LOAD_None, NULL );
}

/*-----------------------------------------------------------------------------
	UInterpTrackAnimControl
-----------------------------------------------------------------------------*/


/**
 * Lets the interface object know that we are beginning a drag operation.
 */
void UInterpTrackAnimControl::BeginDrag(FInterpEdInputData &InputData)
{
	// Store temporary data.
	if((InputData.InputType == ACDT_AnimBlockLeftEdge || InputData.InputType == ACDT_AnimBlockRightEdge) && AnimSeqs.IsValidIndex(InputData.InputData))
	{
		// Store our starting position.
		InputData.TempData = new FAnimControlTrackKey(AnimSeqs(InputData.InputData));
	}
}

/**
 * Lets the interface object know that we are ending a drag operation.
 */
void UInterpTrackAnimControl::EndDrag(FInterpEdInputData &InputData)
{
	// Clean up our temporary data.
	if(InputData.TempData)
	{
		FAnimControlTrackKey* InterpKey = (FAnimControlTrackKey*)InputData.TempData;
		delete InterpKey;
		InputData.TempData = NULL;
	}
}

/**
 * @return Returns the mouse cursor to display when this input interface is moused over.
 */
EMouseCursor UInterpTrackAnimControl::GetMouseCursor(FInterpEdInputData &InputData)
{
	EMouseCursor Result = MC_NoChange;

	switch(InputData.InputType)
	{
	case ACDT_AnimBlockLeftEdge: case ACDT_AnimBlockRightEdge:
		Result = MC_SizeLeftRight;
		break;
	}

	return Result;
}

/**
 * Called when an object is dragged.
 */
void UInterpTrackAnimControl::ObjectDragged(FInterpEdInputData& InputData)
{
	if(AnimSeqs.IsValidIndex(InputData.InputData) && InputData.TempData)
	{
		FAnimControlTrackKey* OriginalKey = (FAnimControlTrackKey*)InputData.TempData;
		FAnimControlTrackKey &AnimSeq = AnimSeqs(InputData.InputData);
		FIntPoint Delta = InputData.MouseCurrent - InputData.MouseStart;
		FLOAT TimeDelta = Delta.X / InputData.PixelsPerSec;
		UAnimSequence* Seq = FindAnimSequenceFromName(AnimSeq.AnimSeqName);
		if(Seq)
		{
			FLOAT ActualLength = (Seq->SequenceLength - (OriginalKey->AnimStartOffset+OriginalKey->AnimEndOffset));
			FLOAT ActualLengthScaled =  ActualLength / OriginalKey->AnimPlayRate;
			switch(InputData.InputType)
			{
			case ACDT_AnimBlockLeftEdge:
				
				// If ctrl is down we are scaling play time, otherwise we are clipping.
				if(InputData.bCtrlDown)
				{
					FLOAT NewLength = Max<FLOAT>(KINDA_SMALL_NUMBER, (ActualLengthScaled - TimeDelta));
					AnimSeq.AnimPlayRate = Max<FLOAT>(KINDA_SMALL_NUMBER, ActualLength / NewLength);
					AnimSeq.StartTime = OriginalKey->StartTime - (ActualLength / AnimSeq.AnimPlayRate - ActualLengthScaled);
				}
				else if(InputData.bAltDown)
				{
					// We are changing the offset but then scaling the animation proportionately so that the start and end times don't change
					AnimSeq.AnimStartOffset = OriginalKey->AnimStartOffset + TimeDelta * AnimSeq.AnimPlayRate;
					AnimSeq.AnimStartOffset = Clamp<FLOAT>(AnimSeq.AnimStartOffset, 0, Seq->SequenceLength-AnimSeq.AnimEndOffset);

					// Fix the play rate to keep the start and end times the same depending on how much the length of the clip actually changed by.
					FLOAT ActualTimeChange = (AnimSeq.AnimStartOffset - OriginalKey->AnimStartOffset) / AnimSeq.AnimPlayRate;
					FLOAT NewLength = Max<FLOAT>(KINDA_SMALL_NUMBER, (ActualLengthScaled + ActualTimeChange));
					AnimSeq.AnimPlayRate = Max<FLOAT>(KINDA_SMALL_NUMBER, ActualLength / NewLength);
				}
				else
				{
					AnimSeq.AnimStartOffset = OriginalKey->AnimStartOffset + TimeDelta * AnimSeq.AnimPlayRate;
					AnimSeq.AnimStartOffset = Clamp<FLOAT>(AnimSeq.AnimStartOffset, 0, Seq->SequenceLength-AnimSeq.AnimEndOffset);
					AnimSeq.StartTime = OriginalKey->StartTime + (AnimSeq.AnimStartOffset - OriginalKey->AnimStartOffset) / AnimSeq.AnimPlayRate;
				}
				break;
			case ACDT_AnimBlockRightEdge:

				// If ctrl is down we are scaling play time, otherwise we are clipping.
				if(InputData.bCtrlDown)
				{
					FLOAT NewLength = Max<FLOAT>(KINDA_SMALL_NUMBER, (ActualLengthScaled + TimeDelta));
					AnimSeq.AnimPlayRate = Max<FLOAT>(KINDA_SMALL_NUMBER, ActualLength / NewLength);
				}
				else if(InputData.bAltDown)
				{
					// We are changing the offset but then scaling the animation proportionately so that the start and end times don't change
					AnimSeq.AnimEndOffset = OriginalKey->AnimEndOffset - TimeDelta * AnimSeq.AnimPlayRate;
					AnimSeq.AnimEndOffset = Clamp<FLOAT>(AnimSeq.AnimEndOffset, 0, Seq->SequenceLength-AnimSeq.AnimStartOffset);

					// Fix the play rate to keep the start and end times the same depending on how much the length of the clip actually changed by.
					FLOAT ActualTimeChange = (AnimSeq.AnimEndOffset - OriginalKey->AnimEndOffset) / AnimSeq.AnimPlayRate;
					FLOAT NewLength = Max<FLOAT>(KINDA_SMALL_NUMBER, (ActualLengthScaled + ActualTimeChange));
					AnimSeq.AnimPlayRate = Max<FLOAT>(KINDA_SMALL_NUMBER, ActualLength / NewLength);
				}
				else
				{
					AnimSeq.AnimEndOffset = OriginalKey->AnimEndOffset - TimeDelta * AnimSeq.AnimPlayRate;
					AnimSeq.AnimEndOffset = Clamp<FLOAT>(AnimSeq.AnimEndOffset, 0, Seq->SequenceLength-AnimSeq.AnimStartOffset);
				}
				break;
			}

			// @todo: Support Undo/Redo for drag-based edits
			MarkPackageDirty();
		}
	}
}

/** 
 *	Assumes RI->Origin is in the correct place to start drawing this track.
 *	SelectedKeyIndex is INDEX_NONE if no key is selected.
 */
void UInterpTrackAnimControl::DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params )
{
	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());
	UInterpData* Data = CastChecked<UInterpData>(Group->GetOuter());
	
	const FColor NormalBlockColor(0,100,200);
	const FColor ReversedBlockColor(100,50,200);

	// Draw the coloured block for each animation.
	FString TimeCursorString;
	for(INT i=0; i<AnimSeqs.Num(); i++)
	{
		const FAnimControlTrackKey& CurKey = AnimSeqs(i);

		FLOAT SeqStartTime = CurKey.StartTime;
		FLOAT SeqEndTime = SeqStartTime;

		FLOAT SeqLength = 0.f;
		UAnimSequence* Seq = FindAnimSequenceFromName(CurKey.AnimSeqName);
		if(Seq)
		{
			SeqLength = ::Max((Seq->SequenceLength - (CurKey.AnimStartOffset + CurKey.AnimEndOffset)) / CurKey.AnimPlayRate, 0.01f);
			SeqEndTime += SeqLength;
		}

		// If there is a sequence following this one - we stop drawing this block where the next one begins.
		FLOAT LoopEndTime = SeqEndTime;
		if(i < AnimSeqs.Num()-1)
		{
			LoopEndTime = AnimSeqs(i+1).StartTime;
			SeqEndTime = ::Min( AnimSeqs(i+1).StartTime, SeqEndTime );
		}
		else
		{
			LoopEndTime = Data->InterpLength;
		}

		INT StartPixelPos = appTrunc((SeqStartTime - Params.StartTime) * Params.PixelsPerSec);
		INT EndPixelPos = appTrunc((SeqEndTime - Params.StartTime) * Params.PixelsPerSec);

		// Find if this key is one of the selected ones.
		UBOOL bKeySelected = false;
		for(INT j=0; j<Params.SelectedKeys.Num() && !bKeySelected; j++)
		{
			if( Params.SelectedKeys(j).Group == Group && 
				Params.SelectedKeys(j).TrackIndex == Params.TrackIndex && 
				Params.SelectedKeys(j).KeyIndex == i )
				bKeySelected = true;
		}

		// Draw border orange if animation is selected.
		FColor BorderColor = bKeySelected ? KeySelectedColor : FColor(0,0,0);

		if( Seq && CurKey.bLooping )
		{
			INT LoopEndPixelPos = appCeil((LoopEndTime - Params.StartTime) * Params.PixelsPerSec);

			DrawTile(Canvas, StartPixelPos, KeyVertOffset, LoopEndPixelPos - StartPixelPos, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset), 0.f, 0.f, 1.f, 1.f, FColor(0,0,0) );
			DrawTile(Canvas, StartPixelPos+1, KeyVertOffset+1, LoopEndPixelPos - StartPixelPos - 1, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset) - 2, 0.f, 0.f, 1.f, 1.f, FColor(0,75,150) );

			check(CurKey.AnimPlayRate > KINDA_SMALL_NUMBER);
			FLOAT LoopTime = SeqEndTime + SeqLength;
			while(LoopTime < LoopEndTime)
			{
				INT DashPixelPos = appTrunc((LoopTime - Params.StartTime) * Params.PixelsPerSec);
				DrawLine2D(Canvas, FVector2D(DashPixelPos, KeyVertOffset + 2), FVector2D(DashPixelPos, Params.TrackHeight - KeyVertOffset - 2), FColor(0,0,0) );
				LoopTime += SeqLength;
			}
		}

		if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HInterpTrackKeypointProxy(Group, Params.TrackIndex, i) );
		
		// Draw background blocks
		DrawTile(Canvas, StartPixelPos, KeyVertOffset, EndPixelPos - StartPixelPos + 1, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset), 0.f, 0.f, 1.f, 1.f, BorderColor );

		// If the current key is reversed then change the color of the block.
		FColor BlockColor;

		if(CurKey.bReverse)
		{
			BlockColor = ReversedBlockColor;
		}
		else
		{
			BlockColor = NormalBlockColor;
		}

		DrawTile(Canvas, StartPixelPos+1, KeyVertOffset+1, EndPixelPos - StartPixelPos - 1, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset) - 2, 0.f, 0.f, 1.f, 1.f,  BlockColor);

		// Draw edge hit proxies if we are selected.
		if(bKeySelected)
		{
			// Left Edge
			Canvas->SetHitProxy(new HInterpEdInputInterface(this, FInterpEdInputData(ACDT_AnimBlockLeftEdge, i)));
			DrawTile(Canvas, StartPixelPos-2, KeyVertOffset, 4, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset), 0.f, 0.f, 1.f, 1.f, FLinearColor::Black );

			// Right Edge
			Canvas->SetHitProxy(new HInterpEdInputInterface(this, FInterpEdInputData(ACDT_AnimBlockRightEdge, i)));
			DrawTile(Canvas, EndPixelPos-1, KeyVertOffset, 4, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset), 0.f, 0.f, 1.f, 1.f, FLinearColor::Black );

			Canvas->SetHitProxy(NULL);
		}


		// Check to see if we should draw a positional info about this key next to the time cursor
		if( Params.bShowTimeCursorPosForAllKeys || bKeySelected )
		{
			const FLOAT VisibleEndTime = CurKey.bLooping ? LoopEndTime : SeqEndTime;
			if( Params.TimeCursorPosition >= CurKey.StartTime && Params.TimeCursorPosition <= VisibleEndTime )
			{
				const FLOAT CursorPosWithinAnim =
					( Params.TimeCursorPosition - CurKey.StartTime ) + CurKey.AnimStartOffset;

				// Does the user want us to draw frame numbers instead of time values?
				if( Params.bPreferFrameNumbers && Params.SnapAmount > KINDA_SMALL_NUMBER )
				{
					// Convert to the animation time values to frame numbers
					const INT CursorFrameWithinAnim = appTrunc( CursorPosWithinAnim / Params.SnapAmount );
					TimeCursorString = FString::Printf( TEXT( "%i" ), CursorFrameWithinAnim );
				}
				else
				{
					TimeCursorString = FString::Printf( TEXT( "%2.2f" ), CursorPosWithinAnim );
				}
			}
		}
	}

	// Use base-class to draw key triangles
	Super::DrawTrack( Canvas, Params );

	// Draw anim sequence name for each block on top.
	for(INT i=0; i<AnimSeqs.Num(); i++)
	{
		const FAnimControlTrackKey& CurKey = AnimSeqs(i);

		UBOOL bKeySelected = false;
		for(INT j=0; j<Params.SelectedKeys.Num() && !bKeySelected; j++)
		{
			if( Params.SelectedKeys(j).Group == Group && 
				Params.SelectedKeys(j).TrackIndex == Params.TrackIndex && 
				Params.SelectedKeys(j).KeyIndex == i )
				bKeySelected = true;
		}

		FLOAT SeqStartTime = CurKey.StartTime;
		INT PixelPos = appTrunc((SeqStartTime - Params.StartTime) * Params.PixelsPerSec);

		FString SeqString = CurKey.AnimSeqName.ToString();
		UAnimSequence* Seq = FindAnimSequenceFromName(CurKey.AnimSeqName);

		if(bKeySelected && Seq)
		{
			if(CurKey.AnimStartOffset > 0.f || CurKey.AnimEndOffset > 0.f)
			{
				// Does the user want us to draw frame numbers instead of time values?
				if( Params.bPreferFrameNumbers && Params.SnapAmount > KINDA_SMALL_NUMBER )
				{
					// Convert to the animation time values to frame numbers
					const INT AnimFrameOffsetFromStart = appRound( CurKey.AnimStartOffset / Params.SnapAmount );
					const INT AnimFrameOffsetFromEnd = appRound( ( Seq->SequenceLength - CurKey.AnimEndOffset ) / Params.SnapAmount );
					SeqString += FString::Printf( TEXT(" (%i->%i)"), AnimFrameOffsetFromStart, AnimFrameOffsetFromEnd );
				}
				else
				{
					SeqString += FString::Printf( TEXT(" (%2.2f->%2.2f)"), CurKey.AnimStartOffset, Seq->SequenceLength - CurKey.AnimEndOffset );
				}
			}

			if(CurKey.AnimPlayRate != 1.f)
			{
				SeqString += FString::Printf( TEXT(" x%2.2f"), CurKey.AnimPlayRate );
			}

			if(CurKey.bReverse)
			{
				SeqString += FString::Printf( TEXT(" (%s)"), *LocalizeUnrealEd("Reverse"));
			}
		}

		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, *SeqString );

		DrawShadowedString(Canvas, PixelPos + 2, Params.TrackHeight - YL - KeyVertOffset, *SeqString, GEngine->SmallFont, KeyLabelColor );
	}


	// Draw the time cursor's position relative to the start of this animation.  We'll draw this
	// right next to the time cursor, on top of this anim track
	if( TimeCursorString.Len() > 0 )
	{
		// Visual settings
		const FLinearColor BackgroundColor( 0.0f, 0.015f, 0.05f, 0.75f );
		const FLinearColor BorderColor( 0.35f, 0.35f, 0.4f, 1.0f );	// NOTE: Opacity is ignored for lines
		const FColor TextColor( 255, 255, 255 );
		const FLOAT TextScale = 0.9f;
		const INT TextHorizOffset = 6;
		const INT TextVertOffset = 6;


		const INT TimeCursorPixelPos =
			appTrunc( ( Params.TimeCursorPosition - Params.StartTime ) * Params.PixelsPerSec );

		INT XL, YL;
		StringSize( GEngine->TinyFont, XL, YL, *TimeCursorString );
		const FLOAT TextWidth = XL * TextScale;
		const FLOAT TextHeight = YL * TextScale;

		// Draw background
		const FLOAT BoxTop = TextVertOffset -( TextHeight + 2 );
		const FLOAT BoxLeft = TimeCursorPixelPos + TextHorizOffset - 2;
		DrawTile(
			Canvas,
			BoxLeft,
			BoxTop,
			TextWidth + 4,
			TextHeight + 3,
			0.0f, 0.0f,
			1.0f, 1.0f,
			BackgroundColor );

		// Draw border
		DrawBox2D(
			Canvas,
			FVector2D( BoxLeft, BoxTop ),
			FVector2D( BoxLeft + TextWidth + 4, BoxTop + TextHeight + 3 ),
			BorderColor );

		// Draw text
		DrawString(
			Canvas,
			TimeCursorPixelPos + TextHorizOffset,
			TextVertOffset - TextHeight,
			*TimeCursorString,
			GEngine->TinyFont,
			TextColor,
			TextScale,
			TextScale );
	}
}

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackAnimControl::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Anim_Mat"), NULL, LOAD_None, NULL );
}

/*-----------------------------------------------------------------------------
	UInterpTrackFaceFX
-----------------------------------------------------------------------------*/

/** 
*	Assumes RI->Origin is in the correct place to start drawing this track.
*	SelectedKeyIndex is INDEX_NONE if no key is selected.
*/
void UInterpTrackFaceFX::DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params )
{
	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());
	UBOOL bHitTesting = Canvas->IsHitTesting();

	// Draw the coloured block for each face fx animation.
	for(INT i=0; i<FaceFXSeqs.Num(); i++)
	{
		FLOAT FaceFXStartTime = FaceFXSeqs(i).StartTime;
		FLOAT FaceFXEndTime = FaceFXStartTime;

		FFaceFXTrackKey& Key = FaceFXSeqs(i);

#if WITH_FACEFX
		if(CachedActorFXAsset)
		{
			// Get the FxActor
			FxActor* fActor = CachedActorFXAsset->GetFxActor();

			// Find the Group by name
			FxSize GroupIndex = fActor->FindAnimGroup(TCHAR_TO_ANSI(*(Key.FaceFXGroupName)));
			if(FxInvalidIndex != GroupIndex)
			{
				FxAnimGroup& fGroup = fActor->GetAnimGroup(GroupIndex);

				// Find the animation by name
				FxSize SeqIndex = fGroup.FindAnim(TCHAR_TO_ANSI(*(Key.FaceFXSeqName)));
				if(FxInvalidIndex != SeqIndex)
				{
					const FxAnim& fAnim = fGroup.GetAnim(SeqIndex);
					FaceFXEndTime += fAnim.GetDuration();
				}
			}
		}
#endif

		// Truncate animation at next anim in the track.
		if(i < FaceFXSeqs.Num()-1)
		{
			FaceFXEndTime = ::Min( FaceFXSeqs(i+1).StartTime, FaceFXEndTime );
		}


		INT StartPixelPos = appTrunc((FaceFXStartTime - Params.StartTime) * Params.PixelsPerSec);
		INT EndPixelPos = appTrunc((FaceFXEndTime - Params.StartTime) * Params.PixelsPerSec);

		// Find if this sound is one of the selected ones.
		UBOOL bKeySelected = FALSE;
		for(INT j=0; j<Params.SelectedKeys.Num() && !bKeySelected; j++)
		{
			if( Params.SelectedKeys(j).Group == Group && 
				Params.SelectedKeys(j).TrackIndex == Params.TrackIndex && 
				Params.SelectedKeys(j).KeyIndex == i )
				bKeySelected = TRUE;
		}

		// Draw border orange if sound is selected.
		FColor BorderColor = bKeySelected ? KeySelectedColor : FColor(0,0,0);

		//if(bHitTesting) Canvas->SetHitProxy( new HInterpTrackKeypointProxy(Group, Params.TrackIndex, i) );
		DrawTile( Canvas, StartPixelPos, KeyVertOffset, EndPixelPos - StartPixelPos + 1, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset), 0.f, 0.f, 1.f, 1.f, BorderColor );
		DrawTile( Canvas, StartPixelPos+1, KeyVertOffset+1, EndPixelPos - StartPixelPos - 1, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset) - 2, 0.f, 0.f, 1.f, 1.f, FColor(0,150,220) );
		//if(bHitTesting) Canvas->SetHitProxy(NULL);
	}

	// Use base-class to draw key triangles
	Super::DrawTrack( Canvas, Params );

	// Draw FaceFX sequence name for each key.
	for(INT i=0; i<FaceFXSeqs.Num(); i++)
	{
		UBOOL bKeySelected = FALSE;
		for(INT j=0; j<Params.SelectedKeys.Num() && !bKeySelected; j++)
		{
			if( Params.SelectedKeys(j).Group == Group && 
				Params.SelectedKeys(j).TrackIndex == Params.TrackIndex && 
				Params.SelectedKeys(j).KeyIndex == i )
				bKeySelected = TRUE;
		}

		FLOAT SeqStartTime = FaceFXSeqs(i).StartTime;
		INT PixelPos = appTrunc((SeqStartTime - Params.StartTime) * Params.PixelsPerSec);

		FString SeqString = FString::Printf( TEXT("%s.%s"), *(FaceFXSeqs(i).FaceFXGroupName), *(FaceFXSeqs(i).FaceFXSeqName) );

		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, *SeqString );

		DrawShadowedString(Canvas, PixelPos + 2, Params.TrackHeight - YL - KeyVertOffset, *SeqString, GEngine->SmallFont, KeyLabelColor );
	}
}

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackFaceFX::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Anim_Mat"), NULL, LOAD_None, NULL );
}

/*-----------------------------------------------------------------------------
	UInterpTrackSound
-----------------------------------------------------------------------------*/

/** 
*	Assumes RI->Origin is in the correct place to start drawing this track.
*	SelectedKeyIndex is INDEX_NONE if no key is selected.
*/
void UInterpTrackSound::DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params )
{
	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());
	UInterpData* Data = CastChecked<UInterpData>(Group->GetOuter());

	// Draw the coloured block for each sound.
	for(INT i=0; i<Sounds.Num(); i++)
	{
		FLOAT SoundStartTime = Sounds(i).Time;
		FLOAT SoundEndTime = SoundStartTime;

		// Make block as long as the SoundCue is.
		USoundCue* Cue = Sounds(i).Sound;
		if(Cue)
		{
			SoundEndTime += Cue->GetCueDuration();
		}

		// Truncate sound cue at next sound in the track.
		if(i < Sounds.Num()-1)
		{
			SoundEndTime = ::Min( Sounds(i+1).Time, SoundEndTime );
		}


		INT StartPixelPos = appTrunc((SoundStartTime - Params.StartTime) * Params.PixelsPerSec);
		INT EndPixelPos = appTrunc((SoundEndTime - Params.StartTime) * Params.PixelsPerSec);

		// Find if this sound is one of the selected ones.
		UBOOL bKeySelected = false;
		for(INT j=0; j<Params.SelectedKeys.Num() && !bKeySelected; j++)
		{
			if( Params.SelectedKeys(j).Group == Group && 
				Params.SelectedKeys(j).TrackIndex == Params.TrackIndex && 
				Params.SelectedKeys(j).KeyIndex == i )
				bKeySelected = true;
		}

		// Draw border orange if sound is selected.
		FColor BorderColor = bKeySelected ? KeySelectedColor : FColor(0,0,0);

		//if(bHitTesting) Canvas->SetHitProxy( new HInterpTrackKeypointProxy(Group, Params.TrackIndex, i) );
		DrawTile(Canvas, StartPixelPos, KeyVertOffset, EndPixelPos - StartPixelPos + 1, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset), 0.f, 0.f, 1.f, 1.f, BorderColor );
		DrawTile(Canvas, StartPixelPos+1, KeyVertOffset+1, EndPixelPos - StartPixelPos - 1, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset) - 2, 0.f, 0.f, 1.f, 1.f, FColor(0,200,100) );
		//if(bHitTesting) Canvas->SetHitProxy(NULL);
	}

	// Use base-class to draw key triangles
	Super::DrawTrack( Canvas, Params );

	// Draw sound cue name for each block on top.
	for(INT i=0; i<Sounds.Num(); i++)
	{
		FLOAT SoundStartTime = Sounds(i).Time;
		INT PixelPos = appTrunc((SoundStartTime - Params.StartTime) * Params.PixelsPerSec);

		USoundCue* Cue = Sounds(i).Sound;
	
		FString SoundString( TEXT("None") );
		if(Cue)
		{
			SoundString = FString( *Cue->GetName() );
			if ( Sounds(i).Volume != 1.0f )
			{
				SoundString += FString::Printf( TEXT(" v%2.2f"), Sounds(i).Volume );
			}
			if ( Sounds(i).Pitch != 1.0f )
			{
				SoundString += FString::Printf( TEXT(" p%2.2f"), Sounds(i).Pitch );
			}
		}
		
		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, *SoundString );

		DrawShadowedString(Canvas, PixelPos + 2, Params.TrackHeight - YL - KeyVertOffset, *SoundString, GEngine->SmallFont, KeyLabelColor );
	}
}

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackSound::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Sound_Mat"), NULL, LOAD_None, NULL );
}

/*-----------------------------------------------------------------------------
	UInterpTrackFade
-----------------------------------------------------------------------------*/

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackFade::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Fade_Mat"), NULL, LOAD_None, NULL );
}


/*-----------------------------------------------------------------------------
	UInterpTrackSlomo
-----------------------------------------------------------------------------*/

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackSlomo::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Slomo_Mat"), NULL, LOAD_None, NULL );
}

/*-----------------------------------------------------------------------------
	UInterpTrackColorScale
-----------------------------------------------------------------------------*/

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackColorScale::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Fade_Mat"), NULL, LOAD_None, NULL );
}


/*-----------------------------------------------------------------------------
	UInterpTrackAudioMaster
-----------------------------------------------------------------------------*/

/** Get the icon to draw for this track in Matinee. */
UMaterial* UInterpTrackAudioMaster::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_AudioMaster_Mat"), NULL, LOAD_None, NULL );
}



/*-----------------------------------------------------------------------------
	UInterpTrackVisibility
-----------------------------------------------------------------------------*/
UMaterial* UInterpTrackVisibility::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_Visibility_Mat"), NULL, LOAD_None, NULL );
}

void UInterpTrackVisibility::DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params )
{
	INT NumKeys = GetNumKeyframes();

	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());

	// Draw the 'on' blocks in green
	INT LastPixelPos = -1;
	UBOOL bLastPosWasOn = FALSE;
	for (INT i=0; i<NumKeys; i++)
	{
		FLOAT KeyTime = GetKeyframeTime(i);
		INT PixelPos = appTrunc((KeyTime - Params.StartTime) * Params.PixelsPerSec);

		FVisibilityTrackKey& Key = VisibilityTrack(i);
		if ((Key.Action == EVTA_Hide) && bLastPosWasOn)
		{
			DrawTile(Canvas, LastPixelPos, KeyVertOffset, PixelPos - LastPixelPos, Params.TrackHeight - (2 * KeyVertOffset), 
				0.0f, 0.0f, 0.0f, 0.0f, FLinearColor(0.0f, 1.0f, 0.0f));
		}

		LastPixelPos = PixelPos;
		bLastPosWasOn = (Key.Action == EVTA_Show) ? TRUE : FALSE;
	}

	// Draw the keyframe points after, so they are on top
	for (INT i=0; i<NumKeys; i++)
	{
		FLOAT KeyTime = GetKeyframeTime(i);

		INT PixelPos = appTrunc((KeyTime - Params.StartTime) * Params.PixelsPerSec);

		FIntPoint A, A_Offset;
		FIntPoint B, B_Offset;
		FIntPoint C, C_Offset;

		FVisibilityTrackKey& Key = VisibilityTrack(i);
		if (Key.Action == EVTA_Hide)
		{
			// Point the triangle down...
			A = FIntPoint(PixelPos - KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset - KeyHalfTriSize);
			B = FIntPoint(PixelPos + KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset - KeyHalfTriSize);
			C = FIntPoint(PixelPos,						Params.TrackHeight - KeyVertOffset);

			A_Offset = FIntPoint(-2,-2);
			B_Offset = FIntPoint( 2,-2);
			C_Offset = FIntPoint( 0, 1);
		}
		else if (Key.Action == EVTA_Toggle)
		{
			// Point the triangle up
			A = FIntPoint(PixelPos - KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
			B = FIntPoint(PixelPos + KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
			C = FIntPoint(PixelPos,						Params.TrackHeight - KeyVertOffset - KeyHalfTriSize);

			A_Offset = FIntPoint(-2, 1);
			B_Offset = FIntPoint( 2, 1);
			C_Offset = FIntPoint( 0,-2);

			DrawTile(Canvas, PixelPos - 4, KeyVertOffset, 7, Params.TrackHeight - (2 * KeyVertOffset), 
				0.0f, 0.0f, 0.0f, 0.0f, FLinearColor(1.0f, 0.0f, 0.0f));
		}
		else
		{
			// Point the triangle up
			A = FIntPoint(PixelPos - KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
			B = FIntPoint(PixelPos + KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
			C = FIntPoint(PixelPos,						Params.TrackHeight - KeyVertOffset - KeyHalfTriSize);

			A_Offset = FIntPoint(-2, 1);
			B_Offset = FIntPoint( 2, 1);
			C_Offset = FIntPoint( 0,-2);
		}

		UBOOL bKeySelected = false;
		for (INT j=0; j<Params.SelectedKeys.Num() && !bKeySelected; j++)
		{
			if (Params.SelectedKeys(j).Group == Group && 
				Params.SelectedKeys(j).TrackIndex == Params.TrackIndex && 
				Params.SelectedKeys(j).KeyIndex == i)
			{
				bKeySelected = true;
			}
		}

		FColor KeyColor = GetKeyframeColor(i);
		
		if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HInterpTrackKeypointProxy(Group, Params.TrackIndex, i) );
		if (bKeySelected)
		{
			DrawTriangle2D(Canvas, A + A_Offset, FVector2D(0,0), B + B_Offset, FVector2D(0,0), C + C_Offset, FVector2D(0,0), KeySelectedColor );
		}

		DrawTriangle2D(Canvas, A, FVector2D(0,0), B, FVector2D(0,0), C, FVector2D(0,0), KeyColor );
		if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );
	}
}



/*-----------------------------------------------------------------------------
	UInterpTrackParticleReplay
-----------------------------------------------------------------------------*/
UMaterial* UInterpTrackParticleReplay::GetTrackIcon()
{
	return (UMaterial*)StaticLoadObject( UMaterial::StaticClass(), NULL, TEXT("EditorMaterials.MatineeGroups.MAT_Groups_ParticleReplay_Mat"), NULL, LOAD_None, NULL );
}


/** Lets the interface object know that we are beginning a drag operation. */
void UInterpTrackParticleReplay::BeginDrag(FInterpEdInputData &InputData)
{
	// Store temporary data.
	if((InputData.InputType == PRDT_LeftEdge || InputData.InputType == PRDT_RightEdge) && TrackKeys.IsValidIndex(InputData.InputData))
	{
		// Store our starting position.
		FParticleReplayTrackKey* SavedKey = new FParticleReplayTrackKey();
		*SavedKey = TrackKeys( InputData.InputData );
		InputData.TempData = SavedKey;
	}
}

/** Lets the interface object know that we are ending a drag operation. */
void UInterpTrackParticleReplay::EndDrag(FInterpEdInputData &InputData)
{
	// Clean up our temporary data.
	if(InputData.TempData)
	{
		FParticleReplayTrackKey* InterpKey = (FParticleReplayTrackKey*)InputData.TempData;
		delete InterpKey;
		InputData.TempData = NULL;
	}
}


/** @return Returns the mouse cursor to display when this input interface is moused over. */
EMouseCursor UInterpTrackParticleReplay::GetMouseCursor(FInterpEdInputData &InputData)
{
	EMouseCursor Result = MC_NoChange;

	switch(InputData.InputType)
	{
		case PRDT_LeftEdge:
		case PRDT_RightEdge:
			Result = MC_SizeLeftRight;
			break;
	}

	return Result;
}


/** Called when an object is dragged. */
void UInterpTrackParticleReplay::ObjectDragged(FInterpEdInputData& InputData)
{
	if(TrackKeys.IsValidIndex(InputData.InputData) && InputData.TempData)
	{
		const FParticleReplayTrackKey& OriginalKey = *(FParticleReplayTrackKey*)InputData.TempData;
		FParticleReplayTrackKey& SelectedKey = TrackKeys(InputData.InputData);
		FIntPoint Delta = InputData.MouseCurrent - InputData.MouseStart;
		FLOAT TimeDelta = Delta.X / InputData.PixelsPerSec;
		{
			switch(InputData.InputType)
			{
				case PRDT_LeftEdge:
					{
						SelectedKey.Time = OriginalKey.Time + TimeDelta;

						// Snap the new time position
						if( FixedTimeStep > SMALL_NUMBER )
						{
							const INT InterpPositionInFrames = appRound( SelectedKey.Time / FixedTimeStep );
							SelectedKey.Time = InterpPositionInFrames * FixedTimeStep;
						}

						if( SelectedKey.Time > OriginalKey.Time + OriginalKey.Duration )
						{
							SelectedKey.Time = OriginalKey.Time + OriginalKey.Duration;
						}
						FLOAT NewDelta = SelectedKey.Time - OriginalKey.Time;

						SelectedKey.Duration = OriginalKey.Duration - NewDelta;
						if( SelectedKey.Duration < 0.0f )
						{
							SelectedKey.Duration = 0.0f;
						}
					}
					break;

				case PRDT_RightEdge:
					{
						SelectedKey.Duration = OriginalKey.Duration + TimeDelta;
						if( SelectedKey.Duration < 0.0f )
						{
							SelectedKey.Duration = 0.0f;
						}

						// Snap the new end position
						if( FixedTimeStep > SMALL_NUMBER )
						{
							FLOAT EndTime = SelectedKey.Time + SelectedKey.Duration;

							const INT InterpPositionInFrames = appRound( EndTime / FixedTimeStep );
							EndTime = InterpPositionInFrames * FixedTimeStep;

							if( EndTime < SelectedKey.Time )
							{
								EndTime = SelectedKey.Time;
							}
							SelectedKey.Duration = EndTime - SelectedKey.Time;
						}
					}
					break;
			}
		}

		// @todo: Support Undo/Redo for drag-based edits
		MarkPackageDirty();
	}
}


void UInterpTrackParticleReplay::DrawTrack( FCanvas* Canvas, const FInterpTrackDrawParams& Params )
{
	UInterpGroup* Group = CastChecked<UInterpGroup>(GetOuter());

	const INT NumKeys = GetNumKeyframes();
	for( INT CurKeyIndex = 0; CurKeyIndex < NumKeys; ++CurKeyIndex )
	{
		const FParticleReplayTrackKey& CurKey = TrackKeys( CurKeyIndex );

		FLOAT KeyTime = GetKeyframeTime( CurKeyIndex );

		const INT StartPixelPos = appTrunc( ( KeyTime - Params.StartTime ) * Params.PixelsPerSec );
		const INT EndPixelPos = appTrunc( ( KeyTime - Params.StartTime + CurKey.Duration ) * Params.PixelsPerSec );


		// Is this key selected?
		UBOOL bKeySelected = false;
		for (INT j=0; j<Params.SelectedKeys.Num() && !bKeySelected; j++)
		{
			if( Params.SelectedKeys(j).Group == Group && 
				Params.SelectedKeys(j).TrackIndex == Params.TrackIndex && 
				Params.SelectedKeys(j).KeyIndex == CurKeyIndex )
			{
				bKeySelected = true;
			}
		}


		// Draw background tile for the capture/playback range
		{
			FColor BackgroundTileColor = bKeySelected ? KeySelectedColor : FColor(80,00,80);
			if( bIsCapturingReplay )
			{
				// When capturing make the background color more red
				BackgroundTileColor.R = 200;
			}
			
			if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HInterpTrackKeypointProxy(Group, Params.TrackIndex, CurKeyIndex) );

			DrawTile(
				Canvas,
				StartPixelPos,
				KeyVertOffset,
				EndPixelPos - StartPixelPos + 1,
				appTrunc( Params.TrackHeight - 2.0f * KeyVertOffset ),
				0.f, 0.f, 1.f, 1.f,
				BackgroundTileColor );
		}


		// Draw edge hit proxies if we are selected.
		if( bKeySelected )
		{
			// Left Edge
			Canvas->SetHitProxy(new HInterpEdInputInterface(this, FInterpEdInputData(PRDT_LeftEdge, CurKeyIndex)));
			DrawTile(Canvas, StartPixelPos-2, KeyVertOffset, 4, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset), 0.f, 0.f, 1.f, 1.f, FLinearColor::Black );

			// Right Edge
			Canvas->SetHitProxy(new HInterpEdInputInterface(this, FInterpEdInputData(PRDT_RightEdge, CurKeyIndex)));
			DrawTile(Canvas, EndPixelPos-1, KeyVertOffset, 4, appTrunc(Params.TrackHeight - 2.f*KeyVertOffset), 0.f, 0.f, 1.f, 1.f, FLinearColor::Black );

			Canvas->SetHitProxy(NULL);
		}


		// Draw key frame information text
		{
			const INT DurationInFrames = appRound( CurKey.Duration / FixedTimeStep );
			FString InfoText = FString::Printf( TEXT( "[Clip %i] %i frames (%.2f s)" ), CurKey.ClipIDNumber, DurationInFrames, CurKey.Duration );

			INT XL, YL;
			StringSize( GEngine->SmallFont, XL, YL, *InfoText );
			DrawShadowedString(Canvas, StartPixelPos + 2, Params.TrackHeight - YL - KeyVertOffset, *InfoText, GEngine->SmallFont, KeyLabelColor );
		}


		// Draw key frame triangle (selectable)
		{
			FIntPoint A, A_Offset;
			FIntPoint B, B_Offset;
			FIntPoint C, C_Offset;

			A = FIntPoint(StartPixelPos - KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
			B = FIntPoint(StartPixelPos + KeyHalfTriSize,	Params.TrackHeight - KeyVertOffset);
			C = FIntPoint(StartPixelPos,					Params.TrackHeight - KeyVertOffset - KeyHalfTriSize);

			A_Offset = FIntPoint(-2, 1);
			B_Offset = FIntPoint( 2, 1);
			C_Offset = FIntPoint( 0,-2);

			FColor KeyColor = GetKeyframeColor( CurKeyIndex );
			
			if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HInterpTrackKeypointProxy(Group, Params.TrackIndex, CurKeyIndex) );
			if (bKeySelected)
			{
				DrawTriangle2D(Canvas, A + A_Offset, FVector2D(0,0), B + B_Offset, FVector2D(0,0), C + C_Offset, FVector2D(0,0), KeySelectedColor );
			}

			DrawTriangle2D(Canvas, A, FVector2D(0,0), B, FVector2D(0,0), C, FVector2D(0,0), KeyColor );
			if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );
		}
	}
}

