/*=============================================================================
	DlgNewGeneric.h: UnrealEd's new generic dialog.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include <wx/fontdlg.h>

class WxDlgNewGeneric :
	public wxDialog,
	public FSerializableObject
{
public:
	WxDlgNewGeneric();

	int ShowModal( const FString& InPackage, const FString& InGroup, UClass* DefaultFactoryClass=NULL );

	/////////////////////////
	// wxWindow interface.
	virtual bool Validate();

	/**
	 * Since we create a UFactory object, it needs to be serialized
	 *
	 * @param Ar The archive to serialize with
	 */
	virtual void Serialize(FArchive& Ar);

private:
	FString Package, Group, Name;
	UFactory* Factory;
	WxPropertyWindow* PropertyWindow;

	wxBoxSizer* PGNSizer;
	WxPkgGrpNameCtrl* PGNCtrl;
	wxComboBox *FactoryCombo;
	/**
	 * The child ids for the font controls
	 */
	enum EFontControlIDs
	{
		ID_CHOOSE_FONT,
		IDCB_FACTORY,
		ID_PKGGRPNAME,
		ID_PROPERTY_WINDOW
	};
	/**
	 * Cached sizer so that we can add the font controls to it if needed
	 */
	wxStaticBoxSizer* FactorySizer;
	/**
	 * This sizer contains the font specific controls that are adding when
	 * creating a new font
	 */
	wxBoxSizer* FontSizer;
	/**
	 * The button that was added
	 */
	wxButton* ChooseFontButton;
	/**
	 * The static text that will show the font
	 */
	wxStaticText* FontStaticText;

	/**
	 * Adds the specific font controls to the dialog
	 */
	void AppendFontControls(void);

	/**
	 * Removes the font controls from the dialog
	 */
	void RemoveFontControls(void);

	/**
	 * Lets the user select the font that is going to be imported
	 */
	void OnChooseFont(wxCommandEvent& In);

	void OnFactorySelChange( wxCommandEvent& In );

	DECLARE_EVENT_TABLE()

public:
	FString GetPackage()	{	return Package;	}
	FString GetGroup()		{	return Group;	}
	FString GetName()		{	return Name;	}
};
