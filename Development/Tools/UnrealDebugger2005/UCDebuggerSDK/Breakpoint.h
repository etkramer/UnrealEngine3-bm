
// Breakpoint.h : Declaration of the CPendingBreakpoint

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

#include "WTUnrealDbgSDKATL.h"

typedef CComEnumWithCount<
    IEnumDebugBoundBreakpoints2,
    &IID_IEnumDebugBoundBreakpoints2,
    IDebugBoundBreakpoint2*,
    _CopyInterface<IDebugBoundBreakpoint2>
> CEnumDebugBoundBreakpoints;


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif



// CPendingBreakpoint

class ATL_NO_VTABLE CPendingBreakpoint : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDebugPendingBreakpoint2
{
public:
	CPendingBreakpoint()
	{
	}


BEGIN_COM_MAP(CPendingBreakpoint)
	COM_INTERFACE_ENTRY(IDebugPendingBreakpoint2)
END_COM_MAP()

    ////////////////////////////////////////////////////////////
    // IDebugPendingBreakpoint2
    STDMETHOD(CanBind)(IEnumDebugErrorBreakpoints2** ppErrorEnum);
    STDMETHOD(Bind)(void);
    STDMETHOD(GetState)(PENDING_BP_STATE_INFO* pState);
    STDMETHOD(GetBreakpointRequest)(IDebugBreakpointRequest2** ppBPRequest);
    STDMETHOD(Virtualize)(BOOL fVirtualize);
    STDMETHOD(Enable)(BOOL fEnable);
    STDMETHOD(SetCondition)(BP_CONDITION bpCondition);
    STDMETHOD(SetPassCount)(BP_PASSCOUNT bpPassCount);
    STDMETHOD(EnumBoundBreakpoints)(IEnumDebugBoundBreakpoints2** ppEnum);
    STDMETHOD(EnumErrorBreakpoints)(
        BP_ERROR_TYPE bpErrorType,
        IEnumDebugErrorBreakpoints2** ppEnum);
    STDMETHOD(Delete)(void);


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
	CComPtr<IDebugEventCallback2> m_spCallback;
	CComPtr<IDebugEngine2> m_spEngine;
	CComPtr<IDebugBreakpointRequest2> m_spBPRequest;
public:
	void Initialize(IDebugBreakpointRequest2 * pBPRequest, IDebugEventCallback2 * pCallback, IDebugEngine2 * pEngine);
	void SendBoundEvent(IDebugBoundBreakpoint2 * pBoundBP);
protected:
	CComBSTR m_sbstrDoc;
	TEXT_POSITION m_posBeg;
	CComBSTR m_pkg;
	CComBSTR m_cls;

};


// CBoundBreakpoint

class ATL_NO_VTABLE CBoundBreakpoint : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDebugBoundBreakpoint2
{
public:
	CBoundBreakpoint()
	{
	}


BEGIN_COM_MAP(CBoundBreakpoint)
	COM_INTERFACE_ENTRY(IDebugBoundBreakpoint2)
END_COM_MAP()

    ////////////////////////////////////////////////////////////
    // IDebugBoundBreakpoint2
    STDMETHOD(GetPendingBreakpoint)(IDebugPendingBreakpoint2** ppPendingBreakpoint);
    STDMETHOD(GetState)(BP_STATE* pState);
    STDMETHOD(GetHitCount)(DWORD* pdwHitCount);
    STDMETHOD(GetBreakpointResolution)(IDebugBreakpointResolution2** ppBPResolution);
    STDMETHOD(Enable)(BOOL fEnable);
    STDMETHOD(SetHitCount)(DWORD dwHitCount);
    STDMETHOD(SetCondition)(BP_CONDITION bpCondition);
    STDMETHOD(SetPassCount)(BP_PASSCOUNT bpPassCount);
    STDMETHOD(Delete)(void);


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
	CComPtr<IDebugPendingBreakpoint2> m_spPendingBP;
	CComPtr<IDebugBreakpointResolution2> m_spBPRes;
public:
	HRESULT Initialize(IDebugPendingBreakpoint2 * pPending, IDebugBreakpointResolution2 * pBPRes);
};

extern CApartmentSafePointer<IDebugBoundBreakpoint2> gBoundBreakpoint;
