//------------------------------------------------------------------------------
// Dialog used for creating viewport cameras.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCreateCameraDialog_H__
#define FxCreateCameraDialog_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxActor.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/listctrl.h"
#endif

namespace OC3Ent
{

namespace Face
{

#ifndef wxCLOSE_BOX
	#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
	#define wxFIXED_MINSIZE 0
#endif

// A viewport camera creation dialog.
class FxCreateCameraDialog : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxCreateCameraDialog)
	DECLARE_EVENT_TABLE()
	typedef wxDialog Super;

public:
	// Constructors.
	FxCreateCameraDialog();

	// Creation.
	FxBool Create( wxWindow* parent, wxWindowID id = wxID_ANY, 
		const wxString& caption = _("Create Viewport Camera"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxDefaultSize, 
		long style = wxDEFAULT_DIALOG_STYLE );

	// Creates the controls and sizers.
	void CreateControls( void );

	// Initializes the dialog.  This should be called after the default
	// constructor but before Create().
	void Initialize( FxActor* pActor, FxString* cameraName, FxBool* usesFixedAspectRatio, 
		FxBool* isBoundToBone, FxString* boneName );

	// Should we show tooltips?
	static FxBool ShowToolTips( void );
	
protected:
	// The camera name edit box.
	wxTextCtrl* _cameraNameEditBox;
	// The "use fixed aspect ratio" check box.
	wxCheckBox* _usesFixedAspectRatioCheckBox;
	// The "bind" to bone check box.
	wxCheckBox* _bindToBoneCheckBox;
	// The bone list box.
	wxListCtrl* _boneListBox;
	// The create button.
	wxButton* _createButton;
	// The cancel button.
	wxButton* _cancelButton;
	// The name of the camera.
	FxString* _cameraName;
	// FxTrue if the user selected a fixed aspect ratio.
	FxBool* _usesFixedAspectRatio;
	// FxTrue if the user selected to "bind" the camera to a bone.
	FxBool* _isBoundToBone;
	// The name of the bone the camera is "bound" to, if any.
	FxString* _boneName;
	// The actor.
	FxActor* _actor;

	void OnBindToBoneChecked( wxCommandEvent& event );
	void OnColumnClick( wxListEvent& event );
	void OnCreateButtonPressed( wxCommandEvent& event );
	void OnCancelButtonPressed( wxCommandEvent& event );
};

} // namespace Face

} // namespace OC3Ent

#endif
