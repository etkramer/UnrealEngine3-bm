/*=============================================================================
	DlgUIWidgetEvents.h:	Dialog that lets the user enable/disable UI event key aliases for a paticular widget.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DLGUIWIDGETEVENTS_H__
#define __DLGUIWIDGETEVENTS_H__

class WxDlgUIWidgetEvents : public wxDialog
{
public:
	WxDlgUIWidgetEvents(wxWindow* InParent, UUIScreenObject* InWidget);
	~WxDlgUIWidgetEvents();

private:

	/** Generates the list of all of the event aliases for the widget. */
	void GenerateEventList();

	/** Callback for when the user clicks the OK button. */
	void OnOK( wxCommandEvent& Event );

	/** Shows the bind default event keys dialog. */
	void OnModifyDefaults( wxCommandEvent &Event );

	/** List of all of the events that this widget supports. */
	wxCheckListBox* EventList;

	/** Widget that we are modifying. */
	UUIScreenObject* Widget;

	DECLARE_EVENT_TABLE()
};

#endif



