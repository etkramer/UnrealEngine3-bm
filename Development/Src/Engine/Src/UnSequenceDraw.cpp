/*=============================================================================
	UnSequenceDraw.cpp: Utils for drawing sequence objects.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineSequenceClasses.h"
#include "UnLinkedObjDrawUtils.h"

#define LO_CIRCLE_SIDES			16

static const FColor SelectedColor(255,255,0);
static const FColor MouseOverLogicColor(225,225,0);
static const FLOAT	MouseOverColorScale(0.8f);
static const FColor	TitleBkgColor(112,112,112);
static const FColor	SequenceTitleBkgColor(112,112,200);
static const FColor	MatineeTitleBkgColor(200,112,0);


//-----------------------------------------------------------------------------
//	USequence
//-----------------------------------------------------------------------------

// Draw an entire gameplay sequence
void USequence::DrawSequence(FCanvas* Canvas, TArray<USequenceObject*>& SelectedSeqObjs, USequenceObject* MouseOverSeqObj, INT MouseOverConnType, INT MouseOverConnIndex, FLOAT MouseOverTime, UBOOL bCurves)
{
	// Cache sequence object selection status.
	TArray<UBOOL> SelectionStatus;
	for(INT i=0; i<SequenceObjects.Num(); i++)
	{
		const UBOOL bSelected = SelectedSeqObjs.ContainsItem( SequenceObjects(i) );
		SelectionStatus.AddItem( bSelected );
	}

	// draw first 
	for(INT i=0; i<SequenceObjects.Num(); i++)
	{
		if( SequenceObjects(i)->bDrawFirst )
		{
			const UBOOL bSelected = SelectionStatus(i);
			const UBOOL bMouseOver = (SequenceObjects(i) == MouseOverSeqObj);
			SequenceObjects(i)->DrawSeqObj(Canvas, bSelected, bMouseOver, MouseOverConnType, MouseOverConnIndex, MouseOverTime);
		}
	}

	// first pass draw most sequence ops
	for(INT i=0; i<SequenceObjects.Num(); i++)
	{
		if( !SequenceObjects(i)->bDrawFirst && !SequenceObjects(i)->bDrawLast )
		{
			const UBOOL bSelected = SelectionStatus(i);
			const UBOOL bMouseOver = (SequenceObjects(i) == MouseOverSeqObj);
			SequenceObjects(i)->DrawSeqObj(Canvas, bSelected, bMouseOver, MouseOverConnType, MouseOverConnIndex, MouseOverTime);
		}
	}

	// draw logic and variable Links
	for (INT i = 0; i < SequenceObjects.Num(); i++)
	{
		SequenceObjects(i)->DrawLogicLinks(Canvas, bCurves, SelectedSeqObjs, MouseOverSeqObj, MouseOverConnType, MouseOverConnIndex);
		SequenceObjects(i)->DrawVariableLinks(Canvas, bCurves, SelectedSeqObjs, MouseOverSeqObj, MouseOverConnType, MouseOverConnIndex);
	}

	// draw final layer, for variables etc
	for(INT i=0; i<SequenceObjects.Num(); i++)
	{
		if( SequenceObjects(i)->bDrawLast )
		{
			const UBOOL bSelected = SelectionStatus(i);
			const UBOOL bMouseOver = (SequenceObjects(i) == MouseOverSeqObj);
			SequenceObjects(i)->DrawSeqObj(Canvas, bSelected, bMouseOver, MouseOverConnType, MouseOverConnIndex, MouseOverTime);
		}
	}
}

//-----------------------------------------------------------------------------
//	USequenceObject
//-----------------------------------------------------------------------------

FColor USequenceObject::GetBorderColor(UBOOL bSelected, UBOOL bMouseOver)
{
	if( bSelected )
	{
		return FColor(255,255,0);
	}
	else
	{
		if(bMouseOver)
		{
			return ObjColor;
		}
		else
		{
			return FColor( FLinearColor(ObjColor) * MouseOverColorScale );
		}
	}
}

FIntPoint USequenceObject::GetTitleBarSize(FCanvas* Canvas)
{
	// remove the category from the title
	FString Title = GetDisplayTitle();
	return FLinkedObjDrawUtils::GetTitleBarSize(Canvas, *Title);
}

FString USequenceObject::GetDisplayTitle() const
{
	return ObjName;
}

void USequenceObject::DrawTitleBar(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, const FIntPoint& Pos, const FIntPoint& Size)
{
	FColor BkgColor = TitleBkgColor;
	if( this->IsA(USequence::StaticClass()) )
	{
		BkgColor = SequenceTitleBkgColor;
	}
	else if( this->IsA(USeqAct_Interp::StaticClass()) )
	{
		BkgColor = MatineeTitleBkgColor;
	}
	// remove the category from the title

	FString Title = GetDisplayTitle();

	// check for out of date objects
	if (eventGetObjClassVersion() != ObjInstanceVersion && !IsA(USequence::StaticClass()))
	{
		Title += TEXT(" (Outdated)");
		BkgColor = FColor(128,0,0);
	}

	// Make a string of all the 'extra' information that is desired by the action.
	FString AutoComment;
	if(!bSuppressAutoComment)
	{
		for( TFieldIterator<UProperty> It( GetClass() ); It; ++It )
		{
			UProperty* Property = *It;
			if(Property && (Property->PropertyFlags & CPF_Edit))
			{
				FString MetaString = Property->GetMetaData(TEXT("autocomment"));
				if(MetaString.Len() > 0)
				{
					UNameProperty* NameProp = Cast<UNameProperty>(Property);
					if(NameProp)
					{
						FName* NamePtr = (FName*)(((BYTE*)this) + NameProp->Offset);
						AutoComment += FString::Printf(TEXT("%s=%s "), *(NameProp->GetName()), *(NamePtr->ToString()));
					}

					UStrProperty* StrProp = Cast<UStrProperty>(Property);
					if(StrProp)
					{
						FString* StringPtr = (FString*)(((BYTE*)this) + StrProp->Offset);
						AutoComment += FString::Printf(TEXT("%s=%s "), *(StrProp->GetName()), *(*StringPtr));
					}

					UIntProperty* IntProp = Cast<UIntProperty>(Property);
					if(IntProp)
					{
						INT* IntPtr = (INT*)(((BYTE*)this) + IntProp->Offset);
						AutoComment += FString::Printf(TEXT("%s=%d "), *(IntProp->GetName()), *IntPtr);
					}

					UFloatProperty* FloatProp = Cast<UFloatProperty>(Property);
					if(FloatProp)
					{
						FLOAT* FloatPtr = (FLOAT*)(((BYTE*)this) + FloatProp->Offset);
						AutoComment += FString::Printf(TEXT("%s=%f "), *(FloatProp->GetName()), *FloatPtr);
					}

					UBoolProperty* BoolProp = Cast<UBoolProperty>(Property);
					if(BoolProp)
					{
						UBOOL bValue = *(BITFIELD*)((BYTE*)this + BoolProp->Offset) & BoolProp->BitMask;
						AutoComment += FString::Printf(TEXT("%s=%s "), *(BoolProp->GetName()), bValue ? TEXT("TRUE") : TEXT("FALSE"));
					}
				}
			}
		}
	}

	// draw the title bar
	FLinkedObjDrawUtils::DrawTitleBar( Canvas, Pos, Size, GetBorderColor(bSelected, bMouseOver), BkgColor, *Title, ObjComment.Len() > 0 ? *ObjComment : NULL, AutoComment.Len() > 0 ? *AutoComment : NULL );
}

/** Calculate the bounding box of this sequence object. For use by Kismet. */
FIntRect USequenceObject::GetSeqObjBoundingBox()
{
	return FIntRect(ObjPosX, ObjPosY, ObjPosX + DrawWidth, ObjPosY + DrawHeight);
}

//-----------------------------------------------------------------------------
//	USequenceOp
//-----------------------------------------------------------------------------

FColor USequenceOp::GetConnectionColor( INT ConnType, INT ConnIndex, INT MouseOverConnType, INT MouseOverConnIndex )
{
	if( ConnType == LOC_INPUT )
	{
		if( MouseOverConnType == LOC_INPUT && MouseOverConnIndex == ConnIndex ) 
		{
			return MouseOverLogicColor;
		}
		else if( InputLinks(ConnIndex).bDisabled )
		{
			return FColor(255,0,0);
		}
		else if( InputLinks(ConnIndex).bDisabledPIE )
		{
			return FColor(255,128,0);
		}
	}
	else if( ConnType == LOC_OUTPUT )
	{
		if( MouseOverConnType == LOC_OUTPUT && MouseOverConnIndex == ConnIndex ) 
		{
			return MouseOverLogicColor;
		}
		else if( OutputLinks(ConnIndex).bDisabled )
		{
			return FColor(255,0,0);
		}
		else if( OutputLinks(ConnIndex).bDisabledPIE )
		{
			return FColor(255,128,0);
		}
	}
	else if( ConnType == LOC_VARIABLE )
	{
		FColor VarColor = GetVarConnectorColor(ConnIndex);
		if( MouseOverConnType == LOC_VARIABLE && MouseOverConnIndex == ConnIndex )
		{
			return VarColor;
		}
		else
		{
			return FColor( FLinearColor(VarColor) * MouseOverColorScale );
		}
	}
	else if( ConnType == LOC_EVENT )
	{
		FColor EventColor = FColor(255,0,0);
		if( MouseOverConnType == LOC_EVENT && MouseOverConnIndex == ConnIndex ) 
		{
			return EventColor;
		}
		else
		{ 
			return FColor( FLinearColor(EventColor) * MouseOverColorScale );
		}
	}

	return FColor(0,0,0);
}

void USequenceOp::MakeLinkedObjDrawInfo(FLinkedObjDrawInfo& ObjInfo, INT MouseOverConnType, INT MouseOverConnIndex)
{
	// add all input Links
	for(INT i=0; i<InputLinks.Num(); i++)
	{
		// only add if visible
		if (!InputLinks(i).bHidden)
		{
			const FColor ConnColor = GetConnectionColor( LOC_INPUT, i, MouseOverConnType, MouseOverConnIndex );
			ObjInfo.Inputs.AddItem( FLinkedObjConnInfo(*InputLinks(i).LinkDesc, ConnColor ) );
		}
	}
	// add all output Links
	for(INT i=0; i<OutputLinks.Num(); i++)
	{
		// only add if visible
		if (!OutputLinks(i).bHidden)
		{
			const FColor ConnColor = GetConnectionColor( LOC_OUTPUT, i, MouseOverConnType, MouseOverConnIndex );
			ObjInfo.Outputs.AddItem( FLinkedObjConnInfo(*OutputLinks(i).LinkDesc, ConnColor ) );
		}
	}
	// add all variable Links
	for(INT i=0; i<VariableLinks.Num(); i++)
	{
		// only add if visible
		if (!VariableLinks(i).bHidden)
		{
			const FColor ConnColor = GetConnectionColor( LOC_VARIABLE, i, MouseOverConnType, MouseOverConnIndex );
			ObjInfo.Variables.AddItem( FLinkedObjConnInfo(*VariableLinks(i).LinkDesc, ConnColor, VariableLinks(i).bWriteable) );
		}
	}
	// add all event Links
	for(INT i=0; i<EventLinks.Num(); i++)
	{
		// only add if visible
		if (!EventLinks(i).bHidden)
		{
			const FColor ConnColor = GetConnectionColor( LOC_EVENT, i, MouseOverConnType, MouseOverConnIndex );
			ObjInfo.Events.AddItem( FLinkedObjConnInfo(*EventLinks(i).LinkDesc,  ConnColor) );
		}
	}
	// set the object reference to this object, for later use
	ObjInfo.ObjObject = this;
}

/** Utility for converting the 'visible' index of a connector to its actual connector index. */
INT USequenceOp::VisibleIndexToActualIndex(INT ConnType, INT VisibleIndex)
{
	INT CurrentVisIndex = -1;
	if(ConnType == LOC_INPUT)
	{
		for(INT i=0; i<InputLinks.Num(); i++)
		{
			if(!InputLinks(i).bHidden)
			{
				CurrentVisIndex++;
			}

			if(CurrentVisIndex == VisibleIndex)
			{
				return i;
			}
		}	
	}
	else if(ConnType == LOC_OUTPUT)
	{
		for(INT i=0; i<OutputLinks.Num(); i++)
		{
			if(!OutputLinks(i).bHidden)
			{
				CurrentVisIndex++;
			}

			if(CurrentVisIndex == VisibleIndex)
			{
				return i;
			}
		}
	}
	else if(ConnType == LOC_VARIABLE)
	{
		for(INT i=0; i<VariableLinks.Num(); i++)
		{
			if(!VariableLinks(i).bHidden)
			{
				CurrentVisIndex++;
			}

			if(CurrentVisIndex == VisibleIndex)
			{
				return i;
			}
		}
	}
	else if(ConnType == LOC_EVENT)
	{
		for(INT i=0; i<EventLinks.Num(); i++)
		{
			if(!EventLinks(i).bHidden)
			{
				CurrentVisIndex++;
			}

			if(CurrentVisIndex == VisibleIndex)
			{
				return i;
			}
		}
	}

	// Shouldn't get here!
	return 0;
}

FIntPoint USequenceOp::GetLogicConnectorsSize(FCanvas* Canvas, INT* InputY, INT* OutputY)
{
	FLinkedObjDrawInfo ObjInfo;
	MakeLinkedObjDrawInfo(ObjInfo);

	return FLinkedObjDrawUtils::GetLogicConnectorsSize(Canvas, ObjInfo, InputY, OutputY);
}

FIntPoint USequenceOp::GetVariableConnectorsSize(FCanvas* Canvas)
{
	FLinkedObjDrawInfo ObjInfo;
	MakeLinkedObjDrawInfo(ObjInfo);

	return FLinkedObjDrawUtils::GetVariableConnectorsSize(Canvas, ObjInfo);
}

void USequenceOp::DrawLogicConnectors(FCanvas* Canvas, const FIntPoint& Pos, const FIntPoint& Size, INT MouseOverConnType, INT MouseOverConnIndex)
{
	DrawWidth = Size.X;

	FLinkedObjDrawInfo ObjInfo;
	MakeLinkedObjDrawInfo(ObjInfo, MouseOverConnType, MouseOverConnIndex);

	FLinkedObjDrawUtils::DrawLogicConnectors( Canvas, ObjInfo, Pos, Size );

	INT LinkIdx = 0;
	for(INT i=0; i<InputLinks.Num(); i++)
	{
		if (!InputLinks(i).bHidden)
		{
			InputLinks(i).DrawY = ObjInfo.InputY(LinkIdx);
			LinkIdx++;
		}
	}

	LinkIdx = 0;
	for(INT i=0; i<OutputLinks.Num(); i++)
	{
		if (!OutputLinks(i).bHidden)
		{
			OutputLinks(i).DrawY = ObjInfo.OutputY(LinkIdx);
			LinkIdx++;
		}
	}
}

FColor USequenceOp::GetVarConnectorColor(INT ConnIndex)
{
	if( ConnIndex < 0 || ConnIndex >= VariableLinks.Num() )
	{
		return FColor(0,0,0);
	}

	FSeqVarLink* VarLink = &VariableLinks(ConnIndex);

	if(VarLink->ExpectedType == NULL)
	{
		return FColor(0,0,0);
	}

	USequenceVariable* DefVar = (USequenceVariable*)VarLink->ExpectedType->GetDefaultObject();
	return DefVar->ObjColor;
}


void USequenceOp::DrawVariableConnectors(FCanvas* Canvas, const FIntPoint& Pos, const FIntPoint& Size, INT MouseOverConnType, INT MouseOverConnIndex, INT VarWidth)
{
	DrawHeight = (Pos.Y + Size.Y) - ObjPosY;

	FLinkedObjDrawInfo ObjInfo;
	MakeLinkedObjDrawInfo(ObjInfo, MouseOverConnType, MouseOverConnIndex);

	FLinkedObjDrawUtils::DrawVariableConnectors( Canvas, ObjInfo, Pos, Size, VarWidth );

	INT idx, LinkIdx = 0;
	for (idx = 0; idx < VariableLinks.Num(); idx++)
	{
		if (!VariableLinks(idx).bHidden)
		{
			VariableLinks(idx).DrawX = ObjInfo.VariableX(LinkIdx);
			LinkIdx++;
		}
	}

	LinkIdx = 0;
	for (idx = 0; idx < EventLinks.Num(); idx++)
	{
		if (!EventLinks(idx).bHidden)
		{
			EventLinks(idx).DrawX = ObjInfo.EventX(LinkIdx);
			LinkIdx++;
		}
	}
}

/** Utility for determining if the mouse is currently over a connector on one end of this link. */
static UBOOL MouseOverLink(const USequenceOp* Me, const FSeqOpOutputLink& Output, INT OutputIndex, INT LinkIndex, const USequenceObject* MouseOverSeqObj, INT MouseOverConnType, INT MouseOverConnIndex)
{
	if( MouseOverConnType == LOC_OUTPUT && 
		Me == MouseOverSeqObj && 
		OutputIndex == MouseOverConnIndex)
	{
		return TRUE;
	}

	if( MouseOverConnType == LOC_INPUT && 
		Output.Links(LinkIndex).LinkedOp == MouseOverSeqObj && 
		Output.Links(LinkIndex).InputLinkIdx == MouseOverConnIndex )
	{
		return TRUE;
	}

	return FALSE;
}

void USequenceOp::DrawLogicLinks(FCanvas* Canvas, UBOOL bCurves, TArray<USequenceObject*> &SelectedSeqObjs, USequenceObject* MouseOverSeqObj, INT MouseOverConnType, INT MouseOverConnIndex)
{
	const UBOOL bSelected = SelectedSeqObjs.ContainsItem(this);
	for (INT i=0; i<InputLinks.Num(); i++)
	{
		const FSeqOpInputLink &Link = InputLinks(i);
		if (Link.ActivateDelay > 0.f)
		{
			FString DelayStr = FString::Printf(TEXT("Delay %2.1f"),Link.ActivateDelay);
			FIntPoint Start = this->GetConnectionLocation(LOC_INPUT, i);
			INT SizeX, SizeY;
			UCanvas::ClippedStrLen(GEngine->SmallFont,1.f,1.f,SizeX,SizeY,*DelayStr);
			Start.X -= SizeX;
			Start.Y -= SizeY;
			DrawString( Canvas, Start.X, Start.Y, *DelayStr, GEngine->SmallFont, FLinearColor::White );
		}
	}
	// for each valid output Link,
	for(INT i=0; i<OutputLinks.Num(); i++)
	{
		const FSeqOpOutputLink &Link = OutputLinks(i);
		// grab the start point for this line
		const FIntPoint Start = this->GetConnectionLocation(LOC_OUTPUT, i);
		// iterate through all Linked inputs,
		for (INT LinkIdx = 0; LinkIdx < Link.Links.Num(); LinkIdx++)
		{
			const FSeqOpOutputInputLink &InLink = Link.Links(LinkIdx);
			if (InLink.LinkedOp != NULL &&
				InLink.InputLinkIdx >= 0 &&
				InLink.InputLinkIdx < InLink.LinkedOp->InputLinks.Num())
			{
				// grab the end point
				const FIntPoint End = InLink.LinkedOp->GetConnectionLocation(LOC_INPUT, InLink.InputLinkIdx);
				FColor LineColor = FColor(0,0,0);
				if( bSelected || SelectedSeqObjs.ContainsItem(InLink.LinkedOp) )
				{
					LineColor = FColor(255,255,0);
				}
				else if( MouseOverLink(this, Link, i, LinkIdx, MouseOverSeqObj, MouseOverConnType, MouseOverConnIndex) )
				{
					LineColor = FColor(255,200,0);
				}
				else if( Link.bDisabled || InLink.LinkedOp->InputLinks(InLink.InputLinkIdx).bDisabled )
				{
					LineColor = FColor(255,0,0);
				} 
				else if( Link.bDisabledPIE || InLink.LinkedOp->InputLinks(InLink.InputLinkIdx).bDisabledPIE )
				{
					LineColor = FColor(255,128,0);
				} 
				else if( Link.bHasImpulse )
				{
					LineColor = FColor(0,255,0);
				}

				if (Canvas->IsHitTesting())
				{
					Canvas->SetHitProxy(new HLinkedObjLineProxy(this,i,InLink.LinkedOp,InLink.InputLinkIdx));
				}

				if(bCurves)
				{
					const FLOAT Tension = Abs<INT>(Start.X - End.X);
					//const FLOAT Tension = 100.f;
					FLinkedObjDrawUtils::DrawSpline(Canvas, Start, Tension * FVector2D(1,0), End, Tension * FVector2D(1,0), LineColor, true);
				}
				else
				{
					// Draw the line, highlighted if it has an impulse
					DrawLine2D( Canvas, Start, End, LineColor);

					// Make a pretty little arrow!
					const FVector2D Dir = (FVector2D(End) - FVector2D(Start)).SafeNormal();
					FLinkedObjDrawUtils::DrawArrowhead(Canvas, End, Dir, LineColor);
				}

				if (Canvas->IsHitTesting())
				{
					Canvas->SetHitProxy(NULL);
				}
			}
		}
		// draw the activate delay if set
		if (Link.ActivateDelay > 0.f)
		{
			DrawString( Canvas, Start.X, Start.Y, *FString::Printf(TEXT("Delay %2.1f"),Link.ActivateDelay), GEngine->SmallFont, FLinearColor::White );
		}
	}
}

/**
 * Draws lines to all Linked variables and events.
 */
void USequenceOp::DrawVariableLinks(FCanvas* Canvas, UBOOL bCurves, TArray<USequenceObject*> &SelectedSeqObjs, USequenceObject* MouseOverSeqObj, INT MouseOverConnType, INT MouseOverConnIndex)
{
	const UBOOL bSelected = SelectedSeqObjs.ContainsItem(this);
	for (INT VarIdx = 0; VarIdx < VariableLinks.Num(); VarIdx++)
	{
		FSeqVarLink &VarLink = VariableLinks(VarIdx);
		const FIntPoint Start = this->GetConnectionLocation(LOC_VARIABLE, VarIdx);
		// draw Links for each variable connected to this connection
		for (INT Idx = 0; Idx < VarLink.LinkedVariables.Num(); Idx++)
		{
			USequenceVariable *Var = VarLink.LinkedVariables(Idx);
			// remove any null entries
			if (Var == NULL)
			{
				VarLink.LinkedVariables.Remove(Idx--,1);
				continue;
			}
			const FIntPoint End = Var->GetVarConnectionLocation();
			FColor LinkColor;
			if (bSelected || SelectedSeqObjs.ContainsItem(Var))
			{
				LinkColor = FColor(255,255,0);
			}
			else
			{
				LinkColor = (Var->ObjColor.ReinterpretAsLinear() * MouseOverColorScale).Quantize();
			}

			if(bCurves)
			{
				const FLOAT Tension = Abs<INT>(Start.Y - End.Y);

				if(!VarLink.bWriteable)
				{
					FLinkedObjDrawUtils::DrawSpline(Canvas, End, FVector2D(0,0), Start, Tension*FVector2D(0,-1), LinkColor, true);
				}
				else
				{
					FLinkedObjDrawUtils::DrawSpline(Canvas, Start, Tension*FVector2D(0,1), End, FVector2D(0,0), LinkColor, true);
				}
			}
			else
			{
				DrawLine2D(Canvas, Start, End, LinkColor);

				// make a pretty little arrow!
				if (!VarLink.bWriteable)
				{
					const FVector2D Dir = (FVector2D(Start) - FVector2D(End)).SafeNormal();
					FLinkedObjDrawUtils::DrawArrowhead(Canvas, Start, Dir, LinkColor );
				}
				else
				{
					const FVector2D Dir = (FVector2D(End) - FVector2D(Start)).SafeNormal();
					FLinkedObjDrawUtils::DrawArrowhead(Canvas, End, Dir, LinkColor );
				}
			}
		}
	}
	for (INT EvtIdx = 0; EvtIdx < EventLinks.Num(); EvtIdx++)
	{
		FSeqEventLink &EvtLink = EventLinks(EvtIdx);
		const FIntPoint Start = this->GetConnectionLocation(LOC_EVENT, EvtIdx);
		// draw Links for each variable connected to this connection
		for (INT Idx = 0; Idx < EvtLink.LinkedEvents.Num(); Idx++)
		{
			USequenceEvent *Evt = EvtLink.LinkedEvents(Idx);
			// remove any null entries
			if (Evt == NULL)
			{
				EvtLink.LinkedEvents.Remove(Idx--,1);
				continue;
			}
			FColor LinkColor;
			if (bSelected || SelectedSeqObjs.ContainsItem(Evt))
			{
				LinkColor = FColor(255,255,0);
			}
			else
			{
				LinkColor = Evt->ObjColor;
			}
			const FIntPoint End = Evt->GetCenterPoint(Canvas);
			if (bCurves)
			{
				const FLOAT Tension = Abs<INT>(Start.Y - End.Y);
				FLinkedObjDrawUtils::DrawSpline(Canvas, Start, Tension*FVector2D(0,1), End, Tension*FVector2D(0,1), LinkColor, true);
			}
			else
			{
				DrawLine2D(Canvas, Start, End, LinkColor);
				const FVector2D Dir = (FVector2D(End) - FVector2D(Start)).SafeNormal();
				FLinkedObjDrawUtils::DrawArrowhead(Canvas, End, Dir, LinkColor);
			}
		}
	}
}

FIntPoint USequenceOp::GetConnectionLocation(INT Type, INT ConnIndex)
{
	if(Type == LOC_INPUT)
	{
		if( ConnIndex < 0 || ConnIndex >= InputLinks.Num() )
			return FIntPoint(0,0);

		return FIntPoint( ObjPosX - LO_CONNECTOR_LENGTH, InputLinks(ConnIndex).DrawY );
	}
	else if(Type == LOC_OUTPUT)
	{
		if( ConnIndex < 0 || ConnIndex >= OutputLinks.Num() )
			return FIntPoint(0,0);

		return FIntPoint( ObjPosX + DrawWidth + LO_CONNECTOR_LENGTH, OutputLinks(ConnIndex).DrawY );
	}
	else
	if (Type == LOC_VARIABLE)
	{
		if( ConnIndex < 0 || ConnIndex >= VariableLinks.Num() )
			return FIntPoint(0,0);

		return FIntPoint( VariableLinks(ConnIndex).DrawX, ObjPosY + DrawHeight + LO_CONNECTOR_LENGTH);
	}
	else
	if (Type == LOC_EVENT)
	{
		if (ConnIndex >= 0 &&
			ConnIndex < EventLinks.Num())
		{
			return FIntPoint(EventLinks(ConnIndex).DrawX, ObjPosY + DrawHeight + LO_CONNECTOR_LENGTH);
		}
	}
	return FIntPoint(0,0);
}

void USequenceOp::PostEditChange(UProperty* PropertyThatChanged)
{
	UpdateDynamicLinks();
	Super::PostEditChange(PropertyThatChanged);
}

void USequenceOp::DrawSeqObj(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, INT MouseOverConnType, INT MouseOverConnIndex, FLOAT MouseOverTime)
{
	const UBOOL bHitTesting = Canvas->IsHitTesting();

	const FIntPoint TitleSize = GetTitleBarSize(Canvas);
	const FIntPoint LogicSize = GetLogicConnectorsSize(Canvas);
	const FIntPoint VarSize = GetVariableConnectorsSize(Canvas);

	const INT Width = Max3(TitleSize.X, LogicSize.X, VarSize.X);
	const INT Height = TitleSize.Y + LogicSize.Y + VarSize.Y + 3;

	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HLinkedObjProxy(this) );

	DrawTitleBar(Canvas, bSelected, bMouseOver, FIntPoint(ObjPosX, ObjPosY), FIntPoint(Width, TitleSize.Y));

	DrawTile(Canvas,ObjPosX,		ObjPosY + TitleSize.Y + 1,	Width,		LogicSize.Y + VarSize.Y,		0.0f,0.0f,0.0f,0.0f, GetBorderColor(bSelected, bMouseOver) );
	DrawTile(Canvas,ObjPosX + 1,	ObjPosY + TitleSize.Y + 2,	Width - 2,	LogicSize.Y + VarSize.Y - 2,	0.0f,0.0f,0.0f,0.0f, FColor(140,140,140) );

	DrawExtraInfo(Canvas,FVector(ObjPosX + (Width/2),ObjPosY + (LogicSize.Y + VarSize.Y + TitleSize.Y)/2,0));

	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );

	if(bMouseOver)
	{
		DrawLogicConnectors(	Canvas, FIntPoint(ObjPosX, ObjPosY + TitleSize.Y + 1),					FIntPoint(Width, LogicSize.Y), MouseOverConnType, MouseOverConnIndex);
		DrawVariableConnectors(	Canvas, FIntPoint(ObjPosX, ObjPosY + TitleSize.Y + 1 + LogicSize.Y),	FIntPoint(Width, VarSize.Y), MouseOverConnType, MouseOverConnIndex, VarSize.X);
	}
	else
	{
		DrawLogicConnectors(	Canvas, FIntPoint(ObjPosX, ObjPosY + TitleSize.Y + 1),					FIntPoint(Width, LogicSize.Y), -1, INDEX_NONE);
		DrawVariableConnectors(	Canvas, FIntPoint(ObjPosX, ObjPosY + TitleSize.Y + 1 + LogicSize.Y),	FIntPoint(Width, VarSize.Y), -1, INDEX_NONE, VarSize.X);
	}
}

//-----------------------------------------------------------------------------
//	USequenceEvent
//-----------------------------------------------------------------------------

FIntPoint USequenceEvent::GetCenterPoint(FCanvas* Canvas)
{
	return FIntPoint(ObjPosX + MaxWidth / 2, ObjPosY);
}

FIntRect USequenceEvent::GetSeqObjBoundingBox()
{
	return FIntRect(ObjPosX, ObjPosY, ObjPosX + MaxWidth, ObjPosY + DrawHeight);
}	

void USequenceEvent::DrawSeqObj(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, INT MouseOverConnType, INT MouseOverConnIndex, FLOAT MouseOverTime)
{
	const UBOOL bHitTesting = Canvas->IsHitTesting();
	const FColor BorderColor = GetBorderColor(bSelected, bMouseOver);

	const FIntPoint TitleSize = GetTitleBarSize(Canvas);
	const FIntPoint LogicSize = GetLogicConnectorsSize(Canvas);
	const FIntPoint VarSize = GetVariableConnectorsSize(Canvas);

	MaxWidth = Max3(TitleSize.X, LogicSize.X, VarSize.X);
	const INT Height = TitleSize.Y + LO_MIN_SHAPE_SIZE + 3 + VarSize.Y;

	// Draw diamond
	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HLinkedObjProxy(this) );

	DrawTitleBar(Canvas, bSelected, bMouseOver, FIntPoint(ObjPosX, ObjPosY), FIntPoint(MaxWidth, TitleSize.Y));

#define TRIANGLE_SIZE 32

	const INT CenterX = ObjPosX + MaxWidth / 2;
	const INT TriangleTop = ObjPosY + TitleSize.Y + 1;
	const INT LogicLeft = CenterX - LogicSize.X / 2;
	const INT LogicRight = CenterX + LogicSize.X / 2;
	// draw top triangle
	DrawTriangle2D(Canvas,
		FIntPoint(CenterX, TriangleTop), FVector2D(0.f, 0.f),
		FIntPoint(LogicLeft - 1, TriangleTop + TRIANGLE_SIZE), FVector2D(0.f, 0.f),
        FIntPoint(LogicRight + 1, TriangleTop + TRIANGLE_SIZE), FVector2D(0.f, 0.f),
		BorderColor
		);
	DrawTriangle2D(Canvas,
		FIntPoint(CenterX, TriangleTop + 1), FVector2D(0.f, 0.f),
		FIntPoint(LogicLeft, TriangleTop + TRIANGLE_SIZE), FVector2D(0.f, 0.f),
		FIntPoint(LogicRight, TriangleTop + TRIANGLE_SIZE), FVector2D(0.f, 0.f),
		FColor(140,140,140)
		);
	// draw bottom triangle
	DrawTriangle2D(Canvas,
		FIntPoint(LogicLeft - 1, TriangleTop + TRIANGLE_SIZE + LogicSize.Y), FVector2D(0.f, 0.f),
		FIntPoint(LogicRight + 1, TriangleTop + TRIANGLE_SIZE + LogicSize.Y), FVector2D(0.f, 0.f),
		FIntPoint(CenterX, TriangleTop + TRIANGLE_SIZE * 2 + LogicSize.Y), FVector2D(0.f, 0.f),
		BorderColor
		);
	DrawTriangle2D(Canvas,
		FIntPoint(LogicLeft, TriangleTop + TRIANGLE_SIZE + LogicSize.Y - 1), FVector2D(0.f, 0.f),
		FIntPoint(LogicRight, TriangleTop + TRIANGLE_SIZE + LogicSize.Y - 1), FVector2D(0.f, 0.f),
		FIntPoint(CenterX, TriangleTop + TRIANGLE_SIZE * 2 + LogicSize.Y - 1), FVector2D(0.f, 0.f),
		FColor(140,140,140)
		);
	// draw center rectangle
	DrawTile(Canvas,LogicLeft - 1, TriangleTop + TRIANGLE_SIZE, LogicSize.X + 2, LogicSize.Y, 0.f, 0.f, 0.f, 0.f, BorderColor);
	DrawTile(Canvas,LogicLeft, TriangleTop + TRIANGLE_SIZE, LogicSize.X, LogicSize.Y, 0.f, 0.f, 0.f, 0.f, FColor(140,140,140));

	// try to draw the originator's sprite icon
	if (Originator != NULL)
	{
		USpriteComponent *SpriteComp = NULL;
		ULightComponent *LightComp = NULL;
		for (INT Idx = 0; Idx < Originator->Components.Num(); Idx++)
		{
			UComponent *Comp = Originator->Components(Idx);
			if (Comp != NULL)
			{
				if (Comp->IsA(USpriteComponent::StaticClass()))
				{
					if (!((USpriteComponent*)Comp)->HiddenEditor)
					{
						SpriteComp = (USpriteComponent*)Comp;
					}
				}
				else if (Comp->IsA(ULightComponent::StaticClass()))
				{
					LightComp = (ULightComponent*)Comp;
				}
				// if we found everything,
				if (SpriteComp != NULL && LightComp != NULL)
				{
					// no need to keep looking
					break;
				}
			}
		}
		if (SpriteComp != NULL)
		{
			FColor DrawColor(128,128,128,255);
			// use the light color if available
			if (LightComp != NULL)
			{
				DrawColor = LightComp->LightColor;
				DrawColor.A = 255;
			}
			UTexture2D *Sprite = SpriteComp->Sprite;
			INT SizeX = Min<INT>(Sprite->SizeX,LO_MIN_SHAPE_SIZE);
			INT SizeY = Min<INT>(Sprite->SizeY,LO_MIN_SHAPE_SIZE);
			DrawTile(Canvas, appTrunc(CenterX - SizeX/2),TriangleTop + TRIANGLE_SIZE + LogicSize.Y/2 - SizeY/2,
						 SizeX,SizeY,
						 0.f,0.f,
						 1.f,1.f,
						 DrawColor,
						 Sprite->Resource);
		}
	}

	DrawExtraInfo(Canvas,FVector(CenterX,TriangleTop + TRIANGLE_SIZE,0));

	// If there are any variable connectors visible, draw the the box under the event that contains them.
	INT NumVisibleVarLinks = 0;
	for(INT i=0; i<VariableLinks.Num(); i++)
	{
		if(!VariableLinks(i).bHidden)
		{
			NumVisibleVarLinks++;
		}
	}

	if(NumVisibleVarLinks > 0)
	{
		// Draw the variable connector bar at the bottom
		DrawTile(Canvas,ObjPosX, TriangleTop + TRIANGLE_SIZE * 2 + LogicSize.Y + 3, MaxWidth, VarSize.Y, 0.f, 0.f, 0.f, 0.f, BorderColor);
		DrawTile(Canvas,ObjPosX + 1, TriangleTop + TRIANGLE_SIZE * 2 + LogicSize.Y + 3 + 1, MaxWidth - 2, VarSize.Y - 2, 0.f, 0.f, 0.f, 0.f, FColor(140,140,140));

		if(bMouseOver)
		{
			DrawVariableConnectors(Canvas, FIntPoint(ObjPosX, TriangleTop + TRIANGLE_SIZE * 2 + LogicSize.Y + 3), FIntPoint(MaxWidth, VarSize.Y), MouseOverConnType, MouseOverConnIndex, VarSize.X);
		}
		else
		{
			DrawVariableConnectors(Canvas, FIntPoint(ObjPosX, TriangleTop + TRIANGLE_SIZE * 2 + LogicSize.Y + 3), FIntPoint(MaxWidth, VarSize.Y), -1, INDEX_NONE, VarSize.X);
		}
	}

	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );

	// Draw output connectors
	if (bMouseOver)
	{
		DrawLogicConnectors(Canvas, FIntPoint(LogicLeft, TriangleTop + TRIANGLE_SIZE), FIntPoint(LogicSize.X + 1, LogicSize.Y), MouseOverConnType, MouseOverConnIndex);
	}
	else
	{
		DrawLogicConnectors(Canvas, FIntPoint(LogicLeft, TriangleTop + TRIANGLE_SIZE), FIntPoint(LogicSize.X + 1, LogicSize.Y), -1, INDEX_NONE);
	}
	DrawWidth = (MaxWidth + LogicSize.X) / 2 + 1;
	DrawHeight = Height;
}

//-----------------------------------------------------------------------------
//	USequenceVariable
//-----------------------------------------------------------------------------

void USequenceVariable::DrawSeqObj(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, INT MouseOverConnType, INT MouseOverConnIndex, FLOAT MouseOverTime)
{
	const UBOOL bHitTesting = Canvas->IsHitTesting();
	const FColor BorderColor = GetBorderColor(bSelected, bMouseOver);

	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HLinkedObjProxy(this) );

	// Draw comment for variable
	if(ObjComment.Len() > 0)
	{
		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, *ObjComment );

		const INT CommentX = ObjPosX + 2;
		const INT CommentY = ObjPosY - YL - 2;
		if ( FLinkedObjDrawUtils::AABBLiesWithinViewport( Canvas, CommentX, CommentY, XL, YL ) )
		{
			FLinkedObjDrawUtils::DrawShadowedString( Canvas, CommentX, CommentY, *ObjComment, GEngine->SmallFont, FColor(64,64,192) );
		}
	}

	// Draw circle
	const FIntPoint CircleCenter(ObjPosX + LO_MIN_SHAPE_SIZE / 2, ObjPosY + LO_MIN_SHAPE_SIZE / 2);

	DrawWidth = CircleCenter.X - ObjPosX;
	DrawHeight = CircleCenter.Y - ObjPosY;

	FLinkedObjDrawUtils::DrawNGon( Canvas, CircleCenter, BorderColor, LO_CIRCLE_SIDES, LO_MIN_SHAPE_SIZE / 2 );
	FLinkedObjDrawUtils::DrawNGon( Canvas, CircleCenter, FColor(140,140,140), LO_CIRCLE_SIDES, LO_MIN_SHAPE_SIZE / 2 -1 );

	// Give subclasses a chance to draw extra stuff
	DrawExtraInfo(Canvas, CircleCenter);

	// Draw the actual value of the variable in the middle of the circle
	FString VarString = GetValueStr();

	INT XL, YL;
	StringSize(GEngine->SmallFont, XL, YL, *VarString);

	// check if we need to elipse the string due to width constraint
	if (!bSelected &&
		VarString.Len() > 8 &&
		XL > LO_MIN_SHAPE_SIZE &&
		!this->IsA(USeqVar_Named::StaticClass()))
	{
		//TODO: calculate the optimal number of chars to fit instead of hard-coded
		VarString = VarString.Left(4) + TEXT("..") + VarString.Right(4);
		StringSize(GEngine->SmallFont, XL, YL, *VarString);
	}

	{
		const FIntPoint StringPos( CircleCenter.X - XL / 2, CircleCenter.Y - YL / 2 );
		if ( FLinkedObjDrawUtils::AABBLiesWithinViewport( Canvas, StringPos.X, StringPos.Y, XL, YL ) )
		{
			FLinkedObjDrawUtils::DrawShadowedString( Canvas, StringPos.X, StringPos.Y, *VarString, GEngine->SmallFont, FLinearColor::White );
		}
	}

	// If this variable has a name, write it underneath the variable
	if(VarName != NAME_None)
	{
		StringSize(GEngine->SmallFont, XL, YL, *VarName.ToString());
		const FIntPoint StringPos( CircleCenter.X - XL / 2, ObjPosY + LO_MIN_SHAPE_SIZE + 2 );
		if ( FLinkedObjDrawUtils::AABBLiesWithinViewport( Canvas, StringPos.X, StringPos.Y, XL, YL ) )
		{
			FLinkedObjDrawUtils::DrawShadowedString( Canvas, StringPos.X, StringPos.Y, *VarName.ToString(), GEngine->SmallFont, FColor(255,64,64) );
		}
	}

	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );
}

FIntPoint USequenceVariable::GetVarConnectionLocation()
{
	return FIntPoint(DrawWidth + ObjPosX, DrawHeight + ObjPosY);
}

FIntRect USequenceVariable::GetSeqObjBoundingBox()
{
	return FIntRect(ObjPosX, ObjPosY, ObjPosX + LO_MIN_SHAPE_SIZE, ObjPosY + LO_MIN_SHAPE_SIZE);
}

//-----------------------------------------------------------------------------
//	USeqVar_Object
//-----------------------------------------------------------------------------

void USeqVar_Object::DrawExtraInfo(FCanvas* Canvas, const FVector& CircleCenter)
{
	// try to draw the object's sprite icon
	AActor *Originator = Cast<AActor>(ObjValue);
	if (Originator != NULL)
	{
		USpriteComponent *SpriteComp = NULL;
		ULightComponent *LightComp = NULL;
		for (INT Idx = 0; Idx < Originator->Components.Num(); Idx++)
		{
			UComponent *Comp = Originator->Components(Idx);
			if (Comp != NULL)
			{
				if (Comp->IsA(USpriteComponent::StaticClass()))
				{
					if (!((USpriteComponent*)Comp)->HiddenEditor)
					{
						SpriteComp = (USpriteComponent*)Comp;
					}
				}
				else if (Comp->IsA(ULightComponent::StaticClass()))
				{
					LightComp = (ULightComponent*)Comp;
				}
				// if we found everything,
				if (SpriteComp != NULL && LightComp != NULL)
				{
					// no need to keep looking
					break;
				}
			}
		}
		if (SpriteComp != NULL)
		{
			FColor DrawColor(128,128,128,255);
			// use the light color if available
			if (LightComp != NULL)
			{
				DrawColor = LightComp->LightColor;
				DrawColor.A = 255;
			}
			UTexture2D *Sprite = SpriteComp->Sprite;
			INT SizeX = Min<INT>(Sprite->SizeX,LO_MIN_SHAPE_SIZE);
			INT SizeY = Min<INT>(Sprite->SizeY,LO_MIN_SHAPE_SIZE);
			DrawTile(Canvas, appTrunc(CircleCenter.X - SizeX/2),appTrunc(CircleCenter.Y - SizeY/2),
						 SizeX,SizeY,
						 0.f,0.f,
						 1.f,1.f,
						 DrawColor,
						 Sprite->Resource);
		}
	}
}

void USeqVar_ObjectVolume::DrawExtraInfo(FCanvas* Canvas, const FVector& CircleCenter)
{
	Super::DrawExtraInfo(Canvas,CircleCenter);
	// draw a little mark to differentiate this object var from all other types
	DrawTile(Canvas,appTrunc(CircleCenter.X - LO_MIN_SHAPE_SIZE/2), appTrunc(CircleCenter.Y - LO_MIN_SHAPE_SIZE/2),
				 8, 8,
				 0.f, 0.f,
				 1.f, 1.f,
				 ObjColor,
				 NULL);
	DrawTile(Canvas,appTrunc(CircleCenter.X + LO_MIN_SHAPE_SIZE/2), appTrunc(CircleCenter.Y - LO_MIN_SHAPE_SIZE/2),
				 8, 8,
				 0.f, 0.f,
				 1.f, 1.f,
				 ObjColor,
				 NULL);
}

//-----------------------------------------------------------------------------
//	USeqVar_ObjectList
//-----------------------------------------------------------------------------

void USeqVar_ObjectList::DrawExtraInfo(FCanvas* Canvas, const FVector& CircleCenter)
{

	// if the list is empty then return as we can't draw anything
	if( 0 == ObjList.Num() )	
	{
		return;
	}
	// try to draw the object's sprite icon of the FIRST entry in the list
	AActor* Originator = Cast<AActor>(ObjList(0));
	if (Originator != NULL)
	{
		USpriteComponent* spriteComponent = NULL;
		for (INT idx = 0; idx < Originator->Components.Num() && spriteComponent == NULL; idx++)
		{
			// check if this is a sprite component
			spriteComponent = Cast<USpriteComponent>(Originator->Components(idx));
		}
		if (spriteComponent != NULL)
		{
			UTexture2D *sprite = spriteComponent->Sprite;
			DrawTile(Canvas, appTrunc(CircleCenter.X - sprite->SizeX/2)
				,appTrunc(CircleCenter.Y - sprite->SizeY / 2)
				,sprite->SizeX,sprite->SizeY
				,0.f,0.f
				,1.f,1.f
				,FColor(102,0,102,255)
				,sprite->Resource
				);
		}
	}
}

//-----------------------------------------------------------------------------
//	USeqVar_Named
//-----------------------------------------------------------------------------

/** Draw cross or tick to indicate status of this USeqVar_Named. */
void USeqVar_Named::DrawExtraInfo(FCanvas* Canvas, const FVector& CircleCenter)
{
	// Ensure materials are there.
	if(!GEngine->TickMaterial || !GEngine->CrossMaterial)
	{
		return;
	}

	const UMaterial* UseMaterial = bStatusIsOk ? GEngine->TickMaterial : GEngine->CrossMaterial;
	DrawTile(Canvas, appTrunc(CircleCenter.X-16), appTrunc(CircleCenter.Y-16), 32, 32, 0.f, 0.f, 1.f, 1.f, UseMaterial->GetRenderProxy(0) );
}

//-----------------------------------------------------------------------------
//	USeqAct_ActivateRemoteEvent
//-----------------------------------------------------------------------------

/** Draw cross or tick to indicate status of this remote event (has at least one matching event). */
void USeqAct_ActivateRemoteEvent::DrawExtraInfo(FCanvas* Canvas, const FVector& BoxCenter)
{
	// Ensure materials are there.
	if(!GEngine->TickMaterial || !GEngine->CrossMaterial)
	{
		return;
	}

	const UMaterial* UseMaterial = bStatusIsOk ? GEngine->TickMaterial : GEngine->CrossMaterial;
	DrawTile(Canvas, appTrunc(BoxCenter.X-16), appTrunc(BoxCenter.Y-8), 32, 32, 0.f, 0.f, 1.f, 1.f, UseMaterial->GetRenderProxy(0) );
}

/** Same for the remote events (has at least one matching action). */
void USeqEvent_RemoteEvent::DrawExtraInfo(FCanvas* Canvas, const FVector& BoxCenter)
{
	// Ensure materials are there.
	if(!GEngine->TickMaterial || !GEngine->CrossMaterial)
	{
		return;
	}

	const UMaterial* UseMaterial = bStatusIsOk ? GEngine->TickMaterial : GEngine->CrossMaterial;
	DrawTile(Canvas, appTrunc(BoxCenter.X-16), appTrunc(BoxCenter.Y-8), 32, 32, 0.f, 0.f, 1.f, 1.f, UseMaterial->GetRenderProxy(0) );
}

/* Level streaming actions */

void USeqAct_LevelVisibility::DrawExtraInfo(FCanvas* Canvas, const FVector& BoxCenter)
{
	// Ensure materials are there.
	if(!GEngine->TickMaterial || !GEngine->CrossMaterial)
	{
		return;
	}

	const UMaterial* UseMaterial = bStatusIsOk ? GEngine->TickMaterial : GEngine->CrossMaterial;
	DrawTile(Canvas, appTrunc(BoxCenter.X-16), appTrunc(BoxCenter.Y-8), 32, 32, 0.f, 0.f, 1.f, 1.f, UseMaterial->GetRenderProxy(0) );
}


void USeqAct_LevelStreaming::DrawExtraInfo(FCanvas* Canvas, const FVector& BoxCenter)
{
	// Ensure materials are there.
	if(!GEngine->TickMaterial || !GEngine->CrossMaterial)
	{
		return;
	}

	const UMaterial* UseMaterial = bStatusIsOk ? GEngine->TickMaterial : GEngine->CrossMaterial;
	DrawTile(Canvas, appTrunc(BoxCenter.X-16), appTrunc(BoxCenter.Y-8), 32, 32, 0.f, 0.f, 1.f, 1.f, UseMaterial->GetRenderProxy(0) );
}

void USeqAct_MultiLevelStreaming::DrawExtraInfo(FCanvas* Canvas, const FVector& BoxCenter)
{
	// Ensure materials are there.
	if(!GEngine->TickMaterial || !GEngine->CrossMaterial)
	{
		return;
	}

	const UMaterial* UseMaterial = bStatusIsOk ? GEngine->TickMaterial : GEngine->CrossMaterial;
	DrawTile(Canvas, appTrunc(BoxCenter.X-16), appTrunc(BoxCenter.Y-8), 32, 32, 0.f, 0.f, 1.f, 1.f, UseMaterial->GetRenderProxy(0) );
}

void USeqAct_PrepareMapChange::DrawExtraInfo(FCanvas* Canvas, const FVector& BoxCenter)
{
	// Ensure materials are there.
	if(!GEngine->TickMaterial || !GEngine->CrossMaterial)
	{
		return;
	}

	const UMaterial* UseMaterial = bStatusIsOk ? GEngine->TickMaterial : GEngine->CrossMaterial;
	DrawTile(Canvas, appTrunc(BoxCenter.X-16), appTrunc(BoxCenter.Y-8), 32, 32, 0.f, 0.f, 1.f, 1.f, UseMaterial->GetRenderProxy(0) );
}


//-----------------------------------------------------------------------------
//	USequenceFrame
//-----------------------------------------------------------------------------

void USequenceFrame::DrawFrameBox(FCanvas* Canvas, UBOOL bSelected)
{
	// Draw filled center if desired.
	if(bFilled)
	{
		// If texture, use it...
		if(FillMaterial)
		{
			// Tiling is every 64 pixels.
			if(bTileFill)
			{
				DrawTile(Canvas, ObjPosX, ObjPosY, SizeX, SizeY, 0.f, 0.f, (FLOAT)SizeX/64.f, (FLOAT)SizeY/64.f, FillMaterial->GetRenderProxy(0) );
			}
			else
			{
				DrawTile(Canvas, ObjPosX, ObjPosY, SizeX, SizeY, 0.f, 0.f, 1.f, 1.f, FillMaterial->GetRenderProxy(0) );
			}
		}
		else if(FillTexture)
		{
			if(bTileFill)
			{
				DrawTile(Canvas, ObjPosX, ObjPosY, SizeX, SizeY, 0.f, 0.f, (FLOAT)SizeX/64.f, (FLOAT)SizeY/64.f, FillColor, FillTexture->Resource );
			}
			else
			{
				DrawTile(Canvas, ObjPosX, ObjPosY, SizeX, SizeY, 0.f, 0.f, 1.f, 1.f, FillColor, FillTexture->Resource );
			}
		}
		// .. otherwise just use a solid color.
		else
		{
			DrawTile(Canvas, ObjPosX, ObjPosY, SizeX, SizeY, 0.f, 0.f, 1.f, 1.f, FillColor );
		}
	}

	// Draw frame
	const FColor FrameColor = bSelected ? FColor(255,255,0) : BorderColor;

	const INT MinDim = Min(SizeX, SizeY);
	const INT UseBorderWidth = Clamp( BorderWidth, 0, (MinDim/2)-3 );

	for(INT i=0; i<UseBorderWidth; i++)
	{
		DrawLine2D(Canvas, FVector2D(ObjPosX,				ObjPosY + i),			FVector2D(ObjPosX + SizeX,		ObjPosY + i),			FrameColor );
		DrawLine2D(Canvas, FVector2D(ObjPosX + SizeX - i,	ObjPosY),				FVector2D(ObjPosX + SizeX - i,	ObjPosY + SizeY),		FrameColor );
		DrawLine2D(Canvas, FVector2D(ObjPosX + SizeX,		ObjPosY + SizeY - i),	FVector2D(ObjPosX,				ObjPosY + SizeY - i),	FrameColor );
		DrawLine2D(Canvas, FVector2D(ObjPosX + i,			ObjPosY + SizeY),		FVector2D(ObjPosX + i,			ObjPosY - 1),			FrameColor );
	}

	// Draw little sizing triangle in bottom left.
	const INT HandleSize = 16;
	const FIntPoint A(ObjPosX + SizeX,				ObjPosY + SizeY);
	const FIntPoint B(ObjPosX + SizeX,				ObjPosY + SizeY - HandleSize);
	const FIntPoint C(ObjPosX + SizeX - HandleSize,	ObjPosY + SizeY);
	const BYTE TriangleAlpha = (bSelected) ? 255 : 32; // Make it more transparent if comment is not selected.

	const UBOOL bHitTesting = Canvas->IsHitTesting();

	if(bHitTesting)  Canvas->SetHitProxy( new HLinkedObjProxySpecial(this, 1) );
	DrawTriangle2D(Canvas, A, FVector2D(0,0), B, FVector2D(0,0), C, FVector2D(0,0), FColor(0,0,0,TriangleAlpha) );
	if(bHitTesting)  Canvas->SetHitProxy( NULL );
}

void USequenceFrame::DrawSeqObj(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, INT MouseOverConnType, INT MouseOverConnIndex, FLOAT MouseOverTime)
{
	// Draw box
	if(bDrawBox)
	{
		DrawFrameBox(Canvas, bSelected);
	}

	// Draw comment text

	// Check there are some valid chars in string. If not - we can't select it! So we force it back to default.
	INT NumProperChars = 0;
	for(INT i=0; i<ObjComment.Len(); i++)
	{
		if(ObjComment[i] != ' ')
		{
			NumProperChars++;
		}
	}

	if(NumProperChars == 0)
	{
		ObjComment = TEXT("Comment");
	}

	const FLOAT OldZoom2D = FLinkedObjDrawUtils::GetUniformScaleFromMatrix(Canvas->GetTransform());

	FRenderParameters Parameters(GEngine->SmallFont, 1.f, 1.f);
	FLOAT& XL = Parameters.DrawXL, &YL = Parameters.DrawYL;

	UUIString::StringSize( Parameters, *ObjComment );

	// We always draw comment-box text at normal size (don't scale it as we zoom in and out.)
	const INT x = appTrunc(ObjPosX*OldZoom2D + 2);
	const INT y = appTrunc(ObjPosY*OldZoom2D - YL - 2);


	// Viewport cull at a zoom of 1.0, because that's what we'll be drawing with.
	if ( FLinkedObjDrawUtils::AABBLiesWithinViewport( Canvas, ObjPosX, ObjPosY-YL-2, SizeX * OldZoom2D, YL ) )
	{
		Canvas->PushRelativeTransform(FScaleMatrix(1.0f / OldZoom2D));
		{
			const UBOOL bHitTesting = Canvas->IsHitTesting();

			DrawShadowedString(Canvas, x, y, *ObjComment, GEngine->SmallFont, FColor(64,64,192) );

			// We only set the hit proxy for the area above the comment box with the height of the comment text
			if(bHitTesting)
			{
				Canvas->SetHitProxy(new HLinkedObjProxy(this));

				// account for the +2 when x was assigned
				DrawTile(Canvas, x - 2, y, SizeX * OldZoom2D, YL, 0.f, 0.f, 1.f, 1.f, FLinearColor(1.0f, 0.0f, 0.0f));

				Canvas->SetHitProxy(NULL);
			}
		}
		Canvas->PopTransform();
	}

	// Fill in base SequenceObject rendering info (used by bounding box calculation).
	DrawWidth = SizeX;
	DrawHeight = SizeY;
}

void USequenceFrameWrapped::DrawSeqObj(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, INT MouseOverConnType, INT MouseOverConnIndex, FLOAT MouseOverTime)
{
	// Draw box
	if(bDrawBox)
	{
		DrawFrameBox(Canvas, bSelected);
	}

	// For the comment - first break into words
	TArray<FString> Words;
	ObjComment.ParseIntoArrayWS(&Words);

	// Ensure there is at least one word!
	if(Words.Num() == 0)
	{
		Words.AddItem( TEXT("Comment") );
	}

	INT LineNumber = 0;
	INT WordIndex = 0;

	const UBOOL bHitTesting = Canvas->IsHitTesting();
	if(bHitTesting)  Canvas->SetHitProxy( new HLinkedObjProxy(this) );

	while(WordIndex < Words.Num())
	{
		FRenderParameters Parameters(GEngine->SmallFont, 1.f, 1.f);
		FLOAT& LineSizeX = Parameters.DrawXL, &LineSizeY = Parameters.DrawYL;

		INT LineStartWordIndex = WordIndex;
		FString Line, LineBackup;
		while((LineSizeX < SizeX-20) && (WordIndex < Words.Num()))
		{
			LineBackup = Line;
			if(WordIndex - LineStartWordIndex > 0)
			{
				Line += TEXT(" ");
			}
			Line += Words(WordIndex);
			WordIndex++;

			UUIString::StringSize(Parameters, *Line);
		}

		// If we went over the length, and we have more than one word - strip off the last word.
		if((LineSizeX >= SizeX-20) && (WordIndex - LineStartWordIndex > 1))
		{
			Line = LineBackup;
			WordIndex--;
		}

		DrawShadowedString(Canvas, ObjPosX+10, ObjPosY+10+(20*LineNumber), *Line, GEngine->SmallFont, FColor(255,255,255) );

		LineNumber++;
	}

	if(bHitTesting) Canvas->SetHitProxy( NULL );
}

/**
 * Returns the color that should be used for an input, variable, or output link connector in the kismet editor.
 *
 * @param	ConnType	the type of connection this represents.  Valid values are:
 *							LOC_INPUT		(input link)
 *							LOC_OUTPUT		(output link)
 *							LOC_VARIABLE	(variable link)
 *							LOC_EVENT		(event link)
 * @param	ConnIndex	the index [into the corresponding array (i.e. InputLinks, OutputLinks, etc.)] for the link
 *						being queried.
 * @param	MouseOverConnType
 *						INDEX_NONE if the user is not currently mousing over the specified link connector.  One of the values
 *						listed for ConnType otherwise.
 * @param	MouseOverConnIndex
 *						INDEX_NONE if the user is not currently mousing over the specified link connector.  The index for the
 *						link being moused over otherwise.
 */
FColor USeqCond_SwitchBase::GetConnectionColor( INT ConnType, INT ConnIndex, INT MouseOverConnType, INT MouseOverConnIndex )
{
	if( ConnType == LOC_OUTPUT )
	{
		if( (MouseOverConnType != LOC_OUTPUT || MouseOverConnIndex != ConnIndex) && !eventIsFallThruEnabled(ConnIndex) ) 
		{
			return FColor(255,0,0);
		}
	}

	return Super::GetConnectionColor( ConnType, ConnIndex, MouseOverConnType, MouseOverConnIndex );
}

/** Overridden to append ExtraDelay information to comment area. */
void USeqAct_PlaySound::DrawTitleBar(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, const FIntPoint& Pos, const FIntPoint& Size)
{
	if (ExtraDelay > 0.f)
	{
		FString OrigObjComment = ObjComment;
		ObjComment = FString::Printf(TEXT("%s (ExtraDelay=%.1f)"), *ObjComment, ExtraDelay);
		Super::DrawTitleBar(Canvas, bSelected, bMouseOver, Pos, Size);
		ObjComment = OrigObjComment;
	}
	else
	{
		Super::DrawTitleBar(Canvas, bSelected, bMouseOver, Pos, Size);
	}
}


FString USeqAct_Delay::GetDisplayTitle() const
{
	FString Title = Super::GetDisplayTitle();

	// if duration is a constant and not set via variable, display delay in title
	if ( (VariableLinks.Num() > 0) && (VariableLinks(0).LinkedVariables.Num() == 0) )
	{
		Title += FString::Printf(TEXT(" (%.1f)"), Duration);
	}

	return Title;
}

FString USeqAct_StreamInTextures::GetDisplayTitle() const
{
	FString Title = Super::GetDisplayTitle();
	Title += bLocationBased ? TEXT(" (LOCATION)") : TEXT(" (ACTOR)");
	return Title;
}

FString USequenceEvent::GetDisplayTitle() const
{
	FString Title = Super::GetDisplayTitle();

	if (bClientSideOnly)
	{
		Title += TEXT(" (ClientSideOnly)");
	}
	if (!bEnabled)
	{
		Title += TEXT(" (Disabled)");
	}

	return Title;
}

FString USeqEvent_Console::GetDisplayTitle() const
{
	FString Title = Super::GetDisplayTitle();

	if (ConsoleEventName != NAME_None)
	{
		Title += FString::Printf( TEXT(" \"%s\""), *ConsoleEventName.ToString() );
	}

	return Title;
}



