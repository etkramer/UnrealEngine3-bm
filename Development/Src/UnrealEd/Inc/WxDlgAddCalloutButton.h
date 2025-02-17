/*=============================================================================
    WxDlgAddCalloutButton.h: 
    Copyright 2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#ifndef _WXDLGADDCALLOUTBUTTON_H_
#define _WXDLGADDCALLOUTBUTTON_H_

/**
 * Includes
 */

/**
 * Forward declarations
 */


/*!
 * WxDlgAddCalloutButton class declaration
 */

class WxDlgAddCalloutButton: public wxDialog
{    
	DECLARE_DYNAMIC_CLASS(WxDlgAddCalloutButton);
	DECLARE_EVENT_TABLE()

public:
	/** Constructors */
	WxDlgAddCalloutButton();
	WxDlgAddCalloutButton( UUICalloutButtonPanel* InOwnerPanel, wxWindow* parent, wxWindowID id=ID_UI_ADDCALLOUT_DLG );
	WxDlgAddCalloutButton( UUICalloutButton* InButton, wxWindow* parent, wxWindowID id=ID_UI_ADDCALLOUT_DLG );

	/** Destructor */
	~WxDlgAddCalloutButton();

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InOwnerPanel			the button panel that will contain the new button
	 * @param	InParent				the window that opened this dialog
	 * @param	InID					the ID to use for this dialog
	 */
	void Create( UUICalloutButtonPanel* InOwnerPanel, wxWindow* parent, wxWindowID id=ID_UI_ADDCALLOUT_DLG );

	/**
	 * Initialize this control when using two-stage dynamic window creation.  Must be the first function called after creation.
	 *
	 * @param	InButton				when this dialog is invoked to modify an existing button, the button that was selected 
	 * @param	InParent				the window that opened this dialog
	 * @param	InID					the ID to use for this dialog
	 */
	void Create( UUICalloutButton* InButton, wxWindow* parent, wxWindowID id=ID_UI_ADDCALLOUT_DLG );

	/**
	 * Initializes member variables
	 */
	void Init();

	/**
	 * @return	the input alias chosen for the new button
	 */
	FName GetSelectedAlias() const;

	/**
	 * @return	the name of the class chosen for the new button
	 */
	FString GetSelectedClassName() const;

	/**
	 * @return	the tag to use for the new button.
	 */
	FName GetButtonTag() const;

protected:
	/**
     * Creates the controls for this window
     */
	void CreateControls();

	/**
	 * Populates the combo controls with the available selections for each type.
	 */
	void InitializeControlValues();

public:
	/** Called when the user clicks the OK button - verifies that the names entered are unique */
	virtual bool Validate();

	/** the panel where this button will be added */
	class UUICalloutButtonPanel* OwnerPanel;

	/** when displaying the dialog for an existing button (such as when changing its alias), the button we're modifying */
	class UUICalloutButton* SelectedButton;

	/** local storage for the widget tag - necessary for wxTextValidator */
	wxString			DesiredWidgetTag;

	wxChoice*			cmb_InputAlias;
	wxChoice*			cmb_ButtonClass;
	WxTextCtrl*			txt_WidgetTag;

	/** the OK button */
	wxButton*			OKButton;

private:
	void OnTextEntered( wxCommandEvent& Event );
};

#endif
    // _WXDLGADDCALLOUTBUTTON_H_
