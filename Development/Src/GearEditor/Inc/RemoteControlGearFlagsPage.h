//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================

#ifndef _REMOTE_CONTROL_WARFARE_FLAGS_PAGE_H_
#define _REMOTE_CONTROL_WARFARE_FLAGS_PAGE_H_

#include "RemoteControlPage.h"

// Forward declarations.
class wxNotebook;
class wxChoice;
class wxTextCtrl;
class wxCheckBox;
class wxCommandEvent;

/**
* Standard RemoteControl render page
*/
class WxRemoteControlGearFlagsPage : public WxRemoteControlPage
{
	/**
	 * DebugAI
	 **/
	wxCheckBox* ShowDebugAICheckBox;
	void DebugAI_Activated( wxCommandEvent& In );
	void DebugAI_UpdateWidget();
	UBOOL DebugAI_Worker( UBOOL bShouldSet, UBOOL bValueToSet );


	/**
	 * ShowAccuracy
	 **/
	wxCheckBox* ShowAccuracyCheckBox;
	void ShowAccuracy_Activated( wxCommandEvent& In );
	void ShowAccuracy_UpdateWidget();
	UBOOL ShowAccuracy_Worker( UBOOL bShouldSet, UBOOL bValueToSet );


	/**
	 * ShowCover methods
	 **/
	wxCheckBox* ShowCoverCheckBox;
	void ShowCover_Activated( wxCommandEvent& In );
	void ShowCover_UpdateWidget();
	UBOOL ShowCover_Worker( UBOOL bShouldSet, UBOOL bValueToSet );




public:
	WxRemoteControlGearFlagsPage( FRemoteControlGame* Game, wxNotebook* Notebook );

	/**
	* Return's the page's title, displayed on the notebook tab.
	**/
	virtual const TCHAR* GetPageTitle() const;

	/**
	* Refreshes page contents.
	**/
	virtual void RefreshPage(UBOOL bForce = FALSE);

};

#endif // _REMOTE_CONTROL_WARFARE_FLAGS_PAGE_H_

