//------------------------------------------------------------------------------
// The gesture widget, a testbed for tuning new speech gesture algorithms.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxGestureWidget_H__
#define FxGestureWidget_H__

#include "stdwx.h"

#include "FxPlatform.h"
#include "FxWidget.h"
#include "FxAnim.h"
#include "FxPhonWordList.h"
#include "FxSpeechGestures.h"
#include "FxTimeManager.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/grid.h"
#endif

namespace OC3Ent
{

namespace Face
{

class FxAudio;

class FxGestureWidget : public wxScrolledWindow, public FxWidget
{
	typedef wxScrolledWindow Super;
	WX_DECLARE_DYNAMIC_CLASS(FxGestureWidget)
	DECLARE_EVENT_TABLE()

public:
	// Construction
	FxGestureWidget( wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL );
	~FxGestureWidget();

	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, FxAnim* anim );
	virtual void OnAnimPhonemeListChanged( FxWidget* sender, FxAnim* anim, FxPhonWordList* phonemeList );
	virtual void OnCreateAnim( FxWidget* sender, const FxName& animGroupName, FxAnim* anim );
	virtual void OnRecalculateGrid(	FxWidget* sender );
	virtual void OnGestureConfigChanged( FxWidget* sender, FxGestureConfig* config );
	
protected:
	void OnRegenerateGestures( wxCommandEvent& event );
	void OnReinitializeGestures( wxCommandEvent& event );
	void OnEmphasisActionSelChange( wxCommandEvent& event );
	void OnEmphasisActionUpdateButton( wxCommandEvent& event );
	void OnEmphasisActionTrackGridChange( wxGridEvent& event );
	void OnSerializeConfig( wxCommandEvent& event );
	void OnResetDefaultConfig( wxCommandEvent& event );

	wxString GetName( FxEmphasisAction emphasisAction );
	wxString GetName( FxGestureTrack gestureTrack );
	wxString GetName( FxStressCategory stressCategory );

	void CreateControls();
	void FillEAGrids();
	void FillTrackGrid();
	void FillProbScaleGrid();
	void FillODGrid();
	void FillConfigName();

	void ReadEAGrids();
	void ReadTrackGrid();
	void ReadProbScaleGrid();
	void ReadODGrid();
	void ReadConfigName();

	FxName  _animGroupName;
	FxAnim* _pAnim;
	FxAudio* _pAudio;
	FxPhonWordList* _pPhonList;
	FxSpeechGestures _speechGestures;

	FxGestureConfig _config;

	wxChoice* _eaAnimSel;
	wxGrid* _eaTrackGrid;
	wxGrid* _eaProbScaleGrid;
	wxButton* _eaUpdateButton;
	wxGrid* _odPropGrid;
	wxTextCtrl* _configName;

	FxTimeGridFormat _timeFormat;

	FxInt32 _randomSeed;
};

} // namespace Face

} // namespace OC3Ent

#endif
