/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __MATERIALEDITORTOOLBAR_H__
#define __MATERIALEDITORTOOLBAR_H__


/**
 * The base toolbar for material editor and material instance editor.
 */
class WxMaterialEditorToolBarBase : public wxToolBar
{
public:
	WxMaterialEditorToolBarBase(wxWindow* InParent, wxWindowID InID);

	void SetShowGrid(UBOOL bValue);
	void SetShowBackground(UBOOL bValue);
	void SetRealtimeMaterialPreview(UBOOL bValue);

protected:
	WxMaskedBitmap	BackgroundB;
	WxMaskedBitmap	CylinderB;
	WxMaskedBitmap	CubeB;
	WxMaskedBitmap	SphereB;
	WxMaskedBitmap	RealTimeB;
	WxMaskedBitmap	RealTimePreviewsB;
	WxMaskedBitmap	CleanB;
	WxMaskedBitmap	UseButtonB;
	WxMaskedBitmap	ToggleGridB;
	WxMaskedBitmap  ShowAllParametersB;
};

/**
 * The toolbar appearing along the top of the material editor.
 */
class WxMaterialEditorToolBar : public WxMaterialEditorToolBarBase
{
public:
	WxMaterialEditorToolBar(wxWindow* InParent, wxWindowID InID);

	void SetHideConnectors(UBOOL bValue);
	void SetDrawCurves(UBOOL bValue);
	void SetRealtimeExpressionPreview(UBOOL bValue);
	void SetAlwaysRefreshAllPreviews(UBOOL bValue);

protected:
	WxMaskedBitmap	HideConnectorsB;
	WxMaskedBitmap	ShowConnectorsB;
	WxMaskedBitmap	DrawCurvesB;
	WxMaskedBitmap	HomeB;
	WxMaskedBitmap	ApplyB;
	WxMaskedBitmap	ApplyDisabledB;
	WxMaskedBitmap	PropFallbackB;
	WxMaskedBitmap	PropFallbackDisabledB;
	WxMaskedBitmap	RegenAutoFallbackB;
	WxMaskedBitmap	RegenAutoFallbackDisabledB;
	WxMaskedBitmap	OpenFallbackB;
	WxMaskedBitmap  OpenFallbackDisabledB;
};

/**
 * The toolbar appearing along the top of the material instance editor.
 */
class WxMaterialInstanceConstantEditorToolBar : public WxMaterialEditorToolBarBase
{
public:
	WxMaterialInstanceConstantEditorToolBar(wxWindow* InParent, wxWindowID InID);
};

class WxMaterialInstanceTimeVaryingEditorToolBar : public WxMaterialEditorToolBarBase
{
public:
	WxMaterialInstanceTimeVaryingEditorToolBar(wxWindow* InParent, wxWindowID InID);
};


#endif // __MATERIALEDITORTOOLBAR_H__
