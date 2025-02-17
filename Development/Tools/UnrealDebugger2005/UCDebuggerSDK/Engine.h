// Engine.h : Declaration of the CEngine

#pragma once
#include "resource.h"       // main symbols

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif
#ifdef POCKETPC2003_UI_MODEL
#include "resourceppc.h"
#endif 
#ifdef SMARTPHONE2003_UI_MODEL
#include "resourcesp.h"
#endif
#ifdef AYGSHELL_UI_MODEL
#include "resourceayg.h"
#endif

#include "UCDebuggerSDK.h"
#include "Program.h"
#include <ee.h>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif



// CEngine

class ATL_NO_VTABLE CEngine : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CEngine, &CLSID_Engine>,
	public IDebugEngine2,
	public IDebugExpressionContext2
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_ENGINE)


BEGIN_COM_MAP(CEngine)
	COM_INTERFACE_ENTRY(IDebugEngine2)
	COM_INTERFACE_ENTRY(IDebugExpressionContext2)
END_COM_MAP()

    ////////////////////////////////////////////////////////////
    //IDebugEngine2
    STDMETHOD(EnumPrograms)(IEnumDebugPrograms2** ppEnum);
    STDMETHOD(Attach)(
        IDebugProgram2 **rgpPrograms,
        IDebugProgramNode2 **rgpProgramNodes,
        DWORD celtPrograms,
        IDebugEventCallback2 *pCallback,
        ATTACH_REASON dwReason);
    STDMETHOD(CreatePendingBreakpoint)(
        IDebugBreakpointRequest2 *pBPRequest,
        IDebugPendingBreakpoint2** ppPendingBP);
    STDMETHOD(SetException)(EXCEPTION_INFO* pException);
    STDMETHOD(RemoveSetException)(EXCEPTION_INFO* pException);
    STDMETHOD(RemoveAllSetExceptions)(REFGUID guidType);
    STDMETHOD(GetEngineId)(GUID *pguidEngine);
    STDMETHOD(DestroyProgram)(IDebugProgram2* pProgram);
    STDMETHOD(ContinueFromSynchronousEvent)(IDebugEvent2* pEvent);
    STDMETHOD(SetLocale)(WORD wLangID);
    STDMETHOD(SetRegistryRoot)(LPCOLESTR pszRegistryRoot);
    STDMETHOD(SetMetric)(LPCOLESTR pszMetric, VARIANT varValue);
    STDMETHOD(CauseBreak)( void);
	//STDMETHOD(SetLocale)(WORD wLangID);

	//////////////////////////////////////////////////////////////////////////
	// IDebugExpressionContext2 methods
	STDMETHOD(GetName)(BSTR *pbstrName);
	STDMETHOD(ParseText)(
		LPCOLESTR pszCode,
		PARSEFLAGS dwFlags,
		UINT nRadix,
		IDebugExpression2 **ppExpr,
		BSTR *pbstrError,
		UINT *pichError);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

public:

protected:
	CComPtr<IDebugProgramNode2>         m_spProgram;
    CComPtr<IDebugEventCallback2>       m_spCallback;
	CComPtr<IDebugSymbolProvider>       m_spSymbolProvider;
	CComPtr<IDebugExpressionEvaluator>  m_spExpressionEvaluator;
};

OBJECT_ENTRY_AUTO(__uuidof(Engine), CEngine)
extern CEngine *g_Engine;

