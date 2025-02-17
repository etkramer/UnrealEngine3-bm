/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Log.h: Logging windows for displaying engine log info
 */

#ifndef __LOG_H__
#define __LOG_H__

/**
 * This version of the container cell only keeps a specified number of cells
 * in it's list.
 */
class WxBoundedHtmlContainerCell : public wxHtmlContainerCell
{
	/**
	 * The number of cells to have in the list before popping off stale ones
	 */
	DWORD MaxCellCount;
	/**
	 * The current size of the cell list
	 */
	DWORD CurrentCellCount;

public:
	/**
	 * Constructor. Sets the max cell count to the specified value
	 *
	 * @param InMaxCellCount the max cells to have before popping them off
	 */
	WxBoundedHtmlContainerCell(DWORD InMaxCellCount,wxHtmlContainerCell* Parent) :
		MaxCellCount(InMaxCellCount), CurrentCellCount(0),
		wxHtmlContainerCell(Parent)
	{
	}

	/**
	 * Handles adding a cell (or string of cells) to the list. If the additions
	 * go beyond the maximum specified, the oldest ones are removed
	 *
	 * @param Cell the new set of cells to append to our container
	 */
    void InsertCell(wxHtmlCell* Cell);
};

/**
 * This window class handles displaying the last N number of HTML lines, where
 * N is configurable.
 */
class WxBoundedHtmlWindow : public wxHtmlWindow
{
protected:
	/**
	 * The container that only holds N number of cells before purging the oldest
	 */
	WxBoundedHtmlContainerCell* ContainerCell;

public:
	/**
	 * Constructor that creates the bounded cell container and puts that in
	 * the html window's cell list
	 *
	 * @param InMaxCellsToDisplay the number of lines to display
	 * @param InParent the parent of this window
	 * @param InID the ID to assign to the new window
	 */
	WxBoundedHtmlWindow(DWORD InMaxCellsToDisplay,wxWindow* InParent,
		wxWindowID InID);
};

/*-----------------------------------------------------------------------------
	WxHtmlWindow
-----------------------------------------------------------------------------*/

class WxHtmlWindow : public WxBoundedHtmlWindow
{
public:
	WxHtmlWindow( DWORD InMaxCellsToDisplay, wxWindow* InParent, wxWindowID InID = -1 );

	void AppendLine( const FString& InLine );

protected:
	void ScrollToBottom();
	void OnSize( wxSizeEvent& In );

	DECLARE_EVENT_TABLE();
};

/*-----------------------------------------------------------------------------
	WxLogWindowFrame
-----------------------------------------------------------------------------*/

class WxLogWindowFrame : public wxFrame
{
public:
	DECLARE_DYNAMIC_CLASS(WxLogWindowFrame);

	class WxLogWindow* LogWindow;

	WxLogWindowFrame()
	{}
	WxLogWindowFrame( wxWindow* parent );
	virtual ~WxLogWindowFrame();

	void OnSize( wxSizeEvent& In );

	DECLARE_EVENT_TABLE();
};

/*-----------------------------------------------------------------------------
	WxLogWindow
-----------------------------------------------------------------------------*/

class WxLogWindow : public wxPanel, public FOutputDevice
{
public:
	DECLARE_DYNAMIC_CLASS(WxLogWindow);

	WxLogWindow()
	{}
	WxLogWindow( wxWindow* InParent );
	virtual ~WxLogWindow();

	wxPanel* Panel;
	WxHtmlWindow* HTMLWindow; 
	wxComboBox* CommandCombo;

	virtual void Serialize( const TCHAR* V, EName Event );
	void ExecCommand();

	void OnSize( wxSizeEvent& In );
	void OnExecCommand( wxCommandEvent& In );

	DECLARE_EVENT_TABLE();
};

#endif // __LOG_H__
