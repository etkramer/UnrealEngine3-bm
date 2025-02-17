// Expression.h : Declaration of the CExpression

#pragma once
#include "resource.h"       // main symbols

#include "UCDebuggerSDK.h"
#include <ee.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif



// CExpression

class ATL_NO_VTABLE CExpression : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDebugExpression2
{
public:
	CExpression()
	{
	}


BEGIN_COM_MAP(CExpression)
    COM_INTERFACE_ENTRY(IDebugExpression2)
END_COM_MAP()

	STDMETHOD(Abort)(void);
	STDMETHOD(EvaluateSync)(
		EVALFLAGS dwFlags,
		DWORD dwTimeout,
		IDebugEventCallback2 *pExprCallback,
		IDebugProperty2 **ppResult);
	STDMETHOD(EvaluateAsync)(
		EVALFLAGS dwFlags,
		IDebugEventCallback2 *pExprCallback);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

public:
	void Init(IDebugParsedExpression *pParsedExpression,
		PARSEFLAGS parseFlags,
		IDebugSymbolProvider *pProvider,
		IDebugBinder *pBinder,
		IDebugAddress *pAddress,
		IDebugEventCallback2 *pCallback,
		IDebugEngine2 *pEngine,
		IDebugProgram2 *pProgram,
		IDebugThread2 *pThread);
protected:
	CComPtr<IDebugBinder> m_spBinder;
	CComPtr<IDebugAddress> m_spAddress;
	CComPtr<IDebugSymbolProvider> m_spSymbolProvider;
	CComPtr<IDebugParsedExpression> m_spParsedExpression;
	CComPtr<IDebugEventCallback2> m_spCallback;
	CComPtr<IDebugEngine2> m_spEngine;
	CComPtr<IDebugProgram2> m_spProgram;
	CComPtr<IDebugThread2> m_spThread;
	PARSEFLAGS m_parseFlags;
};

