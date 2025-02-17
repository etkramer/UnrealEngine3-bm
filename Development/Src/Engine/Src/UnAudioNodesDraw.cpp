/*=============================================================================
	UnAudioNodesDraw.cpp: Unreal audio node drawing support.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h" 
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"
#include "UnLinkedObjDrawUtils.h"

#define SCE_CAPTION_HEIGHT		22
#define SCE_CAPTION_XPAD		4
#define SCE_CONNECTOR_WIDTH		8
#define SCE_CONNECTOR_LENGTH	10
#define SCE_CONNECTOR_SPACING	10

#define SCE_TEXT_BORDER			3
#define SCE_MIN_NODE_DIM		64

// Though this function could just traverse tree and draw all nodes that way, passing in the array means we can use it in the SoundCueEditor where not 
// all nodes are actually connected to the tree.
void USoundCue::DrawCue(FCanvas* Canvas, TArray<USoundNode*>& SelectedNodes)
{
	// First draw speaker 
	UTexture2D* SpeakerTexture = LoadObject<UTexture2D>(NULL, TEXT("EngineMaterials.SoundCue_SpeakerIcon"), NULL, LOAD_None, NULL);
	if(!SpeakerTexture)
		return;

	DrawTile(Canvas,-128 - SCE_CONNECTOR_LENGTH, -64, 128, 128, 0.f, 0.f, 1.f, 1.f, FLinearColor::White, SpeakerTexture->Resource);

	// Draw special 'root' input connector.
	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HLinkedObjConnectorProxy(this, LOC_INPUT, 0) );
	DrawTile(Canvas, -SCE_CONNECTOR_LENGTH, -SCE_CONNECTOR_WIDTH / 2, SCE_CONNECTOR_LENGTH, SCE_CONNECTOR_WIDTH, 0.f, 0.f, 0.f, 0.f, FLinearColor::Black );
	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL);

	// Draw link to first node
	if(FirstNode)
	{
		FSoundNodeEditorData* EndEdData = EditorData.Find( FirstNode );
		if(EndEdData)
		{
			FIntPoint EndPos = FirstNode->GetConnectionLocation(Canvas, LOC_OUTPUT, 0, *EndEdData);
			DrawLine2D(Canvas, FVector2D(0, 0), FVector2D(EndPos.X, EndPos.Y), FColor(0,0,0) );
		}
	}

	// Draw each SoundNode (will draw links as well)
	for( TMap<USoundNode*, FSoundNodeEditorData>::TIterator It(EditorData);It;++It )
	{
		USoundNode* Node = It.Key();

		// the node was null so just continue to the next one
		//@todo audio:  should this ever be null?
		if( Node == NULL )
		{
			continue;
		}

		UBOOL bNodeIsSelected = SelectedNodes.ContainsItem( Node );		

		FSoundNodeEditorData EdData = It.Value();

		Node->DrawSoundNode(Canvas, EdData, bNodeIsSelected);

		// Draw lines to children
		for(INT i=0; i<Node->ChildNodes.Num(); i++)
		{
			if( Node->ChildNodes(i) )
			{
				FIntPoint StartPos = Node->GetConnectionLocation(Canvas, LOC_INPUT, i, EdData);

				FSoundNodeEditorData* EndEdData = EditorData.Find( Node->ChildNodes(i) );
				if(EndEdData)
				{
					FIntPoint EndPos = Node->ChildNodes(i)->GetConnectionLocation(Canvas, LOC_OUTPUT, 0, *EndEdData);
					DrawLine2D(Canvas, StartPos, EndPos, FColor(0,0,0) );
				}
			}
		}
	}
}

void USoundNode::DrawSoundNode(FCanvas* Canvas, const FSoundNodeEditorData& EdData, UBOOL bSelected)
{
	FString Description;
	if( IsA(USoundNodeWave::StaticClass()) )
		Description = GetFName().ToString();
	else
		Description = GetClass()->GetDescription();

	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HLinkedObjProxy(this) );
	
	INT NodePosX = EdData.NodePosX;
	INT NodePosY = EdData.NodePosY;
	INT Width = SCE_MIN_NODE_DIM;
	INT Height = SCE_MIN_NODE_DIM;

	INT XL, YL;
	StringSize( GEngine->SmallFont, XL, YL, *Description );
	Width = Max( Width, XL + 2*SCE_TEXT_BORDER + 2*SCE_CAPTION_XPAD );

	INT NumConnectors = Max( 1, ChildNodes.Num() );

	Height = Max( Height, SCE_CAPTION_HEIGHT + NumConnectors*SCE_CONNECTOR_WIDTH + (NumConnectors+1)*SCE_CONNECTOR_SPACING );

	FColor BorderColor = bSelected ? FColor(255,255,0) : FColor(0,0,0);	

	// Draw basic box
	DrawTile(Canvas,NodePosX,		NodePosY,						Width,		Height,					0.f, 0.f, 0.f, 0.f, BorderColor );
	DrawTile(Canvas,NodePosX+1,	NodePosY+1,						Width-2,	Height-2,				0.f, 0.f, 0.f, 0.f, FColor(112,112,112) );
	DrawTile(Canvas,NodePosX+1,	NodePosY+SCE_CAPTION_HEIGHT,	Width-2,	1,						0.f, 0.f, 0.f, 0.f, FLinearColor::Black );

	// Draw title bar
	DrawTile(Canvas,NodePosX+1,	NodePosY+1,						Width-2,	SCE_CAPTION_HEIGHT-1,	0.f, 0.f, 0.f, 0.f, FColor(140,140,140) );
	DrawShadowedString(Canvas, NodePosX + (Width - XL) / 2, NodePosY + (SCE_CAPTION_HEIGHT - YL) / 2, *Description, GEngine->SmallFont, FColor(255,255,128) );

	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );

	// Draw Output connector

	INT ConnectorRangeY = Height - SCE_CAPTION_HEIGHT;
	INT CenterY = NodePosY + SCE_CAPTION_HEIGHT + ConnectorRangeY / 2;

	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HLinkedObjConnectorProxy(this, LOC_OUTPUT, 0) );
	DrawTile(Canvas, NodePosX - SCE_CONNECTOR_LENGTH, CenterY - SCE_CONNECTOR_WIDTH / 2, SCE_CONNECTOR_LENGTH, SCE_CONNECTOR_WIDTH, 0.f, 0.f, 0.f, 0.f, FLinearColor::Black );
	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );

	// Draw input connectors
	if( ChildNodes.Num() > 0 )
	{

		INT SpacingY = ConnectorRangeY/ChildNodes.Num();
		INT StartY = CenterY - (ChildNodes.Num()-1) * SpacingY /  2;

		for(INT i=0; i<ChildNodes.Num(); i++)
		{
			INT LinkY = StartY + i * SpacingY;

			if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HLinkedObjConnectorProxy(this, LOC_INPUT, i) );
			DrawTile(Canvas, NodePosX + Width, LinkY - SCE_CONNECTOR_WIDTH / 2, SCE_CONNECTOR_LENGTH, SCE_CONNECTOR_WIDTH, 0.f, 0.f, 0.f, 0.f, FLinearColor::Black );
			if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );
		}
	}
}

FIntPoint USoundNode::GetConnectionLocation(FCanvas* Canvas, INT ConnType, INT ConnIndex, const FSoundNodeEditorData& EdData)
{
	FString Description;
	if( IsA(USoundNodeWave::StaticClass()) )
		Description = GetFName().ToString();
	else
		Description = GetClass()->GetDescription();

	INT NodePosX = EdData.NodePosX;
	INT NodePosY = EdData.NodePosY;

	INT NumConnectors = Max( 1, ChildNodes.Num() );
	INT Height = SCE_MIN_NODE_DIM;
	Height = Max( Height, SCE_CAPTION_HEIGHT + NumConnectors*SCE_CONNECTOR_WIDTH + (NumConnectors+1)*SCE_CONNECTOR_SPACING );

	INT ConnectorRangeY = Height - SCE_CAPTION_HEIGHT;

	if(ConnType == LOC_OUTPUT)
	{
		check(ConnIndex == 0);

		INT NodeOutputYOffset = SCE_CAPTION_HEIGHT + ConnectorRangeY / 2;

		return FIntPoint(NodePosX - SCE_CONNECTOR_LENGTH, NodePosY + NodeOutputYOffset);
	}
	else
	{
		check( ConnIndex >= 0 && ConnIndex < ChildNodes.Num() );

		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, *Description );

		INT NodeDrawWidth = SCE_MIN_NODE_DIM;
		NodeDrawWidth = Max( NodeDrawWidth, XL + 2*SCE_TEXT_BORDER );

		INT SpacingY = ConnectorRangeY/ChildNodes.Num();
		INT CenterYOffset = SCE_CAPTION_HEIGHT + ConnectorRangeY / 2;
		INT StartYOffset = CenterYOffset - (ChildNodes.Num()-1) * SpacingY / 2;
		INT NodeYOffset = StartYOffset + ConnIndex * SpacingY;

		return FIntPoint(NodePosX + NodeDrawWidth + SCE_CONNECTOR_LENGTH, NodePosY + NodeYOffset);
	}
}
