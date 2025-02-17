#include "StdAfx.h"
#include "eventbase.h"

typedef CComEnumWithCount<
    IEnumDebugBoundBreakpoints2,
    &IID_IEnumDebugBoundBreakpoints2,
    IDebugBoundBreakpoint2*,
    _CopyInterface<IDebugBoundBreakpoint2>
> CEnumDebugBoundBreakpoints;

// IUnknown methods implementation macro
#define IMPLEMENT_IUNKNOWN(T, I) \
ULONG T::AddRef(void) { return CEventBase::AddRef(); } \
ULONG T::Release(void) { return CEventBase::Release(); } \
HRESULT T::QueryInterface(REFIID riid, LPVOID *ppvObj) { \
    if (IID_##I == riid) { \
        *ppvObj = static_cast<I*>(this); AddRef(); return S_OK; \
    } \
    return CEventBase::QueryInterface(riid, ppvObj); \
}


CEventBase::CEventBase(REFIID iid, DWORD dwAttrib)
: m_dwAttrib(dwAttrib)
, m_iRefCount(0)
, m_riid(iid)
{
}

CEventBase::~CEventBase(void)
{
}

//////////////////////////////////////////////////////////////////////////////
// IUnknown methods
ULONG CEventBase::AddRef(void)
{
    return ++m_iRefCount;
}

ULONG CEventBase::Release(void)
{
    m_iRefCount--;
    if (m_iRefCount == 0) {
        delete this;
        return 0;
    }
    return m_iRefCount;
}

HRESULT CEventBase::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    if (riid == IID_IUnknown)
        *ppvObj = (IUnknown*) this;
    else if (riid == IID_IDebugEvent2)
        *ppvObj = (IDebugEvent2*) this;
    else return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// IDebugEvent2 methods
HRESULT CEventBase::GetAttributes(DWORD* pdwAttrib)
{
    if (NULL == pdwAttrib)
        return E_POINTER;
    *pdwAttrib = m_dwAttrib;
    return S_OK;
}

CEngineCreateEvent::CEngineCreateEvent(IDebugEngine2 *pEngine)
: CEventBase(IID_IDebugEngineCreateEvent2, EVENT_ASYNCHRONOUS)
{
    m_spEngine = pEngine;
}

CEngineCreateEvent::~CEngineCreateEvent(void)
{
}

//////////////////////////////////////////////////////////////////////////////
// IDebugEngineCreateEvent2
HRESULT CEngineCreateEvent::GetEngine(IDebugEngine2 **ppEngine)
{
    if (NULL == ppEngine)
        return E_POINTER;
    *ppEngine = m_spEngine;
    if (NULL != *ppEngine)
        (*ppEngine)->AddRef();
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// IUnknown
IMPLEMENT_IUNKNOWN(CEngineCreateEvent, IDebugEngineCreateEvent2)

CProgramCreateEvent::CProgramCreateEvent(void)
: CEventBase(IID_IDebugProgramCreateEvent2, EVENT_ASYNCHRONOUS)
{
}

CProgramCreateEvent::~CProgramCreateEvent(void)
{
}

//////////////////////////////////////////////////////////////////////////////
// IUnknown
IMPLEMENT_IUNKNOWN(CProgramCreateEvent, IDebugProgramCreateEvent2)

HRESULT CEventBase::SendEvent(IDebugEventCallback2 * pCallback, IDebugEngine2 * pEngine, IDebugProgram2 * pProgram, IDebugThread2 * pThread)
{
    AddRef();
    HRESULT hr = E_POINTER;
    if (NULL != pCallback)
    {
        // Send the event to the SDM
        hr = pCallback->Event(
            pEngine, NULL, pProgram, pThread, (IDebugEvent2 *) this,
            m_riid, m_dwAttrib);
    }

    // If event is synchronous, pump messages and wait for continuation.
    if (SUCCEEDED(hr) && (m_dwAttrib == EVENT_SYNCHRONOUS))
    {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (msg.message == WM_CONTINUE_SYNC_EVENT)
                break;
            DispatchMessage(&msg);
        }
    }
    Release();
    return hr;
}

CLoadCompleteEvent::CLoadCompleteEvent(void)
: CEventBase(IID_IDebugLoadCompleteEvent2, EVENT_SYNC_STOP)
{
}

CLoadCompleteEvent::~CLoadCompleteEvent(void)
{
}
//////////////////////////////////////////////////////////////////////////////
// IUnknown
IMPLEMENT_IUNKNOWN(CLoadCompleteEvent, IDebugLoadCompleteEvent2)


CEntryPointEvent::CEntryPointEvent(void)
: CEventBase(IID_IDebugEntryPointEvent2, EVENT_SYNC_STOP)
{
}

CEntryPointEvent::~CEntryPointEvent(void)
{
}
//////////////////////////////////////////////////////////////////////////////
// IUnknown
IMPLEMENT_IUNKNOWN(CEntryPointEvent, IDebugEntryPointEvent2)

CBreakpointBoundEvent::CBreakpointBoundEvent(
   IEnumDebugBoundBreakpoints2 *pEnum, 
   IDebugPendingBreakpoint2 *pPending)
: CEventBase(IID_IDebugBreakpointBoundEvent2, EVENT_ASYNCHRONOUS)
{
   m_spEnum = pEnum;
   m_spPending = pPending;
}

CBreakpointBoundEvent::~CBreakpointBoundEvent(void)
{
}

//////////////////////////////////////////////////////////////////////////////
// IUnknown
IMPLEMENT_IUNKNOWN(CBreakpointBoundEvent, IDebugBreakpointBoundEvent2)

//////////////////////////////////////////////////////////////////////////////
// IDebugBreakpointBoundEvent2
HRESULT CBreakpointBoundEvent::GetPendingBreakpoint(IDebugPendingBreakpoint2** ppPendingBP) {
   if (NULL == ppPendingBP)
      return E_POINTER;
    // Get the pending breakpoint that was bound to the breakpoint
   *ppPendingBP = m_spPending;
   if (NULL != *ppPendingBP)
      (*ppPendingBP)->AddRef();
   return S_OK;
}

HRESULT CBreakpointBoundEvent::EnumBoundBreakpoints(IEnumDebugBoundBreakpoints2** ppEnum) {
   if (NULL == ppEnum)
      return E_POINTER;
    // Get the list of breakpoints bound on this event
   *ppEnum = m_spEnum;
   if (NULL != *ppEnum)
      (*ppEnum)->AddRef();
   return S_OK;
}

CCodeBreakpointEvent::CCodeBreakpointEvent(IDebugBoundBreakpoint2 *pBP)
: CEventBase(IID_IDebugBreakpointEvent2, EVENT_SYNC_STOP)
{
   m_spBP = pBP;
}

CCodeBreakpointEvent::~CCodeBreakpointEvent(void)
{
}

//////////////////////////////////////////////////////////////////////////////
// IUnknown
IMPLEMENT_IUNKNOWN(CCodeBreakpointEvent, IDebugBreakpointEvent2)

//////////////////////////////////////////////////////////////////////////////
// IDebugBreakpointEvent2
HRESULT CCodeBreakpointEvent::EnumBreakpoints(IEnumDebugBoundBreakpoints2 **ppEnum)
{
    // Check the out parameter
    if (NULL == ppEnum)
        return E_POINTER;

    // Create the bound enumerator
    CComObject<CEnumDebugBoundBreakpoints>* pBoundEnum;
    HRESULT hr = CComObject<CEnumDebugBoundBreakpoints>::CreateInstance(&pBoundEnum);
    if (FAILED(hr))
       return hr;

    IDebugBoundBreakpoint2* rgpBoundBP[] = { m_spBP };
    pBoundEnum->Init(rgpBoundBP, &(rgpBoundBP[1]), NULL, AtlFlagCopy);

    hr = pBoundEnum->QueryInterface(IID_IEnumDebugBoundBreakpoints2, (void**)ppEnum);
    if ( FAILED(hr) )
        delete pBoundEnum;

    return hr;
}

CProgramDestroyEvent::CProgramDestroyEvent(DWORD dwExitCode)
: CEventBase(IID_IDebugProgramDestroyEvent2, EVENT_ASYNCHRONOUS)
{
   m_dwExitCode = dwExitCode;
}

CProgramDestroyEvent::~CProgramDestroyEvent(void)
{
}
//////////////////////////////////////////////////////////////////////////////
// IUnknown
IMPLEMENT_IUNKNOWN(CProgramDestroyEvent, IDebugProgramDestroyEvent2)

//////////////////////////////////////////////////////////////////////////////
// IDebugProgramDestroyEvent2
HRESULT CProgramDestroyEvent::GetExitCode(DWORD *pdwExit) {
   if (NULL == pdwExit)
      return E_POINTER;
   *pdwExit = m_dwExitCode;
   return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// IDebugExpressionEvaluationCompletEvent2
CExpressionEvaluationCompleteEvent::CExpressionEvaluationCompleteEvent(IDebugExpression2 *pExpression, IDebugProperty2 *pResult)
: CEventBase(IID_IDebugExpressionEvaluationCompleteEvent2,EVENT_ASYNCHRONOUS)
{
    m_spExpression = pExpression;
    m_spResult = pResult;
}

CExpressionEvaluationCompleteEvent::~CExpressionEvaluationCompleteEvent(void)
{
}

HRESULT CExpressionEvaluationCompleteEvent::GetResult(IDebugProperty2 **ppResult) 
{
    HRESULT retval = E_INVALIDARG;
    if (ppResult != NULL) {
        *ppResult = m_spResult;
        retval = S_OK;
    }
    return retval;
}
HRESULT CExpressionEvaluationCompleteEvent::GetExpression(IDebugExpression2 **ppExpr) 
{
    HRESULT retval = E_INVALIDARG;
    if (ppExpr != NULL) {
        *ppExpr = m_spExpression;
        retval = S_OK;
    }
    return retval;
}
//////////////////////////////////////////////////////////////////////////////
// IUnknown
IMPLEMENT_IUNKNOWN(CExpressionEvaluationCompleteEvent, IDebugExpressionEvaluationCompleteEvent2)

IMPLEMENT_IUNKNOWN(CDebugBreakEvent, IDebugBreakEvent2)
IMPLEMENT_IUNKNOWN(CDebugOutputStringEvent2, IDebugOutputStringEvent2);
