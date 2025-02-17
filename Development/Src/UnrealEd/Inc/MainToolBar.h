/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __MAINTOOLBAR_H__
#define __MAINTOOLBAR_H__

class wxComboBox;
class wxWindow;
class WxAlphaBitmap;


/**
 * WxMatineeMenuListToolBarButton
 */
class WxMatineeMenuListToolBarButton
	: public WxBitmapButton
{

public:

private:

	/** Called when the tool bar button is clicked */
	void OnClick( wxCommandEvent &In );

	DECLARE_EVENT_TABLE()
};



/**
 * The toolbar that sits at the top of the main editor frame.
 */
class WxMainToolBar : public wxToolBar
{
public:
	WxMainToolBar(wxWindow* InParent, wxWindowID InID);

	enum EPlatformButtons { B_PC=0, B_PS3, B_XBox360, B_MAX };

	/** Updates the 'Push View' toggle's bitmap and hint text based on the input state. */
	void SetPushViewState(UBOOL bIsPushingView);

	/** Enables/disables the 'Push View' button. */
	void EnablePushView(UBOOL bEnabled);

private:
	WxMaskedBitmap NewB, OpenB, SaveB, SaveAllB, UndoB, RedoB, CutB, CopyB, PasteB, SearchB, FullScreenB, GenericB, KismetB, TranslateB,
		ShowWidgetB, RotateB, ScaleB, ScaleNonUniformB, MouseLockB, BrushPolysB, PrefabLockB, CamSlowB, CamNormalB, CamFastB, ViewPushStartB, ViewPushStopB, ViewPushSyncB,
		DistributionToggleB, SocketsB, PublishCookB, PublishCopyB, MatineeListB, SentinelB;
	wxToolBarToolBase* ViewPushStartStopButton;
	WxAlphaBitmap *BuildGeomB, *BuildLightingB, *BuildPathsB, *BuildCoverNodesB, *BuildAllB, *PlayOnB[B_MAX], *PlayInEditorB;
	WxMenuButton MRUButton, PasteSpecialButton;
	wxMenu PasteSpecialMenu;

	/** Drop down of all available Matinee sequences in the level */
	WxMatineeMenuListToolBarButton MatineeListButton;

	DECLARE_EVENT_TABLE();

	/** Called when the trans/rot/scale widget toolbar buttons are right-clicked. */
	void OnTransformButtonRightClick(wxCommandEvent& In);

	/** Called when the Matinee list tool bar button is clicked */
	void OnMatineeListMenu( wxCommandEvent& In );

public:
	wxComboBox* CoordSystemCombo;
};

#endif // __MAINTOOLBAR_H__
