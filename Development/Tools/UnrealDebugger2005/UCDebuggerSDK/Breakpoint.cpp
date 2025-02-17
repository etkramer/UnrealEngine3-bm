// Breakpoint.cpp : Implementation of CPendingBreakpoint

#include "stdafx.h"
#include "Breakpoint.h"
#include "EventBase.h"
#include "Context.h"
#include "misc.h"

// Last breakpoint that was bound.
extern CApartmentSafePointer<IDebugBoundBreakpoint2> gBoundBreakpoint(NULL);


// CPendingBreakpoint

//////////////////////////////////////////////////////////////////////////////
// IDebugPendingBreakpoint2
HRESULT CPendingBreakpoint::CanBind(IEnumDebugErrorBreakpoints2** ppErrorEnum)
{ return E_NOTIMPL; }
HRESULT CPendingBreakpoint::GetState(PENDING_BP_STATE_INFO* pState)
{ return E_NOTIMPL; }
HRESULT CPendingBreakpoint::Virtualize(BOOL fVirtualize)
{ return S_OK; }
HRESULT CPendingBreakpoint::Enable(BOOL fEnable)
{ return S_OK; }
HRESULT CPendingBreakpoint::SetCondition(BP_CONDITION bpCondition)
{ return E_NOTIMPL; }
HRESULT CPendingBreakpoint::SetPassCount(BP_PASSCOUNT bpPassCount)
{ return E_NOTIMPL; }
HRESULT CPendingBreakpoint::EnumBoundBreakpoints(IEnumDebugBoundBreakpoints2** ppEnum)
{ return E_NOTIMPL; }
HRESULT CPendingBreakpoint::EnumErrorBreakpoints(BP_ERROR_TYPE bpErrorType, IEnumDebugErrorBreakpoints2** ppEnum)
{ return E_NOTIMPL; }
HRESULT CPendingBreakpoint::Delete(void)
{ 
	WCHAR cmd[512];
	PkgClsFromFile(m_sbstrDoc, m_pkg, m_cls);
	wsprintfW(cmd, L"removebreakpoint %s.%s %d", m_pkg, m_cls, 
		m_posBeg.dwLine+1);
	SendCommandToUC(1, cmd);
	return S_OK; 
}

HRESULT CPendingBreakpoint::GetBreakpointRequest(IDebugBreakpointRequest2** ppBPRequest)
{
    *ppBPRequest = m_spBPRequest;
    (*ppBPRequest)->AddRef();

    return S_OK;
}

HRESULT CPendingBreakpoint::Bind(void)
{
    BP_REQUEST_INFO bpRequestInfo;
    HRESULT hr = m_spBPRequest->GetRequestInfo(BPREQI_ALLFIELDS, &bpRequestInfo);
    if (FAILED(hr)) return hr;

    if (bpRequestInfo.dwFields & BPREQI_BPLOCATION &&
        bpRequestInfo.bpLocation.bpLocationType == BPLT_CODE_FILE_LINE) {
    
        if (NULL == bpRequestInfo.bpLocation.bpLocation.bplocCodeFileLine.pDocPos)
            return E_UNEXPECTED;

        // Get the file name of the document.
        bpRequestInfo.bpLocation.bpLocation.bplocCodeFileLine.pDocPos->
            GetFileName(&m_sbstrDoc);
        // Get the position in the document
        bpRequestInfo.bpLocation.bpLocation.bplocCodeFileLine.pDocPos->
            GetRange(&m_posBeg, NULL);
	  // Get classname from file;
	  PkgClsFromFile(m_sbstrDoc, m_pkg, m_cls);
	  WCHAR cmd[512];
	  wsprintfW(cmd, L"addbreakpoint %s.%s %d", m_pkg, m_cls, 
		  m_posBeg.dwLine+1	);
	  SendCommandToUC(1, cmd);

        // Create a context
        CComObject<CDebugContext>* pContext;
        hr = CComObject<CDebugContext>::CreateInstance(&pContext);
        if (FAILED(hr)) return hr;
        // Create an automatic object to do the AddRef and release it.
        CComPtr<IUnknown> srpContextUnk;
        hr = pContext->QueryInterface(IID_IUnknown, (void**)&srpContextUnk);
        if (FAILED(hr)) return hr;

        pContext->Initialize(m_sbstrDoc, m_posBeg);

        // Create a breakpoint resolution
        CComObject<CBreakpointResolution>* pBPRes;
        hr = CComObject<CBreakpointResolution>::CreateInstance(&pBPRes);
        if (FAILED(hr)) return hr;
        // Create an automatic object to do the AddRef and release it.
        CComPtr<IUnknown> srpBPResUnk;
        hr = pBPRes->QueryInterface(IID_IUnknown, (void**)&srpBPResUnk);
        if (FAILED(hr)) return hr;

        pBPRes->Initialize(pContext);

        // Create a bound breakpoint
        CComObject<CBoundBreakpoint>* pBoundBP;
        hr = CComObject<CBoundBreakpoint>::CreateInstance(&pBoundBP);
        if (FAILED(hr)) return hr;
        // Create an automatic object to do the AddRef and release it.
        CComPtr<IUnknown> srpBoundBPUnk;
        hr = pBoundBP->QueryInterface(IID_IUnknown, (void**)&srpBoundBPUnk);
        if (FAILED(hr)) return hr;

        pBoundBP->Initialize(this, pBPRes);

        SendBoundEvent(pBoundBP);
        
        // Save the last bound event.
        gBoundBreakpoint = pBoundBP;

        return S_OK;
    }
    return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////////
// CPendingBreakpoint methods.

void CPendingBreakpoint::Initialize(IDebugBreakpointRequest2 * pBPRequest, IDebugEventCallback2 * pCallback, IDebugEngine2 * pEngine)
{
    m_spCallback = pCallback;
    m_spEngine = pEngine;
    m_spBPRequest = pBPRequest;
}




//////////////////////////////////////////////////////////////////////////////
// CBoundBreakpoint

//////////////////////////////////////////////////////////////////////////////
// IDebugBoundBreakpoint2
HRESULT CBoundBreakpoint::GetHitCount(DWORD* pdwHitCount)
{ return E_NOTIMPL; }
HRESULT CBoundBreakpoint::Enable(BOOL fEnable)
{ return E_NOTIMPL; }
HRESULT CBoundBreakpoint::SetHitCount(DWORD dwHitCount)
{ return E_NOTIMPL; }
HRESULT CBoundBreakpoint::SetCondition(BP_CONDITION bpCondition)
{ return E_NOTIMPL; }
HRESULT CBoundBreakpoint::SetPassCount(BP_PASSCOUNT bpPassCount)
{ return E_NOTIMPL; }
HRESULT CBoundBreakpoint::Delete(void)
{
		return S_OK;
}

HRESULT CBoundBreakpoint::GetState(BP_STATE* pState)
{
	HRESULT hr = E_POINTER;
	if (pState != NULL)
	{
		*pState = BPS_NONE;
		if ( gBoundBreakpoint )
		{
			*pState = BPS_ENABLED;
		}
		hr = S_OK;
	}
	return hr;
}

HRESULT CBoundBreakpoint::GetPendingBreakpoint(
    IDebugPendingBreakpoint2** ppPendingBreakpoint)
{
    if (NULL == ppPendingBreakpoint)
        return E_POINTER;
    *ppPendingBreakpoint = m_spPendingBP;
    (*ppPendingBreakpoint)->AddRef();
    return S_OK;
}

HRESULT CBoundBreakpoint::GetBreakpointResolution(
    IDebugBreakpointResolution2** ppBPResolution)
{
   if (NULL == ppBPResolution)
       return E_POINTER;
   *ppBPResolution = m_spBPRes;
   (*ppBPResolution)->AddRef();
   return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CBoundBreakpoint methods.
HRESULT CBoundBreakpoint::Initialize(IDebugPendingBreakpoint2 * pPending, IDebugBreakpointResolution2 * pBPRes)
{
    m_spPendingBP = pPending;
    m_spBPRes = pBPRes;
    return S_OK;
}

void CPendingBreakpoint::SendBoundEvent(IDebugBoundBreakpoint2 * pBoundBP)
{
    CComObject<CEnumDebugBoundBreakpoints>* pBoundEnum;
    HRESULT hr = CComObject<CEnumDebugBoundBreakpoints>::CreateInstance(&pBoundEnum);
    if (FAILED(hr) || (NULL==pBoundEnum))
        return;
    pBoundEnum->AddRef();

    IDebugBoundBreakpoint2* rgpBoundBP[] = { pBoundBP };
    pBoundEnum->Init(rgpBoundBP, &(rgpBoundBP[1]), NULL, AtlFlagCopy);

    CBreakpointBoundEvent* pBoundEvent = 
         new CBreakpointBoundEvent(pBoundEnum, this);
    if (pBoundEvent)
    {
        pBoundEvent->SendEvent(m_spCallback, m_spEngine, NULL, NULL);
    }
    pBoundEnum->Release();
}
