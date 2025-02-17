/*=============================================================================

Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGGENERATEUVOPTIONS_H__
#define __DLGGENERATEUVOPTIONS_H__


class WxDlgGenerateUVOptions : public wxDialog
{
public:
	WxDlgGenerateUVOptions(
		wxWindow* Parent, 
		const TArray<FString>& LODComboOptions, 
		const TArray<INT>& InLODNumTexcoords,
		FLOAT MaxDesiredStretchDefault);

	virtual ~WxDlgGenerateUVOptions();

	int ShowModal();

	UINT ChosenLODIndex;
	UINT ChosenTexIndex;
	FLOAT MaxStretch;
	UBOOL bKeepExistingUVs;

private:
	TArray<INT> LODNumTexcoords;

	void OnOk( wxCommandEvent& In );
	void OnCancel( wxCommandEvent& In );
	void OnChangeLODCombo( wxCommandEvent& In );

	wxButton*		OkButton;
	wxButton*		CancelButton;
	wxComboBox*		LODComboBox;
	wxComboBox*		TexIndexComboBox;
	wxTextCtrl*		MaxStretchEntry;
	wxCheckBox*		KeepExistingUVCheckBox;

	DECLARE_EVENT_TABLE()
};

#endif // __DLGGENERATEUVOPTIONS_H__
