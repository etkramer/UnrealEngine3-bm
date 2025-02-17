/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNLINKEDOBJDRAWUTILS_H__
#define __UNLINKEDOBJDRAWUTILS_H__

#define LO_CAPTION_HEIGHT		(22)
#define LO_CONNECTOR_WIDTH		(8)
#define LO_CONNECTOR_LENGTH		(10)
#define LO_DESC_X_PADDING		(8)
#define LO_DESC_Y_PADDING		(8)
#define LO_TEXT_BORDER			(3)
#define LO_MIN_SHAPE_SIZE		(64)

#define LO_SLIDER_HANDLE_WIDTH	(7)
#define LO_SLIDER_HANDLE_HEIGHT	(15)

//
//	HLinkedObjProxy
//
struct HLinkedObjProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HLinkedObjProxy,HHitProxy);

	UObject*	Obj;

	HLinkedObjProxy(UObject* InObj):
		HHitProxy(HPP_UI),
		Obj(InObj)
	{}
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Obj;
	}
};

//
//	HLinkedObjProxySpecial
//
struct HLinkedObjProxySpecial : public HHitProxy
{
	DECLARE_HIT_PROXY(HLinkedObjProxySpecial,HHitProxy);

	UObject*	Obj;
	INT			SpecialIndex;

	HLinkedObjProxySpecial(UObject* InObj, INT InSpecialIndex):
		HHitProxy(HPP_UI),
		Obj(InObj),
		SpecialIndex(InSpecialIndex)
	{}
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Obj;
	}
};

/** Determines the type of connector a HLinkedObjConnectorProxy represents */
enum EConnectorHitProxyType
{
	/** an input connector */
	LOC_INPUT,

	/** output connector */
	LOC_OUTPUT,

	/** variable connector */
	LOC_VARIABLE,

	/** event connector */
	LOC_EVENT,
};

/**
 * In a linked object drawing, represents a connector for an object's link
 */
struct FLinkedObjectConnector
{
	/** the object that this proxy's connector belongs to */
	UObject* 				ConnObj;

	/** the type of connector this proxy is attached to */
	EConnectorHitProxyType	ConnType;

	/** the link index for this proxy's connector */
	INT						ConnIndex;

	/** Constructor */
	FLinkedObjectConnector(UObject* InObj, EConnectorHitProxyType InConnType, INT InConnIndex):
		ConnObj(InObj), ConnType(InConnType), ConnIndex(InConnIndex)
	{}

	void Serialize( FArchive& Ar )
	{
		Ar << ConnObj;
	}
};

/**
 * Hit proxy for link connectors in a linked object drawing.
 */
struct HLinkedObjConnectorProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HLinkedObjConnectorProxy,HHitProxy);

	FLinkedObjectConnector Connector;

	HLinkedObjConnectorProxy(UObject* InObj, EConnectorHitProxyType InConnType, INT InConnIndex):
		HHitProxy(HPP_UI),
		Connector(InObj, InConnType, InConnIndex)
	{}

	virtual void Serialize(FArchive& Ar)
	{
		Connector.Serialize(Ar);
	}
};

/**
 * Hit proxy for a line connection between two objects.
 */
struct HLinkedObjLineProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HLinkedObjProxySpecial,HHitProxy);

	FLinkedObjectConnector Src, Dest;

	HLinkedObjLineProxy(UObject *SrcObj, INT SrcIdx, UObject *DestObj, INT DestIdx) :
		HHitProxy(HPP_UI),
		Src(SrcObj,LOC_OUTPUT,SrcIdx),
		Dest(DestObj,LOC_INPUT,DestIdx)
	{}

	virtual void Serialize(FArchive& Ar)
	{
		Src.Serialize(Ar);
		Dest.Serialize(Ar);
	}
};

struct FLinkedObjConnInfo
{
	FString			Name;
	FColor			Color;
	UBOOL			bOutput;

	FLinkedObjConnInfo(const TCHAR* InName, const FColor& InColor, const UBOOL InbOutput = 0)
	{
		Name = InName;
		Color = InColor;
		bOutput = InbOutput;
	}
};

struct FLinkedObjDrawInfo
{
	TArray<FLinkedObjConnInfo>	Inputs;
	TArray<FLinkedObjConnInfo>	Outputs;
	TArray<FLinkedObjConnInfo>	Variables;
	TArray<FLinkedObjConnInfo>	Events;

	UObject*		ObjObject;

	// Outputs - so you can remember where connectors are for drawing links
	TArray<INT>		InputY;
	TArray<INT>		OutputY;
	TArray<INT>		VariableX;
	TArray<INT>		EventX;
	INT				DrawWidth;
	INT				DrawHeight;

	FLinkedObjDrawInfo()
	{
		ObjObject = NULL;
	}
};

/**
 * A collection of static drawing functions commonly used by linked object editors.
 */
class FLinkedObjDrawUtils
{
public:
	static void DrawNGon(FCanvas* Canvas, const FVector2D& Center, const FColor& Color, INT NumSides, FLOAT Radius);
	static void DrawSpline(FCanvas* Canvas, const FIntPoint& Start, const FVector2D& StartDir, const FIntPoint& End, const FVector2D& EndDir, const FColor& LineColor, UBOOL bArrowhead, UBOOL bInterpolateArrowPositon=FALSE);
	static void DrawArrowhead(FCanvas* Canvas, const FIntPoint& Pos, const FVector2D& Dir, const FColor& Color);

	static FIntPoint GetTitleBarSize(FCanvas* Canvas, const TCHAR* Name);
	static void DrawTitleBar(FCanvas* Canvas, const FIntPoint& Pos, const FIntPoint& Size, const FColor& BorderColor, const FColor& BkgColor, const TCHAR* Name, const TCHAR* Comment=NULL, const TCHAR* Comment2=NULL);

	static FIntPoint GetLogicConnectorsSize(FCanvas* Canvas, const FLinkedObjDrawInfo& ObjInfo, INT* InputY=NULL, INT* OutputY=NULL);
	static void DrawLogicConnectors(FCanvas* Canvas, FLinkedObjDrawInfo& ObjInfo, const FIntPoint& Pos, const FIntPoint& Size, const FLinearColor* ConnectorTileBackgroundColor=NULL);

	static FIntPoint GetVariableConnectorsSize(FCanvas* Canvas, const FLinkedObjDrawInfo& ObjInfo);
	static void DrawVariableConnectors(FCanvas* Canvas, FLinkedObjDrawInfo& ObjInfo, const FIntPoint& Pos, const FIntPoint& Size, const INT VarWidth);

	static void DrawLinkedObj(FCanvas* Canvas, FLinkedObjDrawInfo& ObjInfo, const TCHAR* Name, const TCHAR* Comment, const FColor& BorderColor, const FColor& TitleBkgColor, const FIntPoint& Pos);

	static INT ComputeSliderHeight(INT SliderWidth);
	static INT Compute2DSliderHeight(INT SliderWidth);
	// returns height of drawn slider
	static INT DrawSlider( FCanvas* Canvas, const FIntPoint& SliderPos, INT SliderWidth, const FColor& BorderColor, const FColor& BackGroundColor, FLOAT SliderPosition, const FString& ValText, UObject* Obj, int SliderIndex=0, UBOOL bDrawTextOnSide=FALSE);
	// returns height of drawn slider
	static INT Draw2DSlider(FCanvas* Canvas, const FIntPoint &SliderPos, INT SliderWidth, const FColor& BorderColor, const FColor& BackGroundColor, FLOAT SliderPositionX, FLOAT SliderPositionY, const FString &ValText, UObject *Obj, int SliderIndex, UBOOL bDrawTextOnSide);

	/**
	 * @return		TRUE if the current viewport contains some portion of the specified AABB.
	 */
	static UBOOL AABBLiesWithinViewport(FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY);

	/**
	 * Convenience function for filtering calls to FRenderInterface::DrawTile via AABBLiesWithinViewport.
	 */
	static void DrawTile(FCanvas* Canvas,FLOAT X,FLOAT Y,FLOAT SizeX,FLOAT SizeY,FLOAT U,FLOAT V,FLOAT SizeU,FLOAT SizeV,const FLinearColor& Color,FTexture* Texture = NULL,UBOOL AlphaBlend = 1);

	/**
	 * Convenience function for filtering calls to FRenderInterface::DrawTile via AABBLiesWithinViewport.
	 */
	static void DrawTile(FCanvas* Canvas, FLOAT X,FLOAT Y,FLOAT SizeX,FLOAT SizeY,FLOAT U,FLOAT V,FLOAT SizeU,FLOAT SizeV,FMaterialRenderProxy* MaterialRenderProxy);

	/**
	 * Convenience function for filtering calls to FRenderInterface::DrawString through culling heuristics.
	 */
	static INT DrawString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color);

	/**
	 * Convenience function for filtering calls to FRenderInterface::DrawShadowedString through culling heuristics.
	 */
	static INT DrawShadowedString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color);

	/**
	 * Takes a transformation matrix and returns a uniform scaling factor based on the 
	 * length of the rotation vectors.
	 *
	 * @param Matrix	A matrix to use to calculate a uniform scaling parameter.
	 *
	 * @return A uniform scaling factor based on the matrix passed in.
	 */
	static FLOAT GetUniformScaleFromMatrix(const FMatrix &Matrix);
};

#endif // __UNLINKEDOBJDRAWUTILS_H__
