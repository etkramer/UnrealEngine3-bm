/*=============================================================================
	DlgUISoundCue.h : UISoundCue dialog class declarations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


#ifndef __DLGUISOUNDCUE_H__
#define __DLGUISOUNDCUE_H__

/** Readonly textbox that only responds to special keys used to clear current value of the textbox */
class WxSoundCueTextCtrl : public WxTextCtrl
{
public:
	WxSoundCueTextCtrl( wxWindow* parent,
		wxWindowID id,
		const wxString& value = TEXT(""),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0 )
	: WxTextCtrl(parent, id, value, pos, size, style)
	{}

private:
	/** On Character event that responds to backspace only. */
	void OnChar( wxKeyEvent& Event);

	DECLARE_EVENT_TABLE()
};

class WxDlgEditSoundCue : public wxDialog
{
	DECLARE_DYNAMIC_CLASS( WxDlgAddSoundCue )
	DECLARE_EVENT_TABLE();

public:
	/**
	 * Constructor
	 * Initializes the list of available UISoundCue names
	 */
    WxDlgEditSoundCue();

	/**
	 * Destructor
	 * Saves the window position and size to the ini
	 */

	~WxDlgEditSoundCue();

	/**
	 * Initialize this dialog.  Must be the first function called after creating the dialog.
	 *
	 * @param	ActiveSkin				the skin that is currently active
	 * @param	InParent				the window that opened this dialog
	 * @param	InID					the ID to use for this dialog
	 * @param	InitialSoundCueName		the name for the UISoundCue currently selected in the skin editor, if applicable
	 * @param	InitialSoundCueSound	the SoundCue object associated with the UISoundCue currently selected in the skin editor, if applicable
	 */
    void Create( class UUISkin* ActiveSkin, class WxDlgUISkinEditor* InParent, wxWindowID InID=ID_UI_EDITUISOUNDCUE_DLG, FName InitialSoundCueName=NAME_None, class USoundCue* InitialSoundCueSound=NULL );

	/**
	 * Returns the sound cue data that the user selected in the dialog.
	 */
	FUISoundCue GetSoundCueData() const;

protected:
	
    /**
     * Creates the controls and sizers for this dialog.
     */
    void CreateControls();

	/**
	 * Initializes the name combo with the available names as configured in the UIInteraction's UISoundCueNames array.
	 */
	void InitializeNameCombo();

	/** the UISkin editor dialog that owns this dialog; necessary since this dialog isn't modal */
	class WxDlgUISkinEditor* ParentSkinEditor;

	/** the skin that was active when this sound cue editor was invoked;used to remove the names that are already in use */
	UUISkin* CurrentlyActiveSkin;

	/** displays the list of UISoundCue names that are available */
	wxChoice* cmb_AvailableNames;

	/** displays the path name of the USoundCue currently assigned to this UISoundCue */
	wxTextCtrl*	txt_SoundCuePath;

	/** the button for assigning the USoundCue currently selected in the GB to the text control */
	wxBitmapButton* btn_UseSelected;

	/** brings up the generic browser */
	wxBitmapButton* btn_ShowBrowser;

	/** stores the sound cue data that was passed to this dialog when it was created */
	struct FUISoundCue InitialSoundCueData;

	/** contains the currently selected options in the sound cue editor */
	struct FUISoundCue CurrentSoundCueData;

private:
	void OnSelectSoundCueName( wxCommandEvent& Event );
	void OnSoundCuePathChanged( wxCommandEvent& Event );

	void OnClick_CloseDialog( wxCommandEvent& Event );
	void OnClick_ShowInBrowser( wxCommandEvent& Event );
	void OnClick_UseSelected( wxCommandEvent& Event );

	void OnClose(wxCloseEvent& Event);
};


#endif	//	__DLGUISOUNDCUE_H__






