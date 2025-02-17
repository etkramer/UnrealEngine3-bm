//------------------------------------------------------------------------------
// The coarticulation widget, a testbed for new coarticulation algorithms.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCoarticulationWidget_H__
#define FxCoarticulationWidget_H__

#include "stdwx.h"

#include "FxPlatform.h"
#include "FxWidget.h"
#include "FxTimeManager.h"
#include "FxCoarticulationConfig.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/grid.h"
#endif

namespace OC3Ent
{

namespace Face
{

struct FxPhonemeData
{
	FxPhoneme phoneme;
	FxReal suppression;
	FxAnimKey leadIn;
	FxAnimKey peak;
	FxAnimKey leadOut;
};

class FxAnim;
class FxPhonWordList;

class FxCoarticulationWidget : public wxWindow, public FxWidget
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS(FxCoarticulationWidget)
	DECLARE_EVENT_TABLE()

public:
	// Construction
	FxCoarticulationWidget( wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL );
	~FxCoarticulationWidget();

	// Widget messages
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	virtual void OnPhonemeMappingChanged( FxWidget* sender, FxPhonemeMap* phonemeMap );
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, FxAnim* anim );
	virtual void OnAnimPhonemeListChanged( FxWidget* sender, FxAnim* anim, 
		FxPhonWordList* phonemeList );
	virtual void OnRecalculateGrid( FxWidget* sender );
	virtual void OnGenericCoarticulationConfigChanged( FxWidget* sender, FxCoarticulationConfig* config );

protected:
	// Message handlers
	void OnSize( wxSizeEvent& event );
	void OnCustomCoarticulationCheck( wxCommandEvent& event );
	void OnCellChange( wxGridEvent& event );
	void OnConfigChange( wxCommandEvent& event );

private:
	// Functions
	// One-time initialization of the arrays.
	void SetDefaultParameters( void );
	// Creates the grid.
	void FillGrid( void );
	// Fills the name.
	void FillConfigName( void );

	// Controls
	wxCheckBox* _splitDiphthongsCheck;
	wxTextCtrl* _diphthongBoundaryCtrl;
	wxCheckBox* _convertToFlapsCheck;
	wxGrid*		_parameterGrid;
	wxTextCtrl* _suppressionWindowMaxCtrl;
	wxTextCtrl* _suppressionWindowMaxSlopeCtrl;
	wxTextCtrl* _suppressionWindowMinCtrl;
	wxTextCtrl* _suppressionWindowMinSlopeCtrl;
	wxTextCtrl* _shortSilenceDurationCtrl;
	wxTextCtrl* _leadInTimeCtrl;
	wxTextCtrl* _leadOutTimeCtrl;
	FxTimeGridFormat _timeFormat;
	wxTextCtrl* _configName;

	// Cache the animation and phoneme map for on-demand coarticulation.
	FxAnim* _pAnim;
	FxPhonWordList* _pPhonemeList;

	FxBool _doCoarticulation;

	// The configuration
	FxCoarticulationConfig _config;
};

} // namespace Face

} // namespace OC3Ent

#endif

