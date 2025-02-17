//------------------------------------------------------------------------------
// A spreadsheet-like view of the character's mapping.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMappingWidget_H__
#define FxMappingWidget_H__

#include "FxPlatform.h"
#include "FxWidget.h"
#include "FxPhonemeMap.h"
#include "FxFaceGraph.h"

#include "FxButton.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/grid.h"
#endif

namespace OC3Ent
{

namespace Face
{

class FxMappingWidget : public wxGrid, public FxWidget
{
	typedef wxGrid Super;
	WX_DECLARE_DYNAMIC_CLASS(FxMappingWidget)
	DECLARE_EVENT_TABLE();

public:
	// Constructor.
	FxMappingWidget( wxWindow* parent = NULL, 
		FxWidgetMediator* mediator = NULL, wxWindowID id = -1,  
		wxPoint position = wxDefaultPosition, wxSize size = wxDefaultSize, 
		FxInt32 style = wxWANTS_CHARS );

	// Destructor.
	~FxMappingWidget();

	// FxWidget message handlers.
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	virtual void OnPhonemeMappingChanged( FxWidget* sender, FxPhonemeMap* phonemeMap );
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, FxAnim* anim );
	virtual void OnAnimPhonemeListChanged( FxWidget* sender, FxAnim* anim, FxPhonWordList* phonemeList );
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );

protected:
	// Called when the user right-clicks on a column label.
	void OnLabelRightClick( wxGridEvent& event );
	// Called when the user selects to remove a target from the popup menu.
	void OnRemoveTarget( wxCommandEvent& event );
	// Called when the user selects to make the default mapping from the popup.
	void OnMakeDefault( wxCommandEvent& event );
	// Called when the user selects to add a target from the popup menu.
	void OnAddTarget( wxCommandEvent& event );
	// Called when the user selects to sync the current anim to the mapping.
	void OnSyncAnim( wxCommandEvent& event );
	// Called to clear the mapping.
	void OnClearMapping( wxCommandEvent& event);

	// Called when the user changes a cell in the table.
	void OnCellChange( wxGridEvent& event );

	void OnHelp( wxHelpEvent& event );

	// Rebuilds the grid
	void RebuildGrid();

	FxPhonemeMap*   _phonemeMap;
	FxFaceGraph*    _faceGraph;
	FxName			_animGroup;
	FxAnim*			_anim;
	FxPhonWordList* _phonList;

	FxName			_lastTarget;
	FxBool			_ignoreMappingChange;
};

class FxMappingWidgetContainer : public wxWindow
{
	typedef wxWindow Super;
	DECLARE_EVENT_TABLE();

public:
	FxMappingWidgetContainer(wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL);
	~FxMappingWidgetContainer();

	void UpdateControlStates(FxBool state);

protected:
	void OnPaint(wxPaintEvent& event);
	void DrawToolbar(wxDC* pDC);
	void RefreshChildren();

	void OnSize(wxSizeEvent& event);
	void DoResize();

	void CreateToolbarControls();

	void OnMakeDefaultMappingButton(wxCommandEvent& event);
	void OnClearMappingButton(wxCommandEvent& event);
	void OnAddTargetButton(wxCommandEvent& event);
	void OnSyncAnimButton(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);

	void LoadIcons();

	FxMappingWidget*	_mappingWidget;
	wxRect				_toolbarRect;
	wxRect				_widgetRect;

	FxButton*			_toolbarMakeDefaultMappingButton;
	FxButton*			_toolbarClearMappingButton;
	FxButton*			_toolbarAddTargetButton;
	FxButton*			_toolbarSyncAnimButton;
};

} // namespace Face

} // namespace OC3Ent

#endif
