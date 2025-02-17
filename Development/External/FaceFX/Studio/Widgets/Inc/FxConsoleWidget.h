//------------------------------------------------------------------------------
// A widget to view the HTML console output in real-time.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxConsoleWidget_H__
#define FxConsoleWidget_H__

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/wxhtml.h"
#endif

#include "FxConsole.h"
#include "FxConsoleVariable.h"
#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

// A widget for an FxConsole view.
class FxConsoleWidget : public FxConsoleView
{
public:
	// Constructor.
	FxConsoleWidget();
	// Destructor.
	virtual ~FxConsoleWidget();

	// Initialization.
	void Initialize( wxWindow* parent = NULL );
	void ClearHtmlWindowText( void );

	// Append HTML text to the view.
	virtual void Append( const FxString& source );
	virtual void AppendW( const FxWString& source );

	// Returns the wxHtmlWindow.
	wxHtmlWindow* GetHtmlWindow( void );

protected:
	// The parent.
	wxWindow* _parent;
	// The HTML log window.
	class FxHtmlLogWindow : public wxHtmlWindow
	{
	public:
		// Constructor.
		FxHtmlLogWindow( FxConsoleWidget* consoleWidget, wxWindow* parent = NULL )
			: wxHtmlWindow(parent)
			, _pConsoleWidget(consoleWidget)
			, _numCells(0)
			, cw_historylength(FxVM::FindConsoleVariable("cw_historylength"))
		{
			FxAssert(cw_historylength);
		}

		void AppendLine( const wxString& line )
		{
			if( _pConsoleWidget )
			{
				FxInt32 historyLength = static_cast<FxInt32>(*cw_historylength);
				if( _numCells >= historyLength )
				{
					_pConsoleWidget->ClearHtmlWindowText();
					_numCells = 0;
				}
			}
			wxClientDC dc(this);
			dc.SetMapMode(wxMM_TEXT);
			wxHtmlWinParser p2(this);
			p2.SetFS(m_FS);
			p2.SetDC(&dc);
			wxHtmlContainerCell* c2 = (wxHtmlContainerCell*)p2.Parse(line);
			m_Cell->InsertCell(c2);
			_numCells++;
			CreateLayout();
			ScrollToBottom();
			Refresh();
		}

		void ScrollToBottom( void )
		{
			FxInt32 x, y;
			GetVirtualSize(&x, &y);
			Scroll(0, y);
			CalcScrolledPosition(0, m_Cell->GetHeight(), NULL, NULL);
		}

	protected:
		// A pointer to the console widget that contains the FxHtmlLogWindow.
		FxConsoleWidget* _pConsoleWidget;
		// The number of html cells added to the FxHtmlLogWindow.
		FxInt32 _numCells;
		// The cw_historylength console variable.
		FxConsoleVariable* cw_historylength;
	};
	FxHtmlLogWindow* _htmlWin;
};

} // namespace Face

} // namespace OC3Ent

#endif
