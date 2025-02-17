/*=============================================================================
	UnAnimTreeDraw.cpp: Function for drawing different AnimNode classes for AnimTreeEditor
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "UnLinkedObjDrawUtils.h"

static const FColor SkelControlTitleColorDisabled(50,100,50);

static const FColor FullWeightColor(255, 130, 30);
static const FColor ZeroWeightColor(0,0,0);
/** BackGround color when a node has been deprecated */
static const FColor AnimTree_DeprecatedBGColor(200,0,0);

static inline FColor GetNodeColorFromWeight(FLOAT Weight)
{
	return FColor( ZeroWeightColor.R + (BYTE)(Weight * (FLOAT)(FullWeightColor.R - ZeroWeightColor.R)),
					ZeroWeightColor.G + (BYTE)(Weight * (FLOAT)(FullWeightColor.G - ZeroWeightColor.G)),
					ZeroWeightColor.B + (BYTE)(Weight * (FLOAT)(FullWeightColor.B - ZeroWeightColor.B)) );
}

/*-----------------------------------------------------------------------------
	UAnimTree
-----------------------------------------------------------------------------*/

FIntPoint UAnimTree::GetConnectionLocation(INT ConnType, INT ConnIndex)
{
	if(ConnType == LOC_INPUT)
	{
		check(ConnIndex == 0);
		return FIntPoint( NodePosX - LO_CONNECTOR_LENGTH, OutDrawY );
	}
	else if(ConnType == LOC_OUTPUT)
	{
		if(ConnIndex == 0)
		{
			return FIntPoint( NodePosX + DrawWidth + LO_CONNECTOR_LENGTH, Children(0).DrawY );
		}
		else if(ConnIndex == 1)
		{
			return FIntPoint( NodePosX + DrawWidth + LO_CONNECTOR_LENGTH, MorphConnDrawY );
		}
		else
		{
			return FIntPoint( NodePosX + DrawWidth + LO_CONNECTOR_LENGTH, SkelControlLists(ConnIndex-2).DrawY );
		}
	}

	return FIntPoint(0,0);
}

void UAnimTree::DrawAnimNode( FCanvas* Canvas, UBOOL bSelected, UBOOL bShowWeight, UBOOL bCurves )
{
	check(Children.Num() == 1);

	// Construct the FLinkedObjDrawInfo for use the linked-obj drawing utils.
	FLinkedObjDrawInfo ObjInfo;

	const FColor WeightOneColor( GetNodeColorFromWeight(1.f) );

	// Add one connector for animation tree.
	ObjInfo.Outputs.AddItem( FLinkedObjConnInfo(TEXT("Animation"),WeightOneColor) );

	// Add one connector for morph nodes.
	ObjInfo.Outputs.AddItem( FLinkedObjConnInfo(TEXT("Morph"), FColor(50,50,100)) );

	// Add connector for attaching controller chains.
	for(INT i=0; i<SkelControlLists.Num(); i++)
	{
		ObjInfo.Outputs.AddItem( FLinkedObjConnInfo(*SkelControlLists(i).BoneName.ToString(),SkelControlTitleColorDisabled) );
	}

	ObjInfo.ObjObject = this;

	// Generate border color
	const FColor BorderColor( bSelected ? FColor(255, 255, 0) : FColor(0, 0, 200) );

	// Use util to draw box with connectors etc.
	FString ControlDesc = GetClass()->GetDescription();
	FLinkedObjDrawUtils::DrawLinkedObj( Canvas, ObjInfo, *ControlDesc, NULL, BorderColor, FColor(112,112,112), FIntPoint(NodePosX, NodePosY) );

	// Read back draw locations of connectors, so we can draw lines in the correct places.
	Children(0).DrawY = ObjInfo.OutputY(0);
	MorphConnDrawY = ObjInfo.OutputY(1);

	for(INT i=0; i<SkelControlLists.Num(); i++)
	{
		SkelControlLists(i).DrawY = ObjInfo.OutputY(i+2);
	}

	DrawWidth = ObjInfo.DrawWidth;

	// Now draw links to animation children
	UAnimNode* ChildNode = Children(0).Anim;
	if(ChildNode)
	{
		FIntPoint Start = GetConnectionLocation(LOC_OUTPUT, 0);
		FIntPoint End = ChildNode->GetConnectionLocation(LOC_INPUT, 0);

		FLOAT Tension = Abs<INT>(Start.X - End.X);
		FLinkedObjDrawUtils::DrawSpline(Canvas, End, -Tension * FVector2D(1,0), Start, -Tension * FVector2D(1,0), GetNodeColorFromWeight(1.f), true);
	}

	// Draw links to child morph nodes.
	for(INT i=0; i<RootMorphNodes.Num(); i++)
	{
		UMorphNodeBase* MorphNode = RootMorphNodes(i);
		if(MorphNode)
		{
			FIntPoint Start = GetConnectionLocation(LOC_OUTPUT, 1);
			FIntPoint End = MorphNode->GetConnectionLocation(LOC_INPUT, 0);

			FLOAT Tension = Abs<INT>(Start.X - End.X);
			FLinkedObjDrawUtils::DrawSpline(Canvas, End, -Tension * FVector2D(1,0), Start, -Tension * FVector2D(1,0), FColor(50,50,100), true);
		}
	}

	// If an AnimTree, draw links to start of head SkelControl list.
	for(INT i=0; i<SkelControlLists.Num(); i++)
	{
		USkelControlBase* Control = SkelControlLists(i).ControlHead;
		if(Control)
		{
			FIntPoint Start = GetConnectionLocation(LOC_OUTPUT, i+2);
			FIntPoint End = Control->GetConnectionLocation(LOC_INPUT);

			FLOAT Tension = Abs<INT>(Start.X - End.X);
			FLinkedObjDrawUtils::DrawSpline(Canvas, Start, Tension * FVector2D(1,0), End, Tension * FVector2D(1,0), SkelControlTitleColorDisabled, true);
		}
	}
}

/*-----------------------------------------------------------------------------
  UAnimNode
-----------------------------------------------------------------------------*/

FIntPoint UAnimNode::GetConnectionLocation(INT ConnType, INT ConnIndex)
{
	if(ConnType == LOC_INPUT)
	{
		check(ConnIndex == 0);
		return FIntPoint( NodePosX - LO_CONNECTOR_LENGTH, OutDrawY );
	}

	return FIntPoint(0,0);
}

/*-----------------------------------------------------------------------------
  UAnimNodeBlendBase
-----------------------------------------------------------------------------*/

FString UAnimNodeBlendBase::GetNodeTitle()
{
	// Generate name for node. User-give name if entered - otherwise just class name.
	FString NodeTitle;
	const FString NodeDesc = GetClass()->GetDescription(); // Need to assign it here, so pointer isn't invalid by the time we draw it.
	if( NodeName != NAME_None )
	{
		NodeTitle = NodeName.ToString();
	}
	else
	{
		NodeTitle = NodeDesc;
	}
	return NodeTitle;
}

void UAnimNodeBlendBase::DrawAnimNode( FCanvas* Canvas, UBOOL bSelected, UBOOL bShowWeight, UBOOL bCurves)
{
	// Construct the FLinkedObjDrawInfo for use the linked-obj drawing utils.
	FLinkedObjDrawInfo ObjInfo;

	// AnimTree's don't have an output connector on left
	ObjInfo.Inputs.AddItem( FLinkedObjConnInfo(TEXT("Out"), GetNodeColorFromWeight(NodeTotalWeight) ) );

	// Add output for each child.
	for(INT i=0; i<Children.Num(); i++)
	{
		UAnimNode* ChildNode = Children(i).Anim;
		if( ChildNode )
		{
			ObjInfo.Outputs.AddItem( FLinkedObjConnInfo(*Children(i).Name.ToString(), GetNodeColorFromWeight(Children(i).TotalWeight)) );
		}
		else
		{
			ObjInfo.Outputs.AddItem( FLinkedObjConnInfo(*Children(i).Name.ToString(), FColor(0,0,0)) );
		}
	}

	ObjInfo.ObjObject = this;

	// Generate border color
	const FColor BorderColor = bSelected ? FColor(255, 255, 0) : GetNodeColorFromWeight(NodeTotalWeight);

	FString NodeTitle = GetNodeTitle();

	// Draw deprecated nodes with a red color background
	const FColor BackGroundColor = (GetClass()->ClassFlags & CLASS_Deprecated) ? AnimTree_DeprecatedBGColor : FColor(112,112,112);

	// Use util to draw box with connectors etc.
	FLinkedObjDrawUtils::DrawLinkedObj( Canvas, ObjInfo, *NodeTitle, NULL, BorderColor, BackGroundColor, FIntPoint(NodePosX, NodePosY) );

	// Read back draw locations of connectors, so we can draw lines in the correct places.
	OutDrawY = ObjInfo.InputY(0);

	for(INT i=0; i<Children.Num(); i++)
	{
		Children(i).DrawY = ObjInfo.OutputY(i);
	}

	DrawWidth = ObjInfo.DrawWidth;
	DrawHeight = ObjInfo.DrawHeight;

	// Show the global percentage weight of this node if we wish.
	if( bShowWeight )
	{
		const FString WeightString = FString::Printf( TEXT("%d - %2.1f pct"), ParentNodes.Num(), NodeTotalWeight * 100.f );
		//const FString WeightString = FString::Printf( TEXT("%2.1f pct"), NodeTotalWeight * 100.f );

		INT XL, YL;
		StringSize(GEngine->SmallFont, XL, YL, *WeightString);

		DrawShadowedString(Canvas, NodePosX + DrawWidth - XL, NodePosY - YL, *WeightString, GEngine->SmallFont, GetNodeColorFromWeight(NodeTotalWeight) );
	}

	INT SliderDrawY = NodePosY + ObjInfo.DrawHeight;

	// Draw sliders underneath this node if desired.
	const UBOOL bDrawTextOnSide = GetNumSliders() > 1;

	// Draw deprecated nodes with a red color background
	const FColor SliderBackGroundColor = (GetClass()->ClassFlags & CLASS_Deprecated) ? AnimTree_DeprecatedBGColor : FColor(140,140,140);

	for(INT i=0; i<GetNumSliders(); ++i)
	{
		if(GetSliderType(i) == ST_1D)
		{
			SliderDrawY += FLinkedObjDrawUtils::DrawSlider(Canvas, FIntPoint(NodePosX, SliderDrawY), DrawWidth, BorderColor, SliderBackGroundColor, GetSliderPosition(i, 0), GetSliderDrawValue(i), this, i, bDrawTextOnSide);
		}
		else if(GetSliderType(i) == ST_2D)
		{
			SliderDrawY += FLinkedObjDrawUtils::Draw2DSlider(Canvas, FIntPoint(NodePosX, SliderDrawY), DrawWidth, BorderColor, SliderBackGroundColor, GetSliderPosition(i, 0), GetSliderPosition(i, 1), GetSliderDrawValue(i), this, i, bDrawTextOnSide);
		}
		else
		{
			check(false);
		}
	}

	// Now draw links to animation children
	for(INT i=0; i<Children.Num(); i++)
	{
		UAnimNode* ChildNode = Children(i).Anim;
		if(ChildNode)
		{
			FIntPoint Start = GetConnectionLocation(LOC_OUTPUT, i);
			FIntPoint End = ChildNode->GetConnectionLocation(LOC_INPUT, 0);

			FLOAT Tension = Abs<INT>(Start.X - End.X);
			FLinkedObjDrawUtils::DrawSpline(Canvas, End, -Tension * FVector2D(1,0), Start, -Tension * FVector2D(1,0), GetNodeColorFromWeight(Children(i).TotalWeight), TRUE);
		}
	}
}

FIntPoint UAnimNodeBlendBase::GetConnectionLocation(INT ConnType, INT ConnIndex)
{
	if(ConnType == LOC_INPUT)
	{
		check(ConnIndex == 0);
		return FIntPoint( NodePosX - LO_CONNECTOR_LENGTH, OutDrawY );
	}
	else if(ConnType == LOC_OUTPUT)
	{
		check( ConnIndex >= 0 && ConnIndex < Children.Num() );
		return FIntPoint( NodePosX + DrawWidth + LO_CONNECTOR_LENGTH, Children(ConnIndex).DrawY );
	}

	return FIntPoint(0,0);
}

/*-----------------------------------------------------------------------------
  UAnimNodeSequence
-----------------------------------------------------------------------------*/

FString UAnimNodeSequence::GetNodeTitle()
{
	FString NodeTitle = NodeName != NAME_None ? NodeName.ToString() : AnimSeqName.ToString();

	return NodeTitle;
}

void UAnimNodeSequence::DrawAnimNode(FCanvas* Canvas, UBOOL bSelected, UBOOL bShowWeight, UBOOL bCurves)
{
	FLinkedObjDrawInfo ObjInfo;

	ObjInfo.Inputs.AddItem(FLinkedObjConnInfo(TEXT("Out"), GetNodeColorFromWeight(NodeTotalWeight)));
	ObjInfo.ObjObject = this;

	const FColor BorderColor = bSelected ? FColor(255, 255, 0) : GetNodeColorFromWeight(NodeTotalWeight);

	// Generate name for node. User-give name if entered - otherwise just class name.
	FString NodeTitle = GetNodeTitle();

	// Draw deprecated nodes with a red color background
	const FColor BackGroundColor = (GetClass()->ClassFlags & CLASS_Deprecated) ? AnimTree_DeprecatedBGColor : FColor(100,50,50);

	FLinkedObjDrawUtils::DrawLinkedObj(Canvas, ObjInfo, *NodeTitle, NULL, BorderColor, BackGroundColor, FIntPoint(NodePosX, NodePosY));

	DrawWidth	= ObjInfo.DrawWidth;
	DrawHeight	= ObjInfo.DrawHeight;
	OutDrawY	= ObjInfo.InputY(0);

	// Show the global percentage weight of this node if we wish.
	if( bShowWeight )
	{
		//const FString WeightString = FString::Printf( TEXT("%d - %2.1f pct"), ParentNodes.Num(), NodeTotalWeight * 100.f );
		const FString WeightString = FString::Printf( TEXT("%2.1f pct"), NodeTotalWeight * 100.f );
		INT XL, YL;
		StringSize(GEngine->SmallFont, XL, YL, *WeightString);

		DrawShadowedString(Canvas, NodePosX + DrawWidth - XL, NodePosY - YL, *WeightString, GEngine->SmallFont, GetNodeColorFromWeight(NodeTotalWeight) );
	}

	INT SliderDrawY = NodePosY + ObjInfo.DrawHeight;

	// Draw tick indicating animation position.
	FLOAT AnimPerc = GetNormalizedPosition();
	INT DrawPosX = NodePosX + appRound(AnimPerc * (FLOAT)DrawWidth);
	DrawTile(Canvas, DrawPosX, SliderDrawY, 2, 5, 0.f, 0.f, 0.f, 0.f, FLinearColor::Black);
	SliderDrawY += 5;
	
	// Draw sliders underneath this node if desired.
	const UBOOL bDrawTextOnSide = GetNumSliders() > 1;

	// Draw deprecated nodes with a red color background
	const FColor SliderBackGroundColor = (GetClass()->ClassFlags & CLASS_Deprecated) ? AnimTree_DeprecatedBGColor : FColor(140,140,140);

	for(INT i=0; i<GetNumSliders(); ++i)
	{
		if( GetSliderType(i) == ST_1D )
		{
			SliderDrawY += FLinkedObjDrawUtils::DrawSlider(Canvas, FIntPoint(NodePosX, SliderDrawY), DrawWidth, BorderColor, SliderBackGroundColor, GetSliderPosition(i, 0), GetSliderDrawValue(i), this, i, bDrawTextOnSide);
		}
		else if( GetSliderType(i) == ST_2D )
		{
			SliderDrawY += FLinkedObjDrawUtils::Draw2DSlider(Canvas, FIntPoint(NodePosX, SliderDrawY), DrawWidth, BorderColor, SliderBackGroundColor, GetSliderPosition(i, 0), GetSliderPosition(i, 1), GetSliderDrawValue(i), this, i, bDrawTextOnSide);
		}
		else
		{
			check(FALSE);
		}
	}

}

/*-----------------------------------------------------------------------------
	UAnimNodeBlend
-----------------------------------------------------------------------------*/

FLOAT UAnimNodeBlend::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	return Child2Weight;
}

void UAnimNodeBlend::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	check( Children.Num() == 2 );

	Child2WeightTarget = NewSliderValue;
	Child2Weight = NewSliderValue;
}

FString UAnimNodeBlend::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( TEXT("%3.2f"), Child2Weight );
}
/*-----------------------------------------------------------------------------
	AnimNodeBlendBySpeed
-----------------------------------------------------------------------------*/

FLOAT UAnimNodeBlendBySpeed::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	FLOAT MaxSpeed = Constraints( Constraints.Num() - 1 );
	return Speed / (MaxSpeed * 1.1f);
}

void UAnimNodeBlendBySpeed::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	FLOAT MaxSpeed = Constraints( Constraints.Num() - 1 );
	Speed = NewSliderValue * MaxSpeed * 1.1f;
}

FString UAnimNodeBlendBySpeed::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( TEXT("%3.2f"), Speed );
}

/*-----------------------------------------------------------------------------
	UAnimNodeBlendDirectional
-----------------------------------------------------------------------------*/

FLOAT UAnimNodeBlendDirectional::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	// DirAngle is between -PI and PI. Return value between 0.0 and 1.0 - so 0.5 is straight ahead.
	return 0.5f + (0.5f * (DirAngle / (FLOAT)PI));
}

void UAnimNodeBlendDirectional::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	// Convert from 0.0 -> 1.0 to -PI to PI.
	DirAngle = (FLOAT)PI * 2.f * (NewSliderValue - 0.5f);
}

FString UAnimNodeBlendDirectional::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( TEXT("%3.2f%c"), DirAngle * (180.f/(FLOAT)PI), 176 );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// SKELCONTROL
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------------
	USkelControlBase
-----------------------------------------------------------------------------*/

FIntPoint USkelControlBase::GetConnectionLocation(INT ConnType)
{
	static const INT ConnectorYOffset = 35;

	if(ConnType == LOC_INPUT)
	{
		return FIntPoint( ControlPosX - LO_CONNECTOR_LENGTH, ControlPosY + ConnectorYOffset );
	}
	else
	{
		return FIntPoint( ControlPosX + DrawWidth + LO_CONNECTOR_LENGTH, ControlPosY + ConnectorYOffset );
	}
}

/** 
 * Draw this SkelControl in the AnimTreeEditor.
 *
 * @param	Canvas			The canvas to use.
 * @param	bSelected		TRUE if this node is selected.
 * @param	bCurves			If TRUE, render links as splines; if FALSE, render links as straight lines.
 */
void USkelControlBase::DrawSkelControl(FCanvas* Canvas, UBOOL bSelected, UBOOL bCurves)
{
	FLinkedObjDrawInfo ObjInfo;
	ObjInfo.Inputs.AddItem( FLinkedObjConnInfo(TEXT("In"),SkelControlTitleColorDisabled) );
	ObjInfo.Outputs.AddItem( FLinkedObjConnInfo(TEXT("Out"),SkelControlTitleColorDisabled) );
	ObjInfo.ObjObject = this;

	const FString ControlTitle = FString::Printf( TEXT("%s : %s"), *GetClass()->GetDescription(), *ControlName.ToString());
	const FColor BorderColor = bSelected ? FColor(255, 255, 0) : FColor(0, 0, 0);
	const FIntPoint ControlPos(ControlPosX, ControlPosY);

	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HLinkedObjProxy(this) );
	const FColor BackGroundColor = (GetClass()->ClassFlags & CLASS_Deprecated) ? AnimTree_DeprecatedBGColor : SkelControlTitleColorDisabled;
	FLinkedObjDrawUtils::DrawLinkedObj(Canvas, ObjInfo, *ControlTitle, NULL, BorderColor, BackGroundColor, ControlPos);
	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );

	DrawWidth = ObjInfo.DrawWidth;

	const FString StrengthString = FString::Printf( TEXT("%3.2f"), ControlStrength );

	const FColor SliderBackGroundColor = (GetClass()->ClassFlags & CLASS_Deprecated) ? AnimTree_DeprecatedBGColor : FColor(140,140,140);

	FLinkedObjDrawUtils::DrawSlider(Canvas, FIntPoint(ControlPosX, ControlPosY + ObjInfo.DrawHeight), DrawWidth - 15, BorderColor, SliderBackGroundColor, ControlStrength, StrengthString, this );

	// Draw little button to toggle auto-blend.
	if ( FLinkedObjDrawUtils::AABBLiesWithinViewport( Canvas, ControlPosX + DrawWidth - 15 + 1, ControlPosY + ObjInfo.DrawHeight - 1, 14, LO_SLIDER_HANDLE_HEIGHT + 4 ) )
	{
		if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HLinkedObjProxySpecial(this, 1) );
		DrawTile( Canvas, ControlPosX + DrawWidth - 15 + 1, ControlPosY + ObjInfo.DrawHeight - 1, 14, LO_SLIDER_HANDLE_HEIGHT + 4, 0.f, 0.f, 1.f, 1.f, BorderColor );
		DrawTile( Canvas, ControlPosX + DrawWidth - 15 + 2, ControlPosY + ObjInfo.DrawHeight + 0, 14 - 2, LO_SLIDER_HANDLE_HEIGHT + 4 - 2, 0.f, 0.f, 1.f, 1.f, FColor(255,128,0) );
		if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );
	}

	// Now draw to next node, if there is one.
	if(NextControl)
	{
		const FIntPoint Start	= GetConnectionLocation(LOC_OUTPUT);
		const FIntPoint End		= NextControl->GetConnectionLocation(LOC_INPUT);
		if ( bCurves )
		{
			const FLOAT Tension		= Abs<INT>(Start.X - End.X);
			FLinkedObjDrawUtils::DrawSpline(Canvas, Start, Tension * FVector2D(1,0), End, Tension * FVector2D(1,0), SkelControlTitleColorDisabled, true);
		}
		else
		{
			DrawLine2D(Canvas, Start, End, SkelControlTitleColorDisabled);
			const FVector2D Dir( FVector2D(Start) - FVector2D(End) );
			FLinkedObjDrawUtils::DrawArrowhead(Canvas, End, Dir.SafeNormal(), SkelControlTitleColorDisabled);
		}
	}
}
