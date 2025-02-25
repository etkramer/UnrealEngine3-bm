/*=============================================================================
	UnStatChart.h: Stats Graphing Utility
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_STATCHART
#define _INC_STATCHART

class FStatChartLine
{
public:
	UBOOL							bHideLine;

	TArray<FLOAT>					DataHistory;
	INT								DataPos;

	FColor							LineColor;
	FString							LineName;
	FLOAT							YRange[2]; // [Min,Max]
	FLOAT							XSpacing;
	UBOOL							bAutoScale;

	FStatChartLine() {}

	UBOOL operator==( const FStatChartLine& Other ) const
	{
		return this == &Other;
	}
};

class FStatChart
{
public:
	UBOOL							bHideChart;
	UBOOL							bLockScale;

	TMap<FString,INT>				LineNameMap;
	TArray<FStatChartLine>			Lines;

	// Screen [x.y] size of chart
	FLOAT							ChartSize[2];

	// Screen [x,y] origin (bottom left) of chart
	FLOAT							ChartOrigin[2];

	INT								XRange;
	UBOOL							bHideKey;
	FLOAT							ZeroYRatio;
	BYTE							BackgroundAlpha;
	FString							FilterString;

	/* *** Interface *** */
	FStatChart();
	~FStatChart();

	void AddDataPoint(const FString& LineName, FLOAT Data);

	void AddLineAutoRange(const FString& LineName, FColor Color);
	void AddLine(const FString& LineName, FColor Color, FLOAT YRangeMin, FLOAT YRangeMax);
	void DestroyLine(const FString& LineName);

	void Reset(); 
	UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar);
	void Render(FViewport* Viewport, FCanvas* Canvas);
};

#endif
