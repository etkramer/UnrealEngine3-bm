//------------------------------------------------------------------------------
// A dialog for modifying bone weights.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxBoneWeightDialog_H__
#define FxBoneWeightDialog_H__

#include "FxArray.h"
#include "FxAnim.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/grid.h"
#endif

namespace OC3Ent
{

namespace Face
{

class FxBoneWeightDialog : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxBoneWeightDialog);
	DECLARE_EVENT_TABLE();
public:
	FxBoneWeightDialog( wxWindow* parent = NULL, wxString title = _("Select Bone Weights") );
	void SetInitialBoneWeights( const FxArray<FxAnimBoneWeight>& boneWeights );
	FxArray<FxAnimBoneWeight> GetFinalBoneWeights();

protected:
	void OnNavKey( wxNavigationKeyEvent& event );
	void OnOK( wxCommandEvent& event );

	wxGrid* _weightGrid;
};

} // namespace Face

} // namespace OC3Ent

#endif
