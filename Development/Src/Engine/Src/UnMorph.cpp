/*=============================================================================
	UnMorph.cpp
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "UnLinkedObjDrawUtils.h"

IMPLEMENT_CLASS(UMorphTarget);
IMPLEMENT_CLASS(UMorphTargetSet);
IMPLEMENT_CLASS(UMorphWeightSequence);

IMPLEMENT_CLASS(UMorphNodeBase);
IMPLEMENT_CLASS(UMorphNodeWeightBase);
IMPLEMENT_CLASS(UMorphNodeWeight);
IMPLEMENT_CLASS(UMorphNodePose);

static const FColor MorphConnColor(50,50,100);
static const FColor MorphWeightColor(60,60,90);
/** BackGround color when a node has been deprecated */
static const FColor Morph_DeprecatedBGColor(200,0,0);

#define ZERO_MORPHWEIGHT_THRESH (0.01f)  // Below this weight threshold, morphs won't be blended in.


//////////////////////////////////////////////////////////////////////////
// UMorphTargetSet
//////////////////////////////////////////////////////////////////////////

FString UMorphTargetSet::GetDesc()
{
	return FString::Printf( TEXT("%d MorphTargets"), Targets.Num() );
}

/** Find a morph target by name in this MorphTargetSet. */ 
UMorphTarget* UMorphTargetSet::FindMorphTarget(FName MorphTargetName)
{
	if(MorphTargetName == NAME_None)
	{
		return NULL;
	}

	for(INT i=0; i<Targets.Num(); i++)
	{
		if( Targets(i)->GetFName() == MorphTargetName )
		{
			return Targets(i);
		}
	}

	return NULL;
}


INT UMorphTargetSet::GetResourceSize()
{
	INT Retval = 0;

	FArchiveCountMem CountBytesSize( this );
	INT ResourceSize = CountBytesSize.GetNum();
	for( INT i=0; i < Targets.Num(); ++i )
	{
		ResourceSize += Targets(i)->GetResourceSize();
	}

	Retval = ResourceSize;

	return Retval;
}


//////////////////////////////////////////////////////////////////////////
// UMorphWeightSequence
//////////////////////////////////////////////////////////////////////////

FString UMorphWeightSequence::GetDesc()
{
	return FString( TEXT("") );
}

//////////////////////////////////////////////////////////////////////////
// UMorphNodeBase
//////////////////////////////////////////////////////////////////////////

void UMorphNodeBase::InitMorphNode(USkeletalMeshComponent* InSkelComp)
{
	// Save a reference to the SkeletalMeshComponent that owns this MorphNode.
	SkelComponent = InSkelComp;
}

void UMorphNodeBase::GetNodes(TArray<UMorphNodeBase*>& OutNodes)
{
	// Add myself to the list.
	OutNodes.AddUniqueItem(this);
}

FIntPoint UMorphNodeBase::GetConnectionLocation(INT ConnType, INT ConnIndex)
{
	if(ConnType == LOC_INPUT)
	{
		check(ConnIndex == 0);
		return FIntPoint( NodePosX - LO_CONNECTOR_LENGTH, OutDrawY );
	}

	return FIntPoint(0,0);
}

//////////////////////////////////////////////////////////////////////////
// UMorphNodeWeightBase
//////////////////////////////////////////////////////////////////////////

void UMorphNodeWeightBase::GetNodes(TArray<UMorphNodeBase*>& OutNodes)
{
	// Add myself to the list.
	OutNodes.AddUniqueItem(this);

	// Iterate over each connector
	for(INT i=0; i<NodeConns.Num(); i++)
	{
		FMorphNodeConn& Conn = NodeConns(i);

		// Iterate over each link from this connector.
		for(INT j=0; j<Conn.ChildNodes.Num(); j++)
		{
			// If there is a child node, call GetNodes on it.
			if( Conn.ChildNodes(j) )
			{
				Conn.ChildNodes(j)->GetNodes(OutNodes);
			}
		}
	}
}

void UMorphNodeWeightBase::DrawMorphNode(FCanvas* Canvas, UBOOL bSelected, UBOOL bCurves)
{
	// Construct the FLinkedObjDrawInfo for use the linked-obj drawing utils.
	FLinkedObjDrawInfo ObjInfo;

	// AnimTree's don't have an output connector on left
	ObjInfo.Inputs.AddItem( FLinkedObjConnInfo(TEXT("Out"), MorphConnColor ) );

	// Add output for each child.
	for(INT i=0; i<NodeConns.Num(); i++)
	{
		ObjInfo.Outputs.AddItem( FLinkedObjConnInfo(*NodeConns(i).ConnName.ToString(), MorphConnColor) );
	}

	ObjInfo.ObjObject = this;

	// Generate border color
	const FColor BorderColor( bSelected ? FColor(255,255,0) : FColor(0,0,0) );

	// Generate name for node. User-give name if entered - otherwise just class name.
	FString NodeTitle;
	FString NodeDesc = GetClass()->GetDescription(); // Need to assign it here, so pointer isn't invalid by the time we draw it.
	if( NodeName != NAME_None )
	{
		NodeTitle = NodeName.ToString();
	}
	else
	{
		NodeTitle = NodeDesc;
	}

	// Use util to draw box with connectors etc.
	const FColor BackGroundColor = (GetClass()->ClassFlags & CLASS_Deprecated) ? Morph_DeprecatedBGColor : MorphWeightColor;
	FLinkedObjDrawUtils::DrawLinkedObj( Canvas, ObjInfo, *NodeTitle, NULL, BorderColor, BackGroundColor, FIntPoint(NodePosX, NodePosY) );

	// Read back draw locations of connectors, so we can draw lines in the correct places.
	OutDrawY = ObjInfo.InputY(0);

	for(INT i=0; i<NodeConns.Num(); i++)
	{
		NodeConns(i).DrawY = ObjInfo.OutputY(i);
	}

	DrawWidth = ObjInfo.DrawWidth;
	DrawHeight = ObjInfo.DrawHeight;

	// If desired, draw a slider for this node.
	if( bDrawSlider )
	{
		const INT SliderDrawY = NodePosY + ObjInfo.DrawHeight;
		const FColor SliderBackGroundColor = (GetClass()->ClassFlags & CLASS_Deprecated) ? Morph_DeprecatedBGColor : FColor(140,140,140);
		FLinkedObjDrawUtils::DrawSlider(Canvas, FIntPoint(NodePosX, SliderDrawY), DrawWidth, BorderColor, SliderBackGroundColor, GetSliderPosition(), FString(TEXT("")), this, 0, FALSE);
	}

	// Iterate over each connector
	for(INT i=0; i<NodeConns.Num(); i++)
	{
		FMorphNodeConn& Conn = NodeConns(i);

		// Iterate over each link from this connector.
		for(INT j=0; j<Conn.ChildNodes.Num(); j++)
		{
			// If there is a child node, call GetNodes on it.
			UMorphNodeBase* ChildNode = Conn.ChildNodes(j);
			if( ChildNode )
			{
				const FIntPoint Start	= GetConnectionLocation(LOC_OUTPUT, i);
				const FIntPoint End		= ChildNode->GetConnectionLocation(LOC_INPUT, 0);
				if ( bCurves )
				{
					const FLOAT Tension		= Abs<INT>(Start.X - End.X);
					FLinkedObjDrawUtils::DrawSpline(Canvas, End, -Tension * FVector2D(1,0), Start, -Tension * FVector2D(1,0), MorphConnColor, TRUE);
				}
				else
				{
					DrawLine2D( Canvas, Start, End, MorphConnColor );
					const FVector2D Dir( FVector2D(Start) - FVector2D(End) );
					FLinkedObjDrawUtils::DrawArrowhead( Canvas, End, Dir.SafeNormal(), MorphConnColor );
				}
			}
		}
	}
}

FIntPoint UMorphNodeWeightBase::GetConnectionLocation(INT ConnType, INT ConnIndex)
{
	if(ConnType == LOC_INPUT)
	{
		check(ConnIndex == 0);
		return FIntPoint( NodePosX - LO_CONNECTOR_LENGTH, OutDrawY );
	}
	else if(ConnType == LOC_OUTPUT)
	{
		check( ConnIndex >= 0 && ConnIndex < NodeConns.Num() );
		return FIntPoint( NodePosX + DrawWidth + LO_CONNECTOR_LENGTH, NodeConns(ConnIndex).DrawY );
	}

	return FIntPoint(0,0);
}

//////////////////////////////////////////////////////////////////////////
// UMorphNodeWeight
//////////////////////////////////////////////////////////////////////////

void UMorphNodeWeight::SetNodeWeight(FLOAT NewWeight)
{
	NodeWeight = NewWeight;
}

void UMorphNodeWeight::GetActiveMorphs(TArray<FActiveMorph>& OutMorphs)
{
	// If weight is low enough, do nothing (add no morph targets)
	if(NodeWeight < ZERO_MORPHWEIGHT_THRESH)
	{
		return;
	}

	// This node should only have one connector.
	check(NodeConns.Num() == 1);
	FMorphNodeConn& Conn = NodeConns(0);

	// Temp storage.
	TArray<FActiveMorph> TempMorphs;

	// Iterate over each link from this connector.
	for(INT j=0; j<Conn.ChildNodes.Num(); j++)
	{
		// If there is a child node, call GetActiveMorphs on it.
		if( Conn.ChildNodes(j) )
		{
			TempMorphs.Empty();
			Conn.ChildNodes(j)->GetActiveMorphs(TempMorphs);

			// Iterate over each active morph, scaling it by this nodes weight, and adding to output array.
			for(INT k=0; k<TempMorphs.Num(); k++)
			{
				OutMorphs.AddItem( FActiveMorph(TempMorphs(k).Target, TempMorphs(k).Weight * NodeWeight) );
			}
		}
	}
}

FLOAT UMorphNodeWeight::GetSliderPosition()
{
	return NodeWeight;
}

void UMorphNodeWeight::HandleSliderMove(FLOAT NewSliderValue)
{
	NodeWeight = NewSliderValue;
}


//////////////////////////////////////////////////////////////////////////
// UMorphNodePose
//////////////////////////////////////////////////////////////////////////

/** If someone changes name of animation - update to take affect. */
void UMorphNodePose::PostEditChange(UProperty* PropertyThatChanged)
{
	SetMorphTarget(MorphName);

	Super::PostEditChange(PropertyThatChanged);
}

/** 
 *	Set the MorphTarget to use for this MorphNodePose by name. 
 *	Will find it in the owning SkeletalMeshComponent MorphSets array using FindMorphTarget.
 */
void UMorphNodePose::SetMorphTarget(FName MorphTargetName)
{
	MorphName = MorphTargetName;
	Target = NULL;

	if(MorphTargetName == NAME_None || !SkelComponent)
	{
		return;
	}

	Target = SkelComponent->FindMorphTarget(MorphTargetName);
	if(!Target)
	{
		warnf(NAME_DevAnim, TEXT("%s - Failed to find MorphTarget '%s' on SkeletalMeshComponent: %s"),
			*GetName(),
			*MorphTargetName.ToString(),
			*SkelComponent->GetFullName()
			);
	}
}

void UMorphNodePose::GetActiveMorphs(TArray<FActiveMorph>& OutMorphs)
{
	// If we have a target, add it to the array.
	if( Target )
	{
		OutMorphs.AddItem( FActiveMorph(Target, Weight) );
		// @todo see if its already in there.
	}
}

void UMorphNodePose::InitMorphNode(USkeletalMeshComponent* InSkelComp)
{
	Super::InitMorphNode(InSkelComp);

	// On initialize, look up the morph target by the name we have saved.
	SetMorphTarget(MorphName);
}

/** 
 * Draws this morph node in the AnimTreeEditor.
 *
 * @param	Canvas			The canvas to use.
 * @param	bSelected		TRUE if this node is selected.
 * @param	bCurves			If TRUE, render links as splines; if FALSE, render links as straight lines.
 */
void UMorphNodePose::DrawMorphNode(FCanvas* Canvas, UBOOL bSelected, UBOOL bCurves)
{
	FLinkedObjDrawInfo ObjInfo;

	ObjInfo.Inputs.AddItem( FLinkedObjConnInfo(TEXT("Out"), MorphConnColor ) );
	ObjInfo.ObjObject = this;

	const FColor BorderColor( bSelected ? FColor(255,255,0) : FColor(0,0,0) );
	const FIntPoint Point(NodePosX, NodePosY);
	FLinkedObjDrawUtils::DrawLinkedObj( Canvas, ObjInfo, *MorphName.ToString(), NULL, BorderColor, MorphConnColor, Point );

	DrawWidth = ObjInfo.DrawWidth;
	OutDrawY = ObjInfo.InputY(0);
}
