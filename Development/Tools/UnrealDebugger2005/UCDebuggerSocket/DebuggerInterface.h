// DebuggerInterface.h : main header file for the DebuggerInterface DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CDebuggerInterfaceApp
// See DebuggerInterface.cpp for the implementation of this class
//

class CDebuggerInterfaceApp : public CWinApp
{
public:
	CDebuggerInterfaceApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
