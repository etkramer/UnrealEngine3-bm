/*=============================================================================
DlgTipOfTheDay.h: Tip of the day dialog
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGTIPOFTHEDAY_H__
#define __DLGTIPOFTHEDAY_H__

/** Provides tips to the tip of the day dialog. */
class WxLocalizedTipProvider
{
public:
	WxLocalizedTipProvider();
	wxString GetTip(INT &TipNumber);
private:
	class FConfigSection* TipSection;

	/** Cached filename to use to load the tips from */
	FString TipFilename;
};


/** Tip of the day dialog, displays a random tip at startup. */
class WxDlgTipOfTheDay : public wxDialog
{
public:
	WxDlgTipOfTheDay(wxWindow* Parent);
	virtual ~WxDlgTipOfTheDay();

	/**
	* @return Whether or not the application should display this dialog at startup.
	*/
	UBOOL GetShowAtStartup() const
	{
#if BATMAN
		// This is a bit annoying, so disable it.
		return FALSE;
#endif // BATMAN

		return CheckStartupShow->GetValue();
	}

private:

	/** Saves options for the tip box. */
	void SaveOptions() const;

	/** Loads options for the tip box. */
	void LoadOptions();

	/** Updates the currently displayed tip with a new randomized tip. */
	void UpdateTip();

	/** Updates the tip text with a new tip. */
	void OnNextTip(wxCommandEvent &Event);

	/** wxWidgets tip provider interface to get tips for this dialog. */
	WxLocalizedTipProvider TipProvider;

	/** Button to generate a new tip. */
	wxButton* NextTip;

	/** Text control to display tips in. */
	wxTextCtrl* TipText;

	/** Checkbox to determine whether or not we show at startup. */
	wxCheckBox* CheckStartupShow;

	/** Index of the current tip. */
	INT CurrentTip;

	DECLARE_EVENT_TABLE()
};


#endif

