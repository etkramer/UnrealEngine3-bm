// DebugContext.h : Declaration of the CDebugContext

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



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif



// CDebugContext

class ATL_NO_VTABLE CDebugContext : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDebugCodeContext2,
    public IDebugDocumentContext2
{
public:
	CDebugContext()
	{
	}


BEGIN_COM_MAP(CDebugContext)
    COM_INTERFACE_ENTRY(IDebugCodeContext2)
    COM_INTERFACE_ENTRY(IDebugDocumentContext2)
END_COM_MAP()

    ////////////////////////////////////////////////////////////
    // IDebugCodeContext2 methods
    STDMETHOD(GetDocumentContext)(IDebugDocumentContext2 **ppSrcCxt);
    // GetLanguageInfo is declared in the IDebugDocumentContext2 interface.

    ////////////////////////////////////////////////////////////
    // IDebugCodeContext2 methods inherited from IDebugMemoryContext2
    STDMETHOD(GetName)(BSTR* pbstrName);
    STDMETHOD(GetInfo)(CONTEXT_INFO_FIELDS dwFields, CONTEXT_INFO* pInfo);
    STDMETHOD(Add)(UINT64 dwCount, IDebugMemoryContext2** ppMemCxt);
    STDMETHOD(Subtract)(UINT64 dwCount, IDebugMemoryContext2** ppMemCxt);
    STDMETHOD(Compare)(
        CONTEXT_COMPARE compare,
        IDebugMemoryContext2** rgpMemoryContextSet,
        DWORD dwMemoryContextSetLen,
        DWORD* pdwMemoryContext);

    ////////////////////////////////////////////////////////////
    // IDebugDocumentContext2 methods
    STDMETHOD(GetDocument)(IDebugDocument2 **ppDocument);
    STDMETHOD(GetName)(GETNAME_TYPE gnType, BSTR *pbstrFileName);
    STDMETHOD(EnumCodeContexts)(IEnumDebugCodeContexts2 **ppEnumCodeCxts);
    STDMETHOD(GetLanguageInfo)(BSTR* pbstrLanguage, GUID* pguidLanguage);
    STDMETHOD(GetStatementRange)(
        TEXT_POSITION* pBegPosition,
        TEXT_POSITION* pEndPosition);
    STDMETHOD(GetSourceRange)(TEXT_POSITION* pBegPosition, TEXT_POSITION* pEndPosition);
    STDMETHOD(Compare)(
        DOCCONTEXT_COMPARE compare,
        IDebugDocumentContext2** rgpDocContextSet,
        DWORD dwDocContextSetLen,
        DWORD* pdwDocContext);
    STDMETHOD(Seek)(int ncount, IDebugDocumentContext2 **ppDocContext);


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
	CComBSTR m_sbstrFileName;
	TEXT_POSITION m_pos;
public:
	void Initialize(LPCWSTR pszFileName, TEXT_POSITION& pos);
};


// CBreakpointResolution

class ATL_NO_VTABLE CBreakpointResolution : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDebugBreakpointResolution2
{
public:
    CBreakpointResolution();
    virtual ~CBreakpointResolution();


BEGIN_COM_MAP(CBreakpointResolution)
    COM_INTERFACE_ENTRY(IDebugBreakpointResolution2)
END_COM_MAP()

    ////////////////////////////////////////////////////////////
    // IDebugBreakpointResolution2
    STDMETHOD(GetBreakpointType)(BP_TYPE* pBPType);
    STDMETHOD(GetResolutionInfo)(
        BPRESI_FIELDS dwFields,
        BP_RESOLUTION_INFO* pBPResolutionInfo);


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
	BP_RESOLUTION_INFO m_bpResolutionInfo;
public:
	void Initialize(IDebugCodeContext2 * pCodeContext);
};

