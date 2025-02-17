/*=============================================================================
	UnLinkedObjDrawUtils.cpp: Utils for drawing linked objects.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnLinkedObjDrawUtils.h"

/** Minimum viewport zoom at which text will be rendered. */
static const FLOAT	TextMinZoom(0.3f);

/** Minimum viewport zoom at which arrowheads will be rendered. */
static const FLOAT	ArrowheadMinZoom(0.3f);

/** Minimum viewport zoom at which connectors will be rendered. */
static const FLOAT	ConnectorMinZoom(0.2f);

/** Minimum viewport zoom at which connectors will be rendered. */
static const FLOAT	SliderMinZoom(0.2f);

static const FLOAT	MaxPixelsPerStep(15.f);

static const INT	ArrowheadLength(14);
static const INT	ArrowheadWidth(4);

static const FColor SliderHandleColor(0, 0, 0);


void FLinkedObjDrawUtils::DrawNGon(FCanvas* Canvas, const FVector2D& Center, const FColor& Color, INT NumSides, FLOAT Radius)
{
	if ( AABBLiesWithinViewport( Canvas, Center.X-Radius, Center.Y-Radius, Radius*2, Radius*2) )
	{
		FVector2D Verts[256];
		NumSides = Clamp(NumSides, 3, 255);

		for(INT i=0; i<NumSides+1; i++)
		{
			const FLOAT Angle = (2 * (FLOAT)PI) * (FLOAT)i/(FLOAT)NumSides;
			Verts[i] = Center + FVector2D( Radius*appCos(Angle), Radius*appSin(Angle) );
		}

	    for(INT i=0; i<NumSides; i++)
	    {
		    DrawTriangle2D(
			    Canvas,
			    FVector2D(Center), FVector2D(0,0),
			    FVector2D(Verts[i+0]), FVector2D(0,0),
			    FVector2D(Verts[i+1]), FVector2D(0,0),
			    Color
			    );
	    }
	}
}

/**
 *	@param EndDir End tangent. Note that this points in the same direction as StartDir ie 'along' the curve. So if you are dealing with 'handles' you will need to invert when you pass it in.
 *
 */
void FLinkedObjDrawUtils::DrawSpline(FCanvas* Canvas, const FIntPoint& Start, const FVector2D& StartDir, const FIntPoint& End, const FVector2D& EndDir, const FColor& LineColor, UBOOL bArrowhead, UBOOL bInterpolateArrowDirection/*=FALSE*/)
{
	const INT MinX = Min( Start.X, End.X );
	const INT MaxX = Max( Start.X, End.X );
	const INT MinY = Min( Start.Y, End.Y );
	const INT MaxY = Max( Start.Y, End.Y );

	if ( AABBLiesWithinViewport( Canvas, MinX, MinY, MaxX - MinX, MaxY - MinY ) )
	{
		// Don't draw the arrowhead if the editor is zoomed out most of the way.
		const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
		if ( Zoom2D < ArrowheadMinZoom )
		{
			bArrowhead = FALSE;
		}

		const FVector2D StartVec( Start );
		const FVector2D EndVec( End );

		// Rough estimate of length of curve. Use direct length and distance between 'handles'. Sure we can do better.
		const FLOAT DirectLength = (EndVec - StartVec).Size();
		const FLOAT HandleLength = ((EndVec - EndDir) - (StartVec + StartDir)).Size();
		
		const INT NumSteps = appCeil(Max(DirectLength,HandleLength)/MaxPixelsPerStep);

		FVector2D OldPos = StartVec;

		for(INT i=0; i<NumSteps; i++)
		{
			const FLOAT Alpha = ((FLOAT)i+1.f)/(FLOAT)NumSteps;
			const FVector2D NewPos = CubicInterp(StartVec, StartDir, EndVec, EndDir, Alpha);

			const FIntPoint OldIntPos = OldPos.IntPoint();
			const FIntPoint NewIntPos = NewPos.IntPoint();

			DrawLine2D( Canvas, OldIntPos, NewIntPos, LineColor );

			// If this is the last section, use its direction to draw the arrowhead.
			if( (i == NumSteps-1) && (i >= 2) && bArrowhead )
			{
				// Go back 3 steps to give us decent direction for arrowhead
				FVector2D ArrowStartPos;

				if(bInterpolateArrowDirection)
				{
					const FLOAT ArrowStartAlpha = ((FLOAT)i-2.f)/(FLOAT)NumSteps;
					ArrowStartPos = CubicInterp(StartVec, StartDir, EndVec, EndDir, ArrowStartAlpha);
				}
				else
				{
					ArrowStartPos = OldPos;
				}

				const FVector2D StepDir = (NewPos - ArrowStartPos).SafeNormal();
				DrawArrowhead( Canvas, NewIntPos, StepDir, LineColor );
			}

			OldPos = NewPos;
		}
	}
}

void FLinkedObjDrawUtils::DrawArrowhead(FCanvas* Canvas, const FIntPoint& Pos, const FVector2D& Dir, const FColor& Color)
{
	// Don't draw the arrowhead if the editor is zoomed out most of the way.
	const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
	if ( Zoom2D > ArrowheadMinZoom )
	{
		const FVector2D Orth(Dir.Y, -Dir.X);
		const FVector2D PosVec(Pos);
		const FVector2D pt2 = PosVec - (Dir * ArrowheadLength) - (Orth * ArrowheadWidth);
		const FVector2D pt1 = PosVec;
		const FVector2D pt3 = PosVec - (Dir * ArrowheadLength) + (Orth * ArrowheadWidth);
		DrawTriangle2D(Canvas,
		    pt1,FVector2D(0,0),
		    pt2,FVector2D(0,0),
		    pt3,FVector2D(0,0),
		    Color,NULL,0);
	}
}


FIntPoint FLinkedObjDrawUtils::GetTitleBarSize(FCanvas* Canvas, const TCHAR* Name)
{
	INT XL, YL;
	StringSize( GEngine->SmallFont, XL, YL, Name );

	const INT LabelWidth = XL + (LO_TEXT_BORDER*2) + 4;

	return FIntPoint( Max(LabelWidth, LO_MIN_SHAPE_SIZE), LO_CAPTION_HEIGHT );
}


void FLinkedObjDrawUtils::DrawTitleBar(FCanvas* Canvas, const FIntPoint& Pos, const FIntPoint& Size, const FColor& BorderColor, const FColor& BkgColor, const TCHAR* Name, const TCHAR* Comment, const TCHAR* Comment2)
{
	// Draw label at top
	if ( AABBLiesWithinViewport( Canvas, Pos.X, Pos.Y, Size.X, Size.Y ) )
	{
		DrawTile( Canvas, Pos.X,		Pos.Y,		Size.X,		Size.Y,		0.0f,0.0f,0.0f,0.0f, BorderColor );
		DrawTile( Canvas, Pos.X+1,	Pos.Y+1,	Size.X-2,	Size.Y-2,	0.0f,0.0f,0.0f,0.0f, BkgColor );
	}

	if ( Name )
	{
		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, Name );

		const FIntPoint StringPos( Pos.X+((Size.X-XL)/2), Pos.Y+((Size.Y-YL)/2)+1 );
		if ( AABBLiesWithinViewport( Canvas, StringPos.X, StringPos.Y, XL, YL ) )
		{
			DrawShadowedString( Canvas, StringPos.X, StringPos.Y, Name, GEngine->SmallFont, FColor(255,255,128) );
		}
	}

	const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
	INT CommentY = Pos.Y - 2;

	// See if there is a second line of comment - that goes closest to the window (below Comment)
	if( !Canvas->IsHitTesting() && Comment2 )
	{
		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, Comment2 );
		CommentY -= YL;

		const FIntPoint StringPos( Pos.X + 2, CommentY );
		if ( AABBLiesWithinViewport( Canvas, StringPos.X, StringPos.Y, XL, YL ) )
		{
			DrawString( Canvas, StringPos.X, StringPos.Y, Comment2, GEngine->SmallFont, FColor(0,0,0) );
			if( Zoom2D > 1.f - DELTA )
			{
				DrawString( Canvas, StringPos.X+1, StringPos.Y, Comment2, GEngine->SmallFont, FColor(120,120,255) );
			}
		}
		CommentY -= 2;
	}

	// Level designer description above the window
	if( !Canvas->IsHitTesting() && Comment )
	{
		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, Comment );
		CommentY -= YL;

		const FIntPoint StringPos( Pos.X + 2, CommentY );
		if ( AABBLiesWithinViewport( Canvas, StringPos.X, StringPos.Y, XL, YL ) )
		{
			DrawString( Canvas, StringPos.X, StringPos.Y, Comment, GEngine->SmallFont, FColor(0,0,0) );
			if( Zoom2D > 1.f - DELTA )
			{
				DrawString( Canvas, StringPos.X+1, StringPos.Y, Comment, GEngine->SmallFont, FColor(64,64,192) );
			}
		}
	}
}



// The InputY and OuputY are optional extra outputs, giving the size of the input and output connectors

FIntPoint FLinkedObjDrawUtils::GetLogicConnectorsSize(FCanvas* Canvas, const FLinkedObjDrawInfo& ObjInfo, INT* InputY, INT* OutputY)
{
	INT MaxInputDescX = 0;
	INT MaxInputDescY = 0;
	for(INT i=0; i<ObjInfo.Inputs.Num(); i++)
	{
		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, *ObjInfo.Inputs(i).Name );

		MaxInputDescX = Max(XL, MaxInputDescX);

		if(i>0) MaxInputDescY += LO_DESC_Y_PADDING;
		MaxInputDescY += Max(YL, LO_CONNECTOR_WIDTH);
	}

	INT MaxOutputDescX = 0;
	INT MaxOutputDescY = 0;
	for(INT i=0; i<ObjInfo.Outputs.Num(); i++)
	{
		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, *ObjInfo.Outputs(i).Name );

		MaxOutputDescX = Max(XL, MaxOutputDescX);

		if(i>0) MaxOutputDescY += LO_DESC_Y_PADDING;
		MaxOutputDescY += Max(YL, LO_CONNECTOR_WIDTH);
	}

	const INT NeededX = MaxInputDescX + MaxOutputDescX + LO_DESC_X_PADDING + (2*LO_TEXT_BORDER);
	const INT NeededY = Max( MaxInputDescY, MaxOutputDescY ) + (2*LO_TEXT_BORDER);

	if(InputY)
	{
		*InputY = MaxInputDescY + (2*LO_TEXT_BORDER);
	}

	if(OutputY)
	{
		*OutputY = MaxOutputDescY + (2*LO_TEXT_BORDER);
	}

	return FIntPoint(NeededX, NeededY);
}

void FLinkedObjDrawUtils::DrawLogicConnectors(FCanvas* Canvas, FLinkedObjDrawInfo& ObjInfo, const FIntPoint& Pos, const FIntPoint& Size, const FLinearColor* ConnectorTileBackgroundColor)
{
	const UBOOL bHitTesting				= Canvas->IsHitTesting();
	const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
	const UBOOL bSufficientlyZoomedIn	= Zoom2D > ConnectorMinZoom;

	INT XL, YL;
	StringSize(GEngine->SmallFont, XL, YL, TEXT("GgIhy"));

	const INT ConnectorWidth	= Max(YL, LO_CONNECTOR_WIDTH);
	const INT ConnectorRangeY	= Size.Y - 2*LO_TEXT_BORDER;
	const INT CenterY			= Pos.Y + LO_TEXT_BORDER + ConnectorRangeY / 2;

	// Do nothing if no Input connectors
	if( ObjInfo.Inputs.Num() > 0 )
	{
		const INT SpacingY	= ConnectorRangeY/ObjInfo.Inputs.Num();
		const INT StartY	= CenterY - (ObjInfo.Inputs.Num()-1) * SpacingY / 2;
		ObjInfo.InputY.Add( ObjInfo.Inputs.Num() );

		for(INT i=0; i<ObjInfo.Inputs.Num(); i++)
		{
			const INT LinkY		= StartY + i * SpacingY;
			ObjInfo.InputY(i)	= LinkY;

			if ( bSufficientlyZoomedIn )
			{
				if(bHitTesting) Canvas->SetHitProxy( new HLinkedObjConnectorProxy(ObjInfo.ObjObject, LOC_INPUT, i) );
				DrawTile( Canvas, Pos.X - LO_CONNECTOR_LENGTH, LinkY - LO_CONNECTOR_WIDTH / 2, LO_CONNECTOR_LENGTH, LO_CONNECTOR_WIDTH, 0.f, 0.f, 0.f, 0.f, ObjInfo.Inputs(i).Color );
				if(bHitTesting) Canvas->SetHitProxy( NULL );

				StringSize( GEngine->SmallFont, XL, YL, *ObjInfo.Inputs(i).Name );
				const FIntPoint StringPos( Pos.X + LO_TEXT_BORDER, LinkY - YL / 2 );
				if ( AABBLiesWithinViewport( Canvas, StringPos.X, StringPos.Y, XL, YL ) ) 
				{
					if ( ConnectorTileBackgroundColor )
					{
						DrawTile( Canvas, StringPos.X, StringPos.Y, XL, YL, 0.f, 0.f, 0.f, 0.f, *ConnectorTileBackgroundColor, NULL, TRUE );
					}
					DrawShadowedString( Canvas, StringPos.X, StringPos.Y, *ObjInfo.Inputs(i).Name, GEngine->SmallFont, FLinearColor::White );
				}
			}
		}
	}

	// Do nothing if no Output connectors
	if( ObjInfo.Outputs.Num() > 0 )
	{
		const INT SpacingY	= ConnectorRangeY/ObjInfo.Outputs.Num();
		const INT StartY	= CenterY - (ObjInfo.Outputs.Num()-1) * SpacingY / 2;
		ObjInfo.OutputY.Add( ObjInfo.Outputs.Num() );

		for(INT i=0; i<ObjInfo.Outputs.Num(); i++)
		{
			const INT LinkY		= StartY + i * SpacingY;
			ObjInfo.OutputY(i)	= LinkY;

			if ( bSufficientlyZoomedIn )
			{
				if(bHitTesting) Canvas->SetHitProxy( new HLinkedObjConnectorProxy(ObjInfo.ObjObject, LOC_OUTPUT, i) );
				DrawTile( Canvas, Pos.X + Size.X, LinkY - LO_CONNECTOR_WIDTH / 2, LO_CONNECTOR_LENGTH, LO_CONNECTOR_WIDTH, 0.f, 0.f, 0.f, 0.f, ObjInfo.Outputs(i).Color );
				if(bHitTesting) Canvas->SetHitProxy( NULL );

				StringSize( GEngine->SmallFont, XL, YL, *ObjInfo.Outputs(i).Name );
				const FIntPoint StringPos( Pos.X + Size.X - XL - LO_TEXT_BORDER, LinkY - YL / 2 );
				if ( AABBLiesWithinViewport( Canvas, StringPos.X, StringPos.Y, XL, YL ) )
				{
					if ( ConnectorTileBackgroundColor )
					{
						DrawTile( Canvas, StringPos.X, StringPos.Y, XL, YL, 0.f, 0.f, 0.f, 0.f, *ConnectorTileBackgroundColor, NULL, TRUE );
					}
					DrawShadowedString( Canvas, StringPos.X, StringPos.Y, *ObjInfo.Outputs(i).Name, GEngine->SmallFont, FLinearColor::White );
				}
			}
		}
	}
}

/**
 * Special version of string size that handles unique wrapping of variable names.
 */
static UBOOL VarStringSize(UFont *Font, INT &XL, INT &YL, FString Text, FString *LeftSplit = NULL, INT *LeftXL = NULL, FString *RightSplit = NULL, INT *RightXL = NULL)
{
	if (Text.Len() >= 4)
	{
		// walk through the string and find the wrap point (skip the first few chars since wrapping early would be pointless)
		for (INT Idx = 4; Idx < Text.Len(); Idx++)
		{
			TCHAR TextChar = Text[Idx];
			if (TextChar == ' ' || appIsUpper(TextChar))
			{
				// found wrap point, find the size of the first string
				FString FirstPart = Text.Left(Idx);
				FString SecondPart = Text.Right(Text.Len() - Idx);
				INT FirstXL, FirstYL, SecondXL, SecondYL;
				StringSize(Font, FirstXL, FirstYL, *FirstPart);
				StringSize(Font, SecondXL, SecondYL, *SecondPart);
				XL = Max(FirstXL, SecondXL);
				YL = FirstYL + SecondYL;
				if (LeftSplit != NULL)
				{
					*LeftSplit = FirstPart;
				}
				if (LeftXL != NULL)
				{
					*LeftXL = FirstXL;
				}
				if (RightSplit != NULL)
				{
					*RightSplit = SecondPart;
				}
				if (RightXL != NULL)
				{
					*RightXL = SecondXL;
				}
				return TRUE;
			}
		}
	}
	// no wrap, normal size
	StringSize(Font, XL, YL,*Text);
	return FALSE;
}

FIntPoint FLinkedObjDrawUtils::GetVariableConnectorsSize(FCanvas* Canvas, const FLinkedObjDrawInfo& ObjInfo)
{
	// sum the length of all the var/event names and add extra padding
	INT TotalXL = 0, MaxYL = 0;
	for (INT Idx = 0; Idx < ObjInfo.Variables.Num(); Idx++)
	{
		INT XL, YL;
		VarStringSize( GEngine->SmallFont, XL, YL, ObjInfo.Variables(Idx).Name );
		TotalXL += XL;
		MaxYL = Max(MaxYL,YL);
	}
	for (INT Idx = 0; Idx < ObjInfo.Events.Num(); Idx++)
	{
		INT XL, YL;
		VarStringSize( GEngine->SmallFont, XL, YL, ObjInfo.Events(Idx).Name );
		TotalXL += XL;
		MaxYL = Max(MaxYL,YL);
	}
	// add the final padding based on number of connectors
	TotalXL += (2 * LO_DESC_X_PADDING) * (ObjInfo.Variables.Num() + ObjInfo.Events.Num()) + (2 * LO_DESC_X_PADDING);
	return FIntPoint(TotalXL,MaxYL);
}


void FLinkedObjDrawUtils::DrawVariableConnectors(FCanvas* Canvas, FLinkedObjDrawInfo& ObjInfo, const FIntPoint& Pos, const FIntPoint& Size, const INT VarWidth)
{
	// Do nothing here if no variables or event connectors.
	if( ObjInfo.Variables.Num() == 0 && ObjInfo.Events.Num() == 0 )
	{
		return;
	}
	const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
	const UBOOL bHitTesting = Canvas->IsHitTesting();
	const UBOOL bSufficientlyZoomedIn = Zoom2D > ConnectorMinZoom;
	const INT LabelY = Pos.Y - LO_TEXT_BORDER;

	INT LastX = Pos.X, LastXL = 0;
	// if the title is wider than needed for the variables
	if (VarWidth < Size.X)
	{
		// then center the variables
		LastX += (Size.X - VarWidth)/2;
	}
	ObjInfo.VariableX.Add(ObjInfo.Variables.Num());
	FString LeftSplit, RightSplit;
	INT LeftXL, RightXL;
	for (INT Idx = 0; Idx < ObjInfo.Variables.Num(); Idx++)
	{
		INT VarX = LastX + LastXL + (LO_DESC_X_PADDING * 2);
		INT XL, YL;
		UBOOL bSplit = VarStringSize( GEngine->SmallFont, XL, YL, ObjInfo.Variables(Idx).Name, &LeftSplit, &LeftXL, &RightSplit, &RightXL );
		ObjInfo.VariableX(Idx) = VarX + XL/2;
		if ( bSufficientlyZoomedIn )
		{

			if(bHitTesting) Canvas->SetHitProxy( new HLinkedObjConnectorProxy(ObjInfo.ObjObject, LOC_VARIABLE, Idx) );
			if (ObjInfo.Variables(Idx).bOutput)
			{
				FIntPoint Vert0, Vert1, Vert2;
				Vert0.X = -2 + VarX + XL/2 - LO_CONNECTOR_WIDTH/2;
				Vert0.Y = Pos.Y + Size.Y;
				Vert1.X = Vert0.X + LO_CONNECTOR_WIDTH + 2;
				Vert1.Y = Vert0.Y;
				Vert2.X = (Vert1.X + Vert0.X) / 2;
				Vert2.Y = Vert0.Y + LO_CONNECTOR_LENGTH + 2;
				DrawTriangle2D(Canvas,Vert0,FVector2D(0,0),Vert1,FVector2D(0,0),Vert2,FVector2D(0,0),ObjInfo.Variables(Idx).Color);
			}
			else
			{
				DrawTile( Canvas, VarX + XL/2 - LO_CONNECTOR_WIDTH / 2, Pos.Y + Size.Y, LO_CONNECTOR_WIDTH, LO_CONNECTOR_LENGTH, 0.f, 0.f, 0.f, 0.f, ObjInfo.Variables(Idx).Color );
			}
			if(bHitTesting) Canvas->SetHitProxy( NULL );

			if ( AABBLiesWithinViewport( Canvas, VarX, LabelY, XL, YL ) )
			{
				if (bSplit)
				{
					DrawShadowedString( Canvas, VarX + XL/2 - RightXL/2, LabelY + YL/2, *RightSplit, GEngine->SmallFont, FLinearColor::White );
					DrawShadowedString( Canvas, VarX + XL/2 - LeftXL/2, LabelY, *LeftSplit, GEngine->SmallFont, FLinearColor::White );
				}
				else
				{
					DrawShadowedString( Canvas, VarX, LabelY, *ObjInfo.Variables(Idx).Name, GEngine->SmallFont, FLinearColor::White );
				}
			}
		}
		LastX = VarX;
		LastXL = XL;
	}
	// Draw event connectors.
	ObjInfo.EventX.Add( ObjInfo.Events.Num() );
	for (INT Idx = 0; Idx < ObjInfo.Events.Num(); Idx++)
	{
		INT VarX = LastX + LastXL + (LO_DESC_X_PADDING * 2);
		INT XL, YL;
		VarStringSize( GEngine->SmallFont, XL, YL, ObjInfo.Events(Idx).Name );
		ObjInfo.EventX(Idx)	= VarX + XL/2;

		if ( bSufficientlyZoomedIn )
		{
			if(bHitTesting) Canvas->SetHitProxy( new HLinkedObjConnectorProxy(ObjInfo.ObjObject, LOC_EVENT, Idx) );
			DrawTile( Canvas, VarX + XL/2 - LO_CONNECTOR_WIDTH / 2, Pos.Y + Size.Y, LO_CONNECTOR_WIDTH, LO_CONNECTOR_LENGTH, 0.f, 0.f, 0.f, 0.f, ObjInfo.Events(Idx).Color );
			if(bHitTesting) Canvas->SetHitProxy( NULL );

			if ( AABBLiesWithinViewport( Canvas, VarX, LabelY, XL, YL ) )
			{
				DrawShadowedString( Canvas, VarX, LabelY, *ObjInfo.Events(Idx).Name, GEngine->SmallFont, FLinearColor::White );
			}
		}
		LastX = VarX;
		LastXL = XL;
	}

}

void FLinkedObjDrawUtils::DrawLinkedObj(FCanvas* Canvas, FLinkedObjDrawInfo& ObjInfo, const TCHAR* Name, const TCHAR* Comment, const FColor& BorderColor, const FColor& TitleBkgColor, const FIntPoint& Pos)
{
	const UBOOL bHitTesting = Canvas->IsHitTesting();

	const FIntPoint TitleSize	= GetTitleBarSize(Canvas, Name);
	const FIntPoint LogicSize	= GetLogicConnectorsSize(Canvas, ObjInfo);
	const FIntPoint VarSize		= GetVariableConnectorsSize(Canvas, ObjInfo);

	ObjInfo.DrawWidth	= Max3(TitleSize.X, LogicSize.X, VarSize.X);
	ObjInfo.DrawHeight	= TitleSize.Y + LogicSize.Y + VarSize.Y + 3;

	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( new HLinkedObjProxy(ObjInfo.ObjObject) );

	DrawTitleBar(Canvas, Pos, FIntPoint(ObjInfo.DrawWidth, TitleSize.Y), BorderColor, TitleBkgColor, Name, Comment);

	DrawTile( Canvas, Pos.X,		Pos.Y + TitleSize.Y + 1,	ObjInfo.DrawWidth,		LogicSize.Y + VarSize.Y,		0.0f,0.0f,0.0f,0.0f, BorderColor );
	DrawTile( Canvas, Pos.X + 1,	Pos.Y + TitleSize.Y + 2,	ObjInfo.DrawWidth - 2,	LogicSize.Y + VarSize.Y - 2,	0.0f,0.0f,0.0f,0.0f, FColor(140,140,140) );

	DrawTile( Canvas, Pos.X,Pos.Y + TitleSize.Y + LogicSize.Y,ObjInfo.DrawWidth - 2,2,0.f,0.f,0.f,0.f,BorderColor);

	if(Canvas->IsHitTesting()) Canvas->SetHitProxy( NULL );

	DrawLogicConnectors(Canvas, ObjInfo, Pos + FIntPoint(0, TitleSize.Y + 1), FIntPoint(ObjInfo.DrawWidth, LogicSize.Y));
	DrawVariableConnectors(Canvas, ObjInfo, Pos + FIntPoint(0, TitleSize.Y + 1 + LogicSize.Y), FIntPoint(ObjInfo.DrawWidth, VarSize.Y), VarSize.X);
}

// if the rendering changes, these need to change
INT FLinkedObjDrawUtils::ComputeSliderHeight(INT SliderWidth)
{
	return LO_SLIDER_HANDLE_HEIGHT+4;
}

INT FLinkedObjDrawUtils::Compute2DSliderHeight(INT SliderWidth)
{
	return SliderWidth;
}

INT FLinkedObjDrawUtils::DrawSlider( FCanvas* Canvas, const FIntPoint& SliderPos, INT SliderWidth, const FColor& BorderColor, const FColor& BackGroundColor, FLOAT SliderPosition, const FString& ValText, UObject* Obj, int SliderIndex , UBOOL bDrawTextOnSide)
{
	const UBOOL bHitTesting = Canvas->IsHitTesting();
	const INT SliderBoxHeight = LO_SLIDER_HANDLE_HEIGHT + 4;

	if ( AABBLiesWithinViewport( Canvas, SliderPos.X, SliderPos.Y, SliderWidth, SliderBoxHeight ) )
	{
		const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
		const INT SliderRange = (SliderWidth - 4 - LO_SLIDER_HANDLE_WIDTH);
		const INT SliderHandlePosX = appTrunc(SliderPos.X + 2 + (SliderPosition * SliderRange));

		if(bHitTesting) Canvas->SetHitProxy( new HLinkedObjProxySpecial(Obj, SliderIndex) );
		DrawTile( Canvas, SliderPos.X,		SliderPos.Y - 1,	SliderWidth,		SliderBoxHeight,		0.0f,0.0f,0.0f,0.0f, BorderColor );
		DrawTile( Canvas, SliderPos.X + 1,	SliderPos.Y,		SliderWidth - 2,	SliderBoxHeight - 2,	0.0f,0.0f,0.0f,0.0f, BackGroundColor );

		if ( Zoom2D > SliderMinZoom )
		{
			DrawTile( Canvas, SliderHandlePosX, SliderPos.Y + 1, LO_SLIDER_HANDLE_WIDTH, LO_SLIDER_HANDLE_HEIGHT, 0.f, 0.f, 1.f, 1.f, SliderHandleColor );
		}
		if(bHitTesting) Canvas->SetHitProxy( NULL );
	}

	if(bDrawTextOnSide)
	{
		INT SizeX, SizeY;
		StringSize(GEngine->SmallFont, SizeX, SizeY, TEXT("%s"), *ValText);

		const INT PosX = SliderPos.X - 2 - SizeX; 
		const INT PosY = SliderPos.Y + (SliderBoxHeight + 1 - SizeY)/2;
		if ( AABBLiesWithinViewport( Canvas, PosX, PosY, SizeX, SizeY ) )
		{
			DrawString( Canvas, PosX, PosY, *ValText, GEngine->SmallFont, FColor(0,0,0) );
		}
	}
	else
	{
		DrawString( Canvas, SliderPos.X + 2, SliderPos.Y + SliderBoxHeight + 1, *ValText, GEngine->SmallFont, FColor(0,0,0) );
	}
	return SliderBoxHeight;
}

INT FLinkedObjDrawUtils::Draw2DSlider(FCanvas* Canvas, const FIntPoint &SliderPos, INT SliderWidth, const FColor& BorderColor, const FColor& BackGroundColor, FLOAT SliderPositionX, FLOAT SliderPositionY, const FString &ValText, UObject *Obj, int SliderIndex, UBOOL bDrawTextOnSide)
{
	const UBOOL bHitTesting = Canvas->IsHitTesting();

	const INT SliderBoxHeight = SliderWidth;

	if ( AABBLiesWithinViewport( Canvas, SliderPos.X, SliderPos.Y, SliderWidth, SliderBoxHeight ) )
	{
		const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
		const INT SliderRangeX = (SliderWidth - 4 - LO_SLIDER_HANDLE_HEIGHT);
		const INT SliderRangeY = (SliderBoxHeight - 4 - LO_SLIDER_HANDLE_HEIGHT);
		const INT SliderHandlePosX = SliderPos.X + 2 + appTrunc(SliderPositionX * SliderRangeX);
		const INT SliderHandlePosY = SliderPos.Y + 2 + appTrunc(SliderPositionY * SliderRangeY);

		if(bHitTesting) Canvas->SetHitProxy( new HLinkedObjProxySpecial(Obj, SliderIndex) );
		DrawTile( Canvas, SliderPos.X,		SliderPos.Y - 1,	SliderWidth,		SliderBoxHeight,		0.0f,0.0f,0.0f,0.0f, BorderColor );
		DrawTile( Canvas, SliderPos.X + 1,	SliderPos.Y,		SliderWidth - 2,	SliderBoxHeight - 2,	0.0f,0.0f,0.0f,0.0f, BackGroundColor );

		if ( Zoom2D > SliderMinZoom )
		{
			DrawTile( Canvas, SliderHandlePosX, SliderHandlePosY, LO_SLIDER_HANDLE_HEIGHT, LO_SLIDER_HANDLE_HEIGHT, 0.f, 0.f, 1.f, 1.f, SliderHandleColor );
		}
		if(bHitTesting) Canvas->SetHitProxy( NULL );
	}

	if(bDrawTextOnSide)
	{
		INT SizeX, SizeY;
		StringSize(GEngine->SmallFont, SizeX, SizeY, *ValText);
		const INT PosX = SliderPos.X - 2 - SizeX;
		const INT PosY = SliderPos.Y + (SliderBoxHeight + 1 - SizeY)/2;
		if ( AABBLiesWithinViewport( Canvas, PosX, PosY, SizeX, SizeY ) )
		{
			DrawString( Canvas, PosX, PosY, *ValText, GEngine->SmallFont, FColor(0,0,0));
		}
	}
	else
	{
		DrawString( Canvas, SliderPos.X + 2, SliderPos.Y + SliderBoxHeight + 1, *ValText, GEngine->SmallFont, FColor(0,0,0) );
	}

	return SliderBoxHeight;
}


/**
 * @return		TRUE if the current viewport contains some portion of the specified AABB.
 */
UBOOL FLinkedObjDrawUtils::AABBLiesWithinViewport(FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY)
{
	const FMatrix TransformMatrix = Canvas->GetTransform();
	const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
	FRenderTarget* RenderTarget = Canvas->GetRenderTarget();
	if ( !RenderTarget )
	{
		return FALSE;
	}

	// Transform the 2D point by the current transform matrix.
	FVector Point(X,Y, 0.0f);
	Point = TransformMatrix.TransformFVector(Point);
	X = Point.X;
	Y = Point.Y;

	// Check right side.
	if ( X > RenderTarget->GetSizeX() )
	{
		return FALSE;
	}

	// Check left side.
	if ( X+SizeX*Zoom2D < 0.f )
	{
		return FALSE;
	}

	// Check bottom side.
	if ( Y > RenderTarget->GetSizeY() )
	{
		return FALSE;
	}

	// Check top side.
	if ( Y+SizeY*Zoom2D < 0.f )
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * Convenience function for filtering calls to FRenderInterface::DrawTile via AABBLiesWithinViewport.
 */
void FLinkedObjDrawUtils::DrawTile(FCanvas* Canvas,FLOAT X,FLOAT Y,FLOAT SizeX,FLOAT SizeY,FLOAT U,FLOAT V,FLOAT SizeU,FLOAT SizeV,const FLinearColor& Color,FTexture* Texture,UBOOL AlphaBlend)
{
	if ( AABBLiesWithinViewport( Canvas, X, Y, SizeX, SizeY ) )
	{
		::DrawTile(Canvas,X,Y,SizeX,SizeY,U,V,SizeU,SizeV,Color,Texture,AlphaBlend);
	}
}

/**
 * Convenience function for filtering calls to FRenderInterface::DrawTile via AABBLiesWithinViewport.
 */
void FLinkedObjDrawUtils::DrawTile(FCanvas* Canvas,FLOAT X,FLOAT Y,FLOAT SizeX,FLOAT SizeY,FLOAT U,FLOAT V,FLOAT SizeU,FLOAT SizeV,FMaterialRenderProxy* MaterialRenderProxy)
{
	if ( AABBLiesWithinViewport( Canvas, X, Y, SizeX, SizeY ) )
	{
		::DrawTile(Canvas,X,Y,SizeX,SizeY,U,V,SizeU,SizeV,MaterialRenderProxy);
	}
}

/**
 * Convenience function for filtering calls to FRenderInterface::DrawString through culling heuristics.
 */
INT FLinkedObjDrawUtils::DrawString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,UFont* Font,const FLinearColor& Color)
{
	const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());

	if ( Zoom2D > TextMinZoom )
	{
		return ::DrawString(Canvas,StartX,StartY,Text,Font,Color);
	}
	else
	{
		return 0;
	}
}

/**
 * Convenience function for filtering calls to FRenderInterface::DrawShadowedString through culling heuristics.
 */
INT FLinkedObjDrawUtils::DrawShadowedString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,UFont* Font,const FLinearColor& Color)
{
	const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());

	if ( Zoom2D > TextMinZoom )
	{
		if ( Zoom2D < 1.f - DELTA )
		{
			return ::DrawString(Canvas,StartX,StartY,Text,Font,Color);
		}
		else
		{
			return ::DrawShadowedString(Canvas,StartX,StartY,Text,Font,Color);
		}
	}
	else
	{
		return 0;
	}
}

/**
* Takes a transformation matrix and returns a uniform scaling factor based on the 
* length of the rotation vectors.
*/
FLOAT FLinkedObjDrawUtils::GetUniformScaleFromMatrix(const FMatrix &Matrix)
{
	const FVector XAxis = Matrix.GetAxis(0);
	const FVector YAxis = Matrix.GetAxis(1);
	const FVector ZAxis = Matrix.GetAxis(2);

	FLOAT Scale = Max(XAxis.Size(), YAxis.Size());
	Scale = Max(Scale, ZAxis.Size());

	return Scale;
}

